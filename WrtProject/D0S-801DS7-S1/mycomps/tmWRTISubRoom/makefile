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

CFLAGS =  -Os -O2




CXXFLAGS = -DISROOM -Os -O2
INCLUDE = -I./src -I./include  -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL \
	-I../tmMediaNegotiate/include -I$(COMMON_COMPS)/tmWrtCommon/include \
	-I../tmSystemInfo/include -I$(COMMON_COMPS)/tmWrtLog/include


LIBS_TARGET =libtmWrtSubRoom.a
LIBS_CXXOBJS= src/wrt_subroom.o

all:$(LIBS_TARGET) 

lib: $(LIBS_TARGET)

$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET): $(LIBS_CXXOBJS)
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_CXXOBJS)

