#!/usr/bin/env python
# -*- coding: ISO-8859-2 -*-

MAJOR_VERSION = 0
MINOR_VERSION = 1

VERSION = "%d.%d" % (MAJOR_VERSION, MINOR_VERSION)

# should be 0.90, but it is not yet released :-)
GAMMU_REQUIRED = '0.90.0'

from distutils.core import setup, Extension
from commands import getstatusoutput
import sys
import os

def getoutput(cmd):
    """Return output (stdout or stderr) of executing cmd in a shell."""
    return getstatusoutput(cmd)[1]

if getstatusoutput('pkg-config --version')[0] != 0:
    print "Error, could not find pkg-config"
    raise SystemExit

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

pkgc_version_check('gammu', 'Gammu', GAMMU_REQUIRED)

gammumodule = Extension('gammu', 
    include_dirs = get_include_dirs('gammu'),
    libraries = get_libraries('gammu'),
    library_dirs = get_library_dirs('gammu'),
    sources = ['gammu.c', 'convertors.c'])

setup (name = 'gammu',
    version = VERSION,
    description = 'Gammu bindings',
    author = 'Michal Čihař',
    author_email = 'gammu@cihar.com',
    license = 'GPL',
    url = 'http://cihar.com/gammu/python',
    long_description = '''
This is experimental interface to Gammu (GNU All Phone Management Utilities).
''',
    ext_modules = [gammumodule]
    )

