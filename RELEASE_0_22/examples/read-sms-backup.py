#!/usr/bin/env python

import gammu
import sys
import codecs

if len(sys.argv) != 2:
    print 'This requires parameter: backup file!'
    sys.exit(1)

#sm = gammu.StateMachine()
#sm.ReadConfig()
#sm.Init()

charsetencoder = codecs.getencoder(sys.getdefaultencoding())

filename = sys.argv[1]

backup = gammu.ReadSMSBackup(filename)

# Make nested array
list = [[x] for x in backup]

data = gammu.LinkSMS(list)

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
        print '\n%s' % charsetencoder(m['Text'], 'replace')[0]
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
                print charsetencoder(e['Buffer'], 'replace')
                print
