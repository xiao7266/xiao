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
//�ֻ�ָ��
#define ROOM_CALLCENTER		      0x0001               /*��������*/
#define ROOM_EMERG		          0x0002               /*��������*/
#define ROOM_INQFEE		          0x0003               /*���ò�ѯ*/
#define ROOM_REPAIR		          0x0004               /*����ά��*/
#define ROOM_BROWSERING		      0x0005               /*���������б�*/
#define ROOM_RINGDOWN		      0x0006               /*��������*/
#define ROOM_GETTIME		      0x0007               /*���ʱ��*/
#define ROOM_WARNING              0x0008               /*����*/
#define ROOM_CALLROOM		      0x0009               /*����ͨ*/
#define ROOM_INQADDR              0x000a               /*�����Ӧ����ŵ�IP��ַ*/
#define ROOM_GETCONFIGACK         0x000b               /*��Ӧ����ϵͳ��Ϣ*/

#define ROOM_ASKFOR_SYSINFO       0X000C                /*�ֻ���������ֻ�������Ϣ*/
#define ROOM_GET_SYSINFO_ACK      0x000d                /*�ֻ���Ӧ������Ϣ*/
#define ROOM_INQSERVICEINFO       0x000e                /*�������������Ϣ����ʱ�Զ���*/
#define ROOM_INQHELPINFO          0X000F                /*�ֻ����������Ϣ */
//
#define ROOM_STARTMONT		      0x0020               /*��ʼ����*/
#define ROOM_STOPMONT		      0x0021               /*ֹͣ����*/
//�Ҿ������ļҾ�Э��
#define ROOM_STATUSACK            0x0010               /*�����Ӧ��ѯ���еļҾ�״̬*/
#define ROOM_SETELECACK           0x0011               /*��Ӧ���Ŀ��ƼҾ�*/
#define ROOM_GETELECACK           0x0012               /*��Ӧ���Ĳ�ѯĳ���Ҿӵ�״̬*/
#define ROOM_SCENEACK             0x0013               /*��Ӧ�����龰����*/

/*2009-7-29  new add cmd*/
#define ROOM_REMOTE_SWITCH  0X0015             /*֪ͨ���ģ��ֻ����á��ر�Զ�̿���*/
#define ROOM_SIGNLE_GUARD_ACK 0X0016       /*��Ӧ���ģ��������Ʒ�������ҪΪ�绰��������*/
#define ROOM_GET_ELECFIILE_ACK   0x0017      /*��Ӧ���ģ���üҾ����ñ�*/
#define ROOM_SET_ELECFILE_ACK   0X0018      /*��Ӧ���ģ����üҾ����ñ�*/

#define ROOM_SEND_PHOTO_VOICE   0X1107              /*�ֻ�����ץ��ͼƬ������*/


//
#define ROOM_BUSY		              0x0090              /*��æ*/
#define ROOM_IDLE	                  0x0091              /*����*/
#define ROOM_HOLDON		              0x0092              /*ժ��*/
#define ROOM_HANGUP		              0x0093              /*�һ�*/
#define ROOM_UNLOCK		              0x0094              /*����*/
#define ROOM_AGENT                    0x0095              /*�ֻ��й�*/
#define ROOM_QUIET                    0x0096              /*�ֻ�����*/
#define ROOM_BROADCASTACK             0x0022              /*��Ӧ�յ�����ɹ�/ʧ��*/
#define ROOM_UPGRADEACK               0x0023              /*������Ӧ*/
#define ROOM_LEAVEWORDACK             0x0024              /*���Ի�Ӧ*/
#define ROOM_STOPLEAVEWORDACK         0x0025              /*��Ӧ���Խ���*/

#define ROOM_CALL_LIFT                0x0030              /*�ٻ�����*/

//�ֻ���ֻ�֮�������
#define ROOM_SYNC_AGENT               0x0080              /*�ֻ�ͬ���й�*/
#define ROOM_CANCEL_SYNC_AGENT        0x0081              /*�ֻ�ȡ��ͬ���й�*/
#define ROOM_SYNC_AVOID               0x0082              /*�ֻ�ͬ�������*/
#define ROOM_CANCEL_SYNC_AVOID        0x0083              /*�ֻ�ȡ��ͬ�������*/
#define ROOM_SYNC_RING_TIME           0x0084              /*�ֻ�����ʱ��ͬ��*/
#define ROOM_SMALLDOOR_HOLDON         0X0085              /*С�ſڻ���ժ��ͬ��*/
#define ROOM_SMALLDOOR_HANGUP         0X0086              /*С�ſڻ����У��һ�ͬ��*/
#define ROOM_REQ_SYNC                 0X0087              /*�ӷֻ�����ʱ�����ֻ���������ͬ������Ϣ*/
#define ROOM_HOST_BUSY                0X0088              /*����Ƕ�ֻ���С�ſڻ����У����ֻ���æ�����͸�ͬ����Ϣ*/
#define ROOM_SEND_KEY                 0x0089              /*�ֻ����ͼ�*/


#define ROOM_SEND_ALARM                  0X1000        /*�����������*/

//����ָ��

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

#define DOOR_BROADCAST              0x010a             /*ת������*/
#define DOOR_BROADCASTPIC           0x010b             /*ת����ͼƬ����*/
#define DOOR_STARTLEAVEWORD         0x010e             /*����*/
#define DOOR_STOPTLEAVEWORD         0x010f             /*��������*/


//����ָ��
#define CENTER_EMERGACK		        0x0201             /*���Ļ�Ӧ��������*/
#define CENTER_INQFEEACK	        0x0202             /*���Ļ�Ӧ���ò�ѯ*/
#define CENTER_REPAIREACK	        0x0203             /*���Ļ�Ӧ����ά��*/
#define CENTER_TIMEACK		        0x0204             /*���Ļ�Ӧ���ʱ��*/
#define CENTER_CALLROOM		        0x0205             /*���ĺ����û�*/
#define CENTER_RINGERRACK           0x0206             /*���Ļ�Ӧ��ȡ����ʧ��*/
#define CENTER_RINGLSTACK           0x0207             /*���Ļ�Ӧ��ȡ�����б�*/
#define CENTER_RINGDOWNACK          0x0208             /*���Ļ�Ӧ��������*/
#define CENTER_CALLROOMACK          0x0209             /*����Ӧ���ѯIP��ַ*/

#define CENTER_SERVERINFOACK        0x020a             /*���Ļ�Ӧ��ȡ������Ϣ����ʱ�Զ�����*/

#define CENTER_STARTMONT	        0x0227             /*������������*/
#define CENTER_STARTLSN		        0x0228             /*������������*/
#define CENTER_STOPMONT		        0x0229             /*����ֹͣ����*/
#define CENTER_STIPLSN		        0x0230             /*����ֹͣ����*/
#define CENTER_SETIDTOIPTABLE       0X0238             /*��������ID TO IP ��*/
#define CENTER_GETIDTOIPTABLE       0x237              /*���Ļ�õ�ַ��*/
#define ROOM_ACK_GETIDTOIP          0x0111             /*�����Ӧ��÷ֻ���ַ��*/

#define CENTER_SEND_RESET           0x0270             /*�������÷ֻ�*/
#define CENTER_BUSY			        0x0290             /*���ķ�æ*/
#define CENTER_IDLE			        0x0291             /*���Ŀ���*/
#define CENTER_HOLDON		        0x0292             /*����ժ��*/
#define CENTER_HANGUP		        0x0293             /*���Ĺһ�*/
#define CENTER_UNLOAK		        0x0294
#define CENTER_WARNACK              0x0295             /*����Ӧ�𱨾�*/
//#define CENTER_RELOGIN              0x0296

#define CENTER_AGENT                0x0296             /*�����й�*/
#define CENTER_MINOR                0x0297             /*�����������û�*/
#define CENTER_DIVERT_CENTER        0x0298             /*����ת�ӵ������*/
#define CENTER_DIVERT_ROOM          0x0299             /*����ת�ӵ��ֻ�*/

#define CENTER_ASKFOR_SYSINFO_ACK   0x0242              /*���Ļ�Ӧ����������Ϣ*/
#define CENTER_GET_SYSINFO          0x0244              /*���Ĳ�ѯ���ϵͳ��Ϣ*/
#define CENTER_SEND_SYSINFO         0x0243              /*���ĸ���ϵͳ��Ϣ*/
#define CENTER_FACTORY_SETTING      0x0245              /*��������IP�����豸*/

#define CENTER_NOPERSON		              0x02a5
#define CENTER_OVERTIMETALK	            0x20a6

#define CENTER_BROADCASTDATA            0x231            /*���Ĺ㲥���ֹ���*/
#define CENTER_BROADCASTPIC             0x232            /*���Ĺ㲥ͼƬ����*/

#define CENTER_UPGRADEROOM              0x236            /*���������ֻ����*/
#define DEV_UPGRADE_ACK                 0x1180           /*��������Ӧ��ָ��*/

#define BLTN_SETUP                      0x2A0            /*�����·�����ͼƬ*/

#define CENTER_IS_DEVONLINE             0x02F0          /*�������߼���豸*/
#define DEVONLINE_ACK                   0X1181           /*�豸��Ӧ*/

//�Ҿ������ļҾ�Э��
#define CENTER_GETALLSTATUS             0x0210           /*���Ĳ�ѯ���еļҾ�״̬*/
#define CENTER_SETELEC                  0x0211           /*���Ŀ��ƼҾ�*/
#define CENTER_GETELEC                  0x0212           /*���Ĳ�ѯĳ���Ҿӵ�״̬*/
#define CENTER_SETSCENE                 0x0213           /*���������龰ģʽ*/


/*2009-7-29  new  add cmd*/
#define CENTER_REMOTE_SINGLE_GUARD          0x0216       /*����Զ�̵������Ʒ���*/
#define CENTER_GET_ELEC_CONFIG              0x0217       /*���Ļ�üҾ����ñ�*/
#define CENTER_SET_ELEC_CONFIG              0X0218       /*�������üҾ����ñ�*/

/*----------------------����Զ�̿��ư���ָ��---------------------------------*/
#define CENTER_ARRANGE_GUARD                0X256           /*����Զ�̿��Ʒ���*/
#define CENTER_CHECK_GUARD                  0X257           /*���Ļ�÷�������*/
#define CENTER_RESET_ROOM_PASS              0X250           /*�������÷ֻ�����*/
#define ROOM_RESET_PASS_ACK                 0XA01           /*�ֻ�Ӧ����������*/
#define ROOM_CENTER_ARRANGE_GUARD_ACK       0XA02           /*�ֻ�Ӧ��Զ�̿��Ʒ���*/
#define ROOM_CENTER_CHECK_GUARD_ACK         0XA04           /*�ֻ�Ӧ�����Ļ�÷���״̬*/

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
msg_buf[0]:������
msg_buf[1]:���������Ƿ�������ݣ������ʾ�������ݳ���
msg_buf[2]:���ݣ�������ݴ���4���ֽڣ����ʾһ��ָ�롣
msg_buf[3]��ָ�򷿼�š�
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
                WRT_WARNING("����socket failed");              
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
                        WRT_MESSAGE("���ӵ��ӷֻ��ɹ�0x%x:%d\n",ipaddr,port);
                        return tmp_sock;
                }  
        }else{
                WRT_MESSAGE("���ӵ��ӷֻ�ʧ��%x",errno);
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
                                    printf("xxxxxxxxxxxxxx�������ݵ��ӷֻ����� %x --%d\n",errno,fd);
                                }else
                                    printf("xxxxxxxxxxxxxx�������ݵ��ӷֻ��ɹ�%d�ֽ� --%d\n",ret,fd);
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
							printf("xxxxxxxxxxxxxx�յ����� %d����\n",irecvlen);
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
	//�жϸ�IP�Ƿ����
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

        int delay = 30000; //30��
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

				//	printf("xxxxxxxxxxxxxx���� �ӷֻ� 0x%x\n",t_msg[0]);
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
						printf("xxxxxxxxxxxxxx������Ϣ%x�����еĴӷֻ�\n",b_msg[0]);
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
						printf("ת����������ý��Э��ת�����ӻ�\n");
						
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
										g_subroom_callback(b_msg); //�ӻ�����
										break;
									case  ROOM_BUSY:
										b_msg[0] = SUBROOM_SESSION_END;
										b_msg[1] = ROOM_BUSY;
										b_msg[2] = index;
										b_msg[3] = 0;
										g_subroom_callback(b_msg); //�ӻ���æ
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
										//������Ϣ�һ����������ӻ�
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
										printf("���մӷֻ��ֻ��һ�ָ��\n");
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
							g_subroom_callback(b_msg) ;//���Ӵӻ�ʧ��	

							//���fd������ر�fd 2012-2-17 9:10:54 added
							closesocket(fd[index]);
							fd[index] = -1;

							printf("xxxxxxxxxxxxxx���ӱ��Է��ж� %x:%d \n",gp_subroom_ip[index],index);

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

