/*
�ļ�����   tmWRTICaller.h
���ܣ�   ����ip�豸���й���ģ��
�汾��   Version 0.1
�������ڣ�   2008-10-22
��Ȩ��   WRT 
���ߣ�   ��ѭ��
*/
#ifndef _TM_WRTI_CALLER_H_
#define _TM_WRTI_CALLER_H_
#define PROTOCOL_VERSION2

#define MSGID_START_CALL_ROOM                            10000                         //��ʼ���зֻ�
#define MSGID_START_CALL_CENTER                          (MSGID_START_CALL_ROOM+1)     //��ʼ��������
#define MSGID_START_CALL_ANOTHRE_CENTER                  (MSGID_START_CALL_ROOM+2)     //��ʼ���б�������
#define MSGID_DEVICE_CONNECTING                          (MSGID_START_CALL_ROOM+3)     //��������Ŀ��...
#define MSGID_CONNECTED_ERROR                            (MSGID_START_CALL_ROOM+4)     //����Ŀ��ʧ��
#define MSGID_SENDING_CALL_REQUEST                       (MSGID_START_CALL_ROOM+5)     //���ڷ��ͺ�������
#define MSGID_SEND_CALL_REQUEST_ERROR                    (MSGID_START_CALL_ROOM+6)     //���ͺ�������ʧ��
#define MSGID_WAIT_CALL_REQUEST_ACK                      (MSGID_START_CALL_ROOM+7)     //�ȴ�Ŀ����Ӧ...
#define MSGID_CENTER_BUSY                                (MSGID_START_CALL_ROOM+8)     //���ķ�æ
#define MSGID_CENTER_AGENT_CONNECTING                    (MSGID_START_CALL_ROOM+9)     //�������й�,ת����...
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

#define MSGID_CMD_REST_ROOM_PASSWORD                   (MSGID_START_CALL_ROOM+37)    //�޸ķ�������,����������������
#define MSGID_CMD_REST_DOOR_PASSWORD                   (MSGID_START_CALL_ROOM+38)    //�޸��ſڻ���������

typedef void (*pSHOW_FUN)      (int,const char* );
typedef void (*pCALLEND_FUN)   (unsigned long);
typedef void (*pHOLDON_FUN)    (unsigned long);
typedef void (*pRINGON_FUN)    (void);
typedef void (*pRINGOFF_FUN)   (void);
typedef int  (*pDOOROPEN_FUN)  (void);
typedef int  (*pCCDON_FUN)     (void);
typedef int  (*pCCDOFF_FUN)    (void);
typedef int  (*pLEAVEWORD_FUN) (void);
typedef void (*pCALLPREP_FUN)  (void);

typedef struct _callparam_{
        int sd;
        unsigned long ip;
        int ret;
        unsigned char name[16];
}CallParam_t;
typedef struct{
        int                     camera_flag;             /*�豸�Ƿ�������ͷ��0�ޣ�������ֵΪ��*/
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
        /*==================end==============================*/

        int AV_Server_GetDestStatus();
        void AV_Server_SetDest1(const char *addr,int port);
        void AV_Server_SetDest2(const char *addr,int port);
        void AV_Server_SetDest3(const char *addr,int port);
        void AV_Server_SetDest4(const char *addr,int port);
        void AV_Server_DelDest1(const char *addr,int port);
        void AV_Server_DelDest2(const char *addr,int port);
        void AV_Server_DelDest3(const char *addr,int port);

        void AV_Server_DelDest4(const char *addr,int port);


#ifdef __cplusplus
}
#endif

#endif
