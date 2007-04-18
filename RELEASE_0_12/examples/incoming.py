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
    pass
try:
    sm.SetIncomingCB()
except gammu.ERR_NOTSUPPORTED:
    pass
try:
    sm.SetIncomingSMS()
except gammu.ERR_NOTSUPPORTED:
    pass
try:
    sm.SetIncomingUSSD()
except gammu.ERR_NOTSUPPORTED:
    pass

# We need to keep commincation with phone to get notifications
print 'Press Ctrl+C to interrupt'
while 1:
    time.sleep(1)
    q = sm.GetSignalQuality()
    print 'Signal is at %d%%' % q['SignalPercent']
