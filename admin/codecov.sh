#!/bin/sh

set -x

if [ ! -f cobertura1.xml ] ; then
    exit 0
fi

# Download codecov script
curl -s -o codecov.sh https://codecov.io/bash

# Upload in chunks of 5 files
ls cobertura*.xml | sed -e 's/^/-f /' | xargs -n 10 bash ./codecov.sh -X nocolor -X gcov
