#!/bin/sh

set -x

if [ ! -d coverage ] ; then
    exit 0
fi

curl -s https://codecov.io/bash -O codecov.sh

find coverage -name cobertura.xml | sed -e 's/^/-f /' input.txt | xargs bash -x ./codecov.sh -X gcov -F "${CODECOV_FLAG},${CONFIGURATION}"
