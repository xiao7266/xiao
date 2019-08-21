


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#include "videoDecoder.h"

int g_dec_mode = 1;//1 --MP4 0---H264

VideoDecoder* VideoDecoder::objectInstance = NULL;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


/* function to get AV_RB16 value for VDEC library */
static int Get_avrb16_value(unsigned char *src)
{
	return 0;
}


static void set_bpp(struct fb_var_screeninfo *p, int bpp)
{
    p->bits_per_pixel = FFALIGN(bpp, 2);
    p->red.msb_right  = p->green.msb_right = p->blue.msb_right = p->transp.msb_right = 0;
    p->transp.offset  = p->transp.length = 0;
    p->blue.offset    = 0;
    switch (bpp) {
    case 32:
        p->transp.offset = 24;
        p->transp.length = 8;
    case 24:
        p->red.offset   = 16;
        p->red.length   = 8;
        p->green.offset = 8;
        p->green.length = 8;
        p->blue.length  = 8;
        break;
    case 16:
        p->red.offset   = 11;
        p->green.length = 6;
        p->red.length   = 5;
        p->green.offset = 5;
        p->blue.length  = 5;
        break;
    case 15:
        p->red.offset   = 10;
        p->green.length = 5;
        p->red.length   = 5;
        p->green.offset = 5;
        p->blue.length  = 5;
        break;
    case 12:
        p->red.offset   = 8;
        p->green.length = 4;
        p->red.length   = 4;
        p->green.offset = 4;
        p->blue.length  = 4;
        break;
    }
}

VideoDecoder::VideoDecoder()
{
	fb_page = 0;
	cxfbinfo_init = 0;
	fb_offset = 1;//默认是1  我们使用的是0
	fb_dev_fd = -1;
	vo_doublebuffering = 1;
	no_more_flip = 0;
	initialized = 0;
	displayImgeFirst = 0;

	pCxfb_phy_ptr = 0, pCxfb_virt_ptr = 0;

	frame_buffer = NULL;
	center = NULL;

}

VideoDecoder::~VideoDecoder()
{
	if (initialized == 1)
		unInitilize();
}

int VideoDecoder::initialize(videoDecoderPara *arg)
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
	cxOn2DecContext.height = arg->pictureHeight;
	cxOn2DecContext.hwaccel = 1;

	cxOn2DecContext.out_format = 0;
	cxOn2DecContext.picture_number = 0;
	cxOn2DecContext.pix_fmt = 0;
	cxOn2DecContext.private_data = NULL;
	cxOn2DecContext.unrestricted_mv = 0;
	cxOn2DecContext.width = arg->pictureWidth;
	cxOn2DecContext.workaround_bugs = 0;
	cxOn2DecContext.fps = 30 ; // SRI sh->fps;

	cxOn2DecContext.op_xOffset = 0;
	cxOn2DecContext.op_yOffset = 0;
	cxOn2DecContext.fullscreen = 0;

	if ((fb_dev_fd = open("/dev/fb0", O_RDWR)) == -1) {
			printf("Can't open /dev/fb0\n");
			return decResult;
	}
	
	if (ioctl(fb_dev_fd, FBIOGET_VSCREENINFO, &fb_vinfo) < 0)
	{
		printf("Can't ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
		return decResult;
	}
	
	close(fb_dev_fd);
	fb_dev_fd = -1;
	lcdWidth = fb_vinfo.xres;
	lcdHeight = fb_vinfo.yres;


	cxOn2DecContext.op_height = lcdHeight;
	cxOn2DecContext.op_width  = lcdWidth;

	cxOn2DecContext.hw_mem_context = NULL;
	cxOn2DecContext.hw_mem_in_use = 0;

	cxOn2DecContext.nal_length_size = 2;

	/* Initialize decoder */
    	cxOn2DecContext.pav_rb16 = Get_avrb16_value;
	if(g_dec_mode)
		decResult = cx_on2_mpg4_init(&cxOn2DecContext, pipe_line_mode);
	else
    	decResult = cx_on2_h264_init(&cxOn2DecContext, pipe_line_mode);
   	 

	printf("On2 8190 Init Completed %d\n", decResult);



	if (decResult >= 0)
	{
		initialized = 1;
		displayImgeFirst = 0;

		memcpy(&videoDecoderArg, arg, sizeof(videoDecoderPara));
	}

	return decResult;
}


int VideoDecoder::processFrame(char *buffer, int len)
{

    int decResult=0;


	//printf("cx_on2_start_pp decode buffer and len is :%p %d\n",buffer,len);
	//printf("1111111111111\n");
	/* call the actual decode function */
	if(g_dec_mode)
		decResult = cx_on2_mpg4_decode( &cxOn2DecContext,
			NULL, 0, (const unsigned char *)buffer, len);
	else
		decResult = cx_on2_h264_decode( &cxOn2DecContext,
			NULL, 0, (const unsigned char *)buffer, len);

    if (displayImgeFirst == 0)
    {
		 if (initializeDisplay(videoDecoderArg.screen_xpos, videoDecoderArg.screen_ypos,
		 											videoDecoderArg.width, videoDecoderArg.height,
		 											videoDecoderArg.pictureWidth, videoDecoderArg.pictureHeight) >= 0 )
						displayImgeFirst = 1;
	}


	if (displayImgeFirst == 1)
	{
		
		cx_on2_start_pp(&cxOn2DecContext);
		//printf("222222222222\n");
	}
	//printf("exit processFrame \n");
    return displayImgeFirst==1 ? len : -1;
}


void VideoDecoder::unInitilize()
{
	if (initialized == 1) {
/* release decoder */
		printf("being cx_on2_mpg4_release************\n");
		if(g_dec_mode)
			cx_on2_mpg4_release(&cxOn2DecContext);
		else
			cx_on2_h264_release(&cxOn2DecContext);
		cxOn2DecContext.avctx = NULL;
		printf("On2 8190 uninit Exit \n");
		
		if (cxfbinfo_init == 1)
		{

			no_more_flip = 1;
			fb_vinfo.yoffset = 0;
			if( !fb_page) { //Flip back to fir frame buffer
				cx_force_flip_page();
			}

			ioctl(fb_dev_fd, FBIOGET_VSCREENINFO, &fb_vinfo);
			fb_orig_vinfo.xoffset = fb_vinfo.xoffset;
			fb_orig_vinfo.yoffset = fb_vinfo.yoffset;
			ioctl(fb_dev_fd, FBIOPUT_VSCREENINFO, (void *)&fb_orig_vinfo);
			if (frame_buffer)
					munmap(frame_buffer, fb_size);
			frame_buffer = NULL;

			close(fb_dev_fd);
			fb_dev_fd = -1;
			cxfbinfo_init = 0;
		}

		

		initialized = 0;

	}

}

int VideoDecoder::initializeDisplay(int screen_xpos, int screen_ypos, int width, int height, int pictureWidth, int pictureHeight)
{
	if (fb_dev_fd >= 0)
			return 0;

	if ((fb_dev_fd = open("/dev/fb0", O_RDWR)) == -1) {
		printf("Can't open /dev/fb0\n");
		return -1;
	}

	if (ioctl(fb_dev_fd, FBIOGET_VSCREENINFO, &fb_vinfo)) {
	        printf("Can't get VSCREENINFO: %s\n", strerror(errno));
	        close(fb_dev_fd);
	        return -1;
    }

    fb_orig_vinfo = fb_vinfo;

    int result = config(screen_xpos, screen_ypos, width, height, pictureWidth, pictureHeight);

    if (result < 0)
    {
		close(fb_dev_fd);
	}

	return result;
}


int VideoDecoder::config(int xOffset, int yOffset , int displayWidth, int displayHeight, int picWidth, int picHeight)
{

    int in_width     = picWidth;
    int in_height    = picHeight;
    int out_width  = picWidth;
    int out_height = picHeight;

	fb_bpp = fb_vinfo.bits_per_pixel;
	fb_bpp_we_want = fb_bpp;
    set_bpp(&fb_vinfo, fb_bpp);
    fb_page = 0;

    if (vo_doublebuffering) {
        fb_vinfo.yoffset = 0;
        fb_page = 1; // start writing into the page we don't display
    }


    if (ioctl(fb_dev_fd, FBIOPUT_VSCREENINFO, &fb_vinfo))
        // Intel drivers fail if we request a transparency channel
        fb_vinfo.transp.length = fb_vinfo.transp.offset = 0;

    if (ioctl(fb_dev_fd, FBIOPUT_VSCREENINFO, &fb_vinfo)) {
        printf("Can't put VSCREENINFO: %s\n", strerror(errno));
        return -1;
    }

    int fb_pixel_size = fb_vinfo.bits_per_pixel / 8;
    fb_bpp = fb_vinfo.bits_per_pixel;
    if (fb_bpp == 16)
        fb_bpp = fb_vinfo.red.length  + fb_vinfo.green.length + fb_vinfo.blue.length;
    if (fb_bpp_we_want != fb_bpp)
        printf("requested %d bpp, got %d bpp!!!\n",
               fb_bpp_we_want, fb_bpp);


    fb_yres = fb_vinfo.yres;

    if (ioctl(fb_dev_fd, FBIOGET_FSCREENINFO, &fb_finfo)) {
        printf("Can't get FSCREENINFO: %s\n", strerror(errno));
        return -1;
    }

    if (fb_finfo.type != FB_TYPE_PACKED_PIXELS) {
        printf("type %d not supported\n", fb_finfo.type);
        return -1;
    }

    fb_line_len = fb_finfo.line_length;
    fb_size = fb_finfo.smem_len;
    if (vo_doublebuffering && fb_size < 2 * fb_yres * fb_line_len)
    {
        printf("framebuffer too small for double-buffering, disabling\n");
        vo_doublebuffering = 0;
        fb_page = 0;
    }


    {
        frame_buffer = (uint8_t *)mmap(0, fb_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fb_dev_fd, 0);
        if (frame_buffer == (uint8_t *) -1) {
            printf("Can't mmap /dev/fb0, %s\n", strerror(errno));
            return -1;
        }

        center = frame_buffer +
                 ( (out_width  - in_width)  / 2 ) * fb_pixel_size +
                 ( (out_height - in_height) / 2 ) * fb_line_len +
                 xOffset * fb_pixel_size + yOffset * fb_line_len +
                 fb_page * fb_yres * fb_line_len;


    }


	no_more_flip = 0;

	if (vo_doublebuffering ) {
		fb_page = 1;


		if (fb_offset)
		{
			pCxfb_virt_ptr = (uint32_t)frame_buffer;
			pCxfb_phy_ptr = fb_finfo.smem_start;

			pCxfb_phy_ptr += (fb_offset) * fb_yres * fb_line_len;
			pCxfb_virt_ptr += (fb_offset) * fb_yres * fb_line_len;

			/* copy the initial frame to second frame */
			memcpy( (void *) pCxfb_virt_ptr, (void *)frame_buffer, (fb_yres * fb_line_len) );
		}



		pCxfb_virt_ptr = (uint32_t)frame_buffer;
		pCxfb_phy_ptr = fb_finfo.smem_start;

		pCxfb_phy_ptr += (fb_page + fb_offset) * fb_yres * fb_line_len;
		pCxfb_virt_ptr += (fb_page + fb_offset) * fb_yres * fb_line_len;


		/* copy the initial frame to second frame */
		memcpy( (void *) pCxfb_virt_ptr, (void *)frame_buffer, (fb_yres * fb_line_len) );
	}


	cxon2fbinfo_local.bits_per_pixel = fb_pixel_size;
	cxon2fbinfo_local.fullscreen = 0;
	cxon2fbinfo_local.line_length = fb_line_len;
	cxon2fbinfo_local.op_width = displayWidth;
	cxon2fbinfo_local.op_height = displayHeight;
	cxon2fbinfo_local.op_xOffset = xOffset;
	cxon2fbinfo_local.op_yOffset = yOffset;
	cxon2fbinfo_local.fbDevVirtualAddr = pCxfb_virt_ptr ;
	cxon2fbinfo_local.fbDevPhysicalAddr = pCxfb_phy_ptr ;
	cxon2fbinfo_local.double_buffering = vo_doublebuffering;
	cxon2fbinfo_local.fb_size = fb_line_len * fb_yres;
	cxon2fbinfo_local.xres = fb_vinfo.xres;
	cxon2fbinfo_local.yres = fb_vinfo.yres;
	cxon2fbinfo_local.xres_virtual = fb_vinfo.xres_virtual;
	cxon2fbinfo_local.yres_virtual = fb_vinfo.yres_virtual;
	cxon2fbinfo_local.is_slaveModeEnabled = 0;
	cxon2fbinfo_local.foregroundAlphaFactor = 240;
	cxon2fbinfo_local.colorKey = 5;
	cxon2fbinfo_local.fb_offset = fb_offset;
	cxon2fbinfo_local.enable_overlay = 0;
	cxon2fbinfo_local.deinterlace = 0;

	cxon2fbinfo_local.startOfFBDevPhysicalAddress = fb_finfo.smem_start ;
	cxon2fbinfo_local.startOfFBDevVirtualAddress = (uint32_t)frame_buffer ;

	cxfbinfo_init = 0;
	cx_on2_pp_update_fb_params(&cxOn2DecContext, &cxon2fbinfo_local, VideoDecoder::callBackFunc);
	cxfbinfo_init = 1;

	if(vo_doublebuffering) 
	{
		cx_on2_pp_update_framebuffer(&cxOn2DecContext,	pCxfb_phy_ptr, pCxfb_virt_ptr);
	}

    return 0;
}

void VideoDecoder::callBackFunc(unsigned long *p_phy_ptr, unsigned long *p_virt_ptr)
{
	objectInstance->cxon2_flip_page(p_phy_ptr, p_virt_ptr);
}

void VideoDecoder::cxon2_flip_page(unsigned long *p_phy_ptr, unsigned long *p_virt_ptr)
{
    int next_page = !fb_page;
    int page_delta = next_page - fb_page;
    int i;


    if (!vo_doublebuffering)
        return;

    if(	no_more_flip ) {
		if(vo_doublebuffering ) {
			*p_phy_ptr = pCxfb_phy_ptr;
			*p_virt_ptr = pCxfb_virt_ptr;
		}
		return;
    }

	if (fb_page == 1)
	{
#if 1	
	int frameSize = lcdWidth * lcdHeight * (fb_bpp/8);
#if 0//我们使用的自动拷贝
		
		for(i=0 ;i<lcdHeight;i++)
		{
			memcpy(frame_buffer+frameSize+(i*lcdWidth*(fb_bpp/8))+(fb_bpp/8)*850,frame_buffer+(i*lcdWidth*(fb_bpp/8))+(fb_bpp/8)*850,(fb_bpp/8)*(lcdWidth-850));
		}
#else //默认处理代码
		
		memcpy(frame_buffer+frameSize, frame_buffer, videoDecoderArg.screen_ypos*lcdWidth*(fb_bpp/8));
		memcpy(frame_buffer+frameSize + lcdWidth*(videoDecoderArg.screen_ypos+videoDecoderArg.height)*(fb_bpp/8),
									frame_buffer + lcdWidth*(videoDecoderArg.screen_ypos+videoDecoderArg.height)*(fb_bpp/8),
									videoDecoderArg.screen_ypos*lcdWidth*(fb_bpp/8));
									
#endif
	

#endif			
	}
	

	fb_vinfo.yoffset = (fb_page + fb_offset) * fb_yres;
    ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &fb_vinfo);




    center += page_delta * fb_yres * fb_line_len;
    fb_page = next_page;

	/* update phy. addr of framebuffer */
	pCxfb_phy_ptr += page_delta * fb_yres * fb_line_len;
	pCxfb_virt_ptr += page_delta * fb_yres * fb_line_len;

	if(vo_doublebuffering ) {
		*p_phy_ptr = pCxfb_phy_ptr;
		*p_virt_ptr = pCxfb_virt_ptr;
	}
}

void VideoDecoder::cx_force_flip_page()
{
	int ret = 0;
    int next_page = !fb_page;
    int page_delta = next_page - fb_page;


		if (!vo_doublebuffering)
			return;

		fb_vinfo.yoffset = fb_page * fb_yres;
		ret = ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &fb_vinfo);

		center += page_delta * fb_yres * fb_line_len;
		fb_page = next_page;

}

void VideoDecoder::stopDisplay()
{
	fb_vinfo.yoffset = 0;
    ioctl(fb_dev_fd, FBIOPAN_DISPLAY, &fb_vinfo);
}

