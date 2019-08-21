/** 
 * Function prototypes for dealing with images using libjpeg.
 *
 * @author Tomas Berndtsson <tomas@nocrew.org>
 */

#ifndef _IMAGE_JPEG_H_
#define _IMAGE_JPEG_H_

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
#include "image.h"

/* Function prototype. */

#ifdef __cplusplus
extern "C"{
#endif

struct image_data *iptv_image_jpeg_read(unsigned char *buf, long length);
struct image_data *iptv_image_jpeg_read2(FILE* file);


int iptv_image_write_jpeg(struct image_data *picture,unsigned char *outputbuf,int quality,int bits);

int iptv_image_write_jpeg_420(unsigned char *y,unsigned char *u,unsigned char *v,int width,int height,unsigned char *outputbuf,int quality,int destwidth);

#ifdef __cplusplus
}
#endif

#endif /* _IMAGE_JPEG_H_ */
