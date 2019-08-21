/*
 * Copyright (C) 2000,2001,2002,2003   Koninklijke Philips Electronics N.V.   All Rights Reserved.
 *
 * this source code and any compilation or derivative thereof is the proprietary information 
 * of Koninklijke Philips Electronics N.V. and is confidential in nature. Under no circumstances 
 * is this software to be exposed to or placed under an Open Source License of any type without 
 * the expressed written permission of Koninklijke Philips Electronics N.V.
 *
 */

#ifndef HTTPCLIENT_H 
#define HTTPCLIENT_H 

#if defined(__cplusplus)
extern "C"
{
#endif        


int  HttpClientInit();
unsigned long   HttpClientGetContentLength(int fd);
int  HttpClientOpen(char *url); //return fd if(fd < 0)error;
int  HttpClientRead(int fd, char *buf, int len);
int  HttpClientClose(int fd);

#ifdef __cplusplus
}
#endif

#endif /* TMHTTPIODRIVER_IOD_H  */

