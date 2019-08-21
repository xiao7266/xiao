#ifndef __CTRLCMD_H__
#define __CTRLCMD_H__

#if defined(__cplusplus)
extern "C" {
#endif

#define FILE_LEN  60
#define MAX_FILE_LEN 256

#define SEND_TO_CENTER 1   //�Ƿ������Ժ�ͼƬ������
        //#undef SEND_TO_CENTER

#define HAVE_PLAY_JPEG 1

#define HAVE_USED_CLIENT 1
#define SMALLCALL_SOCK_INDEX 65536
#define HAVE_USED_SDL 1     //�Ƿ�ʹ��SDL�ж��У������ź����Ⱥ���
        //#undef HAVE_USED_SDL
#define DOWNLOAD_FROM_HTTP 1    //�Ƿ�ʹ�ô�WEB server����������
        //#undef  DOWNLOAD_FROM_HTTP
        //#define USED_FFS  1
#define USED_NEW_CMD  1       /*�����°�������ʽ*/
        //#undef  USED_NEW_CMD
#define  UDP_PORT         20008

#define CAP_FRAME_TIME  6


#define MONITOR_TYPE_DOOR 0
#define MONITOR_TYPE_SMALLDOOR 1
#define MONITOR_TYPE_GATE     2
#define MONITOR_TYPE_OTHER   3
#define MONITOR_TYPE_IPCAMERA 4

        /*��ը��
        */
#define USE_BOMB 0x01

//
#define ENGLISH 0x01
#define CHINESE 0x02
        /*
        #ifndef MEM_DEBUG
        #define MEM_DEBUG 1
        #endif

        #ifdef MEM_DEBUG
        #include "zenmalloc.h"
        #endif
        */

        //�Զ�������
#define  CUSTOM_CMD_CANCEL        0x900            /*ȡ��֮ǰ�����������Ķ���*/
#define  CUSTOM_CMD_PHOTO         0X901            /*�ֶ�ץ��*/
#define  CUSTOM_CMD_LEAVE         0X902            /*С�ſڻ�����*/
#define  CUSTOM_CMD_DOWNLOAD_MP3  0X903            /*����MP3*/
#define  CUSTOM_CMD_DOWNLOAD_OVER 0X904            /*MP3�������*/
#define  CUSTOM_CMD_ROOM_STOP_RECORD 0x905          /*ֹͣ¼��*/

#define  CUSTOM_CMD_CALLER_SHOW_MSG  0x906          /*Ϊ�������п��ṩ����Ϣ*/
#define  CUSTOM_CMD_CALLER_RINGON    0X907
#define  CUSTOM_CMD_CALLER_RINGOFF   0X908
#define  CUSTOM_CMD_CALLER_HOLDON    0x909
#define  CUSTOM_CMD_CALLER_CALLEND   0X910
#define  CUSTOM_CMD_CALLER_PREP      0X911

#define  CUSTOM_CMD_CALL_ROOM        0X912            /*�Զ�������������������*/
#define  CUSTOM_CMD_WARNING_TIMER    0X913            /*�������ö�ʱ������*/
#define  CUSTOM_CMD_IS_SCREEN_ADJUST  0x914           /*��ĻУ׼����*/
#define  CUSTOM_CMD_ROOM_START_RECORD 0x915            /*ס������(¼��)*/

#define CUSTOM_CMD_LOCAL_UPGRADE      0x916

//


#ifdef HAVE_PLAY_JPEG
#define CUSTOM_CMD_RECORD_JPEG_FRAME  0x919
#define CUSTOM_CMD_PLAY_JPEG_FRAME    0X920
#define CUSTOM_CMD_STOP_JPEG_FRAME    0X921
#endif

#define  CUSTOM_SYSTEM_FORMAT         0x922          /*ϵͳ���ڸ�ʽ��*/


        //ϵͳ״̬
#define SYS_BUSY                    100              /*ϵͳ��æ״̬*/
#define SYS_IDLE                    0                /*ϵͳ����״̬*/
#define SYS_CENTERCALLROOMTALK      1                /*���ĺ����û�״̬*/
#define SYS_TALKTOMAIN              2                /*�ſں����û�״̬*/
#define SYS_MONT                    3                /*����״̬*/
#define SYS_TRANSMITFILE            4
#define SYS_RECVFILE                5
#define SYS_HELP                    6
#define SYS_MONEY                   7
#define SYS_ROOMCALLCENTERTALK      8                /*��������״̬*/
#define SYS_ROOMTALKROOM            9                /*��������ͨ״̬*/
#define SYS_GETSYSINFO              10               /*���ϵͳ��Ϣ״̬*/
#define SYS_WARNING                 11               /*����״̬*/
#define SYS_REQUESTFEE              12               /*�����ѯ����״̬*/
#define SYS_REQUESTREPAIR           13               /*����ά��״̬*/
#define SYS_REQEUSTRINGLIST         14               /*���������б�״̬*/
#define SYS_DOWNLOADRING            15               /*��������״̬*/
#define SYS_EMERG                   16               /*��������״̬*/
#define SYS_GETROOMIP               17               /*��÷ֻ���IP*/
#define SYS_LEAVEWORD               18               /*ϵͳ����*/
#define SYS_UPGRADE                 19               /*ϵͳ����*/
#define SYS_GETTIME                 20               /*ϵͳ��ȡʱ��*/
#define SYS_SMALLDOORCALL           21               /*С�ſڻ�����״̬*/
#define SYS_MONTSMALLDOOR           22               /*����С�ſڻ�*/
#define SYS_MYROOMTALKROOM          23               /*��������ͨ״̬*/
#define SYS_REQDOCINFO              24               /*�ֻ����������Ϣ*/
#define SYS_ACTIVE_CALL             25               /*ϵͳ������������״̬*/
#define SYS_REQHELPINFO             26               /*�ֻ����������Ϣ*/
#define SYS_ISSCREENADJUST          27               /*����У׼��Ļ*/
#define SYS_ISRECORD                28               /*ϵͳ����¼��״̬*/
#define SYS_CALLSIP                 29               /*ϵͳ�������SIP�绰*/
#define SYS_SIPCALL                 30               /*ϵͳ����SIP �����ն�*/
        //#define SYS_CALL_LIFT               29               /*ϵͳ������е���״̬*/
#define SYS_FORMAT                  31                     /*ϵͳ�����ʽ��״̬*/

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

#define ROOM_GET_WEATHER          0x0019                /*�ֻ���������������Ԥ��*/
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
#define ROOM_SEND_COLOR_MSG           0x0031              /*�ֻ��������ķ��Ͳ�ɫ����*/


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
#define ROOM_USED_SIMULATE            0x008a              /*�Ƿ���ģ��ͨ����ʹ�á�*/
#define ROOM_USED_AGENT               0x008b              /*�зֻ��ֶ��й�*/


#define ROOM_SCR_ACK                  0x0097

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
#define CENTER_WEATHER_ACK          0x0209             /*����Ӧ���ѯIP��ַ*/

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

//#define CENTER_NOPERSON		              0x02a5
//#define CENTER_OVERTIMETALK	            0x20a6
#define CENTER_SET_ROOM_SCR         0x02a6              /*�������÷ֻ�ͼƬ��������������ȣ�*/

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


        /*********************************************************************************
        С�ſڻ���ֻ�����
        *********************************************************************************/
#define SMALLDOOR_IDLE                      0x601           //С�ſڿ���
#define SMALLDOOR_ACK_STOPMONT              0x602           //С�ſڻ�Ӧ��ֹͣ����
#define SMALLDOOR_CALLROOM                  0x603           //С�ſڻ����зֻ�
#define SMALLDOOR_ACK_HOLDON                0x604           //С�ſڻ�Ӧ��ժ��
#define SMALLDOOR_ACK_OPENLOCK              0x605           //С�ſڻ�Ӧ����
#define SMALLDOOR_ACK_HANGUP                0x606           //С�ſ�Ӧ��һ�
#define SMALLDOOR_ACK_LEAVE                 0x607           //С�ſ�Ӧ������
#define SMALLDOOR_ACK_VERSION               0x608           //С�ſ�Ӧ��汾

#define ROOM_START_MONT_SMALLDOOR           0x601           //�ֻ�����С�ſ�
#define ROOM_STOP_MONT_SMALLDOOR            0x602           //�ֻ�ֹͣ����С�ſ�
#define ROOM_ACK_SMALLDOOR                  0x603           //�ֻ�Ӧ��С�ſڻ�����
#define ROOM_HOLDON_SMALLDOOR               0x604           //�ֻ�ժ��С�ſڻ�
#define ROOM_OPENLOCK_SMALLDOOR             0x605           //�ֻ�����С�ſڻ�
#define ROOM_HANGUP_SMALLDOOR               0x606           //�ֻ��һ�С�ſڻ�
#define ROOM_SMALLDOOR_LEAVE                0x607           //�ֻ�����
#define ROOM_GET_SMALLDOOR_VER              0x608           //�ֻ���ȡ�汾

        /*2009-7-29 13:45:07 new add cmd �ֻ���RIU*/
#define RIU_KEY                             0X0635          /*RIU key*/        
#define ROOM_RIU_DEFEND_ACK                0X0636          /*�ֻ�Ӧ��RIUȫ������*/
#define ROOM_RIU_REMOVE_DEFEND_ACK          0x0637          /*�ֻ�Ӧ��RIUȫ������*/
#define ROOM_RIU_OPEN_LOCK_ACK              0x0638          /*�ֻ�Ӧ��RIU����*/
#define ROOM_RIU_REMOVE_ACK                 0X0639          /*�ֻ�Ӧ��RIU����������*/
#define ROOM_RIU_ROOM_WARNING_ACK           0x0640          /*�ֻ�Ӧ��RIU��������*/       

#define RIU_ROOM_DEFEND                     0X0636          /*RIU ֪ͨ�ֻ�����*/
#define RIU_ROOM_REMOVE_DEFEND              0X0637          /*RIU֪ͨ�ֻ�����*/
#define RIU_ROOM_OPEN_LOCK                  0X0638          /*RIU֪ͨ�ֻ�����*/
#define RIU_ROOM_REMOVE                     0X0639          /*RIU֪ͨ�ֻ�����������*/
#define RIU_ROOM_WARNING                    0x0640          /*RIU֪ͨ�ֻ���������*/

//modify [2012-12-21 13:54  add cmd by wyx]
#define MEDIA_NEGOTIATE                0x1190
#define MEDIA_NEGOTIATE_ACK            0x1191

#define ROOM_STOP_MONT_BYKEY           0x1192 //�����ڱ��س����ڲ�������ժ���밴��ժ��

//add by wyx [2014-6-9]
#define CENTER_SET_IOLEVEL			   0x1193
#define CENTER_SET_IOLEVEL_ACK		   0x1194

//add by wyx [2014 6 26] ת��ý��Э�����ӻ�
#define MEDIA_NEGOTIATE_SESSION		   0x1195
//add by wyx [2014 6 26] ת�������ַ�б����ӻ� 
#define TRANSMIT_IP_TO_OTHERROOM	   0x1196
//add by wyx [2014 7 2] �ֻ����Ӽ���豸
#define IPCAMERA_MONITOR_START		   0x1197
#define IPCAMERA_MONITOR_END	   0x1198

//2011-12-7 16:28:56
#define ROOM_ALARM_1                        0X0F01 //Ƿѹ
#define ROOM_ALARM_2                        0X0F02 //��ѹ�ָ�
#define ROOM_ALARM_3                        0X0F03//�����ϵ�
#define ROOM_ALARM_4                        0X0F04 //�����ָ�

//2012-2-14 //�����°�ġ�
#define ROOM_ALARM_11                        0X0F11 //Ƿѹ
#define ROOM_ALARM_12                        0X0F12 //��ѹ�ָ�
#define ROOM_ALARM_13                        0X0F13//�����ϵ�
#define ROOM_ALARM_14                        0X0F14 //�����ָ�

        /*2009-10-26,new add cmd �ּ���ǿ��ӷֻ�*/ 
// modify [2011��3��21�� 8:46:24 by ljw]
#define  CALL_SIMULATE                0x609               /*����ת�Ƶ��ǿ��ӷֻ�*/
#define  SIMULATE_HOLDON              0x60a
#define  SIMULATE_HANGUP              0x60b
#define  SIMULATE_UNLOCK              0x60c
#define  SIMULATE_ALARM               0x60d
#define  INFORM_SIMULATE_HANGUP       0x60e

#define ROOM_ACK_SIMULATE_ALARM       0x60d
#define ROOM_ACK_SIMULATE_UNLOCK      0x60c
#define ROOM_ACK_SIMULATE_HANGUP      0x60b
#define ROOM_ACK_SIMLUATE_HOLDON      0x60a

        /**************************************SIP caller**********************/
#define  SIP_CALL                   0x0301
#define  SIP_CALL_PROCESSING        0X0302
#define  SIP_BUSY                   0x0303
#define  SIP_CALL_RING              0x0304
#define  SIP_CLOSE                  0x0305
#define  SIP_START_AUDIO            0x0306

        /*--------------------------------------------------------------*/

        /*
        * �ֻ����ƼҾӡ�����Э��
        */
#define PHONE_INV_ROOM              0x0611
#define PHONE_INQ_ALL_HOME_STATUS   0x0612
#define PHONE_CTRL_MORE_SAFE_STATUS 0X0613
#define PHONE_CTRL_SAFE_STATU       0x0614
#define PHONE_INQ_ALL_SAFE_STATUS   0x0615
#define PHONE_CTRL_SCENE            0x0616
#define PHONE_CTRL_ONE_DEVICES      0x0617
#define PHONE_INQ_ONE_DEVICES       0x0618
#define PHONE_ACK_KEEP_LIVE         0x0619


#define ROOM_ACK_INV_PHONE                       0x0811
#define ROOM_ACK_ALL_HOME_STATUS                 0x0812
#define ROOM_ACK_CTRL_MORE_SAFE_STATUS           0x0813
#define ROOM_ACK_CTRL_SAFE_STATUS                0x0814
#define ROOM_ACK_INQ_ALL_SAFE_STATUS             0x0815
#define ROOM_ACK_CTRL_SCENE                      0x0816
#define ROOM_ACK_CTRL_ONE_DEVICES                0x0817
#define ROOM_ACK_INQ_ONE_DEVICES                 0x0818
#define ROOM_KEEP_LIVE                           0x0819


        /*************************************************************
        add by ljw
        *************************************************************/
        typedef void (*display)(unsigned long event[4]);
        static const int STATUS_BASE=0x1000;

#define  STATUS_CENTER_CALLROOM              STATUS_BASE+0            /*֪ͨGUI�����ĺ��зֻ�*/
#define  STATUS_DOOR_CALLROOM                STATUS_BASE+1            /*֪ͨGUI���ſڻ����зֻ�*/
#define  STATUS_CENTER_HANGUP                STATUS_BASE+2            /*֪ͨGUI�����Ĺһ�*/
#define  STATUS_DOOR_HANGUP                  STATUS_BASE+3            /*֪ͨGUI���ſڻ����зֻ�*/
#define  STATUS_DOOR_BUSY                    STATUS_BASE+4            /*֪ͨGUI���ſڻ���æ*/
#define  STATUS_CENTER_BUSY                  STATUS_BASE+5            /*֪ͨGUI�����ķ�æ*/
#define  STATUS_CENTER_HOLDON                STATUS_BASE+6            /*֪ͨGUI������ժ��*/
#define  STATUS_ROOM_STOPMONT                STATUS_BASE+7            /*֪ͨGUI ֹͣ����*/
#define  STATUS_ROOMTALKROOM                 STATUS_BASE+8            /*֪ͨGUI���ֻ��յ���ķֻ��ĺ���*/
#define  STATUS_ROOM_BUSY                    STATUS_BASE+9            /*֪ͨGUI���ֻ���æ*/
#define  STATUS_NET_EXCEPTION                STATUS_BASE+10           /*֪ͨGUI�������쳣�������ж�*/
#define  STATUS_DOOR_UNLOCK                  STATUS_BASE+11           /*֪ͨGUI���ſڻ������ɹ�*/
#define  STATUS_ROOM_WARING_OK               STATUS_BASE+12           /*֪ͨGUI�������ɹ�*/
#define  STATUS_GET_RING_ERROR               STATUS_BASE+13           /*֪ͨGUI����ȡ��������*/
#define  STATUS_RING_LIST                    STATUS_BASE+14           /*֪ͨGUI�������б�*/
#define  STATUS_RING_DOWNLOAD                STATUS_BASE+15           /*֪ͨGUI����������*/
#define  STATUS_REQUEST_REPAIR               STATUS_BASE+16           /*֪ͨGUI��ά��*/
#define  STATUS_PUBLIC_TEXT_INFO             STATUS_BASE+17           /*֪ͨGUI�������ı�����*/
#define  STATUS_PUBLIC_PIC_INFO              STATUS_BASE+18           /*֪ͨGUI������ͼƬ����*/
#define  STATUS_ROOM_HANGUP                  STATUS_BASE+19           /*֪ͨGUI���ֻ��һ�*/
#define  STATUS_QUERY_FEELIST                STATUS_BASE+20           /*֪ͨGUI�����Ļ�Ӧ���ò�ѯ*/
        //#define  STATUS_SET_SYS_INFO                 STATUS_BASE+21           /*֪ͨGUI����������ϵͳ��Ϣ*/
#define  STATUS_DOOR_LEAVEWORD               STATUS_BASE+22           /*֪ͨGUI���ſڻ�����*/
#define  STATUS_ROOM_EMERG                   STATUS_BASE+23           /*֪ͨGUI��������ȳɹ�*/
#define  STATUS_ROOM_SYS_UPDATE              STATUS_BASE+24           /*֪ͨGUI��ϵͳ������*/
        //#define  STATUS_ROOM_GET_IP                  STATUS_BASE+25           /*֪ͨGUI����ȡROOM��ַ*/
#define  STATUS_ROOM_CALLING_ROOM            STATUS_BASE+26           /*֪ͨGUI�����ں���ROOM*/
#define  STATUS_ROOM_TALK_ROOM               STATUS_BASE+27           /*֪ͨGUI����ʼͨ��*/
#define  STATUS_CONNECTING                   STATUS_BASE+28           /*֪ͨGUI����������*/
#define  STATUS_RING_LIST_OVER               STATUS_BASE+30           /*֪ͨGUI,�����б����*/
#define  STATUS_FEE_LIST_OVER                STATUS_BASE+31           /*֪ͨGUI�������б����*/

#define STATUS_START_EMERG                   STATUS_BASE+32           /*֪ͨ��ʼ����*/
        //
        //#define STATUS_ROOM_GET_IP_ERROR            STATUS_BASE+29             /*GET IP error */

#define STATUS_ROOM_VIEW_TIME               STATUS_BASE+33             /*֪ͨGUI��ʾ�¼�*/

#define STATUS_SMALLDOOR_CALL_ROOM          STATUS_BASE+34             /*֪ͨGUI��С�ſڻ����зֻ�*/
#define STATUS_SMALLDOOR_BUSY               STATUS_BASE+35             /*С�ſڻ���æ*/

#define STATUS_AVOID_TIMES_TIMEOUT          STATUS_BASE +36           /*֪ͨGUI�������ʱ�䵽��*/

#define STATUS_SAFE_WARNING                 STATUS_BASE+37            /*��������*/

#define STATUS_PAUSE_VIDEO                  STATUS_BASE+38            /*��Ƶ��ͣ������*/

#define STATUS_REQDOCINFO_OVER              STATUS_BASE+39            /*��ñ�����Ϣ����*/


#define STATUS_START_WARNING                STATUS_BASE+40

#define STATUS_REMOTE_ALARM                 STATUS_BASE+41           /*Զ�̿��Ʋ�������*/

#define STATUS_RECORD_START                 STATUS_BASE+42           /*�뿪ʼ¼��*/
#define STATUS_RECORD_STOP                  STATUS_BASE+43           /*ֹͣ¼��*/
#define STATUS_REMOTE_SCENE                STATUS_BASE+44          /*Զ�̿����龰*/

#define STATUS_RMCORD_TIME                  STATUS_BASE+45         /*¼��ʱ��*/
#define STATUS_LEAVE_PLAY_OVER              STATUS_BASE+46           /*���Բ������*/

#ifdef HAVE_PLAY_JPEG
#define STATUS_PLAY_JPEG                    STATUS_BASE+47
#define STATUS_DISPLAY_REC                  STATUS_BASE+48
#endif

        //sip status
#define STATUS_SIP_CONNECTING               STATUS_BASE+49
#define STATUS_SIP_CLOSE                    STATUS_BASE+50
#define STATUS_SIP_BUSY                     STATUS_BASE+51
#define STATUS_SIP_START_TALK               STATUS_BASE+52
#define STATUS_SIP_CALL_ROOM              STATUS_BASE+53

        //2010/10/8
#define STATUS_CAP_NEW_PHONE               STATUS_BASE+54
#define STATUS_SCREEN_CLOSE                STATUS_BASE+55
//2011-4-28 15:37:34
#define STATUS_CHECK_JIAJU                 STATUS_BASE+56

//2011-5-16 15:13:02
#define STATUS_ALARM_CLOCK                 STATUS_BASE+57

//2011-8-11 10:47:38
#define STATUS_WEATHER_RESULT              STATUS_BASE+58

#define STATUS_REMOTE_UPATE_SYSTEM_INFO    STATUS_BASE+59             /*Զ�̸�����ϵͳ��Ϣ*/
#define STATUS_DIVERT_ROOM_TO_ROOM    	   STATUS_BASE+60             /*Զ�̸�����ϵͳ��Ϣ*/
#define STATUS_DIVERT_ROOM_TO_CENTER       STATUS_BASE+61             /*Զ�̸�����ϵͳ��Ϣ*/
#define  STATUS_DOOR_LEAVEWORD_STOP        STATUS_BASE+62           /*֪ͨGUI���ſڻ�����*/
#define  STATUS_TS_CAL        				STATUS_BASE+90  
#define  STATUS_PHONE_OR_EXTERNDEVICE_HOLDON   STATUS_BASE+91 
#define STATUS_IOLEVEL_TEST					STATUS_BASE+92
#define STATUS_KEYLOCK_EVENT				STATUS_BASE+93
#define STATUS_KEYCALL_EVENT				STATUS_BASE+94
#define STATUS_KEYHOLD_EVENT				STATUS_BASE+95
#define STATUS_SET_IOLEVEL_EVENT			STATUS_BASE+96

#define STATUS_DEBUG_MSG                    STATUS_BASE+100           /*debug msg*/



        //////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////
        //�����ʱ�¼�
#define TIME_MONT_EVENT_START           0x1      //���Ӽ�ʱ��ʼ
#define TIME_MONT_EVENT_TIMEOUT      0x2      //��ʱ��ʱ�䵽
#define TIME_MONT_EVENT_STOP            0x4      //���Ӽ�ʱֹͣ

#define TIME_MONT2_EVENT_START           0x11      //���Ӽ�ʱ��ʼ
#define TIME_MONT2_EVENT_TIMEOUT      0x21      //��ʱ��ʱ�䵽
#define TIME_MONT2_EVENT_STOP            0x41      //���Ӽ�ʱֹͣ
#define TIME_RING_EVENT_START            0x8      //������ʱ��ʼ
#define TIME_RING_EVENT_TIMEOUT       0x10      //��ʱ��ʱ�䵽
#define TIME_RING_EVENT_STOP             0x20      //������ʱֹͣ

#define TIME_TALK_EVENT_START           0x40      //ͨ����ʱ��ʼ
#define TIME_TALK_EVENT_TIMEOUT      0x80
#define TIME_TALK_EVENT_STOP             0x100      //ͨ����ʱֹͣ

#define TIME_WARNING_EVENT_START        0x200     //�������߽���������ʱ��ʼ
#define TIME_WARNING_EVENT_TIMEOUT     0x400     //
#define TIME_WARNING_EVENT_STOP         0x800    //�������߽���������ʱֹͣ



#define TIME_MP3_EVENT_START            0x1000     //��Ҫ��Ե��Ƕ���Ϣ����
#define TIME_MP3_EVENT_TIMEOUT       0x2000    //MSGʱ�䳬ʱ
#define TIME_MP3_EVENT_STOP             0x4000     //MSG��ʱ��ֹͣ



#define TIME_SMALLDOOR_LEAVE_EVENT_START 0x8000     //С�ſ����Կ�ʼ
#define TIME_SMALLDOOR_LEAVE_EVENT_TIMEOUT 0x10000    //��ʱ
#define TIME_SMALLDOOR_LEAVE_EVENT_STOP    0x20000   //ֹͣ

#define TIME_AVOID_SETTIMES_EVENT_START        0x40000   //�����ʱ�������¼�����
#define TIME_AVOID_SETTIMES_EVENT_TIMEOUT      0x80000   //�����ʱ�������¼���ʱ
#define TIME_AVOID_SETTIMES_EVENT_STOP         0x100000   //�����ʱ�������¼�ֹͣ

#define TIME_UNLOCK_EVENT_START                0x200000   //������5��һ����¼�����
#define TIME_UNLOCK_EVENT_TIMEOUT              0x400000  //��ʱ
#define TIME_UNLOCK_EVENT_STOP                 0x800000  //ֹͣ

#define TIME_DOOR_LEAVE_EVENT_START            0x1000000  //�ſڻ����Կ�ʼ
#define TIME_DOOR_LEAVE_EVENT_TIMEOUT       0x2000000  //�ſڻ����Գ�ʱ
#define TIME_DOOR_LEAVE_EVENT_STOP             0x4000000  //�ſڻ����Լ�ʱֹͣ

#define TIME_AGENT_SETTIMES_EVENT_START        0x8000000   //�й�ʱ�������¼�����
#define TIME_AGENT_SETTIMES_EVENT_TIMEOUT      0x10000000   //�й�ʱ�������¼���ʱ
#define TIME_AGENT_SETTIMES_EVENT_STOP         0x20000000   //�й�ʱ�������¼�ֹͣ

        ///////////////////////////////////////////////////////////
        //timer task 2

#define TIME_DISPLAY_TIME_EVENT_START   0x1     //��ʾʱ���¼�������1������ʾһ�ε�ǰʱ�� //timer2
#define TIME_DISP_TIME_EVENT_TIMEOUT    0x2     //�¼���ʱ�� //timer2

#define TIME_SMALLDOOR_EVENT_START      0x4      //С�ſڻ��¼� //timer2
#define TIME_SMALLDOOR_EVENT_TIMEOUT    0x8 //time2
#define TIME_SMALLDOOR_EVENT_STOP       0x10   //timer2

#define TIME_RECORD_EVENT_START                    0x20 //��ʼ¼����ʱ
#define TIME_RECORD_EVENT_TIMEOUT               0x40 //��ʼ¼����ʱ
#define TIME_RECORD_EVENT_STOP                    0x80//��ʼ¼����ʱ
#if 1
#define TIME_JIAJU_CHECK_START            0x100     //�����Ѿ����ӵ������� 5s���һ��
#define TIME_JIAJU_CHECK_STOP             0x200
#define TIME_JIAJU_CHECK                       0x400     //�������

#endif

#ifdef HAVE_PLAY_JPEG

#define TIME_PLAY_JPEG_START                    0x800
#define TIME_PLAY_JPEG                          0x1000
#define TIME_PLAY_JPEG_STOP                     0x2000

#endif
/*Ƿѹ������ʱ��*/
#define TIME_LOWTENSION_WARNING_EVENT_START               0x4000
#define TIME_LOWTENSION_WARNING_EVENT_TIMEOUT             0x8000
#define TIME_LOWTENSION_WARNING_EVENT_STOP                0x10000
        


        /////////////////////////////////////////////////////////////////////////
#define CENTER_CALL_RING         0
#define WALL_CALL_RING           1
#define DOOR_CALL_RING           2
#define SMALL_DOOR_CALL_RING     3
#define ROOM_CALL_RING           4
#define WARNING_RING             5
#define MSG_RING                 6
#define ALARM_CLOCK_RING         7


        typedef struct socket_packet_t{
                int sock_index;
                int  validlen;
                unsigned char buf[256];
        }SOCKET_PACKET,*PSOCKET_PACKET;

        typedef struct socket_packet_t2{
                int sock_index;
                int  validlen;
                unsigned char* buf;
        }SOCKET_PACKET2,*PSOCKET_PACKET2;

        typedef struct _tagcmd{
                char cmdhead[4];
                int len;
                short cmd;
        }CmdHeader,*PCmdHeader;

        typedef struct _tagRoomWaring{
                unsigned long ipaddr;
                unsigned char type;
                short    cmd;
        }ROOMWARING,*PROOMWARING;
        /*
        typedef struct _tagGUICMD{
        short cmd;
        int   len;
        unsigned char type[1];
        }GUICMD,*PGUICMD;
        */
        typedef struct _tagPHOTOINFO{
                int            lock;
                int            idle;
                unsigned char  name[FILE_LEN];
                unsigned char  *addr;
                unsigned long   len;
                int             read;  //��2�ֽڱ�ʾ�ص㣬0���ſڻ���1��С�ſڻ���2��Χǽ���� ��2�ֽڣ���ʾ�Ƿ��Ķ���0��δ����1�Ѷ���
                unsigned long   dates;
                unsigned long   times;
        }PHOTOINFO;

        typedef struct _tagVOICEINFO{
                int            lock;
                int            idle;
                unsigned char  name[FILE_LEN];
                unsigned char  *addr;
                unsigned long   len;
                int             read;
                unsigned long   dates;
                unsigned long   times;
        }VOICEINFO;

        typedef struct _tagDOWNRINGINFO{
                int           idle;
                int 	      lock;
                char          name[MAX_FILE_LEN];
        }DOWNRINGINFO;

        typedef struct _tagringlist{
                bool isdown;
                char name[41];
                unsigned char* addr;
                unsigned long len;
                struct _tagringlist* next;
        }RINGLIST;

        typedef struct _tagfeelist{
                unsigned char month;
                unsigned char buf[33];
                struct _tagfeelist* next;
        }FEELIST;

        typedef struct _tagLOGINFO{
                int len;
                //#ifdef USED_FFS
                unsigned char   addr[400];
                //#else
                //   unsigned char*  addr;
                //#endif
                int             read;
                int             idle;
                int             lock;
                unsigned long   dates;
                unsigned long   times;
        }LOGINFO;


        typedef struct _tagMSGINFO{
                int read;
                int len;
                //#ifdef USED_FFS
                unsigned char addr[400];
                //#else
                //   unsigned char* addr;
                //#endif
                int  lock;
                int             idle;
                unsigned long   dates;
                unsigned long   times;
        }MSGINFO;

        typedef struct _tagRINGSET{
                //unsigned char* addr;
                //long size;
                char ring_path[MAX_FILE_LEN];
        }RINGSET;

        typedef struct _tagtelephone{
                int idle;
                int lock;
                char name[21];
                char room_number[16];
                unsigned long ipaddr[4];
                unsigned long   dates;
                unsigned long   times;
        }TELEPHONE;

        typedef struct _tagROOMINFO{
                int valid;
                int id;
                unsigned long ipaddr;
        }ROOMINFO;

#if defined(__cplusplus)
}
#endif
#endif

