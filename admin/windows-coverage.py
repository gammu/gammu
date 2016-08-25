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


import shutil
import subprocess
import sys


ROOT_DIR = 'c:\\projects\\gammu'
COVERAGE_BIN = 'c:\\projects\\gammu\\coverage.cov'
COVERAGE_TMP = '{0}.tmp'.format(COVERAGE_BIN)


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

    shutil.copy(COVERAGE_BIN, COVERAGE_TMP)

    # Coverage output
    cmd = [
        'OpenCppCoverage.exe',
        '--quiet',
        '--input_coverage', COVERAGE_TMP,
        '--export_type', 'binary:{0}'.format(COVERAGE_BIN),
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
