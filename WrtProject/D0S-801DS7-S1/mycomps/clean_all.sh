#!/bin/sh
rm -rf build_lib/*.so 
rm -rf build_lib/*.*
rm -rf build_lib/*

cd tmSystemInfo
make clean
cd ..

cd tmWrtCommon
make clean
cd ..

cd tmWrtLog
make clean
cd ..

cd tmAppPort
make clean
cd ..

cd tmJitter
make clean
cd ..

cd tmG711
make clean
cd ..

cd tmWRTICaller
make clean
cd ..

cd ENet
make clean
cd ..

cd HttpClient
make clean
cd ..

cd tmWarning
make clean
cd ..

cd tmSipDataSource
make clean
cd ..

cd tmWRTSubPhone
make clean
cd ..

cd tmAudioInOut
make clean
cd ..

cd madplay
make clean
cd ..

cd SDL_draw-1.2.13
make clean
cd ..

cd SDL_ttf-2.0.11
make clean
cd ..

cd TMIMAGE
make clean
cd ..

cd tmWrtJiaju
make clean
cd ..

cd tmMediaNegotiate
make clean
cd ..

cd wrtip_room_flash
make clean
cd ..
