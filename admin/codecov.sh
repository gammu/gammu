#!/bin/sh

set -x

if [ ! -d coverage ] ; then
    exit 0
fi

find coverage -name cobertura.xml | sed -e 's/^/-f /' input.txt | xargs codecov -X search -X gcov -F "${CODECOV_FLAG},${CONFIGURATION}"
