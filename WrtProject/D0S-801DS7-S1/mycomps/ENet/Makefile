#============================================
#Makefile for enet
#============================================
#arc-linux
#CC = ccache arc-linux-uclibc-gcc
#AR = arc-linux-uclibc-ar
#STRIP = arc-linux-uclibc-strip

#arm-linux
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++
#linux
#CC = gcc
#AR = ar
#STRIP = strip

CFLAGS = -DHAS_SOCKLEN_T  -Os
INCLUDE = -I./include


LIBS_TARGET = libENet.a
LIBS_OBJS=./src/unix.o ./src/callbacks.o ./src/host.o ./src/list.o ./src/packet.o ./src/peer.o ./src/protocol.o 

######################################
#test
BIN_TARGET  = ./enetdemo
BIN_OBJS=./test/test.o
DEP_LIBS = $(LIBS_TARGET) -lpthread
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
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_OBJS) $(BIN_OBJS) $(BIN_TARGET)

