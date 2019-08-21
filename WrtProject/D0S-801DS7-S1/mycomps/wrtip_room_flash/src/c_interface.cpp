#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include <linux/input.h>  
#include <sys/time.h> 
#include <errno.h>
#include "wrt_log.h"
#include "sock_handler.h"
#include "cmd_handler.h"
#include "c_interface.h"
#include "audiovideo.h"
#include "RoomGpioUtil.h"
#include "warningtask.h"
#include "tmSysConfig.h"
#include "info_list_handler.h"
#include "smalldoor_handler.h" //2010-6-30 11:57:38

extern T_SYSTEMINFO* pSystemInfo;
extern T_ROOM_MORE_CFG  *p_MorecfgInfo;

CSocketManager* m_pSocketManager=NULL;
CCmdHandler*    m_pCmdHandler=NULL;

unsigned long          tid_help; //紧急报警
//extern  void screen_open();
extern void initSocketServer();
extern unsigned char doorbell[46080];

static FILE* OpenGpio(char* dev)
{
	FILE* file = NULL;
	file = fopen(dev,"r");
	if(file == NULL){
		WRT_DEBUG("打开设备%s 失败Error %d",dev,errno);
		return NULL;
	}
	return file;
}


static int ReadGpio(FILE* fp){
	struct input_event ie;  
	if(fp == NULL)
		return 0;
	memset(&ie,0,sizeof(ie));
	fread((void *)&ie, sizeof(ie), 1, fp);  
	if( ie.code  == 294 && ie.value == 1){
		return 1;
	}
	return 0;
}

static int CloseGpio(FILE* fp)
{
	if(fp)
		fclose(fp);
	fp = NULL;
}

extern int ts_calibrate_fun();
extern int g_gpio_fd;


#ifdef __cplusplus
extern "C"
{
#endif

bool get_security_status(int index);

#ifdef __cplusplus
}
#endif

extern bool g_isstartdelay;
extern int IoTestFlag;
extern int IoTestFlagFirst;
extern int IoLevelArray[18];
extern int IoPassArray[18];
static unsigned long validLevel[18]={0};

extern bool  g_is_talk;
extern int g_holdon_flag;
extern int g_phone_or_externdevice_holdon;
extern int g_exit_status;

static void send_msg_toMainprogram()
{
	unsigned long event[4];
	event[0] = STATUS_IOLEVEL_TEST;
	event[1] = 0;
	event[2] = event[3] = 0;

	if(m_pCmdHandler)
	{
		m_pCmdHandler->CallDisplayCallBack(event);
	}
}

static int Gpio_status_task(void * pviod)
{
	int ret = -2;
	int i = 0;
	unsigned int down_time =0;
	unsigned int up_time  =0;
	unsigned int last_ring_time = 0;
	unsigned int cur_ring_time =0;
	unsigned int last_fangchai_time = 0;
	unsigned int cur_fangchai_time = 0;
	int down_flag = 0;
	int diff = 0;
	
	int last_warning_status = GetGpioValue(CTL_IO_GET_EMERGCY);
	int last_outside_warning_status = GetGpioValue(CTL_IO_GET_EMERGCY_OUTSIDE);
	int last_outside_warning_time = 0;
	int last_warning_time = 0;
	int last_sub_hold_status  = 0;
	int last_sub_lock_status  = 0;
	int last_sub_alarm_status = GetGpioValue(CTL_IO_GET_SUB_ALARM);
	int last_sub_alarm_time   = 0;
	int last_doorbell_status = GetGpioValue(CTL_IO_GET_DOORBELL);
	int last_fangchai_status = GetGpioValue(CTL_IO_GET_FANGCHAI);
	int last_fangchai_warning_time = 0;
	int result = -1;
	int alarmtype = 0;
	unsigned long tmp_event[4];
	IoTestFlag = 0;
	IoTestFlagFirst = 0;
	for(i = 0;i<18;i++)
	{
		validLevel[i] = p_MorecfgInfo->validlevel[i];
	}
	while(1)
	{
		SDL_Delay(100);
		
		if(IoTestFlag)//factory test
		{
			if(IoTestFlagFirst)
			{
				SDL_Delay(1200);
				for(i = 0;i<16;i++)
				{
					IoLevelArray[i]  = 0;// GetGpioValue(CTL_IO_GET_ARM_ZONE1+i);
				}
				IoLevelArray[8]  = GetGpioValue(CTL_IO_GET_SUB_ALARM);
				IoLevelArray[9]  = GetGpioValue(CTL_IO_GET_CALLLIFT);
				IoLevelArray[10] = GetGpioValue(CTL_IO_GET_EMERGCY);
				IoLevelArray[11] = !GetGpioValue(CTL_IO_GET_EMERGCY_OUTSIDE);
				IoLevelArray[12] = GetGpioValue(CTL_IO_GET_SUB_LOCK);
				IoLevelArray[13] = GetGpioValue(CTL_IO_GET_SUB_HOLD);
				IoLevelArray[14] = !GetGpioValue(CTL_IO_GET_DOORBELL);
				IoTestFlagFirst = 0;
				for(i = 0;i<15;i++)
					IoPassArray[i] = 0;
				send_msg_toMainprogram();
			}else
			{
				/*for(i = 0;i<8;i++)
				{
					if(IoLevelArray[i] != GetGpioValue(CTL_IO_GET_ARM_ZONE1+i))
					{
						IoLevelArray[i]=!IoLevelArray[i];
						if(IoPassArray[i] == 0)
						{
							IoPassArray[i] = 1;
						}
						send_msg_toMainprogram();
						SDL_Delay(500);
					}
				}*/
				if(IoLevelArray[8] != GetGpioValue(CTL_IO_GET_SUB_ALARM))
				{
					IoLevelArray[8]=!IoLevelArray[8];
					if(IoPassArray[8] == 0)
					{
						IoPassArray[8] = 1;
					}
					//SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,0);
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
				if(IoLevelArray[9] != GetGpioValue(CTL_IO_GET_CALLLIFT))
				{
					IoLevelArray[9]=!IoLevelArray[9];
					if(IoPassArray[9] == 0)
					{
						IoPassArray[9] = 1;
					}
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
				if(IoLevelArray[10] != GetGpioValue(CTL_IO_GET_EMERGCY))
				{
					IoLevelArray[10]=!IoLevelArray[10];
					if(IoPassArray[10] == 0)
					{
						IoPassArray[10] = 1;
					}
					//SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,0);
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
				if(IoLevelArray[11] != !GetGpioValue(CTL_IO_GET_EMERGCY_OUTSIDE))
				{
					IoLevelArray[11]=!IoLevelArray[11];
					if(IoPassArray[11] == 0)
					{
						IoPassArray[11] = 1;
					}
					//SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,0);
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
				if(IoLevelArray[12] != GetGpioValue(CTL_IO_GET_SUB_LOCK))
				{
					IoLevelArray[12]=!IoLevelArray[12];
					if(IoPassArray[12] == 0)
					{
						IoPassArray[12] = 1;
					}
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
				if(IoLevelArray[13] != GetGpioValue(CTL_IO_GET_SUB_HOLD))
				{
					IoLevelArray[13]=!IoLevelArray[13];
					if(IoPassArray[13] == 0)
					{
						IoPassArray[13] = 1;
					}
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
				if(IoLevelArray[14] != !GetGpioValue(CTL_IO_GET_DOORBELL))
				{
					IoLevelArray[14]=!IoLevelArray[14];
					if(IoPassArray[14] == 0)
					{
						IoPassArray[14] = 1;
					}
					//mp3stop();
					//SDL_Delay(100);
					//mp3play((char*)doorbell,sizeof(doorbell),8);
					send_msg_toMainprogram();
					SDL_Delay(500);
				}
			}
		}else //非工厂测试状态
		{
			if(last_outside_warning_time++ > 600000)
				last_outside_warning_time = 0;
			if(last_warning_time++ > 600000)
				last_warning_time = 0;
			if(last_sub_alarm_time++ > 600000)
				last_sub_alarm_time = 0;
			if(last_fangchai_warning_time++ > 600000)
				last_fangchai_warning_time = 0;
			ret = -2;//获取梯控校屏状态
			ret = GetGpioValue(CTL_IO_GET_CALLLIFT);
			//printf("梯控电平 %d\n",ret);
			if(ret == validLevel[9])
			{
				printf("call lift %d\n",ret);
				if(down_flag == 0)
					down_time = SDL_GetTicks();
				SDL_Delay(300);
				down_flag = 1;
				}else if(ret == !validLevel[9])
				{
				if(down_flag == 1)
				{
					up_time = SDL_GetTicks();
					diff = up_time - down_time;
					if(diff>300 && diff <1500)
					{
							printf("电梯控制2\n");
						tmp_event[0] = ROOM_CALL_LIFT;
						tmp_event[1] =  pSystemInfo->LocalSetting.publicroom.Door[0].IP;
						tmp_event[2] = tmp_event[3]=0;
						wrthost_send_cmd(tmp_event);
					}else if(diff > 3000)
					{
							printf("校屏\n");
							//wrthost_send_tscal();
					}

					down_flag = 0;
					down_time = 0;
					up_time = 0;
				}
			}
			SDL_Delay(20);
		ret = -2;//获取外接紧急报警状态
		ret = GetGpioValue(CTL_IO_GET_EMERGCY_OUTSIDE);
		//printf("报警电平 %d\n",ret);
		if(ret == !validLevel[11])
		{
			//printf("外接报警状态 %d\n",ret);
				if(last_outside_warning_status == validLevel[11])
			{
				int cnt = 0;
alarmloop:
					alarmtype = 0;
					if(last_outside_warning_time >15)
					{
						last_outside_warning_time = 0;
				result = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,alarmtype);
                if(result == 0){
                        if(pSystemInfo->mySysInfo.isenglish == 0x01)
                				add_log("Emergency alarm");
                        else
                				add_log("紧急报警成功");
                }else
                {
                	if(cnt++ < 3)
                	{
                		goto alarmloop;
                	}
                }
			}
				}
			last_outside_warning_status = ret ;
		}
		else
			last_outside_warning_status = ret ;
			SDL_Delay(20);
			
		ret = -2;//获取外接门铃状态
		ret = GetGpioValue(CTL_IO_GET_DOORBELL);
		if(ret == !validLevel[14])
		{
			if(last_doorbell_status == validLevel[14])
			{
				cur_ring_time = SDL_GetTicks();
				if(cur_ring_time - last_ring_time > 10000)
				{
					last_ring_time = cur_ring_time;

					mp3play((char*)doorbell,sizeof(doorbell),8);
	            }
			}
            last_doorbell_status = ret;
		}else
		{
			last_doorbell_status = ret;
		}
		ret = -2;//获取外接门铃状态
		ret = GetGpioValue(CTL_IO_GET_FANGCHAI);
		if(ret == 1)
		{
			int fangchai_cnt = 0;
			if(last_fangchai_status == 0)
			{
				if(last_fangchai_warning_time > 15)
				{
					last_fangchai_warning_time = 0;
					if(pSystemInfo->mySysInfo.fangchai_switch == 1)
					{
fangchaialarmloop:
						result = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,100);
		                if(result == 0){
		                    if(pSystemInfo->mySysInfo.isenglish == 0x01)
		            				add_log("Tamper alarm succeed");
		                    else
		            				add_log("防拆报警成功");
		                }else
		                {
		                	if(fangchai_cnt++ < 3)
		                	{
		                		goto fangchaialarmloop;
		                	}
		                }
					}
				}
			}
			last_fangchai_status = ret;
		}else
		{
			last_fangchai_status = ret;
		}
		
			SDL_Delay(5);
			ret = -2;//获取模拟分机摘机 0 
			ret = GetGpioValue(CTL_IO_GET_SUB_HOLD);//13
			if(ret == validLevel[13])
			{
				if(last_sub_hold_status == !validLevel[13]  && (g_holdon_flag == 3))
				{
					if(!g_is_talk){
						if(wrthost_get_holdon_device() != NONE_HOLDON)
							continue;
						g_holdon_flag = 2;
						g_phone_or_externdevice_holdon = 0;
						g_exit_status = 1;
						printf("外挂分机摘机\n");
						unsigned long tmpevent[4];
	                    tmpevent[0] = ROOM_HOLDON;
	                    tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
	                    wrthost_send_cmd(tmpevent);
	                    g_is_talk = !g_is_talk;
	                    wrthost_set_holdon_device(WRTPX_HOLDON);
					}
				}
				last_sub_hold_status = ret;
			}else if(ret == !validLevel[13])
			{
				if((last_sub_hold_status == validLevel[13] ) && (g_holdon_flag == 2))
				{
					if(g_is_talk){
						if(wrthost_get_holdon_device() != WRTPX_HOLDON)
							continue;
						printf("外挂分机挂机\n");
						unsigned long tmpevent[4];
	                    tmpevent[0] = ROOM_HANGUP;
	                    tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
	                    wrthost_send_cmd(tmpevent);
	                    g_holdon_flag = 0;
	                    g_exit_status = 0;
					}
				}
				last_sub_hold_status = ret;
			}
			
			SDL_Delay(5);
			ret = -2;//获取模拟分机开锁 0
			ret = GetGpioValue(CTL_IO_GET_SUB_LOCK);//12
			if(ret == validLevel[12])
			{
				if(last_sub_lock_status == !validLevel[12])
				{
					if(g_is_talk){
						unsigned long tmpevent[4];
	                    tmpevent[0] = ROOM_UNLOCK;
	                    tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
	                    wrthost_send_cmd(tmpevent);
	                }
				}
				last_sub_lock_status = ret;
			}else
			{
				last_sub_lock_status = ret;
			}
			SDL_Delay(5);
			ret = -2;//获取模拟分机报警 1
			ret = GetGpioValue(CTL_IO_GET_SUB_ALARM);//8
			if(ret == validLevel[8])
			{
				if(last_sub_alarm_status == !validLevel[8])
	            {
					int cnt = 0;
	alarmloop1:
					alarmtype = 0;
					if(last_sub_alarm_time > 15)
					{
						last_sub_alarm_time = 0;
	                    result = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,alarmtype);
	                    if(result == 0){
                            if(pSystemInfo->mySysInfo.isenglish == 0x01)
                                    add_log("Emergency alarm");
                            else
                                    add_log("紧急报警成功");
			            }else
			            {
			            	if(cnt++ < 3)
			            	{
			            		goto alarmloop1;
			            	}
			            }
		            }
	            }
	            last_sub_alarm_status = ret;
			}else
				last_sub_alarm_status = ret;
				SDL_Delay(5);
				
			ret = -2;//获取紧急报警状态
			ret = GetGpioValue(CTL_IO_GET_EMERGCY);//10
			if(ret == validLevel[10])
			{
				if(last_warning_status == !validLevel[10])
				{
					int cnt = 0;
	alarmloop2:
					alarmtype = 0;
					if(last_warning_time > 15)
					{
						last_warning_time = 0;
						result = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,alarmtype);
			            if(result == 0){
                            if(pSystemInfo->mySysInfo.isenglish == 0x01)
			                    add_log("Emergency alarm");
                            else
			                    add_log("紧急报警成功");
			            }else
			            {
			            	if(cnt++ < 3)
			            	{
			            		goto alarmloop2;
                    		}	 	      
			            }
                	}
	            }
	            last_warning_status = ret;
			}else
				last_warning_status = ret;

			//for(int k=0;k<8;k++)
			//	ArmZone_process(CTL_IO_GET_ARM_ZONE1+k);
            }
        }
}

/*******************************************************************
函数名称：wrthost_init
函数功能：初始化分机的网络接收和命令分析接口
输入参数：
输出参数：
返回值：
0： 成功
-1： 失败
-2： 之前已经初始化完毕。
********************************************************************/
int wrthost_init()
{

        if(m_pSocketManager)
                return -2;
        if(m_pCmdHandler)
                return -2;

        m_pSocketManager = new CSocketManager();
        if(m_pSocketManager == NULL)
                return -1;

        m_pCmdHandler = new CCmdHandler();
        if(m_pCmdHandler == NULL){
                delete m_pSocketManager;
                m_pSocketManager = NULL;
                return -1;
        }

#if 1         
        JStartVideoProc();
        InitAudioDevice();     
        SDL_Delay(100);
        //PauseAndPlayVideo(0);
#endif
        WRT_DEBUG("m_pSocketManager->StartThread()\n");
        m_pSocketManager->StartThread();    
        
        m_pCmdHandler->StartThread();
      
        m_pSocketManager->SetCmdSink(m_pCmdHandler);
        m_pCmdHandler->SetSocketSink(m_pSocketManager);
        m_pCmdHandler->Start();         
        m_pSocketManager->Start();

#if 1
        SDL_CreateThread(Gpio_status_task,NULL);
#endif
        while(1){
                WRT_MESSAGE("等待命令处理任务和网络任务初始化");
                if(m_pCmdHandler->IsInit() == 1 && m_pSocketManager->IsInit() == 1){
                        WRT_MESSAGE("命令处理任务和网络任务初始化完毕");
                        break;
                }
                SDL_Delay(1000);
        }
        return 0;

}

/*******************************************************************
函数名称：wrthost_init
函数功能：初始化分机的网络接收和命令分析接口
输入参数：
输出参数：
返回值：
0： 成功 
-1： 失败 
-2： 之前已经初始化完毕。
********************************************************************/
void wrthost_uninit()
{
	if(m_pCmdHandler)
		delete m_pCmdHandler;
	if(m_pSocketManager)
		delete m_pSocketManager;
	m_pCmdHandler = NULL;
	m_pSocketManager = NULL;
}

void wrthost_Set_RoomCallRoom_LocalID(char* id){
	if(m_pCmdHandler){
		m_pCmdHandler->SetRoomCallRoomName(id);
	}
}

/*******************************************************************
函数名称：wrthost_send_cmd
函数功能：发送各种命令给对端具体命令可在ctrlqueue.h中查看
输入参数：cmd  具体命令
buf: 命令带的其他参数
buflen: 参数的长度          
输出参数：
返回值：
********************************************************************/
/*void wrthost_send_cmd(short cmd,unsigned char* buf, int buflen)
{
	if(m_pCmdHandler){
		m_pCmdHandler->Send_cmd(cmd,buf,buflen);
	}
}*/

void wrthost_send_cmd(unsigned long event[4]){
        if(m_pCmdHandler){

                m_pCmdHandler->Send_cmd(event);
        }
}

/*******************************************************************
函数名称：wrthost_set_centerip
函数功能：设置中心的IP地址，当主动呼叫中心的时候需要调用该函数
输入参数：ip:为网络IP地址
输出参数：
返回值：
********************************************************************/
void wrthost_set_centerip(unsigned long ip){
        if(m_pCmdHandler){
                m_pCmdHandler->SetCenterIP(ip);
        }
}

void wrthost_get_centerip(unsigned long* ip){
        if(m_pCmdHandler){
                m_pCmdHandler->GetCenterIP(ip);
        }
}

/*******************************************************************
函数名称：wrthost_set_doorip
函数功能：设置主机的IP地址，当主动监视各主机的时候需要调用该函数
输入参数：ip:为网络IP地址
输出参数：
返回值：
********************************************************************/
void wrthost_set_doorip(unsigned long ip){
        if(m_pCmdHandler){
                m_pCmdHandler->SetDoorIP(ip);
        }
}

void wrthost_set_montip(unsigned long ip){
        if(m_pCmdHandler){
                m_pCmdHandler->SetMontIP(ip);
        }	
}

void wrthost_get_doorip(unsigned long* ip){
        if(m_pCmdHandler){
                m_pCmdHandler->GetDoorIP(ip);
        }
}

/*******************************************************************
函数名称：wrthost_set_peer_room_ip
函数功能：设置对端分机的IP地址，当呼叫别的分机的时候需要调用该函数
输入参数：ip:为网络IP地址
输出参数：
返回值：
********************************************************************/
void wrthost_set_peer_room_ip(unsigned long ip[4])
{
        if(m_pCmdHandler){
                m_pCmdHandler->SetPeerRoomIP(ip);
        }
}

void wrthost_get_peer_room_ip(unsigned long* ip)
{
        if(m_pCmdHandler){
                m_pCmdHandler->GetPeerRoomIP(ip);
        }
}

/*******************************************************************
函数名称：wrthost_set_displaycallback
函数功能：设置显示状态的回调函数，当用户收到命令或者处理各种命令的时候
其处理状态，将通过该回调函数反映给调用者。
回调函数的实现最好不要是阻塞的。
输入参数：p_display：回调函数指针
输出参数：
返回值：
********************************************************************/
void wrthost_set_displaycallback(display  p_display){
        if(m_pCmdHandler){
                m_pCmdHandler->SetDisplayCallback(p_display);
        }
}

void wrthost_pause_play_video(int status)
{
        return;
        PausePlayVideo(status);
}

void wrthost_debug(int type){
        if(type == 1){
                m_pSocketManager->socket_printf();
        }
}

void wrthost_get_managerip(unsigned long *ip){
        if(m_pCmdHandler){
                m_pCmdHandler->GetManagerIP(ip);
        }
}
void wrthost_set_managerip(unsigned long ip){
        if(m_pCmdHandler){
                m_pCmdHandler->SetManagerIP(ip);
        }
}

void wrthost_get_garageip(unsigned long *ip){
        if(m_pCmdHandler){
                m_pCmdHandler->GetgarageIP(ip);
        }    
}
void wrthost_set_garageip(unsigned long ip){
        if(m_pCmdHandler){
                m_pCmdHandler->SetgarageIP(ip);
        }        
}


void wrthost_req_sync(unsigned long ip){
        if(m_pCmdHandler){
                m_pCmdHandler->room_req_sync(ip);
        }     
}

void wrthost_syssetbusy()
{
	if(m_pCmdHandler){
                m_pCmdHandler->SetCurrentSysBusy();
        }
}

void wrthost_syssetidle()
{
	if(m_pCmdHandler){
                m_pCmdHandler->SetCurrentSysIdle();
        }
}

void wrthost_send_tscal()
{
	unsigned long event[4];
	event[0] = STATUS_TS_CAL;
	event[1] = 0;
	event[2] = event[3] = 0;
	wrthost_syssetbusy();
	if(m_pCmdHandler)
	{
		m_pCmdHandler->CallDisplayCallBack(event);
	}
}

int wrthost_sysidle(){
        if(m_pCmdHandler){
                if(m_pCmdHandler->GetCurrentSysStatus() == SYS_IDLE)
                        return 1;
        }
        return 0;
}

int wrthost_get_sys_status()
{
        return m_pCmdHandler->GetCurrentSysStatus();
}

void wrthost_initWatchDog(){
        init_16safe();
}

/*******************************************************************
*******************************************************************/

void StartGpio(){
        RoomGpioUtilOpen();
}

int  GetGpioSignal(){
        return RoomGpioUtilStart();
}

void StopGpio(){
        RoomGpioUtilClose();
}

int get_host_ip(unsigned long* ip){
        if(m_pCmdHandler){
                return m_pCmdHandler->GetHostIP(ip);
        }
        return 0;
}
int get_host_mask(unsigned long*  mask){
        if(m_pCmdHandler){
                return m_pCmdHandler->GetHostMask(mask);
        }
        return 0;
}
int get_host_gateway(unsigned long* gateway){
        if(m_pCmdHandler){
                return m_pCmdHandler->GetHostGateWay(gateway);
        }
        return 0;
}

int set_host_ip(unsigned long ip){
        if(m_pCmdHandler){
                return m_pCmdHandler->SetHostIP(ip);
        }
        return 0;
}

int set_host_mask(unsigned long mask){
        if(m_pCmdHandler){
                return m_pCmdHandler->SetHostMask(mask);
        }
        return 0;	
}

int set_host_gateway(unsigned long gateway){
        if(m_pCmdHandler){
                return m_pCmdHandler->SetHostGateWay(gateway);
        }
        return 0;
}

int save_modify(){
        if(m_pCmdHandler){
                return m_pCmdHandler->SaveModify();
        }
        return 0;

}

void SetRingtimes(unsigned long times){
        if(m_pCmdHandler){
                return m_pCmdHandler->SetRingtimes(times);
        }        
}
unsigned long GetRingtimes(){
        if(m_pCmdHandler){
                return m_pCmdHandler->GetRingtimes();
        }               
}

void SetAvoidtimes(unsigned long hours,unsigned long min){
        if(m_pCmdHandler){
                return m_pCmdHandler->SetAvoidtimes(hours,min);
        }          
}

void GetAvoidtimes(unsigned long * hours,unsigned long* min){
        if(m_pCmdHandler){
                return m_pCmdHandler->GetAvoidtimes(hours,min);
        }              
}

void SetAgenttimes(unsigned long hours,unsigned long min){
        if(m_pCmdHandler){
                return m_pCmdHandler->SetAgenttimes(hours,min);
        }          
}

void GetAgenttimes(unsigned long * hours,unsigned long* min){
        if(m_pCmdHandler){
                return m_pCmdHandler->GetAgenttimes(hours,min);
        }              
}

void wrtip_play_voice(int index){
        if(m_pCmdHandler)
                m_pCmdHandler->playvoice(index);
}
void wrtip_default_set_ring()
{
	if(m_pCmdHandler)
    	m_pCmdHandler->SetDefaultRing();
}
void wrtip_set_ring(int type, char* addr,int size){
        switch(type){
        case CENTER_CALL_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetCenterCallRing((char*)addr,size);
                break;
        case WALL_CALL_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetWallCallRing((char*)addr,size);
                break;
        case DOOR_CALL_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetDoorCallRing((char*)addr,size);
                break;
        case SMALL_DOOR_CALL_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetSmallDoorCallRing((char*)addr,size);
                break;
        case ROOM_CALL_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetRoomCallRing((char*)addr,size);
                break;
        case WARNING_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetWarningRing((char*)addr,size);
                break;
        case MSG_RING:
                if(m_pCmdHandler)
                        m_pCmdHandler->SetMsgRing((char*)addr,size);
                break;
        case ALARM_CLOCK_RING:
        	 if(m_pCmdHandler)
                        m_pCmdHandler->SetAlarmClockRing((char*)addr,size);
                 break;
        }
}

int wrtip_is_config_ring(char* addr,int size){
        int i =0;
        int tmplen = 0;
        char* tmpaddr = NULL;
        for(i =0;i<8;i++){
                if(m_pCmdHandler){
                        tmpaddr = NULL;
                        tmplen = 0;
                        tmpaddr = m_pCmdHandler->GetRingConfig(i,&tmplen);
                        if(strcmp(tmpaddr,addr) == 0)
                                return 1;
                }
        }
        return 0;
}
/*
函数功能:设置摘机对象给变量，保存当前是谁摘机，用于判断挂机对象挂机是否有效
输入参数:device 0-6，见头
输出参数:无
返回值:无
*/
void wrthost_set_holdon_device(int device)
{
	if(m_pCmdHandler)
		m_pCmdHandler->set_holdon_device(device);
}
/*
函数功能:获取当前摘机情况
输入参数:device 0-6，见头
输出参数:无
返回值:当前摘机情况，是否摘机，谁摘机的
*/
int wrthost_get_holdon_device()
{
	if(m_pCmdHandler){
		return m_pCmdHandler->get_holdon_device();
	}
	return 0;
}

void wrthost_config_networking()
{
	if(pSystemInfo->LocalSetting.privateinfo.LocalIP!= 0 && pSystemInfo->LocalSetting.privateinfo.LocalIP!= 0xffffffff)
			m_pSocketManager->ModifyHostIpAndMask(pSystemInfo->LocalSetting.privateinfo.LocalIP,0);
	if(pSystemInfo->LocalSetting.privateinfo.SubMaskIP != 0 &&
			pSystemInfo->LocalSetting.privateinfo.SubMaskIP != 0xffffffff)
			m_pSocketManager->ModifyHostIpAndMask(0,pSystemInfo->LocalSetting.privateinfo.SubMaskIP);
	if(pSystemInfo->LocalSetting.privateinfo.GateWayIP != 0 &&
			pSystemInfo->LocalSetting.privateinfo.GateWayIP != 0xffffffff )
			m_pSocketManager->ModifyHostGateWay(pSystemInfo->LocalSetting.privateinfo.GateWayIP);
}

