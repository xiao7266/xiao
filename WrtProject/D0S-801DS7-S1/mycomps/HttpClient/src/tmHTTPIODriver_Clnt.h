/*
 * Copyright (C) 2000,2001,2002,2003   Koninklijke Philips Electronics N.V.   All Rights Reserved.
 *
 * this source code and any compilation or derivative thereof is the proprietary information 
 * of Koninklijke Philips Electronics N.V. and is confidential in nature. Under no circumstances 
 * is this software to be exposed to or placed under an Open Source License of any type without 
 * the expressed written permission of Koninklijke Philips Electronics N.V.
 *
 */

#ifndef TMHTTPIODRIVER_CLNT_H
#define TMHTTPIODRIVER_CLNT_H

#ifdef __cplusplus
extern "C" {
#endif



/*****************************************************************************
 * 
 * includes
 *
 *****************************************************************************/



/*****************************************************************************
* 
* includes
*
*****************************************************************************/
#include <sys/stat.h>
#include <fcntl.h>


typedef void            Void;       /* Void (typeless) */
typedef signed   char   Int8;       /*  8-bit   signed integer */
typedef signed   short  Int16;      /* 16-bit   signed integer */
typedef signed   long   Int32;      /* 32-bit   signed integer */
typedef unsigned char   UInt8;      /*  8-bit unsigned integer */
typedef unsigned short  UInt16;     /* 16-bit unsigned integer */
typedef unsigned long   UInt32;     /* 32-bit unsigned integer */
typedef float           Float;      /* 32-bit floating point */
typedef unsigned int    Bool;       /* Boolean (True/False) */
typedef char            Char;       /* character, character array ptr */
typedef int             Int;        /* machine-natural integer */
typedef unsigned int    UInt;       /* machine-natural unsigned integer */

typedef UInt32 tmErrorCode_t;



/*****************************************************************************
* 
* defines
*
*****************************************************************************/

#define False  0
#define True   1

#define TM_HTTPIODRIVER_SENDBUF_SIZE   0/*(16 * 1024)*/
#define TM_HTTPIODRIVER_RECVBUF_SIZE   0/*(16 * 1024)*/

/* seek constants */
#define TM_HTTPIODRIVER_SEEK_SET 0
#define TM_HTTPIODRIVER_SEEK_CUR 1
#define TM_HTTPIODRIVER_SEEK_END 2

/* io control commands */
#define TM_HTTPIODRIVER_GET_HTTPSTATUS         8800

/* error codes */

#define TM_ERR_HTTPIODRIVER_BASE			(0x1000) 
#define TM_ERR_HTTPIODRIVER_INSTALL_FAILED		( TM_ERR_HTTPIODRIVER_BASE + 0x001 )
#define TM_ERR_HTTPIODRIVER_DNS_FAILED			( TM_ERR_HTTPIODRIVER_BASE + 0x002 )
#define TM_ERR_HTTPIODRIVER_CONNECT_FAILED		( TM_ERR_HTTPIODRIVER_BASE + 0x003 )  /* Server is unreachable */
#define TM_ERR_HTTPIODRIVER_CONNECT_LOST		( TM_ERR_HTTPIODRIVER_BASE + 0x004 )

#define TM_ERR_HTTPIODRIVER_FAILED				-1

/*****************************************************************************
 * 
 * defines
 *
 *****************************************************************************/
/* sizes for strings */
#define HTTPCLIENT_SERVER_SIZE      128
#define HTTPCLIENT_USER_SIZE        64
#define HTTPCLIENT_URL_SIZE         1024
#define HTTPCLIENT_VERSION_SIZE     16
#define HTTPCLIENT_HEADER_SIZE      1024
#define HTTPCLIENT_AGENT_SIZE       64
#define HTTPCLIENT_ACCEPT_SIZE      128
#define HTTPCLIENT_METHOD_SIZE      (1024 + 32)
#define HTTPCLIENT_STATUS_SIZE      128
#define HTTPCLIENT_RESPONS_VALUE_SIZE       64

#define HTTPCLIENT_SO_RCVBUF        (32 * 1024)

/*****************************************************************************
 * 
 * typedefs
 *
 *****************************************************************************/
typedef enum {
  tmHTTPIODriver_GetMethod = 0,
  tmHTTPIODriver_HeadMethod = 1,
  tmHTTPIODriver_PostMethod = 2,
  tmHTTPIODriver_PutMethod = 3
} tmHTTPIODriver_Method_t;

typedef enum {
  tmHTTPIODriver_HTTP09 = 0,
  tmHTTPIODriver_HTTP10,
  tmHTTPIODriver_SHOUTCAST,
  tmHTTPIODriver_HTTP11
} tmHTTPIODriver_Version_t;

typedef enum {
  tmHTTPIODriver_ClientIdle = 0,
  tmHTTPIODriver_ClientBusy
} tmHTTPIODriver_State_t;

typedef enum
{
    tmHTTPIODriver_RangeToEndOfFile,    
    tmHTTPIODriver_RangeBlock
}tmHTTPIODriver_RangeMode_t;



typedef struct 
{
  Int			s;

  /* proxy */
  Int           proxyUse;
  Char          proxyServer[HTTPCLIENT_SERVER_SIZE];
  Int           proxyPort;
  Int           proxyUseAuth;
  Char          proxyUser[HTTPCLIENT_USER_SIZE];
  Char          proxyPasswd[HTTPCLIENT_USER_SIZE];
  Int			DisableProxyCache;
  
  /* server */
  Char          httpServer[HTTPCLIENT_SERVER_SIZE];
  Int           httpPort;
  Int           httpUseAuth;
  Char          httpUser[HTTPCLIENT_USER_SIZE];
  Char          httpPasswd[HTTPCLIENT_USER_SIZE];

  /* request */
  tmHTTPIODriver_State_t httpRequestState;
  tmHTTPIODriver_Version_t httpRequestVersion;
  Int           httpRequestProtocol;
  Char          httpRequestMethod[HTTPCLIENT_METHOD_SIZE];
  Char          httpRequestResource[HTTPCLIENT_URL_SIZE];
  Char          httpRequestDefaultHeaders[HTTPCLIENT_HEADER_SIZE];
  Char          httpRequestHeaders[HTTPCLIENT_HEADER_SIZE];
  Char          httpRequestUserAgent[HTTPCLIENT_AGENT_SIZE];
  Char          httpRequestAccept[HTTPCLIENT_ACCEPT_SIZE];
  Char          httpRequestUrl[HTTPCLIENT_URL_SIZE];
  Int           httpRequestPersistent;
  Int           httpRequestChunked;

  Int           httpRequestRangeLow;
  Int           httpRequestRangeHigh;
  Int           httpRequestRange;

  
  /* response */
  tmHTTPIODriver_State_t httpResponseState;
  tmHTTPIODriver_Version_t httpResponseVersion;
  Char          httpResponseStatusString[HTTPCLIENT_STATUS_SIZE];
  Int           httpResponseStatusCode;
  Char          httpResponseHeaders[HTTPCLIENT_HEADER_SIZE];
  Int           httpResponseChunked;
  Int           httpResponseChunkSize, httpResponseChunkPos;

  /* buffers */
  Char *        rBuf;
  Int           rBufSize;
  Int			rBufOffset;
  Int			rBufNumBytes;
  Int			rBufAllocated;
  Char *        sBuf;
  Int           sBufSize;
  Int			sBufNumBytes;
  Int			sBufAllocated;
} tmHTTPIODriver_Client_t;

/*****************************************************************************
 * 
 * prototypes
 *
 *****************************************************************************/

void	tmHTTPIODriver_ClientRegister(void);
void	tmHTTPIODriver_ClientDeRegister(void);
Int		tmHTTPIODriver_ClientCreate(tmHTTPIODriver_Client_t *hc, Char *rBuf, Int rBufSize, Char *sBuf,Int sBufSize);
Int		tmHTTPIODriver_ClientClose(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientConnect(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientIsConnected(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientDisableProxyCache(tmHTTPIODriver_Client_t *hc, Int disable);
Int		tmHTTPIODriver_ClientSetGlobalTimeOuts( Int connect, Int send, Int recv);
Int		tmHTTPIODriver_ClientSetProxy(tmHTTPIODriver_Client_t *hc, Char *proxy, Int port, Int useProxy);
Int		tmHTTPIODriver_ClientSetProxyAuth(tmHTTPIODriver_Client_t *hc, Char *user, Char *passwd, Int useProxyAuth);
Int		tmHTTPIODriver_ClientSetUrl(tmHTTPIODriver_Client_t *hc, Char *url);
Int		tmHTTPIODriver_ClientSetRequestVersion(tmHTTPIODriver_Client_t *hc, tmHTTPIODriver_Version_t httpVersion);
Int		tmHTTPIODriver_ClientSetServerAuth(tmHTTPIODriver_Client_t *hc, Char *user, Char *passwd, Int useAuth);
Int		tmHTTPIODriver_ClientClearHeaders(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientSetChunked(tmHTTPIODriver_Client_t *hc, Int useChunked);
Int		tmHTTPIODriver_ClientSetUserAgent(tmHTTPIODriver_Client_t *hc, Char *agent);
Int		tmHTTPIODriver_ClientSetAccept(tmHTTPIODriver_Client_t *hc, Char *accept);
Int		tmHTTPIODriver_ClientSetPersistentConnection(tmHTTPIODriver_Client_t *hc, Int usePersistent);
Int		tmHTTPIODriver_ClientSetRange(tmHTTPIODriver_Client_t *hc, Int low, Int high);
Int		tmHTTPIODriver_ClientGetRangeRequested(tmHTTPIODriver_Client_t *hc);
void	tmHTTPIODriver_ClientTurnoffRanging(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientSetRangeMode( tmHTTPIODriver_RangeMode_t RangeMode);
Int		tmHTTPIODriver_ClientResetConnection(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientSendMethod(tmHTTPIODriver_Client_t *hc, tmHTTPIODriver_Method_t method);
Int		tmHTTPIODriver_ClientSendMessageBodyEnd(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientGetResponse(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientGetStatusCode(tmHTTPIODriver_Client_t *hc);
Char *	tmHTTPIODriver_ClientGetStatusString(tmHTTPIODriver_Client_t *hc);
Int		tmHTTPIODriver_ClientGetResponseHeader(tmHTTPIODriver_Client_t *hc, Char *header, Char *value, Int len);
Int		tmHTTPIODriver_ClientGetResponseBody(tmHTTPIODriver_Client_t *hc, Char *buf, Int len);
tmHTTPIODriver_Version_t tmHTTPIODriver_ClientGetStatusVersion(tmHTTPIODriver_Client_t *hc);
tmHTTPIODriver_RangeMode_t	tmHTTPIODriver_ClientGetRangeMode();




tmErrorCode_t tmHTTPIODriver_GetLastHttpStatus(void);
tmErrorCode_t tmHTTPIODriver_GetLastTcpStatus(void);
tmErrorCode_t tmHTTPIODriver_SetTimeout(int millisecs);
tmErrorCode_t tmHTTPIODriver_SetRangeToEndOfFile( Bool EndOfFile);
tmErrorCode_t tmHTTPIODriver_SetServerAuth(int useAuth, char *user, char *passwd);
tmErrorCode_t tmHTTPIODriver_SetProxy(char *proxy, int port);
tmErrorCode_t tmHTTPIODriver_SetUserAgent(char*agent);
tmErrorCode_t tmHTTPIODriver_Recog(char *path);


#ifdef __cplusplus
}
#endif

#endif /* TMHTTPIODRIVER_CLNT_H */

