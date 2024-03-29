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
#include "wrt_subroom.h"
#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "wrt_log.h"
#include "wrt_msg_queue.h"
#include "media_negotiate.h"

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

//-----------------------------------2014-6-25
#define MEDIA_NEGOTIATE_SESSION		   0x1195

/*-------------------------------------*/
#define EXIT_SUBROOM                    0xFE
#define MESSAGE_SUBROOM                 0xFD

static T_SYSTEMINFO* g_pSystemInfo = NULL;

static CWRTMsgQueue g_subroom_queue;
static SDL_sem     *g_psubroom_sem;
static int 			g_subroom_exit = 0;

static unsigned long gp_subroom_ip[4] = {0};

static SUBROOM_CALLBACK g_subroom_callback =  NULL;

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
                }else{
	                memcpy(packbuf+10,id,15);
                }
                memcpy(packbuf+25,"000000000000000",15);
        }

        if(cmdlen > 40){
                memcpy(packbuf+40,(void*)tmpdata,msg_buf[1]);
        }
        if(packetlen)
                *packetlen = cmdlen;
        return packbuf;
}

static int connect_to_subroom(unsigned long ipaddr){
        struct sockaddr_in addr;
        int tmp_sock = -1;
        int port = 20200;
        fd_set writefd;
        struct timeval timeout;    
        int max_fd = -1; 
        int flag = 1;
        int rc =0;
        if(ipaddr == 0 || ipaddr == 0xffffffff)
        	return -1;
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
        timeout.tv_sec = 0;
        timeout.tv_usec = 400*1000;

        rc = select(max_fd+1,NULL,&writefd,NULL,&timeout);
        if(rc > 0){      
                if(FD_ISSET(tmp_sock,&writefd)){
                        WRT_MESSAGE("连接到从分机成功0x%x:%d\n",ipaddr,port);
                        return tmp_sock;
                }  
        }else{
                WRT_MESSAGE("连接到从分机失败%x",errno);
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
        timeout.tv_usec = 100000;

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
                                    printf("xxxxxxxxxxxxxx发送数据到从分机错误 %x --%d\n",errno,fd);
                                }else
                                    printf("xxxxxxxxxxxxxx发送数据到从分机成功%d字节 --%d\n",ret,fd);
                        }
                        ZENFREE(packet);
                        packet  = NULL;
                }
        }
        return result;
}

static int  recv_msg_from_net(int fd,char* databuffer,int* datalen,int sec, int usec)
{

        fd_set readfd;
        int rc=0;
        int ret = 0;
        int irecvlen = 0;
        int buflen = *datalen;
        char* packet = NULL;
        struct timeval timeout;
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;//300000;

        FD_ZERO(&readfd);
        FD_SET(fd,&readfd);
        memset(databuffer,0,buflen);
    
        while(irecvlen < buflen)
        {
                rc =  select(fd+1,&readfd,NULL,NULL,&timeout);
       
                if(rc > 0)
                {
                        if(FD_ISSET(fd,&readfd))
                        {

                                ret = recv(fd,databuffer+irecvlen,(size_t)databuffer-irecvlen,0);
                                if(ret ==  0)
                                {
                                        if(errno != EWOULDBLOCK)
                                        {
                                                return -2;
                                        }
                                        *datalen = irecvlen;
                                        return 1;
                                }
                                else if(ret < 0)
                                {
                                        return  -2;
                                }
                                irecvlen += ret;
                                printf("xxxxxxxxxxxxxxrecv_msg_from_net %d\n",irecvlen);
                        }
                }else if(rc < 0)
                {
                        return -2;
                }else{
						if(irecvlen > 0){
							printf("xxxxxxxxxxxxxx收到数据 %d返回\n",irecvlen);
							 *datalen = irecvlen;
							return 1;
						}
                        return -1;
                }
        }
		*datalen = irecvlen;
        return 1;
}

static void clear_subroom_queue()
{
	while(1)
	{
		CWRTMsg* pMsg = NULL;
	    pMsg  = g_subroom_queue.get_message();
	    if(pMsg){
	    	int ivalue = pMsg->get_value();
	    	if(ivalue == MESSAGE_SUBROOM){
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

static int get_subroom_sd_index(unsigned long* subroomip,unsigned long ip)
{
	int i =0;
	//判断该IP是否存在
	for(i = 0; i<MAX_SUBROOM;i++){
		if(subroomip[i] == ip)
			return i;
	}
	
	for(i =0; i<MAX_SUBROOM;i++){
		if(subroomip[i] == 0)
			return i;
	}
	return -1;
}

static int get_subroom_idle_sd(unsigned long* subroomip){
	 int i = 0;
	 for(i =0;i<MAX_SUBROOM;i++){
	 	if(subroomip[i] == 0)
	 		return i;	
	 }
	 return -1;
}

static int send_msg_other_subroom(int state,int* fd,int index){
	int i;
	unsigned long b_msg[4]={0};
	if(state == 1)
		b_msg[0] = DOOR_HANGUP;
	else if(state == 2)
		b_msg[0] = CENTER_HANGUP;
	else
		b_msg[0] = ROOM_HANGUP;
		
	b_msg[1] = b_msg[2] = 0; 
	b_msg[3] = 0;
	for(i =0;i<MAX_SUBROOM;i++){
		if(fd[i] != -1 && (i != index) ){
			printf("xxxxxxxxxxxxxxfd[i] = %d i= %d index = %d %x\n",fd[i],i,index,gp_subroom_ip[i]);
			send_net_msg(fd[i],b_msg);
		}
	}
}

static int subroom_task(void* pv)
{
        unsigned long ret = 0;
        unsigned long t_msg[4]={0};
        unsigned long b_msg[4] = {0};
        int fd[MAX_SUBROOM] = {-1};
        char header[48];
        int keeplive[MAX_SUBROOM] = {0};
        int iscontinue =0;

        int delay = 30000; //30秒
        int index =0;
        int curstate = 0;
 
        fd[0] = fd[1] = fd[2] = fd[3] = -1;
        while(1)
        {
			memset(t_msg,0,sizeof(t_msg));
			iscontinue = 0;
			
			delay = 30000;
			ret = SDL_SemWaitTimeout(g_psubroom_sem,delay);
			delay = 30000;
            if(ret != 0)
			{
				continue;
			}

			if(ret == 0){
            	CWRTMsg* pMsg = NULL;
            	pMsg  = g_subroom_queue.get_message();
            	if(pMsg){
            		int ivalue = pMsg->get_value();
            		if(ivalue == MESSAGE_SUBROOM){
            			uint32_t ilen = 0;
            			memset(t_msg,0,sizeof(t_msg));
            			unsigned char* buf = NULL;
            			buf =(unsigned char*) pMsg->get_message(ilen);
            			if(buf && (ilen  == sizeof(t_msg))){
            				memcpy((void*)t_msg,buf,ilen);
            				ZENFREE(buf);
            			}
            			
            		}
            		if(ivalue == EXIT_SUBROOM){
            			ZENFREE(pMsg);
            			continue;
            		}
            		ZENFREE(pMsg);
            	}
            }
			switch(t_msg[0])
			{
				case OTHERROOM_CALL_SUBROOM:
				case DOOR_CALL_SUBROOM:
				case CENTER_CALL_SUBROOM:

				//	printf("xxxxxxxxxxxxxx呼叫 从分机 0x%x\n",t_msg[0]);
					for(index = 0; index<MAX_SUBROOM;index++){
						
						fd[index] = connect_to_subroom(gp_subroom_ip[index]);
				//		printf("fd[%d] = %d  ip=0x%x\n",index,fd[index],gp_subroom_ip[index]);
					}
					
					if(t_msg[0] == DOOR_CALL_SUBROOM){
						b_msg[0] = DOOR_CALLROOM;
						curstate = 1;
						
					}else if(t_msg[0] == CENTER_CALL_SUBROOM){
						b_msg[0] = CENTER_CALLROOM;
						curstate = 2;
					}else{
						b_msg[0] = ROOM_CALLROOM;
						curstate = 3;
					}

					b_msg[1] = b_msg[2] = 0; 
					b_msg[3] = t_msg[3];
					for(index =0; index < MAX_SUBROOM;index++){
						if(fd[index] != -1)
							send_net_msg(fd[index],b_msg);   
					}
					if(b_msg[3] != 0)
						ZENFREE((void*)b_msg[3]);
						
					break;
				
				default:
					iscontinue = 1;
					break;
			}
            if(iscontinue == 1)
            	continue;
            
            memset(header,0,48);
            
            while(1)
			{
				int i_end = 0;
				int  msglen = 0;
				memset(t_msg,0,sizeof(t_msg));
				
				memset(header,0,48);
				ret = SDL_SemWaitTimeout(g_psubroom_sem,300);
				if(ret == 0)
				{
					CWRTMsg* pMsg = NULL;
					pMsg  = g_subroom_queue.get_message();
					if(pMsg){
						int ivalue = pMsg->get_value();
						if(ivalue == MESSAGE_SUBROOM){
							uint32_t ilen = 0;
							memset(t_msg,0,sizeof(t_msg));
							unsigned char* buf = NULL;
							buf = (unsigned char*)pMsg->get_message(ilen);
							if(buf && (ilen  == sizeof(t_msg))){
								memcpy((void*)t_msg,buf,ilen);
								ZENFREE(buf);

							}
							ZENFREE(pMsg);

						}else if(ivalue == EXIT_SUBROOM){
							ZENFREE(pMsg);
							break;
						}else{
							ZENFREE(pMsg);
						}
					}
					
					if(t_msg[0] == CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM)
					{
						if(t_msg[1] == 1)
							b_msg[0] = DOOR_HANGUP;
						else if(t_msg[1] == 2)
							b_msg[0] = CENTER_HANGUP;
						else if(t_msg[1] == 3)
							b_msg[0] = ROOM_HANGUP;

						b_msg[1] = b_msg[2] = 0; 
						b_msg[3] = t_msg[3];
						for(index=0;index<MAX_SUBROOM;index++){
							if(fd[index] != -1){
								send_net_msg(fd[index],b_msg);
								closesocket(fd[index]);
								fd[index] = -1;
							}
						}
						printf("xxxxxxxxxxxxxx发送消息%x到所有的从分机\n",b_msg[0]);
						break;
					}else if(t_msg[0] == MEDIA_NEGOTIATE_SUBROOM)
					{
						b_msg[0] = MEDIA_NEGOTIATE_SESSION;
						
						b_msg[1] = b_msg[2] = 0; 
						b_msg[3] = t_msg[3];
						
						for(index=0;index<MAX_SUBROOM;index++){
							if(fd[index] != -1){
								send_net_msg(fd[index],b_msg);
								
							}
						}
						printf("转发主机呼叫媒体协商转发至从机\n");
						
					}
				}
				
				for(index = 0; index <MAX_SUBROOM;index++){
					if(fd[index] != -1){
						
						msglen = 40;
						if((ret = recv_msg_from_net(fd[index],header,&msglen,0,100*1000)) == 1)
						{
							if(strncmp(header,"WRTI",4) == 0)
							{
								int len = *(int*)(header+4);
								short scmd = *(short*)(header+8);
								scmd = ntohs(scmd);
								printf("xxxxxxxxxxxxxxlen = %d scmd = 0x%x ip=%x %d\n",len ,scmd,gp_subroom_ip[index],index);
								
								switch(scmd)
								{
									case  ROOM_IDLE:

										b_msg[0] = SUBROOM_SESSION_END;
										b_msg[1] = ROOM_IDLE;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg); //从机空闲
										break;
									case  ROOM_BUSY:
										b_msg[0] = SUBROOM_SESSION_END;
										b_msg[1] = ROOM_BUSY;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg); //从机繁忙
										closesocket(fd[index]);
										fd[index] = -1;
										break;
									case ROOM_HOLDON:
										b_msg[0] = SUBROOM_HOLDON;
										b_msg[1] = 1;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg);
										printf("ROOM_HOLDON 44444\n");
										//发送消息挂机，给其他从机
										send_msg_other_subroom(curstate,fd,index);

										break;
									case ROOM_HANGUP:
										b_msg[0] = SUBROOM_HANGUP;
										b_msg[1] = 2;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg); 
										send_msg_other_subroom(curstate,fd,index);
										i_end = 1;
										printf("接收从分机分机挂机指令\n");
										break;
									case ROOM_UNLOCK:
										b_msg[0] = SUBROOM_UNLOCK;
										b_msg[1] = 2;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg); 
										break;
									case ROOM_AGENT:
										b_msg[0] = SUBROOM_AGENT;
										b_msg[1] = 2;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg); 
										break;
									
									default:
										printf("000000\n");
										break;

								}
							}
						}

						if(ret == -2){

							b_msg[0] = SUBROOM_SESSION_END;
							b_msg[1] = 0xfd;
							b_msg[2] = index;
							b_msg[3] = 0;
							g_subroom_callback(b_msg) ;//连接从机失败	

							//如果fd错误，则关闭fd 2012-2-17 9:10:54 added
							closesocket(fd[index]);
							fd[index] = -1;

							printf("xxxxxxxxxxxxxx连接被对方中断 %x:%d \n",gp_subroom_ip[index],index);

						}
					}
				}
				
				if(i_end == 1)
					break;
			}
				
            for(index =0 ;index < MAX_SUBROOM;index++){
            	if(fd[index] != -1){
            		printf("xxxxxxxx close sock %d\n",fd[index]);
            		closesocket(fd[index]);
            		fd[index] = -1;
            	}
        	}
    }
        
}

extern unsigned long get_sub_room_ip(int num);
void init_subroom()
{
    int ret  = 0 ;
	unsigned long tmpip;
    g_pSystemInfo = GetSystemInfo();
	//if(g_pSystemInfo){
	for(int i = 5;i<9;i++)
	{
		tmpip = 0;
		tmpip = get_sub_room_ip(i);
		//printf("tmpip = 0x%x\n"),tmpip;
		if(tmpip != 0)
		{
			gp_subroom_ip[i-5] = tmpip;
		}
		else
		{
			gp_subroom_ip[i-5] = 0;
		}
	}
	if(g_psubroom_sem == NULL)
		g_psubroom_sem = SDL_CreateSemaphore(0);

	SDL_CreateThread(subroom_task,NULL);
}

void reinit_subroom_ip()
{
	unsigned long tmpip;
	int i ;
	for(i = 0;i<4;i++)
	{
		gp_subroom_ip[i] = 0;
	}
	
	for( i=5;i<9;i++)
	{
		tmpip = 0;
		tmpip = get_sub_room_ip(i);
		//printf("room %d ip = 0x%x\n",i,tmpip);
		if(tmpip != 0)
			gp_subroom_ip[i-5] = get_sub_room_ip(i);
		else
			gp_subroom_ip[i-5] = 0;
	}
}

unsigned long get_subroom_ip_by_index(int index)
{
	//printf("gp_subroom_ip[%d] = 0x%x\n",index,gp_subroom_ip[index]);
	return gp_subroom_ip[index];
}
void uninit_sub_room()
{
	g_subroom_exit = 1;
	g_subroom_queue.send_message(EXIT_SUBROOM,g_psubroom_sem);
}

int send_msg_to_subroom(unsigned long t_msg[4])
{
	
      g_subroom_queue.send_message(MESSAGE_SUBROOM,(void*)t_msg,sizeof(int)*4,g_psubroom_sem);
      return 1;
}

void set_subroom_callback(SUBROOM_CALLBACK  _callback)
{
        g_subroom_callback =  NULL;
        g_subroom_callback = _callback;
}

unsigned long get_subroom_connect_ip(int index)
{
	if(index <0 || index > (MAX_SUBROOM-1))
		return 0;
		
	return gp_subroom_ip[index];
}

int have_idle_subroom()
{
	int i =0;

	for(i = 0; i<MAX_SUBROOM;i++){
		if(gp_subroom_ip[i] == 0)
			return 1;
	}
	return 0;
}

int have_connect_subroom()
{
	int i =0;

	for(i = 0; i<MAX_SUBROOM;i++){
		if(gp_subroom_ip[i] != 0)
			return 1;
	}
	return 0;	
}

