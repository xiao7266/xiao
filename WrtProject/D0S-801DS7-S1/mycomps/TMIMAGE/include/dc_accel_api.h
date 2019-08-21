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
 * File Name: dc_accel_api.h
 * 
 * Description: Conexant proprietary extension to libjpeg library for HW
 * 				acceleration of JPEG decompression
 * 
 *----------------------------------------------------------------------------
 */

#ifndef _dc_accel_api_h_
#define _dc_accel_api_h_

/******************************************************************************
*  1. INCLUDES
******************************************************************************/

#include "jpeglib.h"

/******************************************************************************
*  2. ENUMERATIONS
******************************************************************************/
/******************************************************************************
*  3. DEFINES
******************************************************************************/
/******************************************************************************
*  4. STRUCTURES
******************************************************************************/
/******************************************************************************
*  5. GLOBALS
******************************************************************************/
/******************************************************************************
*  6. PUBLIC FUNCTION PROTOTYPES
******************************************************************************/

void dc_accel_init(struct jpeg_decompress_struct *ds_ptr);
void dc_accel_init_file_handle(struct jpeg_decompress_struct *ds_ptr,FILE *file_handle);
void dc_accel_consider_job(struct jpeg_decompress_struct *ds_ptr);
boolean dc_accel_accelerate(struct jpeg_decompress_struct *ds_ptr);
void dc_accel_start(struct jpeg_decompress_struct *ds_ptr);
void dc_accel_read_scanlines(struct jpeg_decompress_struct *ds_ptr,
								JSAMPARRAY scanlines,
								JDIMENSION *row_count_ptr,
								JDIMENSION max_lines);
void dc_accel_teardown_resources(struct jpeg_decompress_struct *ds_ptr);
void dc_accel_shadow_stream(struct jpeg_decompress_struct *ds_ptr);
void dc_accel_skip_input_data(struct jpeg_decompress_struct *ds_ptr,long num_bytes);

#endif

