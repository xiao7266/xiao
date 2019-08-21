/*
 * g711.h
 *
 * Header file for CCITT conversion routines.
 *
 */

#include "math.h"

#ifndef _G711_H
#define	_G711_H

#if defined(__cplusplus)
extern "C" {
#endif

int linear2alaw(int	pcm_val);

int alaw2linear(int	a_val);

int linear2ulaw( int	pcm_val);

int ulaw2linear( int	u_val);

#if defined(__cplusplus)
}
#endif

#endif /* !_G72X_H */

