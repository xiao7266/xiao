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

CFLAGS = -DARM_LINUX   -O3
INCLUDE = -I./src -I./include  -I../ENet/include/ -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I../tmWrtCommon/include -I../tmDefenceInfo/include

#LIBPATH=./lib
LIBS_TARGET =libtmWarning.a
LIBS_OBJS=./src/warningtask.o 
#LIBS_DEP_LIBS= ./deplib/libENet.a ./deplib/libpSOS2Linux.a -lpthread

######################################
#warndemo
BIN_TARGET  = warndemo
BIN_OBJS=./demo/test.o
DEP_LIBS = -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib -L../build_lib -lSDL -lpthread  -ltmWarning  -lENet -ltmWrtCommon -lts -liconv
#####################################

all:$(LIBS_TARGET) $(BIN_TARGET)
lib: $(LIBS_TARGET)
	cp $(LIBS_TARGET) ../build_lib
test: $(BIN_TARGET)

$(LIBS_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE)  -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib

$(BIN_OBJS):%.o:%.c
	$(CC) $(INCLUDE)  -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)

clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_OBJS) $(BIN_OBJS) $(BIN_TARGET)
