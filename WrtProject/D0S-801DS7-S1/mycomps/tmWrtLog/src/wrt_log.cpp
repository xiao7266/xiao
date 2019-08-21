#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "wrt_msg_queue.h"
#include "wrt_log.h"
#include "zenmalloc.h"

#define closesocket close

static WrtLogCallbacks callbacks = { malloc,free,realloc};

unsigned int wrt_log_mask=LOG_MESSAGE|LOG_DEBUG|LOG_ERROR|LOG_WARNING;

FILE* __log_file = NULL;

static char wrt_id[16]={'\0'};
static int g_log_init = 0;
static unsigned char g_log_module_exit  = 0;
static SDL_sem*  g_logsem = NULL;
static CWRTMsgQueue  g_msgqueue;
static SDL_mutex* g_log_lock = NULL;

static int            g_remote = 0;

#define INVALID_SOCKET -1
#define ENDLINE "\r\n"
#define MSG_SEND_LOG  0x01
#define MSG_EXIT      0x02
#define MSG_NET_INIT  0x03

static int send_log_msg(char* msg,int len);

static void enter_log_lock()
{
        if(g_log_lock != NULL)
                SDL_LockMutex(g_log_lock);
}

static void exit_log_lock()
{
        if(g_log_lock != NULL)
                SDL_UnlockMutex(g_log_lock);
}


static char * wrt_strdup_vprintf(const char *fmt, va_list ap)
{
	/* Guess we need no more than 100 bytes. */

	int n, size = 200;
	char *p,*np;

	if ((p = (char *) callbacks.malloc (size)) == NULL)
		return NULL;
	while (1)
	{
		/* Try to print in the allocated space. */

		n = vsprintf (p, fmt, ap);

		/* If that worked, return the string. */
		if (n > -1 && n < size)
			return p;
		//printf("Reallocing space.\n");
		/* Else try again with more space. */
		
		if (n > -1)	/* glibc 2.1 */
			size = n + 1;	/* precisely what is needed */
		else		/* glibc 2.0 */
			size *= 2;	/* twice the old size */
			
		if ((np = (char *) callbacks.realloc (p, size)) == NULL)
		{
			callbacks.free(p);
			return NULL;
		}
		else
		{
			p = np;
		}
	}//end of while
	return NULL;
}

static void __wrt_logv_out(WrtLogLevel lev, const char *fmt, va_list args,const char* _file,const int _line)
{
        const char *lname="undef";
        char* msg;
        time_t now;
        char  _date[30];  
        if (__log_file == NULL) 
                __log_file = stderr;
        switch(lev){
                case LOG_DEBUG:
                        lname="#wrt-debug";
                        break;
                case LOG_MESSAGE:
                        lname="#wrt-message";
                        break;
                case LOG_WARNING:
                        lname="#wrt-warning";
                        break;
                case LOG_ERROR:
                        lname="#wrt-error";
                        break;
                default:
                        lname="#wrt-undef";
                        break;
        }
       
        msg = wrt_strdup_vprintf(fmt,args);
        if(msg == NULL){
        	printf("wrt_strdup_vprintf failed \n");
        	return;
        }
        time(&now);
        sprintf(_date,"%s",ctime(&now));
        if(g_remote == 1){
                int len;
                char* msgbuf=NULL;
                if(_file)
                	len = strlen(msg)+ strlen(lname)+strlen(wrt_id)+strlen(_date)+30+strlen(_file);
                else
                	len = strlen(msg)+ strlen(lname)+strlen(wrt_id)+strlen(_date)+30;
                msgbuf = (char*)callbacks.malloc(len);
                if(msgbuf == NULL){
                	callbacks.free(msg);
                	return;
                }
                memset(msgbuf,0,len);
                if(_file == NULL)
                        sprintf(msgbuf,"%s %s %s %s\r\n",lname,wrt_id,msg,_date);
                else
                        sprintf(msgbuf,"%s %s %s %s:%d %s\r\n",lname,wrt_id,msg,_file,_line,_date);
              //  fprintf(__log_file,"%s",msgbuf);
                send_log_msg(msgbuf,len);
                callbacks.free(msg);
                callbacks.free(msgbuf);
     
              //  fflush(__log_file);
                return;
        }else{
                 if(_file == NULL)
                 	fprintf(__log_file,"%s %s %s %s" ENDLINE,lname,wrt_id,msg,_date);
                 else
                 	fprintf(__log_file,"%s %s %s %s:%d %s" ENDLINE,lname,wrt_id,msg,_file,_line,_date);
                 fflush(__log_file);//打印到标准输出
                //__log_file == stderr ,stdout
                //printf("%s %s %s %s:%d %s" ENDLINE,lname,wrt_id,msg,_file,_line,_date);
        }
        callbacks.free(msg);
}

WrtLogFunc wrt_logv_out = __wrt_logv_out;

void wrt_set_log_level_mask(int levelmask)
{
    wrt_log_mask = levelmask;
}

void wrt_set_log_handler(WrtLogFunc func)
{
    wrt_logv_out=func;
}

void wrt_set_log_id(char* id)
{
    memset(wrt_id,'\0',16);
    memcpy(wrt_id,id,15);
}

void wrt_debug(char* _file,int _line,const char *fmt,...)
{
	va_list args;
	if(wrt_log_level_enabled(LOG_END))
	    return;
	enter_log_lock();
	va_start (args, fmt);
	wrt_logv(LOG_DEBUG, fmt, args,_file,_line);
	va_end (args);
	exit_log_lock();
}

void wrt_message(const char *fmt,...)
{
	va_list args;
	if(wrt_log_level_enabled(LOG_END))
	    return;
	enter_log_lock();
	va_start (args, fmt);
	wrt_logv(LOG_MESSAGE, fmt, args,NULL,0);
	va_end (args);
	exit_log_lock();
}

void wrt_warning(char* _file,int _line,const char *fmt,...)
{
	va_list args;
	if(wrt_log_level_enabled(LOG_END))
	    return;
	enter_log_lock();
	va_start (args, fmt);
	wrt_logv(LOG_WARNING, fmt, args,_file,_line);
	va_end (args);
	exit_log_lock();
}

void wrt_error(char* _file,int _line,const char *fmt,...)
{
	va_list args;
	if(wrt_log_level_enabled(LOG_END))
	    return;
	enter_log_lock();
	va_start (args, fmt);
	wrt_logv(LOG_ERROR, fmt, args,_file,_line);
	va_end (args);
	exit_log_lock();
}

//  [2011年3月30日 14:17:15 by ljw]
//////////////////////////////////////////////////////////////////////////////////////////////
//
static int net_init(short port)
{
	unsigned int sckop;
	int sd;
	struct sockaddr_in addr;
	sd = socket(AF_INET,SOCK_DGRAM,0);
	if(sd == INVALID_SOCKET){
		printf("create socket failed");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port+2);/*iPort);*/
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sd,(struct sockaddr*)&addr,sizeof(struct sockaddr))){
		printf("bind socket failed");
		return -1;
	}
	return sd;
}

static int net_send(int fd,struct sockaddr_in* addr,char* msg ,int len)
{
	return sendto(fd,msg,len,0,(struct sockaddr *)addr,sizeof(struct sockaddr));
}

static int send_log_msg(char* msg,int len)
{
	return  g_msgqueue.send_message(MSG_SEND_LOG,msg,len,g_logsem);
}

static int log_process(void* pvoid){
        CWRTMsg* pMsg;
        int sd = -1;
        int ivalue = 0;
        struct sockaddr_in _addr;

        while(!g_log_module_exit){

                SDL_SemWait(g_logsem);
                pMsg = g_msgqueue.get_message();
                if(pMsg == NULL)
                	continue;
                ivalue = pMsg->get_value();
                if(ivalue == MSG_EXIT){
                	if(pMsg)
                		ZENFREE(pMsg);
                        break;
                }
                if(ivalue == MSG_SEND_LOG){
                        int ret;
                        uint32_t msglen = 0;
                        char* pTemp = NULL;
                        pTemp = (char*)pMsg->get_message(msglen);
                        if(sd != -1)
                                ret  =  net_send(sd,&_addr,(char*)pTemp,msglen);
                       
                        	
                }
                if(ivalue == MSG_NET_INIT)
                {
                	uint32_t msglen = 0;
                        char* pTemp = NULL;
                        unsigned long msg[2];
                        pTemp = (char*)pMsg->get_message(msglen);

                        memcpy(msg,(void*)pTemp,msglen);

                	if(sd != -1){
                		_addr.sin_family = AF_INET;
                        _addr.sin_port = htons(msg[1]);/*iPort);*/
                        _addr.sin_addr.s_addr = msg[0];
                        pMsg->free_message();
						if(pMsg)
                			ZENFREE(pMsg);                                
                        continue;
                		
                	}
                    sd = net_init(msg[2]);
                    if(sd != -1)
                    {
                        _addr.sin_family = AF_INET;
                        _addr.sin_port = htons(msg[1]);/*iPort);*/
                        _addr.sin_addr.s_addr = msg[0];
                    }else{
                        printf("log connnect sever(%x:%d) failed",msg[1],msg[2]);
                    }
                }
                pMsg->free_message();
		if(pMsg)
			ZENFREE(pMsg);                
                
        }
        closesocket(sd);

}

void wrt_log_init()
{
	if(g_log_init == 1)
		return;
	g_log_module_exit = 0;
	g_log_lock = SDL_CreateMutex();
	g_logsem =  SDL_CreateSemaphore(0);
	SDL_CreateThread((int (*)(void*))log_process, NULL);
	g_log_init = 1;
	strcpy(wrt_id,"000000000000000");
}

void wrt_log_uninit()
{
	g_log_module_exit = 1;
    g_msgqueue.send_message(MSG_EXIT,g_logsem);
    SDL_DestroyMutex(g_log_lock);
    g_log_lock = NULL;
    SDL_DestroySemaphore(g_logsem);
}

void wrt_set_log_server_addr(unsigned long ip,short port)
{
    unsigned long msg[2]={0};
    if(ip == 0)
        return;
    if(port == 0)
        port = 60000;

    MSG_NET_INIT;
    msg[0] = ip;
    msg[1] = port;
    g_msgqueue.send_message(MSG_NET_INIT,(void*)msg,sizeof(msg),g_logsem);
}

int wrt_get_log_remote()
{
	return g_remote;
}

void wrt_set_log_remote(int _start)
{
	g_remote = _start;
}

void wrt_set_log_memory_adapter(WrtLogCallbacks* _callback)
{
	if(_callback == NULL)
		return;
	if(_callback->malloc)
		callbacks.malloc = _callback->malloc;
	if(_callback->free)
		callbacks.free = _callback->free;
	if(_callback->realloc)
		callbacks.realloc = _callback->realloc;
}

