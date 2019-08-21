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

CFLAGS = -Os

INCLUDE =  -I./include  


LIBS_TARGET =libtmAppPort.a

LIBS_COBJS= src/appport.o


all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)

	
$(LIBS_COBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
	
$(LIBS_TARGET): $(LIBS_COBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib  
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_COBJS)

