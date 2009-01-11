#!/usr/bin/env python

import gammu

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

status = sm.GetToDoStatus()

remain = status['Used']

start = True

while remain > 0:
    if start:
        entry = sm.GetNextToDo(Start = True)
        start = False
    else:
        entry = sm.GetNextToDo(Location = entry['Location'])
    remain = remain - 1

    print
    print '%-15s: %d' % ('Location',entry['Location'])
    print '%-15s: %s' % ('Priority',entry['Priority'])
    for v in entry['Entries']:
        print '%-15s: %s' % (v['Type'], str(v['Value']))
