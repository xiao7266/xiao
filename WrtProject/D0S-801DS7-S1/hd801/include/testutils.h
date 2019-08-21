#ifndef _TESTUTILS_H
#define _TESTUTILS_H
/*
 *  tslib/tests/testutils.h
 *
 *  Copyright (C) 2004 Michael Opdenacker <michaelo@handhelds.org>
 *
 * This file is placed under the LGPL.
 *
 * $Id: testutils.h
 *
 * Misc utils for ts test programs
 */
#ifdef __cplusplus
	extern "C"{
#endif

void getxy(struct tsdev *ts, int *x, int *y);
void ts_flush (struct tsdev *ts);
void showtscalinfo();
#ifdef __cplusplus
}
#endif

#endif /* _TESTUTILS_H */
