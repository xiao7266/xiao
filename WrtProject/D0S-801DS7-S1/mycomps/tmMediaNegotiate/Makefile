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


CFLAGS =  -Os -pipe -O2
#CXXFLAGS = -shared 
INCLUDE =  -I./include


LIBS_TARGET =libtmMediaNegotiate.a
#LIBS_CXXOBJS= src/Myjitter.o
LIBS_COBJS= src/media_negotiate.o

#LIBS_DEP_LIBS= ./deplib/libENet.a ./deplib/libpSOS2Linux.a -lpthread



all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)

	
$(LIBS_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET): $(LIBS_COBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_COBJS)

