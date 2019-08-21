/*-----------------------------------------------------------
*  Copyright (C) 2008 WRT co,ltd,
*  All Rights Reserved.
*
*
*----------------------------------------------------------*/
//
#ifndef ARM_LINUX //by ljw modified 2011-1-14 9:42:45
#include <tmStdLib.h>
#include <tmNxTypes.h>
#include <tmDbg.h>
#include <tmosal.h>
#include <stdio.h>
#include <tmMain.h>
#include <psos.h>
#include <pna.h>
#include <stdlib.h>
#include "warningtask.h"
#include "enet.h"

#include "tmDefenceInfo.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "warningtask.h"
#include "enet/enet.h"
#include "zenmalloc.h"
#include <SDL.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <time.h>
#endif

enum{
	WARNING_FIRE = 0,
	WARNING_GAS,
	WARNING_INFRARED,
	WARNING_DOORSENSOR,
	WARNING_WINDOWSENSOR,
	WARNING_FIRSTDOOR,
	WARNING_PARLOURROOM,
	WARNING_DININGROOM,
	WARNING_BEDROOMMAIN,
	WARNING_BEDROOMBRANCH,
	WARNING_STUDY,
	WARNING_TOILET,
	WARNING_BATHROOM,
	WARNING_KITCHEN,
	WARNING_URGENT,
	WARNING_UNDEFINE,
};

enum{
	WARNING_ALLTIME=14,
	WARNING_COMMON,
	WARNING_REALTIME,
	WARNING_DELAY,
};

#define  CMD_ID(cmdid) ((cmdid>>8)|((cmdid&0x00FF)<<8))

#define ROOM_SEND_WARNING          CMD_ID(0X1000)
#define ROOM_HANDLE_WARNING_ACK    CMD_ID(0X1001)
#define ROOM_CLEAR_WARNING         CMD_ID(0X1002)
#define SEND_WARNING_ACK           CMD_ID(0X0380)
#define HANDLE_WARNING             CMD_ID(0X0381)
#define CLEAR_WARNING_ACK          CMD_ID(0X0382)
#define CENTER_AGENT               CMD_ID(0x0296)    //�����й�

#pragma pack(1)

#define HEAD_LENGTH	             (4/*Э��ͷWRTI*/+2/*ָ��*/+4/*ָ���*/)   //ָ��ͷ����
#define RECVBUF                  512                        //���ܻ����� ����Ϊ514��ԭ�� cpu���룬����2���ֽ�
#define DATA_SIZE                (RECVBUF-HEAD_LENGTH)     //���ݴ�С



#define DEFAULT_MAX_WARNING_NUM  200
#define DEFAULT_MAX_FAIL_NUM     2          //�����Ĺ���� �ط�2��



#define MAX_ROOMNO_SIZE          15


struct _net_pack_
{
        char head[4];//WRTI    
        int len;
        unsigned short uCmd;
        char SrcNo[MAX_ROOMNO_SIZE];
        char DstNo[MAX_ROOMNO_SIZE];
        char Data[DATA_SIZE];
};


#pragma pack( )

typedef struct warning_msg_def
{
        char RoomNo[16];    //�����
        int  WarningID;     //����
        int  WarningNO;     //�������
        char FailTimes[6];  //����ʧ�ܴ���
        char IPAck[6];      //�ر��
        unsigned char standbyflag;       //���ñ����豸��־  0-������  1-����
        unsigned char delflag;       //ɾ����־
        struct warning_msg_def *prev;
        struct warning_msg_def *next;

}WARNING_MSG, *PWARNING_MSG;



////��Ϣ���нṹ
struct msgbuf 
{
      long mtype;
        long msg[4];
        char mdata[256];
};




#define SEND_WARNINGMSG 0
#define SEND_HANDLEMSG  1
#define RESEND_WARNINGMSG 2

unsigned long IP[6];        //    0  -���ĵ�ַ  1�������ַ 2������������ַ 5�йܵ�ַ 3 �������ĵ�ַ 4���ñ�����������ַ  
struct in_addr addr;
static SDL_mutex  	*g_pmutex = NULL;
static SDL_sem 		*g_psem_t = NULL;
static int        g_exit = 0;


#define WARNING_NULL          0   //����δʹ��
#define WARNING_NACK          1   //���ͱ��� δ��Ӧ״̬
#define WARNING_ACK           2
#define WARNING_NACK_TIMEOUT  3   //������ʱ�����ñ��û�
#define WARNING_NO_ONLINE     4   //������ʱ�����ñ��û�


int  CheckStatusTable();
static unsigned short checksum(unsigned short *buffer, int size);

void RemoveSameIP();
static int g_warning_msg_count;
WARNING_MSG  *g_warning_msg_head = NULL;
WARNING_MSG  *g_warning_msg_tail = NULL;

WARNING_MSG *pWarningMsg = NULL;


static  AppENetCallbacks enet_callbacks;
static   char buf1[1024];
static void RecvCallback(unsigned long ip,short port ,void* buf,int len);
WARNING_MSG * search_warning_msg_by_warningNO(int WarningID,int WarningNO);




static void RecvCallback(unsigned long ip,short port ,void* buf,int len){

        unsigned long msg[4]; 
        struct _net_pack_  warning_np;              
        int WarningNO,WarningID;
        unsigned long from_ip;   
        char temp1[16];
        char temp2[16];
        unsigned long center_agent_ip;
        int i=0;

        memset(buf1,0,len+1);
        memcpy(buf1,buf,len);
        //printf("\nrecv:%x,%d,%d,%s\n",ip,port,len,buf1);

        memcpy((char *)&warning_np, buf1, 200);    //from net udp 

        if(strncmp(warning_np.head,"WRTI",4)==0)
                ;//printf("\nRecvCallback udp head: %s ,len: %d,CMD: 0x%04x\n",warning_np.head,warning_np.len,warning_np.uCmd);
        else
                return;     

        from_ip =ip;
        //printf("\n  q_receive head: %s ,len: %d,CMD: 0x%04x\n",warning_np.head,warning_np.len,warning_np.uCmd); 

        switch(warning_np.uCmd)
        {       
	        case SEND_WARNING_ACK:
	                memcpy((char *)&WarningNO,(char *)&warning_np.Data[4],4);
	                memcpy((char *)&WarningID,(char *)&warning_np.Data,4);           	    

	                pWarningMsg =search_warning_msg_by_warningNO(WarningID,WarningNO);
	                if(pWarningMsg == NULL)
	                	break;

	                for(i=0;i<6;i++)
	                {

	                        if(from_ip == IP[i])      //���Ӧ��IP��״̬λ
	                        {
	                                pWarningMsg->IPAck[i]= WARNING_ACK;

	                                //printf("\n������Ӧ from_ip =%x   pWarningMsg->WarningNO=%d",from_ip,pWarningMsg->WarningNO); 
	                        }
	                }
	                break;

	        case CENTER_AGENT:
	                memcpy((char *)&center_agent_ip,(char *)&warning_np.Data,4);

	                IP[3] =center_agent_ip;
	                for(i=0;i<6;i++)
	                {
	                        if(i==3)
	                                continue;

	                        if(IP[i]==center_agent_ip)
	                                IP[5]=0;    
	                }	
	                break;

	        case HANDLE_WARNING:

	                memcpy((char *)&WarningNO,(char *)&warning_np.Data[4],4);
	                memcpy((char *)&WarningID,(char *)warning_np.Data,4);

	                memcpy((char *)temp1,(char *)&warning_np.SrcNo,15);
	                memcpy((char *)temp2,(char *)&warning_np.DstNo,15);

	                memcpy((char *)&warning_np.SrcNo,(char *)temp2,15);
	                memcpy((char *)&warning_np.DstNo,(char *)temp1,15);

	                warning_np.uCmd =ROOM_HANDLE_WARNING_ACK;          
	                memcpy(&addr, &from_ip, 4);                              
	                App_Enet_Send((char *)inet_ntoa(addr),0,(char *)&warning_np,warning_np.len);            

	                warning_np.uCmd =ROOM_CLEAR_WARNING;  //(from_ip) �����˱��� ��������Ӧ�������	           			             				             			             		

	                for(i=0;i<6;i++)
	                {  
	                        if(IP[i]==0)
	                                continue;

	                        if(from_ip ==IP[i])
	                                continue;



	                        memcpy(&addr, &IP[i], 4);                              
	                        App_Enet_Send((char *)inet_ntoa(addr),0,(char *)&warning_np,warning_np.len);             	                                          
	                }	  	
	                warning_msg_del(WarningID,WarningNO);    
	                break;	

	        case CLEAR_WARNING_ACK:
				
	                break;	

	        default:
	                //printf("\n default Recv Callback\n");
	                break;
        }		


}

//�������г�ʼ��
int warning_msg_init()
{
        while(g_warning_msg_count > 0)
        {
                warning_msg_del_tail();
        }
        g_warning_msg_count = 0;
        g_warning_msg_head = g_warning_msg_tail = NULL;

        return 0;
}

int warning_msg_del_tail(void)
{
        WARNING_MSG  *p;
        if(g_pmutex != NULL)
                SDL_LockMutex(g_pmutex);


        if(g_warning_msg_tail == NULL) {
                if(g_pmutex != NULL)
                        SDL_UnlockMutex(g_pmutex);
                return 0;
        }


        p = g_warning_msg_tail;

        g_warning_msg_tail = g_warning_msg_tail->prev;
        g_warning_msg_tail->next = NULL;
        p->prev = NULL;
        ZENFREE(p);
        g_warning_msg_count --;
        if(g_pmutex != NULL)
		SDL_UnlockMutex(g_pmutex);
        return 0;
}

int warning_msg_del(int warningID,int warningNO)
{
        WARNING_MSG* p;

        if(g_pmutex != NULL)
                SDL_LockMutex(g_pmutex);
        for(p = g_warning_msg_head; p !=NULL; p = p->next)
        {
                if((p->WarningID == warningID)&&(p->WarningNO == warningNO))
                {
                       // printf("\nwarning_msg_del p->WarningNO=%d",p->WarningNO);
                        if(p == g_warning_msg_head)
                        {
                                g_warning_msg_head = g_warning_msg_head->next;
                        }
                        else
                        {
                                p->prev->next = p->next;
                        }
                        if(p == g_warning_msg_tail)
                                g_warning_msg_tail = p->prev;
                        if(p->next)
                                p->next->prev = p->prev;

                        ZENFREE(p);
                        g_warning_msg_count --;

                        break;
                }
        }
        if(g_pmutex != NULL)
		SDL_UnlockMutex(g_pmutex);        
        return 0;
}

int warning_msg_add(int warningID,int warningNO,char *roomID)
{
        WARNING_MSG   *p;
        int i;
        if(g_pmutex != NULL)
                SDL_LockMutex(g_pmutex);
	
        if(g_warning_msg_count >= DEFAULT_MAX_WARNING_NUM)
        {
                if(warning_msg_del_tail()) {
	        		if(g_pmutex != NULL)
						SDL_UnlockMutex(g_pmutex);                 	
                	return -4;
                }
        }

        p = (WARNING_MSG*)ZENMALLOC(sizeof(WARNING_MSG));

        if(p == NULL){ 
			if(g_pmutex != NULL)
				SDL_UnlockMutex(g_pmutex);            	
	                return -5;
        }

        p->WarningID = warningID;
        p->WarningNO = warningNO;	
        strncpy(p->RoomNo,roomID,15);
        p->delflag=0;

        for(i=0;i<6;i++)
        {
			if(IP[i] != 0 || IP[i] != 0xFFFFFFFF)
				p->IPAck[i] = WARNING_NACK;
			else
				p->IPAck[i] = WARNING_NULL;
			
			p->FailTimes[i] = 0;
        }

        p->standbyflag=0;
        p->delflag=0;

        if(g_warning_msg_head == NULL)
        {
                p->prev = p->next = NULL;
                g_warning_msg_head = g_warning_msg_tail= p;
        }
        else
        {
                p->next = g_warning_msg_head;
                g_warning_msg_head->prev = p;
                g_warning_msg_head = p;
        }
		
        g_warning_msg_count ++;

		if(g_pmutex != NULL)
			SDL_UnlockMutex(g_pmutex);    

		if(g_psem_t)
			SDL_SemPost(g_psem_t);
		
        //printf(" warning_msg_add   %d %d\n",p->WarningID,p->WarningNO);
        return 0;
}

WARNING_MSG * search_warning_msg_by_warningNO(int WarningID,int WarningNO)
{
        WARNING_MSG *p;
        if(g_pmutex != NULL)
                SDL_LockMutex(g_pmutex);
        if(g_warning_msg_head == NULL) 
        {
			if(g_pmutex != NULL)
				SDL_UnlockMutex(g_pmutex);     
            //printf("\n  return NULL start");
            return NULL;

        }        
        p = g_warning_msg_head;
        while(p != NULL)
        {

                //printf("\n p->WarningID=%d  p->WarningNO=%d WarningID=%d  WarningNO=%d",p->WarningID,p->WarningNO,WarningID,WarningNO);


                if((p->WarningID==WarningID)&&(p->WarningNO==WarningNO))
                {

                    //printf("\n  return p");
					if(g_pmutex != NULL)
						SDL_UnlockMutex(g_pmutex);                          
	                        return p;
                }
                p = p->next;

        }
		if(g_pmutex != NULL)
			SDL_UnlockMutex(g_pmutex);  
        //printf("\n  return NULL end");
        return NULL;  
}

// IP���к����ظ�IP����Ϊ0
void RemoveSameIP()
{
        int i=0,j=0;	

        for(i=0;i<5;i++)
        {
                for(j=1+i;j<6;j++)
                {
                        if( IP[i]==IP[j])       //�����ظ� IP==0
                        {
                                IP[j]=0;
                        }
                }
        }
}


///////////////////////////////////////////////////////////////////////
//int WarningInit(int reflag,unsigned long centerip,unsigned long managerip,unsigned long warningip,unsigned long centerminorip,unsigned long warningminorip)
//int      reflag              0Ϊ��ʼ������  1�޸�����
//unsigned long centerip       ���ĵ�ַ
//unsigned long managerip      �������ַ
//unsigned long warningip      ������������ַ
//unsigned long centerminorip  �������ĵ�ַ
//unsigned long warningminorip ���ñ�����������ַ
//return  0        ������                   
//////////////////////////////////////////////////////////////////////
int WarningInit(int reflag,unsigned long centerip,unsigned long managerip,unsigned long warningip,unsigned long centerminorip,unsigned long warningminorip)
{
        int i,k;
        int iRet;
        int ret;
        pthread_attr_t attr;
        key_t msgkey;   

        if(reflag==0 )
        {
        	g_exit = 0;
            g_pmutex=SDL_CreateMutex();
            g_psem_t = SDL_CreateSemaphore(0);
            SDL_CreateThread(WarningSend,NULL);

        }
		
        IP[0] = centerip;
        IP[1] = managerip;
        IP[2] = warningip;
        IP[3] = centerminorip;
        IP[4] = warningminorip;
        IP[5] = 0;               //�й�IP

        RemoveSameIP();

        if(reflag == 2)
        	return 0;
		
        warning_msg_init();

        AppENetCallbacks init;
#ifdef ARM_LINUX //by ljw modifyed 2011-1-14 9:48:49
        init.AppRecvCallback = RecvCallback;
        init.AppRecvCallback2 = NULL;
        App_InitENet(0,0,&init);
        App_SetCallBack2(&init);
#else 
        init.AppRecvCallback = RecvCallback;
        init.AppRecvCallback2 = NULL;

        App_InitENet_For_Send(0);
        App_InitENet_For_Recv(0);

        App_SetCallBack2(&init);
#endif
        return 0;
}

void WarningUninit()
{
	g_exit = 1;
	SDL_SemPost(g_psem_t);
}

int WRTD_get_warning_area_by_type(int WarningID)
{
    return WARNING_FIRE;
}

int WarningSend(void* pvoid)
{
        unsigned long msg[4];
        int i,ret;
        struct _net_pack_  warning_np;
        int WarningNO,WarningID;
        WARNING_MSG* p;
		int send_flag = 0;
		unsigned long m_ip[4];
		char str[10];
		
		int test_sss = 1;
        while(!g_exit)
        {
                //sleep(1);  
				SDL_SemWait(g_psem_t);
                if( g_warning_msg_count > 0 )
                {
                        for(p = g_warning_msg_head; p !=NULL; p = p->next)
                        {
                                if(p->delflag == 1)
                                {
                                        warning_msg_del(p->WarningID,p->WarningNO);
                                        continue;
                                }         	
								
                                if(g_pmutex != NULL)
                					SDL_LockMutex(g_pmutex);
								
                                memcpy((char*)&warning_np.head,"WRTI",4);
								
								if(p->WarningID != 0 && p->WarningID != 100)//����ʹ��
								{
									//printf("64 64 64 64  \n");
	                                warning_np.len = 48+16;
	                                warning_np.uCmd = ROOM_SEND_WARNING;
	                                memcpy((char*)&warning_np.SrcNo,(char *)p->RoomNo,MAX_ROOMNO_SIZE);		
	                                memcpy((char*)&warning_np.DstNo,(char *)"000000000000000",MAX_ROOMNO_SIZE);	
									memset((char *)&warning_np.Data,0,DATA_SIZE);
	                                memcpy((char *)&warning_np.Data,(char *)&p->WarningID,4);
	                                memcpy((char *)&warning_np.Data[4],(char *)&p->WarningNO,4);
									//printf("id = %d  area = %d\n",p->WarningID,WRTD_get_warning_area_by_type(p->WarningID));
									switch(WRTD_get_warning_area_by_type(p->WarningID))
									{
										case WARNING_FIRE:
												memcpy((char *)&warning_np.Data[8],"�̸�",strlen("�̸�"));
								                break;
								        case WARNING_INFRARED:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
								        case WARNING_GAS:
												memcpy((char *)&warning_np.Data[8],"��˹",strlen("��˹"));
								                break;
								        case WARNING_DOORSENSOR:
												memcpy((char *)&warning_np.Data[8],"�Ŵ�",strlen("�Ŵ�"));
												break;
								        case WARNING_WINDOWSENSOR:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
								        case WARNING_FIRSTDOOR:
												memcpy((char *)&warning_np.Data[8],"������",strlen("������"));
								                break;
								        case WARNING_PARLOURROOM:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
								        case WARNING_DININGROOM:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
								        case WARNING_BEDROOMMAIN:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
								        case WARNING_BEDROOMBRANCH:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
								        case WARNING_STUDY:
												memcpy((char *)&warning_np.Data[8],"�鷿",strlen("�鷿"));
								                break;
								        case WARNING_TOILET:
												memcpy((char *)&warning_np.Data[8],"������",strlen("������"));
								                break;
								        case WARNING_BATHROOM:
												memcpy((char *)&warning_np.Data[8],"ԡ��",strlen("ԡ��"));
								                break;
								        case WARNING_KITCHEN:
												memcpy((char *)&warning_np.Data[8],"����",strlen("����"));
								                break;
										case WARNING_URGENT:
												memcpy((char *)&warning_np.Data[8],"��������",strlen("��������"));
								                break;
										case WARNING_UNDEFINE:
												memset(str,0,10);
												sprintf(str,"%d�ŷ���",p->WarningID);
												memcpy((char *)&warning_np.Data[8],str,strlen(str));
												break;
								                
									}
									
									
								}else
								{
									//printf("48 48 48 48 \n");
									
									warning_np.len = 48;
	                                warning_np.uCmd = ROOM_SEND_WARNING;
	                                memcpy((char*)&warning_np.SrcNo,(char *)p->RoomNo,MAX_ROOMNO_SIZE);		
	                                memcpy((char*)&warning_np.DstNo,(char *)"000000000000000",MAX_ROOMNO_SIZE);		
	                                memcpy((char *)&warning_np.Data,(char *)&p->WarningID,4);
	                                memcpy((char *)&warning_np.Data[4],(char *)&p->WarningNO,4);
								}
								
                                if((p->FailTimes[0]++ > 1) || (p->FailTimes[1]++ > 1))
                                {
                                        p->standbyflag=1;//���ñ����豸
                                }
								
								m_ip[0] = m_ip[1] = m_ip[2] = m_ip[3] = 0;

                                if(p->standbyflag == 0)//���ġ������������������
                                {
                                        for(i=0;i<4;i++)
                                        {
                                                if(p->FailTimes[i]++ > 1)
                                                {
                                                    p->standbyflag = 1;
                                                    continue;
                                                }

                                                if(IP[i] == 0 || IP[i] == 0xFFFFFFFF)
                                                    continue;

                                                if((p->IPAck[i] == WARNING_NACK) || (p->IPAck[i] == WARNING_NULL))
                                                {            
                                                    send_flag = 1;//need send
													m_ip[i] = IP[i];
                                                }
                                        }
                                }
                                else
                                {	                  		
                                        for(i=4;i<6;i++)
                                        {

                                                if(p->FailTimes[i]++ > 1)
                                                {
                                                        p->delflag=1;
                                                        continue;
                                                }
                                                if(IP[i]==0 || IP[i] == 0xFFFFFFFF)
                                                        continue;

                                                if((p->IPAck[i]== WARNING_NACK)||(p->IPAck[i]== WARNING_NULL))
                                                {            
                                                        send_flag = 2;
														m_ip[i-4] = IP[i];
                                                }            
                                        }

                                }  
								
	                        if(g_pmutex != NULL)
	                			SDL_UnlockMutex(g_pmutex);
							
	                		if(send_flag == 2)
                			{
                				for(i=0;i<2;i++)
            					{
                                    if(m_ip[i]==0 || m_ip[i] == 0xFFFFFFFF)
                                            continue;
									
                                    memcpy(&addr, &m_ip[i], 4);
                                    App_Enet_Send((char *)inet_ntoa(addr),0,(char *)&warning_np,warning_np.len);  
                                    //printf("BBBBB5-6 send ip = %x\n",m_ip[i]);  

            					}
                			}else if(send_flag == 1)
            				{
            					for(i=0;i<4;i++)
            					{
                                    if(m_ip[i]==0 || m_ip[i] == 0xFFFFFFFF)
                                            continue;
          
	                                memcpy(&addr, &m_ip[i], 4);                              
	                                App_Enet_Send((char *)inet_ntoa(addr),0,(char *)&warning_np,warning_np.len);  
	                                //printf("AAAAA1-4 send ip = %x\n",m_ip[i]); 
									 
            					}
            				}
							
							send_flag = 0;
							
                        }
                }	     
        }//for end
        
        warning_msg_init(); //���msglist
		SDL_DestroySemaphore(g_psem_t);
		g_psem_t = NULL;
		SDL_DestroyMutex(g_pmutex);
		g_pmutex = NULL;
	
}

#if 0
void WarningRecv()
{
        unsigned long msg[4]; 
        int i=0,k=0,ret;
        int WarningNO,WarningID;
        unsigned long from_ip;   
        char temp1[16];
        char temp2[16];
        unsigned long center_agent_ip;
        struct _net_pack_  warning_np;    



        for(;;)
        {
                // ret=q_receive(qid_warningrcv, Q_WAIT, 0, msg);
                //    if(ret==1)
                //     	   {
                //     	   	CheckStatusTable();
                //    	   	IP[5]=0;
                //    	   	continue;     	   	
                //    	   	}


                memcpy((char *)&warning_np,(char *) msg[0], 200);
                from_ip =msg[1];

                printf("\n  q_receive head: %s ,len: %d,CMD: 0x%04x\n",warning_np.head,warning_np.len,warning_np.uCmd); 

                switch(warning_np.uCmd)
                {       
                case SEND_WARNING_ACK:
                        memcpy((char *)&WarningNO,(char *)&warning_np.Data[4],4);
                        memcpy((char *)&WarningID,(char *)&warning_np.Data,4);           	    

                        pWarningMsg =search_warning_msg_by_warningNO(WarningID,WarningNO);

                        for(i=0;i<6;i++)
                        {

                                if(from_ip ==IP[i])      //���Ӧ��IP��״̬λ
                                {
                                        pWarningMsg->IPAck[i]= WARNING_ACK;

                                        printf("\n������Ӧ from_ip =%x   pWarningMsg->WarningNO=%d",from_ip,pWarningMsg->WarningNO); 
                                }
                        }
                        break;

                case CENTER_AGENT:
                        //	     printf("\n�����й� IP = center_agent_ip");
                        memcpy((char *)&center_agent_ip,(char *)&warning_np.Data,4);

                        IP[5] =center_agent_ip;

                        for(i=0;i<5;i++)
                        {
                                if(IP[i]==center_agent_ip)
                                        IP[5]=0;    
                        }	
                        break;

                case HANDLE_WARNING:

                        memcpy((char *)&WarningNO,(char *)&warning_np.Data[4],4);
                        memcpy((char *)&WarningID,(char *)warning_np.Data,4);

                        memcpy((char *)temp1,(char *)&warning_np.SrcNo,15);
                        memcpy((char *)temp2,(char *)&warning_np.DstNo,15);

                        memcpy((char *)&warning_np.SrcNo,(char *)temp2,15);
                        memcpy((char *)&warning_np.DstNo,(char *)temp1,15);

                        warning_np.uCmd =ROOM_HANDLE_WARNING_ACK;
                        msg[0] = (unsigned long)&warning_np;   	        
                        msg[1] = from_ip;
                        msg[2] = SEND_HANDLEMSG;
                        //    q_send(qid_warning,msg); 

                        warning_np.uCmd =ROOM_CLEAR_WARNING;  //(from_ip) �����˱��� ��������Ӧ�������	           			             				             			             		

                        for(i=0;i<6;i++)
                        {  
                                if(IP[i]==0)
                                        continue;

                                if(from_ip ==IP[i])
                                        continue;


                                //     if(hxf_ping1Ip(IP[i])>0)
                                {		
                                        msg[0] = (unsigned long)&warning_np;   	        
                                        msg[1] = IP[i];
                                        msg[2] = SEND_HANDLEMSG;
                                        //        q_send(qid_warning,msg);

                                }      	                                          
                        }	  	
                        warning_msg_del(WarningID,WarningNO);    
                        break;	

                case CLEAR_WARNING_ACK:

                        break;	

                default:
                        printf("\n default Recv Callback\n");
                        break;
                }	

        }

}
#endif






///////////////////////////////////////////////////////////////////////
//int SendWarning(char *RoomNO,int WarningID)
//char *RoomNO    ���������
//int WarningID   ��������
//return          0 -�����ɹ�  1 -������������������ʧ��  2- 3����ͬ����
//////////////////////////////////////////////////////////////////////
int SendWarning(char *RoomNO,int WarningID)
{
        unsigned long WarningNO =0; 
        time_t tp;
        int ret=0,i=0;
        struct _net_pack_  warning_np;
        WARNING_MSG* p;
		time(&tp);

        WarningNO =tp;
        printf("\nSendWarning() WarningNO=%d  WarningID==%d\n",WarningNO,WarningID);
        if(g_warning_msg_count > DEFAULT_MAX_WARNING_NUM)
                return 1;
#if 0	 	

        SameWarningIDindex=CheckSameWarningID(RoomNO,WarningID,WarningNO);

        if(SameWarningIDindex>-1)
	    {	 	   
		    printf("\n time =%d ",abs(SysTickCount -t_warning.t_warning_info[SameWarningIDindex].WarningNO));

	        if(abs(SysTickCount -t_warning.t_warning_info[SameWarningIDindex].WarningNO)<5000)   
	        {
	                printf("\n��ͬ�ı���5�����ڲ��ظ�����");
	                return 2;
	        }
        }
#endif
        warning_msg_add(WarningID,WarningNO,RoomNO)   ;    //������Ϣ�������

        return 0;
		
}




