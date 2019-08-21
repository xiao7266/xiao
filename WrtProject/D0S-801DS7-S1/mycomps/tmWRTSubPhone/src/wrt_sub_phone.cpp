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
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        rc = select(max_fd+1,NULL,&writefd,NULL,&timeout);
        if(rc > 0){      
                if(FD_ISSET(tmp_sock,&writefd)){
                        WRT_DEBUG((const char*)"���ӵ��ֻ��ɹ�0x%x:%d",ipaddr,port);
                        return tmp_sock;
                }  
        }else{
                WRT_DEBUG((const char*)"���ӵ��ֻ�ʧ��%x",errno);
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
                                        WRT_DEBUG("�������ݵ��ֻ����� %x",errno);
                                }else
                                        WRT_DEBUG("�������ݵ��ֻ��ɹ�%d�ֽ�",ret);
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
	//�жϸ�IP�Ƿ���ڣ�
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
		WRT_DEBUG("���ֻ����Ѵ��ڻ��壬���ø��ֻ��ŵĻ���");
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
        int delay = 30000; //30��
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
							WRT_DEBUG("δ֪ԭ�������ж�1- %d",index);
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
							WRT_DEBUG("δ֪ԭ�������ж�2- %d",index);
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
								WRT_DEBUG("����������δ֪ԭ�� close�Ժ󣬳����ٴ�����");
								keeplive[index] = 0;
								fd[index] = connect_to_phone(g_phone_ip[index].phone_ip);
								if(fd[index] == -1)
								{
									WRT_DEBUG("���ӵ��ֻ�ʧ�� 0x%x",g_phone_ip[index].phone_ip);
									b_msg[0] = PHONE_SESSION_END;
									b_msg[1] = 0x0000FFFF;
									b_msg[2] = b_msg[3] = 0;
									g_phone_callback(b_msg) ;//�����ֻ�ʧ��
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
                        WRT_DEBUG("���� phone xxxxx");
                        for(index = 0; index < MAX_PHONE;index++){
                        	if(fd[index] == -1 && g_phone_ip[index].phone_ip != 0 &&  g_phone_ip[index].phone_ip != 0xffffffff){
                                	fd[index] = connect_to_phone(g_phone_ip[index].phone_ip);
                        	}
                        	if(fd[index] == -1 )
                        	{
                        		if(g_phone_ip[index].phone_ip != 0 &&  g_phone_ip[index].phone_ip != 0xffffffff)
                                		WRT_DEBUG("���ӵ��ֻ�ʧ�� 0x%x��%d",g_phone_ip[index].phone_ip,index);
                                	/*
			                               		 b_msg[0] = PHONE_SESSION_END;
			                               		 b_msg[1] = 0x0000FFFF;
			                                	 b_msg[2] = b_msg[3] = 0;
			                                	g_phone_callback(b_msg) ;//�����ֻ�ʧ��
			                                	iscontinue = 1;
			                               		break;
			                               		*/
                        	}else{
                        		WRT_DEBUG("����%x:%d:%d ",g_phone_ip[index].phone_ip,fd[index],index);
                        	}
                        	
                	}
                        
                        //�������ĺ��е���Ϣ���ֻ�
                        if(t_msg[0] == DOOR_CALL_PHONE){
                                b_msg[0] = DOOR_CALLROOM;
                                curstate = 1;
                                //ͬʱ������Ƶ����...
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
                            		WRT_DEBUG("���ӵ��ֻ�ʧ�� 0x%x��%d",g_phone_ip[index].phone_ip,index);
                    	}else{
                    		WRT_DEBUG("����ý��ָ��%x:%d:%d ",g_phone_ip[index].phone_ip,fd[index],index);
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
                	WRT_DEBUG("���������ֻ�IP:%x phonenumber%d\n",t_msg[1],t_msg[2]);
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
                        		WRT_DEBUG("�ֻ����岻�������֧��4̨�豸");
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
                        WRT_DEBUG("��������:%x fd =%d ok \n",t_msg[1],fd[index]);
			if(fd[index] == -1)
                        {
                                WRT_DEBUG("�������ӵ��ֻ�ʧ�� 0x%x��%d",t_msg[1],t_msg[0]);
                                b_msg[0] = PHONE_SESSION_END;
                                b_msg[1] = 0x0000FFFF;
                                b_msg[2] = b_msg[3] = 0;
                                g_phone_callback(b_msg) ;//�����ֻ�ʧ��
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
                      //  ret = q_receive(qid_phone,Q_WAIT,100,t_msg); //t_msg[3] ��Ҫ�ͷŵ�(id).
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
                                        WRT_DEBUG((char*)"������Ϣ0x%x���ֻ�",b_msg[0]);
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
                        		                        g_phone_callback(b_msg); //�ֻ���æ
                        		                        break;
                        		                case  ROOM_BUSY:
                        		                        b_msg[0] = PHONE_SESSION_END;
                        		                        b_msg[1] = ROOM_BUSY;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg); //�ֻ���æ
                        		                        break;
                        		                case ROOM_HOLDON:
                        		                        b_msg[0] = PHONE_HOLDON;
                        		                        b_msg[1] = 1;
                        		                        b_msg[2] = index;
                        		                        b_msg[3] = 0;
                        		                        g_phone_callback(b_msg);
                        		                        //������Ϣ�һ����������ֻ�
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
                                		 	g_phone_callback(b_msg) ;//�����ֻ�ʧ��
                                		 	i_end = 1;
                                		}else{
                                		        b_msg[0] = PHONE_SESSION_END;
                               			 	b_msg[1] = 0xfd;
                                		 	b_msg[2] = index;
                                		 	b_msg[3] = 0;
                                		 	g_phone_callback(b_msg) ;//�����ֻ�ʧ��	
                                		}
                                		//���fd������ر�fd 2012-2-17 9:10:54 added
                                		closesocket(fd[index]);
                                		fd[index] = -1;
                                		WRT_DEBUG("���ӱ��Է��ж� %x:%d  ��·����count =%d\n",g_phone_ip[index].phone_ip,index,count);
                                		
                        		}
                        
                		}
                	}
                        if(i_end == 1)
                                break;
                }
          /* //2012-2-17 9:10:57 delete 
                WRT_MESSAGE("�ر��ֻ�����%d",fd);
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
