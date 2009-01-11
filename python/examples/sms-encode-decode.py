#!/usr/bin/env python

# Simple test for testing SMS decoding/encoding

import gammu

# text of sms
txt = '.........1.........2.........3.........4.........5.........6.........7.........8.........9........0.........1.........2.........3.........4.........5.........6.........7.........8.........9........0.........1.........2.........3.........4.........5.........6.........7.........8.........9........0'

# SMS info about message
smsinfo = {'Entries':[{'ID': 'ConcatenatedTextLong', 'Buffer': txt}]}

# encode SMS
sms = gammu.EncodeSMS(smsinfo)

# decode back SMS
decodedsms = gammu.DecodeSMS(sms)

# show results
print "Text:", decodedsms['Entries'][0]['Buffer']
print "Comparsion:", (decodedsms['Entries'][0]['Buffer'] == txt)
