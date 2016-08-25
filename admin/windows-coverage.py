#!/usr/bin/env python

# Wrapper for CTest to execute every test through OpenCPPCoverage.

# Unfortunately there doesn't seem to be way to hook into the process than
# pretending to be valgrind memory tester.

# Usage:

# cmake \
#   -DMEMORYCHECK_COMMAND=windows-coverage.bat \
#   -DMEMORYCHECK_COMMAND_OPTIONS=--separator \
#   -DMEMORYCHECK_TYPE=Valgrind
# ctest -D NightlyMemCheck


import os
import subprocess
import sys


ROOT_DIR = 'c:\\projects\\gammu'
COVERAGE = 'c:\\projects\\gammu\\cobertura{0}.xml'


def main():

    # Parse params passed by CTest
    logfile = None
    for arg in sys.argv:
        if arg.startswith('--log-file='):
            logfile = arg.split('=', 1)[1]
            break
    if logfile is None:
        raise Exception('Missing --log-file')

    # Create empty file (CTest expects to find it)
    open(logfile, 'w')

    # Figure out test number (it's included in log name)
    test_num = os.path.basename(logfile).split('.')[1]
    coverage_file = COVERAGE.format(test_num)

    # Coverage command
    cmd = [
        'OpenCppCoverage.exe',
        '--quiet',
        '--export_type', 'cobertura:{0}'.format(coverage_file),
        '--modules', ROOT_DIR,
        '--sources', ROOT_DIR,
        '--'
    ]

    # Get test command out of passed args
    command = sys.argv[sys.argv.index('--separator') + 1:]

    # Execute with code coverage
    result = subprocess.call(cmd + command)

    # Propagate error code
    sys.exit(result)


if __name__ == '__main__':
    main()
