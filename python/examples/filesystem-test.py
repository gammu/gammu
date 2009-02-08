#!/usr/bin/env python
#
# This file should provide me with a test frame for the filesystem
# functions. It can't be run automatically, but you should be able
# to decide, wheather the output looks sensible
#
# BEWARE - the test WILL TOUCH AND WRITE FILESYSTEM!!
#
# Matthias Blaesing <matthias.blaesing@rwth-aachen.de>
#
# I asume you call the script from the directory were it lays and
# have the grafic there and you have write permission there and that
# there is a file called cgi.jpg to be used as test file
#
# READY:
# - DeleteFile
# - AddFilePart
# - GetFilePart
# - GetNextRootFolder
# - GetNextFileFolder
# - GetFolderListing
# - SetFileAttributes
# - DeleteFolder
# - GetFileSystemStatus
# - AddFolder

import gammu
import os
import datetime
import sys
from optparse import OptionParser

parser = OptionParser(usage = "usage: %prog [options]")

parser.add_option("-c", "--config",
                  action="store", type="string",
                  dest="config", default=None,
                  help="Config file path")
parser.add_option("-f", "--folder",
                  action="store", type="string",
                  dest="folder", default=None,
                  help="Folder to be used for testing")
parser.add_option("-t", "--test-file",
                  action="store", type="string",
                  dest="testfile", default="./data/cgi.jpg",
                  help="Local file to be used for testing")
(options, args) = parser.parse_args()

if options.folder is None:
    print "You have to select folder where testing will be done!"
    print "And even better, you should read the script before you run it."
    sys.exit(1)

if not os.path.exists(options.testfile):
    print "You have to select file which will be used for testing!"
    sys.exit(1)

sm = gammu.StateMachine()
if options.config is not None:
    sm.ReadConfig(Filename = options.config)
else:
    sm.ReadConfig()
sm.Init()

# Check GetFileSystemStatus
print "Expection: Info about filesystem usage"
try:
	fs_info = sm.GetFileSystemStatus()
	fs_info["Total"] = fs_info["Free"] + fs_info["Used"]
	print "Used: %(Used)d, Free: %(Free)d, Total: %(Total)d" % fs_info
except gammu.ERR_NOTSUPPORTED:
	print "You will have to live without this knowledge"

# Check DeleteFile
print "\n\nExpection: Deleting cgi.jpg from memorycard"
try:
	sm.DeleteFile(unicode(options.folder + "/cgi.jpg"))
except gammu.ERR_FILENOTEXIST:
	print "Oh well - we copy it now ;-) (You SHOULD read this)"

# Check AddFilePart
print "\n\nExpection: Put cgi.jpg onto Memorycard on phone"
file_handle = open(options.testfile, "r")
file_stat = os.stat(options.testfile)
ttime = datetime.datetime.fromtimestamp(file_stat[8])
file_f = {
"ID_FullName": options.folder,
"Name": u"cgi.jpg",
"Modified": ttime,
"Folder": 0,
"Level": 1,
"Used": file_stat[6],
"Buffer": file_handle.read(),
"Type": "Other",
"Protected": 0,
"ReadOnly": 0,
"Hidden": 0,
"System": 0,
"Handle": 0,
"Pos": 0,
"Finished": 0
}
while (not file_f["Finished"]):
	file_f = sm.AddFilePart(file_f)


# Check GetFilePart
print "\n\nExpection: Get cgi.jpg from memorycard and write it as test.jpg"
f = file('./test.jpg', 'w')
file_f = {
	"ID_FullName": options.folder + "/cgi.jpg",
	"Finished": 0
}
while (not file_f["Finished"]):
	file_f = sm.GetFilePart(file_f)
f.write(file_f["Buffer"])
f.flush();

# Check correct transfer
print "\n\nExpection: test.jpg and cgi.jpg to be the same"
f1 = open(options.testfile, "r")
f2 = open("./test.jpg", "r")
if(f1.read() == f2.read()):
	print "Same files"
else:
	print "Files differ!"

os.remove("./test.jpg")

# Check GetNextRootFolder
print "\n\nExpection: Root Folder List"
try:
    file = sm.GetNextRootFolder(u"");
    while 1:
        print file["ID_FullName"] + " - " + file["Name"]
        try:
            file = sm.GetNextRootFolder(file["ID_FullName"])
        except gammu.ERR_EMPTY:
            break
except gammu.ERR_NOTSUPPORTED:
    print "Not supported..."


# Check GetNextFileFolder
print "\n\nExpection: Info for a file of the phone (cgi.jpg)"
file_f = sm.GetNextFileFolder(1)
while 1:
	if(file_f["Name"] != "cgi.jpg"):
		file_f = sm.GetNextFileFolder(0)
	else:
		attribute = ""
		if file_f["Protected"]:
			attribute = attribute + "P"
		if file_f["ReadOnly"]:
			attribute = attribute + "R"
		if file_f["Hidden"]:
			attribute = attribute + "H"
		if file_f["System"]:
			attribute = attribute + "S"
		print "ID:         " + file_f["ID_FullName"] + "\n" + \
		      "Name:       " + file_f["Name"] + "\n" + \
		      "Folder:     " + str(file_f["Folder"]) + "\n" + \
		      "Used:       " + str(file_f["Used"]) + "\n" + \
		      "Modified:   " + file_f["Modified"].strftime("%x %X") + "\n" + \
		      "Type:       " + file_f["Type"] + "\n" + \
		      "Level:      " + str(file_f["Level"]) + "\n" + \
		      "Attribute:  " + attribute

		break

# Check SetFileAttributes
# Protected is spared, as my mobile nokia 6230i says it's unsupported
print "\n\nExpection: Modifying attributes (readonly=1, protected=0, system=1, hidden=1)"
sm.SetFileAttributes(unicode(options.folder + "/cgi.jpg"),1,0,1,1)

# Check GetFolderListing
print "\n\nExpection: Listing of cgi.jpg's properties"
file_f = sm.GetFolderListing(unicode(options.folder), 1)
while 1:
	if(file_f["Name"] != "cgi.jpg"):
		file_f = sm.GetFolderListing(unicode(options.folder), 0)
	else:
		attribute = ""
		if file_f["Protected"]:
			attribute = attribute + "P"
		if file_f["ReadOnly"]:
			attribute = attribute + "R"
		if file_f["Hidden"]:
			attribute = attribute + "H"
		if file_f["System"]:
			attribute = attribute + "S"
		print "ID:         " + file_f["ID_FullName"] + "\n" + \
		      "Name:       " + file_f["Name"] + "\n" + \
		      "Folder:     " + str(file_f["Folder"]) + "\n" + \
		      "Used:       " + str(file_f["Used"]) + "\n" + \
		      "Modified:   " + file_f["Modified"].strftime("%x %X") + "\n" + \
		      "Type:       " + file_f["Type"] + "\n" + \
		      "Level:      " + str(file_f["Level"]) + "\n" + \
		      "Attribute:  " + attribute

		break

# Check DeleteFile
print "\n\nExpection: Deletion of cgi.jpg from memorycard"
try:
	sm.DeleteFile(unicode(options.folder + "cgi.jpg"))
	print "Deleted"
except gammu.ERR_FILENOTEXIST:
	print "Something is wrong ..."

# Check AddFolder
print "\n\nExpection: Creation of a folder on the memorycard \"42alpha\""
file_f = sm.AddFolder(unicode(options.folder), u"42alpha")

# Check GetFolderListing again *wired*
print "\n\nExpection: Print properties of newly created folder"
file_f = sm.GetFolderListing(unicode(options.folder), 1)
while 1:
	if(file_f["Name"] != "42alpha"):
		file_f = sm.GetFolderListing(unicode(options.folder), 0)
	else:
		attribute = ""
		if file_f["Protected"]:
			attribute = attribute + "P"
		if file_f["ReadOnly"]:
			attribute = attribute + "R"
		if file_f["Hidden"]:
			attribute = attribute + "H"
		if file_f["System"]:
			attribute = attribute + "S"
		print "ID:         " + file_f["ID_FullName"] + "\n" + \
		      "Name:       " + file_f["Name"] + "\n" + \
		      "Folder:     " + str(file_f["Folder"]) + "\n" + \
		      "Used:       " + str(file_f["Used"]) + "\n" + \
		      "Modified:   " + file_f["Modified"].strftime("%x %X") + "\n" + \
		      "Type:       " + file_f["Type"] + "\n" + \
		      "Level:      " + str(file_f["Level"]) + "\n" + \
		      "Attribute:  " + attribute

		break

# Check DeleteFolder
print "\n\nExpection: Deletion of previously created folder \"42alpha\""
sm.DeleteFolder(unicode(options.folder + "/42alpha"))
