#ifndef _C_INTERFACE_H_
#define _C_INTERFACE_H_

#include "ctrlcmd.h"

//2015-2-5 wyx
//增加定义用于区别谁呼叫分机
enum{
	TYPE_DOOR_CALLME = 0,
	TYPE_MANAGER_CALLME,
	TYPE_WALL_CALLME,
	TYPE_ROOM_CALLME,
	TYPE_IPDOOR_CALLME,
};
//增加定义用于区别谁摘机对讲 1-对方摘机 2-分机 3-按键 4-PX 5-手机 6-多分机 
enum{
	NONE_HOLDON = 0,
	REMOTE_HOLDON,
	LOCAL_HOLDON,
	PHYKEY_HOLDON,
	WRTPX_HOLDON,
	IOSDEV_HOLDON,
	MOREROOM_HOLDON,
};
#if defined(__cplusplus)
extern "C" {
#endif

        /***********************************************************
        接口说明：
        1：display 为状态显示函数指针，当前处于什么样的通信状态，我
        将通过参数cmd反馈给调用者，该函数的最好不要阻塞。
        2：调用循序：
        wrthost_init();
        wrthost_send_cmd();
        //主动呼叫中心，调用该函数设置IP。
        // wrthost_set_centerip();//以后不用这些接口了
        //监视主机，调用该函数设置IP。
        //   wrthost_set_doorip(); //以后不用这些接口了
        wrthost_set_displaycallback();
        //程序退出的时候调用
        wrthost_uninit（）。
        3：例子：
        请看test_wrtip_host.c。          
        ***********************************************************/


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
        int  wrthost_init();

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
        void wrthost_uninit();

        /*******************************************************************
        函数名称：wrthost_send_cmd
        函数功能：发送各种命令给对端具体命令可在ctrlqueue.h中查看
        输入参数：cmd  具体命令
        buf   命令的其他参数
        buflen 长度
        输出参数：
        返回值：
        ********************************************************************/
        //void wrthost_send_cmd(short cmd,unsigned char* buf, int buflen);
        void wrthost_send_cmd(unsigned long event[4]);

        /*******************************************************************
        函数名称：wrthost_set_centerip
        函数功能：设置中心的IP地址，当主动呼叫中心的时候需要调用该函数
        输入参数：ip:为网络IP地址
        输出参数：
        返回值：
        说明：    暂时没用
        ********************************************************************/
        void wrthost_set_centerip(unsigned long ip);

        void wrthost_get_centerip(unsigned long* ip);

        /*******************************************************************
        函数名称：wrthost_set_doorip
        函数功能：设置主机的IP地址，当主动监视各主机的时候需要调用该函数
        输入参数：ip:为网络IP地址
        输出参数：
        返回值：
        说明：    作为设置监视点IP的接口来使用
        ********************************************************************/
        void wrthost_set_doorip(unsigned long ip);

        void wrthost_get_doorip(unsigned long* ip);

        void wrthost_set_montip(unsigned long ip);

        void wrthost_get_managerip(unsigned long *ip);
        void wrthost_set_managerip(unsigned long ip);

        void wrthost_get_garageip(unsigned long *ip);
        void wrthost_set_garageip(unsigned long ip);

        int wrthost_sysidle();

        int wrthost_get_sys_status();

        void wrthost_initWatchDog();

        /*******************************************************************
        函数名称：wrthost_set_peer_room_ip
        函数功能：设置对端分机的IP地址，当呼叫别的分机的时候需要调用该函数
        输入参数：ip:为网络IP地址
        输出参数：
        返回值：
        说明：    暂时没用
        ********************************************************************/
        void wrthost_set_peer_room_ip(unsigned long ip[4]);
        void wrthost_get_peer_room_ip(unsigned long* ip);

        /*******************************************************************
        函数名称：wrthost_set_displaycallback
        函数功能：设置显示状态的回调函数，当用户收到命令或者处理各种命令的时候
        其处理状态，将通过该回调函数反映给调用者。
        回调函数的实现最好不要是阻塞的。
        输入参数：p_display：回调函数指针
        输出参数：
        返回值：  
        ********************************************************************/
        void wrthost_set_displaycallback(display  p_display);

        void wrthost_pause_play_video(int status);


        void wrthost_Set_RoomCallRoom_LocalID(char* id);


        void wrthost_req_sync(unsigned long ip);
		void wrthost_syssetbusy();
		void wrthost_syssetidle();
		void wrthost_send_tscal();
        /*******************************************************************
        DEBUG
        type 1: socket res.
        2: memory  res.
        ********************************************************************/
        void wrthost_debug(int type);

        //////////////////////////////////////////////////////////
        void StartGpio();
        int  GetGpioSignal();
        void StopGpio();
        /////////////////////////////////////////////////////////

        int get_host_ip(unsigned long* ip);
        int get_host_mask(unsigned long*  mask);
        int get_host_gateway(unsigned long* gateway);

        int set_host_ip(unsigned long ip);
        int set_host_mask(unsigned long mask);
        int set_host_gateway(unsigned long gateway);

        int save_modify();


        void SetRingtimes(unsigned long times);
        unsigned long GetRingtimes();
        void SetAvoidtimes(unsigned long hours,unsigned long min);
        void GetAvoidtimes(unsigned long * hours,unsigned long* min);
        void SetAgenttimes(unsigned long hours,unsigned long min);
        void GetAgenttimes(unsigned long * hours,unsigned long* min);
        void wrtip_play_voice(int index);

		void wrtip_default_set_ring();
        void wrtip_set_ring(int type,char* addr,int size);
        int wrtip_is_config_ring(char* addr,int size);
		void wrthost_set_holdon_device(int device);
		int wrthost_get_holdon_device();
		void wrthost_config_networking();
#if defined (__cplusplus)
}
#endif
#endif 