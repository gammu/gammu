#!/usr/bin/env python

import gammu
import time

def Callback(sm, type, data):
    print 'Received incoming event type %s, data:' % type
    print data

sm = gammu.StateMachine()
sm.ReadConfig()
sm.Init()
sm.SetIncomingCallback(Callback)

try:
    sm.SetIncomingCall()
except gammu.ERR_NOTSUPPORTED:
    print 'Incoming calls notification is not supported.'

try:
    sm.SetIncomingCB()
except gammu.ERR_NOTSUPPORTED:
    print 'Incoming CB notification is not supported.'
except gammu.ERR_SOURCENOTAVAILABLE:
    print 'Cell broadcasts support not enabled in Gammu.'

try:
    sm.SetIncomingSMS()
except gammu.ERR_NOTSUPPORTED:
    print 'Incoming SMS notification is not supported.'

try:
    sm.SetIncomingUSSD()
except gammu.ERR_NOTSUPPORTED:
    print 'Incoming USSD notification is not supported.'

# We need to keep communication with phone to get notifications
print 'Press Ctrl+C to interrupt'
while 1:
    q = sm.GetSignalQuality()
    print 'Signal is at %d%%' % q['SignalPercent']
    time.sleep(1)
