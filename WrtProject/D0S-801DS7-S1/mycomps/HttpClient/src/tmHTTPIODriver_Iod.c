/*
* Copyright (C) 2000,2001,2002,2003   Koninklijke Philips Electronics N.V.   All Rights Reserved.
*
* this source code and any compilation or derivative thereof is the proprietary information 
* of Koninklijke Philips Electronics N.V. and is confidential in nature. Under no circumstances 
* is this software to be exposed to or placed under an Open Source License of any type without 
* the expressed written permission of Koninklijke Philips Electronics N.V.
*
*/

/*****************************************************************************
*
* function         : includes
* 
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <sys/stat.h>
#include <errno.h>


#include "tmHTTPIODriver_Clnt.h"
#include "httpclient.h"




/*****************************************************************************
*
* function         : defines
* 
*****************************************************************************/



/*****************************************************************************
* 
* typedefs
*
*****************************************************************************/

#define IODHTTP_ADD_HEADER_SIZE 500

#define IODHTTP_MAX_FDS             64/*10*/

#define IODHTTP_USER_SIZE           64
#define IODHTTP_PROXY_SIZE          128


/* Smallest decimal value that doesn't fit in an unsigned long */
#define IODHTTP_MAX_INT_PLUS_1_DECIMAL "4294967296"

typedef enum
{
        tmHTTPIODriver_IodIdle,
        tmHTTPIODriver_IodReading
}tmHTTPIODriver_IodState_t;

typedef struct 
{
        tmHTTPIODriver_Client_t	hc;
        UInt32					size;
        Int						sizeValid;
        UInt32					pos;
        tmHTTPIODriver_IodState_t State;
        Int						CloseConnection;
        Char						filepath[HTTPCLIENT_URL_SIZE];
} tmHTTPIODriver_FD_t, *ptmHTTPIODriver_FD_t;

/*****************************************************************************
* 
* prototypes
*
*****************************************************************************/

tmErrorCode_t gtmHTTPIOClient_LastTcpStatus(void);

/*****************************************************************************
*
* function         : local variables
* 
*****************************************************************************/

static Int gHttpInit = 0;

static Int gtmHTTPIODriver_LastHttpStatus=0;

static Char iodHttpRecogPathStr[] = "http://";

static ptmHTTPIODriver_FD_t iodHttpFDs[IODHTTP_MAX_FDS];

static Int iodHttpUseServerAuth = 0;
static Char iodHttpServerAuthUser[IODHTTP_USER_SIZE] = "";
static Char iodHttpServerAuthPasswd[IODHTTP_USER_SIZE] = "";

static Int iodHttpUseProxy = 0;
static Char iodHttpProxy[IODHTTP_PROXY_SIZE] = "";
static Int iodHttpProxyPort = 0;

static Char iodHttpProxyAuthUser[IODHTTP_USER_SIZE] = "";
static Char iodHttpProxyAuthPasswd[IODHTTP_USER_SIZE] = "";

static Char iodHttpUserAgent[IODHTTP_USER_SIZE] ="Streamium/1.0";

static int stricmp(char* s1,char* s2)
{
        int retval = 1;

        while((*s1 != 0) && (*s2 != 0))
        {
                retval = toupper(*s1++) - toupper(*s2++);
                if(retval != 0)
                {
                        return retval;
                }
        }
        if ((*s1 == 0) && (*s2 == 0))
        {
                retval =0;
        }

        return retval;
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    last http status
*
*  description:  
*
****************************************************************************/
tmErrorCode_t tmHTTPIODriver_GetLastHttpStatus(void)
{
        return gtmHTTPIODriver_LastHttpStatus;
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    last http status
*
*  description:  
*
****************************************************************************/
tmErrorCode_t tmHTTPIODriver_GetLastTcpStatus(void)
{
        return gtmHTTPIOClient_LastTcpStatus();
}

/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    last http status
*
*  description:  
*
****************************************************************************/
tmErrorCode_t tmHTTPIODriver_SetTimeout(int millisecs)
{
        return tmHTTPIODriver_ClientSetGlobalTimeOuts( millisecs, millisecs, millisecs);
}


/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    last http status
*
*  description:  
*
****************************************************************************/
tmErrorCode_t tmHTTPIODriver_SetRangeToEndOfFile( Bool EndOfFile)
{
        int retval=0;

        if(EndOfFile)
        {
                retval = tmHTTPIODriver_ClientSetRangeMode( tmHTTPIODriver_RangeToEndOfFile);
        }
        else
        {
                retval = tmHTTPIODriver_ClientSetRangeMode( tmHTTPIODriver_RangeBlock);
        }
        return retval;;
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
static int iodHttpInit(void)
{
        memset(iodHttpFDs, 0, (IODHTTP_MAX_FDS * sizeof(ptmHTTPIODriver_FD_t)));  
        return 1;
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
static int iodHttpClose(ptmHTTPIODriver_FD_t f)
{
        int i;


        for (i = 0; (i < IODHTTP_MAX_FDS) && (iodHttpFDs[i] != f); ++i);

        if (i == IODHTTP_MAX_FDS)
        {
                printf("invalid file descriptor\n");
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        if(f->CloseConnection==1)
        {
                tmHTTPIODriver_ClientResetConnection(&(f->hc));
                //    tmHTTPIODriver_ClientClose(&(f->hc));
                f->CloseConnection=0;
        }

        free(f);
        iodHttpFDs[i] = NULL;

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
static void iodHttpTerm(void)
{
        int i;


        for (i = 0; i < IODHTTP_MAX_FDS; ++i)
        {
                if (iodHttpFDs[i] != NULL)
                {
                        iodHttpClose(iodHttpFDs[i]);
                }
        }

        tmHTTPIODriver_ClientDeRegister();
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

static int local_stricmp(const char*s1,const char*s2)
{
        int s;

        if((s1==0) || (s2==0))
                return TM_ERR_HTTPIODRIVER_FAILED;

        s = tolower(*s2) - tolower(*s1);

        while((s==0) && (*s1))
        {
                s1++;
                s2++;
                s = tolower(*s2) - tolower(*s1);
        }    
        if((*s1) == 0)
                s=0;
        return s;
}


/****************************************************************************
*
*  function: tmHTTPIODriver_Recog   
*
*  parameters:  char *path 
*
*  return value: int
*                    0 on error, 1 on success
*
*  description:  
*
****************************************************************************/
tmErrorCode_t  tmHTTPIODriver_Recog(char *path)
{

        //if (strncmp(path, iodHttpRecogPathStr, strlen(iodHttpRecogPathStr)) != 0)
        if (local_stricmp(iodHttpRecogPathStr, path) != 0)
        {
                printf("path does not match gtmHTTPIODriver_iod_DebugUnit's\n");
                return 0;
        }

        printf("path matches gtmHTTPIODriver_iod_DebugUnit's\n");
        return 1;
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
static int iodMakeHttpConnection(ptmHTTPIODriver_FD_t f, tmHTTPIODriver_Method_t httpClientMethod)
{
        int     i;
        int     httpStatus;
        char    path[HTTPCLIENT_URL_SIZE];
        int     notConnected = 1;
        int     ConnectionTries=0;
        char	buf[128];




        for (i = 0; (i < IODHTTP_MAX_FDS) && (iodHttpFDs[i] != f); ++i);

        if (i == IODHTTP_MAX_FDS)
        {
                printf("iodMakeHttpConnection [%p] invalid file descriptor\n", f);
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        strncpy(path, f->filepath, HTTPCLIENT_URL_SIZE);
        path[HTTPCLIENT_URL_SIZE - 1] = '\0';

        //verify connection is still alive.
        if(!tmHTTPIODriver_ClientIsConnected(&(f->hc)))   // if not connected on tcp level: connect
        {
                //        memset((void*)&(f->hc.s),0,sizeof(tcpSocket_t));

                memset((&(f->hc))->httpRequestHeaders, 0, HTTPCLIENT_HEADER_SIZE);
                memset((&(f->hc))->httpRequestMethod, 0, HTTPCLIENT_METHOD_SIZE);
                memset((&(f->hc))->httpRequestDefaultHeaders, 0, HTTPCLIENT_HEADER_SIZE);
                memset((&(f->hc))->httpRequestHeaders, 0, HTTPCLIENT_HEADER_SIZE);
                memset((&(f->hc))->httpResponseStatusString, 0, HTTPCLIENT_STATUS_SIZE);
                memset((&(f->hc))->httpResponseHeaders, 0, HTTPCLIENT_HEADER_SIZE);

        }

        if (iodHttpUseServerAuth == 1)
        {
                tmHTTPIODriver_ClientSetServerAuth(&(f->hc), iodHttpServerAuthUser, 
                        iodHttpServerAuthPasswd, 1);
        }
        else
        {
                tmHTTPIODriver_ClientSetServerAuth(&(f->hc), NULL, NULL, 0);
        }

        if (iodHttpUseProxy == 1)
        {
                tmHTTPIODriver_ClientSetProxy(&(f->hc), iodHttpProxy, iodHttpProxyPort, 1);

                tmHTTPIODriver_ClientSetProxyAuth(&(f->hc), iodHttpProxyAuthUser, 
                        iodHttpProxyAuthPasswd, 1);
        }
        else
        {
                tmHTTPIODriver_ClientSetProxy(&(f->hc), NULL, 0, 0);

                tmHTTPIODriver_ClientSetProxyAuth(&(f->hc), NULL, NULL, 0);
        }
        //tmHTTPIODriver_ClientSetPersistentConnection(&(f->hc), 0);
        tmHTTPIODriver_ClientSetPersistentConnection(&(f->hc), 1);

        tmHTTPIODriver_ClientSetUserAgent(&(f->hc), iodHttpUserAgent);
        tmHTTPIODriver_ClientSetAccept(&(f->hc), "*/*");

        tmHTTPIODriver_ClientClearHeaders(&(f->hc));

        tmHTTPIODriver_ClientSetChunked(&(f->hc), 0);
        tmHTTPIODriver_ClientSetRequestVersion(&(f->hc), tmHTTPIODriver_HTTP11);

        // by default enable the use of the cache
        tmHTTPIODriver_ClientDisableProxyCache(&(f->hc), False);
        for(ConnectionTries=0, notConnected = 1; (notConnected == 1) && (ConnectionTries < 10); ConnectionTries++)
        {
                if (tmHTTPIODriver_ClientSetUrl(&(f->hc), path) == TM_ERR_HTTPIODRIVER_FAILED)
                {
                        printf("invalid url\n");
                        //tmHTTPIODriver_ClientClose(&(f->hc));
                        printf("iodMakeHttpConnection [%p] invalid url\n", f);
                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
                if (tmHTTPIODriver_ClientConnect(&(f->hc)) == TM_ERR_HTTPIODRIVER_FAILED)
                {


                        //tmHTTPIODriver_ClientClose(&(f->hc));
                        printf("iodMakeHttpConnection [%p] failed to connect\n", f);
                        return TM_ERR_HTTPIODRIVER_FAILED;
                }
                if (tmHTTPIODriver_ClientSendMethod(&(f->hc), httpClientMethod) == TM_ERR_HTTPIODRIVER_FAILED)
                {


                        //tmHTTPIODriver_ClientClose(&(f->hc));
                        printf("iodMakeHttpConnection [%p] failed to send request\n", f);
                        return TM_ERR_HTTPIODRIVER_FAILED;
                }

                if (tmHTTPIODriver_ClientSendMessageBodyEnd(&(f->hc)) == TM_ERR_HTTPIODRIVER_FAILED)
                {

                        tmHTTPIODriver_ClientClose(&(f->hc));
                        printf("iodMakeHttpConnection [%p] failed to send message body\n", f);
                        return TM_ERR_HTTPIODRIVER_FAILED;
                }

                if (tmHTTPIODriver_ClientGetResponse(&(f->hc)) == TM_ERR_HTTPIODRIVER_FAILED)
                {


                        if ( httpClientMethod == tmHTTPIODriver_HeadMethod )
                        {
                                /* This is possibly a shoutcast server. A shoutcast server responds to a HEAD
                                request by dropping the TCP connection. As a workaround, step back to 1.0
                                and use GET method */
                                httpClientMethod = tmHTTPIODriver_GetMethod;
                                tmHTTPIODriver_ClientSetRequestVersion(&(f->hc), tmHTTPIODriver_HTTP10);
                                tmHTTPIODriver_ClientSetPersistentConnection(&(f->hc), 0 );

                                notConnected = 1;
                        }
                        else
                        {
                                tmHTTPIODriver_ClientClose(&(f->hc));
                                printf("iodMakeHttpConnection [%p] failed to get response\n", f);
                                return TM_ERR_HTTPIODRIVER_FAILED;
                        }
                }
                else
                {
                        httpStatus = tmHTTPIODriver_ClientGetStatusCode(&(f->hc));
                        gtmHTTPIODriver_LastHttpStatus = httpStatus;
                        switch(httpStatus)
                        {
                        case 100:       //Continue
                        case 200:       //OK
                                tmHTTPIODriver_ClientTurnoffRanging(&(f->hc));                
                                printf("status [%d] [%s]\n", httpStatus, tmHTTPIODriver_ClientGetStatusString(&(f->hc)));

                                // close connectin when done?
                                f->CloseConnection=0;
                                if (tmHTTPIODriver_ClientGetResponseHeader(&(f->hc), "Connection", buf, 128) > 0)
                                {
                                        if(stricmp(buf,"Close") == 0)
                                        {   
                                                f->CloseConnection=1;
                                        }

                                        printf("Connection: [%s]\n", buf);
                                }
                                else
                                {
                                        if( (tmHTTPIODriver_ClientGetStatusVersion(&(f->hc)) == tmHTTPIODriver_HTTP09) ||
                                                (tmHTTPIODriver_ClientGetStatusVersion(&(f->hc)) == tmHTTPIODriver_HTTP10) )
                                        {
                                                f->CloseConnection=1;
                                        }
                                }
                                notConnected = 0;
                                break;

                        case 206:       // Partial Content (after range)
                                printf("status [%d] [%s]\n", httpStatus, tmHTTPIODriver_ClientGetStatusString(&(f->hc)));

                                // close connectin when done?
                                f->CloseConnection=0;
                                if (tmHTTPIODriver_ClientGetResponseHeader(&(f->hc), "Connection", buf, 128) > 0)
                                {
                                        if(stricmp(buf,"Close") == 0)
                                        {   
                                                f->CloseConnection=1;
                                        }

                                        printf("Connection: [%s]\n", buf);
                                }
                                else
                                {
                                        if( (tmHTTPIODriver_ClientGetStatusVersion(&(f->hc)) == tmHTTPIODriver_HTTP09) ||
                                                (tmHTTPIODriver_ClientGetStatusVersion(&(f->hc)) == tmHTTPIODriver_HTTP10) )
                                        {
                                                f->CloseConnection=1;
                                        }
                                }
                                notConnected = 0;
                                break;
                        case 301:       //Moved Permanently
                        case 302:       //Found
                        case 303:       //See Other
                        case 307:       //Temporary Redirect
                                printf("status [%d] [%s]\n", httpStatus, tmHTTPIODriver_ClientGetStatusString(&(f->hc)));

                                tmHTTPIODriver_ClientClose(&(f->hc));

                                if (tmHTTPIODriver_ClientGetResponseHeader(&(f->hc), "Location", path, 
                                        HTTPCLIENT_URL_SIZE) == 0)
                                {

                                        tmHTTPIODriver_ClientClose(&(f->hc));

                                        printf("iodMakeHttpConnection [%p] no Location field found\n", f);
                                        return TM_ERR_HTTPIODRIVER_FAILED;
                                }
                                path[HTTPCLIENT_URL_SIZE - 1] = '\0';

                                // copy into filepath when moved permanently
                                if(httpStatus == 301 || httpStatus == 302 || httpStatus == 303)
                                {
                                        strncpy(f->filepath,path, HTTPCLIENT_URL_SIZE);
                                        f->filepath[HTTPCLIENT_URL_SIZE - 1] = '\0';
                                }

                                printf("redirecting to [%s]\n", path);

                                notConnected = 1;
                                break;
                        case 505:       //HTTP Version not supported
                                // step back to 1.0
                                tmHTTPIODriver_ClientSetRequestVersion(&(f->hc), tmHTTPIODriver_HTTP10);
                                tmHTTPIODriver_ClientSetPersistentConnection(&(f->hc), 0 );    //int usePersistent)

                                notConnected = 1;
                                break;

                        case 500:       //Internal Server Error
                                if ( (httpClientMethod == tmHTTPIODriver_HeadMethod) && (iodHttpUseProxy == 1) )
                                {
                                        /* This is possibly a shoutcast server. A shoutcast server responds to a HEAD
                                        request by dropping the TCP connection. The proxy will handle this by
                                        responding with status 500 to the client. As a workaround, step back to 1.0
                                        and use GET method */
                                        httpClientMethod = tmHTTPIODriver_GetMethod;
                                        tmHTTPIODriver_ClientSetRequestVersion(&(f->hc), tmHTTPIODriver_HTTP10);
                                        tmHTTPIODriver_ClientSetPersistentConnection(&(f->hc), 0 );

                                        notConnected = 1;
                                }
                                else
                                {
                                        printf("status [%d] [%s]\n", httpStatus, tmHTTPIODriver_ClientGetStatusString(&(f->hc)));
                                        //tmHTTPIODriver_ClientClose(&(f->hc));
                                        printf("iodMakeHttpConnection [%p] httpStatus [%d]\n", f, httpStatus);
                                        return TM_ERR_HTTPIODRIVER_FAILED;
                                }
                                break;

                        case 405:       //Method Not Allowed
                        case 501:       //Not Implemented
                                if (httpClientMethod == tmHTTPIODriver_HeadMethod)
                                {
                                        /* Retry with GET method */
                                        httpClientMethod = tmHTTPIODriver_GetMethod;

                                        notConnected = 1;
                                }
                                else
                                {
                                        printf("status [%d] [%s]\n", httpStatus, tmHTTPIODriver_ClientGetStatusString(&(f->hc)));
                                        //tmHTTPIODriver_ClientClose(&(f->hc));
                                        printf("iodMakeHttpConnection [%p] httpStatus [%d]\n", f, httpStatus);
                                        return TM_ERR_HTTPIODRIVER_FAILED;
                                }
                                break;
                        case 416:       //Requested range not satisfiable
                                // retry with cache disabled
                                if (iodHttpUseProxy)
                                {
                                        // only usefull when using proxy
                                        tmHTTPIODriver_ClientDisableProxyCache(&(f->hc), True);
                                        printf("status [%d]: procy cache temporarily disabled and retry.\n",httpStatus);
                                        // need to make a new connection as server sends a RST
                                        tmHTTPIODriver_ClientClose(&(f->hc));
                                        notConnected = 1;
                                }


                        case 101:       //Switching Protocols
                        case 201:       //Created
                        case 202:       //Accepted
                        case 203:       //Non-Authoritative Information
                        case 204:       //No Content
                        case 205:       //Reset Content
                        case 300:       //Multiple Choices
                        case 304:       //Not Modified
                        case 305:       //Use Proxy
                        case 400:       //Bad Request
                        case 401:       //Unauthorized
                        case 402:       //Payment Required
                        case 403:       //Forbidden
                        case 404:       //Not Found
                        case 406:       //Not Acceptable
                        case 407:       //Proxy Authentication Required
                        case 408:       //Request Time-out
                        case 409:       //Conflict
                        case 410:       //Gone
                        case 411:       //Length Required
                        case 412:       //Precondition Failed
                        case 413:       //Request Entity Too Large
                        case 414:       //Request-URI Too Large
                        case 415:       //Unsupported Media Type
                        case 417:       //Expectation Failed
                        case 502:       //Bad Gateway
                        case 503:       //Service Unavailable
                        case 504:       //Gateway Time-out
                        default:
                                printf("status [%d] [%s]\n", httpStatus, tmHTTPIODriver_ClientGetStatusString(&(f->hc)));
                                //tmHTTPIODriver_ClientClose(&(f->hc));
                                printf("iodMakeHttpConnection [%p] httpStatus [%d]\n", f, httpStatus);
                                return TM_ERR_HTTPIODRIVER_FAILED;
                        }
                }
        }

        /*
        tmHTTPIODriver_ClientGetStatusVersion(&(f->hc));
        tmHTTPIODriver_ClientGetStatusString(&(f->hc));
        */
        printf("iodMakeHttpConnection [%p] \n", f);
        return 0;
}

static ptmHTTPIODriver_FD_t iodHttpOpen(char *filepath, int oflag, int mode)
{
        int     i;
        ptmHTTPIODriver_FD_t   f;
        int		sizeLen;
        char buf[128];
        oflag=oflag;		/* parameter "oflag" was never referenced */
        mode=mode;			/* parameter "mode" was never referenced  */


        printf("iodHttpOpen [%s] [%d] [%d]\n", filepath, oflag, mode);

        gtmHTTPIODriver_LastHttpStatus=0;

        for (i = 0; (i < IODHTTP_MAX_FDS) && (iodHttpFDs[i] != NULL); ++i);

        if (i == IODHTTP_MAX_FDS)
        {
                printf("no more file descriptors\n");
                return (ptmHTTPIODriver_FD_t)TM_ERR_HTTPIODRIVER_FAILED;
        }

        if ((f = (ptmHTTPIODriver_FD_t) malloc(sizeof(tmHTTPIODriver_FD_t))) == NULL)
        {
                printf("failed to allocated file descriptor\n");
                return (ptmHTTPIODriver_FD_t)TM_ERR_HTTPIODRIVER_FAILED;
        }
        memset(f, 0, sizeof(tmHTTPIODriver_FD_t));
       

        iodHttpFDs[i] = f;

        //strncpy(f->filepath, filepath, HTTPCLIENT_URL_SIZE);
        {   int i=0,j=0;

        while((filepath[i] != 0) && (j < HTTPCLIENT_URL_SIZE-1))
        {
                switch(filepath[i])
                {
                case ' ':
                        f->filepath[j]='%';j++;
                        f->filepath[j]='2';j++;
                        f->filepath[j]='0';j++;
                        i++;
                        break;
                default:
                        f->filepath[j]=filepath[i];
                        i++;j++;
                        break;
                }
        }
        f->filepath[j]=0;
        }



        f->filepath[HTTPCLIENT_URL_SIZE - 1] = '\0';
        f->filepath[0]=tolower(f->filepath[0]);
        f->filepath[1]=tolower(f->filepath[1]);
        f->filepath[2]=tolower(f->filepath[2]);
        f->filepath[3]=tolower(f->filepath[3]);

        f->State=tmHTTPIODriver_IodIdle;
        f->CloseConnection=0;

        if ((tmHTTPIODriver_ClientCreate(&(f->hc), NULL, TM_HTTPIODRIVER_RECVBUF_SIZE, NULL,TM_HTTPIODRIVER_SENDBUF_SIZE)) == TM_ERR_HTTPIODRIVER_FAILED)
        {
                printf("failed to create http client\n");
                free(f);
                iodHttpFDs[i] = NULL;
                return (ptmHTTPIODriver_FD_t)TM_ERR_HTTPIODRIVER_FAILED;
        }

        /* We use the GET method in the open because there are a lot of incorrect implementations
        of the HEAD method out there. (e.g. IIS, shoutcast, Allegro, ...)
        Also, some URL's (e.g.) musicmatch) exist that can only be opened once because they
        contain a generated ticket. Opening them with a HEAD method would invalidate the
        subsequent GET request. */
        if(iodMakeHttpConnection(f,tmHTTPIODriver_GetMethod) == TM_ERR_HTTPIODRIVER_FAILED)
        {
                tmHTTPIODriver_ClientClose(&(f->hc));

                // remove file descriptor from table
                free(f);
                iodHttpFDs[i] = NULL;
        }
        else
        {
                sizeLen = tmHTTPIODriver_ClientGetResponseHeader(&(f->hc), "Content-Length", buf, 128);
                /* pSOS limits the size of a file to 0xffffffff. So, we consider a size valid if:
                - a Content-Length (which is not mandatory) is present (sizeLen > 0)
                - the size of the string is less than the size of the smallest decimal number that
                cannot be stored in an unsigned long
                - the size of the string is equal to	the size of the smallest decimal number that
                cannot be stored in an unsigned long AND the content of the string is alphabetically
                smaller than the smallest decimal number that cannot be stored in an unsigned long
                This is needed for shoutcast servers, which don't return a Content-Length, and for
                files larger than approximately 4 Gigabyte. */
                if ( ((sizeLen > 0) && (sizeLen < strlen(IODHTTP_MAX_INT_PLUS_1_DECIMAL))) ||
                        ((sizeLen == strlen(IODHTTP_MAX_INT_PLUS_1_DECIMAL)) && (strcmp(buf, IODHTTP_MAX_INT_PLUS_1_DECIMAL) < 0))
                        )
                {
                        f->size = atoi(buf);
                        f->sizeValid = 1;
                        printf("file size valid [%s] [%d]\n", buf, f->size);
                }
                else
                {
                        f->size = 0;
                        f->sizeValid = 0;
                        printf("file size invalid\n");
                }
                f->pos = 0;


                f->State=tmHTTPIODriver_IodReading;

                return f;
        }

        return (ptmHTTPIODriver_FD_t)TM_ERR_HTTPIODRIVER_FAILED;
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
static int iodHttpRead(ptmHTTPIODriver_FD_t f, char *buf, int len)
{
        int     i;
        int     r=0;
        int     finished=0;
        int		nrOfTries=0;

        printf("iodHttpRead [%p] [%p] [%d]\n", f, buf, len);

        for (i = 0; (i < IODHTTP_MAX_FDS) && (iodHttpFDs[i] != f); ++i);

        if (i == IODHTTP_MAX_FDS)
        {
                printf("invalid file descriptor\n");
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        if (f->sizeValid)
        {
                if (f->pos >= f->size)
                {			 
                        f->pos = f->size;

                        printf("Reading beyond end of file \n");
                        // EOF
                        return 0;
                }

                if ( (f->pos + len) > f->size )
                {
                        // decreasing len to avoid timeouts
                        len = f->size - f->pos;
                }

                if( tmHTTPIODriver_ClientGetRangeRequested(&(f->hc)))
                {
                        int endrange = 0;
                        //endrange = (f->pos+len-1 < f->size) ? f->pos+len-1 : f->size;
                        endrange = f->pos + len-1;

                        tmHTTPIODriver_ClientSetRange(&(f->hc), f->pos, endrange);
                }
        }

        while((!finished) && (nrOfTries < 5))
        {
                nrOfTries++;

                if(!tmHTTPIODriver_ClientIsConnected(&(f->hc)))
                {
                        if(f->State == tmHTTPIODriver_IodReading)
                        {
                                //if( tmHTTPIODriver_ClientGetRangeRequested(&(f->hc)) == 0)
                                //{
                                // no range requested before
                                tmHTTPIODriver_ClientSetRange(&(f->hc),f->pos,f->size);
                                //}
                                //else
                                //{   
                                //    // complete ranged request
                                // /   tmHTTPIODriver_ClientSetRange(&(f->hc),f->pos,f->hc.httpRequestRangeHigh);
                                //}
                        }        
                        f->State = tmHTTPIODriver_IodIdle;
                }

                if(f->State == tmHTTPIODriver_IodIdle)
                {
                        char rangebuf[HTTPCLIENT_RESPONS_VALUE_SIZE];

                        r = iodMakeHttpConnection(f,tmHTTPIODriver_GetMethod);

                        if (r==0)
                        {
                                if (tmHTTPIODriver_ClientGetResponseHeader(&(f->hc), "Content-Range", rangebuf, HTTPCLIENT_RESPONS_VALUE_SIZE) > 0)
                                {
                                        unsigned long start=0;
                                        unsigned long end=0;
                                        unsigned long size=0;

                                        sscanf(rangebuf,"bytes %d-%d/%d",&start,&end,&size);

                                        printf("%s %d Range:%d-%d/%d\n ",__FILE__,__LINE__,start,end,size);

                                        if(start != f->pos)
                                        {
                                                tmHTTPIODriver_ClientClose(&(f->hc));
                                                f->CloseConnection=0;
                                                return TM_ERR_HTTPIODRIVER_FAILED;
                                        }
                                }
                        }
                }
                else
                {
                        r=0;
                }

                if(r == 0)
                {
                        f->State = tmHTTPIODriver_IodReading;

                        r =  tmHTTPIODriver_ClientGetResponseBody(&(f->hc), buf, len);
                        printf("got [%d] bytes\n", r);

                        // finished reading 
                        if(r == 0) //f->pos == f->size)
                        {
                                f->State=tmHTTPIODriver_IodIdle;

                                if(f->CloseConnection==1)
                                {
                                        printf("Close connection\n");

                                        tmHTTPIODriver_ClientClose(&(f->hc));
                                        f->CloseConnection=0;
                                }
                        }
                        else if(r > 0)
                        {
                                // bump position
                                f->pos+=r;

                                // DBG_PRINT((gtmHTTPIODriver_iod_DebugUnit, DBG_STATE, "new pos: %d\n", f->pos))

                                if(f->sizeValid)
                                {
                                        if(f->pos == f->size)
                                        {
                                                printf("%s %d f->State = tmHTTPIODriver_IodIdle;\n",__FILE__,__LINE__);

                                                f->State=tmHTTPIODriver_IodIdle;
                                        }

                                        if(tmHTTPIODriver_ClientGetRangeMode() == tmHTTPIODriver_RangeBlock)
                                        {
                                                if(tmHTTPIODriver_ClientGetRangeRequested(&(f->hc)))
                                                {
                                                        printf("%s %d f->State = tmHTTPIODriver_IodIdle;\n",__FILE__,__LINE__);

                                                        f->State=tmHTTPIODriver_IodIdle;
                                                }
                                        }
                                }
                        }
                        finished=1;
                }
                else
                {
                        printf("%s %d iodMakeHttpConnection returned unknown value \n",__FILE__,__LINE__);
                        f->State=tmHTTPIODriver_IodIdle;
                }
        }
        return r;
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
static int iodHttpWrite(ptmHTTPIODriver_FD_t f, char *buf, int len)
{
        int i;
        int r=TM_ERR_HTTPIODRIVER_FAILED;
        buf=buf;		/* parameter "buf" was never referenced */
        len=len;		/* parameter "len" was never referenced */

        printf("iodHttpWrite [%p] [%p] [%d]\n", f, buf, len);

        for (i = 0; (i < IODHTTP_MAX_FDS) && (iodHttpFDs[i] != f); ++i);

        if (i == IODHTTP_MAX_FDS)
        {
                printf("invalid file descriptor\n");

                return TM_ERR_HTTPIODRIVER_FAILED;
        }


        return r;
}

//
/****************************************************************************
*
*  function:     
*
*  parameters:   
*
*  return value: int
*                    TM_ERR_HTTPIODRIVER_FAILED on error, offset from start of file on success
*
*  description:  Define the seek function which sets the current file position.
*
****************************************************************************/

static int iodHttpSeek(ptmHTTPIODriver_FD_t f, long offset, int whence)
{
        long origin;

        printf("%s %d iodHttpSeek [%p]\n",__FILE__,__LINE__, f);

        if (f == NULL)
        {
                printf("The file is not opened\n");

                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        if ((!(f->sizeValid)) && (offset!=0))  // Size will only be invalid when size was larger then 4 gigabytes.
        {                                      // In case size is invalid only jumps to beginning of file are allowed.


                printf("The file size is not valid, seek not supported\n");

                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        /* Determine new absolute file position */

        switch(whence)
        {
        case TM_HTTPIODRIVER_SEEK_SET:

                origin=offset;
                break;

        case TM_HTTPIODRIVER_SEEK_CUR:

                origin = f->pos + offset;
                break;

        case TM_HTTPIODRIVER_SEEK_END:
                origin = f->size + offset;
                break;

        default:
                return TM_ERR_HTTPIODRIVER_FAILED;
        }

        if (f->pos != origin)
        {
                f->pos = origin;

                tmHTTPIODriver_ClientSetRange(&(f->hc), f->pos, f->size);

                // if state is still reading, reset connection  
                if(f->State != tmHTTPIODriver_IodIdle)
                {
                        printf("%s %d tmHTTPIODriver_ClientResetConnection\n",__FILE__,__LINE__);

                        tmHTTPIODriver_ClientResetConnection(&(f->hc));
                        f->State = tmHTTPIODriver_IodIdle;
                        f->CloseConnection=0;
                }  
        }
        else
        {
                printf("seek to filepos (%u) is equal to current file pos\n", origin);
                // no need to change range, file pos or connection
        }

        printf("Current file position = %d\n", f->pos);

        return f->pos;
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
static int iodHttpIsatty(ptmHTTPIODriver_FD_t fd)
{
        fd=fd; /* parameter "fd" was never referenced */
        printf("iodHttpIsatty [%p]\n", fd);

        return 1;
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
static int iodHttpFstat(ptmHTTPIODriver_FD_t fd,  struct stat *st)
{
        int retval = 0;

        printf("iodHttpFstat [%p] [%p]\n", fd, st);

        memset(st, 0, sizeof(*st));

        if (fd->sizeValid)
        {	
                st->st_size = fd->size;

                st->st_blksize = 1460; // data in 1 ethernet packet
                st->st_blocks = (fd->size / st->st_blksize) + 1;
        }
        else
        {
                printf("The file size is not valid\n");
                errno = EFBIG;
                retval = TM_ERR_HTTPIODRIVER_FAILED;
        }

        return retval;

        /*
        struct  stat {
        dev_t   st_dev;
        ino_t   st_ino;
        mode_t  st_mode;
        short   st_nlink;
        uid_t   st_uid;
        gid_t   st_gid;
        dev_t   st_rdev;
        off_t   st_size;
        time_t  st_atime;
        int st_spare1;
        time_t  st_mtime;
        int st_spare2;
        time_t  st_ctime;
        int st_spare3;
        long    st_blksize;
        long    st_blocks;
        long    st_spare4[2];
        };

        */

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
tmErrorCode_t tmHTTPIODriver_SetServerAuth(int useAuth, char *user, char *passwd)
{
        printf("tmHTTPIODriver_SetServerAuth [%d] [%s] [%s]\n", useAuth, user, 
                passwd);

        iodHttpUseServerAuth = useAuth;

        if (useAuth == 1)
        {
                memcpy(iodHttpServerAuthUser, user, IODHTTP_USER_SIZE);
                memcpy(iodHttpServerAuthPasswd, passwd, IODHTTP_USER_SIZE);
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
tmErrorCode_t tmHTTPIODriver_SetProxy(char *proxy, int port)
{
        printf("tmHTTPIODriver_SetProxy [%s] [%d]\n", proxy, port);

        if (proxy != 0)
        {
                int len=strlen(proxy) >  IODHTTP_PROXY_SIZE-1 ? IODHTTP_PROXY_SIZE-1 : strlen(proxy);

                memcpy(iodHttpProxy, proxy, len);
                iodHttpProxy[len]=0;
                iodHttpProxy[IODHTTP_PROXY_SIZE-1]=0;

                if(port != 0)
                        iodHttpProxyPort = port;
                else
                        iodHttpProxyPort = 80;

                iodHttpUseProxy = 1;
        }
        else
        {
                iodHttpUseProxy = 0;
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
tmErrorCode_t tmHTTPIODriver_SetUserAgent(char*agent)
{
        printf("iodHttpUseragent [%s] \n", agent);

        if(agent != 0)
        {
                int len=strlen(agent) > IODHTTP_USER_SIZE-1 ? IODHTTP_USER_SIZE-1 : strlen(agent);


                strncpy(iodHttpUserAgent,agent,len+1);
                iodHttpUserAgent[IODHTTP_USER_SIZE-1]=0;

                printf("%s %d iodHttpUserAgent:[%s] len:%d \n", __FILE__,__LINE__,iodHttpUserAgent,len);

        }
        else
        {
                strcpy(iodHttpUserAgent,"Streamium/1.0");
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
static int ioHttpFcntl(ptmHTTPIODriver_FD_t fd, int cmd, int flags)
{
        int result=0;
        flags=flags;	/* parameter "flags" was never referenced */

        printf("ioHttpFcntl\n");


        switch(cmd)
        {
        case TM_HTTPIODRIVER_GET_HTTPSTATUS:
                printf("ioctl: IOCTL_GET_HTTPSTATUS\n");

                result = tmHTTPIODriver_ClientGetStatusCode(&(fd->hc));
                gtmHTTPIODriver_LastHttpStatus = result;
                break;
        default:
                result=TM_ERR_HTTPIODRIVER_FAILED;
                printf("unrecognised ioHttpFcntl\n");
                break;
        }
        return result;
}

/************************************************************************/
/*重新封装HTTPclient库                                                  */
/************************************************************************/
//  [2011年1月20日 9:48:31 by ljw]
int  HttpClientInit()
{
        if(gHttpInit == 0)
        {
                gHttpInit = 1;
                iodHttpInit();
        }
}

int  HttpClientOpen(char *url) //return fd if(fd < 0)error;
{
        ptmHTTPIODriver_FD_t f = NULL;
        int i = 0;
        HttpClientInit(); //初始化。
        f = iodHttpOpen(url,0,0);

        for (i = 0; (i < IODHTTP_MAX_FDS) && (iodHttpFDs[i] != f); ++i);

        if (i == IODHTTP_MAX_FDS)
        {
                printf("open failed url = %s\n",url);
                return TM_ERR_HTTPIODRIVER_FAILED;
        }
        return i;

}

unsigned long  HttpClientGetContentLength(int fd)
{
        ptmHTTPIODriver_FD_t f = NULL;

        f = iodHttpFDs[fd];
        if(f != NULL && f != 0xffffffff)
        {

                return f->size;
        }
        return TM_ERR_HTTPIODRIVER_FAILED;
}


int  HttpClientRead(int fd, char *buf, int len)
{
        ptmHTTPIODriver_FD_t f = NULL;

        f = iodHttpFDs[fd];
        if(f != NULL && f != 0xffffffff)
        {
               return iodHttpRead(f,buf,len);
               
        }
        return TM_ERR_HTTPIODRIVER_FAILED;      
}
int  HttpClientClose(int fd)
{
        ptmHTTPIODriver_FD_t f = NULL;

        f = iodHttpFDs[fd];
        if(f != NULL && f != 0xffffffff)
        {
                return iodHttpClose(f);
        }
        return TM_ERR_HTTPIODRIVER_FAILED;
}

