#!/usr/bin/env python

'''
Really more a testcase for DecodeSMS than real example.
'''

import gammu
import datetime

message = [{'RejectDuplicates': 0, 'SMSCDateTime': datetime.datetime(2010, 7, 22, 17, 4, 11), 'Class': -1, 'Name': u'', 'InboxFolder': 0, 'Text': '\x04\x06\x03\xbe\xaf\x84\x8c\x82\x981277970059\x00\x8d\x92\x89\x19\x80\x16\x0433707520030/TYPE=PLMN\x00\x96yBO\x00\x8a\x80\x8e\x01"\x88\x04\x81\x02\x0b\xb8\x83http://mmsc.labmctel.fr:9090/m33\x00', 'SMSC': {'DefaultNumber': u'', 'Format': 'Text', 'Number': u'+33700065030', 'Validity': 'NA', 'Location': 0, 'Name': u''}, 'ReplaceMessage': 0, 'Coding': '8bit', 'Number': u'33707520030', 'DateTime': datetime.datetime(2010, 7, 1, 9, 40, 21), 'DeliveryStatus': 0, 'State': 'UnSent', 'MessageReference': 0, 'Length': 99, 'Location': 0, 'Memory': '', 'ReplyViaSameSMSC': 0, 'UDH': {'Text': '\x06\x05\x04\x0b\x84#\xf0', 'ID16bit': -1, 'AllParts': -1, 'ID8bit': -1, 'PartNumber': -1, 'Type': 'UserUDH'}, 'Type': 'Deliver', 'Folder': 2}]

print gammu.DecodeSMS(message)
