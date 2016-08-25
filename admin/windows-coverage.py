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


import os
import subprocess
import sys


ROOT_DIR = 'c:\\projects\\gammu'
COVERAGE_BIN = 'c:\\projects\\gammu\\coverage%d.cov'
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

    # Figure out test number
    test_num = os.path.basename(logfile).split('.')[1]

    # Test block
    test_block = int(test_num) / 100
    coverage_tmp = COVERAGE_TMP % test_block
    coverage_bin = COVERAGE_BIN % test_block

    # Create temporary file for coverage merge
    if os.path.exists(coverage_tmp):
        os.remove(coverage_tmp)
    os.rename(coverage_bin, coverage_tmp)

    # Coverage output
    cmd = [
        'OpenCppCoverage.exe',
        '--quiet',
        '--input_coverage', coverage_tmp,
        '--export_type', 'binary:{0}'.format(coverage_bin),
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
