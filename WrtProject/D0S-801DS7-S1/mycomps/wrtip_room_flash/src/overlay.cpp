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




#include "overlay.h"

extern int g_dec_mode;
#define ENABLE_OVERLAY_FEATURE

/************************************************
 *       Overlay Alpha and Colorkey support     *
 ************************************************/
#define MAKE_565_PIXEL(r,g,b)     		\
(((r & 0xF8) << 8) |          			\
 ((g & 0xFC) << 3) |          			\
 ((b & 0xF8) >> 3))

#define MAKE_8888_PIXEL(t,r,g,b)     	\
(((t & 0xFF) << 24) |          			\
 ((r & 0xFF) << 16) |          			\
 ((g & 0xFF) << 8)  |          			\
 ((b & 0xFF) << 0))

#define RED_565_PIXEL                   MAKE_565_PIXEL(255,0,0)
#define GREEN_565_PIXEL                 MAKE_565_PIXEL(0,255,0)
#define BLUE_565_PIXEL                  MAKE_565_PIXEL(0,0,255)
#define WHITE_565_PIXEL                 MAKE_565_PIXEL(255,255,255)
#define BLACK_565_PIXEL                 MAKE_565_PIXEL(0,0,0)
#define CYAN_565_PIXEL                  MAKE_565_PIXEL(0,255,255)
#define MAGENTA_565_PIXEL               MAKE_565_PIXEL(255,0,255)
#define YELLOW_565_PIXEL                MAKE_565_PIXEL(255,255,0)

#define RED_8888_PIXEL       			MAKE_8888_PIXEL(0xFF,255,0,0)
#define GREEN_8888_PIXEL                MAKE_8888_PIXEL(0xFF,0,255,0)
#define BLUE_8888_PIXEL                 MAKE_8888_PIXEL(0xFF,0,0,255)
#define WHITE_8888_PIXEL                MAKE_8888_PIXEL(0xFF,255,255,255)
#define BLACK_8888_PIXEL                MAKE_8888_PIXEL(0xFF,0,0,0)
#define CYAN_8888_PIXEL                 MAKE_8888_PIXEL(0xFF,0,255,255)
#define MAGENTA_8888_PIXEL              MAKE_8888_PIXEL(0xFF,255,0,255)
#define YELLOW_8888_PIXEL               MAKE_8888_PIXEL(0xFF,255,255,0)

// Color enums
#define DISABLE_COLOR_KEY	0xFFFF0000
#define RED_PIXEL       	RED_565_PIXEL
#define GREEN_PIXEL     	GREEN_565_PIXEL
#define BLUE_PIXEL      	BLUE_565_PIXEL
#define WHITE_PIXEL     	WHITE_565_PIXEL
#define BLACK_PIXEL     	BLACK_565_PIXEL
#define CYAN_PIXEL      	CYAN_565_PIXEL
#define MAGENTA_PIXEL   	MAGENTA_565_PIXEL
#define YELLOW_PIXEL    	YELLOW_565_PIXEL




/*
 * AlphaFactor range is 0x00 to 0xFF, with 0x00 meaning totally transparent foreground
 * and oxFF meaning totally opaque foreground
 *
 * ColorKey has a range of 0 through 8
 */
static int foregroundAlphaFactor = 0xFF;
static int currentColorKey = BLUE_PIXEL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



Overlay::Overlay()
{
	fb_dev_fd = -1;
	outputAddr = 0;
	initialized = 0;
	displayImgeFirst = 0;
	currentDecodeBuffer = 1;

	frameFlag = 0;
	handle = NULL;
	tempBufferVirtualAddr = NULL;
	displayFrameCount = 0;
	localBufferVirtualAddr = NULL;

	displayFormat = FMT_PIX_FMT_RGB16_5_6_5;
	dataFormat = FMT_YCBCR_4_2_0_SEMIPLANAR;
	per_pixel_bytes = 2;

	memoryPara.type = CMEM_HEAP;
	memoryPara.flags = CMEM_NONCACHED;
	memoryPara.alignment = 32;
	CMEM_init();
	cx_ga_init();
}



Overlay::~Overlay()
{
	if (initialized == 1)
		unInitilize();

	CMEM_exit();
	cx_ga_release();
}

int Overlay::setParams(int x, int y, int pictureWidth, int pictureHeight, int displayWidth,
						int displayHeight, int inputFormat, int outputFormat, int flipFlags)
{
	//Only support to initialixe the device for one time.
	if (initialized == 1)
			return -1;

	if ((fb_dev_fd = open("/dev/fb0", O_RDWR)) == -1) {
			printf("Can't open /dev/fb0\n");
			return -1;
	}
	if (ioctl(fb_dev_fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
	{
		printf("Can't ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
		return -1;
	}

	close(fb_dev_fd);
	fb_dev_fd = -1;
	lcdWidth = vinfo.xres;
	lcdHeight = vinfo.yres;


	//To verify the displaying width/height
	if ((displayWidth + x) > lcdWidth)
	{
		printf("Error: Displaying width + X offset > LCD width\n");
		return -1;
	}

	if ((displayHeight + y) > lcdHeight)
	{
		printf("Error: Displaying width + Y offset > LCD height\n");
		return -1;
	}

	//To verify the input/output format;
	switch (inputFormat)
	{
		case FMT_YCBCR_4_2_2_INTERLEAVED:
		case FMT_YCRYCB_4_2_2_INTERLEAVED:
		case FMT_CBYCRY_4_2_2_INTERLEAVED:
		case FMT_CRYCBY_4_2_2_INTERLEAVED:
		case FMT_YCBCR_4_2_2_SEMIPLANAR:
		case FMT_YCBCR_4_2_0_SEMIPLANAR:
			break;
		default:
			printf("Unknown inputting format\n");
			return -1;
	}

	//To verify the input/output format;
	switch (outputFormat)
	{
		case FMT_PIX_FMT_RGB16_5_6_5:
		case FMT_PIX_FMT_RGB32:
			break;
		default:
			printf("Unknown outputting format\n");
			return -1;
	}

	displayFormat = outputFormat;
	dataFormat = inputFormat;

	videoDecoderPara tempArg;
	tempArg.screenXpos = x;
	tempArg.screenYpos = y;
	tempArg.pictureWidth = pictureWidth;
	tempArg.pictureHeight = pictureHeight;
	tempArg.width = displayWidth;
	tempArg.height = displayHeight;

	if (initialize(&tempArg) >= 0)
	{
		initialized = 1;
		return 0;
	}
	else
	{
		return 1;
	}
}


int Overlay::initialize(videoDecoderPara *arg)
{
	int decResult = 0;


	if (initialized == 1)
			return 0;


	if (decResult >= 0)
	{
		initialized = 1;
		displayImgeFirst = 0;

		memcpy(&videoDecoderArg, arg, sizeof(videoDecoderPara));

	}
	else
	{
		printf("Initialize VDEC failed\n");
	}

	return decResult;
}


int Overlay::postFrameBuffer(const void *yuvFrameVirAddr, uint32_t yuvFramePhyAddr, size_t len)
{

	int decResult = 0;
	int currentDecodeAddr = 0;
	int currentDisplayAddr = 0;


	if (initialized == 0)
	{
		printf("Device had not beent initialized\n");
		return -1;
	}

	if (displayImgeFirst == 0)
	{
		if (initializeDisplay(videoDecoderArg.screenXpos, videoDecoderArg.screenYpos,videoDecoderArg.width, videoDecoderArg.height,
													videoDecoderArg.pictureWidth, videoDecoderArg.pictureHeight) >= 0 )
		{
			displayImgeFirst = 1;
		}
	}

	if (yuvFrameVirAddr == NULL)
		return -1;

	if (decResult >= 0)
	{

		if (displayFormat == FMT_PIX_FMT_RGB16_5_6_5)
		{
			per_pixel_bytes = 2;//For the 32BPP LCD, it should be set to 4.
		}
		else if (displayFormat == FMT_PIX_FMT_RGB32)
		{
			per_pixel_bytes = 4;
		}
		else
		{
			per_pixel_bytes = 2;
		}


		if (frameFlag++ < 5)//Work around for one bug
		{
			scaleOneFrame(handle,  yuvFramePhyAddr,  tempBufferBusAddr);
        	return decResult;
		}

		frameFlag = 5;

		displayFrameCount++;

#ifdef ENABLE_OVERLAY_FEATURE

		if (currentDecodeBuffer == 0)
		{
			currentDecodeBuffer = 1;
			scaleOneFrame(handle,  yuvFramePhyAddr,  localBufferBusAddr);
			currentDecodeAddr = localBufferBusAddr;
			
			currentDisplayAddr = outputAddr + lcdWidth * lcdHeight * per_pixel_bytes * 2;

			drawImage(outputAddr, currentDisplayAddr,
								lcdWidth * per_pixel_bytes, lcdWidth * per_pixel_bytes,
								lcdWidth, lcdHeight, 0, 0, 0, 0,
								per_pixel_bytes, 0xFF, DISABLE_COLOR_KEY);

			drawImage(currentDecodeAddr, currentDisplayAddr,
								videoDecoderArg.width * per_pixel_bytes, lcdWidth * per_pixel_bytes,
								videoDecoderArg.width, videoDecoderArg.height,
								0, 0, videoDecoderArg.screenXpos, videoDecoderArg.screenYpos,
								per_pixel_bytes,foregroundAlphaFactor, currentColorKey);
			

			vinfo.yoffset = lcdHeight * 2;
			
			ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);

		} else if (currentDecodeBuffer == 1)
		{
			currentDecodeBuffer = 0;
			scaleOneFrame(handle,  yuvFramePhyAddr,  localBufferBusAddr);
			currentDecodeAddr = localBufferBusAddr;
			
			currentDisplayAddr = outputAddr + lcdWidth * lcdHeight * per_pixel_bytes;

			drawImage(outputAddr, currentDisplayAddr,
								lcdWidth * per_pixel_bytes, lcdWidth * per_pixel_bytes,
								lcdWidth, lcdHeight, 0, 0, 0, 0,
								per_pixel_bytes, 0xFF, DISABLE_COLOR_KEY);

			drawImage(currentDecodeAddr, currentDisplayAddr,
								videoDecoderArg.width * per_pixel_bytes, lcdWidth * per_pixel_bytes,
								videoDecoderArg.width, videoDecoderArg.height,
								0, 0, videoDecoderArg.screenXpos, videoDecoderArg.screenYpos,
								per_pixel_bytes,foregroundAlphaFactor, currentColorKey);


			vinfo.yoffset = lcdHeight;
			
			ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);
		}

#else

		if (currentDecodeBuffer == 0)
		{
			vinfo.yoffset = 0;
			if (fb_dev_fd >= 0)
			{
				ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);
			}
			scaleOneFrame(handle,  yuvFramePhyAddr,  outputAddr + lcdWidth * lcdHeight * per_pixel_bytes);
			currentDecodeBuffer = 1;
		}
		else if (currentDecodeBuffer == 1)
		{
			vinfo.yoffset = lcdHeight;
			if (fb_dev_fd >= 0)
			{
				ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);
			}
			scaleOneFrame(handle,  yuvFramePhyAddr,  outputAddr + lcdWidth * lcdHeight * per_pixel_bytes * 2);
			currentDecodeBuffer = 2;
		}
		else
		{
			vinfo.yoffset = lcdHeight * 2;
			if (fb_dev_fd >= 0)
			{
				ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);
			}
			scaleOneFrame(handle,  yuvFramePhyAddr,  outputAddr);
			currentDecodeBuffer = 0;
		}
#endif


	}


	return decResult;

}


void Overlay::unInitilize()
{
	if (initialized == 1) {

		stopDisplay();

		if (handle)
		{
			uinitScale(handle);
			handle = NULL;
		}


		if (fb_dev_fd >= 0)
		{
			close(fb_dev_fd);
			fb_dev_fd = -1;
		}

		if (tempBufferVirtualAddr)
		{
			CMEM_free(tempBufferVirtualAddr, &memoryPara);
			tempBufferVirtualAddr = NULL;
		}

		if (localBufferVirtualAddr)
		{
			CMEM_free(localBufferVirtualAddr, &memoryPara);
			localBufferVirtualAddr = NULL;
		}

		initialized = 0;
	}

}
extern int g_videobright;
extern int g_videocontrast;
extern int g_videosaturation;

int Overlay::initializeDisplay(int screenXpos, int screenYpos, int width, int height, int pictureWidth, int pictureHeight)
{
	int result = -1;
	int tmpcontrast = 0;
	//To open the frame buffer device.
	if (fb_dev_fd >= 0)
			return 0;

	if ((fb_dev_fd = open("/dev/fb0", O_RDWR)) == -1) {
			printf("Can't open /dev/fb0\n");
			return -1;
	}

	//To get the parameters of frame buffer device.
	ioctl(fb_dev_fd, FBIOGET_VSCREENINFO, &vinfo);
	ioctl(fb_dev_fd,FBIOGET_FSCREENINFO,&fb_fix_info);
	outputAddr = fb_fix_info.smem_start;




	if (displayFormat == FMT_PIX_FMT_RGB16_5_6_5)
	{
		per_pixel_bytes = 2;//For the 32BPP LCD, it should be set to 4.
	}
	else if (displayFormat == FMT_PIX_FMT_RGB32)
	{
		per_pixel_bytes = 4;
	}
	else
	{
		per_pixel_bytes = 2;
	}

	tempBufferVirtualAddr = CMEM_alloc(lcdWidth * lcdHeight * per_pixel_bytes, &memoryPara);
	if (tempBufferVirtualAddr == NULL)
	{
		printf("Allocate CMEM failed\n");
		close(fb_dev_fd);
		fb_dev_fd = -1;
		return -1;
	}
	tempBufferBusAddr = CMEM_getPhys(tempBufferVirtualAddr);
	tempBufferSize = lcdWidth * lcdHeight * per_pixel_bytes;
	memset(tempBufferVirtualAddr, 0x0, tempBufferSize);



	//To initialize the YUV scaler.
	scaleParams.ip_width  = pictureWidth;
	scaleParams.ip_height = pictureHeight / 16 * 16;
	scaleParams.ip_pixFormat = dataFormat;
	scaleParams.op_width  = width;
	scaleParams.op_height = height;
	scaleParams.op_pixFormat = displayFormat;


	scaleParams.framebuffer_enable  = 1;
#ifdef ENABLE_OVERLAY_FEATURE
	scaleParams.op_xPos   = 0;
	scaleParams.op_yPos   = 0;
	scaleParams.framebuffer_width   = width;
	scaleParams.framebuffer_height  = height;
#else
	scaleParams.op_xPos   = screenXpos;
	scaleParams.op_yPos   = screenYpos;
	scaleParams.framebuffer_width   = lcdWidth;
	scaleParams.framebuffer_height  = lcdHeight;
#endif

	result = initScale(&handle, scaleParams);
	tmpcontrast = g_videocontrast-10;
	if(tmpcontrast >= 50)
		tmpcontrast = 50;
	if(g_dec_mode)
	{
		setBrightness(handle,g_videobright-10);//ori 0
		setContrast(handle,tmpcontrast);
		setSaturation(handle,g_videosaturation-10);
		printf("mp4 %d_%d_%d\n",g_videobright,g_videocontrast,g_videosaturation);
	}else
	{
		setBrightness(handle,g_videobright-10);
		setContrast(handle,tmpcontrast);
		setSaturation(handle,g_videosaturation-20);
		printf("264 %d_%d_%d\n",g_videobright,g_videocontrast,g_videosaturation);
	}
	if (result < 0 || handle == NULL)
	{
		printf("Initialize YUV Scaler failed\n");
		close(fb_dev_fd);
		fb_dev_fd = -1;
	}


#ifdef ENABLE_OVERLAY_FEATURE
	localBufferVirtualAddr = (void *)CMEM_alloc(width * height * per_pixel_bytes, &memoryPara);
	localBufferBusAddr = CMEM_getPhys(localBufferVirtualAddr);
	localBufferSize = width * height * per_pixel_bytes;
	memset(localBufferVirtualAddr, 0, localBufferSize);


	vinfo.yoffset = lcdHeight;
    ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);


	drawImage(outputAddr, outputAddr + lcdWidth * lcdHeight * per_pixel_bytes,
							lcdWidth * per_pixel_bytes, lcdWidth * per_pixel_bytes,
							lcdWidth, lcdHeight, 0, 0, 0, 0,
							per_pixel_bytes,0xFF, DISABLE_COLOR_KEY);

	drawImage(outputAddr, outputAddr + lcdWidth * lcdHeight * per_pixel_bytes * 2,
							lcdWidth * per_pixel_bytes, lcdWidth * per_pixel_bytes,
							lcdWidth, lcdHeight, 0, 0, 0, 0,
							per_pixel_bytes,0xFF, DISABLE_COLOR_KEY);
#else
	vinfo.yoffset = 0;
    ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);
    void *frame_buffer = mmap(0, lcdWidth * lcdHeight * per_pixel_bytes * 3, PROT_READ | PROT_WRITE,
                            		MAP_SHARED, fb_dev_fd, 0);
    if (frame_buffer)
    {
		memcpy(frame_buffer + lcdWidth * lcdHeight * per_pixel_bytes, frame_buffer, lcdWidth * lcdHeight * per_pixel_bytes);
		memcpy(frame_buffer + lcdWidth * lcdHeight * per_pixel_bytes * 2, frame_buffer, lcdWidth * lcdHeight * per_pixel_bytes);
		munmap(frame_buffer, lcdWidth * lcdHeight * per_pixel_bytes * 3);
	}

#endif
	return result;
}

//modify by wyx 2015-1-27 增加退出判断是否为1层，
//如果不是第1层就再次切换至第一层然后退出
void Overlay::stopDisplay()
{
	vinfo.yoffset = 0;
	frameFlag = 0;

fbloop:
	if (fb_dev_fd >= 0)
	{
	    ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &vinfo);
	}

	usleep(10*1000);

	if (ioctl(fb_dev_fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
	{
		printf("Can't ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
		return;
	}

	if(vinfo.yoffset != 0)
	{
		usleep(100*1000);
		goto fbloop;
	}
}

void Overlay::drawImage(UINT32 src, UINT32 dst, UINT32 srcStride,UINT32 dstStride,
                    UINT32 width,UINT32 height, int srcXOffset, int srcYOffset,
                    int dstXOffset,int dstYOffset, int bytesPerPixel,
                    UINT32 foregroundAlphaFactor, UINT32 colorKey)
{
    DC_GA_COMPLETION_DESC cd = {1,NULL,0};
    DC_GFX_POINT origin = {srcXOffset,srcYOffset};
	DC_GFX_POINT dst_ul= {dstXOffset,dstYOffset};

    dc_ga_set_source(src,srcStride);
    dc_ga_set_destination(dst,dstStride);
    dc_ga_set_cmd_rop(DC_GA_ROP_COPY);

	if (colorKey == DISABLE_COLOR_KEY)
	{
		dc_ga_set_buffer_control(DC_GA_KEY_CTRL_NONE,
                             (bytesPerPixel == 2) ? DC_GA_PIXEL_FORMAT_565 : DC_GA_PIXEL_FORMAT_8888,
                             0,0,0,0);
	}
	else
	{
		dc_ga_set_color_key(colorKey);

		dc_ga_set_buffer_control(DC_GA_KEY_CTRL_DST,
                             (bytesPerPixel == 2) ? DC_GA_PIXEL_FORMAT_565 : DC_GA_PIXEL_FORMAT_8888,
                             0,0,0,0);
	}

	dc_ga_set_cmd_style(DC_GA_CMD_STYLE_NONE);

	dc_ga_set_alpha_factor(foregroundAlphaFactor,0x00);
	dc_ga_set_alpha_control(DC_GA_ALPHA_SRC_SRC_ALPHA,DC_GA_ALPHA_DST_ONE_MINUS_SRC_ALPHA,1);

    dc_ga_bitblt(&origin,&dst_ul,width,height,DC_GA_BITBLT_DIR_LRTB,&cd);
}

