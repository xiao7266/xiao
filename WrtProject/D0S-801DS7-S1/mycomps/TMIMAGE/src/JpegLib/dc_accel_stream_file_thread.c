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
 * File Name: dc_accel_stream_file_thread.c
 * 
 * Description: Conexant proprietary extension to libjpeg library for HW
 * 				acceleration of JPEG decompression
 * 
 *----------------------------------------------------------------------------
 */

#include <string.h>
#include <stdio.h>

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
#include "dc_jpeg_api.h"
#include "dc_container_api.h"
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

boolean dc_accel_startStreamFileThread(DC_ACCEL_DESC *dad_ptr)
{
    UINT result;
    DC_RETURN_CODE dcStatus;
	RESOURCE_TEARDOWN_STATUS *RTS_p = (RESOURCE_TEARDOWN_STATUS *) &(dad_ptr->RTS);

    DC_ACCEL_LOG("dc_accel_startStreamFileThread\n");

    dad_ptr->streamFileThreadState = DASFS_INIT;
    dad_ptr->cancelStreamFileThread = FALSE;
    dad_ptr->streamFileThreadError = FALSE;
	dad_ptr->DASFSStringArray[DASFS_IDLE] = "IDLE";
	dad_ptr->DASFSStringArray[DASFS_INIT] = "INIT";
	dad_ptr->DASFSStringArray[DASFS_CHECK_SRC_SETUP] = "CHECK_SRC_SETUP";
	dad_ptr->DASFSStringArray[DASFS_CHECK_SSMD_SETUP] = "CHECK_SSMD_SETUP";
	dad_ptr->DASFSStringArray[DASFS_CHECK_FIB_SETUP] = "CHECK_FIB_SETUP";
	dad_ptr->DASFSStringArray[DASFS_CHECK_DST_SETUP] = "CHECK_DST_SETUP";
	dad_ptr->DASFSStringArray[DASFS_WAIT_OUTPUT_PAGE] = "WAIT_OUTPUT_PAGE";
	dad_ptr->DASFSStringArray[DASFS_TRANSFER_BYTES] = "TRANSFER_BYTES";
	dad_ptr->DASFSStringArray[DASFS_PRODUCE_DMB] = "PRODUCE_DMB";

	// Create the DMBIMG that the file will be streamed to JPEGPIPE in
	
	DC_ACCEL_LOG("Issuing dc_dmbimg_create\n");
    dcStatus = dc_dmbimg_create(&(dad_ptr->dmbimg),
                                "DC_ACCEL_DMBIMG",
                                (dad_ptr->jpegComponentCount == 1) ? DC_IMAGE_CHANNELS_1 : DC_IMAGE_CHANNELS_3,
                                DC_IMAGE_BITDEPTH_8,
                                DC_IMAGE_FORMAT_JPEG,
                                0,
                                dad_ptr->jpegImageWidth,
                                dad_ptr->jpegImageHeight);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_dmbimg_create error: %#x\n",dcStatus);
        return(FALSE);
    }
    else
    {
        RTS_SET_BIT(RTS_p,RTS_DC_DMBIMG_CREATED);
    }

	DC_ACCEL_LOG("Issuing dc_dmbimg_producer_setup\n");
    dcStatus = dc_dmbimg_producer_setup(&(dad_ptr->dmbimgProducer),"DC_ACCEL",0,0);
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_dmbimg_producer_setup error: %#x\n",dcStatus);
        return(FALSE);
    }

	DC_ACCEL_LOG("Issuing dc_dmbimg_producer_register\n");
    dcStatus = dc_dmbimg_producer_register(&(dad_ptr->dmbimg),&(dad_ptr->dmbimgProducer));
    if (dcStatus != DC_SUCCESS)
    {
        DC_ACCEL_LOG("dc_dmbimg_producer_register error: %#x\n",dcStatus);
        return(FALSE);
    }
    else
    {
        RTS_SET_BIT(RTS_p,RTS_DC_DMBIMG_PRODUCER_REGISTERED);
    }

	DC_ACCEL_LOG("Issuing ospl_thread_create on streamFileThread\n"); 
    result = ospl_thread_create(&(dad_ptr->streamFileThread),
                                "dasf",
                                dc_accel_streamFileThread,
                                (ULONG) dad_ptr,
                                dad_ptr->dc_accel_stream_file_thread_stack,
                                sizeof(dad_ptr->dc_accel_stream_file_thread_stack),
                                thread_priority_get(DC_THREAD_PRIORITY_MEDIUM),
                                thread_priority_get(DC_THREAD_PRIORITY_MEDIUM),
                                1,
                                OSPL_AUTO_START);
    if (result != OSPL_SUCCESS)
    {
        DC_ACCEL_LOG("ospl_thread_create error: %#x\n",result);
        return(FALSE);
    }

    return(TRUE);
}

boolean dc_accel_cancelStreamFileThread(DC_ACCEL_DESC *dad_ptr)
{
    UINT result;
    
    DC_ACCEL_LOG("dc_accel_cancelStreamFileThread\n");

    dad_ptr->cancelStreamFileThread = TRUE;

    while (dad_ptr->streamFileThreadState != DASFS_IDLE) ospl_thread_sleep(1);
    
	ospl_thread_sleep(1);
	       
	DC_ACCEL_LOG("Issuing ospl_thread_delete on streamFileThread\n"); 
    result = ospl_thread_delete(&(dad_ptr->streamFileThread));
    if (result != OSPL_SUCCESS)
    {
        DC_ACCEL_LOG("ospl_thread_delete error: %#x\n",result);
        return(FALSE);
    }

    return(TRUE);
}

void dc_accel_streamFileThread(ULONG parms)
{
    UINT result;
    DC_ACCEL_STREAM_FILE_STATUS streamFileStatus;
    boolean streamFileDone = FALSE;
    boolean stateMachineError = FALSE;
    DC_RETURN_CODE dcStatus;
	DC_ACCEL_DESC *dad_ptr = (DC_ACCEL_DESC *) parms;
	RESOURCE_TEARDOWN_STATUS *RTS_p;
    
    
	DC_ACCEL_LOG("Stream File Thread Active\n");

	// Validate DC_ACCEL_DESC pointer
	
	if ((!dad_ptr) || (dad_ptr->signature != DC_ACCEL_DESC_SIGNATURE))
	{
		DC_ACCEL_LOG("Invalid DC_ACCEL_DESC - terminating!\n");
		return;
	}

	RTS_p = (RESOURCE_TEARDOWN_STATUS *) &(dad_ptr->RTS);

    while (1)
    {
        if ((dad_ptr->cancelStreamFileThread == TRUE) ||
            (stateMachineError == TRUE) ||
            (streamFileDone == TRUE))
        {
            DC_ACCEL_LOG("Unregistering DC_DMBIMG Producer\n");
            dcStatus = dc_dmbimg_producer_unregister(&(dad_ptr->dmbimgProducer));
            if (dcStatus != DC_SUCCESS)
            {
                DC_ACCEL_LOG("dc_dmbimg_producer_unregister error: %#x\n",dcStatus);
            }
            else
            {
                RTS_CLEAR_BIT(RTS_p,RTS_DC_DMBIMG_PRODUCER_REGISTERED);
            }

            DC_ACCEL_LOG("Terminating Stream File Thread\n");
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
			while(1) ospl_thread_sleep(100);
            result = ospl_thread_terminate(&(dad_ptr->streamFileThread));
            if (result != OSPL_SUCCESS)
            {
                DC_ACCEL_LOG("ospl_thread_terminate() error: %#x\n",result);
                return;
            }
            // We shouldn't have returned. 
            DC_ACCEL_LOG("Unexpected return from ospl_thread_terminate!\n");
            return;
        }

        streamFileStatus = dc_accel_streamFileStateMachine(dad_ptr);

        switch (streamFileStatus)
        {
        case DASF_DONE:
            streamFileDone = TRUE;
            break;
            
        case DASF_CONTINUE:
            break;
            
        case DASF_SLEEP:
            ospl_thread_sleep(1);
            break;
            
        case DASF_ERROR:
            DC_ACCEL_LOG("streamFileStateMachine error!\n");
            stateMachineError = TRUE;
            dad_ptr->streamFileThreadError = TRUE;
            break;
            
        default:
            DC_ACCEL_LOG("Invalid DC_ACCEL_STREAM_FILE_STATUS: %#x\n",streamFileStatus);
            stateMachineError = TRUE;
            dad_ptr->streamFileThreadError = TRUE;
            break;
        }
    }

}

DC_ACCEL_STREAM_FILE_STATUS dc_accel_streamFileStateMachine(DC_ACCEL_DESC *dad_ptr)
{
    DC_ACCEL_STREAM_FILE_STATUS returnStatus = DASF_SLEEP;
    DC_RETURN_CODE dcStatus;
	DC_ACCEL_STREAM_FILE_THREAD_DST_DESC *dstdp = &(dad_ptr->sft_dst);
	DC_ACCEL_STREAM_FILE_THREAD_SRC_DESC *srcdp = &(dad_ptr->sft_src);
	struct jpeg_decompress_struct *ds_ptr = (struct jpeg_decompress_struct *) dad_ptr->jpeg_decompress_struct_ptr;
	DC_ACCEL_SHADOW_STREAM_MEMORY_DESC *ssmdp;
    UINT32 outputPage_nBytes;
	UINT32 bytes_to_transfer;
    UINT32 scanlinesProduced;


	// State Machine
	
    switch (dad_ptr->streamFileThreadState)
    {
    case DASFS_IDLE:
        returnStatus = DASF_DONE;
        break;

    case DASFS_INIT:
		// Set up destination descriptor
        dstdp->dmbPageSize = dc_dmb_size_get();
		dstdp->baseAddress = NULL;
		dstdp->curAddress = NULL;
        dstdp->bytesRemaining = 0;
        dstdp->lastPage = FALSE;

		// Set up source descriptor
		srcdp->bufferSize = 0;
		srcdp->baseAddress = NULL;
		srcdp->curAddress = NULL;
		srcdp->bytesRemaining = 0;
		srcdp->sawEOI = FALSE;

        returnStatus = DASF_CONTINUE;
        dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_SRC_SETUP);
        break;

	case DASFS_CHECK_SRC_SETUP:
		if ((srcdp->curAddress != NULL) && (srcdp->bytesRemaining > 0))
		{
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_DST_SETUP);
		}
		else
		{
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_SSMD_SETUP);
		}
        returnStatus = DASF_CONTINUE;
        break;

   	case DASFS_CHECK_SSMD_SETUP:
		if (dad_ptr->shadow_stream.ssmd_q.count == 0)
		{
        	returnStatus = DASF_CONTINUE;
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_FIB_SETUP);
        	break;
		}

		ssmdp = (DC_ACCEL_SHADOW_STREAM_MEMORY_DESC *) dad_ptr->shadow_stream.ssmd_q.first;
		DAQueueRemove(&(dad_ptr->shadow_stream.ssmd_q),(void *) ssmdp);
		srcdp->bufferSize = ssmdp->byte_count;
		srcdp->bytesRemaining = srcdp->bufferSize;
		srcdp->baseAddress = srcdp->curAddress = ssmdp->buffer;

        returnStatus = DASF_CONTINUE;
        dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_DST_SETUP);
        break;

   	case DASFS_CHECK_FIB_SETUP:
		if (srcdp->sawEOI == TRUE)
		{
          	DC_ACCEL_LOG("Already saw EOI, should not be looking for more data!\n");
			returnStatus = DASF_ERROR;
          	dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
          	break; 
		}

		// Zero out "fill_input_buffer" method buffer byte count, since all buffer
		// data in current buffer has already been consumed.

		ds_ptr->src->bytes_in_buffer = 0;

		DC_ACCEL_LOG("fill_input_buffer routine address: %#x\n",ds_ptr->src->fill_input_buffer);
		if (ds_ptr->src->fill_input_buffer(ds_ptr) == FALSE)
		{
			returnStatus = DASF_SLEEP;
			break;
		}

		if (ds_ptr->src->bytes_in_buffer <= 0)
		{
			DC_ACCEL_LOG("fill_input_buffer returned <= 0 bytes!\n");
			returnStatus = DASF_ERROR;
          	dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
          	break; 
		}
		else if ((ds_ptr->src->bytes_in_buffer == 2) &&
			 	 (ds_ptr->src->next_input_byte[0] == (JOCTET) 0xFF) &&
			 	 (ds_ptr->src->next_input_byte[1] == (JOCTET) JPEG_EOI))
		{
			srcdp->sawEOI = TRUE;
		}

		srcdp->bufferSize = ds_ptr->src->bytes_in_buffer;
		srcdp->bytesRemaining = srcdp->bufferSize;
		srcdp->baseAddress = (UINT8 *) ds_ptr->src->next_input_byte;
		srcdp->curAddress = srcdp->baseAddress;

        returnStatus = DASF_CONTINUE;
        dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_DST_SETUP);
        break;

	case DASFS_CHECK_DST_SETUP:
		if ((dstdp->baseAddress != NULL) &&
			(dstdp->curAddress != NULL) &&
			(dstdp->bytesRemaining > 0))
		{
        	returnStatus = DASF_CONTINUE;
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_TRANSFER_BYTES);
        	break;
		}
		else
		{
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_WAIT_OUTPUT_PAGE);
		}
        returnStatus = DASF_CONTINUE;
        break;

    case DASFS_WAIT_OUTPUT_PAGE:
        dcStatus = dc_dmbimg_producer_allocate(&(dad_ptr->dmbimgProducer),dstdp->dmbPageSize);
        if (dcStatus == DC_OUT_OF_MEMORY)
        {
			DC_ACCEL_LOG("dc_dmbimg_producer_allocate returns DC_OUT_OF_MEMORY!\n");
            returnStatus = DASF_SLEEP;
            break;
        }
        else if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_dmbimg_producer_allocate error: %#x\n",dcStatus);
            returnStatus = DASF_ERROR;
            dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
            break; 
        }
        
        dcStatus = dc_dmbimg_producer_segment_get(&(dad_ptr->dmbimgProducer),
												  (void **)&(dstdp->baseAddress),
												  &outputPage_nBytes);
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_dmbimg_producer_segment_get error: %#x\n",dcStatus);
            returnStatus = DASF_ERROR;
            dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
            break; 
        }

        if (outputPage_nBytes != dstdp->dmbPageSize)
        {
            DC_ACCEL_LOG("outputPage_nBytes (%#x) != dmbPageSize (%#x)\n",outputPage_nBytes,dstdp->dmbPageSize);
            returnStatus = DASF_ERROR;
            dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
            break; 
        }

		dstdp->curAddress = dstdp->baseAddress;
		dstdp->bytesRemaining = dstdp->dmbPageSize;
        
        returnStatus = DASF_CONTINUE;
        dc_accel_changeStreamFileState(dad_ptr,DASFS_TRANSFER_BYTES);
        break;

    case DASFS_TRANSFER_BYTES:
		DC_ACCEL_LOG("srcdp->bytesRemaining: %#x, dstdp->bytesRemaining: %#x\n",
										srcdp->bytesRemaining,dstdp->bytesRemaining);
		bytes_to_transfer = min(srcdp->bytesRemaining,dstdp->bytesRemaining);

		DC_ACCEL_LOG("memcpy: %#x -> %#x, %#x bytes\n",srcdp->curAddress,dstdp->curAddress,bytes_to_transfer);
		memcpy(dstdp->curAddress,srcdp->curAddress,bytes_to_transfer);

		// Update source and destination descriptors
		srcdp->curAddress += bytes_to_transfer;
		srcdp->bytesRemaining -= bytes_to_transfer;
		if (srcdp->bytesRemaining == 0)
		{
			srcdp->baseAddress = srcdp->curAddress = NULL;
		}
		dstdp->curAddress += bytes_to_transfer;
		dstdp->bytesRemaining -= bytes_to_transfer;
		dstdp->lastPage = srcdp->sawEOI;

		if (dstdp->lastPage == TRUE)
		{
        	returnStatus = DASF_CONTINUE;
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_PRODUCE_DMB);
        	break;
		}

		if (dstdp->bytesRemaining > 0)
		{
        	returnStatus = DASF_CONTINUE;
        	dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_SRC_SETUP);
        	break;
		}

        returnStatus = DASF_CONTINUE;
        dc_accel_changeStreamFileState(dad_ptr,DASFS_PRODUCE_DMB);
        break;

	case DASFS_PRODUCE_DMB:
        // Compressed data, unclear how many scanlines until finished
        scanlinesProduced = (dstdp->lastPage == TRUE) ? dad_ptr->jpegImageHeight : 0;
       
	   	{
			UINT8 *byte_ptr = dstdp->baseAddress;

	   		DC_ACCEL_LOG("dc_dmbimg_producer_produce: %#x: %#x %#x %#x %#x\n",
										byte_ptr,byte_ptr[0],byte_ptr[1],byte_ptr[2],byte_ptr[3]);
		}

        dcStatus = dc_dmbimg_producer_produce(&(dad_ptr->dmbimgProducer),
											  (dstdp->dmbPageSize - dstdp->bytesRemaining),
											  scanlinesProduced);
        if (dcStatus != DC_SUCCESS)
        {
            DC_ACCEL_LOG("dc_dmbimg_producer_produce error: %#x\n",dcStatus);
            returnStatus = DASF_ERROR;
            dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
            break; 
        }
        
		// Clear destination status
		dstdp->baseAddress = dstdp->curAddress = NULL;
		dstdp->bytesRemaining = 0;

        if (dstdp->lastPage == TRUE)
        {
            DC_ACCEL_LOG("Conversion of file to DMBIMG stream done\n");
            returnStatus = DASF_DONE;
            dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
            break;
        }

        dc_accel_changeStreamFileState(dad_ptr,DASFS_CHECK_SRC_SETUP);
        returnStatus = DASF_CONTINUE;
        break;

    default:
        DC_ACCEL_LOG("Invalid DC_ACCEL_STREAM_FILE_STATE: %#x!\n",dad_ptr->streamFileThreadState);
        dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
        returnStatus = DASF_ERROR;
        break;
    }

    return(returnStatus);
}

void dc_accel_changeStreamFileState(DC_ACCEL_DESC *dad_ptr,DC_ACCEL_STREAM_FILE_STATE state)
{

    if (state >= DASFS_COUNT)
    {
        DC_ACCEL_LOG("Bad DC_ACCEL_STREAM_FILE_STATE: %#x!\n",state);
        dc_accel_changeStreamFileState(dad_ptr,DASFS_IDLE);
        return;
    }

    DC_ACCEL_LOG("DASFS: %s\n",dad_ptr->DASFSStringArray[state]);
    dad_ptr->streamFileThreadState = state;

}



