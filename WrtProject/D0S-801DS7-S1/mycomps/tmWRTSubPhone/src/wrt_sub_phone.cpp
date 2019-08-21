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
#include "SDL.h"
#include "wrt_sub_phone.h"
#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "wrt_log.h"
#include "wrt_msg_queue.h"

#define closesocket close 
#define ioctlsocket ioctl
//分机指令
#define ROOM_CALLCENTER		      0x0001               /*呼叫中心*/
#define ROOM_EMERG		          0x0002               /*紧急求助*/
#define ROOM_INQFEE		          0x0003               /*费用查询*/
#define ROOM_REPAIR		          0x0004               /*请求维修*/
#define ROOM_BROWSERING		      0x0005               /*请求铃声列表*/
#define ROOM_RINGDOWN		      0x0006               /*铃声下载*/
#define ROOM_GETTIME		      0x0007               /*获得时间*/
#define ROOM_WARNING              0x0008               /*报警*/
#define ROOM_CALLROOM		      0x0009               /*户户通*/
#define ROOM_INQADDR              0x000a               /*请求对应房间号的IP地址*/
#define ROOM_GETCONFIGACK         0x000b               /*回应请求系统信息*/

#define ROOM_ASKFOR_SYSINFO       0X000C                /*分机主动请求分机配置信息*/
#define ROOM_GET_SYSINFO_ACK      0x000d                /*分机呼应基本信息*/
#define ROOM_INQSERVICEINFO       0x000e                /*房间请求便民信息，暂时自定义*/
#define ROOM_INQHELPINFO          0X000F                /*分机请求帮助信息 */
//
#define ROOM_STARTMONT		      0x0020               /*开始监视*/
#define ROOM_STOPMONT		      0x0021               /*停止监视*/
//家居与中心家居协议
#define ROOM_STATUSACK            0x0010               /*房间回应查询所有的家居状态*/
#define ROOM_SETELECACK           0x0011               /*回应中心控制家居*/
#define ROOM_GETELECACK           0x0012               /*回应中心查询某个家居的状态*/
#define ROOM_SCENEACK             0x0013               /*回应中心情景控制*/

/*2009-7-29  new add cmd*/
#define ROOM_REMOTE_SWITCH  0X0015             /*通知中心，分机启用、关闭远程控制*/
#define ROOM_SIGNLE_GUARD_ACK 0X0016       /*回应中心，单独控制防区，主要为电话控制所用*/
#define ROOM_GET_ELECFIILE_ACK   0x0017      /*回应中心，获得家居配置表*/
#define ROOM_SET_ELECFILE_ACK   0X0018      /*回应中心，设置家居配置表*/

#define ROOM_SEND_PHOTO_VOICE   0X1107              /*分机发送抓拍图片到中心*/


//
#define ROOM_BUSY		              0x0090              /*繁忙*/
#define ROOM_IDLE	                  0x0091              /*空闲*/
#define ROOM_HOLDON		              0x0092              /*摘机*/
#define ROOM_HANGUP		              0x0093              /*挂机*/
#define ROOM_UNLOCK		              0x0094              /*开锁*/
#define ROOM_AGENT                    0x0095              /*分机托管*/
#define ROOM_QUIET                    0x0096              /*分机免扰*/
#define ROOM_BROADCASTACK             0x0022              /*回应收到公告成功/失败*/
#define ROOM_UPGRADEACK               0x0023              /*升级回应*/
#define ROOM_LEAVEWORDACK             0x0024              /*留言回应*/
#define ROOM_STOPLEAVEWORDACK         0x0025              /*回应留言结束*/

#define ROOM_CALL_LIFT                0x0030              /*召唤电梯*/

//分机与分机之间的命令
#define ROOM_SYNC_AGENT               0x0080              /*分机同步托管*/
#define ROOM_CANCEL_SYNC_AGENT        0x0081              /*分机取消同步托管*/
#define ROOM_SYNC_AVOID               0x0082              /*分机同步免打扰*/
#define ROOM_CANCEL_SYNC_AVOID        0x0083              /*分机取消同步免打扰*/
#define ROOM_SYNC_RING_TIME           0x0084              /*分机铃声时间同步*/
#define ROOM_SMALLDOOR_HOLDON         0X0085              /*小门口机，摘机同步*/
#define ROOM_SMALLDOOR_HANGUP         0X0086              /*小门口机呼叫，挂机同步*/
#define ROOM_REQ_SYNC                 0X0087              /*从分机启动时向主分机发送请求同步的消息*/
#define ROOM_HOST_BUSY                0X0088              /*如果是多分机，小门口机呼叫，主分机繁忙，发送该同步消息*/
#define ROOM_SEND_KEY                 0x0089              /*分机发送键*/


#define ROOM_SEND_ALARM                  0X1000        /*房间紧急报警*/

//主机指令

#define DOOR_CALLCENTER		        0x0101
#define DOOR_MONTACK		        0x0102
#define DOOR_BUSY			        0x0103
#define DOOR_UNLOCKACK		        0x0104
#define DOOR_GETTIME		        0x0105
#define DOOR_CALLROOM		        0x0106
#define DOOR_HANGUP			        0x0107
#define DOOR_ENDMONT                0x0108


#define DOOR_NOPERSON		        0x01a5
#define DOOR_OVERTIMETALK	        0x01a6
#define DOOR_ROOMRINGOFFTIME        0x01a7
#define DOOR_MONITORTIME	        0x01a8

#define DOOR_BROADCAST              0x010a             /*转发公告*/
#define DOOR_BROADCASTPIC           0x010b             /*转发的图片公告*/
#define DOOR_STARTLEAVEWORD         0x010e             /*留言*/
#define DOOR_STOPTLEAVEWORD         0x010f             /*结束留言*/


//中心指令
#define CENTER_EMERGACK		        0x0201             /*中心回应紧急求助*/
#define CENTER_INQFEEACK	        0x0202             /*中心回应费用查询*/
#define CENTER_REPAIREACK	        0x0203             /*中心回应请求维修*/
#define CENTER_TIMEACK		        0x0204             /*中心回应获得时间*/
#define CENTER_CALLROOM		        0x0205             /*中心呼叫用户*/
#define CENTER_RINGERRACK           0x0206             /*中心回应获取铃声失败*/
#define CENTER_RINGLSTACK           0x0207             /*中心回应获取铃声列表*/
#define CENTER_RINGDOWNACK          0x0208             /*中心回应铃声下载*/
#define CENTER_CALLROOMACK          0x0209             /*中心应答查询IP地址*/

#define CENTER_SERVERINFOACK        0x020a             /*中心回应获取便民信息，暂时自定义了*/

#define CENTER_STARTMONT	        0x0227             /*中心启动监视*/
#define CENTER_STARTLSN		        0x0228             /*中心启动监听*/
#define CENTER_STOPMONT		        0x0229             /*中心停止监视*/
#define CENTER_STIPLSN		        0x0230             /*中心停止监听*/
#define CENTER_SETIDTOIPTABLE       0X0238             /*中心设置ID TO IP 表*/
#define CENTER_GETIDTOIPTABLE       0x237              /*中心获得地址表*/
#define ROOM_ACK_GETIDTOIP          0x0111             /*房间回应获得分机地址表*/

#define CENTER_SEND_RESET           0x0270             /*中心重置分机*/
#define CENTER_BUSY			        0x0290             /*中心繁忙*/
#define CENTER_IDLE			        0x0291             /*中心空闲*/
#define CENTER_HOLDON		        0x0292             /*中心摘机*/
#define CENTER_HANGUP		        0x0293             /*中心挂机*/
#define CENTER_UNLOAK		        0x0294
#define CENTER_WARNACK              0x0295             /*中心应答报警*/
//#define CENTER_RELOGIN              0x0296

#define CENTER_AGENT                0x0296             /*中心托管*/
#define CENTER_MINOR                0x0297             /*中心启动备用机*/
#define CENTER_DIVERT_CENTER        0x0298             /*中心转接到管理机*/
#define CENTER_DIVERT_ROOM          0x0299             /*中心转接到分机*/

#define CENTER_ASKFOR_SYSINFO_ACK   0x0242              /*中心回应请求配置信息*/
#define CENTER_GET_SYSINFO          0x0244              /*中心查询获得系统信息*/
#define CENTER_SEND_SYSINFO         0x0243              /*中心更新系统信息*/
#define CENTER_FACTORY_SETTING      0x0245              /*中心设置IP出厂设备*/

#define CENTER_NOPERSON		              0x02a5
#define CENTER_OVERTIMETALK	            0x20a6

#define CENTER_BROADCASTDATA            0x231            /*中心广播文字公告*/
#define CENTER_BROADCASTPIC             0x232            /*中心广播图片公告*/

#define CENTER_UPGRADEROOM              0x236            /*中心升级分机软件*/
#define DEV_UPGRADE_ACK                 0x1180           /*中心升级应答指令*/

#define BLTN_SETUP                      0x2A0            /*中心下发帮助图片*/

#define CENTER_IS_DEVONLINE             0x02F0          /*中心在线检测设备*/
#define DEVONLINE_ACK                   0X1181           /*设备回应*/

//家居与中心家居协议
#define CENTER_GETALLSTATUS             0x0210           /*中心查询所有的家居状态*/
#define CENTER_SETELEC                  0x0211           /*中心控制家居*/
#define CENTER_GETELEC                  0x0212           /*中心查询某个家居的状态*/
#define CENTER_SETSCENE                 0x0213           /*中心设置情景模式*/


/*2009-7-29  new  add cmd*/
#define CENTER_REMOTE_SINGLE_GUARD          0x0216       /*中心远程单独控制防区*/
#define CENTER_GET_ELEC_CONFIG              0x0217       /*中心获得家居配置表*/
#define CENTER_SET_ELEC_CONFIG              0X0218       /*中心设置家居配置表*/

/*----------------------中心远程控制安防指令---------------------------------*/
#define CENTER_ARRANGE_GUARD                0X256           /*中心远程控制防区*/
#define CENTER_CHECK_GUARD                  0X257           /*中心获得防区配置*/
#define CENTER_RESET_ROOM_PASS              0X250           /*中心重置分机密码*/
#define ROOM_RESET_PASS_ACK                 0XA01           /*分机应答中心密码*/
#define ROOM_CENTER_ARRANGE_GUARD_ACK       0XA02           /*分机应答远程控制防区*/
#define ROOM_CENTER_CHECK_GUARD_ACK         0XA04           /*分机应答中心获得防区状态*/

/*-----------------------------------2010/8/2---------------------------------------*/
#define CENTER_BOMB                         0x02b1
#define CENTER_UP_20                        0x02b0
#define DEV_UP_20_ACK                       0x11b0

/*----------------------------------2010-5-31---------------------------------*/
#define ROOM_SERVICES_REQUEST               0x0014
#define CENTER_SERVICES_ACK                 0X020b


/************************************2012-2-24 9:42:07*******************************/
#define PHONE_ACK_KEEP_LIVE         0x0619
#define ROOM_KEEP_LIVE                           0x0819



#define MAX_PHONE 4

typedef struct _tagPhoneInfo{
	unsigned long phone_ip;
	int phoneindex;
}PHONEINFO;
T_SYSTEMINFO* g_pSystemInfo = NULL;

static CWRTMsgQueue g_subphone_queue;
static SDL_sem     *g_psubphone_sem;
static int 	g_subphone_exit = 0;
static PHONE_CALLBACK g_phone_callback =  NULL;
//static unsigned long g_phone_ip[MAX_PHONE]={0};
static PHONEINFO g_phone_ip[MAX_PHONE]= {0};

/**************************************************************************************/
#define EXIT_SUBPHONE                    0xFE
#define MESSAGE_SUBPHONE                 0xFD

/************************************************************************
msg_buf[0]:命令字
msg_buf[1]:该命令字是否带有数据，有则表示带有数据长度
msg_buf[2]:数据，如果数据大于4个字节，则表示一个指针。
msg_buf[3]：指向房间号。
************************************************************************/
static char* make_packet(unsigned long msg_buf[4],int* packetlen)
{
        char* packbuf = NULL;
        char* tmpdata = NULL;
        char* id = NULL;
        int cmdlen = 40;
        short tmpcmd = (msg_buf[0] & 0x0000FFFF);
        if(msg_buf[3] == 0)
                id = NULL;
        else
                id = (char*)msg_buf[3];

        if(msg_buf[1] == 0)
                packbuf = (char*)ZENMALLOC(cmdlen);
        else{
                cmdlen += msg_buf[1];
                tmpdata = (char*)msg_buf[2];
                packbuf = (char*)ZENMALLOC(cmdlen);
        }
        if(packbuf == NULL){
                if(packetlen)
                        *packetlen =0;
                return NULL;
        }

        tmpcmd =htons(tmpcmd);
        memset(packbuf,0,cmdlen);
        strncpy((char*)packbuf,"WRTI",4);
        memcpy(packbuf+4,&cmdlen,4);
        memcpy(packbuf+8,&tmpcmd,2);

        if(g_pSystemInfo){
                if(id == NULL){
                	memcpy(packbuf+10,g_pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                        //memcpy(packbuf+10,"000000000000000",15);
                }else
                        memcpy(packbuf+10,id,15);
                memcpy(packbuf+25,g_pSystemInfo->mySysInfo.phonenumber[0],15);
        }

        if(cmdlen > 40){
                memcpy(packbuf+40,(void*)tmpdata,msg_buf[1]);
        }
        if(packetlen)
                *packetlen = cmdlen;
        return packbuf;
}




static int connect_to_phone(unsigned long ipaddr){
        struct sockaddr_in addr;
        int tmp_sock = -1;
        int port = 20200;
        fd_set writefd;
        struct timeval timeout;    
        int max_fd = -1; 
        int flag = 1;
        int rc =0;
        tmp_sock  = socket(AF_INET,SOCK_STREAM, 0);
        if(tmp_sock < 0){
                WRT_WARNING("创建socket failed");              
                return -1;
        }          
        memset(&addr,0,sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;      
        addr.sin_addr.s_addr = ipaddr;
        addr.sin_port   = htons(port);

        ioctlsocket(tmp_sock,FIONBIO,(char*)&flag);
        if(connect(tmp_sock,(struct sockaddr*)&addr,sizeof(struct sockaddr_in)) < 0){
                if(errno  != EINPROGRESS){           
                        closesocket(tmp_sock);
                        tmp_sock = -1;  
                        return tmp_sock;      
                }
        }

        FD_ZERO(&writefd);
        if(max_fd < tmp_sock)
                max_fd = tmp_sock;
        FD_SET(tmp_sock,&writefd);   
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        rc = select(max_fd+1,NULL,&writefd,NULL,&timeout);
        if(rc > 0){      
                if(FD_ISSET(tmp_sock,&writefd)){
                        WRT_DEBUG((const char*)"连接到手机成功0x%x:%d",ipaddr,port);
                        return tmp_sock;
                }  
        }else{
                WRT_DEBUG((const char*)"连接到手机失败%x",errno);
        }

        closesocket(tmp_sock);
        tmp_sock = -1;
        return tmp_sock;       
}


static int send_net_msg(int fd,unsigned long msg_buf[4])
{
        fd_set writefd;
        int rc=0;
        int result = 0;
        int datalen;
        char* packet = NULL;
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 300000;

        FD_ZERO(&writefd);
        FD_SET(fd,&writefd);

        rc =  select(fd+1,NULL,&writefd,NULL,&timeout);
        if(rc > 0)
        {
                if(FD_ISSET(fd,&writefd))
                {
                        datalen  = 0;
                        packet = make_packet(msg_buf,&datalen);
                        if(packet && (datalen > 0)){
                                int ret = 0;
                                ret = send(fd,packet,datalen,0);
                                if(ret != datalen)
                                {
                                	result = -1;
                                        WRT_DEBUG("发送数据到手机错误 %x",errno);
                                }else
                                        WRT_DEBUG("发送数据到手机成功%d字节",ret);
                        }
                        ZENFREE(packet);
                        packet  = NULL;
                }
        }
        return result;
}

static int  recv_msg_from_net(int fd,char* databuffer,int datalen,int sec, int usec)
{

        fd_set readfd;
        int rc=0;
        int ret = 0;
        int irecvlen = 0;
        char* packet = NULL;
        struct timeval timeout;
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;//300000;

        FD_ZERO(&readfd);
        FD_SET(fd,&readfd);
        memset(databuffer,0,datalen);

		if(fd < 0)
		{
			//printf("socket close55555555555555\n");
			return -2;
		}
        while(irecvlen < datalen)
        {
	//		reread:

				rc =  select(fd+1,&readfd,NULL,NULL,&timeout);
                if(rc > 0)
                {
                        if(FD_ISSET(fd,&readfd))
                        {

                                ret = recv(fd,databuffer+irecvlen,datalen-irecvlen,0);
                                if(ret == 0)
                                {
                                        if(errno != EWOULDBLOCK)
                                        {
                                        	//printf("111111111111111111\n");
											//goto reread;
                                            return -2;
                                        }
                                        return 1;
                                }
                                else if(ret < 0)
                                {
                                	//printf("2222222222222222\n");
                                        return  -2;
                                }
                                irecvlen += ret;
                                WRT_DEBUG("recv_msg_from_net %d\n",irecvlen);
                        }
                }else if(rc < 0)
                {
                	//printf("333333333333333333\n");
                        return -2;
                }else
                        return -1;
        }
        return 1;
}

static void clear_subphone_queue()
{
	while(1)
	{
		CWRTMsg* pMsg = NULL;
                pMsg  = g_subphone_queue.get_message();
                if(pMsg){
                	int ivalue = pMsg->get_value();
                	if(ivalue == MESSAGE_SUBPHONE){
                		uint32_t ilen = 0;
                		unsigned char* buf = NULL;
                		buf = (unsigned char*)pMsg->get_message(ilen);
                		if(buf){
                			ZENFREE(buf);
                		}
                		
                	}
                	ZENFREE(pMsg);
                }else
			break;
	}
}


static int get_sd_index(PHONEINFO* phoneip,unsigned long ip)
{
	int i =0;
	//判断改IP是否存在，
	for(i = 0; i<MAX_PHONE;i++){
		if(phoneip[i].phone_ip == ip)
			return i;
	}
	
	for(i =0; i<MAX_PHONE;i++){
		if(phoneip[i].phone_ip == 0)
			return i;
	}
	return -1;
}

static int process_exist_phonenumber(PHONEINFO* phoneip,int index){
	int i =0;
	for(i = 0; i<MAX_PHONE;i++){
		if(phoneip[i].phoneindex == index && phoneip[i].phone_ip != 0){
			break;
		}
	}
	if(i != MAX_PHONE){
		WRT_DEBUG("该手机号已存在缓冲，重置该手机号的缓冲");
		return i;
	}
	return -1;	
}

static int get_idle_sd(PHONEINFO* phoneip){
	 int i = 0;
	 for(i =0;i<MAX_PHONE;i++){
	 	if(phoneip[i].phone_ip == 0)
	 		return i;	
	 }
	 return -1;
}

static int send_msg_other_phone(int state,int* fd,int index){
	int i;
	unsigned long b_msg[4]={0};
	if(state == 1)
		b_msg[0] = DOOR_HANGUP;
	else if(state == 2)
		b_msg[0] = CENTER_HANGUP;
		
	b_msg[1] = b_msg[2] = 0; 
	b_msg[3] = 0;
	for(i =0;i<MAX_PHONE;i++){
		if(fd[i] != -1 && (i != index) ){
			WRT_DEBUG("fd[i] = %d i= %d index = %d %x",fd[i],i,index,g_phone_ip[i].phone_ip);
			send_net_msg(fd[i],b_msg);
		}
	}
}

static int sub_phone_task(void* pv)
{
        unsigned long ret = 0;
        unsigned long t_msg[4]={0};
        unsigned long b_msg[4] = {0};
        int fd[MAX_PHONE] = {-1};
        char header[48];
        int keeplive[MAX_PHONE] = {0};
        int iscontinue =0;
        int delay = 30000; //30秒
        int index =0;
        int curstate = 0;
        fd[0] = fd[1] = fd[2] = fd[3] = -1;        
        while(!g_subphone_exit)
        {
			iscontinue = 0;
			memset(t_msg,0,sizeof(t_msg));
			ret = SDL_SemWaitTimeout(g_psubphone_sem,delay);
			delay = 30000;
            if(ret != 0)
            {
				for(index = 0; index < MAX_PHONE;index++){
					if(fd[index] != -1){
						if(keeplive[index] > 3){
							WRT_DEBUG("未知原因，连接中断1- %d",index);
							closesocket(fd[index]);
							fd[index] = -1;
							keeplive[index] = 0;
							continue;
						}
						memset(header,0,48);
						b_msg[0]  = ROOM_KEEP_LIVE;
						b_msg[1] = 0;
						b_msg[2] = 0;
						b_msg[3] = 0;
						if(send_net_msg(fd[index],b_msg) < 0){
							WRT_DEBUG("未知原因，连接中断2- %d",index);
							closesocket(fd[index]);
							fd[index] = -1;
							keeplive[index] = 0;
							continue;
						}
						keeplive[index]++;
						if((ret = recv_msg_from_net(fd[index],header,48,1,0)) == 1){
							short scmd = *(short*)(header+8);
							scmd = ntohs(scmd);
							if(scmd == PHONE_ACK_KEEP_LIVE){
								keeplive[index] = 0;
							}
						}
						}else{
                		if(g_phone_ip[index].phone_ip != 0)
                		{
								WRT_DEBUG("当网络由于未知原因被 close以后，尝试再次连接");
								keeplive[index] = 0;
								fd[index] = connect_to_phone(g_phone_ip[index].phone_ip);
								if(fd[index] == -1)
								{
									WRT_DEBUG("连接到手机失败 0x%x",g_phone_ip[index].phone_ip);
									b_msg[0] = PHONE_SESSION_END;
									b_msg[1] = 0x0000FFFF;
									b_msg[2] = b_msg[3] = 0;
									g_phone_callback(b_msg) ;//连接手机失败
									delay = 0;
									g_phone_ip[index].phone_ip = 0;
								}else{
									delay = 200;
								}                       		
							}

					}
				}
				continue;
			}
                if(ret == 0){
                	CWRTMsg* pMsg = NULL;
                	pMsg  = g_subphone_queue.get_message();
                	if(pMsg){
                		int ivalue = pMsg->get_value();
                		if(ivalue == MESSAGE_SUBPHONE){
                			uint32_t ilen = 0;
                			memset(t_msg,0,sizeof(t_msg));
                			unsigned char* buf = NULL;
                			buf =(unsigned char*) pMsg->get_message(ilen);
                			if(buf && (ilen  == sizeof(t_msg))){
                				memcpy((void*)t_msg,buf,ilen);
                				ZENFREE(buf);
                			}
                			
                		}
                		if(ivalue == EXIT_SUBPHONE){
                			ZENFREE(pMsg);
                			continue;
                		}
                		ZENFREE(pMsg);
                	}
                }
                switch(t_msg[0])
                {
                case DOOR_CALL_PHONE:
                case CENTER_CALL_PHONE:
                        WRT_DEBUG("呼叫 phone xxxxx");
                        for(index = 0; index < MAX_PHONE;index++){
                        	if(fd[index] == -1 && g_phone_ip[index].phone_ip != 0 &&  g_phone_ip[index].phone_ip != 0xffffffff){
                                	fd[index] = connect_to_phone(g_phone_ip[index].phone_ip);
                        	}
                        	if(fd[index] == -1 )
                        	{
                        		if(g_phone_ip[index].phone_ip != 0 &&  g_phone_ip[index].phone_ip != 0xffffffff)
                                		WRT_DEBUG("连接到手机失败 0x%x：%d",g_phone_ip[index].phone_ip,index);
                                	/*
			                               		 b_msg[0] = PHONE_SESSION_END;
			                               		 b_msg[1] = 0x0000FFFF;
			                                	 b_msg[2] = b_msg[3] = 0;
			                                	g_phone_callback(b_msg) ;//连接手机失败
			                                	iscontinue = 1;
			                               		break;
			                               		*/
                        	}else{
                        		WRT_DEBUG("呼叫%x:%d:%d ",g_phone_ip[index].phone_ip,fd[index],index);
                        	}
                        	
                	}
                        
                        //发送中心呼叫的消息给手机
                        if(t_msg[0] == DOOR_CALL_PHONE){
                                b_msg[0] = DOOR_CALLROOM;
                                curstate = 1;
                                //同时启动视频处理...
                        }else{
                                b_msg[0] = CENTER_CALLROOM;
                                curstate = 2;
                        }
                        b_msg[1] = b_msg[2] = 0; 
                        b_msg[3] = t_msg[3];
                    for(index = 0; index <MAX_PHONE;index++)
                    {
                        	if(fd[index] != -1)
                        	{
                        		if(send_net_msg(fd[index],b_msg)<0)
                    		{
                    			close(fd[index]);
                    			fd[index] = -1;
                    		}
                    	}
                    }                     
                    break;      
                case PHONE_MEDIA_INFO:
                	 for(index = 0; index < MAX_PHONE;index++){
                    	if(fd[index] == -1 && g_phone_ip[index].phone_ip != 0 &&  g_phone_ip[index].phone_ip != 0xffffffff){
                            	fd[index] = connect_to_phone(g_phone_ip[index].phone_ip);
                    	}
                    	if(fd[index] == -1 )
                    	{
                    		if(g_phone_ip[index].phone_ip != 0 &&  g_phone_ip[index].phone_ip != 0xffffffff)
                            		WRT_DEBUG("连接到手机失败 0x%x：%d",g_phone_ip[index].phone_ip,index);
                    	}else{
                    		WRT_DEBUG("发送媒体指令%x:%d:%d ",g_phone_ip[index].phone_ip,fd[index],index);
                    	}
	            	}
                    for(index = 0; index <MAX_PHONE;index++)
                    {
                    	if(fd[index] != -1)
                    	{
                    		if(send_net_msg(fd[index],t_msg)<0)
                        		{
                        			close(fd[index]);
                        			fd[index] = -1;
                        		}
                        	}
                        }                     
                        break;      
                case CONNECT_PHONE:
                	WRT_DEBUG("主动连接手机IP:%x phonenumber%d\n",t_msg[1],t_msg[2]);
                	index = process_exist_phonenumber(g_phone_ip,t_msg[2]);
                	if(index == -1){
                	index = get_sd_index(g_phone_ip,t_msg[1]);
                	if(index < 0){
                		index =  get_idle_sd(g_phone_ip);
                	}else{
                		
                        	if(fd[index] != -1){
                        		closesocket(fd[index]);
                        		fd[index] = -1;
                        	}
                        	
                        	
                        }
                        	if(index == -1){
                        		WRT_DEBUG("手机缓冲不够，最多支持4台设备");
                        		iscontinue = 1;
                        		break;
                        	}
                	}else{
                		if(fd[index] != -1)
                			closesocket(fd[index]);
                        	fd[index] = -1;
                        	g_phone_ip[index].phone_ip = 0;
                	}
                      
                        g_phone_ip[index].phone_ip = t_msg[1];
                        g_phone_ip[index].phoneindex = t_msg[2];
                        fd[index] = connect_to_phone(t_msg[1]);
                        WRT_DEBUG("主动连接:%x fd =%d ok \n",t_msg[1],fd[index]);
			if(fd[index] == -1)
                        {
                                WRT_DEBUG("主动连接到手机失败 0x%x：%d",t_msg[1],t_msg[0]);
                                b_msg[0] = PHONE_SESSION_END;
                                b_msg[1] = 0x0000FFFF;
                                b_msg[2] = b_msg[3] = 0;
                                g_phone_callback(b_msg) ;//连接手机失败
                        }else
                        	delay = 200;                        
                	iscontinue = 1;
                	break;
                default:
                	iscontinue = 1;
                        break;
                }
                if(iscontinue)
                	continue;
                ZENFREE((void*)t_msg[3]);
                memset(header,0,48);
                while(1)
                {
                        int i_end = 0;
                        memset(t_msg,0,sizeof(t_msg));
                       // WRT_DEBUG((char*)"wait recv msg");
                        memset(header,0,48);
                        ret = SDL_SemWaitTimeout(g_psubphone_sem,300);
                      //  ret = q_receive(qid_phone,Q_WAIT,100,t_msg); //t_msg[3] 需要释放的(id).
                        if(ret == 0)
                        {
                		CWRTMsg* pMsg = NULL;
                		pMsg  = g_subphone_queue.get_message();
                		if(pMsg){
                			int ivalue = pMsg->get_value();
                			if(ivalue == MESSAGE_SUBPHONE){
                				uint32_t ilen = 0;
                				memset(t_msg,0,sizeof(t_msg));
                				unsigned char* buf = NULL;
                				buf = (unsigned char*)pMsg->get_message(ilen);
                				if(buf && (ilen  == sizeof(t_msg))){
                					memcpy((void*)t_msg,buf,ilen);
                					ZENFREE(buf);
                				}
                				ZENFREE(pMsg);
                				
                			}else if(ivalue == EXIT_SUBPHONE){
                				ZENFREE(pMsg);
                				break;
                			}else{
                				ZENFREE(pMsg);
                			}
                		}    
                                if(t_msg[0] == CENTER_AND_DOOR_ROOM_HANGUP)
                                {
                                        if(t_msg[1] == 1)
                                                b_msg[0] = DOOR_HANGUP;
                                        else if(t_msg[1] == 2)
                                                b_msg[0] = CENTER_HANGUP;
                                        else if(t_msg[1] == 3)
                                                b_msg[0] = ROOM_HANGUP;

                                        b_msg[1] = b_msg[2] = 0; 
                                        b_msg[3] = t_msg[3];
                                        for(index=0;index<MAX_PHONE;index++){
                                        	if(fd[index] != -1)
                                        		send_net_msg(fd[index],b_msg);
                                        }
                                        WRT_DEBUG((char*)"发送消息0x%x到手机",b_msg[0]);
                                        break;
                                }
                                else{
                                        b_msg[0] = t_msg[1];
                                        b_msg[1] =b_msg[2] = b_msg[3] = 0;
                                         for(index=0;index<MAX_PHONE;index++){
                                         	if(fd[index] != -1)
                                        		send_net_msg(fd[index],b_msg);
                                        }
                                }

                        } 
						for(index = 0; index <MAX_PHONE;index++){
							if(fd[index] != -1){
								//WRT_DEBUG("recv_msg_from_net index %x",index);
                        		if((ret = recv_msg_from_net(fd[index],header,48,0,300000)) == 1)
                        		{
                        			WRT_DEBUG("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ",header[0],
                        						header[1],header[2],header[3],header[4],header[5],header[6],header[7],
                        						header[8],header[9]);
                        		        if(strncmp(header,"WRTI",4) == 0)
                        		        {
                        		        	int len = *(int*)(header+4);
                        		                short scmd = *(short*)(header+8);
                        		                scmd = ntohs(scmd);
                        		                WRT_DEBUG("len = %d scmd = 0x%x ip=%x %d",len ,scmd,g_phone_ip[index].phone_ip,index);
                        		                switch(scmd)
                        		                {
                        		                case  ROOM_IDLE:
                        		
                        		                        b_msg[0] = PHONE_SESSION_END;
                        		                        b_msg[1] = ROOM_IDLE;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg); //手机繁忙
                        		                        break;
                        		                case  ROOM_BUSY:
                        		                        b_msg[0] = PHONE_SESSION_END;
                        		                        b_msg[1] = ROOM_BUSY;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg); //手机繁忙
                        		                        break;
                        		                case ROOM_HOLDON:
                        		                        b_msg[0] = PHONE_HOLDON;
                        		                        b_msg[1] = 1;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg);
                        		                        //发送消息挂机，给其他手机
														send_msg_other_phone(curstate,fd,index);
                                                             		                        
                        		                        break;
                        		                case ROOM_HANGUP:
                        		                        b_msg[0] = PHONE_HANGUP;
                        		                        b_msg[1] = 2;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg); 
                        		                        i_end = 1;
                        		                        send_msg_other_phone(curstate,fd,index);
                        		                        break;
                        		                case ROOM_UNLOCK:
                        		                        b_msg[0] = PHONE_UNLOCK;
                        		                        b_msg[1] = 2;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg); 
                        		                        break;
                        		
                        		                }
                        		
                        		
                        		        }
                        		}
      
                        		if(ret == -2){
                        			 int j = 0;
                        			 int count =0;
                        			 for(j =0;j<MAX_PHONE;j++){
                        			 	if(fd[j] != -1){
                        			 		count++;
                        			 	}
                        			 }
                        			 if(count == 1){
                               			 	b_msg[0] = PHONE_HANGUP;
                               			 	b_msg[1] = 3;
                                		 	b_msg[2] = index;
                                		 	b_msg[3] = 0;
                                		 	g_phone_callback(b_msg) ;//连接手机失败
                                		 	i_end = 1;
                                		}else{
                                		        b_msg[0] = PHONE_SESSION_END;
                               			 	b_msg[1] = 0xfd;
                                		 	b_msg[2] = index;
                                		 	b_msg[3] = 0;
                                		 	g_phone_callback(b_msg) ;//连接手机失败	
                                		}
                                		//如果fd错误，则关闭fd 2012-2-17 9:10:54 added
                                		closesocket(fd[index]);
                                		fd[index] = -1;
                                		WRT_DEBUG("连接被对方中断 %x:%d  多路连接count =%d\n",g_phone_ip[index].phone_ip,index,count);
                                		
                        		}
                        
                		}
                	}
                        if(i_end == 1)
                                break;
                }
          /* //2012-2-17 9:10:57 delete 
                WRT_MESSAGE("关闭手机连接%d",fd);
                closesocket(fd);
                fd = -1;
                */
        }
		
        for(index = 0; index<MAX_PHONE; index++){
        	if(fd[index]  != -1)
        		closesocket(fd[index]);
        	fd[index] = -1;
        }
        
        clear_subphone_queue();
        SDL_DestroySemaphore(g_psubphone_sem);
        g_psubphone_sem = NULL;
       
}

void init_sub_phone()
{
    int ret  = 0 ;
    g_subphone_exit = 0;
    g_pSystemInfo = GetSystemInfo();
    if(g_psubphone_sem == NULL)
    	g_psubphone_sem = SDL_CreateSemaphore(0);
	SDL_CreateThread(sub_phone_task,NULL);
}

void uninit_sub_phone()
{
	g_subphone_exit = 1;
	g_subphone_queue.send_message(EXIT_SUBPHONE,g_psubphone_sem);
}

int send_msg_to_sub_phone(unsigned long t_msg[4])
{
      g_subphone_queue.send_message(MESSAGE_SUBPHONE,(void*)t_msg,sizeof(int)*4,g_psubphone_sem);
      return 1;
}

void set_phone_callback(PHONE_CALLBACK  _callback)
{
        g_phone_callback =  NULL;
        g_phone_callback = _callback;

}

unsigned long get_connect_ip(int index)
{
	if(index <0 || index > (MAX_PHONE-1))
		return 0;
	return g_phone_ip[index].phone_ip;
}

int have_idle_phone()
{
	int i =0;
	for(i = 0; i<MAX_PHONE;i++){
		if(g_phone_ip[i].phone_ip == 0)
			return 1;
	}
	return 0;
}
int have_connect_phone()
{
	int i =0;
	for(i = 0; i<MAX_PHONE;i++){
		if(g_phone_ip[i].phone_ip != 0)
			return 1;
	}
	return 0;	
}
