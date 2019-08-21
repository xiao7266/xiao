#!/bin/bash
./configure \
		 --target=arm-linux-uclibcgnueabi\
		--host=arm-linux-uclibcgnueabi\
		 --prefix =/opt/northstar_equinox_ip_camera_ext_SDK_2_6_4_IPCAM_sdk/output/staging/usr\
		 --with-sdl-prefix=../SDL-1.2.14/include\
		 --with-sdl-exec-prefix=../build_lib\


make clean
make all

