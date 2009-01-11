#!/bin/bash

if ! type $1-gcc > /dev/null; then
    echo "Usage: $0 MinGW-prefix"
    exit 1
fi

$1-gcc mingw/wine_linux_launcher.c -o wine_linux_launcher.exe

for i in gcc g++ dllwrap windres wx-config; do
    ln -sf wine_linux_launcher.exe $i.exe
done
