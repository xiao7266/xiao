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

CFLAGS =  -Os
#CXXFLAGS = -shared 
INCLUDE =  -I./include -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL -I$(PROJECT_PATH)/release/include -I../tmWrtLog/include -I../tmWrtCommon/include

LIBS_TARGET =libtmAudioInOut.a

#src/sndwav_common.o 

LIBS_CXXOBJS= 
#src/wav_parser.o \
		#src/sndwav_common.o\
		
LIBS_COBJS= src/wrt_audio2.o

#LIBS_DEP_LIBS= ./deplib/libENet.a ./deplib/libpSOS2Linux.a -lpthread

####################################
LD_LIB_PATH += -L../build_lib
LD_SYS_PATH += -L$(TARGET_SYSLIB_PATH)/lib
LD_SYS_PATH += -L$(TARGET_SYSLIB_PATH)/usr/lib
BIN_TARGET  = tstaudio
BIN_OBJS=tst/tst.o
DEP_LIBS = $(LD_LIB_PATH) $(LD_SYS_PATH)  -ltmAudioInOut -ltmWrtLog -ltmWrtCommon  -ltmSystemInfo -lSDL -lts  -lpthread -lasound  -lm -lstdc++ \


all:$(LIBS_TARGET) 
#$(BIN_TARGET)

lib: $(LIBS_TARGET)

test: $(BIN_TARGET)

$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(LIBS_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c  $< -o $@

$(LIBS_TARGET): $(LIBS_COBJS) $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^
	cp $(LIBS_TARGET) ../build_lib

$(BIN_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS)  
	$(STRIP) $(BIN_TARGET)
		
clean:
	rm -f *.so *.o $(LIBS_TARGET) $(LIBS_COBJS) $(LIBS_CXXOBJS) $(BIN_TARGET)	

