#============================================
#Makefile for enet
#============================================
#arc-linux
#CC = ccache arc-linux-uclibc-gcc
#AR = arc-linux-uclibc-ar
#STRIP = arc-linux-uclibc-strip
#CXX = arc-linux-uclibc-g++

#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++

#linux
#CC = gcc
#AR = ar
#STRIP = strip

CFLAGS = -DISROOM -Os -pipe -O2 
INCLUDE = -I./include -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I ../tmWRTICaller/include 

LIBPATH=.
LIBS_TARGET = $(LIBPATH)/libtmSystemInfo.a
LIBS_OBJS=./src/tmSystemInfo.o




######################################
#test
BIN_TARGET  = $(LIBPATH)/sysinfodemo
BIN_OBJS=./demo/demo.o
DEP_LIBS = -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib $(LIBS_TARGET) -lSDL -lpthread -lts -liconv
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
	rm -f *.o $(LIBS_TARGET) $(LIBS_OBJS) $(LIBS_CXXOBJS) $(BIN_OBJS) $(BIN_TARGET)

