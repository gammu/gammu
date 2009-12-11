#!/usr/bin/python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Script to decode USB Snoop log to human readable form.

This is probably only useful for AT commands.
'''

import sys

if len(sys.argv) < 2:
    print 'Usage: decode.py LOGFILE'
    sys.exit(1)

f = open(sys.argv[1])
output = ""
for line in f:
    if line[:7] == '    000':
        line = line.strip()
        pos, data = line.split(':')
        if pos == '00000000':
            if output != '':
                print output.decode('hex')
        data = ''.join(data.strip().split(' '))
        output += data

