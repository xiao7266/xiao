/******************************************************************************

File name     : tmSysConfig.h
Author        : l
Version       : 1.0
Date          :
Others        :
Function List :

History:

******************************************************************************/

#ifndef _SYSTEM_CONFIG_H
#define _SYSTEM_CONFIG_H

//#include <tmNxTypes.h>
//#include "..\..\comps_cfg\comps_config.h"


#define _ROOM_SET_  1



#undef _DOOR_SET_
#undef _GATE_SET_
#undef _IP_BRIDGE_


//MAC SN配置
typedef struct
{
	 unsigned char flag;   //0:need get 1:have get
	 unsigned char	sn[32];
	 unsigned char	 mac[6];
}T_MACINFO;

typedef struct 
{
	char head[4];//WRTI    
    int len;
    unsigned short uCmd;
    char srcid[15];
    char dstid[15];
   // union DataBuf
   // {
       char DATA[64];
   // }dbMsg;
}mNetPack;
#ifdef __cplusplus
extern "C" {
#endif

    /*****************************************************
    *
    *
    *****************************************************/

#pragma pack(1)

#if _ROOM_SET_
#define T_LOCALCONFIG_ROOM T_LOCALCONFIG
#define T_MYSYSINFO_ROOM   T_MYSYSINFO
#endif

#if (_DOOR_SET_||_GATE_SET_||_IP_BRIDGE_)
#define T_LOCALCONFIG_DOOR T_LOCALCONFIG
#define T_MYSYSINFO_DOOR   T_MYSYSINFO
#endif


    typedef struct
    {
        unsigned long IP;
        char name[20];
    }T_DEV_ITEM;


    //启动配置
    typedef struct
    {
        char SN[32];
        char Version[10];
        unsigned char MAC[6];
        unsigned long program_position;
        unsigned long program_size;
        unsigned long Font24Position;
        unsigned long Font24Len;
        unsigned long Font16Position;
        unsigned long Font16Len;
    }T_BOOTINFO;


    //主机和分机共有的
    typedef struct {
        unsigned long CenterMajorIP;
        unsigned long CenterMinorIP;
        unsigned long ManagerMajorIP;
        unsigned long ManagerMinorIP;
        unsigned long CenterWarningIP;
        unsigned long CenterMenjinIP;
        unsigned long CenterFileIP;
        unsigned long CenterNetIP;
    }T_PUBLIC_ALL;


    //DHCP数据：对于分机，回复门口机和中心的IP；对于主机，回复中心IP。
    typedef struct{
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
        unsigned long MainDoorIP;//主门口机IP，提供DHCP服务
        unsigned long MainCenterIP;//中心IP，提供DHCP服务
    }T_DHCP_INFO;


    //分机共有的
    typedef struct {
        //Gate
        unsigned long NumOfGate;
        T_DEV_ITEM MainGate[32];
        //Door
        unsigned long NumOfDoor;
        T_DEV_ITEM Door[32];

        //维修和费用项目
        unsigned long NumOfRepaire;
        char  NameOfRepaire[12][20];
        unsigned long NumOfFee;
        char  NameOfFee[12][20];
    }T_PUBLIC_ROOM;



    //主机私有的
    typedef struct{
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
    }T_PRIVATE_DOOR, T_DEV_BASE_INFO;



    //分机私有的
    typedef struct{
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
        //扩展分机
        unsigned long extensionipaddr[3];
        //Moni
        unsigned long NumOfMONI;
        T_DEV_ITEM Moni[8];
        //SmallDoor
        unsigned long NumOfSmallDoor;
        T_DEV_ITEM SmallDoor[8];
    }T_PRIVATE_ROOM;


    //分机的
    typedef struct{
        T_PRIVATE_ROOM       privateinfo;
        T_PUBLIC_ALL         publicinfo;
        T_PUBLIC_ROOM        publicroom;
    }T_LOCALCONFIG_ROOM;

#if _ROOM_SET_

    typedef struct {

        //int            localhost_type;
        //char           extensionipaddr[3][16];    //如果是主分机就存其他三个分机的IP，如果是从分机0中就存主分机IP，其他两个为0
        //家居
        //int            localhost_485addr ;    //同ID后2位
        char  			sysID[16];             //家居网关的ID
        char            gatewaypwd[16];          //为了判断情景默哀是是否第一次发送。
        unsigned long   gatewayip;
        //本机设置
        long           bright;
        long           contrast;
        long           saturation;
        int            ringvolume;           //铃声的音量
        long           ringtimes;            //响铃时间
        unsigned long  avoidtimes;           //免打扰的时间长度，单位秒
        int            safestatus[32];      //32防区的基本状态
        char           ringname[8][256];      //八种铃声的
        int            isavoid  ;          //是否设置免打扰
        int            isagent;           //是否设置托管
        unsigned long   agenttimes;       //托管的时间
        int            isbuttonvoice;      //是否有按键音
        unsigned char  TouchScreenPara[24];
        char           password[8];
        char           superpassword[8];

        //模块配置
        int            isjiaju;
        int            iskeepscreen;       //是否打开屏保
        int            isusedffs;           //是否使用文件系统
        int            ishttpdown;          //是否使用HTTP下载数据

        //铃声采用默认名称
        unsigned long NumOfMp3;
        unsigned long Mp3Position[5];
        unsigned long Mp3Len[5];
        //帮助信息
        unsigned long HelpPosition;
        unsigned long HelpLen;

        //保留
        unsigned long idrule[5];
#if defined (ISROOM)
        unsigned long isautocap;    //是否自动抓拍
        unsigned long rvalue;
        unsigned long gvalue;
        unsigned long bvalue;
        unsigned long talkvolume;   //通话音量
        unsigned long alarmtime;    //布防延时时间
        unsigned long alarmtime1; //撤防延时时间
        unsigned long alarmvolume;  //报警音量
        unsigned long isrecord; //是否录音
        unsigned long isremote; //是否关闭远程控制
        unsigned long alarmvoicetime; //报警声音持续时间
        unsigned short maxphoto;
        unsigned short photoqualit;
        unsigned long  bakliftip; //备用电梯IP		
        unsigned char  isnvo;
        unsigned char  framerate;
		unsigned char  hassip;
		unsigned char  isenglish;
		unsigned long  sipproxyip;
		char           phonenumber[8][15];
        unsigned char  isbomb;
        unsigned char  isdoorbell;
        unsigned char  screensize;
        unsigned char  isuseui;   //0x1:第一版UI,0x02: 第二版UI,用来做再不同版UI之间切换控制。
        unsigned char  scenenumber[10]; //存放情景号
        unsigned char  cur_scene; //备用 改成保存当前情景号
        unsigned char  wifi_dhcp;//备用
        char  gatewayroomid[16];//网关分机id
        unsigned long  micvolume;
        unsigned short maxvideo;
        unsigned short videoqualit;
        unsigned long  delay_ms;
        unsigned long  doorcallproir;
		unsigned long  validlevel[18];
		unsigned long  usecenternegotiate;//是否是用呼叫优先
//wyx add 2014-11-11 
		unsigned long  wifiip;
		unsigned long  wifimask;
		unsigned long  wifigateway;
		unsigned long  wifidns;
		unsigned char  wifissid[16];
		unsigned char  wifipwd[16];
		unsigned char  wifiauthmode[8];
		unsigned char  wifiencryptype[8];
//end 4*4+16*2 = 48 bytes;
		//unsigned long  reserved3[4];//last ver 16
	
		unsigned char  smartgatewayid[12];//智能网关id
		unsigned char  fangchai_switch;
		unsigned char  wifi_switch;
		unsigned char  chengfangcnt;
		unsigned char  chengfangtime;
		unsigned long  reserved4vvv[12];
		unsigned long  reserved5[16];
		unsigned long  reserved6[16];
        // unsigned long reserved0[3]; //管理机中reserved0[0] 做为转接IP地址，reserved0[1]作为托管IP地址,
#elif defined (ISMANAGE)
        unsigned long avertip;
        unsigned long agentip;
        unsigned char hasrecvalarm;
        unsigned char resettime1;
        unsigned char resettime2;
        unsigned char isenglish;
        unsigned char  isbomb;
        unsigned char  talkvolume;
        unsigned char  bak2;
        unsigned char  bak3;
        unsigned long reserved0[7];
        unsigned long reserved1[16];
        unsigned long reserved2[16];
        unsigned long reserved3[16];        
#else
        unsigned long reserved0[11];
        unsigned long reserved1[16];
        unsigned long reserved2[16];
        unsigned long reserved3[16];        
#endif

    }T_MYSYSINFO_ROOM;

typedef struct _room_morecfg
{
	unsigned long validlevel[18];
	unsigned long reserverd0[6];
	unsigned long reserverd1[48];
	unsigned long reserverd2[128];
}T_ROOM_MORE_CFG;
#endif



    //主机的
    typedef struct{
        T_PRIVATE_DOOR       privateinfo;
        T_PUBLIC_ALL         publicinfo;
    }T_LOCALCONFIG_DOOR;

#if (_DOOR_SET_||_GATE_SET_||_IP_BRIDGE_)

    typedef struct {

    }T_MYSYSINFO_DOOR;
#endif


    //===========================================================
    // 中心发给主机的
    typedef struct{
        T_PRIVATE_DOOR       privateinfo;
        T_PUBLIC_ALL         publicinfo;
        T_PUBLIC_ROOM        publicroom;
    }T_CENTER_TO_DOOR;

    //IP table
    typedef struct{
        unsigned char dev_ID[16];
        unsigned long dev_IP;
    }T_ROOM_ID_TABLE;

    //---------------------------------------------------------
    typedef struct
    {
        T_BOOTINFO      BootInfo;
        T_LOCALCONFIG   LocalSetting;
        T_MYSYSINFO     mySysInfo;
    }T_SYSTEMINFO;

    //======================================================================
    //(主机或中心)发送到分机的配置信息：T_LOCALCONFIG_ROOM
    //
    //发送到主门口机的配置信息：T_CENTER_TO_DOOR+分机个数(包括副分机)+T_PRIVATE_ROOM[]+T_ROOM_ID_TABLE[]
    //
    //发送到副门口机的配置信息：T_CENTER_TO_DOOR+分机个数(包括副分机)+T_ROOM_ID_TABLE[]
    //
    //发送到围墙机的配置信息：T_CENTER_TO_DOOR+分机个数+T_ROOM_ID_TABLE[]
    //
    //发送到IP适配器的配置信息：T_CENTER_TO_DOOR
    //
    //======================================================================


    //
    int write_flash(unsigned short *addr, unsigned long length, unsigned long toaddr);

    //
    void read_flash(unsigned long  srcAddr, unsigned char*  pDst, unsigned long numBytes);

    //
    void erase_flash(unsigned long startsec, unsigned long endsec);


    void InitSystemInfo(); //在多任务中，同步处理。
    void UninitSystemInfo();

    void EnterMutex();
    void ExitMutex();


    //
    T_SYSTEMINFO* GetSystemInfo();

    //
    void FlushSystemInfo();

    void UpdateSystemInfo();
    void printf_sys_info();
    
    void SetDefaultSystemInfo();
	void getmacsnFileFlag(char *flag);
	
	void setRoomSN(char *sSn);
	void setRoomMAC(char *sMac);
	void getmacsnFileMAC(char *sMac);
	void getmacsnFileSN(char *sSn);

	static void read_cfg_file();
	static void write_cfg_file();
	int severce_macsnfile(void *pParam);
	int task_macsnfile(void *pParam);

//add by wyx 2014-5-30  start
	void InitSystemMoreInfo(); //在多任务中，同步处理。
	void UninitSystemMoreInfo();
	void EnterMutexMore();
	void ExitMutexMore();
	T_ROOM_MORE_CFG * GetSystemMoreInfo();
	static void read_Morecfg_file();
	void write_Morecfg_file();
#pragma pack()

#ifdef __cplusplus
}
#endif

#endif/* end of #ifndef _SYSTEM_CONFIG_H */

