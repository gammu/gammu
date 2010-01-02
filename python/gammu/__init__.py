# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Phone communication libary - python wrapper for Gammu library.

This module wraps all python-gammu functionality. Core library is
named L{gammu._gammu} and all of it's content is automatically
imported also into this gammu module, so you can use both
L{gammu.StateMachine} or L{gammu._gammu.StateMachine}.
'''
__author__ = 'Michal Čihař'
__email__ = 'michal@cihar.com'
__license__ = '''
Copyright © 2003 - 2010 Michal Čihař

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

# Submodules
__all__ = [
    'Data',
    'Worker',
    'SMSD',
    'StateMachine'
    ]

# Conveniency and backward compatibility import
from gammu._gammu import *

import gammu._gammu
__version__ = 'Gammu %s, python-gammu %s' % (
        gammu._gammu.Version()[0],
        gammu._gammu.Version()[1]
        )
