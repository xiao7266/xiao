#!/bin/bash
make clean
make lib
cp libtmAudioInOut.so  ../build_lib
make test


