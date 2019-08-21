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
 * File Name: dc_decompress_test.h
 * 
 * Description: Application that tests the libjpeg API
 * 
 *----------------------------------------------------------------------------
 */

#ifndef _dc_decompression_test_h_
#define _dc_decompression_test_h_
/******************************************************************************
*  1. INCLUDES
******************************************************************************/

/******************************************************************************
*  2. ENUMERATIONS
******************************************************************************/

typedef enum DCDT_MODE
{
	DCDT_MODE_DUMP_TO_FILE = 0,
	DCDT_MODE_NO_OUTPUT = 1,
	DCDT_MODE_SCREEN_OUTPUT = 2,
} DCDT_MODE;

/******************************************************************************
*  3. DEFINES
******************************************************************************/

typedef UINT32 RESOURCE_TEARDOWN_STATUS;

typedef union DWORD_UNION
{
	UINT32 dword;
	UINT8 b[4];
} DWORD_UNION;

#define RTS_FB_OPENED						0x00000001
#define RTS_FB_MAPPED						0x00000002
#define RTS_OUTPUT_FILE_OPEN				0x00000004
#define RTS_IMAGE_FILE_OPEN					0x00000008
#define RTS_DECOMP_STRUCT_INIT				0x00000010

#define RTS_SET_BIT(statusPtr,bitMask) \
*statusPtr |= bitMask

#define RTS_CLEAR_BIT(statusPtr,bitMask) \
*statusPtr &= ~bitMask

#define RTS_BIT_IS_SET(statusPtr,bitMask) \
((*statusPtr & bitMask) ? TRUE : FALSE)

#define RTS_INIT(statusPtr) \
*statusPtr = 0

#define MAX_IMAGE_FILE_COUNT	2000
#define MAX_IMAGE_FILE_LEN		256
#define MAX_LINE_WIDTH_BYTES	(16 * 1024 * 4)
#define MAX_FB_DIMENSION		2048

#define DUPLICATE_OPTION_ERROR(s)									\
{ 																	\
printf("\nParse Error: Option \"%s\" already specified!\n\n",(s));	\
return(1);															\
}

#define INVALID_PARAMETER_ERROR(s)										\
{ 																		\
printf("\nParse Error: Option \"%s\" has an invalid parameter!\n\n",(s));	\
return(1);																\
}

#define CHECK_SIGNAL_RECEIVED()										\
{																	\
	int signalReceived;												\
	if ((signalReceived = ospl_check_signal()) != 0)				\
	{																\
		printf("\nSignal %d Received!\n\n",signalReceived);			\
		teardownResources();										\
		exit(-1);													\
	}																\
}

/******************************************************************************
*  4. STRUCTURES
******************************************************************************/

typedef struct ERROR_HANDLER_DESC
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
} ERROR_HANDLER_DESC;

typedef struct FB_DESC
{
	UINT32 base;
	int yoffset;
} FB_DESC;

/******************************************************************************
*  5. GLOBALS
******************************************************************************/

/******************************************************************************
*  6. PUBLIC FUNCTION PROTOTYPES
******************************************************************************/

#endif

