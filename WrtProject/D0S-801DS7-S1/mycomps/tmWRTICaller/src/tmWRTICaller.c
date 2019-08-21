/*
文件名：   tmWRTICaller.c
功能：   数字ip设备呼叫功能模块实现
版本：   Version 1.0
创建日期：   2011-12-07
版权：   
作者：   
版本：   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h> 
#include <dirent.h>
#include <malloc.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/vfs.h>
#include <sys/ioctl.h>
//#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/resource.h>
#include <sys/un.h>
#include <arpa/inet.h>


#include "tmWRTICaller.h"
#include "CmdId.h"
#include "media_negotiate.h"

/*****************************************/

static sleepms(int ms)
{
        usleep(ms*1000);	
}

#define PORT_TO_LINUX
#define tm_wkafter sleepms
#define closesocket close
#define ioctlsocket ioctl

pthread_t thread_call, thread_ctrl;
sem_t sem_call, sem_ctrl, sem_stop;
pthread_mutex_t ctrl_mutex, call_mutex, stop_mutex;
unsigned long msg_ctrl[4];
unsigned long msg_call[4];
unsigned long msg_stop[4];

static int send_ctrl_msg(unsigned long *msg);
static int send_call_msg(unsigned long *msg);
static int send_stop_msg(unsigned long *msg);
/*****************************************/

#define CALLER_VERSION_STRING  "Ver 1.00.00"
#define DEBUG_FOR_CALLERMODULE 0	//add by hu 2010.12.17
#define WAIT_CALL_ACK_TIMEOUT  30//40 //等待空闲繁忙，托管，备用 回应包 15=3s
#define CONNECT_MAXTIME 5       //连接超时
#define AV_ROUTERNUM  4       //视频路数总共4路，当前硬件只能承受3路
#define MAX_SOCKET_DESC 4
#define MAX_VIDEO_NUM 4
#define TCP_PORT      20200
#define CALL_FORMAT_STRING "%02s%03s%02s%03s%03s\0"

static struct _wrti_AV_arg_ AV_Arg[4]={{0,0,0,DR_IDLE,0},{0,0,0,DR_IDLE,0},{0,0,0,DR_IDLE,0},{0,0,0,DR_IDLE,0}};
static int AV_Monit_Flags[4] ={0},f_canexitcaller=1;
//static unsigned char g_StopCalling=0;//add by hu 2010.10.22
static unsigned char g_StartCalling=0;//add by hu 2011.04.15
static unsigned long tid_ctrl,tid_call,tid_cap_picture;
static unsigned long call_qid,ctrl_qid,stop_qid,capturepic_qid;
static int msgid=10000;
static int divert_room2room=0;

static WRTI_CallerSetup_t CallerSetup={0};
static void taskcall(); 
static void taskctrl();
static void taskcapturepicture(void);
static int filter_valid_ip(CallParam_t cpCaller[]);
static int is_valid_ip(unsigned long ip);
static void StopAV_Arg_MultiRoom(unsigned long sd, int index,int flag); /*多分机情况下，不关闭音频*/
static void pre_v_server();
/*****************************************/
/*extern char * inet_ntoa( struct   in_addr in );
extern int AV_Server_Init();
extern int AV_Server_GetDestStatus();
extern int AV_Server_SetVideoBitrate(unsigned int VideoBitrate);
extern unsigned int AV_Server_GetVideoBitrate();
extern void AV_Server_Start();
extern void AV_Server_Stop();
extern void AV_Server_Pause();
extern void AV_Server_Restart();
extern void AV_Server_SetDefault();
extern void AV_Server_SetDest1(char *addr,int port);
extern void AV_Server_DelDest1();
extern void AV_Server_SetDest2(char *addr,int port);
extern void AV_Server_DelDest2();
extern void AV_Server_SetDest3(char *addr,int port);
extern void AV_Server_DelDest3();
extern void AV_Server_SetDest4(char *addr,int port);
extern void AV_Server_DelDest4();
*/
//char * inet_ntoa( struct   in_addr in ){return 0;}
//extern int AV_Server_Init();//{return 0;}
int AV_Server_GetDestStatus(){return 0;}
//int AV_Server_SetVideoBitrate(unsigned int VideoBitrate){return 0;}
//unsigned int AV_Server_GetVideoBitrate(){return 0;}
//extern void AV_Server_Start();//{return ;}
//extern void AV_Server_Stop();//{return ;}
//void AV_Server_Pause(){return ;}
//void AV_Server_Restart(){return ;}
//void AV_Server_SetDefault(){return ;}
//void AV_Server_SetDest1(char *addr,int port){return ;}
//void AV_Server_DelDest1(){return ;}
//void AV_Server_SetDest2(char *addr,int port){return ;}
//void AV_Server_DelDest2(){return ;}
//void AV_Server_SetDest3(char *addr,int port){return ;}
//void AV_Server_DelDest3(){return ;}
//void AV_Server_SetDest4(char *addr,int port){return ;}
//void AV_Server_DelDest4(){return ;}

void V_Server_SetDest1(const char *addr,int port){
}
void A_Server_SetDest1(const char *addr,int port){
}
void A_Server_SetDest2(const char *addr,int port){
}
void V_Server_SetDest2(const char *addr,int port){
}

void A_Server_SetDest3(const char *addr,int port){
}
void V_Server_SetDest3(const char *addr,int port){
}
void A_Server_SetDest4(const char *addr,int port){
}
void V_Server_SetDest4(const char *addr,int port){
}
void A_Server_DelDest1(){
}
void V_Server_DelDest1(){
}
void A_Server_DelDest2(){
}
void V_Server_DelDest2(){
}
void A_Server_DelDest3(){
}
void V_Server_DelDest3(){
}
void A_Server_DelDest4(){
}
void V_Server_DelDest4(){
}
void V_Server_SetIFrame(int iparam){
}

/*****************************************/
static unsigned short createcallcmd(unsigned char devicetype,int cmdtype)
{
        unsigned short cmdid;
        switch(devicetype)
        {
        case 0x01:
                {
                        if(cmdtype){
                                cmdid = ROOM_CALLCENTER;
                        }else{
                                cmdid = ROOM_CALLROOM;
                        }
                }
                break;
        case 0x02:
                {
                        if(cmdtype){
                                cmdid = CENTER_CALL_CENTER;
                        }else{
                                cmdid = CENTER_CALLROOM;
                        }
                }
                break;
        case 0x00:
        default:
                {
                        if(cmdtype){
                                cmdid = DOOR_CALLCENTER;
                        }else{
                                cmdid = DOOR_CALLROOM;
                        }
                }
                break;
        }
        return cmdid;
}

//修改增加支持0段网络
static int is_valid_ip(unsigned long ip)
{
        int ret=0;
        unsigned char *temp;
        int j;

        temp = (unsigned char*)&ip;	
        for(j=0; j<4; j++)
        {
                if((*temp>254)||(*temp<1))
                {
                        //printf("error\n");
                        if(j==2)
                        {
                        	if(*temp == 0)
                        	{
                        		temp++;
                        		if((*temp>254)||(*temp<1))
                        			;
                        		else
                        			break;
                        	}
                        }
                        ret= -1;
                        break;
                }
                temp++;
        }
        return ret;
}


static void display_ip(CallParam_t cpCaller[])
{
        int i;
        for(i=0; i<4; i++)
        {
                printf("IP[%d] = %08x\n", i, cpCaller[i].ip);
        }
        printf("==================\n");
}


static int filter_valid_ip(CallParam_t cpCaller[])
{
        unsigned char *temp;
        int i, j;
        int index = 0;

        for(i=0; i<4; i++)
        {
                if(is_valid_ip(cpCaller[i].ip)==-1)  //无效
                {
                        continue;
                }

                if(index != i) //有效
                {
                        memcpy(&cpCaller[index], &cpCaller[i], sizeof(CallParam_t));
                }
                index++;

        }

        for(i=index; i<4; i++)
        {
                memset(&cpCaller[i], 0, sizeof(CallParam_t));
        }

        return index;

}



int wrti_devicecaller(CallParam_t cpCaller[], int num, unsigned short uCmdId)
{
        int i;
        unsigned char *temp;
        unsigned long msg[4]={0};
        int ret;

        ret = filter_valid_ip(cpCaller);
        display_ip(cpCaller);
	  //printf("================num = %d\n", ret );

        msg[0] =(unsigned long)cpCaller;
        msg[1] = uCmdId;
        msg[2] = ret;

#ifndef PORT_TO_LINUX    
        return q_send(ctrl_qid,msg);
#else
        return send_ctrl_msg(msg);//转到taskctrl 任务
#endif    
} 

int wrti_devicecallcenter(CallParam_t cpCaller[], int num, unsigned short uCmdId)
{
        //unsigned long msg[4]={0};
        int i;
        char str_msg[32]="";
        for(i=0;i<num;i++){
                sprintf(str_msg,"呼叫备用中心%d\0",i+1);
                msgid = MSGID_START_CALL_ANOTHRE_CENTER;      //呼叫备用中心%d
                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,str_msg);
                wrti_devicecaller( &cpCaller[i], 1, uCmdId);
                while(CallerSetup.callerstatus==0)tm_wkafter(10);
                while(CallerSetup.callerstatus==2)tm_wkafter(100);
                if(CallerSetup.callerstatus==1)return CallerSetup.callerstatus;
        }
        return CallerSetup.callerstatus;
}

int wrti_devicehungup( unsigned short uCmdId)
{
        unsigned long msg[4]={0};
        msg[0] =0;
        msg[1] = uCmdId;

#ifndef PORT_TO_LINUX
        return q_send(ctrl_qid,msg);
#else
        return send_ctrl_msg(msg);   
#endif    
}



static void taskcall()
{
#if DEBUG_FOR_CALLERMODULE
        char szMsg[32]="";//add by hu 2010.11.23
#endif
        unsigned long msg[4]={0};
        static char deadlock=0;
        CallParam_t callparam[4]={0};
        unsigned long maxfd;
        int res[MAX_SOCKET_DESC],time,num/*,status,resnum*/,i,index,mode=1;
        int unacknum=0,closenum=0;
        int error=-1;
        struct sockaddr_in server[4];
        int len,result,ackcount,busycount,selectresult,media_negotiate;
        int tempret, ret;
        struct _net_pack_ np; 
        struct timeval timeout;
        fd_set readfd;

    	int tempcount =0;

        for(;;)
        {
RESTARTWAIT:
                //InputType = INPUT_TYPE_ROOMID;
                if(CallerSetup.fun_ringoff)
                	CallerSetup.fun_ringoff();
                //CallerSetup.fun_show(msgid,"     欢迎使用...");
                if(CallerSetup.fun_prep&&f_canexitcaller)
                	CallerSetup.fun_prep();
                //CallerSetup.callipnum = 0;
                CallerSetup.callerstatus = 0; 
                deadlock = 0;


RERECEIVE_CALL_MSG:
                printf("wating for sem_call...\n");
#ifndef PORT_TO_LINUX
                q_receive(call_qid,Q_WAIT, 0, msg);
#else
                sem_wait(&sem_call);
                pthread_mutex_lock(&call_mutex);
                memcpy(msg, msg_call, sizeof(msg_call));
                pthread_mutex_unlock(&call_mutex);
#endif
                CallerSetup.callerstatus = 2;
                printf("get call message,num:%d\n",msg[3]);
                if(CallerSetup.fun_show)
                	CallerSetup.fun_show(msgid,(const char*)msg[1]);
                printf("%s\n",(char*)msg[1]);
                num = msg[3];
                num = (num>4||num<=0)?1:num;
                CallerSetup.callipnum = num;
                memcpy(callparam , (CallParam_t *)msg[0],num*sizeof(CallParam_t));

                //#filter input ip
                ret = filter_valid_ip(callparam);
                display_ip(callparam);
                //printf("================num = %d\n", ret );

                if(ret<num)
                        num = ret;

                //------------------------
                index = 0;
                mode=1;
                time = 0;
                maxfd=2;
                for(i=0;i<num;i++){
                        callparam[index].sd = socket(AF_INET, SOCK_STREAM, 0);
                        if(callparam[index].sd==-1){
                                printf("Create Call task Error %d",errno);
                                continue;
                        }else{
                                ioctlsocket(callparam[index].sd, FIONBIO, (char *)&mode); 
                                maxfd=callparam[index].sd+1;
                                index++;
                                printf("创建socket成功\n");
                        }
                }
                if(index<1){
                        continue;         //创建socket失败
                }
                num = index;
                index = 0;

                msgid = MSGID_DEVICE_CONNECTING;   //正在连接目标
                if(CallerSetup.fun_show)
                	CallerSetup.fun_show(msgid,"正在连接目标...");
                
                result=0;
                tempret = 0;
				if(divert_room2room == 1)//add by wyx 20130104
				{
					msgid = MSGID_ROOM_DIVERT_ROOM;   //中心正转接至房间
					if(CallerSetup.fun_show)
						CallerSetup.fun_show(msgid,"中心正转接至房间!");
				}else if (divert_room2room == 2)
				{
					msgid = MSGID_ROOM_DIVERT_CENTER;   //中心正转接至房间
					if(CallerSetup.fun_show)
						CallerSetup.fun_show(msgid,"中心正转接至房间!");
				}
				divert_room2room = 0;
                for(i=0;i<num;i++){			
                        printf("%d, sd:%d,ip: %08x,num:%d\n",i,callparam[i].sd,callparam[i].ip,num);
                        if(callparam[i].sd==-1)
                        	continue;
                        server[i].sin_family = AF_INET;
                        server[i].sin_port = htons(TCP_PORT);
                        server[i].sin_addr.s_addr = callparam[i].ip;
                        tempret = connect(callparam[i].sd, (struct sockaddr*)&server[i], sizeof(struct sockaddr));
                        printf("connect dst ip 20200 socket %d\n",callparam[i].sd);

                        if(errno == 115)//设置为非阻塞时，目标没有即时应答，但是socket还是可以用
                                tempret = 0;
                        if(tempret == 0)//connect 返回成功 
                                callparam[i].ret = 0;
                        else
                                callparam[i].ret = 1;
                        result =callparam[i].ret?1:result;
                        printf("<<taskcall>> result=%d\n",result);//add by hu 2010.05.21
                }		

                if(result){		//只要result为1表明在num个connect中至少有一个错误	
RESELECT:
						printf("taskcall connecting error!\n");
                        time ++;
                        selectresult = 0;
                        error=-1;
                        len=sizeof(int);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = 100*1000;
                        FD_ZERO(&readfd);
                        for( i=0;i<num;i++){
                                printf("callparam[%d].ret = %d",i,callparam[i].ret);
                                if(callparam[i].ret){
                                        printf("sd :%d connect %s with error: %d,num:%d\n",callparam[i].sd,inet_ntoa(server[i].sin_addr),errno,num);
                                        if(callparam[i].sd >0)
                                        {
                                                FD_SET(callparam[i].sd, &readfd);
                                                maxfd = (callparam[i].sd+1)>maxfd?(callparam[i].sd+1):maxfd;
                                        }
                                }
                        }
                        printf("select maxfd: %d\n",maxfd);
                        result = select(maxfd, NULL, &readfd, NULL, &timeout);
                        if(result<0){
                                for(i=0;i<num;i++)
                                        closesocket(callparam[i].sd);    
                                msgid = MSGID_CONNECTED_ERROR;   //连接目标失败
                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"连接目标失败...");
                                tm_wkafter(2000);
                                printf("select<0 result:%d maxfd: %d,错误: %d\n",result,maxfd, errno);
                                //uLastKeyIndex =0xFFFFFFFF;        	    
                                continue;
                        }else if(result==0){
                                //
                                printf("select=0 ,result:%d ,maxfd: %d,错误: %d\n",result,maxfd, errno);
                                if(time<CallerSetup.maxconnecttime)goto RESELECT;
                        }            		
                        error=-1,len=sizeof(int);
                        for( i=0;i<num;i++){
                                if(callparam[i].ret){
                                        if(FD_ISSET(callparam[i].sd,&readfd)){
                                                getsockopt(callparam[i].sd, SOL_SOCKET, SO_ERROR, (char*)&error, (int *)&len);
                                                if(error == 0) callparam[i].ret=0; //ret = true;
                                                else{
                                                        if(time>=CallerSetup.maxconnecttime){
                                                                closesocket(callparam[i].sd);
                                                                callparam[i].sd = -1;
                                                        }    						
                                                }
                                        }else{
                                                if(time>=CallerSetup.maxconnecttime){
                                                        closesocket(callparam[i].sd);
                                                        callparam[i].sd = -1;
                                                }
                                        }
                                }
                        }
                }
                for(i=0;i<num&&callparam[i].ret==0;i++);
                if(i<num)
                        selectresult = 1;//表明有connect 无效的socket
                else 
                        selectresult = 0;
                printf("select time:%d, selectresult=%d\n",time,selectresult);
                if(time<CallerSetup.maxconnecttime&&selectresult)
                	goto RESELECT;
                result=callparam[0].ret;
                printf("index:%d, num: %d\n",index,num);
                for(i=1;i<num;i++){
                        result =callparam[i].ret?result:0;
                }
                if(result){
                        printf("no connected ...\n");
                        for(i=0;i<num;i++){
                                if(callparam[i].sd!=-1){
                                        //sd[index]=sd[i];
                                        //index++;
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd = -1;
                                }
                        }
                        //uLastKeyIndex =0xFFFFFFFF;   	    
                        msgid = MSGID_CONNECTED_ERROR;   //连接目标失败
                        if(CallerSetup.fun_show)
                        	CallerSetup.fun_show(msgid,"连接目标失败...");
                        continue;
                }
                //过滤无效的socket
                index = 0;
                for(i=0;i<num;i++){
                        printf("select end: i:%d, sd[%d]: %d, index:%d, num: %d\n",i,i,callparam[i].sd,index,num);
                        if(callparam[i].sd!=-1&&index!=i){
                                if(callparam[i].ret){
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd = -1;
                                        continue;
                                }
                                callparam[index].sd=callparam[i].sd;
                                callparam[index].ip=callparam[i].ip;
                                callparam[index].ret=callparam[i].ret;
                                memcpy(callparam[index].name,callparam[i].name,16);
                                index++;
                        }else if(index==i&&callparam[i].sd!=-1)
                        {
                                //*<add by hu 2010.11.18
                                if(callparam[i].ret){
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd = -1;
                                        continue;
                                }
                                //add by hu 2010.11.18>*/
                                index++;
                        }
                }
                if(index<1){     //无有效socket
                        printf("no active socket...\n");
                        //uLastKeyIndex =0xFFFFFFFF;   	    
                        msgid = MSGID_CONNECTED_ERROR;   //连接目标失败
                        if(CallerSetup.fun_show)
                        	CallerSetup.fun_show(msgid,"连接目标失败...");
                        printf("connect error\n");
                        continue;
                }
                num = index;		
                //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                msgid = MSGID_CMD_CALL_DEBUG;
                sprintf(szMsg,"index:%d, num: %d,start send...\0",index,num);
                if(CallerSetup.fun_show)
                	CallerSetup.fun_show(msgid,szMsg);
#endif
                //add by hu 2010.11.24>*/
                //printf("index:%d, num: %d\n",index,num);
                //CALL_CONNECTED:          //已经连接modify by hu 2010.07.13
                //printf("start send...\n");
                msgid = MSGID_SENDING_CALL_REQUEST;   //正在发送呼叫请求
                if(CallerSetup.fun_show)
                	CallerSetup.fun_show(msgid,"正在发送呼叫请求...");
                printf("come sending call cmd \n");
                //tm_wkafter(1000);
                if(tcp_send(callparam,num,(char*)msg[2],HEAD_LENGTH)<=0){  //tcp_send error return 0;
                        for(i=0;i<num;i++){
                                closesocket(callparam[i].sd);    
                                callparam[i].sd = -1;
                        }
                        msgid = MSGID_SEND_CALL_REQUEST_ERROR;   //发送呼叫请求失败
                        if(CallerSetup.fun_show)
                        	CallerSetup.fun_show(msgid,"发送请求失败...");
                        tm_wkafter(2000);
                        printf("send error: %d",errno);
                        //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                        msgid = MSGID_CMD_CALL_DEBUG;
                        sprintf(szMsg,"发送命令失败,错误: %d\0",errno);
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
                        //add by hu 2010.11.24>*/
                        //uLastKeyIndex =0xFFFFFFFF;   	    
                        continue;
                }else{
                        index=0;
                        for(i=0;i<num;i++){
                                //printf("i:%d, sd[%d]: %d, index:%d, num: %d\n",i,i,callparam[i].sd,index,num);
                                //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                                msgid = MSGID_CMD_CALL_DEBUG;
                                sprintf(szMsg,"i:%d, sd[%d]: %d, index:%d, num: %d\0",i,i,callparam[i].sd,index,num);
                                if(CallerSetup.fun_show)
                                	CallerSetup.fun_show(msgid,szMsg);	
#endif
                                //add by hu 2010.11.24>*/
                                if(callparam[i].ret<=0){
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd=-1;
                                        //index--;
                                }else{
                                        if(index!=i){
                                                callparam[index].sd=callparam[i].sd;
                                                callparam[index].ip=callparam[i].ip;
                                                callparam[index].ret=callparam[i].ret;
                                                memcpy(callparam[index].name,callparam[i].name,16);
                                        }
                                        index++;
                                }
                        }
                        num = index;
                }
                printf("index:%d, num: %d\n",index,num);
                msgid = MSGID_WAIT_CALL_REQUEST_ACK;   //等待目标响应
                if(CallerSetup.fun_show)
                	CallerSetup.fun_show(msgid,"等待目标响应...");    	
                //printf("head: %s ,len: %d,CMD: 0x%04x\n",gnp_data.head,gnp_data.len,gnp_data.uCmd);
                printf("waiting for idle ack\n");
                np.head[0]=0;
                //ioctlsocket(sd, FIONBIO, (char *)&mode); 

                ackcount = 0;
                busycount =0;
                time = 0;
                //tm_evafter(5000,0x02,&tmid_callctrl);
                //tm_talkctrl_flag = 1;

RE_RECV:
                FD_ZERO(&readfd);
                timeout.tv_sec = 0;
                timeout.tv_usec = 200000; 

                maxfd = callparam[num-1].sd+1;
                for(i=0;i<num;i++){
                        printf("sd[%d]=%d\n",i,callparam[i].sd);
                        if(callparam[i].sd!=-1){
                                closenum++;
                                FD_SET(callparam[i].sd, &readfd);
                        }
                        maxfd = (callparam[i].sd+1)>maxfd?(callparam[i].sd+1):maxfd;
                }
#if 0
                if(closenum==0){
                        goto MULCALL_START;
                }
#endif
                closenum=0;
                result=select(maxfd, &readfd, NULL, NULL, &timeout);//modify bu hu 2011.05.11
                //*<add by hu 2011.05.12
                if(result<0){
                        for(i=0;i<num;i++)
                                closesocket(callparam[i].sd);
                        tm_wkafter(1000);
#if DEBUG_FOR_CALLERMODULE
                        msgid = MSGID_CMD_CALL_DEBUG;
                        sprintf(szMsg,"<wait_ack_for_target>result:%d,maxfd: %d,错误: %04x\0",result,maxfd, errno);
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);
#endif
                        continue;
                }
                //add by hu 2011.05.11>*/
                time ++;
                for(i=0;i<num;i++){
                        int j;
                        if(callparam[i].sd==-1){
                                closenum++;
                                unacknum = num -ackcount-busycount;
                                if(closenum>=num){
                                        printf("close num =%d unacknum=%d\n",closenum,unacknum);
                                        goto MULCALL_START;/*此for循环处理完毕*/
                                }else
                                        continue;
                        }
                        for(j=0;j<ackcount&&i!=res[j];j++);
                        if(j<ackcount)
                        	continue;
                        if(FD_ISSET(callparam[i].sd, &readfd)==0)
                        {
                                if(time>=WAIT_CALL_ACK_TIMEOUT ){
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd=-1;
                                }
                                continue;
                        }else{
                                //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                                msgid = MSGID_CMD_CALL_DEBUG;
                                sprintf(szMsg,"命令接收\0");
                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);
#endif
                                //add by hu 2010.11.24>*/
                                if((len = recv(callparam[i].sd, (char*)&np, 200, 0))<=0){
                                        if(errno==0x5023&&time<WAIT_CALL_ACK_TIMEOUT)continue;
                                        printf("ret: %d,head: %s ,len: %d,CMD: 0x%04x errno:%x\n",len,np.head,np.len,np.uCmd,errno);
                                        //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                                        msgid = MSGID_CMD_CALL_DEBUG;
                                        sprintf(szMsg,"1ret: %d,head: %s ,len: %d,CMD: 0x%04x errno:%x\0",len,np.head,np.len,np.uCmd,errno);
                                        if(CallerSetup.fun_show)
                                        	CallerSetup.fun_show(msgid,szMsg);
#endif
                                        //add by hu 2010.11.24>*/
                                        //if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"目标强制结束会话..."); 
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd=-1;
                                        //tm_wkafter(2000);
                                }else if((len==np.len)&&(strncmp(np.head,"WRTI",4)==0))//判断接受数据格式是否正确2007-09-22
                                {
                                        printf("recv ret:sd %d %d,head: %s ,len: %d,CMD: 0x%04x errno:%x\n",callparam[i].sd,len,np.head,np.len,np.uCmd,errno);
                                        //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                                        msgid = MSGID_CMD_CALL_DEBUG;
                                        sprintf(szMsg,"2ret: %d,head: %s ,len: %d,CMD: 0x%04x errno:%x ROOM_IDLE:0x%04x\0",len,np.head,np.len,np.uCmd,errno,ROOM_IDLE);
                                        if(CallerSetup.fun_show)
                                        	CallerSetup.fun_show(msgid,szMsg);	
#endif
                                        //add by hu 2010.11.24>*/
                                        //非呼叫命令接收信息:
                                        switch(np.uCmd)
                                        {
                                        case CENTER_IDLE:
                                                {
                                                        //启动音频视频
                                                        printf("\tCenter is idle\n");
                                                        //int size = sizeof(struct sockaddr);//modify by hu 2010.07.13
                                                        for(j=0;j<num;j++){
                                                                if(j!=i&&callparam[i].sd!=-1){
                                                                        closesocket(callparam[i].sd);
                                                                        callparam[i].sd=-1;
                                                                }
                                                        }
                                                        //getpeername(sd[i],(struct sockaddr*)&server[i],&size);
                                                        index = StartAV_Server(callparam[i].sd,callparam[i].ip,DR_CALLCENTER,20000);
                                                        printf("\t\t StartAV_Server ip = 0x%x\n", callparam[i].ip);
                                                        np.head[0]='\0';
                                                        np.len =0;
                                                        WaitCallCentTaskEnd(index,callparam[i].sd,np);
                                                        if(callparam[i].sd!=-1){
                                                                closesocket(callparam[i].sd);
                                                                callparam[i].sd=-1;
                                                        }
                                                        //uLastKeyIndex =0xFFFFFFFF;
                                                        CallerSetup.callerstatus = 0;
                                                        goto RESTARTWAIT;
                                                }
                                                //break;//modify by hu 2010.07.13
                                        case ROOM_IDLE:        //回应信息是否空闲
                                                {
                                                		printf("收到空闲回复 i %d  errno %d\n",i,errno);
                                                        res[ackcount]=i;
                                                        ackcount++;
                                                        
                                                        //tempcount =ackcount;
                                                        //FD_ZERO(&readfd);
                										tcp_send_media_negotiate(callparam[i].sd);
                										//ackcount = 0;
                										//time  = 10;
#if DEBUG_FOR_CALLERMODULE
                                                        msgid = MSGID_CMD_CALL_DEBUG;
                                                        
                                                        sprintf(szMsg,"接收命令ROOM_IDLE ackcount=%d\0",ackcount);
                                                        if(CallerSetup.fun_show)
                                                        	CallerSetup.fun_show(msgid,szMsg);
#endif
                                                        //add by hu 2010.11.24>*/

                                                        continue; //
                                                        //break;
                                                } //

                                                // break;//modify by hu 2010.07.13
                                        case CENTER_BUSY:
                                                {
                                                        msgid = MSGID_CENTER_BUSY;   //中心繁忙
                                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"中心繁忙..."); 
                                                        tm_wkafter(2000);
                                                }
                                                break;
                                        case CENTER_MINOR:
                                        case CENTER_AGENT:
                                                {
                                                        printf("中心托管...\n");
                                                        if(np.len!=HEAD_LENGTH+4)break;
                                                        CallerSetup.callerstatus=3;
                                                        for(i=0;i<num;i++){
                                                                if(callparam[i].sd!=-1){
                                                                        closesocket(callparam[i].sd);
                                                                        callparam[i].sd = -1;
                                                                }
                                                        }
                                                        CallerSetup.cp_calldata[0].ip = np.CMD_DATA_IP;
                                                        deadlock++;
                                                        printf("中心托管IP: %08x, MaxTimes: %d, curtime:%d\n",
                                                                CallerSetup.cp_calldata[0].ip,CallerSetup.MaxDivertTimes,deadlock);
                                                        if(CallerSetup.MaxDivertTimes<=deadlock){
                                                                msgid = MSGID_CENTER_DIVERT_TIMEOUT;   //中心转接次数太多
                                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"中心转接次数太多");
                                                                tm_wkafter(2000);
                                                                f_canexitcaller=1;
                                                                goto RESTARTWAIT;
                                                        }
                                                        if(np.uCmd==CENTER_AGENT){
                                                                msgid = MSGID_CENTER_AGENT_CONNECTING;   //中心已托管,转接中
                                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"中心已托管,转接中...");
                                                        }else{
                                                                msgid = MSGID_CENTER_MINOR_CONNECTING;   //中心备用,转接中
                                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"中心备用,转接中...");
                                                        }
                                                        tm_wkafter(10);
                                                        i=0;
                                                        while(CallerSetup.callerstatus==3&&i<5){
                                                                //printf("wait control return CallerSetup.callerstatus=%d ,i=%d\n",CallerSetup.callerstatus,i);
                                                                tm_wkafter(50);
                                                                i++;
                                                        }
                                                        msg[0]= (unsigned long)CallerSetup.cp_calldata;//SysConfig.CenterIp; /*htonl(CENTER_PC_IP);*/
                                                        msg[1]=createcallcmd(CallerSetup.DeviceType,1);//DOOR_CALLCENTER;
                                                        CallerSetup.callipnum = 1;
                                                        msg[2] = CallerSetup.callipnum;
#ifndef PORT_TO_LINUX	 	 	                             
                                                        q_send(ctrl_qid,msg);
#else
                                                        send_ctrl_msg(msg);	 	                             
#endif	 	 	                             
                                                        //tm_wkafter(5);
                                                        goto RERECEIVE_CALL_MSG;
                                                }
                                                //break;//modify by hu 2010.07.13
                                        case ROOM_QUIET:
                                                {
                                                        printf("房间免打扰设置，退出...\n");
                                                        msgid = MSGID_ROOM_QUIET_MODE;   //房间已设免打扰
                                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"房间已设免打扰"); 
                                                        tm_wkafter(1500);
                                                }
                                                break;
                                        case ROOM_AGENT: //房间托管
                                                {
                                                        if(np.len!=HEAD_LENGTH+4)break;
                                                        for(i=0;i<num;i++){
                                                                if(callparam[i].sd!=-1){
                                                                        closesocket(callparam[i].sd);
                                                                        callparam[i].sd = -1;
                                                                }
                                                        }
                                                        deadlock++;
                                                        if(CallerSetup.MaxDivertTimes<=deadlock){	     							
                                                                msgid = MSGID_ROOM_DIVERT_TIMEOUT;   //分机转接次数太多
                                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"分机转接次数太多"); 
                                                                tm_wkafter(2000);
                                                                f_canexitcaller=1;
                                                                goto RESTARTWAIT;
                                                        }
                                                        printf("中心托管...\n");
                                                        msgid = MSGID_ROOM_AGENT_CONNECTING;   //房间已托管,转接中...			
                                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"房间已托管,转接中...");
                                                        CallerSetup.cp_calldata[0].ip = np.CMD_DATA_IP;
                                                        msg[0]= (unsigned long)CallerSetup.cp_calldata;//SysConfig.CenterIp; /*htonl(CENTER_PC_IP);*/
                                                        msg[1]=createcallcmd(CallerSetup.DeviceType,1);//DOOR_CALLCENTER;//DOOR_CALLROOM;
                                                        CallerSetup.callipnum = 1;
                                                        msg[2] = CallerSetup.callipnum;
#ifndef PORT_TO_LINUX	 	 	                             
                                                        q_send(ctrl_qid,msg);
#else
                                                        send_ctrl_msg(msg);	 	 	                             
#endif	 	 	                             
                                                        //tm_wkafter(5);
                                                        goto RERECEIVE_CALL_MSG;
                                                }
                                                //break;//modify by hu 2010.07.13
                                        case ROOM_BUSY:
                                                closesocket(callparam[i].sd);
                                                callparam[i].sd =-1;
                                                busycount++;
                                                printf("busycount=%d\n",busycount);
                                                continue;
                                        		//printMediaInfo();
                                        case ROOM_HOLDON:

                                        		continue;
                                        default:
                                                {
                                                        printf("非法回复,结束通话...");
                                                        msgid = MSGID_CMD_DATA_ERROR;   //非法回复,结束通话...
                                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"非法回复,结束通话..."); 
                                                }
                                                break;
                                        }
                                        for(i=0;i<num;i++){
                                                if(callparam[i].sd!=-1){
                                                        closesocket(callparam[i].sd);
                                                        callparam[i].sd = -1;
                                                }
                                        }
                                        //uLastKeyIndex =0xFFFFFFFF;

                                        goto RESTARTWAIT;

                                }else{
                                        //printf("return len: %d,head: %s ,len: %d,CMD: 0x%04x errno:%x\n",len,np.head,np.len,np.uCmd,errno);
                                        //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                                        msgid = MSGID_CMD_CALL_DEBUG;
                                        sprintf(szMsg,"3ret: %d,head: %s ,len: %d,CMD: 0x%04x errno:%x\0",len,np.head,np.len,np.uCmd,errno);
                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
                                        //add by hu 2010.11.24>*/
                                        printf("taskcall 收到非法命令数据.丢弃该命令\n");
                                        msgid = MSGID_CMD_DATA_ERROR;   //收到非法命令数据
                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"收到非法命令数据");
                                        closesocket(callparam[i].sd);
                                        callparam[i].sd = -1;
                                        continue;
                                }  
                        }
                }
                
                printf("ackcount: %d ,busycount: %d,num: %d, time :%d \n",ackcount,busycount,num,time);
                if(ackcount+busycount<(num)&&time<WAIT_CALL_ACK_TIMEOUT)
                	goto RE_RECV;
                //if(ackcount == 0)
                //	ackcount = tempcount;
                //tempcount = 0;

MULCALL_START:
                printf("unacknum: %d ack:%d ,busycount: %d,num: %d\n",ackcount,busycount,num);
                //tm_cancel(tmid_callctrl);
                //printf("recv\n");
                if(ackcount>0){
                        //
                        index = 0;
                        for(i=0;i<num;i++){
                                printf("i=%d, index=%d, callparam[i].sd=%d\n",i,index,callparam[i].sd);
                                if(callparam[i].sd!=-1){
                                        if(index!=i){
                                                callparam[index].sd=callparam[i].sd;
                                                callparam[index].ip=callparam[i].ip;
                                                callparam[index].ret=callparam[i].ret;
                                                strncpy(callparam[index].name,callparam[i].name,16);
                                                index++;
                                        }else index++;
                                }
                        }
                        //printf("ackcount :%d  index:%d, num:%d\n",ackcount,index,num);
                        //*<add by hu 2010.11.24
#if DEBUG_FOR_CALLERMODULE
                        msgid = MSGID_CMD_CALL_DEBUG;
                        sprintf(szMsg,"<room>ackcount :%d  index:%d, num:%d\0",ackcount,index,num);
                        if(CallerSetup.fun_show)
                        	CallerSetup.fun_show(msgid,szMsg);	
#endif
                        //add by hu 2010.11.24>*/
                        if(index>0){     //有效socket
                                int indexs[8]={0,1,2,3,4,5,6,7};
                                //index = StartAV_Server(sd,server.sin_addr.s_addr,DR_CALLROOM,20000);
                                num = StartAV_ServerList_CallRoom(callparam,index,DR_CALLROOM,20000);
                                np.head[0]='\0';
                                np.len =0;
                                //strncpy(np.dstid,np.srcid,15);
                                WaitCallRoomTaskEnd(indexs, callparam,num,np);
                                CallerSetup.callerstatus = 0;//add by hu 2011.03.24
                        }else{
                                msgid = MSGID_NO_CALL_REQUEST_ACK;   //目标无应答
                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"目标无应答...");
                                tm_wkafter(2000); 
                        }

                }else if(busycount){
                        msgid = MSGID_ROOM_BUSY;   //房间繁忙		
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"房间繁忙...");
                        tm_wkafter(2000);
                }else{
                        msgid = MSGID_NO_CALL_REQUEST_ACK;   //目标无应答
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"目标无应答...");
                        tm_wkafter(2000);
                }
                //DIVERT_END://add by hu 2011.04.12
                for(i=0;i<num;i++){
                        if(callparam[i].sd!=-1)
                                closesocket(callparam[i].sd);
                }
                //uLastKeyIndex =0xFFFFFFFF;
        }
        //t_delete(0L);//modify by hu 2010.07.13
        //pthread_exit(0);
}



static void taskcapturepicture(void)
{
        unsigned char id[16]="";
        unsigned long msg[4]={0};
        for(;;){
                memset(msg,0,16);
#ifndef PORT_TO_LINUX		
                q_receive(capturepic_qid,Q_WAIT,0,msg);
#endif		
                if(msg[0])
                        strncpy((char *)id,(char*)msg[1],15);
                else
                        strncpy((char *)id,"000000000000000",15);	
                tm_wkafter(2000);	
                CallerSetup.fun_capturepic(id);
        }
}


static void taskctrl()
{
        char szMsg[32]="";
        struct _net_pack_ np; 

        unsigned long msg[4]={0},msgrcv[4]={0};

        for(;;)
        {
START_RECV_CTRL_MSG:
                memset(msgrcv,0,16);
                
#ifndef PORT_TO_LINUX        
                q_receive(ctrl_qid,Q_WAIT,0,msgrcv);
#else
                sem_wait(&sem_ctrl); 
                printf("wait a control message...\n");
                pthread_mutex_lock(&ctrl_mutex);
                memcpy(msgrcv, msg_ctrl, sizeof(msg_ctrl));
                pthread_mutex_unlock(&ctrl_mutex);       
#endif        
                
                strcpy(np.head,"WRTI");
                np.len =HEAD_LENGTH;
                printf("recv ctrl_qid msg send len %d\n",np.len);
#ifdef PROTOCOL_VERSION2
                strncpy(np.srcid,CallerSetup.localid,15);
#endif
                //np.FILE_PACKNO=msgrcv[0];
                np.uCmd = msgrcv[1];
                //printf("ip: 0x%08x, cmd: %04x \n",msgrcv[0],msgrcv[1]);
                //*<add by hu 2010.11.23
#if DEBUG_FOR_CALLERMODULE
                msgid = MSGID_CMD_CALL_DEBUG;
                sprintf(szMsg,"ip: 0x%08x, cmd: %04x \0",msgrcv[0],msgrcv[1]);
                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
                //add by hu 2010.11.23>*/
                switch(np.uCmd)
                {
                case CENTER_CALLROOM:
                case ROOM_CALLROOM:
                case DOOR_CALLROOM:         //呼叫分机  
                        {
                                f_canexitcaller = 1;
                                strcpy(szMsg,"正在呼叫分机...");
                                msgid = MSGID_START_CALL_ROOM;             //显示开始呼叫分机
                                g_StartCalling=1;//add by hu 2011.04.15
                                DeviceIsWatching(0,1);//停止监视
                                g_StartCalling=0;//add by hu 2011.04.15
                                tm_wkafter(1000);
                                //np.uCmd = DOOR_CALLROOM;
                                //printf("loaclid is 222%s\n",CallerSetup.localid);
#ifdef PROTOCOL_VERSION2
                                strncpy(np.srcid,CallerSetup.localid,15);
                                //printf("loaclid is %s\n",CallerSetup.localid);
                                //strcpy(np.dstid,g_szrealid);
#endif
								
                                msg[0]=msgrcv[0];//np.FILE_PACKNO;
                                msg[1] = (unsigned long)szMsg;//正在呼叫分机
                                msg[2] = (unsigned long)&np;//
                                msg[3] = msgrcv[2];
#ifndef PORT_TO_LINUX                
                                q_send(call_qid, msg);
#else
                                send_call_msg(msg);
#endif                
                                printf("control call room end...\n");
                        }
                        break;  
                case CENTER_CALL_CENTER:
                case ROOM_CALLCENTER:
                case DOOR_CALLCENTER:
                        {
                                strcpy(szMsg,"正在呼叫中心...");
                                printf("..calling center \n");
                                msgid = MSGID_START_CALL_CENTER;        //显示呼叫中心
                                g_StartCalling=1;//add by hu 2011.04.15
                                DeviceIsWatching(0,1);//停止监视
                                g_StartCalling=0;//add by hu 2011.04.15
                                tm_wkafter(1000);
                                //np.uCmd = DOOR_CALLCENTER;
#ifdef PROTOCOL_VERSION2
                                strncpy(np.srcid,CallerSetup.localid,15);
#endif
								//msgrcv[2]=1;
                                if(msgrcv[2]==1){
                                        f_canexitcaller=1;
                                        msg[0] = msgrcv[0];//np.FILE_PACKNO;
                                        msg[1] = (unsigned long)szMsg;
                                        msg[2] = (unsigned long)&np;
                                        msg[3] =  msgrcv[2];
#ifndef PORT_TO_LINUX                    
                                        q_send(call_qid, msg);
#else
                                        send_call_msg(msg);                    
#endif                    
                                }else{
                                        CallParam_t center1, cpCaller[8]={0};
                                        unsigned long tmpmsg[4]={0};
                                        int i,num;
                                        char str_msg[32]="";
                                        num = msgrcv[2];
                                        printf("cpCaller num : %d\n",num);
                                        memcpy(cpCaller,(void*)msgrcv[0],num*sizeof(CallParam_t));
                                        printf("memcpy end , error: %04x \n",errno);
                                        f_canexitcaller = 0;
                                        for(i=0;i<num;i++){
                                                if(i==num-1)
                                                	f_canexitcaller = 1;
                                                sprintf(str_msg,"呼叫备用中心%d\0",i+1);
                                                //tm_wkafter(1000);
                                                msgid = MSGID_START_CALL_ANOTHRE_CENTER;      //呼叫备用中心%d
                                                if(CallerSetup.fun_show)
                                                	CallerSetup.fun_show(msgid,str_msg);
                                                memcpy(&center1,&cpCaller[i],sizeof(CallParam_t));
                                                printf("center ip: %08x\n",center1.ip);
                                                msg[0]=(unsigned long)&center1;//np.FILE_PACKNO;
                                                msg[1] =(unsigned long)szMsg;
                                                msg[2] = (unsigned long)&np;
                                                msg[3] = 1;
#ifndef PORT_TO_LINUX                            
                                                q_send(call_qid, msg);
#else
                                                send_call_msg(msg);
                                                tm_wkafter(500);
#endif                            
                                                tm_wkafter(5);
                                                while(CallerSetup.callerstatus==0){
                                                        printf("wait status change CallerSetup.callerstatus=%d\n",CallerSetup.callerstatus);
                                                        tm_wkafter(100);
                                                }
                                                while(CallerSetup.callerstatus==2){
                                                        printf("wait status change2 CallerSetup.callerstatus=%d\n",CallerSetup.callerstatus);
                                                        tm_wkafter(100);
                                                }
                                                printf("status changed CallerSetup.callerstatus=%d\n",CallerSetup.callerstatus);
                                                if(CallerSetup.callerstatus){
                                                        if(CallerSetup.callerstatus==3){
                                                                CallerSetup.callerstatus=1;
                                                        }
                                                        f_canexitcaller = 1;
                                                        goto START_RECV_CTRL_MSG;
                                                }
#ifndef PORT_TO_LINUX
                                                if(q_receive(ctrl_qid,Q_NOWAIT,0,tmpmsg)==0){
#else
                                                if(sem_trywait(&sem_ctrl)==0){
                                                        pthread_mutex_lock(&ctrl_mutex);
                                                        memcpy(msg, msg_ctrl, sizeof(msg_ctrl));
                                                        pthread_mutex_unlock(&ctrl_mutex);
#endif                			
                                                        if(tmpmsg[1]==CENTER_HANGUP||tmpmsg[1]==ROOM_HANGUP||tmpmsg[1]==DOOR_HANGUP)
                                                                break;
                                                        else{
                                                                msgid = MSGID_DEVICE_BUSY;
                                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"本机繁忙，请稍候再试");
                                                                break;
                                                        }
                                                }

                                        }
                                        f_canexitcaller = 1;
                                        //return CallerSetup.callerstatus;
                                }
                                printf("control call center end...\n");
                        }
                        break;
                case CENTER_HANGUP:
                case ROOM_HANGUP:         
                case DOOR_HANGUP:
                        {
                        	printf("ctrl hangup 111\n");	
                                /*<add by hu 2010.10.22
                                if(CallerSetup.callerstatus==2)
                                {
                                g_StopCalling=1;
                                break;
                                }
                                //add by hu 2010.10.22>*/
                                //i_addr.s_addr = msgrcv[0];//np.FILE_PACKNO; 
#ifdef PROTOCOL_VERSION2
                                strncpy(np.srcid,CallerSetup.localid,15);
#endif
                                msg[0] = (unsigned long)&np;
                                msg[1] = HEAD_LENGTH;
                                msg[2] = np.uCmd;
#if 1
                                if(DeviceIsCalling(0,0)>0){
                                        //msg[0]=i_addr.s_addr;  
#ifndef PORT_TO_LINUX                    
                                        q_send(stop_qid, msg);
#else
                                        send_stop_msg(msg);                    
#endif                    
                                }
#endif
                        }
                        break;
                        //普通数据命令
                        //case DOOR_GETTIME:
                        //case DOOR_SENDLOG:
                        //case DOOR_LOGIN:
                        //   {
                        //       //WRTI_CmdData cd;
                        //       //cd.ip = from.sin_addr.s_addr;
                        //      //memcpy(&cd.np,&np,np.len);
                        //       //q_send(normalcmd_qid,(unsigned long *)&cd);
                        //   }
                        //   break;        
                default:
                        printf("呼叫库未定义的命令 %04x \n",CMD_ID(np.uCmd));
                        break;
                }    
        }
        //printf("control function end...\n");//modify by hu 2010.07.13
}


void WaitMontEnd(int index, int sd,unsigned long ip,unsigned short uCmd)
{
        int ret,timecount=0;    
        struct timeval tmo;
        fd_set readfd;
        struct _net_pack_ np={0};
#if DEBUG_FOR_CALLERMODULE
        char szMsg[32]="";//add by hu 2011.04.19
#endif
        //unsigned long msg[4]={0};//modify by hu 2010.07.13
        unsigned long uIp =ip;
        AV_RunStatus rs;
        /*<add by hu 2010.09.07
        msgid = MSGID_ROOM_OPEN_LOCK; 
        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"Enter WaitMontEnd");
        //add by hu 2010.09.07>*/
        printf("Enter WaitMontEnd...uIp=%d\n",uIp);//modify by hu 2010.07.13
        strcpy(np.head,"WRTI");
#ifdef PROTOCOL_VERSION2
        strncpy(np.srcid,CallerSetup.localid,15);
        if(AV_Arg[index].dstid[0])
                strncpy(np.dstid,AV_Arg[index].dstid,15);
#endif
        switch(CallerSetup.DeviceType){
                case 0:
                default:
                        np.uCmd = DOOR_ENDMONT;
                        break;
                case 1:
                        np.uCmd = ROOM_STOPMONT;
                        break;
                case 2:
                        np.uCmd = CENTER_STOPMONT;
                        break;
        }
        np.len = HEAD_LENGTH;

        //msg_skc[0] = sd;
        switch(uCmd){
        case ROOM_STOPMONT:
                rs=RM_MONT;
                break;
        case CENTER_STOPMONT:
                rs=CT_MONT;
                break;
        case CENTER_STOPLSN:
                rs=CT_LSN;
                break;
        default:
                send(sd,(char*)&np,np.len,0);
                return;
        }
        //*<add by hu 2011.04.19
#if DEBUG_FOR_CALLERMODULE
        msgid = MSGID_CMD_CALL_DEBUG;
        sprintf(szMsg,"<watiMontEnd>rs=%d",rs);
        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
        //add by hu 2011.04.19>*/
        printf("<watiMontEnd>rs=%d",rs);//add by hu 2010.07.13
        for(;;){
                //printf("monit is running %d...",timecount);
#if DEBUG_FOR_CALLERMODULE
                msgid = MSGID_CMD_CALL_DEBUG;
                sprintf(szMsg,"monit is running timecount=%d",timecount);
                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
                while(AV_Arg[index].writing_flag);
                AV_Arg[index].writing_flag =1;
                AV_Arg[index].time++;
                AV_Arg[index].writing_flag =0;
                if(AV_Monit_Flags[index]==0){
                        StopAV_Arg(sd,index,1);	
                        break;
                }
                //错误处理超过30s线程退出
#ifndef NO_MONITOR_DELAY
                if(timecount>=CallerSetup.timemonit){
                        StopAV_Arg(sd,index,1);	 
                        break; 
                }
#endif
                FD_ZERO(&readfd);
                FD_SET(sd,&readfd);
                tmo.tv_sec = 0;
                tmo.tv_usec =200000;
                if(select(sd+1,&readfd,NULL,NULL,&tmo)<0){//<0 modify by hu 2011.05.11
                        StopAV_Arg(sd,index,0);
                        break;	
                }
                if(errno==0x5009){
                        StopAV_Arg(sd,index,0);
                        break;	
                }
                timecount ++;
                if(FD_ISSET(sd,&readfd)){
                        np.head[0]='\0';
                        np.len = -2;
                        ret = recv(sd, (char*)&np, 100, 0);
                        if(ret<=0){
                                //printf("select recv error:%d \n",errno);
                                if(errno==0x5023)continue;                
                                StopAV_Arg(sd,index,1);
                                break;
                        }
                        //*<add by hu 2011.04.19
#if DEBUG_FOR_CALLERMODULE
                        msgid = MSGID_CMD_CALL_DEBUG;
                        sprintf(szMsg,"head: %s ,len: %d,CMD: 0x%04x\n",np.head,np.len,np.uCmd); 
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
                        //add by hu 2011.04.19>*/
                        //printf("head: %s ,len: %d,CMD: 0x%04x\n",np.head,np.len,np.uCmd); 
                        if((ret==np.len)&&(strncmp(np.head,"WRTI",4)==0)){
                                if(np.uCmd==uCmd){
                                        StopAV_Arg(sd,index,0);
                                        break;	 
                                }
                                switch(np.uCmd)
                                {
                                case ROOM_UNLOCK:
                                        {
                                                msgid = MSGID_ROOM_OPEN_LOCK;   //分机已开锁,挂机按*键
                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"已开锁,请随手关门");
                                                if(CallerSetup.fun_opendoor) //DoorGpioUtilLockOpen();
                                                        CallerSetup.fun_opendoor(IP_CALL_OK,0,NULL,np.srcid,0,0);
                                                else
                                                        printf("Warning ! Unistall opendoor fun未初始化n");
                                        }
                                        break;
                                case CENTER_UNLOCK:
                                        {
                                                msgid = MSGID_MONITOR_OPEN_LOCK;   //MSGID_CENTER_OPEN_LOCK;已开锁,挂机按*键   modify by hu 2011.05.11        	    	
                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"已开锁,请随手关门");
                                                if(CallerSetup.fun_opendoor) //DoorGpioUtilLockOpen();
                                                        CallerSetup.fun_opendoor(IP_CALL_OK,0,NULL,np.srcid,0,0);
                                                else
                                                        printf("Warning ! Unistall opendoor fun_opendoor\n");
                                        }
                                        break;
                                default:
                                        break;
                                }
                        }  
                }
        }
}


void WaitCallCentTaskEnd(int index, int sd, struct _net_pack_ np)
{
        int ret,iflag_holdon,iflag_opendoor=0,timecount=0,autoshuttime=0;
#if DEBUG_FOR_CALLERMODULE
        char szMsg[32]="";//add by hu 2010.11.24
#endif
        unsigned long msg[4]={0};
        struct timeval timeout;
        fd_set readfd;
        unsigned long uIp;
        struct sockaddr_in sa;
        int size = sizeof(struct sockaddr);
        getpeername(sd,(struct sockaddr*)&sa,&size);
        uIp = sa.sin_addr.s_addr;
        //sprintf(szTmpIp,"%d.%d.%d.%d",uIp&0xFF,(uIp>>8)&0xFF,(uIp>>16)&0xFF,(uIp>>24)&0xFF);
        //ioctlsocket(sd, FIONBIO, (char *)&mode); 
        //mp3play((char*)bufmp3test44k, MUSIC_LEN); 
        if(CallerSetup.fun_ringon)
        	CallerSetup.fun_ringon();
        CallerSetup.callerstatus = 1;
        msgid = MSGID_CENTER_RINGON;   //呼叫中心响铃中挂机按*
        if(CallerSetup.fun_show)
        	CallerSetup.fun_show(msgid,"呼叫中心响铃中");
        printf("\tCall Center :ring on\n");
        if(CallerSetup.fun_capturepic){
                msg[0] = 0;// 1; //modify by hu 2011.06.09
                msg[1] = (unsigned long)np.srcid;
#ifndef PORT_TO_LINUX      		
                q_send(capturepic_qid,msg);   		
#endif      		
        }
        iflag_holdon =0;
		
        for(;;)
        {
                while(AV_Arg[index].writing_flag);
                AV_Arg[index].writing_flag=1;
                AV_Arg[index].time++;
                AV_Arg[index].writing_flag=0;
                if(AV_Monit_Flags[index]==0){
                        StopAV_Arg(sd,index,1);
                        break;         
                }
#ifndef PORT_TO_LINUX 
                printf("\tCall Center : waiting stop\n");          
                if(q_receive(stop_qid, Q_NOWAIT, 0, msg) == 0){
#else
                if(sem_trywait(&sem_stop)==0){	  
                        pthread_mutex_lock(&stop_mutex);
                        memcpy(msg, msg_stop, sizeof(msg_stop));
                        pthread_mutex_unlock(&stop_mutex);  	
#endif	    	

                        //发送挂机命令
                        
                        if(msg[2]!=DOOR_HANGUP){
#ifdef PORT_TO_LINUX            		
								printf("stop_qid uIp: 0x%08x, msg[0]: 0x%08x \n", uIp,msg[0]);

								if(sd > 0)
								{
									int ret= 0;
                                	ret = send(sd,(const char*)msg[0],msg[1],0);
                                	if(ret == -1)
                                	{
                                		printf("send error!!! \n");
                                	}else
                                	{
                                		char tmpbuf[40];
                                		memset(tmpbuf,0,40);
                                		memcpy(tmpbuf,msg,msg[1]);
                                		for(ret = 0;ret<40;ret++)
                                			printf("0x%x",tmpbuf[ret]);
                                		printf("\n");
                                	}
                                }
                                else 
                                	printf("Socket err!!!!\n");
#endif                  

                                StopAV_Arg(sd,index,0);
                        }else{
                        		
                                StopAV_Arg(sd,index,1);
                        }
                        break;
                }

                //错误处理超过2分钟线程退出
                //printf("\n timecount:%d,CallerSetup.timecallstop:%d,CallerSetup.timering:%d,iflag_holdon:%d\n",timecount,CallerSetup.timecallstop,CallerSetup.timering,iflag_holdon);
                if(timecount>CallerSetup.timecallstop){
                        //msg[0]=uIp;
                        msgid = MSGID_CMD_CENTER_CALL_TIMEOVER;   //呼叫中心通话时间到，同分机
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"呼叫通话时间到");
                        StopAV_Arg(sd,index,1);	 
                        break; 
                }else{
                        if(timecount>CallerSetup.timering&&iflag_holdon==0){

                                msgid = MSGID_CMD_CENTER_NO_HOLDON;   //中心无人接听
                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"中心无人接听");
                                StopAV_Arg(sd,index,1);
                                break; 
                        }
                }
                if(autoshuttime>25){ //开锁后5s 主机挂机
                        printf(" Open door delay stop\n");
                        StopAV_Arg(sd,index,1);	 
                        break; 
                }
                if(iflag_opendoor)autoshuttime++;
                FD_ZERO(&readfd);
                FD_SET(sd, &readfd);
                timeout.tv_sec = 0;
                timeout.tv_usec = 200000;  
                ret = select(sd+1, &readfd, NULL, NULL, &timeout);
                if(ret<0){
                        //printf(" select return %d,errno: %08x\n",ret,errno);
                        //*<add by hu 2011.05.11
#if DEBUG_FOR_CALLERMODULE
                        msgid = MSGID_CMD_CALL_DEBUG;
                        sprintf(szMsg,"<CallCent-select>ret: %d errno:%04x\0",ret,errno);
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,szMsg);	
#endif
                        //add by hu 2011.05.11>*/
                        StopAV_Arg(sd,index,1);
                        break;	
                }

                if(errno==0x5009){
                        printf("socket invalid\n");
                        StopAV_Arg(sd,index,1);
                        break;	
                }

                timecount ++;
                //if(ret==0)continue;
                if(FD_ISSET(sd, &readfd))
                {
                        memset(&np,0,256);
                        ret = recv(sd, (char*)&np, 256, 0);
                        printf("1ret :%d\n",ret);
                        if(ret==0&&errno==0x5009)  //socket is disconnect  modify by hu 2011.05.11
                        {
                                StopAV_Arg(sd,index,0);
                                break;
                        }
                        printf(" recv return %d,errno: %08x\n",ret,errno);
                        if((ret==np.len)&&(strncmp(np.head,"WRTI",4)==0))
                        {
                            switch(np.uCmd){
					        case CENTER_RESET_ROOM_PASS:
					                {
					                        msgid = MSGID_CMD_REST_ROOM_PASSWORD;   //恢复房间密码
					                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"恢复房间密码");
					                        np.len = HEAD_LENGTH;
					                        np.uCmd =ROOM_REST_PASS_ACK;
					                        send(sd,(const char*)&np,np.len,0);
					                        tm_wkafter(30);
					                }
					                break;
					        case CENTER_RESET_DOOR_PASS:
					                {
					                        msgid = MSGID_CMD_REST_DOOR_PASSWORD;   //修改门口机开门密码
					                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"恢复主机密码"); 
					                        np.len = HEAD_LENGTH;
					                        np.uCmd =DOOR_RESET_PASS_ACK;
					                        send(sd,(const char*)&np,np.len,0);
					                        tm_wkafter(30);
					                }
					                break;
					        case CENTER_DIVERT_CENTER:	
					                {
					                        if(np.len!= HEAD_LENGTH+4)break;
					                        msgid = MSGID_CENTER_DIVERT_CENTER;   //中心正转接至中心
					                        if(CallerSetup.fun_show)
					                        	CallerSetup.fun_show(msgid,"中心正转接至中心!");
					                        StopAV_Arg(sd,index,1);
					                        CallerSetup.cp_calldata[0].ip = np.CMD_DATA_IP;
					                        CallerSetup.cp_calldata[0].name[0]=0;
					                        msg[0]= (unsigned long)CallerSetup.cp_calldata;//SysConfig.CenterIp; /*htonl(CENTER_PC_IP);*/
					                        msg[1]=createcallcmd(CallerSetup.DeviceType,1);//DOOR_CALLCENTER;
					                        CallerSetup.callipnum = 1;
					                        msg[2] = CallerSetup.callipnum;
#ifndef PORT_TO_LINUX                            
					                        q_send(ctrl_qid,msg);
					                        tm_wkafter(5);
#else
					                        send_ctrl_msg(msg);
					                        usleep(5);  
					                        divert_room2room = 2;
					                        if(CallerSetup.fun_ringoff)
						                		CallerSetup.fun_ringoff();
#endif
					                        goto CALLCENTER_END;						
					                }					
					                //break;//modify by hu 2010.07.13									
					        case CENTER_DIVERT_ROOM  :											
					                {
					                        int i=0;
					                        //printf("np.len=%d,sizeof(IpAddrTable)=%d\n",np.len,sizeof(IpAddrTable));
					                        if(np.len<HEAD_LENGTH+sizeof(IpAddrTable))break;
					                        msgid = MSGID_CENTER_DIVERT_ROOM;   //中心正转接至房间
					                        if(CallerSetup.fun_show)
					                        	CallerSetup.fun_show(msgid,"中心正转接至房间!");
					                        StopAV_Arg(sd,index,1);
					                        CallerSetup.callipnum = (np.len-HEAD_LENGTH)/sizeof(IpAddrTable);
					                        printf("Num %d\n",CallerSetup.callipnum);
					                        CallerSetup.callipnum = CallerSetup.callipnum>4?4:CallerSetup.callipnum;
					                        for(i=0;i<CallerSetup.callipnum;i++){
					                                //memcpy(&CallerSetup.cp_calldata
					                                CallerSetup.cp_calldata[i].ip = np.CMD_DATA_ROOMIPLIST[i].ip;
					                                strncpy((char *)CallerSetup.cp_calldata[i].name,np.CMD_DATA_ROOMIPLIST[i].name,16);
					                        }
					                        msg[0]= (unsigned long)CallerSetup.cp_calldata;//SysConfig.CenterIp; /*htonl(CENTER_PC_IP);*/
					                        msg[1]=createcallcmd(CallerSetup.DeviceType,0);//DOOR_CALLROOM;
					                        msg[2] = CallerSetup.callipnum;   
#ifndef PORT_TO_LINUX                                                     
					                        q_send(ctrl_qid,msg);
#else
					                        send_ctrl_msg(msg);                            
#endif
											tm_wkafter(5);
											divert_room2room = 1;
											if(CallerSetup.fun_ringoff)
						                		CallerSetup.fun_ringoff();
											goto CALLCENTER_END;

					                }
					                //break;//modify by hu 2010.07.13
					        case CENTER_HANGUP:
					                {
					                        printf("\tCenter is endcall\n");
					                        StopAV_Arg(sd,index,0);						
					                        msgid = MSGID_CMD_CENTER_HUNGUP;   //与中心通话中,中心挂机
					                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"中心挂机");
					                        goto CALLCENTER_END;
					                }
					        case CENTER_HOLDON:
					                {
					                        //struct sockaddr_in sa;
					                        //int size = sizeof(struct sockaddr);
					                        //getpeername(sd,(struct sockaddr*)&sa,&size);
					                        //uIp = sa.sin_addr.s_addr;
					                        printf("\tCenter is holdon\n");
					                        iflag_holdon =1;
					                        timecount =0;
					                        msgid = MSGID_TALKING_TO_CENTER;   //与中心通话中,挂机按*键    		
					                        if(CallerSetup.fun_show)
					                        	CallerSetup.fun_show(msgid,"通话中,挂机按*键");
					                        if(CallerSetup.fun_holdon)
					                        	CallerSetup.fun_holdon(uIp);
					                }
					                break;
					        case CENTER_UNLOCK:
					                {
					                        printf("\tCenter is unlock\n");
					                        iflag_opendoor = 1;
					                        msgid = MSGID_CENTER_OPEN_LOCK;   //已开锁,挂机按*键
					                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"已开锁,挂机按*键");
					                        if(CallerSetup.fun_opendoor) //DoorGpioUtilLockOpen();
					                                CallerSetup.fun_opendoor(IP_CALL_OK,0,NULL,np.srcid,0,0);
					                        else
					                                printf("Warning ! Unistall opendoor fun_opendoor\n");
					                }
					                break;
					        case MEDIA_NEGOTIATE_ACK://获取协商包，暂时只协商了是否使用264编解码
		                    		Set_local_mediaInfo(&np.dbMsg);
		                    		printMediaInfo();
	                             	break;
					        default:

					                tm_wkafter(200);
					                printf("err data cmd\n");
					                break;
                                }
                        }else{
                                printf("err head len:%d %d\n",ret,np.len);
                                tm_wkafter(200);
                        }
                }            
        }
CALLCENTER_END:
        //if(iflag_opendoor&&CallerSetup.fun_lockdoor)CallerSetup.fun_lockdoor();
        if(iflag_holdon){
                //talkstop(uIp);
                //talktostop(uIp);
                if(CallerSetup.fun_callend)CallerSetup.fun_callend(uIp);
        }else{
                if(CallerSetup.fun_ringoff)CallerSetup.fun_ringoff();
        }
CALLCENTER_END2:
		printf("\tCall Center end \n");
}




void WaitCallRoomTaskEnd(int index[], CallParam_t cp[], int num, struct _net_pack_ np)
{
        int i,ret,/*size,*/j=0,iflag_leaveword=0,iflag_holdon=0,iflag_opendoor=0,autoshuttime=0,timecount=0,audiotime=0;
        unsigned long msg[4]={0};

        struct timeval timeout;
        fd_set readfd;
        int talkindex=-1;
        unsigned long max_sd,uIp=0;
        iflag_leaveword = 0;
        iflag_holdon=0;
        iflag_opendoor=0;
        autoshuttime = 0;
        timecount =0;
        audiotime = 0;

        if(CallerSetup.fun_ringon)
        	CallerSetup.fun_ringon();
        CallerSetup.callerstatus = 1;
        msgid = MSGID_ROOM_RINGON;   //分机响铃中,挂机按*键
        if(CallerSetup.fun_show)
        	CallerSetup.fun_show(msgid,"分机响铃中");
        if(CallerSetup.fun_capturepic)
        {
                msg[0] = 1;
                msg[1] = (unsigned long)np.srcid;

                //q_send(capturepic_qid,msg);
                //printf("srcid: %s, dstid: %s\n",np.srcid,np.dstid);


        }
        iflag_holdon =0;
        max_sd=cp[num-1].sd+1;
		if(sem_trywait(&sem_stop)==0)
		{
			pthread_mutex_lock(&stop_mutex);
			memcpy(msg, msg_stop, sizeof(msg_stop));
			pthread_mutex_unlock(&stop_mutex);					
		}
		usleep(5000);
		if(sem_trywait(&sem_stop)==0)
		{
			pthread_mutex_lock(&stop_mutex);
			memcpy(msg, msg_stop, sizeof(msg_stop));
			pthread_mutex_unlock(&stop_mutex);
		}
		usleep(5000);
		if(sem_trywait(&sem_stop)==0)
		{
			pthread_mutex_lock(&stop_mutex);
			memcpy(msg, msg_stop, sizeof(msg_stop));
			pthread_mutex_unlock(&stop_mutex);
		}
		usleep(5000);
		if(sem_trywait(&sem_stop)==0)
		{
			pthread_mutex_lock(&stop_mutex);
			memcpy(msg, msg_stop, sizeof(msg_stop));
			pthread_mutex_unlock(&stop_mutex);
		}

        for(;;)
        {
RECYCLE:
                if(iflag_holdon==0&&iflag_leaveword<=0)
                {
                        for(i=0;i<num;i++)
                        {
                                while(AV_Arg[index[i]].writing_flag);
                                AV_Arg[index[i]].writing_flag=1;
                                AV_Arg[index[i]].time++;
                                AV_Arg[index[i]].writing_flag=0;
                                if(AV_Monit_Flags[index[i]]==0){
                                        StopAV_ArgList(cp,index,num,1);
                                        goto CALLROOM_END;
                                }
                        }
                }
                else
                {
                        if(talkindex<0)
                        {
                                //printf("TALK<0 AV_Monit_Flags[index[%d]]=%d talkindex=%d\n",talkindex,AV_Monit_Flags[index[talkindex]],talkindex);
                                StopAV_ArgList(cp,index,num,1);
                                goto CALLROOM_END;
                        }
                        while(AV_Arg[index[talkindex]].writing_flag)
                        {
                                usleep(100);
                        }
                        AV_Arg[index[talkindex]].writing_flag=1;
                        AV_Arg[index[talkindex]].time++;
                        AV_Arg[index[talkindex]].writing_flag=0;
                        if(AV_Monit_Flags[index[talkindex]]==0)
                        {
                                //printf("TALK close AV_Monit_Flags[index[%d]]=%d talkindex=%d\n",talkindex,AV_Monit_Flags[index[talkindex]],talkindex);
                                StopAV_Arg(cp[talkindex].sd,talkindex,1);	
                                goto CALLROOM_END;
                        }
                }

                if(sem_trywait(&sem_stop)==0)
                {
                        pthread_mutex_lock(&stop_mutex);
                        memcpy(msg, msg_stop, sizeof(msg_stop));
                        pthread_mutex_unlock(&stop_mutex);	    		   	

                        //收到挂机信号
                        printf("sd= 0x%08x, stop_qid uIp: 0x%08x, msg[0]: 0x%08x \n",cp[talkindex].sd, uIp,msg[0]);     
                        if(iflag_leaveword>0){
                                doorendleaveword(cp[talkindex].sd,(const char*)cp[talkindex].name);
                        }
                        if(iflag_holdon==0&&iflag_leaveword<=0)
                        {
                                if(msg[2]!=DOOR_HANGUP){
                                        for(i=0;i<num;i++)
                                        {
                                                send(cp[i].sd,(const char*)msg[0],msg[1],0);
	                                    }
                                        StopAV_ArgList(cp,index,num,0);
                                }else{
                                        StopAV_ArgList(cp,index,num,1);
                                }
                        }else
                        {
                                if(msg[2]!=DOOR_HANGUP){
                                        send(cp[talkindex].sd,(const char*)msg[0],msg[1],0);
                                        StopAV_Arg(cp[talkindex].sd,talkindex,0);
                                }else{
                                        StopAV_Arg(cp[talkindex].sd,talkindex,1);
                                }
                        }
                        goto CALLROOM_END;
                }

                //printf("\t\t===timecount = %d\n", timecount);
                //错误处理超过2分钟线程退出
                if(timecount>CallerSetup.timecallstop)
                {
                        msgid = MSGID_CMD_ROOM_CALL_TIMEOVER;   //呼叫分机通话时间到，可自由设置，一般为2分钟
                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"呼叫通话时间到");
                        StopAV_Arg(cp[talkindex].sd,talkindex,1);
                        break; 
                }else
                {
                        if(timecount>CallerSetup.timering&&iflag_holdon==0&&iflag_leaveword==0)
                        {
                                printf("响铃结束,CallerSetup.device_leavewordflag=%d,iflag_leaveword=%d,CallerSetup.fun_leaveword=%d,audiotime=%d \n",
                                        CallerSetup.device_leavewordflag,iflag_leaveword,CallerSetup.fun_leaveword,audiotime);
                                if(CallerSetup.device_leavewordflag==0||iflag_leaveword==-1||CallerSetup.fun_leaveword==0)
                                {
                                        if(CallerSetup.device_leavewordflag&&iflag_leaveword==-1){
                                                msgid = MSGID_ROOM_LEAVEWORD_NO_ACK;   //留言失败，无应答
                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"留言失败，无应答");
                                        }
                                        if(CallerSetup.device_leavewordflag&&CallerSetup.fun_leaveword==0)
                                        {
                                                msgid = MSGID_ROOM_UNINIT_LEAVEWORD_INTERFACE;   //未初始化留言接口
                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"未初始化留言接口");
                                        }
                                        msgid = MSGID_CMD_ROOM_NO_HOLDON;   //分机无人接听
                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"分机无人接听");
                                        StopAV_ArgList(cp,index,num,1);
                                        break;
                                }
                                else if(iflag_leaveword==0)
                                {

                                        if(audiotime==0)
                                        { //弹出是否留言对话框
                                                if(CallerSetup.fun_leaveword())
                                                { //外部返回确认留言，发送请求
                                                        leavewordrequest(cp,num);
                                                        msgid = MSGID_CMD_LEAVEWORD_SENT_WAITFOR_ACK;
                                                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"留言请求已发送,请稍候");
                                                }else{                           //不留言，关闭所有连接退出
                                                        StopAV_ArgList(cp,index,num,1);
                                                        break;
                                                }
                                        }

                                        audiotime ++;
                                        //printf("\t\t========== audiotime = %d : %d\n", audiotime, iflag_leaveword);

                                        if(audiotime>25)
                                        {    //5s没有回复，留言失败
                                                iflag_leaveword=-1;
                                                msgid = MSGID_ROOM_LEAVEWORD_NO_ACK;
                                                if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"留言失败，无应答");
                                                StopAV_ArgList(cp,index,num,1);
                                                break;
                                        }
                                }
                                //StopAV_Arg(sd,index,1);	 
                                //break; 
                        }
                }

                if(autoshuttime>25)
                { //开锁后5s 主机挂机
                        if(iflag_holdon)
                                StopAV_Arg(cp[talkindex].sd,talkindex,1);	 
                        else
                                StopAV_ArgList(cp,index,num,1);
                        break; 
                }
                //printf("audiotime=%d\n",audiotime);
                if(audiotime>100)
                {//留言最多20s
                        doorendleaveword(cp[talkindex].sd,(const char*)cp[talkindex].name);
                        tm_wkafter(200);
                        StopAV_Arg(cp[talkindex].sd,talkindex,1);
                        goto CALLROOM_END;
                }
                if(iflag_opendoor)autoshuttime++;
                if(iflag_leaveword>0)audiotime++;
                FD_ZERO(&readfd);

                if(iflag_holdon==0&&iflag_leaveword<=0){
                        for(i=0;i<num;i++){
                                FD_SET(cp[i].sd,&readfd);
                        }
                        //max_sd=sd[num-1]+1;
                }else{
                        FD_SET(cp[talkindex].sd, &readfd);
                        //max_sd=sd[talkindex]+1;
                }

                timeout.tv_sec = 0;
                timeout.tv_usec = 200000;  
                ret=select(max_sd, &readfd, NULL, NULL, &timeout);
                if(ret<0)
                {     
                        if(iflag_holdon)
                        {
                                //for(i=0;i<num;i++)
                                //	printf("StopAV_ArgList sd[%d]=%d,index:%d,num:%d\n",i,cp[i].sd,index,num);
                                StopAV_ArgList(cp,index,num,1);
                        }else{
                                StopAV_Arg(cp[talkindex].sd,talkindex,1);	 
                        }
                        break;
                }
                timecount++;
                for(i=0;i<num;i++)
                {	

                        if(FD_ISSET(cp[i].sd, &readfd))//FD_ISSET(cp[i].sd, &readfd)
                        {
                                memset(&np,0,256);				
                                ret = recv(cp[i].sd, (char*)&np, 256, 0);
                                
                                if(ret<=0)
                                {
                                		msgid = MSGID_CMD_CENTER_HUNGUP;
				                        if(CallerSetup.fun_show)
				                        	CallerSetup.fun_show(msgid,"network error!");
                                        StopAV_Arg(cp[i].sd,i,0);	                        
                                        break;
                                }

                                if(ret>0)
                                {
                                        if((ret==np.len)&&(strncmp(np.head,"WRTI",4)==0))
                                        {
                                                switch(np.uCmd){
                                                        printf("\t\t=======RECV = %x\n",np.uCmd);
                                                        
                                                        
				        case ROOM_HANGUP:
				                {
				                		printf("呼叫发起端收到分机挂机信号\n");
				                        if(iflag_holdon){
				                                StopAV_Arg(cp[talkindex].sd,talkindex,0);
				                        }else{
				                                for(j=0;j<num;j++){
				                                        if(j==i)
				                                                StopAV_Arg(cp[j].sd,index[j],0);
				                                        else
				                                                StopAV_Arg(cp[j].sd,index[j],1);
				                                }
				                        }
				                        msgid = MSGID_CMD_ROOM_HUNGUP;
				                        if(CallerSetup.fun_show)
				                        	CallerSetup.fun_show(msgid,"分机挂机");
				                        goto CALLROOM_END;
				                }
				                //break;//modify by hu 2010.07.13
				        case ROOM_LEAVEWORDACK:
				                {
				                        iflag_leaveword =1;
				                        talkindex = i;
				                        //size = sizeof(struct sockaddr);
				                        //getpeername(sd[i],(struct sockaddr*)&sa,&size);
				                        //uIp = sa.sin_addr.s_addr;
				                        uIp = cp[i].ip;
				                        msgid = MSGID_ROOM_DO_LEAVEWORD_NO_ACK;   //呼叫分机留言中,停止按*键
				                        timecount = 0;
				                        audiotime = 0;
				                        if(CallerSetup.fun_show)
				                        	CallerSetup.fun_show(msgid,"呼叫分机留言中,停止按*键");
				                        if(CallerSetup.fun_holdon)
				                        	CallerSetup.fun_holdon(uIp);
				                        /*for(j=0;j<num;j++){
				                        printf("j= %d talkindex: %d\n",j, talkindex);
				                        if(j==talkindex)continue;
				                        StopAV_Arg(cp[j].sd,index[j],1);
				                        printf("Close socket_index  j= %d talkindex: %d\n",j, talkindex);
				                        }*/
				                        max_sd =cp[talkindex].sd+1;
				                        goto RECYCLE;
				                }
				                //break;//modify by hu 2010.07.13
				        case MEDIA_NEGOTIATE_ACK://获取协商包，暂时只协商了是否使用264编解码
	                    		Set_local_mediaInfo(&np.dbMsg);
	                    		printMediaInfo();
                             goto RECYCLE;
				        case ROOM_HOLDON:
				                {   //摘机, 开始启动音频收发
				                        iflag_holdon =1;
				                        timecount =0;
				                        //size = sizeof(struct sockaddr);
				                        //getpeername(sd[i],(struct sockaddr*)&sa,&size);
				                        //uIp = sa.sin_addr.s_addr;
				                        printf("room_holdon = %d \n",iflag_holdon);
				                        uIp = cp[i].ip;
				                        msgid = MSGID_ROOM_TALKING;   //与分机通话中,挂机按*键
				                        if(CallerSetup.fun_show)
				                        	CallerSetup.fun_show(msgid,"通话中,挂机按*键");
				                        if(iflag_leaveword>0){
				                                if(talkindex != i)
				                                        StopAV_Arg(cp[talkindex].sd,talkindex,1);
				                                iflag_leaveword = 0;
				                                talkindex = i;
				                        }else{
				                                if(CallerSetup.fun_holdon)
				                                	CallerSetup.fun_holdon(uIp);
				                                talkindex = i;
				                                for(j=0;j<num&&num>1;j++){
				                                        printf("trice in.. j: %d, talkindex: %d\n",j,talkindex);
				                                        if(j!=talkindex){
				                                                //StopAV_Arg(cp[j].sd,index[j],1);
				                                                StopAV_Arg_MultiRoom(cp[j].sd,index[j],1);
				                                        }
						                        }
				                        }
				                        printf("talkindex: %d\n",talkindex);
				                        max_sd =cp[talkindex].sd+1;
				                        goto RECYCLE;
				                }
				                //break;//modify by hu 2010.07.13
				        case ROOM_UNLOCK:
				                {
				                        msgid = MSGID_ROOM_OPEN_LOCK;   //分机已开锁,挂机按*键
				                        if(CallerSetup.fun_show)CallerSetup.fun_show(msgid,"已开锁,挂机按*键");
				                        if(CallerSetup.fun_opendoor) //DoorGpioUtilLockOpen();
				                                CallerSetup.fun_opendoor(IP_CALL_OK,0,NULL,np.srcid,0,0);
				                        else
				                                printf("Warning ! Unistall opendoor fun未初始化n");
				                        iflag_opendoor=1;
				                }
				                break;
				                //*<add by hu 2011.04.12
				        case ROOM_AGENT:
				                {
				                        printf("房间托管...\n");						
				                        msgid = MSGID_ROOM_AGENT_CONNECTING;   //房间已托管,转接中...			
				                        if(CallerSetup.fun_show)
				                        	CallerSetup.fun_show(msgid,"房间已托管,转接中...");
				                        tm_wkafter(500);
				                        CallerSetup.cp_calldata[0].ip = np.CMD_DATA_IP;							
				                        msg[0]= (unsigned long)CallerSetup.cp_calldata;

				                        msg[1]=createcallcmd(CallerSetup.DeviceType,1);
				                        CallerSetup.callipnum = 1;
				                        msg[2] = CallerSetup.callipnum;
				                        send_ctrl_msg(msg);		 	 	                      
				                        usleep(5*1000);
				                        if(iflag_holdon){
				                                StopAV_Arg(cp[talkindex].sd,talkindex,0);
				                        }else{
				                                for(j=0;j<num;j++){
				                                        if(j==i)
				                                                StopAV_Arg(cp[j].sd,index[j],0);
				                                        else
				                                                StopAV_Arg(cp[j].sd,index[j],1);
				                                }
				                        }
				                        divert_room2room = 2;
				                        if(CallerSetup.fun_ringoff)
						                	CallerSetup.fun_ringoff();
				                        goto CALLROOM_END;
				                }
				                //break;
				        default:
				                break;
                                                }
                                        }
                                }
                        }
                }

        }

CALLROOM_END:

        if(iflag_holdon||iflag_leaveword>0)
        {
                if(CallerSetup.fun_callend)
                	CallerSetup.fun_callend(uIp);
        }else{
                printf("CALLROOM_END:mpestop uIp=%08x\n",uIp);
                //mp3stop();
                if(CallerSetup.fun_ringoff)
                	CallerSetup.fun_ringoff();
        }
}

//以下函数实现4路视频调度
int InitAV_Arg(unsigned int sd,int index,unsigned long ip,AV_RunStatus runstatus,const char *dstid)
{
        printf("<InitAV_Arg>sd=%d\n",sd);//add by hu 2010.07.13
        while(AV_Arg[index].writing_flag);
        AV_Arg[index].writing_flag =1;
        AV_Arg[index].ip = ip;
        AV_Arg[index].f_running = runstatus;
        //AV_Arg[index].sd = sd;
        AV_Arg[index].time =0;
        AV_Arg[index].writing_flag = 0;
        if(dstid)
                strncpy(AV_Arg[index].dstid,dstid,(strlen(dstid)>16)?16:strlen(dstid));
        else
                memset(AV_Arg[index].dstid,0,16);
        AV_Monit_Flags[index] = 1;
        return 0;
}

int StartAV_Server(unsigned int sd,unsigned long ip,AV_RunStatus rs,unsigned short AvPort)
{

        int index;
        struct in_addr i_addr;
        //char szMsgTmp[64]="";
        i_addr.s_addr = ip; 

        index =GetAVIndex(ip,rs);
        printf("index= %d,ip=0x%08x,port=%d, status=%d\n",index,ip,AvPort,AV_Server_GetDestStatus()&0x0f);
        if(index<0) return index;

        if(InitAV_Arg(sd,index,ip,rs,0)) return -1;
        if(CallerSetup.camera_flag){
                if(AV_Arg[index].f_running!=CT_LSN){
                        if(CallerSetup.fun_ccdon)
                                CallerSetup.fun_ccdon(); 
                        else{
                                printf("Warning !Uuninstall caller funccdon function!!!!\n"); 
                                return index;
                        }
                        switch(index+1){
                case 1:
                        //AV_Server_SetDest1(inet_ntoa(i_addr),AvPort);
                        V_Server_SetDest1(inet_ntoa(i_addr), 20000);
                        pre_v_server();
                        printf("\tV_Server_SetDest1\n");
                        break;
                case 2:
                        //AV_Server_SetDest2(inet_ntoa(i_addr),AvPort);
                        V_Server_SetDest2(inet_ntoa(i_addr), 20000);
                        pre_v_server();
                        break;
                case 3:
                        //AV_Server_SetDest3(inet_ntoa(i_addr),AvPort);
                        V_Server_SetDest3(inet_ntoa(i_addr), 20000);
                        pre_v_server();
                        break;
                case 4:
                        //AV_Server_SetDest4(inet_ntoa(i_addr),AvPort);//20000);
                        V_Server_SetDest4(inet_ntoa(i_addr), 20000);
                        pre_v_server();

                        break;
                default:
                        return -1;      //理论上不会出现
                        } 

                        //AV_Server_Restart(); 
                }
        }
        printf("start AV_Server_SetDest%d, index=%d,ip=0x%08x,port=%d, status=%d\n",index+1,index,ip,AvPort,AV_Server_GetDestStatus()&0x0f);
        tm_wkafter(50);   
        return index;
}

void StopAV_Arg(unsigned long sd, int index,int flag)/*flag =0 被动挂机(收到挂机命令),flag=1 主动挂机*/
{
        struct _net_pack_ np={0};
        int itmp = index+1;
        strcpy(np.head,"WRTI");
        np.len = HEAD_LENGTH; 
#ifdef PROTOCOL_VERSION2
        strcpy(np.srcid,CallerSetup.localid);
        if(AV_Arg[index].dstid[0])
                strncpy(np.dstid,AV_Arg[index].dstid,15);
#endif
#if 1
        if(flag){ 
                printf("send stop cmd \n");
                switch(AV_Arg[index].f_running){

            case CT_LSN :
            case CT_MONT: //停止监视音频
                    A_Server_DelDest2();
            case RM_MONT:
                    switch(CallerSetup.DeviceType){
            case 0:
            default:
                    np.uCmd = DOOR_ENDMONT;
                    break;
            case 1:
                    np.uCmd = ROOM_STOPMONT;
                    break;
            case 2:
                    np.uCmd = CENTER_STOPMONT;
                    break;
                    }
                    if(send(sd, (char*)&np,np.len,0)<=0) 
                            printf("stop error: 0x%08x\n",errno);
                    break;
            case DR_CALLCENTER:
            case DR_CALLROOM: 
                    switch(CallerSetup.DeviceType){
            case 0:
            default:
                    np.uCmd = DOOR_HANGUP;
                    break;
            case 1:
                    np.uCmd = ROOM_HANGUP;
                    break;
            case 2:
                    np.uCmd = CENTER_HANGUP;
                    break;
                    }
                    if(send(sd, (char*)&np,np.len,0)<=0) 
                            printf("stop error: 0x%08x\n",errno);         
                    break;   
                }
                tm_wkafter(50);//等待TCP ACK包
        }
#else
        if(flag){
                np.uCmd = DOOR_HANGUP; 
                if(send(sd, (char*)&np,np.len,0)<=0) 
                        printf("stop error: 0x%08x\n",errno);
        }
#endif
        //closesocket(sd);
        if(CallerSetup.camera_flag){ //有摄像头
                printf("start AV_Server_DelDest%d,ip=0x%08x,\n",itmp,AV_Arg[index].ip);
                switch(itmp){
            case 1:
                    //AV_Server_DelDest1();
                    V_Server_DelDest1();
                    A_Server_DelDest1();
                    break;
            case 2:
                    //AV_Server_DelDest2();
                    V_Server_DelDest2();
                    A_Server_DelDest1();
                    break;
            case 3:
                    //AV_Server_DelDest3(); 
                    V_Server_DelDest3();
                    A_Server_DelDest1();
                    break;
            case 4:
                    //AV_Server_DelDest4();
                    V_Server_DelDest4();
                    A_Server_DelDest1();
                    break;
                }
        }
#if 0
        if(AV_Arg[index].f_running!=RM_MONT&&AV_Arg[index].f_running!=DR_IDLE){
                talkstop(AV_Arg[index].ip);
                tm_wkafter(100); 
                talktostop(inet_ntoa(AV_Arg[index].ip));
                tm_wkafter(100);
        }
#endif
        while(AV_Arg[index].writing_flag);
        AV_Arg[index].writing_flag =1;
        AV_Arg[index].ip =0;
        AV_Arg[index].time =0;    
        AV_Arg[index].f_running = DR_IDLE;
        AV_Arg[index].writing_flag =0;
        AV_Monit_Flags[index] = 0;
        printf("StopAV_Arg end ... status=%d\n", AV_Server_GetDestStatus()&0x0f);
        for(itmp=0;itmp<AV_ROUTERNUM&&AV_Monit_Flags[itmp]==0;itmp++);
        if(CallerSetup.camera_flag){
                if(itmp>=AV_ROUTERNUM) {     //没有通话，关闭摄像头电源
                        if(CallerSetup.fun_ccdoff)
                                CallerSetup.fun_ccdoff();  //DoorGpioUtilCCDPwrOff();
                }
        }
}

static void StopAV_Arg_MultiRoom(unsigned long sd, int index,int flag) /*多分机情况下，不关闭音频*/
{
        struct _net_pack_ np={0};
        int itmp = index+1;
        strcpy(np.head,"WRTI");
        np.len = HEAD_LENGTH; 
#ifdef PROTOCOL_VERSION2
        strcpy(np.srcid,CallerSetup.localid);
        if(AV_Arg[index].dstid[0])
                strncpy(np.dstid,AV_Arg[index].dstid,15);
#endif
#if 1
        if(flag){ 
                printf("send stop cmd  %d\n",AV_Arg[index].f_running);
                switch(AV_Arg[index].f_running){

            case CT_LSN :
            case CT_MONT: //停止监视音频
                    A_Server_DelDest2();
            case RM_MONT:
                    switch(CallerSetup.DeviceType){
            case 0:
            default:
                    np.uCmd = DOOR_ENDMONT;
                    break;
            case 1:
                    np.uCmd = ROOM_STOPMONT;
                    break;
            case 2:
                    np.uCmd = CENTER_STOPMONT;
                    break;
                    }
                    if(send(sd, (char*)&np,np.len,0)<=0) 
                            printf("stop error: 0x%08x\n",errno);
                    break;
            case DR_CALLCENTER:
            case DR_CALLROOM: 
                    switch(CallerSetup.DeviceType){
            case 0:
            default:
                    np.uCmd = DOOR_HANGUP;
                    break;
            case 1:
                    np.uCmd = ROOM_HANGUP;
                    break;
            case 2:
                    np.uCmd = CENTER_HANGUP;
                    break;
                    }
                    if(send(sd, (char*)&np,np.len,0)<=0) 
                            printf("stop error: 0x%08x\n",errno);         
                    else
                    {
                    	printf("send cmd 0x%x\n",np.uCmd);
                    }
                    break;   
                }
                tm_wkafter(50);//等待TCP ACK包
        }
#else
        if(flag){
                np.uCmd = DOOR_HANGUP; 
                if(send(sd, (char*)&np,np.len,0)<=0) 
                        printf("stop error: 0x%08x\n",errno);
        }
#endif
        //closesocket(sd);
        if(CallerSetup.camera_flag){ //有摄像头
                printf("start AV_Server_DelDest%d,ip=0x%08x,\n",itmp,AV_Arg[index].ip);
                switch(itmp){
            case 1:
                    //AV_Server_DelDest1();
                    V_Server_DelDest1();
                    //A_Server_DelDest1();
                    break;
            case 2:
                    //AV_Server_DelDest2();
                    V_Server_DelDest2();
                    //A_Server_DelDest1();
                    break;
            case 3:
                    //AV_Server_DelDest3(); 
                    V_Server_DelDest3();
                    //A_Server_DelDest1();
                    break;
            case 4:
                    //AV_Server_DelDest4();
                    V_Server_DelDest4();
                    //A_Server_DelDest1();
                    break;
                }
        }
#if 0
        if(AV_Arg[index].f_running!=RM_MONT&&AV_Arg[index].f_running!=DR_IDLE){
                talkstop(AV_Arg[index].ip);
                tm_wkafter(100); 
                talktostop(inet_ntoa(AV_Arg[index].ip));
                tm_wkafter(100);
        }
#endif
        while(AV_Arg[index].writing_flag);
        AV_Arg[index].writing_flag =1;
        AV_Arg[index].ip =0;
        AV_Arg[index].time =0;    
        AV_Arg[index].f_running = DR_IDLE;
        AV_Arg[index].writing_flag =0;
        AV_Monit_Flags[index] = 0;
        printf("StopAV_Arg end ... status=%d\n", AV_Server_GetDestStatus()&0x0f);
        for(itmp=0;itmp<AV_ROUTERNUM&&AV_Monit_Flags[itmp]==0;itmp++);
        if(CallerSetup.camera_flag){
                if(itmp>=AV_ROUTERNUM) {     //没有通话，关闭摄像头电源
                        if(CallerSetup.fun_ccdoff)
                                CallerSetup.fun_ccdoff();  //DoorGpioUtilCCDPwrOff();
                }
        }
}

int GetAVIndex(unsigned long ip, AV_RunStatus runstatus)
{
        int i=0,index=-1,ret;
        long MaxTime=0;
        static int run_flag =0; /*临界资源标志，此函数被多任务调用*/

        while(run_flag)tm_wkafter(100);
        run_flag = 1;
        for(i=0;i<AV_ROUTERNUM;i++){
                if(ip==AV_Arg[i].ip){
                        while(AV_Arg[i].writing_flag);
                        if(AV_Arg[i].f_running==runstatus&&AV_Arg[i].time>0){
                                ret = -3;  //该ip正在被使用,忙...
                                goto GET_INDEX_END;
                        }
                        else {
                                //added by yao
                                //if(AV_Arg[i].f_running>CT_MONT&&AV_Arg[i].time>0)
                                //{/*呼叫通话中，无权监视*/                    
                                //    ret = -4;
                                //    goto GET_INDEX_END;
                                // }
                                if(AV_Monit_Flags[i]){
                                        AV_Monit_Flags[i] = 0;
                                        tm_wkafter(1000);
                                }
                        }
                }
        }

        if(runstatus<=CT_LSN){ /*||(runstatus==CT_MONT))监视调度处理*/
                //判断是否有权限
                for(i=0;i<AV_ROUTERNUM;i++){       /*扫描AV_ROUTERNUM路房间监视情况*/
                        while(AV_Arg[i].writing_flag);
                        if(AV_Arg[i].f_running>CT_MONT){
                                ret = -1;   //有通话,无权房间监视
                                goto GET_INDEX_END;
                        }
                }
                /*找空闲的视频*/
                for(i=0;i<AV_ROUTERNUM;i++){  
                        while(AV_Arg[i].writing_flag);
                        printf("Monit GetIDLE Router %d, f_running= %d\n",i,AV_Arg[i].f_running);
                        if(AV_Arg[i].f_running<=DR_IDLE){
                                ret = i;
                                goto GET_INDEX_END;
                        }
                }
                MaxTime=0;
                index =-1; 
                for(i=0;i<AV_ROUTERNUM;i++){       /*扫描AV_ROUTERNUM路房间监视情况*/
                        while(AV_Arg[i].writing_flag);
                        if(AV_Arg[i].f_running>=CT_MONT)continue;     /*只能关断房间监视的视频路 */
                        if(AV_Arg[i].time>=MaxTime){/*调度选中最久房间监视的视频*/
                                MaxTime = AV_Arg[i].time;
                                index = i;            
                        }
                }
                if(index>=0&&index<AV_ROUTERNUM){
                        if(AV_Monit_Flags[index]){
                                AV_Monit_Flags[index] = 0;
                                tm_wkafter(1000);
                        }
                        ret = index;
                }
                else
                        ret = -2;      //繁忙...
        }else{
                if(runstatus>CT_MONT){
                        for(i=0;i<AV_ROUTERNUM;i++){    /*呼叫，停止所有监视*/
                                while(AV_Arg[i].writing_flag);
                                if(AV_Arg[i].time>0||AV_Arg[i].f_running>DR_IDLE){
                                        AV_Monit_Flags[i] = 0;
                                        tm_wkafter(1000);
                                }
                        }
                        ret = 0;
                        goto GET_INDEX_END;
                }else{
                        /*找空闲的视频*/
                        for(i=0;i<AV_ROUTERNUM;i++){  
                                while(AV_Arg[i].writing_flag);
                                //printf("Monit GetIDLE Router %d, f_running= %d\n",i,AV_Arg[i].f_running);
                                if(AV_Arg[i].f_running<=DR_IDLE){
                                        ret = i;
                                        goto GET_INDEX_END;
                                }
                        }
                        MaxTime=0;
                        index =-1; 
                        for(i=0;i<AV_ROUTERNUM;i++){       /*扫描AV_ROUTERNUM路监视情况*/
                                while(AV_Arg[i].writing_flag);
                                //if(AV_Arg[i].f_running>CT_MONT)continue;     //只能关断房间监视的视频路 
                                if(AV_Arg[i].time>=MaxTime){/*调度选中最久监视的视频*/
                                        MaxTime = AV_Arg[i].time;
                                        index = i;            
                                }
                        }
                        if(index>=0&&index<AV_ROUTERNUM){
                                if(AV_Monit_Flags[index]){
                                        AV_Monit_Flags[index] = 0;
                                        tm_wkafter(1000);
                                }
                                ret= index;
                        }
                        else
                                ret= -2;      //繁忙...
                }
        }
GET_INDEX_END:
        run_flag = 0;
        return ret;
}   

int tcp_send_media_negotiate(int sock)
{
        int i=0;
        int res=0;
        int result;
        int count;
        int nZero;
        int cnt=0;
        struct timeval timeout;
        fd_set writefd;
        unsigned long maxfd=0;
        struct _net_pack_ np={0};
        
        strncpy(np.srcid,CallerSetup.localid,15);
        //strncpy(np.dstid,CallerSetup.localid,15);
        np.uCmd = MEDIA_NEGOTIATE;//htons(MEDIA_NEGOTIATE);
        
        strcpy(np.head,"WRTI");
        //memcpy(&np.dbMsg,data,len);
        Get_local_mediainfo(&np.dbMsg);
        np.len  = sizeof(MEDIA_DATA)+40;
        printf("media negotiate sturct size is %d %d\n",np.len,errno);
        
        nZero=1024;       //128K
		setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(char*)&nZero,sizeof((char*)&nZero));
        timeout.tv_sec = 0;
        timeout.tv_usec = 100*1000;
RESEND2:

        if(sock >0)
        {
        	FD_ZERO(&writefd);
        	FD_SET(sock, &writefd);
        	maxfd = (sock+1)>maxfd?(sock+1):maxfd;
        	result = select(maxfd, NULL, &writefd, NULL, &timeout);
        	printf("select result is %d %d\n",result,np.len);
        	if(result)
        	{
        		if(FD_ISSET(sock,&writefd))
            	{
            		res = send(sock,(const char*)&np,sizeof(MEDIA_DATA)+40,0);
            		if(res == sizeof(MEDIA_DATA)+40)
        				printf("tcp_send sd %d=%d  %d\n",sock,res,errno); 
        			else
        				printf("tcp_send sd %d=%d  %d\n",sock,res,errno);
            	}
        	}
			
        }
        else
        	printf("tcp_send sock < 0\n");

        if(res ==0 && cnt++<3)
        	goto RESEND2;
        
}


int tcp_send(CallParam_t cp[], int num,const char *data, int len)
{
        int i=0;
        int res=0;
        int result;
        int retflag = 0;
        int count;
        int mmsgid;
        struct timeval timeout;
        fd_set writefd;
        unsigned long maxfd;
        struct _net_pack_ np={0};
        memcpy(&np,data,len);

		Reset_negotiate_result();
        
        
        for(i=0;i<num;i++){
		        timeout.tv_sec = 0;
		        timeout.tv_usec = 100*1000;
                //if(np.uCmd==DOOR_CALLROOM)
                printf("tcp_send %d len :%d\n",i,len);
                strncpy(np.dstid,(const char *)cp[i].name,15);
                if(cp[i].sd >0)
                {
RESEND:
					timeout.tv_sec = 0;
			        timeout.tv_usec = 100*1000;
                	FD_ZERO(&writefd);
                	FD_SET(cp[i].sd, &writefd);
                	maxfd = (cp[i].sd+1)>maxfd?(cp[i].sd+1):maxfd;
                	result = select(maxfd, NULL, &writefd, NULL, &timeout);
                	if(result)
                	{
                		if(FD_ISSET(cp[i].sd,&writefd))
	                	{
	                		cp[i].ret = send(cp[i].sd,(const char*)&np,len,0);
	                		res =cp[i].ret>0?1:res;
	                	}
                	}
                	count ++;
                	if(cp[i].ret >0)
                	{
                		retflag = 1;
                	}
                	if(cp[i].ret <= 0)
                	{
	                	if(count >3 )
	                	{
	                		count = 0;
	                		continue;
	                	}else
	                		goto RESEND;
                	}
                }
                else
                	printf("tcp_send sock < 0\n");
                	
                printf("tcp_send =%d\n", cp[i].ret);
                
        }
        printf("retflag %d\n",retflag);
        return retflag;
}

int StartAV_ServerList_CallRoom(CallParam_t cpParam[],int devicecount,AV_RunStatus rs,unsigned short AvPort)
{
        int i,/*size,*/num;
        struct in_addr i_addr;
        //struct sockaddr_in sa;
        //IpAddrTable_t ips[MAX_VIDEO_NUM];
        num = (devicecount>MAX_VIDEO_NUM||devicecount<=0)?1:devicecount;

        //DeviceIsWatching(0,1);
        //DeviceIsCalling(0,1);
        //tm_wkafter(500);//modify by hu 2011.04.12
        if(CallerSetup.camera_flag){
                if(CallerSetup.fun_ccdon)CallerSetup.fun_ccdon();  // DoorGpioUtilCCDPwrOn();
        }
        for(i=0;i<num;i++){
                //
                //size = sizeof(struct sockaddr);
                //getpeername(sd[i],(struct sockaddr*)&sa,&size);
                //ips[i].ip = sa.sin_addr.s_addr;
                printf("\n%d,%d,ips:%d,rs:%d\n",cpParam[i].sd,i,cpParam[i].ip,rs);
                InitAV_Arg(cpParam[i].sd,i,cpParam[i].ip,rs,(const char *)cpParam[i].name);
                AV_Monit_Flags[i] = 1;
                if(!CallerSetup.camera_flag)continue;
                i_addr.s_addr = cpParam[i].ip;     //A_Server_SetDest1 用于对讲，A_Server_SetDest2用于监视
                switch(i+1){
            case 1:
                    //AV_Server_SetDest1(inet_ntoa(i_addr),AvPort);
                    V_Server_SetDest1(inet_ntoa(i_addr), 20000);
                    pre_v_server();
                    printf("=======V_Server_SetDest1 \n");
                    break;
            case 2:
                    //AV_Server_SetDest2(inet_ntoa(i_addr),AvPort);
                    V_Server_SetDest2(inet_ntoa(i_addr), 20000);
                    pre_v_server();
                    printf("=======V_Server_SetDest2 \n");
                    break;
            case 3:
                    //AV_Server_SetDest3(inet_ntoa(i_addr),AvPort);
                    V_Server_SetDest3(inet_ntoa(i_addr), 20000);
                    pre_v_server();
                    printf("=======V_Server_SetDest3 \n");
                    break;
            case 4:
                    //AV_Server_SetDest4(inet_ntoa(i_addr),AvPort);//20000);
                    V_Server_SetDest4(inet_ntoa(i_addr), 20000);
                    pre_v_server();
                    printf("=======V_Server_SetDest4 \n");
                    break;
            default:
                    return -1;      //理论上不会出现
                } 
        }
        //if(ip)
        //  	memcpy(ip,ips,num*sizeof(IpAddrTable_t));
        printf("start AV_Server_SetDest status=%d\n",AV_Server_GetDestStatus()&0x0f);

        return num;
}

void StopAV_ArgList(CallParam_t cp[],int indexs[],int num,int flag)
{
        int i;
        for(i=0;i<num;i++){
                StopAV_Arg(cp[i].sd,indexs[i],flag);
        }
}

int DeviceIsCalling(unsigned long ip,int close_flag)
{
        int i;
        for(i=0;i<AV_ROUTERNUM;i++){
                //printf("ip=%d, Av_arg[%d].ip=0x%x,Av_arg[%d].time=%d\n",ip,i,AV_Arg[i].ip,i,AV_Arg[i].time);    	
                if((ip==AV_Arg[i].ip&&AV_Arg[i].time>0)||(ip==0&&AV_Arg[i].time>0)){

                        if(AV_Arg[i].f_running==DR_CALLCENTER||AV_Arg[i].f_running==DR_CALLROOM){
                       
                                if(close_flag){
                                        AV_Monit_Flags[i]=0;
                                        tm_wkafter(700);
                                }
                                return AV_Arg[i].f_running;
                        }
                }
        }
        return 0;
}
AV_RunStatus DeviceIsWatching(unsigned long ip, int close_flag)
{
        int i;
        for(i=0;i<AV_ROUTERNUM;i++){
                if((ip==AV_Arg[i].ip&&AV_Arg[i].time>0)||(ip==0&&AV_Arg[i].time>0)){
                        if(AV_Arg[i].f_running==CT_MONT||AV_Arg[i].f_running==RM_MONT||AV_Arg[i].f_running==CT_LSN){
                                if(close_flag){
                                        AV_Monit_Flags[i]=0;
                                        tm_wkafter(700);
                                }
                                if(g_StartCalling==0)//add by hu 2011.04.15
                                        return AV_Arg[i].f_running;
                        }
                }
        }	
        return DR_IDLE;
        /*modify by hu 2011.03.24
        if(AV_Arg[i].f_running==CT_MONT){
        if(close_flag){
        AV_Monit_Flags[i]=0;
        tm_wkafter(700);
        }
        //return AV_Arg[i].f_running;//modify by hu 2011.04.07
        }			   
        }
        }
        for(i=0;i<AV_ROUTERNUM;i++){
        if((ip==AV_Arg[i].ip&&AV_Arg[i].time>0)||(ip==0&&AV_Arg[i].time>0)){
        if(AV_Arg[i].f_running==RM_MONT||AV_Arg[i].f_running==CT_LSN){
        if(close_flag){
        AV_Monit_Flags[i]=0;
        tm_wkafter(700);
        }
        //return AV_Arg[i].f_running;modify by hu 2011.04.07
        }	//modify by hu 2011.04.07*/		   
}

/*<add by hu 2011.03.24
AV_RunStatus CenterIsWatching()
{
int i;
for(i=0;i<AV_ROUTERNUM;i++)
{
if(AV_Arg[i].f_running==CT_MONT)
return AV_Arg[i].f_running;
}
return DR_IDLE;
}
//add by hu 2011.03.24>*/

int DeviceStopAllSpec(AV_RunStatus rs, int close_flag)
{
        int i,ret =0;
        for(i=0;i<AV_ROUTERNUM;i++){
                if(AV_Arg[i].f_running==rs){
                        if(close_flag){
                                AV_Arg[i].writing_flag = 1;
                                AV_Arg[i].time += 600;
                                AV_Arg[i].writing_flag = 0;
                                tm_wkafter(300);
                        }
                        ret ++;
                }
        }
        return ret;
}
int wrti_CallerInitByDefault(const char *id)
{
        int i;
        printf("===========wrti_CallerInitByDefault start===========\n");
        /*初始化数据*/
        for(i=0;i<4;i++){
                AV_Arg[i].f_running = DR_IDLE;
                AV_Arg[i].time =0;
        }
        CallerSetup.callerstatus = 0;
        CallerSetup.callipnum = 0;
        CallerSetup.device_leavewordflag = 0;
        strncpy(CallerSetup.localid, id,16);

        CallerSetup.camera_flag     = 1;
        CallerSetup.timecallstop    = 600;
        CallerSetup.timering        = 200;
        CallerSetup.timemonit       = 150;
        CallerSetup.MaxVideo        = 4;
        CallerSetup.maxconnecttime  = 3;
        CallerSetup.MaxDivertTimes  = 4;
        return 0;
}

void wrti_setlocalid(char *localid)
{
	strncpy(CallerSetup.localid,localid,16);
}

int wrti_CallerSetup(WRTI_CallerSetup_t callersetup)
{
        //
        int i;
        printf("===========wrti_CallerSetup start===========\n");
        /*初始化数据*/
        for(i=0;i<4;i++){
                AV_Arg[i].f_running = DR_IDLE;
                AV_Arg[i].time =0;
        }
        CallerSetup.callerstatus = 0;
        CallerSetup.callipnum = 0;
        CallerSetup.device_leavewordflag = callersetup.device_leavewordflag;
        strncpy(CallerSetup.localid, callersetup.localid,16);
        //CallerSetup.ch_en_switch=callersetup.ch_en_switch;//add by hu 2010.07.13
        CallerSetup.camera_flag     = callersetup.camera_flag;
        CallerSetup.timecallstop    = callersetup.timecallstop;
        CallerSetup.timering        = callersetup.timering;
        CallerSetup.timemonit       = callersetup.timemonit;
        CallerSetup.MaxVideo        = callersetup.MaxVideo;
        CallerSetup.maxconnecttime  = callersetup.maxconnecttime;
        CallerSetup.MaxDivertTimes  = (callersetup.MaxDivertTimes<=0||callersetup.MaxDivertTimes>10)?4:callersetup.MaxDivertTimes;
        CallerSetup.DeviceType      = callersetup.DeviceType;
        CallerSetup.fun_show        = callersetup.fun_show;
        CallerSetup.fun_ringon      = callersetup.fun_ringon;
        CallerSetup.fun_ringoff     = callersetup.fun_ringoff;
        CallerSetup.fun_holdon      = callersetup.fun_holdon;
        CallerSetup.fun_callend     = callersetup.fun_callend;
        CallerSetup.fun_opendoor    = callersetup.fun_opendoor;
        CallerSetup.fun_ccdon       = callersetup.fun_ccdon;   
        CallerSetup.fun_ccdoff      = callersetup.fun_ccdoff;  
        CallerSetup.fun_prep        = callersetup.fun_prep;
        CallerSetup.fun_leaveword   = callersetup.fun_leaveword;
        CallerSetup.fun_capturepic  = callersetup.fun_capturepic;
        printf("maxVideo:%d, CONENCTTIME:%d,maxdriver:%d\n",
                callersetup.MaxVideo,
                callersetup.maxconnecttime,
                callersetup.MaxDivertTimes);
        if(CallerSetup.fun_ccdoff)
                CallerSetup.fun_ccdoff();  //DoorGpioUtilCCDPwrOff();

#ifndef PORT_TO_LINUX
        q_create("call",4,Q_LIMIT,&call_qid);   //call ctrl msg
        q_create("ctlq",4,Q_LIMIT,&ctrl_qid);
        q_create("stop",4,Q_LIMIT,&stop_qid);

        t_create("ctrl", 225, 0x20000, 0x20000, 0, &tid_ctrl);
        t_start(tid_ctrl, 0, (void (*)())taskctrl, NULL);

        t_create("call", 217, 0x20000, 0x20000, 0, &tid_call);
        t_start(tid_call, 0, (void (*)())taskcall, NULL);
        //capture
        if(CallerSetup.fun_capturepic){
                q_create("cpic",4,Q_LIMIT,&capturepic_qid);
                t_create("cap0",210,0x2000,0x2000,0,&tid_cap_picture);
                t_start(tid_cap_picture,0,(void(*)())taskcapturepicture,NULL);
        }
#else
        sem_init(&sem_call,0,0);
        sem_init(&sem_ctrl,0,0);
        sem_init(&sem_stop,0,0);

        pthread_create(&thread_call, NULL, (void *)taskcall, NULL);
        pthread_create(&thread_ctrl, NULL, (void *)taskctrl, NULL);

#endif    

        printf("===========wrti_CallerSetup end===========\n");
        return 0;
}

const WRTI_CallerSetup_t * wrti_GetCallerParam()
{
        return ((const WRTI_CallerSetup_t *)&CallerSetup);
}

void wrti_CallerCleanup()
{
        //
#ifndef PORT_TO_LINUX	
        t_delete(tid_ctrl);
        t_delete(tid_call);
        q_delete(call_qid);
        q_delete(ctrl_qid);
        q_delete(stop_qid);
#endif	
}

int wrti_GetCallerStatus()
{
        return CallerSetup.callerstatus;
}

int doorendleaveword(unsigned long sd,const char *destid)
{
        //
        struct _net_pack_ np={0};
        strncpy(np.head,"WRTI",4);
        np.len = HEAD_LENGTH;
        np.uCmd = DOOR_ENDLEAVEWORD;
        strncpy(np.srcid,CallerSetup.localid,15);
        strncpy(np.dstid,destid,strlen(destid)>15?15:strlen(destid));
        return send(sd,(const char*)&np,np.len,0);
}
int leavewordrequest(CallParam_t cp[], int num)
{
        int i,ret=0;
        struct _net_pack_ np={0};
        strncpy(np.head,"WRTI",4);
        np.len = HEAD_LENGTH;
        np.uCmd = DOOR_STARTLEAVEWORD;
        strncpy(np.srcid,CallerSetup.localid,15);
        for(i=0;i<num;i++){
                strncpy(np.dstid,(const char*)cp[i].name,15);
                ret|=send(cp[i].sd,(const char*)&np,np.len,0);
        }
        return ret;
}

void wrti_SetCallerTime(int call, int ring, int monit)
{
        CallerSetup.timecallstop = call;
        CallerSetup.timering     = ring;
        CallerSetup.timemonit    = monit;
}
void wrti_SetCallerCntNego(int usecntnego)
{
        CallerSetup.usecenternegotiate = usecntnego;
}
const char *wrti_GetCallerVersion()
{
        return CALLER_VERSION_STRING;
}

/*****************************************/
int send_ctrl_msg(unsigned long *msg)
{
        pthread_mutex_lock(&ctrl_mutex);
        memcpy(msg_ctrl, msg, sizeof(msg_ctrl));
        pthread_mutex_unlock(&ctrl_mutex);

        printf("send sem_ctrl\n");
        return sem_post(&sem_ctrl);	 	
}

int send_call_msg(unsigned long *msg)
{
        pthread_mutex_lock(&call_mutex);
        memcpy(msg_call, msg, sizeof(msg_call));
        pthread_mutex_unlock(&call_mutex);

        printf("send sem_call\n");
        return sem_post(&sem_call);	 	
}

int send_stop_msg(unsigned long *msg)//发送结束信息
{
        pthread_mutex_lock(&stop_mutex);
        memcpy(msg_stop, msg, sizeof(msg_stop));
        pthread_mutex_unlock(&stop_mutex);

        printf("send sem_stop\n");
        return sem_post(&sem_stop);	 	
}

static void pre_v_server()
{
        V_Server_SetIFrame(5);
}



