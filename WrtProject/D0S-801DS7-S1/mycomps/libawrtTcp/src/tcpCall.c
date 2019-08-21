/*
attention:
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
pthread_mutex_t  conntracker_lock;
char globalLocalID[15];
struct conntracker_st taskcall_801x_tracker;
struct conntracker_st sendCallToRoom_tracker[OTHER_INDOORMACHINE_NUM];
extern int (*tcpServerToMain)(cJSON *json,cJSON **jsonReturn,char *structBody,char**structBodyReturn);

static int handlerReciveAndSaveIpTable(int sd, struct _net_pack_ *np, int usedbytes,char *fileName)
{
	
}
int time_out_801x()
{
	switch(taskcall_801x_tracker.state)
		{
		case CNN_ST_801X_WAIT_ACK:
			break;
		case CNN_ST_801X_IDLE:
			//set_state_801x(CNN_ST_801X_TIMEOUT);
			break;
		case CNN_ST_801X_HOLDON:
			//set_state_801x(CNN_ST_801X_TIMEOUT);
			break;
		}
	return 0;
}

/*
getLocalID:get local ID when one call star
*/
int getLocalID()
{
	printf("getLocalID\r\n");
	cJSON *jsonData = NULL,*jsonReturn = NULL;//must be released before return
	char *localID = NULL;
	int out = DEFAULT_FAILED_INT;
	if((jsonData=cJSON_CreateObject()) == NULL){printf("json create error\r\n");return false;}
	//else{printf("error:cJSON_CreateObject\r\n");}
	cJSON_AddNumberToObject(jsonData,CJSON_DATE_CMD,CJSON_CMD_GET_LOCAL_ID);
	printf("getLocalID2\r\n");
	if(tcpServerToMain == NULL){printf("sfsfsdfsdfsdfsdfsdfsd\n");}
	tcpServerToMain(jsonData,&jsonReturn,localID,NULL);//handle it in the callback function
	if(jsonReturn){
		if((localID = acquireStringFromCjsonAbsolutely(jsonReturn,CJSON_LOCAL_ID))==NULL){printf("error:localId can't be found\r\n");out = false;}
		else{memcpy(globalLocalID,localID,15);printf("get localid:%s\r\n",globalLocalID);
			out = ture;}
		}
	else{
		printf("error:get local ID failed\r\n");
		out = false;
		}
	cJSON_Delete(jsonData);
	if(jsonReturn){cJSON_Delete(jsonReturn);}
	return out;
}
static int send_cmd(int sd, unsigned short cmd, char *data, int datalen)
{
    int ret=0;
    char sendbuf[128];
    struct _net_pack_ *np = (struct _net_pack_ *)sendbuf;
    if(datalen > 128-HEAD_LENGTH){printf("error:dataLen too long,return\r\n");return false;}
    strncpy(np->head,"WRTI",4);
    np->uCmd = cmd; 
    np->len = HEAD_LENGTH+datalen;
    memcpy(np->srcid, globalLocalID, 15);
    memcpy(sendbuf+HEAD_LENGTH, data, datalen);
    printf("send_cmd sendbuf len=%d\r\n",np->len);
    ret = send(sd, sendbuf, np->len, 0);
    usleep(10);
    
    return ret;
}

int initContracker()
{
    int mode = 1,i = 0;
	pthread_mutex_init (&conntracker_lock, NULL);
	memset(&taskcall_801x_tracker, 0, sizeof(struct conntracker_st));
	sem_init(&taskcall_801x_tracker.sem, 0, 0);
	if(pipe(taskcall_801x_tracker.pipe_fd) == false ){
		perror("failed pipe,system reboot");
		system("reboot");
	}
	ioctl(taskcall_801x_tracker.pipe_fd[0], FIONBIO, (char *)&mode);
	ioctl(taskcall_801x_tracker.pipe_fd[1], FIONBIO, (char *)&mode);
	for (i = 0; i < OTHER_INDOORMACHINE_NUM; i++){
	    memset(&sendCallToRoom_tracker[i], 0, sizeof(struct conntracker_st));
	    sem_init(&sendCallToRoom_tracker[i].sem, 0, 0);
	    if(pipe(sendCallToRoom_tracker[i].pipe_fd) == false ){
			perror("failed pipe,system reboot");
			system("reboot");
	    }
	    ioctl(sendCallToRoom_tracker[i].pipe_fd[0], FIONBIO, (char *)&mode);
	    ioctl(sendCallToRoom_tracker[i].pipe_fd[1], FIONBIO, (char *)&mode);
		//pthread_create(&tid_call[i], NULL, r2ip_caller_pthread, &sendCallToRoom_tracker[i]);
	}
	return 0;
}
int endOtherRoomCall(struct conntracker_st *cker)
{
	int i = 0 ;
	for(i = 0;i < OTHER_INDOORMACHINE_NUM;i++){
		if(cker != &sendCallToRoom_tracker[i]){sendCallToRoom_tracker[i].state = INDOOR_MACHINE_IDLE;}
		}
}
/*
setStateOfCall:change the call state
attention: call state only can be changed in this function
*/
int setStateOfCall(int state,struct conntracker_st *target)
{
	int newstate = -1;
	struct conntracker_st *p = &sendCallToRoom_tracker[0];
	printf("set_state_801x:pthread_mutex_lock\n");
	pthread_mutex_lock(&conntracker_lock);
	printf("roomIP =0x%x set_state_801x: state now is %d,want to be %d\r\n",target->srcidIP,target->state,state);
	switch(state)
	{
	 case INDOOR_MACHINE_IDLE://indoor machine come into a idle condition and any one can call it 
		target->state = INDOOR_MACHINE_IDLE;
		target->timer = 0;
		target->state_screen = 0;
		target->srcidIP = 0;
		break;
	 case INDOOR_MACHINE_WORKING:
	 	if(target->state == INDOOR_MACHINE_IDLE){target->state = state;}
		else{
			pthread_mutex_unlock(&conntracker_lock);
			printf("set_state_801x----pthread_mutex_unlock\n");
			return false;
			}
	 	break;
	 case SENDCALL_ROOM_CALL_ROOM_START:
		if(target->state == INDOOR_MACHINE_IDLE){
			target->state = state;
			for(int i = 0;i < OTHER_INDOORMACHINE_NUM;i++){
				if((p+i)->state != INDOOR_MACHINE_IDLE){
					//target->state = INDOOR_MACHINE_IDLE;
					break;
					}
				}
			//target->state = state;
			}
		break;
	 case SENDCALL_ROOM_CALL_ROOM_IDLE:
		target->state = state;
		//callingStateMsgShow(0);
		if(target->cnn_cmd == ROOM_CALLROOM){//we should end other 3 indoor machines's call
			endOtherRoomCall(target);
			}
	 	break;
	 case SENDCALL_ROOM_WATCH_START:
	 	if(target->state == INDOOR_MACHINE_IDLE){target->state = state;}
	 	break;
	 default:
		printf("quan:--------------------set_state_801x:error state\n");
	}
	pthread_mutex_unlock(&conntracker_lock);
	printf("set_state_801x----pthread_mutex_unlock\n");
	return target->state;
}

/*
roomResponseMeadia

*/
int roomResponseMeadia(int fd)
{
	int ret;
	NEGOTIATE_RESULT tmp_negotiate;
	//#define WRT350X288
	//#define WRT720X576
	#define WRT1280X720
	#if defined( WRT720X576)
	tmp_negotiate.v_enc_type=V_H264;//V_XH264:high-definition indoor machine   720P
	tmp_negotiate.v_enc_resolution_w=720;//1280;
	tmp_negotiate.v_enc_resolution_h=576;//720;
	#elif defined(WRT1280X720)
	tmp_negotiate.v_enc_type=V_H264;
	tmp_negotiate.v_enc_resolution_w=1280;
	tmp_negotiate.v_enc_resolution_h=720;	
	#else
	tmp_negotiate.v_enc_type=V_H264;
	#endif
	ret = send_cmd(fd, MEDIA_NEGOTIATE_ACK, (char *)&tmp_negotiate, sizeof(NEGOTIATE_RESULT));
	if(ret<=0)
	{
		printf("roomResponseMeadia send MEDIA_NEGOTIATE_ACK error:%d\r\n",errno);
		return -1;
	}
	return 1;
}
static int recvOutsideCall(struct conntracker_st *cker, struct cnn_msg_st *cnnmsg)
{
	
}
static int recvFromTCP(struct conntracker_st *cker, struct _net_pack_ *np)
{
    int ret = -1;
	printf("indoor machine recv deal uCmd:%x  len:%d roomIP=0x%x\r\n",np->uCmd, np->len,cker->srcidIP);
    switch(np->uCmd)
	{
	case MEDIA_NEGOTIATE:
		printf("room recv MEDIA_NEGOTIATE roomIP=0x%x\r\n",cker->srcidIP);
		roomResponseMeadia(cker->fd);
		break;
	case DOOR_HANGUP:
		//NetworkCallIdle();
		cker->timer = 0;
		break;
	case ROOM_IDLE:
		//gotoCallingLayerNow();
		//NetworkCallIdle();
		setStateOfCall(SENDCALL_ROOM_CALL_ROOM_IDLE,cker);
		//roomSendMeadia(cker->fd);//not support media negotiate
		cker->timer = 30;
		break;
	case ROOM_MONT_IDLE:
		setStateOfCall(SENDCALL_ROOM_WATCH_IDLE,cker);
		cker->timer = 30;
		break;
	case ROOM_HANGUP:
		cker->timer = 0;
		break;
	case ROOM_HOLDON:
		cker->timer = 120;
		break;
	case DOOR_STARTLEAVEWORD://indoor machine star to record the audio now
		cker->timer = 10;
		break;
	default:
		printf("recvFromTCP:ERROR cmd  roomIP=0x%x  %s\n",cker->srcidIP,ERROR_MSG);
		//start_media_streams();
	}
	return 0;
}
//room_idle:
int room_idle(int fd)
{
	int ret;
	ret = send_cmd(fd, ROOM_IDLE, NULL, 0);
	if(ret<=0){
		printf("room_idle send ROOM_IDLE error:%d\r\n",errno);
		return false;
	}
	return ture;
}

/*
initConntracker:
initialize the resource which will be used during one call
attention:
*/
int initConntrackerBeforeCall(struct conntracker_st *cker)
{
	printf("initConntrackerBeforeCall----pthread_mutex_lock\n");
    int cnt;
    struct cnn_msg_st cnn_msg[16];
    pthread_mutex_lock(&conntracker_lock);
	cker->fd = 0;
	//cker->state = 0;
	cker->timer = 0;
	cker->state_screen = 0;
	cker->srcidIP = 0;
	memcpy(&cker->localid, globalLocalID, 15);
	memcpy(&cker->dstid, "000000000000000", 15);
	ioctl(cker->pipe_fd[0], FIONREAD, &cnt);
	if (cnt)//attention:maybe a bug exits here,what happen when the cnt is larger than PIPE_BUF
		read(cker->pipe_fd[0], cnn_msg,  cnt < sizeof(cnn_msg) ? cnt : sizeof(cnn_msg));
	pthread_mutex_unlock(&conntracker_lock);
	printf("initConntrackerBeforeCall----pthread_mutex_unlock\n");
	return 0;
}
/*
recycleResourceAfterCall:make sure that next call can be run
*/
int recycleResourceAfterCall(struct conntracker_st *cker)
{
printf("recycleResourceAfterCall----pthread_mutex_lock\n");
	int cnt;
	struct cnn_msg_st cnn_msg[8];
	pthread_mutex_lock(&conntracker_lock);
	cker->fd = 0;
	cker->state = INDOOR_MACHINE_IDLE;
	cker->timer = 0;
	cker->state_screen = 0;
	cker->srcidIP = 0;
	memcpy(&cker->localid, globalLocalID, 15);
	memcpy(&cker->dstid, "000000000000000", 15);
	ioctl(cker->pipe_fd[0], FIONREAD, &cnt);
	if (cnt)//attention:maybe a bug exits here,what happen when the cnt is larger than PIPE_BUF
		read(cker->pipe_fd[0], cnn_msg,  cnt < sizeof(cnn_msg) ? cnt : sizeof(cnn_msg));
	pthread_mutex_unlock(&conntracker_lock);
	printf("recycleResourceAfterCall----pthread_mutex_unlock\n");
	return 0;
}


void *handlingCall(void *pParam)
{
	T_CTLTHREADINFO *sockInformation = pParam;
	int fd = sockInformation->index;
	printf("getCall:some one call indoor machine fd=%d \r\n",fd);
	struct timeval timeout;
	fd_set readfd;
	unsigned long maxfd;
	int result, ret;
	struct cnn_msg_st cnn_msg;
	struct _net_pack_ np = { 0 };
	struct _net_pack_ *np_tmp;
	struct conntracker_st *target = sockInformation->cker;
	initConntrackerBeforeCall(target);
	if(room_idle(fd)==false){printf(" room_idle error\r\n");	return (void *)0;}	
	target->fd = fd;
	target->srcidIP = sockInformation->in_client_addr.sin_addr.s_addr;
	target->timer = 30;
//if(uCmd == ROOM_CALLROOM){NetworkCallWrt_Earlymida(GET_CALL_FROM_ROOM);}
//else if(uCmd == DOOR_CALLROOM){NetworkCallWrt_Earlymida(GET_CALL_FROM_DOOR);}

	usleep(50000);
RECV801X://
	printf("indoor machine waiting for ack, fd=%d timer=%d\n",target->fd,target->timer);
	FD_ZERO(&readfd);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	FD_SET(target->fd, &readfd);
	maxfd = target->fd > target->pipe_fd[0] ? target->fd : target->pipe_fd[0];
	result=select(maxfd+1, &readfd, NULL, NULL, &timeout);// 0:time out  -1:error
	if(result < 0){//
		printf("ERROR! goto END_801X_CALL %s\r\n",ERROR_MSG);
		goto END_801X_CALL;
		}
	target->timer--;
	if(result > 0){
		if (FD_ISSET(target->fd, &readfd)){
			 ret = -1;
			 memset((char *)(&np),0,HEAD_LENGTH);//
			 ret = read(target->fd, &np, RECVBUF);//
			 printf("select > 0 taskcall_801x_tracker.fd size:%d\r\n",ret);
			 if(ret == 0){
				 printf("server close socket call end\r\n");
				goto END_801X_CALL;
				}
			 np_tmp = &np;
			 while(1)//maybe receive 2 package 
				{
				 if(memcmp(np_tmp, "WRTI",4) != 0){
					 printf("select > 0 taskcall_801x_tracker.fd msg end\r\n");
					 break;
					}
				 ret = recvFromTCP(target, np_tmp);
				 np_tmp = (struct _net_pack_ *)((char *)np_tmp+np_tmp->len);
				}
			 }
		else if (FD_ISSET(target->pipe_fd[0], &readfd)){
                if (read(target->pipe_fd[0], &cnn_msg, sizeof(struct cnn_msg_st)) > 0){
                 	if (recvOutsideCall(target,&cnn_msg) <= 0){                		
                    	goto END_801X_CALL;
            		}
                }
            }
		if(target->timer > 0) goto RECV801X;
		}
	else
		{
		if(target->timer > 0) goto RECV801X;
		else{//
			time_out_801x();
			}
		}
END_801X_CALL:
	printf("Call has finshed \r\n");
	//NetworkCall_Stop();
	usleep(10000);
	return (void *)ture;
}

int handleComingCall(T_CTLTHREADINFO *p)
{
	int ret = false;
	struct conntracker_st *target = &taskcall_801x_tracker;
	if(setStateOfCall(INDOOR_MACHINE_WORKING,target)==INDOOR_MACHINE_WORKING){
		p->cker = target;
		if((ret = pthread_create(&p->ptid_process, NULL, handlingCall, (void *)p))!=0){printf("error:handleComingCall:pthread_create failed");}
		pthread_join(p->ptid_process, NULL);//wait for thread exit,so that we can recycle all the resource
		}
	else{
		printf("indoor machine busy\r\n");
		ret = send_cmd(p->index, ROOM_BUSY, NULL, 0);
		}
    recycleResourceAfterCall(p->cker);
	printf("handleComingCall:call finshed accurately\r\n");
	return ture;
}
//getSrcIP:get a ip with one id
unsigned long getSrcIP(char *targetID)
{
	
}
int duringSendCall(T_CTLTHREADINFO *pParam)
{
	T_CTLTHREADINFO *p = pParam;
	struct conntracker_st *cker=p->cker;  
	char log_src[15];
	char log_dst[15];
	int forward;
    int error;
    int len;
    int cnt = 0, ret;
    struct timeval timeout;
    struct _net_pack_ np = {0};
    memcpy(np.head, "WRTI", 4);
    int result;
    int maxfd;
	int need_log;
    fd_set write_fdset;
    fd_set read_fdset;	
	cker->timer = 3;//the call use at most X seconds to connect
    while(cker->timer)
	{
        error = -1;
        len = sizeof(int);
        timeout.tv_sec = cker->timer;
        timeout.tv_usec = 0;
        FD_ZERO(&write_fdset);
        FD_ZERO(&read_fdset);
        FD_SET(cker->fd, &write_fdset);
        maxfd = cker->fd;
        result = select(maxfd + 1, &read_fdset, &write_fdset, NULL, &timeout);
        if (result < 0){
			printf("duringSendCall:something wrong happened,roomIP=0x%x\r\n",cker->srcidIP);
            goto endCall;
        }
        else if (result == 0){
			printf("duringSendCall:something wrong happened,maybe the target isn't online,roomIP=0x%x\r\n",cker->srcidIP);
            goto endCall;
        }	
        else if(FD_ISSET(cker->fd, &write_fdset)){//
            len = 4;
			error = -1;
            getsockopt(cker->fd, SOL_SOCKET, SO_ERROR, (char *)&error, (int *)&len);
            if(error == 0){
                printf("duringSendCall connect: successfully np.srcid=%s roomIP=0x%x\r\n",cker->srcid,cker->srcidIP);
                memcpy(np.srcid, cker->srcid, 15);
                memcpy(np.dstid, cker->dstid, 15);
				np.uCmd = p->uCmd;
                np.len = HEAD_LENGTH;
                send(cker->fd, (char *)&np, np.len, 0) ;  
				break;
            }
            else { //connect unsuccessfully
                printf("duringSendCall connect: unsuccessfully roomIP=0x%x\r\n",cker->srcidIP);
				goto endCall;
            }
        }
    }
	
    cnt = 0;
    cker->timer = 30; 

	while(cker->state && (cker->timer-->0))
	{
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&write_fdset);
        FD_ZERO(&read_fdset);
        FD_SET(cker->fd, &read_fdset);
        maxfd = cker->fd;
		//we can't tell if the server is off line with running the following lines.
        result = select(maxfd + 1, &read_fdset, NULL, NULL, &timeout);
        if (result < 0){
			printf("duringSendCall:something wrong happened roomIP=0x%x\r\n",cker->srcidIP);
            break;
        }
		else if(result == 0){printf("duringSendCall:waiting roomIP=0x%x\r\n",cker->srcidIP);}
        else if (result > 0){
            if (FD_ISSET(cker->fd , &read_fdset)){
            	len = 4;
                getsockopt(cker->fd, SOL_SOCKET, SO_ERROR, (char *)&error, (int *)&len);
                if(error == 0){
                    if ((ret = read(cker->fd, &np, HEAD_LENGTH*2))  >= HEAD_LENGTH){
                        if (recvFromTCP(cker, &np) <= 0){
                           // break;
                        }
                    }
                    else if (ret < 0){
						printf("something wrong happened during calling,can't read anything roomIP=0x%x\r\n",cker->srcidIP);
                        break;
                    }
                }
                else {
					printf("something wrong happened during calling,getsockopt error roomIP=0x%x\r\n",cker->srcidIP);
                    break;
                }
            }
        }
		//printf("calling cker->timer=%d  cker->state=%d roomIP=0x%x\r\n",cker->timer, cker->state,cker->srcidIP);
    }
	
endCall: 
	printf("duringSendCall:end one call roomIP=0x%x\r\n",cker->srcidIP);
	if(cker->fd > 0){close(cker->fd);}
	return ture;
}

void *handlingSendCall(void *pParam)
{
	unsigned int mode  = 1, ret;
	T_CTLTHREADINFO *p = pParam;
	struct conntracker_st *cker=p->cker;   
	struct _net_pack_ np = {0};
	struct sockaddr_in server;
	initConntrackerBeforeCall(cker);
	cker->fd = socket(AF_INET, SOCK_STREAM, 0);
	if(cker->fd == -1){
	    printf("Create socket Error %d", errno);
	    cker->state = 0;
	    return (void *)0;
	}
	ioctl(cker->fd, FIONBIO, (char *)&mode);
	mode  = 1;
	setsockopt(cker->fd, IPPROTO_TCP, TCP_NODELAY, &mode, sizeof(mode));

	server.sin_family = AF_INET;
	server.sin_port = htons(TCP_PORT);
	server.sin_addr.s_addr = cker->srcidIP;
	//server.sin_addr.s_addr = 0x530aa8c0;

	if (connect(cker->fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{
	    if (errno != EINPROGRESS){//
			printf("RoomCallRoom connect errno == %d,roomIP = 0x%x\r\n", errno,cker->srcidIP);
	network_fail:        
	        close(cker->fd);
	        return (void *)false;
	    	}
		else{//no matter whether the target is online or not ,run to this line,meaning the connect is EINPROGRESS
			printf("RoomCallRoom: connecting errno == %d,waiting for connecting roomIP = 0x%x\r\n", errno,cker->srcidIP);
			}
	}
	duringSendCall(p);
	return (void *)ture;
}

void *roomSendCallPthread(void *pParam)
{
	printf("roomCallRoomStarPthread\r\n");
	int ret = false;
	T_CTLTHREADINFO *call;
	T_CTLTHREADINFO *p = pParam;
	call->cker = p->cker;
	call->uCmd = p->uCmd;
	sem_post(&p->sem);
	
	if((ret = pthread_create(&call->ptid_process, NULL, handlingSendCall, (void *)call))!=0){printf("error:handleComingCall:pthread_create failed");}
	pthread_join(call->ptid_process, NULL);//wait for thread exit,so that we can recycle all the resource
    recycleResourceAfterCall(call->cker);
	return (void *)ture;
}
int roomCallCenterStar(cJSON *json)
{
	int ret = false;
	pthread_attr_t attr;
	pthread_t tid;
    //struct sockaddr_in server;
    unsigned long srcIP;
	T_CTLTHREADINFO *p;
	char *callTargetID,*callTargetIP;
	struct conntracker_st *target=&taskcall_801x_tracker;
	if(setStateOfCall(INDOOR_MACHINE_WORKING,target)==INDOOR_MACHINE_WORKING){
		p->cker = target;
		p->uCmd = ROOM_CALLCENTER;
		if((callTargetID = acquireStringFromCjsonAbsolutely(json,CJSON_TARGET_ID))==NULL){printf("error:target id can't be found\r\n");}
		else{memcpy(target->dstid,callTargetID,15);}
		if((callTargetIP = acquireStringFromCjsonAbsolutely(json,CJSON_TARGET_IP))==NULL){printf("error:target ip can't be found\r\n");}
		else{
			if( !inet_aton(callTargetIP, (struct in_addr *)&srcIP)){printf("error:target ip maybe wrong\r\n");}
			target->srcidIP=srcIP;
			}
		sem_init(&p->sem, 0,0);
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if((ret=pthread_create(&tid, &attr, roomSendCallPthread, (void *)p))!=0){printf("error:roomCallCenterStar:pthread_create failed");}
		pthread_attr_destroy(&attr);
		sem_wait(&p->sem);//wait until roomCallRoomStarPthread get what it wants;the roomSendCallPthread pthread will sem_post
		sem_destroy(&p->sem);
		}
	else {printf("call room failed,indoor machine is busy\r\n");return false;}
	return ture;
}


int makeOneCall(cJSON *json)
{
	int target;
	if((target = acquireNumberFromCjsonAbsolutely(json,CJSON_TARGET_WANT_TO_CALL)) == DEFAULT_FAILED_INT){printf("call error:target not found\r\n");return false;}
	switch(target)
		{
		case CJSON_CMD_CALL_CENTER://call center
			roomCallCenterStar(json);
			break;
		default:
			break;
		}
	return ture;
}


