#============================================
#Makefile for enet
#============================================
#arc-linux
#CC = ccache arc-linux-uclibc-gcc
#AR = arc-linux-uclibc-ar
#STRIP = arc-linux-uclibc-strip
#CXX = arc-linux-uclibc-g++

#linux
#CC = gcc
#AR = ar
#STRIP = strip

#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++

CFLAGS= -Os
LIBCFLAGS = -Os
INCLUDE = -I./include  -I../tmWrtCommon/include

LIBPATH=./lib
LIBS_TARGET = libtmSipDataSource.a
#LIBS_OBJS=

LIBS_CXXOBJS=./src/sip_data_sources.o




all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)

$(LIBS_OBJS):%.o:%.c
	$(CC) $(LIBCFLAGS) $(INCLUDE) -c $< -o $@
	
$(LIBS_CXXOBJS):%.o:%.cpp
	$(CC) $(LIBCFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS) $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
	
clean:
	rm -f *.o *.a $(LIBS_TARGET) $(LIBS_OBJS) $(LIBS_CXXOBJS)

