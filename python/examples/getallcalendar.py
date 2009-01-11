#!/usr/bin/env python

import gammu

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

status = sm.GetCalendarStatus()

remain = status['Used']

start = True

while remain > 0:
    if start:
        entry = sm.GetNextCalendar(Start = True)
        start = False
    else:
        entry = sm.GetNextCalendar(Location = entry['Location'])
    remain = remain - 1

    print
    print '%-20s: %d' % ('Location',entry['Location'])
    print '%-20s: %s' % ('Type',entry['Type'])
    for v in entry['Entries']:
        print '%-20s: %s' % (v['Type'], str(v['Value']))
