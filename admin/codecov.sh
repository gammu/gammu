#!/bin/sh

set -x

if [ ! -f cobertura1.xml ] ; then
    exit 0
fi

curl -o codecov.sh https://codecov.io/bash

# Upload in chunks of 15 files
ls cobertura*.xml | sed -e 's/^/-f /' | xargs -n 30 bash ./codecov.sh -X nocolor -X gcov -F "${CODECOV_FLAG},${CONFIGURATION}"
