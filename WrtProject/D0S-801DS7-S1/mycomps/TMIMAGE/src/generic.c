/**
 * Functions to generically deal with images. This exists only to
 * delegate the actual work to the appropriate image handlers.
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

#include "image.h"
#include "jpeg.h"
//#include "pngs.h"
//#include "iptv_gif.h"
#include "magick.h"

/**
 * This will not read from the stream given to it, but will examine the
 * MIME content type string given to it, and pass the file descriptor for
 * the stream on to the correct image handler, which will read and convert
 * the image into the format that the user interface wants.
 *
 * @param fd The file descriptor for the image stream.
 * @param width The requested width of the image, or a negative value if the
 * @param width width is to be taken from the image.
 * @param height The requested height of the image, or a negative value if the
 * @param height height is to be taken from the image.
 *
 * @return an allocated struct containing the converted image and information
 * @return about it, or NULL if an error occurred.
 */

struct image_data *iptv_image_open2(FILE *file)
{
	return iptv_image_jpeg_read2(file);
}

struct image_data *iptv_image_open(unsigned char *buf, long length, char *type)
{
    struct image_data *picture;
    
    picture = NULL;
    
    if(!strcmp(type, "jpeg"))
    {
        picture = iptv_image_jpeg_read(buf, length);
        return picture;
    }
    /*
    if(!strcmp(type, "gif"))
    {
        picture = iptv_image_gif_read(buf, length); 
        return picture;
    }
    */
    return picture;
}

/**
 * Frees an image that has been allocated by one of the image handlers.
 *
 * @param picture A pointer to the image data to be freed.
 *
 * @return a non-zero value if an error occurred.
 */
int image_free(struct image_data *picture)
{
    if(picture)
    {
        if(picture->data) 
            free(picture->data);
        free(picture);
    }
    return 0;
}
