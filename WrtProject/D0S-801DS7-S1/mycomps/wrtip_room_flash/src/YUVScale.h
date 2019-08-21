/*+++ *******************************************************************\
*
*  Copyright and Disclaimer:
*
*     ---------------------------------------------------------------
*     This software is provided "AS IS" without warranty of any kind,
*     either expressed or implied, including but not limited to the
*     implied warranties of noninfringement, merchantability and/or
*     fitness for a particular purpose.
*     ---------------------------------------------------------------
*
*     Copyright ©2012 Conexant Systems, Inc.
*     All rights reserved.
*
\******************************************************************* ---*/

#ifndef __YUVSCALE_H__
#define __YUVSCALE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>
#include <sys/socket.h>
#include <netinet/in.h>
 #include <arpa/inet.h>



 #include <ctype.h>
 #include <limits.h>
 #include <time.h>
 #include <locale.h>
 #include <sys/unistd.h>
 #include <alsa/asoundlib.h>

#define FMT_YCBCR_4_2_2_INTERLEAVED              0x010001U
#define FMT_YCRYCB_4_2_2_INTERLEAVED             0x010005U
#define FMT_CBYCRY_4_2_2_INTERLEAVED             0x010006U
#define FMT_CRYCBY_4_2_2_INTERLEAVED             0x010007U
#define FMT_YCBCR_4_2_2_SEMIPLANAR               		0x010002U
#define FMT_YCBCR_4_2_0_SEMIPLANAR               		0x020001U


#define FMT_PIX_FMT_RGB16_5_6_5                          							0x040002U
#define FMT_PIX_FMT_RGB32                                									0x041001U
#define FMT_PIX_FMT_CBYCRY_4_2_2_INTERLEAVED             0x010006U

typedef struct _PPStandalone_Params
{
		unsigned int ip_width;
		unsigned int ip_height;
		unsigned int ip_pixFormat;
		unsigned int op_width;
		unsigned int op_height;
		unsigned int op_pixFormat;
		int op_xPos;
		int op_yPos;
		unsigned int framebuffer_enable;
		unsigned int framebuffer_width;
		unsigned int framebuffer_height;
}YUVScalerParams;

int initScale(void **handle,YUVScalerParams scalerParams);
int scaleOneFrame(void *handle,int input_address, int output_address);
void uinitScale(void *handle);
int setContrast(void *handle, int contrast);
int setBrightness(void *handle, int brightness);
int setSaturation(void *handle, int saturation);
int getColorSetting(void *handle, int *contrast, int *brightness, int *saturation);

void initScalerLibrary();
void unInitScalerLibrary();


#endif
