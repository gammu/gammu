#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
python-gammu - Test script to test several Gammu operations (usually using dummy driver, but it depends on config)
'''
__author__ = 'Michal Čihař'
__email__ = 'michal@cihar.com'
__license__ = '''
Copyright © 2003 - 2009 Michal Čihař

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
'''

import gammu
import sys
if len(sys.argv) != 2:
    print 'This requires one parameter with location of config file!'
    sys.exit(1)

sm = gammu.StateMachine()
sm.ReadConfig(Filename = sys.argv[1])
sm.Init()

def GetAllMemory(type):
    status = sm.GetMemoryStatus(Type = type)

    remain = status['Used']

    start = True

    while remain > 0:
        if start:
            entry = sm.GetNextMemory(Start = True, Type = type)
            start = False
        else:
            entry = sm.GetNextMemory(Location = entry['Location'], Type = type)
        remain = remain - 1

        print
        print '%-15s: %d' % ('Location',entry['Location'])
        for v in entry['Entries']:
            if v['Type'] in ('Photo'):
                print '%-15s: %s...' % (v['Type'], repr(v['Value'])[:30])
            else:
                print '%-15s: %s' % (v['Type'], str(v['Value']))

def GetAllCalendar():
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

def Battery():
    status = sm.GetBatteryCharge()

    for x in status:
        if status[x] != -1:
            print "%20s: %s" % (x, status[x])

def GetAllSMS():
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

    return sms

def PrintAllSMS(sms):
    for m in sms:
        print
        print '%-15s: %s' % ('Number', m['Number'])
        print '%-15s: %s' %  ('Date', str(m['DateTime']))
        print '%-15s: %s' % ('State', m['State'])
        print '\n%s' % m['Text']

def LinkAllSMS(sms):
    data = gammu.LinkSMS([[msg] for msg in sms])

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

def GetAllTodo():
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

GetAllMemory('ME')
GetAllMemory('SM')
GetAllMemory('MC')
GetAllMemory('RC')
GetAllMemory('DC')
Battery()
GetAllCalendar()
GetAllTodo()
smslist = GetAllSMS()
PrintAllSMS(smslist)
LinkAllSMS(smslist)
