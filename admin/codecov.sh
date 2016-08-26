#!/bin/sh

set -x

if [ ! -f cobertura1.xml ] ; then
    exit 0
fi

# Download codecov script
curl -s -o codecov.sh https://codecov.io/bash

# Upload in chunks of 50 files
#ls cobertura*.xml | sed -e 's/^/-f /' | xargs -n 10 bash ./codecov.sh -X nocolor -X gcov
ls cobertura*.xml | sed -e 's/^/-f /' | xargs -n 100 ./admin/codecov-uploader.sh -X nocolor -X gcov
