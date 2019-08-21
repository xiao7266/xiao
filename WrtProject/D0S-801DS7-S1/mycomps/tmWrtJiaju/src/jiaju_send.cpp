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
#include "tmJiaju.h"
#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "wrt_log.h"
#include "wrt_msg_queue.h"

#define closesocket close 
#define ioctlsocket ioctl

#define CONNECT_GATEWAY 0x1
#define SEND_SCENE_TO_GATEWAY 0x02 

/************************************2012-2-24 9:42:07*******************************/
#define GATEWAY_ACK_KEEP_LIVE         		0x0819
#define ROOM_KEEP_LIVE                           0x0619
#define ROOM_SEND_SCENE				0x0616
#define GATEWAY_ACK_SEND_SCENE			0x0816
#define REGISTER_GATEWAY			0x0691
#define GATEWAY_ACK_REGISTER			0x0891

static T_SYSTEMINFO* g_pSystemInfo = NULL;

static CWRTMsgQueue g_net_jiaju_queue;
static SDL_sem     *g_pnetjiaju_sem;
static int 	g_netjiaju_exit = 0;
static JIAJU_POOL_RESULT_CALLBACK g_netjiaju_callback =  NULL;

/**************************************************************************************/
#define EXIT_NETJIAJU                   0xFE
#define MESSAGE_NETJIAJU                0xFD

/************************************************************************
msg_buf[0]:������
msg_buf[1]:���������Ƿ�������ݣ������ʾ�������ݳ���
msg_buf[2]:���ݣ�������ݴ���4���ֽڣ����ʾһ��ָ�롣
msg_buf[3]��ָ�򷿼�š�
************************************************************************/
static char* make_packet(unsigned long msg_buf[4],int* packetlen)
{
        char* packbuf = NULL;
		char ourbuf[12];
        unsigned char tmpdata = NULL;
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
                tmpdata = (unsigned char)(msg_buf[2]&0x000000ff);
                packbuf = (char*)ZENMALLOC(cmdlen);
        }
        if(packbuf == NULL){
                if(packetlen)
                        *packetlen =0;
                return NULL;
        }

        tmpcmd = htons(tmpcmd);
        memset(packbuf,0,cmdlen);
        strncpy((char*)packbuf,"WRTI",4);
        memcpy(packbuf+4,&cmdlen,4);
        memcpy(packbuf+8,&tmpcmd,2);

		memcpy(packbuf+4,&tmpcmd,2);
		memcpy(packbuf+6,&cmdlen,4);

//: hex = 0x3A
        if(g_pSystemInfo){
                if(id == NULL){
                	memcpy(packbuf+10,g_pSystemInfo->mySysInfo.gatewayroomid,15);
                }else
                        memcpy(packbuf+10,id,15);
                memcpy(packbuf+25,g_pSystemInfo->mySysInfo.sysID,15);
        }

		memset(ourbuf,0,12);
		sprintf(ourbuf,"%02x%02x%02x%02x%02x%02x",g_pSystemInfo->BootInfo.MAC[0],
                g_pSystemInfo->BootInfo.MAC[1],g_pSystemInfo->BootInfo.MAC[2],g_pSystemInfo->BootInfo.MAC[3],
                g_pSystemInfo->BootInfo.MAC[4],g_pSystemInfo->BootInfo.MAC[5]);
		packbuf[10]=0x02;
		memcpy(packbuf+11,ourbuf,12);
		
        //memcpy(packbuf+40,g_pSystemInfo->mySysInfo.gatewaypwd,8);

        if(cmdlen > 40){
                memcpy(packbuf+40,(void*)&tmpdata,msg_buf[1]);
        }
        if(packetlen)
                *packetlen = cmdlen;

        return packbuf;
}

static int connect_to_netjiaju_gateway(unsigned long ipaddr){
        struct sockaddr_in addr;
        int tmp_sock = -1;
        int port = 8899;
        fd_set writefd;
        struct timeval timeout;    
        int max_fd = -1; 
        int flag = 1;
        int rc =0;
        if(ipaddr == 0 || ipaddr == 0xFFFFFFFF){
        	printf("Ŀ��IP��ַ��Ч\n");
        	return -1;
        }
        tmp_sock  = socket(AF_INET,SOCK_STREAM, 0);
        if(tmp_sock < 0){
                printf("����socket failed\n");              
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
                        printf((const char*)"���ӵ��Ҿ����سɹ�0x%x:%d\n",ipaddr,port);
                        return tmp_sock;
                }  
        }else{
                printf((const char*)"���ӵ��Ҿ�����ʧ��%x\n",errno);
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
                                    printf("�������ݵ��Ҿ����ش��� %x\n",errno);
                                }else
                                    printf("�������ݵ��Ҿ����سɹ�%d�ֽ�\n",ret);
                        }
                        ZENFREE(packet);
                        packet  = NULL;
                }
        }
        return result;
}

static int  recv_cmd_msg_from_net(int fd,char* databuffer,int datalen,int sec, int usec)
{

reget:
        fd_set readfd;
        int rc=0;
        int ret = 0;
        int flag = 0;
        int irecvlen = 0;
        char* packet = NULL;
        struct timeval timeout;
        timeout.tv_sec = sec;
        timeout.tv_usec = usec;//300000;

        FD_ZERO(&readfd);
        FD_SET(fd,&readfd);
        memset(databuffer,0,datalen);
        while(irecvlen < datalen)
        {
                rc =  select(fd+1,&readfd,NULL,NULL,&timeout);
                if(rc > 0)
                {
                        if(FD_ISSET(fd,&readfd))
                        {

                                ret = recv(fd,databuffer+irecvlen,datalen-irecvlen,0);
                                if(ret ==  0)
                                {
                                        if(errno != EWOULDBLOCK)
                                        {
                                                return -2;
                                        }
                                        return 1;
                                }
                                else if(ret < 0)
                                {

                                        return  -2;
                                }
                                irecvlen += ret;
                                //for(int i=0;i<irecvlen;i++)
                                {
                                	//printf("0x%x ",databuffer[i]);
                                }
                                //printf("\n");
                                
                                WRT_DEBUG("���հ�:recv_msg_from_net %d\n",irecvlen);
                               
                        }
                }else if(rc < 0)
                {
						WRT_DEBUG("���հ�ʧ��");
                        return -2;
                }else{
                	WRT_DEBUG("�Ҿ����糬ʱ����irecvlen = %d",irecvlen);
                	if(irecvlen > 0)
                		return 1;
                    return -1;
                }
        }
        return 1;
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
        while(irecvlen < datalen)
        {
                rc =  select(fd+1,&readfd,NULL,NULL,&timeout);
                if(rc > 0)
                {
                        if(FD_ISSET(fd,&readfd))
                        {

                                ret = recv(fd,databuffer+irecvlen,datalen-irecvlen,0);
                                if(ret ==  0)
                                {
                                        if(errno != EWOULDBLOCK)
                                        {
                                                return -2;
                                        }
                                        return 1;
                                }
                                else if(ret < 0)
                                {

                                        return  -2;
                                }
                                irecvlen += ret;
                                WRT_DEBUG("���հ�:recv_msg_from_net %d\n",irecvlen);
                               
                        }
                }else if(rc < 0)
                {
						WRT_DEBUG("���հ�ʧ��");
                        return -2;
                }else{
                	WRT_DEBUG("�Ҿ����糬ʱ����irecvlen = %d",irecvlen);
                	if(irecvlen > 0)
                		return 1;
                        return -1;
                }
        }
        return 1;
}

static void clear_netjiaju_queue()
{
	while(1)
	{
		CWRTMsg* pMsg = NULL;
                pMsg  = g_net_jiaju_queue.get_message();
                if(pMsg){
                	int ivalue = pMsg->get_value();
                	if(ivalue == MESSAGE_NETJIAJU){
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
static int gateway_fd = -1;
static int net_jiaju_task(void* pv)
{
        unsigned long ret = 0;
        unsigned long gatewayip = 0;
        unsigned long t_msg[4]={0};
        unsigned long b_msg[4] = {0};
		//int fd;
        char header[48];
        int keeplive = 0;
        int iscontinue =0;
        int delay = 10000; //30��
        while(!g_netjiaju_exit)
        {
        		iscontinue = 0;
                memset(t_msg,0,sizeof(t_msg));
                ret = SDL_SemWaitTimeout(g_pnetjiaju_sem,delay);
                delay = 10000;
                if(ret != 0)
					;
                if(ret == 0){
                	CWRTMsg* pMsg = NULL;
                	pMsg  = g_net_jiaju_queue.get_message();
                	if(pMsg){
                		int ivalue = pMsg->get_value();
                		if(ivalue == MESSAGE_NETJIAJU){
                			uint32_t ilen = 0;
                			memset(t_msg,0,sizeof(t_msg));
                			unsigned char* buf = NULL;
                			buf =(unsigned char*) pMsg->get_message(ilen);
                			if(buf && (ilen  == sizeof(t_msg))){
                				memcpy((void*)t_msg,buf,ilen);
                				ZENFREE(buf);
                			}
                			
                		}
                		if(ivalue == EXIT_NETJIAJU){
                			ZENFREE(pMsg);
                			continue;
                		}
                		ZENFREE(pMsg);
                	}
                }
                switch(t_msg[0])
                {
	                case SEND_SCENE_TO_GATEWAY: //�����龰��

					gatewayip = t_msg[2];
                    gateway_fd = connect_to_netjiaju_gateway(t_msg[2]);
					
                	printf("SEND_SCENE_TO_GATEWAY  scene = %d ��fd=%d",t_msg[1],gateway_fd);
                	if(gateway_fd != -1){
                		int ret = 0;
                		unsigned long tmp[4];
                		tmp[0] = ROOM_SEND_SCENE;
                		tmp[1] = 1;
                		tmp[2] = t_msg[1];
                		tmp[3] = 0;
                		if(send_net_msg(gateway_fd,tmp) < 0){
								printf((char*)"δ֪ԭ�������ж�\n");
								closesocket(gateway_fd);
								gateway_fd = -1;
								if(g_netjiaju_callback){
									tmp[0] = 0;
									tmp[1] = 2;
									tmp[2] = 0;
									tmp[3] = 0;
									g_netjiaju_callback(tmp);
							}                     
							break;
						}else
						{
							if(g_netjiaju_callback){
								tmp[0] = 1;//����ͳɹ�
								tmp[1] = 2;
								tmp[2] = 0;
								tmp[3] = 0;
								g_netjiaju_callback(tmp);
							}
						}
						printf("scene = %d ,���ͳɹ����ȴ���Ӧ\n",t_msg[1]);
regetack:
						if((ret = recv_cmd_msg_from_net(gateway_fd,header,42,1,0)) == 1){
							short scmd = *(short*)(header+4);
							
							scmd = ntohs(scmd);
							
							if(scmd == GATEWAY_ACK_SEND_SCENE){
								printf("�������� %d header40 %d header41%d\n",scmd,header[40],header[41]);
								unsigned long scene = header[40];
								if(scene == t_msg[1] && header[41] == 0x01){
									//���Ƴɹ�����ص���
									
									if(g_netjiaju_callback){
										tmp[0] = 3;//����ͳɹ�
										tmp[1] = 2;
										tmp[2] = 0;
										tmp[3] = 0;
										g_netjiaju_callback(tmp);
									}

									break;
								}else
								{
									goto regetack;
								}
							}
                         }else
                         {
                         	//����ʧ�ܡ�����ص�
							if(g_netjiaju_callback){
								tmp[0] = 4;
								tmp[1] = 2;
								tmp[2] = 0;
								tmp[3] = 0;
								g_netjiaju_callback(tmp);
							}
							printf("���ճ�ʱ......\n");
                         }
                         
                	}else{//����ʧ�ܡ�����ص�
                        printf("����ʧ��,fd =-1,�������Ӳ���\n");
                        
						if(g_netjiaju_callback){
							unsigned long tmp[4];
							tmp[0] = 0;
							tmp[1] = 2;
							tmp[2] = 0;
							tmp[3] = 0;
							g_netjiaju_callback(tmp);
						}                    	
					}		
                	break;
					
                default:
                	iscontinue = 1;
                    break;
                }
				
				if(gateway_fd > 0)
				{
					printf("�ر������׽���\n");
					closesocket(gateway_fd);
					gateway_fd = -1;
				}
        }
        
        clear_netjiaju_queue();
        SDL_DestroySemaphore(g_pnetjiaju_sem);
        g_pnetjiaju_sem = NULL;
       
}

void init_net_jiaju(JIAJU_POOL_RESULT_CALLBACK _callback)
{
    int ret  = 0 ;
    g_netjiaju_callback = _callback;
    g_netjiaju_exit = 0;
    g_pSystemInfo = GetSystemInfo();
    if(g_pnetjiaju_sem == NULL)
    	g_pnetjiaju_sem = SDL_CreateSemaphore(0);
	SDL_CreateThread(net_jiaju_task,NULL);
}

void uninit_net_jiaju()
{
	g_netjiaju_exit = 1;
	g_net_jiaju_queue.send_message(EXIT_NETJIAJU,g_pnetjiaju_sem);
}


void set_net_jiaju_callback(JIAJU_POOL_RESULT_CALLBACK _callback)
{
	   g_netjiaju_callback = _callback;
}

int control_scene(int scene)
{
      unsigned long t_msg[4];
  	
      t_msg[0] = SEND_SCENE_TO_GATEWAY;
      t_msg[1] = scene;
      t_msg[2] = g_pSystemInfo->mySysInfo.gatewayip;
      t_msg[3] = 0;
      
      g_net_jiaju_queue.send_message(MESSAGE_NETJIAJU,(void*)t_msg,sizeof(t_msg),g_pnetjiaju_sem);
      return 1;	
}

void connect_net_jiaju_gateway()
{
  	unsigned long t_msg[4];
  	
      t_msg[0] = CONNECT_GATEWAY;
      t_msg[1] = g_pSystemInfo->mySysInfo.gatewayip ;
      t_msg[2] = 0;
      t_msg[3] = 0;
      
      g_net_jiaju_queue.send_message(MESSAGE_NETJIAJU,(void*)t_msg,sizeof(t_msg),g_pnetjiaju_sem);
		
}
