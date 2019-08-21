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



#include "H264Decoder.h"

int g_dec_mode = 1;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



H264Decoder::H264Decoder()
{
	initialized = 0;
	CMEM_init();
}

H264Decoder::~H264Decoder()
{
	if (initialized == 1)
		unInitilize();
	CMEM_exit();
}

int H264Decoder::setParams(int outputPictureWidth, int outputPictureHeight)
{
    if (initialized == 1)
        return 0;

    return initialize(outputPictureWidth, outputPictureHeight);
}


int H264Decoder::initialize(int outputPictureWidth, int outputPictureHeight)
{
	int decResult = -1;
	unsigned long pipe_line_mode = 0;

	if (initialized == 1)
			return 0;

	memset(&cxOn2DecContext, 0, sizeof(cxOn2DecContext));

	if(g_dec_mode)
		cxOn2DecContext.in_format = CX_MPG4;
	else
		cxOn2DecContext.in_format = CX_H264;
	cxOn2DecContext.avctx = this;
	cxOn2DecContext.codec_id = 0;
	cxOn2DecContext.height = outputPictureHeight;
	cxOn2DecContext.hwaccel = 1;

	cxOn2DecContext.out_format = 0;
	cxOn2DecContext.picture_number = 0;
	cxOn2DecContext.pix_fmt = 0;
	cxOn2DecContext.private_data = NULL;
	cxOn2DecContext.unrestricted_mv = 0;
	cxOn2DecContext.width = outputPictureWidth;
	cxOn2DecContext.workaround_bugs = 0;
	cxOn2DecContext.fps = 30 ; // SRI sh->fps;

	cxOn2DecContext.op_xOffset = 0;
	cxOn2DecContext.op_yOffset = 0;
	cxOn2DecContext.fullscreen = 0;

	cxOn2DecContext.enable_pp = 0;
	cxOn2DecContext.direct_rendering = 0;


	cxOn2DecContext.hw_mem_context = NULL;
	cxOn2DecContext.hw_mem_in_use = 0;

	cxOn2DecContext.nal_length_size = 2;


	/* Initialize decoder */
    cxOn2DecContext.pav_rb16 = NULL;

    //decResult = cx_on2_h264_init(&cxOn2DecContext, pipe_line_mode);
	if(g_dec_mode)
		decResult = cx_on2_mpg4_init(&cxOn2DecContext, pipe_line_mode);
    else
    	decResult = cx_on2_h264_init(&cxOn2DecContext, pipe_line_mode);

	printf("On2 8190 initialize result:%d\n", decResult);

	if (decResult >= 0)
	{
		initialized = 1;
	}
	else
	{
		printf("Initialize VDEC failed\n");
	}

	return decResult;
}



int H264Decoder::decode(const void *h264FrameVirAddr, size_t h264FrameSize,
               const void **yuvFrameVirAddr, uint32_t *yuvFramePhyAddr)
{
    int decResult = 0;
    int yuvDataLen = 0;

    if(!g_dec_mode)
        decResult = cx_on2_h264_decode( &cxOn2DecContext, NULL, &yuvDataLen,
    							(const unsigned char *)h264FrameVirAddr, h264FrameSize);
    else
        decResult = cx_on2_mpg4_decode( &cxOn2DecContext, NULL, &yuvDataLen,
    							(const unsigned char *)h264FrameVirAddr, h264FrameSize);

	*yuvFrameVirAddr = (const void *)cxOn2DecContext.avctx;
	if (*yuvFrameVirAddr)
	{
		unsigned long imageBufferBusAddr = CMEM_getPhys((void *)cxOn2DecContext.avctx);	
		*yuvFramePhyAddr = (uint32_t)imageBufferBusAddr;
	}

	return decResult;
}

void H264Decoder::unInitilize()
{
	if (initialized == 1) {

		/* release decoder */
		if(g_dec_mode)
			cx_on2_mpg4_release(&cxOn2DecContext);
		else
			cx_on2_h264_release(&cxOn2DecContext);
		cxOn2DecContext.avctx = NULL;
		printf("On2 8190 uninit Exit \n");
		initialized = 0;
	}

}
