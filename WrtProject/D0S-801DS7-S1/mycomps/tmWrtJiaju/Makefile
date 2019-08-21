#============================================
#Makefile for enet
#============================================
#arc-linux
#CC = ccache arc-linux-uclibc-gcc
#AR = arc-linux-uclibc-ar
#STRIP = arc-linux-uclibc-strip

#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++
#linux
#CC = gcc
#AR = ar
#STRIP = strip

CFLAGS =  -Os -DISROOM=1
CXXFLAGS = -Os -DISROOM=1
INCLUDE =  -I./include  -I../tmWrtCommon/include -I ../tmWrtLog/include -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I../tmSystemInfo/include


LIBS_TARGET =libtmWrtJiaju.a

LIBS_CXXOBJS= src/jiaju_send.o
LIBS_COBJS = src/tmJiaju.o

#LIBS_DEP_LIBS= ./deplib/libENet.a ./deplib/libpSOS2Linux.a -lpthread



all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)

$(LIBS_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET):  $(LIBS_COBJS) $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^	
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_CXXOBJS) $(LIBS_COBJS) 

