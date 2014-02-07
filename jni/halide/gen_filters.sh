#!/bin/bash

filters=blur

if [ "$1" == clean ]
then
    for name in $filters
    do
        rm -rf "$name"{,_cpu{.s,.h},_gpu{.s,.h}}
    done
    exit 0
fi

for name in $filters
do
    if [ $name.cpp -ot $name ]
    then
        echo "Skipping generation of halide $name"
        continue
    fi

    g++ -lHalide -o $name $name.cpp
    if [ $? -ne 0 ]
    then
        exit 1
    fi

    ./$name
    if [ $? -ne 0 ]
    then
        exit 1
    fi
done
