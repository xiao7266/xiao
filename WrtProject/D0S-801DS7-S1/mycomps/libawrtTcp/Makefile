#============================================
#Makefile for libwrtcall build for linux library and android JNI library ,by xiao
#============================================
#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++

CFLAGS = 
INCLUDE = -I./ -I./src -I./include \
				  -I$(COMMON_COMPS)/libawrtcjson

LIBPATH=.
LIBS_TARGET = $(LIBPATH)/libwrtTcp.a
LIBS_OBJS=./src/wrtTcp.o \
					./src/tcpServerHandler.o \
					./src/tcpCall.o 
DEP_LIBS = $(COMMON_COMPS)/build_lib/libwrtCjson.a 

all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)


$(LIBS_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE)  -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS) $(DEP_LIBS)
	$(AR) -rcs $@ $^ 
	cp $(LIBS_TARGET) ../build_lib

clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_OBJS) 

