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

CFLAGS =  -Os
CXXFLAGS = -Os
INCLUDE =  -I./include  -I../tmWrtCommon/include


LIBS_TARGET =libtmJitter.a
LIBS_CXXOBJS= src/Myjitter.o
LIBS_COBJS= src/jitterbuf.o

#LIBS_DEP_LIBS= ./deplib/libENet.a ./deplib/libpSOS2Linux.a -lpthread



all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)

$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
	
$(LIBS_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET): $(LIBS_CXXOBJS) $(LIBS_COBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_CXXOBJS) $(LIBS_COBJS)

