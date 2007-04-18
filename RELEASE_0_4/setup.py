#!/usr/bin/env python
# -*- coding: UTF-8 -*-

from distutils.core import setup, Extension
from commands import getstatusoutput
import sys
import os

# some defines
MAJOR_VERSION = 0
MINOR_VERSION = 4
VERSION_APPEND = ''

VERSION = '%d.%d' % (MAJOR_VERSION, MINOR_VERSION) + VERSION_APPEND

GAMMU_REQUIRED = '0.92.7'

GAMMU_LIB = os.getenv('GAMMU_LIB')
GAMMU_INC = os.getenv('GAMMU_INC')

if sys.version_info < (2,3):
    print 'You need python 2.3 to compile this!'
    raise SystemExit

def getoutput(cmd):
    """Return output (stdout or stderr) of executing cmd in a shell."""
    return getstatusoutput(cmd)[1]

def pkgc_version_check(name, longname, req_version, msg = None):
    is_installed = not os.system('pkg-config --exists %s' % name)
    if not is_installed:
        print "ERROR: Could not find %s" % longname
        if msg != None:
            print msg
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

    pkgc_version_check('gammu', 'Gammu', GAMMU_REQUIRED,
        '\nYou need installed gammu as shared library and enable pkg-config to find it.\n\n' +
        'Default Gammu installation goes to /usr/local, where pkg-config usually doesn\'t\n' +
        'search. Try setting PKG_CONFIG_PATH=/usr/local/lib/pkgconfig in this case before\n' +
        'running this setup.'
    )
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

defs = [('PYTHON_GAMMU_VERSION', '"' + VERSION + '"')]
undefs = []

gammumodule = Extension('gammu', 
    include_dirs = incs + ['.'],
    libraries = get_libraries('gammu'),
    library_dirs = libs,
    define_macros = defs,
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
        'convertors/calendar.c', 
        'convertors/bitmap.c', 
        'convertors/ringtone.c', 
        'convertors/backup.c', 
        'gammu.c',
        ])

setup (name = 'python-gammu',
    version = VERSION,
    description = 'Gammu bindings',
    long_description = 'Bindings to libGammu, which allows access many phones.',
    author = "Michal Čihař",
    author_email = 'gammu@cihar.com',
    license = 'GPL',
    url = 'http://cihar.com/gammu/python',
    ext_modules = [gammumodule]
    )

