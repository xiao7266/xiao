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
#CC = gcc CHIPID_NORTHSTAR
#AR = ar
#STRIP = strip
#-lop_env -lcsc -ldcapi -lpthread -lrt -ljpegpipe -limage_container
#-D__ARM -DOA_CHIPID=CHIPID_NORTHSTAR -DBUILD_DC_ACCEL_LIB=1

CFLAGS =  -Os -pipe -O2
INCLUDE =  -I./include \
	-I$(BUILD_APP_PATH)/3PartyLib/3PARTY_INSTALL/include/SDL \
	-I$(COMMON_COMPS)/tmWrtCommon/include \
	-I$(COMMON_COMPS)/tmWrtLog/include \

LIBS_TARGET =libtmImage.a
LIBS_COBJS= \
		src/jpeg.o\
		src/generic.o \
		src/helpers.o \
		src/JpegLib/jcapimin.o\
        src/JpegLib/jcapistd.o\
        src/JpegLib/jccoefct.o\
        src/JpegLib/jccolor.o\
        src/JpegLib/jcdctmgr.o\
        src/JpegLib/jchuff.o\
        src/JpegLib/jcinit.o\
        src/JpegLib/jcmainct.o\
        src/JpegLib/jcmarker.o\
        src/JpegLib/jcmaster.o\
        src/JpegLib/jcomapi.o\
        src/JpegLib/jcparam.o\
        src/JpegLib/jcphuff.o\
        src/JpegLib/jcprepct.o\
        src/JpegLib/jcsample.o\
        src/JpegLib/jctrans.o\
        src/JpegLib/jdapimin.o\
        src/JpegLib/jdapistd.o\
        src/JpegLib/jdatadst.o\
        src/JpegLib/jdatasrc.o\
        src/JpegLib/jdcoefct.o\
        src/JpegLib/jdcolor.o\
        src/JpegLib/jddctmgr.o\
        src/JpegLib/jdhuff.o\
        src/JpegLib/jdinput.o\
        src/JpegLib/jdmainct.o\
        src/JpegLib/jdmarker.o\
        src/JpegLib/jdmaster.o\
        src/JpegLib/jdmerge.o\
        src/JpegLib/jdphuff.o\
        src/JpegLib/jdpostct.o\
        src/JpegLib/jdsample.o\
        src/JpegLib/jdtrans.o\
        src/JpegLib/jerror.o\
        src/JpegLib/jfdctflt.o\
        src/JpegLib/jfdctfst.o\
        src/JpegLib/jfdctint.o\
        src/JpegLib/jidctflt.o\
        src/JpegLib/jidctfst.o\
        src/JpegLib/jidctint.o\
        src/JpegLib/jidctred.o\
        src/JpegLib/jmemmgr.o\
        src/JpegLib/jmemnobs.o\
        src/JpegLib/jquant1.o\
        src/JpegLib/jquant2.o\
        src/JpegLib/jutils.o\


######################################
#test
DEMO_CFLAGS=
BIN_TARGET  = dc_decompress_test
BIN_OBJS=./demo/dc_decompress_test.o
LD_PATH= ../build_lib
SYS_PATH= $(TARGET_DC_LIB)
DEP_LIBS = -L$(LD_PATH) -L$(SYS_PATH) ./libtmImage.a ../build_lib/libSDL.a -ltmWrtCommon ../build_lib/libSDL.a  -lop_env -lcsc -ldcapi -lpthread -lrt -ljpegpipe -limage_container  
#-lSDL
#####################################



all:$(LIBS_TARGET)
lib:$(LIBS_TARGET)
app: $(BIN_TARGET)

$(LIBS_COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -fPIC $< -o $@

$(LIBS_TARGET):$(LIBS_COBJS)
	$(AR) -rcs $@ $^
#	$(CC)-shared -o $@ $^
	cp $(LIBS_TARGET) ../build_lib
	
$(BIN_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(BIN_TARGET):$(BIN_OBJS)
	$(CC) -o $(BIN_TARGET) $(BIN_OBJS) $(DEP_LIBS) -lm
	$(STRIP) $(BIN_TARGET)
	cp $(BIN_TARGET) /mnt/hgfs/DM365_software/tftpd32.323
	
clean:
	rm -f *.o $(LIBS_TARGET) $(LIBS_COBJS) $(BIN_OBJS) $(BIN_TARGET)

