#============================================
#Makefile for enet
#============================================
#arc-linux
#CC = ccache arc-linux-uclibc-gcc
#AR = arc-linux-uclibc-ar
#STRIP = arc-linux-uclibc-strip
#CXX = arc-linux-uclibc-g++

#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++

#linux
#CC = gcc
#AR = ar
#STRIP = strip

#

CFLAGS =  -Os -O2 -g -Wall -W -DT_SHAPE -DISROOM -DUSED_FLASH32M -DHAVE_USED_SDL

INCLUDE = -I./include \
	-I./src \
	-I../tmSystemInfo/include \
	-I../tmMediaNegotiate/include \
	-I../tmWRTICaller/include \
    	-I../TMIMAGE/include \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include \
	-I$(PROJECT_PATH)/release/include \
	-I$(COMMON_COMPS)/tmWrtCommon/include \
	-I$(COMMON_COMPS)/tmWRTSubPhone/include \
	-I$(COMMON_COMPS)/tmWarning/include \
	-I$(COMMON_COMPS)/ENet/include  \
	-I$(COMMON_COMPS)/tmWrtLog/include \
	-I$(COMMON_COMPS)/tmSipDataSource/include \
	-I$(COMMON_COMPS)/madplay/include \
	-I$(COMMON_COMPS)/tmG711/include \
	-I$(COMMON_COMPS)/tmJitter/include \
	-I$(COMMON_COMPS)/tmAppPort/include \
	-I$(COMMON_COMPS)/tmAudioInOut/include \
	-I$(COMMON_COMPS)/HttpClient/include  \
	-I$(COMMON_COMPS)/tmWRTISubRoom/include  \
	-I$(COMMON_COMPS)/tmIPCamera/include  \
	




LIBS_TARGET = libwrtip_room_flash.a
LIBS_OBJS=

LIBS_CXXOBJS=./src/smalldoor_handler.o \
	./src/cmd_handler.o \
	./src/sock_handler.o \
	./src/c_interface.o \
	./src/RoomGpioUtil.o \
	./src/info_handler.o \
	./src/audiovideo.o \
	./src/video.o \
	./src/sendvideo.o \
	./src/sstarvideo.o \
	./src/sstardisp.o \
	./src/sstarcommon.o \
	./src/sstargpio.o \
######################################
#test
LD_LIB_PATH=../build_lib
LD_SYS_PATH += -L$(TARGET_SYSLIB_PATH)/lib
LD_SYS_PATH += -L$(TARGET_SYSLIB_PATH)/usr/lib
BIN_TARGET  = wrtipdemo
BIN_OBJS=./demo/demo.o
DEP_LIBS = --sysroot $(TARGET_SYSLIB_PATH) $(LD_SYS_PATH) -L$(LD_LIB_PATH)   -lwrtip_room_flash\
		../dclib/libcxon2venc.a ../dclib/cxon2dec-6.0.0.a ../dclib/lib_dc_gfx.a ../dclib/libdcapi.a ../dclib/libcmem.a ../dclib/libYUVScale.a\
		-ltmWRTICaller   -lENet -lHttpClient  -lmadplay -lmad -ltmG711 ltmAppPort \
		-ltmJitter -ltmSipDataSource   -ltmWrtSubPhone -ltmAudioInOut \
		 -ltmWarning -ltmWrtLog -ltmWrtCommon  -ltmSystemInfo -lasound -lts   -lSDL -lpthread -lstdc++ \
		
#####################################

all:$(LIBS_TARGET) 
#$(BIN_TARGET)

lib: $(LIBS_TARGET)

test: $(BIN_TARGET)

$(LIBS_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c  $< -o $@
	
$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c  $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS) $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib

$(BIN_OBJS):%.o:%.cpp
	$(CC)  $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	cp wrtipdemo ../build_lib/
	
clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_OBJS) $(LIBS_CXXOBJS) $(BIN_OBJS) $(BIN_TARGET)

