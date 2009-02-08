#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
python-gammu - Test script to test several Gammu SMSD operations (usually using dummy driver, but it depends on config)
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

SQLITE_BIN='@SQLITE_BIN@'
DATA_DIR='@CMAKE_CURRENT_BINARY_DIR@'
GAMMU_SRC_DIR='@Gammu_SOURCE_DIR@'

if SQLITE_BIN == '' or SQLITE_BIN[0] == '@':
    raise Exception('Please configure this script!')

import gammu
import sys
import os
import threading
import time
import traceback

def GenerateSMSDRC(path, gammurc):
    gammu_config = file(gammurc).read()
    out = file(path, 'w')
    out.write('''
[smsd]
commtimeout = 1
debuglevel = 255
logfile = stderr
service = dbi
driver = sqlite3
database = smsd.db
dbdir = %s

%s
    ''' % (DATA_DIR, gammu_config))

def CreateSQLiteDB():
    smsddb = os.path.join(DATA_DIR, 'smsd.db')
    if os.path.exists(smsddb):
        os.unlink(smsddb)
    os.system('%s %s < %s' % (
        SQLITE_BIN,
        smsddb,
        os.path.join(GAMMU_SRC_DIR, 'docs', 'sql', 'sqlite.sql')))

if __name__ == '__main__':
    try:
        smsdrc = os.path.join(DATA_DIR, '.smsdrc')

        # Prepare test environment
        GenerateSMSDRC(smsdrc, os.path.join(DATA_DIR, '.gammurc'))
        CreateSQLiteDB()

        # Load configuration
        smsd = gammu.SMSD(smsdrc)

        # Start SMSD thread
        smsd_thread = threading.Thread(target = smsd.MainLoop)
        smsd_thread.start()

        time.sleep(10)

        # Inject SMS messages
        message = {'Text': 'python-gammu testing message', 'SMSC': {'Location': 1}, 'Number': '1234567890'}
        smsd.InjectSMS([message])
        message = {'Text': 'python-gammu second testing message', 'SMSC': {'Location': 1}, 'Number': '1234567890'}
        smsd.InjectSMS([message])

        time.sleep(20)

        # Show SMSD status
        status = smsd.GetStatus()
        print status
        if status['Sent'] != 2:
            raise Exception('Messages were not sent!')

        time.sleep(1)

        # Signal SMSD to stop
        smsd.Shutdown()

        # Wait for it
        smsd_thread.join()
    # Catch any exception and signal smsd to stop and reraise it
    except Exception, exc:
        traceback.print_exc()
        try:
            smsd.Shutdown()
        except:
            pass
        raise exc

