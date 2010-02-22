# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
Gammu exceptions.
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

import gammu._gammu

# Import base exception
from gammu._gammu import GSMError

# Import all exceptions
for _name in dir(gammu._gammu):
    if not _name.startswith('ERR_'):
        continue
    _temp = __import__('gammu._gammu', globals(), locals(), [_name], -1)
    locals()[_name] = getattr(_temp, _name)

# Cleanup
del _name
del _temp
del gammu
