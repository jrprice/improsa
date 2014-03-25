#!/bin/bash
#
# stringify_kernels.sh (ImProSA)
# Copyright (c) 2014, James Price and Simon McIntosh-Smith,
# University of Bristol. All rights reserved.
#
# This program is provided under a three-clause BSD license. For full
# license terms please see the LICENSE file distributed with this
# source code.

kernels="bilateral blur sharpen sobel"

for name in $kernels
do
    IN=$name.cl
    OUT=$name.h

    if [ $IN -ot $OUT ]
    then
        echo "Skipping generation of OpenCL $name kernel"
        continue
    fi
    echo "Generating OpenCL $name kernel"

    echo "const char *"$name"_kernel =" >$OUT
    sed -e 's/\\/\\\\/g;s/"/\\"/g;s/  /\\t/g;s/^/"/;s/$/\\n"/' $IN >>$OUT
    if [ $? -ne 0 ]
    then
        exit 1
    fi
    echo ";" >>$OUT
done
