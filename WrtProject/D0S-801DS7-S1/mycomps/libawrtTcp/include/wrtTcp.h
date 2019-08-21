//WRT 2016-7-5
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <errno.h>
//#include "wrtCjson.h"
#include "wrtCjsonShare.h"

#define iptable_name "B:/iptable"
#define cfg_name	 "B:/cfg_name"
#define ERROR_MSG	"XXXXXXXX"
#pragma pack(1)


struct FtpBuf
{
    uint8_t     *buf;
    uint32_t    pos;
};

#ifdef __cplusplus
extern "C" {
#endif
//防区
#define DEFENCE_RESERVERD  16
typedef struct
{
	int defence_id;
	int warning_area;
	int defence_delay;
	int report_delay;
	int defence_type;
	int is_use_defence;
	int is_enable_defence;
	int tmp_defence_delay;
	int tmp_report_delay;
	char reserverd[DEFENCE_RESERVERD];
}T_DEFENCE_INFO;

typedef struct
{
	int defence_id;
	int warning_area;
	int defence_type;
	int defence_delay;
	int report_delay;
	int roomExist;
	int is_use_defence;
	int is_enable_defence;
	int is_insecure_defence;
	int is_warning_defence;
	unsigned int heartbeatOfOtherRoom;
	char roomID[15];
}T_DEFENCE_INFO_FOR_OTHER_ROOM;


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
	WARNING_BALCONY,
	WARNING_UNDEFINE,
};

enum{
	WARNING_ALLTIME=14,
	WARNING_COMMON,
	WARNING_REALTIME,
	WARNING_DELAY,
};
enum{
	//NO_DEFENCE_STATUS=0,
	DEFENCE_EXIST=0,
	DEFENCE_ENABLED=1,
	DEFENCE_WORKING=2,
	DEFENCE_WARNING=3,
};
enum{
	//PLAY_STOP=0,
	PLAY_KEY = 0,
	PLAY_AUDIO,
	PLAY_CALL,
	PLAY_WARNING,
	PLAY_STOP,
};

#define ROOM_SEND_WARNING          CMD_ID(0X1000)
#define ROOM_LEAVE_HOME            CMD_ID(0X1201)
#define ROOM_AT_HOME               CMD_ID(0X1202)
#define ROOM_SEND_WARNING_TO_OTHERS            CMD_ID(0X1203)
#define ROOM_REPEAL_WARNING            CMD_ID(0X1204)
#define DEFENCE_SITUATION_PORT  16789

// 告诉其他分机本机有报警
#define SAFE_SYNC_ALARMING	    0x1203 //send local alarm to other
// 告诉其他分机，本机要给报警防区撤防
#define SAFE_SYNC_DISALARM	    0x1204//send local releasing alarm to other
// 指定防区进行布撤防
#define SAFE_SYNC_BCF		    0x1205//send local working the defence to other
// 告诉其他分机本机可以上报报警(针对有些防区需先响铃再报警)
#define SAFE_SYNC_ALARMReport	    0x1206
//告诉其他分机本机安防端口的使能情况
#define SAFE_SYNC_PORT_ENABLE	    0x1207
//告诉其他分机本机安防端口的布防情况
#define SAFE_SYNC_PORT_BUFANG_STATE	 0x1208
//告诉其他分机本机安防端口的撤防情况
#define SAFE_SYNC_PORT_CHEFANG_STATE	 0x1209
//告诉其他分机本机安防端口的报警情况
#define SAFE_SYNC_PORT_ALARM_STATE	 0x120a

//告诉其他分机本机安防端口是否是24小时防区
#define SAFE_SYNC_PORT_TYPE_ALLDAY	 0x120b
//获取其他分机安防端口的类型
#define SAFE_GET_PORT_TYPE	 0x120c
//告诉其他分机本机安防端口是否是即时防区
#define SAFE_SYNC_PORT_TYPE_INSTANT	 0x120d

//仅作定时同步
#define JUST_SYNC_SAFE_PORT_ENABLE   0x1300   //同步端口使能
#define JUST_SYNC_SAFE_PORT_BUFANG_STAT   0x1301  //同步布防状态
#define JUST_SYNC_SAFE_PORT_ALARMING_STAT   0x1302 //同步报警状态


typedef struct t_ctlthreadinfo
{
	pthread_t  ptid_process;
	int  index;
	unsigned short uCmd;
	struct conntracker_st *cker;
	sem_t  sem;
	struct sockaddr_in in_client_addr;
	struct t_ctlthreadinfo *prev;
	struct t_ctlthreadinfo *next;
	//unsigned char delflag;       //删除标志
}T_CTLTHREADINFO, *PT_CTLTHREADINFO;
#define HEAD_LENGTH	40   
#define RECVBUF 512                        
#define DATA_SIZE (RECVBUF-HEAD_LENGTH)

struct _net_pack_
{
    char head[4];//WRTI    
    int len;
    unsigned short uCmd;
    char srcid[15];
    char dstid[15];
    char Data[DATA_SIZE];
};


#define defenceCfg "B:/defenceCfg"
#define ver_name "B:/version" 
#define TCP_PORT 20200
//T_SYSTEMINFO  g_SystemInfo;
//T_SYSTEMINFO      *pSystemInfo;

int udpSocketSend;//used by send defence situation to other indoor machines

//#define LOCALID  g_SystemInfo.LocalSetting.privateinfo.LocalID
//#define LOCALIP  g_SystemInfo.LocalSetting.privateinfo.LocalIP
//#define LOCALNETMASK  g_SystemInfo.LocalSetting.privateinfo.SubMaskIP
//#define LOCALGATEWAY  g_SystemInfo.LocalSetting.privateinfo.GateWayIP
/************************************
呼叫
//************************************/
#define SEND_CALL_TO_ROOM 1
#define GET_CALL_FROM_ROOM 2
#define GET_CALL_FROM_DOOR 3

//#define  INDOOR_MACHINE_IDLE			0
enum{
	INDOOR_MACHINE_IDLE=0,
	INDOOR_MACHINE_WORKING,
	
};

enum{
	SENDCALL_ROOM_CALL_ROOM_END=0,
	SENDCALL_ROOM_CALL_ROOM_START=200,
	SENDCALL_ROOM_CALL_ROOM_IDLE,
	
};
enum{
	SENDCALL_ROOM_WATCH_END=0,
	SENDCALL_ROOM_WATCH_START=100,
	SENDCALL_ROOM_WATCH_IDLE,
	
};

struct conntracker_st
{
#define  CNN_ST_801X_END				0
#define  CNN_ST_801X_TASK_START         1
#define  CNN_ST_801X_WAIT_ACK           2
#define  CNN_ST_801X_IDLE        		3
#define  CNN_ST_801_IDLE         		4
#define  CNN_ST_801_HOLDON              5  //801分机摘机
#define  CNN_ST_801X_HOLDON             6  //互联网摘机
#define  CNN_ST_801_MONI                7  //801分机监视
#define  CNN_ST_801X_MONI               8  //互联网监视  
#define  CNN_ST_801_BUSY         		9
#define  CNN_ST_801X_BUSY         		10
#define  CNN_ST_801X_TIMEOUT            11  //互联网超时  
#define  CNN_ST_801X_CONNECT_FAIL       12

#define  CNN_CALL_WAIT_ACK              	0x80
#define  CNN_CALL_ACK                  		0x81
#define  CNN_TALKING                        0x82
#define  CNN_CALL_END                       0X8F
 
	int state;//呼叫服务器状态记录
	int state_801;//原801系统状态记录
	int state_screen;//屏幕显示状态
	unsigned long srcidIP;//目标设备IP地址
	int fd;
	int pipe_fd[2];//use pipe to receive message from main function or send message to main function 
	short int handle;
	short int wait_flage;//监视时呼叫等待时间
	int timer;
    unsigned short cnn_cmd;
	//short int cnn_cmd;
	short int r2_room;	
	char srcid[16];
	char dstid[16];
	char localid[16];
	int forward;
	sem_t  sem;	
};
struct cnn_msg_st
{

#define CNNMSG_801_HOLDON   1
#define CNNMSG_HOST_HUANGUP 2 //*号挂机
#define CNNMSG_801_HANGUP   3
#define CNNMSG_MONI_END     4
#define CNNMSG_R2_RAW_DATA 5

    int cmd;
    unsigned char buf[12];
};
#define MQ_DATA_LEN 256
#define MQ_WRT_LEN (sizeof(struct mq_wrt_msg))
#define MQ_HEAD_LEN (MQ_WRT_LEN-MQ_DATA_LEN)
struct mq_wrt_msg
{
	unsigned long cmd;
	int len;
	char data[MQ_DATA_LEN];
};


//媒体协商
typedef struct _MediaNegotRes_t
{
    int index;
	int v_type;
	int ack_idle;
	int ack_MediaNegot;
	int timeout_MediaNegot;
	unsigned long ip;
}MediaNegotRes_t;
typedef struct{
	int v_enc_type;
	int v_dec_type;
	int v_enc_bitrate;
	int a_packtime;
	int a_samplechannel;//音频读写 channel、bit、rate使用同一参数
	int a_samplebit;
	int a_samplerate;
	int a_enc_type;
	int a_dec_type;
	int v_recvport;
	int a_recvport;
	int v_sendport;
	int a_sendport;
	unsigned short v_dec_resolution_w;
	unsigned short v_dec_resolution_h;
	unsigned short v_enc_resolution_w;
	unsigned short v_enc_resolution_h;
	unsigned int remote_platform_type;
}NEGOTIATE_RESULT;//64 byte 
typedef enum
{
	V_801A=0,
    V_NONE,
    V_MPEG4,       //is used by 801A
    V_H264,
    V_MPEG4_H264,
	V_XH264,//high-definition indoor machine
}V_TYEP;
#define ture 0
#define false -1
#define ERROR_TCP_SERVER_CALLBACK_FUNCTION_NOT_DEFINED				100//can not call main thread to do something

#define DEFAULT_MAX_CTRLTHREAD_NUM  200 //1023
#define DETECTING_CONTINUOUSLY1 100 //how many times should we detect continuously int step 1
#define DETECTING_CONTINUOUSLY2 10 //how many times should we detect continuously int step 2
#define REPORT_INTERVAL_TIME 40 //interval time of report to the center
#define REPORT_TIMES 2 //the times of report to center
#define DEFENCE_NUM  8
#define initInt -1
#define OTHER_INDOORMACHINE_NUM 4 //support 4 indoor machines in one room
#define DEFAULT_SMOOTHING_TIME 1000*600
#define SMOOTHING_FASTER	  1000*50
#define DEFENCE_GPIO_ALARM_Z 47
#define DEFENCE_GPIO_OUT_A 43
#define DEFENCE_GPIO_OUT_B 44
#define DEFENCE_GPIO_OUT_C 45
#define DEFENCE_GPIO_OUT_D 46 

#define MAX_ROOMNO_SIZE          15

/*
defence struct:add by work number 000223
*/
//struct defenceDetectingResult
//{
// int effectiveDetecting;//effective detecting times which can count smoothing times
// int effectiveTime;//effective detecting time which can tell when to alarm to the center
//};
//struct defenceDetectingResult detectingResult[DEFENCE_NUM];
//unsigned long otherIndoorMachineIP[OTHER_INDOORMACHINE_NUM];
//unsigned int otherIndoorMachineTableNum[OTHER_INDOORMACHINE_NUM];//use it when we send defences situation to other indoor machines 
//unsigned int callIndoorMachineTableNum[OTHER_INDOORMACHINE_NUM];//use it when we send call to up to 4 indoor machines
//extern T_DEFENCE_INFO_FOR_OTHER_ROOM defenceInfoOfRoom[OTHER_INDOORMACHINE_NUM][DEFENCE_NUM];//amount of defences
//extern T_DEFENCE_INFO	g_DefenceInfo[DEFENCE_NUM];



int ctrl_thread_info_init();
int ctrl_thread_info_del_tail();
int ctrl_thread_info_del(int sd_channel);
int ctrl_thread_info_add(int sd_channel,struct sockaddr_in c_addr);
int create_listen_thread();
void *taskprocess(void *pParam);
void wrt_main_000223(); //by编号000223员工
void *wrt_listen(void *pParam);        //have some problems
int getCall(T_CTLTHREADINFO *sockInformation,short int uCmd);
int initConntrackerBeforeCall();
int init_contracker();
int set_state_801x(int state);
static int send_cmd(int sd, unsigned short cmd, char *data, int datalen);
int room_idle(int fd);
//static int handler_addrtable(int sd, struct _net_pack_ *np, int usedbytes);
int ReadRoomIpTable();
int downLoadFromCenter(T_CTLTHREADINFO *sockInformation,struct _net_pack_ *np);
void WRT_init_defence();
static void EnterDefenceMutex();
static void  ExitDefenceMutex();
void write_defence_cfg_file();
static void read_defence_cfg_file();
static void handle_my_own_alarm();
void check_warning_queue(void* pv);
void defence_check_thread(void* pv);
int WRTD_get_warning_area_by_type(int type);
int init_defence_GPIO();
int WarningInit(int reflag,unsigned long centerip,unsigned long managerip,unsigned long warningip,unsigned long centerminorip,unsigned long warningminorip);
void WRT_init_defence();
//int initTcp(int (*tcpServerToMainCallBack)(cJSON *,cJSON **,char *,char**));
//void returnMsgProcessor(char **returnCjsonMsg,char *basalString,char *jsonString,int returnCode);
//int acquireNumberFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName);
//bool wrt_CalledAnswerButtonOnPress();//sandra
//bool wrt_CalledHangUpButtonOnPress();
//bool wrt_CalledUnlockButtonOnPress();
//bool call_get_roominfo(char * pData);//sandra
//bool call_get_name(char* pData);//sandra
#ifdef __cplusplus
}
#endif
#pragma pack()

