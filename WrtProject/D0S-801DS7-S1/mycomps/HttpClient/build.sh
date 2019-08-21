#!/bin/bash
make clean
make lib
cp  ./libHttpClient.so  ../build_lib
make test
