#!/usr/bin/env python

import gammu

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()

status = sm.GetSMSStatus()

remain = status['SIMUsed'] + status['PhoneUsed'] + status['TemplatesUsed']

sms = []
start = True

while remain > 0:
    if start:
        cursms = sm.GetNextSMS(Start = True, Folder = 0)
        start = False
    else:
        cursms = sm.GetNextSMS(Location = cursms[0]['Location'], Folder = 0)
    remain = remain - len(cursms)
    sms.append(cursms)

data = gammu.LinkSMS(sms)

for x in data:
    v = gammu.DecodeSMS(x)

    m = x[0]
    print
    print '%-15s: %s' % ('Number', m['Number'])
    print '%-15s: %s' % ('Date', str(m['DateTime']))
    print '%-15s: %s' % ('State', m['State'])
    print '%-15s: %s' % ('Folder', m['Folder'])
    print '%-15s: %s' % ('Validity', m['SMSC']['Validity'])
    loc = []
    for m in x:
        loc.append(str(m['Location']))
    print '%-15s: %s' % ('Location(s)', ', '.join(loc))
    if v == None:
        print '\n%s' % m['Text']
    else:
        for e in v['Entries']:
            print
            print '%-15s: %s' % ('Type', e['ID'])
            if e['Bitmap'] != None:
                for bmp in e['Bitmap']:
                    print 'Bitmap:'
                    for row in bmp['XPM'][3:]:
                        print row
                print
            if e['Buffer'] != None:
                print 'Text:'
                print e['Buffer']
                print
