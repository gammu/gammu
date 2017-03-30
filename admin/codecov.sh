#!/bin/sh

set -x

if [ ! -f cobertura1.xml ] ; then
    exit 0
fi

export PATH="C:/MinGW/bin:$PATH"

# Download codecov script
curl -s -o codecov.sh https://codecov.io/bash

# Merge files
python ./contrib/coveragehelper/merge-cobertura.py --output cobertura.xml --match 'cobertura*.xml'

# Upload in chunks of 5 files
bash ./codecov.sh -X nocolor -X gcov -F windows -f cobertura.xml -f cobertura1.xml
