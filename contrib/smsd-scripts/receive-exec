#!/bin/sh
PROGRAM=/bin/echo
for i in `seq $SMS_MESSAGES` ; do
    eval "$PROGRAM \"\${SMS_${i}_NUMBER}\" \"\${SMS_${i}_TEXT}\""
done
