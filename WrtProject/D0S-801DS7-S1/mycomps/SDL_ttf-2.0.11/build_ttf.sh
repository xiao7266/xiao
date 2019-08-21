#!/bin/bash
./configure  \
		--target=arm-linux-uclibcgnueabi \
		--host=arm-linux-uclibcgnueabi  \
		--build=i386-pc-linux-gnu \
		--prefix=/opt/northstar_equinox_ip_camera_ext_SDK_2_6_4_IPCAM_sdk/output/staging/usr \
		--exec-prefix=/usr \
		--bindir=/bin \
		--sbindir=/sbin \
		--libdir=/lib \
		--libexecdir=/lib \
		--sysconfdir=/etc \
		--datadir=/share \
		--localstatedir=/var \
		--includedir=/include \
		--mandir=/man \
		--infodir=/info \
		--without-x \
		--with-freetype-prefix=/opt/northstar_equinox_ip_camera_ext_SDK_2_6_4_IPCAM_sdk/output/staging/usr/include/freetype2 \
		--with-sdl-prefix=/opt/northstar_equinox_ip_camera_ext_SDK_2_6_4_IPCAM_sdk/output/staging/usr \


make clean
make all

