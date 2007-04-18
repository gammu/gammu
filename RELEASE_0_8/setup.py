#!/usr/bin/env python
# -*- coding: UTF-8 -*-

from distutils.core import setup, Extension
from commands import getstatusoutput
import sys
import os
import os.path
import re

# some defines
MAJOR_VERSION = 0
MINOR_VERSION = 8
EXTRA_VERSION = None

if EXTRA_VERSION == None:
    VERSION = '%d.%d' % (MAJOR_VERSION, MINOR_VERSION)
else:
    VERSION = '%d.%d.%d' % (MAJOR_VERSION, MINOR_VERSION, EXTRA_VERSION)

GAMMU_REQUIRED = (1,01,00)
PYTHON_REQUIRED = (2,3)

GAMMU_LIB = os.getenv('GAMMU_LIB')
GAMMU_INC = os.getenv('GAMMU_INC')

PKGC_PATH = ''

if sys.version_info < PYTHON_REQUIRED:
    print 'You need python %s to compile this!' % '.'.join(map(str, PYTHON_REQUIRED))
    sys.exit(1)

def getoutput(cmd):
    """Return output (stdout or stderr) of executing cmd in a shell."""
    return getstatusoutput(cmd)[1]

def pkgc_version_check(name, longname, req_version, msg = None):
    global PKGC_PATH
    is_installed = not os.system(PKGC_PATH + 'pkg-config --exists %s' % name)
    if not is_installed:
        print "ERROR: Could not find %s" % longname
        if PKGC_PATH == '':
            print "Trying to search in prefix /usr/local"
            PKGC_PATH='PKG_CONFIG_PATH=/usr/local/lib/pkgconfig '
            pkgc_version_check(name, longname, req_version, msg)
        else:
            if msg != None:
                print msg
            sys.exit(1)

    version = getoutput(PKGC_PATH + 'pkg-config --modversion %s' % name)
    pkc_version = tuple(map(int, version.split('.')))

    if pkc_version < req_version:
        print "ERROR: Too old version of %s" % longname
        print "       Need %s, but %s is installed" % \
              ('.'.join(map(str, req_version)), version)
        sys.exit(1)

def get_include_dirs(name):
    global PKGC_PATH
    output = getoutput(PKGC_PATH + 'pkg-config --cflags-only-I %s' % name)
    return output.replace('-I', '').split()

def get_libraries(name):
    global PKGC_PATH
    output = getoutput(PKGC_PATH + 'pkg-config --libs-only-l %s' % name)
    return output.replace('-l', '').split()

def get_library_dirs(name):
    global PKGC_PATH
    output = getoutput(PKGC_PATH + 'pkg-config --libs-only-L %s' % name)
    return output.replace('-L', '').split()

pkgc_check = False

def check_pkgconfig():
    global pkgc_check
    if pkgc_check:
        return
    if getstatusoutput('pkg-config --version')[0] != 0:
        print "Error, could not find pkg-config"
        sys.exit(1)

    pkgc_version_check('gammu', 'Gammu', GAMMU_REQUIRED,
        '\nYou need installed gammu as shared library and enable pkg-config to find it.\n\n' +
        'This is done by invoking make installshared in gammu sources.'
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

# We didn't yet check version
if not pkgc_check:
    try:
        content = file(os.path.join(incs[0], 'config.h')).read()
        found = re.search('VERSION "([^"]*)"', content)
    except IOError:
        found = None

    # Aren't we in source tree?
    if found == None:
        try:
            content = file(os.path.join(incs[0], '..', 'cfg', 'config.h')).read()
        except IOError:
            print 'Can not find config.h!'
            sys.exit(1)

        found = re.search('VERSION "([^"]*)"', content)

    if found == None:
        print 'Can not read version from config.h!'
        sys.exit(1)

    found = found.groups()

    if len(found) != 1:
        print 'Can not read version from config.h!'
        sys.exit(1)

    cfg_version = tuple(map(int, found[0].split('.')))

    if cfg_version < GAMMU_REQUIRED:
        print "ERROR: Too old version of Gammu"
        print "       Need %s, but %s is installed" % \
              ('.'.join(map(str, GAMMU_REQUIRED)),
              ('.'.join(map(str, cfg_version))))
        sys.exit(1)
    linklibs = ['Gammu']
else:
    linklibs = get_libraries('gammu')

defs = [('PYTHON_GAMMU_VERSION', '"' + VERSION + '"')]
undefs = []

gammumodule = Extension('gammu',
    include_dirs = incs + ['.'],
    libraries = linklibs,
    library_dirs = libs,
    define_macros = defs,
    undef_macros = undefs,
    sources = [
        'errors.c',
        'misc.c',
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
    maintainer = "Michal Čihař",
    maintainer_email = "michal@cihar.com",
    license = 'GPL',
    url = 'http://cihar.com/gammu/python',
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'License :: OSI Approved :: GNU General Public License (GPL)',
        'Operating System :: Microsoft :: Windows :: Windows 95/98/ME',
        'Operating System :: Microsoft :: Windows :: Windows NT/2000/XP',
        'Operating System :: POSIX',
        'Operating System :: Unix',
        'Programming Language :: Python',
        'Programming Language :: C',
        'Topic :: Communications :: Telephony'
    ],
    ext_modules = [gammumodule]
    )

