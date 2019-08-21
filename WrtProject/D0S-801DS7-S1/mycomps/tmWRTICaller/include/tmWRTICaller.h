/*
	  �ļ�����   tmWRTICaller.h
	    ���ܣ�   ����ip�豸���й���ģ��ʵ��
	    �汾��   Version 1.0
	�������ڣ�   2011-12-07
	    ��Ȩ��   
	    ���ߣ�   
	    �汾��   
*/

#ifndef _TM_WRTI_CALLER_H_
#define _TM_WRTI_CALLER_H_
#define PROTOCOL_VERSION2

#include "DoorStruct.h"
#include <DoorCmd.h>

#define MSGID_START_CALL_ROOM                            10000                          //��ʼ���зֻ�
#define MSGID_START_CALL_CENTER                          (MSGID_START_CALL_ROOM+1)      //��ʼ��������
#define MSGID_START_CALL_ANOTHRE_CENTER                  (MSGID_START_CALL_ROOM+2)      //��ʼ���б�������
#define MSGID_DEVICE_CONNECTING                          (MSGID_START_CALL_ROOM+3)      //��������Ŀ��...
#define MSGID_CONNECTED_ERROR                            (MSGID_START_CALL_ROOM+4)      //����Ŀ��ʧ��
#define MSGID_SENDING_CALL_REQUEST                       (MSGID_START_CALL_ROOM+5)      //���ڷ��ͺ�������
#define MSGID_SEND_CALL_REQUEST_ERROR                    (MSGID_START_CALL_ROOM+6)      //���ͺ�������ʧ��
#define MSGID_WAIT_CALL_REQUEST_ACK                      (MSGID_START_CALL_ROOM+7)      //�ȴ�Ŀ����Ӧ...
#define MSGID_CENTER_BUSY                                (MSGID_START_CALL_ROOM+8)      //���ķ�æ
#define MSGID_CENTER_AGENT_CONNECTING                    (MSGID_START_CALL_ROOM+9)      //�������й�,ת����...
#define MSGID_CENTER_MINOR_CONNECTING                    (MSGID_START_CALL_ROOM+10)     //���ı���,ת����...
#define MSGID_CENTER_DIVERT_TIMEOUT                      (MSGID_START_CALL_ROOM+11)     //����ת�Ӵ���̫��
#define MSGID_ROOM_QUIET_MODE                            (MSGID_START_CALL_ROOM+12)     //�������������
#define MSGID_ROOM_DIVERT_TIMEOUT                        (MSGID_START_CALL_ROOM+13)     //�ֻ�ת�Ӵ���̫��...
#define MSGID_ROOM_AGENT_CONNECTING                      (MSGID_START_CALL_ROOM+14)     //�������й�,ת����...
#define MSGID_ROOM_BUSY                                  (MSGID_START_CALL_ROOM+15)     //���䷱æ...
#define MSGID_NO_CALL_REQUEST_ACK                        (MSGID_START_CALL_ROOM+16)     //Ŀ����Ӧ��...
#define MSGID_CENTER_RINGON                              (MSGID_START_CALL_ROOM+17)     //"��������������,�һ���*��
#define MSGID_CENTER_DIVERT_CENTER                       (MSGID_START_CALL_ROOM+18)     //������ת��������!
#define MSGID_CENTER_DIVERT_ROOM                         (MSGID_START_CALL_ROOM+19)     //������ת��������...
#define MSGID_TALKING_TO_CENTER                          (MSGID_START_CALL_ROOM+20)     //������ͨ����,�һ���*��
#define MSGID_CENTER_OPEN_LOCK                           (MSGID_START_CALL_ROOM+21)     //�����ѿ���,�һ���*��
#define MSGID_ROOM_RINGON                                (MSGID_START_CALL_ROOM+22)     //�ֻ�������,�һ���*��
#define MSGID_ROOM_LEAVEWORD_NO_ACK                      (MSGID_START_CALL_ROOM+23)     //����ʧ�ܣ���Ӧ��
#define MSGID_ROOM_UNINIT_LEAVEWORD_INTERFACE            (MSGID_START_CALL_ROOM+24)     //δ��ʼ�����Խӿ�
#define MSGID_ROOM_DO_LEAVEWORD_NO_ACK                   (MSGID_START_CALL_ROOM+25)     //���зֻ�������,ֹͣ��*��
#define MSGID_ROOM_TALKING                               (MSGID_START_CALL_ROOM+26)     //��ֻ�ͨ����,�һ���*��
#define MSGID_ROOM_OPEN_LOCK                             (MSGID_START_CALL_ROOM+27)     //�ֻ��ѿ���,�һ���*��
#define MSGID_CMD_DATA_ERROR                             (MSGID_START_CALL_ROOM+28)     //�������ݴ���

//2008-11-28����  
#define MSGID_CMD_ROOM_HUNGUP                            (MSGID_START_CALL_ROOM+29)     //����ֻ��һ�
#define MSGID_CMD_ROOM_STOPTLEAVEWORDACK                 (MSGID_START_CALL_ROOM+30)     //����ֻ�ֹͣ����
#define MSGID_CMD_CENTER_HUNGUP                          (MSGID_START_CALL_ROOM+31)     //���Ĺһ�
#define MSGID_DEVICE_BUSY                                (MSGID_START_CALL_ROOM+32)     //������æ�����Ժ�����

//2008-12-09����
#define MSGID_CMD_ROOM_CALL_TIMEOVER                     (MSGID_START_CALL_ROOM+33)    //���зֻ�ͨ��ʱ�䵽�����������ã�һ��Ϊ2����
#define MSGID_CMD_ROOM_NO_HOLDON                         (MSGID_START_CALL_ROOM+34)    //�ֻ����˽���
#define MSGID_CMD_CENTER_CALL_TIMEOVER                   (MSGID_START_CALL_ROOM+35)    //��������ͨ��ʱ�䵽��ͬ�ֻ�
#define MSGID_CMD_CENTER_NO_HOLDON                       (MSGID_START_CALL_ROOM+36)    //�������˽���

//2008-12-10����
#define MSGID_CMD_REST_ROOM_PASSWORD                   (MSGID_START_CALL_ROOM+37)    //�޸ķ�������,����������������
#define MSGID_CMD_REST_DOOR_PASSWORD                   (MSGID_START_CALL_ROOM+38)    //�޸��ſڻ���������

//2009-04-01����
#define MSGID_CMD_LEAVEWORD_SENT_WAITFOR_ACK           (MSGID_START_CALL_ROOM+39)    //���������ѷ��ͣ��ȴ���Ӧ
#define MSGID_CMD_CALL_DEBUG 		(MSGID_START_CALL_ROOM+40)    //������ʾ��ʽ��Ϣadd by hu 2010.11.23
#define MSGID_MONITOR_OPEN_LOCK                           (MSGID_START_CALL_ROOM+41) 	//add by hu 2011.05.10
//2013-01-04����
#define MSGID_ROOM_DIVERT_ROOM                         (MSGID_START_CALL_ROOM+42)    //����ת������е�����
#define MSGID_ROOM_DIVERT_CENTER                         (MSGID_START_CALL_ROOM+43)    //����ת������е�����

typedef void (*pSHOW_FUN)      (int,const char* );
typedef void (*pCALLEND_FUN)   (unsigned long);
typedef void (*pHOLDON_FUN)    (unsigned long);
typedef void (*pRINGON_FUN)    (void);
typedef void (*pRINGOFF_FUN)   (void);
#if 0      /*v 0.1.04������ǰ�汾*/
typedef int  (*pDOOROPEN_FUN)  (void);
#else
typedef int  (*pDOOROPEN_FUN)  (unsigned short /*�ݿ�ָ��*/,unsigned char /*����ͷ��ַ δ��*/,char */*����ͷID  δ�ü����Ž���*/,char */*������*/,int /*����δ��*/,int/*�������ͣ�0��ʾ���п���*/);
#endif
typedef int  (*pCCDON_FUN)     (void);
typedef int  (*pCCDOFF_FUN)    (void);
typedef int  (*pLEAVEWORD_FUN) (void);
typedef void (*pCALLPREP_FUN)  (void);
typedef int  (*pDOORLOCK_FUN)  (void);
typedef unsigned char  (*pMP3FLAG_FUN)  (void);	//add by hu 2011.05.23
typedef void (*pCAPTUREPIC_FUN)(unsigned char *);             //ץ�Ļص����� ���� unsigned char* dstroomidĿ�귿����
typedef struct _callparam_{
	int sd;
	unsigned long ip;
	int ret;
	unsigned char name[16];
}CallParam_t;
typedef struct{
	unsigned int  camera_flag;             /*�豸�Ƿ�������ͷ��0�ޣ�������ֵΪ��*/
	//unsigned short ch_en_switch;	/*��Ӣ���л���־add by hu 2010.07.13*/
	int                     callerstatus;            /*0:���� 1���Ѿ���ͨ���������ɹҶ� 2�� ���ڽ������Ӳ��ɹҶ�*/
	int                     timecallstop;            /*ͨ��ʱ������λ200ms���� һ��600x200ms  = 2min*/
	int                     timering;                /*����ʱ������λ200ms���� һ�� 200x200ms = 40s*/ 
	int                     timemonit;               /*����ʱ�䣨��λ200ms��,  һ�� 150x200ms = 30s*/
	unsigned int            MaxVideo:3;              /*�����Ƶ·�������֧��7·����ǰӲ����4·*/
	unsigned int            callipnum:3;             /*��ǰͨ��·�������7·*/
	unsigned int            maxconnecttime:4;        /*������ӳ�ʱʱ�� (��λ 1s)*/
	unsigned int            device_leavewordflag:1;  /*�豸�Ƿ�֧�����ԣ�0��֧�֣� ����֧��*/
	unsigned int            MaxDivertTimes:3;        /*���ת�Ӵ���*/
	unsigned int            DeviceType:3;            /*�豸����: 0������ 1���ֻ��� 2�����Ļ��߹���� 3~7����  2008-12-04����*/
	CallParam_t    cp_calldata[4];          /*��ǰͨ�����ݣ��ɲ��ó�ʼ��  */
	char           localid[16];             /*����id��ʾ*/	
	pCALLPREP_FUN  fun_prep;                /* ����ǰ׼������һ��ͨ������ʱ��Ҳ���ã��Ա�׼����һ��ͨ����,
											   ���ڴ˺����У���ʼ���豸����״̬����Ϣ*/
	pSHOW_FUN      fun_show;                /*������ʾ��Ϣ��ʾ�ص�����*/
	pRINGON_FUN    fun_ringon;              /*�������崦��ص�����*/
	pRINGOFF_FUN   fun_ringoff;             /*���йر������ص�����*/
	pHOLDON_FUN    fun_holdon;              /*ժ������*/
	pCALLEND_FUN   fun_callend;             /*���н�������*/
	pDOOROPEN_FUN  fun_opendoor;            /*���Ŵ���*/
	pCCDON_FUN     fun_ccdon;               /*��������ͷ*/
	pCCDOFF_FUN    fun_ccdoff;              /*�ر�����ͷ*/
	pLEAVEWORD_FUN  fun_leaveword;			/*���Դ���,����0�����ԣ���������*/
	//pDOORLOCK_FUN   fun_lockdoor;           /*2009-02-24���ź���*/
	pCAPTUREPIC_FUN fun_capturepic;         /*2009-02-24ץ�Ļص�����*/
	pMP3FLAG_FUN	fun_getmp3flag;	//add by hu 2011.05.23
	unsigned int usecenternegotiate;
}WRTI_CallerSetup_t;
#ifdef __cplusplus
extern "C" {
#endif

/*����IP�豸�������У����У��ⲿ�ӿ�*/
/*==================start==============================*/
int wrti_CallerSetup(WRTI_CallerSetup_t callersetup);                              //��ʼ������
void wrti_CallerCleanup();                                                         //�ͷź�����Դ

/*==========================���ж�ֻ�����==========================


	                        ��ʼ���ж�ֻ�
	                       /      |      \  
	                      /       |       \
	                     /        |        \
	                 ���зֻ�1   ...     ���зֻ�n
	                     |        |         |
	                     |        |         |
	                 �ֻ�1����   ...     �ֻ�n����
	                     \                  /
	                      \  �ȴ��ֻ�ժ��  /
	                              |
	                              |
	                 ��һ�ֻ�ժ�����Ҷ�����ֻ����Ӽ�����
	                              |
	                              |
	                       ��ͨ����ͨ������
	                              |
	                           ����ͨ��

*/
int wrti_devicecaller(CallParam_t cpCaller[], int num, unsigned short uCmdId);     //���У�������ʽ��,ֻ��ʼ��CallParam_t �� ip��name��Ա
int wrti_devicehungup( unsigned short uCmdId);                                     //�һ���uCmdId��Ӧ�豸�Ĺһ�ָ��
const WRTI_CallerSetup_t * wrti_GetCallerParam();                                  //��ȡ��ǰWRTI_CallerSetup_t�ṹ����
int wrti_GetCallerStatus();                                                        //��ȡ����״̬
int wrti_CallerInitByDefault(const char *id);


/*==========================���ж���������==========================

	                        ��ʼ���ж�����
	                             |             
	                         ��������-----------------------|
	                             |                          |
	                         ����ʧ��                       |
	                             |                          |
	                      ���б�������1---------------------|
	                             .          ...             |
	                      ���б�������n     ...       ����Ӧ��ɹ�
	                             .          ...             |
	                             |                  ��ͨ����ͨ������
	                             |                          |
	                           ����ͨ��---------------------|
	                           
*/
int wrti_devicecallcenter(CallParam_t cpCaller[], int num, unsigned short uCmdId); //��������(����ʽ)�������ip
void wrti_setlocalid(char *localid);//add by wyx 2013-6-5
void wrti_SetCallerCntNego(int usecntnego);
void wrti_SetCallerTime(int call, int ring, int monit);                          //���ú���ʱ��
const char *wrti_GetCallerVersion(void);                                         //��ȡ���п�汾��
/*==================end==============================*/


/*
	ע�⣺
	     �����ʹ�ú��й��ܣ�����ĺ������Բ������
*/

/*
	
    IP�ſ�������ipΧǽ�����������Ǵ�����
	StartAV_Server ����
	WaitMontEnd �ȴ�����
	StopAV_Arg �쳣����
	DeviceIsCalling ������ip������0��ѯIP�豸�Ƿ��ں���״̬�£�������ip=0,�������к��У�close_flag��0 ʱ������رգ��������ѯ��
	DeviceIsWatching������ip������0��ѯIP�豸�Ƿ��ڼ���״̬�£�������ip=0,�������м��ӣ�close_flag��0 ʱ������رգ��������ѯ��
	DeviceStopAllSpec ֹͣ(close_flag!=0)���߲�ѯ(close_flag=0)ָ��״̬�������豸
*/
/*==================start==============================*/
int StartAV_Server(unsigned int sd,unsigned long ip,AV_RunStatus rs,unsigned short AvPort);
void WaitMontEnd(int index, int sd,unsigned long ip,unsigned short uCmd);
void StopAV_Arg(unsigned long sd, int index,int flag);/*flag =0 �����һ�(�յ��һ�����),flag=1 �����һ�*/
int DeviceIsCalling(unsigned long ip,int close_flag);
AV_RunStatus DeviceIsWatching(unsigned long ip, int close_flag);
//AV_RunStatus CenterIsWatching();//add by hu 2011.03.24
int DeviceStopAllSpec(AV_RunStatus rs, int close_flag);
/*==================end==============================*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*���п����ڲ�˽�нӿڣ�ʹ�ô�ģ����Ժ������º���*/
/*==================start==============================*/
void WaitCallCentTaskEnd(int index, int sd, struct _net_pack_ np);
void WaitCallRoomTaskEnd(int index[], CallParam_t cp[], int num, struct _net_pack_ np);
int InitAV_Arg(unsigned int sd,int index,unsigned long ip,AV_RunStatus runstatus,const char *dstid);
void StopAV_Arg(unsigned long sd, int index,int flag);/*flag =0 �����һ�(�յ��һ�����),flag=1 �����һ�*/
int GetAVIndex(unsigned long ip, AV_RunStatus runstatus);
int tcp_send(CallParam_t cp[], int num,const char *data, int len);
int StartAV_ServerList_CallRoom(CallParam_t cpParam[],int devicecount,AV_RunStatus rs,unsigned short AvPort);
void StopAV_ArgList(CallParam_t cp[],int indexs[],int num,int flag);

int doorendleaveword(unsigned long sd,const char *destid);
int leavewordrequest(CallParam_t cp[], int num);
/*==================end==============================*/


#ifdef __cplusplus
}
#endif
#endif

