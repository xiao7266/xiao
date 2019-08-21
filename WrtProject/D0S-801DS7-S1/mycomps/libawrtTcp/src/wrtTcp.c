/*
build  for linux library and android JNI library,by xiao
attention:never use any global variable from main function or any other library in this file;
		main function or any other library never use any global variable from this library;
		we use the callback function(tcpServerToMain) to send and get global variable.
		we use json data to communicate because we can add any data in json data at will.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <malloc.h>
#include <getopt.h>
#include <strings.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/ioctl.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <time.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <net/if.h>
#include <linux/sockios.h>	
#include <unistd.h>
#include "wrtTcp.h"
#include "CmdId.h"
static pthread_t tidListen;
static unsigned char g_sysupdata=0;
int handleComingCall(T_CTLTHREADINFO *p);
int initContracker();
int getLocalID();
int makeOneCall(cJSON *json);
/*
tcpServerToMain: this is a callback function,we can use it to send or ask data to main function
attention:jsonReturn: can't be NULL when we call this callback function,and jsonReturn must be released before we "return" in any function
		structBodyReturn:reserved for expansion when the library want to get some parameter from main funciton,not used by xiao
*/
int (*tcpServerToMain)(cJSON *json,cJSON **jsonReturn,char *structBody,char**structBodyReturn) = NULL;
int defaultTcpServerToMain(cJSON *json,cJSON **jsonReturn,char *structBody,char**structBodyReturn)
{
	printf("error:tcpServerToMain didn't defined\r\n");
	return ERROR_TCP_SERVER_CALLBACK_FUNCTION_NOT_DEFINED;
}
static int handlerAddrtable(int sd, struct _net_pack_ *np, int usedbytes)
{
	cJSON *jsonData = NULL,*jsonReturn = NULL;//must be released before return
	char *fileName = NULL;
	if((jsonData=cJSON_CreateObject()) == NULL){printf("json create error\r\n");return false;};
	cJSON_AddNumberToObject(jsonData,CJSON_DATE_CMD,CJSON_CMD_HANDLE_IP_ADDRESS_TABLE);
	cJSON_AddNumberToObject(jsonData,CJSON_IP_SOCKET_FD,sd);
	cJSON_AddNumberToObject(jsonData,CJSON_TCP_USED_BYTES,usedbytes);
	tcpServerToMain(jsonData,&jsonReturn,(char *)np,NULL);//handle it in the callback function
	cJSON_Delete(jsonData);
	if(jsonReturn){cJSON_Delete(jsonReturn);}
	return ture;
}

/*
struct threadinfo_t : only used in this file (in function taskListen)
*/
struct threadinfo_t
{
	pthread_t tid;
	int  fd;     
	unsigned int ip;
	sem_t  sem;
};
struct threadinfo_t threadinfo;

void *taskprocess(void *pParam)
{
    T_CTLTHREADINFO   p;
    //int sd = -1;
    int ret,count=0;
    int mode = 1;
    char buf[512]="";
    fd_set readfd;
    struct timeval timeout;
    struct _net_pack_ np;
    unsigned long  ip;
    ip = threadinfo.ip;
	p.index = threadinfo.fd;
	sem_post(&threadinfo.sem);//make sure listen thread can listen again   
	getLocalID();
    printf("  taskprocess sd = %d, ip=%x, port = %d\n", p.index, ip, htons(p.in_client_addr.sin_port));
    
    ioctl(p.index,FIONBIO,(char*)&mode);

WAITING_MONT_RECV:
    timeout.tv_sec = 1;
    timeout.tv_usec =0;
    FD_ZERO(&readfd);
    if(p.index > 0)
        FD_SET(p.index,&readfd);

    ret = select(p.index+1,&readfd,NULL,NULL,&timeout);
    if(!FD_ISSET(p.index,&readfd))
    {
        count ++;
        if(count<6)goto WAITING_MONT_RECV;  //6
         printf("WAITING_MONT_RECV time_out1 \n");
        goto PROCESS_END;
    }
    ret = recv(p.index, (char*)buf, 512, 0);
    printf("taskprocess ret=%d\r\n",ret);
    if(ret<=0){
        if(errno==0x5023){
            count ++;
            if(count<6)goto WAITING_MONT_RECV;  //6
        }
		printf("quan:----------WAITING_MONT_RECV time_out2 \r\n");
        goto PROCESS_END;
    }
    memcpy(&np,buf,ret);

    printf("recv %d bytes,head: %s ip: %08x,cmd: 0X%04X, len: 0x%08x\r\n", ret,np.head,ip,np.uCmd,np.len);
    if(strncmp(np.head,"WRTI",4))
    { 
		    printf("data illegal: do not contain WRTI\r\n");
        goto PROCESS_END;
    }
    if(ret>np.len)
    {
		    printf("recv len do not match\r\n");   
        goto PROCESS_END;
    }
    p.uCmd = np.uCmd;
    switch(np.uCmd)
    	{
    	case DOOR_CALLROOM://outdoor machine call 
    	case ROOM_CALLROOM://indoor machine call indoor machine
		case CENTER_CALLROOM://control center call
			printf("ddddddddddd  getCall cmd=0x%x\r\n",np.uCmd);
			handleComingCall(&p);
			break;
		case CENTER_UPGRADEROOM://control center upgrade indoor machine
			//printf("ccccccccccc  CENTER_UPGRADEROOM\r\n");
			//downLoadFromCenter(&p,&np);
			break;
		case CENTER_DOWNLOADROOM://down load the address table from center
			printf("ccccccccccc  CENTER_DOWNLOADROOM\r\n");
			if(g_sysupdata == 1){printf("updating now\r\n");break;}
			g_sysupdata = 1;
			handlerAddrtable(p.index, &np, ret);
			g_sysupdata = 0;
			break;
		case CENTER_SEND_SYSINFO://center send configuration to the indoor machine
			printf("ccccccccccc  CENTER_SEND_SYSINFO\r\n");
			//centerSendConfiguration(sd, &np, ret);
			break;
		case CENTER_GET_SYSINFO://center request local information of indoor machine
			break;
		case CENTER_BROADCASTDATA://center send character message
			printf("%s:center send character message\r\n",__FUNCTION__);
			break;
		case CENTER_BROADCASTPIC://center send picture message
			printf("%s:center send picture message\r\n",__FUNCTION__);
			break;
		default:
			printf("eeeeeeeeeeeee error cmd\r\n");
    	}
    //end of switch(np.uCmd)
     
PROCESS_END:
    usleep(10);
    //ctrl_thread_info_del(sd);
    close(p.index);
    printf("have close socket sd=%d\n",p.index);
    //pthread_detach(pthread_self()); 
    pthread_exit(0);
	return NULL;
}

void *taskListen(void *pParam)      
{
    int result;
    int ret;
    int sd_local, sd_channel;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addrlen;
    int reuse;
    int index;
    pthread_attr_t attr;
    addrlen = sizeof(struct sockaddr_in);
    sem_init(&threadinfo.sem, 0,0);
	
    sd_local = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(20200);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    result = bind(sd_local, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if(result == 0){printf("bind successfully\r\n");}
	else{printf("bind error\r\n");}
    listen(sd_local, 512);
    while(1)
    {
        sd_channel = accept(sd_local,  (struct sockaddr *)&client_addr, &addrlen);
        printf("accept something sd_channel = %d,client port = %d. IP=%x \n", sd_channel, htons(client_addr.sin_port), client_addr.sin_addr.s_addr);
		if(sd_channel > 0){
            int mode  = 1;
            setsockopt(sd_channel, IPPROTO_TCP, TCP_NODELAY, &mode, sizeof(mode));
            threadinfo.ip = client_addr.sin_addr.s_addr;
			threadinfo.fd = sd_channel;
			
			pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            ret = pthread_create(&threadinfo.tid, &attr, taskprocess, NULL);
            pthread_attr_destroy(&attr);
            if(ret != 0){
            	perror("pthread create fail:");
				usleep(10000);
                close(sd_channel);
                continue;
            }
			sem_wait(&threadinfo.sem);
        }
    }
}

static int createTcpServerThread()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&tidListen, &attr, taskListen, NULL);
    pthread_attr_destroy(&attr);
}

int mainToTcpServer(cJSON *json,cJSON **jsonReturn,char *structBody,char**structBodyReturn)
{
	int out = false;
	if(structBodyReturn == NULL){printf("structBodyReturn:unused parameter\r\n");}
	if(jsonReturn == NULL){printf("jsonReturn:unused parameter,please send cJSON **jsonReturn to me\r\n");return false;}
	else{*jsonReturn = cJSON_CreateObject();}
	if (!json) {
		printf("NULL json pointer\r\n");
		cJSON_AddNumberToObject(*jsonReturn, CJSON_DATE_returnCode, CJSON_ERROR_illegalDate);
		return CJSON_ERROR_illegalDate;
	}
	else{
		int cmd = DEFAULT_FAILED_INT;
		if((cmd = acquireNumberFromCjsonAbsolutely(json,CJSON_DATE_CMD)) == DEFAULT_FAILED_INT){//missing data"cmd"
			cJSON_AddNumberToObject(*jsonReturn, CJSON_DATE_returnCode, CJSON_ERROR_canNotAcquireCmd);
			out = CJSON_ERROR_canNotAcquireCmd;
			goto END;
			}
		switch(cmd){
			case CJSON_CMD_MAKE_CALL://make one call,such as room call room,room call center,room wathc outdoor machines
				makeOneCall(json);
				break;
			default:
				cJSON_AddNumberToObject(*jsonReturn, CJSON_DATE_returnCode, CJSON_ERROR_cmdIllegal);
				out = CJSON_ERROR_cmdIllegal;
				goto END;
				break;
			}
	}
	END:
	return out;
}

/*
call initTcp function when we use this tcp library
tcpServerToMainCallBack:we can use this function to handle system information,such as local configuration information
*/
int initTcp(int (*tcpServerToMainCallBack)(cJSON *,cJSON **,char *,char**))
{
	if(tcpServerToMainCallBack == NULL){
		tcpServerToMain = defaultTcpServerToMain;printf("warning:tcpServerToMain init error\r\n");}
	else {tcpServerToMain = tcpServerToMainCallBack;}
	initContracker();
	createTcpServerThread();
	//tcpServerToMain(NULL,NULL,"3333");
}

