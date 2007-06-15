# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Phone communication libary - python wrapper for Gammu library.

This module wraps all python-gammu functionality. Core library is 
named L{gammu.Core} and all of it's content is automatically 
imported also into this gammu module, so you can use both
L{gammu.StateMachine} or L{gammu.Core.StateMachine}.
'''
__author__ = 'Michal Čihař'
__email__ = 'michal@cihar.com'
__license__ = '''
Copyright (c) 2003 - 2007 Michal Čihař

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
    'Core',
    '__author__',
    '__email__',
    '__license__',
    '__version__',
    '__gammu_version__',
    ]

# Conveniency and backward compatibility import
from gammu.Core import *

import gammu.Core
__version__ = gammu.Core.Version()[1]
__gammu_version__ = gammu.Core.Version()[0]
