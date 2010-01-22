#!/usr/bin/python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Script to decode USB Snoop Pro log to human readable form.

This is probably only useful for AT commands.
'''

import sys

if len(sys.argv) < 2:
    print 'Usage: decode.py LOGFILE'
    sys.exit(1)

f = open(sys.argv[1])
output = ""
for line in f:
    if line[:14] == '<payloadbytes>':
        line = line[14:-17]
        print line
        print line.decode('hex')

