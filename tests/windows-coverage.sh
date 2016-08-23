#!/bin/sh

# Wrapper for CTest to execute every test through OpenCPPCoverage.

# Unfortunately there doesn't seem to be way to hook into the process than
# pretending to be memory tester.

# Usage:

# cmake -DMEMORYCHECK_COMMAND=windows-coverage.sh -DMEMORYCHECK_COMMAND_OPTIONS=--separator -DMEMORYCHECK_TYPE=Valgrind
# ctest -D NightlyMemCheck

# Configuration:

ROOT_DIR=c:/projects/gammu

# Consume arguments up to --separator
while [ "x$1" != "x--separator" -a "$#" -gt 0 ] ; do
    shift
done

# Check if something is left
if [ "$#" -eq 0 ] ; then
    echo "Bad invocation!"
    exit 1
fi

# Remove --seaprator from args
shift

# Execute
echo OpenCppCoverage.exe --quiet --export_type cobertura:coverage-tmp.xml --modules $ROOT_DIR --sources $ROOT_DIR -- "$@"

# Submit
codecov -X gcov -F "${CODECOV_FLAG}-${CONFIGURATION}" -f "coverage-tmp.xml"

