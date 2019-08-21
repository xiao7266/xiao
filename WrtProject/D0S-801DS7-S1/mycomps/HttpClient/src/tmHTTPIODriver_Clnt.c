/*
* Copyright (C) 2000,2001,2002,2003   Koninklijke Philips Electronics N.V.   All Rights Reserved.
*
* this source code and any compilation or derivative thereof is the proprietary information 
* of Koninklijke Philips Electronics N.V. and is confidential in nature. Under no circumstances 
* is this software to be exposed to or placed under an Open Source License of any type without 
* the expressed written permission of Koninklijke Philips Electronics N.V.
*
*/
/****************************************************************************
* 修改内容：主要修改了，与SOCKET有关的部分
* 修改  人： 李建文
* 日    期：2008.1.3
*****************************************************************************/

/*****************************************************************************
*
* function         : includes
* 
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


#include <errno.h>
/*
#include <tmDbg.h>
#include <tmNetConfig.h>
#include <tmNetSocket.h>
#include <tmNetInet.h>
#include <pna.h>
#include <dns.h>
*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "tmHTTPIODriver_Clnt.h"
#include "httpclient.h"

/*
#if defined(__cplusplus)
extern "C"
{
#endif  
unsigned long inet_addr(const char *cp);

#if defined(__cplusplus)
}
#endif  

*/

#define closesocket close

#define SO_CONTIMEO  0x100A
/*****************************************************************************
*
* function         : defines
* 
*****************************************************************************/
/* protocol types */
#define HTTPCLIENT_PROTO_UNKNOWN      0
#define HTTPCLIENT_PROTO_HTTP         1

#define HTTPCLIENT_DEFAULT_HTTP_PORT  80

static tmHTTPIODriver_RangeMode_t ghttpRangeMode=tmHTTPIODriver_RangeToEndOfFile;

static Int gtmHTTPIODriver_connectTimeout = 30000;
static Int gtmHTTPIODriver_receiveTimeout = 10000;
static Int gtmHTTPIODriver_sendTimeout    = 10000;
static Int gtmHTTPIODriver_LastTcpStatus  = 0;


/*****************************************************************************
*
* function         : local variables
* 
*****************************************************************************/

static Char gtmHTTPIODriver_ProtoStr[] = "http://";
static Char gtmHTTPIODriver_DefaultResourceStr[] = "/";

static Char *httpClientHttpVersionStr[] = {
        "HTTP/0.9",
        "HTTP/1.0",
        "ICY",
        "HTTP/1.1"
};

static Char *httpClientHttpMethodStr[] = {
        "GET",
        "HEAD",
        "POST",
        "PUT"
};


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
static Char *stristr(Char *h, Char *n)
{
        char *hr, *hp;
        char *np;

        hr = hp = h;
        np = n;



        while (*hp != '\0')
        {
                while ((tolower(*hp) == tolower(*np)) && (*hp != '\0'))
                {
                        ++hp;
                        ++np;
                        if (*np == '\0')
                        {


                                return hr;
                        }
                }
                hr = hp + 1;
                hp = hr;
                np = n;
        }



        return NULL;
}
/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
static Int httpClientFlush(tmHTTPIODriver_Client_t *hc)
{
        int r;
        int bytesToCopy = hc->sBufNumBytes;
        int bytesCopied = 0;

        printf("httpClientFlush [%p]\n", hc->s);

        if (hc->sBuf != NULL)
        {
                r = 1;
                while ((bytesToCopy > 0) && (r > 0))
                {
                        if ((r = /*tmNetSocket_Send*/send(hc->s, hc->sBuf + bytesCopied, bytesToCopy, 0)) > 0)
                        {
                                bytesToCopy -= r;
                                bytesCopied += r;
                        }
                }
                hc->sBufNumBytes = 0;
        }

        return bytesCopied;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*
*  description:  
*
****************************************************************************/
tmErrorCode_t gtmHTTPIOClient_LastTcpStatus(void)
{
        return gtmHTTPIODriver_LastTcpStatus;
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
static Int httpClientRead(tmHTTPIODriver_Client_t *hc, Char *buf, Int len)
{
		int result;
		fd_set readfd;
		int timeoutcnt = 0;
		unsigned long maxfd=0;
		struct timeval timeout;

        if (hc->rBuf == NULL)
        {   /* no buffering */
                int r = 1;
                int bytesToCopy = len;
				
				
                while ((bytesToCopy > 0) && (r > 0))
                {	
                		
                		timeout.tv_sec = 0;
						timeout.tv_usec = 6000000;
						FD_ZERO(&readfd);
						FD_SET(hc->s, &readfd);
						maxfd = (hc->s+1)>maxfd?(hc->s+1):maxfd;
						result = select(maxfd,&readfd, NULL,  NULL, &timeout);
						
						if(result)
						{
							if(FD_ISSET(hc->s,&readfd))
							{
								
								r = /*tmNetSocket_Recv*/recv(hc->s, buf, bytesToCopy, 0);
								
								if (r > 0)
								{
										buf += r;
										bytesToCopy -= r;
								}else if ((r<0)&& (errno==ETIMEDOUT))
								{
										gtmHTTPIODriver_LastTcpStatus=(Int)TM_ERR_HTTPIODRIVER_CONNECT_LOST;
										return TM_ERR_HTTPIODRIVER_FAILED;
								}
							}
						}else
						{	
							if(timeoutcnt++ >= 10)
								return TM_ERR_HTTPIODRIVER_FAILED;
						}
						
                }
                return len - bytesToCopy;
        }
        else
        {   /* buffering */
                int bytesLeft = hc->rBufNumBytes - hc->rBufOffset;
                int bytesToCopy = len;

                if ((bytesLeft <= bytesToCopy))
                {
                        memcpy(buf, hc->rBuf + hc->rBufOffset, bytesLeft);
                        bytesToCopy -= bytesLeft;
                        buf += bytesLeft;
                        bytesLeft = 0;
                        hc->rBufOffset = hc->rBufNumBytes = 0;
                }
                else
                {
                        memcpy(buf, hc->rBuf + hc->rBufOffset, bytesToCopy);
                        hc->rBufOffset += bytesToCopy;

                        return bytesToCopy;
                }

                bytesLeft = 1;
                
                while ((bytesToCopy > 0) && (bytesLeft > 0))
                {
                		
                        if ((bytesLeft = /*tmNetSocket_Recv*/recv(hc->s, hc->rBuf, hc->rBufSize, 0)) > 0)
                        {
                                hc->rBufNumBytes = bytesLeft;
                                hc->rBufOffset = 0;

                                if (bytesLeft <= bytesToCopy)
                                {
                                        memcpy(buf, hc->rBuf + hc->rBufOffset, bytesLeft);
                                        hc->rBufOffset += bytesLeft;
                                        buf += bytesLeft;
                                        bytesToCopy -= bytesLeft;
                                        hc->rBufOffset = hc->rBufNumBytes = 0;
                                }
                                else
                                {
                                        memcpy(buf, hc->rBuf + hc->rBufOffset, bytesToCopy);
                                        hc->rBufOffset += bytesToCopy;
                                        buf += bytesToCopy;
                                        bytesLeft -= bytesToCopy;
                                        bytesToCopy = 0;
                                }
                        }else if ((bytesLeft<0)&& (errno==ETIMEDOUT))
                        {
                                gtmHTTPIODriver_LastTcpStatus=(Int)TM_ERR_HTTPIODRIVER_CONNECT_LOST;
                                
                                return TM_ERR_HTTPIODRIVER_FAILED;
                        }
                        else
                        {
                                if ((len - bytesToCopy) != 0)
                                {
                                        return (len - bytesToCopy);
                                }
                                else
                                {
                                        return bytesLeft;
                                }
                        }
                }
                return (len - bytesToCopy);
        }    

}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
static Int httpClientReadLine(tmHTTPIODriver_Client_t *hc, Char *buf, Int len, Char endChar)
{

        int i = 0;
        int r;



        if (hc->rBuf == NULL)
        {   /* no buffering */
                i = 0;
                do
                {
                        r = /*tmNetSocket_Recv*/recv(hc->s, buf, 1, 0);
                        ++buf;
                        --len;
                        ++i;
                }
                while ((len > 0) && (*(buf - 1) != endChar) && (r > 0));

                if (r > 0)
                {
                        return i;
                }
                else if (r == 0)
                {
                        return 0;
                }
                else
                {
                        gtmHTTPIODriver_LastTcpStatus=(Int)TM_ERR_HTTPIODRIVER_CONNECT_LOST;
                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
        }
        else
        {   /* buffering */
                int bytesToCopy = len;
                int bytesLeft = hc->rBufNumBytes - hc->rBufOffset;
                int endCharFound = 0;

                while ((bytesToCopy > 0) && (!endCharFound))
                {
                        if (bytesLeft == 0)
                        {
                                if ((hc->rBufNumBytes =/* tmNetSocket_Recv*/recv(hc->s, hc->rBuf, hc->rBufSize, 0)) <= 0)
                                {
                                        if (errno==ETIMEDOUT)
                                        {
                                                gtmHTTPIODriver_LastTcpStatus=(Int)TM_ERR_HTTPIODRIVER_CONNECT_LOST;
                                                return TM_ERR_HTTPIODRIVER_FAILED;
                                        }else
                                        {

                                                hc->rBufNumBytes = 0;
                                                hc->rBufOffset = 0;
                                                return (len - bytesToCopy);
                                        }
                                }
                                hc->rBufOffset = 0;
                                bytesLeft = hc->rBufNumBytes;
                        }

                        *buf = hc->rBuf[hc->rBufOffset];
                        ++(hc->rBufOffset);
                        --bytesToCopy;
                        --bytesLeft;
                        if ((*(buf) == endChar))
                        {
                                endCharFound = 1;
                        }
                        ++buf;
                }
                return (len - bytesToCopy);
        }

}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
static Int httpClientWrite(tmHTTPIODriver_Client_t *hc, Char *buf, Int len)
{
        int r;



        if (hc->sBuf == NULL)
        {   /* no buffering */
                int bytesToCopy = len;

                r = 1;
                while ((bytesToCopy > 0) && (r > 0))
                {
                        if ((r = /*tmNetSocket_Send*/send(hc->s, buf, bytesToCopy, 0)) > 0)
                        {
                                buf += r;
                                bytesToCopy -= r;
                        }
                        else
                        {
                                return r;
                        }
                }
                return len - bytesToCopy;
        }
        else
        {
                int bytesToCopy = len;
                int bytesCopied = 0;

                if (len < (hc->sBufSize - hc->sBufNumBytes))
                {
                        memcpy(hc->sBuf + hc->sBufNumBytes, buf, len);
                        hc->sBufNumBytes += len;

                        return len;
                }
                else if (hc->sBufNumBytes > 0)
                {
                        /* fill buffer and flush */
                        memcpy(hc->sBuf + hc->sBufNumBytes, buf, (hc->sBufSize - hc->sBufNumBytes));

                        bytesToCopy -= (hc->sBufSize - hc->sBufNumBytes);
                        bytesCopied += (hc->sBufSize - hc->sBufNumBytes);
                        hc->sBufNumBytes += (hc->sBufSize - hc->sBufNumBytes);

                        httpClientFlush(hc);
                }

                r = 1;
                while ((bytesToCopy > hc->sBufSize) && (r >= 0))
                {
                        if ((r = /*tmNetSocket_Send*/send(hc->s, buf + bytesCopied, bytesToCopy, 0)) > 0)
                        {
                                bytesToCopy -= r;
                                bytesCopied += r;
                                buf += r;
                        }
                }

                if (bytesToCopy > 0)
                {
                        memcpy(hc->sBuf, buf, bytesToCopy);
                        hc->sBufNumBytes = bytesToCopy;
                        bytesCopied += bytesToCopy;
                }

                return bytesCopied;
        }

}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
void tmHTTPIODriver_ClientRegister(void)
{

}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
void tmHTTPIODriver_ClientDeRegister(void)
{

}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientCreate(tmHTTPIODriver_Client_t *hc, Char *rBuf, Int rBufSize, Char *sBuf,
                                int sBufSize)
{


        gtmHTTPIODriver_LastTcpStatus=0;

        memset(hc, 0, sizeof(*hc));
        
        hc->s = -1;

        hc->httpRequestState = tmHTTPIODriver_ClientIdle;
        hc->httpResponseState = tmHTTPIODriver_ClientIdle;

        hc->proxyUse = 0;
        hc->proxyUseAuth = 0;
        hc->httpUseAuth = 0;

        hc->DisableProxyCache = False;

        hc->httpRequestProtocol = HTTPCLIENT_PROTO_HTTP;
        hc->httpRequestVersion = tmHTTPIODriver_HTTP11;    // defaults to 1.1

        memset(hc->httpRequestHeaders, 0, HTTPCLIENT_HEADER_SIZE);

        hc->rBuf = rBuf;
        hc->rBufSize = rBufSize;
        hc->rBufAllocated=0;
        hc->sBuf = sBuf;
        hc->sBufSize = sBufSize;
        hc->sBufAllocated=0;

        hc->httpPort=HTTPCLIENT_DEFAULT_HTTP_PORT;

        // receive buffers are allocated when the buffer == NULL and the size > 0. 
        // Buffering is turned off when the buffer == NULL and the size == 0.
        if ((hc->rBuf == NULL) && (hc->rBufSize > 0))
        {
                if ((hc->rBuf = malloc(hc->rBufSize)) == NULL)
                {

                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
                hc->rBufAllocated=1;
        }
        // send buffers are allocated when the buffer == NULL and the size > 0. 
        // Buffering is turned off when the buffer == NULL and the size == 0.
        if ((hc->sBuf == NULL) && (hc->sBufSize > 0))
        {
                if ((hc->sBuf = malloc(hc->sBufSize)) == NULL)
                {

                        if (hc->rBufAllocated==1) 
                        {
                                free(hc->rBuf);
                                hc->rBufAllocated=0;
                        }

                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
                hc->sBufAllocated=1;
        }

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientDisableProxyCache(tmHTTPIODriver_Client_t *hc, Int disable)
{
        hc->DisableProxyCache = disable;
        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientClose(tmHTTPIODriver_Client_t *hc)
{


        /* clean up buffers */
        if(hc->rBufAllocated) free(hc->rBuf);
        hc->rBuf = NULL;
        hc->rBufAllocated = 0;

        if(hc->sBufAllocated) free(hc->sBuf);
        hc->sBuf = NULL;
        hc->sBufAllocated = 0;

        if (/*tmNetSocket_CloseSocket*/closesocket(hc->s) == -1)
        {

                return TM_ERR_HTTPIODRIVER_FAILED;
        }
        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
***************************************************************************/
Int tmHTTPIODriver_ClientConnect(tmHTTPIODriver_Client_t *hc)
{
        //	struct	_tmNetSocket_Linger_t		opt;
        struct linger                       opt;
        char								*host;
        int									port;
        unsigned long						hostIp = 0;
        unsigned long                       myhostip =0;
        short								hostPort = 0;
        //    struct _tmNetSocket_SockAddrIn_t	laddr;
        struct sockaddr_in                  laddr;
        int									retries = 0;
        int									lastError;
        int									size;
        //	struct _tmNetInet_HostEnt_t			*addr;
        struct hostent*                      addr;
       // memset((void*)&addr,0,sizeof(struct hostent));
        //addr.h_addr_list[0] = (char*)&myhostip;





        if( tmHTTPIODriver_ClientIsConnected(hc))
        {
                printf("tmHTTPIODriver_ClientConnect already connected [%p] %d\n", hc,hc->s);
                return 0;
        }
        else
        {
                //Create a tcp socket
                if ((hc->s = socket( AF_INET ,SOCK_STREAM, 0)) == -1/*tmNetSocket_Socket(TM_NETSOCKET_AF_INET, TM_NETSOCKET_SOCK_STREAM, TM_NETINET_IPPROTO_TCP)) == TM_ERR_HTTPIODRIVER_FAILED*/)
                {
                        printf("tmHTTPIODriver_ClientConnect [%p] tcpSocketCreate error\n", hc);


                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
                size=TM_HTTPIODRIVER_RECVBUF_SIZE;
                if (size!=0)
                {
                        /*tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_RCVBUF,(char*)&size,sizeof(size));*/
                        setsockopt(hc->s,SOL_SOCKET,SO_RCVBUF,(char*)&size,sizeof(size));
                }else
                {
                        size= 16*1024;
                        /*tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_RCVBUF,(char*)&size,sizeof(size));*/
                        setsockopt(hc->s,SOL_SOCKET,SO_RCVBUF,(char*)&size,sizeof(size));
                }
                size=TM_HTTPIODRIVER_SENDBUF_SIZE;
                if (size!=0)
                {
                        /*tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_SNDBUF,(char*)&size,sizeof(size));*/
                        setsockopt(hc->s,SOL_SOCKET,SO_SNDBUF,(char*)&size,sizeof(size));
                }else
                {
                        size= 16*1024;
                        /*tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_SNDBUF,(char*)&size,sizeof(size));*/
                        setsockopt(hc->s,SOL_SOCKET,SO_SNDBUF,(char*)&size,sizeof(size));
                }

                // set timeouts
                /*
                tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_CONTIMEO,&gtmHTTPIODriver_connectTimeout,sizeof(gtmHTTPIODriver_connectTimeout));
                tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_RCVTIMEO,&gtmHTTPIODriver_receiveTimeout,sizeof(gtmHTTPIODriver_receiveTimeout));
                tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_SNDTIMEO,&gtmHTTPIODriver_sendTimeout,sizeof(gtmHTTPIODriver_sendTimeout));
                */
                setsockopt(hc->s,SOL_SOCKET,SO_CONTIMEO,&gtmHTTPIODriver_connectTimeout,sizeof(gtmHTTPIODriver_connectTimeout));
                setsockopt(hc->s,SOL_SOCKET,SO_RCVTIMEO,&gtmHTTPIODriver_receiveTimeout,sizeof(gtmHTTPIODriver_receiveTimeout));
                setsockopt(hc->s,SOL_SOCKET,SO_SNDTIMEO,&gtmHTTPIODriver_sendTimeout,sizeof(gtmHTTPIODriver_sendTimeout));    		
                // Set socket linger
                opt.l_onoff = 0;
                opt.l_linger = 10;
                if(setsockopt(hc->s,SOL_SOCKET,SO_LINGER,(char*)&opt,sizeof(opt)) == -1)
                        /*if (tmNetSocket_SetSockOpt(hc->s, TM_NETSOCKET_SOL_SOCKET, TM_NETSOCKET_SO_LINGER, (char *)&opt,sizeof(opt)) == TM_ERR_HTTPIODRIVER_FAILED)*/
                { 
                        printf("setsockopt error\n");
                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
                if (hc->proxyUse)
                {
                        host=hc->proxyServer;
                        port=hc->proxyPort;
                }else
                {
                        host=hc->httpServer;
                        port=hc->httpPort;
                }

                /*if ((hostIp = tmNetInet_InetAddr(host)) == TM_NETINET_INADDR_NONE)*/
                if((hostIp = inet_addr(host)) == -1)
                {
                        printf("invalid address\n");
                        if ( (addr = gethostbyname(host)) == NULL)
                        {
                                printf("gethostbyname failed\n");
                                gtmHTTPIODriver_LastTcpStatus=(Int)TM_ERR_HTTPIODRIVER_DNS_FAILED;
                                return TM_ERR_HTTPIODRIVER_FAILED;
                        }else
                        {
                                hostIp=(((int)addr->h_addr_list[0][3] << 24)&0xff000000)	;
                                hostIp|=(((int)addr->h_addr_list[0][2] << 16)&0x00ff0000)	;
                                hostIp|=(((int)addr->h_addr_list[0][1] << 8)&0x0000ff00)	;
                                hostIp|=(((int) addr->h_addr_list[0][0])&0x000000ff)	;

                        }

                }

                hostPort = htons(port);/*TM_NETINET_HTONS((unsigned short)port)*/;
                memset(&laddr, 0, sizeof(laddr));
                laddr.sin_family = AF_INET;
                laddr.sin_port = hostPort;
                laddr.sin_addr.s_addr = hostIp;

                while (retries < 5)
                {

                        /* This is a work-around for the problem that when the board is reset, the connection on the
                        server is still open. On a reboot, this might lead to a "Connection Refused". */
                        /*if (tmNetSocket_Connect(hc->s, (void *)&laddr, sizeof(laddr)) == TM_ERR_HTTPIODRIVER_FAILED)*/
                        if(connect(hc->s,(struct sockaddr*)&laddr,sizeof(laddr)) == -1)
                        {

                                lastError = errno;
                                retries++;
                                if ((lastError == ECONNREFUSED)||(lastError == EADDRINUSE))
                                {
                                        /*tmNetSocket_CloseSocket(hc->s);*/
                                        closesocket(hc->s);
                                        if ((hc->s =  socket( AF_INET ,SOCK_STREAM, 0)) == -1/*tmNetSocket_Socket(TM_NETSOCKET_AF_INET, TM_NETSOCKET_SOCK_STREAM, TM_NETINET_IPPROTO_TCP)) == TM_ERR_HTTPIODRIVER_FAILED*/)
                                        {
                                                printf("tmHTTPIODriver_ClientConnect [%p] tcpSocketCreate error\n", hc);
                                                return TM_ERR_HTTPIODRIVER_FAILED;
                                        }
                                        // set timeouts
                                        /*
                                        tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_CONTIMEO,&gtmHTTPIODriver_connectTimeout,sizeof(gtmHTTPIODriver_connectTimeout));
                                        tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_RCVTIMEO,&gtmHTTPIODriver_receiveTimeout,sizeof(gtmHTTPIODriver_receiveTimeout));
                                        tmNetSocket_SetSockOpt(hc->s,TM_NETSOCKET_SOL_SOCKET,TM_NETSOCKET_SO_SNDTIMEO,&gtmHTTPIODriver_sendTimeout,sizeof(gtmHTTPIODriver_sendTimeout));
                                        */ 
                                        setsockopt(hc->s,SOL_SOCKET,SO_CONTIMEO,&gtmHTTPIODriver_connectTimeout,sizeof(gtmHTTPIODriver_connectTimeout));
                                        setsockopt(hc->s,SOL_SOCKET,SO_RCVTIMEO,&gtmHTTPIODriver_receiveTimeout,sizeof(gtmHTTPIODriver_receiveTimeout));
                                        setsockopt(hc->s,SOL_SOCKET,SO_SNDTIMEO,&gtmHTTPIODriver_sendTimeout,sizeof(gtmHTTPIODriver_sendTimeout));   	                  
                                        continue;
                                }

                                if(lastError != EINPROGRESS)
                                {
                                        printf("connect error [%d] retry = %d\n", lastError,retries);
                                        gtmHTTPIODriver_LastTcpStatus=(Int)TM_ERR_HTTPIODRIVER_CONNECT_FAILED;
                                        return TM_ERR_HTTPIODRIVER_FAILED;
                                }
                        } 
                        // exit while if fall through
                        break;
                }

                // reset buffers 
                hc->rBufOffset = 0;
                hc->rBufNumBytes = 0;
                hc->sBufNumBytes = 0;



        }


        return 0;  
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    1 if connection on tcp level exsists
*                    0 if no connection
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientIsConnected(tmHTTPIODriver_Client_t *hc)
{
        int									result=0;
        /*	struct _tmNetSocket_TimeVal_t		timeout;*/
        struct timeval                      timeout;
        /*	tmNetSocket_FdSet_t					fdset;*/
        fd_set                              fdset;
        int									count = 0;
        /*    struct _tmNetSocket_SockAddrIn_t	addr;*/
        struct sockaddr_in                  addr;
        int									addr_size=sizeof(addr);
        int									socket_error;
        int									socket_error_size=sizeof(socket_error);

        if(hc->s == -1){
        	printf("Client is not connected [%p] %d\n", hc,hc->s);
        	return 0;
        }

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        /*tmNetSocket_FdZero(&fdset);
        tmNetSocket_FdSet(hc->s, &fdset);*/
        FD_ZERO(&fdset);
        FD_SET(hc->s,&fdset);

        
        if ((count = /*tmNetSocket_Select*/select(hc->s+1,NULL, &fdset, NULL, &timeout)) > 0 )
        {
                //if (tmNetSocket_GetSockName(hc->s,(struct _tmNetSocket_SockAddr_t *) &addr,&addr_size)==0)
                if(getsockname(hc->s,(struct sockaddr_in*)&addr,&addr_size) == 0)
                {
                        result = 1;
                }
        }
        else if (count == 0)
        {
                //	tmNetSocket_FdZero(&fdset);
                //	tmNetSocket_FdSet(hc->s, &fdset);
                FD_ZERO(&fdset);
                FD_SET(hc->s,&fdset);

                if ((count =select(FD_SETSIZE,NULL,NULL,&fdset,&timeout) /*tmNetSocket_Select(TM_NETSOCKET_FD_SETSIZE, NULL, NULL, &fdset, &timeout)*/)>0)
                {
                        /*
                        if ( tmNetSocket_GetSockOpt(hc->s, TM_NETSOCKET_SOL_SOCKET,
                        TM_NETSOCKET_SO_ERROR, (char *) &socket_error, &socket_error_size)==0)
                        */
                        if(getsockopt(hc->s,SOL_SOCKET,SO_ERROR,(char*)&socket_error,&socket_error_size) == 0);
                        {
                                printf("%s %d socket_error:%d \n",__FILE__,__LINE__,socket_error);
                        }
                }
        }
        else // select failed
        {
                printf("Client is not connected [%p] %d\n", hc,hc->s);
                result = 0; // not connected
        }

        printf("tmHTTPIODriver_ClientIsConnected [%p] %d\n", hc,hc->s);
        return result;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetGlobalTimeOuts( Int connect, Int send, Int recv)
{
        printf("tmHTTPIODriver_ClientSetGlobalTimeOuts [%d] [%d] [%d]\n", connect, send, recv);

        gtmHTTPIODriver_connectTimeout = connect;
        gtmHTTPIODriver_receiveTimeout = recv;
        gtmHTTPIODriver_sendTimeout    = send;

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetProxy(tmHTTPIODriver_Client_t *hc, Char *proxy, Int port, Int useProxy)
{
        printf("tmHTTPIODriver_ClientSetProxy [%p] [%s] [%d] [%d]\n", hc, proxy, port, 
                useProxy);

        hc->proxyUse = useProxy;

        if (hc->proxyUse == 1)
        {
                if (proxy != NULL)
                {
                        strncpy(hc->proxyServer, proxy, HTTPCLIENT_SERVER_SIZE);
                        hc->proxyServer[HTTPCLIENT_SERVER_SIZE - 1] = '\0';
                }

                hc->proxyPort = port;
        }

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetProxyAuth(tmHTTPIODriver_Client_t *hc, Char *user, Char *passwd, 
                                      int useProxyAuth)
{
        printf("tmHTTPIODriver_ClientSetProxyAuth [%p] [%s] [%s] [%d]\n", hc, user,
                passwd, useProxyAuth);

        hc->proxyUseAuth = useProxyAuth;

        if (hc->proxyUseAuth == 1)
        {
                if (user != NULL)
                {
                        strncpy(hc->proxyUser, user, HTTPCLIENT_USER_SIZE);
                        hc->proxyUser[HTTPCLIENT_USER_SIZE - 1] = '\0';
                }

                if (passwd != NULL)
                {
                        strncpy(hc->proxyPasswd, passwd, HTTPCLIENT_USER_SIZE);
                        hc->proxyPasswd[HTTPCLIENT_USER_SIZE - 1] = '\0';
                }
        }

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetRequestVersion(tmHTTPIODriver_Client_t *hc, tmHTTPIODriver_Version_t httpVersion)
{
        printf("tmHTTPIODriver_ClientSetRequestVersion [%p] [%s]\n", hc, 
                httpClientHttpVersionStr[httpVersion]);

        if ((httpVersion >= tmHTTPIODriver_HTTP09) && (httpVersion <= tmHTTPIODriver_HTTP11))
        {
                hc->httpRequestVersion = httpVersion;

                return 0;
        }
        else
        {
                //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_WARNING, "invalid http version\n"))

                        return TM_ERR_HTTPIODRIVER_FAILED;
        }
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetUrl(tmHTTPIODriver_Client_t *hc, Char *url)
{
        char *ps, *pe;



        if (url == NULL)
        {
                printf("tmHTTPIODriver_ClientSetUrl [%p] URL = NULL\n", hc);
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        strncpy(hc->httpRequestUrl, url, HTTPCLIENT_URL_SIZE);
        hc->httpRequestUrl[HTTPCLIENT_URL_SIZE - 1] = '\0';

        /* parse url */

        /* get protocol */
        if ((ps = strstr(url, gtmHTTPIODriver_ProtoStr)) == 0)
        {
                printf("unknown protocol\n");

                hc->httpRequestProtocol = HTTPCLIENT_PROTO_UNKNOWN;

                printf("tmHTTPIODriver_ClientSetUrl [%p] unknown proto\n", hc);
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        hc->httpRequestProtocol = HTTPCLIENT_PROTO_HTTP;

        /* get server */
        ps += strlen(gtmHTTPIODriver_ProtoStr);
        strncpy(hc->httpServer, ps, HTTPCLIENT_SERVER_SIZE);
        hc->httpServer[HTTPCLIENT_SERVER_SIZE - 1] = '\0';

        if ((pe = strchr(hc->httpServer, '/')) != NULL)
        { /* terminate at slash */
                *pe = '\0';
        }

        /* get port */
        if ((ps = strchr(hc->httpServer, ':')) != NULL)
        { /* got a port */
                /* terminate server string */
                *ps = '\0';

                /* get port */
                hc->httpPort = atoi(ps + 1);
        }
        else
        {
                hc->httpPort = HTTPCLIENT_DEFAULT_HTTP_PORT;
        }

        /* get resource */
        ps = strstr(url, gtmHTTPIODriver_ProtoStr);
        ps += strlen(gtmHTTPIODriver_ProtoStr);
        if ((ps = strchr(ps, '/')) == NULL)
        { /* no resource, default to / */
                ps = gtmHTTPIODriver_DefaultResourceStr;
        }
        strncpy(hc->httpRequestResource, ps, HTTPCLIENT_URL_SIZE);
        hc->httpRequestResource[HTTPCLIENT_URL_SIZE - 1] = '\0';

        printf("url[%s]\n", hc->httpRequestUrl);
        printf("server[%s] port[%d]\n", hc->httpServer, hc->httpPort);
        printf("resource[%s]\n", hc->httpRequestResource);

        printf("tmHTTPIODriver_ClientSetUrl [%p]\n", hc);
        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetServerAuth(tmHTTPIODriver_Client_t *hc, Char *user, Char *passwd, 
                                       int useAuth)
{


        hc->httpUseAuth = useAuth;

        if (hc->httpUseAuth == 1)
        {
                if (user != NULL)
                {
                        strncpy(hc->httpUser, user, HTTPCLIENT_USER_SIZE);
                        hc->httpUser[HTTPCLIENT_USER_SIZE - 1] = '\0';
                }

                if (passwd != NULL)
                {
                        strncpy(hc->proxyPasswd, passwd, HTTPCLIENT_USER_SIZE);
                        hc->httpPasswd[HTTPCLIENT_USER_SIZE - 1] = '\0';
                }
        }

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientClearHeaders(tmHTTPIODriver_Client_t *hc)
{


        memset(hc->httpRequestHeaders, 0, HTTPCLIENT_HEADER_SIZE);

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetChunked(tmHTTPIODriver_Client_t *hc, Int useChunked)
{


        hc->httpRequestChunked = useChunked;

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetUserAgent(tmHTTPIODriver_Client_t *hc, Char *agentSmith)
{
        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientSetUserAgent [%p] [%s]\n", hc, agentSmith))

        if (agentSmith == NULL)
        {
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        strncpy(hc->httpRequestUserAgent, agentSmith, HTTPCLIENT_AGENT_SIZE);
        hc->httpRequestUserAgent[HTTPCLIENT_AGENT_SIZE - 1] = '\0';

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetAccept(tmHTTPIODriver_Client_t *hc, Char *accept)
{
        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientSetAccept [%p] [%s]\n", hc, accept))

        hc->httpRequestAccept[0] = '\0';
        strncat(hc->httpRequestAccept, accept, HTTPCLIENT_ACCEPT_SIZE);

        hc->httpRequestAccept[HTTPCLIENT_ACCEPT_SIZE-1]=0;

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetPersistentConnection(tmHTTPIODriver_Client_t *hc, Int usePersistent)
{
        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientSetPersistentConnection [%p] [%d]\n", hc,
        //usePersistent))

                hc->httpRequestPersistent = usePersistent;

        return 0;
}
/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSetRange(tmHTTPIODriver_Client_t *hc, Int low, Int high)
{
        // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientSetRange [%p] [%d][%d]\n", hc,low,high))

        hc->httpRequestRangeLow=low;
        hc->httpRequestRangeHigh=high;

        hc->httpRequestRange=1;

        return 0;
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientGetRangeRequested(tmHTTPIODriver_Client_t *hc)
{
        return hc->httpRequestRange;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
void tmHTTPIODriver_ClientTurnoffRanging(tmHTTPIODriver_Client_t *hc)
{
        hc->httpRequestRange =0 ;
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientResetConnection(tmHTTPIODriver_Client_t *hc)
{
        //  struct _tmNetSocket_Linger_t opt;
        struct linger  opt;

        opt.l_onoff = 1;
        opt.l_linger = 0;

        //  if (tmNetSocket_SetSockOpt(hc->s, TM_NETSOCKET_SOL_SOCKET, TM_NETSOCKET_SO_LINGER, (char *)&opt,sizeof(opt)) == TM_ERR_HTTPIODRIVER_FAILED)
        if(setsockopt(hc->s,SOL_SOCKET,SO_LINGER,(char*)&opt,sizeof(opt))== -1) 
        { 
                // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_WARNING, "setsockopt error\n"))
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        //if ((tmNetSocket_Shutdown(hc->s,TM_NETSOCKET_SHUT_RDWR))==TM_ERR_HTTPIODRIVER_FAILED)
        if(shutdown(hc->s,2) == -1)
        { 
                // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_WARNING, "shutdown error\n"))
                return TM_ERR_HTTPIODRIVER_FAILED;
        }


        return	tmHTTPIODriver_ClientClose(hc);
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
static Int httpClientRequestSetupDefaultHeaders(tmHTTPIODriver_Client_t *hc)
{
        // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "httpClientRequestSetupDefaultHeaders [%p]\n", hc))

        hc->httpRequestDefaultHeaders[0] = '\0';

        if (hc->httpServer[0] != '\0')
        {


                strncat(hc->httpRequestDefaultHeaders, "Host: ", 
                        HTTPCLIENT_HEADER_SIZE);
                strncat(hc->httpRequestDefaultHeaders, hc->httpServer, 
                        HTTPCLIENT_HEADER_SIZE);
                if(hc->httpPort != 80)
                {
                        char buf[20];
                        sprintf(buf,"%d",hc->httpPort);

                        strncat(hc->httpRequestDefaultHeaders, ":", 
                                HTTPCLIENT_HEADER_SIZE);
                        strncat(hc->httpRequestDefaultHeaders, buf, 
                                HTTPCLIENT_HEADER_SIZE);
                }
                strncat(hc->httpRequestDefaultHeaders, "\r\n", 
                        HTTPCLIENT_HEADER_SIZE);
        }

        if (hc->httpRequestChunked)
        {
                strncat(hc->httpRequestDefaultHeaders, "Transfer-Encoding: chunked\r\n", 
                        HTTPCLIENT_HEADER_SIZE);
        }

        if (hc->httpRequestAccept[0] != '\0')
        {
                strncat(hc->httpRequestDefaultHeaders, "Accept: ", 
                        HTTPCLIENT_HEADER_SIZE);
                strncat(hc->httpRequestDefaultHeaders, hc->httpRequestAccept, 
                        HTTPCLIENT_HEADER_SIZE);
                strncat(hc->httpRequestDefaultHeaders, "\r\n", 
                        HTTPCLIENT_HEADER_SIZE);
        }

        if (hc->httpRequestPersistent == 0)
        {
                if(hc->proxyUse == 1)
                        strncat(hc->httpRequestDefaultHeaders, "Proxy-Connection: close\r\n", HTTPCLIENT_HEADER_SIZE);
                else
                        strncat(hc->httpRequestDefaultHeaders, "Connection: close\r\n", HTTPCLIENT_HEADER_SIZE);
        }
        else
        {
                if(hc->proxyUse == 1)
                        strncat(hc->httpRequestDefaultHeaders, "Proxy-Connection: keep-alive\r\n", HTTPCLIENT_HEADER_SIZE);
                else
                        strncat(hc->httpRequestDefaultHeaders, "Connection: keep-alive\r\n", HTTPCLIENT_HEADER_SIZE);
        }


        if (hc->httpRequestUserAgent[0] != '\0')
        {
                strncat(hc->httpRequestDefaultHeaders, "User-Agent: ", 
                        HTTPCLIENT_HEADER_SIZE);
                strncat(hc->httpRequestDefaultHeaders, hc->httpRequestUserAgent, 
                        HTTPCLIENT_HEADER_SIZE);
                strncat(hc->httpRequestDefaultHeaders, "\r\n", 
                        HTTPCLIENT_HEADER_SIZE);
        }

        if (hc->proxyUseAuth)
        {
                /* TODO: add proxy authentication */
        }

        if (hc->httpUseAuth)
        {
                /* TODO: add server authentication */
        }

        return 0;
}



int tmHTTPIODriver_ClientSetRangeMode(tmHTTPIODriver_RangeMode_t RangeMode)
{
        switch(RangeMode)
        {
        case tmHTTPIODriver_RangeToEndOfFile:
        case tmHTTPIODriver_RangeBlock:
                ghttpRangeMode = RangeMode;
                return 0;
        default:
                ghttpRangeMode = tmHTTPIODriver_RangeBlock;
                return TM_ERR_HTTPIODRIVER_FAILED;
        }
}

tmHTTPIODriver_RangeMode_t tmHTTPIODriver_ClientGetRangeMode(void)
{
        return ghttpRangeMode;
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSendMethod(tmHTTPIODriver_Client_t *hc, tmHTTPIODriver_Method_t method)
{
        int len,totallen=0;

        // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientSendMethod [%p] [%s]\n", hc, 
        //   httpClientHttpMethodStr[method]))

        /* assemble request */
        /* method */
        hc->httpRequestMethod[0] = '\0';
        strncat(hc->httpRequestMethod, httpClientHttpMethodStr[method], HTTPCLIENT_METHOD_SIZE);
        strncat(hc->httpRequestMethod, " ", HTTPCLIENT_METHOD_SIZE);
        if (hc->proxyUse)
        {
                strncat(hc->httpRequestMethod, hc->httpRequestUrl, HTTPCLIENT_METHOD_SIZE);
        }
        else
        {
                strncat(hc->httpRequestMethod, hc->httpRequestResource, HTTPCLIENT_METHOD_SIZE);
        }
        strncat(hc->httpRequestMethod, " ", HTTPCLIENT_METHOD_SIZE);
        strncat(hc->httpRequestMethod, httpClientHttpVersionStr[hc->httpRequestVersion], 
                HTTPCLIENT_METHOD_SIZE);
        strncat(hc->httpRequestMethod, "\r\n", HTTPCLIENT_METHOD_SIZE);

        // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "method\n[%s]\n", hc->httpRequestMethod))

        if (hc->httpRequestRange == 1) 
        {
                char t[100];

                if(ghttpRangeMode == tmHTTPIODriver_RangeToEndOfFile)
                {
                        /* If your requesting a range untill end-of-file, it doesn't make sense to
                        start at byte 0, as this is exactly the same as a GET request without range.
                        Some servers don't respond well to a request with "Range: bytes=0-", so we
                        just do a GET without range in this case.*/
                        if (hc->httpRequestRangeLow != 0)
                        {
                                sprintf(t,"Range: bytes=%d-\r\n",hc->httpRequestRangeLow);
                        }
                        else
                        {
                                t[0] = '\0';
                        }
                }
                else
                {
                        sprintf(t,"Range: bytes=%d-%d\r\n",hc->httpRequestRangeLow,hc->httpRequestRangeHigh);
                }

                strncat(hc->httpRequestMethod, t, HTTPCLIENT_HEADER_SIZE);
        }


        /* default headers */
        httpClientRequestSetupDefaultHeaders(hc);

        // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "default headers\n[%s]\n", hc->httpRequestDefaultHeaders))

        totallen = strlen(hc->httpRequestMethod);
        len=strlen(hc->httpRequestDefaultHeaders);
        totallen += len;
        strncat(hc->httpRequestMethod, hc->httpRequestDefaultHeaders, len);
        len = strlen(hc->httpRequestHeaders);
        totallen += len;
        strncat(hc->httpRequestMethod, hc->httpRequestHeaders, len);
        if (hc->proxyUse && hc->DisableProxyCache)
        {
                len = strlen("Cache-Control: no-cache\r\n");
                totallen += len;
                strncat(hc->httpRequestMethod, "Cache-Control: no-cache\r\n", len);
        }

        len = strlen("\r\n");
        totallen += len;
        strncat(hc->httpRequestMethod, "\r\n", len);

        httpClientWrite(hc, hc->httpRequestMethod, totallen);

        //  httpClientFlush(hc);

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientSendMessageBodyEnd(tmHTTPIODriver_Client_t *hc)
{
        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientSendMessageBodyEnd [%p]\n", hc))

        if (hc->httpRequestChunked)
        {
                /* TODO: send 0 chunk */
        }

        httpClientFlush(hc);

        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientGetResponse(tmHTTPIODriver_Client_t *hc)
{
        int r;
        int i;
        tmHTTPIODriver_Version_t hv;
        static char   temp[100];
        char  *endchar;


        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientGetResponse [%p]\n", hc))

        memset(hc->httpResponseStatusString, 0, HTTPCLIENT_STATUS_SIZE);

        /* get the status */
        if ((r = httpClientReadLine(hc, hc->httpResponseStatusString, HTTPCLIENT_STATUS_SIZE, 
                '\n')) <= 0)
        {
                hc->httpResponseStatusString[0] = '\0';
                return TM_ERR_HTTPIODRIVER_FAILED;
        }
        else
        {
                hc->httpResponseStatusString[r] = '\0';
                //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "response string [%s] len [%d]\n", hc->httpResponseStatusString,r))

                hc->httpResponseVersion = tmHTTPIODriver_HTTP09;
                for (hv = tmHTTPIODriver_HTTP09; hv <= tmHTTPIODriver_HTTP11; ++hv)
                {
                        if (strstr(hc->httpResponseStatusString, httpClientHttpVersionStr[hv]) != NULL)
                        {
                                hc->httpResponseVersion = hv;
                                break;
                        }
                }
                hc->httpResponseStatusCode = atoi(strchr(hc->httpResponseStatusString, ' ') + 1);

                //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "status [%s]\n", hc->httpResponseStatusString))
                // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "response version [%s] status code[%d]\n", 
                //httpClientHttpVersionStr[hc->httpResponseVersion], hc->httpResponseStatusCode))

        }


        /* get the headers */
        i = 0;
        while((r = httpClientReadLine(hc, hc->httpResponseHeaders + i, 
                (HTTPCLIENT_HEADER_SIZE - 1) - i, '\n')) > 2)
        {
                i += r;
        }
        hc->httpResponseHeaders[i] = '\0';


        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "response headers [%.200s]\n", hc->httpResponseHeaders))

        /* check for chunking */

        if (stristr(hc->httpResponseHeaders, "transfer-encoding" )!= NULL)
        {
                if (stristr(hc->httpResponseHeaders, "chunked" )!= NULL)
                {
                        hc->httpResponseChunked=True;
                        /* Check what the chuncking size is */
                        if (httpClientReadLine(hc, temp, 100, '\n')>2)
                        {
                                //Make sure the length is hexadecimal.
                                hc->httpResponseChunkSize=strtol(temp,&endchar,16);
                                hc->httpResponseChunkPos = 0;
                        }
                }else
                {
                        hc->httpResponseChunked=False;
                }
        }else
        {
                hc->httpResponseChunked=False;
        }


        return 0;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientGetStatusCode(tmHTTPIODriver_Client_t *hc)
{
        return hc->httpResponseStatusCode;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
tmHTTPIODriver_Version_t tmHTTPIODriver_ClientGetStatusVersion(tmHTTPIODriver_Client_t *hc)
{
        return hc->httpResponseVersion;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Char *tmHTTPIODriver_ClientGetStatusString(tmHTTPIODriver_Client_t *hc)
{
        return hc->httpResponseStatusString;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientGetResponseHeader(tmHTTPIODriver_Client_t *hc, Char *header, Char *value, Int len)
{
        char *p;

        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientGetResponseHeader [%p] [%s]\n", hc, header))

        if ((p = stristr(hc->httpResponseHeaders, header)) == NULL)
        {
                // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_WARNING, "failed to find header : %s\n",header))
                return TM_ERR_HTTPIODRIVER_FAILED;
        }
        p += strlen(header);

        if ((p = strchr(p, ':')) == NULL)
        {
                //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_WARNING, "failed to find :\n"))
                return TM_ERR_HTTPIODRIVER_FAILED;
        }
        ++p;

        while (*p && isspace(*p))
        {
                ++p;
        }

        strncpy(value, p, len);
        value[len - 1] = '\0';

        p = strchr(value, '\r');

        *p = '\0';

        //DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_STATE, "value [%s]\n", value))
        return strlen(value);
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, 0 on success
*
*  description:  
*
****************************************************************************/
Int tmHTTPIODriver_ClientGetResponseBody(tmHTTPIODriver_Client_t *hc, Char *buf, Int len)
{
        int result = 0;
        int readSoFar=0;
        int totalRead=0;
        char *endchar;
        static char chunkTemp[32];
        int isFinished=False;
        int readlen=0;

        totalRead=totalRead;

        // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_INTERFACE_ENTER, "tmHTTPIODriver_ClientGetResponseBody [%p] [%p] [%d]\n", hc, buf, len))

        if (hc->httpResponseChunked)
        {
                // DBG_PRINT((gtmHTTPIODriver_clnt_DebugUnit, DBG_WARNING, "tmHTTPIODriver_ClientGetResponseBody: httpResponseChunked not supported!\n"))

                if (hc->httpResponseChunkSize!=0)
                {
                        // Read the first chunk or what's left of it to make a clean start.
                        if (hc->httpResponseChunkPos!=0) 
                        {
                                if (hc->httpResponseChunkPos < len ) readlen=hc->httpResponseChunkPos; else readlen=len;
                        }else 
                        {
                                if (hc->httpResponseChunkSize < len ) readlen=hc->httpResponseChunkSize; else readlen=len;
                        }
                        //first read the rest of the previous chunk
                        if ((result = httpClientRead(hc,buf, readlen)) == TM_ERR_HTTPIODRIVER_FAILED)
                        {
                                totalRead = 0;
                                isFinished=True;
                        }else
                        {
                                readSoFar=result;
                                if (readSoFar!=len)
                                {
                                        hc->httpResponseChunkPos=0;
                                        //Read the next chunk size
                                        hc->httpResponseChunkSize=0;
                                        httpClientReadLine(hc, chunkTemp, 31, '\n');
                                        if (httpClientReadLine(hc, chunkTemp, 31, '\n')>2)
                                        {
                                                //Make sure the length is hexadecimal.
                                                hc->httpResponseChunkSize=strtol(chunkTemp,&endchar,16);
                                                if (hc->httpResponseChunkSize==0)isFinished=True;
                                                hc->httpResponseChunkPos=0;
                                        }
                                }else
                                {
                                        if (hc->httpResponseChunkPos > 0) 
                                        {
                                                hc->httpResponseChunkPos-=readSoFar; 
                                                // hc->httpResponseChunkPos became 0, but we still need to read the next chunksize...
                                                if (hc->httpResponseChunkPos==0) 
                                                {
                                                        hc->httpResponseChunkSize=0;
                                                        httpClientReadLine(hc, chunkTemp, 31, '\n');
                                                        if (httpClientReadLine(hc, chunkTemp, 31, '\n')>2)
                                                        {
                                                                //Make sure the length is hexadecimal.
                                                                hc->httpResponseChunkSize=strtol(chunkTemp,&endchar,16);
                                                                if (hc->httpResponseChunkSize==0)isFinished=True;
                                                                hc->httpResponseChunkPos=0;
                                                        }
                                                }
                                        }else 
                                        {
                                                hc->httpResponseChunkPos=hc->httpResponseChunkSize-readSoFar;
                                        }
                                        totalRead=readSoFar;
                                        isFinished=True;
                                }
                        }
                } else
                {
                        totalRead = readSoFar;
                        isFinished=True;
                }

                //keep reading chunks until len is read
                while ((readSoFar< len)&&(!isFinished))
                {
                        //Check if what's left to read is smaller than a chunk
                        if ((len-readSoFar)<hc->httpResponseChunkSize)
                        {
                                // we do not have a complete chunk left
                                readlen=len-readSoFar;
                                // Calculate the length we need to read next time 
                                hc->httpResponseChunkPos=hc->httpResponseChunkSize - readlen;
                                if ((result = httpClientRead(hc,buf+readSoFar, readlen)) == TM_ERR_HTTPIODRIVER_FAILED)
                                {
                                        totalRead = readSoFar;
                                        isFinished=True;
                                }else
                                {
                                        readSoFar+=result;
                                }

                        }else
                        {
                                readlen=hc->httpResponseChunkSize;
                                //Read a complete chunk
                                if ((result = httpClientRead(hc,buf+readSoFar, readlen)) == TM_ERR_HTTPIODRIVER_FAILED)
                                {
                                        totalRead = readSoFar;
                                        isFinished=True;
                                }else
                                {
                                        readSoFar+=result;

                                        //Read the next chunk size
                                        hc->httpResponseChunkSize=0;
                                        httpClientReadLine(hc, chunkTemp, 31, '\n');
                                        if (httpClientReadLine(hc, chunkTemp, 31, '\n')>2)
                                        {
                                                //Make sure the length is hexadecimal.
                                                hc->httpResponseChunkSize=strtol(chunkTemp,&endchar,16);
                                                if (hc->httpResponseChunkSize==0)isFinished=True;
                                                hc->httpResponseChunkPos=0;
                                        }
                                }
                        }

                }
                if (!isFinished) totalRead = readSoFar;

        }
        else
        {

                if ((totalRead = httpClientRead(hc, buf, len)) == TM_ERR_HTTPIODRIVER_FAILED)
                {
                        totalRead = -1;
                }
        }
        return totalRead;
}

