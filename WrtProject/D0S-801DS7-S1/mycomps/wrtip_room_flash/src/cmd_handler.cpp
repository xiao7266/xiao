/*******************************************************************
文件描述：
该文件实现了，WRTIP对讲机的分机的所有命令处理。
2007.11.16
1：添加了当包的大小大于10 小于256的处理。
2：添加了当所需要处理包大于256的时候，需要将该包缓存
，等到接收完所有的包后，再处理。
2007.12.3
1:添家协议的处理，处理：铃声下载，升级。公告，
户户通，系统信息，留言等。
2008.3.5     1：修改m_display回调函数的参数传递。
2008.9.17    新版的命令处理
2008.11.12   1：修改AddPacketList函数，发送长数据的缓冲处理
2：修改了当用户监视或者呼叫别的用户时，连接不通，而此时刚好有一路连接过来的时候
界面会刷回主界面的错误。
2009.3.30
1:添加了中心重置分机的命令。
2:修改了，中心获取分机信息以及设置分机信息。
未处理：
版本：   0.2
日期：   2007.10.25
作者：   李建文
*******************************************************************/
//#include <tmNxTypes.h>
//#include <pna.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <mtd/mtd-user.h>
#include <errno.h>

#include "public.h"
#include "wrt_log.h"
#include "cmd_handler.h"
#include "sock_handler.h"
//#include "warningHandler.h"

#include "media_negotiate.h"


#include "audiovideo.h"
#include "tmSysConfig.h"
#include "warningtask.h"

//#include "zlib.h"
#include "info_list_handler.h"
//#include "tmGpio485.h"
#include "smalldoor_handler.h"
#include "zenmalloc.h"
#include "enet/enet.h"
#include "RoomGpioUtil.h"
#include "tmWRTICaller.h"
//#include "Audio.h"
#include "sip_data_sources.h"
#include "httpclient.h"
#include "c_interface.h"

#ifdef HAVE_SUB_PHONE
#include "wrt_sub_phone.h"
#endif
#include "wrt_subroom.h"
#include "videocommon.h"

#include "wrt_audiostream.h"

typedef unsigned short  UInt16;

/////////////////////////////////////////////////////////////
//


#define DEBUG_TEST_MSG 1        /*测试短消息*/
#undef DEBUG_TEST_MSG

#define DEBUG_TEST_VIDEO_1 1   /*调试测试不停的播放视频*/
#undef  DEBUG_TEST_VIDEO_1

#define DEBUG_TEST_VIDEO_2 1
//#undef  DEBUG_TEST_VIDEO_2



#define REQUEST_IP 1         /*户户呼叫时，是否采用向中心请求IP*/
#undef  REQUEST_IP

#define  USE_CENTERMONT  0    /*测试启用中心监视*/

extern int g_dec_mode;

extern int g_phone_mont;
extern int g_phone_or_externdevice_holdon;
extern int   g_holdon_flag;

extern unsigned long g_cap_time;

//
////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
//房号表处理
/*
extern void init_idtoip();
extern void save_idtoip(unsigned char* buf,long buflen);
////////////////////////////////////////////////////
//   [in] id  输入参数
//   [out] ip  查找到数据存入ip
//   [in,out]  num 传入分配ip的个数，输出num得到的ip个数
extern void get_ipbyid(char* id,unsigned long* ip,int* num);
extern unsigned char* get_idtoipdata(int* len);
extern void get_id(char* p,int len);
extern void get_ipbyid2(char* id,unsigned long *ip);
*/
#include "idtoiptable.h"
//extern unsigned char button_sound1[9456]; //按键声音

#include "tmAppport.h"
extern int g_video_rec_flag;
#ifdef __cplusplus
extern "C"
{
#endif
        /*hander.h,wrtip/apps/wrtip_room_flash/handler.h,JiajuSetHandler.cpp*/
        // #define CHINESE 0x0
        //  #define ENGLISH 0x1

        void CreateDialogWin(char* text,int type);
        void toinet_addr(unsigned long ip_addr, unsigned char *localip);
        int send_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec,unsigned char status);
        unsigned char* get_jiaju_config_buf_for_center(int* len);
        int get_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec);
        int send_scene_mode(int mode);
        void set_security_status(int index,bool status);
        bool isenablesafe();
        bool get_security_status(int index);
        int GetIsHaveJiaju();
        void ReDrawCurrentWindow2(int type);
        void ReDrawCurrentWindow();
        void modifypwd();
        char* Get_jiaju_table_file(int *len);
        int Set_jiaju_table_file(unsigned char* buf,int buflen);

        void sync_pwd(unsigned char id,unsigned char* buf,int len);

         int GetCurrentLanguage();

#ifdef __cplusplus
}
#endif



#ifdef DOWNLOAD_FROM_HTTP
/*
unsigned long tid_download = -1;
unsigned long qid_download = -1;
void init_download_task();
void download_data_task();
*/
#endif



#define  MAX_MSG_BUFFER           400
#define  MP3_RING_STOP            0x888
#define  VIEW_TIME                0X889

#define  SMALL_LEAVE_START        0x890


extern T_SYSTEMINFO* pSystemInfo;

extern void Send_Test();

extern unsigned long  subroomipaddr[8];

static char msg_buffer[MAX_MSG_BUFFER] = {0};
static int m_call_police_count = 0;
static int m_smalldoor_cmd_count = 0;

static int g_smalldoor_call_agent = 0;
static int m_smalldoor_cmd= -1;
static unsigned char m_smalldoor_id = -1;

static int g_issmalldoor_hangup = 0;

static int g_simulate_talk = 0; //  [2011年3月21日 9:00:57 by ljw]
static int g_ismyholdon = 0; 
static int g_used_simulate_channel = 0;
int g_holdonbyme = 0;



static CallParam_t callparam[4];

static SDL_TimerID       m_ringtimerid = 0;                    //铃声计时器
static SDL_TimerID       m_talktimerid = 0;                    //谈话计时器
static SDL_TimerID       m_watchtimerid = 0;                   //监视计时器
static SDL_TimerID       m_waringtimerid = 0;                  //报警计时器
static SDL_TimerID       m_emergtimerid = 0;                   //紧急求助计时器
static SDL_TimerID       m_netchecktimerid = 0;                //接收网络消息的计时器
static SDL_TimerID       m_mp3timerid = 0;                     //短消息铃声的计时器
static SDL_TimerID       m_viewtimetimerid = 0;                //显示时间计时器
static SDL_TimerID       m_smalldoor_timerid = 0;
static SDL_TimerID       m_smalldoor_timerid2 = 0;
static SDL_TimerID       m_avoidtime_timerid = 0 ;              //免打扰计时器
static SDL_TimerID       m_agenttime_timerid = 0;               //托管计时器
static SDL_TimerID       m_unlock_timerid  = 0;                 //开锁，5秒挂机事件
static SDL_TimerID       m_doorleave_timerid = 0;
static SDL_TimerID       m_recordtimerid = 0;                   // 录音计时器

static SDL_TimerID       m_lowtension_timerid = 0; 		//欠压报警计时器

#ifdef HAVE_PLAY_JPEG
static SDL_TimerID       m_playjpegtimerid = 0;
#endif

static unsigned long       g_timeout_qid = -1;                   //计时任务的消息队列
static unsigned long       g_timeout_tid = -1;                   //计时任务的任务ID
static unsigned long       g_timeout_tid2 = -1;                   //触发任务2

static unsigned long       g_save_event[4];
static bool                is_small_call = false;

static unsigned long      g_ring_times = 25;                 //响铃声时间
static unsigned long      g_avoid_times = 0;                      //免打扰时间单位秒。
static unsigned long      g_agent_times = 0;                       //托管的时间单位秒
static int      g_record_time = 20;                      //录音时间。


static int                g_save_waring_start = 0;               //保存启动报警演示的位置。0，布防，1，有报警的时间触发。

static int                g_vod_select =  -1;

extern CCmdHandler*      m_pCmdHandler;
extern CSocketManager*   m_pSocketManager;
//extern unsigned long     socketmanager_task_id;
extern int g_alarmtype;
extern bool g_is_talk;

int    g_localid =-1;

WRTI_CallerSetup_t    callersetup;

static int m_is_transmit_to_phone = 0; //2010/10/12
static int m_is_phone_to_transmit = 0; //2010/10/12
static int m_is_phone_holdon = 0; //2010/10/14
static int m_is_phone_call_room = 0 ; //2020/10/14

int g_phone_connect_index = -1;
static int g_is_leave = 0;

#ifdef HAVE_PLAY_JPEG
        unsigned long g_framenum = 0;
#endif
/****************************************************
手机处理回调函数
****************************************************/
#ifdef HAVE_SUB_PHONE
void  phone_callback(unsigned long msg_buf[4])
{
	if(m_pCmdHandler)
		m_pCmdHandler->msg_from_phone(msg_buf);
}
#endif

int _iswall(char* input){
	char* tmpbuf="00000000";        
	char* tmpbuf2 = (char*)(input+13);        
	if((memcmp(input+2,tmpbuf,8) == 0) && (strcmp(tmpbuf2,"00") != 0) )                
		return 1;        
	return 0;
}

int GetTransmitToPhone()
{
	return m_is_transmit_to_phone;
}

void SetTransmitToPhone(int param)
{
	m_is_transmit_to_phone = param;
}

/****************************************************
*tmEnetUDP 接收回调函数
*****************************************************/
void  app_enet_recv(unsigned long ip,short port,void* buf,int len)
{
        if(m_pCmdHandler){
                unsigned char* tmpbuf = (unsigned char*)ZENMALLOC(len);
                if(tmpbuf == NULL)
                        return;
                memcpy(tmpbuf,buf,len);
                unsigned long tmpevent[4];
                tmpevent[0] = ip;
                tmpevent[1] = (unsigned long)tmpbuf;
                tmpevent[2] = len;
                tmpevent[3] = 0;
                int msgtype = MSG_PROCESS_UDP_MSG;
                m_pCmdHandler->Send_msg(msgtype,tmpevent,sizeof(tmpevent));

                //m_pCmdHandler->ProcessUDP(ip,(unsigned char*)buf,len);
        }
        //SDL_Delay(1000);
}

/****************************************************
*wrticaller 回调函数
****************************************************/

static int save_status = -1;
static int save_status1 = -1;

void caller_showmsg(int type,const char* str)
{
        WRT_MESSAGE("呼叫库回调消息type = %d",type);
        unsigned long event[4];
        memset(event,0,sizeof(event));
        if(m_pCmdHandler == NULL)
                return;
        switch(type)
        {
        case MSGID_DEVICE_CONNECTING: //正在连接....
                if(m_is_phone_to_transmit == 0){
                        event[0] = STATUS_CONNECTING;
                        m_pCmdHandler->CallDisplayCallBack(event);
                }
                break;
        case MSGID_CONNECTED_ERROR: //连接错误
                save_status = STATUS_NET_EXCEPTION;
                break;
        case MSGID_WAIT_CALL_REQUEST_ACK://等待目标响应
                break;
        case MSGID_CENTER_BUSY: //中心繁忙
                save_status = STATUS_CENTER_BUSY;
                break;
        case MSGID_CENTER_AGENT_CONNECTING://中心已经托管，转接中...
                break;
        case MSGID_ROOM_QUIET_MODE://房间已经设置免打扰
                save_status = STATUS_ROOM_BUSY;
                break;
        case MSGID_ROOM_AGENT_CONNECTING: //房间已经托管，转接中...
                break;
        case MSGID_ROOM_BUSY: //房间繁忙
                save_status = STATUS_ROOM_BUSY;
                break;
        case MSGID_NO_CALL_REQUEST_ACK: //目标无应答
                save_status = STATUS_NET_EXCEPTION;
                break;
        case MSGID_TALKING_TO_CENTER: //与中心通话中...
                if(m_is_phone_to_transmit == 0){
                        event[0] = STATUS_ROOM_TALK_ROOM;
                        m_pCmdHandler->CallDisplayCallBack(event);
                }
                break;
        case MSGID_CMD_REST_ROOM_PASSWORD: //重置分机密码
                if(m_is_phone_to_transmit == 0)
                {
                        event[0] = CENTER_RESET_ROOM_PASS;
                        event[1] = event[2] = event[3] = -1;
                        m_pCmdHandler->Send_cmd(event);
                }
                break;
        case MSGID_CMD_ROOM_CALL_TIMEOVER:
        case MSGID_CMD_ROOM_NO_HOLDON:
        case MSGID_CMD_CENTER_CALL_TIMEOVER:
        case MSGID_CMD_CENTER_NO_HOLDON:
        case MSGID_CMD_ROOM_HUNGUP:
                save_status =STATUS_ROOM_HANGUP;
                break;
        case MSGID_CMD_CENTER_HUNGUP:
                save_status = STATUS_CENTER_HANGUP;
                break;
        case MSGID_SEND_CALL_REQUEST_ERROR:
        case MSGID_CMD_DATA_ERROR: //数据错误
                save_status = STATUS_NET_EXCEPTION;
                break;
        case MSGID_CENTER_DIVERT_CENTER:  //中心正转接至中心!
                save_status1 = 2;
                break;
        case MSGID_CENTER_DIVERT_ROOM:   //中心转接到分机
                save_status1 = 3;
                break;
        case MSGID_CENTER_MINOR_CONNECTING:
                break;
        case MSGID_CENTER_DIVERT_TIMEOUT:
        case MSGID_ROOM_DIVERT_TIMEOUT:
                save_status =STATUS_ROOM_HANGUP;
                break;
        case MSGID_ROOM_DIVERT_ROOM://中心正在转接中心
                event[0] = CENTER_DIVERT_ROOM;
                event[1] = event[2] = event[3] = -1;
                m_pCmdHandler->Send_cmd(event);
                break;
        case MSGID_ROOM_DIVERT_CENTER://中心正在转接分机
                event[0] = CENTER_DIVERT_CENTER;
                event[1] = event[2] = event[3] = -1;
                m_pCmdHandler->Send_cmd(event);
                break;
        default:
                break;
        }

}

void caller_ringon()
{
        if(m_pCmdHandler)
        {
                unsigned long event[4];
                event[0] = CUSTOM_CMD_CALLER_RINGON;
                event[1] = event[2] = event[3] =0;
                m_pCmdHandler->Send_cmd(event);
        }
}

void caller_ringoff()
{
        if(m_pCmdHandler)
        {
                unsigned long event[4];
                event[0] = CUSTOM_CMD_CALLER_RINGOFF;
                event[1] = event[2] = event[3] =0;
                m_pCmdHandler->Send_cmd(event);
        }
}

void caller_holdon(unsigned long param){
        if(m_pCmdHandler)
        {
                unsigned long event[4];
                event[0] = CUSTOM_CMD_CALLER_HOLDON;
                event[1] = param;
                event[2] = event[3] =0;
                m_pCmdHandler->Send_cmd(event);
        }
}

void caller_callend(unsigned long param)
{
        /*   printf("caller_callend  = %d \n",save_status1);
        if(save_status1 == 2 || save_status1 == 3){
        save_status = -1;
        save_status1 = -1;
        talktostop();
        talkstop();
        return;
        }

        save_status = 0;
        */
}

void caller_prep()
{
        WRT_MESSAGE("呼叫库 caller_prep save_status = %d ",save_status);
        /*  static int m_once =0;
        if(m_once == 0){
        m_once = 1;
        return;
        }
        */
        //if(m_pCmdHandler && save_status != -1)
        {
                unsigned long event[4];
                event[0] = CUSTOM_CMD_CALLER_CALLEND;
                event[1] = save_status;
                event[2] = event[3] =0;
                m_pCmdHandler->Send_cmd(event);
                save_status = -1;
        }
}


/*******************************************
设置响铃的时间
*******************************************/
void CCmdHandler::SetRingtimes(unsigned long times){
        static int first_set_ring_time = 0;
        if(times == 0)
                g_ring_times = 25;
        else
        {
                if(times < 25)
                        g_ring_times = 25;
                else if(times > 60)
                        g_ring_times = 60;
                else
                        g_ring_times = times;
        }
        if(first_set_ring_time == 0){
                first_set_ring_time = 1;
                if(g_localid != 0x01){ //如果是从分机第一次执行该函数，则不发送同步消息
                        return;
                }
        }
        int msgtype =MSG_NODE_RECV_UDP_MSG ;
        unsigned long tmpmsg[4];
        tmpmsg[0] = ROOM_SYNC_RING_TIME;
        tmpmsg[1] = 0;
        tmpmsg[2] = g_ring_times;
        tmpmsg[3] = 4;
        m_pCmdHandler->Send_msg(msgtype,(void*)tmpmsg,sizeof(tmpmsg));

}

/*******************************************
获得响铃的时间
*******************************************/
unsigned long CCmdHandler::GetRingtimes(){
        return g_ring_times;
}

/**
* 免打扰时间设置
*/
void CCmdHandler::SetAvoidtimes(unsigned long hours,unsigned long min){

        static int first_set_avoid_time = 0;
        int msgtype =MSG_NODE_RECV_UDP_MSG ;
        unsigned long tmpmsg[4];
        if(first_set_avoid_time == 0){
                first_set_avoid_time = 1;
                if(g_localid != 0x01){//如果是从分机第一次执行该函数，则不发送同步消息
                        return;
                }
        }

        if(hours == 0 && min == 0){
                g_avoid_times=0;
     
                add_and_del_timer(TIME_AVOID_SETTIMES_EVENT_STOP);

                tmpmsg[0] = ROOM_CANCEL_SYNC_AVOID;
                tmpmsg[1] = 0;
                tmpmsg[2] = 0;
                tmpmsg[3] = 0;
                m_pCmdHandler->Send_msg(msgtype,(void*)tmpmsg,sizeof(tmpmsg));
        }else{
                g_avoid_times = hours*3600+min*60;
                //向计时任务发送消息，启动计时器
                WRT_MESSAGE("SetAvoidtimes =%d.%d",hours,min);

                add_and_del_timer(TIME_AVOID_SETTIMES_EVENT_START);
                tmpmsg[0] = ROOM_SYNC_AVOID;
                tmpmsg[1] = 0;
                tmpmsg[2] = g_avoid_times;
                tmpmsg[3] = 4;
                m_pCmdHandler->Send_msg(msgtype,(void*)tmpmsg,sizeof(tmpmsg));
        }

};

/**
*托管时间设置
*/
void CCmdHandler::SetAgenttimes(unsigned long hours,unsigned long min){

        static int first_set_agent_time = 0;
        int msgtype =MSG_NODE_RECV_UDP_MSG ;
        unsigned long tmpmsg[4];
        if(first_set_agent_time == 0){
                first_set_agent_time = 1;
                if(g_localid != 0x01){//如果是从分机第一次执行该函数，则不发送同步消息
                        return;
                }
        }
        if(hours == 0 && min == 0){
                g_agent_times=0;
                add_and_del_timer(TIME_AGENT_SETTIMES_EVENT_STOP);
                tmpmsg[0] = ROOM_CANCEL_SYNC_AGENT;
                tmpmsg[1] = 0;
                tmpmsg[2] = 0;
                tmpmsg[3] = 0;
                m_pCmdHandler->Send_msg(msgtype,(void*)tmpmsg,sizeof(tmpmsg));

        }else{
                g_agent_times = hours*3600+min*60;
                //向计时任务发送消息，启动计时器

                 add_and_del_timer(TIME_AGENT_SETTIMES_EVENT_START);
                tmpmsg[0] = ROOM_SYNC_AGENT;
                tmpmsg[1] = 0;
                tmpmsg[2] = g_agent_times;
                tmpmsg[3] = 4;
                m_pCmdHandler->Send_msg(msgtype,(void*)tmpmsg,sizeof(tmpmsg));
        }
}

/**
*获得托管时间
*/
void CCmdHandler::GetAgenttimes(unsigned long* hours,unsigned long * min){
        *hours = g_agent_times/3600;
        *min = (g_agent_times%3600)/60;
}


/**
* 获得免打扰时间设置
*/
void CCmdHandler::GetAvoidtimes(unsigned long * hours,unsigned long* min){
        *hours = g_avoid_times/3600;
        *min = (g_avoid_times%3600)/60;
}

/*
*定时器回调函数系列
*/
static Uint32 display_time_callback(Uint32 interval,void* pvoid)
{
        unsigned long msg[4];
		WRT_DEBUG("定时器事件回调函数");
        if(m_pCmdHandler){
                msg[0] = STATUS_ROOM_VIEW_TIME;
                msg[1] = msg[2] = msg[3] = 0;
                m_pCmdHandler->CallDisplayCallBack(msg);

        }
        return interval;
}
static Uint32 check_homestatus_callback(Uint32 interval,void* pvoid)
{
         unsigned long msg[4];
        if(m_pCmdHandler){
                msg[0] = STATUS_CHECK_JIAJU;
                msg[1] = msg[2] = msg[3] = 0;
                m_pCmdHandler->CallDisplayCallBack(msg);

        }
        return interval;
}

static Uint32 play_jpeg_callback(Uint32 interval,void* pvoid)
{

        if(m_pCmdHandler){
                unsigned char* data = NULL;
                unsigned long event[4];
                event[2] = get_vod_frame(&data,g_vod_select,g_framenum);
                if(event[2] > 0){
                        event[0] = STATUS_PLAY_JPEG;
                        event[1] = (unsigned long)data;
                        event[3] = 0;
                        m_pCmdHandler->CallDisplayCallBack(event);
                        g_framenum++;
                        if(g_framenum > (pSystemInfo->mySysInfo.framerate * CAP_FRAME_TIME - 1)){
                                                             
                                m_playjpegtimerid  = 0;
                                g_framenum = 0;
                                g_vod_select = -1;
                                return 0;
                        }
                }else{
                                                 
                        m_playjpegtimerid  = 0;
                        g_framenum = 0;
                        g_vod_select = -1;
                        
                        return 0;
                }

        }
        return interval;
}


static Uint32 record_event_callback(Uint32 interval,void* pvoid)
{
        if(m_pCmdHandler){
                unsigned long tmpevent[4] ={0};
                g_record_time --;
                if(g_record_time < 0){
                        tmpevent[0] = CUSTOM_CMD_ROOM_STOP_RECORD;
                        m_pCmdHandler->Send_cmd(tmpevent);
                        printf("send CUSTOM_CMD_ROOM_STOP_RECORD \n");
                        m_recordtimerid   = 0;
                        return 0;
                }else{
                        tmpevent[0] = STATUS_RMCORD_TIME;
                        tmpevent[1] = g_record_time;
                        m_pCmdHandler->CallDisplayCallBack(tmpevent);
                }
                return interval;

        }
        m_recordtimerid = 0;
        return 0;
        
}

static Uint32 lowtension_warning_event_callback(Uint32 interval,void* pvoid)
{
	WRT_DEBUG("重发欠压报警");
	if(g_alarmtype != 0){
		SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,g_alarmtype); 
		// 2011-9-29 10:26:09,添加简易语音提示 “嘀两声".
		if(m_pCmdHandler)
			m_pCmdHandler->playvoice2(WARNING_RING,0);
	        return interval;
	}
	m_lowtension_timerid = 0;
	return 0;
}

/**
*计时器任务函数
*/

void  add_and_del_timer2(unsigned long evt){

#if 1 //家居检测
                        if(evt & TIME_JIAJU_CHECK_START){
                        	m_netchecktimerid = SDL_AddTimer(60000,check_homestatus_callback,NULL);
                        	return;
                        }
                        if(evt & TIME_JIAJU_CHECK_STOP){
                                if(m_netchecktimerid != 0)
                                       SDL_RemoveTimer(m_netchecktimerid);
                                m_netchecktimerid = 0;
                                return;
                        }
#endif
                        if(evt & TIME_DISPLAY_TIME_EVENT_START){
                        	WRT_DEBUG("开启事件定时器");
                        	m_viewtimetimerid = SDL_AddTimer(60*1000,display_time_callback,NULL);
                        	
                        	return;
                        }

#ifdef HAVE_PLAY_JPEG
                        if(evt & TIME_PLAY_JPEG_START){
  				if(m_playjpegtimerid != 0)
                                        SDL_RemoveTimer(m_playjpegtimerid);
                                m_playjpegtimerid = 0;
                                
                                int count = pSystemInfo->mySysInfo.framerate * CAP_FRAME_TIME;
                                if(count > 60 || count < 30)
                                        count = 30;
                                int frametime = (CAP_FRAME_TIME*1000)/count;
                                m_playjpegtimerid  = SDL_AddTimer(frametime,play_jpeg_callback,NULL);
                                return;
                        }
      
                        if(evt & TIME_PLAY_JPEG_STOP){
                                if(m_playjpegtimerid != 0)
                                        SDL_RemoveTimer(m_playjpegtimerid);
                                m_playjpegtimerid  = 0;
                                g_framenum = 0;
                                g_vod_select = -1;
                                return;
                        }
#endif
                        if(evt & TIME_RECORD_EVENT_START){
                                WRT_MESSAGE("启动录音计时器");
                                g_record_time = 20;
                              //  tm_evevery(1000,TIME_RECORD_EVENT_TIMEOUT,&m_recordtimerid);
                                m_recordtimerid = SDL_AddTimer(1000,record_event_callback,NULL);
                                return;
                        }                       
                        if(evt & TIME_RECORD_EVENT_STOP){
                                if(m_recordtimerid != 0){
                                        SDL_RemoveTimer(m_recordtimerid);
                                        m_recordtimerid   = 0;
                                        WRT_MESSAGE("收到停止录音事件，停止录音");
                                }
                                return;
                        }
                        
                        if(evt & TIME_LOWTENSION_WARNING_EVENT_START){
                        	WRT_DEBUG("启动欠压报警重发机制");
                        	m_lowtension_timerid = SDL_AddTimer(4*60*1000,lowtension_warning_event_callback,NULL);
                        	return;
                        	
                        }
 
                        if(evt & TIME_LOWTENSION_WARNING_EVENT_STOP){
   					if(m_lowtension_timerid != 0){
   					WRT_DEBUG("停止欠压报警重发机制");
                                        SDL_RemoveTimer(m_lowtension_timerid);
                                        m_lowtension_timerid   = 0;                        	
                                }
                                return;
                        }

}


void send_msg_to_qid(unsigned long id,int cmd){
        unsigned long tmpmsgbuf[4];
        tmpmsgbuf[0] = cmd;
        tmpmsgbuf[1] = tmpmsgbuf[2] = tmpmsgbuf[3] = 0;
     //   q_send(id,tmpmsgbuf);
}
/************************************************************************/
/* 定时器回调函数                                                        */
/************************************************************************/
static Uint32 mont_event_callback(Uint32 Interval,void* pvoid)
{
        unsigned long event[4];
        if(m_pCmdHandler){
                event[0] = ROOM_STOPMONT;
                event[1] = event[2] = event[3] = 0;
                m_pCmdHandler->Send_cmd(event);
        }
        m_watchtimerid  = 0;
        return 0;
}

static Uint32 mont2_event_callback(Uint32 Interval,void* pvoid)
{
        unsigned long event[4];
        if(m_pCmdHandler){
                event[0] = IPCAMERA_MONITOR_END;
                event[1] = event[2] = event[3] = 0;
                m_pCmdHandler->Send_cmd(event);
        }
        m_watchtimerid  = 0;
        return 0;
}
static Uint32 ring_event_callback(Uint32 Interval,void* pvoid)
{
  		
        unsigned long event[4];
		if(m_pCmdHandler){ //当无人接听，如果是小门口机呼叫，则发送命令是否需要留言。
	
	        if(m_pCmdHandler->GetCurrentSysStatus() == SYS_SMALLDOORCALL ){
	                if(g_localid == 0x01){
	                	
	                	event[0] = CUSTOM_CMD_LEAVE;
	                        event[1] = event[2] = event[3] = 0;
	                        m_pCmdHandler->Send_cmd(event);
	                        WRT_MESSAGE("响铃计时，时间到，小门口呼叫，启动小门口机留言");
	                        
	                	event[0] = ROOM_SEND_COLOR_MSG;
	                	event[1] = event[2] = event[3] = 0;
	                	m_pCmdHandler->Send_cmd(event); 
	                	WRT_MESSAGE("小门口机呼叫，响铃时间到，通知中心发送彩信");
	                	
	                        
	                }else{
	                        //非主分机不能留言
	                        if(m_pCmdHandler->GetHostRoomBusy()){ //如果其他分机繁忙 2009-7-16
	                                event[0] = ROOM_HANGUP;
	                                event[1] = 10;
	                                event[2] = event[3] = 0;
	                                m_pCmdHandler->Send_cmd(event);
	                                WRT_MESSAGE("响铃计时，时间到，小门口呼叫，多分机配置，由本机发送挂机消息");
	                        }else{
	                                event[0] = SMALLDOOR_ACK_HANGUP;
	                                event[1] = 2;
	                                event[2] = event[3] = 0;
	                                m_pCmdHandler->Send_cmd(event);
	                                WRT_MESSAGE("响铃计时，时间到，小门口呼叫，销毁自身资源");
	                        }
	
	                }
	
	        }else{
	
	                if(m_pCmdHandler->GetCurrentSysStatus()  == SYS_TALKTOMAIN){
	                        WRT_MESSAGE("响铃计时，时间到，主机呼叫，挂机");  
	                        event[0] = ROOM_HANGUP;
	                        event[1] = 8;
	                        event[2] = event[3] = 0;
	                        m_pCmdHandler->Send_cmd(event);
	                        
	                        if(g_localid == 0x01){
								event[0] = ROOM_SEND_COLOR_MSG;
	                			event[1] = event[2] = event[3] = 0;
	                			m_pCmdHandler->Send_cmd(event); 
	                			WRT_MESSAGE("主机呼叫，响铃时间到，通知中心发送彩信");
	                        }
	
	                }else if(m_pCmdHandler->GetCurrentSysStatus()  == SYS_ROOMCALLCENTERTALK){
	                        if(g_smalldoor_call_agent == 1){
	                                WRT_MESSAGE("响铃计时,时间到,托管呼叫，挂机事件CENTER_HANGUP");
	                                event[0] = CENTER_HANGUP;
	                        }else
	                                event[0] = ROOM_HANGUP;
	                        event[1] = 9;
	                        event[2] = event[3] = 0;
	                        m_pCmdHandler->Send_cmd(event);
	                }else{
	                        WRT_MESSAGE("响铃计时,时间到,挂机时间ROOM_HANGUP");
	                        event[0] = ROOM_HANGUP;
	                        event[1] = 1;
	                        event[2] = event[3] = 0;
	                        m_pCmdHandler->Send_cmd(event);
	
	                }
	        }
	}
	m_ringtimerid = 0;
        return 0;
}
static Uint32 unlock_event_callback(Uint32 Interval, void* pvoid)
{
        unsigned long event[4];
	WRT_MESSAGE("开锁计时时间到，自动挂机");
	if(m_pCmdHandler){
	        if(g_issmalldoor_hangup == 0)
	                event[0] = ROOM_HANGUP;
	        else
	                event[0] = SMALLDOOR_ACK_HANGUP;
	                
	        if(g_simulate_talk == 1)
	                event[0] = SIMULATE_HANGUP;
	                
	        event[1] = 2;
	        event[2] = 0;
	        event[3] = 0x03;
	        m_pCmdHandler->Send_cmd(event);
	}
	m_unlock_timerid = 0;
    return 0;

}
static Uint32 talk_event_callback(Uint32 Interval,void* pvoid)
{
        unsigned long event[4];
	WRT_MESSAGE("通话计时时间到，自动挂机");
	if(m_pCmdHandler){
	
	        event[0] = ROOM_HANGUP;
	        if(g_simulate_talk == 1)
	                event[0] = SIMULATE_HANGUP;                                        
	        event[1] = 2;
	        event[2] = event[3] = 0;
	        m_pCmdHandler->Send_cmd(event);
	}
	m_talktimerid = 0;
        return 0;
}
static Uint32 mp3_event_callback(Uint32 Interval,void* pvoid)
{
        unsigned long event[4];
        if(m_pCmdHandler){
                event[0] = MP3_RING_STOP;
                event[1] = event[2] = event[3] = 0;
                m_pCmdHandler->Send_cmd(event);
        }
        m_mp3timerid = 0;

        return 0;
}
static Uint32 smalldoor_leave_event_callback(Uint32 Interval,void *pvoid)
{
        unsigned long event[4];
        if(m_pCmdHandler->GetCurrentSysStatus() == SYS_SMALLDOORCALL ){

                event[0] = SMALL_LEAVE_START;
                event[1] = event[2] = event[3] = 0;
                m_pCmdHandler->Send_cmd(event);
                // add_and_del_timer(TIME_DOOR_LEAVE_EVENT_START);
        }

        return 0;
}
static Uint32 avoid_event_callback(Uint32 Interval,void *pvoid)
{
        OpenAndCloseRing(false);
        if(m_pCmdHandler)
                m_pCmdHandler->avoidtimeout(0);
        g_avoid_times = 0;
        m_avoidtime_timerid = 0;
        return 0;

}

static Uint32 agent_settimes_event_callback(Uint32 interval,void *pvoid)
{
        if(m_pCmdHandler)
                m_pCmdHandler->avoidtimeout(1);
        g_agent_times = 0;
        m_agenttime_timerid = 0;
        return 0;

}

static Uint32 door_leave_event_callback(Uint32 interval,void* pvoid)
{
        unsigned long event[4];

        event[0] = ROOM_HANGUP;
        event[1] = 4;
        event[2] = event[3] = 0;
        m_pCmdHandler->Send_cmd(event);
        m_doorleave_timerid = 0;
        return 0;

}

static Uint32 warning_event_callback(Uint32 interval,void* pvoid)
{
        unsigned long event[4];
		event[0] = STATUS_START_WARNING;
		event[1] = g_save_waring_start;
		event[2] = event[3] = 0;
		WRT_MESSAGE("报警 延时时间到");
		m_pCmdHandler->CallDisplayCallBack(event);
        m_waringtimerid = 0;
        return 0;

}
/*******************************************************************
函数名称：time_handler_task
函数功能：计时任务函数，进入点.
输入参数：
输出参数：
返回值：
********************************************************************/
void add_and_del_timer(unsigned long evt){

                        if(evt & TIME_MONT2_EVENT_START){
                                if(m_watchtimerid != 0) {
                                        SDL_RemoveTimer(m_watchtimerid);
                                }
                                WRT_MESSAGE("监视计时开始");
                                m_watchtimerid = 0;
                                m_watchtimerid = SDL_AddTimer(30*1000,mont2_event_callback,NULL);
                                return;
                        }
                        if(evt & TIME_MONT2_EVENT_STOP){
                                if(m_watchtimerid != 0){
                                        SDL_RemoveTimer(m_watchtimerid);
                                        WRT_MESSAGE("停止监视计时");
                                }
                                m_watchtimerid = 0;
                                return;
                        }
                        if(evt & TIME_MONT_EVENT_START){
                                if(m_watchtimerid != 0) {
                                        SDL_RemoveTimer(m_watchtimerid);
                                }
                                WRT_MESSAGE("监视计时开始");
                                m_watchtimerid = 0;
                                m_watchtimerid = SDL_AddTimer(30*1000,mont_event_callback,NULL);
                                return;
                               
                        }
                        if(evt & TIME_MONT_EVENT_STOP){
                                if(m_watchtimerid != 0){
                                        SDL_RemoveTimer(m_watchtimerid);
                                        WRT_MESSAGE("停止监视计时");
                                }
                                
                                m_watchtimerid = 0;
                                return;
                        }
                        if(evt & TIME_RING_EVENT_START){
                                if(m_ringtimerid != 0){
                                        SDL_RemoveTimer(m_ringtimerid);
                                }
                                m_ringtimerid = 0;
								if(m_doorleave_timerid != 0)
										SDL_RemoveTimer(m_doorleave_timerid);
								m_doorleave_timerid = 0;
                                WRT_MESSAGE("响铃计时开始");
                                if(g_ring_times <= 30)
                                	m_ringtimerid = SDL_AddTimer(30*1000,ring_event_callback,NULL);
                                else
	                                m_ringtimerid = SDL_AddTimer(g_ring_times*1000,ring_event_callback,NULL);
                                //printf("响铃计时开始 %d\n",g_ring_times*1000);
                                return;
                        }
                        if(evt & TIME_RING_EVENT_STOP){
                                if(m_ringtimerid != 0){
                                        SDL_RemoveTimer(m_ringtimerid);
                                        WRT_MESSAGE("停止响铃计时");
                                }
                                m_ringtimerid = 0;
                                return;
                        }
                        if(evt & TIME_UNLOCK_EVENT_START){
                                if(m_unlock_timerid != 0)
                                        SDL_RemoveTimer(m_unlock_timerid);
                                m_unlock_timerid  = 0;
                                WRT_MESSAGE("启动开锁计时");
                                m_unlock_timerid = SDL_AddTimer(5*1000,unlock_event_callback,NULL);
                                return;
                              //  tm_evafter(5*1000,TIME_UNLOCK_EVENT_TIMEOUT,&m_unlock_timerid);
                        }

                        if(evt & TIME_UNLOCK_EVENT_STOP){
                                if(m_unlock_timerid != 0){
                                        SDL_RemoveTimer(m_unlock_timerid);
                                        WRT_MESSAGE("停止开锁计时");
                                }
                                m_unlock_timerid  = 0;
                                return;
                                
                        }
                        if(evt & TIME_TALK_EVENT_START){
                                if(m_talktimerid != 0){
                                        SDL_RemoveTimer(m_talktimerid);
                                }
                                m_talktimerid = 0;
                                WRT_MESSAGE("启动通话计时");
                                  m_talktimerid = SDL_AddTimer(120*1000,talk_event_callback,NULL);
                                  return;

                        }
                        if(evt  & TIME_TALK_EVENT_STOP){
                                if(m_talktimerid != 0){
                                        SDL_RemoveTimer(m_talktimerid);
                                        WRT_MESSAGE("停止通话计时");
                                }
                                m_talktimerid = 0;
                                return;
                        }
                        if(evt & TIME_MP3_EVENT_START){
                                if(m_mp3timerid != 0){
                                        SDL_RemoveTimer(m_mp3timerid);
                                }
                                m_mp3timerid = 0;
                                WRT_MESSAGE("启动MP3播放计时");
                                m_mp3timerid = SDL_AddTimer(300*1000,mp3_event_callback,NULL);
                                return;
                        }
                        if(evt & TIME_MP3_EVENT_STOP){
                                if(m_mp3timerid != 0){
                                        SDL_RemoveTimer(m_mp3timerid);
                                        WRT_MESSAGE("停止MP3播放计时");
                                }
                                m_mp3timerid =0;
                                return;
                        }
                        if(evt & TIME_SMALLDOOR_LEAVE_EVENT_START ){
                                if(m_smalldoor_timerid2 != 0){
                                        SDL_RemoveTimer(m_smalldoor_timerid2);
                                }
                                m_smalldoor_timerid2 =0;
                                WRT_MESSAGE("启动小门口机留言声音播放计时");
                                m_smalldoor_timerid2 = SDL_AddTimer(5*1000,smalldoor_leave_event_callback,NULL);
                                return;
                        }
                        if(evt & TIME_SMALLDOOR_LEAVE_EVENT_STOP){
                                if(m_smalldoor_timerid2 != 0){
                                        SDL_RemoveTimer(m_smalldoor_timerid2);
                                        WRT_MESSAGE("停止小门口机留言声音播放计时");
                                }
                                m_smalldoor_timerid2 = 0;
                                return;
                        }
                        if(evt & TIME_AVOID_SETTIMES_EVENT_START){
                                if(m_avoidtime_timerid != 0){
                                        SDL_RemoveTimer(m_avoidtime_timerid);
                                }
                                m_avoidtime_timerid = 0;
                                WRT_MESSAGE("启动免打扰计时");
                                m_avoidtime_timerid = SDL_AddTimer(g_avoid_times*1000,avoid_event_callback,NULL);
                                return;
                              //  tm_evafter(g_avoid_times*1000,TIME_AVOID_SETTIMES_EVENT_TIMEOUT,&m_avoidtime_timerid);
                               // timecount++;
                        }
                        if(evt & TIME_AVOID_SETTIMES_EVENT_STOP){
                                if(m_avoidtime_timerid != 0){
                                        SDL_RemoveTimer(m_avoidtime_timerid);
                                        WRT_MESSAGE("取消免打扰计时");
                                }
                                m_avoidtime_timerid = 0;
                                if(g_avoid_times != 0){

                                        g_avoid_times = 0;
                                        OpenAndCloseRing(false);
                                        if(m_pCmdHandler)
                                                m_pCmdHandler->avoidtimeout(0);
                                }
                                return;
                        }
                        if(evt & TIME_AGENT_SETTIMES_EVENT_START){
                                if(m_agenttime_timerid != 0){
                                        SDL_RemoveTimer(m_agenttime_timerid);
                                }
                                m_agenttime_timerid = 0;
                                WRT_MESSAGE("启动托管计时");
                                m_agenttime_timerid = SDL_AddTimer(g_agent_times*1000,agent_settimes_event_callback,NULL);
                                return;
                        }
  
                        if(evt & TIME_AGENT_SETTIMES_EVENT_STOP){
                                if(m_agenttime_timerid != 0){
                                        SDL_RemoveTimer(m_agenttime_timerid);
                                        WRT_MESSAGE("取消托管计时");
                                }
                                m_agenttime_timerid = 0;
                                if(g_agent_times != 0){
                                        g_agent_times = 0;
                                        if(m_pCmdHandler)
                                                m_pCmdHandler->avoidtimeout(1);
                                }
                                return;
                        }
                        if(evt & TIME_DOOR_LEAVE_EVENT_START){
                                WRT_MESSAGE("启动主机留言计时");
                                m_doorleave_timerid = SDL_AddTimer(25*1000,door_leave_event_callback,NULL);
                                return;
                              //  tm_evafter(20*1000,TIME_DOOR_LEAVE_EVENT_TIMEOUT,&m_doorleave_timerid);
                        }
                        if(evt & TIME_DOOR_LEAVE_EVENT_STOP){
                                if(m_doorleave_timerid != 0){
                                        SDL_RemoveTimer(m_doorleave_timerid);
                                        WRT_MESSAGE("停止主机留言计时");
                                }
                                m_doorleave_timerid = 0;
                                return;
                        }
                        if(evt & TIME_WARNING_EVENT_START){
                                if(m_waringtimerid != 0){
                                        SDL_RemoveTimer(m_waringtimerid);
                                        m_waringtimerid = 0;
                                }
                                if(m_waringtimerid == 0){
                                        int times = 0;
                                        if(g_save_waring_start == 0){
                                                times =  pSystemInfo->mySysInfo.alarmtime*1000; //布防延时 默认100S
                                                WRT_MESSAGE("开始布防，延时时间为%d ",times);
                                        }else{
                                                times = pSystemInfo->mySysInfo.alarmtime1*1000;//撤防延时默认40s
                                                WRT_MESSAGE("有报警，延时报警时间为%d ",times);
                                        }
                                        m_waringtimerid = SDL_AddTimer(times,warning_event_callback,NULL);
                                      //  tm_evafter(times,TIME_WARNING_EVENT_TIMEOUT,&m_waringtimerid);
                                }
                                return;
                        }
                        if(evt & TIME_WARNING_EVENT_STOP){
                                if(m_waringtimerid != 0){
                                        SDL_RemoveTimer(m_waringtimerid);
                                        WRT_MESSAGE("取消报警延时计时");
                                }
                                m_waringtimerid = 0;
                                return;
                        }
}


unsigned char* CCmdHandler::GetTestRingAddr(int index ,unsigned long*  len){
       return NULL;
}
void CCmdHandler::lowtension_timer(int start)
{
	if(start == 1){
		add_and_del_timer2(TIME_LOWTENSION_WARNING_EVENT_START);
	}else{
		add_and_del_timer2(TIME_LOWTENSION_WARNING_EVENT_STOP);
	}
}
/*******************************************************************
函数名称：Init
函数功能：初始化函数，包括初始化网络配置以及计时任务一起服务初始化。
输入参数：
输出参数：
返回值：
********************************************************************/


void CCmdHandler::Init()
{
    AppENetCallbacks   enetcallbacks;
    ENetCallbacks      enetcallback2;
    struct sockaddr_in senderaddr;
    int    ret=0;
    char tmpbuf[3]={'\0','\0','\0'};
    memcpy(tmpbuf,pSystemInfo->LocalSetting.privateinfo.LocalID+13,2);
    g_localid = atoi(tmpbuf); //本地ID的编号，用来判断是否主分机
    WRT_MESSAGE("cmdhander init start");

    if(m_pSocketManager){
        m_centeripaddr =pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;//只是主中心地址
        m_waringipaddr =  pSystemInfo->LocalSetting.publicinfo.CenterWarningIP; //报警中心地址
        m_Groupwaringipaddr = 0;
        m_manageripaddr = pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;    //只是主管理机地址
    }

    WRT_MESSAGE("init callersetup");
    memset(&callersetup,0,sizeof(callersetup));
    callersetup.timecallstop = 600; //2分
    callersetup.timering  = 35*5;//pSystemInfo->mySysInfo.ringtimes*5; //30s
    callersetup.timemonit =  callersetup.timering;
    callersetup.fun_show = caller_showmsg;
    callersetup.fun_ringon = caller_ringon;
    callersetup.fun_ringoff = caller_ringoff;
    callersetup.fun_holdon = caller_holdon;
    callersetup.fun_callend = caller_callend;
    callersetup.fun_prep = caller_prep;
    callersetup.maxconnecttime = 5;//5s
    callersetup.MaxDivertTimes = 4;//3次
    callersetup.MaxVideo = 4;
    callersetup.DeviceType = 1;
    callersetup.usecenternegotiate = pSystemInfo->mySysInfo.usecenternegotiate;
    memcpy(callersetup.localid,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
    wrti_CallerSetup(callersetup);

#if USED_OPTIMIZE_FFS
    WRT_MESSAGE("init save");
    init_optimize_save();
#endif
    WRT_MESSAGE("init idtoip table");
    init_idtoip();

    WRT_MESSAGE("init phote and message");
    init_photo();

    init_video();
    init_msg();
    init_log();
    init_voice();
    init_telephone();
    init_tmp();

    init_screen_save();

    //SDL_CreateThread(time_handler_task,NULL);
    //SDL_CreateThread(time_handler_task2,NULL);

    //SDL_Delay(2000);//保证g_timeout_tid 任务启动起来
    add_and_del_timer2(TIME_DISPLAY_TIME_EVENT_START);

    init_smalldoor();
    init_downring();
#ifdef HAVE_SUB_PHONE
    set_phone_callback(phone_callback);

    init_sub_phone();
    init_phone_video(); 

    set_subroom_callback(phone_callback);
#endif
    init_subroom();

    Init_Local_MediaInfo();
    //         init_16safe(); //2010-6-30 11:56:49
#if 1    
    enetcallback2.malloc = zenMalloc;
    enetcallback2.free = zenFree;
    enetcallback2.rand = 0;
    enet_initialize_with_callbacks(ENET_VERSION,&enetcallback2);
    //App_SetPri(124);
    // App_InitENet_For_Recv(20302);
    // App_InitENet_For_Send(20300);

    enetcallbacks.AppRecvCallback = 0;
    enetcallbacks.AppRecvCallback2 = app_enet_recv;
    //App_InitENet(20302,20300, &enetcallbacks);
    App_SetCallBack2(&enetcallbacks);
#endif

#ifdef DOWNLOAD_FROM_HTTP
    /*
    if(pSystemInfo->mySysInfo.ishttpdown == 1)
    init_download_task();
    */                
#endif

    //2011-4-28 15:39:07
    //add_and_del_timer2(TIME_JIAJU_CHECK_START);

    RoomGpioUtilSetDoorAudioOff();
    WRT_MESSAGE("cmd_handler init ok");

    wrthost_config_networking();

    //system("ifconfig eth0 down");
    //SDL_Delay(500);
    //system("ifconfig eth0 up");

}

/*******************************************************************
函数名称：ThreadMain
函数功能：命令处理任务进入点。
输入参数：
输出参数：
返回值：
********************************************************************/
int CCmdHandler::ThreadMain(){
        short cmd = -1;
        int   sock_index = -1;
        uint32_t len;
        unsigned long evt;
        int  once =0;
        unsigned long cmd_event[4];
        char   pPeerLocalID[16];
        short tmpcmd = 0;
        int offset =0;
        int PacketLen=0;

#if HAVE_USED_SDL
        int rc;
        int type=0;
        const int type1 = MSG_NODE_RECV_UDP_MSG;
        const int type2 = MSG_PROCESS_UDP_MSG;
#else
        int rc;
        unsigned char*  msgbuf = (unsigned char*)ZENMALLOC(400);
        int type=0;
#endif

        while (true) {
		WRT_DEBUG(" SDL_SemWait cmdhandler\n");
#if HAVE_USED_SDL
                if (m_bstart) {
                        rc = SDL_SemWait(m_myMsgQueueSemaphore);//SDL_SemWaitTimeout(m_myMsgQueueSemaphore,100);
                } else {
                        rc = SDL_SemWait(m_myMsgQueueSemaphore);
                }
                if (rc == -1) {
                        break;
                }
#else
                memset(msgbuf,0,400);
                rc =  q_vreceive(m_myMsgID,Q_WAIT,0,(void *)msgbuf,400,&len);
#endif

                if (rc == 0) {
#if HAVE_USED_SDL
                        CWRTMsg* pMsg = m_myMsgQueue.get_message();

                        if (pMsg != NULL) {
                                type = pMsg->get_value();
#else
                        type = *(int*)msgbuf;
#endif
                        WRT_MESSAGE("收到需要处理的消息类型%d",type);
                        switch (type) 
                        {
                        case type2: //MSG_PROCESS_UDP_MSG
                                {
                                        WRT_MESSAGE("处理UDP网络消息");
                                        unsigned char* buf = (unsigned char*)pMsg->get_message(len);
                                        if(buf == NULL)
                                                break;
                                        memset(cmd_event,0,sizeof(cmd_event));
                                        memcpy(cmd_event,buf, sizeof(cmd_event));
                                        ProcessUDP(cmd_event[0],(unsigned char*)cmd_event[1],cmd_event[2]);
                                }
                                break;
                        case type1: //MSG_NODE_RECV_UDP_MSG
#if HAVE_USED_SDL
                                {
                                        unsigned char* buf = (unsigned char*)pMsg->get_message(len);
                                        if(buf == NULL)
                                                break;
                                        memset(cmd_event,0,sizeof(cmd_event));
                                        memcpy(cmd_event,buf, sizeof(cmd_event));
                                        udp_send_data(cmd_event);
                                }
#endif
                                break;
                        case MSG_NODE_STOP_THREAD:
#if HAVE_USED_SDL
                                pMsg->free_message();
                                ZENFREE(pMsg);

#endif
                                return 0;
                                
                        case MSG_NODE_START:
                                Init();
                                m_bstart = true;
                                m_SysStatus = SYS_IDLE;
                                break;
                        case MSG_NODE_STOP:
                                break;
                        case MSG_NODE_SOCKET_CUT:
                                {
#if HAVE_USED_SDL
                                        unsigned char* tbuf = (unsigned char*)pMsg->get_message(len);
                                        if(tbuf == NULL)
                                                break;
                                        int sock_index = *(int*)tbuf;
#else
                                        int sock_index = *(int*)(msgbuf+4);
#endif
                                        net_shutdown(sock_index);
                                }
                                break;
                        case MSG_NODE_NET_EXCEPTION:
                                {
#if HAVE_USED_SDL
                                        unsigned char* tbuf =(unsigned char*) pMsg->get_message(len);
                                        if(tbuf == NULL)
                                                break;
                                        int index = *(int*)tbuf;
#else
                                        int index = *(int*)(msgbuf+4);
#endif
                                        net_exception(index);
                                }
                                break;
                        case MSG_NODE_CONNECT_OK: //主动连接对方，收到连接OK或者failed 消息
                                {
#if HAVE_USED_SDL
                                        unsigned char* tbuf = (unsigned char*)pMsg->get_message(len);
                                        if(tbuf == NULL)
                                                break;
                                        int index = *(int*)tbuf;
#else
                                        int index = *(int*)(msgbuf+4);
#endif
                                        if(index != -1){
                                                ProcessConnectOK(index);
                                        }else{
                                        	//需要添加处理如果是小门口机托管，则应答小门口机挂机.//2011-6-1 10:14:24
                                                //显示连接失败，同时如果,
                                                if(m_CurrentSocketIndex == -1){
                                                	
                                                        if(m_SysStatus != SYS_IDLE && m_SysStatus != SYS_SMALLDOORCALL ){
                                                        		
																if(g_smalldoor_call_agent == 1){ //2011-6-1 10:22:10
																	door_and_center_hangup(-1); 
																	break;
																}
										
                                                                m_displayevent[0] = STATUS_NET_EXCEPTION;
                                                                if(SYS_REQHELPINFO == m_SysStatus || 
                                                                	SYS_GETTIME == m_SysStatus || 
                                                                	SYS_REQDOCINFO == m_SysStatus )
                                                                        m_displayevent[1] = 0;
                                                                else
                                                                        m_displayevent[1] =3;
                                                                m_displayevent[2]=0;
                                                                m_displayevent[3]= 0;
                                                                m_SysStatus = SYS_IDLE;
                                                                m_display(m_displayevent);
#ifdef HAVE_SUB_PHONE                                                              
 												if(g_phone_connect_index != -1)
                                				{
                                        				if(m_pSocketManager)
                                                				m_pSocketManager->CloseSocket(g_phone_connect_index);
                                        				g_phone_connect_index = -1;
                                				}
#endif                                                                  
                                                        }
                                                }else
                                                	printf("xxxxxxxxxxxxxxxxxxxxxx\n");
                                        }
                                }
                                break;
                        case MSG_NODE_SMALLDOOR_CMD:
                                {
#if HAVE_USED_SDL
                                        unsigned char* tbuf =( unsigned char* ) pMsg->get_message(len);
#else
                                        unsigned char* tbuf =(unsigned char*) (msgbuf+4);
#endif
                                        if(tbuf == NULL)
                                                break;
                                        memset(cmd_event,0,sizeof(unsigned long)*4);
                                        memcpy(cmd_event,tbuf, sizeof(unsigned long)*4);

                                        ProcessCmd(-1,cmd_event[0],cmd_event);

                                }
                                break;
                        case MSG_NODE_PHONE:
#ifdef HAVE_SUB_PHONE
                                {
                                        unsigned char* tbuf =( unsigned char* ) pMsg->get_message(len);
                                        if(tbuf == NULL)
                                                break;
                                        memset(cmd_event,0,sizeof(unsigned long)*4);
                                        memcpy(cmd_event,tbuf, sizeof(unsigned long)*4);
                                        WRT_MESSAGE("处理手机事件0x%x",cmd_event[0]);

                                        ProcessPhone(cmd_event);

                                }
#endif
                                break;
                                
                        case MSG_NODE_CMD: //收到的是GUI 发来的命令
                                {
#if HAVE_USED_SDL
                                        unsigned char* tbuf =( unsigned char* ) pMsg->get_message(len);
#else
                                        unsigned char* tbuf =(unsigned char*) (msgbuf+4);
#endif
                                        if(tbuf == NULL)
                                                break;
                                        memset(cmd_event,0,sizeof(unsigned long)*4);
                                        memcpy(cmd_event,tbuf, sizeof(unsigned long)*4);
                                        WRT_MESSAGE("处理GUI事件0x%x",cmd_event[0]);

                                        ProcessCmd(-1,cmd_event[0],cmd_event);
                                        sock_index = -1;
                                }
                                break;
                        case MSG_NODE_RECV_MSG:  //收到网络端的数据
#if HAVE_USED_SDL
                                SOCKET_PACKET2* pPacket   = (SOCKET_PACKET2*)pMsg->get_message(len);
#else
                                SOCKET_PACKET2* pPacket = (SOCKET_PACKET2 *)(msgbuf+4);
#endif
                                if(pPacket != NULL&& pPacket->buf != NULL && pPacket->validlen > 0){
                                        if(strncmp("WRTI",(char*)pPacket->buf,4) == 0){
                                                sock_index = pPacket->sock_index;
                                                tmpcmd = *(short*)(pPacket->buf+8);
                                                tmpcmd = ntohs(tmpcmd);
#if USED_NEW_CMD
                                                memset(pPeerLocalID,0,16);
#ifdef HAVE_SUB_PHONE
                                                memset(m_DestNumber,0,16);
#endif
                                                memcpy(pPeerLocalID,pPacket->buf+10,15);
                                                memset(m_UserPhoneNumber,0,16);
                                                strncpy(m_UserPhoneNumber,pPeerLocalID,15);
#ifdef HAVE_SUB_PHONE
                                                if(tmpcmd == ROOM_STARTMONT || tmpcmd == ROOM_CALLCENTER || tmpcmd == ROOM_CALLROOM)
                                                        memcpy(m_DestNumber,pPacket->buf+25,15);
#endif
                                                offset = 40;
#else
                                                offset = 10;
#endif
                                                if(pPacket->validlen == offset){
                                                        memset(cmd_event,0,sizeof(unsigned long)*4);
                                                        cmd_event[0] = (unsigned long)pPeerLocalID;
                                                        WRT_MESSAGE("处理TCP网络事件(40B) pPeerLocalID= %s,cmd = 0x%x",pPeerLocalID,tmpcmd);
                                                        ProcessCmd(sock_index,tmpcmd,cmd_event);
                                                        cmd = -1;
                                                        sock_index = -1;
                                                        ZENFREE(pPacket->buf);
                                                        pPacket->buf = NULL;
                                                }else if(pPacket->validlen > offset) {
                                                        WRT_MESSAGE("处理TCP网络事件(>40B) pPeerLocalID= %s,cmd = 0x%x",pPeerLocalID,tmpcmd);
                                                        //if(tmpcmd == ROOM_CALLROOM)
                                                        //{
                                                        //	ProcessPacket(sock_index,tmpcmd,pPacket->buf,pPacket->validlen);
                                                        //}
                                                        //else
                                                        ProcessPacket(sock_index,tmpcmd,pPacket->buf+40,pPacket->validlen-40);
                                                        ZENFREE(pPacket->buf);
                                                        pPacket->buf = NULL;
                                                        cmd = -1;
                                                        sock_index = -1;
                                                }else{
                                                		WRT_MESSAGE("TCP收到数据异常");
                                                        ZENFREE(pPacket->buf);
                                                        pPacket->buf = NULL;
                                                        cmd = -1;
                                                        sock_index = -1;
                                                }

                                        }
                                }
                                break;
                        }
#if HAVE_USED_SDL
			WRT_DEBUG(" pMsg->free_message\n");
                        pMsg->free_message();
                        ZENFREE(pMsg);
                }
#endif
        }
}
return -1;
}

/************************************************************************
函数功能：处理主动连接外部成功的时候，发送的各种命令
***********************************************************************/
void CCmdHandler::ProcessConnectOK(int index){
        switch(m_SysStatus){
        case SYS_MONT: //监视
                m_CurrentSocketIndex  = index;
                break;
        case SYS_ROOMCALLCENTERTALK: //呼叫中心
                m_CurrentSocketIndex  = index;
                break;
        case SYS_GETROOMIP: //获取IP地址
                m_CurrentSocketIndex  = index;
                break;
        case SYS_REQUESTFEE: //请求费用列表
                m_CurrentSocketIndex  = index;
                break;
        case SYS_REQUESTREPAIR: //请求维修
                m_CurrentSocketIndex  = index;
                break;
        case SYS_REQEUSTRINGLIST://请求铃声列表
                m_CurrentSocketIndex  = index;
                break;
        case SYS_DOWNLOADRING: //请求下载铃声
                m_CurrentSocketIndex  = index;
                break;
        case SYS_REQDOCINFO:
                m_CurrentSocketIndex = index;
                break;
       // case SYS_GETTIME:
       //         m_CurrentSocketIndex  = index;
       //         break;
        case SYS_REQHELPINFO:
                m_CurrentSocketIndex  = index;
                break;
       // case SYS_GETSYSINFO:
       //         m_CurrentSocketIndex = index;
       //         break;
#if 0
        case SYS_CALL_LIFT:
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(index);
                m_SysStatus = SYS_IDLE;
#endif
                break;
        default:
                break;

        }
}

/*----------------------------------------------------------------------
函数名称：net_shutdown
函数功能：网络连接被对方重置
输入参数：
sock_index   当前被重置连接的索引
作者:     李建文
日期：    2008.4.22
------------------------------------------------------------------------*/
void CCmdHandler::net_shutdown(int sock_index){

       WRT_MESSAGE("网络切断net_shutdown sock_index = %d ,m_CurrentSocketIndex=%d ,当前状态0x%x",
        		sock_index,m_CurrentSocketIndex,m_SysStatus);
#if USE_PACKELIST
        if(sock_index != -1)
                DelPackList(sock_index);
#endif
        if(m_SysStatus  == SYS_ISSCREENADJUST){
                if(sock_index != -1)
                        m_pSocketManager->CloseSocket(sock_index);
                if(sock_index == m_CurrentSocketIndex)
                        m_CurrentSocketIndex  = -1;
                return ;
        }

        if(sock_index != -1 && m_CurrentSocketIndex == sock_index && m_SysStatus != SYS_IDLE && m_SysStatus != SYS_SMALLDOORCALL ){
		WRT_MESSAGE("net_shutdown 当前状态0x%x",m_SysStatus);
		if(g_smalldoor_call_agent == 1){ //2011-6-1 10:22:10
			door_and_center_hangup(-1); 
			return;
		}
                m_CurrentItemID = -1;
                m_CurrentSocketIndex = -1;
                if(m_watchtimerid != 0){
                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                }
                mp3stop();
                if(m_ringtimerid != 0){
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                talkLeaveStop();
                talktostop();
                talkstop();
                if(m_talktimerid != 0){
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
                }
                /* if(m_SysStatus == SYS_LEAVEWORD){
                talkLeaveStop();
                talkstop();
                }*/
                if(m_SysStatus != SYS_IDLE){
                        m_displayevent[0] = STATUS_NET_EXCEPTION;
                        m_displayevent[1] =1;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }
                m_SysStatus = SYS_IDLE;
        }
}

/************************************************************************
函数名称：net_exception
函数功能：当网络发生异常的时候，取消当前的所有状态，还原初始状态
输入参数：
输出参数： 无
作者:     李建文
日期：    2007.10.25
************************************************************************/
void CCmdHandler::net_exception(int sock_index){

#if USE_PACKELIST
        if(sock_index != -1)
                DelPackList(sock_index);
#endif
        WRT_MESSAGE("网络异常当前状态0x%x",m_SysStatus);
        if(m_SysStatus  == SYS_ISSCREENADJUST){
                if(sock_index != -1)
                        m_pSocketManager->CloseSocket(sock_index);
                if(sock_index == m_CurrentSocketIndex)
                        m_CurrentSocketIndex  = -1;
                return ;
        }

        if(sock_index != -1 && m_CurrentSocketIndex == sock_index && m_SysStatus != SYS_IDLE && m_SysStatus != SYS_SMALLDOORCALL ){

                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
				if(g_smalldoor_call_agent == 1){ //2011-6-1 10:22:10
					door_and_center_hangup(-1); 
					return;
				}                
                m_CurrentItemID = -1;
                m_CurrentSocketIndex = -1;
                if(m_watchtimerid != 0){
                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                }
                mp3stop();
                if(m_ringtimerid != 0){
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                talkLeaveStop();
                talktostop();
                talkstop();
                if(m_talktimerid != 0){
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
                }
                if(m_SysStatus != SYS_IDLE){
                        m_displayevent[0] = STATUS_NET_EXCEPTION;
                        if(SYS_GETSYSINFO == m_SysStatus )
                                m_displayevent[1] = 3;
                        else
                                m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);

                } 
                //要添加小门口机托管异常的处理
                m_SysStatus = SYS_IDLE;
                return;
        }
        if(sock_index != -1)
                m_pSocketManager->CloseSocket(sock_index);
        
}


/************************************************************************
函数名称：PackCmd
函数功能：打包命令
输入参数：
输出参数： 无
作者:     李建文
日期：
************************************************************************/
unsigned char*  CCmdHandler::PackCmd(short cmd,unsigned char* buf,int buflen,int* packlen){
        unsigned char* packbuf = NULL;
#ifdef USED_NEW_CMD
        int cmdlen = 40;
        short tmpcmd = cmd;
#else
        int cmdlen = 10;
#endif
        if(buflen == 0)
                packbuf = (unsigned char*)ZENMALLOC(cmdlen);
        else{
                cmdlen += buflen;
                packbuf = (unsigned char*)ZENMALLOC(cmdlen);
        }
        if(packbuf == NULL){
                *packlen =0;
                return NULL;
        }

        cmd =htons(cmd);
        memset(packbuf,0,cmdlen);
        strncpy((char*)packbuf,"WRTI",4);
        memcpy(packbuf+4,&cmdlen,4);
        memcpy(packbuf+8,&cmd,2);
#ifdef USED_NEW_CMD
        memcpy(packbuf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        // memcpy(packbuf+25,"020020100000001",15);//2009-9-3 01:47下午
        if(cmdlen > 40){
                memcpy(packbuf+40,(void*)buf,buflen);
        }
#else
        if(cmdlen > 10){
                memcpy(packbuf+10,(void*)buf,buflen);
        }
#endif
        *packlen = cmdlen;
        return packbuf;
}

/************************************************************************
函数名称：SendCmd
函数功能：将命令打包发送给SOCKET处理程序。
输入参数：
sock_index :当前的socket的索引
cmd :       当前需要发送的命令
id:         命令所带的参数
输出参数： 无
作者:     李建文
日期：    2007.10.25
************************************************************************/
void CCmdHandler::SendCmd(int sock_index,short cmd,int id){
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                int templen =0;
                int tmpcmd = cmd;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
#ifdef USED_NEW_CMD
                int cmdlen = 40;
                int offset = 40;
#else
                int cmdlen = 10;
                int offset = 10;
#endif
                cmd = htons(cmd);
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                memset(packet.buf,0,256);
                strncpy((char*)packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
#ifdef USED_NEW_CMD
                memcpy(packet.buf + 10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
#endif
                if(tmpcmd == ROOM_AGENT){
#ifdef USED_NEW_CMD
                        cmdlen = 44;
#else
                        cmdlen = 14;
#endif
                        memcpy(packet.buf+4,&cmdlen,4);
                        memcpy(packet.buf+offset,&m_centeripaddr,sizeof(unsigned long));
#ifdef USED_NEW_CMD
                        offset = 44;
#else
                        offset = 14;
#endif
                }


#ifdef USED_NEW_CMD
                if(id != -1){
                        unsigned char temp_c = (unsigned char)id;
                        memcpy(packet.buf+offset,&temp_c,1);
                        templen = sizeof( unsigned char);
                        cmdlen +=templen;
                        memcpy(packet.buf+4,&cmdlen,4);
                }
#else
                if(id != -1){
                        unsigned char temp_c = (unsigned char)id;
                        memcpy(packet.buf+offset,&temp_c,1);
                        templen = sizeof( unsigned char);
                        cmdlen +=templen;
                        memcpy(packet.buf+4,&cmdlen,4);
                }
#endif
                packet.validlen  = cmdlen;
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
        }

}

void CCmdHandler::SendCmd2(int sock_index,short cmd,char *buf,int buflen,int id){
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                int templen =0;
                int tmpcmd = cmd;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
#ifdef USED_NEW_CMD
                int cmdlen = 40;
                int offset = 40;
#else
                int cmdlen = 10;
                int offset = 10;
#endif
				
                cmd = htons(cmd);
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                memset(packet.buf,0,256);
                strncpy((char*)packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
#ifdef USED_NEW_CMD
                memcpy(packet.buf + 10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
#endif
				if(tmpcmd == MEDIA_NEGOTIATE_ACK)
				{
					WRT_DEBUG("回复分机空闲带媒体协商结果");

					//media_negotiate_process(buf,buflen);
					Get_negotiate_result_of_remote((char *)(packet.buf + offset));
					//int venctype = get_venc_type();

					

					cmdlen = 40 + sizeof(NEGOTIATE_RESULT);
				}else if(tmpcmd == MEDIA_NEGOTIATE)
				{
					Get_local_mediainfo((char *)(packet.buf + offset));
					cmdlen = 40 + sizeof(MEDIA_DATA);
				}
                if(tmpcmd == ROOM_AGENT){
#ifdef USED_NEW_CMD
                        cmdlen = 44;
#else
                        cmdlen = 14;
#endif
                        memcpy(packet.buf+4,&cmdlen,4);
                        memcpy(packet.buf+offset,&m_centeripaddr,sizeof(unsigned long));
#ifdef USED_NEW_CMD
                        offset = 44;
#else
                        offset = 14;
#endif
                }
#ifdef USED_NEW_CMD
                if(id != -1){
                        unsigned char temp_c = (unsigned char)id;
                        memcpy(packet.buf+offset,&temp_c,1);
                        templen = sizeof( unsigned char);
                        cmdlen +=templen;
                        memcpy(packet.buf+4,&cmdlen,4);
                }
#else
                if(id != -1){
                        unsigned char temp_c = (unsigned char)id;
                        memcpy(packet.buf+offset,&temp_c,1);
                        templen = sizeof( unsigned char);
                        cmdlen +=templen;
                        memcpy(packet.buf+4,&cmdlen,4);
                }
#endif
				memcpy(packet.buf+4,&cmdlen,4);
                packet.validlen  = cmdlen;

                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
        }

}

void CCmdHandler::SendRoomCmd(int sock_index)
{
#if 0
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                int cmdlen = 21;
                short cmd;
                unsigned char roomid[15]={0};
                memset(&packet,-1,sizeof(SOCKET_PACKET));
                cmd = htons(ROOM_CALLROOM);
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                memset(packet.buf,0,256);
                strncpy(packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
                mmemcpy(roomid,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                memcpy(packet.buf+10,roomid,15);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
        }
#endif
}

void CCmdHandler::SendDownRingCmd(int sock_index){
#if 0
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                int cmdlen = 50;
                short cmd;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
                cmd = htons(ROOM_RINGDOWN);
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                memset(packet.buf,0,256);
                strncpy(packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
                memcpy(packet.buf+10,m_downname,40);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
        }
#endif
}

/******************
向中心发送请求某个
房间的IP地址
******************/
void CCmdHandler::RequestIP(int sock_index,short cmd,unsigned char buf[15]){
#if 0
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                int templen =0;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
#ifdef USED_NEW_CMD
                int cmdlen = 55;
#else
                int cmdlen = 25;
#endif
                cmd = htons(cmd);
                memset(packet.buf,0,256);
                strncpy(packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
#ifdef USED_NEW_CMD
                //memcpy(packet.buf+10,roomtest.LocalRoom.LocalID,15);
                memcpy(packet.buf + 10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                memcpy(packet.buf+40,(void*)buf,15);
#else
                memcpy(packet.buf+10,(void*)buf,15);
#endif
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                m_displayevent[0] = STATUS_ROOM_GET_IP;
                m_displayevent[1] = m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));

        }
#endif
}

void CCmdHandler::room_req_sync(unsigned long ip){
        if(g_localid == 0x01)
                return;
        unsigned long tmpevent[4];
        tmpevent[0] = ROOM_REQ_SYNC;
        tmpevent[1] = tmpevent[2] = tmpevent[3] =0;
        udp_send_data2(ip,tmpevent);
}

void CCmdHandler::room_active_sync(unsigned long ip)
{
        unsigned long tmpevent[4];
        if(g_localid != 0x01)
                return;
        if(pSystemInfo->mySysInfo.isagent == 0){
                tmpevent[0] = ROOM_CANCEL_SYNC_AGENT;
                tmpevent[1] = 0;
                tmpevent[2] = 0;
                tmpevent[3] = 0;
        }else{
                tmpevent[0] = ROOM_SYNC_AGENT;
                tmpevent[1] = 0;
                tmpevent[2] = g_agent_times;
                tmpevent[3] = 4;
        }

        udp_send_data2(ip,tmpevent);
        if(pSystemInfo->mySysInfo.isavoid  == 0){
                tmpevent[0] = ROOM_CANCEL_SYNC_AVOID;
                tmpevent[1] = 0;
                tmpevent[2] = 0;
                tmpevent[3] = 0;
        }else{
                tmpevent[0] = ROOM_SYNC_AVOID;
                tmpevent[1] = 0;
                tmpevent[2] = g_avoid_times;
                tmpevent[3] = 4;
        }
        udp_send_data2(ip,tmpevent);

        tmpevent[0] = ROOM_SYNC_RING_TIME;
        tmpevent[1] = 0;
        tmpevent[2] = pSystemInfo->mySysInfo.ringtimes;
        tmpevent[3] = 4;
        udp_send_data2(ip,tmpevent);
}

/************************************************************************
函数名称：ProcessUDP
函数功能：处理收到的UDP命令包（注意需要释放data 的内存）。
输入参数：
ip :发送者的IP
data:       数据buffer
datalen:     数据长度
输出参数： 无
作者:     李建文
日期：    2007.10.25
************************************************************************/
void CCmdHandler::ProcessUDP(unsigned long ip,unsigned char* data,int datalen){
        int isupdateview = 0;
        unsigned long tmptime =0;
        unsigned long sipevent[4];
        char  srcID[16]={0};
#ifdef _DEBUG
        unsigned long debugmsg[4];
#endif
        if(strncmp("WRTI",(char*)data,4) != 0){
                return;
        }
        short cmd = *(short*)(data+8);
        cmd = ntohs(cmd);
        WRT_MESSAGE("处理UDP指令cmd 0x%x",cmd );
        strncpy(srcID,(char*)(data+10),15);
        switch(cmd){
                //模拟分级支持2011-3-19 20:34:08

                case ROOM_USED_AGENT:
                        if(m_SysStatus != SYS_IDLE)
                        {
                                //本机是否通话。
                                if(g_localid == 0x1){
#ifdef HAVE_SUB_PHONE
                                    if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
                                        	unsigned long msg_buf[4]={0};
                                        	msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                                        	msg_buf[1] = 1;
                                        	send_msg_to_sub_phone(msg_buf);
                                        	SetTransmitToPhone(0);
                                    }                                 	
#endif
                                }
                                if(g_holdonbyme == 1)
                                        return;
                                if(g_ismyholdon == 1)
                                        return;
                                if(g_simulate_talk == 1)
                                        return;
                                if(m_SysStatus == SYS_SMALLDOORCALL){
                                        sipevent[0] = 0;
                                        sipevent[1] = 2;
                                        sipevent[2]=sipevent[3]=0;
                                        smalldoor_ack_hangup(sipevent);
                                        break;
                                }
                                if(g_smalldoor_call_agent==1)
                                        return;
                                room_hangup(-6); //不发挂机，发托管
                        }
                        break;
                case ROOM_USED_SIMULATE:
                        //if(m_SysStatus == SYS_IDLE){
                        sipevent[0] = *(unsigned long*)(data+40);                         
                        g_used_simulate_channel = sipevent[0];
                        WRT_MESSAGE("模拟通道是否占用 g_used_simulate_channel %d",g_used_simulate_channel);
                        //}
                        break;
                case INFORM_SIMULATE_HANGUP: //主分机通知模拟分级，有主分机摘机。
                        if(g_localid == 0x01){                       	
                                if(g_simulate_talk == 1)
                                        break;
                                unsigned short tmpevent1[4];
                                tmpevent1[0] = INFORM_SIMULATE_HANGUP;
                                tmpevent1[1] = 0;
                                tmpevent1[2] = 0x0; //目标ID
                                tmpevent1[3] = 0 ;
                                SDL_Delay(200);
                                send_smalldoor_cmd(tmpevent1); //通知模拟分机挂机
                                SDL_Delay(200);
                                send_smalldoor_cmd(tmpevent1); //通知模拟分机挂机
                        }else{
                                if(m_SysStatus == SYS_IDLE)
                                {     
                                        sipevent[0]=STATUS_SCREEN_CLOSE;
                                        sipevent[1] = 0;
                                        sipevent[2]=sipevent[3]= 0;
                                        m_display(sipevent);	
                                }
                        }
                        break;
                        //sip
                case SIP_CALL: //收到wrtproxy的呼叫请求
                        sipevent[0] = ip;
                        sipevent[1] = *(unsigned long*)(data+40);
                        sipevent[2] = *(unsigned long*)(data+44);
                        sipevent[3] = (unsigned long)srcID;
                        proxy_call_room(sipevent);
                        break;
                case SIP_BUSY:
                        call_sip_busy(sipevent);
                        break;
                case SIP_CALL_PROCESSING:
                        call_sip_processing(sipevent);
                        break;
                case SIP_CALL_RING:
                        call_sip_ring(sipevent);
                        break;
                case SIP_CLOSE:
                        call_sip_close(sipevent);
                        break;
                case SIP_START_AUDIO:
                        sipevent[0]=sipevent[1] = 0;
                        sipevent[2] = *(unsigned long*)(data+40);
                        sipevent[3] = *(unsigned long*)(data+44);
                        call_sip_start_audio(sipevent);
                        break;
                case ROOM_REQ_SYNC:
#ifdef _DEBUG
                        debugmsg[0] = STATUS_DEBUG_MSG;
                        debugmsg[1] = 1;
                        debugmsg[2] = 0;
                        debugmsg[3] = 0;
                        m_display(debugmsg);
#endif
                        room_active_sync(ip);
                        break;
                case ROOM_CANCEL_SYNC_AGENT:
                        pSystemInfo->mySysInfo.isagent = 0;
                        if(g_timeout_tid != -1)
                                add_and_del_timer(TIME_AGENT_SETTIMES_EVENT_STOP);
                        g_agent_times = 0;
                        pSystemInfo->mySysInfo.agenttimes = 0;
                        FlushSystemInfo();
                        isupdateview = 1;
                        break;
                case ROOM_SYNC_AGENT:
                        tmptime = *(unsigned long *)(data+40);
                        pSystemInfo->mySysInfo.isagent = 1;
                        g_agent_times = tmptime;
                        pSystemInfo->mySysInfo.agenttimes = tmptime;
                        if(g_timeout_tid != -1)
                                add_and_del_timer(TIME_AGENT_SETTIMES_EVENT_START);

                        if(pSystemInfo->mySysInfo.isavoid == 1){

                                        add_and_del_timer(TIME_AVOID_SETTIMES_EVENT_STOP);
                                pSystemInfo->mySysInfo.isavoid = 0;
                                g_avoid_times = 0;
                                pSystemInfo->mySysInfo.avoidtimes = 0;
                        }

                        FlushSystemInfo();
                        isupdateview = 1;
                        break;
                case ROOM_SYNC_AVOID:
                        tmptime = *(unsigned long *)(data+40);
                        pSystemInfo->mySysInfo.isavoid = 1;
                        g_avoid_times = tmptime;
                        pSystemInfo->mySysInfo.avoidtimes = tmptime;

                                add_and_del_timer(TIME_AVOID_SETTIMES_EVENT_START);
                        //取消托管
                        if(pSystemInfo->mySysInfo.isagent == 1){
                                pSystemInfo->mySysInfo.isagent = 0;

                                        add_and_del_timer(TIME_AGENT_SETTIMES_EVENT_STOP);
                                g_agent_times = 0;
                                pSystemInfo->mySysInfo.agenttimes = 0;
                        }
                        FlushSystemInfo();
                        isupdateview = 1;
                        break;
                case ROOM_CANCEL_SYNC_AVOID:

                                add_and_del_timer(TIME_AVOID_SETTIMES_EVENT_STOP);
                        pSystemInfo->mySysInfo.isavoid = 0;
                        g_avoid_times = 0;
                        pSystemInfo->mySysInfo.avoidtimes = 0;
                        FlushSystemInfo();
                        isupdateview = 1;
                        break;
                case ROOM_SYNC_RING_TIME:{
                        unsigned long tmptime= 0;
                        tmptime = *(unsigned long*)(data+40);
                        //SetRingtimes(tmptime);
                        g_ring_times = tmptime;
                        pSystemInfo->mySysInfo.ringtimes = tmptime;
                        FlushSystemInfo();
                	}
                        break;
                case ROOM_SMALLDOOR_HOLDON:
                        if(m_SysStatus == SYS_SMALLDOORCALL){
                                if(g_localid == 0x01){
#ifdef HAVE_SUB_PHONE
                                	 if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
                                                unsigned long msg_buf[4]={0};
                                                msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                                                msg_buf[1] = 1;
                                                send_msg_to_sub_phone(msg_buf);
                                                SetTransmitToPhone(0);
                                        }
#endif
                                }                        	
                                mp3stop();
                                m_smalldoor_cmd =  -1;
                                m_smalldoor_id = -1;
                                if(m_ringtimerid != 0){
                                        add_and_del_timer(TIME_RING_EVENT_STOP);
                                }
                                m_smalldoor_cmd_count = 0;
                                stop_auto_cap_smalldoor_photo();//如果启动自动抓拍，则停止它
                                stop_smalldoor_video();//停止小门口视频
                                unsigned long tmpevent[4];
                                tmpevent[0]=STATUS_ROOM_HANGUP;
                                tmpevent[1] = 1;
                                tmpevent[2]=tmpevent[3]= 0;
                                m_display(tmpevent);
                                if(pSystemInfo->mySysInfo.isnvo == 0x01)
                                {
                                        tmpevent[0]=STATUS_SCREEN_CLOSE;
                                        tmpevent[1] = 0;
                                        tmpevent[2]=tmpevent[3]= 0;
                                        m_display(tmpevent);
                                }
                                m_SysStatus = SYS_IDLE;



                        }
                        break;
                case ROOM_SMALLDOOR_HANGUP:
                        if(m_SysStatus == SYS_SMALLDOORCALL){
                                if(g_localid == 0x01){
#ifdef HAVE_SUB_PHONE
                                	 if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
                                                unsigned long msg_buf[4]={0};
                                                msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                                                msg_buf[1] = 1;
                                                send_msg_to_sub_phone(msg_buf);
                                                SetTransmitToPhone(0);
                                        }
#endif
                                }                        	
                                mp3stop();
                                m_smalldoor_cmd =  -1;
                                m_smalldoor_id = -1;
                                if(m_ringtimerid != 0){
                                        add_and_del_timer(TIME_RING_EVENT_STOP);
                                }
                                if(GetsmalldoorLeaveStatus()){
                                        stopsmalldoorleave();
                                }
                                m_smalldoor_cmd_count = 0;
                                stop_auto_cap_smalldoor_photo();//如果启动自动抓拍，则停止它
                                stop_smalldoor_video();//停止小门口视频
                                unsigned long tmpevent[4];
                                tmpevent[0]=STATUS_ROOM_HANGUP;
                                tmpevent[1] = 1;
                                tmpevent[2]=tmpevent[3]= 0;
                                m_display(tmpevent);
                                m_SysStatus = SYS_IDLE;
                        }
                        break;
                case CENTER_IS_DEVONLINE:
                        {
                                if(ip == pSystemInfo->LocalSetting.privateinfo.LocalIP)
                                        return;
                                char ctmpbuf[42];
                                int len = 42;
                                short tmpcmd = DEVONLINE_ACK;
                                tmpcmd  = htons(tmpcmd);
                                memset(ctmpbuf,0,42);
                                strncpy(ctmpbuf,"WRTI",4);
                                memcpy(ctmpbuf+4,&len,4);
                                memcpy(ctmpbuf+8,&tmpcmd,2);
                                memcpy(ctmpbuf +10 ,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                ctmpbuf[40] = 0x04;
                                ctmpbuf[41] = 0x01;
                                App_Enet_Send2(ip,20302,ctmpbuf,len);
                        }
                        break;
                case ROOM_HOST_BUSY: //多分机中，主分机繁忙。2009-7-17
                        if(g_localid != 0x01){
                                WRT_DEBUG("收到其他分机的忙碌的消息\n");
                                switch(g_localid)
                                {
                                case 0x02:

                                        if(m_SysStatus == SYS_SMALLDOORCALL){
                                                m_ishostbusy = 1; //响铃时间到时，由2号分机挂机。
                                        }else{
                                                unsigned long tmpevent1[4];
                                                unsigned long tmpip = 0;
                                                int  tmpsmallid = 0;
                                                char tmpid[16];
                                                memset(tmpid,0,sizeof(tmpid));
                                                memcpy(tmpid,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                                tmpid[13] = '0';
                                                tmpid[14] = '3';
                                                get_ipbyid2(tmpid,&tmpip);
                                                tmpsmallid = *(int*)(data+40);
                                                if(tmpip != 0){
                                                        tmpevent1[0] = ROOM_HOST_BUSY;
                                                        tmpevent1[1] = 0;
                                                        tmpevent1[2] = tmpsmallid;
                                                        tmpevent1[3] = 4;
                                                        udp_send_data2(tmpip,tmpevent1);
                                                }else{ //无地3号分机。不处理
                                                        unsigned short tmpevent[4];
                                                        tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                                                        tmpevent[1] = 0;
                                                        tmpevent[2] = tmpsmallid; //目标ID
                                                        tmpevent[3] =  0; 
                                                        send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                                                }
                                        }
                                        break;
                                case 0x03:
                                        if(m_SysStatus == SYS_SMALLDOORCALL){
                                                m_ishostbusy = 1; ////响铃时间到时，由3号分机挂机。
                                        }else{
                                                unsigned long tmpevent1[4];
                                                unsigned long tmpip = 0;
                                                char tmpid[16];
                                                int  tmpsmallid = 0;
                                                memset(tmpid,0,sizeof(tmpid));
                                                memcpy(tmpid,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                                tmpid[13] = '0';
                                                tmpid[14] = '4';
                                                get_ipbyid2(tmpid,&tmpip);
                                                tmpsmallid = *(int*)(data+40);
                                                if(tmpip != 0){
                                                        tmpevent1[0] = ROOM_HOST_BUSY;
                                                        tmpevent1[1] = 0;
                                                        tmpevent1[2] = tmpsmallid;
                                                        tmpevent1[3] = 4;
                                                        udp_send_data2(tmpip,tmpevent1);
                                                }else{ ////无地4号分机。直接挂机
                                                        unsigned short tmpevent[4];

                                                        tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                                                        tmpevent[1] = 0;
                                                        tmpevent[2] = tmpsmallid; //目标ID
                                                        tmpevent[3] =  0; //不需要应答 //表示有数据
                                                        send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                                                }
                                        }
                                        break;
                                case 0x04:
                                        if(m_SysStatus == SYS_SMALLDOORCALL)
                                                m_ishostbusy = 1; //响铃时间到时，由4号分机挂机。
                                        else{ //四号分机，繁忙。直接挂机
                                                unsigned short tmpevent[4];
                                                int  tmpsmallid = 0;
                                                tmpsmallid = *(int*)(data+40);
                                                tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                                                tmpevent[1] = 0;
                                                tmpevent[2] = tmpsmallid; //目标ID
                                                tmpevent[3] =  0; //不需要应答 //表示有数据
                                                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                                        }
                                        break;
                                }
                        }
                        break;
                default:
                        break;
        }
        // end:
        if(data != NULL)
                ZENFREE(data);
        data = NULL;
        if(isupdateview == 1){

                ReDrawCurrentWindow2(6); //通知上层，重新刷新主窗口。

        }

}
void CCmdHandler::custom_caller_prep(unsigned long event[4])
{
        if(m_SysStatus == SYS_ACTIVE_CALL){
                if(m_is_phone_to_transmit == 0)
                        m_SysStatus = SYS_IDLE;
        }
}

void CCmdHandler::custom_caller_ringoff(unsigned long event[4])
{
        if(m_SysStatus == SYS_ACTIVE_CALL){
                //if(m_is_phone_to_transmit == 0)
        	mp3stop();
                        
        }

}
void CCmdHandler::custom_caller_ringon(unsigned long event[4])
{
        if(m_SysStatus == SYS_ACTIVE_CALL){
                if(m_mp3timerid != 0){
                        add_and_del_timer(TIME_MP3_EVENT_STOP);
                }
                if(m_ringtimerid != 0){
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
#ifdef HAVE_SUB_PHONE

                if(m_is_phone_call_room == 1)
                {
                        if(m_CurrentSocketIndex != -1)
                                SendCmd(m_CurrentSocketIndex,ROOM_IDLE);
                        return;
                }
                if(m_is_phone_to_transmit == 1)
                {
                        if(m_CurrentSocketIndex != -1)
                                SendCmd(m_CurrentSocketIndex,CENTER_IDLE);
                        return;

                }
#endif
				mp3stop();
                mp3play((char*)"/home/wrt/default/ringback.mp3",0,2);
        }
}

void CCmdHandler::custom_caller_holdon(unsigned long event[4])
{
        if(m_SysStatus == SYS_ACTIVE_CALL)
        {
                //通知GUI 中心摘机
                unsigned long event1[4];
                event1[0] = STATUS_CENTER_HOLDON;
                event1[1] = event1[2] = event1[3] =0;
                m_display(event1);
                mp3stop();
                talktostop();
                talkstop();
                SDL_Delay(100);
                
                g_holdonbyme = 1;
				wrthost_set_holdon_device(REMOTE_HOLDON);
#ifdef HAVE_SUB_PHONE
                if(m_is_phone_call_room == 1)
                {
                        if(m_CurrentSocketIndex != -1){
                        	SendCmd(m_CurrentSocketIndex,ROOM_HOLDON);
                            unsigned long phoneip = get_connect_ip(event[2]);
                            unsigned long tmpip = m_pSocketManager->GetIPAddrByIndex(m_CurrentSocketIndex);

							start_wrt_subroomaudio(phoneip,15004,event[1],15004,15004,15002);//phone == src,event[1]== dst
                        }
                        return;

                }
                if(m_is_phone_to_transmit == 1)
                {
                        if(m_CurrentSocketIndex != -1){
                        	SendCmd(m_CurrentSocketIndex,CENTER_HOLDON);
                            unsigned long phoneip = get_connect_ip(event[2]);
                            unsigned long tmpip= m_pSocketManager->GetIPAddrByIndex(m_CurrentSocketIndex);
							start_wrt_subroomaudio(phoneip,15004,event[1],15004,15004,15002);//phone == src,event[1]== dst
                                //通知对方，中心摘机。
                                WRT_MESSAGE("转发中心摘机消息给手机%d",m_CurrentSocketIndex);
                        }
                        return;

                }
#endif
                talkstart();
                talktostart( event[1],15004);
                SDL_Delay(300);
                if(get_vdec_type() >= V_MPEG4)
				{
					Startplayvideo(NULL);
				}
				if(get_venc_type()>= V_MPEG4)
                {
	            	Startsendvideo(&event[1]);
                }
        }
}

void CCmdHandler::custom_caller_callend(unsigned long event[4])
{
        if(m_SysStatus == SYS_ACTIVE_CALL) //
        {
                //通知GUI 中心摘机
                unsigned long event1[4];
                if(event[1] != 0)
                        event1[0] = event[1];
                else
                        event1[0] = STATUS_CENTER_HANGUP;
                event1[1] = event1[2] = event1[3] =0;
                m_display(event1);

                
                m_SysStatus  = SYS_IDLE;

                if(g_holdonbyme == 1)
                	g_holdonbyme = 0;
#ifdef HAVE_SUB_PHONE
                if(m_is_phone_call_room == 1){
                		stop_wrt_subroomaudio();
                        if((m_CurrentSocketIndex != -1) && (event[1]  == STATUS_ROOM_BUSY))
                                SendCmd(m_CurrentSocketIndex,ROOM_BUSY);
                        else
                                SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);


                        if(m_pSocketManager)
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;
                        m_is_phone_call_room = 0;
                }
                if(m_is_phone_to_transmit == 1)
                {
                        stop_wrt_subroomaudio();
                        if(m_CurrentSocketIndex != -1){
                                if(event[1]  == STATUS_CENTER_BUSY)
                                {

                                        SendCmd(m_CurrentSocketIndex,CENTER_BUSY);

                                }else{
                                        SendCmd(m_CurrentSocketIndex,CENTER_HANGUP);
                                }
                                //printf("room to phone send CENTER_HANGUP m_CurrentSocketIndex = %d\n",m_CurrentSocketIndex);
                                WRT_MESSAGE("转发中心挂机/繁忙消息给手机%d",m_CurrentSocketIndex);
                        }

                        if(m_pSocketManager)
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;
                        m_is_phone_to_transmit = 0;

                }
#endif
				talktostop();
                talkstop();
                mp3stop();
                Stopsendvideo();
               	if(get_vdec_type() >= V_MPEG4)
               	{
               		WRT_DEBUG("分机挂机关闭视频3");
                	Stopplayvideo();
               	}
        }
}

void StopTimer(int type)
{
        add_and_del_timer(type);
}

void CCmdHandler::ProcessPhone(unsigned long msg_buf[4])
{
#ifdef HAVE_SUB_PHONE
        WRT_MESSAGE("处理手机指令cmd %d ",msg_buf[0]);
        switch(msg_buf[0])
        {

        //	printf("ProcessPhone :0x%x\n",msg_buf[0]);
        case PHONE_SESSION_END:
               if(msg_buf[1] == ROOM_IDLE){
                        //启动视频
#ifdef HAVE_SUB_PHONE
                        //printf("PHONE_SESSION_END   msg_buf[1]  %d \n",ROOM_IDLE);
                        WRT_MESSAGE("手机应答空闲");
                        if((have_connect_phone() != 0 )&& (m_is_transmit_to_phone == 1))
                        {
                        	if(m_SysStatus == SYS_CENTERCALLROOMTALK)
                        		break;
                                if(m_SysStatus == SYS_SMALLDOORCALL)
                                        start_or_stop_smvideo_phone(m_phone_ip,15005);
                                else{
                                        start_phone_video2(get_connect_ip(msg_buf[2]),msg_buf[2]);
                                       // start_phone_video2(0x1901a8c0,0);
                                }
                        }
                        break;
#endif			
                }else if(msg_buf[1] == ROOM_BUSY){
                	break;
                }else if(msg_buf[1] == 0xfd){
                	stop_phone_video2(msg_buf[1]);
                	break;
                }
                m_is_transmit_to_phone = 0;
                stop_phone_video2(-1);
                break;
        case PHONE_HOLDON:
                room_holdon(-2,msg_buf); //摘机
                //通知上面回到主界面。
                break;
        case PHONE_HANGUP:
                room_hangup(-3); //手机挂机
                SetTransmitToPhone(0);
                //通知上面回到主界面时
                break;
        case PHONE_UNLOCK:
        case SUBROOM_UNLOCK:
                room_unlock(-1);
                break;
        case SUBROOM_AGENT:
        	room_manual_agent(-1);
        	break;
        case SUBROOM_SESSION_END:
        	if(msg_buf[1] == ROOM_IDLE){
        		if(m_SysStatus == SYS_TALKTOMAIN){
        			start_phone_video2(get_subroom_connect_ip(msg_buf[2]),msg_buf[2]);
        			break;
        		}
        	}else if(msg_buf[1] == ROOM_BUSY)
        		break;
        	else if(msg_buf[1] == 0xfd){
        		stop_phone_video2(msg_buf[2]);
			break;
		}
		break;
		case SUBROOM_HOLDON:
        	room_holdon(-4,msg_buf); //摘机
        	break;
        case SUBROOM_HANGUP:
        	room_hangup(-4);
                break;
        default:
                break;
        }
#endif
}

/************************************************************************
函数名称：ProcessCmd
函数功能：处理收到的所有的命令
输入参数：
sock_index :当前的socket的索引
cmd :       当前需要处理的命令
cmd_event:     当前命令所带的参数缓冲
buflen:     参数缓冲的大小
输出参数： 无
作者:     李建文
日期：    2007.10.25
************************************************************************/
void CCmdHandler::ProcessCmd(int sock_index,short cmd,unsigned long cmd_event[4])
{
        unsigned char* buf =0;
		printf("ProceessCmd9 :0x%x\n",cmd);

        switch(cmd){
        	
                //调用wrticaller
        case CUSTOM_CMD_CALLER_PREP:
                custom_caller_prep(cmd_event);
                break;
        case CUSTOM_CMD_CALLER_CALLEND:
                custom_caller_callend(cmd_event);
                break;
        case CUSTOM_CMD_CALLER_HOLDON:
                custom_caller_holdon(cmd_event);
                break;
        case CUSTOM_CMD_CALLER_RINGON:
                custom_caller_ringon(cmd_event);
                break;
        case CUSTOM_CMD_CALLER_RINGOFF:
                custom_caller_ringoff(cmd_event);
                break;
        case CUSTOM_CMD_LOCAL_UPGRADE:
	        	local_upgrade_room(cmd_event);
	        	break;
        case ROOM_ASKFOR_SYSINFO:

                room_ask_for_sysinfo(cmd_event);
                break;

        case CUSTOM_CMD_WARNING_TIMER: //启用60秒后防区报警
                g_save_waring_start = cmd_event[1];
                add_and_del_timer(TIME_WARNING_EVENT_START);
                break;
#ifdef HAVE_PLAY_JPEG
        case CUSTOM_CMD_RECORD_JPEG_FRAME:
                if(m_SysStatus  == SYS_SMALLDOORCALL)
                {
                		printf("小门口机呼叫...\n");
                        cap_smalldoor_photo_frame();//小门口机呼叫任务关闭
                }
                else
                {
                		printf("非小门口机呼叫...\n");
                        //CapturePhotoFrame();
                        g_video_rec_flag = 1;
                }
                
                break;
        case CUSTOM_CMD_PLAY_JPEG_FRAME:
                g_vod_select = cmd_event[1];
                add_and_del_timer2(TIME_PLAY_JPEG_START);
                break;
        case CUSTOM_CMD_STOP_JPEG_FRAME:
        		g_video_rec_flag = 0;
                //release_frame_header();
                //g_vod_select = -1;
                //add_and_del_timer2(TIME_PLAY_JPEG_STOP);
                break;
#endif
                //家居遥控器远程控制命令2009-7-30 9:05:57
        case RIU_KEY:
                riu_key(cmd_event);
                break;
        case RIU_ROOM_DEFEND:
                riu_room_defend(cmd_event);
                break;
        case RIU_ROOM_REMOVE_DEFEND:
                riu_room_remove_defend(cmd_event);
                break;
        case RIU_ROOM_OPEN_LOCK:
                riu_room_open_lock(cmd_event);
                break;
        case RIU_ROOM_REMOVE:
                riu_room_remove(cmd_event);
                break;
        case RIU_ROOM_WARNING:
        	riu_room_warning(cmd_event);
        	break;
                //小门口机命令
        case SMALLDOOR_CALLROOM://小门口机呼叫分机
                smalldoor_call_room(cmd_event);
                break;
        case SMALLDOOR_ACK_HOLDON://小门口机回应摘机
                smalldoor_ack_holdon(cmd_event);
                break;
        case SMALLDOOR_ACK_OPENLOCK://小门口机回应开锁
                smalldoor_ack_openlock(cmd_event);
                break;
        case SMALLDOOR_ACK_HANGUP://小门口机回应挂机
                smalldoor_ack_hangup(cmd_event);
                break;
        case SMALLDOOR_ACK_LEAVE:
                smalldoor_ack_leave(cmd_event);
                break;
        case SMALLDOOR_IDLE:
                smalldoor_ack_mont(cmd_event);
                break;
        case SMALLDOOR_ACK_STOPMONT:
                smalldoor_ack_stopmont(cmd_event);
                break;
        case CUSTOM_CMD_LEAVE:
                smalldoor_leave(cmd_event);
                break;
        case SMALLDOOR_ACK_VERSION://小门口应答版本
                break;
        case SIMULATE_UNLOCK:
                nvo_unlock(cmd_event);
                break;
        case SIMULATE_HANGUP:
                nvo_hangup(cmd_event);
                break;
        case SIMULATE_HOLDON: //检测到模拟分机已经摘机
                nvo_holdon(cmd_event);
                break;
                //中心、门口机呼叫分机流程
        case DOOR_CALLROOM: //门口机呼叫分机
        		Reset_negotiate_result();
                door_call_room(sock_index,cmd_event);
                break;
                //////////////////
                //2008.10.13
                //end   
        case ROOM_UNLOCK://分机开锁
                room_unlock(sock_index);
                break;
        case ROOM_HOLDON: //分机摘机，启动接收和发送audio
                room_holdon(sock_index,cmd_event);
                break;
        case ROOM_HANGUP://分机挂机
                room_hangup(sock_index);
                break;
        case CENTER_CALLROOM: //中心呼叫分机
        		Reset_negotiate_result();
                center_call_room(sock_index);
                break;
        case DOOR_UNLOCKACK: //门口机应答开门
                door_ack_unlock(sock_index);
                break;
        case DOOR_HANGUP: //门口机挂机
        		//SDL_Delay(1000);
                door_hangup(sock_index);
                //door_and_center_hangup(sock_index);
                break;
        case ROOM_AGENT: // 手动托管。
                room_manual_agent(sock_index);
                break;
        case CENTER_HANGUP://中心挂机
                door_and_center_hangup(sock_index);
                break;

                //分机呼叫中心流程
        case ROOM_CALLCENTER://分机呼叫中心，向中心发起连接

                room_call_center(sock_index);
                break;
        case CENTER_IDLE://中心应答空闲
                center_ack_idle(sock_index);
                break;
        case MEDIA_NEGOTIATE_SESSION://由主分机发送过来的协商指令
				set_vdec_type(V_H264);
				set_venc_type(V_NONE);
				set_adec_type(A_G711);
				set_aenc_type(A_G711);
                break;
        case ROOM_BUSY:    //分机繁忙
        case DOOR_BUSY:   //主机繁忙
        case CENTER_BUSY: //中心繁忙
                room_door_center_ack_busy(sock_index);
                break;
        case CENTER_HOLDON://中心应答摘机
                center_holdon(sock_index);
                break;
                //分机监视主机流程
        case ROOM_STARTMONT://分机启动监视，向监视点发起连接

                room_start_mont(sock_index,cmd_event);
                break;
        case DOOR_MONTACK: //主机应答监视
                door_ack_montack(sock_index);
                break;
        case ROOM_STOPMONT://分机停止监视
                room_stop_mont(sock_index);
                break;
        case ROOM_STOP_MONT_BYKEY:
        		room_stop_mont_byKey(sock_index);
                break;
        case DOOR_ENDMONT:
                door_stop_mont(sock_index);
                break;
                //分机呼叫分机
        case CUSTOM_CMD_CALL_ROOM://向其他分机发起呼叫
        		WRT_DEBUG("我呼叫分机");
                room_call_room(-1,0,0);
                break;
        case ROOM_CALLROOM:
        		WRT_DEBUG("分机呼叫我");
        		Reset_negotiate_result();
                room_call_room(sock_index,(unsigned char*)cmd_event[0],15);
                break;
        case CENTER_DIVERT_ROOM:
        		divert_room_to_room_deal();
        		break;
        case CENTER_DIVERT_CENTER:
        		divert_room_to_center_deal();
        		break;
        case ROOM_QUIET:
                room_ack_quiet(sock_index);
                break;
        case ROOM_IDLE: //分机应答空闲
                room_ack_idle(sock_index);
                break;
                //紧急求助
        case ROOM_EMERG://向中心发起连接
                room_emerg(sock_index);
                break;
        case ROOM_SERVICES_REQUEST:
                room_services_request(cmd_event);
                break;//
        case ROOM_INQSERVICEINFO:
                room_reqdocinfo(sock_index);
        case ROOM_INQFEE: //查询费用 400
                room_queryfee(sock_index,(unsigned char)cmd_event[1]);
                break;
        case ROOM_REPAIR: //维修求助
                room_repair(sock_index,(unsigned char)cmd_event[1]);
                break;
        case CENTER_REPAIREACK: //中心回应维修求助
                center_ack_repair(sock_index);
                break;
        case CENTER_GETIDTOIPTABLE:
                center_get_idtoiptable(sock_index);
                break;
        case ROOM_BROWSERING: //获得铃声列表，向中心发起连接
                room_browsering(sock_index);
                break;
        case ROOM_RINGDOWN: //下载铃声，向中心发起连接
                buf = (unsigned char*)cmd_event[1];


                if(pSystemInfo->mySysInfo.ishttpdown == 1){
#ifdef DOWNLOAD_FROM_HTTP 
/*               	
                        unsigned long tmpmsg[4];
                        tmpmsg[0] = CUSTOM_CMD_DOWNLOAD_MP3;
                        tmpmsg[1] = (unsigned long)buf;
                        tmpmsg[2] = cmd_event[2];
                        tmpmsg[3] = 0;
                        q_send(qid_download,tmpmsg);
*/                        
#endif                        
                }else{
                        if(buf != 0&& cmd_event[2] > 0){
                                memset(m_downname,0,41);
                                strcpy(m_downname,(char*)buf);
                                ZENFREE(buf);
                                room_ringdownload(sock_index,0);
                        }
                }
                break;
        case ROOM_INQHELPINFO:
                m_helpfilelen = cmd_event[1];
                room_reqhelpinfo(sock_index);
                break;
        case CENTER_CHECK_GUARD:
                center_check_guard(sock_index,ROOM_CENTER_CHECK_GUARD_ACK);
                break;
        case CENTER_RESET_ROOM_PASS://中心重置分机密码
                center_reset_room_pass(sock_index);
                break;
        case ROOM_WARNING: //报警，向中心发起连接
                room_waring(sock_index,(unsigned char)cmd_event[1]);
                break;
        case CENTER_RINGERRACK: //中心回应获取铃声错误
                center_ack_ring_error(sock_index);
                break;
        case CENTER_GET_SYSINFO: //中心查询用户的系统信息
                center_get_sysinfo(sock_index);
                break;
        case DOOR_STARTLEAVEWORD: //留言
        		if(g_is_talk == true)
        			break;
                door_start_leaveword(sock_index);
                break;
        case DOOR_STOPTLEAVEWORD://停止留言
                door_stop_leaveword(sock_index);
                break;
                /*
                case CENTER_EMERGACK:   //中心应答紧急求助
                // center_ack_emerg(sock_index);
                break;
                */
        case CENTER_GETALLSTATUS:
                ;//center_req_jiaju_status(sock_index,ROOM_STATUSACK);
                break;
        case ROOM_SYNC_AGENT:
                room_sync_agent(sock_index);
                break;
        case ROOM_SYNC_AVOID:
                room_sync_avoid(sock_index);
                break;
        case ROOM_CANCEL_SYNC_AGENT:
                room_cancel_sync_agent(sock_index);
                break;
        case ROOM_CANCEL_SYNC_AVOID:
                room_cancel_sync_avoid(sock_index);
                break;
        case ROOM_GETTIME:
                room_gettime(sock_index);
                break;
        case CENTER_SEND_RESET:
                {
                    unsigned long tmpevent[4];
                    m_pSocketManager->CloseSocket(sock_index);
                        
                    tmpevent[0] = STATUS_ROOM_SYS_UPDATE;
                    tmpevent[1] = 2;
                    tmpevent[2] = tmpevent[3] = 0;
                    m_display(tmpevent);
                }
                break;
        case SMALL_LEAVE_START:
                stopsmalldoorleavevoice();
                talkstart();
                talktostart(0,0);
                startsmalldoorleave();
                add_and_del_timer(TIME_DOOR_LEAVE_EVENT_START);
                break;
        case CUSTOM_CMD_CANCEL:  //取消主动连接
                custom_cancel_cmd(cmd_event);
                break;
        case CUSTOM_CMD_PHOTO:
              //  if(m_SysStatus  == SYS_SMALLDOORCALL){
              //          cap_smalldoor_photo();//小门口机处理此命令任务关闭
              //          break;
              //  }
                WRT_DEBUG("CUSTOM_CMD_PHOTO");
                CapturePhoto();
                break;
        case CUSTOM_SYSTEM_FORMAT:
                if(cmd_event[1] == 1){
                        if(m_SysStatus == SYS_IDLE)
                                m_SysStatus = SYS_FORMAT;
                }else{
                        if(m_SysStatus == SYS_FORMAT)
                                m_SysStatus = SYS_IDLE;
                }
                break;
        case MP3_RING_STOP:
                mp3stop();
                add_and_del_timer(TIME_MP3_EVENT_STOP);
                break;
        case VIEW_TIME:
                {
                        m_displayevent[0] = STATUS_ROOM_VIEW_TIME;
                        m_displayevent[1] = m_displayevent[2] = m_displayevent[3] = 0;
                        m_display(m_displayevent);
                }
                break;
        case CENTER_GET_ELEC_CONFIG:  //2009-7-29 14:24:15
                center_get_elec_file_config(sock_index);
                break;
        case ROOM_REMOTE_SWITCH: //2009-7-29 14:23:25
                send_control_cmd_to_center(cmd_event);
                break;
        case  ROOM_SEND_COLOR_MSG:
        	if(sock_index == -1)
        		SendColorMsg(cmd_event);
        	break;
        case ROOM_CALL_LIFT:
                if(sock_index == -1)
                        room_call_lift(cmd_event);
                break;
        case CUSTOM_CMD_ROOM_START_RECORD:   //2009-7-15 录音
                if(m_SysStatus == SYS_IDLE){
                        m_SysStatus = SYS_ISRECORD;
                        talkstart();
                        //talktostart(0,0);
                        start_record();
                        add_and_del_timer2(TIME_RECORD_EVENT_START);
                        m_displayevent[0] = STATUS_RECORD_START;
                        m_displayevent[1] = m_displayevent[2] = m_displayevent[3] = 0;
                        m_display(m_displayevent);
                }
                break;
        case CUSTOM_CMD_ROOM_STOP_RECORD:  //2009-7-15 停止录音
                if(m_SysStatus == SYS_ISRECORD){
                        WRT_MESSAGE("接收指令停止本机留言 CUSTOM_CMD_ROOM_STOP_RECORD");
						
                        stop_record();
                        
                        //talktostop();
                        talkstop();
                        add_and_del_timer2(TIME_RECORD_EVENT_STOP);
                        m_displayevent[0] = STATUS_RECORD_STOP;
                        m_displayevent[1] = m_displayevent[2] = m_displayevent[3] = 0;
                        m_display(m_displayevent);
                        m_SysStatus = SYS_IDLE;
                }
                break;
        case CUSTOM_CMD_IS_SCREEN_ADJUST:
                if(cmd_event[1] == 0){

                        if(m_SysStatus == SYS_IDLE){
                                m_SysStatus = SYS_ISSCREENADJUST;
                        }else{
                                room_hangup(-2);
                        }
 
                }else if(cmd_event[1] == 1){

                        if(m_SysStatus == SYS_ISSCREENADJUST){
                                m_SysStatus = SYS_IDLE;
                        }
                }
                break;


#ifdef DOWNLOAD_FROM_HTTP
/*
        case CUSTOM_CMD_DOWNLOAD_OVER:
                if(pSystemInfo->mySysInfo.ishttpdown == 1){
                        unsigned long tmpevent[4];
                        if(cmd_event[2] == 0 && cmd_event[1] == 0){
                                ZENFREE((char*)cmd_event[3]);
                                tmpevent[0] = STATUS_RING_DOWNLOAD;
                                tmpevent[1] = (unsigned long)0;
                                tmpevent[2] = tmpevent[3] = -2;
                                m_display(tmpevent);

                                break;
                        }

                        RINGLIST* pTemp = GetRingInfo((char*)cmd_event[3]);
                        if(pTemp != NULL){
                                if(pTemp->addr != 0)
                                        ZENFREE(pTemp->addr);
                                pTemp->addr = (unsigned char*)cmd_event[1];
                                pTemp->len  = cmd_event[2];
                                ZENFREE((char*)cmd_event[3]);
                                //pTemp->isdown = AddRingToDownList(pTemp->name,buflen-40,buf+40);
                                tmpevent[0] = STATUS_RING_DOWNLOAD;
                                tmpevent[1] = (unsigned long)pTemp;
                                tmpevent[2] = tmpevent[3] = 0;
                                m_display(tmpevent);
                        }
                }
                break;
*/                
#endif
        case SIP_CALL:
                room_call_sip(cmd_event);
                break;
        case ROOM_GET_WEATHER:
        	room_get_weather(sock_index);
        	break;
		case IPCAMERA_MONITOR_START:
			room_start_mont_ipc(sock_index,cmd_event);
			break;
		case IPCAMERA_MONITOR_END:
			room_stop_mont_ipc(sock_index);
			break;

        default:
                if(sock_index != -1 && m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
                break;
        }
}
/**
*函数名称：ProcessPacket
*参    数：int   socket_index
short cmd
unsigned char* buf
int buflen
*功    能：处理带缓冲包的命令
*返    回：
*/

void CCmdHandler::ProcessPacket(int socket_index,short cmd,unsigned char* buf,int buflen){
		printf("ProcessPacket :0x%x\n",cmd);

        switch(cmd){
        
        case CENTER_AGENT:
                center_ack_agent(socket_index,buf,buflen);
                break;
        case CENTER_MINOR:
                center_ack_minor(socket_index,buf,buflen);
                break;
        case CENTER_DIVERT_ROOM:
                center_ack_divert_room(socket_index,buf,buflen);
                break;
        case CENTER_DIVERT_CENTER:
                center_ack_divert_center(socket_index,buf,buflen);
                break;
        case CENTER_SERVERINFOACK:
                center_ack_reqdocinfo(socket_index,buf,buflen);
                break;
        case CENTER_SERVICES_ACK:
                center_services_ack(socket_index,buf,buflen);
                break;
        case CENTER_INQFEEACK: //中心回应费用查询
                center_ack_queryfee(socket_index,buf,buflen);
                break;
        case CENTER_RINGLSTACK://中心回应的铃声列表
                center_ack_browserring(socket_index,buf,buflen);
                break;
        case CENTER_RINGDOWNACK://中心回应铃声下载
                center_ack_downloadring(socket_index,buf,buflen);
                break;
        case CENTER_SEND_SYSINFO://中心更新系统信息
                center_set_sys_config(socket_index,buf,buflen);
                break;
        case CENTER_SET_IOLEVEL:
        		center_set_iolevel(socket_index,buf,buflen);
        		break;
        case CENTER_FACTORY_SETTING:
                center_set_factory_info(socket_index,buf,buflen);
                break;
        case CENTER_ASKFOR_SYSINFO_ACK:
                center_ask_for_sysinfo_ack(socket_index,buf,buflen);
                break;
        case CENTER_BROADCASTPIC:
        case DOOR_BROADCASTPIC: //中心发布图片公告
                door_broadcast_pic(socket_index,buf,buflen);
                break;
        case CENTER_BROADCASTDATA:
        case DOOR_BROADCAST://中心发布文字公告
                door_public_info(socket_index,buf,buflen);
                break;
        case CENTER_WEATHER_ACK: //中心回应获取IP地址
                center_ack_weather(socket_index,buf,buflen);
                break;
        case CENTER_UPGRADEROOM: //升级
                center_upgrade_room(socket_index,buf,buflen,0);
                break;
                //分机呼叫分机
                // case ROOM_CALLROOM://其他分机呼叫我
                //     room_call_room(socket_index,buf,buflen);
        case CENTER_UP_20:
                center_upgrade_room(socket_index,buf,buflen,1);
                break;
        case ROOM_AGENT: //户户通的时候，其他分机应答托管
                room_ack_agent(socket_index,buf,buflen);
                break;
        case CENTER_TIMEACK:
                center_ack_gettime(socket_index,buf,buflen);
                break;
        case CENTER_SETELEC:
                center_set_jiaju(socket_index,buf,buflen,ROOM_SETELECACK);
                break;
        case CENTER_GETELEC:
                center_get_jiaju_status(socket_index,buf,buflen,ROOM_GETELECACK);
                break;
        case CENTER_SETSCENE:
                center_set_jiaju_scene(socket_index,buf,buflen,ROOM_SCENEACK);
                break;
        case CENTER_ARRANGE_GUARD:
                center_arrange_guard(socket_index,buf,buflen,ROOM_CENTER_ARRANGE_GUARD_ACK);
                break;
        case  CENTER_SETIDTOIPTABLE:
                center_set_idtoip_table(socket_index,buf,buflen);
                break;
        case BLTN_SETUP:
                center_set_pic(socket_index,buf,buflen);
                break;
        case CENTER_SET_ELEC_CONFIG: //2009-7-29 14:25:32
                center_set_elec_file_config(socket_index,buf,buflen);
                break;
        case CENTER_REMOTE_SINGLE_GUARD://2009-7-29 14:25:36
                center_remote_single_guard(socket_index,buf,buflen,ROOM_SIGNLE_GUARD_ACK);
                break;
        case PHONE_INV_ROOM:
                phone_invite_room(socket_index,buf,buflen);
                break;
        case PHONE_INQ_ALL_HOME_STATUS:
                phone_inq_all_home_status(socket_index,buf,buflen);
                break;
        case PHONE_CTRL_MORE_SAFE_STATUS:
                phone_ctrl_more_safe_status(socket_index,buf,buflen);
                break;
        case PHONE_CTRL_SAFE_STATU:
                phone_ctrl_safe_status(socket_index,buf,buflen);
                break;
        case PHONE_INQ_ALL_SAFE_STATUS:
                phone_inq_all_safe_status(socket_index,buf,buflen);
                break;
        case PHONE_CTRL_SCENE:
                phone_ctrl_scene(socket_index,buf,buflen);
                break;
        case PHONE_CTRL_ONE_DEVICES:
                phone_ctrl_one_devices(socket_index,buf,buflen);
                break;
        case PHONE_INQ_ONE_DEVICES:
                phone_inq_one_devices(socket_index,buf,buflen);
                break;
                //手机处理
        case ROOM_CALLCENTER:
#ifdef HAVE_SUB_PHONE
                room_call_center(socket_index); //in SYS_ACTIVE_CALL
#endif
                break;
                
        case ROOM_HANGUP: //手机挂机
#ifdef HAVE_SUB_PHONE
                if(m_SysStatus == SYS_ACTIVE_CALL)
                {
					room_hangup(-3);
                }
#endif
                break;
                
        case ROOM_STOPMONT:
#ifdef HAVE_SUB_PHONE
                room_stop_mont(socket_index);
                //start_phone_video(m_pSocketManager->GetIPAddrByIndex(socket_index),15005);
               // g_phone_connect_index = -1;
#endif
                break;
        case ROOM_STARTMONT:
#ifdef HAVE_SUB_PHONE
                {
                        unsigned long msgbuf[4];
                        msgbuf[0] = 0;
                        msgbuf[1] = 0;
                        msgbuf[2] = ROOM_STARTMONT;
                        WRT_DEBUG("m_DestNumber = %s",m_DestNumber);
                        m_Montipaddr = 0;
                        printf("mont -111- m_DestNumber = %s m_Montipaddr = 0x%x\n",m_DestNumber,m_Montipaddr);
                        if(strncmp(m_DestNumber+2,"00000000",8) == 0 && strncmp(m_DestNumber+13,"00",2) !=0){
                                m_Montipaddr = pSystemInfo->LocalSetting.publicroom.MainGate[0].IP;

                        }else if(strncmp((char*)m_DestNumber,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0){ //小门口机
                        	char isdi[3]={0};
                        	int id = 0;
                        	isdi[0] = m_DestNumber[13];
                        	isdi[1] = m_DestNumber[14];
                        	id = strtoul(isdi,NULL,16);
                        	if(id> 16 && id < 32){
                                	/*
                                	msgbuf[1] = MONITOR_TYPE_SMALLDOOR;
                                	m_Montipaddr = id;
                                	*///去掉监视模拟小门口机
                                	m_Montipaddr =  0;
                        	}else if( id > 31){
                        			WRT_DEBUG("是数字小门口机\n");
                                	get_ipbyid2(m_DestNumber,&m_Montipaddr);
                        		
                        	}
                        }else
                                m_Montipaddr = pSystemInfo->LocalSetting.publicroom.Door[0].IP;
                                
                        if(m_Montipaddr == 0 && socket_index != -1){
                                	SendCmd(socket_index,DOOR_BUSY);
                                	if(m_pSocketManager)
                                        	m_pSocketManager->CloseSocket(socket_index);
                                	g_phone_connect_index = -1;
                                	return;
                        }                                
                        if(SYS_MONT == m_SysStatus )
                        {
                        	SendCmd(socket_index,DOOR_BUSY);
                        	if(m_pSocketManager)
                                	m_pSocketManager->CloseSocket(socket_index);
                        	g_phone_connect_index = -1;
                        	printf("手机监视主机 111\n");
                        	return;
                        }
						g_phone_mont = 1;
                        room_start_mont(socket_index,msgbuf);
                        //保存手机监视SOCKET;
                        if(g_phone_connect_index != -1){
                                msgbuf[0] = STATUS_SCREEN_CLOSE;
                                msgbuf[1] = msgbuf[2] = msgbuf[3] = 0;
                                m_display(msgbuf);
                        }
                }
#endif
                break;
        case ROOM_CALL_LIFT:
#ifdef HAVE_SUB_PHONE
                {
                        unsigned long msgbuf[4]={0};
                        msgbuf[1] = pSystemInfo->LocalSetting.publicroom.Door[0].IP;
                        room_call_lift(msgbuf);
                        if(m_pSocketManager)
                                m_pSocketManager->CloseSocket(socket_index);
                }
#endif
                break;
        case MEDIA_NEGOTIATE:
        		WRT_DEBUG("媒体协商处理");
        		room_ack_media_negotiate(socket_index,buf,buflen);

        		break;
        case MEDIA_NEGOTIATE_ACK:
        		WRT_DEBUG("收到媒体协商处理");

        		Set_local_mediaInfo((char *)(buf));
        		printf("本地解码%d\n",get_vdec_type());
        		
        		break;
        case ROOM_CALLROOM:
        		
        		//room_call_room2(socket_index,buf,buflen);
#if 1
#ifdef HAVE_SUB_PHONE
                {
                       
                        //更具m_DestName 来查询IP.
                        unsigned long  ulip[4]={0};
                        int num = 4;
                        if(strncmp((char*)m_DestNumber,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0){
                                WRT_MESSAGE("通知手机分机繁忙");
                                SendCmd(socket_index,ROOM_BUSY);
                                if(m_pSocketManager)
                                        m_pSocketManager->CloseSocket(socket_index);
                                break;
                        }

                        get_ipbyid(m_DestNumber,ulip,&num);
                        WRT_MESSAGE("手机呼叫分机m_destname = %s ,ip = 0x%x ,num = %d",m_DestNumber,ulip[0],num);
                        if(num == 0){
                                SendCmd(socket_index,ROOM_BUSY);
                                if(m_pSocketManager)
                                        m_pSocketManager->CloseSocket(socket_index);
                                break;
                        }
                        for(int i = 0; i<num ;i++)
                        {
                                m_callipaddr[i] = ulip[i];
                        }
						m_is_phone_call_room = 1;
                        room_call_room(socket_index,0,0);
                }
#endif
#endif
                break;

                //
        case ROOM_BUSY:    //分机繁忙
        case DOOR_BUSY:   //主机繁忙
        case CENTER_BUSY: //中心繁忙
                room_door_center_ack_busy(socket_index,buf,buflen);
                break;
        case CENTER_BOMB:
                center_set_bomb(socket_index,buf,buflen);
                break;
        case CENTER_SET_ROOM_SCR: //[2011-5-16 8:53:58]
        	center_set_room_scr(socket_index,buf,buflen);
        	break;
        default:
                if(socket_index != -1 && m_pSocketManager){
                        m_pSocketManager->CloseSocket(socket_index);
                }
                break;
        }
}
/*
*手机控制家居协议处理
*/

/*
* 功能：手机号码和密码验证
*返回值：0：未知指令，1:非法手机，2，密码错误，3，连接成功
*/
unsigned char CCmdHandler::phone_confirm(unsigned char* buf,int buflen)
{
        int i =0;
        int result = 0;
        if(buflen < 8) //至少8个字节的密码
                return 0x0; 
        for(i = 0 ;i<8; i++)
        {
                if(strncmp(m_UserPhoneNumber,pSystemInfo->mySysInfo.phonenumber[i],strlen(m_UserPhoneNumber)) == 0){
                        result = 1;
                        break;
                }
        }
        if(result  == 1)
        {
                char ch_pwd[9];
                char ch_pwd1[9];
                memset(ch_pwd,0,9);
                memset(ch_pwd1,0,9);
                memcpy(ch_pwd,buf,8);
                memcpy(ch_pwd1,pSystemInfo->mySysInfo.password,8);
                if(strlen(ch_pwd) != strlen(ch_pwd1))
                        return 0x02;
                if(strncmp(ch_pwd,ch_pwd1,strlen(ch_pwd1)) == 0)
                        return 0x03;
                return 0x02;

        }
        return  0x01;
}

/*
* 功能：手机握手分机
*/
void CCmdHandler::phone_invite_room(int sock_index,unsigned char* buf,int buflen)
{
        unsigned long tmpevent[4];
        int packlen=0;
        unsigned char ret = phone_confirm(buf,buflen);
        tmpevent[0] = sock_index;
        tmpevent[1] = (unsigned long)PackCmd(ROOM_ACK_INV_PHONE,&ret,1,&packlen);
        tmpevent[2] = packlen;
        tmpevent[3] = 0;
        if(m_pSocketManager){
#ifdef HAVE_SUB_PHONE
                if(ret ==0x03)
                {
                	int i =0;
					unsigned long msg_buf[4]={0};
					msg_buf[2] = -1;
					for(i = 0 ;i<3; i++)
					{
						 if(strncmp(m_UserPhoneNumber,pSystemInfo->mySysInfo.phonenumber[i],strlen(m_UserPhoneNumber)) == 0){
							msg_buf[2] = i;
							 break;
						}
					}
                    m_phone_ip = m_pSocketManager->GetIPAddrByIndex(sock_index);
                    WRT_MESSAGE("手机注册ip = 0x%x ",m_phone_ip);
		        	msg_buf[0] = CONNECT_PHONE;
		       		msg_buf[1] = m_phone_ip;
		        	send_msg_to_sub_phone(msg_buf);
                }
#endif
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,(void*)tmpevent,sizeof(unsigned long)*4);
                //  m_pSocketManager->CloseSocket(sock_index);
        }
}

/*
*功能： 手机请求所有的家居状态
*/
void CCmdHandler::phone_inq_all_home_status(int sock_index,unsigned char* buf,int buflen)
{

        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                // if(m_pSocketManager)
                //        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_req_jiaju_status(sock_index,ROOM_ACK_ALL_HOME_STATUS);

}

/*
*功能： 手机控制多个防区
*/
void CCmdHandler::phone_ctrl_more_safe_status(int sock_index,unsigned char* buf,int buflen)
{
        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                // if(m_pSocketManager)
                //         m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_arrange_guard(sock_index,buf+8,buflen-8,ROOM_ACK_CTRL_MORE_SAFE_STATUS);
}

/*
*功能： 手机控制单个设备
*/
void CCmdHandler::phone_ctrl_one_devices(int sock_index,unsigned char* buf,int buflen)
{
        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                //if(m_pSocketManager)
                //        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_set_jiaju(sock_index,buf+8,buflen-8,ROOM_ACK_CTRL_ONE_DEVICES);
}

/*
*功能： 手机控制单个防区
*/
void CCmdHandler::phone_ctrl_safe_status(int sock_index,unsigned char* buf,int buflen)
{
        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                // if(m_pSocketManager)
                //        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_remote_single_guard(sock_index,buf+8,buflen-8,ROOM_ACK_CTRL_SAFE_STATUS);
}


/*
*功能： 手机控制情景
*/
void CCmdHandler::phone_ctrl_scene(int sock_index,unsigned char* buf,int buflen)
{
        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                // if(m_pSocketManager)
                //         m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_set_jiaju_scene(sock_index,buf+8,buflen-8,ROOM_ACK_CTRL_SCENE);
}


/*
*功能： 手机请求所有的防区状态
*/
void CCmdHandler::phone_inq_all_safe_status(int sock_index,unsigned char* buf,int buflen)
{
        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                // if(m_pSocketManager)
                //         m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_check_guard(sock_index,ROOM_ACK_INQ_ALL_SAFE_STATUS);

}

/*
*功能： 手机请求单一设备状态
*/
void CCmdHandler::phone_inq_one_devices(int sock_index,unsigned char* buf,int buflen)
{
        unsigned char ret = phone_confirm(buf,buflen);
        if(ret != 0x03){
                // if(m_pSocketManager)
                //         m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        center_get_jiaju_status(sock_index,buf+8,buflen-8,ROOM_ACK_INQ_ONE_DEVICES);
}


/*----------------------------------------------------------------------------------------*/

void CCmdHandler::center_get_idtoiptable(int sock_index)
{

        unsigned char* buf = 0;
        int len =0;
        buf = get_idtoipdata(&len);
        if(buf == NULL){
                SendCmd(sock_index,ROOM_ACK_GETIDTOIP);
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
                return ;
        }
        if(len > 216){

                SOCKET_PACKET packet;
                int cmdlen = len+40;
                short cmd;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
                cmd = htons(ROOM_ACK_GETIDTOIP);
                packet.sock_index = sock_index;
                packet.validlen = 256;
                memset((char*)packet.buf,0,256);
                strncpy((char*)packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
                memcpy(packet.buf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                memcpy(packet.buf+40,buf,256-40);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
                int relen = 0;
                relen = len-216;
                int count = relen /256;
                int count1 = relen%256;
                int i =0;
                for(i = 0; i<count;i++){
                        memset(&packet,-1,sizeof(SOCKET_PACKET));
                        packet.sock_index = sock_index;
                        packet.validlen = 256;
                        memset((char*)packet.buf,0,256);
                        memcpy((char*)packet.buf,buf+216+count*256,256);
                        m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
                }
                if(count1 != 0){
                        memset(&packet,-1,sizeof(SOCKET_PACKET));
                        packet.sock_index = sock_index;
                        packet.validlen = len-216-(count)*256;
                        memset((char*)packet.buf,0,256);
                        memcpy((char*)packet.buf,buf+216+(count)*256,packet.validlen);
                        m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
                }
        }else{
                SOCKET_PACKET packet;
                int cmdlen = len+40;
                short cmd;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
                cmd = htons(ROOM_ACK_GETIDTOIP);
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                memset((char*)packet.buf,0,256);
                strncpy((char*)packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);
                memcpy(packet.buf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                memcpy(packet.buf+40,buf,len);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
        }
        ZENFREE(buf);
        if(m_pSocketManager)
                m_pSocketManager->CloseSocket(sock_index);


}

extern void reinit_subroom_ip();
extern unsigned long get_sub_room_ip(int num);
void CCmdHandler::center_set_idtoip_table(int sock_index,unsigned char* buf,int buflen){
        unsigned long time1,time2;
		int i = 0;
		int j = 0;

        //  tm_getticks(NULL,&time1);
        save_idtoip(buf,buflen);
        //  tm_getticks(NULL,&time2);
        if(m_SysStatus != SYS_GETSYSINFO){
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
        }

        /*重新设置扩展分机*/
        //查询自己的扩展分机,要求从新启动机器,或者在自动配置的时候，自己查询？？
        //查询自己的扩展分机,要求从新启动机器,或者在自动配置的时候，自己查询？？
        for(i =1; i<9; i++) //1-8
        {
        	subroomipaddr[j] = 0;
    		subroomipaddr[j] = get_sub_room_ip(i);
    		j++;
                }
        if(g_localid == 0x01)//1
        {
        	for(i = 1;i<8;i++)
        	{
        		if(subroomipaddr[i] != 0 && subroomipaddr[i] != 0xffffffff)
        		{
	        		room_active_sync(subroomipaddr[i]);
                        }
                }
        }else//2-4
        {
        	if(subroomipaddr[0] != 0 && subroomipaddr[0] != 0xffffffff)
	        	room_req_sync(subroomipaddr[0]);
        }
		reinit_subroom_ip();
        WRT_MESSAGE("中心重新设置地址表 m_SysStatus = %d",m_SysStatus);
}
extern T_ROOM_MORE_CFG  *p_MorecfgInfo;
void CCmdHandler::center_set_iolevel(int sock_index,unsigned char* buf,int buflen){
	int i =0;
	unsigned long tmpevent[4]={0};
	if(buflen != 18)
		return;
	for(i = 0;i<18;i++)
		p_MorecfgInfo->validlevel[i] = buf[i];
	write_Morecfg_file();
	SendCmd(sock_index,CENTER_SET_IOLEVEL_ACK);
	tmpevent[0]=STATUS_SET_IOLEVEL_EVENT;
    tmpevent[1]=0;
    tmpevent[2]=tmpevent[3] = 0;
    m_display(tmpevent);
}

//中心远程控制防区
void CCmdHandler::center_arrange_guard(int sock_index,unsigned char* buf,int buflen,short scmd)
{
        unsigned long tmpevent[4];
        unsigned char cbuf[4];
        int isdefend=0;
        int ret = 0;
        int ret2 = 0;
        /* 2009-7-29 暂时去掉 远程返去开关量
        if(pSystemInfo->mySysInfo.isremote == 0){ //关闭了控制功能。
        SendCmd(sock_index,ROOM_CENTER_ARRANGE_GUARD_ACK,0);
        if(m_pSocketManager)
        m_pSocketManager->CloseSocket(sock_index);
        return ;
        }
        */
		WRT_MESSAGE("中心远程控制防区");
        if(buflen != 4){

                SendCmd(sock_index,scmd,0);
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
		memcpy(cbuf,buf,4);
		if((cbuf[0]&0x1) || (cbuf[0]&0x2))
			;
		else
		{
        	if(!isenablesafe()){
                SendCmd(sock_index,scmd,0);
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
	        }
        }
        for(int i =0 ;i < 4;i++){
                if(cbuf[i] & 0x1){ //1防区设置
                        isdefend = 1;
                        ret2++;
                        if((i == 0) && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS){
                        	if(pSystemInfo->mySysInfo.safestatus[i*4+16] == 0)
                        		pSystemInfo->mySysInfo.safestatus[i*4+16] = 1;
                        }
                        if(pSystemInfo->mySysInfo.safestatus[i*4+16] != 0x0)
                                set_security_status(i*4,true);
                        else
                                ret++;
                }else{
                        set_security_status(i*4,false);
                        if((i == 0) && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS)
                        	pSystemInfo->mySysInfo.safestatus[i*4+16]  = 0x0;
                }

                if(cbuf[i] & 0x2){ //1防区设置
                        isdefend = 1;
                        ret2++;
                        if((i == 0) && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS){
                        	if(pSystemInfo->mySysInfo.safestatus[i*4+1+16] == 0)
                        		pSystemInfo->mySysInfo.safestatus[i*4+1+16] = 1;
                        }
	                        if(pSystemInfo->mySysInfo.safestatus[i*4+1+16] != 0x0)
	                                set_security_status(i*4+1,true);
	                        else
	                                ret++;
                }else{
                        set_security_status(i*4+1,false);
                        if((i == 0) && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS)
                        	pSystemInfo->mySysInfo.safestatus[i*4+1+16]  = 0x0;
                }

                if(cbuf[i] & 0x4){ //1防区设置
                        isdefend = 1;
                        ret2++;
                        if(pSystemInfo->mySysInfo.safestatus[i*4+2+16] != 0x0)
                                set_security_status(i*4+2,true);
                        else
                                ret++;
                }else
                	set_security_status(i*4+2,false);

                if(cbuf[i] & 0x8){ //1防区设置
                        isdefend = 1;
                        ret2++;
                        if(pSystemInfo->mySysInfo.safestatus[i*4+3+16] != 0x0)
                                set_security_status(i*4+3,true);
                        else
                                ret++;
                }else
                        set_security_status(i*4+3,false);

        }
        if(ret == ret2 && isdefend == 1){
                SendCmd(sock_index,scmd,0);
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }


        SendCmd(sock_index,scmd,1);
        if(m_pSocketManager && scmd != ROOM_ACK_CTRL_MORE_SAFE_STATUS)
                m_pSocketManager->CloseSocket(sock_index);

        tmpevent[0]=STATUS_REMOTE_ALARM;
        tmpevent[1]=isdefend;
        tmpevent[2]=tmpevent[3] = 0;
        m_display(tmpevent);
        ReDrawCurrentWindow2(3);
        //if(isdefend == 1)
                UpdateSystemInfo();
}

void CCmdHandler::center_check_guard(int sock_index,short scmd)
{
        int itmp = 0;
        unsigned char tmpbuf[4];
        memset(tmpbuf,0,4);
        WRT_MESSAGE("中心查询防区状态");
        for(int i =0; i<16;i+=4){
                itmp = i/4;
                if(get_security_status(i)){
                        tmpbuf[itmp] |= 0x1;
                }
                if(get_security_status(i+1)){
                        tmpbuf[itmp] |= 0x2;
                }
                if(get_security_status(i+2)){
                        tmpbuf[itmp] |= 0x4;
                }
                if(get_security_status(i+3)){
                        tmpbuf[itmp] |= 0x8;
                }
        }
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                int templen =0;
                short cmd;
                //int tmpcmd = ROOM_CENTER_CHECK_GUARD_ACK;
                memset(&packet,-1,sizeof(SOCKET_PACKET));
#ifdef USED_NEW_CMD
                int cmdlen = 40;
                int offset = 40;
#else
                int cmdlen = 10;
                int offset = 10;
#endif
                cmd = htons(scmd);
                packet.sock_index = sock_index;
                packet.validlen = cmdlen;
                memset(packet.buf,0,256);
                strncpy((char*)packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&cmdlen,4);
                memcpy(packet.buf+8,&cmd,2);

                memcpy(packet.buf+offset,tmpbuf,4);
                templen =4;
                cmdlen +=templen;
                memcpy(packet.buf+4,&cmdlen,4);
                packet.validlen  = cmdlen;
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
                if(scmd != ROOM_ACK_INQ_ALL_SAFE_STATUS)
                        m_pSocketManager->CloseSocket( sock_index);
        }

}
void CCmdHandler::center_reset_room_pass(int sock_index)
{
        if(SYS_ACTIVE_CALL == m_SysStatus )
        {
		memset(pSystemInfo->mySysInfo.password,0,8);
                memcpy(pSystemInfo->mySysInfo.password,"88888888",8);
                modifypwd();
		UpdateSystemInfo();
                WRT_MESSAGE("中心重置密码");
#ifdef WRT_MORE_ROOM
                //当触控分机修改了防区布撤防状态，同步防区状态
                sync_pwd(0,pSystemInfo->mySysInfo.password,8);                       
#endif
                if(sock_index != -1)
                        SendCmd(sock_index,ROOM_RESET_PASS_ACK);
        }
        if(sock_index != -1)
                m_pSocketManager->CloseSocket(sock_index);

}


//2009-7-29 14:17:11
/**
*中心远程单独控制防区 
*/
void CCmdHandler::center_remote_single_guard(int sock_index,unsigned char* buf,int buflen,short scmd){
        if(buflen != 2 ){
                if(scmd != ROOM_ACK_CTRL_SAFE_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        unsigned long tmpevent[4];
        unsigned char tmpbuf[3];
        int packlen = 0;
        tmpbuf[0] = buf[0];
        tmpbuf[1] = buf[1];
        WRT_MESSAGE("中心远程单独控制防区");
        if(buf[0] == 0x0){ //撤防
                int safeindex = buf[1];
                if(pSystemInfo->mySysInfo.safestatus[(safeindex-1)+16] == 0)
                        tmpbuf[2] = 0x02;
                else{
                        set_security_status(safeindex-1,false);
                        
                        tmpevent[0]=STATUS_REMOTE_ALARM;
                        tmpevent[1]=0;
                        tmpevent[2]=tmpevent[3] = 0;
                        m_display(tmpevent);
                        ReDrawCurrentWindow2(3);
                        if(safeindex == 1 || safeindex == 2)
                                tmpbuf[2] = 0x04;
                        else
                                tmpbuf[2] = 0x03;
                        UpdateSystemInfo();
                }

        }else if(buf[0] == 0x01){ //布防
                int safeindex = buf[1];
                if(pSystemInfo->mySysInfo.safestatus[(safeindex-1)+16] == 0)
                        tmpbuf[2] = 0x02;
                else{
                        set_security_status(safeindex-1,true);
                        
                        tmpevent[0]=STATUS_REMOTE_ALARM;
                        tmpevent[1]=1;
                        tmpevent[2]=tmpevent[3] = 0;
                        m_display(tmpevent);
                        ReDrawCurrentWindow2(3);
                        tmpbuf[2] = 0x03;
                        UpdateSystemInfo();
                }
        }else if(buf[0] == 0x02){ //查询
                int safeindex = buf[1];
                if(pSystemInfo->mySysInfo.safestatus[(safeindex-1)+16] == 0)
                        tmpbuf[2] = 0x02;
                else{
                        tmpbuf[2] = (unsigned char)get_security_status(safeindex-1);
                }
        }
        tmpevent[0] = sock_index;
        tmpevent[1] = (unsigned long)PackCmd(scmd,tmpbuf,3,&packlen);
        tmpevent[2] = packlen;
        tmpevent[3] = 0;
        if(m_pSocketManager){
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,(void*)tmpevent,sizeof(unsigned long)*4);
                if(scmd != ROOM_ACK_CTRL_SAFE_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
        }
       
}
/**
*中心设置家居配置表
*2009-7-29 14:46:56
*/
void CCmdHandler::center_set_elec_file_config(int sock_index,unsigned char* buf,int buflen){
	WRT_MESSAGE("中心远程设置家居配置表");
        int ret = Set_jiaju_table_file(buf,buflen);

        SOCKET_PACKET packet;
        int cmdlen = 41;
        short cmd;
        memset(&packet,0,sizeof(SOCKET_PACKET));
        cmd = htons(ROOM_SET_ELECFILE_ACK);
        packet.sock_index = sock_index;
        packet.validlen = cmdlen;
        memset(packet.buf,0,256);
        strncpy((char*)packet.buf,"WRTI",4);
        memcpy(packet.buf+4,&cmdlen,4);
        memcpy(packet.buf+8,&cmd,2);
        memcpy(packet.buf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        if(ret == 0){
                packet.buf[40] = 0x0;
        }else{
                packet.buf[40] = 0x1;
        }
        m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));

        m_pSocketManager->CloseSocket(sock_index);
}

/**
*中心获取家居配置表
*2009-7-29 14:47:00
*/
void CCmdHandler::center_get_elec_file_config(int sock_index){
	WRT_MESSAGE("中心远程获取家居配置表");
        unsigned long tmpmsg[4];
        unsigned char* tmpbuf = NULL;
        int buflen = 0;
        int packlen =0;
        tmpmsg[0]  = sock_index;
        tmpbuf=(unsigned char*)Get_jiaju_table_file(&buflen);
        tmpmsg[1] = (unsigned long)PackCmd(ROOM_GET_ELECFIILE_ACK,tmpbuf,buflen,&packlen);
        tmpmsg[2] = packlen;
         
        if(m_pSocketManager){
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,(void*)tmpmsg,sizeof(unsigned long)*4);
                m_pSocketManager->CloseSocket(sock_index);
        }
        if(tmpbuf != NULL)
                ZENFREE(tmpbuf);
}


//----------------------------------------------------------------------------------------
/**
*函数名称：center_req_jiaju_status
*参    数：int sock_index ,socket索引
*功    能：回应中心请求家居的配置及当前状态
*返    回：
*/
void CCmdHandler::center_req_jiaju_status(int sock_index,short scmd){
	WRT_MESSAGE("中心请求家居配置表以及当前状态");
        unsigned long tmpmsg[4];
        int buflen = 0;
        unsigned char* tmpbuf = 0;
        int packlen = 0;
        if(GetIsHaveJiaju() == 0){
                if(m_pSocketManager && scmd!= ROOM_ACK_ALL_HOME_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        tmpmsg[0] = tmpmsg[1] = tmpmsg[2] = tmpmsg[3] = 0;
        tmpmsg[0]  = sock_index;
        tmpbuf=get_jiaju_config_buf_for_center(&buflen);
        tmpmsg[1] = (unsigned long)PackCmd(scmd,tmpbuf,buflen,&packlen);
        tmpmsg[2] = packlen;

        if(m_pSocketManager){
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,(void*)tmpmsg,sizeof(unsigned long)*4);
                if(scmd != ROOM_ACK_ALL_HOME_STATUS)
                        m_pSocketManager->CloseSocket(sock_index);
        }
        if(tmpbuf != NULL)
                ZENFREE(tmpbuf);

}

/**
*函数名称：center_set_jiaju
*参    数：int sock_index ,socket索引
unsigned char* buf   家居数据
int buflen           数据的长度
*功    能：中心设置某个家居的状态
*返    回： 
*/
void CCmdHandler::center_set_jiaju(int sock_index,unsigned char* buf,int buflen,short scmd){
	WRT_MESSAGE("中心远程设置家居状态");
        unsigned long tmpmsg[4];
        int packlen = 0;
        int ret=0;
        if(pSystemInfo->mySysInfo.isremote == 0){ //关闭了控制功能。
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
                return ;
        }
        if(GetIsHaveJiaju() == 0){
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        tmpmsg[0] = tmpmsg[1] = tmpmsg[2] = tmpmsg[3] = 0;
        tmpmsg[0]  = sock_index;
        if(buflen > 5){
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        unsigned char tmp[6];
        tmp[0] = *(unsigned char*)buf;
        tmp[1] = *(unsigned char*)(buf+1);
        tmp[2] = *(unsigned char*)(buf+2);
        tmp[3] = *(unsigned char*)(buf+3);
        tmp[4] =*(unsigned char*)(buf+4);
        if(tmp[0] == 0x0 && tmp[1] == 0x0 && tmp[2] == 0x0 ){
                //控制防区
                set_security_status(tmp[3]-1,tmp[4]); //隐式转换
                UpdateSystemInfo();
                ret = 0;
        }else{
                ret = send_jiaju_status_for_center(tmp[0]-1,tmp[1]-1,tmp[2]-1,tmp[3]-1,tmp[4]);
        }
        if(ret == -2){
                tmp[5] = 2;
        }else if(ret == 0){
                tmp[5] = 1;
        }else if(ret == -3){
                tmp[5] = 1;
        }else
                tmp[5] = 0;
        tmpmsg[1] = (unsigned long)PackCmd(scmd ,tmp,6,&packlen);
        tmpmsg[2] = packlen;
        if(m_pSocketManager){
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,tmpmsg,sizeof(unsigned long)*4);
                if(scmd != ROOM_ACK_CTRL_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
        }

}

/**
*函数名称：center_get_jiaju_status
*参    数：int sock_index ,socket索引
unsigned char* buf   家居数据
int buflen           数据的长度
*功    能：回应中心获得某个家居的状态
*返    回：
*/
void CCmdHandler::center_get_jiaju_status(int sock_index,unsigned char* buf,int buflen,short scmd){
	WRT_MESSAGE("中心获取家居状态");
        unsigned long tmpmsg[4];
        int packlen = 0;
        tmpmsg[0] = tmpmsg[1] = tmpmsg[2] = tmpmsg[3] = 0;
        tmpmsg[0]  = sock_index;
        if(GetIsHaveJiaju() == 0){
                if(m_pSocketManager && scmd != ROOM_ACK_INQ_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        if(buflen > 4){
                if(m_pSocketManager && scmd != ROOM_ACK_INQ_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
		unsigned char tmp[5];
		tmp[0] = *(unsigned char*)buf;
		tmp[1] = *(unsigned char*)(buf+1);
		tmp[2] = *(unsigned char*)(buf+2);
		tmp[3] = *(unsigned char*)(buf+3);
        int ret = get_jiaju_status_for_center(tmp[0]-1,tmp[1]-1,tmp[2]-1,tmp[3]-1);
        if(ret == -2 || ret == -1 || ret == -3){
                tmp[4] = 2;
        }else
                tmp[4] = (unsigned char)ret;
        tmpmsg[1] = (unsigned long)PackCmd(scmd,tmp,5,&packlen);
        tmpmsg[2] = packlen;
        if(m_pSocketManager){
        	WRT_DEBUG("家居状态(0x%x,0x%x,0x%x,0x%x,0x%x)",tmp[0]-1,tmp[1]-1,tmp[2]-1,tmp[3]-1,tmp[4]);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,tmpmsg,sizeof(unsigned long)*4);
                if(scmd != ROOM_ACK_INQ_ONE_DEVICES)
                        m_pSocketManager->CloseSocket(sock_index);
        }

}

/**
*函数名称：center_set_jiaju_scene
*参    数：int sock_index ,socket索引
unsigned char* buf   家居数据
int buflen           数据的长度
*功    能：中心设置情景模式
*返    回：
*/
void CCmdHandler::center_set_jiaju_scene(int sock_index,unsigned char* buf,int buflen,short scmd){
        unsigned long tmpmsg[4];
        int packlen = 0;
        int ret;
        int issend = 0;

        unsigned char tmp[2];
        tmp[0] = *(unsigned char*)buf;       
        //if(strncmp(pPeerLocalID,"000000000000000",15) != 0){
        WRT_MESSAGE("中心远程控制情景");
        if(tmp[0] == 0x05){
			unsigned long  tmpevent[4];
			tmpevent[0] = STATUS_REMOTE_SCENE;
			tmpevent[1] = tmp[0];
			tmpevent[2] = tmpevent[3] = 0;
			m_display(tmpevent);
			issend = 1;
        }

        if(pSystemInfo->mySysInfo.isremote == 0){ //关闭了控制功能。
			if(m_pSocketManager && scmd != ROOM_ACK_CTRL_SCENE)
				m_pSocketManager->CloseSocket(sock_index);
			return ;
        }
        tmpmsg[0] = tmpmsg[1] = tmpmsg[2] = tmpmsg[3] = 0;
        tmpmsg[0]  = sock_index;
        if(GetIsHaveJiaju() == 0){
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_SCENE)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        if(buflen > 4){
                if(m_pSocketManager && scmd != ROOM_ACK_CTRL_SCENE)
                        m_pSocketManager->CloseSocket(sock_index);
                return;
        }
	
        ret = send_scene_mode(tmp[0]);
        if(ret == 0){//2009-7-23
                unsigned long  tmpevent[4];
                tmp[1] = 0x01;
                tmpevent[0]  = STATUS_REMOTE_SCENE;
                tmpevent[1] = tmp[0];
                tmpevent[2] = tmpevent[3] = 0;
                if(issend == 0)
                        m_display(tmpevent);
        }else
                tmp[1] = 0x0;
	
        tmpmsg[1] = (unsigned long)PackCmd(scmd,tmp,2,&packlen);
        tmpmsg[2] = packlen;
        if(m_pSocketManager){
        	WRT_DEBUG("中心控制情景0x%x 结果0x%x",tmp[0],tmp[1]);
                m_pSocketManager->Send_msg(MSG_NODE_SEND_JIAJU_INFO,tmpmsg,sizeof(unsigned long)*4);
                if(scmd != ROOM_ACK_CTRL_SCENE)
                        m_pSocketManager->CloseSocket(sock_index);
        }
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/*#define FLASH_CODE1_START_SEC       11
#define FLASH_CODE1_END_SEC         42
#define FLASH_CODE2_START_SEC       43
#define FLASH_CODE2_END_SEC         74

#define FLASH_CODE1_START_ADDR      0x160000
#define FLASH_CODE2_START_ADDR      0x560000
*/
#ifdef USED_FLASH32M

#define FLASH_CODE1_START_SEC       64
#define FLASH_CODE1_END_SEC         119
#define FLASH_CODE2_START_SEC       120
#define FLASH_CODE2_END_SEC         175

#define FLASH_CODE1_START_ADDR      0x800000
#define FLASH_CODE2_START_ADDR      0xF00000

#else

#if 0
#define FLASH_CODE1_START_SEC       8
#define FLASH_CODE1_END_SEC         43
#define FLASH_CODE2_START_SEC       44
#define FLASH_CODE2_END_SEC         79

#define FLASH_CODE1_START_ADDR      0x100000
#define FLASH_CODE2_START_ADDR      0x580000
#else
#define FLASH_CODE1_START_SEC       8
#define FLASH_CODE1_END_SEC         47
#define FLASH_CODE2_START_SEC       48
#define FLASH_CODE2_END_SEC         87

#define FLASH_CODE1_START_ADDR      0x100000
#define FLASH_CODE2_START_ADDR      0x600000
#endif

#endif

static int ProcessVer(unsigned char* newver,unsigned char* oldver){
        char* tmp = (char*)oldver;
        if(strchr(tmp,'.') != NULL){
                WRT_DEBUG("升级文件是 old header");
                return 1;
        }
        
        if(newver[0] == 0x0 || newver[0] == 0x99)
                return 1;
		
        if(newver[0] != 0x38){//0x31 D平台高清屏电容屏 1024x600
                return 0;
        }
#if 1
        
        if(newver[2] > oldver[1])
                return 1;
        
        if(newver[3] > oldver[2] )
                return 1;

        if(newver[4] > oldver[3])
                return 1;
        return 0;
#else
        if(newver[2] < oldver[0])
                return 0;
        if(newver[2] > oldver[0])
                return 1;
        if(newver[3] < oldver[1] )
                return 0;
        if(newver[3] > oldver[1] )
                return 1;

        if(newver[4] < oldver[2])
                return 0;
        return 1;
#endif
}

static int BurnUpgradeData(unsigned char *buf, int buflen)
{

    int fd;

    fd = open ("/dev/mtd4", O_SYNC | O_RDWR);

    struct mtd_info_user 
    {
        __u8 type; 
        __u32 flags;
        __u32 size; // Total size of the MTD
        __u32 erasesize;
        __u32 writesize;
        __u32 oobsize;// Amount of OOB data per block (e.g. 16)
        /*The below two fields are obsolete and broken, do not use them * (TODO: remove at some point) */
        __u32 ecctype;
        __u32 eccsize;
    };

    struct mtd_info_user mtd;
    ioctl(fd, MEMGETINFO, &mtd);

    struct erase_info_user {
        __u32 start;
        __u32 length;
    };

    typedef struct erase_info_user erase_info_t;

    erase_info_t erase;

    int isNAND, bbtest = 1;
    struct mtd_info_user *DevInfo = &mtd;

    erase.length = DevInfo->erasesize;

    isNAND = (DevInfo->type == MTD_NANDFLASH) ? 1 : 0;

    for (erase.start = 0; erase.start <  DevInfo->size; erase.start += DevInfo->erasesize) 
    {
        if (bbtest) 
        {
            loff_t offset = erase.start;
            int ret = ioctl(fd, MEMGETBADBLOCK, &offset); //判断是不是坏块
            if (ret > 0) 
            {
                //if (!quiet)
                printf ("\nSkipping bad block at 0x%08x\n", erase.start);
                continue;//发现是坏块，应该跳过
            }
        }

        fprintf(stderr, "\rErasing %d Kibyte @ %x -- %2llu %% complete.", \
            (DevInfo->erasesize) / 1024, erase.start,
            (unsigned long long) erase.start * 100 / (DevInfo->size));

        if (ioctl(fd, MEMERASE, &erase) != 0) //执行擦除操作
        {
            printf("MTD Erase failure\n");
            continue;
        }
    }

    int num = (buflen + 2*1024-1)/(2*1024);
    int j = 0x0;
    int alwlen = 0x0;
    int cur_write_pos = 0;
    int write_ret = 0;
    int wlen = 0x0;
    int write_cnt = 0;
    printf("Start write\n");
    for(j=0x0; j<num; j++)
    {
        alwlen = 0x0;
        loff_t offset;
        cur_write_pos = 0;
        write_ret = 0;
        wlen = 0x0;

        offset = 0/*FlashDevice.MtdDevice[i].StartAddress*/ + j * 2*1024;
        lseek(fd, offset, 0);
        wlen = ((buflen - alwlen)>(2*1024)) ? (2*1024) : (buflen-alwlen);
        printf("+");

        while(cur_write_pos != wlen)
        {
            //printf("wlen: %d,cur_write_pos: %d\n",wlen,cur_write_pos);
            write_ret = write(fd, buf + offset + cur_write_pos, wlen - cur_write_pos);
            if (write_ret < 0)
            {
                printf("  (%d)\nFlashWrite failed, not erase?  (%d)\n", write_cnt, write_ret);
                return -1;
            }
            else if (write_ret == 0)
            {
                break;
            }
            cur_write_pos += write_ret;
        }
        write_cnt++;
    }
    printf("write  (%d)*2k\n", write_cnt);
    printf("finish write\n");

    return 1;
}

/**
*系统升级
*/
static int save_update_file(char* name,unsigned char* addr,long len){

	FILE* file = 0;
	printf("save file name = %s\n",name);
	file = fopen(name, "wb");
	if(file){
		fseek(file,0,SEEK_SET);
		long write_size = fwrite(addr,1,len,file);
		if(write_size == len){
			fclose(file);
			return 1;
		}
		printf("errno = %d  write_size=%d \n",errno,write_size);
		fclose(file);
		unlink(name);
	}
	printf("errno = %d \n",errno);
	system("sync");
	return 0;
}

static int checkupdatefile(char *name,unsigned char* addr,int len)
{
	FILE* file = 0;
	int noerror  = 1;
	file = fopen(name,"rb");
	
	if(file){
		int i,index;
		int totallen = 0;
		int realsize;
		unsigned char* buf = (unsigned char*)ZENMALLOC(4096);
		if(buf == NULL){
			fclose(file);
			file = NULL;
			return noerror;
		}
		i = 0; 
		index = 0;
		while(!feof(file)){
			memset(buf,0,4096);
			realsize = 0;
			realsize = fread(buf,1,4096,file);
			if(ferror( file )){
				noerror = 0;
				break;
			}
			totallen += realsize;
			if(totallen > len){
				noerror = 0;
				break;
			}
				
			for(i = 0; i< realsize;i++){
				if(buf[i] != addr[index++]){
					noerror = 0;
					break;
				}
			}
			if(noerror == 0)
				break;
		}
		ZENFREE(buf);
		buf = NULL;
		fclose(file);
		file = NULL;
	}else
		noerror = 0;
	return noerror;
}

int CCmdHandler::Update(unsigned char* buf,unsigned long len,const char* ver ,int is2)
{
    volatile unsigned short *pFlash;
    unsigned char* databuffer1 = NULL;
    unsigned long  dst_len = 0;
    unsigned long temp_len =0;
    int image_flag = 0;
    DIR* mydir;
    unsigned char newver[16];
    int prelen = 128*1024;
    int writelen = 0;
    int ret = 0;
    int i = 0;
    unsigned long  dst_addr = 0;
    unsigned long time1,time2;
    if (is2 == 0)
    {
        memset(newver, 0, 16);
        memcpy(newver, buf, 16);
        if(!ProcessVer(newver,(unsigned char*)pSystemInfo->BootInfo.Version)){
            return 3;
        }
    }
    else
    { //处理版本。
        unsigned char tmpdata[16];//FF FF 00 00 00 59 4C 4C 4D 48
        unsigned char save_data[10] = {0xFF,0xFF,0x00,0x00,0x00,0x59,0x4c,0x4c, 0x4d,0x48};

        //处理前面16字节
        memset(tmpdata,0,16); 
        memcpy(tmpdata,buf,16);
        if (memcmp(tmpdata, save_data, 10) != 0)
        {
            return 3;
        }

        //处理随后的16字节版本比较
        memset(newver,0,16);
        memcpy(newver,buf+16,16);
        if (!ProcessVer(newver,(unsigned char*)pSystemInfo->BootInfo.Version)){
            return 3;
        }

        //第3和第4个16字节，做交换。
        memset(tmpdata,0,16);
        memcpy(tmpdata,buf+48,16);
        memmove(buf+48,buf+32,16);
        memcpy(buf+32,tmpdata,16);
    }

    if (newver[0] == 0x99)
    {
        image_flag = 1;
    }

    if (is2 == 0){
        buf = (unsigned char*)buf+16;
        len -= 16;
    }
    else
    {
        buf = (unsigned char*)buf +32;
        len -= 32;
    }

    //保存到升级文件夹
    if (image_flag == 1)
    {
        if (save_update_file("/system.tar.gz",buf,len) == 0)
        {
            return 2;
        }
        system("chmod 777 /system.tar.gz");
        //数据校验
        //从升级文件夹中读出数据，进行校验。
        if (checkupdatefile("/system.tar.gz",buf,len) == 1)
        {
        }
        else
        {
            system("rm -rf /system.tar.gz");
            return 2;
        }
    }
    else
    {
        mydir = opendir("/tmp/wrt");
        if(mydir == NULL)
            mkdir ("/tmp/wrt",1);
        else{
            closedir(mydir);
            mydir = NULL;
        }

        if (save_update_file("/var/update_wrt_room", buf, len) == 0)
        {
            system("rm -rf /tmp/wrt");
            return 2;
        }
        system("chmod 777 /var/update_wrt_room");
        //数据校验
        //从升级文件夹中读出数据，进行校验。
        if (checkupdatefile("/var/update_wrt_room", buf, len) == 1)
        {
            memset(pSystemInfo->BootInfo.Version,0,10);
            memcpy(pSystemInfo->BootInfo.Version,newver+1,4);//
            memcpy(pSystemInfo->BootInfo.Version+4,newver+5,6);

            pSystemInfo->BootInfo.program_position = 0;
            pSystemInfo->BootInfo.program_size = len;
            UpdateSystemInfo();
        }
        else
        {
            system("rm -rf /tmp/wrt");
            return 2;
        }
    }
    return 1;
}

int  CCmdHandler::UpdateFile(const char* url,const char* ver,int is2)
{
    int fd = -1;
    int ws;
    int ret = 0;
    unsigned long   total=0;
    unsigned long len = 0;
    unsigned char* read_data = NULL;
    WRT_MESSAGE("upgrade file = %s",url);
    if ((fd = HttpClientOpen((char*)url))!= -1)
    {
        len = HttpClientGetContentLength(fd);
        read_data = (unsigned char*)malloc(len);
        if(read_data == NULL)
        {
            HttpClientClose(fd);
            fd = -1;
            return 6;
        }

        do
        {
            if ((ws  = HttpClientRead(fd,(char*)read_data+total,len-total))>0)
            {
                total+=ws;
            }
        }while(ws > 0);

        HttpClientClose(fd);
        fd = -1;
        if(ws == -1)
        {
            free(read_data);
            read_data = NULL; 
            return 6;
        }
        //if ((ret = Update(read_data,len,ver,is2)) != 1)
        if ((ret = BurnUpgradeData(read_data,len)) != 1)
        {
            free(read_data);
            read_data = NULL;
            return ret;
        }
        free(read_data);
        read_data = NULL; 
    }
    else
    {
        WRT_DEBUG("open url %s faield ",url);
        return 5;
    }

    return 1;
}

void CCmdHandler::local_upgrade_room(unsigned long event[4])
{
        if( m_SysStatus == SYS_UPGRADE)
                return;
       	char* buf = NULL;
        DIR *mydir =  NULL;	
        m_SysStatus = SYS_UPGRADE;
        m_displayevent[0] = STATUS_ROOM_SYS_UPDATE;
        m_displayevent[1] = 0;
        m_displayevent[2] = m_displayevent[3] = 0;
        m_display(m_displayevent);
        mydir = opendir("/mnt/mmc/wrt");
        if(mydir == NULL){
UPGRADEERROR:    

            WRT_DEBUG("升级失败");
           
            m_displayevent[0] = STATUS_ROOM_SYS_UPDATE;
            m_displayevent[1] = -1;
            m_displayevent[2] = m_displayevent[3] = 0;
            m_display(m_displayevent);
            m_SysStatus = SYS_IDLE;
            return;
        }
        closedir(mydir);
        mydir = NULL;
        FILE* file = NULL;
        int readsize = 0;
        file = fopen("/mnt/mmc/wrt/wrt_room","rb");
        if(file == NULL){
        	goto UPGRADEERROR;
        	return;
        }
        unsigned char newver[16];
        memset(newver,0,16);
        readsize = fread((char*)newver,1,16,file);
        if(readsize != 16){
        	fclose(file);
        	file = NULL;
        	goto UPGRADEERROR;
        	return;
        }
		if(!ProcessVer(newver,(unsigned char*)pSystemInfo->BootInfo.Version)){
        	fclose(file);
        	file = NULL;
        	WRT_DEBUG("版本一样，无需升级");
        	m_displayevent[0] = STATUS_ROOM_SYS_UPDATE;
            m_displayevent[1] = -2;
            m_displayevent[2] = m_displayevent[3] = 0;
            m_display(m_displayevent);
            m_SysStatus = SYS_IDLE;	
            return;
        }
        buf = (char*)ZENMALLOC(4096);
        if(buf == NULL){
        	fclose(file);
        	file = NULL;
        	goto UPGRADEERROR;
        	return;
        }
    	mydir = opendir("/tmp/wrt");
        if(mydir == NULL)
        	mkdir ("/tmp/wrt",1);
        else{
        	closedir(mydir);
        	mydir = NULL;
        }
        FILE* wfile = NULL;
        wfile = fopen("/tmp/wrt/update_wrt_room","wb");
	if(wfile == NULL){
        	fclose(file);
        	file = NULL;
        	ZENFREE(buf);
        	system("rm -rf /tmp/wrt");
        	goto UPGRADEERROR;
        	return;		
	}
	int error = 0;
	int writesize = 0;
	int len = 0;
    while(!feof(file)){
    	memset(buf,0,4096);
    	readsize = fread(buf,1,4096,file);
    	if(ferror(file)){
    		error = 1;
    		break;
    	}
    	writesize += fwrite(buf,1,readsize,wfile);
    	if(ferror(wfile)){
    		error = 1;
    		break;
    	}
    	len += readsize;
    }
    
	ZENFREE(buf);
	buf =NULL;
	buf = (char *)ZENMALLOC(len);
	fseek(file,16,SEEK_SET);
	fread(buf,len,1,file);

	fclose(file);
	file = NULL;
	fclose(wfile);
	wfile = NULL;
	system("sync");
    
	if(error == 1){
		system("rm -rf /tmp/wrt");
		ZENFREE(buf);
		buf = NULL;
		goto UPGRADEERROR;        	
	}
	system("chmod 777 /tmp/wrt/update_wrt_room");

	//数据校验
	//从升级文件夹中读出数据，进行校验。
	if(checkupdatefile("/tmp/wrt/update_wrt_room",(unsigned char*)buf,len) == 1){

		ZENFREE(buf);
		buf = NULL;
		memset(pSystemInfo->BootInfo.Version,0,10);
		memcpy(pSystemInfo->BootInfo.Version,newver+1,4);
		memcpy(pSystemInfo->BootInfo.Version+4,newver+5,6);//12

		pSystemInfo->BootInfo.program_position = 0;
		pSystemInfo->BootInfo.program_size = len;
		UpdateSystemInfo();
		WRT_DEBUG("升级成功");
		m_displayevent[0] = STATUS_ROOM_SYS_UPDATE;
		m_displayevent[1] = 10;
		m_displayevent[2] = m_displayevent[3] = 0;
		m_display(m_displayevent);
		mcu_reset_sys(1);
		SDL_Delay(5000); //等待5s如果系统未重启，则自己重启。
		mcu_reset_sys(0);
		m_displayevent[1] = 1;
		m_displayevent[2] = m_displayevent[3] = 0;
		m_display(m_displayevent);
	}else{
		ZENFREE(buf);
		buf = NULL;
		goto UPGRADEERROR;
	}

	return;
}

void CCmdHandler::center_upgrade_room(int sock_index,unsigned char* buf,int buflen,int is2){
    if( m_SysStatus == SYS_UPGRADE)
        return;
    WRT_MESSAGE("device upgrade");
    if (SYS_TALKTOMAIN == m_SysStatus || SYS_MONT == m_SysStatus)
    {
        Stopplayvideo();
        if (SYS_MONT == m_SysStatus)
        {
            m_displayevent[0] = STATUS_ROOM_STOPMONT;
        }
        else
        {
            m_displayevent[0] = STATUS_DOOR_HANGUP;
        }
        m_displayevent[1] = 0;
        m_displayevent[2]=m_displayevent[3]= 0;
        m_display(m_displayevent);
    }
    mp3stop();
    if (m_ringtimerid != 0)
    {//如果没有摘机，取消震铃计时
        add_and_del_timer(TIME_RING_EVENT_STOP);
    }
    talktostop();
    talkstop();
    if (m_talktimerid != 0)
    {//如果有摘机，时间到，或者主动挂机，取消通话计时
        add_and_del_timer(TIME_TALK_EVENT_STOP);
    }
    unsigned char version[8]={0};
    int ret = 0;
    memcpy(version,buf,7);

    if(m_pSocketManager)
    m_pSocketManager->CloseSocket(sock_index);
    m_SysStatus = SYS_UPGRADE;
    unsigned char url[100];
    memcpy(url,buf+8,buflen-8);
    url[buflen-8] = '\0';
    m_displayevent[0] = STATUS_ROOM_SYS_UPDATE;
    m_displayevent[1] = 0;
    m_displayevent[2] = m_displayevent[3] = 0;
    m_display(m_displayevent);

    ret = UpdateFile((const char*)url,(char*)version,is2);
    unsigned char* packetbuf;
    int packetlen =0;
    unsigned char tmpbuf[12];
    memset(tmpbuf,0,12);
    tmpbuf[0] = 0x04;
    tmpbuf[1] = (unsigned char)ret;
    memcpy(tmpbuf+2,pSystemInfo->BootInfo.Version,10);
    packetbuf = PackCmd(DEV_UPGRADE_ACK,(unsigned char*)tmpbuf,12,&packetlen);
    m_pSocketManager->SendResultToCenter(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,packetbuf,packetlen);
    ZENFREE(packetbuf);
    if (ret == 1)
    {
        WRT_DEBUG("upgrade success!");
        m_displayevent[0] = STATUS_ROOM_SYS_UPDATE;
        m_displayevent[1] = 1;
        m_displayevent[2] = m_displayevent[3] = 0;
        m_display(m_displayevent);
    }
    else
    {
        WRT_DEBUG("upgrade fail!");
        mcu_reset_sys(0);
        m_displayevent[1] = -1;
        m_displayevent[2] = m_displayevent[3] = 0;
        m_display(m_displayevent);
    }
    m_SysStatus = SYS_IDLE;
}

///////////////////////////////////////////////////////////
//
//
//中心应答托管
void CCmdHandler::center_ack_agent(int sock_index,unsigned char* buf,int buflen)
{
        if(m_SysStatus == SYS_ROOMCALLCENTERTALK)
        {
        	WRT_MESSAGE("中心已托管");
                unsigned long newip = 0;
                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                m_CurrentSocketIndex = -1;
                newip = *(unsigned long*)buf;
                if(newip != 0)
                {
                        if( m_dirvertcount > 3){
                                m_dirvertcount =0;
                                door_and_center_hangup(-1);
                                return;
                        }
                        unsigned long tmppack[4];
                        int packlen=0;
                        memset(tmppack,0,sizeof(unsigned long)*4);
                        tmppack[0] = newip;
                        tmppack[1] = (unsigned long)PackCmd(ROOM_CALLCENTER,0,0,&packlen);
                        tmppack[2] = packlen;
                        tmppack[3] = 0;
                        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                        m_SysStatus = SYS_ROOMCALLCENTERTALK;
                        m_isdivert = 1;
                        m_dirvertcount++;
                }else
                        m_SysStatus = SYS_IDLE;
        }
}

/*
*中心应答启用备用机
*/
void CCmdHandler::center_ack_minor(int sock_index,unsigned char* buf,int buflen){
        if(m_SysStatus == SYS_ROOMCALLCENTERTALK)
        {
        	WRT_MESSAGE("中心应答启用备用机");
                unsigned long newip = 0;
                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                m_CurrentSocketIndex = -1;
                newip = *(unsigned long*)buf;
                if(newip != 0)
                {
                        unsigned long tmppack[4];
                        int packlen=0;
                        memset(tmppack,0,sizeof(unsigned long)*4);
                        tmppack[0] = newip;
                        tmppack[1] = (unsigned long)PackCmd(ROOM_CALLCENTER,0,0,&packlen);
                        tmppack[2] = packlen;
                        tmppack[3] = 0;
                        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                        m_SysStatus = SYS_ROOMCALLCENTERTALK;
                }else
                        m_SysStatus = SYS_IDLE;
        }
}

/*
*中心应答转接到管理机
*/
void CCmdHandler::center_ack_divert_center(int sock_index,unsigned char* buf,int buflen)
{
        if(m_SysStatus == SYS_ROOMCALLCENTERTALK)
        {
        	WRT_MESSAGE("中心转接到管理机或者其他中心");
                unsigned long newip = 0;
                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                m_CurrentSocketIndex = -1;
                mp3stop();
                if(m_ringtimerid != 0){//如果没有取消的，则取消铃声计时器，
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                talktostop();
                talkstop();
                if(m_talktimerid != 0){//如果有摘机，取消通话计时器
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
                }
                newip = *(unsigned long*)buf;
                if(newip != 0)
                {
                        unsigned long tmppack[4];
                        int packlen=0;
                        memset(tmppack,0,sizeof(unsigned long)*4);
                        tmppack[0] = newip;
                        tmppack[1] = (unsigned long)PackCmd(ROOM_CALLCENTER,0,0,&packlen);
                        tmppack[2] = packlen;
                        tmppack[3] = 0;
                        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                        m_SysStatus = SYS_ROOMCALLCENTERTALK;
                        m_isdivert = 1;
                }else{
                        if(g_smalldoor_call_agent == 1){
                                door_and_center_hangup(-1);
                        }else
                                m_SysStatus = SYS_IDLE;
                }
        }
}
/*
*中心应答转接分机
*/
void CCmdHandler::center_ack_divert_room(int sock_index,unsigned char* buf,int buflen)
{
        if(m_SysStatus == SYS_ROOMCALLCENTERTALK)
        {
                WRT_MESSAGE("中心转接到其他分机");
                door_and_center_hangup(sock_index);
        }
}
/*
void CCmdHandler::center_ack_emerg(int sock_index)
{
}
*/


void CCmdHandler::center_services_ack(int sock_index,unsigned char* buf,int buflen)
{
        if(m_SysStatus == SYS_REQDOCINFO ){

                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                m_CurrentSocketIndex = -1;   
                if(pSystemInfo->mySysInfo.hassip == 1){ 
                        unsigned char type = *(unsigned char*)buf;
                        WRT_MESSAGE("从中心获取服务信息0x%x",type);
                        if(type == 0){
                                m_displayevent[0] = STATUS_REQDOCINFO_OVER;
                                m_displayevent[1] = 1;
                                m_displayevent[2] = m_displayevent[3] = 0;
                                m_display(m_displayevent);
                                m_SysStatus = SYS_IDLE;  
                                return;                
                        }else if(type == 0x01){
                                int count = 0;
                                unsigned char datatype = 0x0;
                                int len = 0;
                                int cpylen=0;
                                char code[16];
                                unsigned char* data = NULL;
                                memcpy((void*)&count,buf+1,4);
                                if(count > 100){
                                        m_displayevent[0] = STATUS_REQDOCINFO_OVER;
                                        m_displayevent[1] = 1;
                                        m_displayevent[2] = m_displayevent[3] = 0;
                                        m_display(m_displayevent);
                                        m_SysStatus = SYS_IDLE;                    
                                        return;
                                }
                                delAllSipDataSource(); //删除原来的所有的数据
                                WRT_DEBUG("从中心获取SIP服务信息 count = %d",count);
                                while(count--){
                                        memset(code,0,16);
                                        datatype = *(unsigned char*)(buf+5+cpylen);
                                        memcpy((void*)&len,(buf+6+cpylen),4);
                                        memcpy(code,buf+10+cpylen,16);
                                        data = (unsigned char*)(buf+26+cpylen);
                                        addSipDataSource(datatype,len,code,data);
                                        cpylen += (len+21);
                                }

                        }
                }  
        }
        m_displayevent[0] = STATUS_REQDOCINFO_OVER;
        m_displayevent[1] = 0;
        m_displayevent[2] = m_displayevent[3] = 0;
        m_display(m_displayevent);
        m_SysStatus = SYS_IDLE;        
}
/*
*中心应答便民信息,2010-4-7 9:07:31 modify by ljw
*/
void CCmdHandler::center_ack_reqdocinfo(int sock_index,unsigned char* buf,int buflen){
        if(m_SysStatus == SYS_REQDOCINFO){

                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                m_CurrentSocketIndex = -1;
                //-

                int tlen = *(int*)buf;
                WRT_MESSAGE("从中心获取便民信息 %d--0x%x",tlen,buflen);
                if(tlen != 0){
                        AddDocInfo(buf+4,tlen);
                        m_displayevent[1] = 0;
                }else{
                        m_displayevent[1] = 1;
                }

                m_displayevent[0] = STATUS_REQDOCINFO_OVER;

                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);
                m_SysStatus = SYS_IDLE;
        }
}
/***
分机向中心请求天气预报信息
*/
void CCmdHandler::room_get_weather(int sock_index)
{
	WRT_MESSAGE("分机请求天气预报,系统状态0x%x,中心地址:0x%x",m_SysStatus,m_centeripaddr);
        if(m_centeripaddr == 0 || m_centeripaddr == 0xffffffff)
                return;
        unsigned long tmppack[4];
        int packlen=0;
        memset(tmppack,0,sizeof(unsigned long)*4);
        tmppack[0] = m_centeripaddr;
        tmppack[1] = (unsigned long)PackCmd(ROOM_GET_WEATHER,0,0,&packlen);
        tmppack[2] = packlen;
        tmppack[3] = 0;
        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);	
}

/***
中心应答天气预报信息
*/
void  CCmdHandler::center_ack_weather(int sock_index,unsigned char* buf,int buflen)
{
	WRT_MESSAGE("中心应答天气预报");
	if(buflen != 12){
		 if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
                 return;
	}
	unsigned char cWeather = *(unsigned char*)buf;
	int HiTemperature = *(int*)(buf+8);
	int LiTemperature = *(int*)(buf+4);
 	if(m_pSocketManager)
                       m_pSocketManager->CloseSocket(sock_index);	
        unsigned long tmpmsg[4];
        tmpmsg[0] = STATUS_WEATHER_RESULT;
        tmpmsg[1] = cWeather;
        tmpmsg[2] = LiTemperature;
        tmpmsg[3] = HiTemperature;
        m_display(tmpmsg);
        
	WRT_MESSAGE("天气预报数据%x,%x,%x",cWeather,HiTemperature,LiTemperature);

}

void CCmdHandler::center_ack_ring_error(int sock_index){
        if(m_SysStatus == SYS_REQEUSTRINGLIST || SYS_DOWNLOADRING == m_SysStatus){
#if USE_PACKELIST
                DelPackList(sock_index); //如果有包存起来了，则将其删除
#endif
		WRT_MESSAGE("从中心获取铃声错误");
                m_displayevent[0] = STATUS_GET_RING_ERROR;
                m_displayevent[1] = 0;
                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);
                m_CurrentSocketIndex = -1;
                m_SysStatus = SYS_IDLE;
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
        }
}

//中心回应浏览铃声
void CCmdHandler::center_ack_browserring(int sock_index,unsigned char* buf,int buflen){
        if(m_SysStatus == SYS_REQEUSTRINGLIST){
                unsigned char* temp = buf;
                unsigned char count= *temp;
                char name[41];
                int total = (int)count;
                WRT_MESSAGE("从中心获取铃声列表数量%d",total);
                if(total == 0 || total < 0){
                        m_displayevent[1] = 1;
                        goto end;
                }
                temp +=1;
                while(total--){//铃声加入下载列表
                        memset(name,0,41);
                        memcpy(name,temp,40);
                        AddRingToList(name);
                        temp += 40;
                }
                m_displayevent[1] = 0;
end:
                m_SysStatus  = SYS_IDLE;
                m_displayevent[0] = STATUS_RING_LIST_OVER;
                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);
                m_CurrentSocketIndex = -1;
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
        }


}

#ifdef DOWNLOAD_FROM_HTTP
/*
void CCmdHandler::download_mp3_over(unsigned char* buf,int buflen){

}
*/
#endif

//中心回应下载铃声
void CCmdHandler::center_ack_downloadring(int sock_index,unsigned char* buf,int buflen){
        //当解析完后，系统进入了空闲状态
        if(m_SysStatus == SYS_DOWNLOADRING){
        	WRT_MESSAGE("从中心下载铃声");
                unsigned char tmpbuf[8]={0};
                unsigned long  tmpaddr;
                char name[41];
                memset(name,0,41);
                memcpy(name,buf,40);
                RINGLIST* pTemp = GetRingInfo(name);
                if(pTemp != NULL){
                        unsigned char* addr = (unsigned char*)ZENMALLOC(buflen-40);
                        if(addr != NULL){

                                memcpy(addr,buf+40,buflen-40);
                                if(pTemp->addr != 0)
                                        ZENFREE(pTemp->addr);
                                pTemp->addr = addr;
                                pTemp->len  = buflen-40;
                                //pTemp->isdown = AddRingToDownList(pTemp->name,buflen-40,buf+40);
                                m_displayevent[0] = STATUS_RING_DOWNLOAD;
                                m_displayevent[1] = (unsigned long)pTemp;
                                m_displayevent[2] = m_displayevent[3] = 0;
                                m_display(m_displayevent);
                        }
                }
                m_CurrentSocketIndex = -1;
                m_SysStatus  = SYS_IDLE;
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
        }

}

//中心回应维修
void CCmdHandler::center_ack_repair(int sock_index)
{
        if(m_SysStatus == SYS_REQUESTREPAIR){
        	WRT_MESSAGE("中心应答维修求助");
                m_displayevent[0] = STATUS_REQUEST_REPAIR;
                m_displayevent[1] = 0;
                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);
                m_CurrentSocketIndex = -1;
                m_SysStatus  = SYS_IDLE;
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);

        }
}
#pragma pack(1)
typedef struct __tagCUTSOMTIME{
        unsigned short year;
        unsigned char  month;
        unsigned char  day;
        unsigned char  hour;
        unsigned char  min;
        unsigned char  sec;
        unsigned char  week;
}CUTSOMTIME;
#pragma pack()

//中心回应获得时间事件
void CCmdHandler::center_ack_gettime(int sock_index,unsigned char* buf,int buflen){
       // if(m_SysStatus == SYS_GETTIME){
	        if(buflen == sizeof(CUTSOMTIME)){
	                CUTSOMTIME* tmp = (CUTSOMTIME*)buf;
	                
	                unsigned long date =0;
	                unsigned long time =0;
	                date = ((tmp->year&0xffff) << 16)|((tmp->month& 0xff)<<8) | (tmp->day & 0xff);
	                time = ((tmp->hour&0xff) << 16) | ((tmp->min& 0xff)<<8 | (tmp->sec & 0xff));              
	                unsigned long ret = tm_set(date,time,0);

	        }

	        if(m_pSocketManager)
	                m_pSocketManager->CloseSocket(sock_index);
	                //m_CurrentSocketIndex = -1;
	        WRT_MESSAGE("center_ack_gettime m_SysStatus = %d",m_SysStatus);
	        if(m_SysStatus == SYS_GETSYSINFO){
	                if(pSystemInfo->LocalSetting.privateinfo.LocalIP != 0 && pSystemInfo->LocalSetting.privateinfo.LocalIP != 0xffffffff ){
	                        m_pSocketManager->ModifyHostIpAndMask(pSystemInfo->LocalSetting.privateinfo.LocalIP,0);
	                }
	                if(pSystemInfo->LocalSetting.privateinfo.SubMaskIP != 0 && pSystemInfo->LocalSetting.privateinfo.SubMaskIP != 0xffffffff ){
	                        m_pSocketManager->ModifyHostIpAndMask(0,pSystemInfo->LocalSetting.privateinfo.SubMaskIP);
	                }
	                if(pSystemInfo->LocalSetting.privateinfo.GateWayIP != 0 && pSystemInfo->LocalSetting.privateinfo.GateWayIP != 0xffffffff ){
	                        m_pSocketManager->ModifyHostGateWay(pSystemInfo->LocalSetting.privateinfo.GateWayIP);
	                }
	                m_displayevent[0] = STATUS_REQDOCINFO_OVER;
	                m_displayevent[1] =3;
	                m_displayevent[2] = m_displayevent[3] = 0;
	                m_display(m_displayevent);
	        }
	        else
	                ReDrawCurrentWindow2(7);
	        m_SysStatus = SYS_IDLE;

      //  }

}
//中心回应费用查询
void CCmdHandler::center_ack_queryfee(int sock_index,unsigned char* buf,int buflen)
{
#define SINGLE_FEE_LEN 32
        //当解析完后，系统进入了空闲状态
        if(m_SysStatus == SYS_REQUESTFEE){
        	WRT_MESSAGE("从中心查询费用");
                unsigned char* temp = buf;
                unsigned char count = *buf;
                unsigned char id = 0;
                char csinfo[SINGLE_FEE_LEN];
                int total =  (int)count;
                if(total == 0 || total < 0 ){

                        m_displayevent[1] = 1;
                        goto end;
                }

                temp += 1;
                while(total--){
                        memset(csinfo,0,SINGLE_FEE_LEN);
                        memcpy(csinfo,temp,SINGLE_FEE_LEN);
                        AddFeeToList(0,(unsigned char*)csinfo);
                        temp += (SINGLE_FEE_LEN);
                        if(temp > (buf+buflen)){
                                break;
                        }
                }
                m_displayevent[1] = 0;
end:
                m_displayevent[0] = STATUS_FEE_LIST_OVER;
                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);
                m_CurrentSocketIndex = -1;
                m_SysStatus = SYS_IDLE;
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(sock_index);
        }

}
//中心发布的信息
void CCmdHandler::door_public_info(int sock_index,unsigned char* buf,int buflen)
{


        if(buflen < MAX_MSG_BUFFER){
		WRT_MESSAGE("中心发布信息");
                if(*buf  == 0x0){
                        SendCmd(sock_index,ROOM_BROADCASTACK,1); //回应成功

                        if(/*m_SysStatus == SYS_MYROOMTALKROOM ||*/ m_SysStatus == SYS_ROOMTALKROOM ||  m_SysStatus == SYS_TALKTOMAIN ||
                                m_SysStatus == SYS_CENTERCALLROOMTALK || m_SysStatus == SYS_ROOMCALLCENTERTALK ||
                                m_SysStatus == SYS_ACTIVE_CALL || m_SysStatus == SYS_SMALLDOORCALL){
                                        m_pSocketManager->CloseSocket(sock_index);

                        }else{

                                if(m_mp3timerid != 0){
                                        add_and_del_timer(TIME_MP3_EVENT_STOP);
                                }
                                
                                mp3stop();
                                mp3play((char*)g_ringset[MSG_RING].ring_path,0,0);
                        }
                        unsigned char ctmpbuf[5];
                        unsigned char ctmpbuf2[50];
                        unsigned char ctmpbuf3[50];
                        int    timelen  =0;
                        memset(ctmpbuf,0,5);
                        memset(ctmpbuf2,0,50);
                        memcpy(ctmpbuf,buf+1,5);
                        sprintf((char*)ctmpbuf2,"20%02x-%02x-%02x %02x:%02x",ctmpbuf[0],ctmpbuf[1],ctmpbuf[2],ctmpbuf[3],ctmpbuf[4]);
                        timelen = strlen((char*)ctmpbuf2);
                        memset(msg_buffer,'\0',MAX_MSG_BUFFER);
                        memcpy(msg_buffer,ctmpbuf2,timelen);
                        memcpy(msg_buffer+timelen,"  ",2);
                        memcpy(msg_buffer+timelen+2,buf+6,buflen-6);
                        add_msg(msg_buffer);
                        m_displayevent[0] = STATUS_PUBLIC_TEXT_INFO;
                        m_displayevent[1] = 0;
                        m_displayevent[2] = m_displayevent[3] = 0;
                        m_display(m_displayevent);

                }

                // add_and_del_timer(TIME_MP3_EVENT_START);
        }

        m_pSocketManager->CloseSocket(sock_index);
}

/*
*中心主动获得系统信息
*/
void CCmdHandler::center_get_sysinfo(int sock_index){

	WRT_MESSAGE("中心主动获取分机系统信息");
        RoomAckSysinfo(sock_index);

        m_pSocketManager->CloseSocket(sock_index); //关闭会话

}

/**
*中心设置出厂SN,MAC
*/
void CCmdHandler::center_set_factory_info(int sock_index,unsigned char* buf,int buflen)
{
	WRT_MESSAGE("中心设置出厂SN,MAC");
        if(m_pSocketManager == NULL){
               m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        if(buflen < 38){
                m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        memcpy(pSystemInfo->BootInfo.SN,buf,32);
        memcpy(pSystemInfo->BootInfo.MAC,buf+32,6);
        FlushSystemInfo();//刷新系统信息头
        m_pSocketManager->CloseSocket(sock_index);
}

/**
* 中心回应分机主动请求系统信息
*/
void CCmdHandler::center_ask_for_sysinfo_ack(int sock_index,unsigned char* buf,int buflen)
{
        if(m_pSocketManager == NULL){
                if(m_SysStatus != SYS_GETSYSINFO)
               		 m_pSocketManager->CloseSocket(sock_index);
                return;
        }

        //if(m_SysStatus == SYS_GETSYSINFO){ //是不是任何时候都可以发来这个消息呢？
        if(buflen < sizeof(T_LOCALCONFIG_ROOM)){
		if(m_SysStatus != SYS_GETSYSINFO)
                m_pSocketManager->CloseSocket(sock_index);
                //m_CurrentSocketIndex = -1;
                return;
        }


        T_LOCALCONFIG_ROOM* proomsysinfo = (T_LOCALCONFIG_ROOM*)buf;
#if 0
        if(proomsysinfo->privateinfo.LocalIP !=  pSystemInfo->LocalSetting.privateinfo.LocalIP ){
                m_pSocketManager->ModifyHostIpAndMask(proomsysinfo->privateinfo.LocalIP,0);
        }
        if(proomsysinfo->privateinfo.SubMaskIP !=  pSystemInfo->LocalSetting.privateinfo.SubMaskIP){
                m_pSocketManager->ModifyHostIpAndMask(0,proomsysinfo->privateinfo.SubMaskIP);
        }
        if(proomsysinfo->privateinfo.GateWayIP != pSystemInfo->LocalSetting.privateinfo.GateWayIP){
                m_pSocketManager->ModifyHostGateWay(proomsysinfo->privateinfo.GateWayIP);
        }
#endif
        if(m_centeripaddr  != proomsysinfo->publicinfo.CenterMajorIP)
                m_centeripaddr = proomsysinfo->publicinfo.CenterMajorIP;
        if(m_waringipaddr  != proomsysinfo->publicinfo.CenterWarningIP)
                m_waringipaddr =  proomsysinfo->publicinfo.CenterWarningIP;
        if(m_manageripaddr != proomsysinfo->publicinfo.ManagerMajorIP)
                m_manageripaddr = proomsysinfo->publicinfo.ManagerMajorIP;

        memcpy((void*)&pSystemInfo->LocalSetting,proomsysinfo,sizeof(T_LOCALCONFIG_ROOM));
        pSystemInfo->mySysInfo.sipproxyip = pSystemInfo->LocalSetting.publicinfo.CenterNetIP; //设置好SIP代理服务器地址.2010.6.3
        FlushSystemInfo();//刷新系统信息头
        WRT_MESSAGE("center_ask_for_sysinfo_ack buflen %d ---%d",buflen,sizeof(T_LOCALCONFIG_ROOM));
        m_displayevent[0] = STATUS_REMOTE_UPATE_SYSTEM_INFO;
        m_displayevent[1] = 0;
        m_displayevent[2] = m_displayevent[3] = 0;
        m_display(m_displayevent);

        if(m_SysStatus != SYS_GETSYSINFO)
                m_pSocketManager->CloseSocket(sock_index);
}

/**
*中心主动设置分机系统信息
*/
void CCmdHandler::center_set_sys_config(int sock_index,unsigned char* buf,int buflen)
{
        if(m_pSocketManager == NULL){
                  m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        WRT_MESSAGE("中心主动设置分机系统信息");
        //if(m_SysStatus == SYS_GETSYSINFO){ //是不是任何时候都可以发来这个消息呢？
        if(buflen < sizeof(T_LOCALCONFIG_ROOM)){
                m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        
        T_LOCALCONFIG_ROOM* proomsysinfo = (T_LOCALCONFIG_ROOM*)buf;
       // memcpy(callersetup.localid,proomsysinfo->privateinfo.LocalID,15);
        
        if(proomsysinfo->privateinfo.LocalIP !=  pSystemInfo->LocalSetting.privateinfo.LocalIP ){
                m_pSocketManager->ModifyHostIpAndMask(proomsysinfo->privateinfo.LocalIP,0);
        }
        if(proomsysinfo->privateinfo.SubMaskIP !=  pSystemInfo->LocalSetting.privateinfo.SubMaskIP){
                m_pSocketManager->ModifyHostIpAndMask(0,proomsysinfo->privateinfo.SubMaskIP);
        }
        if(proomsysinfo->privateinfo.GateWayIP != pSystemInfo->LocalSetting.privateinfo.GateWayIP){
                m_pSocketManager->ModifyHostGateWay(proomsysinfo->privateinfo.GateWayIP);
        }
        if(m_centeripaddr  != proomsysinfo->publicinfo.CenterMajorIP)
                m_centeripaddr = proomsysinfo->publicinfo.CenterMajorIP;
        if(m_waringipaddr  != proomsysinfo->publicinfo.CenterWarningIP)
                m_waringipaddr =  proomsysinfo->publicinfo.CenterWarningIP;
        if(m_manageripaddr != proomsysinfo->publicinfo.ManagerMajorIP)
                m_manageripaddr = proomsysinfo->publicinfo.ManagerMajorIP;

        memcpy((void*)&pSystemInfo->LocalSetting,proomsysinfo,sizeof(T_LOCALCONFIG_ROOM));
        pSystemInfo->mySysInfo.sipproxyip = pSystemInfo->LocalSetting.publicinfo.CenterNetIP; //设置好SIP代理服务器地址.2010.6.3
        m_displayevent[0] = STATUS_REMOTE_UPATE_SYSTEM_INFO;
        m_displayevent[1] = 0;
        m_displayevent[2] = m_displayevent[3] = 0;
        m_display(m_displayevent);
        pSystemInfo->LocalSetting.privateinfo.LocalID[15] = '\0';
        FlushSystemInfo();//刷新系统信息头
        char tmpbuf[256];
        memset(tmpbuf,0,256);
        memcpy(tmpbuf,pSystemInfo->LocalSetting.privateinfo.LocalID+13,2);
        g_localid = atoi(tmpbuf);
        memset(tmpbuf,0,256);
        memcpy(tmpbuf,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        wrti_setlocalid(tmpbuf);
        m_pSocketManager->CloseSocket(sock_index);

                        }

/**********************************************************************
函数功能：门口机发布广告图片
***********************************************************************/
void CCmdHandler::door_broadcast_pic(int sock_index,unsigned char* buf,int buflen){
        unsigned char type = *(buf);
        if(type == 0){
        	WRT_MESSAGE("中心，门口机，下发图片");
                SendCmd(sock_index,ROOM_BROADCASTACK,1); //回应成功
                if(/*m_SysStatus == SYS_MYROOMTALKROOM ||*/ m_SysStatus == SYS_ROOMTALKROOM ||  m_SysStatus == SYS_TALKTOMAIN ||
                        m_SysStatus == SYS_CENTERCALLROOMTALK || m_SysStatus == SYS_ROOMCALLCENTERTALK ||
                        m_SysStatus == SYS_ACTIVE_CALL || m_SysStatus == SYS_SMALLDOORCALL){
                                m_pSocketManager->CloseSocket(sock_index);

                }else{

                        if(m_mp3timerid != 0){
                                add_and_del_timer(TIME_MP3_EVENT_STOP);
                        }
                          mp3stop();
                        mp3play((char*)g_ringset[MSG_RING].ring_path,0,0);
                }

                unsigned char ctmpbuf[5];
                unsigned char ctmpbuf2[50];
                int    timelen  =0;
                memset(ctmpbuf,0,5);
                memset(ctmpbuf2,0,50);
                memcpy(ctmpbuf,buf+1,5);
                uint32_t ticks1;
                unsigned char sec;
                tm_get(NULL,&ticks1,NULL);
                sec = ticks1 & 0x000000ff;
                sprintf((char*)ctmpbuf2,"msg_20%02x-%02x-%02x_%02x-%02x-%02d.jpg",ctmpbuf[0],ctmpbuf[1],ctmpbuf[2],ctmpbuf[3],ctmpbuf[4],sec);
                add_msg2((char*)ctmpbuf2,buf+6,buflen-6);

                m_displayevent[0] = STATUS_PUBLIC_TEXT_INFO;
                m_displayevent[1] = 0;
                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent);


        }else
                SendCmd(sock_index,ROOM_BROADCASTACK,0); //回应失败

        m_pSocketManager->CloseSocket(sock_index);
}

/*****************************************
*
*中心设置帮助图片
******************************************/
void CCmdHandler::center_set_pic(int sock_index,unsigned char* buf,int buflen){

	WRT_MESSAGE("中心下载帮助图片");
        int count = 0;
        int isupdate = 0;
        count= *(int*)buf;
        if(count <16){
                int piclen[16];
                int i=0;
                unsigned char* tmpbuf = (unsigned char*)buf+76;
                memset(piclen,0,sizeof(piclen));
                memcpy(piclen,(void*)(buf+12),sizeof(piclen));
                char tmpname[10];
                int totallen =0;
                for(i =0; i<count;i++)
                        totallen += piclen[i];
                if(totallen != m_helpfilelen){
                        totallen = 0;
                        m_helpfilelen = 0;
                        isupdate = 1;
                        del_all_tmppic();
                        for(i=0; i<count;i++){
                                if(totallen > buflen)
                                        break;
                                memset(tmpname,0,10);
                                sprintf(tmpname,"pic%d",i);
                                save_tmppic(tmpbuf+totallen,piclen[i],tmpname);
                                totallen += piclen[i];
                        }
                }
        }

        if(m_SysStatus == SYS_REQHELPINFO){
                m_SysStatus = SYS_IDLE;
                m_CurrentSocketIndex = -1;
                if(sock_index != -1){
                        m_pSocketManager->CloseSocket(sock_index);
                }
        }
        WRT_DEBUG("center_set_pic 帮助信息m_SysStatus = %d ",m_SysStatus);
        if(isupdate){
                ReDrawCurrentWindow(); //重画当前窗口
        }
}

void CCmdHandler::door_start_leaveword(int sock_index){
        if(m_SysStatus == SYS_TALKTOMAIN){
                //只有本机是主分机的时候才有留言功能 否则留言消息失败
                // if(roomtest.LocalRoom.LocalID[13] == 0x00 && roomtest.LocalRoom.LocalID[14] == 0x01){

                printf("本机房号 %d\n",g_localid);
                if(g_localid == 0x01){
                		WRT_DEBUG("door_start_leaveword %d",m_CurrentSocketIndex);
                        SendCmd(m_CurrentSocketIndex,ROOM_LEAVEWORDACK);
                        
                        mp3stop();
                        //talkstart();
                        //talktostart(0,0);//ip 0 port 0 表示门口机留言本地直接收
                        talkLeaveStart();
                        m_displayevent[0] = STATUS_DOOR_LEAVEWORD;
                        m_displayevent[1] = 1;
                        m_displayevent[2] = m_displayevent[3] = 0;
                        m_display(m_displayevent); //通知GUI,开始留言
                        add_and_del_timer(TIME_DOOR_LEAVE_EVENT_START);
                        mp3stop();
                        if(m_ringtimerid != 0){
                                add_and_del_timer(TIME_RING_EVENT_STOP);
                        }
						g_is_leave = 1;
			
#ifdef HAVE_SUB_PHONE
						if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
				       			unsigned long msg_buf[4]={0};
				        		msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
				        		msg_buf[1] = 1;
				        		send_msg_to_sub_phone(msg_buf);
				        		SetTransmitToPhone(0);
						}
#endif		
			if(m_is_divert_subroom == 1){
                        //通知模拟分机挂机。
				unsigned long msg_buf[4]={0};
				msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
				msg_buf[1] = 1;
				send_msg_to_subroom(msg_buf);
				m_is_divert_subroom =0;


                        }                        
                }
                //m_display(STATUS_DOOR_LEAVEWORD,0,0); //通知GUI门口机呼叫分机
        }
}

//门口机留言
void CCmdHandler::door_stop_leaveword(int sock_index){
        if( m_SysStatus == SYS_TALKTOMAIN){
                //if(GetLeaveStatus()  == 1)
                //        talkLeaveStop();
                //talktostop();
                m_displayevent[0] = STATUS_DOOR_LEAVEWORD_STOP;
                m_displayevent[1] = 0;
                m_displayevent[2] = m_displayevent[3] = 0;
                m_display(m_displayevent); //通知GUI,留言完毕
                WRT_MESSAGE("主机停止留言");
        }
        door_hangup(sock_index);
        g_is_leave = 0;
        // door_and_center_hangup(sock_index);
}

static int isdoor(char* id){
        char tmp[7];
        memset(tmp,0,7);
        memcpy(tmp,(id+2),5);
        if(atoi(tmp) == 0){
                return 2;
        }
        return 0;
}




void CCmdHandler::door_call_room(int sock_index,unsigned long event[4])
{
    if (USE_BOMB == pSystemInfo->mySysInfo.isbomb)
        goto BUSY;

    if(SYS_IDLE == m_SysStatus || SYS_MONT == m_SysStatus )
    {
        WRT_MESSAGE("Door call room, now sysStatus is SYS_MONT");
        int isexit = 0;
        char *id = (char*)event[0];
        char tmpid[16];
        memset(tmpid, 0, 16);
        memcpy(tmpid, (char*)(id), 15);
        WRT_DEBUG("id = %s", tmpid);
        if ( pSystemInfo->mySysInfo.isagent == 1) //如果设置了托管
        { 
            SendCmd(sock_index, ROOM_AGENT);
            m_pSocketManager->CloseSocket(sock_index);
            return;
        }
        if (pSystemInfo->mySysInfo.isavoid ==1) //如果设置了免打扰
        {
            SendCmd(sock_index, ROOM_QUIET);
            m_pSocketManager->CloseSocket(sock_index);
            return;
        }
        if (SYS_MONT == m_SysStatus)
        {
            WRT_DEBUG("Stop Monitor");
#ifdef HAVE_SUB_PHONE
            if (g_phone_connect_index != -1)
            {
                stop_phone_video2(-1);
                WRT_DEBUG("SendCmd = g_phone_connect_index %d DOOR_ENDMONT",g_phone_connect_index);
                SendCmd(g_phone_connect_index, DOOR_ENDMONT);
                if(m_pSocketManager)
                    m_pSocketManager->CloseSocket(g_phone_connect_index);
                g_phone_connect_index = -1;
            }
#endif
            if (m_watchtimerid != 0) //取消监视计时器
            {
                add_and_del_timer(TIME_MONT_EVENT_STOP);
                if(m_CurrentSocketIndex != -1)
                    SendCmd(m_CurrentSocketIndex,ROOM_STOPMONT);
            }

            Stopplayvideo();

            isexit = 1;
            if(m_CurrentSocketIndex != -1)
                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
            m_CurrentSocketIndex = -1;
        }

        SendCmd(sock_index,ROOM_IDLE);
        m_CurrentSocketIndex =sock_index;     
        m_SysStatus = SYS_TALKTOMAIN;
        m_displayevent[0]=STATUS_DOOR_CALLROOM;
        m_displayevent[1] = isexit;
        if (_iswall(tmpid) == 1)
        {
            m_displayevent[2] =2; //围墙机
            m_isdoor = TYPE_WALL_CALLME;
            m_displayevent[3]= 0;
        }
        else if (strncmp((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,tmpid,13) == 0)
        {
            m_displayevent[2] = 3;
            m_displayevent[3] = atoi(id+13) - 31;
            m_isdoor = TYPE_IPDOOR_CALLME; //数字小门口机
        }
        else //门口机
        {
            m_displayevent[2] = 0;
            m_isdoor = TYPE_DOOR_CALLME; //门口机
            m_displayevent[3]= 0;
        }
        leavestop();

        mp3stop();

        m_display(m_displayevent);
#ifdef HAVE_SUB_PHONE
        if(have_idle_phone() != 0 && m_phone_ip != 0 )
        {
            unsigned long msg_buf[4];
            char* pPeerID  = (char*)ZENMALLOC(16);
            if(pPeerID){
                memset(pPeerID,0,16);
                memcpy(pPeerID,m_UserPhoneNumber,15);
                msg_buf[0] = DOOR_CALL_PHONE;
                msg_buf[1] = m_phone_ip;
                msg_buf[2] = 0;
                msg_buf[3] = (unsigned long)pPeerID;
                send_msg_to_sub_phone(msg_buf);
                m_is_transmit_to_phone = 1;
            }
        }
#endif
        printf("g_localid %d\n", g_localid);
        if (g_localid == 0x01)
        {
            WRT_DEBUG("send call cmd to subroom 0000");
            int i = 0;
            for(i = 0; i<MAX_SUBROOM;i++){
                unsigned long tmpsubroomip = get_subroom_ip_by_index(i); 
                if(tmpsubroomip != 0 && tmpsubroomip != 0xffffffff)
                    break;
            }
            if (MAX_SUBROOM != i)
            {
                WRT_DEBUG("send call cmd to subroom 1111");
                unsigned long msg_buf[4];
                char* pPeerID  = (char*)ZENMALLOC(16);
                if(pPeerID)
                {
                    memset(pPeerID,0,16);
                    memcpy(pPeerID,m_UserPhoneNumber,15);
                    msg_buf[0] = DOOR_CALL_SUBROOM;
                    msg_buf[1] = 0;
                    msg_buf[2] = 0;
                    msg_buf[3] = (unsigned long)pPeerID;
                    send_msg_to_subroom(msg_buf);
                    m_is_divert_subroom = 1;
                }
            }
        }
        Startplayvideo(NULL);
        if (1 == pSystemInfo->mySysInfo.isautocap)
        {
            WRT_DEBUG("Auto cap enable, try to Capture Photo1");
            uint32_t newtime;
            unsigned long result=0;
            tm_getticks(NULL,(uint32_t *)&newtime);
            result = newtime - g_cap_time;
            if (result > 6000)
            {
                g_cap_time = newtime;
                CapturePhoto1();//启动自动抓拍
            }
        }

        if(m_ringtimerid != 0){
            add_and_del_timer(TIME_RING_EVENT_STOP);
        }
        if(m_mp3timerid != 0){
            add_and_del_timer(TIME_MP3_EVENT_STOP);
        }

        add_and_del_timer(TIME_RING_EVENT_START);

        SDL_Delay(100);
        SetSubSpkStatus(1);
    }
    else 
    {
BUSY:
        SendCmd(sock_index,ROOM_BUSY);
        m_pSocketManager->CloseSocket(sock_index);
    }
}

void CCmdHandler::door_ack_unlock(int sock_index)
{
	/*
	if(SYS_TALKTOMAIN == m_SysStatus){
	SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);
	m_displayevent[0]=STATUS_DOOR_UNLOCK;
	m_displayevent[1] = 0;
	m_displayevent[2]=m_displayevent[3]= 0;
	m_display(m_displayevent);
	if(m_pSocketManager)
	m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
	m_SysStatus = SYS_IDLE;
	m_CurrentSocketIndex = -1;
	}
	*/
}

void CCmdHandler::room_unlock(int sock_index)
{
		WRT_MESSAGE("分机开锁");
        if(SYS_TALKTOMAIN  == m_SysStatus ){
                SendCmd(m_CurrentSocketIndex,ROOM_UNLOCK);
                add_and_del_timer(TIME_UNLOCK_EVENT_START); //5M后 自动挂机
        }
        if(SYS_SMALLDOORCALL  == m_SysStatus){
                unsigned short tmpevent[4];
                tmpevent[0] = ROOM_OPENLOCK_SMALLDOOR;
                tmpevent[1] = 0;
                tmpevent[2] = m_smalldoor_id; //目标ID
                tmpevent[3] = 0x0;
                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                //  m_smalldoor_cmd =  ROOM_UNLOCK;


                // add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);

        }
}

int g_exit_status = 0;
void CCmdHandler::room_holdon(int sock_index,unsigned long event[4])
{
	WRT_MESSAGE("room holdon");
		
        if( m_SysStatus == SYS_SIPCALL){
                unsigned long tmpevent[4];
                tmpevent[0] = ROOM_HOLDON;
                tmpevent[1] = tmpevent[2] =tmpevent[3] =0;
                udp_send_data2(pSystemInfo->mySysInfo.sipproxyip,tmpevent);
                talktostop();
                talkstop();
                m_displayevent[0] = STATUS_SIP_START_TALK;
                m_displayevent[1] = 1;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
                WRT_DEBUG("SYS_SIPCALL 0x%x,%d",m_sipmediaaddr,m_sipmediaport);
                talkstart();
                talktostart(m_sipmediaaddr,m_sipmediaport);
                return;
        }
        if(SYS_TALKTOMAIN == m_SysStatus || SYS_CENTERCALLROOMTALK == m_SysStatus||
			SYS_ROOMTALKROOM == m_SysStatus ||SYS_SMALLDOORCALL == m_SysStatus)
		{

                        unsigned long tmpip = 0;
                        //////////////////
                        //2008.10.13
                        tmpip= m_pSocketManager->GetIPAddrByIndex(m_CurrentSocketIndex); // 2008.10.8 by ljw add
                        //end

                        g_holdonbyme = 1;

                        mp3stop();

                        
                        if(m_ringtimerid != 0){//取消震铃计时
                                WRT_DEBUG("cancel TIME_RING_EVENT_STOP");
                                add_and_del_timer(TIME_RING_EVENT_STOP);
                        }
                        if(m_doorleave_timerid != 0){ //如果响铃时间刚好到了，同时摘机取消第二次计时器
                                add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);
                        }
                        if(GetLeaveStatus() == 1){ //如果在留言过程中摘机，则取消留言。
                                talkLeaveCancel();
                                //talktostart(tmpip,15004);
                               // WRT_DEBUG("本机摘机");
                                //SendCmd(m_CurrentSocketIndex,ROOM_HOLDON);
                                //add_and_del_timer(TIME_TALK_EVENT_START);//120s通话时间
                                //return ;
                        }else
                        {
                        	talktostop();
                        	talkstop();
                        }
                        if(GetsmalldoorLeaveStatus()){
                                WRT_DEBUG("GetsmalldoorLeaveStatus");

                                cancelsmalldoorleave();
                                add_and_del_timer(TIME_SMALLDOOR_LEAVE_EVENT_STOP); //取消播放留言声音计时器
                        }
                        
                        if(m_talktimerid != 0){
                                add_and_del_timer(TIME_TALK_EVENT_STOP);
                        }


                        if(sock_index == -1){//是我主动摘机，是否不需要启动通话计时
#ifdef HAVE_SUB_PHONE
                                if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
                                        unsigned long msg_buf[4]={0};
                                        msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                                        msg_buf[1] = 1;
                                        send_msg_to_sub_phone(msg_buf);
                                        SetTransmitToPhone(0);
                                }
#endif
				if(g_localid == 0x01 && m_is_divert_subroom == 1){
					printf("xxxxxxx zzzz\n");
					unsigned long msg_buf[4];
					msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
					if(SYS_TALKTOMAIN == m_SysStatus)
						msg_buf[1] = 1;
					else if(SYS_CENTERCALLROOMTALK == m_SysStatus)
						msg_buf[1] = 2;
					else if(SYS_ROOMTALKROOM == m_SysStatus)
						msg_buf[1] = 3;
					msg_buf[2] = 0;
					msg_buf[3] = 0;
					send_msg_to_subroom(msg_buf);
					m_is_divert_subroom = 0;
				}
								int tmp = 0;
								tmp = GetIsDoorCall();

								if(tmp == 3)
				{
					if(strncmp((const char *)m_UserPhoneNumber,(const char *)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0)
					{//呼叫方为本户其它房间
									SendCmd2(m_CurrentSocketIndex,MEDIA_NEGOTIATE_ACK,NULL,0);
					}else if(g_localid >= 0x01 && g_localid <= 0x04)
					{
						SendCmd2(m_CurrentSocketIndex,MEDIA_NEGOTIATE_ACK,NULL,0);
					}
				}
                                SendCmd(m_CurrentSocketIndex,ROOM_HOLDON);
                                m_displayevent[1] = 0;

				if(wrthost_get_holdon_device() ==  NONE_HOLDON)
					wrthost_set_holdon_device(LOCAL_HOLDON);
                        }
                        if(sock_index == -2) //手机摘机
                        {

#ifdef HAVE_SUB_PHONE
								int tmp = 0;
								tmp = GetIsDoorCall();
								if(tmp == 3)//区别与主机，主机在收到协商后 马上回复
									SendCmd2(m_CurrentSocketIndex,MEDIA_NEGOTIATE_ACK,NULL,0);
                                SendCmd(m_CurrentSocketIndex,ROOM_HOLDON);
                                m_displayevent[1] = 1;
				if(g_localid == 0x01 && m_is_divert_subroom == 1){
					unsigned long msg_buf[4];
					msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
                    if(SYS_TALKTOMAIN == m_SysStatus)
                    	msg_buf[1] = 1;
                    else if(SYS_CENTERCALLROOMTALK == m_SysStatus)
                    	msg_buf[1] = 2;
                    else if(SYS_ROOMTALKROOM == m_SysStatus)
                    	msg_buf[1] = 3;
					msg_buf[2] = 0;
					msg_buf[3] = 0;
					send_msg_to_subroom(msg_buf);
					m_is_divert_subroom = 0;
				}
				wrthost_set_holdon_device(IOSDEV_HOLDON);
#endif
                        }
			if(sock_index == -4){
            	SendCmd(m_CurrentSocketIndex,ROOM_HOLDON);
                m_displayevent[1] = 1;
#ifdef HAVE_SUB_PHONE
                if(GetTransmitToPhone() == 1){
                    unsigned long msg_buf[4]={0};
                    msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                    if(SYS_TALKTOMAIN == m_SysStatus)
                    	msg_buf[1] = 1;
                    else if(SYS_CENTERCALLROOMTALK == m_SysStatus)
                    	msg_buf[1] = 2;
                    else if(SYS_ROOMTALKROOM == m_SysStatus)
                    	msg_buf[1] = 3;
                    send_msg_to_sub_phone(msg_buf);
                    SetTransmitToPhone(0);
	            }
#endif
	            wrthost_set_holdon_device(MOREROOM_HOLDON);
            }
                        
						if(SYS_TALKTOMAIN == m_SysStatus || SYS_SMALLDOORCALL == m_SysStatus){
							m_displayevent[1] = 2;
						}

                        m_displayevent[0]=STATUS_ROOM_TALK_ROOM;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                        WRT_DEBUG("TIME_TALK_EVENT_START");
                        add_and_del_timer(TIME_TALK_EVENT_START);//120s通话时间

                        if(sock_index == -2)
                        {
#ifdef HAVE_SUB_PHONE
								unsigned long phoneip = get_connect_ip(event[2]);
								start_wrt_subroomaudio(phoneip,15004,tmpip,15004,15004,15002);
			
                                
								g_phone_or_externdevice_holdon = 0;
								g_exit_status = 1;
#endif
            }else if(sock_index == -4){//5-8分机摘机
            	unsigned long phoneip = get_subroom_connect_ip(event[2]);
            	printf("send to subroom ip 0x%x\n",phoneip);
                start_wrt_subroomaudio(phoneip,15004,tmpip,15004,15004,15002);
                g_phone_or_externdevice_holdon = 0;
				g_exit_status = 1;
				printf("sub room holdon\n");
						}else{
#ifdef HAVE_SUB_PHONE
                if(GetTransmitToPhone() == 1){ //当手机呼叫其他住户，其他住户摘机
                        start_talkto_phone(m_phone_ip,15004); //转发音频数据到手机
                        start_phone_audio(tmpip,15004);//转发手机音频数据到对端。
                        return;
                }else{
					if(g_localid >= 5 && g_localid <= 8){
						char idbuf[4];
						memset(idbuf,0,4);
						memcpy(idbuf,(char*)m_UserPhoneNumber+13,2);
						if(strncmp((char*)m_UserPhoneNumber,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0)
						{
							if(atoi(idbuf) < 32)
								talktostart(tmpip,15004);
							else
								talktostart(tmpip,15002);
						}
						else
							talktostart(tmpip,15002);
					}else
                                        talktostart(tmpip,15004);
					                    SDL_Delay(200);
                                        int tmp = 0;
										tmp = GetIsDoorCall();
										printf("\n ===============restart %d\n",tmp);

										if(tmp == 3)
										{
											if(get_vdec_type() >= V_MPEG4)
												Startplayvideo(NULL);

	                                        if(get_venc_type()>= V_MPEG4)//协商为1则为H264编解码
	                                        {
	                                			unsigned long ip[4];
	                                			ip[1]=tmpip;
	                                			Startsendvideo(&ip[1]);
	                                		}
                                		}
                                			
                                        WRT_DEBUG("开始视频传输 IP:0x%x",tmpip);
                                		WRT_DEBUG("开始通话");
                                }
#else
                                talkstart();
                                talktostart(tmpip,15004);//本机启动音频输入输出
#endif

                        }

					if(g_phone_or_externdevice_holdon == 0 || g_exit_status == 1)//外接分机或者手机摘机
					{
						m_displayevent[0] = STATUS_PHONE_OR_EXTERNDEVICE_HOLDON;
						m_displayevent[2] = m_displayevent[3]= 0;
						m_display(m_displayevent);
						g_exit_status = 0;
			        }
	}
}

void CCmdHandler::room_hangup(int sock_index){

		wrthost_set_holdon_device(NONE_HOLDON);
	WRT_MESSAGE("分机挂机 index:%d",sock_index);
        if(m_SysStatus == SYS_CALLSIP || m_SysStatus  == SYS_SIPCALL){
        
                talktostop();
                unsigned long tmpevent[4];
                tmpevent[0] = ROOM_HANGUP;
                tmpevent[1] = tmpevent[2] =tmpevent[3] =0;
                udp_send_data2(pSystemInfo->mySysInfo.sipproxyip,tmpevent);
                m_displayevent[0]=STATUS_ROOM_HANGUP;
                m_displayevent[1] = 0;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
                m_SysStatus = SYS_IDLE;

                return;
        }

        if(m_SysStatus == SYS_ACTIVE_CALL){

                const  WRTI_CallerSetup_t * p = wrti_GetCallerParam();
#ifdef HAVE_SUB_PHONE
                if(m_is_phone_to_transmit == 1)
                {
                        while(1){
                                if(p && (p->callerstatus == 2)  || (p->callerstatus == 0))
                                {
                                        SDL_Delay(1000);
                                }else
                                        break;
                        }
                }
#endif
                if(p && (p->callerstatus == 2) || (p->callerstatus == 0))
                {
                        //是否通知上面，暂时不能挂机
                        return;
                }
				
				
                WRT_DEBUG("房间挂机....");
                wrti_devicehungup(htons(ROOM_HANGUP));
                mp3stop();

                	Stopsendvideo();

                	Stopplayvideo();
                SDL_Delay(100);
                m_displayevent[0]=STATUS_ROOM_HANGUP;
                m_displayevent[1] = 0;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
                m_SysStatus = SYS_IDLE;
#ifdef HAVE_SUB_PHONE
                if(m_is_phone_to_transmit == 1 )
                {
                        //stop_phone_audio();
						stop_wrt_subroomaudio();
                        m_is_phone_to_transmit = 0;
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;
			return;//
                }
                if(m_is_phone_call_room == 1){
                        //stop_phone_audio();
						stop_wrt_subroomaudio();
                        m_is_phone_call_room = 0;
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;                	
			return ;
                }
#endif
		talktostop();
                return;
        }
        if(SYS_TALKTOMAIN  == m_SysStatus  ||
                SYS_CENTERCALLROOMTALK == m_SysStatus ||
                SYS_ROOMTALKROOM == m_SysStatus ||
                /*  SYS_MYROOMTALKROOM == m_SysStatus ||*/
                SYS_ROOMCALLCENTERTALK == m_SysStatus  ||
                SYS_GETROOMIP == m_SysStatus ||
		SYS_SMALLDOORCALL == m_SysStatus )
	{
                        if(g_holdonbyme == 1)
                                g_holdonbyme  = 0;
#ifdef HAVE_SUB_PHONE
                        if((sock_index == -3) && (m_is_transmit_to_phone == 1)){
                        		
                                //stop_phone_audio();
                                
								stop_wrt_subroomaudio();
                            	stop_phone_video2(-1);

								if(g_localid == 0x01 && m_is_divert_subroom == 1){
									unsigned long msg_buf[4];
									msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
									if(SYS_TALKTOMAIN == m_SysStatus)
											msg_buf[1] = 1;
									else if(SYS_CENTERCALLROOMTALK == m_SysStatus)
											msg_buf[1] = 2;
									else if(SYS_ROOMTALKROOM == m_SysStatus)
											msg_buf[1] = 3;
									msg_buf[2] = 0;
									msg_buf[3] = 0;
									send_msg_to_subroom(msg_buf);
									m_is_divert_subroom = 0;									
								}
                                m_is_transmit_to_phone =0;
                                if(g_is_leave == 1)
                                	return;
                        }

                        if((sock_index != -3) && (m_is_transmit_to_phone == 1))
                        {
                        		
                        	 	//stop_phone_audio();
								stop_wrt_subroomaudio();
                                if(m_SysStatus == SYS_SMALLDOORCALL)
                                {
                                        start_or_stop_smvideo_phone(0,0);
                                }
                                unsigned long msgbuf[4];
                                msgbuf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                                msgbuf[1] = 1;
                                msgbuf[2] = msgbuf[3] = 0;
                                send_msg_to_sub_phone(msgbuf);
                                //talkstop();

                                m_is_transmit_to_phone = 0;
                        }
#endif
					if(g_localid == 0x01 && (sock_index != -4) && m_is_divert_subroom == 1 ){ //通知副机挂机
						unsigned long msg_buf[4];
						msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
						if(SYS_TALKTOMAIN == m_SysStatus)
                            msg_buf[1] = 1;
                        else if(SYS_CENTERCALLROOMTALK == m_SysStatus)
                            msg_buf[1] = 2;
                        else if(SYS_ROOMTALKROOM == m_SysStatus)
                            msg_buf[1] = 3;
						msg_buf[2] = 0;
						msg_buf[3] = 0;
						send_msg_to_subroom(msg_buf);
						m_is_divert_subroom = 0;
					}
					if(sock_index == -4){ //从分机挂机
						stop_wrt_subroomaudio();
		                stop_phone_video2(-1);
		                if(g_is_leave == 1)
		                	return;                                			
					}
						mp3stop();
						if(m_ringtimerid != 0){//如果没有摘机，取消震铃计时
								add_and_del_timer(TIME_RING_EVENT_STOP);
						}
						if(sock_index == -1) { //表示我主动挂机
                                WRT_DEBUG("m_CurrentSocketIndex %d主动挂机",m_CurrentSocketIndex);
                                SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);
                        }
						SDL_Delay(100);
						if(GetLeaveStatus() == 1 && SYS_TALKTOMAIN == m_SysStatus) //如果在留言过程中挂机，则取消留言
								talkLeaveStop();
                        
						if(GetsmalldoorLeaveStatus()){
								stopsmalldoorleave();
						}
                      

						if(g_phone_or_externdevice_holdon != 1 && g_holdon_flag != 2 && g_holdon_flag != 0)
				   			stop_wrt_subroomaudio();
				   		else
							talktostop();//双向音频消回声关闭
						
                        
						
                        int i =0;
						Stopsendvideo();
                        if(SYS_TALKTOMAIN  == m_SysStatus)
						{
                                
				            Stopplayvideo();
							WRT_DEBUG("分机挂机关闭视频1");
						}
                        if(g_dec_mode == 0)
		                {
		                	WRT_DEBUG("分机挂机关闭视频2");
		                	Stopplayvideo();
		                }

                        SDL_Delay(100);
                        
						if(g_is_leave == 1)
							g_is_leave = 0;

                        if(m_talktimerid != 0){//如果有摘机，时间到，或者主动挂机，取消通话计时
                                add_and_del_timer(TIME_TALK_EVENT_STOP);
                        }
                        if(m_unlock_timerid != 0){ //取消开锁计时器
                                add_and_del_timer(TIME_UNLOCK_EVENT_STOP);
                        }
                        if(m_doorleave_timerid != 0){ //取消第二次计时器
                                add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);
                        }

 


		switch(sock_index){
			case -6:
                                WRT_DEBUG("m_CurrentSocketIndex %d,托管",m_CurrentSocketIndex);
                                SendCmd(m_CurrentSocketIndex,ROOM_AGENT);
				break;
                        
				
			case -3:
			case -4:
				WRT_DEBUG("m_CurrentSocketIndex %d ",m_CurrentSocketIndex);
                                SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);
				break;

                        }
                        int ok = 0;
                        if(SYS_TALKTOMAIN  == m_SysStatus ){
                                if(pSystemInfo->mySysInfo.isautocap == 1)
                                        CancelCapturePhoto1();//取消自动抓拍图片
                                ok = 1; //关闭视频
                        }
                        m_displayevent[0]=STATUS_ROOM_HANGUP;
                        m_displayevent[1] = ok;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        if(sock_index != -2)
                                m_display(m_displayevent);
                        //if(SYS_MYROOMTALKROOM != m_SysStatus)  //如果不是主动户户通挂机
                        WRT_DEBUG("主动关闭m_CurrentSocketIndex %d",m_CurrentSocketIndex);
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_SysStatus = SYS_IDLE;
                        m_CurrentSocketIndex = -1;
                        if(g_used_simulate_channel == 1){
                                m_displayevent[0]=STATUS_SCREEN_CLOSE;
                                m_displayevent[1] = 0;
                                m_displayevent[2]=m_displayevent[3]= 0;
                                m_display(m_displayevent);
                        }


        }else{
				m_displayevent[0] = STATUS_ROOM_HANGUP;
				m_displayevent[1] = 0;
				m_displayevent[2] = m_displayevent[3]= 0;

                if(m_CurrentSocketIndex != -1)
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                m_SysStatus = SYS_IDLE;
                m_CurrentSocketIndex = -1;
                if(sock_index != -2)
                        m_display(m_displayevent);
        }
}

/*中心呼叫房间*/
void CCmdHandler::center_call_room(int sock_index){
        if(pSystemInfo->mySysInfo.isbomb == USE_BOMB)
                goto BUSY;
		Reset_negotiate_result();

        if(m_SysStatus == SYS_IDLE || SYS_MONT == m_SysStatus){

                int isexit = 0;
                if(pSystemInfo->mySysInfo.isagent == 1){//如果设置托管
                        SendCmd(sock_index,ROOM_AGENT);
                        m_pSocketManager->CloseSocket(sock_index);
                        return ;
                }
                if(pSystemInfo->mySysInfo.isavoid == 1){ //如果设置了免打扰
                        SendCmd(sock_index,ROOM_QUIET);
                        m_pSocketManager->CloseSocket(sock_index);
                        return;
                }
                if(m_SysStatus == SYS_MONT){
#ifdef HAVE_SUB_PHONE
                        if(g_phone_connect_index != -1)
                        {
                                stop_phone_video2(-1);
                                WRT_DEBUG("g_phone_connect_index = %d DOOR_ENDMONT",g_phone_connect_index);
                                SendCmd(g_phone_connect_index,DOOR_ENDMONT);
                                if(m_pSocketManager)
                                        m_pSocketManager->CloseSocket(g_phone_connect_index);
                                g_phone_connect_index = -1;

                        }
#endif                	
                        if(m_watchtimerid != 0){ //取消监视计时器
                                add_and_del_timer(TIME_MONT_EVENT_STOP);
                                SendCmd(m_CurrentSocketIndex,ROOM_STOPMONT);
                        }

                        Stopplayvideo();
                        isexit = 1;
                        if(m_CurrentSocketIndex != -1)
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;
                        m_displayevent[0]=STATUS_ROOM_STOPMONT;
                        m_displayevent[1] = 1;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);

                }
                
                



                SendCmd(sock_index,ROOM_IDLE);
                m_CurrentSocketIndex =sock_index;
                m_SysStatus = SYS_CENTERCALLROOMTALK;

#ifdef HAVE_SUB_PHONE
                if(have_idle_phone() != 0 && m_phone_ip != 0)
                {
                        unsigned long msg_buf[4];
                        char* pPeerID  = (char*)ZENMALLOC(16);
                        if(pPeerID){
                                memset(pPeerID,0,16);
                                memcpy(pPeerID,m_UserPhoneNumber,15);
                                msg_buf[0] = CENTER_CALL_PHONE;
                                msg_buf[1] = m_phone_ip;
                                msg_buf[2] = 0;
                                msg_buf[3] = (unsigned long)pPeerID;
                                send_msg_to_sub_phone(msg_buf);
                                m_is_transmit_to_phone = 1;
                        }
                }
#endif
				if(g_localid == 0x01){
					int i = 0;
					for(i = 0; i<MAX_SUBROOM;i++){
						unsigned long tmpsubroomip = get_subroom_ip_by_index(i); 
						 if(tmpsubroomip != 0 && tmpsubroomip != 0xffffffff)
								break;
					}
					if(i != MAX_SUBROOM){
						unsigned long msg_buf[4];
						char* pPeerID  = (char*)ZENMALLOC(16);
						if(pPeerID){
							memset(pPeerID,0,16);
							memcpy(pPeerID,m_UserPhoneNumber,15);
							msg_buf[0] = CENTER_CALL_SUBROOM;
							msg_buf[1] = 0;
							msg_buf[2] = 0;
							msg_buf[3] = (unsigned long)pPeerID;
							send_msg_to_subroom(msg_buf);
							m_is_divert_subroom = 1;
						}
					}	
				}

                m_displayevent[0]=STATUS_CENTER_CALLROOM;
                m_displayevent[1] = isexit; 
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);

                if((pSystemInfo->mySysInfo.isnvo == 1) && (g_localid == 0x01) && (g_used_simulate_channel == 0)){ //启动挂接模拟分机
                        unsigned short tmpe[4];
                        tmpe[0] = CALL_SIMULATE;
                        tmpe[1] = 0x01; //数据网络呼叫。
                        tmpe[2] = 0x00; //目的ID 表示广播
                        tmpe[3] = 0x01; //表示是否有数据，0，没带数据，1，带有数据
                        send_smalldoor_cmd(tmpe);

                        send_smalldoor_cmd(tmpe);
                }    


                if(m_ringtimerid != 0){
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                if(m_mp3timerid != 0){
                        add_and_del_timer(TIME_MP3_EVENT_STOP);
                }
                leavestop();
                mp3stop();
                SDL_Delay(200);
                mp3play((char*)g_ringset[CENTER_CALL_RING].ring_path, 0,3);
                add_and_del_timer(TIME_RING_EVENT_START);
                SDL_Delay(100);
				SetSubSpkStatus(1);

        }else{
BUSY:
				//if(m_SysStatus == SYS_CENTERCALLROOMTALK)
				//	;//m_pSocketManager->CloseSocket(sock_index);
				//else
				//{
	                SendCmd(sock_index,ROOM_BUSY);
	                m_pSocketManager->CloseSocket(sock_index);
                //}
        }
}

void CCmdHandler::door_hangup(int sock_index){
	WRT_MESSAGE("门口机挂机1");
        if(SYS_TALKTOMAIN  == m_SysStatus){
				wrthost_set_holdon_device(NONE_HOLDON);
                if(g_holdonbyme == 1)
                        g_holdonbyme  = 0;
                else{
                	if(g_localid == 0x01){
                		unsigned long tmpevent[4]={0};
		           		SendColorMsg(tmpevent);  
                            	WRT_MESSAGE("主机呼叫，未摘机，门口机主动挂机，通知中心发送彩信");
                      	}
                }

#ifdef HAVE_SUB_PHONE
                if(have_connect_phone() != 0 && m_is_transmit_to_phone == 1){
                        stop_phone_video2(-1);
                        unsigned long msgbuf[4] ={0};
                        msgbuf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                        msgbuf[1] = 1;
                        send_msg_to_sub_phone(msgbuf);
                        //stop_phone_audio();
                    stop_wrt_subroomaudio();
                        m_is_transmit_to_phone = 0;
                }
#endif
				if(g_localid == 0x01 && m_is_divert_subroom == 1){
					unsigned long msg_buf[4];
					msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
					msg_buf[1] = 1;
					msg_buf[2] = 0;
					msg_buf[3] = 0;
					send_msg_to_subroom(msg_buf);
					m_is_divert_subroom = 0;
				}


                if(pSystemInfo->mySysInfo.isautocap == 1)
                        CancelCapturePhoto1();//取消自动抓拍图片
                
                if(m_ringtimerid != 0){//门口机挂机，取消震铃计时器
                		mp3stop();
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                Stopplayvideo();
                Stopsendvideo();
				SDL_Delay(300);
                
                if(GetLeaveStatus()  == 1) //如果留言启动停止留言
                {
               		if(m_doorleave_timerid != 0)  //如果留言启动停止留言
                        add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);
                    talkLeaveStop();
                        
                }
                else
                {
                	if(m_talktimerid != 0){//如果有摘机，取消通话计时器
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
               		}
	               	if(g_phone_or_externdevice_holdon != 1 && g_holdon_flag != 2 && g_holdon_flag != 0)
               			stop_wrt_subroomaudio();
               		else
                	talktostop();
                }
                

                if(g_smalldoor_call_agent == 1){
                        if(m_CurrentSocketIndex != -1)
                                SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);
                        unsigned long tmpevent1[4] ={0};
                        tmpevent1[0] = ROOM_USED_SIMULATE;
                        tmpevent1[1] = 0;
                        tmpevent1[2] = 0;
                        tmpevent1[3] = 4;
                        udp_send_data(tmpevent1);  //通知其他主、从分机,通道释放。
                }

                if(m_pSocketManager){
                        if(m_CurrentSocketIndex != -1)
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;
                }
                if(g_smalldoor_call_agent == 1){ //如果小门口呼叫，分机托管 ,给小门口机发送消息
                        unsigned short tmpevent[4];

                        tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                        tmpevent[1] = 0;
                        tmpevent[2] = m_smalldoor_id; //目标ID
                        tmpevent[3] = 0;
                        send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机

                        WRT_DEBUG("g_smalldoor_call_agent ，托管，中心挂机");

                        return;
                }
                if(pSystemInfo->mySysInfo.isnvo == 1 && g_localid == 0x01 )
                {


                        if(g_simulate_talk == 1){
                                g_simulate_talk = 0;	
                                RoomGpioUtilSetDoorAudioOff(); 
                        }
                        unsigned short tmpevent[4];

                        tmpevent[0] = INFORM_SIMULATE_HANGUP;
                        tmpevent[1] = 0;
                        tmpevent[2] = 0x0; //目标ID
                        tmpevent[3] = 0;
                        send_smalldoor_cmd(tmpevent); //通知模拟分机，挂机。												             	
                        send_smalldoor_cmd(tmpevent); //通知模拟分机，挂机。	


                }
                if(m_SysStatus == SYS_TALKTOMAIN ){
                        //通知GUI门口机挂机
                        m_displayevent[0]=STATUS_DOOR_HANGUP;
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }

                m_SysStatus = SYS_IDLE;
                if(g_used_simulate_channel == 1){
                        m_displayevent[0]=STATUS_SCREEN_CLOSE;
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }
        }
}


/*中心挂机*/
void CCmdHandler::door_and_center_hangup(int sock_index){
        WRT_MESSAGE("中心挂机 = %d ",m_SysStatus);
        
        if(SYS_CENTERCALLROOMTALK == m_SysStatus  ||
                SYS_ROOMCALLCENTERTALK == m_SysStatus)
        {
				wrthost_set_holdon_device(NONE_HOLDON);
                if(g_holdonbyme == 1)
                        g_holdonbyme  = 0;
#ifdef HAVE_SUB_PHONE
                if(have_connect_phone() != 0 && m_is_transmit_to_phone == 1){
                        stop_phone_video2(-1);
                        unsigned long msgbuf[4] ={0};
                        msgbuf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                        msgbuf[1] = 2;
                        send_msg_to_sub_phone(msgbuf);
                        //stop_phone_audio();
                        stop_wrt_subroomaudio();
                        m_is_transmit_to_phone = 0;
                        
                }
#endif

				if(g_localid == 0x01 && m_is_divert_subroom == 1){
					unsigned long msg_buf[4];
					msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM;
					msg_buf[1] = 2;
					msg_buf[2] = 0;
					msg_buf[3] = 0;
					send_msg_to_subroom(msg_buf);
					m_is_divert_subroom = 0;
				}
                
                if(SYS_TALKTOMAIN == m_SysStatus ){
                        if(pSystemInfo->mySysInfo.isautocap == 1)
                        	CancelCapturePhoto1();//取消自动抓拍图片
                        Stopplayvideo();
                }
                
                mp3stop();
                if(m_ringtimerid != 0){//门口机挂机，取消震铃计时器
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                if(m_doorleave_timerid != 0)  //如果留言启动停止留言
                        add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);

				
                if(GetLeaveStatus()  == 1) //如果留言启动停止留言
                        talkLeaveStop();
                else
	            {
	            	if(g_phone_or_externdevice_holdon != 1 && g_holdon_flag != 2 && g_holdon_flag != 0)
               			stop_wrt_subroomaudio();
               		else
                	talktostop();
	            }
                
                
                if(m_talktimerid != 0){//如果有摘机，取消通话计时器
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
                }
                if(g_smalldoor_call_agent == 1){
                        if(m_CurrentSocketIndex != -1)
                                SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);
                        //通知其他分机，释放模拟通道
                        unsigned long tmpevent1[4] ={0};
                        tmpevent1[0] = ROOM_USED_SIMULATE;
                        tmpevent1[1] = 0;
                        tmpevent1[2] = 0;
                        tmpevent1[3] = 4;
                        udp_send_data(tmpevent1);  //通知其他主、从分机,通道已经释放。
                }

                if(m_pSocketManager){
                        if(m_CurrentSocketIndex != -1)
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                        m_CurrentSocketIndex = -1;
                }
                if(g_smalldoor_call_agent == 1){ //如果小门口呼叫，分机托管 ,给小门口机发送消息
                        unsigned short tmpevent[4];

                        tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                        tmpevent[1] = 0;
                        tmpevent[2] = m_smalldoor_id; //目标ID
                        tmpevent[3] = 0;
                        send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                        // m_smalldoor_cmd = CENTER_HANGUP;
                        // add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);


                        WRT_DEBUG("g_smalldoor_call_agent ，托管，中心挂机");

                        return;
                }

                if(pSystemInfo->mySysInfo.isnvo == 1  && g_localid == 0x01 ) //只有主分机处理该消息
                {
                        if(g_simulate_talk == 1){
                                g_simulate_talk = 0;	
                                RoomGpioUtilSetDoorAudioOff(); //如果是托管挂机，则还原为通常状态   
                        }
                        unsigned short tmpevent[4];

                        tmpevent[0] = INFORM_SIMULATE_HANGUP;
                        tmpevent[1] = 0;
                        tmpevent[2] = 0x0; //目标ID
                        tmpevent[3] = 0;
                        send_smalldoor_cmd(tmpevent); //通知模拟分机，挂机。												             	

                        send_smalldoor_cmd(tmpevent); //通知模拟分机，挂机。	

                }

                if(m_SysStatus == SYS_CENTERCALLROOMTALK || SYS_ROOMCALLCENTERTALK == m_SysStatus){
                        //通知GUI中心挂机
                        m_displayevent[0]=STATUS_CENTER_HANGUP;
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }
                if(m_SysStatus == SYS_TALKTOMAIN ){
                        //通知GUI门口机挂机
                        m_displayevent[0]=STATUS_DOOR_HANGUP;
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }

                m_SysStatus = SYS_IDLE;
                if(g_used_simulate_channel == 1){
                        m_displayevent[0]=STATUS_SCREEN_CLOSE;
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }

        }
}

void CCmdHandler::room_call_center(int sock_index){
	WRT_MESSAGE("分机呼叫中心");
	Reset_negotiate_result();
        if(SYS_IDLE == m_SysStatus){
                int num = 0;
                int i=0;

#ifdef HAVE_SUB_PHONE
                if(sock_index != -1) //表示是手机呼叫
                {

                        m_is_phone_to_transmit = 1;
                        m_CurrentSocketIndex = sock_index;
                        WRT_DEBUG("手机呼叫中心...");
                        m_callipaddr[0] = pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
                        m_callipaddr[1] = pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
                        m_callipaddr[2] = 0;
                        m_callipaddr[3] = 0;
                }
#endif
                // CallParam_t callparam;

                memset(&callparam,0,sizeof(callparam));
                for(i = 0 ;i < 4;i++){
                        if(m_callipaddr[i]!= 0 && m_callipaddr[i] != 0xffffffff){
                                callparam[num].ip = m_callipaddr[i];
                                num++;
                        }
                }
                if(num != 0){
                        WRT_DEBUG("房间呼叫中心....num%d",num);//wrti_devicecaller  wrti_devicecallcenter
                        wrti_devicecaller(callparam,num,htons(ROOM_CALLCENTER));
                        m_SysStatus = SYS_ACTIVE_CALL;
                }
        }else{
#ifdef HAVE_SUB_PHONE
                if(sock_index != -1)
                {
                        SendCmd(sock_index,CENTER_BUSY);
                        if(m_pSocketManager)
                                m_pSocketManager->CloseSocket(sock_index);
                        sock_index = -1;
                        return;
                }
#endif
                m_displayevent[0]=STATUS_ROOM_BUSY;
                m_displayevent[1] = 1;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
        }
}

void CCmdHandler::center_ack_idle(int sock_index)
{
	WRT_MESSAGE("中心应答空闲");
        if(SYS_ROOMCALLCENTERTALK == m_SysStatus){
                //m_CurrentSocketIndex = sock_index;
                if(g_smalldoor_call_agent == 1){ //如果小门口机呼叫，分机托管
                        add_and_del_timer(TIME_RING_EVENT_START);//60s 没有人接听，则挂机
                        return;
                }

                if(m_ringtimerid != 0){
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                if(m_mp3timerid != 0){
                        add_and_del_timer(TIME_MP3_EVENT_STOP);
                }
                leavestop();
                mp3stop();
                mp3play((char*)g_ringset[CENTER_CALL_RING].ring_path,0,3);
                add_and_del_timer(TIME_RING_EVENT_START);//60s 没有人接听，则挂机
        }
}

void CCmdHandler::room_door_center_ack_busy(int sock_index,unsigned char* buf,int buflen){
        if(SYS_ROOMCALLCENTERTALK == m_SysStatus || SYS_TALKTOMAIN  == m_SysStatus
                /*||SYS_MYROOMTALKROOM == m_SysStatus*/ ){
                        WRT_MESSAGE("主机或者中心繁忙%d",g_smalldoor_call_agent);
                        if(m_pSocketManager){
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                                m_CurrentSocketIndex = -1;
                        }
                        unsigned long tmpip = *(unsigned long*)buf;
                        if(tmpip == 0){
                                if(m_SysStatus == SYS_TALKTOMAIN || SYS_MONT == m_SysStatus )
                                        m_displayevent[0]=STATUS_DOOR_BUSY; //通知GUI门口机繁忙
                                if(m_SysStatus == SYS_ROOMCALLCENTERTALK)
                                        m_displayevent[0]=STATUS_CENTER_BUSY;//通知GUI 中心繁忙
                                /* if(m_SysStatus == SYS_MYROOMTALKROOM)
                                m_displayevent[0]=STATUS_ROOM_BUSY;//通知GUI 分机繁忙          */
                                m_displayevent[1] = 0;
                                m_displayevent[2]=m_displayevent[3]= 0;
                                m_display(m_displayevent);
                                m_SysStatus = SYS_IDLE;
                                return;
                        }
                        if(SYS_ROOMCALLCENTERTALK == m_SysStatus){
                                //重新连接新的IP，状态不变
                                if(m_dirvertcount > 3){ //如果转移的次数太多，则挂机
                                        door_and_center_hangup(-1);
                                        m_dirvertcount = 0;
                                        return;
                                }

                                unsigned long tmpmsg[4];
                                int packlen=0;
                                tmpmsg[0] = tmpip;

                                tmpmsg[1] = (unsigned long)PackCmd(ROOM_CALLCENTER,0,0,&packlen);
                                tmpmsg[2] = packlen;
                                tmpmsg[3] = 0;
                                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmpmsg,sizeof(unsigned long)*4);
                                m_dirvertcount++;
                        }
        }
}

void CCmdHandler::room_door_center_ack_busy(int sock_index)
{
		//PauseAndPlayVideo(0);
		Stopplayvideo();
        if(SYS_ROOMCALLCENTERTALK == m_SysStatus || SYS_TALKTOMAIN  == m_SysStatus ||
                /* SYS_MYROOMTALKROOM == m_SysStatus ||*/ SYS_MONT == m_SysStatus ){
                        WRT_DEBUG("room_door_center_ack_busy=%d m_smalldoor_id = %d",g_smalldoor_call_agent,m_smalldoor_id);
                        if(m_pSocketManager){
                                m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                                m_CurrentSocketIndex = -1;
                        }
                        if(g_smalldoor_call_agent == 1){ //如果小门口机呼叫，分机托管

                                unsigned short tmpevent[4];
                                unsigned long tmpevent1[4] ={0};
                                /*
                                tmpevent[0] = ROOM_ACK_SMALLDOOR;
                                tmpevent[1] = 0x00;//繁忙
                                tmpevent[2] = m_smalldoor_id;
                                tmpevent[3] = 0x01; //表示有数据
                                send_smalldoor_cmd(tmpevent);
                                m_smalldoor_cmd = CENTER_BUSY;
                                printf("TIME_SMALLDOOR_EVENT_START \n");
                                add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);
                                */

                                tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                                tmpevent[1] = 0;
                                tmpevent[2] = m_smalldoor_id; //目标ID
                                tmpevent[3] = 0;
                                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                                //  m_smalldoor_cmd = CENTER_BUSY;
                                //  add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);

                                tmpevent1[0] = ROOM_USED_SIMULATE;
                                tmpevent1[1] = 0;
                                tmpevent1[2] = 0;
                                tmpevent1[3] = 4;
                                udp_send_data(tmpevent1);  //通知其他主、从分机,通道释放。

                                return;
                        }
                        if(m_SysStatus == SYS_TALKTOMAIN || SYS_MONT == m_SysStatus )
                                m_displayevent[0]=STATUS_DOOR_BUSY; //通知GUI门口机繁忙
                        if(m_SysStatus == SYS_ROOMCALLCENTERTALK)
                                m_displayevent[0]=STATUS_CENTER_BUSY;//通知GUI 中心繁忙
                        /*if(m_SysStatus == SYS_MYROOMTALKROOM)
                        m_displayevent[0]=STATUS_ROOM_BUSY;//通知GUI 分机繁忙          */
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                        m_SysStatus = SYS_IDLE;

        }
}

void CCmdHandler::center_holdon(int sock_index){
	WRT_MESSAGE("中心摘机");
        if(SYS_ROOMCALLCENTERTALK == m_SysStatus ){
        	g_holdonbyme = 1;
                if(g_smalldoor_call_agent == 1) //如果小门口机呼叫，分机托管
                {
                        if(m_ringtimerid != 0){ //取消震铃的计时器
                                add_and_del_timer(TIME_RING_EVENT_STOP);
                        }

                        if(m_istalkandsmalldoor == 1)
                        {
                                unsigned long tmpevent[4];
                                tmpevent[0] = tmpevent[1] = tmpevent[2]  = tmpevent[3] = 0;
                                smalldoor_ack_holdon(tmpevent);	
                                m_istalkandsmalldoor = 0;
                                return;

                        }
                        if(m_isdivert == 0){
                                unsigned short tmpevent[4];
                                tmpevent[0] = ROOM_HOLDON_SMALLDOOR;
                                tmpevent[1] = 0;
                                tmpevent[2] = m_smalldoor_id; //目标ID
                                tmpevent[3] = 0;
                                send_smalldoor_cmd(tmpevent); //通知小门口机，分级摘机
                                //  m_smalldoor_cmd = CENTER_HOLDON;

                                //启动一个计时器。
                                //  add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);

                        }else{
                                unsigned long tmpevent[4];
                                tmpevent[0] = tmpevent[1] = tmpevent[2]  = tmpevent[3] = 0;
                                smalldoor_ack_holdon(tmpevent);
                                m_isdivert = 0;
                                WRT_DEBUG("托管摘机");
                        }
                        return;
                }
                mp3stop();
                if(m_ringtimerid != 0){ //取消震铃的计时器
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                //通知GUI 中心摘机
                m_displayevent[0] = STATUS_CENTER_HOLDON;
                m_displayevent[1] = m_displayevent[2] = m_displayevent[3] =0;
                m_display(m_displayevent);
                talktostop();
                talkstop();
                if(m_talktimerid != 0){
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
                }

                unsigned long tmpip = m_pSocketManager->GetIPAddrByIndex(sock_index);
                //SetSendIPAndPort(tmpip,15004);
                talkstart();
                talktostart(tmpip,15004);
                add_and_del_timer(TIME_TALK_EVENT_START);
        }
}

unsigned char*  CCmdHandler::PacketMontCmd(int index,int * packetlen)
{
        short cmd = ROOM_STARTMONT ;
        int cmdlen = 40;
        char tmpid[16] = {0};
        unsigned char* packbuf = (unsigned char*)ZENMALLOC(cmdlen);
        if(packbuf == NULL){
                *packetlen = 0;
                return NULL;
        }
        cmd =htons(cmd);
        memset(packbuf,0,cmdlen);
        strncpy((char*)packbuf,"WRTI",4);
        memcpy(packbuf+4,&cmdlen,4);
        memcpy(packbuf+8,&cmd,2);

        memcpy(packbuf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        memset(tmpid,0,16);
        sprintf(tmpid,"%015d",index);
        memcpy(packbuf+25,tmpid,15);
        // memcpy(packbuf+25,"020020100000001",15);//2009-9-3 01:47下午
        *packetlen = cmdlen;    
        return packbuf;

}

void CCmdHandler::room_start_mont_ipc(int sock_index,unsigned long event[4]){
	Reset_negotiate_result();
	if(SYS_IDLE == m_SysStatus )
	{
		if(m_watchtimerid != 0){
	        add_and_del_timer(TIME_MONT2_EVENT_STOP);
        }
        Startplayvideo(NULL);
        add_and_del_timer(TIME_MONT2_EVENT_START);
		m_SysStatus = SYS_MONT;
	}else
	{
		m_displayevent[0] = STATUS_ROOM_BUSY;
        m_displayevent[1] = 1;
        m_displayevent[2] = m_displayevent[3]= 0;
        m_display(m_displayevent);
	}
}

void CCmdHandler::room_stop_mont_ipc(int sock_index)
{
	
	if(SYS_MONT == m_SysStatus){
		int isexit = 0;
		if(m_watchtimerid != 0){
			add_and_del_timer(TIME_MONT2_EVENT_STOP);
		}
		m_displayevent[0] = STATUS_ROOM_STOPMONT;
		m_displayevent[1] = isexit;
		m_displayevent[2] = m_displayevent[3]= 0;
		Stopplayvideo();
		m_display(m_displayevent);
		m_SysStatus = SYS_IDLE;
		return;
	}
	m_displayevent[0] = STATUS_ROOM_STOPMONT;
	m_displayevent[1] = 0;
	m_displayevent[2]=m_displayevent[3]= 0;
	m_display(m_displayevent);
}
/*启动监视*/
void CCmdHandler::room_start_mont(int sock_index,unsigned long event[4]){

	WRT_MESSAGE("分机启动监视");
	Reset_negotiate_result();
	//printf("分机启动监视\n");
	

        if(event[1] != MONITOR_TYPE_SMALLDOOR){
        	//	printf("======jianshi=====\n");
                WRT_DEBUG("m_SysStatus = %d ",m_SysStatus);
                if(SYS_IDLE == m_SysStatus ){
#ifdef HAVE_SUB_PHONE
                        if(sock_index != -1){
                                if(g_phone_connect_index != -1)
                                {
                                        if(m_pSocketManager)
                                                m_pSocketManager->CloseSocket(g_phone_connect_index);
                                        g_phone_connect_index = -1;
                                }
                                g_phone_connect_index = sock_index;
                        }
#endif
                        unsigned long tmppack[4];
                        int packlen=0;
                        memset(tmppack,0,sizeof(unsigned long)*4);
                        tmppack[0] = m_Montipaddr;
#if USE_CENTERMONT
                        tmppack[1] = (unsigned long)PackCmd(/*ROOM_STARTMONT*/CENTER_STARTMONT,0,0,&packlen);//2009-9-3 01:47下午
#else
                        tmppack[1] = (unsigned long)PacketMontCmd(event[2],&packlen);//2009-9-3 01:47下午
#endif
                        tmppack[2] = packlen;
                        tmppack[3] = 0;

                        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                        m_SysStatus = SYS_MONT;

                }else{
#ifdef HAVE_SUB_PHONE
                        if(sock_index != -1)
                        {
                                SendCmd(sock_index,DOOR_BUSY);
                                if(m_pSocketManager)
                                        m_pSocketManager->CloseSocket(sock_index);
                                g_phone_connect_index = -1;
                                return;
                        }
#endif

                        m_displayevent[0]=STATUS_ROOM_BUSY;
                        m_displayevent[1] = 1;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }
        }else{

                if(SYS_IDLE == m_SysStatus || m_SysStatus == SYS_MONTSMALLDOOR){
#ifdef HAVE_SUB_PHONE
/*
                        if(sock_index != -1){
                                if(g_phone_connect_index != -1)
                                {
                                        if(m_pSocketManager)
                                                m_pSocketManager->CloseSocket(g_phone_connect_index);
                                        g_phone_connect_index = -1;
                                }
                                g_phone_connect_index = sock_index;
                        }
*/                        
#endif
                        unsigned short tmpbuf[4];
                        tmpbuf[0] = ROOM_START_MONT_SMALLDOOR;
                        tmpbuf[1] = 0;
                        tmpbuf[2] = m_Montipaddr; //小门口的ID 0x11
                        tmpbuf[3] = 0;
                        m_smalldoor_id = m_Montipaddr; //保存的是目标ID 默认的是0x11
                        send_smalldoor_cmd(tmpbuf);//通知小门口机开始监视
                        m_SysStatus = SYS_MONTSMALLDOOR;
                        //  m_smalldoor_cmd = ROOM_STARTMONT;
                        //  add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);



                }else{
#ifdef HAVE_SUB_PHONE
                        if(sock_index != -1)
                        {
                                SendCmd(sock_index,DOOR_BUSY);
                                if(m_pSocketManager)
                                        m_pSocketManager->CloseSocket(sock_index);
                                return;
                        }
#endif
                        //通知GUI 停止监视
                        m_displayevent[0] = STATUS_ROOM_STOPMONT;
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }
        }
        //}
}

void CCmdHandler::door_stop_mont(int sock_index){
	WRT_DEBUG("门口机停止监视");
        if(m_SysStatus == SYS_MONT){
        	int isexit = 0;
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                if(m_watchtimerid != 0){
                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                }
#ifdef HAVE_SUB_PHONE
                if(g_phone_connect_index != -1)
                {
                        stop_phone_video2(-1);
                        WRT_DEBUG("门口机停止监视g_phone_connect_index = %d DOOR_HANGUP",g_phone_connect_index);
                        SendCmd(g_phone_connect_index,DOOR_HANGUP);
                        if(m_pSocketManager)
                                m_pSocketManager->CloseSocket(g_phone_connect_index);
                        g_phone_connect_index = -1;
                        isexit = 1;

                }
#endif
                Stopplayvideo();

                //通知GUI 停止监视
                m_displayevent[0] = STATUS_ROOM_STOPMONT;
                m_displayevent[1] = isexit;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
                m_SysStatus = SYS_IDLE;
                m_CurrentSocketIndex = -1;

        }
}

void CCmdHandler::door_ack_montack(int sock_index)
{
		WRT_MESSAGE("门口机应答监视");
        if(SYS_MONT == m_SysStatus){
#if USE_CENTERMONT
                talktostop();
                talkstop();
                unsigned long tmpip = m_pSocketManager->GetIPAddrByIndex(sock_index);
#endif
                //开始计时30s
                if(m_watchtimerid != 0){
                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                }
                
                Startplayvideo(NULL);
                if(g_phone_mont != 1)
                	SendCmd2(sock_index,MEDIA_NEGOTIATE,NULL,0);

#ifdef HAVE_SUB_PHONE
                if(g_phone_connect_index != -1)
                {
                        SendCmd(g_phone_connect_index,DOOR_MONTACK);
                        WRT_DEBUG("DOOR_MONTACK %d 0x%x",g_phone_connect_index,m_pSocketManager->GetIPAddrByIndex(g_phone_connect_index));
                        start_phone_video2(m_pSocketManager->GetIPAddrByIndex(g_phone_connect_index),0);

                }
#endif

#if USE_CENTERMONT
                talkstart();
                talktostart(tmpip,15004);
#endif
				add_and_del_timer(TIME_MONT_EVENT_START);//30s stop MONT
        }
}

//[2011-5-17 13:50:40]
void CCmdHandler::SendColorMsg(unsigned long event[4])
{
	WRT_MESSAGE("通知中心发送彩信到注册手机");
	unsigned long tmpevent[4] = {0};
	tmpevent[0] = ROOM_SEND_COLOR_MSG;
	send_control_cmd_to_center(tmpevent);
	/*
        unsigned char* tmpbuf = 0;
        int packlen=0;
        tmpbuf = (unsigned char*)PackCmd(ROOM_SEND_COLOR_MSG,0,0,&packlen);
        if(0 != tmpbuf){
                m_pSocketManager->SendResultToCenter(m_centeripaddr,tmpbuf,packlen);
                ZENFREE(tmpbuf);
        }
        */		
}

void CCmdHandler::room_call_lift(unsigned long event[4]){
	WRT_MESSAGE("分机呼叫电梯");
        unsigned char* tmpbuf = 0;
        int packlen=0;
        tmpbuf = (unsigned char*)PackCmd(ROOM_CALL_LIFT,0,0,&packlen);
        if(0 != tmpbuf){
                m_pSocketManager->SendResultToCenter(event[1],tmpbuf,packlen);
                ZENFREE(tmpbuf);
        }
        /*
        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
        m_SysStatus = SYS_CALL_LIFT;
        */
}
void CCmdHandler::room_gettime(int sock_index){
        //	if(m_SysStatus == SYS_IDLE){
		WRT_MESSAGE("分机获取时间,系统状态0x%x",m_SysStatus);
		if(m_centeripaddr == 0 || m_centeripaddr == 0xffffffff){printf("centerIP NULL\r\n");return;}
			
		//if(m_SysStatus == SYS_GETTIME){         	
		//	return;
		//}
		unsigned long tmppack[4];
		int packlen=0;
		memset(tmppack,0,sizeof(unsigned long)*4);
		tmppack[0] = m_centeripaddr;
		tmppack[1] = (unsigned long)PackCmd(ROOM_GETTIME,0,0,&packlen);
		tmppack[2] = packlen;
		tmppack[3] = 0;
printf("room get time from center\r\n");
		m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
		//m_SysStatus = SYS_GETTIME;
        //	}
}

void CCmdHandler::room_stop_leave(int sock_index){
}
void CCmdHandler::room_stop_mont_byKey(int sock_index)
{
	if(m_CurrentSocketIndex >= 0)
	{
		printf("======================================\n");
		printf("close socket index %d\n",m_CurrentSocketIndex);
		printf("======================================\n");
		SendCmd(m_CurrentSocketIndex,ROOM_STOPMONT);
		SDL_Delay(100);
		if(m_pSocketManager)
            m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
        if(m_watchtimerid != 0){
            add_and_del_timer(TIME_MONT_EVENT_STOP);
        }
        Stopplayvideo();
        m_SysStatus = SYS_IDLE;
        m_CurrentSocketIndex = -1;
	}
}
/*停止监视*/
void CCmdHandler::room_stop_mont(int sock_index)
{

#if 0

#ifdef DEBUG_TEST_VIDEO_2
        PausePlayVideo(0);//关闭视频
        //SDL_Delay(20*1000);
        m_displayevent[0] = STATUS_ROOM_STOPMONT;
        m_displayevent[1] = 0;
        m_displayevent[2]=m_displayevent[3]= 0;
        m_display(m_displayevent);
        talkstop();
        talktostop();
        m_SysStatus = SYS_IDLE;
         
        return;
#endif

#if 0
        if(m_CurrentSocketIndex == -1)
                return;
#endif

#endif
        if(SYS_MONT == m_SysStatus){
				int isexit = 0;
#if USE_CENTERMONT
                talktostop();
                talkstop();
                SendCmd(m_CurrentSocketIndex,/*ROOM_STOPMONT*/CENTER_STOPMONT);//2009-9-3 01:48下午
#else
                SendCmd(m_CurrentSocketIndex,ROOM_STOPMONT);//2009-9-3 01:48下午
#endif

#ifdef HAVE_SUB_PHONE
                if( g_phone_connect_index != -1)
                {
                        stop_phone_video2(-1);
                        if(sock_index == -1)
                        {
                                printf("g_phone_connect_index = %d send DOOR_HANGUP\n",g_phone_connect_index);
                                SendCmd(g_phone_connect_index,DOOR_ENDMONT);

                        }
                        if(m_pSocketManager)
                                m_pSocketManager->CloseSocket(g_phone_connect_index);
                        g_phone_connect_index = -1;
                        isexit = 1;
                }
#endif

                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                if(m_watchtimerid != 0){
                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                }
                

                //通知GUI 停止监视
                m_displayevent[0] = STATUS_ROOM_STOPMONT;
                m_displayevent[1] = isexit;
                m_displayevent[2]=m_displayevent[3]= 0;

                Stopplayvideo();
                m_display(m_displayevent);
                m_SysStatus = SYS_IDLE;
                m_CurrentSocketIndex = -1;

                return;

        }
        if(SYS_MONTSMALLDOOR == m_SysStatus){
                unsigned short tmpevent[4];
                add_and_del_timer(TIME_MONT_EVENT_STOP);
                tmpevent[0] = ROOM_STOP_MONT_SMALLDOOR;
                tmpevent[1] = 0;
                tmpevent[2] = m_smalldoor_id; //目标ID(可能需要保存目标ID？？？)
                tmpevent[3] = 0;
                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                // m_smalldoor_cmd = ROOM_STOPMONT;
                // add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);
                if(g_phone_connect_index != -1)
                {
                        stop_smalldoor_video();
                }

                return;
        }
        m_displayevent[0] = STATUS_ROOM_STOPMONT;
        m_displayevent[1] = 0;
        m_displayevent[2]=m_displayevent[3]= 0;
        m_display(m_displayevent);

}

void CCmdHandler::room_ack_media_negotiate(int sock_index,unsigned char* buf,unsigned long buflen)
{
	if(sock_index != -1)
	{
		printf("回复媒体协商\n");
		media_negotiate_process((char *)buf,buflen);
		
		if(m_isdoor != TYPE_ROOM_CALLME)//分机是收到摘机再发送协商结果至对方
		{
			if(g_localid >= 0x05 && g_localid <= 0x08)
				printf("xxxxxx   Sub room \n");
			else
			{
                NEGOTIATE_RESULT stNegotiateResult;
                stNegotiateResult.v_enc_type = get_vdec_type();
                stNegotiateResult.v_enc_resolution_w = get_vdec_width();
                stNegotiateResult.v_enc_resolution_h = get_vdec_height();
                printf("xxxxxx   Main room w %d h %d\n", get_vdec_width(), get_vdec_height());
                SendCmd2(sock_index, MEDIA_NEGOTIATE_ACK, (char *)&stNegotiateResult, sizeof(NEGOTIATE_RESULT));
			}
			//2012-12-5 14:28:52 by ljw 此部分是发送主机呼叫后主分机的协商结果
			//由于主机不可能转发多种流，所以从分机必须支持的视频格式和主分机一样
			if(g_localid == 0x01){
				int i = 0;
				for(i = 0; i<MAX_SUBROOM;i++){
					unsigned long tmpsubroomip = get_subroom_ip_by_index(i); 
					 if(tmpsubroomip != 0 && tmpsubroomip != 0xffffffff)
							break;
				}
				if(i != MAX_SUBROOM){
					unsigned long msg_buf[4];
					char* pPeerID  = (char*)ZENMALLOC(16);
					if(pPeerID){
						memset(pPeerID,0,16);
						memcpy(pPeerID,m_UserPhoneNumber,15);
						msg_buf[0] = MEDIA_NEGOTIATE_SUBROOM;
						msg_buf[1] = get_venc_type();
						msg_buf[2] = get_vdec_type();
						msg_buf[3] = (unsigned long)pPeerID;
						send_msg_to_subroom(msg_buf);
					}
				}
			}
		}
#ifdef HAVE_SUB_PHONE
		if(have_connect_phone() != 0 && m_is_transmit_to_phone == 1){
            unsigned long msgbuf[4] ={0};
            msgbuf[0] = PHONE_MEDIA_INFO;//发送媒体信息至手机
            if(get_aenc_type() >= A_PCM || get_adec_type() >= A_PCM)
            	msgbuf[1] = PHONE_1CHNL_8K_16BIT;//
            else
            	msgbuf[1] = PHONE_2CHNL_16K_16BIT;//
            if(get_vdec_type() >= V_H264)
            	msgbuf[2] = PHONE_H264;
            else
                msgbuf[2] = PHONE_MP4;
            send_msg_to_sub_phone(msgbuf);
        }
#endif
	}
}

void CCmdHandler::room_call_room2(int sock_index,unsigned char* buf,unsigned long buflen)
{
	WRT_MESSAGE("分机呼叫并协商 %d",buflen);
    if(SYS_IDLE == m_SysStatus || SYS_MONT == m_SysStatus)
    {
            if(sock_index == -1) //表示是我主动呼叫别的分机
            {
                    int i;
                    int count=0;

                    memset(callparam,0,sizeof(callparam));
                    for(i =0; i< 4;i++){
                            if( m_callipaddr[i] != 0 && m_callipaddr[i]!= 0xffffffff){
                            		//pSystemInfo->LocalSetting.publicinfo.
                      			    if(pSystemInfo->LocalSetting.privateinfo.LocalIP != m_callipaddr[i])
                            		{
                                        callparam[count].ip  = m_callipaddr[i];
                                        count++;
                                    }
                            }
                    }
                    if(count != 0){
                    		WRT_DEBUG("调用呼叫库呼叫");
                            wrti_devicecaller(callparam,count,htons(ROOM_CALLROOM));
                            m_SysStatus = SYS_ACTIVE_CALL;
                    }else{
                    		WRT_DEBUG("没有呼叫对象");
                            m_displayevent[0] = STATUS_NET_EXCEPTION;
                            m_displayevent[1] = 0;
                            m_displayevent[2]=m_displayevent[3]= 0;
                            m_display(m_displayevent);
                    }

            }else{//别人呼叫我。
                    if(pSystemInfo->mySysInfo.isbomb == USE_BOMB)
                    {
                            SendCmd(sock_index,ROOM_BUSY);
                            m_pSocketManager->CloseSocket(sock_index);
                            return;
                    }


                    int isexit = 0;
                    char* temp_buf =0;

                    if(m_SysStatus == SYS_MONT){
#ifdef HAVE_SUB_PHONE
                            if(g_phone_connect_index != -1)
                            {
                                    stop_phone_video2(-1);
                                    WRT_DEBUG("g_phone_connect_index = %d DOOR_ENDMONT\n",g_phone_connect_index);
                                    SendCmd(g_phone_connect_index,DOOR_ENDMONT);
                                    if(m_pSocketManager)
                                            m_pSocketManager->CloseSocket(g_phone_connect_index);
                                    g_phone_connect_index = -1;

                            }
#endif                          	
                            if(m_watchtimerid != 0){ //取消监视计时器
                                    add_and_del_timer(TIME_MONT_EVENT_STOP);
                                    if(m_CurrentSocketIndex != -1)
                                            SendCmd(m_CurrentSocketIndex,ROOM_STOPMONT);
                            }

                            Stopplayvideo();
                            WRT_DEBUG("关闭监视");
                            isexit = 1;
                            if(m_CurrentSocketIndex != -1)
                                    m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                            m_CurrentSocketIndex = -1;
                            m_displayevent[0]=STATUS_ROOM_STOPMONT;
                            m_displayevent[1] = 1;
                            m_displayevent[2]=m_displayevent[3]= 0;
                            m_display(m_displayevent);

                    }
#ifdef HAVE_SUB_PHONE
                    if(m_is_phone_call_room == 1)
                    {
                            int i;
                            int count=0;

                            memset(callparam,0,sizeof(callparam));
                            for(i =0; i< 4;i++){
                                    if( m_callipaddr[i] != 0 && m_callipaddr[i]!= 0xffffffff){
                                            callparam[count].ip  = m_callipaddr[i];
                                            count++;
                                    }
                            }
                            if(count != 0){
                                    m_CurrentSocketIndex = sock_index;
                                    wrti_devicecaller(callparam,count,htons(ROOM_CALLROOM));
                                    m_SysStatus = SYS_ACTIVE_CALL;
                            }else{
                                    m_displayevent[0] = STATUS_NET_EXCEPTION;
                                    m_displayevent[1] = 0;
                                    m_displayevent[2]=m_displayevent[3]= 0;
                                    m_display(m_displayevent);
                            }

                            return;
                    }
#endif
                    if(pSystemInfo->mySysInfo.isagent  == 1){ //如果设置了托管
                            SendCmd(sock_index,ROOM_AGENT);
                            m_pSocketManager->CloseSocket(sock_index);
                            return;
                    }

                    if(pSystemInfo->mySysInfo.isavoid == 1){ //如果设置了免打扰
                            SendCmd(sock_index,ROOM_QUIET);
                            m_pSocketManager->CloseSocket(sock_index);
                            return;
                    }
                    SendCmd2(sock_index,ROOM_IDLE,(char *)buf,buflen);
                    m_CurrentSocketIndex =sock_index;
                    m_SysStatus = SYS_ROOMTALKROOM;


                    temp_buf =( char*)ZENMALLOC(16);
                    if(temp_buf){
                            memset(temp_buf,0,16);
                            memcpy(temp_buf,buf+10,15);
                    }

                    m_displayevent[0] = STATUS_ROOMTALKROOM;
                    m_displayevent[1] = isexit;
                    m_displayevent[2]= (unsigned long)temp_buf;
                    m_displayevent[3]= 0;
                    m_display(m_displayevent);

                    if((pSystemInfo->mySysInfo.isnvo == 1) && (g_localid == 0x01) && (g_used_simulate_channel == 0)){ //启动挂接模拟分机
                            unsigned short tmpe[4];
                            tmpe[0] = CALL_SIMULATE;
                            tmpe[1] = 0x01; //数据,网络呼叫
                            tmpe[2] = 0x0; //目的ID
                            tmpe[3] = 0x01; //表示是否有数据，0，没带数据，1，带有数据
                            send_smalldoor_cmd(tmpe);
                            // SDL_Delay(350);

                            send_smalldoor_cmd(tmpe);
                    }

                    if(m_ringtimerid != 0){
                            add_and_del_timer(TIME_RING_EVENT_STOP);
                    }
                    if(m_mp3timerid != 0){
                            add_and_del_timer(TIME_MP3_EVENT_STOP);
                    }
                    leavestop();
                    mp3stop();
                    mp3play((char*)g_ringset[ROOM_CALL_RING].ring_path,0,3);
                    add_and_del_timer(TIME_RING_EVENT_START);
            }
    }else {
            if(sock_index != -1){
                    SendCmd(sock_index,ROOM_BUSY);
                    m_pSocketManager->CloseSocket(sock_index);
            }else{
                    m_displayevent[0]=STATUS_ROOM_BUSY;
                    m_displayevent[1] = 1;
                    m_displayevent[2]=m_displayevent[3]= 0;
                    m_display(m_displayevent);
            }
    }
}

void CCmdHandler::divert_room_to_room_deal()
{
	m_displayevent[0] = STATUS_DIVERT_ROOM_TO_ROOM;
	m_displayevent[1] = 1;
	m_displayevent[2] = m_displayevent[3]= 0;
	m_display(m_displayevent);
	m_SysStatus = SYS_ACTIVE_CALL;
}

void CCmdHandler::divert_room_to_center_deal()
{
	m_displayevent[0]=STATUS_DIVERT_ROOM_TO_CENTER;
	m_displayevent[1] = 1;
	m_displayevent[2]=m_displayevent[3]= 0;
	m_display(m_displayevent);
	m_SysStatus = SYS_ACTIVE_CALL;
}

/*户户通*/
void CCmdHandler::room_call_room(int sock_index,unsigned char* buf,unsigned long buflen)
{
		Reset_negotiate_result();
	
        if(SYS_IDLE == m_SysStatus || SYS_MONT == m_SysStatus)
        {
        		m_isdoor = TYPE_ROOM_CALLME;
                if(sock_index == -1) //表示是我主动呼叫别的分机
                {

                        int i;
                        int count=0;
						WRT_MESSAGE("分机呼叫分机-主叫");
                        memset(callparam,0,sizeof(callparam));
                        for(i =0; i< 4;i++){
                                if( m_callipaddr[i] != 0 && m_callipaddr[i]!= 0xffffffff){
                                		//pSystemInfo->LocalSetting.publicinfo.
                          			    if(pSystemInfo->LocalSetting.privateinfo.LocalIP != m_callipaddr[i])
                                		{
	                                        callparam[count].ip  = m_callipaddr[i];
	                                        count++;
                                        }
                                }
                        }
                        if(count != 0){
                        		WRT_DEBUG("调用呼叫库呼叫");
                                wrti_devicecaller(callparam,count,htons(ROOM_CALLROOM));
                                m_SysStatus = SYS_ACTIVE_CALL;
                        }else{
                        		WRT_DEBUG("没有呼叫对象");
                                m_displayevent[0] = STATUS_NET_EXCEPTION;
                                m_displayevent[1] = 0;
                                m_displayevent[2]=m_displayevent[3]= 0;
                                m_display(m_displayevent);
                        }

                }else{        //别人呼叫我。
                        if(pSystemInfo->mySysInfo.isbomb == USE_BOMB)
                        {
                                SendCmd(sock_index,ROOM_BUSY);
                                m_pSocketManager->CloseSocket(sock_index);
                                return;
                        }
						WRT_MESSAGE("分机呼叫分机-被叫");

                        int isexit = 0;
                        char* temp_buf =0;
						if(pSystemInfo->mySysInfo.isagent  == 1){ //如果设置了托管
                                SendCmd(sock_index,ROOM_AGENT);
                                m_pSocketManager->CloseSocket(sock_index);
                                return;
                        }

                        if(pSystemInfo->mySysInfo.isavoid == 1){ //如果设置了免打扰
                                SendCmd(sock_index,ROOM_QUIET);
                                m_pSocketManager->CloseSocket(sock_index);
                                return;
                        }
                        if(m_SysStatus == SYS_MONT){
#ifdef HAVE_SUB_PHONE
                                if(g_phone_connect_index != -1)
                                {
                                        stop_phone_video2(-1);
                                        WRT_DEBUG("g_phone_connect_index = %d DOOR_ENDMONT\n",g_phone_connect_index);
                                        SendCmd(g_phone_connect_index,DOOR_ENDMONT);
                                        if(m_pSocketManager)
                                                m_pSocketManager->CloseSocket(g_phone_connect_index);
                                        g_phone_connect_index = -1;

                                }
#endif                          	
                                if(m_watchtimerid != 0){ //取消监视计时器
                                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                                        if(m_CurrentSocketIndex != -1)
                                                SendCmd(m_CurrentSocketIndex,ROOM_STOPMONT);
                                }

                                Stopplayvideo();
                                WRT_DEBUG("CLOSE MONT VIDEO");
                                isexit = 1;
                                if(m_CurrentSocketIndex != -1)
                                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
                                m_CurrentSocketIndex = -1;
                                m_displayevent[0]=STATUS_ROOM_STOPMONT;
                                m_displayevent[1] = 1;
                                m_displayevent[2]=m_displayevent[3]= 0;
                                m_display(m_displayevent);

                        }
#ifdef HAVE_SUB_PHONE
                        if(m_is_phone_call_room == 1)
                        {
                                int i;
                                int count=0;

                                memset(callparam,0,sizeof(callparam));
                                for(i =0; i< 4;i++){
                                        if( m_callipaddr[i] != 0 && m_callipaddr[i]!= 0xffffffff){
                                                callparam[count].ip  = m_callipaddr[i];
                                                count++;
                                        }
                                }
                                if(count != 0){
                                        m_CurrentSocketIndex = sock_index;
                                        wrti_devicecaller(callparam,count,htons(ROOM_CALLROOM));
                                        m_SysStatus = SYS_ACTIVE_CALL;
                                }else{
                                        m_displayevent[0] = STATUS_NET_EXCEPTION;
                                        m_displayevent[1] = 0;
                                        m_displayevent[2]=m_displayevent[3]= 0;
                                        m_display(m_displayevent);
                                }

                                return;
                        }
#endif
                        

                        SendCmd(sock_index,ROOM_IDLE);
                        m_CurrentSocketIndex =sock_index;
                        m_SysStatus = SYS_ROOMTALKROOM;
						if(g_localid == 0x01){
							//如果是本地分机户户通的话
							if(strncmp((const char *)m_UserPhoneNumber,(const char *)pSystemInfo->LocalSetting.privateinfo.LocalID,13) != 0){
								int i = 0;
								for(i = 0; i<MAX_SUBROOM;i++){
									unsigned long tmpsubroomip = get_subroom_ip_by_index(i); 
									 if(tmpsubroomip != 0 && tmpsubroomip != 0xffffffff)
											break;
								}
								if(i != MAX_SUBROOM){
									unsigned long msg_buf[4];
									char* pPeerID  = (char*)ZENMALLOC(16);
									if(pPeerID){
										memset(pPeerID,0,16);
										memcpy(pPeerID,m_UserPhoneNumber,15);
										msg_buf[0] = OTHERROOM_CALL_SUBROOM;
										msg_buf[1] = 0;
										msg_buf[2] = 0;
										msg_buf[3] = (unsigned long)pPeerID;
										send_msg_to_subroom(msg_buf);
										m_is_divert_subroom = 1;
									}
								}
							}
						}

                        temp_buf =( char*)ZENMALLOC(buflen+1);
                        if(temp_buf){
                                memset(temp_buf,0,buflen+1);
                                memcpy(temp_buf,buf,buflen);
                        }
                        WRT_DEBUG("创建被叫界面");

                        m_displayevent[0] = STATUS_ROOMTALKROOM;
                        m_displayevent[1] = isexit;
                        m_displayevent[2] = (unsigned long)temp_buf;
                        m_displayevent[3] = 0;
                        m_display(m_displayevent);

                        if((pSystemInfo->mySysInfo.isnvo == 1) && (g_localid == 0x01) && (g_used_simulate_channel == 0)){ //启动挂接模拟分机
                                unsigned short tmpe[4];
                                tmpe[0] = CALL_SIMULATE;
                                tmpe[1] = 0x01; //数据,网络呼叫
                                tmpe[2] = 0x0; //目的ID
                                tmpe[3] = 0x01; //表示是否有数据，0，没带数据，1，带有数据
                                send_smalldoor_cmd(tmpe);
                                // SDL_Delay(350);

                                send_smalldoor_cmd(tmpe);
                        }

                        if(m_ringtimerid != 0){
                                add_and_del_timer(TIME_RING_EVENT_STOP);
                        }
                        if(m_mp3timerid != 0){
                                add_and_del_timer(TIME_MP3_EVENT_STOP);
                        }
                        leavestop();
                        mp3stop();
						SDL_Delay(100);
                        mp3play((char*)g_ringset[ROOM_CALL_RING].ring_path,0,3);
                        add_and_del_timer(TIME_RING_EVENT_START);
						SDL_Delay(100);//外挂分机响铃
						SetSubSpkStatus(1);
                }
        }else {
                if(sock_index != -1){
                        SendCmd(sock_index,ROOM_BUSY);
                        m_pSocketManager->CloseSocket(sock_index);
                }else{
                        m_displayevent[0]=STATUS_ROOM_BUSY;
                        m_displayevent[1] = 1;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                }
        }
}
/**
*房间应答托管
*主动户户通收到一个托管的应答时,将四路连接断开，向心的IP(中心IP)，发起另外一次中心呼叫
*状态变为ROOM_CALL_CENTER
*/

void CCmdHandler::room_ack_agent(int sock_index,unsigned char* buf,unsigned long buflen){
        /*	 if(SYS_MYROOMTALKROOM == m_SysStatus)
        {
        int i=0;
        for(i=0;i<4;i++){
        if(m_roomconnect[i].connectindex != 0xff)
        {
        m_pSocketManager->CloseSocket(m_roomconnect[i].connectindex);
        m_roomconnect[i].connectindex = 0xff;
        m_roomconnect[i].isackidle = 0;
        }
        }
        unsigned long newip = *(unsigned long*)buf;
        if(newip != 0)
        {
        unsigned long tmppack[4];
        int packlen=0;
        memset(tmppack,0,sizeof(unsigned long)*4);
        tmppack[0] = newip;
        tmppack[1] = (unsigned long)PackCmd(ROOM_CALLCENTER,0,0,&packlen);
        tmppack[2] = packlen;
        tmppack[3] = 0;
        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
        m_SysStatus = SYS_ROOMCALLCENTERTALK;
        }
        }
        */
}

/**
*房间回应免打扰
*/
void CCmdHandler::room_ack_quiet(int sock_index){
        /*      if(SYS_MYROOMTALKROOM == m_SysStatus){
        int i;
        for(i=0;i<4;i++){
        if(m_roomconnect[i].connectindex != 0xff){
        SendCmd(m_roomconnect[i].connectindex,ROOM_HANGUP);
        m_pSocketManager->CloseSocket(m_roomconnect[i].connectindex);
        m_roomconnect[i].connectindex = 0xff;
        m_roomconnect[i].isackidle = 0;
        }
        }
        m_SysStatus = SYS_IDLE;
        }
        */
}

/*回应空闲*/
void CCmdHandler::room_ack_idle(int sock_index)
{
        /*   if(SYS_MYROOMTALKROOM == m_SysStatus){
        if(sock_index != -1){
        // m_CurrentSocketIndex = sock_index;
        //只有等到所有的连上的连接恢复空闲后，才能播放铃声.
        int i=0,count1=0,count2=0;
        for(i=0;i<4;i++){
        if(m_roomconnect[i].connectindex == sock_index)
        m_roomconnect[i].isackidle = 1;
        if(m_roomconnect[i].connectindex != 0xff)
        count1++;
        if( m_roomconnect[i].isackidle ==1)
        count2++;
        }
        if(count1 == count2){
        mp3stop();
        if(m_ringtimerid != 0){
        add_and_del_timer(TIME_RING_EVENT_STOP);
        }
        mp3play((char*)g_ringset[ROOM_CALL_RING].ring_path, 0,1);
        add_and_del_timer(TIME_RING_EVENT_START);//30s 没人接 挂机
        }

        }
        }
        */
}



/*********************************************************************************
----------------------------------------------------------------------------------
*********************************************************************************/
void CCmdHandler::room_cancel_sync_agent(int sock_index)
{
        if(pSystemInfo /*&& g_localid != 0x01*/)
        {
                pSystemInfo->mySysInfo.isagent = 0;
                ReDrawCurrentWindow2(6);
        }
}
void CCmdHandler::room_cancel_sync_avoid(int sock_index)
{
        if(pSystemInfo/* && g_localid != 0x01*/)
        {
                pSystemInfo->mySysInfo.isavoid = 0;
                ReDrawCurrentWindow2(6);
        }
}

void CCmdHandler::room_sync_agent(int sock_index)
{
        if(pSystemInfo /*&& g_localid != 0x01*/)
        {
                pSystemInfo->mySysInfo.isagent = 1;
                ReDrawCurrentWindow2(6);
        }

}
void CCmdHandler::room_sync_avoid(int sock_index)
{
        if(pSystemInfo /*&& g_localid != 0x01*/)
        {
                pSystemInfo->mySysInfo.isavoid = 1;
                ReDrawCurrentWindow2(6);
        }

}

///////////////////////////////////////////////////////
//

void CCmdHandler::custom_cancel_cmd(unsigned long event[4]){
        int ret = 0;
        switch(event[1]){
        case ROOM_RINGDOWN:
                if(m_SysStatus == SYS_DOWNLOADRING)
                        ret = 1;
                break;
        case ROOM_BROWSERING:
                if(m_SysStatus == SYS_REQEUSTRINGLIST)
                        ret =1 ;
                break;
        case ROOM_INQHELPINFO:
                if(m_SysStatus == SYS_REQHELPINFO)
                        ret = 1;
                break;
        case ROOM_INQFEE:
                if(m_SysStatus == SYS_REQUESTFEE)
                        ret = 1;            
                break;
        case ROOM_INQSERVICEINFO:
                if(m_SysStatus == SYS_REQDOCINFO)
                        ret = 1;             
                break;
        case ROOM_REPAIR:
                if(m_SysStatus == SYS_REQUESTREPAIR)
                        ret = 1;         
                break;
        }
        if(ret){
                if(m_CurrentSocketIndex != -1 && m_pSocketManager){
                        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
#if USE_PACKELIST
                        DelPackList(m_CurrentSocketIndex);
#endif
                        m_CurrentSocketIndex  = -1;
                }
                WRT_DEBUG("cmd = CUSTOM_CMD_CANCEL");
                m_SysStatus = SYS_IDLE;        
        }
}


void CCmdHandler::room_services_request(unsigned long event[4]){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_SERVICES_REQUEST,0,0,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_REQDOCINFO;
                WRT_DEBUG("room_services_request");
        }    
}
void CCmdHandler::room_reqhelpinfo(int sock_index){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_INQHELPINFO,0,0,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_REQHELPINFO;
                WRT_DEBUG("room_reqhelpinfo");
        }
}

void CCmdHandler::room_reqdocinfo(int sock_index){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_INQSERVICEINFO,0,0,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                //FreeDocInfo(); //删除原来的便民信息
                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_REQDOCINFO;
        }
}
/*警告*/
void CCmdHandler::room_waring(int sock_index,unsigned char id){

}

/*紧急求助*/
void CCmdHandler::room_emerg(int sock_index){

}
/*费用查询*/
void CCmdHandler::room_queryfee(int sock_index,int id){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                unsigned char tmpid = (unsigned char)id;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_INQFEE,&tmpid,1,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_REQUESTFEE;
        }
}

/*维修*/
void CCmdHandler::room_repair(int sock_index,int id){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                unsigned char tmpid = (unsigned char)id;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_REPAIR,&tmpid,1,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_REQUESTREPAIR;
        }
}

/*铃声浏览*/
void CCmdHandler::room_browsering(int sock_index){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_BROWSERING,0,0,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_REQEUSTRINGLIST;
        }
}

/*铃声下载*/
void CCmdHandler::room_ringdownload(int sock_index,int id){
        if(m_SysStatus == SYS_IDLE){
                unsigned long tmppack[4];
                int packlen=0;
                memset(tmppack,0,sizeof(unsigned long)*4);

                tmppack[0] = m_centeripaddr;
                tmppack[1] = (unsigned long)PackCmd(ROOM_RINGDOWN,(unsigned char*)m_downname,40,&packlen);
                tmppack[2] = packlen;
                tmppack[3] = 0;

                m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                m_SysStatus = SYS_DOWNLOADRING;

                m_displayevent[0] = STATUS_RING_DOWNLOAD;
                m_displayevent[1] = 0;
                m_displayevent[2] = m_displayevent[3] = -1;
                m_display(m_displayevent);
        }
}

/**
*房间主动请求系统信息
*/
void CCmdHandler::room_ask_for_sysinfo(unsigned long event[4])
{
        unsigned long tmppack[4];
        int packlen=0;
        memset(tmppack,0,sizeof(unsigned long)*4);
        tmppack[0] = m_centeripaddr;
        tmppack[1] = (unsigned long)PackCmd(ROOM_ASKFOR_SYSINFO,0,0,&packlen);
        tmppack[2] = packlen;
        tmppack[3] = 0;

        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
        m_SysStatus = SYS_GETSYSINFO;

}
#pragma pack(1)
typedef struct{
        char SN[32];
        char Version[10];
        unsigned char MAC[6];
        unsigned long program_position;
        unsigned long program_size;
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
        unsigned long CenterMajorIP;
        unsigned long CenterMinorIP;
        unsigned long ManagerMajorIP;
        unsigned long ManagerMinorIP;
        unsigned long CenterWarningIP;
        unsigned long CenterMenjinIP;
        unsigned long CenterFileIP;
        unsigned long CenterNetIP;
}T_DEV_INFO;

#pragma pack()

void CCmdHandler::RoomAckSysinfo(int sock_index){
        if(m_pSocketManager){
                SOCKET_PACKET packet;
                T_DEV_INFO tmpdevinfo;
#ifdef USED_NEW_CMD
                int templen =40+sizeof(T_DEV_INFO);
#else
                int templen =10+sizeof(T_DEV_INFO);
#endif
                short cmd = htons(ROOM_GET_SYSINFO_ACK);
#ifdef USED_NEW_CMD
                int cmdlen = 40;
#else
                int cmdlen = 10;
#endif
                memset(&packet,-1,sizeof(SOCKET_PACKET));
                memset(packet.buf,0,256);
                strncpy((char*)packet.buf,"WRTI",4);
                memcpy(packet.buf+4,&templen,4);
                memcpy(packet.buf+8,&cmd,2);
                memset(&tmpdevinfo,0,sizeof(T_DEV_INFO));
#ifdef USED_NEW_CMD
                memcpy(packet.buf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                //memcpy(tmpdevinfo.SN,pSystemInfo->BootInfo.SN,32);
                memcpy(tmpdevinfo.SN,pSystemInfo->BootInfo.SN,12);
                memcpy(tmpdevinfo.SN+12,"-801D-CAPTS-1024x600",strlen("-801D-CAPTS-1024x600"));//modify by wyx 2013-7-30
                //tmpdevinfo.Version[0] = 'v';
                memcpy(tmpdevinfo.Version,pSystemInfo->BootInfo.Version,10);
                memcpy(tmpdevinfo.MAC,pSystemInfo->BootInfo.MAC,6);
                tmpdevinfo.program_position=pSystemInfo->BootInfo.program_position;
                tmpdevinfo.program_size=pSystemInfo->BootInfo.program_size;
                memcpy(tmpdevinfo.LocalID,pSystemInfo->LocalSetting.privateinfo.LocalID,16);
                tmpdevinfo.LocalIP=pSystemInfo->LocalSetting.privateinfo.LocalIP;
                tmpdevinfo.SubMaskIP      =pSystemInfo->LocalSetting.privateinfo.SubMaskIP;
                tmpdevinfo.GateWayIP      =pSystemInfo->LocalSetting.privateinfo.GateWayIP;
                tmpdevinfo.CenterMajorIP  =pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
                tmpdevinfo.CenterMinorIP  =pSystemInfo->LocalSetting.publicinfo.CenterMinorIP;
                tmpdevinfo.ManagerMajorIP=pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
                tmpdevinfo.ManagerMinorIP=pSystemInfo->LocalSetting.publicinfo.ManagerMinorIP;
                tmpdevinfo.CenterWarningIP=pSystemInfo->LocalSetting.publicinfo.CenterWarningIP;
                tmpdevinfo.CenterMenjinIP= pSystemInfo->LocalSetting.publicinfo.CenterMenjinIP;
                tmpdevinfo.CenterFileIP=pSystemInfo->LocalSetting.publicinfo.CenterFileIP;
                tmpdevinfo.CenterNetIP=pSystemInfo->LocalSetting.publicinfo.CenterNetIP;

                memcpy(packet.buf+40,(void*)&tmpdevinfo,sizeof(T_DEV_INFO));

#else
                memcpy(packet.buf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                //memcpy(tmpdevinfo.SN,pSystemInfo->BootInfo.SN,32);
                memcpy(tmpdevinfo.SN,pSystemInfo->BootInfo.SN,12);
                memcpy(tmpdevinfo.SN+12,"-801D-CAPTS-1024x600",strlen("-801D-CAPTS-1024x600"));//modify by wyx 2013-7-30
                memcpy(tmpdevinfo.Version,pSystemInfo->BootInfo.Version,10);
                memcpy(tmpdevinfo.MAC,pSystemInfo->BootInfo.MAC,6);
                tmpdevinfo.program_position=pSystemInfo->BootInfo.program_position;
                tmpdevinfo.program_size=pSystemInfo->BootInfo.program_size;
                memcpy(tmpdevinfo.LocalID,pSystemInfo->LocalSetting.privateinfo.LocalID,16);
                tmpdevinfo.LocalIP        =pSystemInfo->LocalSetting.privateinfo.LocalIP;
                tmpdevinfo.SubMaskIP      =pSystemInfo->LocalSetting.privateinfo.SubMaskIP;
                tmpdevinfo.GateWayIP      =pSystemInfo->LocalSetting.privateinfo.GateWayIP;
                tmpdevinfo.CenterMajorIP  =pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
                tmpdevinfo.CenterMinorIP  =pSystemInfo->LocalSetting.publicinfo.CenterMinorIP;
                tmpdevinfo.ManagerMajorIP=pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
                tmpdevinfo.ManagerMinorIP=pSystemInfo->LocalSetting.publicinfo.ManagerMinorIP;
                tmpdevinfo.CenterWarningIP=pSystemInfo->LocalSetting.publicinfo.CenterWarningIP;
                tmpdevinfo.CenterMenjinIP= pSystemInfo->LocalSetting.publicinfo.CenterMenjinIP;
                tmpdevinfo.CenterFileIP=pSystemInfo->LocalSetting.publicinfo.CenterFileIP;
                tmpdevinfo.CenterNetIP=pSystemInfo->LocalSetting.publicinfo.CenterNetIP;

                memcpy(packet.buf+10,(void*)&tmpdevinfo,sizeof(T_DEV_INFO));
#endif
                packet.sock_index = sock_index;
                packet.validlen = templen;
                m_pSocketManager->Send_msg(MSG_NODE_SEND_MSG,(void*)&packet,sizeof(SOCKET_PACKET));
        }
}
/******************************************************************************************/
/************************家居遥控器远程控制处理*****************************************/
/************************2009-7-30 9:04:24***********************************************/
/******************************************************************************************/
void CCmdHandler::riu_key(unsigned long event[4])
{

        unsigned char* p = (unsigned char*)event[1];
        unsigned long tmpevent[4];
        tmpevent[3] = 4;
        tmpevent[0] = ROOM_SEND_KEY;
        tmpevent[1] = 0;
        tmpevent[2] = *(unsigned char*)p;
        udp_send_data(tmpevent);
        ZENFREE((void*)p);
        WRT_DEBUG("tmpevent[2] =0x%x",tmpevent[2]);

}
/**
*遥控器布防
*/
void CCmdHandler::riu_room_defend(unsigned long event[4]){
        unsigned long tmpevent[4];
        unsigned short tmpmsg[4];
        int i=0;
        for(i =0; i<16;i++)
                set_security_status(i,true);

        tmpmsg[0] = ROOM_RIU_DEFEND_ACK;
        tmpmsg[1] = 0x0;
        tmpmsg[2] = event[3];
        tmpmsg[3] = 0x1 | 0x02; //不需要应答
        send_smalldoor_cmd(tmpmsg);

	
        tmpevent[0]=STATUS_REMOTE_ALARM;
        tmpevent[1]=1;
        tmpevent[2]=tmpevent[3] = 0;
        m_display(tmpevent);
        ReDrawCurrentWindow2(3);
        
        UpdateSystemInfo();

}

/**
*遥控器撤防
*/
void CCmdHandler::riu_room_remove_defend(unsigned long event[4]){
        unsigned short tmpmsg[4];
        unsigned long tmpevent[4];
        tmpmsg[0] = ROOM_RIU_REMOVE_DEFEND_ACK;
        tmpmsg[2] = event[3];
        if(event[1] == 0 && event[2] == 0){
                tmpmsg[1] = 0x1;
                tmpmsg[3] = 0x1  | 0x02; //不需要应答
                send_smalldoor_cmd(tmpmsg);
                return;
        }
        char pws1[16];
        char pws2[16];
        memset(pws1,0,16);
        memset(pws2,0,16);
        memcpy(pws1,pSystemInfo->mySysInfo.password,8);
        memcpy(pws2,(void*)event[1],event[2]);
        WRT_DEBUG("pws1 = %s,pws2 = %s \n",pws1,pws2);
        if(strncmp(pws1,pws2,strlen(pws1)) == 0){
                //if(memcmp(pSystemInfo->mySysInfo.password,(void*)event[1],event[2] ) == 0){
                int i=0;
                for(i =0; i<16;i++)
                        set_security_status(i,false);
                
                tmpevent[0]=STATUS_REMOTE_ALARM;
                tmpevent[1]=0;
                tmpevent[2]=tmpevent[3] = 0;
                m_display(tmpevent);
                ReDrawCurrentWindow2(3);
                
                tmpmsg[1]  = 0x0;
                tmpmsg[3] = 0x01 | 0x02; //不需要应答
                send_smalldoor_cmd(tmpmsg);
                SDL_Delay(200);
                UpdateSystemInfo();
        }else{
                tmpmsg[1] = 0x1;
                tmpmsg[3] = 0x1 | 0x02; //不需要应答
                send_smalldoor_cmd(tmpmsg);
        }
        ZENFREE((void*)event[1]);

}

/**
*遥控器紧急报警
*/
void CCmdHandler::riu_room_warning(unsigned long event[4])
{
        unsigned short tmpmsg[4];
        int ret =0;
        tmpmsg[0] = ROOM_RIU_ROOM_WARNING_ACK;
        tmpmsg[2] = event[3];
        tmpmsg[1] = 0;     //OK
        tmpmsg[3] = 0x1 | 0x02; //不需要应答
        send_smalldoor_cmd(tmpmsg);    
        ret = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,0); 
	if(ret == 0){
                    if(pSystemInfo->mySysInfo.isenglish == 0x01)
                              add_log("Emergency alarm");
                    else
                              add_log("紧急报警成功");
        }else{
                 if(pSystemInfo->mySysInfo.isenglish == 0x01)
                                add_log("Emergency alarm Failed");
                   else
                                add_log("紧急报警失败");
        }
         	 	           
        WRT_MESSAGE("遥控器紧急报警");	
}

/**
*遥控器远程开锁
*/
void CCmdHandler::riu_room_open_lock(unsigned long event[4]){
        unsigned short tmpmsg[4];
        int ret =0;
        tmpmsg[0] = ROOM_RIU_OPEN_LOCK_ACK;
        tmpmsg[2] = event[3];
        //只有在小门口机呼叫或者主机呼叫时才能开锁
         WRT_MESSAGE("遥控器远程开锁");

        if(SYS_TALKTOMAIN  == m_SysStatus ){
                SendCmd(m_CurrentSocketIndex,ROOM_UNLOCK);
                add_and_del_timer(TIME_UNLOCK_EVENT_START); //5M后 自动挂机
                ret = 1;
                tmpmsg[1] = 0;     //OK
                tmpmsg[3] = 0x1 | 0x02; //不需要应答
                send_smalldoor_cmd(tmpmsg);

        }
        if(SYS_SMALLDOORCALL  == m_SysStatus){
                tmpmsg[1] = 0;     //OK
                tmpmsg[3] = 0x1;
                printf("send cmd=%x,%d \n",tmpmsg[0],tmpmsg[1]);
                send_smalldoor_cmd(tmpmsg);
                unsigned short tmpevent[4];
                tmpevent[0] = ROOM_OPENLOCK_SMALLDOOR;
                tmpevent[1] = 0;
                tmpevent[2] = m_smalldoor_id; //目标ID
                tmpevent[3] = 0;
                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                //m_smalldoor_cmd =  ROOM_UNLOCK;
                //add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);
                ret = 1;
                if(m_ringtimerid != 0){//取消震铃计时
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
        }
        if(ret == 0){
                tmpmsg[1] = 1;     //failed
                tmpmsg[3] = 0x1 | 0x02; //不需要应答
                send_smalldoor_cmd(tmpmsg);
        }

}

/**
*遥控器远程无条件撤防
*/
void CCmdHandler::riu_room_remove(unsigned long event[4]){//无条件撤防
        unsigned long tmpevent[4];
        unsigned short tmpmsg[4];
        int i=0;
        for(i =0; i<16;i++)
                set_security_status(i,false);
        
        tmpevent[0]=STATUS_REMOTE_ALARM;
        tmpevent[1]=0;
        tmpevent[2]=tmpevent[3] = 0;
        m_display(tmpevent);
        ReDrawCurrentWindow2(3);
        
        
        tmpmsg[0] = ROOM_RIU_REMOVE_ACK;
        tmpmsg[1] = 0;
        tmpmsg[2] = event[3];
        tmpmsg[3] = 0  | 0x02; //不需要应答
        send_smalldoor_cmd(tmpmsg);
        SDL_Delay(200);
        UpdateSystemInfo();
}
/**
*
*/
void CCmdHandler::center_set_bomb(int sock_index,unsigned char* buffer,int buflen)
{
        if(buffer ==NULL){
                if(m_pSocketManager)
                	m_pSocketManager->CloseSocket(sock_index);
                return;
        }
        if(buflen != 1){
        	if(m_pSocketManager)
                	m_pSocketManager->CloseSocket(sock_index);   
                return;     	
        }
        unsigned char res = *(unsigned char*)buffer;
        pSystemInfo->mySysInfo.isbomb = res;
        UpdateSystemInfo();

        if(m_pSocketManager)
                m_pSocketManager->CloseSocket(sock_index);

}

/***************************************************************************************
小门口机处理

pp***************************************************************************************/

void CCmdHandler::smalldoor_call_room(unsigned long event[4])
  
{

}

//小门口机回应摘机
void CCmdHandler::smalldoor_ack_holdon(unsigned long event[4]){
        WRT_MESSAGE("小门口机应答摘机");
        if(m_SysStatus == SYS_SMALLDOORCALL || g_smalldoor_call_agent == 1)
        {
                if(m_SysStatus == SYS_SMALLDOORCALL )
                {
                        if((g_simulate_talk == 0) && (g_ismyholdon == 0) && (m_is_phone_holdon == 0)){
                                event[1] = 2;
                                smalldoor_ack_hangup(event); //并通知关屏。
                                event[0]=STATUS_SCREEN_CLOSE;
                                event[1] = 0;
                                event[2]=event[3]= 0;
                                m_display(event);
                                return;
                        }
                }
                unsigned long tmpevent[4];
                m_smalldoor_cmd =  -1;
                add_and_del_timer(TIME_TALK_EVENT_START);//120s通话时间

                m_smalldoor_cmd_count = 0;
                if(g_smalldoor_call_agent == 0){
                        if(m_is_phone_holdon == 1){
                   	
#ifdef HAVE_SUB_PHONE
				/*
                                unsigned long ip =0;
                                RoomGpioUtilSetDoorAudioOnD(); //打开小门口数字音频通道(mic->mic);
                                mp3stop();
                                talkstart();
                                talktostart(m_phone_ip,15004);
                                GetHostIP(&ip);
                                start_phone_audio(ip,15004);
                                m_displayevent[0]=STATUS_SCREEN_CLOSE;
                                m_displayevent[1] = 1;
                                m_displayevent[2]=m_displayevent[3]= 0;
                                m_display(m_displayevent);                          
                                */
#endif
                        }else{
                                start_smalldoor_audio(g_simulate_talk);
                                if(g_simulate_talk == 1){
                                        m_displayevent[0]=STATUS_SCREEN_CLOSE;
                                        m_displayevent[1] = 1;
                                        m_displayevent[2]=m_displayevent[3]= 0;
                                        m_display(m_displayevent);                            	                    	
                                        return;
                                }
                        }
                }else{

                        //  RoomGpioUtilSetDoorAudioOff(); //将模拟通道关闭（mic--->spk);
                        RoomGpioUtilSetDoorAudioOnD(); //打开小门口数字音频通道(mic->mic);

                        //修正，当托管到中心，中心又托管到其他地方时，发送的音频的地址不对的BUG
                        unsigned long tmpip = m_pSocketManager->GetIPAddrByIndex(m_CurrentSocketIndex);

                        mp3stop();
                        talkstart();
                        talktostart(tmpip,15004);
                        m_displayevent[0]=STATUS_SCREEN_CLOSE;
                        m_displayevent[1] = 1;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);                


                }

                m_displayevent[0]=STATUS_ROOM_TALK_ROOM;
                m_displayevent[1] = 0;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
        }



}

void CCmdHandler::nvo_holdon(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.isnvo == 0)
                return;
        if(m_SysStatus == SYS_IDLE)
                return;
        WRT_MESSAGE("非可视从分机摘机");
        unsigned short tmpmsg[4]={0};
        unsigned long tmpevent1[4];    
        tmpmsg[0] = ROOM_ACK_SIMLUATE_HOLDON;
        tmpmsg[3] = 0x0 | 0x02; //不需要应答
        send_smalldoor_cmd(tmpmsg);


        tmpevent1[0] = ROOM_USED_SIMULATE;
        tmpevent1[1] = 0;
        tmpevent1[2] = 1;
        tmpevent1[3] = 4;
        udp_send_data(tmpevent1);  //通知其他主、从分机 通道被占用

        tmpevent1[0] = INFORM_SIMULATE_HANGUP;
        tmpevent1[1] = 0;
        tmpevent1[2] = 0; 
        tmpevent1[3] = 0;
        udp_send_data(tmpevent1);           

#ifdef HAVE_SUB_PHONE
	if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
	        unsigned long msg_buf[4]={0};
	        msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
	        msg_buf[1] = 1;
	        send_msg_to_sub_phone(msg_buf);
	        SetTransmitToPhone(0);
	}
#endif

        if(m_SysStatus == SYS_SMALLDOORCALL){ //如果小门口机呼叫
                unsigned long tmpevent1[4];
                if(event[0] == SIMULATE_HANGUP && event[1] == 0x02){        		
                        event[0] = SMALLDOOR_ACK_HANGUP;
                        event[1] = 2;
                        event[2] = 0;
                        event[3] = 0x03;         		
                        smalldoor_ack_hangup(event);
                        return;
                }
                unsigned short tmpevent[4];
                tmpevent[0] = ROOM_HOLDON_SMALLDOOR;
                tmpevent[1] = 0;
                tmpevent[2] = m_smalldoor_id; //目标ID
                tmpevent[3] = 0 ; //需要应答。
                send_smalldoor_cmd(tmpevent); //通知小门口机，分机摘机



                tmpevent1[0] = ROOM_SMALLDOOR_HOLDON;
                tmpevent1[1] = 0;
                tmpevent1[2] = 0;
                tmpevent1[3] = 0;
                udp_send_data(tmpevent1); //通知其他的从分机


                mp3stop();
                if(m_ringtimerid != 0){//如果没有摘机，取消震铃计时
                        add_and_del_timer(TIME_RING_EVENT_STOP);
                }
                if(GetsmalldoorLeaveStatus()){
                        stopsmalldoorleave();
                }

                if(m_talktimerid != 0){//启动通话计时
                        add_and_del_timer(TIME_TALK_EVENT_START);
                }                
                if(m_doorleave_timerid != 0){ //取消第二次计时器
                        add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);
                }
                stop_auto_cap_smalldoor_photo();//如果开启了自动抓拍，则停止
                stop_smalldoor_video();//停止小门口视频

                stopsmalldoorleave();
                if(m_doorleave_timerid != 0)
                        add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);

                //start_smalldoor_audio();

                g_simulate_talk = 1;
                //通知上面回到主界面。
                tmpevent1[0]=STATUS_ROOM_HANGUP;
                tmpevent1[1] = 1;
                tmpevent1[2]=tmpevent1[3]= 0;
                m_display(tmpevent1);

                tmpevent1[0]=STATUS_SCREEN_CLOSE;
                tmpevent1[1] = 0;
                tmpevent1[2]=tmpevent1[3]= 0;
                m_display(tmpevent1);                  


                return;
        }

        mp3stop();
        if(m_ringtimerid != 0){//取消震铃计时
                printf("取消 TIME_RING_EVENT_STOP  \n");
                add_and_del_timer(TIME_RING_EVENT_STOP);
        }
        if(m_doorleave_timerid != 0){ //如果响铃时间刚好到了，同时摘机取消第二次计时器
                add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);
        }
        if(GetLeaveStatus() == 1){ //如果在留言过程中摘机，则取消留言。
                talkLeaveCancel();
        }

        g_simulate_talk = 1; //模拟分机通话



        talktostop();
        talkstop();
        if(m_talktimerid != 0){
                add_and_del_timer(TIME_TALK_EVENT_STOP);
        }

        SendCmd(m_CurrentSocketIndex,ROOM_HOLDON); //通知当前的呼叫已经摘机

        printf("TIME_TALK_EVENT_START\n");
        add_and_del_timer(TIME_TALK_EVENT_START);//120s通话时间





        RoomGpioUtilSetDoorAudioOnD(); //打开小门口数字音频通道(mic->mic);

        //修正，当托管到中心，中心又托管到其他地方时，发送的音频的地址不对的BUG
        unsigned long tmpip = m_pSocketManager->GetIPAddrByIndex(m_CurrentSocketIndex);
        mp3stop();
        talkstart();
        talktostart(tmpip,15004);



        //通知上面回到主界面。
        tmpevent1[0]=STATUS_ROOM_HANGUP;
        tmpevent1[1] = 1;
        tmpevent1[2]=tmpevent1[3]= 0;
        m_display(tmpevent1);


        tmpevent1[0]=STATUS_SCREEN_CLOSE;
        tmpevent1[1] = 0;
        tmpevent1[2]=tmpevent1[3]= 0;
        m_display(tmpevent1);        




}
void CCmdHandler::nvo_unlock(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.isnvo == 0)
                return;	
        unsigned short tmpmsg[4]={0};
        tmpmsg[0] = ROOM_ACK_SIMULATE_UNLOCK;
        tmpmsg[3] = 0x0  | 0x02; //不需要应答
        send_smalldoor_cmd(tmpmsg);  
        WRT_MESSAGE("非可视从分机开锁");

        if(SYS_TALKTOMAIN  == m_SysStatus ){       				

                SendCmd(m_CurrentSocketIndex,ROOM_UNLOCK);
                add_and_del_timer(TIME_UNLOCK_EVENT_START); //5M后 自动挂机
                return;
        }
        if(SYS_SMALLDOORCALL == m_SysStatus)
        {
                unsigned short tmpevent[4];
                tmpevent[0] = ROOM_OPENLOCK_SMALLDOOR;
                tmpevent[1] = 0;
                tmpevent[2] = m_smalldoor_id; //目标ID
                tmpevent[3] = 0x0;
                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机               
                return;

        }
        		
        add_and_del_timer(TIME_UNLOCK_EVENT_START); //5M后 自动挂机

}

void CCmdHandler::nvo_hangup(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.isnvo == 0)
                return;
	unsigned long tmpevent[4]={0};
        unsigned short tmpmsg[4]={0};
        tmpmsg[0] = ROOM_ACK_SIMULATE_HANGUP;
        tmpmsg[3] = 0x0 | 0x02; //不需要应答;
        send_smalldoor_cmd(tmpmsg);
        tmpevent[0] = ROOM_USED_SIMULATE;
        tmpevent[1] = 0;
        tmpevent[2] = 0;
        tmpevent[3] = 4;
        udp_send_data(tmpevent);  //通知其他主、从分机释放通道
        WRT_MESSAGE("非可视从分机挂机");
        
#ifdef HAVE_SUB_PHONE
	if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
	        unsigned long msg_buf[4]={0};
	        msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
	        msg_buf[1] = 1;
	        send_msg_to_sub_phone(msg_buf);
	        SetTransmitToPhone(0);
	}
#endif
        if( m_SysStatus == SYS_IDLE)
                return;
        if(SYS_SMALLDOORCALL == m_SysStatus){
                unsigned short tmpevent[4]={0};
                if(event[0] == SIMULATE_HANGUP && event[1] == 0x02){        		
                        event[0] = SMALLDOOR_ACK_HANGUP;
                        event[1] = 2;
                        event[2] = 0;
                        event[3] = 0x03;         		
                        smalldoor_ack_hangup(event);
                        return;
                }                
                tmpevent[0] = ROOM_HANGUP_SMALLDOOR;
                tmpevent[1] = 0;
                tmpevent[2] = m_smalldoor_id; //目标ID
                tmpevent[3] = 0;
                send_smalldoor_cmd(tmpevent); //通知小门口机，分级挂机
                return;
        }		
        if(SYS_TALKTOMAIN  == m_SysStatus)
                Stopplayvideo();
        mp3stop();
        if(m_ringtimerid != 0){//如果没有摘机，取消震铃计时
                add_and_del_timer(TIME_RING_EVENT_STOP);
        }
        if(GetLeaveStatus() == 1 && SYS_TALKTOMAIN == m_SysStatus) //如果在留言过程中挂机，则取消留言
                talkLeaveStop();
        if(GetsmalldoorLeaveStatus()){
                stopsmalldoorleave();
        }
        talktostop();
        talkstop();

        if(m_talktimerid != 0){//如果有摘机，时间到，或者主动挂机，取消通话计时
                add_and_del_timer(TIME_TALK_EVENT_STOP);
        }
        if(m_unlock_timerid != 0){ //取消开锁计时器
                add_and_del_timer(TIME_UNLOCK_EVENT_STOP);
        }
        if(m_doorleave_timerid != 0){ //取消第二次计时器
                add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);
        }
        SendCmd(m_CurrentSocketIndex,ROOM_HANGUP);

        int ok = 0;
        if(SYS_TALKTOMAIN  == m_SysStatus ){
                if(pSystemInfo->mySysInfo.isautocap == 1)
                        CancelCapturePhoto1();//取消自动抓拍图片
                ok = 1; //关闭视频
        }
        if(g_simulate_talk == 1)
        {
                RoomGpioUtilSetDoorAudioOff();
                g_simulate_talk = 0;

        }
        m_displayevent[0]=STATUS_ROOM_HANGUP;
        m_displayevent[1] = ok;
        m_displayevent[2]=m_displayevent[3]= 0;
        m_display(m_displayevent);
        m_pSocketManager->CloseSocket(m_CurrentSocketIndex);
        m_SysStatus = SYS_IDLE;
        m_CurrentSocketIndex = -1;
}

//小门口机回应挂机
void CCmdHandler::smalldoor_ack_hangup(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.isavoid == 1)
                return;
        WRT_MESSAGE("小门口机应答挂机");
        if(m_SysStatus == SYS_SMALLDOORCALL && event[3] == 0x03)
        {
                unsigned long tmpevent1[4];

                tmpevent1[0] = ROOM_USED_SIMULATE;
                tmpevent1[1] = 0;
                tmpevent1[2] = 0;
                tmpevent1[3] = 4;
                udp_send_data(tmpevent1);  //通知其他主、从分机,通道已经释放。
                m_ishostbusy = 0; //2009-7-16，     
        }
        if(m_SysStatus == SYS_SMALLDOORCALL || g_smalldoor_call_agent == 1){ //2009-7-17
                unsigned long tmpevent[4];

                m_smalldoor_id = -1;
                m_smalldoor_cmd_count = 0;
                if(g_issmalldoor_hangup == 1)
                        g_issmalldoor_hangup = 0;
                if(g_holdonbyme == 1)
                        g_holdonbyme  = 0;
                if(event[1] == 2) //非主分机，不能留言，模拟直接应答挂机。要停止播放MP3.2009-7-16
                        mp3stop();
                add_and_del_timer(TIME_UNLOCK_EVENT_STOP);
                if(g_smalldoor_call_agent == 0)
                {
                        if(m_is_phone_holdon == 1)
                        {
#ifdef HAVE_SUB_PHONE
                                RoomGpioUtilSetDoorAudioOff(); //如果是托管挂机，则还原为通常状态
                                stop_smalldoor_video();//停止小门口视频
                                stop_phone_audio();
                                talkstop();
                                talktostop();
                                unsigned long msg_buf[4]={0};
                                msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
                                msg_buf[1] = 1;
                                send_msg_to_sub_phone(msg_buf);
#endif
                                m_is_phone_holdon =0;
                        }else{
#if 1
                                stop_auto_cap_smalldoor_photo();//如果开启了自动抓拍，则停止
                                stop_smalldoor_video();//停止小门口视频
                                stop_smalldoor_audio(g_simulate_talk);//停止小门口音频
#endif
                        }
                        m_displayevent[0]=STATUS_ROOM_HANGUP;
                        m_displayevent[1] = 1;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        printf("smalldoor_ack_hangup STATUS_ROOM_HANGUP \n");
                        m_display(m_displayevent);

                }else{
                        RoomGpioUtilSetDoorAudioOff(); //如果是托管挂机，则还原为通常状态
                }
		if(GetsmalldoorLeaveStatus()){
                	stopsmalldoorleave();
                	talktostop();
                	
                }
                if(m_doorleave_timerid != 0)
                        add_and_del_timer(TIME_DOOR_LEAVE_EVENT_STOP);

                m_SysStatus = SYS_IDLE;
                g_smalldoor_call_agent = 0;
                if(g_simulate_talk == 1) 
                        g_simulate_talk = 0;
                m_istalkandsmalldoor = 0;
        }

}

//小门口机回应开锁
void CCmdHandler::smalldoor_ack_openlock(unsigned long event[4]){
        // m_smalldoor_cmd =  -1;
        if(m_SysStatus == SYS_SMALLDOORCALL || g_smalldoor_call_agent == 1){
                // add_and_del_timer2(TIME_SMALLDOOR_EVENT_STOP);
                m_smalldoor_cmd_count = 0;
                g_issmalldoor_hangup = 1;
                add_and_del_timer(TIME_UNLOCK_EVENT_START); //5M后 自动挂机
                WRT_MESSAGE("小门口机应答开锁");
        }

}

//小门口机留言
void CCmdHandler::smalldoor_leave(unsigned long event[4]){
        unsigned short tmpevent[4];
        WRT_MESSAGE("小门口机留言");
        mp3stop();

        //通知模拟分机挂机。 //表示无人接听的情况。
        tmpevent[0] = INFORM_SIMULATE_HANGUP;
        tmpevent[1] = 0;
        tmpevent[2] = 0x0; //目标ID
        tmpevent[3] = 0 |0x02;
        send_smalldoor_cmd(tmpevent); //通知小门口机，分级留言
        send_smalldoor_cmd(tmpevent); //通知小门口机，分级留言

        tmpevent[0] = ROOM_SMALLDOOR_LEAVE;
        tmpevent[1] = 0;
        tmpevent[2] = m_smalldoor_id; //目标ID
        tmpevent[3] = 0;
        send_smalldoor_cmd(tmpevent); //通知小门口机，分级留言
        g_is_leave = 1;
        // m_smalldoor_cmd = CUSTOM_CMD_LEAVE;
        // add_and_del_timer2(TIME_SMALLDOOR_EVENT_START);

}

void CCmdHandler::smalldoor_ack_leave(unsigned long event[4]){

        // m_smalldoor_cmd  = -1;

        if(m_SysStatus != SYS_SMALLDOORCALL)
                return;
        WRT_MESSAGE("小门口机应答留言");
        add_and_del_timer(TIME_RING_EVENT_STOP);
        // add_and_del_timer2(TIME_SMALLDOOR_EVENT_STOP);


        RoomGpioUtilSetDoorAudioOnD(); //打开小门口数字音频通道(mic->mic);
        add_and_del_timer(TIME_SMALLDOOR_LEAVE_EVENT_START);

        mp3stop();
        starsmalldoorleavervoice();
        //启动计时器，留言20S.
}

void CCmdHandler::smalldoor_ack_mont(unsigned long event[4]){

        if(SYS_MONTSMALLDOOR == m_SysStatus){

                if(event[2] > 0 ){
                        unsigned char* p = (unsigned char*)event[1];
                        if(*p == 0x01){//应答空闲

                                WRT_MESSAGE("小门机应答监视");
#ifdef HAVE_SUB_PHONE
/*
                                if(g_phone_connect_index != -1)
                                {       
                                        WRT_MESSAGE("小门机应答监视,转发手机");
                                        SendCmd(g_phone_connect_index,DOOR_MONTACK);
                                }
*/                                
#endif
                                // m_smalldoor_cmd =  -1;
                                // add_and_del_timer2(TIME_SMALLDOOR_EVENT_STOP);


                                m_smalldoor_cmd_count = 0;
                                //  start_smalldoor_video();//启动小门口机视频
                                m_displayevent[0] = STATUS_PAUSE_VIDEO;
                                m_displayevent[1] = 0;
                                m_displayevent[2]= 1;
                                m_displayevent[3] = 0;
                                m_display(m_displayevent); //启动小门口机视频
                                if(m_watchtimerid != 0){
                                        add_and_del_timer(TIME_MONT_EVENT_STOP);
                                }
                                add_and_del_timer(TIME_MONT_EVENT_START);


                        }else{
                                WRT_MESSAGE("小门机应答繁忙，不能监视");
#ifdef HAVE_SUB_PHONE
/*
                                if(g_phone_connect_index != -1)
                                {
                                        WRT_MESSAGE("小门机应答繁忙，不能监视，转发手机");
                                        SendCmd(g_phone_connect_index,DOOR_BUSY);
                                        if(m_pSocketManager)
                                                m_pSocketManager->CloseSocket(g_phone_connect_index);
                                        g_phone_connect_index = -1;
                                }
*/                                
#endif
                                //  m_smalldoor_cmd =  -1;
                                //   add_and_del_timer2(TIME_SMALLDOOR_EVENT_STOP);


                                m_smalldoor_cmd_count = 0;
                                m_displayevent[0]=STATUS_DOOR_BUSY;//STATUS_SMALLDOOR_BUSY;//通知GUI 小门口机繁忙
                                m_displayevent[1] = 0;
                                m_displayevent[2]=m_displayevent[3]= 0;
                                m_display(m_displayevent);
                                m_SysStatus = SYS_IDLE;
                        }
                        ZENFREE(p);
                }else{
                        //  m_smalldoor_cmd =  -1;
                        // add_and_del_timer2(TIME_SMALLDOOR_EVENT_STOP);


                        m_smalldoor_cmd_count = 0;
                        m_displayevent[0]=STATUS_DOOR_BUSY;//STATUS_SMALLDOOR_BUSY;//通知GUI 分机繁忙
                        m_displayevent[1] = 0;
                        m_displayevent[2]=m_displayevent[3]= 0;
                        m_display(m_displayevent);
                        m_SysStatus = SYS_IDLE;
                }
        }

}

void CCmdHandler::smalldoor_ack_stopmont(unsigned long event[4]){
        if(m_SysStatus != SYS_MONTSMALLDOOR)
                return;
        WRT_MESSAGE("小门口机应答停止监视");
        unsigned long tmpevent[4];
        m_smalldoor_id = -1;
        m_smalldoor_cmd_count = 0;
        stop_smalldoor_video();

#ifdef HAVE_SUB_PHONE
/*
        if(g_phone_connect_index != -1)
        {
                WRT_MESSAGE("小门口机应答停止监视，转发到手机");
                SendCmd(g_phone_connect_index,DOOR_ENDMONT);
                if(m_pSocketManager)
                        m_pSocketManager->CloseSocket(g_phone_connect_index);
                g_phone_connect_index = -1;
        }
*/        
#endif

        tmpevent[0] = STATUS_ROOM_STOPMONT;
        tmpevent[1] = 0;
        tmpevent[2]= tmpevent[3]= 0;
        m_display(tmpevent);
        m_SysStatus = SYS_IDLE;
        if(is_small_call == true){
                is_small_call = false;
                smalldoor_call_room(g_save_event);

        }
}

/************************************************************************************
-------------------------------------------------------------------------------------
************************************************************************************/
/*初始化缓存包队列*/
#if USE_PACKELIST
void CCmdHandler::InitPackList()
{
        m_PacketList.sockindex = -1;
        m_PacketList.buflen = 0;
        m_PacketList.buf = NULL;
        m_PacketList.next = NULL;
        m_PacketList.PacketLen=0;
}
void CCmdHandler::UnInitPackList()
{
        SOCKPACKETLIST* pTemp = NULL;
        SOCKPACKETLIST* pTemp2 = NULL;
        pTemp = (SOCKPACKETLIST*)m_PacketList.next;
        while(pTemp){
                if(pTemp->buf != NULL){
                        ZENFREE(pTemp->buf);
                        pTemp->buf = NULL;
                }
                pTemp->sockindex = -1;
                pTemp->buflen = 0;
                pTemp2 = pTemp;
                pTemp = pTemp->next;
                ZENFREE(pTemp2);
                pTemp2 = NULL;
        }
        if(m_PacketList.buf != NULL){
                ZENFREE(m_PacketList.buf);
                m_PacketList.buf = NULL;
        }
        m_PacketList.sockindex = -1;
        m_PacketList.buflen = 0;
        m_PacketList.next = NULL;

}

/*将包缓存起来*/
void CCmdHandler::AddPackList(int sock_index,unsigned char* buf,int buflen,int PacketLen,bool isfirst)
{
        SOCKPACKETLIST* pTemp = NULL;
        SOCKPACKETLIST* pTemp2 = NULL;

        pTemp = (SOCKPACKETLIST*)&m_PacketList;
        if(isfirst == true && pTemp != NULL && pTemp->sockindex == -1){
                pTemp->sockindex = sock_index;
                pTemp->buflen = buflen;
                pTemp->PacketLen = PacketLen;
                pTemp->buf = (unsigned char*) ZENMALLOC(PacketLen);
                if(pTemp->buf == NULL){
                        pTemp->sockindex = -1;
                        pTemp->buflen =  -1;
                        pTemp->PacketLen = -1;
                        return;
                }
                memcpy(pTemp->buf,buf,buflen);
                return;
        }
        while(pTemp){
                if(pTemp->sockindex == sock_index){
                        memcpy(pTemp->buf+pTemp->buflen,buf,buflen);
                        pTemp->buflen +=buflen;
                        if(pTemp->buflen == pTemp->PacketLen){
                                if(strncmp("WRTI",(char*)pTemp->buf,4) == 0){
                                        short tmpcmd = 0;
                                        int PacketLen = *(int*)(pTemp->buf+4);
                                        tmpcmd = *(short*)(pTemp->buf+8);
                                        tmpcmd = ntohs(tmpcmd);

#ifdef USED_NEW_CMD
                                        ProcessPacket(pTemp->sockindex,tmpcmd,pTemp->buf+40,pTemp->buflen-40);
#else
                                        ProcessPacket(pTemp->sockindex,tmpcmd,pTemp->buf+10,pTemp->buflen-10);
#endif
                                }
                                DelPackList(sock_index);
                        }
                        return;
                }
                pTemp2 = pTemp;
                pTemp = pTemp->next;
        }


        if(isfirst == true){
                pTemp = NULL;
                pTemp = (SOCKPACKETLIST*)ZENMALLOC(sizeof(SOCKPACKETLIST));
                pTemp->sockindex = sock_index;
                pTemp->buflen = buflen;
                pTemp->PacketLen = PacketLen;
                pTemp->buf = (unsigned  char*)ZENMALLOC(PacketLen);
                if(pTemp->buf == NULL){

                        pTemp->sockindex = -1;
                        pTemp->buflen =  -1;
                        pTemp->PacketLen = -1;
                        return;
                }
                memcpy(pTemp->buf ,buf,buflen);
                pTemp->next = NULL;
                pTemp2->next = pTemp;
                return;
        }
        if(isfirst == false && sock_index != -1){

                m_pSocketManager->CloseSocket(sock_index); //接收到非法数据，关闭改SOCKET。
                return;
        }
}
/*将缓存的包删除*/
void CCmdHandler::DelPackList(int sock_index)
{
        SOCKPACKETLIST* pTemp = NULL;
        SOCKPACKETLIST* pTemp2 = NULL;
        pTemp = (SOCKPACKETLIST*)&m_PacketList;
        if(pTemp != NULL && pTemp->sockindex == sock_index){
                pTemp->sockindex = -1;
                pTemp->buflen = 0;
                pTemp->PacketLen = 0;
                if(pTemp->buf != NULL)
                        ZENFREE(pTemp->buf);
                pTemp->buf = NULL;
                return;
        }
        pTemp2 = pTemp;
        pTemp = pTemp->next;
        while(pTemp){
                if(pTemp->sockindex == sock_index){
                        pTemp->sockindex = -1;
                        pTemp->buflen = 0;
                        pTemp->PacketLen = 0;
                        if(pTemp->buf != NULL)
                                ZENFREE(pTemp->buf);
                        pTemp->buf = NULL;
                        pTemp2->next = pTemp->next;
                        ZENFREE(pTemp);
                        pTemp = NULL;
                        break;
                }
                pTemp2 = pTemp;
                pTemp = pTemp->next;
        }
}
#endif

//---------------------------------------------------------------------
int CCmdHandler::GetHostIP(unsigned long* ip){
        *ip = pSystemInfo->LocalSetting.privateinfo.LocalIP;
        return 1;
}

int CCmdHandler::GetHostMask(unsigned long* mask){
        *mask = pSystemInfo->LocalSetting.privateinfo.SubMaskIP;
        return 1;
}

int CCmdHandler::GetHostGateWay(unsigned long* gateway){
        *gateway  = pSystemInfo->LocalSetting.privateinfo.GateWayIP;
        return 1;
}

int CCmdHandler::SetHostIP(unsigned long ip){
        if(m_pSocketManager->ModifyHostIpAndMask(ip,0)){
                pSystemInfo->LocalSetting.privateinfo.LocalIP = ip;
                return 1;
        }
        return 0;
}

void CCmdHandler::SetgarageIP(unsigned long ip){

        m_garageipaddr = ip;

}

void CCmdHandler::SetManagerIP(unsigned long ip){ //设置分机托管IP

        m_manageripaddr = ip;
        pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP = ip;

}

int CCmdHandler::SetHostMask(unsigned long mask){
        if(m_pSocketManager->ModifyHostIpAndMask(0,mask)){
                pSystemInfo->LocalSetting.privateinfo.SubMaskIP = mask;
                return 1;
        }
        return 0;
}

int CCmdHandler::SetHostGateWay(unsigned long gateway){
        if(m_pSocketManager->ModifyHostGateWay(gateway)){
                pSystemInfo->LocalSetting.privateinfo.GateWayIP = gateway;
                return 1;
        }
        return 0;
}

int CCmdHandler::SaveModify(){
        //FlushSystemInfo();
        UpdateSystemInfo(); //直接写flash
        return 1;
}
int CCmdHandler::ReInitSubRoomIP()
{
	reinit_subroom_ip();
}
void CCmdHandler::SetCenterIP(unsigned long ip)
{

        m_centeripaddr  = ip;
        pSystemInfo->LocalSetting.publicinfo.CenterMajorIP = ip;
        WRT_MESSAGE("重新设置m_centeripaddr = 0x%x",m_centeripaddr);

}

void CCmdHandler::SetDoorIP(unsigned long ip){
        if(pSystemInfo->LocalSetting.publicroom.NumOfDoor == 0)
                pSystemInfo->LocalSetting.publicroom.NumOfDoor = 1;
        pSystemInfo->LocalSetting.publicroom.Door[0].IP = ip; //Door[0]永远指向主门口机
       // strcpy(pSystemInfo->LocalSetting.publicroom.Door[0].name,"门口机");


}

void  CCmdHandler::GetDoorIP(unsigned long* ip){
        //*ip=roomtest.LocalRoom.GroupDoorIP;
        *ip = pSystemInfo->LocalSetting.publicroom.Door[0].IP;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void CCmdHandler::SetCenterCallRing( char* addr,long size){

        if(addr != NULL){
                strcpy(g_ringset[CENTER_CALL_RING].ring_path,(char*)addr);
        }else{
                if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[CENTER_CALL_RING].ring_path,"/home/wrt/en/ring/Guard center.mp3");
                 else
                 	strcpy(g_ringset[CENTER_CALL_RING].ring_path,"/home/wrt/zh/ring/值班室呼叫.mp3");

        }
}

void CCmdHandler::SetDefaultRing(){

    if(GetCurrentLanguage() == ENGLISH)
     	strcpy(g_ringset[CENTER_CALL_RING].ring_path,"/home/wrt/en/ring/Guard center.mp3");
     else
     	strcpy(g_ringset[CENTER_CALL_RING].ring_path,"/home/wrt/zh/ring/值班室呼叫.mp3");
    if(GetCurrentLanguage() == ENGLISH)
     	strcpy(g_ringset[WALL_CALL_RING].ring_path,"/home/wrt/en/ring/Entrance.mp3");
     else
     	strcpy(g_ringset[WALL_CALL_RING].ring_path,"/home/wrt/zh/ring/小区入口呼叫.mp3");
    if(GetCurrentLanguage() == ENGLISH)
	 	strcpy(g_ringset[DOOR_CALL_RING].ring_path,"/home/wrt/en/ring/Unit gate.mp3");
	 else
	 	strcpy(g_ringset[DOOR_CALL_RING].ring_path,"/home/wrt/zh/ring/本栋入口呼叫.mp3");
	if(GetCurrentLanguage() == ENGLISH)
     	strcpy(g_ringset[ROOM_CALL_RING].ring_path,"/home/wrt/en/ring/Tenant.mp3");
     else
     	strcpy(g_ringset[ROOM_CALL_RING].ring_path,"/home/wrt/zh/ring/住户呼叫.mp3");
     	
	if(GetCurrentLanguage() == ENGLISH)
     	strcpy(g_ringset[WARNING_RING].ring_path,"/home/wrt/en/ring/Alarm.mp3");
     else
     	strcpy(g_ringset[WARNING_RING].ring_path,"/home/wrt/zh/ring/报警铃声.mp3");
	if(GetCurrentLanguage() == ENGLISH)
     	strcpy(g_ringset[MSG_RING].ring_path,"/home/wrt/en/ring/New message.mp3");
     else
     	strcpy(g_ringset[MSG_RING].ring_path,"/home/wrt/zh/ring/新信息提示.mp3");
    if(GetCurrentLanguage() == ENGLISH)
     	strcpy(g_ringset[SMALL_DOOR_CALL_RING].ring_path,"/home/wrt/en/ring/Front gate.mp3");
     else
     	strcpy(g_ringset[SMALL_DOOR_CALL_RING].ring_path,"/home/wrt/zh/ring/小门口呼叫.mp3");
 	//if(GetCurrentLanguage() == ENGLISH)
	 // 	strcpy(g_ringset[ALARM_CLOCK_RING].ring_path,"/home/wrt/en/ring/Default.mp3");
	//else
	  	strcpy(g_ringset[ALARM_CLOCK_RING].ring_path,"/home/wrt/zh/ring/默认铃声.mp3");
}

void CCmdHandler::SetWallCallRing(char* addr,long size){

        if(addr != NULL){
        	strcpy(g_ringset[WALL_CALL_RING].ring_path,(char*)addr);

        }else{
                 if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[WALL_CALL_RING].ring_path,"/home/wrt/en/ring/Entrance.mp3");
                 else
                 	strcpy(g_ringset[WALL_CALL_RING].ring_path,"/home/wrt/zh/ring/小区入口呼叫.mp3");
        }
}

void CCmdHandler::SetDoorCallRing(char* addr,long size){

        if(addr != NULL){
        	strcpy(g_ringset[DOOR_CALL_RING].ring_path,(char*)addr);
        }else{
                 if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[DOOR_CALL_RING].ring_path,"/home/wrt/en/ring/Unit gate.mp3");
                 else
                 	strcpy(g_ringset[DOOR_CALL_RING].ring_path,"/home/wrt/zh/ring/本栋入口呼叫.mp3");

        }
}

void CCmdHandler::SetRoomCallRing(char* addr,long size){

        if(addr != NULL ){
        	strcpy(g_ringset[ROOM_CALL_RING].ring_path,(char*)addr);
        }else{
                 if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[ROOM_CALL_RING].ring_path,"/home/wrt/en/ring/Tenant.mp3");
                 else
                 	strcpy(g_ringset[ROOM_CALL_RING].ring_path,"/home/wrt/zh/ring/住户呼叫.mp3");

        }
}

void CCmdHandler::SetSmallDoorCallRing(char* addr,long size){

        if(addr != NULL){
               strcpy(g_ringset[SMALL_DOOR_CALL_RING].ring_path,(char*)addr);

        }else{
                 if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[SMALL_DOOR_CALL_RING].ring_path,"/home/wrt/en/ring/Front gate.mp3");
                 else
                 	strcpy(g_ringset[SMALL_DOOR_CALL_RING].ring_path,"/home/wrt/zh/ring/小门口呼叫.mp3");

        }
}

char* CCmdHandler::GetRingConfig(int type,int* len){
        return (char*)g_ringset[type].ring_path;
}

void CCmdHandler::SetWarningRing(char* addr,long size){

        if(addr != NULL){
                strcpy(g_ringset[WARNING_RING].ring_path,(char*)addr);
        }else{
                 if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[WARNING_RING].ring_path,"/home/wrt/en/ring/Alarm.mp3");
                 else
                 	strcpy(g_ringset[WARNING_RING].ring_path,"/home/wrt/zh/ring/报警铃声.mp3");
        }
}

void CCmdHandler::SetMsgRing(char* addr,long size){

        if(addr != NULL ){
               strcpy(g_ringset[MSG_RING].ring_path,(char*)addr);
        }else{
                 if(GetCurrentLanguage() == ENGLISH)
                 	strcpy(g_ringset[MSG_RING].ring_path,"/home/wrt/en/ring/New message.mp3");
                 else
                 	strcpy(g_ringset[MSG_RING].ring_path,"/home/wrt/zh/ring/新信息提示.mp3");
        }
}

void CCmdHandler::SetAlarmClockRing(char* addr,long size)
{
        if(addr != NULL ){
                 strcpy(g_ringset[ALARM_CLOCK_RING].ring_path,(char*)addr);
        }else{
                 //if(GetCurrentLanguage() == ENGLISH)
                 //	strcpy(g_ringset[ALARM_CLOCK_RING].ring_path,"/home/wrt/en/ring/Default.mp3");
                 //else
                 	strcpy(g_ringset[ALARM_CLOCK_RING].ring_path,"/home/wrt/zh/ring/默认铃声.mp3");
        }	
}

/*
void SendStopWarningTimer(){
add_and_del_timer(TIME_MP3_EVENT_STOP);
}
void SendStartWarningTimer(){
add_and_del_timer(TIME_MP3_EVENT_START);
}
*/
void CCmdHandler::playvoice(int index){
        if(index == WARNING_RING){
                if(GetPlayMp3Status() != 7)
                        mp3stop();
                mp3play((char*)g_ringset[index].ring_path,0,7);
        }else{
        	int  loop = 0;
        	if(index == ALARM_CLOCK_RING)
        		loop = 2;
        	if(index == DOOR_CALL_RING || index == SMALL_DOOR_CALL_RING || index == WALL_CALL_RING)
        		loop = 3;
        	WRT_DEBUG("playvoice index = %d,%s",index,g_ringset[index].ring_path);
        	mp3stop();
            mp3play((char*)g_ringset[index].ring_path,0,loop);
                
        }
               
}

void CCmdHandler::playvoice2(int index,int loop)
{
	mp3stop();
	mp3play((char*)g_ringset[index].ring_path,0,loop);
}

void CCmdHandler::avoidtimeout(int type){
        unsigned long tmpevent[4];

        int msgtype =MSG_NODE_RECV_UDP_MSG ;
        tmpevent[0] = STATUS_AVOID_TIMES_TIMEOUT;
        tmpevent[1]=type;
        tmpevent[2]=tmpevent[3] = 0;
        m_display(tmpevent);
        if(type == 1)
                tmpevent[0] = ROOM_CANCEL_SYNC_AGENT;
        else
                tmpevent[0] = ROOM_CANCEL_SYNC_AVOID;

        tmpevent[1] = 0;
        tmpevent[2] = 0;
        tmpevent[3] = 0;
        m_pCmdHandler->Send_msg(msgtype,(void*)tmpevent,sizeof(tmpevent));
}

int CCmdHandler::udp_send_data2(unsigned long ip,unsigned long event[4],char* destid){
        if(ip == pSystemInfo->LocalSetting.privateinfo.LocalIP)
                return 0;
        unsigned long tmpipaddr = 0;
        char ctmpbuf[44];
        char tmpip[20];
        int len = 44;
        short tmpcmd ;
        int i =0;
        tmpcmd = event[0];
        tmpcmd  = htons(tmpcmd);
        memset(ctmpbuf,0,44);
        strncpy(ctmpbuf,"WRTI",4);
        memcpy(ctmpbuf+8,&tmpcmd,2);
        memcpy(ctmpbuf +10 ,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        if(destid){
                memcpy(ctmpbuf +25,destid,15);
        }
        if(event[3] == 4)
                memcpy(ctmpbuf+40,&event[2],4);
        else
                len = 40;
        memcpy(ctmpbuf+4,&len,4);
        App_Enet_Send2(ip,20302,ctmpbuf,len);
        return 0;
}

int CCmdHandler::udp_send_data(unsigned long event[4]){

        unsigned long tmpipaddr = 0;
        char ctmpbuf[44];
        char tmpip[20];
        int len = 44;
        short tmpcmd ;
        int i =0;
        tmpcmd = event[0];
        tmpcmd  = htons(tmpcmd);
        memset(ctmpbuf,0,44);
        strncpy(ctmpbuf,"WRTI",4);
        memcpy(ctmpbuf+8,&tmpcmd,2);
        memcpy(ctmpbuf +10 ,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        if(event[3] == 4)
                memcpy(ctmpbuf+40,&event[2],4);
        else
                len = 40;
        memcpy(ctmpbuf+4,&len,4);
		for(i = 0;i<8;i++)
		{
			tmpipaddr = subroomipaddr[i];
			if( (i+1) != g_localid && tmpipaddr != 0 && tmpipaddr != 0xffffffff  && tmpipaddr != pSystemInfo->LocalSetting.privateinfo.LocalIP)
			{
				App_Enet_Send2(tmpipaddr,20302,ctmpbuf,len);
			}
		}
        return 0;
}
int CCmdHandler::udp_send_data_hangup(unsigned long event[4]){
        unsigned long tmpipaddr = 0;
        char ctmpbuf[44];
        char tmpip[20];
        int len = 44;
        short tmpcmd ;
        int i =0;
        tmpcmd = event[0];
        tmpcmd  = htons(tmpcmd);
        memset(ctmpbuf,0,44);
        strncpy(ctmpbuf,"WRTI",4);
        memcpy(ctmpbuf+8,&tmpcmd,2);
        memcpy(ctmpbuf +10 ,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        if(event[3] == 4)
                memcpy(ctmpbuf+40,&event[2],4);
        else
                len = 40;
        memcpy(ctmpbuf+4,&len,4);
		for(i = 0;i<4;i++)
		{
			tmpipaddr = subroomipaddr[i];
			if( (i+1) != g_localid && tmpipaddr != 0 && tmpipaddr != 0xffffffff  && tmpipaddr != pSystemInfo->LocalSetting.privateinfo.LocalIP)
                {
                        App_Enet_Send2(tmpipaddr,20302,ctmpbuf,len);
                }
        }
        return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//sip process
void CCmdHandler::room_call_sip(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(m_SysStatus != SYS_IDLE)
                return;
        SIP_DATA_SOURCES* psip = getSipDataSource(event[1]);
        WRT_MESSAGE("分机呼叫%s",psip->code);
        unsigned long tmpevent[4];
        tmpevent[0] = SIP_CALL;
        tmpevent[1] =0;
        tmpevent[2] =0;//strtoul(psip->code,NULL,10);
        tmpevent[3] =0;
        udp_send_data2(pSystemInfo->mySysInfo.sipproxyip,tmpevent,psip->code);
        m_SysStatus = SYS_CALLSIP;
        
}
void CCmdHandler::call_sip_processing(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(m_SysStatus != SYS_CALLSIP)
                return;
        //通知GUI，sip 会话正在连接....
        unsigned long tmpevent[4];
        tmpevent[0] =STATUS_SIP_CONNECTING;
        tmpevent[1] = tmpevent[2] = tmpevent[3]  = 0;
        m_display(tmpevent);
        WRT_MESSAGE("SIP 呼叫正在连接");

}
void CCmdHandler::call_sip_ring(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(m_SysStatus != SYS_CALLSIP)
                return;
        //响铃
        mp3stop();
        mp3play((char*)"/home/wrt/zh/ring/ringback.mp3",0,2);

        unsigned long tmpevent[4];
        tmpevent[0] =STATUS_SIP_CONNECTING;
        tmpevent[1] = 1;
        tmpevent[2] = tmpevent[3]  = 0;
        m_display(tmpevent);
        WRT_MESSAGE("SIP 呼叫响铃");
}
void CCmdHandler::call_sip_close(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(m_SysStatus == SYS_CALLSIP || m_SysStatus  == SYS_SIPCALL){

                mp3stop();
                //通知GUI，sip关闭.
                talktostop();
                talkstop();

                unsigned long tmpevent[4];
                tmpevent[0] =STATUS_ROOM_HANGUP;
                tmpevent[1] = tmpevent[2] = tmpevent[3]  = 0;
                m_display(tmpevent);
                WRT_MESSAGE("SIP 会话结束");
                m_SysStatus = SYS_IDLE;
        }
}
void CCmdHandler::call_sip_busy(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(m_SysStatus != SYS_CALLSIP)
                return;
        mp3stop();
        //通知GUI，sip呼叫繁忙.
        unsigned long tmpevent[4];
        tmpevent[0] =STATUS_SIP_BUSY;
        tmpevent[1] = tmpevent[2] = tmpevent[3]  = 0;
        m_display(tmpevent);
        WRT_MESSAGE("SIP 呼叫,对方繁忙");
        m_SysStatus = SYS_IDLE;
}
void CCmdHandler::call_sip_start_audio(unsigned long event[4]){
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(m_SysStatus != SYS_CALLSIP)
                return;
        //通知GUI开始通话
        mp3stop();
        talktostop();
        talkstop();

        talkstart();
        talktostart(event[2],event[3]);

        unsigned long tmpevent[4];
        tmpevent[0] =STATUS_SIP_START_TALK;
        tmpevent[1] = 1;
        tmpevent[2] = tmpevent[3]  = 0;
        m_display(tmpevent);
        WRT_MESSAGE("call_sip_start_audio addr =0x%x,0x%x",event[2],event[3]);
}

void CCmdHandler::proxy_call_room(unsigned long event[4])
{
        if(pSystemInfo->mySysInfo.isbomb == USE_BOMB)
                goto BUSY;
        if(pSystemInfo->mySysInfo.hassip == 0)
                return;
        if(pSystemInfo->mySysInfo.isagent == 1 || pSystemInfo->mySysInfo.isavoid == 1 || m_SysStatus  != SYS_IDLE){
                //房间忙
BUSY:
                unsigned long  sipevent[4];
                sipevent[0] = ROOM_BUSY;
                sipevent[1] = sipevent[2] = sipevent[3] = 0;
                udp_send_data2(event[0],sipevent,(char*)event[3]);
                return;
        }
        //应答空闲,需要处理，远程audio  地址 port.
        //播放MP3
        unsigned long  sipevent[4];
        unsigned char  *pSrcid = (unsigned char*)ZENMALLOC(16);
        if(pSrcid){
                memset(pSrcid,0,16);
                strncpy((char*)pSrcid,(char*)event[3],15);
        }
        sipevent[0] = ROOM_IDLE;
        sipevent[1] = sipevent[2] = sipevent[3] = 0;
        udp_send_data2(event[0],sipevent,(char*)event[3]);
        m_sipmediaaddr = event[1];
        m_sipmediaport = event[2];
        m_SysStatus = SYS_SIPCALL;
        sipevent[0] = STATUS_SIP_CALL_ROOM;
        sipevent[1] = (unsigned long)pSrcid;
        m_display(sipevent);

        //响铃
        mp3play((char*)"/home/wrt/zh/ring/ringback.mp3",0,2);
        WRT_MESSAGE("%s call room ",(char*)event[3]);
}


int CCmdHandler::GetIsSimulateTalk()
{
        WRT_MESSAGE("获得当前是否有模拟通话%d %d",g_smalldoor_call_agent,g_simulate_talk);
        if(g_smalldoor_call_agent ==1)
                return 1;
        if(g_simulate_talk ==1)
                return 1;
        return 0;
}

void CCmdHandler::room_manual_agent(int sock_index)
{
#ifdef HAVE_SUB_PHONE
        if(m_SysStatus == SYS_TALKTOMAIN || SYS_ROOMTALKROOM == m_SysStatus) //和主机，门口机通话。
        {

                WRT_MESSAGE("主机呼叫或者户户通呼叫，手动托管到%d,并通知其他从分机，分机手动托管",m_CurrentSocketIndex);
                SendCmd(m_CurrentSocketIndex,ROOM_AGENT);
		if(g_localid == 0x1){
			if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
		        	unsigned long msg_buf[4]={0};
		        	msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
		        	msg_buf[1] = 1;
		        	send_msg_to_sub_phone(msg_buf);
		        	SetTransmitToPhone(0);
			}                                 	
		}

                unsigned long tmpevent1[4];
                tmpevent1[0] = ROOM_USED_AGENT;
                tmpevent1[1] = 0;
                tmpevent1[2] = 0;
                tmpevent1[3] = 0;
                udp_send_data_hangup(tmpevent1);  //通知其他主、从分机   

                room_hangup(-5);  //表示不需要发送挂机指令。             
                return;
        }

        if(m_SysStatus == SYS_SMALLDOORCALL){

                WRT_MESSAGE("小门口呼叫，手动托管到,并通知其他从分机，分机手动托管");
                unsigned long tmpevent1[4] ={0};
                tmpevent1[0] = ROOM_USED_SIMULATE;
                tmpevent1[1] = 0;
                tmpevent1[2] = 1;
                tmpevent1[3] = 4;
                udp_send_data(tmpevent1);  //通知其他主、从分机,通道已经释放。
				if(g_localid == 0x1){
					if(GetTransmitToPhone() == 1){ //如果消息转发到手机，则停止手机
				        	unsigned long msg_buf[4]={0};
				        	msg_buf[0] = CENTER_AND_DOOR_ROOM_HANGUP;
				        	msg_buf[1] = 1;
				        	send_msg_to_sub_phone(msg_buf);
				        	SetTransmitToPhone(0);
					} 
				} 


                g_smalldoor_call_agent = 1; //小门口机呼叫分机，分机托管

                mp3stop();
                add_and_del_timer(TIME_RING_EVENT_STOP);

                if(m_talktimerid != 0){//如果有摘机，取消通话计时
                        add_and_del_timer(TIME_TALK_EVENT_STOP);
                }

                stop_auto_cap_smalldoor_photo();//如果开启了自动抓拍，则停止
                stop_smalldoor_video();//停止小门口视频
                if(m_istalkandsmalldoor == 1)
                        stop_smalldoor_audio(0);

                m_displayevent[0]=STATUS_ROOM_HANGUP;
                m_displayevent[1] = 1;
                m_displayevent[2]=m_displayevent[3]= 0;
                m_display(m_displayevent);
                //通知非可视分机挂机
                if(g_localid == 0x01){
                        unsigned short tmpevent1[4];
                        tmpevent1[0] = INFORM_SIMULATE_HANGUP;
                        tmpevent1[1] = 0;
                        tmpevent1[2] = 0x0; //目标ID
                        tmpevent1[3] = 0 ;
                        send_smalldoor_cmd(tmpevent1); //通知小门口机，分机摘机

                        send_smalldoor_cmd(tmpevent1); //通知小门口机，分机摘机 
                }else{
                        //通知主分机，有其他IP分级挂机了。
                        unsigned long tmpevent1[4];
                        unsigned long tmpip = 0;
                        char tmpid[16];
                        memset(tmpid,0,sizeof(tmpid));
                        memcpy(tmpid,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                        tmpid[13] = '0';
                        tmpid[14] = '1';
                        get_ipbyid2(tmpid,&tmpip);
                        tmpevent1[0] = INFORM_SIMULATE_HANGUP;
                        tmpevent1[1] = 0;
                        tmpevent1[2] = 0; //ID
                        tmpevent1[3] = 0;
                        udp_send_data2(tmpip,tmpevent1);                  	
                } 

                //如果存在多分机，通知其他挂机。             
                tmpevent1[0] = ROOM_USED_AGENT;
                tmpevent1[1] = 0;
                tmpevent1[2] = 0;
                tmpevent1[3] = 0;
                udp_send_data(tmpevent1);  //通知其他主、从分机

                //托管到中心
                {
                        unsigned long tmppack[4];
                        int packlen=0;
                        memset(tmppack,0,sizeof(unsigned long)*4);
                        tmppack[0] = m_centeripaddr;
                        tmppack[1] = (unsigned long)PackCmd(ROOM_CALLCENTER,0,0,&packlen);
                        tmppack[2] = packlen;
                        tmppack[3] = 0;
                        m_pSocketManager->Send_msg(MSG_NODE_REQ_CONNECT,tmppack,sizeof(unsigned long)*4);
                        m_SysStatus = SYS_ROOMCALLCENTERTALK;
                }
                return;
        }
#endif
}



//[added by ljw 2011-5-16 8:52:37]
void CCmdHandler::center_set_room_scr(int sock_index,unsigned char* buf,int buflen)
{
	
	int count = 0;
	unsigned char* tmpbuf = buf;
        count= buf[0];
        int datalen = 0;
         char tmpname[10];
        if(count == 0x08){
        	WRT_DEBUG("中心设置屏保图片");
                count = *(int*)(buf+4);
                if(count > 5){
                	WRT_DEBUG("个数不对%d",count);
                	goto FAILED;
                }
                int i=0;
                int iLen[5] = {0};
                int  offset = 0;
                memcpy(&iLen,buf+8,4*count);
               
                int totallen =0;
                for(i =0;i<count;i++)
                   totallen += iLen[i];
                if(totallen != (buflen-8-4*count)){
                	WRT_DEBUG("总长度不对 %d %d \n",totallen,buflen);
                	goto FAILED;
                }
                delete_all_screen_image();
                tmpbuf = (unsigned char*)(buf+8+4*count);
                for(i=0; i<count;i++){
                        
                        memset(tmpname,0,10);
                        sprintf(tmpname,"screen%d",i);
                        if(save_screen_save_image(tmpbuf+offset,iLen[i],tmpname) == 0){
                        	goto FAILED;
                        }
                        offset += iLen[i];
                        
                }
                //应答成功
                SendCmd(sock_index,ROOM_SCR_ACK,1);
               	m_pSocketManager->CloseSocket(sock_index); 
               	SDL_Delay(count*1000);
				m_displayevent[0] = STATUS_PUBLIC_TEXT_INFO;
				m_displayevent[1] = 1;
				m_displayevent[2] = m_displayevent[3] = 0;
				m_display(m_displayevent);               	
                return;
                
        }else if(count == 0x05){ //第一个96*96的图片
        	if(pSystemInfo->mySysInfo.isuseui == 0x01){
        		goto FAILED;
        	}
				count = *(int*)(buf+4);
            if(count > 1){
            	WRT_DEBUG("个数不对%d",count);
            	goto FAILED;
            }
            memcpy(&datalen,buf+8,4);   
            tmpbuf = (unsigned char*)(buf+8+4);
            memset(tmpname,0,10);
            strcpy(tmpname,"main961");
            if(save_screen_save_image(tmpbuf,datalen,tmpname) == 0){
                    	goto FAILED;
            }
            SendCmd(sock_index,ROOM_SCR_ACK,1);
           	m_pSocketManager->CloseSocket(sock_index); 
           	SDL_Delay(1000);
			m_displayevent[0] = STATUS_PUBLIC_TEXT_INFO;
			m_displayevent[1] = 3;
			m_displayevent[2] = m_displayevent[3] = 0;
			m_display(m_displayevent);                	
           	return;       	
        	
        }else if(count == 0x06){ //第二个800*280的图片
        	if(pSystemInfo->mySysInfo.isuseui == 0x01){
        		goto FAILED;
        	}
 			count = *(int*)(buf+4);
                if(count > 1){
                	WRT_DEBUG("个数不对%d",count);
                	goto FAILED;
                }
                memcpy(&datalen,buf+8,4);   
                tmpbuf = (unsigned char*)(buf+8+4);
                memset(tmpname,0,10);
                strcpy(tmpname,"main280");
                if(save_screen_save_image(tmpbuf,datalen,tmpname) == 0){
                        	goto FAILED;
                }
                SendCmd(sock_index,ROOM_SCR_ACK,1);
               	m_pSocketManager->CloseSocket(sock_index); 
               	SDL_Delay(1000);
		m_displayevent[0] = STATUS_PUBLIC_TEXT_INFO;
		m_displayevent[1] = 2;
		m_displayevent[2] = m_displayevent[3] = 0;
		m_display(m_displayevent);                   	   
               	return;                     	        	
        }else if(count == 0x07){//第三个96*96的图片
        	if(pSystemInfo->mySysInfo.isuseui == 0x01){
        		goto FAILED;
        	}
 		count = *(int*)(buf+4);
                if(count > 1){
                	WRT_DEBUG("个数不对%d",count);
                	goto FAILED;
                }
                memcpy(&datalen,buf+8,4);   
                tmpbuf = (unsigned char*)(buf+8+4);
                memset(tmpname,0,10);
                strcpy(tmpname,"main962");
                if(save_screen_save_image(tmpbuf,datalen,tmpname) == 0){
                        	goto FAILED;
                }
                SendCmd(sock_index,ROOM_SCR_ACK,1);
               	m_pSocketManager->CloseSocket(sock_index); 
               	SDL_Delay(1000);
		m_displayevent[0] = STATUS_PUBLIC_TEXT_INFO;
		m_displayevent[1] = 4;
		m_displayevent[2] = m_displayevent[3] = 0;
		m_display(m_displayevent);
               	return;                        	        	
        }else
        	WRT_DEBUG("数据不对，count= %d ",count);
FAILED:
	 //应答失败 
	 SendCmd(sock_index,ROOM_SCR_ACK,0);
         m_pSocketManager->CloseSocket(sock_index); 
         return;
}
void CCmdHandler::set_holdon_device(int device)
{
	m_holdon_device = device;
}
int CCmdHandler::get_holdon_device()
{
	return m_holdon_device;
}
////////////////////////////////////////////////////////////////////////////////////////////
//

