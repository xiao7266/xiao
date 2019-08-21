#============================================
#Makefile for libcjson build by xiao
#============================================
#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++

CFLAGS = 
INCLUDE = -I./

LIBPATH=.
LIBS_TARGET = $(LIBPATH)/libwrtCjson.a
LIBS_OBJS=./wrtCjson.o cJSON.o

all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)


$(LIBS_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE)  -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS)
	$(AR) -rcs $@ $^
	cp $(LIBS_TARGET) ../build_lib

clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_OBJS) 

