#!/usr/bin/env python
# -*- coding: ISO-8859-2 -*-

from distutils.core import setup, Extension
from commands import getstatusoutput
import sys
import os

# some defines
MAJOR_VERSION = 0
MINOR_VERSION = 2
VERSION_APPEND = ''

VERSION = '%d.%d' % (MAJOR_VERSION, MINOR_VERSION) + VERSION_APPEND

GAMMU_REQUIRED = '0.91.0'

GAMMU_LIB = os.getenv('GAMMU_LIB')
GAMMU_INC = os.getenv('GAMMU_INC')

def getoutput(cmd):
    """Return output (stdout or stderr) of executing cmd in a shell."""
    return getstatusoutput(cmd)[1]

def pkgc_version_check(name, longname, req_version):
    is_installed = not os.system('pkg-config --exists %s' % name)
    if not is_installed:
        print "ERROR: Could not find %s" % longname
        raise SystemExit
    
    orig_version = getoutput('pkg-config --modversion %s' % name)
    version = map(int, orig_version.split('.'))
    pkc_version = map(int, req_version.split('.'))
                      
    if version < pkc_version:
        print "ERROR: Too old version of %s" % longname
        print "       Need %s, but %s is installed" % \
              (req_version, orig_version)
        raise SystemExit

def get_include_dirs(name):
    output = getoutput('pkg-config --cflags-only-I %s' % name)
    return output.replace('-I', '').split()

def get_libraries(name):
    output = getoutput('pkg-config --libs-only-l %s' % name)
    return output.replace('-l', '').split()

def get_library_dirs(name):
    output = getoutput('pkg-config --libs-only-L %s' % name)
    return output.replace('-L', '').split()

pkgc_check = False

def check_pkgconfig():
    global pkgc_check
    if pkgc_check:
        return
    if getstatusoutput('pkg-config --version')[0] != 0:
        print "Error, could not find pkg-config"
        raise SystemExit

    pkgc_version_check('gammu', 'Gammu', GAMMU_REQUIRED)
    pkgc_check = True

if GAMMU_LIB == None:
    check_pkgconfig()
    libs = get_library_dirs('gammu')
else:
    libs = [GAMMU_LIB]
    
if GAMMU_INC == None:
    check_pkgconfig()
    incs = get_include_dirs('gammu')
else:
    incs = [GAMMU_INC]

defs = []
undefs = []

# Check whether to use PreferUnicode
f = open(os.path.join(incs[0], 'service', 'gsmpbk.h'))
if f.read().find('PreferUnicode') == -1:
    undefs.append('USE_PREF_UNICODE')
else:
    defs.append(('USE_PREF_UNICODE', None))
f.close()
del f

gammumodule = Extension('gammu', 
    include_dirs = incs + ['.'],
    libraries = get_libraries('gammu'),
    library_dirs = libs,
    define_macros = defs + [('PYTHON_GAMMU_VERSION', '"' + VERSION + '"')],
    undef_macros = undefs,
    sources = [
        'errors.c',
        'convertors/misc.c', 
        'convertors/string.c', 
        'convertors/time.c', 
        'convertors/base.c', 
        'convertors/sms.c', 
        'convertors/memory.c', 
        'convertors/todo.c', 
        'gammu.c',
        ])

setup (name = 'gammu',
    version = VERSION,
    description = 'Gammu bindings',
    author = 'Michal Čihař',
    author_email = 'gammu@cihar.com',
    license = 'GPL',
    url = 'http://cihar.com/gammu/python',
    long_description = '''
This is interface to Gammu.
''',
    ext_modules = [gammumodule]
    )

