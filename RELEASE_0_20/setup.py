#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# vim: expandtab sw=4 ts=4 sts=4:
'''
python-gammu - Phone communication libary
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

# detect whether we should check for dependencies
skip_dependencies = False

# some defines
MAJOR_VERSION = 0
MINOR_VERSION = 20
EXTRA_VERSION = None

if EXTRA_VERSION == None:
    VERSION = '%d.%d' % (MAJOR_VERSION, MINOR_VERSION)
else:
    VERSION = '%d.%d.%d' % (MAJOR_VERSION, MINOR_VERSION, EXTRA_VERSION)

GAMMU_REQUIRED = (1,10,7)
PYTHON_REQUIRED = (2,3)

ADDITIONAL_PARAMS = [
        ('skip-deps', 's', 'skip checking for dependencies'),
        ('pkgconfig=', 'p', 'path to pkg-config binary'),
        ('pkgconfig-search-path=', 'P', 'path where pkg-config searches for installed packages'),
        ('gammu-libs=', None, 'path to libGammu'),
        ('gammu-incs=', None, 'path to gammu.h include'),
        ('gammu-cfg=', None, 'path to gammu config.h include'),
        ('gammu-build=', None, 'path where gammu library has been built'),
        ('gammu-is-static', None, 'link against static libGammu'),
        ('gammu-has-bluez', None, 'whether Gammu has bluetooth support'),
        ]

class install_gammu(distutils.command.install.install, object):
    """
    Install wrapper to support option for skipping deps
    """

    user_options = distutils.command.install.install.user_options + [('skip-deps', 's', 'skip checking for dependencies')]
    boolean_options = distutils.command.install.install.boolean_options + ['skip-deps']

    def initialize_options(self):
        global skip_dependencies
        super(install_gammu, self).initialize_options()
        self.skip_deps = skip_dependencies

    def finalize_options(self):
        global skip_dependencies
        super(install_gammu, self).finalize_options()
        if self.skip_deps:
            skip_dependencies = self.skip_deps

class build_gammu(distutils.command.build.build, object):
    """
    Custom build command to pass option to our build_ext.
    """
    user_options = distutils.command.build.build.user_options + ADDITIONAL_PARAMS
    boolean_options = distutils.command.build.build.boolean_options + ['skip-deps']

    def initialize_options(self):
        global skip_dependencies
        super(build_gammu, self).initialize_options()
        self.skip_deps = skip_dependencies
        self.pkgconfig = 'pkg-config'
        self.pkgconfig_search_path = None
        self.gammu_libs = None
        self.gammu_incs = None
        self.gammu_cfg = None
        self.gammu_build = None
        self.gammu_is_static = False
        self.gammu_has_bluez = False

class build_ext_gammu(distutils.command.build_ext.build_ext, object):
    """
    Custom build_ext command with dependency checking support.
    """
    user_options = distutils.command.build_ext.build_ext.user_options  + ADDITIONAL_PARAMS
    boolean_options = distutils.command.build_ext.build_ext.boolean_options + ['skip-deps']
    linklibs = []

    def initialize_options(self):
        global skip_dependencies
        super(build_ext_gammu, self).initialize_options()
        self.skip_deps = skip_dependencies
        self.pkgconfig = 'pkg-config'
        self.pkgconfig_search_path = None
        self.gammu_libs = None
        self.gammu_incs = None
        self.gammu_cfg = None
        self.gammu_build = None
        self.gammu_is_static = False
        self.gammu_has_bluez = False

    def finalize_options(self):
        global skip_dependencies
        super(build_ext_gammu, self).finalize_options()
        self.set_undefined_options('build',
                                   ('skip_deps', 'skip_deps'),
                                   ('pkgconfig', 'pkgconfig'),
                                   ('pkgconfig_search_path', 'pkgconfig_search_path'),
                                   ('gammu_libs', 'gammu_libs'),
                                   ('gammu_incs', 'gammu_incs'),
                                   ('gammu_cfg', 'gammu_cfg'),
                                   ('gammu_build', 'gammu_build'),
                                   ('gammu_is_static', 'gammu_is_static'),
                                   ('gammu_has_bluez', 'gammu_has_bluez'))
        if self.skip_deps:
            skip_dependencies = self.skip_deps

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
            print '\nYou need installed gammu as shared library and enable pkg-config to find it.\n\n' + \
                'This is done by invoking make installshared in gammu sources.'
            sys.exit(1)

    def read_gammu_version(self, path):
        try:
            distutils.log.info('checking for config.h in %s' % path)
            config_h = os.path.join(path, 'config.h')
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
            print 'WARNING: Can not read version from config.h!'
            self.gammu_incs = None
            return False

        found = found.groups()

        if len(found) != 1:
            print 'WARNING: Can not read version from config.h!'
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
        else:
            if self.gammu_is_static and self.gammu_has_bluez:
                self.linklibs.append('bluetooth')

    def process_build_tree(self):
        self.gammu_cfg = os.path.join(self.gammu_build, 'cfg')
        try:
            f = file(os.path.join(self.gammu_build, 'CMakeCache.txt'), 'r')
            distutils.log.info('detected new (CMake) style build tree')
            for line in f:
                res = re.search('Gammu_SOURCE_DIR:STATIC=(.*)', line)
                if res is not None:
                    self.gammu_incs = os.path.join(res.groups()[0], 'common')
                res = re.search('BUILD_SHARED_LIBS:INTERNAL=(.*)', line)
                if res is not None and res.groups()[0].upper() in ['OFF', 'FALSE', 'DISABLED']:
                    self.gammu_is_static = True
                    distutils.log.info('detected static build')
                res = re.search('BLUEZ_FOUND:INTERNAL=(.*)', line)
                if res is not None and res.groups()[0].upper() in ['1', 'TRUE', 'ON']:
                    self.gammu_has_bluez = True
                    distutils.log.info('detected bluez support')
            f.close()
            self.gammu_libs = self.gammu_build
        except IOError:
            # non CMake build
            distutils.log.info('detected old (autoconf) style build tree')
            self.gammu_incs = os.path.join(self.gammu_build, 'common')
            self.gammu_libs = os.path.join(self.gammu_build, 'common')

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

        if type(self.gammu_libs) is str:
            self.library_dirs.append(self.gammu_libs)
        else:
            self.library_dirs += self.gammu_libs

        if type(self.linklibs) is str:
            self.libraries.append(self.linklibs)
        else:
            self.libraries += self.linklibs

    def write_config(self):
        """
        We could use defines from compiler, but string defines are
        broken on different platforms, so this way is safer.
        """
        include_path = os.path.join(self.build_temp, 'include')
        filename = os.path.join(include_path, 'pyg-config.h')
        distutils.dir_util.mkpath(include_path)
        self.include_dirs.append(include_path)
        if not distutils.dep_util.newer('setup.py', filename):
            return
        distutils.log.info('generating pyg-config.h')
        header = file(filename, 'w')
        dumpconfig(header)
        header.close()

    def run (self):
        global skip_dependencies
        if not skip_dependencies:
            self.check_requirements()
        self.write_config()
        super(build_ext_gammu, self).run()

def dumpconfig(header):
    header.write('/* Automatically generated header file for python-gammu */\n')
    header.write('/* Generated on %s by setup.py */\n' % datetime.datetime.now().ctime())
    header.write('\n')
    header.write('#ifndef PYG_CONFIG_H\n')
    header.write('#define PYG_CONFIG_H\n')
    header.write('\n')
    header.write('#define PYTHON_GAMMU_VERSION "%s"\n' % VERSION)
    if sys.platform == 'win32':
        header.write('#ifndef WIN32\n')
        header.write('#  define WIN32\n')
        header.write('#endif\n')
    header.write('#endif\n')

if sys.argv[1] == '--dump-config':
    dumpconfig(sys.stdout)
    sys.exit(0)

gammumodule = Extension('gammu.Core',
    include_dirs = ['.'],
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
        'convertors/file.c',
        'convertors/call.c',
        'convertors/wap.c',
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
        'build': build_gammu,
        'install': install_gammu,
        },
    packages = ['gammu'],
    ext_modules = [gammumodule]
    )

