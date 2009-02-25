#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
python-gammu - Phone communication libary
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

import distutils
import distutils.sysconfig
import distutils.cygwinccompiler
import distutils.command.build
import distutils.command.build_ext
import distutils.command.install
import distutils.command.bdist_wininst
from distutils.core import setup, Extension
from commands import getstatusoutput
import sys
import os
import os.path
import re
import datetime
import string

# some defines
VERSION = '1.23.1'
VERSION_TUPLE = tuple(map(int, VERSION.split('.')))
GAMMU_REQUIRED = VERSION_TUPLE
PYTHON_REQUIRED = (2,3)

ADDITIONAL_PARAMS = [
        ('skip-deps', 's', 'skip checking for dependencies'),
        ('pkgconfig=', 'p', 'path to pkg-config binary'),
        ('pkgconfig-search-path=', 'P', 'path where pkg-config searches for installed packages'),
        ('gammu-libs=', None, 'path to libGammu'),
        ('gammu-incs=', None, 'path to gammu.h include'),
        ('gammu-cfg=', None, 'path to gammu-config.h include'),
        ('gammu-build=', None, 'path where gammu library has been built'),
        ]

class build_ext_gammu(distutils.command.build_ext.build_ext, object):
    """
    Custom build_ext command with dependency checking support.
    """
    user_options = distutils.command.build_ext.build_ext.user_options  + ADDITIONAL_PARAMS
    boolean_options = distutils.command.build_ext.build_ext.boolean_options + ['skip-deps']
    linklibs = []

    def initialize_options(self):
        super(build_ext_gammu, self).initialize_options()
        self.skip_deps = False
        self.pkgconfig = 'pkg-config'
        self.pkgconfig_search_path = None
        self.gammu_libs = None
        self.gammu_incs = None
        self.gammu_cfg = None
        self.gammu_build = None

    def do_pkgconfig(self, cmd):
        prefix = ''
        if self.pkgconfig_search_path is not None:
            prefix = 'PKG_CONFIG_PATH="%s" ' % self.pkgconfig_search_path
        return getstatusoutput(prefix + 'pkg-config %s' % cmd)

    def pkg_exists(self, name):
        output = self.do_pkgconfig('--exists %s' % name)
        if output[0] != 0:
            if self.pkgconfig_search_path is None:
                self.pkgconfig_search_path = '/usr/local/lib/pkgconfig'
                print 'Package %s not found, adding %s to pkg-config search path' % \
                    (name, self.pkgconfig_search_path)
                output = self.do_pkgconfig('--exists %s' % name)
        if output[0] == 0:
            return True
        print 'ERROR: Could not find package %s!' % name
        return False

    def check_version(self, name, version):
        output = self.do_pkgconfig('--modversion %s' % name)
        distutils.log.info('found %s version %s' % (name, output[1]))
        pkc_version = tuple(map(int, output[1].split('.')))
        if pkc_version >= version:
            return True
        print 'ERROR: Package %s is too old!' % name
        print '       You need version %s, but %s is installed' % \
              ('.'.join(map(str, version)), '.'.join(map(str, pkc_version)))
        return False

    def get_pkg_include_dirs(self, name):
        output = self.do_pkgconfig('--cflags-only-I %s' % name)
        if output[0] != 0:
            return None
        return output[1].replace('-I', '').split()

    def get_pkg_libraries(self, name):
        output = self.do_pkgconfig('--libs-only-l %s' % name)
        if output[0] != 0:
            return None
        return output[1].replace('-l', '').split()

    def get_pkg_library_dirs(self, name):
        output = self.do_pkgconfig('--libs-only-L %s' % name)
        if output[0] != 0:
            return None
        return output[1].replace('-L', '').split()

    def check_pkgconfig(self):
        res = self.do_pkgconfig('--version')
        if res[0] != 0:
            print "ERROR: Could not find pkg-config!"
            sys.exit(1)

    def check_gammu(self):
        if not self.pkg_exists('gammu') or not self.check_version('gammu', GAMMU_REQUIRED):
            print '\nYou need installed gammu and enabled pkg-config to find it.'
            print
            print 'This is done by invoking make install in gammu sources.'
            print
            print 'If package was installed to other prefix, please use'
            print '--pkgconfig-search-path=<install_prefix>/lib/pkgconfig'
            print
            print 'If you get this error when invoking setup.py install, you can'
            print 'try to ignore this with --skip-deps'
            print
            sys.exit(1)

    def read_gammu_version(self, path):
        try:
            distutils.log.info('checking for gammu-config.h in %s' % path)
            config_h = os.path.join(path, 'gammu-config.h')
            content = file(config_h).read()
            return re.search('VERSION "([^"]*)"', content)
        except IOError:
            return None

    def check_includedir(self):
        self.gammu_incs = os.path.expanduser(self.gammu_incs)

        found = None

        if self.gammu_cfg is not None:
            found = self.read_gammu_version(self.gammu_cfg)
            if found is None:
                self.gammu_cfg = None

        if found is None:
            found = self.read_gammu_version(self.gammu_incs)

        # Old build system
        if found is None:
            found = self.read_gammu_version(os.path.join(self.gammu_incs, '..', 'cfg'))

        if found == None:
            print 'WARNING: Can not read version from gammu-config.h!'
            self.gammu_incs = None
            return False

        found = found.groups()

        if len(found) != 1:
            print 'WARNING: Can not read version from gammu-config.h!'
            self.gammu_incs = None
            return False

        cfg_version = tuple(map(int, found[0].split('.')))

        if cfg_version < GAMMU_REQUIRED:
            print "ERROR: Too old version of Gammu"
            print "       Need %s, but %s is installed" % \
                  ('.'.join(map(str, GAMMU_REQUIRED)),
                  ('.'.join(map(str, cfg_version))))
            sys.exit(1)

        return True

    def check_libs(self):
        self.gammu_libs = os.path.expanduser(self.gammu_libs)
        self.linklibs.append('Gammu')
        if sys.platform == 'win32':
            self.linklibs.append('wsock32')
            self.linklibs.append('ws2_32')
            self.linklibs.append('setupapi')
            self.linklibs.append('advapi32')

    def process_build_tree(self):
        if not os.path.exists(os.path.join(self.gammu_build, 'CMakeCache.txt')):
            self.gammu_build = os.path.join(self.gammu_build, 'build-configure')
            if not os.path.exists(os.path.join(self.gammu_build, 'CMakeCache.txt')):
                distutils.log.info('Could not find CMake build files.')
                distutils.log.info('You are probably using too old Gammu version!')
                sys.exit(1)

        distutils.log.info('Detected CMake style build tree')
        # We will use pkgconfig for detecting rest.
        self.pkgconfig_search_path = os.path.join(self.gammu_build, 'pkgconfig')

    def check_requirements(self):
        if sys.version_info < PYTHON_REQUIRED:
            print 'You need python %s to compile this!' % '.'.join(map(str, PYTHON_REQUIRED))
            sys.exit(1)

        if self.gammu_build is not None:
            self.process_build_tree()

        if self.gammu_incs is not None:
            if not self.check_includedir():
                print "ERROR: Could not determine gammu sources version!"
                sys.exit(1)

        if self.gammu_libs is not None:
            self.check_libs()

        if self.gammu_libs is None or self.gammu_incs is None:
            self.check_pkgconfig()
            self.check_gammu()
            if self.gammu_libs is None:
                self.linklibs += self.get_pkg_libraries('gammu')
                self.gammu_libs = self.get_pkg_library_dirs('gammu')
            if self.gammu_incs is None:
                self.gammu_incs = self.get_pkg_include_dirs('gammu')

        if self.gammu_cfg is not None:
            self.include_dirs.append(self.gammu_cfg)

        if type(self.gammu_incs) is str:
            self.include_dirs.append(self.gammu_incs)
        else:
            self.include_dirs += self.gammu_incs

        self.include_dirs.append('./include')

        if type(self.gammu_libs) is str:
            self.library_dirs.append(self.gammu_libs)
        else:
            self.library_dirs += self.gammu_libs

        if type(self.linklibs) is str:
            self.libraries.append(self.linklibs)
        else:
            self.libraries += self.linklibs

    def run (self):
        if not self.skip_deps:
            self.check_requirements()
        super(build_ext_gammu, self).run()

gammumodule = Extension('gammu._gammu',
    include_dirs = ['.'],
    sources = [
        'gammu/src/errors.c',
        'gammu/src/misc.c',
        'gammu/src/convertors/misc.c',
        'gammu/src/convertors/string.c',
        'gammu/src/convertors/time.c',
        'gammu/src/convertors/base.c',
        'gammu/src/convertors/sms.c',
        'gammu/src/convertors/memory.c',
        'gammu/src/convertors/todo.c',
        'gammu/src/convertors/calendar.c',
        'gammu/src/convertors/bitmap.c',
        'gammu/src/convertors/ringtone.c',
        'gammu/src/convertors/backup.c',
        'gammu/src/convertors/file.c',
        'gammu/src/convertors/call.c',
        'gammu/src/convertors/wap.c',
        'gammu/src/gammu.c',
        ])

setup (name = 'python-gammu',
    version = VERSION,
    description = 'Gammu bindings',
    long_description = 'Bindings to libGammu, which allows access many phones.',
    author = "Michal Cihar",
    author_email = 'gammu@cihar.com',
    maintainer = "Michal Cihar",
    maintainer_email = "michal@cihar.com",
    platforms = ['Linux','Mac OSX','Windows XP/2000/NT','Windows 95/98/ME'],
    keywords = ['mobile', 'phone', 'SMS', 'contact', 'gammu', 'calendar', 'todo'],
    license = 'GPL',
    url = 'http://cihar.com/gammu/python',
    download_url = 'http://cihar.com/gammu/python',
    classifiers = [
        'Development Status :: 5 - Production/Stable',
        'License :: OSI Approved :: GNU General Public License (GPL)',
        'Operating System :: Microsoft :: Windows :: Windows 95/98/2000',
        'Operating System :: Microsoft :: Windows :: Windows NT/2000',
        'Operating System :: POSIX',
        'Operating System :: Unix',
        'Programming Language :: Python',
        'Programming Language :: C',
        'Topic :: Communications :: Telephony'
    ],
    cmdclass = {
        'build_ext': build_ext_gammu,
        },
    packages = ['gammu'],
    ext_modules = [gammumodule]
    )

