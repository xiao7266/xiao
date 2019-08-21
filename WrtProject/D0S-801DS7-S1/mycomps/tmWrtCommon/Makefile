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
INCLUDE = -I./include -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I../tmSystemInfo/include

LIBPATH=./lib
LIBS_TARGET = $(LIBPATH)/libtmWrtCommon.a
LIBS_OBJS=./src/pool.o ./src/zenmalloc.o

LIBS_CXXOBJS=./src/wrt_msg_queue.o 


######################################
#test
BIN_TARGET  = $(LIBPATH)/commondemo
BIN_OBJS=./demo/demo.o
DEP_LIBS = -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib $(LIBS_TARGET) -L../build_lib  -lpthread -lstdc++ -lSDL -ltmSystemInfo -lts -liconv
#####################################

all:$(LIBS_TARGET)

lib: $(LIBS_TARGET)

test: $(BIN_TARGET)

$(LIBS_OBJS):%.o:%.c
	$(CC) $(LIBCFLAGS) $(INCLUDE) -c $< -o $@
	
$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(LIBCFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS) $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib

$(BIN_OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_OBJS) $(LIBS_CXXOBJS) $(BIN_OBJS) $(BIN_TARGET)

