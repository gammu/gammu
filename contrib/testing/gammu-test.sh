#!/bin/sh

set -x

cleanup() {
    cd /home/mcihar/private/gammu
    git checkout -- locale/*/*.po
}

trap cleanup INT QUIT EXIT

cd /home/mcihar/private/gammu/build-suse/

make clean
find . -name '*.gc*' | xargs -r rm
find . -type d -name Testing | xargs -r rm -rf
make ExperimentalConfigure
make ExperimentalBuild
#ctest -R 'py-' -D ExperimentalTest
make ExperimentalTest

if grep -A 1 'Status="failed"' Testing/2010*/Test.xml | grep -q py-smsd-testing ; then
    echo 'Failed!'
    exit 1
fi
