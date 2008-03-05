#!/usr/bin/env python

import gammu

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

status = sm.GetSMSStatus()

remain = status['SIMUsed'] + status['PhoneUsed'] + status['TemplatesUsed']

start = True

while remain > 0:
    if start:
        sms = sm.GetNextSMS(Start = True, Folder = 0)
        start = False
    else:
        sms = sm.GetNextSMS(Location = sms[0]['Location'], Folder = 0)
    remain = remain - len(sms)

    for m in sms:
        print
        print '%-15s: %s' % ('Number', m['Number'])
        print '%-15s: %s' %  ('Date', str(m['DateTime']))
        print '%-15s: %s' % ('State', m['State'])
        print '\n%s' % m['Text']
