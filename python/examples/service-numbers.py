#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Service numbers dialogue example.
'''
__author__ = 'Michal Čihař'
__email__ = 'michal@cihar.com'
__license__ = '''
Copyright © 2003 - 2010 Michal Čihař

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

def callback(sm, type, data):
    '''
    Callback on USSD data.
    '''
    if type != 'USSD':
        print 'Unexpected event type: %s' % type
        sys.exit(1)

    print 'Network reply:'
    print 'Status: %s' % data['Status']
    print data['Text']

    if data['Status'] == 'ActionNeeded':
        do_service(sm)

def init():
    '''
    Intializes gammu and callbacks.
    '''
    global sm
    sm = gammu.StateMachine()
    if len(sys.argv) >= 2:
        sm.ReadConfig(Filename = sys.argv[1])
    else:
        sm.ReadConfig()
    sm.Init()
    sm.SetIncomingCallback(callback)
    try:
        sm.SetIncomingUSSD()
    except gammu.ERR_NOTSUPPORTED:
        print 'Incoming USSD notification is not supported.'
        sys.exit(1)
    return sm

def do_service(sm):
    '''
    Main code to talk with worker.
    '''
    if len(sys.argv) >= 3:
        code = sys.argv[2]
        del sys.argv[2]
    else:
        print 'Enter code (empty string to end):',
        code = raw_input()
    if code != '':
        print 'Talking to network...'
        sm.DialService(code)

if __name__ == '__main__':
    sm = init()
    print 'This example shows interaction with network using service codes'
    do_service(sm)
