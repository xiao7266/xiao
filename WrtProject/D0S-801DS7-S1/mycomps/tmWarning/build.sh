#!/bin/bash
make clean
make lib
cp libtmWarning.so ../build_lib
make test
