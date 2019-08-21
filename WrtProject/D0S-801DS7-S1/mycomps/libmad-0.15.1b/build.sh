#!/bin/bash
./configure --prefix=/home/beal.wu/workspace/release/project --disable-static --host=arm-linux-gnueabihf --target=arm-linux-gnueabihf
make clean
make all
cp .libs/libmad.so ../build_lib
cp .libs/libmad.so.0 ../build_lib

