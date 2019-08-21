#!/bin/sh

make_dirs="tmSystemInfo 
tmWrtCommon 
tmWrtLog 
tmAppPort 
tmJitter 
tmG711 
tmWRTICaller 
ENet 
HttpClient
tmWarning
tmSipDataSource
tmWRTSubPhone
tmAudioInOut
madplay
SDL_draw-1.2.13
SDL_ttf-2.0.11
TMIMAGE
tmWrtJiaju
tmMediaNegotiate
wrtip_room_flash
"

BUILD_PATH=$PWD
function check_make_result()
{
	if [ $? != 0 ]; then
		echo -e '\033[0;31;1m'
		echo "编译失败，退出编译！！！"
		echo -e '\033[0m'
		exit		
	fi
}

function make_loop()
{
	rm -rf build_lib/*.so 
	rm -rf build_lib/*.*
	rm -rf build_lib/*
	mkdir -p build_lib
	
	for dir in $make_dirs;do
		cd $BUILD_PATH/$dir
		make clean
		make lib
		check_make_result
	done
}
make_loop