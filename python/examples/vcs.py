#!/usr/bin/env python

# Example for reading data from phone and convering it to and from
# vCard, vTodo, vCalendar

import gammu
import sys

sm = gammu.StateMachine()
if len(sys.argv) == 2:
    sm.ReadConfig(Filename = sys.argv[1])
else:
    sm.ReadConfig()
sm.Init()

# For calendar entry

# Read entry from phone
entry = sm.GetNextCalendar(Start=True)

# Convert it to vCard
vc_entry = gammu.EncodeVCALENDAR(entry)
ic_entry = gammu.EncodeICALENDAR(entry)

# Convert it back to entry
entry2 = gammu.DecodeVCS(vc_entry)
entry3 = gammu.DecodeICS(ic_entry)

# For todo entry

# Read entry from phone
entry = sm.GetNextToDo(Start=True)

# Convert it to vCard
vt_entry = gammu.EncodeVTODO(entry)
it_entry = gammu.EncodeITODO(entry)

# Convert it back to entry
entry2 = gammu.DecodeVCS(vt_entry)
entry3 = gammu.DecodeICS(it_entry)

# For memory entry

# Read entry from phone
entry = sm.GetNextMemory(Start=True, Type = 'ME')

# Convert it to vCard
vc_entry = gammu.EncodeVCARD(entry)

# Convert it back to entry
entry2 = gammu.DecodeVCARD(vc_entry)

