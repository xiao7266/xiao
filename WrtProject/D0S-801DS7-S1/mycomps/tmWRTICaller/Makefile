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

CFLAGS = -Os -pipe -O2 
#CXXFLAGS = -shared 
INCLUDE =  -I./include -I../tmMediaNegotiate/include


LIBS_TARGET =libtmWRTICaller.a

LIBS_COBJS= src/tmWRTICaller.o

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

