/*---------------------------------------------------------------------------- 
 * CONEXANT SYSTEMS, INC. CONFIDENTIAL AND PROPRIETARY
 *
 * The information contained in this source code file
 * is strictly confidential and proprietary to Conexant Systems, Inc.
 * ("Conexant")
 *  
 * No part of this file may be possessed, reproduced or distributed, in
 * any form or by any means for any purpose, without the express written 
 * permission of Conexant Systems, Inc.
 * 
 * Except as otherwise specifically provided through an express agreement
 * with Conexant that governs the confidentiality, possession, use
 * and distribution of the information contained in this file, CONEXANT
 * PROVIDES THIS INFORMATION "AS IS" AND MAKES NO REPRESENTATIONS OR 
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY IMPLIED 
 * WARRANTY OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, 
 * TITLE OR NON-INFRINGEMENT, AND SPECIFICALLY DISCLAIMS SUCH WARRANTIES 
 * AND REPRESENTATIONS.  IN NO EVENT WILL CONEXANT BE LIABLE FOR ANY DAMAGES
 * ARISING FROM THE USE OF THE INFORMATION CONTAINED IN THIS FILE.
 *
 * Copyright (C) 2009-2012 Conexant Systems, Inc. 
 * All rights reserved. 
 * 
 *---------------------------------------------------------------------------- 
 * 
 * File Name: dc_decompress_test.c
 * 
 * Description: Application that tests the libjpeg API
 * 
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>                
#include <stdlib.h>                
#include <unistd.h>                
#include <errno.h>                
#include <malloc.h>
#include <fcntl.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "ospl.h"                
#include "dc_type.h" 
#include "dc_return_codes.h"
#include "dcapi.h"
#define UINT8_ALREADY_A_TYPEDEF
#define UINT16_ALREADY_A_TYPEDEF
#define INT16_ALREADY_A_TYPEDEF
#define INT32_ALREADY_A_TYPEDEF
#define HAVE_BOOLEAN
#include "jpeglib.h"
#undef HAVE_BOOLEAN
#undef INT32_ALREADY_A_TYPEDEF
#undef INT16_ALREADY_A_TYPEDEF
#undef UINT16_ALREADY_A_TYPEDEF
#undef UINT8_ALREADY_A_TYPEDEF
#include "dc_decompress_test.h"

// Function prototypes
int main(int argc, char* argv[]);
int parseCommandLine(int argc,char *argv[]);
boolean initializeFrameBufferDevice(void);
void teardownResources(void);
UINT32 thread_priority_get(DC_THREAD_PRIORITY priority);
void libjpeg_error_handler(j_common_ptr cinfo);
char *get_color_space(J_COLOR_SPACE color_space);
void report_decompress_details(void);
int handle_dump_to_file(void);
int handle_image_loop(void);
unsigned long int get_time_in_ms(void);
void calculate_render_dimensions(void);
void calculate_render_mappings(void);
void render_scanline_to_framebuffer(void);
void fb_clear(FB_DESC *fb);
void fb_flip(FB_DESC *fb);
void fb_swap(void);
void dump_proc_file(char *proc_filename);

// Command line option flags
boolean verboseConsoleOutput = FALSE;
int loopCount = 1;
int minimumTimeToDisplay = 1000;
char imageFileNameArray[MAX_IMAGE_FILE_COUNT][MAX_IMAGE_FILE_LEN];
int imageFileCount = 0;
DCDT_MODE dcdt_mode = DCDT_MODE_SCREEN_OUTPUT;
J_COLOR_SPACE output_format = JCS_RGB;
int scale_n = 1;
int scale_m = 1;
boolean allowImageUpscale = FALSE;
int line_request = 1;
boolean show_proc_stats = FALSE;

// Global data
RESOURCE_TEARDOWN_STATUS teardownStatus,*RTS_p = &teardownStatus;
int fb_fd;
struct fb_var_screeninfo fb_var_info;
struct fb_fix_screeninfo fb_fix_info;
int fb_width;
int fb_virtual_width;
int fb_height;
int fb_virtual_height;
int fb_bpp;
int fb_stride;
UINT32 fb_base;
UINT32 fb_size;
FB_DESC fb1;
FB_DESC fb2;
FB_DESC *front_buffer;
FB_DESC *back_buffer;
UINT8 line_buffer[MAX_LINE_WIDTH_BYTES];
UINT8 *line_buffer_ptr = line_buffer;
struct jpeg_decompress_struct decomp_struct,*ds_ptr = &decomp_struct;
char output_filename[] = "dc_decompress_test.raw";
FILE *output_handle;
ERROR_HANDLER_DESC ehd;
FILE *image_handle;
char *current_filename;
int currentImageFileIndex = 0;
UINT32 currentPass = 0;
int render_width;
int render_height;
int render_offset_x;
int render_offset_y;
int render_row_map[MAX_FB_DIMENSION];
int render_column_map[MAX_FB_DIMENSION];
int render_scanline;

int main(int argc,char **argv)
{
	int status;
	int i;

	printf("test1\n");
 
	if (handle_image_loop())
	{
		exit(-1);
	}


	exit(0);
}

int parseCommandLine(int argc,char *argv[])
{
    char arg[256];
    int i,j;
	char sentinel;
    boolean verboseConsoleOutputSpecified = FALSE;
	boolean loopCountSpecified = FALSE;
	boolean minimumTimeToDisplaySpecified = FALSE;
	boolean modeSpecified = FALSE;
	boolean ofmtSpecified = FALSE;
	boolean scaleSpecified = FALSE;
	boolean allowImageUpscaleSpecified = FALSE;
	boolean lineRequestSpecified = FALSE;
	boolean showProcStatsSpecified = FALSE;

	// Scan parameter list for options first.

    for (i=1;i<argc;i++)
    {
        strncpy(arg,argv[i],256);

        if (arg[0] != '-') continue;

		//printf("arg: %s\n",arg);
        
        for (j=0;j<strlen(arg);j++) arg[j] = toupper(arg[j]);

        if (((arg[1] == 'H') && (arg[2] == 0)) ||
			((arg[1] == 'H') && (arg[2] == 'E') && (arg[3] == 'L') && (arg[4] == 'P') && (arg[5] == 0)) ||
			((arg[1] == '-') && (arg[2] == 'H') && (arg[3] == 'E') && (arg[4] == 'L') && (arg[5] == 'P') &&
																								(arg[6] == 0)))
        {
			return(-2);
        }

        if ((arg[1] == 'V') && (arg[2] == 0))
        {
            if (verboseConsoleOutputSpecified == TRUE) DUPLICATE_OPTION_ERROR("-v");
            verboseConsoleOutputSpecified = TRUE;
			verboseConsoleOutput = TRUE;
            continue;
        }

        if ((arg[1] == 'A') && (arg[2] == 'I') && (arg[3] == 'U') && (arg[4] == 0))
        {
            if (allowImageUpscaleSpecified == TRUE) DUPLICATE_OPTION_ERROR("-aiu");
            allowImageUpscaleSpecified = TRUE;
			allowImageUpscale = TRUE;
            continue;
        }

        if ((arg[1] == 'L') && (arg[2] == 'O') && (arg[3] == 'O') && (arg[4] == 'P') && (arg[5] == ':'))
        {
            if (loopCountSpecified == TRUE) DUPLICATE_OPTION_ERROR("-loop");
            loopCountSpecified = TRUE;
			if (sscanf(&(arg[6]),"%i%c",(int *)&loopCount,&sentinel) != 1) INVALID_PARAMETER_ERROR("-loop");
			if (loopCount < 0) INVALID_PARAMETER_ERROR("-loop");
            continue;
        }

        if ((arg[1] == 'M') && (arg[2] == 'T') && (arg[3] == 'T') && (arg[4] == 'D') && (arg[5] == ':'))
        {
            if (minimumTimeToDisplaySpecified == TRUE) DUPLICATE_OPTION_ERROR("-mttd");
            minimumTimeToDisplaySpecified = TRUE;
			if (sscanf(&(arg[6]),"%i%c",(int *)&minimumTimeToDisplay,&sentinel) != 1) INVALID_PARAMETER_ERROR("-mttd");
			if (minimumTimeToDisplay < 0) INVALID_PARAMETER_ERROR("-mttd");
            continue;
        }

        if ((arg[1] == 'M') && (arg[2] == 'O') && (arg[3] == 'D') && (arg[4] == 'E') && (arg[5] == ':'))
        {
			int temp;

			if (modeSpecified == TRUE) DUPLICATE_OPTION_ERROR("-mode");
            modeSpecified = TRUE;
			if (sscanf(&(arg[6]),"%i%c",(int *)&temp,&sentinel) != 1) INVALID_PARAMETER_ERROR("-mode");
			if (((DCDT_MODE) temp < DCDT_MODE_DUMP_TO_FILE) || 
				((DCDT_MODE) temp > DCDT_MODE_SCREEN_OUTPUT))
			{
				INVALID_PARAMETER_ERROR("-mode");
			}
			dcdt_mode = (DCDT_MODE) temp;
			continue;
        }

        if ((arg[1] == 'O') && (arg[2] == 'F') && (arg[3] == 'M') && (arg[4] == 'T') && (arg[5] == ':'))
        {
			int temp;

			if (ofmtSpecified == TRUE) DUPLICATE_OPTION_ERROR("-ofmt");
            ofmtSpecified = TRUE;
			if (sscanf(&(arg[6]),"%i%c",(int *)&temp,&sentinel) != 1) INVALID_PARAMETER_ERROR("-ofmt");
			if (((J_COLOR_SPACE) temp != JCS_GRAYSCALE) &&
				((J_COLOR_SPACE) temp != JCS_RGB) &&
				((J_COLOR_SPACE) temp != JCS_YCbCr))
			{
				INVALID_PARAMETER_ERROR("-fmt");
			}
			output_format = (J_COLOR_SPACE) temp;
			continue;
        }

        if ((arg[1] == 'S') && (arg[2] == 'C') && (arg[3] == 'A') && (arg[4] == 'L') && (arg[5] == 'E') &&
																							(arg[6] == ':'))
		{
			if (scaleSpecified == TRUE) DUPLICATE_OPTION_ERROR("-scale");
            scaleSpecified = TRUE;
			if (sscanf(&(arg[7]),"%i/%i%c",(int *)&scale_n,&scale_m,&sentinel) != 2) INVALID_PARAMETER_ERROR("-scale");
			continue;
		}

        if ((arg[1] == 'L') && (arg[2] == 'R') && (arg[3] == ':'))
		{
			if (lineRequestSpecified == TRUE) DUPLICATE_OPTION_ERROR("-lr");
            lineRequestSpecified = TRUE;
			if (sscanf(&(arg[4]),"%i%c",(int *)&line_request,&sentinel) != 1) INVALID_PARAMETER_ERROR("-lr");
			continue;
		}

        if ((arg[1] == 'S') && (arg[2] == 'P') && (arg[3] == 'S') && (arg[4] == 0))
		{
			if (showProcStatsSpecified == TRUE) DUPLICATE_OPTION_ERROR("-sps");
            showProcStatsSpecified = TRUE;
			show_proc_stats = TRUE;
			continue;
		}

		printf("\nParse Error: Option \"%s\" is unknown!\n\n",argv[i]);
		return(-1);
    }

	// Scan parameter list for files now.
	
    for (i=1;i<argc;i++)
    {
        strncpy(arg,argv[i],256);
        
		if (arg[0] == '-') continue;

		//printf("arg: %s\n",arg);
        
		if (strlen(arg) > (MAX_IMAGE_FILE_LEN-1))
		{
			printf("\nParse Error: Image file name greater than %d characters!\n",MAX_IMAGE_FILE_LEN-1);
			printf("\n%s\n\n",arg);
			return(-1);
		}

		strncpy(imageFileNameArray[imageFileCount++],arg,MAX_IMAGE_FILE_LEN-1);

		if (imageFileCount >= MAX_IMAGE_FILE_COUNT)
		{
			printf("Capping file count at MAX_IMAGE_FILE_COUNT: %d!\n",MAX_IMAGE_FILE_COUNT);
			break;
		}
	}

	if (imageFileCount == 0)
	{
		printf("\nParse Error: No image files specified!\n\n");
		return(-1);
	}

	if ((imageFileCount > 1) && (dcdt_mode == DCDT_MODE_DUMP_TO_FILE))
	{
		printf("\nParse Error: Too many image files specified for mode!\n\n");
		return(-1);
	}

	// Force mode 2 output format to RGB24
	
	if (dcdt_mode == DCDT_MODE_SCREEN_OUTPUT) output_format = JCS_RGB;

	// Force mode 2 line request to 1
	
	if (dcdt_mode == DCDT_MODE_SCREEN_OUTPUT) line_request = 1;

    return(0);    
}

boolean initializeFrameBufferDevice(void)
{
	if (verboseConsoleOutput == TRUE) printf("\nInitialize frame buffer device\n");
#if defined(ANDROID)
    fb_fd = open("/dev/graphics/fb0",O_RDWR);
    if (!fb_fd) 
    {
        printf("open(/dev/graphics/fb0) error!\n");
        return(FALSE);
    }
#else
    fb_fd = open("/dev/fb0",O_RDWR);
    if (!fb_fd) 
    {
        printf("open(/dev/fb0) error!\n");
        return(FALSE);
    }
#endif
    RTS_SET_BIT(RTS_p,RTS_FB_OPENED);

    if (ioctl(fb_fd,FBIOGET_FSCREENINFO,&fb_fix_info))
    {
        printf("ioctl(fb_fd,FBIOGET_FSCREENINFO) error!\n");
        return(FALSE);
    }
	
    if (ioctl(fb_fd,FBIOGET_VSCREENINFO,&fb_var_info)) 
    {
        printf("ioctl(fb_fd,FBIOGET_VSCREENINFO) error!\n");
        return(FALSE);
    }
	fb_width = fb_var_info.xres;
	if (verboseConsoleOutput == TRUE) printf("  fb_width: %#x (%d)\n",fb_width,fb_width);
	fb_height = fb_var_info.yres;
	if (verboseConsoleOutput == TRUE) printf("  fb_height: %#x (%d)\n",fb_height,fb_height);
	fb_virtual_width = fb_var_info.xres_virtual;
	if (verboseConsoleOutput == TRUE) printf("  fb_virtual_width: %#x (%d)\n",fb_virtual_width,fb_virtual_width);
	fb_virtual_height = fb_var_info.yres_virtual;
	if (verboseConsoleOutput == TRUE) printf("  fb_virtual_height: %#x (%d)\n",fb_virtual_height,fb_virtual_height);
	fb_bpp = fb_var_info.bits_per_pixel / 8;
	if (verboseConsoleOutput == TRUE) printf("  fb_bpp: %d\n",fb_bpp);
	fb_stride = fb_fix_info.line_length;
	if (verboseConsoleOutput == TRUE) printf("  fb_stride: %#x (%d)\n",fb_stride,fb_stride);
    fb_size = fb_fix_info.smem_len;
    if (verboseConsoleOutput == TRUE) printf("  fb_size: %#08lx\n",(unsigned long) fb_size);

	if ((fb_width > MAX_FB_DIMENSION) || (fb_height > MAX_FB_DIMENSION))
	{
		printf("fb_width and/or fb_height greater than %d!\n",MAX_FB_DIMENSION);
		return(FALSE);
	}

    fb_base = (UINT32) mmap(0,fb_size,PROT_READ|PROT_WRITE,MAP_SHARED,fb_fd,0);
    if (fb_base == -1) 
    {
        printf("mmap(fb_fd) error!\n");
        return(FALSE);
    }
	RTS_SET_BIT(RTS_p,RTS_FB_MAPPED);
    if (verboseConsoleOutput == TRUE) printf("  fb_base: %#08lx\n",(unsigned long) fb_base);

	// Set up 2 framebuffer descriptors and act like we always double buffer, even if we don't

	fb1.base = fb_base;
	fb1.yoffset = 0;
	if (fb_virtual_height >= (2 * fb_height))
	{
		fb2.base = fb_base + (fb_height * fb_stride);
		fb2.yoffset = fb_height;
	}
	else
	{
		fb2.base = fb_base;
		fb2.yoffset = 0;
	}
    if (verboseConsoleOutput == TRUE) printf("  fb1.base: %#08lx\n",(unsigned long) fb1.base);
    if (verboseConsoleOutput == TRUE) printf("  fb2.base: %#08lx\n",(unsigned long) fb2.base);

	// Set up front and back buffers
	
	front_buffer = &fb1;
	back_buffer = &fb2;

	// Make front buffer the displayed buffer
	
	fb_clear(front_buffer);
	fb_flip(front_buffer);

	return(TRUE);
}

void teardownResources(void)
{
	int status;

    if (verboseConsoleOutput == TRUE) printf("teardownResources (Entry): 0x%08lX\n",(long unsigned int) *RTS_p);
   
    if (RTS_BIT_IS_SET(RTS_p,RTS_DECOMP_STRUCT_INIT))
    {
   		if (verboseConsoleOutput == TRUE) printf("jpeg_abort_decompress: %#lx\n",(long unsigned int) ospl_get_time());
    	jpeg_abort_decompress(ds_ptr);
   		if (verboseConsoleOutput == TRUE) printf("jpeg_destroy_decompress: %#lx\n",(long unsigned int) ospl_get_time());
    	jpeg_destroy_decompress(ds_ptr);
        RTS_CLEAR_BIT(RTS_p,RTS_DECOMP_STRUCT_INIT);
	}

    if (RTS_BIT_IS_SET(RTS_p,RTS_IMAGE_FILE_OPEN))
    {
   		if (verboseConsoleOutput == TRUE) printf("fclose(image_file): %#lx\n",(long unsigned int) ospl_get_time());
    	status = fclose(image_handle);
        if (status != 0)
        {
            printf("fclose(image_file) error: %#x\n",status);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_IMAGE_FILE_OPEN);
        }
	}

    if (RTS_BIT_IS_SET(RTS_p,RTS_OUTPUT_FILE_OPEN))
    {
   		if (verboseConsoleOutput == TRUE) printf("fclose(output_file): %#lx\n",(long unsigned int) ospl_get_time());
    	status = fclose(output_handle);
        if (status != 0)
        {
            printf("fclose(output_file) error: %#x\n",status);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_OUTPUT_FILE_OPEN);
        }
	}

    if (RTS_BIT_IS_SET(RTS_p,RTS_FB_MAPPED))
    {
   		if (verboseConsoleOutput == TRUE) printf("munmap(fb_base): %#lx\n",(long unsigned int) ospl_get_time());
    	status = munmap((void *) fb_base,fb_size);
        if (status != 0)
        {
            printf("munmap(fb_base) error: %#x\n",status);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_FB_MAPPED);
        }
	}

    if (RTS_BIT_IS_SET(RTS_p,RTS_FB_OPENED))
    {
   		if (verboseConsoleOutput == TRUE) printf("close(fb_fd): %#lx\n",(long unsigned int) ospl_get_time());
    	status = close(fb_fd);
        if (status != 0)
        {
            printf("close(fb_fd) error: %#x\n",status);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_FB_OPENED);
        }
	}

   	if (verboseConsoleOutput == TRUE) printf("Done: %#lx\n",(long unsigned int) ospl_get_time());
    if (verboseConsoleOutput == TRUE) printf("teardownResources (Exit): 0x%08lX\n",(long unsigned int) *RTS_p);
}

UINT32 thread_priority_get(DC_THREAD_PRIORITY priority)
{
	return(priority);
}

void libjpeg_error_handler(j_common_ptr cinfo)
{
    ERROR_HANDLER_DESC *ehd_ptr = (ERROR_HANDLER_DESC *) cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(ehd_ptr->setjmp_buffer,1);			// Returns control to setjmp() call
}

char *get_color_space(J_COLOR_SPACE color_space)
{
	static char Unknown[] = "Unknown";
	static char Grayscale[] = "Grayscale";
	static char RGB24[] = "RGB24";
	static char YCbCr[] = "YCbCr";
	static char CMYK[] = "CMYK";
	static char YCbCrK[] = "YCbCrK";
	char *str_ptr;

	switch (color_space)
	{
	case JCS_GRAYSCALE:
		str_ptr = Grayscale;
		break;
	case JCS_RGB:
		str_ptr = RGB24;
		break;
	case JCS_YCbCr:
		str_ptr = YCbCr;
		break;
	case JCS_CMYK:
		str_ptr = CMYK;
		break;
	case JCS_YCCK:
		str_ptr = YCbCrK;
		break;
	}

	return(str_ptr);
}

void report_decompress_details(void)
{
	printf("\nImage (pass %d, index: %d): %s, %d x %d (%0.4f), %s, %d components\n",
							currentPass,currentImageFileIndex,current_filename,
							ds_ptr->image_width,ds_ptr->image_height,
							((float) ds_ptr->image_width/(float) ds_ptr->image_height),
							get_color_space(ds_ptr->jpeg_color_space),
							ds_ptr->num_components);
	printf("Output: %d x %d, %s, %d components\n",ds_ptr->output_width,ds_ptr->output_height,
							get_color_space(ds_ptr->out_color_space),ds_ptr->output_components);
	printf("Scaling: %0.4f requested, ",(float) ds_ptr->scale_num/(float) ds_ptr->scale_denom);
	printf("%0.4f effective\n",(float) ds_ptr->output_width/(float) ds_ptr->image_width);
    if (dcdt_mode == DCDT_MODE_SCREEN_OUTPUT) 
	{
		 printf("Render: %d x %d at %d,%d\n",
						render_width,render_height,render_offset_x,render_offset_y);
	}
}

int handle_dump_to_file(void)
{
	int status;
	printf("xxxxxxxxxxxxxxxxx\n");
	// Open the output file

	if ((output_handle = fopen(output_filename,"wb")) <= 0)
	{
		printf("fopen(%s) error\n",output_filename);
		return(-1);
	}
	RTS_SET_BIT(RTS_p,RTS_OUTPUT_FILE_OPEN);
	if (verboseConsoleOutput == TRUE) printf("Created %s\n\n",output_filename);

	// Open the (only) image file

	current_filename = imageFileNameArray[0];
	image_handle = fopen(current_filename,"rb");
	if (!image_handle)
	{
		printf("\nfopen error of: %s\n",current_filename);
		teardownResources();
		return(-1);
	}
	RTS_SET_BIT(RTS_p,RTS_IMAGE_FILE_OPEN);

	// Initialize the libjpeg decompress struct

	jpeg_create_decompress(ds_ptr);
	RTS_SET_BIT(RTS_p,RTS_DECOMP_STRUCT_INIT);

	// Set up the input file source

	jpeg_stdio_src(ds_ptr,image_handle);

	// Read image file metadata

	jpeg_read_header(ds_ptr,TRUE);

	// Set requested output format and scaling

	ds_ptr->out_color_space = output_format;
	ds_ptr->scale_num = scale_n;
	ds_ptr->scale_denom = scale_m;
	
	printf("scale n m :%d  %d \n",scale_n,scale_m);
	// Get calculated output dimensions

	jpeg_calc_output_dimensions(ds_ptr);

	// Start decompress

	jpeg_start_decompress(ds_ptr);

	// Report the decompress details

	report_decompress_details();
	printf("\n");

	// Make sure our line buffer is big enough
	
	if ((ds_ptr->output_components * ds_ptr->output_width) > MAX_LINE_WIDTH_BYTES)
	{
		int line_length = ds_ptr->output_components * ds_ptr->output_width;

		printf("Output scanline byte length * line request ");
		printf("(%d * %d = %d) is larger than line buffer size (%d)!\n",
						line_length,line_request,line_length*line_request,MAX_LINE_WIDTH_BYTES);
		jpeg_abort_decompress(ds_ptr);
		return(-1);
	}

	// Process scanlines

	while (ds_ptr->output_scanline < ds_ptr->output_height)
	{
		int lines_returned;

		lines_returned = jpeg_read_scanlines(ds_ptr,&line_buffer_ptr,line_request);
		if (lines_returned)
		{
			status = fwrite(line_buffer,ds_ptr->output_components,ds_ptr->output_width*lines_returned,output_handle);
			if (status != (ds_ptr->output_width * lines_returned))
			{
				printf("\nfwrite error!\n");
				jpeg_abort_decompress(ds_ptr);
				return(-1);
			}
		}
	}

	// Close up the decompress methods

	jpeg_finish_decompress(ds_ptr);

	// Deactivate the decompress struct

	jpeg_destroy_decompress(ds_ptr);
	RTS_CLEAR_BIT(RTS_p,RTS_DECOMP_STRUCT_INIT);

	// Close image file
	
	fclose(image_handle);
	RTS_CLEAR_BIT(RTS_p,RTS_IMAGE_FILE_OPEN);

	// Close output file
	
	fclose(output_handle);
	RTS_CLEAR_BIT(RTS_p,RTS_OUTPUT_FILE_OPEN);

	return(0);
}

int handle_image_loop(void)
{
	unsigned long int start_time;
	unsigned long int end_time;
	unsigned long int total_ms;

	ds_ptr->err = iptv_jpeg_std_error(&(ehd.pub));
	ehd.pub.error_exit = libjpeg_error_handler;
	if (setjmp(ehd.setjmp_buffer) == 1)
	{
		printf("\nError from libjpeg, exiting!\n\n");
		exit(-1);
	}

	image_handle = fopen("test.jpg","rb");
	if (!image_handle)
	{
		printf("\nfopen error of: %s\n",current_filename);
		return(-1);
	}

	total_ms = 0;
	start_time = get_time_in_ms();
printf("test2\n");

	jpeg_create_decompress(ds_ptr);

	printf("test21\n");

	jpeg_stdio_src(ds_ptr,image_handle);

	printf("test22\n");

	jpeg_read_header(ds_ptr,TRUE);

printf("test23\n");	
	scale_n = ds_ptr->image_width;
	scale_m = ds_ptr->image_width;


	ds_ptr->out_color_space = output_format;
	ds_ptr->scale_num = 1024;//scale_n;
	ds_ptr->scale_denom = scale_m;
	printf("scale_num %d   scale_denom  %d \n",scale_n,scale_m);

	jpeg_calc_output_dimensions(ds_ptr);

	jpeg_start_decompress(ds_ptr);

	if ((ds_ptr->output_components * ds_ptr->output_width) > MAX_LINE_WIDTH_BYTES)
	{
		int line_length = ds_ptr->output_components * ds_ptr->output_width;

		printf("Output scanline byte length * line request ");
		printf("(%d * %d = %d) is larger than line buffer size (%d)!\n",
						line_length,line_request,line_length*line_request,MAX_LINE_WIDTH_BYTES);
		jpeg_abort_decompress(ds_ptr);
		return(-1);
	}



	while (ds_ptr->output_scanline < ds_ptr->output_height)
	{
		int lines_returned;

		lines_returned = jpeg_read_scanlines(ds_ptr,&line_buffer_ptr,line_request);

	}


	end_time = get_time_in_ms();
	total_ms += (end_time - start_time);
	printf("Total time: %ld ms\n\n",total_ms);


	jpeg_finish_decompress(ds_ptr);
	jpeg_destroy_decompress(ds_ptr);

	fclose(image_handle);


	printf("\n");

	return(0);
}

unsigned long int get_time_in_ms(void)
{
	struct timeval tv;

	(void) gettimeofday(&tv,NULL);

	return((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void calculate_render_dimensions(void)
{
	int wNorm;
	int hNorm;

	// Adjust the render dimensions to fit in the LCD framebuffer
	
	if ((allowImageUpscale == TRUE) ||
		((ds_ptr->image_width >= fb_width) || (ds_ptr->image_height >= fb_height)))
	{
		wNorm = ds_ptr->image_width * fb_height;
    	hNorm = ds_ptr->image_height * fb_width;

    	if (wNorm == hNorm)
    	{
        	render_width = fb_width;
			render_height = fb_height;
    	}
    	else if (wNorm < hNorm)
    	{
        	render_width = (ds_ptr->image_width * fb_height) / ds_ptr->image_height;
        	render_height = fb_height;
    	}
    	else
    	{
        	render_width = fb_width;
        	render_height = (ds_ptr->image_height * fb_width) / ds_ptr->image_width;
    	}
    }
    else
    {
        // Source image size is less than frame size -- render it
        // with its original dimensions

        render_width = ds_ptr->image_width;
        render_height = ds_ptr->image_height;
    }

	// Ensure there is at least one pixel in width or one line in height to render
	
	if (render_width == 0) render_width = 1;
	if (render_height == 0) render_height = 1;

	// Center the image in the frame

	render_offset_x = (fb_width - render_width) / 2;	
	render_offset_y = (fb_height - render_height) / 2;

}

void calculate_render_mappings(void)
{
	int i;

	for (i=0;i<render_width;i++)
	{
		render_row_map[i] = (i * ds_ptr->output_width) / render_width;
	}

	for (i=0;i<render_height;i++)
	{
		render_column_map[i] = (i * ds_ptr->output_height) / render_height;
	}

}

void render_scanline_to_framebuffer(void)
{
	int in_line_buffer = ds_ptr->output_scanline - 1;
	int scanline_wanted;
	UINT16 *fb16_ptr;
	UINT32 *fb32_ptr;
	UINT8 *lb_ptr; 
	DWORD_UNION dwd_union = {0};
	UINT8 red;
	UINT8 green;
	UINT8 blue;
	int i;

	while (1)
	{
		if (render_scanline >= render_height) break;

		scanline_wanted = render_column_map[render_scanline];

		if (in_line_buffer < scanline_wanted) break;
		if (in_line_buffer > scanline_wanted)
		{
			printf("\nInternal error, missed a scanline!\n");
			printf("render_scanline: %d, in_line_buffer: %d, scanline_wanted: %d\n",
										render_scanline,in_line_buffer,scanline_wanted);
			teardownResources();
			exit(-1);
		}

		// Line buffer is holding the scanline that we need to convert to
		// current framebuffer line.  Line buffer holds RGB24 pixels, framebuffer
		// needs either BGR32 or BGR16 pixels.

		if (fb_bpp == 2)
		{
			fb16_ptr = (UINT16 *) (back_buffer->base + ((render_offset_y + render_scanline) * fb_stride) +
																			(render_offset_x * 2));
			for (i=0;i<render_width;i++)
			{
				lb_ptr = line_buffer;
				lb_ptr += 3 * render_row_map[i];
				red = *lb_ptr++;
				green = *lb_ptr++;
				blue = *lb_ptr++;
				*fb16_ptr++ = (UINT16) (((red << 8) & 0xF800) | ((green << 3) & 0x07E0) | ((blue >> 3) & 0x001F));
			}
		}
		else
		{
			fb32_ptr = (UINT32 *) (back_buffer->base + ((render_offset_y + render_scanline) * fb_stride) +
																			(render_offset_x * 4));
			for (i=0;i<render_width;i++)
			{
				lb_ptr = line_buffer;
				lb_ptr += 3 * render_row_map[i];
				dwd_union.b[2] = *lb_ptr++;
				dwd_union.b[1] = *lb_ptr++;
				dwd_union.b[0] = *lb_ptr++;
				*fb32_ptr++ = dwd_union.dword;
			}
		}

		++render_scanline;
	}

	return;
}

void fb_clear(FB_DESC *fb)
{
	memset((void *)fb->base,0x00,fb_height * fb_stride);
}

void fb_flip(FB_DESC *fb)
{
	fb_var_info.yoffset = fb->yoffset;
    if (ioctl(fb_fd,FBIOPAN_DISPLAY,&fb_var_info)) 
    {
        printf("\nioctl(fb_fd,FBIOPAN_DISPLAY) error!\n");
        teardownResources();
		exit(-1);
    }
}

void fb_swap(void)
{
	FB_DESC *temp;

	temp = front_buffer;
	front_buffer = back_buffer;
	back_buffer = temp;
}

void dump_proc_file(char *proc_filename)
{
	FILE *fd;
	char buffer[8192];
	int bytes_read;

	printf("\nDumping %s:\n",proc_filename);

	fd = fopen(proc_filename,"r");
	if (fd <= 0)
	{
		printf("Error opening %s!\n",proc_filename);
		return;
	}

	bytes_read = fread(buffer,1,8192,fd);
	fclose(fd);
	if (bytes_read <= 0)
	{
		printf("Error reading %s!\n",proc_filename);
		return;
	}

	fwrite(buffer,1,bytes_read,stdout);

	printf("\n");

}

