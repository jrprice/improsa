#!/bin/bash
#
# gen_filters.sh (ImProSA)
# Copyright (c) 2014, James Price and Simon McIntosh-Smith,
# University of Bristol. All rights reserved.
#
# This program is provided under a three-clause BSD license. For full
# license terms please see the LICENSE file distributed with this
# source code.

functions="bilateral blur sharpen sobel"

OUTDIR=${1:-.}
mkdir -p $OUTDIR

for name in $functions
do
  if [ $name.cpp -ot $OUTDIR/$name\_cpu.s ] && \
     [ $name.cpp -ot $OUTDIR/$name\_gpu.s ]
  then
    echo "Skipping generation of halide $name"
    continue
  fi
  echo "Generating halide $name functions"

  g++ -lHalide -o $name $name.cpp
  if [ $? -ne 0 ]
  then
    exit 1
  fi

  for schedule in cpu gpu
  do
    ./$name $schedule halide_$name\_$schedule $OUTDIR/$name\_$schedule
    if [ $? -ne 0 ]
    then
      exit 1
    fi
  done

done
