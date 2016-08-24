#!/usr/bin/env python

# Wrapper for CTest to execute every test through OpenCPPCoverage.

# Unfortunately there doesn't seem to be way to hook into the process than
# pretending to be memory tester.

# Usage:

# cmake \
#   -DMEMORYCHECK_COMMAND=windows-coverage.py \
#   -DMEMORYCHECK_COMMAND_OPTIONS=--separator \
#   -DMEMORYCHECK_TYPE=Valgrind
# ctest -D NightlyMemCheck


import sys
import os
import subprocess


ROOT_DIR = 'c:\\projects\\gammu'
COVERAGE_DIR = 'c:\\projects\\gammu\\coverage'


def main():
    logfile = None
    for arg in sys.argv:
        if arg.startswith('--log-file='):
            logfile = arg.split('=', 1)[1]
            break
    if logfile is None:
        raise Exception('Missing --log-file')

    # Create empty file
    open(logfile, 'w')

    # Create output directory
    if not os.path.exists(COVERAGE_DIR):
        os.makedirs(COVERAGE_DIR)

    # Figure out test number
    test_num = os.path.basename(logfile).split('.')[1]

    # Coverage output
    coverage_file = os.path.join(
        COVERAGE_DIR,
        'coverage-{0}.xml'.format(test_num)
    )
    cmd = [
        'OpenCppCoverage.exe',
        '--quiet',
        '--export_type', 'cobertura:{0}'.format(coverage_file),
        '--modules', ROOT_DIR,
        '--sources', ROOT_DIR,
        '--'
    ]

    # Get command out of passed args
    command = sys.argv[sys.argv.index('--separator') + 1:]

    # Execute with code coverage
    result = subprocess.call(cmd + command)

    # Propagate error code
    sys.exit(result)


if __name__ == '__main__':
    main()
