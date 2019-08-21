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
 * File Name: dc_accel.c
 * 
 * Description: Conexant proprietary extension to libjpeg library for HW
 * 				acceleration of JPEG decompression
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include "ospl.h"                
#include "dc_type.h" 
#include "dc_return_codes.h"
#include "dc_cm_api.h"
#include "dc_rammgr_api.h"
#include "dc_conip_api.h"
#include "dc_conro_api.h"
#include "dc_conjp_api.h"
#include "dc_conjh_api.h"
#include "dc_csc_api.h"
#include "dc_dt_engine_api.h"
#include "dc_dmbimg_api.h"
#include "dc_imgbuf_api.h"
#include "dc_container_api.h"
#include "dc_jpeg_api.h"
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
#include "jerror.h"
#include "dc_accel_queue.h"
#include "dc_accel_api.h"
#include "dc_accel.h"

// Local function prototypes

static void dumpSystemConfig(DC_ACCEL_DESC *ds_ptr);
static DC_ERROR_CODE csc_rammgr_callback(DC_RAMMGR_BUFFER_ENTRY *entry);
static DC_RETURN_CODE csc_library_setup(DC_ACCEL_DESC *dad_ptr);
static DC_RETURN_CODE dc_context_managers_setup(void);
static DC_RETURN_CODE dc_context_managers_init(void);
static DC_RETURN_CODE setup_sdk_csc_cm(DC_ACCEL_DESC *dad_ptr);
static DC_RETURN_CODE init_sdk_csc_cm(DC_ACCEL_DESC *dad_ptr);
static DC_ERROR_CODE jpegpipe_rammgr_callback(DC_RAMMGR_BUFFER_ENTRY *entry);
static DC_ERROR_CODE imgbuf_rammgr_callback(DC_RAMMGR_BUFFER_ENTRY *entry);
static DC_RETURN_CODE dmb_manager_init(DC_RAMMGR_MEMORY_STATUS *status);
static boolean setup_imgbuf(DC_ACCEL_DESC *dad_ptr);
static boolean setup_jpegpipe_csc_trc(DC_ACCEL_DESC *dad_ptr);
static void dc_accel_jpegpipe_callback(void *arg,DC_JPEGPIPE_RESULTS_t *results);
static void analyze_dmbimg(DC_ACCEL_DESC *dad_ptr);

//----------------------------------------------------------------------------------------------------
// API functions below this line - prototypes in dc_accel_api.h
//----------------------------------------------------------------------------------------------------

void dc_accel_init(struct jpeg_decompress_struct *ds_ptr)
{
	DC_ACCEL_DESC *dad_ptr;

	DC_ACCEL_LOG("Initializing DC_ACCELERATION structures\n");

	DC_ACCEL_LOG("Allocate the DC_ACCEL_DESC (%d bytes) and tie it to jpeg_decompress_struct\n",
																			sizeof(DC_ACCEL_DESC));

	dad_ptr = (DC_ACCEL_DESC *)(*ds_ptr->mem->alloc_large)
										((j_common_ptr) ds_ptr,JPOOL_PERMANENT,sizeof(DC_ACCEL_DESC));

	DC_ACCEL_LOG("DC_ACCEL_DESC allocated at: %#x\n",dad_ptr);

	ds_ptr->dc_accel_desc_ptr = (void *) dad_ptr;

	memset(dad_ptr,0,sizeof(DC_ACCEL_DESC));

	dad_ptr->signature = DC_ACCEL_DESC_SIGNATURE;
	dad_ptr->accelerate_decompress_job = FALSE;
	dad_ptr->state = DC_ACCEL_STATE_INIT;
	dad_ptr->shadow_stream.errorEncountered = FALSE;
	dad_ptr->shadow_stream.ssmd_q.count = 0;
	dad_ptr->shadow_stream.ssmd_q.first = NULL;
	dad_ptr->shadow_stream.ssmd_q.last = NULL;
	dad_ptr->shadow_stream.total_byte_count = 0;

    RTS_INIT(&(dad_ptr->RTS)); 					// Clean resource teardown status
}

void dc_accel_consider_job(struct jpeg_decompress_struct *ds_ptr)
{
	DC_ACCEL_DESC *dad_ptr;

	// DC_ACCEL_DESC validity checks
	
	if (ds_ptr->dc_accel_desc_ptr == NULL)
	{
		ERREXIT(ds_ptr,JERR_DC_ACCEL_DESC_PTR_NULL);
	}

	dad_ptr = (DC_ACCEL_DESC *) ds_ptr->dc_accel_desc_ptr;

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_DESC_SIGNATURE);
	}

	if (dad_ptr->state == DC_ACCEL_STATE_UNKNOWN)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_STATE);
	}

	// Have we already been here?  If yes, then all info should still be valid
	
	if (dad_ptr->state >= DC_ACCEL_STATE_CONSIDER)
	{
		DC_ACCEL_LOG("This job has already been considered and will %sbe accelerated\n",
								(dad_ptr->accelerate_decompress_job == FALSE) ? "NOT " : "");
		return;
	}
	else
	{
		DC_ACCEL_LOG("Considering whether job can be accelerated\n");
	}

	dad_ptr->accelerate_decompress_job = FALSE;

	// Check various job parameters
	
	if (getenv("DISABLE_DC_ACCEL"))
	{
		DC_ACCEL_LOG("DISABLE_DC_ACCEL is set!\n");
	}
	else if (dad_ptr->shadow_stream.errorEncountered == TRUE)
	{
		DC_ACCEL_LOG("Shadow stream had errors!\n");
	}
	else if ((ds_ptr->jpeg_color_space != JCS_YCbCr) &&
			 (ds_ptr->jpeg_color_space != JCS_GRAYSCALE))
	{
		DC_ACCEL_LOG("Unsupported jpeg_color_space: %d!\n",(int) ds_ptr->jpeg_color_space);
	}
	else if ((ds_ptr->num_components != 3) &&
			 (ds_ptr->num_components != 1))
	{
		DC_ACCEL_LOG("Invalid num_components: %d!\n",ds_ptr->num_components);
	}
	else if (ds_ptr->comps_in_scan < ds_ptr->num_components)
	{
		DC_ACCEL_LOG("comps_in_scan < num_components!\n");
	}
	else if ((ds_ptr->out_color_space != JCS_RGB) &&
			 (ds_ptr->out_color_space != JCS_GRAYSCALE) &&
			 (ds_ptr->out_color_space != JCS_YCbCr))
	{
		DC_ACCEL_LOG("Unsupported jpeg_color_space: %d!\n",(int) ds_ptr->out_color_space);
	}
	else if ((ds_ptr->jpeg_color_space == JCS_GRAYSCALE) &&
			 (ds_ptr->out_color_space == JCS_YCbCr))
	{
		DC_ACCEL_LOG("Conversion from GRAYSCALE to YCbCr unsupported!\n");
	}
	else if (ds_ptr->output_gamma != 1.0)
	{
		DC_ACCEL_LOG("output_gamma NOT 1.0!\n");
	}
	else if (ds_ptr->buffered_image != FALSE)
	{
		DC_ACCEL_LOG("buffered_image NOT FALSE!\n");
	}
	else if (ds_ptr->raw_data_out != FALSE)
	{
		DC_ACCEL_LOG("raw_data_out NOT FALSE!\n");
	}
	else if (ds_ptr->quantize_colors != FALSE)
	{
		DC_ACCEL_LOG("quantize_colors NOT FALSE!\n");
	}
	else if (ds_ptr->data_precision != 8)
	{
		DC_ACCEL_LOG("data_precision NOT 8!\n");
	}
	else if (ds_ptr->progressive_mode != FALSE)
	{
		DC_ACCEL_LOG("progressive_mode NOT FALSE!\n");
	}
	else if (ds_ptr->arith_code != FALSE)
	{
		DC_ACCEL_LOG("arith_code NOT FALSE!\n");
	}
	else if (ds_ptr->CCIR601_sampling != FALSE)
	{
		DC_ACCEL_LOG("CCIR601_sampling NOT FALSE!\n");
	}
	else if ((ds_ptr->output_width < 256) && (ds_ptr->output_height < 256))
	{
		DC_ACCEL_LOG("Output dimensions less than 256x256, let SW do it!\n");
	}
	else
	{
		dad_ptr->accelerate_decompress_job = TRUE;
	}

	// TODO: Update output_<width,height> based on actual values of
	// ds_ptr->scale_<num,denom>, instead of the libjpeg scale limits
	// of: 1/1, 1/2, 1/4, or 1/8
	
	if (dad_ptr->accelerate_decompress_job == FALSE)
	{
		DC_ACCEL_LOG("Cannot accelerate this job\n");
	}
	else
	{
		DC_ACCEL_LOG("Accelerating this job\n");
	}

	dad_ptr->state = DC_ACCEL_STATE_CONSIDER;
}

boolean dc_accel_accelerate(struct jpeg_decompress_struct *ds_ptr)
{
	DC_ACCEL_DESC *dad_ptr;


	// DC_ACCEL_DESC validity checks
	
	if (ds_ptr->dc_accel_desc_ptr == NULL)
	{
		ERREXIT(ds_ptr,JERR_DC_ACCEL_DESC_PTR_NULL);
	}

	dad_ptr = (DC_ACCEL_DESC *) ds_ptr->dc_accel_desc_ptr;

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_DESC_SIGNATURE);
	}

	if (dad_ptr->state == DC_ACCEL_STATE_UNKNOWN)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_STATE);
	}

	return(dad_ptr->accelerate_decompress_job);
}

void dc_accel_start(struct jpeg_decompress_struct *ds_ptr)
{
	DC_RETURN_CODE dcStatus;
	int status;
	DC_ACCEL_DESC *dad_ptr;
	RESOURCE_TEARDOWN_STATUS *RTS_p;

	// DC_ACCEL_DESC validity checks
	
	if (ds_ptr->dc_accel_desc_ptr == NULL)
	{
		ERREXIT(ds_ptr,JERR_DC_ACCEL_DESC_PTR_NULL);
	}

	dad_ptr = (DC_ACCEL_DESC *) ds_ptr->dc_accel_desc_ptr;

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_DESC_SIGNATURE);
	}

	if (dad_ptr->state != DC_ACCEL_STATE_CONSIDER)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_STATE);
	}

	RTS_p = (RESOURCE_TEARDOWN_STATUS *) &(dad_ptr->RTS);

	// Set up all the players in the JPEGPIPE enviroment

	DC_ACCEL_LOG("Setting up JPEGPIPE environment\n");

	// Initialize RAMMGR subsystem
	
	DC_ACCEL_LOG("Issue dc_rammgr_init\n");
    dcStatus = dc_rammgr_init();
	if (dcStatus != DC_SUCCESS)
	{
		ERREXIT1(ds_ptr,JERR_DC_RAMMGR_INIT_ERROR,dcStatus);
	}

	// Open up DCAPI services
	// If logging, send sub-system dc_log_printf to console

	DC_ACCEL_LOG("Issue dcapi_open\n");
#if defined(ENABLE_DC_ACCEL_LOGGING)
	dad_ptr->dcapiHandle = dcapi_open(0);
#else
	dad_ptr->dcapiHandle = dcapi_open("/dev/null");
#endif
	if (dad_ptr->dcapiHandle == 0)
	{
		ERREXIT(ds_ptr,JERR_DCAPI_OPEN_ERROR);
	}
	RTS_SET_BIT(RTS_p,RTS_DCAPI_OPENED);

	// Get DCAPI system configuration
	
	status = dcapi_get_sys_config(dad_ptr->dcapiHandle,&(dad_ptr->systemConfig));
	if (status != 0)
	{
		ERREXIT1(ds_ptr,JERR_DCAPI_GET_SYS_CONFIG_ERROR,status);
	}
	dumpSystemConfig(dad_ptr);

	// Let DCAPI map all non-kernel RAM

	DC_ACCEL_LOG("Let DCAPI map all non-kernel RAM\n");	
	status = dcapi_map(dad_ptr->dcapiHandle,0,
					   dad_ptr->systemConfig.uncached_memory_size,
					   dad_ptr->systemConfig.io_memory_size);
	if (status != 0)
	{
		ERREXIT1(ds_ptr,JERR_DCAPI_MAP_ERROR,status);
	}
	RTS_SET_BIT(RTS_p,RTS_DCAPI_MAPPED);

	// Get virtual addresses from DCAPI for registers and non-kernel RAM
	
	status = dcapi_get_addresses(dad_ptr->dcapiHandle,&(dad_ptr->virtual_io_base),
													  &(dad_ptr->virtual_cached_base),
													  &(dad_ptr->virtual_uncached_base));
	if (status != 0)
	{
		ERREXIT1(ds_ptr,JERR_DCAPI_GET_ADDRESSES_ERROR,status);
	}
	DC_ACCEL_LOG("virtual_io_base: %#x\n",(unsigned int) dad_ptr->virtual_io_base);
	DC_ACCEL_LOG("virtual_cached_base: %#x\n",(unsigned int) dad_ptr->virtual_cached_base);
	DC_ACCEL_LOG("virtual_uncached_base: %#x\n",(unsigned int) dad_ptr->virtual_uncached_base);

	// Start up kernel-mode context managers for JPEGPIPE (JH,JP,RO,IP)
	
	DC_ACCEL_LOG("Start up kernel-mode context managers for JPEGPIPE (JH,JP,RO,IP)\n");
	status = dcapi_cm_open(dad_ptr->dcapiHandle,JPEGPIPE_CM_MASK,OSPL_PRIORITY_HIGHEST);
	if (status != 0)
	{
		ERREXIT1(ds_ptr,JERR_DCAPI_CM_OPEN_ERROR,status);
	}
	RTS_SET_BIT(RTS_p,RTS_DCAPI_CM_RUNNING);

	// Set up user-mode context managers (JH,JP,RO,IP) and CSC for JPEGPIPE
	
	DC_ACCEL_LOG("Set up user-mode context managers (JH,JP,RO,IP) and CSC\n");
	dcStatus = setup_sdk_csc_cm(dad_ptr);
	if (dcStatus != DC_SUCCESS)
	{
		ERREXIT1(ds_ptr,JERR_SETUP_SDK_CSC_CM_ERROR,dcStatus);
	}

	// Request RAMMGR memory for JPEGPIPE work area
	
	DC_ACCEL_LOG("Issuing dc_rammgr_buffer_request for JPEGPIPE work area: %#x\n",
													DC_ACCEL_JPEGPIPE_WORK_AREA_SIZE);

	dad_ptr->jpegpipe_work_area_mem_params.unprotect.size = DC_ACCEL_JPEGPIPE_WORK_AREA_SIZE;
	dad_ptr->jpegpipe_work_area_mem_params.unprotect.alignment = DC_ACCEL_JPEGPIPE_WORK_AREA_ALIGN;
	dcStatus = dc_rammgr_buffer_request(&(dad_ptr->jpegpipe_rammgr_entry),
                                      	"jpegpipe",
                                      	(UINT32) dad_ptr, 
                                      	dad_ptr->jpegpipe_work_area_mem_params.unprotect.size,
                                        dad_ptr->jpegpipe_work_area_mem_params.unprotect.alignment,
                                        DC_TYPE_PSB,
                                        0,
                                        jpegpipe_rammgr_callback);
	if (dcStatus != DC_SUCCESS)
    {
		ERREXIT1(ds_ptr,JERR_JPEGPIPE_RAMMGR_BUFFER_REQUEST_ERROR,dcStatus);
	}

	// Request RAMMGR memory for internal IMGBUF
	
	DC_ACCEL_LOG("Issuing dc_rammgr_buffer_request for internal IMGBUF: %#x\n",
													DC_ACCEL_INTERNAL_IMGBUF_SIZE);

	strcpy(((DC_MEM_PARAMS_t *) &(dad_ptr->imgbuf_mem_params))->unprotect.name,"DC_ACCEL INTERNAL IMGBUF");
	dad_ptr->imgbuf_mem_params.unprotect.size = DC_ACCEL_INTERNAL_IMGBUF_SIZE;
	dad_ptr->imgbuf_mem_params.unprotect.alignment = DC_ACCEL_INTERNAL_IMGBUF_ALIGN;
	dcStatus = dc_rammgr_buffer_request(&(dad_ptr->imgbuf_rammgr_entry),
                                      	"imgbuf",
                                      	(UINT32) dad_ptr, 
                                      	dad_ptr->imgbuf_mem_params.unprotect.size,
                                        dad_ptr->imgbuf_mem_params.unprotect.alignment,
                                        DC_TYPE_PSB,
                                        0,
                                        imgbuf_rammgr_callback);
	if (dcStatus != DC_SUCCESS)
    {
		ERREXIT1(ds_ptr,JERR_IMGBUF_RAMMGR_BUFFER_REQUEST_ERROR,dcStatus);
	}

	// Start RAMMGR -- it will now start handing out requested buffers
	
	DC_ACCEL_LOG("Issuing dc_rammgr_start\n");
    dcStatus = dc_rammgr_start((UINT32) dad_ptr->virtual_uncached_base,
							   dad_ptr->systemConfig.uncached_memory_size,
							   0,
							   dmb_manager_init);
	if (dcStatus != DC_SUCCESS)
    {
		ERREXIT1(ds_ptr,JERR_DC_RAMMGR_START_ERROR,dcStatus);
    }

	// Initialize user-mode context managers (JH,JP,RO,IP) and CSC for JPEGPIPE
	
	DC_ACCEL_LOG("Initializing user-mode context managers (JH,JP,RO,IP) and CSC\n");
	dcStatus = init_sdk_csc_cm(dad_ptr);
	if (dcStatus != DC_SUCCESS)
	{
		ERREXIT1(ds_ptr,JERR_INIT_SDK_CSC_CM_ERROR,dcStatus);
	}

	// Make sure the source manager methods exist

	if ((ds_ptr->src == NULL) ||
		(ds_ptr->src->init_source == NULL) ||
		(ds_ptr->src->fill_input_buffer == NULL) ||
		(ds_ptr->src->term_source == NULL))
	{
		ERREXIT(ds_ptr,JERR_NO_STREAM_FILE_METHODS);
	}
		
	// Pass jpeg_decompress_struct pointer into stream file thread routines
	// so that source manager methods can be accessed

	dad_ptr->jpeg_decompress_struct_ptr = (void *) ds_ptr;

	// Make sure the shadow stream holds some data
	
	if (dad_ptr->shadow_stream.ssmd_q.count == 0)
	{
		ERREXIT(ds_ptr,JERR_NO_SHADOW_STREAM_DATA);
	}
 	// Start the Stream File Thread

	dad_ptr->jpegImageWidth = ds_ptr->image_width;
	dad_ptr->jpegImageHeight = ds_ptr->image_height;
	dad_ptr->jpegComponentCount = ds_ptr->num_components;

	if (dc_accel_startStreamFileThread(dad_ptr) != TRUE)
	{
		ERREXIT(ds_ptr,JERR_START_STREAM_FILE_THREAD_ERROR);
	}
	RTS_SET_BIT(RTS_p,RTS_SF_THREAD_RUNNING);
 
 	// Make sure the Stream File Thread hasn't failed
 
    if (dad_ptr->streamFileThreadError == TRUE)
    {
		ERREXIT(ds_ptr,JERR_STREAM_FILE_THREAD_ERROR);
    }

 	// Set up the DC_IMGBUF that JPEGPIPE will write into
	// We use 32 in the stride calculation instead of 31 to solve an early chip issue

	dad_ptr->imgbuf_width = ds_ptr->output_width;	
	dad_ptr->imgbuf_height = ds_ptr->output_height;
    dad_ptr->imgbuf_base = dad_ptr->imgbuf_mem_params.unprotect.base;
    dad_ptr->imgbuf_stride = ALIGNED32((dad_ptr->imgbuf_width * ds_ptr->output_components) + 32);
	dad_ptr->imgbuf_bitdepth = DC_IMAGE_BITDEPTH_8;

	switch (ds_ptr->out_color_space)
	{
	case JCS_RGB:
        dad_ptr->imgbuf_channels = DC_IMAGE_CHANNELS_RGB;
		break;
	case JCS_YCbCr:
        dad_ptr->imgbuf_channels = DC_IMAGE_CHANNELS_YCrCb;
		break;
	case JCS_GRAYSCALE:
        dad_ptr->imgbuf_channels = DC_IMAGE_CHANNELS_1;
		break;
	default:
		ERREXIT(ds_ptr,JERR_CONVERSION_NOTIMPL);
		break;
	}

	dad_ptr->imgbuf_ySize = dad_ptr->imgbuf_height - 1;
	if (((dad_ptr->imgbuf_ySize + 1) * dad_ptr->imgbuf_stride) > dad_ptr->imgbuf_mem_params.unprotect.size)
	{
		dad_ptr->imgbuf_ySize = (dad_ptr->imgbuf_mem_params.unprotect.size / dad_ptr->imgbuf_stride) - 1;
	}
 
    if (setup_imgbuf(dad_ptr) == FALSE)
    {
		ERREXIT(ds_ptr,JERR_SETUP_IMGBUF_ERROR);
    }

 	// Make sure the Stream File Thread hasn't failed
 
    if (dad_ptr->streamFileThreadError == TRUE)
    {
		ERREXIT(ds_ptr,JERR_STREAM_FILE_THREAD_ERROR);
    }

	// Set up JPEGPIPE, CSC, TRC
	// Start with HIGH memory profile and back off to MEDIUM or LOW if necessary
	
	dad_ptr->outputColorInversion = FALSE;
	if ((ds_ptr->jpeg_color_space == ds_ptr->out_color_space) ||
		((ds_ptr->jpeg_color_space == JCS_YCbCr) && (ds_ptr->out_color_space == JCS_GRAYSCALE)))
	{
    	dad_ptr->cscMode = DC_JPEGPIPE_CSC_MODE_NONE;
	}
	else
	{
    	dad_ptr->cscMode = DC_JPEGPIPE_CSC_MODE_DEFAULT;
	}
    dad_ptr->colorTable = NULL;
	dad_ptr->trcEnabled = FALSE;
	dad_ptr->trcTable[0] = dad_ptr->trcTable[1] = dad_ptr->trcTable[2] = NULL;
	dad_ptr->jpegpipe_profile = DC_JPEGPIPE_PROFILE_HIGH;

	while (1)
	{
		if (setup_jpegpipe_csc_trc(dad_ptr) == FALSE)
		{
			ERREXIT(ds_ptr,JERR_SETUP_JPEGPIPE_CSC_TRC_ERROR);
		}

        if (dad_ptr->jpegpipe_mem_params.unprotect.alignment > DC_ACCEL_JPEGPIPE_WORK_AREA_ALIGN)
        {
			ERREXIT2(ds_ptr,JERR_JPEGPIPE_MEMORY_ALIGNMENT_ERROR,
						dad_ptr->jpegpipe_mem_params.unprotect.alignment,DC_ACCEL_JPEGPIPE_WORK_AREA_ALIGN);
        }

        if (dad_ptr->jpegpipe_mem_params.unprotect.size > dad_ptr->jpegpipe_work_area_mem_params.unprotect.size)
        {
            if (dad_ptr->jpegpipe_profile == DC_JPEGPIPE_PROFILE_LOW)
            {
				ERREXIT2(ds_ptr,JERR_JPEGPIPE_MEMORY_SIZE_ERROR,
					dad_ptr->jpegpipe_mem_params.unprotect.size,dad_ptr->jpegpipe_work_area_mem_params.unprotect.size);
			}
            else
            {
                DC_ACCEL_LOG("Unable to fulfill JPEGPIPE memory request, needs: %#x, only have: %#x\n",
															dad_ptr->jpegpipe_mem_params.unprotect.size,
															dad_ptr->jpegpipe_work_area_mem_params.unprotect.size);
                DC_ACCEL_LOG("Lowering profile and restarting setup_jpegpipe_csc_trc\n");
				if (dad_ptr->jpegpipe_profile == DC_JPEGPIPE_PROFILE_HIGH)
				{
					dad_ptr->jpegpipe_profile = DC_JPEGPIPE_PROFILE_MEDIUM;
				}
				else
				{
					dad_ptr->jpegpipe_profile = DC_JPEGPIPE_PROFILE_LOW;
				}
				// Check to see if a color table got loaded, and unload if yes
				if (RTS_BIT_IS_SET(RTS_p,RTS_COLOR_TABLE_LOADED))
    			{
   					DC_ACCEL_LOG("Issuing dc_csc_free\n");
        			dcStatus = dc_csc_free(&(dad_ptr->cscTableData));
        			if (dcStatus != DC_SUCCESS)
        			{
						ERREXIT1(ds_ptr,JERR_DC_CSC_FREE_ERROR,dcStatus);
        			}
        			else
        			{
            			RTS_CLEAR_BIT(RTS_p,RTS_COLOR_TABLE_LOADED);
        			}
				}
                continue;
            }
        }

		break;
	}

    DC_ACCEL_LOG("JPEGPIPE Memory Size Requested: %#08lx\n",dad_ptr->jpegpipe_mem_params.unprotect.size);
    DC_ACCEL_LOG("JPEGPIPE Memory Size Given: %#08lx\n",dad_ptr->jpegpipe_work_area_mem_params.unprotect.size);
    DC_ACCEL_LOG("JPEGPIPE Memory Base: %#08lx\n",dad_ptr->jpegpipe_work_area_mem_params.unprotect.base);
    dad_ptr->jpegpipe_mem_params.unprotect.base = dad_ptr->jpegpipe_work_area_mem_params.unprotect.base;
    dad_ptr->jpegpipe_mem_params.unprotect.size = dad_ptr->jpegpipe_work_area_mem_params.unprotect.size;
    dad_ptr->jpegpipe_mem_params.unprotect.alignment = dad_ptr->jpegpipe_work_area_mem_params.unprotect.alignment;
       
    dad_ptr->timerStartImgBufStall = ospl_time_get();
   
   	analyze_dmbimg(dad_ptr);

    // Start the JPEGPIPE thread

    memset(&(dad_ptr->jpegpipeResults),0,sizeof(DC_JPEGPIPE_RESULTS_t));
    dad_ptr->jpegpipe_callback_triggered = FALSE;
    dad_ptr->jpegpipe_callback_error = FALSE;
	dad_ptr->scanlinesConsumed = 0;

    dcStatus = dc_jpegpipe_create(&(dad_ptr->jpegpipe_context),
                                  &(dad_ptr->jpegpipe_mem_params),
                                  "DC_ACCEL",
                                  DC_THREAD_PRIORITY_VERY_HIGH,
                                  dc_accel_jpegpipe_callback,
                                  (void *) dad_ptr,
                                  &(dad_ptr->jpegpipeResults));
    if (dcStatus != DC_SUCCESS)
    {
		ERREXIT1(ds_ptr,JERR_DC_JPEGPIPE_CREATE_ERROR,dcStatus);
    }
    else
    {
        RTS_SET_BIT(RTS_p,RTS_JPEGPIPE_THREAD_RUNNING);
    }
            
	dad_ptr->state = DC_ACCEL_STATE_START;
}

void dc_accel_read_scanlines(struct jpeg_decompress_struct *ds_ptr,
								JSAMPARRAY scanlines,
								JDIMENSION *row_count_ptr,
								JDIMENSION max_lines)
{
	DC_RETURN_CODE dcStatus;
	DC_ACCEL_DESC *dad_ptr;
    UINT32 validScanlines;
	int num_scanlines_processed = 0;

	//DC_ACCEL_LOG("dc_accel_read_scanlines: %d\n",max_lines);

	// DC_ACCEL_DESC validity checks
	
	if (ds_ptr->dc_accel_desc_ptr == NULL)
	{
		ERREXIT(ds_ptr,JERR_DC_ACCEL_DESC_PTR_NULL);
	}

	dad_ptr = (DC_ACCEL_DESC *) ds_ptr->dc_accel_desc_ptr;

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_DESC_SIGNATURE);
	}

	if (dad_ptr->state != DC_ACCEL_STATE_START)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_STATE);
	}

 	// Make sure the Stream File Thread hasn't failed
 
    if (dad_ptr->streamFileThreadError == TRUE)
    {
		ERREXIT(ds_ptr,JERR_STREAM_FILE_THREAD_ERROR);
    }

	// Check for callback or JPEGPIPE errors

    if ((dad_ptr->jpegpipe_callback_triggered == TRUE) &&
        ((dad_ptr->jpegpipe_callback_error == TRUE) ||
         (dad_ptr->jpegpipeResults.error != DC_JPEGPIPE_ERROR_NONE)))
    {
		ERREXIT2(ds_ptr,JERR_DC_JPEGPIPE_CALLBACK_ERROR,
										dad_ptr->jpegpipe_callback_error,dad_ptr->jpegpipeResults.error);
    }

	if (max_lines == 0)
	{
		*row_count_ptr = 0;
		return;
	}

	// Try to deliver the scanlines requested
	
    while (num_scanlines_processed < max_lines)
    {
		UINT32 imgbuf_yOffset;
		UINT32 line_length;
		void *imgbuf_address;
		void *user_buffer_address;

		// Check to make sure that JPEGPIPE is still pumping out scanlines

		if (dad_ptr->scanlinesConsumed >= dad_ptr->imgbuf_height) break;

		//DC_ACCEL_LOG("Issuing dc_imgbuf_consumer_valid_get\n");
        dcStatus = dc_imgbuf_consumer_valid_get(&(dad_ptr->imgbufConsumer),&validScanlines);
        if (dcStatus != DC_SUCCESS)
        {
			ERREXIT1(ds_ptr,JERR_DC_IMGBUF_CONSUMER_VALID_GET_ERROR,dcStatus);
        }
		//DC_ACCEL_LOG("validScanlines: %d\n",validScanlines);
        
        // Are there any scanlines to process?
        
        if (!validScanlines)
		{
			ospl_thread_sleep(5);
			continue;
		}
       
		//DC_ACCEL_LOG("Issuing dc_imgbuf_consumer_yValue_get\n"); 
    	dcStatus = dc_imgbuf_consumer_yValue_get(&(dad_ptr->imgbufConsumer),(UINT32 *) &imgbuf_yOffset);
    	if (dcStatus != DC_SUCCESS)
    	{
			ERREXIT1(ds_ptr,JERR_DC_IMGBUF_CONSUMER_YVALUE_GET_ERROR,dcStatus);
    	}
		imgbuf_address = (void *) (dad_ptr->imgbuf_base + (dad_ptr->imgbuf_stride * imgbuf_yOffset));

		line_length = ds_ptr->output_width * ds_ptr->output_components;
		user_buffer_address = (void *) ((UINT32) *scanlines + (num_scanlines_processed * line_length));

		//DC_ACCEL_LOG("Issuing memcpy(%#x,%#x,%#x)\n",user_buffer_address,imgbuf_address,line_length);
		memcpy(user_buffer_address,imgbuf_address,line_length);

		num_scanlines_processed++;

		//DC_ACCEL_LOG("Issuing dc_imgbuf_consumer_consume\n");
        dcStatus = dc_imgbuf_consumer_consume(&(dad_ptr->imgbufConsumer),1);
        if (dcStatus != DC_SUCCESS)
        {
			ERREXIT1(ds_ptr,JERR_DC_IMGBUF_CONSUMER_CONSUME_ERROR,dcStatus);
        }

		dad_ptr->scanlinesConsumed++;
    }

	dad_ptr->timerEndImgBufStall = ospl_time_get();

    if (num_scanlines_processed)
    {
        dad_ptr->timerStartImgBufStall = dad_ptr->timerEndImgBufStall;
    }
    else
    {
        if (dc_time_diff_msec(dad_ptr->timerStartImgBufStall) > IMGBUF_STALL_TIMEOUT)
        {
			ERREXIT(ds_ptr,JERR_IMGBUF_STALL);
        }
    }
    
	*row_count_ptr = num_scanlines_processed;
}

void dc_accel_teardown_resources(struct jpeg_decompress_struct *ds_ptr)
{
	DC_RETURN_CODE dcStatus;
	int status;
	DC_ACCEL_DESC *dad_ptr;
	RESOURCE_TEARDOWN_STATUS *RTS_p;

	DC_ACCEL_LOG("Entering dc_accel_teardown_resources: %#lx\n",
										(long unsigned int) ospl_get_time());

	// We cannot call ERREXITx from this routine, since we may be being called from ERREXITx.
	// If we error on something, try to make as much forward progress as possible.
	// This is the last chance to clean up resources.
	
	// DC_ACCEL_DESC validity checks
	
	if (ds_ptr->dc_accel_desc_ptr == NULL)
	{
		DC_ACCEL_LOG("dc_accel_desc_ptr is NULL, exiting!\n");
		return;
	}

	dad_ptr = (DC_ACCEL_DESC *) ds_ptr->dc_accel_desc_ptr;

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		DC_ACCEL_LOG("Invalid DC_ACCEL_DESC signature, exiting!\n");
		return;
	}

	if (dad_ptr->accelerate_decompress_job == FALSE)
	{
		DC_ACCEL_LOG("accelerate_decompress_job is FALSE, exiting!\n");
		return;
	}

	RTS_p = (RESOURCE_TEARDOWN_STATUS *) &(dad_ptr->RTS);

    DC_ACCEL_LOG("RTS (Entry): 0x%08lX\n",(long unsigned int) *RTS_p);
  
    if (RTS_BIT_IS_SET(RTS_p,RTS_JPEGPIPE_THREAD_RUNNING))
    {
        DC_JPEGPIPE_STATUS_t jpegpipeStatus;
        DC_JPEGPIPE_ERROR_t jpegpipeError;
        boolean cancelIssued = FALSE;
        UINT32 startTime = ospl_time_get();

        DC_ACCEL_LOG("Wait for JPEGPIPE to go idle ...\n");

        while (1)
        {
            dcStatus = dc_jpegpipe_status_get(&(dad_ptr->jpegpipe_context),&jpegpipeStatus,&jpegpipeError);
            if (dcStatus != DC_SUCCESS)
            {
                DC_ACCEL_LOG("dc_jpegpipe_status_get error: %#x\n",dcStatus);
                break;
            }
            if (jpegpipeStatus == DC_JPEGPIPE_STATUS_IDLE)
            {
                DC_ACCEL_LOG("JPEGPIPE is idle\n");
                break;
            }
            if (cancelIssued == FALSE)
            {
                dcStatus = dc_jpegpipe_cancel(&(dad_ptr->jpegpipe_context));
                if ((dcStatus != DC_SUCCESS) && (dcStatus != DC_PIPE_ALREADY_IDLE))
                {
                    DC_ACCEL_LOG("dc_jpegpipe_cancel error: %#x\n",dcStatus);
                    break;
                }
                cancelIssued = TRUE;
            }
            if (dc_time_diff_msec(startTime) > DC_ACCEL_CANCEL_JPEGPIPE_TIMEOUT)
            {
                DC_ACCEL_LOG("JPEGPIPE won't go IDLE!\n");
                break;
            }
            ospl_thread_sleep(10);
        }

   		DC_ACCEL_LOG("dc_jpegpipe_delete: %#lx\n",(long unsigned int) ospl_get_time());
        dcStatus = dc_jpegpipe_delete(&(dad_ptr->jpegpipe_context));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_jpegpipe_delete error: %#x\n",dcStatus);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_JPEGPIPE_THREAD_RUNNING);
        }
    }

    if (RTS_BIT_IS_SET(RTS_p,RTS_SF_THREAD_RUNNING))
    {
        if (dc_accel_cancelStreamFileThread(dad_ptr) != TRUE)
        {
            DC_ACCEL_LOG("dc_accel_cancelStreamFileThread error!\n");
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_SF_THREAD_RUNNING);
        }
    }
   
    if (RTS_BIT_IS_SET(RTS_p,RTS_DC_IMGBUF_CONSUMER_REGISTERED))
    {
   		DC_ACCEL_LOG("dc_imgbuf_consumer_unregister: %#lx\n",(long unsigned int) ospl_get_time());
        dcStatus = dc_imgbuf_consumer_unregister(&(dad_ptr->imgbufConsumer));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_imgbuf_consumer_unregister error: %#x\n",dcStatus);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_DC_IMGBUF_CONSUMER_REGISTERED);
        }
    }
    
    if (RTS_BIT_IS_SET(RTS_p,RTS_DC_DMBIMG_PRODUCER_REGISTERED))
    {
   		DC_ACCEL_LOG("dc_dmbimg_producer_unregister: %#lx\n",(long unsigned int) ospl_get_time());
        dcStatus = dc_dmbimg_producer_unregister(&(dad_ptr->dmbimgProducer));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_dmbimg_producer_unregister error: %#x\n",dcStatus);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_DC_DMBIMG_PRODUCER_REGISTERED);
        }
    }
    
	if (RTS_BIT_IS_SET(RTS_p,RTS_COLOR_TABLE_LOADED))
    {
   		DC_ACCEL_LOG("dc_csc_free: %#lx\n",(long unsigned int) ospl_get_time());
        dcStatus = dc_csc_free(&(dad_ptr->cscTableData));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_csc_free error: %#x\n",dcStatus);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_COLOR_TABLE_LOADED);
        }
    }
    
    if (RTS_BIT_IS_SET(RTS_p,RTS_DC_IMGBUF_CREATED))
    {
   		DC_ACCEL_LOG("dc_imgbuf_delete: %#lx\n",(long unsigned int) ospl_get_time());
        dcStatus = dc_imgbuf_delete(&(dad_ptr->imgbuf));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_imgbuf_delete error: %#x\n",dcStatus);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_DC_IMGBUF_CREATED);
        }
    }
    
    if (RTS_BIT_IS_SET(RTS_p,RTS_DC_DMBIMG_CREATED))
    {
   		DC_ACCEL_LOG("dc_dmbimg_delete: %#lx\n",(long unsigned int) ospl_get_time());
        dcStatus = dc_dmbimg_delete(&(dad_ptr->dmbimg));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_dmbimg_delete error: %#x\n",dcStatus);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_DC_DMBIMG_CREATED);
        }
    }

    if (RTS_BIT_IS_SET(RTS_p,RTS_DCAPI_CM_RUNNING))
    {
   		DC_ACCEL_LOG("dcapi_cm_close: %#lx\n",(long unsigned int) ospl_get_time());
		status = dcapi_cm_close(dad_ptr->dcapiHandle);
        if (status != 0)
        {
            DC_ACCEL_LOG("dcapi_cm_close error: %#x\n",status);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_DCAPI_CM_RUNNING);
        }
	}

    if (RTS_BIT_IS_SET(RTS_p,RTS_DCAPI_MAPPED))
    {
   		DC_ACCEL_LOG("dcapi_unmap: %#lx\n",(long unsigned int) ospl_get_time());
		status = dcapi_unmap(dad_ptr->dcapiHandle);
        if (status != 0)
        {
            DC_ACCEL_LOG("dcapi_unmap error: %#x\n",status);
        }
        else
        {
            RTS_CLEAR_BIT(RTS_p,RTS_DCAPI_MAPPED);
        }
	}

    if (RTS_BIT_IS_SET(RTS_p,RTS_DCAPI_OPENED))
    {
   		DC_ACCEL_LOG("dcapi_close: %#lx\n",(long unsigned int) ospl_get_time());
		dcapi_close(dad_ptr->dcapiHandle);
        RTS_CLEAR_BIT(RTS_p,RTS_DCAPI_OPENED);
	}

    DC_ACCEL_LOG("RTS (Exit): 0x%08lX\n",(long unsigned int) *RTS_p);
   	DC_ACCEL_LOG("Leaving: %#lx\n",(long unsigned int) ospl_get_time());

}
/*
UINT32 thread_priority_get(DC_THREAD_PRIORITY priority)
{
	return(priority);
}

*/

void dc_accel_shadow_stream(struct jpeg_decompress_struct *ds_ptr)
{
	DC_ACCEL_DESC *dad_ptr;
	DC_ACCEL_SHADOW_STREAM_MEMORY_DESC *ssmd_ptr;
	UINT32 byte_count;
	UINT32 memory_size;
	UINT8 *source;

	DC_ACCEL_LOG("dc_accel_shadow_stream\n");

	// DC_ACCEL_DESC validity checks
	
	if (ds_ptr->dc_accel_desc_ptr == NULL)
	{
		ERREXIT(ds_ptr,JERR_DC_ACCEL_DESC_PTR_NULL);
	}

	dad_ptr = (DC_ACCEL_DESC *) ds_ptr->dc_accel_desc_ptr;

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_DESC_SIGNATURE);
	}

	if (dad_ptr->state == DC_ACCEL_STATE_UNKNOWN)
	{
		ERREXIT(ds_ptr,JERR_BAD_DC_ACCEL_STATE);
	}

	if (dad_ptr->state == DC_ACCEL_STATE_START)
	{
		DC_ACCEL_LOG("dc_accel_shadow_stream called in DC_ACCEL_STATE_START!\n");
		return;
	}

	byte_count = ds_ptr->src->bytes_in_buffer;
	source = (UINT8 *) ds_ptr->src->next_input_byte;

	DC_ACCEL_LOG("Shadow stream %#x bytes at %#x\n",byte_count,source);

	if (byte_count == 0)
	{
		DC_ACCEL_LOG("Shadow stream: No work to do!\n");
		return;
	}

	if (dad_ptr->shadow_stream.errorEncountered == TRUE)
	{
		DC_ACCEL_LOG("Shadow stream: previous error encountered!\n");
		return;
	}

	memory_size = byte_count + sizeof(DC_ACCEL_SHADOW_STREAM_DESC) + 32;

	if (memory_size > 4096)
	{
		ssmd_ptr = (DC_ACCEL_SHADOW_STREAM_MEMORY_DESC *)(*ds_ptr->mem->alloc_large)
										((j_common_ptr) ds_ptr,JPOOL_IMAGE,memory_size);
	}
	else
	{
		ssmd_ptr = (DC_ACCEL_SHADOW_STREAM_MEMORY_DESC *)(*ds_ptr->mem->alloc_small)
										((j_common_ptr) ds_ptr,JPOOL_IMAGE,memory_size);
	}

	if (ssmd_ptr == NULL)
	{
		dad_ptr->shadow_stream.errorEncountered = TRUE;
		return;
	}

	ssmd_ptr->ql.prev = NULL;
	ssmd_ptr->ql.next = NULL;
	ssmd_ptr->byte_count = byte_count;
	ssmd_ptr->buffer = (UINT8 *) (((UINT32) ssmd_ptr) + sizeof(DC_ACCEL_SHADOW_STREAM_MEMORY_DESC));

	DC_ACCEL_LOG("memcpy: %#x -> %#x, %#x bytes\n",source,ssmd_ptr->buffer,byte_count);
	memcpy(ssmd_ptr->buffer,source,byte_count);
	
	DC_ACCEL_LOG("Enqueueing %#x onto ssmd_q\n",ssmd_ptr);
	DAQueueInsertEnd(&(dad_ptr->shadow_stream.ssmd_q),ssmd_ptr);

	dad_ptr->shadow_stream.total_byte_count += byte_count;
	DC_ACCEL_LOG("total_byte_count now: %#x\n",dad_ptr->shadow_stream.total_byte_count);
}

void dc_accel_skip_input_data(struct jpeg_decompress_struct *ds_ptr,long num_bytes)
{
  DC_ACCEL_LOG("dc_accel_skip_input_data: num_bytes: %#x,bytes_in_buffer: %#x\n",
  				num_bytes,ds_ptr->src->bytes_in_buffer);
  
  while (num_bytes > 0)
  {
    if (num_bytes > (long) ds_ptr->src->bytes_in_buffer)
	{
      num_bytes -= (long) ds_ptr->src->bytes_in_buffer;
      (*ds_ptr->src->fill_input_buffer)(ds_ptr);
	  dc_accel_shadow_stream(ds_ptr);
    }
	else
	{
      ds_ptr->src->bytes_in_buffer -= num_bytes;
      ds_ptr->src->next_input_byte += num_bytes;
      num_bytes = 0;
	}

    DC_ACCEL_LOG("dc_accel_skip_input_data: num_bytes: %#x,bytes_in_buffer: %#x\n",
  												num_bytes,ds_ptr->src->bytes_in_buffer);
  }
}

//----------------------------------------------------------------------------------------------------
// Local functions below this line - prototypes in dc_accel.h
//----------------------------------------------------------------------------------------------------

static void dumpSystemConfig(DC_ACCEL_DESC *dad_ptr)
{
	DC_ACCEL_LOG("System Configuration\n");
	DC_ACCEL_LOG("--------------------\n");
	DC_ACCEL_LOG("cpu_clock_hz: %ld\n",dad_ptr->systemConfig.cpu_clock_hz);
	DC_ACCEL_LOG("main_clock_hz: %ld\n",dad_ptr->systemConfig.main_clock_hz);
	DC_ACCEL_LOG("kernel_memory_size: %#lx\n",dad_ptr->systemConfig.kernel_memory_size);
	DC_ACCEL_LOG("io_memory_physical: %#lx\n",dad_ptr->systemConfig.io_memory_physical);
	DC_ACCEL_LOG("io_memory_size: %#lx\n",dad_ptr->systemConfig.io_memory_size);
	DC_ACCEL_LOG("cached_memory_physical: %#lx\n",dad_ptr->systemConfig.cached_memory_physical);
	DC_ACCEL_LOG("cached_memory_size: %#lx\n",dad_ptr->systemConfig.cached_memory_size);
	DC_ACCEL_LOG("uncached_memory_physical: %#lx\n",dad_ptr->systemConfig.uncached_memory_physical);
	DC_ACCEL_LOG("uncached_memory_size: %#lx\n",dad_ptr->systemConfig.uncached_memory_size);
	DC_ACCEL_LOG("--------------------\n");
}

static DC_ERROR_CODE csc_rammgr_callback(DC_RAMMGR_BUFFER_ENTRY *entry)
{
    DC_ERROR_CODE rc = !DC_SUCCESS;

	DC_ACCEL_LOG("csc_rammgr_callback entered: %d\n",entry->status);

    if (entry->status == DC_BUFFER_AVAILABLE)
    {
		DC_ACCEL_DESC *dad_ptr;

		if (!entry->bufferId)
		{
			DC_ACCEL_LOG("Fatal error - DC_RAMMGR_BUFFER_ENTRY bufferId is NULL\n");
			return(rc);
		}

		dad_ptr = (DC_ACCEL_DESC *) entry->bufferId;

		if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
		{
			DC_ACCEL_LOG("Fatal error - Invalid DC_ACCEL_DESC signature\n");
			return(rc);
		}

        dad_ptr->csc_mem_params.unprotect.base = CACHE_BYPASS(entry->address);
		DC_ACCEL_LOG("CSC memory at %#x, %#x bytes\n",entry->address,entry->length);
        rc = DC_SUCCESS;
    }
  
    return(rc);
}

static DC_RETURN_CODE csc_library_setup(DC_ACCEL_DESC *dad_ptr)
{
    DC_ERROR_CODE result = DC_SUCCESS;

    memset(&(dad_ptr->csc_mem_params),0,sizeof(DC_MEM_PARAMS_t));

    // max out components 6
    // max onchip lut size 9x
    // max offchip lut size 17x
    // max onchip table count 2
    // max offchip tabel count 2
    // cascaded?
#if defined(ENABLE_ENHANCED_IP)
    result = dc_csc_setup(6, 17, 17, 2, 2, 1, &(dad_ptr->csc_mem_params));
#else
    result = dc_csc_setup(6, 9,  17, 2, 2, 1, &(dad_ptr->csc_mem_params));
#endif

    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_csc_setup: %#x\n",result);
        return(result);
    }

    DC_ACCEL_LOG("csc size %#x\n",dad_ptr->csc_mem_params.unprotect.size);
    DC_ACCEL_LOG("csc alignment %#x\n",dad_ptr->csc_mem_params.unprotect.alignment);

    if (dad_ptr->csc_mem_params.unprotect.size)
    {
        result = dc_rammgr_buffer_request(&(dad_ptr->csc_rammgr_entry),
                                          "csc",
                                          (UINT32) dad_ptr, 
                                          dad_ptr->csc_mem_params.unprotect.size,
                                          dad_ptr->csc_mem_params.unprotect.alignment,
                                          DC_TYPE_PSB,
                                          0,
                                          csc_rammgr_callback);
  
        if (result != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_rammgr_buffer_request: %#x\n",result);
        }
    }

    return(result);
}

static DC_RETURN_CODE dc_context_managers_setup(void)
{
    DC_ERROR_CODE result = DC_SUCCESS;
    DC_DT_ENGINE_ID sys_dt_id;

    //nQueueEntries = 16
    result = dc_conip_setup(16,NULL);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conip_setup: %#x\n",result);
        return(result);
    }
  
    //nQueueEntries = 8
    result = dc_conro_setup(8,NULL);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conro_setup: %#x\n",result);
        return(result);
    }

    //nQueueEntries = 8
    result = dc_conjp_setup(8,NULL);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conjp_setup: %#x\n",result);
        return(result);
    }

    sys_dt_id = CONJH_FIRST_DT_ENGINE | CONJH_SECOND_DT_ENGINE;

    //nQueueEntries = 8
    result = dc_conjh_setup(8,NULL,
                            sys_dt_id, 
                            CONJH_MI_RD_CHANNEL, 
                            CONJH_MI_WR_CHANNEL, 
                            DC_CONJH_RESOURCE_ACQUISITION_AT_CONTEXT);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conjh_setup: %#x\n",result);
        return(result);
    }

    return(result);

}

static DC_RETURN_CODE dc_context_managers_init(void)
{
    DC_RETURN_CODE result = DC_SUCCESS;
  
	result = dc_conip_initialize(NULL);
    if (result != DC_SUCCESS)
    {
		DC_ACCEL_LOG("dc_conip_initialize: %#x\n",result);
        return(result);
    }

    result = dc_conro_initialize(NULL);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conro_initialize: %#x\n",result);
        return(result);
    }

    result = dc_conjp_initialize(NULL);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conjp_initialize: %#x\n",result);
        return(result);
    }

    result = dc_conjh_initialize(NULL);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_conjh_initialize: %#x\n",result);
        return(result);
    }

    return(result);

}

static DC_RETURN_CODE setup_sdk_csc_cm(DC_ACCEL_DESC *dad_ptr)
{      
    DC_RETURN_CODE result = DC_SUCCESS;

    result = csc_library_setup(dad_ptr);
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("csc_library_setup: %#x\n",result);
        return(result);
    }

    result = dc_context_managers_setup();     
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_context_managers_setup: %#x\n",result);
        return(result);
    }

    return(result);
}


static DC_RETURN_CODE init_sdk_csc_cm(DC_ACCEL_DESC *dad_ptr)
{      
    DC_RETURN_CODE result = DC_SUCCESS;

    result = dc_csc_init(&(dad_ptr->csc_mem_params));
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_csc_init: %#x\n",result);
        return(result);
    }

    result = dc_context_managers_init();
    if (result != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_context_managers_init: %#x\n",result);
        return(result);
    }

    return(result);
}

static DC_ERROR_CODE jpegpipe_rammgr_callback(DC_RAMMGR_BUFFER_ENTRY *entry)
{
    DC_ERROR_CODE rc = !DC_SUCCESS;

	DC_ACCEL_LOG("jpegpipe_rammgr_callback entered: %d\n",entry->status);

    if (entry->status == DC_BUFFER_AVAILABLE)
    {
		DC_ACCEL_DESC *dad_ptr;

		if (!entry->bufferId)
		{
			DC_ACCEL_LOG("Fatal error - DC_RAMMGR_BUFFER_ENTRY bufferId is NULL\n");
			return(rc);
		}

		dad_ptr = (DC_ACCEL_DESC *) entry->bufferId;

		if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
		{
			DC_ACCEL_LOG("Fatal error - Invalid DC_ACCEL_DESC signature\n");
			return(rc);
		}

        dad_ptr->jpegpipe_work_area_mem_params.unprotect.base = CACHE_BYPASS(entry->address);
		DC_ACCEL_LOG("JPEGPIPE memory at %#x, %#x bytes\n",entry->address,entry->length);
        rc = DC_SUCCESS;
    }
  
    return(rc);
}

static DC_ERROR_CODE imgbuf_rammgr_callback(DC_RAMMGR_BUFFER_ENTRY *entry)
{
    DC_ERROR_CODE rc = !DC_SUCCESS;

	DC_ACCEL_LOG("imgbuf_rammgr_callback entered: %d\n",entry->status);

    if (entry->status == DC_BUFFER_AVAILABLE)
    {
		DC_ACCEL_DESC *dad_ptr;

		if (!entry->bufferId)
		{
			DC_ACCEL_LOG("Fatal error - DC_RAMMGR_BUFFER_ENTRY bufferId is NULL\n");
			return(rc);
		}

		dad_ptr = (DC_ACCEL_DESC *) entry->bufferId;

		if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
		{
			DC_ACCEL_LOG("Fatal error - Invalid DC_ACCEL_DESC signature\n");
			return(rc);
		}

        dad_ptr->imgbuf_mem_params.unprotect.base = CACHE_BYPASS(entry->address);
		DC_ACCEL_LOG("IMGBUF memory at %#x, %#x bytes\n",entry->address,entry->length);
        rc = DC_SUCCESS;
    }
  
    return(rc);
}

static DC_RETURN_CODE dmb_manager_init(DC_RAMMGR_MEMORY_STATUS *status)
{
    DC_RETURN_CODE result = DC_SUCCESS;

	DC_ACCEL_LOG("Running dmb_manager_init\n");

    if (status->valid != 0)
    {
        DC_DMB_PARAMS localDMBParams;
        UINT32 endRAMMemory;

#if defined(DCAPI_HACK)
		DC_ACCEL_LOG("DC_RAMMGR_MEMORY_STATUS:\n");
		DC_ACCEL_LOG("start: %#x\n",status->start);
		DC_ACCEL_LOG("total: %#x\n",status->total);
		DC_ACCEL_LOG("currentStateMemoryUsed: %#x\n",status->currentStateMemoryUsed);
		DC_ACCEL_LOG("nextStateMemoryUsed: %#x\n",status->nextStateMemoryUsed);
		DC_ACCEL_LOG("maxMemoryUsed: %#x\n",status->maxMemoryUsed);
		DC_ACCEL_LOG("valid: %#x\n",status->valid);
#endif
	    
        localDMBParams.pageSize = DMB_PAGE_SIZE;
        localDMBParams.base = status->start + status->maxMemoryUsed;
        endRAMMemory = status->start + status->total;
        localDMBParams.memSize = endRAMMemory - localDMBParams.base;
        localDMBParams.base = CACHED_ADDRESS(localDMBParams.base);
#if defined(DCAPI_HACK)
		// Take 3 MB off the top to account for DirectFB mem allocation during
		// DCAPI_HACK.  This amount is enough for 1024x600x32bpp and a bit more.
		// If fbdev resolution is more, adjust amount below
		localDMBParams.memSize -= 0x300000;
		DC_ACCEL_LOG("endRAMMemory: %#x\n",endRAMMemory);
		DC_ACCEL_LOG("DC_DMB_PARAMS:\n");
		DC_ACCEL_LOG("pageSize: %#x\n",localDMBParams.pageSize);
		DC_ACCEL_LOG("base: %#x\n",localDMBParams.base);
		DC_ACCEL_LOG("memSize: %#x\n",localDMBParams.memSize);
#endif
        result = dc_dmb_init(&localDMBParams);
		if (result != DC_SUCCESS)
		{
			DC_ACCEL_LOG("dc_dmb_init error: %#x",result);
		}
    }

    return(result);
}

static boolean setup_imgbuf(DC_ACCEL_DESC *dad_ptr)
{
	DC_RETURN_CODE dcStatus;
	RESOURCE_TEARDOWN_STATUS *RTS_p;

	RTS_p = (RESOURCE_TEARDOWN_STATUS *) &(dad_ptr->RTS);

    // Create DC_IMGBUF 
    
    DC_ACCEL_LOG("Issuing dc_imgbuf_create\n");
    DC_ACCEL_LOG("  imgbuf_stride: %#x\n",dad_ptr->imgbuf_stride);
    DC_ACCEL_LOG("  imgbuf_ySize: %#x\n",dad_ptr->imgbuf_ySize);
    DC_ACCEL_LOG("  imgbuf_width: %#x\n",dad_ptr->imgbuf_width);
    DC_ACCEL_LOG("  imgbuf_height: %#x\n",dad_ptr->imgbuf_height);
    DC_ACCEL_LOG("  imgbuf_base: %#x\n",dad_ptr->imgbuf_base);
    DC_ACCEL_LOG("  imgbuf_channels: %#x\n",dad_ptr->imgbuf_channels);
    DC_ACCEL_LOG("  imgbuf_bitdepth: %#x\n",dad_ptr->imgbuf_bitdepth);
    
    dcStatus = dc_imgbuf_create(&(dad_ptr->imgbuf),
                                "DC_ACCEL_JPEGPIPE_IMGBUF",
                                dad_ptr->imgbuf_channels,
                                dad_ptr->imgbuf_bitdepth,
                                DC_IMAGE_FORMAT_PIXEL_INTERLEAVED,
                                (void *) dad_ptr->imgbuf_base,
                                dad_ptr->imgbuf_stride,
                                dad_ptr->imgbuf_ySize,
                                0,						// yStart always 0
                                dad_ptr->imgbuf_width,
                                dad_ptr->imgbuf_height);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_imgbuf_create error: %#x\n",dcStatus);
		return(FALSE);
    }
    else
    {
        RTS_SET_BIT(RTS_p,RTS_DC_IMGBUF_CREATED);
    }

	DC_ACCEL_LOG("Issuing dc_imgbuf_consumer_setup\n");
    dcStatus = dc_imgbuf_consumer_setup(&(dad_ptr->imgbufConsumer),"DC_ACCEL",0,0);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_imgbuf_consumer_setup error: %#x\n",dcStatus);
		return(FALSE);
    }

	DC_ACCEL_LOG("Issuing dc_imgbuf_consumer_register\n");
    dcStatus = dc_imgbuf_consumer_register(&(dad_ptr->imgbuf),&(dad_ptr->imgbufConsumer));
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_imgbuf_consumer_register error: %#x\n",dcStatus);
		return(FALSE);
    }
    else
    {
        RTS_SET_BIT(RTS_p,RTS_DC_IMGBUF_CONSUMER_REGISTERED);
    }

	return(TRUE);
}

static boolean setup_jpegpipe_csc_trc(DC_ACCEL_DESC *dad_ptr)
{
	DC_RETURN_CODE dcStatus;
	RESOURCE_TEARDOWN_STATUS *RTS_p;

	RTS_p = (RESOURCE_TEARDOWN_STATUS *) &(dad_ptr->RTS);

	DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_begin\n");
    dcStatus = dc_jpegpipe_setup_begin(&(dad_ptr->jpegpipe_context));
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_jpegpipe_setup_begin error: %#x\n",dcStatus);
		return(FALSE);
    }

	DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_mode_set\n");
    dcStatus = dc_jpegpipe_setup_mode_set(&(dad_ptr->jpegpipe_context),DC_JPEGPIPE_MODE_DECODE);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_jpegpipe_setup_mode_set error: %#x\n",dcStatus);
		return(FALSE);
    }

	DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_data_set\n");
    dcStatus = dc_jpegpipe_setup_data_set(&(dad_ptr->jpegpipe_context),&(dad_ptr->imgbuf),&(dad_ptr->dmbimg),TRUE);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_jpegpipe_setup_data_set error: %#x\n",dcStatus);
		return(FALSE);
    }

#if 1
   	DC_ACCEL_LOG("Issuing dc__jpegpipe_setup_decode_partial_mode_disable\n");
    dcStatus = dc__jpegpipe_setup_decode_partial_mode_disable(&(dad_ptr->jpegpipe_context));
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc__jpegpipe_setup_decode_partial_mode_disable error: %#x\n",dcStatus);
		return(FALSE);
    }
#endif

    if (dad_ptr->outputColorInversion == TRUE)
    {
        DC_ACCEL_LOG("Issuing dc__jpegpipe_setup_data_range_invert\n");
        dcStatus = dc__jpegpipe_setup_data_range_invert(&(dad_ptr->jpegpipe_context));
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc__jpegpipe_setup_data_range_invert error: %#x\n",dcStatus);
			return(FALSE);
        }
    }

#if 0
// Just in case we ever add input cropping to the libjpeg API    
    if (pid->icb.enabled == TRUE)
    {
        dcStatus = dc_jpegpipe_setup_crop_set(&(dad_ptr->jpegpipe_context),pid->icb.upperLeftX,
                                                       pid->icb.upperLeftY,
                                                       pid->icb.lowerRightX,
                                                       pid->icb.lowerRightY);
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_jpegpipe_setup_crop_set error: %#x\n",dcStatus);
			return(FALSE);
        }
    }
#endif

	// Set up JPEGPIPE's CSC color table, possibly a custom color table
	
    if (dad_ptr->cscMode == DC_JPEGPIPE_CSC_MODE_CUSTOM)
    {
		if (dad_ptr->colorTable == NULL)
		{
			DC_ACCEL_LOG("colorTable NOT specified!\n");
			return(FALSE);
		}
        sprintf(dad_ptr->cct_address_string,"%-#8lX\n",(unsigned long) dad_ptr->colorTable);
        DC_ACCEL_LOG("cct_address: %s\n",dad_ptr->cct_address_string);
		DC_ACCEL_LOG("Issuing dc_csc_alloc_and_load\n");
        dcStatus = dc_csc_alloc_and_load(&(dad_ptr->cscTableData),
										 dad_ptr->cct_address_string,
										 DC_CSC_OPTIONS_ONCHIP);
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_csc_alloc_and_load error: %#x\n",dcStatus);
			return(FALSE);
        }
        else
        {
            RTS_SET_BIT(RTS_p,RTS_COLOR_TABLE_LOADED);
        }
    
		DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_color_table_set\n");
		dcStatus = dc_jpegpipe_setup_color_table_set(&(dad_ptr->jpegpipe_context),
													dad_ptr->cscMode,&(dad_ptr->cscTableData));
    	if (dcStatus != DC_SUCCESS)
    	{
        	DC_ACCEL_LOG("dc_jpegpipe_setup_color_table_set error: %#x\n",dcStatus);
			return(FALSE);
    	}
    }
	else
	{
		DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_color_table_set\n");
    	dcStatus = dc_jpegpipe_setup_color_table_set(&(dad_ptr->jpegpipe_context),dad_ptr->cscMode,NULL);
    	if (dcStatus != DC_SUCCESS)
    	{
        	DC_ACCEL_LOG("dc_jpegpipe_setup_color_table_set error: %#x\n",dcStatus);
			return(FALSE);
    	}
	}

    // Set up custom Tonal Response Curves, if specified

    if (dad_ptr->trcEnabled == TRUE)
    {
		if ((dad_ptr->trcTable[0] == NULL) ||
			(dad_ptr->trcTable[1] == NULL) ||
			(dad_ptr->trcTable[2] == NULL))
		{
			DC_ACCEL_LOG("trcTable[i] NOT specified!\n");
			return(FALSE);
		}
		DC_ACCEL_LOG("dc_jpegpipe_setup_trc_table_set\n");
        dcStatus = dc_jpegpipe_setup_trc_table_set(&(dad_ptr->jpegpipe_context),(UINT8 **) dad_ptr->trcTable);
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_jpegpipe_setup_trc_table_set error: %#x\n",dcStatus);
			return(FALSE);
        }
    }


	// Set JPEGPIPE memory profile
	
	DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_profile_set with profile: ");
    switch (dad_ptr->jpegpipe_profile)
    {
    case DC_JPEGPIPE_PROFILE_HIGH:
        DC_ACCEL_LOG("HIGH\n");
        break;
    case DC_JPEGPIPE_PROFILE_MEDIUM:
        DC_ACCEL_LOG("MEDIUM\n");
        break;
    case DC_JPEGPIPE_PROFILE_LOW:
        DC_ACCEL_LOG("LOW\n");
        break;
    default:
        DC_ACCEL_LOG("Unknown Profile: %d!\n",dad_ptr->jpegpipe_profile);
		return(FALSE);
    } 
    dcStatus = dc_jpegpipe_setup_profile_set(&(dad_ptr->jpegpipe_context),dad_ptr->jpegpipe_profile);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_jpegpipe_setup_profile_set error: %#x\n",dcStatus);
		return(FALSE);
    }

	// Call dc_jpegpipe_setup_end to get the JPEGPIPE memory requirements
	
	memset(&(dad_ptr->jpegpipe_mem_params),0,sizeof(DC_MEM_PARAMS_t));
	strcpy(((DC_MEM_PARAMS_t *) &(dad_ptr->jpegpipe_mem_params))->unprotect.name,"DC_ACCEL JPEGPIPE WORK AREA");
    
	DC_ACCEL_LOG("Issuing dc_jpegpipe_setup_end\n");
    dcStatus = dc_jpegpipe_setup_end(&(dad_ptr->jpegpipe_context),&(dad_ptr->jpegpipe_mem_params));
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_jpegpipe_setup_end error: %#x",dcStatus);
		return(FALSE);
    }

	return(TRUE);
}

static void dc_accel_jpegpipe_callback(void *arg,DC_JPEGPIPE_RESULTS_t *results)
{
	DC_ACCEL_DESC *dad_ptr = (DC_ACCEL_DESC *) arg;

    DC_ACCEL_LOG("dc_accel_jpegpipe_callback called!\n");

	// DC_ACCEL_DESC validity checks
	
	if (dad_ptr == NULL)
	{
		DC_ACCEL_LOG("Null DC_ACCEL_DESC pointer!\n");
		return;
	}

	if (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE)
	{
		DC_ACCEL_LOG("Bad DC_ACCEL_DESC signature!\n");
		return;
	}

    dad_ptr->jpegpipe_callback_triggered = TRUE;

    if (results != &(dad_ptr->jpegpipeResults))
    {
        DC_ACCEL_LOG("dc_accel_jpegpipe_callback bad DC_JPEGPIPE_RESULTS pointer: %#x\n",results);
        dad_ptr->jpegpipe_callback_error = TRUE;
    }

}

void analyze_dmbimg(DC_ACCEL_DESC *dad_ptr)
{
	DC_DMBIMG_CONSUMER consumer;
	int page_count = 0;
	DC_RETURN_CODE status;
	UINT32 validBytes;
	UINT32 validScanlines;
	UINT32 segment;
	UINT32 nBytes;
	UINT8 *byte_ptr;
	char *name;
	DC_IMAGE_CHANNELS channels=0;
	DC_IMAGE_BITDEPTH bitdepth=0;
	DC_IMAGE_FORMAT format=0;
	UINT32 stride=0;
	UINT32 width=0;
	UINT32 height=0;


	status = dc_dmbimg_info_get(&(dad_ptr->dmbimg),&name,&channels,&bitdepth,&format,&stride,&width,&height);
	if (status != DC_SUCCESS)
	{
		DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_info_get error: %#x\n",status);
		return;
	}

	DC_ACCEL_LOG("DMB_ANALYZE:  name: %s\n",name);
	DC_ACCEL_LOG("DMB_ANALYZE:  channels = %d\n",DC_IMAGE_CHANNELS_nChannels(channels));
	DC_ACCEL_LOG("DMB_ANALYZE:  bitdepth = %d\n",bitdepth);
	DC_ACCEL_LOG("DMB_ANALYZE:  format = %d\n",format);
	DC_ACCEL_LOG("DMB_ANALYZE:  stride = 0x%x\n",stride);
	DC_ACCEL_LOG("DMB_ANALYZE:  width = %d\n",width);
	DC_ACCEL_LOG("DMB_ANALYZE:  height = %d\n",height);

	status = dc_dmbimg_consumer_setup(&consumer,"analyze DMB",NULL,0);
	if (status != DC_SUCCESS)
	{
		DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_consumer_setup error: %#x\n",status);
		return;
	}

	status = dc_dmbimg_consumer_register(&(dad_ptr->dmbimg),&consumer);
	if (status != DC_SUCCESS)
	{
		DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_consumer_register error: %#x\n",status);
		return;
	}

	while(1)
	{
		status = dc_dmbimg_consumer_valid_get(&consumer,&validBytes,&validScanlines);
		if (status != DC_SUCCESS)
		{
			DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_consumer_valid_get error: %#x\n",status);
			return;
		}
		page_count++;
		DC_ACCEL_LOG("DMB_ANALYZE: page[%d], validBytes=%#x, validScanlines=%#x\n",
													page_count-1,validBytes,validScanlines);

		break;

		// TCB, 20120319 -- following code is not working as expected

		if (validBytes <= 0) break;

		status = dc_dmbimg_consumer_segment_get(&consumer,(void **) &segment,&nBytes);
		if (status != DC_SUCCESS)
		{
			DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_consumer_segment_get error: %#x\n",status);
			return;
		}
		DC_ACCEL_LOG("DMB_ANALYZE: page[%d], segment=%#x, nBytes=%#x\n",
													page_count-1,segment,nBytes);
		if (segment == 0) break;
		byte_ptr = (UINT8 *) segment;
		DC_ACCEL_LOG("DMB_ANALYZE: page[%d]: %#x %#x %#x %#x %#x %#x %#x %#x\n",page_count-1,
								byte_ptr[0],
								byte_ptr[1],
								byte_ptr[2],
								byte_ptr[3],
								byte_ptr[4],
								byte_ptr[5],
								byte_ptr[6],
								byte_ptr[7]);

		status = dc_dmbimg_consumer_consume(&consumer,nBytes,0);
		if (status != DC_SUCCESS)
		{
			DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_consumer_consume error: %#x\n",status);
			return;
		}
	}

	status = dc_dmbimg_consumer_unregister(&consumer);
	if (status != DC_SUCCESS)
	{
		DC_ACCEL_LOG("DMB_ANALYZE: dc_dmbimg_consumer_unregister error: %#x\n",status);
		return;
	}

}
