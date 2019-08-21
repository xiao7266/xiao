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

CFLAGS =   -D_GNU_SOURCE=1 -D_REENTRANT -Os
#CXXFLAGS = -shared 
INCLUDE =  -I./include -I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/freetype2 \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/

LIBS_TARGET =libSDL_ttf.a


LIBS_COBJS= ./SDL_ttf.o	\
	./charutil.o\
	./wrtfont.o\
	

BIN_TARGET  = TestSDLttf
DEMOCFLAGS= 
BIN_OBJS=./showfont.o
LD_PATH= ../build_lib
SYS_LD_PATH +=-L$(TARGET_SYSLIB_PATH)/lib
SYS_LD_PATH +=-L$(TARGET_SYSLIB_PATH)/usr/lib
SYS_LD_PATH += -L../dclib
DEP_LIBS = -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib -L$(LD_PATH) $(SYS_LD_PATH) libSDL_ttf.a  -lfreetype \
			-lSDL -lpthread -lts -lstdc++ -liconv\
			
#####################################

all:$(LIBS_TARGET) $(BIN_TARGET)

lib: $(LIBS_TARGET)

test:$(BIN_TARGET)

$(LIBS_CXXOBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE) -c $< -o $@
	
$(LIBS_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(LIBS_TARGET): $(LIBS_COBJS) 
	$(AR) -rcs $@ $^
#	$(CC) -shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
	
$(BIN_OBJS):%.o:%.c
	$(CC) $(DEMOCFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	cp $(BIN_TARGET) ../build_lib	

clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_COBJS) 

