#!/bin/sh

set -x

if [ ! -d coverage ] ; then
    exit 0
fi

#echo ./coverage/*/cobertura.xml | sed -e 's/^/-f /' | xargs codecov -X search -X gcov -F "${CODECOV_FLAG},${CONFIGURATION}"
find ./coverage -name cobertura.xml | sed -e 's/^/-f /' | xargs bash -x ./admin/codecov-uploader.sh -X nocolor -X gcov -F "${CODECOV_FLAG},${CONFIGURATION}"
