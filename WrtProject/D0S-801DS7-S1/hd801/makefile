#============================================
#Makefile for wrt_room_flash
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

#-DHAVE_SUB_PHONE

CFLAGS = -O2 -g -Wall -W -DT_SHAPE -DISROOM -DUSED_FLASH32M -DHAVE_USED_SDL -DTHREE_UI
INCLUDE = -I./include \
	-I./src/res \
	-I./src/screenkeep \
	-I./src \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/ \
	-I$(PRIVATE_COMPS)/tmWRTICaller/include \
	-I$(PRIVATE_COMPS)/tmSystemInfo/include \
	-I$(PRIVATE_COMPS)/TMIMAGE/include \
	-I$(PRIVATE_COMPS)/tmMediaNegotiate/include \
	-I$(PRIVATE_COMPS)/wrtip_room_flash/include \
	-I$(PRIVATE_COMPS)/tmIPCamera/include \
	-I$(COMMON_COMPS)/tmWrtJiaju/include \
	-I$(COMMON_COMPS)/tmWrtLog/include \
	-I$(COMMON_COMPS)/SDL_ttf-2.0.11 \
	-I$(COMMON_COMPS)/SDL_draw-1.2.13/include\
	-I$(COMMON_COMPS)/tmWarning/include  \
	-I$(COMMON_COMPS)/tmSipDataSource/include \
	-I$(COMMON_COMPS)/tmWrtCommon/include \
	-I$(COMMON_COMPS)/tmAppPort/include \
	-I$(COMMON_COMPS)/tmAudioInOut/include \
	-I$(COMMON_COMPS)/ENet/include \
	-I$(PROJECT_PATH)/release/include \
	-I$(COMMON_COMPS)/libawrtTcp/include \
	-I$(COMMON_COMPS)/libawrtcjson \


######################################
#test
LD_LIB_PATH= ../comps/build_lib
LD_LIB_PATH2= $(TARGET_LD_PATH)
LD_SYS_PATH += -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib
BIN_TARGET  = wrt_room

BIN_COBJS= src/SDL_LList.o \
		src/window.o \
		src/arp.o \
		src/mainHandler.o 

BIN_CXXOBJS=src/idtoip.o \
	src/language_res.o\
	src/main.o\
	src/main_program.o\
	src/main/ConfigRoomSys.o\
	src/main/DialogHandle2.o\
	src/main/FeeHandler.o\
	src/main/HelpHandler.o\
	src/main/JiajuHandler.o\
	src/main/JiajuSetHandler.o\
	src/main/LocalSetHandler.o\
	src/main/LogHandler.o\
	src/main/MainHandler.o\
	src/main/main_ui_two.o\
	src/main/modifyipandmask.o\
	src/main/MsgHandler.o\
	src/main/number_res.o\
	src/main/PasswordHandler.o\
	src/main/PhotoHandler.o\
	src/main/Photo_TextHandler.o\
	src/main/public_res.o\
	src/main/public_res_en.o\
	src/main/RingHandler.o\
	src/main/mutilmedia.o\
	src/main/SafeHandler.o\
	src/main/ServiceHandler.o\
	src/main/SetupHandler.o\
	src/main/SysHandler.o\
	src/main/telephoneHandler.o\
	src/main/VoiceHandler.o\
	
DEP_LIBS = -L$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/lib $(LD_SYS_PATH) -L$(LD_LIB_PATH) -L$(BUILD_APP_PATH)/A2S-801D/mycomps \
	-L $(PROJECT_PATH)/release/nvr/i2m/011A/glibc/8.2.1/lib/dynamic \
	$(PRIVATE_COMPS)/build_lib/libwrtip_room_flash.a \
	$(PRIVATE_COMPS)/build_lib/libtmMediaNegotiate.a \
	$(PRIVATE_COMPS)/build_lib/libtmWRTICaller.a \
	$(PRIVATE_COMPS)/build_lib/libtmImage.a\
	$(PRIVATE_COMPS)/build_lib/libtmSystemInfo.a  \
	$(COMMON_COMPS)/build_lib/libSDL_ttf.a   \
	$(COMMON_COMPS)/build_lib/libSDL_draw.a   \
	$(COMMON_COMPS)/build_lib/libtmWrtJiaju.a  \
	$(COMMON_COMPS)/build_lib/libtmAppPort.a \
	$(COMMON_COMPS)/build_lib/libENet.a\
	$(COMMON_COMPS)/build_lib/libHttpClient.a  \
	$(COMMON_COMPS)/build_lib/libmadplay.a \
	$(COMMON_COMPS)/build_lib/libtmG711.a \
	$(COMMON_COMPS)/build_lib/libtmJitter.a \
	$(COMMON_COMPS)/build_lib/libtmSipDataSource.a   \
	$(COMMON_COMPS)/build_lib/libtmWrtSubPhone.a \
	$(COMMON_COMPS)/build_lib/libtmAudioInOut.a \
	$(COMMON_COMPS)/build_lib/libtmWarning.a \
	$(COMMON_COMPS)/build_lib/libtmWrtLog.a \
	$(COMMON_COMPS)/build_lib/libtmWrtCommon.a  \
	$(COMMON_COMPS)/build_lib/libwrtTcp.a  \
	$(COMMON_COMPS)/build_lib/libwrtCjson.a  \
	-lts -lstdc++ \
	-lSDL -lfreetype -lmad  -liconv -lm -lpthread -lmi_sys -lmi_disp -lmi_panel -lmi_ao -lmi_common -lmi_ai -lmi_vdec -lmi_divp\
		
#####################################

all:$(BIN_TARGET)

test: $(BIN_TARGET)

$(BIN_CXXOBJS):%.o:%.cpp
	$(CC)  $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# -lm

$(BIN_TARGET): $(BIN_CXXOBJS) $(BIN_COBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_COBJS) $(BIN_CXXOBJS) $(DEP_LIBS)
	$(STRIP) $(BIN_TARGET)

clean:
	rm -f *.o $(BIN_COBJS) $(BIN_CXXOBJS) $(BIN_TARGET)
