#!/usr/bin/env python
# -*- coding: UTF-8 -*-

# Simple test for testing SMS decoding/encoding
# This passes unicode string which can be transferet to GSM charset
# to Gammu and expects it to be unchanged in the SMS

import gammu

# text of sms
txt = u'.........1$........2..Ø......3...åÅ....4....Λ....5....Æ....6....ñ....7.........8.........9........0.........1.........2.........3.........4.........5.........6.........7.........8.........9........0.........1.........2.........3.........4.........5.........6.........7.........8.........9....¥€..0'

# SMS info about message
smsinfo = {'Entries':[{'ID': 'ConcatenatedTextLong', 'Buffer': txt}]}

# encode SMSes
sms = gammu.EncodeSMS(smsinfo)

# decode back SMSes
decodedsms = gammu.DecodeSMS(sms)

# show results
print "Text:", repr(decodedsms['Entries'][0]['Buffer'])
print "Comparsion:", (decodedsms['Entries'][0]['Buffer'] == txt)

# do conversion to PDU
pdu = [gammu.EncodePDU(s) for s in sms]

# Convert back
pdusms = [gammu.DecodePDU(p) for p in pdu]

# decode back SMS from PDU
decodedsms = gammu.DecodeSMS(pdusms)

# show PDU results
print "PDU Text:", repr(decodedsms['Entries'][0]['Buffer'])
print "PDU Comparsion:", (decodedsms['Entries'][0]['Buffer'] == txt)

