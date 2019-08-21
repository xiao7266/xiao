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
*     Copyright ?012 Conexant Systems, Inc.
*     All rights reserved.
*
\******************************************************************* ---*/


#ifndef __OVERLAY_H__
#define __OVERLAY_H__

#include <linux/fb.h>

#include "common.h"
#include "YUVScale.h"
#include "cmem.h"


typedef int INT32;
typedef unsigned int UINT32;
typedef short INT16;
typedef unsigned short UINT16;
typedef signed char INT8;
typedef unsigned char UINT8;
typedef unsigned char boolean;
typedef unsigned char BOOLEAN;



extern "C" {
	#include "dc_gfx_api.h"
	extern int cx_ga_release();
	extern int cx_ga_init();
}


typedef struct
{
	int pictureWidth;
	int pictureHeight;
	int screenXpos;
	int screenYpos;
	int width;
	int height;
}videoDecoderPara;

class Overlay {

  public:

	virtual ~Overlay();
	Overlay();

    /**
     *
     * @param x         		the display position coord x
     * @param y         		the display position coord y
     * @param pictureWidth     	the width of inputting picture.
     * @param pictureHeight    	the height of inputting picture.
     * @param displayWidth     	display width
     * @param displayHeight    	display height
     * @param inputFormat		color format, like YUYV/YVYU/RGB etc
     * @param outputFormat    	color format, like YUYV/YVYU/RGB etc
     * @param flipFlags			un-used.
     */
    int setParams(int x, int y, int pictureWidth, int pictureHeight, int displayWidth, int displayHeight, int inputFormat, int outputFormat, int flipFlags);

    /**
     *
     * @param yuvFrameVirAddr the virtual address of frame
     * @param yuvFramePhyAddr the physical address of frame
     * @param len frame buffer length
     *
     * @return 0 on success
     */
    int postFrameBuffer(const void *yuvFrameVirAddr, uint32_t yuvFramePhyAddr, size_t len);

	//When the Overlay exits, we have to switch to the first frame buffer. Because GUI only supports the first frame buffer.
	void stopDisplay();

private:
	void unInitilize();
	int initialize(videoDecoderPara *arg);



	void drawImage(UINT32 src, UINT32 dst, UINT32 srcStride,UINT32 dstStride,
                    UINT32 width,UINT32 height, int srcXOffset, int srcYOffset,
                    int dstXOffset,int dstYOffset, int bytesPerPixel,
                    UINT32 foregroundAlphaFactor, UINT32 colorKey);

	//The frame buffer parameters.
	int fb_dev_fd;
	struct fb_fix_screeninfo fb_fix_info;
	struct fb_var_screeninfo vinfo;
	int per_pixel_bytes;



	//Internal varibles.
	int lcdHeight;
	int lcdWidth;
	int outputAddr;
	int frameFlag;
	int displayFrameCount;
	int currentDecodeBuffer;

	//The input/output format such as YUV420 SP, RGB565/RGB32.
	int dataFormat;
	int displayFormat;


	//Initialize displaying device.
	int initializeDisplay(int screenXpos, int screenYpos, int width, int height, int pictureWidth, int pictureHeight);


	videoDecoderPara videoDecoderArg;
	int displayImgeFirst, initialized;
	void *handle;
	YUVScalerParams scaleParams;

	unsigned long 	localBufferBusAddr;
	unsigned long 	localBufferSize;
	void 			*localBufferVirtualAddr;


	unsigned long 	tempBufferBusAddr;
	unsigned long 	tempBufferSize;
	void 			*tempBufferVirtualAddr;
	CMEM_AllocParams memoryPara;
};






#endif
