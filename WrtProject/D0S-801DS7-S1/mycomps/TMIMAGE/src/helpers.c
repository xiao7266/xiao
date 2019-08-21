/**
 * Helper functions when dealing with images. This includes functions
 * that helps converting images into the correct format for the current
 * display. 
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

#include <stdlib.h>

#include "image.h"

/**
 * Find the colour index corresponding to the given colours.
 * This does not try to find the closest match, but rather the exact
 * match, because the image is prepared to match the colourmap used.
 *
 * @param red The red component of the colour to find.
 * @param green The green component of the colour to find.
 * @param blue The blue component of the colour to find.
 *
 * @return the colour index matching the given colour, or a negative value
 * @return if the colour could not be found, which should not happen.
 */
/*static inline int find_colour_index(int red, int green, int blue)*/

/**
 * Convert one colour into the best matching colour for the current
 * bit depth used.
 *
 * This is declared as an inline function to improve performance.
 *
 * @param datap A pointer to the place where the new colour will be
 * @param datap stored.
 * @param red The red element of the colour.
 * @param green The green element of the colour.
 * @param blue The blue element of the colour.
 *
 * @return the number of bytes to step ahead to get to the next pixel.
 */
int iptv_image_get_real_colour(unsigned char *datap, unsigned char red, 
			  unsigned char green, unsigned char blue)
{
    return 1;
}

