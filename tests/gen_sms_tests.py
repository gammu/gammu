#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Gammu SMS backup generator.
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

import os

# Work in both common location when this can be executed:
try:
    os.chdir('tests/at-sms-encode/')
except OSError:
    os.chdir('at-sms-encode/')

# Numbers we're going to test
NUMBERS = [
    '1234',
    '800123456',
    '+420800123456',
    '+41761234567',
    ]

# Text parts we're going to test
TEXTS = [
    '123456',
    'Zkouška sirén',
    'This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 2 as published by the Free Software Foundation.',
    ]

TEMPLATE = '''
[SMSBackup000]
SMSC = "%s"
State = %s
Number = "%s"
Coding = %s
Folder = %d
'''

STATES = [
        'Read',
        'Read',
        'Sent',
        ]
CODINGS = [
    'Default',
    'Unicode',
    ]

def write_text(f, text):
    '''
    Writes text splitted and encoded in same way as Gammu does it for SMS backups.
    '''
    encoded = text.encode('UTF-16-BE').encode('HEX')
    line = 0
    while len(encoded) > 0:
        f.write('Text%02d = %s\n' % (line, encoded[:200]))
        encoded = encoded[200:]
        line = line + 1

def generate_message(index, folder, coding, smscnum, num, text):
    '''
    Generates single message file.
    '''
    f = file('%02d.backup' % index, 'w')
    f.write(TEMPLATE % (
        NUMBERS[smscnum],
        STATES[folder],
        NUMBERS[num],
        CODINGS[coding],
        folder
        ))
    if folder > 1:
        f.write('Sent = 20070605T135630\n')
    write_text(f, TEXTS[text])
    f.close()

def generate():
    '''
    Generates test data based on NUMBERS and TEXTS variables.
    '''
    index = 1

    for smscnum in range(len(NUMBERS)):
        for num in range(len(NUMBERS)):
            for text in range(len(TEXTS)):
                for coding in range(len(CODINGS)):
                    for folder in [1, 2]:
                        generate_message(index,
                                folder,
                                coding,
                                smscnum,
                                num,
                                text)
                        index = index + 1

if __name__ == '__main__':
    generate()
