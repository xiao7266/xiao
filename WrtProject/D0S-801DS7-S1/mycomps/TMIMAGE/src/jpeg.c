/**
 * Functions that will decompress a JPEG image into a presentable
 * format. This is using libjpeg instead of libmagick for JPEG images,
 * because it is much faster and gives better quality that libmagick.
 *
 * @author Tomas Berndtsson <tomas@nocrew.org>
 */

/*
 * Copyright (C) 1999, Tomas Berndtsson <tomas@nocrew.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <setjmp.h>
#include "wrt_log.h"
#include "jpeg.h"
#include "jpeglib.h"
#include "malloc.h"    /* used ours memory pool */
#include "image.h"
#include "SDL_video.h"

int OUTPUT_BUF_SIZE;
int OUTPUT_FILE_LEN;

static unsigned char* jpg_data_buf = 0;

/**
 *
 */
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void) my_error_exit (j_common_ptr cinfo);

//
//	to handle fatal errors.
//	the original JPEG code will just exit(0). can't really
//	do that in Windows....
//

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	/* Always display the message. */
	//MessageBox(NULL,buffer,"JPEG Fatal Error",MB_ICONSTOP);


	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}
struct error_information {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

/**
 *
 */

//========================================================
static void error_handler(j_common_ptr cinfo)
{
  struct error_information *error;

  error = (struct error_information *)cinfo->err;
  longjmp(error->setjmp_buffer, 1);
}

struct jpeg_decompress_struct decomp_struct,*ds_ptr = &decomp_struct;

char line_buffer[4*16*1024];
char *line_buffer_ptr = line_buffer;

typedef struct ERROR_HANDLER_DESC
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
} ERROR_HANDLER_DESC;

ERROR_HANDLER_DESC ehd;
#define MAX_LINE_WIDTH_BYTES	(16 * 1024 * 4)


struct image_data *iptv_image_jpeg_read2(FILE* file)
{	
		int time;
		struct timeval tv_begin, tv_end;
		struct image_data *picture = NULL;
		unsigned char *picturep;
		int step, i, line_in, row_stride, x, number_of_colours;
		JSAMPARRAY buffer, colourmap;

		unsigned long int start_time;
		unsigned long int end_time;
		unsigned long int total_ms;
		//gettimeofday(&tv_begin, NULL);
		if ((picture = (struct image_data *)malloc(sizeof(struct image_data))) == NULL)
		{
		    return NULL;
		}


		picture->data = NULL;
		picture->next = NULL;
		picture->transparent = -1;
		ds_ptr->err = iptv_jpeg_std_error(&(ehd.pub));
		ehd.pub.error_exit = error_handler;
		if (setjmp(ehd.setjmp_buffer) == 1)
		{
			printf("\nError from libjpeg, exiting!\n\n");
			free(picture);
			exit(-1);
		}
		jpeg_create_decompress(ds_ptr);

		jpeg_stdio_src(ds_ptr,file);

		jpeg_read_header(ds_ptr,TRUE);


		ds_ptr->out_color_space = JCS_RGB;
		ds_ptr->scale_num = 1;//scale_n;
		ds_ptr->scale_denom = 1;
	//	printf("scale_num %d   scale_denom	%d \n",ds_ptr->scale_num,ds_ptr->scale_denom);

		jpeg_calc_output_dimensions(ds_ptr);
		picture->width = ds_ptr->output_width;
		picture->height = ds_ptr->output_height;

		/* Allocate space for the image data in our struct. */
		picture->size = (picture->width * picture->height * 24) / 8;
	    if(jpg_data_buf == 0)
	         jpg_data_buf = (unsigned char*)malloc(picture->width * picture->height*3);
	    picture->data  = jpg_data_buf;
	    memset(jpg_data_buf,0,picture->width * picture->height*3);

		//picture->data = (unsigned char *)malloc(picture->size);
		if(picture->data == NULL)
		{
			jpeg_destroy_decompress(ds_ptr);
			free(picture);
			return NULL;
		}
		jpeg_start_decompress(ds_ptr);
	
		if ((ds_ptr->output_components * ds_ptr->output_width) > MAX_LINE_WIDTH_BYTES)
		{
			int line_length = ds_ptr->output_components * ds_ptr->output_width;
	
			printf("Output scanline byte length * line request ");
			printf("(%d * %d = %d) is larger than line buffer size (%d)!\n",
							line_length,1,line_length*1,MAX_LINE_WIDTH_BYTES);
			jpeg_abort_decompress(ds_ptr);
			return(-1);
		}
//		linein = 0;
//		while (linein++ < ds_ptr->output_height)
		
//		{
//			int lines_returned;
	
//			lines_returned = jpeg_read_scanlines(ds_ptr,&line_buffer_ptr,1);
			
//		}
		/* Allocate memory for one row. */
	row_stride = ds_ptr->output_width * ds_ptr->output_components;
	buffer = (*ds_ptr->mem->alloc_sarray)
		((j_common_ptr) ds_ptr, JPOOL_IMAGE, row_stride, 1);

	/* Start reading and converting the image, line by line. */
	line_in = 0;
	picturep = picture->data;
	//gettimeofday(&tv_end, NULL);
	//time = (1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec)/1000;
	//printf("decode time1 :%d  %dx%d\n",time,ds_ptr->output_width,ds_ptr->output_height);
	while(line_in < ds_ptr->output_height)
	{
	    jpeg_read_scanlines(ds_ptr, buffer, 1);

	    i = 0;
#if 1
	    for(x = 0 ; x < ds_ptr->output_width ; x++)
	    {
		unsigned char red, green, blue;

		red   =
			(unsigned char)(((unsigned long)buffer[0][i++] << 8) >> BITS_IN_JSAMPLE);
		green =
			(unsigned char)(((unsigned long)buffer[0][i++] << 8) >> BITS_IN_JSAMPLE);
		blue  =
			(unsigned char)(((unsigned long)buffer[0][i++] << 8) >> BITS_IN_JSAMPLE);

// 		*(picturep + ((cinfo.output_height-line_in-1)*cinfo.output_width+x)*3)   = blue;
//                 *(picturep + ((cinfo.output_height-line_in-1)*cinfo.output_width+x)*3+1) = green;
//                 *(picturep + ((cinfo.output_height-line_in-1)*cinfo.output_width+x)*3+2) = red;
        *(picturep + ((line_in)*ds_ptr->output_width+x)*3) = blue;
        *(picturep + ((line_in)*ds_ptr->output_width+x)*3+1) = green;
        *(picturep + ((line_in)*ds_ptr->output_width+x)*3+2) = red;
   //     *(picturep + ((line_in)*ds_ptr->output_width+x)*4+3) = 0;
	    }
#endif
	    line_in++;
	}
	//gettimeofday(&tv_end, NULL);
	//time = (1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec)/1000;
	//
	//printf("decode time2 :%d  %dx%d\n",time,ds_ptr->output_width,ds_ptr->output_height);
		
		jpeg_finish_decompress(ds_ptr);
		jpeg_destroy_decompress(ds_ptr);
	
		fclose(file);
		

	return picture;	

}

struct SDL_Surface * iptv_image_jpeg_read_sdl(unsigned char *buf, long length)
{
	 int start;
	 struct jpeg_decompress_struct cinfo;
	 JSAMPROW rowptr[1];
	 SDL_Surface *volatile surface = NULL;
	 struct error_information error;

	 

	 /* Create a decompression structure and load the JPEG header */
	 cinfo.err = iptv_jpeg_std_error(&error.pub);
	 error.pub.error_exit = error_handler;
	 if(setjmp(error.setjmp_buffer))
	 {
	  	jpeg_destroy_decompress(&cinfo);
	  	printf("jpeg setjmpg\n");
	  	return NULL;
	 }

	 jpeg_create_decompress(&cinfo);

	 jpeg_mem_src(&cinfo, buf, length);

	 jpeg_read_header(&cinfo, TRUE);

	 if(cinfo.num_components == 4) {
		/* Set 32-bit Raw output */
		cinfo.out_color_space = JCS_CMYK;//JCS_CMYK;
		cinfo.quantize_colors = FALSE;
		jpeg_calc_output_dimensions(&cinfo);

		/* Allocate an output surface to hold the image */
		surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		      cinfo.output_width, cinfo.output_height, 32,
		                 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	 } else {
		/* Set 24-bit RGB output */
		cinfo.out_color_space = JCS_RGB;
		cinfo.quantize_colors = FALSE;//

		jpeg_calc_output_dimensions(&cinfo);

		/* Allocate an output surface to hold the image */
		surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		      cinfo.output_width, cinfo.output_height, 24,
		                 0x0000FF, 0x00FF00,0xFF0000 ,
		                 0 );
	 }

	 if ( surface == NULL ) {
	  	jpeg_destroy_decompress(&cinfo);
	  	printf("jpeg SDL_CreateRGBSurface failed\n");
	  	return NULL;
	 }
	 /* Decompress the image */
	 jpeg_start_decompress(&cinfo);
	 
	 while ( cinfo.output_scanline < cinfo.output_height ) {
	  rowptr[0] = (JSAMPROW)(Uint8 *)surface->pixels +
	                      cinfo.output_scanline * surface->pitch;
	  jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION) 1);
	 }
	 jpeg_finish_decompress(&cinfo);
	 jpeg_destroy_decompress(&cinfo);

	 return (void*)surface;
}


struct image_data *iptv_image_jpeg_read(unsigned char *buf, long length)
{
	struct jpeg_decompress_struct cinfo;
	struct error_information error;
	int step, i, line_in, row_stride, x, number_of_colours;
	JSAMPARRAY buffer, colourmap;
    struct image_data *picture = NULL;
	unsigned char *picturep;
	unsigned char red, green, blue;
	
	OUTPUT_BUF_SIZE = 1024*1024*2;
	//gettimeofday(&tv_begin, NULL);

	if ((picture = (struct image_data *)malloc(sizeof(struct image_data))) == NULL)
	{
	    return NULL;
	}


	picture->data = NULL;
	picture->next = NULL;
	picture->transparent = -1;


	cinfo.err = iptv_jpeg_std_error(&error.pub);
	error.pub.error_exit = error_handler;
	if(setjmp(error.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		free(picture);
		return NULL;
	}

	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, buf, length);

	jpeg_read_header(&cinfo, TRUE);

	/* We cannot do automatic scaling with libjpeg.
	* So, we will have to do this ourselves later.
	*/

	cinfo.do_block_smoothing = FALSE;
	cinfo.dct_method =JDCT_ISLOW;
	cinfo.out_color_space = JCS_RGB;
	cinfo.scale_num =1;
	cinfo.scale_denom =1;

	cinfo.quantize_colors = FALSE;

	jpeg_calc_output_dimensions(&cinfo);


	/* Since scaling is not possible yet, we only use the original
	* geometry of the image.
	*/
	picture->width = cinfo.output_width;
	picture->height = cinfo.output_height;

	/* Allocate space for the image data in our struct. */
	picture->size = (picture->width * picture->height * 24) / 8;
	if(jpg_data_buf == 0)
    	jpg_data_buf = (unsigned char*)malloc(2048*4096*3);

    picture->data  = jpg_data_buf;
    memset(jpg_data_buf,0,2048*4096*3);//24M 内存分配
	
	if(picture->data == NULL)
	{
		jpeg_destroy_decompress(&cinfo);
		free(picture);
		return NULL;
	}

	jpeg_start_decompress(&cinfo);

	/* Allocate memory for one row. */
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	/* Start reading and converting the image, line by line. */
	line_in = 0;
	picturep = picture->data;
	while(line_in < cinfo.output_height)
	{
	    jpeg_read_scanlines(&cinfo, buffer, 1);

	    i = 0;

	    for(x = 0 ; x < cinfo.output_width ; x++)
	    {
			red   = (unsigned char)(((unsigned long)buffer[0][i++] << 8) >> BITS_IN_JSAMPLE);
			green = (unsigned char)(((unsigned long)buffer[0][i++] << 8) >> BITS_IN_JSAMPLE);
			blue  = (unsigned char)(((unsigned long)buffer[0][i++] << 8) >> BITS_IN_JSAMPLE);

	        *(picturep + ((line_in)*cinfo.output_width+x)*3) = blue;
	        *(picturep + ((line_in)*cinfo.output_width+x)*3+1) = green;
	        *(picturep + ((line_in)*cinfo.output_width+x)*3+2) = red;
	    }

	    line_in++;
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return picture;
}


int iptv_image_write_jpeg(struct image_data *picture,unsigned char *outputbuf,int quality,int bits)
{
	struct jpeg_compress_struct cinfo;
  	struct jpeg_error_mgr jerr;
  	FILE * outfile;
  	JSAMPROW row_pointer[1];
  	int row_stride;
	unsigned short *picturep;
	unsigned char *image_buffer;
	int i,y,x;
	unsigned char red, green, blue;
	OUTPUT_BUF_SIZE = 256*1024;
	OUTPUT_FILE_LEN = 0;
	
	image_buffer = (unsigned char *)malloc(640*480*3);
	if(image_buffer == NULL){
		WRT_DEBUG("malloc failed \n");
		return 0;
	}
	//生成JPEG压缩原始数据
	picturep = (unsigned short *)picture->data;
	i = 0;
	y = 0;
	for(y=58;y<538;y++) {
	    for(x = 106 ; x < 746 ; x++) {
			red =   ((*(picturep + ((y)*picture->width+x)))&0xF800)>>11;//16 bit per pixel
        		green =  ((*(picturep + ((y)*picture->width+x)))&0x07E0)>>5;
        		blue =    ((*(picturep + ((y)*picture->width+x)))&0x001F);
			//blue =   *(picturep + ((y)*picture->width+x)*(bits/8)+2);
			//green =  *(picturep + ((y)*picture->width+x)*(bits/8)+1);
			//red =	 *(picturep + ((y)*picture->width+x)*(bits/8));
			red <<=3;
			green <<=2;
			blue <<=3;
			image_buffer[i++] = red;
			image_buffer[i++] = green;
			image_buffer[i++] = blue;
	    }
	    //y++;
	}

  	cinfo.err = iptv_jpeg_std_error(&jerr);
  	jpeg_create_compress(&cinfo);

  	jpeg_stdio_dest(&cinfo, (FILE *)outputbuf);

  	cinfo.image_width = 640;//picture->width;
  	cinfo.image_height = 480;//picture->height;
  	cinfo.input_components = 3;
  	cinfo.in_color_space = JCS_RGB;

  	jpeg_set_defaults(&cinfo);

  	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  	jpeg_start_compress(&cinfo, TRUE);

  	row_stride = 640 * 3;	/*picture->width;  JSAMPLEs per row in image_buffer */
	//一行的数据，下面将每行写入cinfo
  	while (cinfo.next_scanline < cinfo.image_height) {
    	row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  	}

  	jpeg_finish_compress(&cinfo);//compress

  	jpeg_destroy_compress(&cinfo);

	free(image_buffer);

	return OUTPUT_FILE_LEN;
}

#if 0 //old 
int iptv_image_write_jpeg(struct image_data *picture,unsigned char *outputbuf,int quality,int bits)
{
	struct jpeg_compress_struct cinfo;
  	struct jpeg_error_mgr jerr;
  	FILE * outfile;
  	JSAMPROW row_pointer[1];
  	int row_stride;
	unsigned char *picturep;
	unsigned char *image_buffer;
	int i,y,x;


	OUTPUT_BUF_SIZE = 256*1024;
	OUTPUT_FILE_LEN = 0;
	

		

	image_buffer = (unsigned char *)malloc(picture->height*picture->width*3);
	if(image_buffer == NULL){
		WRT_DEBUG("malloc failed \n");
		return 0;
	}
	//生成JPEG压缩原始数据
	picturep = picture->data;
	i = 0;
	y = 0;
	while(y < picture->height) {
	    for(x = 0 ; x < picture->width ; x++) {
			unsigned char red, green, blue;
			blue =   *(picturep + ((y)*picture->width+x)*(bits/8));
        		green =  *(picturep + ((y)*picture->width+x)*(bits/8)+1);
        		red =    *(picturep + ((y)*picture->width+x)*(bits/8)+2);

			image_buffer[i++] = red;
			image_buffer[i++] = green;
			image_buffer[i++] = blue;
	    }
	    y++;
	}

  	cinfo.err = iptv_jpeg_std_error(&jerr);
  	jpeg_create_compress(&cinfo);

  	jpeg_stdio_dest(&cinfo, (FILE *)outputbuf);

  	cinfo.image_width = picture->width;
  	cinfo.image_height = picture->height;
  	cinfo.input_components = 3;
  	cinfo.in_color_space = JCS_RGB;

  	jpeg_set_defaults(&cinfo);

  	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  	jpeg_start_compress(&cinfo, TRUE);

  	row_stride = picture->width * 3;	/* JSAMPLEs per row in image_buffer */

  	while (cinfo.next_scanline < cinfo.image_height) {
    	row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  	}

  	jpeg_finish_compress(&cinfo);

  	jpeg_destroy_compress(&cinfo);

	free(image_buffer);

	return OUTPUT_FILE_LEN;
}
#endif

//24位的位图buf2 alpha -->buf1
//widht2  + x  < width1
//height2 + y  < height1
//RGB(0,0,0).做为透明颜色。没有采用混合算法，只是用于透明某中颜色。
//buf1 src
//buf2 dest
int BufAlpha(int x,int y,unsigned char* buf1,int width1,int height1,unsigned char* buf2,int width2,int height2)
{
   int w=0;
   int h=0;
   int pos1=0;
   int pos2 = 0;
   if(width2 + x > width1)
       return 0;
   if(height2 + y > height1)
       return 0;

   for(h = 0; h < height2; h++)
	   for(w =0; w < width2;w++)
	   {
		          pos1 = ((y+h) * width1 + x+w) * 3;
				  pos2 = (h*width2 + w)*3;

				  if(buf2[pos2] == 0x00 && buf2[pos2+1] == 0x00 && buf2[pos2+2] == 0x00  /*&& buf2[pos2+3] == 0x00*/)
					  continue;

                  buf1[pos1] = buf2[pos2];
				  buf1[pos1+1] = buf2[pos2+1];
				  buf1[pos1+2] = buf2[pos2+2];
				  //buf1[pos1+3] = buf2[pos2+3];
	   }
	return 1;
}

#if 0
int iptv_image_write_jpeg_420(unsigned char *y,unsigned char *u,unsigned char *v,int width,int height,unsigned char *outputbuf,int quality)
{
	struct jpeg_compress_struct cinfo;
  	struct jpeg_error_mgr jerr;
  	FILE * outfile;
  	JSAMPROW row_pointer[1];
  	int row_stride;
	unsigned char *picturep;
	unsigned char *image_buffer;
	unsigned char *inside_u;
	unsigned char *inside_u1;
	unsigned char *inside_v;
	unsigned char *inside_v1;
	int inside_ptr1;
	int ptr1;
	int inside_ptr2;
	int ptr2;
	int width1,height1;
	int value;
	int height_value,width_value;
	int temp1,temp2,temp3,temp4;

	OUTPUT_BUF_SIZE = 256*1024;
	OUTPUT_FILE_LEN = 0;

	image_buffer = (unsigned char *)malloc(height*width*3);
	inside_u = (unsigned char *)malloc(height*width);
	inside_u1 = (unsigned char *)malloc(height*width/2);
	inside_v = (unsigned char *)malloc(height*width);
	inside_v1 = (unsigned char *)malloc(height*width/2);

	width1 = width>>1;
	height1 = height>>1;

	inside_ptr1 = 0;
	ptr1 = 0;

	inside_ptr2 = 0;
	ptr2 = 0;

	//在x方向插值
	for(height_value=0; height_value<height1; height_value++) {
		//对U插值
		inside_u1[inside_ptr1++] = u[ptr1];
		value = (9*(u[ptr1]+u[ptr1+1])-(u[ptr1]+u[ptr1+2])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u1[inside_ptr1++] = value;
		ptr1++;

	    for(width_value=1 ; width_value<(width1-2); width_value++) {
			inside_u1[inside_ptr1++] = u[ptr1];
			value = (9*(u[ptr1]+u[ptr1+1])-(u[ptr1-1]+u[ptr1+2])+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_u1[inside_ptr1++] = value;
			ptr1++;
	    }

		inside_u1[inside_ptr1++] = u[ptr1];
		value = (9*(u[ptr1]+u[ptr1+1])-(u[ptr1-1]+u[ptr1+1])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u1[inside_ptr1++] = value;
		ptr1++;

		inside_u1[inside_ptr1++] = u[ptr1];
		value = (9*(u[ptr1]+u[ptr1])-(u[ptr1-1]+u[ptr1])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u1[inside_ptr1++] = value;
		ptr1++;


		//对V插值
		inside_v1[inside_ptr2++] = v[ptr2];
		value = (9*(v[ptr2]+v[ptr2+1])-(v[ptr2]+v[ptr2+2])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v1[inside_ptr2++] = value;
		ptr2++;

		for(width_value=1 ; width_value<(width1-2); width_value++) {
			inside_v1[inside_ptr2++] = v[ptr2];
			value = (9*(v[ptr2]+v[ptr2+1])-(v[ptr2-1]+v[ptr2+2])+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_v1[inside_ptr2++] = value;
			ptr2++;
		}

		inside_v1[inside_ptr2++] = u[ptr2];
		value = (9*(u[ptr2]+u[ptr2+1])-(u[ptr2-1]+u[ptr2+1])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v1[inside_ptr2++] = value;
		ptr2++;

		inside_v1[inside_ptr2++] = v[ptr2];
		value = (9*(v[ptr2]+v[ptr2])-(v[ptr2-1]+v[ptr2])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v1[inside_ptr2++] = value;
		ptr2++;
	}


	//在y方向插值

	inside_ptr1 = 0;
	ptr1 = 0;

	inside_ptr2 = 0;
	ptr2 = 0;

	memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
	inside_ptr1 += width;
	for(width_value=0; width_value<width; width_value++) {
		value = (9*(inside_u1[ptr1]+inside_u1[ptr1+width])-(inside_u1[ptr1]+inside_u1[ptr1+(width<<1)])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u[inside_ptr1++] = value;
		ptr1++;
	}


	memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
	inside_ptr2 += width;
	for(width_value=0; width_value<width; width_value++) {
		value = (9*(inside_v1[ptr2]+inside_v1[ptr2+width])-(inside_v1[ptr2]+inside_v1[ptr2+(width<<1)])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v[inside_ptr2++] = value;
		ptr2++;
	}

	for(height_value=1; height_value<height1-2; height_value++) {
		ptr1 = height_value*width;
		memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
		inside_ptr1 += width;

		for(width_value=0; width_value<width; width_value++) {
			value = (9*(inside_u1[ptr1]+inside_u1[ptr1+width])-(inside_u1[ptr1-width]+inside_u1[ptr1+(width<<1)])+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_u[inside_ptr1++] = value;
			ptr1++;
	    }



		ptr2 = height_value*width;
		memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
		inside_ptr2 += width;

		for(width_value=0; width_value<width; width_value++) {
			value = (9*(inside_v1[ptr2]+inside_v1[ptr2+width])-(inside_v1[ptr2-width]+inside_v1[ptr2+(width<<1)])+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_v[inside_ptr2++] = value;
			ptr2++;
		}
	}

	ptr1 = height_value*width;
	memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
	inside_ptr1 += width;
	for(width_value=0; width_value<width; width_value++) {
		value = (9*(inside_u1[ptr1]+inside_u1[ptr1+width])-(inside_u1[ptr1-width]+inside_u1[ptr1+width])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u[inside_ptr1++] = value;
		ptr1++;
	}


	ptr2 = height_value*width;
	memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
	inside_ptr2 += width;
	for(width_value=0; width_value<width; width_value++) {
		value = (9*(inside_v1[ptr2]+inside_v1[ptr2+width])-(inside_v1[ptr2-width]+inside_v1[ptr2+width])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v[inside_ptr2++] = value;
		ptr2++;
	}
	height_value++;

	ptr1 = height_value*width;
	memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
	inside_ptr1 += width;

	for(width_value=0; width_value<width; width_value++) {
		value = (9*(inside_u1[ptr1]+inside_u1[ptr1])-(inside_u1[ptr1-width]+inside_u1[ptr1])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u[inside_ptr1++] = value;
		ptr1++;
	}

	ptr2 = height_value*width;
	memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
	inside_ptr2 += width;

	for(width_value=0; width_value<width; width_value++) {
		value = (9*(inside_v1[ptr2]+inside_v1[ptr2])-(inside_v1[ptr2-width]+inside_v1[ptr2])+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v[inside_ptr2++] = value;
		ptr2++;
	}


	//生成JPEG压缩原始数据
	inside_ptr1 = 0;
	ptr1 = 0;
	for(height_value=0; height_value<height; height_value++) {
	    for(width_value=0; width_value<width; width_value++) {
			unsigned char red, green, blue;
			blue =   y[inside_ptr1]+1.402*(inside_u[inside_ptr1]-128);
        	green =  y[inside_ptr1]-0.34414*(inside_v[inside_ptr1]-128)-0.71414*(inside_u[inside_ptr1]-128);
        	red =    y[inside_ptr1]+1.772*(inside_v[inside_ptr1]-128);

			inside_ptr1++;

			image_buffer[ptr1++] = red;
			image_buffer[ptr1++] = green;
			image_buffer[ptr1++] = blue;
	    }
	}

  	cinfo.err = jpeg_std_error(&jerr);
  	jpeg_create_compress(&cinfo);

  	jpeg_stdio_dest(&cinfo, (FILE *)outputbuf);

  	cinfo.image_width = width;
  	cinfo.image_height = height;
  	cinfo.input_components = 3;
  	cinfo.in_color_space = JCS_RGB;

  	jpeg_set_defaults(&cinfo);

  	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  	jpeg_start_compress(&cinfo, TRUE);

  	row_stride = width * 3;	/* JSAMPLEs per row in image_buffer */

  	while (cinfo.next_scanline < cinfo.image_height) {
    	row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  	}

  	jpeg_finish_compress(&cinfo);

  	jpeg_destroy_compress(&cinfo);

	free(image_buffer);
	free(inside_u);
	free(inside_u1);
	free(inside_u);
	free(inside_v1);

	return OUTPUT_FILE_LEN;
}
#else

	static unsigned char *image_buffer=NULL;
	static unsigned char *inside_u=NULL;
	static unsigned char *inside_u1=NULL;
	static unsigned char *inside_v=NULL;
	static unsigned char *inside_v1=NULL;
	static int   g_buffersize[5]={0,0,0,0,0};

int iptv_image_write_jpeg_420(unsigned char *y,unsigned char *u,unsigned char *v,int width,int height,unsigned char *outputbuf,int quality,int destwidth)
{
	struct jpeg_compress_struct cinfo;
  	struct jpeg_error_mgr jerr;
    struct error_information error;
  	FILE * outfile;
  	JSAMPROW row_pointer[1];
  	int row_stride;


	int inside_ptr1;
	int ptr1;
	int inside_ptr2;
	int ptr2;
	int width1,height1;
	int value;
	int height_value,width_value;
	int temp1,temp2,temp3,temp4;

	OUTPUT_BUF_SIZE = 256*1024;
	OUTPUT_FILE_LEN = 0;


    if(image_buffer == NULL){
    	image_buffer = (unsigned char *)malloc(height*destwidth*3);
    	if(image_buffer == NULL){
    		WRT_DEBUG("image_buffer malloc failed size =%d ",height*destwidth*3);
    		return 0;
    	}
    	g_buffersize[0] = height*destwidth*3;
    }else if(g_buffersize[0] < height*destwidth*3 ){
         free(image_buffer);
         image_buffer = NULL;
    	 image_buffer = (unsigned char *)malloc(height*destwidth*3);
    	if(image_buffer == NULL){
    		WRT_DEBUG("image_buffer malloc failed size =%d ",height*destwidth*3);
    		return 0;
    	}
    	g_buffersize[0] = height*destwidth*3;
    }

    if(inside_u == NULL){
    	inside_u = (unsigned char *)malloc(height*width);
    	if(inside_u == NULL){
    		WRT_DEBUG("inside_u malloc failed size = %d",height*width);
    		free(image_buffer);
    		image_buffer = NULL;
    		return 0;
    	}
    	g_buffersize[1] = height*width;
    }else if(g_buffersize[1] < height*width){
        free(inside_u);
        inside_u = NULL;
    	inside_u = (unsigned char *)malloc(height*width);
    	if(inside_u == NULL){
    		WRT_DEBUG("inside_u malloc failed size = %d",height*width);
    		free(image_buffer);
    		image_buffer = NULL;
    		return 0;
    	}
    	g_buffersize[1] = height*width;
    }

    if(inside_u1 == NULL){
    	inside_u1 = (unsigned char *)malloc(height*width/2);
    	if(inside_u1 == NULL){
    		WRT_DEBUG("inside_u1 malloc failed size = %d",height*width/2);
    		free(image_buffer);
    		image_buffer = NULL;
    		free(inside_u);
    		inside_u = NULL;
    		return 0;
    	}
    	g_buffersize[2] = height*width/2;
    }else if(g_buffersize[2] < height*width/2){
    	inside_u1 = (unsigned char *)malloc(height*width/2);
    	if(inside_u1 == NULL){
    		WRT_DEBUG("inside_u1 malloc failed size = %d",height*width/2);
    		free(image_buffer);
    		image_buffer = NULL;
    		free(inside_u);
    		inside_u = NULL;
    		return 0;
    	}
    	g_buffersize[2] = height*width/2;
    }

    if(inside_v == NULL){
    	inside_v = (unsigned char *)malloc(height*width);
    	if(inside_u1 == NULL){
    		WRT_DEBUG("inside_v malloc failed size = %d",height*width);
    		free(image_buffer);
    		image_buffer = NULL;
    		free(inside_u);
    		inside_u  = NULL;
    		free(inside_u1);
    		inside_u1 = NULL;
    		return 0;
    	}
       g_buffersize[3] = height*width;
    }else if(g_buffersize[3] < height*width){
    	inside_v = (unsigned char *)malloc(height*width);
    	if(inside_u1 == NULL){
    		WRT_DEBUG("inside_v malloc failed size = %d",height*width);
    		free(image_buffer);
    		image_buffer = NULL;
    		free(inside_u);
    		inside_u  = NULL;
    		free(inside_u1);
    		inside_u1 = NULL;
    		return 0;
    	}
       g_buffersize[3] = height*width;
    }

    if(inside_v1 == NULL){
    	inside_v1 = (unsigned char *)malloc(height*width/2);
    	if(inside_v1 == NULL){
    		WRT_DEBUG("inside_v1 malloc failed size = %d",height*width/2);
    		free(image_buffer);
    		image_buffer = NULL;
    		free(inside_u);
    		inside_u = NULL;
    		free(inside_u1);
    		inside_u1 = NULL;
    		free(inside_v);
    		inside_v = NULL;
    		return 0;
    	}
        g_buffersize[4] = height*width/2;
    }else if(g_buffersize[4] < height*width/2){
    	inside_v1 = (unsigned char *)malloc(height*width/2);
    	if(inside_v1 == NULL){
    		WRT_DEBUG("inside_v1 malloc failed size = %d",height*width/2);
    		image_buffer = NULL;
    		free(inside_u);
    		inside_u = NULL;
    		free(inside_u1);
    		inside_u1 = NULL;
    		free(inside_v);
    		inside_v = NULL;
    		return 0;
    	}
        g_buffersize[4] = height*width/2;
    }

	width1 = width>>1;
	height1 = height>>1;

	inside_ptr1 = 0;
	ptr1 = 0;

	inside_ptr2 = 0;
	ptr2 = 0;

	//在x方向插值
	for(height_value=0; height_value<height1; height_value++) {
		//对U插值
		inside_u1[inside_ptr1++] = u[ptr1];

		temp1 = u[ptr1];
		temp2 = u[ptr1+1];
		temp3 = u[ptr1+2];

		value = (9*(temp1+temp2)-(temp1+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u1[inside_ptr1++] = value;
		ptr1++;

	    for(width_value=1 ; width_value<(width1-2); width_value++) {
			inside_u1[inside_ptr1++] = u[ptr1];

			temp1 = u[ptr1];
			temp2 = u[ptr1-1];
			temp3 = u[ptr1+1];
			temp4 = u[ptr1+2];

			value = (9*(temp1+temp3)-(temp2+temp4)+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_u1[inside_ptr1++] = value;
			ptr1++;
	    }

		inside_u1[inside_ptr1++] = u[ptr1];

		temp1 = u[ptr1];
		temp2 = u[ptr1-1];
		temp3 = u[ptr1+1];

		value = (9*(temp1+temp3)-(temp2+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u1[inside_ptr1++] = value;
		ptr1++;

		inside_u1[inside_ptr1++] = u[ptr1];

		temp1 = u[ptr1];
		temp2 = u[ptr1-1];

		value = (9*(temp1+temp1)-(temp2+temp1)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u1[inside_ptr1++] = value;
		ptr1++;


		//对V插值
		inside_v1[inside_ptr2++] = v[ptr2];

		temp1 = v[ptr2];
		temp2 = v[ptr2+1];
		temp3 = v[ptr2+2];

		value = (9*(temp1+temp2)-(temp1+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v1[inside_ptr2++] = value;
		ptr2++;

		for(width_value=1 ; width_value<(width1-2); width_value++) {
			inside_v1[inside_ptr2++] = v[ptr2];

			temp1 = v[ptr2];
			temp2 = v[ptr2-1];
			temp3 = v[ptr2+1];
			temp4 = v[ptr2+2];

			value = (9*(temp1+temp3)-(temp2+temp4)+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_v1[inside_ptr2++] = value;
			ptr2++;
		}

		inside_v1[inside_ptr2++] = u[ptr2];

		temp1 = v[ptr2];
		temp2 = v[ptr2-1];
		temp3 = v[ptr2+1];

		value = (9*(temp1+temp3)-(temp2+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v1[inside_ptr2++] = value;
		ptr2++;

		inside_v1[inside_ptr2++] = v[ptr2];

		temp1 = v[ptr2];
		temp2 = v[ptr2-1];

		value = (9*(temp1+temp1)-(temp2+temp1)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v1[inside_ptr2++] = value;
		ptr2++;
	}


	//在y方向插值

	inside_ptr1 = 0;
	ptr1 = 0;

	inside_ptr2 = 0;
	ptr2 = 0;

	memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
	inside_ptr1 += width;
	for(width_value=0; width_value<width; width_value++) {
		temp1 = inside_u1[ptr1];
		temp2 = inside_u1[ptr1+width];
		temp3 = inside_u1[ptr1+(width<<1)];

		value = (9*(temp1+temp2)-(temp1+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u[inside_ptr1++] = value;
		ptr1++;
	}


	memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
	inside_ptr2 += width;
	for(width_value=0; width_value<width; width_value++) {
		temp1 = inside_v1[ptr2];
		temp2 = inside_v1[ptr2+width];
		temp3 = inside_v1[ptr2+(width<<1)];

		value = (9*(temp1+temp2)-(temp1+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v[inside_ptr2++] = value;
		ptr2++;
	}

	for(height_value=1; height_value<height1-2; height_value++) {
		ptr1 = height_value*width;
		memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
		inside_ptr1 += width;

		for(width_value=0; width_value<width; width_value++) {
			temp1 = inside_u1[ptr1];
			temp2 = inside_u1[ptr1-width];
			temp3 = inside_u1[ptr1+width];
			temp4 = inside_u1[ptr1+(width<<1)];

			value = (9*(temp1+temp3)-(temp2+temp4)+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_u[inside_ptr1++] = value;
			ptr1++;
	    }



		ptr2 = height_value*width;
		memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
		inside_ptr2 += width;

		for(width_value=0; width_value<width; width_value++) {
			temp1 = inside_v1[ptr2];
			temp2 = inside_v1[ptr2-width];
			temp3 = inside_v1[ptr2+width];
			temp4 = inside_v1[ptr2+(width<<1)];

			value = (9*(temp1+temp3)-(temp2+temp4)+8)>>4;
			value = value>255?255:((value<0)?0:value);
			inside_v[inside_ptr2++] = value;
			ptr2++;
		}
	}

	ptr1 = height_value*width;
	memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
	inside_ptr1 += width;
	for(width_value=0; width_value<width; width_value++) {
		temp1 = inside_u1[ptr1];
		temp2 = inside_u1[ptr1-width];
		temp3 = inside_u1[ptr1+width];

		value = (9*(temp1+temp3)-(temp2+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u[inside_ptr1++] = value;
		ptr1++;
	}


	ptr2 = height_value*width;
	memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
	inside_ptr2 += width;
	for(width_value=0; width_value<width; width_value++) {
		temp1 = inside_v1[ptr2];
		temp2 = inside_v1[ptr2-width];
		temp3 = inside_v1[ptr2+width];

		value = (9*(temp1+temp3)-(temp2+temp3)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v[inside_ptr2++] = value;
		ptr2++;
	}
	height_value++;

	ptr1 = height_value*width;
	memcpy(inside_u+inside_ptr1,inside_u1+ptr1,width);
	inside_ptr1 += width;
	for(width_value=0; width_value<width; width_value++) {
		temp1 = inside_u1[ptr1];
		temp2 = inside_u1[ptr1-width];

		value = (9*(temp1+temp1)-(temp2+temp1)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_u[inside_ptr1++] = value;
		ptr1++;
	}

	ptr2 = height_value*width;
	memcpy(inside_v+inside_ptr2,inside_v1+ptr2,width);
	inside_ptr2 += width;
	for(width_value=0; width_value<width; width_value++) {
		temp1 = inside_v1[ptr2];
		temp2 = inside_v1[ptr2-width];

		value = (9*(temp1+temp1)-(temp2+temp1)+8)>>4;
		value = value>255?255:((value<0)?0:value);
		inside_v[inside_ptr2++] = value;
		ptr2++;
	}


	//生成JPEG压缩原始数据
	inside_ptr1 = 0;
	ptr1 = 0;
	for(height_value=0; height_value<height; height_value++) {
	    for(width_value=0; width_value<width; width_value++) {
			short red, green, blue;
			blue =   y[inside_ptr1]+1.402*(inside_u[inside_ptr1]-128);
        	green =  y[inside_ptr1]-0.34414*(inside_v[inside_ptr1]-128)-0.71414*(inside_u[inside_ptr1]-128);
        	red =    y[inside_ptr1]+1.772*(inside_v[inside_ptr1]-128);
			if(red < 0)
				red = 0;
			if(red > 255)
				red = 255;
			if(green < 0)
				green = 0;
			if(green> 255)
				green = 255;
			if(blue < 0)
				blue = 0;
			if(blue > 255)
				blue = 255;
			inside_ptr1++;
			if(width_value < destwidth){
			    image_buffer[ptr1++] = red;
			    image_buffer[ptr1++] = green;
			    image_buffer[ptr1++] = blue;
			 }
	    }
	}

  	cinfo.err = iptv_jpeg_std_error(&error.pub);

  	error.pub.error_exit = error_handler;
	if(setjmp(error.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		WRT_DEBUG("setjmp = %d ");
		return 0;
	}
  	jpeg_create_compress(&cinfo);

  	jpeg_stdio_dest(&cinfo, (FILE *)outputbuf);

  	cinfo.image_width = destwidth;
  	cinfo.image_height = height;
  	cinfo.input_components = 3;
  	cinfo.in_color_space = JCS_RGB;

  	jpeg_set_defaults(&cinfo);

  	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  	jpeg_start_compress(&cinfo, TRUE);

  	row_stride = destwidth * 3;	/* JSAMPLEs per row in image_buffer */

  	while (cinfo.next_scanline < cinfo.image_height) {
    	row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  	}

  	jpeg_finish_compress(&cinfo);

  	jpeg_destroy_compress(&cinfo);

	return OUTPUT_FILE_LEN;
}
#endif
