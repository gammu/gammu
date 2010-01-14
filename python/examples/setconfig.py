#!/usr/bin/env python
'''
Mere a test case than a useful example, showing various ways of setting
configuration.
'''

CONFIGS = [
    {
        'StartInfo': True,
        'UseGlobalDebugFile': True,
        'DebugFile': None,
        'SyncTime': True,
        'Connection': 'at',
        'LockDevice': True,
        'DebugLevel': 'textalldate',
        'Device': '',
        'Model': '',
    },
    {
        'StartInfo': 'yes',
        'UseGlobalDebugFile': 'no',
        'DebugFile': 'dbg.log',
        'SyncTime': 'true',
        'Connection': 'fbus',
        'LockDevice': 'FALSE',
        'DebugLevel': 'textall',
        'Device': '',
        'Model': '',
    },
    {
        'StartInfo': None,
        'UseGlobalDebugFile': None,
        'DebugFile': 'dbg.log',
        'SyncTime': 'true',
        'Connection': 'dlr3',
        'LockDevice': 'NO',
        'DebugLevel': 'binary',
        'Device': '',
        'Model': '',
    },
    ]

import gammu

sm = gammu.StateMachine()
for cfg in CONFIGS:
    try:
        sm.SetConfig(0, cfg)
    except Exception, e:
        print cfg
        raise e
