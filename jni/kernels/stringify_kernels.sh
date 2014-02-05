#!/bin/bash

kernels=blur

for name in $kernels
do
    IN=$name.cl
    OUT=$name.h

    echo "extern const char "$name"_kernel[] =" >$OUT
    sed -e 's/\\/\\\\/g;s/"/\\"/g;s/  /\\t/g;s/^/"/;s/$/\\n"/' $IN >>$OUT
    if [ $? -ne 0 ]
    then
        exit 1
    fi
    echo ";" >>$OUT
done
