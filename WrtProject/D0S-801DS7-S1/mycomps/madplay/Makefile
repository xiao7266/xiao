#============================================
#Makefile for enet
#============================================
#ARM-LINUX
CC = $(TARGET_TOOLCHAIN)gcc
AR = $(TARGET_TOOLCHAIN)ar
STRIP = $(TARGET_TOOLCHAIN)strip
CXX=$(TARGET_TOOLCHAIN)g++

CFLAGS = -Os
INCLUDE = -Iinclude -I../libmad-0.15.1b
DEMOINCLUDE=-I../HttpClient/include -I../tmWrtCommon/include -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I../libmad-0.15.1b -I./include -I../tmAudioInOut/include -I$(TARGET_SDK)

LIBS_TARGET = libmadplay.a
LIBS_OBJS=./src/madlld.o 

######################################
#test
BIN_TARGET  = madplaydemo
DEMOCFLAGS= 
BIN_OBJS=./demo/demo.o
LD_PATH= ../build_lib
SYS_LD_PATH +=-L$(TARGET_SYSLIB_PATH)/lib
SYS_LD_PATH +=-L$(TARGET_SYSLIB_PATH)/usr/lib
SYS_LD_PATH += -L../dclib
DEP_LIBS = -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib -L$(LD_PATH) $(SYS_LD_PATH) -lHttpClient -lmadplay -ltmAudioInOut -ltmWrtCommon \
			-lSDL -lpthread -lts  -lz -lmad  -lasound -lstdc++  -ltmWrtLog -ltmSystemInfo
#####################################

all:$(LIBS_TARGET)
# $(BIN_TARGET)

lib: $(LIBS_TARGET)
test: $(BIN_TARGET)

$(LIBS_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET):$(LIBS_OBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib

$(BIN_OBJS):%.o:%.cpp
	$(CC) $(DEMOCFLAGS) $(DEMOINCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	cp madplaydemo ../build_lib
	
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_OBJS) $(BIN_OBJS) $(BIN_TARGET)

