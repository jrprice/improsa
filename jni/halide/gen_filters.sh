#!/bin/bash

functions="blur sharpen sobel"

if [ "$1" == clean ]
then
  for name in $functions
  do
    rm -rf "$name"{,_cpu{.s,.h},_gpu{.s,.h}}
  done
  exit 0
fi

for name in $functions
do
  if [ $name.cpp -ot $name ]
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
    ./$name $schedule
    if [ $? -ne 0 ]
    then
      exit 1
    fi
  done

done
