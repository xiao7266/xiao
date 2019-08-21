#!/bin/bash
cd ../
cd tmWrtCommon
./build.sh
cd ..
cd tmWrtLog
make clean
make lib
cp libtmWrtLog.so ../build_lib
make test


