#!/bin/sh

set -x

if [ ! -d coverage ] ; then
    exit 0
fi

echo ./coverage/*/cobertura.xml | sed -e 's/^/-f /' | xargs codecov -X search -X gcov -F "${CODECOV_FLAG},${CONFIGURATION}"
