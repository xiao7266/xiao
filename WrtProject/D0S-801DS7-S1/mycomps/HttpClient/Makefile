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
INCLUDE = -I./include -I./src

LIBPATH=.
LIBS_TARGET = $(LIBPATH)/libHttpClient.a
LIBS_OBJS=./src/tmHTTPIODriver_Iod.o ./src/tmHTTPIODriver_Clnt.o

######################################
#test
DEMO_CFLAGS=
BIN_TARGET  = $(LIBPATH)/httpclientdemo
BIN_OBJS=./demo/httpclientdemo.o
LD_PATH=../build_lib
DEP_LIBS = -L$(LD_PATH) -lHttpClient
#####################################

all:$(LIBS_TARGET) $(BIN_TARGET)

lib: $(LIBS_TARGET)
test: $(BIN_TARGET)

$(LIBS_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib

$(BIN_OBJS):%.o:%.c
	$(CC) $(DEMO_CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_OBJS) $(BIN_OBJS) $(BIN_TARGET)

