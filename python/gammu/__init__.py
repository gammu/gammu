# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
#
# Copyright © 2003 - 2015 Michal Čihař <michal@cihar.com>
#
# This file is part of Gammu <http://wammu.eu/>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
'''
Phone communication libary - python wrapper for Gammu library.
'''

# Submodules
__all__ = [
    'data',
    'worker',
    'smsd',
    'exception',
    ]

# Import core library, we need to load it into global namespace
# as otherwise libdbi plugins fail to find some symbols
import sys
if not hasattr(sys,'getdlopenflags'):
    from gammu._gammu import *
else:
    try:
        from dl import RTLD_GLOBAL, RTLD_NOW
    except ImportError:
        RTLD_GLOBAL = -1
        RTLD_NOW = -1
        try:
            import os
            osname = os.uname()[0]
            if osname == 'Linux' or osname == 'SunOS':
                RTLD_GLOBAL = 0x00100
                RTLD_NOW = 0x00002
            elif osname == 'Darwin':
                RTLD_GLOBAL = 0x8
                RTLD_NOW = 0x2
        except:
             pass

    if RTLD_GLOBAL != -1 and RTLD_NOW != -1:
        try:
            flags = sys.getdlopenflags()
            sys.setdlopenflags(RTLD_GLOBAL | RTLD_NOW)
            try:
                from gammu._gammu import *
            finally:
                sys.setdlopenflags(flags)
        except:
            from gammu._gammu import *
    else:
        from gammu._gammu import *
    del RTLD_GLOBAL
    del RTLD_NOW

__version__ = 'Gammu %s, python-gammu %s' % (
        Version()[0],
        Version()[1]
        )
