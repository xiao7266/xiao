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
CXX = $(TARGET_TOOLCHAIN)g++

#linux
#CC = gcc
#AR = ar
#STRIP = strip

CFLAGS =  -Os
CXXFLAGS = -DISROOM -Os
INCLUDE = -I./src -I./include  -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I../tmWrtCommon/include -I../tmSystemInfo/include -I../tmWrtLog/include

#LIBPATH = ./lib
LIBS_TARGET = libtmWrtSubPhone.a
LIBS_CXXOBJS = src/wrt_sub_phone.o

#LIBS_DEP_LIBS = ./deplib/libENet.a ./deplib/libpSOS2Linux.a -lpthread

all:$(LIBS_TARGET)
lib:$(LIBS_TARGET)

$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET): $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_CXXOBJS)

