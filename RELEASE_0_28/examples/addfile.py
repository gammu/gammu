#!/usr/bin/env python

import gammu
import os
import datetime
import sys

if len(sys.argv) != 3:
    print 'This requires two parameters: file to upload and path!'
    sys.exit(1)

f = open(sys.argv[1] ,'r')
data = f.read()

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

# Check AddFilePart
print "\n\nExpection: Put cgi.jpg onto Memorycard on phone"
file_f = {
"ID_FullName": sys.argv[2],
"Name": os.path.basename(sys.argv[1]),
"Buffer": data,
"Protected": 0,
"ReadOnly": 0,
"Hidden": 0,
"System": 0,
'Finished': 0,
'Folder': 0,
'Level': 0,
'Type': 'Other',
"Pos": 0,
}
while (not file_f['Finished']):
    file_f = sm.AddFilePart(file_f)

