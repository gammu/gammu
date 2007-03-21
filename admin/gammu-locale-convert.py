#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:

'''
Conversion script to convert Gammu locales to gettext.
'''

import ConfigParser
import codecs
import glob
import re

STRING_NUMBER=807
HEADER = u'''
# Gammu translation.
# Copyright (C) 2003 - 2007 Michal Čihař
# This file is distributed under the same license as the Gammu package.
# Michal Čihař <michal@cihar.com>, 2007.
#
msgid ""
msgstr ""
"Project-Id-Version: PACKAGE VERSION\\n"
"Report-Msgid-Bugs-To: michal@cihar.com\\n"
"POT-Creation-Date: 2007-03-20 09:12+0100\\n"
"PO-Revision-Date: 2007-03-20 09:13+0100\\n"
"Last-Translator: %(translator)s\\n"
"Language-Team: Language =%(language)s\\n"
"MIME-Version: 1.0\\n"
"Content-Type: text/plain; charset=UTF-8\\n"
"Content-Transfer-Encoding: 8bit\\n"
'''

def fixup_message(text):
    '''
    Fixes some common mistakes that were in old localisations.
    '''
    p = re.compile('([^\\\\])"')
    text = p.sub(r'{\1}\\"', text)
    if text[0] == '"':
        text = '\\' + text
    text = text.replace('\\s', '%s')
    text = text.replace('""', '"')
    text = text.replace('\\%', '\\"%')
    text = text.replace('\\m', '\\n')
    return text

def write_message(fout, orig, trans, fuzzy = False):
    '''
    Write message to output file.
    '''
    fout.write('\n')
    if fuzzy:
        fout.write('#, fuzzy\n')
    fout.write('msgid "%s"\n' % fixup_message(orig))
    fout.write('msgstr "%s"\n' % fixup_message(trans))

def convert_file(language):
    '''
    Converts language file from Gammu to gettext.
    '''
    output = '%s.po' % language
    input = 'gammu_%s.txt' % language
    count = 0
    duplicates = 0
    messages = []

    fin = codecs.open(input, 'r', 'utf-16-be')
    # skip BOM
    fin.read(1)

    # initialize parser
    parser = ConfigParser.RawConfigParser()
    parser.readfp(fin)

    # write ouput file
    fout = codecs.open(output, 'w', 'utf-8')

    # read translation information
    try:
        author = parser.get('locale', 'Author')[1:-1]
    except ConfigParser.NoOptionError:
        author = ''
    try:
        email = parser.get('locale', 'AuthorContact')[1:-1]
    except ConfigParser.NoOptionError:
        email = ''

    if email != '' and author != '':
        translator = '%s <%s>' % (author, email)
    elif email != '':
        translator = '<%s>' % email
    else:
        translator = author

    # write gettext header
    fout.write(HEADER % {'translator': translator, 'language': language})

    # process translations
    for i in range(0, STRING_NUMBER):
        try:
            orig = parser.get('common', 'F%04d' % i)[1:-1]
            trans = parser.get('common', 'T%04d' % i)[1:-1]
            if orig != trans:
                if orig in messages:
                    print 'WARNING: Duplicate translation for %s' % orig
                    duplicates += 1
                else:
                    messages.append(orig)
                    write_message(fout, orig, trans)
                    count += 1
        except ConfigParser.NoOptionError:
            pass
        print 'Converting: %d %%\r' % (100 * i / STRING_NUMBER),

    print '%s converted successfully, %d (%d %%) translated (%d duplicates)' % (language, count, 100 * count / STRING_NUMBER, duplicates)

if __name__ == '__main__':
    for name in glob.glob('gammu_??.txt'):
        lang = name[6:8]
        if lang == 'us':
            continue
        convert_file(lang)
