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


#ifndef __H264_DECODER_H__
#define __H264_DECODER_H__


#include "common.h"
extern "C" {
	#include "cxon2_api.h"
	#include "cmem.h"
}


class H264Decoder  {

  public:

	virtual ~H264Decoder();
	H264Decoder();

    /**
     *
     * @param outputPictureWidth  the output h264 video's width
     * @param outputPictureHeight the output h264 video's height
     */
    int setParams(int outputPictureWidth, int outputPictureHeight);

    /**
     *
     * @param[in] h264FrameVirAddr the input h264 frame's virtual address
     * @param[in] h264FrameSize     the input h264 frame's buffer size
     * @param[out] yuvFrameVirAddr  the output yuv frame's virtual address
     * @param[out] yuvFramePhyAddr  the output yuv frame's physical address
     *
     * @return 0 on success
     */
    int decode(const void *h264FrameVirAddr, size_t h264FrameSize,
               const void **yuvFrameVirAddr, uint32_t *yuvFramePhyAddr);


private:
	void unInitilize();
	int initialize(int outputPictureWidth, int outputPictureHeight);


	CnxtOn2DecContext cxOn2DecContext;


	int initialized;
};


#endif
