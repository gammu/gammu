#!/usr/bin/env python

from __future__ import print_function # keep compatibility with python3
import gammu
import os
import errno
import re

def createFolderIfNotExist(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def getInternationalizedNumber(number):
    if not number:
        return("Unknown")

    if number.startswith("0"):
        return(number.replace("0", "+49", 1))
    else:
        return(number)

def getFilename(mydir, mysms):
    if mysms[0]["DateTime"]:
        return(mysms[0]["DateTime"].strftime("%Y-%m-%d-%Hh%Mm%Ss"))

    # no date available so calculate unknown number
    myfiles = os.listdir(mydir)
    
    nextitem = 0
    for i in myfiles:
        mo = re.match("^Unknown-([0-9]*)", i)
        if mo and int(mo.group(1)) > nextitem:
            nextitem = int(mo.group(1))

    return("Unknown-" + str(nextitem + 1))

def saveSMS(mysms, allContacts):
    myNumber = getInternationalizedNumber(mysms[0]["Number"])

    try:
        mydir = allContacts[myNumber]
    except KeyError:
        mydir = myNumber

    createFolderIfNotExist(mydir)

    myfile = getFilename(mydir, mysms)

    f = open(os.path.join(mydir, myfile), "a")
    for i in mysms:
        f.write(i["Text"].encode("UTF-8"))
    f.write(u"\n".encode("UTF-8"))
    f.close()

def getContacts(myStMa):

    ## Get all contacts
    numContacts = myStMa.GetMemoryStatus(Type = "SM")["Used"]
    myContacts = dict()

    start = True

    try:
        while numContacts > 0:
            if start:
                entry = myStMa.GetNextMemory(Start = True, Type = "SM")
                start = False
            else:
                entry = myStMa.GetNextMemory(Location = entry["Location"], Type = "SM")
                numContacts = numContacts - 1
    
            numbers = list()
            for v in entry["Entries"]:
                if v["Type"] == "Text_FirstName":
                    name = v["Value"]
                else:
                    numbers.append(getInternationalizedNumber(v["Value"]))
    
            for v in numbers:
                myContacts[v] = name
    
    except gammu.ERR_EMPTY:
        # error is raised if memory is empty (this induces wrong reportet memory
        # status)
        pass

    return(myContacts)

def getAndDeleteAllSMS(myStMa):
    # Read SMS memory status ...
    memory = myStMa.GetSMSStatus()
    # ... and calculate number of messages
    numMessages = memory["SIMUsed"] + memory["PhoneUsed"]
    
    # Get all sms
    start = True
    entriesAll = list()
    
    try:
        while numMessages > 0:
            if start:
                entry = myStMa.GetNextSMS(Folder = 0, Start = True)
                start = False
            else:
                entry = myStMa.GetNextSMS(Folder = 0, Location = entry[0]["Location"])
        
            numMessages = numMessages - 1
            entriesAll.append(entry)
    
            # delete retrieved sms
            myStMa.DeleteSMS(Folder = 0, Location = entry[0]["Location"])
    
    except gammu.ERR_EMPTY:
        # error is raised if memory is empty (this induces wrong reportet memory
        # status)
        pass
    
    # Link all SMS when there are concatenated messages
    entriesAll = gammu.LinkSMS(entriesAll)

    return(entriesAll)


if __name__ == "__main__":

    # Get all contacts
    sm = gammu.StateMachine()
    sm.ReadConfig()
    sm.Init()
    myNumbers = getContacts(sm)
    sm.Terminate()

    # Get all sms
    # why in two steps? ERR_TIMOUT is raised without closing the connection
    sm = gammu.StateMachine()
    sm.ReadConfig()
    sm.Init()
    myMessages = getAndDeleteAllSMS(sm)
    sm.Terminate()

    for message in myMessages:
        saveSMS(message, myNumbers)
