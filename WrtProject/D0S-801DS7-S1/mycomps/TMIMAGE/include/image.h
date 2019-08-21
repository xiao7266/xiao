/** 
 * Structs and function prototypes for dealing with images, loading them
 * and converting them.
 *
 * @author Tomas Berndtsson <tomas@nocrew.org>
 */

#ifndef _IMAGE_IMAGE_H_
#define _IMAGE_IMAGE_H_

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

#include <sys/types.h>
#include <stdio.h>
/**
 * Consists of whatever information the user interface might need to
 * know about the image to show.
 *
 * @member width The width of the image in pixels.
 * @member height The height of the image in pixels.
 * @member size The size of the image in bytes.
 * @member data A pointer to the actual image data.
 */
struct image_data
{
    int width;
    int height;
    int transparent;
    int delay;
    size_t size;
    unsigned char *data;
    struct image_data *next;
};
#ifdef __cplusplus
extern "C"{
#endif	

/* Function prototype. */
struct image_data *iptv_image_open(unsigned char *buf, long length, char *type);
struct image_data *iptv_image_open2(FILE *file);
struct SDL_Surface * iptv_image_jpeg_read_sdl(unsigned char *buf, long length);

/* Helper function. */
int iptv_image_get_real_colour(unsigned char *datap, unsigned char red, 
				 unsigned char green, unsigned char blue);
#ifdef __cplusplus
}
#endif					 

#endif /* _IMAGE_IMAGE_H_ */
