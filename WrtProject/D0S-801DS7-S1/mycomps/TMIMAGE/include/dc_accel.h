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
 * File Name: dc_accel.h
 * 
 * Description: Conexant proprietary extension to libjpeg library for HW
 * 				acceleration of JPEG decompression
 * 
 *----------------------------------------------------------------------------
 */

#ifndef _dc_accel_h_
#define _dc_accel_h_
/******************************************************************************
*  1. INCLUDES
******************************************************************************/

#include "dc_accel_queue.h"

/******************************************************************************
*  2. ENUMERATIONS
******************************************************************************/

typedef enum DC_ACCEL_STATE
{
	DC_ACCEL_STATE_UNKNOWN = 0,
	DC_ACCEL_STATE_INIT = 1,
	DC_ACCEL_STATE_CONSIDER = 2,
	DC_ACCEL_STATE_START = 3,
} DC_ACCEL_STATE;

typedef enum DC_ACCEL_STREAM_FILE_STATUS
{
    DASF_DONE = 0,
    DASF_CONTINUE,
    DASF_SLEEP,
    DASF_ERROR,
} DC_ACCEL_STREAM_FILE_STATUS;

typedef enum DC_ACCEL_STREAM_FILE_STATE
{
    DASFS_IDLE = 0,
    DASFS_INIT,
	DASFS_CHECK_SRC_SETUP,
	DASFS_CHECK_SSMD_SETUP,
	DASFS_CHECK_FIB_SETUP,
	DASFS_CHECK_DST_SETUP,
    DASFS_WAIT_OUTPUT_PAGE,
    DASFS_TRANSFER_BYTES,
	DASFS_PRODUCE_DMB,
    DASFS_COUNT,        // always last
} DC_ACCEL_STREAM_FILE_STATE;

/******************************************************************************
*  3. DEFINES
******************************************************************************/

#if defined(ENABLE_DC_ACCEL_LOGGING)
#define DC_ACCEL_LOG(fmt,args...) {printf("DC_ACCEL: ");printf(fmt,##args);}
#else
#define DC_ACCEL_LOG(fmt,args...)
#endif

#define DC_ACCEL_DESC_SIGNATURE  ((UINT32) (('D' << 24) | ('A') << 16) | ('D' << 8) | ('S' << 0))

#define DMB_PAGE_SIZE			(32*1024)

#define JPEGPIPE_CM_MASK		(DC_CM_MASK(DC_CM_JH) | \
								 DC_CM_MASK(DC_CM_JP) | \
								 DC_CM_MASK(DC_CM_RO) | \
								 DC_CM_MASK(DC_CM_IP))

typedef UINT32 RESOURCE_TEARDOWN_STATUS;

#define RTS_DCAPI_OPENED					0x00000001
#define RTS_DCAPI_MAPPED					0x00000002
#define RTS_DCAPI_CM_RUNNING 				0x00000004
#define RTS_SF_THREAD_RUNNING				0x00000008
#define RTS_DC_DMBIMG_PRODUCER_REGISTERED	0x00000010
#define RTS_DC_DMBIMG_CREATED				0x00000020
#define RTS_DC_IMGBUF_CONSUMER_REGISTERED	0x00000040
#define RTS_DC_IMGBUF_CREATED				0x00000080
#define RTS_COLOR_TABLE_LOADED				0x00000100
#define RTS_JPEGPIPE_THREAD_RUNNING			0x00000200

#define RTS_SET_BIT(statusPtr,bitMask) \
*statusPtr |= bitMask

#define RTS_CLEAR_BIT(statusPtr,bitMask) \
*statusPtr &= ~bitMask

#define RTS_BIT_IS_SET(statusPtr,bitMask) \
((*statusPtr & bitMask) ? TRUE : FALSE)

#define RTS_INIT(statusPtr) \
*statusPtr = 0

#define DC_ACCEL_INTERNAL_IMGBUF_SIZE			0x100000
#define DC_ACCEL_INTERNAL_IMGBUF_ALIGN			5
#define DC_ACCEL_JPEGPIPE_WORK_AREA_SIZE		0x600000
#define DC_ACCEL_JPEGPIPE_WORK_AREA_ALIGN		5

#define CONJH_FIRST_DT_ENGINE       DC_DT_ENGINE_2
#define CONJH_SECOND_DT_ENGINE      DC_DT_ENGINE_3
#define CONJH_MI_RD_CHANNEL         DC_MI_RD_6
#define CONJH_MI_WR_CHANNEL         DC_MI_WR_6

#define ALIGNED32(x)		((x) & ~0x1F)
#define DEFAULT_STACK_BYTES (16 * 1024)

#define IMGBUF_STALL_TIMEOUT            		3000       	// in milliseconds

#define DC_ACCEL_CANCEL_JPEGPIPE_TIMEOUT     	1000        // in milliseconds

/******************************************************************************
*  4. STRUCTURES
******************************************************************************/

typedef struct DC_ACCEL_STREAM_FILE_THREAD_DST_DESC
{
	UINT32 dmbPageSize;
	UINT8 *baseAddress;
	UINT8 *curAddress;
	UINT32 bytesRemaining;
	boolean lastPage;
} DC_ACCEL_STREAM_FILE_THREAD_DST_DESC;

typedef struct DC_ACCEL_STREAM_FILE_THREAD_SRC_DESC
{
	UINT32 bufferSize;
	UINT8 *baseAddress;
	UINT8 *curAddress;
	UINT32 bytesRemaining;
	boolean sawEOI;
} DC_ACCEL_STREAM_FILE_THREAD_SRC_DESC;

typedef struct DC_ACCEL_SHADOW_STREAM_DESC
{
	boolean errorEncountered;
	DA_QUEUE_DESC ssmd_q;
	UINT32 total_byte_count;
} DC_ACCEL_SHADOW_STREAM_DESC;

typedef struct DC_ACCEL_SHADOW_STREAM_MEMORY_DESC
{
	DA_QUEUE_LINK ql;
	UINT32 byte_count;
	UINT8 *buffer;
} DC_ACCEL_SHADOW_STREAM_MEMORY_DESC;

typedef struct DC_ACCEL_DESC
{
	UINT32 signature;
	DC_ACCEL_STATE state;
	boolean accelerate_decompress_job;
	RESOURCE_TEARDOWN_STATUS RTS;
	HDCAPI dcapiHandle;
	dc_mm_sys_config_t systemConfig;
	void *virtual_io_base;
	void *virtual_cached_base;
	void *virtual_uncached_base;
    DC_RAMMGR_BUFFER_ENTRY csc_rammgr_entry;
	DC_MEM_PARAMS_t csc_mem_params;
    DC_RAMMGR_BUFFER_ENTRY jpegpipe_rammgr_entry;
    DC_MEM_PARAMS_t jpegpipe_work_area_mem_params;
    DC_RAMMGR_BUFFER_ENTRY imgbuf_rammgr_entry;
    DC_MEM_PARAMS_t imgbuf_mem_params;
	UINT32 dc_accel_stream_file_thread_stack[DEFAULT_STACK_BYTES/4];
    OSPL_THREAD streamFileThread;
    boolean cancelStreamFileThread;
    boolean streamFileThreadError;
    DC_ACCEL_STREAM_FILE_STATE streamFileThreadState;
	void *jpeg_decompress_struct_ptr;
    UINT32 jpegComponentCount;
    UINT32 jpegImageWidth;
    UINT32 jpegImageHeight;
    DC_DMBIMG dmbimg;
    DC_DMBIMG_PRODUCER dmbimgProducer;
    DC_IMGBUF imgbuf;
    DC_IMGBUF_CONSUMER imgbufConsumer;
	char *DASFSStringArray[DASFS_COUNT];
	DC_ACCEL_STREAM_FILE_THREAD_DST_DESC sft_dst;
	DC_ACCEL_STREAM_FILE_THREAD_SRC_DESC sft_src;
    DC_JPEGPIPE_CONTEXT_t jpegpipe_context;
	boolean outputColorInversion;
    DC_JPEGPIPE_PROFILE_t jpegpipe_profile;
    DC_MEM_PARAMS_t jpegpipe_mem_params;
    // Color Table 
    // Set cscMode to DC_JPEGPIPE_CSC_MODE_DEFAULT for default built-in tables
    // Set cscMode to DC_JPEGPIPE_CSC_MODE_NONE for native YCBCr output
    // Set cscMode to DC_JPEGPIPE_CSC_MODE_CUSTOM for a user-supplied table
    // If DC_JPEGPIPE_CSC_MODE_CUSTOM, set colorTable to custom 9x9x9x4 color table, compressed using dc_ctzip.exe
	DC_JPEGPIPE_CSC_MODE_t cscMode;
    UINT8 *colorTable;
	DC_CONIP_CSC_TABLE_DATA cscTableData;
    char cct_address_string[16];
    // Tonal Response Curves
    // 65 bytes: [0,...,64] map to [0,...,255], HW performs linear interpolation
    // If trcEnabled is TRUE, then trcTable[3] must have pointers to TRC tables
    // If trcEnabled is FALSE, then trcTable[3] should be NULL
    boolean trcEnabled;
    UINT8 *trcTable[3];
    UINT32 imgbuf_width;
    UINT32 imgbuf_height;
    UINT32 imgbuf_base;
    UINT32 imgbuf_stride;
    UINT32 imgbuf_ySize;
    DC_IMAGE_CHANNELS imgbuf_channels;
    DC_IMAGE_BITDEPTH imgbuf_bitdepth;
    boolean jpegpipe_callback_triggered;
    boolean jpegpipe_callback_error;
    UINT32 timerStartImgBufStall;
	UINT32 timerEndImgBufStall;
	int scanlinesConsumed;
    DC_JPEGPIPE_STATUS_t jpegpipeStatus;
    DC_JPEGPIPE_ERROR_t jpegpipeError;
    DC_JPEGPIPE_RESULTS_t jpegpipeResults;
	DC_ACCEL_SHADOW_STREAM_DESC shadow_stream;
} DC_ACCEL_DESC;


/******************************************************************************
*  5. GLOBALS
******************************************************************************/

/******************************************************************************
*  6. LOCAL FUNCTION PROTOTYPES
******************************************************************************/
// Prototypes for functions in dc_accel_stream_file_thread.c

boolean dc_accel_startStreamFileThread(DC_ACCEL_DESC *dad_ptr);
boolean dc_accel_cancelStreamFileThread(DC_ACCEL_DESC *dad_ptr);
void dc_accel_streamFileThread(ULONG parms);
DC_ACCEL_STREAM_FILE_STATUS dc_accel_streamFileStateMachine(DC_ACCEL_DESC *dad_ptr);
void dc_accel_changeStreamFileState(DC_ACCEL_DESC *dad_ptr,DC_ACCEL_STREAM_FILE_STATE state);


#endif

