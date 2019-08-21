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


//MAC SN����
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


    //��������
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


    //�����ͷֻ����е�
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


    //DHCP���ݣ����ڷֻ����ظ��ſڻ������ĵ�IP�������������ظ�����IP��
    typedef struct{
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
        unsigned long MainDoorIP;//���ſڻ�IP���ṩDHCP����
        unsigned long MainCenterIP;//����IP���ṩDHCP����
    }T_DHCP_INFO;


    //�ֻ����е�
    typedef struct {
        //Gate
        unsigned long NumOfGate;
        T_DEV_ITEM MainGate[32];
        //Door
        unsigned long NumOfDoor;
        T_DEV_ITEM Door[32];

        //ά�޺ͷ�����Ŀ
        unsigned long NumOfRepaire;
        char  NameOfRepaire[12][20];
        unsigned long NumOfFee;
        char  NameOfFee[12][20];
    }T_PUBLIC_ROOM;



    //����˽�е�
    typedef struct{
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
    }T_PRIVATE_DOOR, T_DEV_BASE_INFO;



    //�ֻ�˽�е�
    typedef struct{
        unsigned char LocalID[16];
        unsigned long LocalIP;
        unsigned long SubMaskIP;
        unsigned long GateWayIP;
        //��չ�ֻ�
        unsigned long extensionipaddr[3];
        //Moni
        unsigned long NumOfMONI;
        T_DEV_ITEM Moni[8];
        //SmallDoor
        unsigned long NumOfSmallDoor;
        T_DEV_ITEM SmallDoor[8];
    }T_PRIVATE_ROOM;


    //�ֻ���
    typedef struct{
        T_PRIVATE_ROOM       privateinfo;
        T_PUBLIC_ALL         publicinfo;
        T_PUBLIC_ROOM        publicroom;
    }T_LOCALCONFIG_ROOM;

#if _ROOM_SET_

    typedef struct {

        //int            localhost_type;
        //char           extensionipaddr[3][16];    //��������ֻ��ʹ����������ֻ���IP������Ǵӷֻ�0�оʹ����ֻ�IP����������Ϊ0
        //�Ҿ�
        //int            localhost_485addr ;    //ͬID��2λ
        char  			sysID[16];             //�Ҿ����ص�ID
        char            gatewaypwd[16];          //Ϊ���ж��龰Ĭ�����Ƿ��һ�η��͡�
        unsigned long   gatewayip;
        //��������
        long           bright;
        long           contrast;
        long           saturation;
        int            ringvolume;           //����������
        long           ringtimes;            //����ʱ��
        unsigned long  avoidtimes;           //����ŵ�ʱ�䳤�ȣ���λ��
        int            safestatus[32];      //32�����Ļ���״̬
        char           ringname[8][256];      //����������
        int            isavoid  ;          //�Ƿ����������
        int            isagent;           //�Ƿ������й�
        unsigned long   agenttimes;       //�йܵ�ʱ��
        int            isbuttonvoice;      //�Ƿ��а�����
        unsigned char  TouchScreenPara[24];
        char           password[8];
        char           superpassword[8];

        //ģ������
        int            isjiaju;
        int            iskeepscreen;       //�Ƿ������
        int            isusedffs;           //�Ƿ�ʹ���ļ�ϵͳ
        int            ishttpdown;          //�Ƿ�ʹ��HTTP��������

        //��������Ĭ������
        unsigned long NumOfMp3;
        unsigned long Mp3Position[5];
        unsigned long Mp3Len[5];
        //������Ϣ
        unsigned long HelpPosition;
        unsigned long HelpLen;

        //����
        unsigned long idrule[5];
#if defined (ISROOM)
        unsigned long isautocap;    //�Ƿ��Զ�ץ��
        unsigned long rvalue;
        unsigned long gvalue;
        unsigned long bvalue;
        unsigned long talkvolume;   //ͨ������
        unsigned long alarmtime;    //������ʱʱ��
        unsigned long alarmtime1; //������ʱʱ��
        unsigned long alarmvolume;  //��������
        unsigned long isrecord; //�Ƿ�¼��
        unsigned long isremote; //�Ƿ�ر�Զ�̿���
        unsigned long alarmvoicetime; //������������ʱ��
        unsigned short maxphoto;
        unsigned short photoqualit;
        unsigned long  bakliftip; //���õ���IP		
        unsigned char  isnvo;
        unsigned char  framerate;
		unsigned char  hassip;
		unsigned char  isenglish;
		unsigned long  sipproxyip;
		char           phonenumber[8][15];
        unsigned char  isbomb;
        unsigned char  isdoorbell;
        unsigned char  screensize;
        unsigned char  isuseui;   //0x1:��һ��UI,0x02: �ڶ���UI,�������ٲ�ͬ��UI֮���л����ơ�
        unsigned char  scenenumber[10]; //����龰��
        unsigned char  cur_scene; //���� �ĳɱ��浱ǰ�龰��
        unsigned char  wifi_dhcp;//����
        char  gatewayroomid[16];//���طֻ�id
        unsigned long  micvolume;
        unsigned short maxvideo;
        unsigned short videoqualit;
        unsigned long  delay_ms;
        unsigned long  doorcallproir;
		unsigned long  validlevel[18];
		unsigned long  usecenternegotiate;//�Ƿ����ú�������
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
	
		unsigned char  smartgatewayid[12];//��������id
		unsigned char  fangchai_switch;
		unsigned char  wifi_switch;
		unsigned char  chengfangcnt;
		unsigned char  chengfangtime;
		unsigned long  reserved4vvv[12];
		unsigned long  reserved5[16];
		unsigned long  reserved6[16];
        // unsigned long reserved0[3]; //�������reserved0[0] ��Ϊת��IP��ַ��reserved0[1]��Ϊ�й�IP��ַ,
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



    //������
    typedef struct{
        T_PRIVATE_DOOR       privateinfo;
        T_PUBLIC_ALL         publicinfo;
    }T_LOCALCONFIG_DOOR;

#if (_DOOR_SET_||_GATE_SET_||_IP_BRIDGE_)

    typedef struct {

    }T_MYSYSINFO_DOOR;
#endif


    //===========================================================
    // ���ķ���������
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
    //(����������)���͵��ֻ���������Ϣ��T_LOCALCONFIG_ROOM
    //
    //���͵����ſڻ���������Ϣ��T_CENTER_TO_DOOR+�ֻ�����(�������ֻ�)+T_PRIVATE_ROOM[]+T_ROOM_ID_TABLE[]
    //
    //���͵����ſڻ���������Ϣ��T_CENTER_TO_DOOR+�ֻ�����(�������ֻ�)+T_ROOM_ID_TABLE[]
    //
    //���͵�Χǽ����������Ϣ��T_CENTER_TO_DOOR+�ֻ�����+T_ROOM_ID_TABLE[]
    //
    //���͵�IP��������������Ϣ��T_CENTER_TO_DOOR
    //
    //======================================================================


    //
    int write_flash(unsigned short *addr, unsigned long length, unsigned long toaddr);

    //
    void read_flash(unsigned long  srcAddr, unsigned char*  pDst, unsigned long numBytes);

    //
    void erase_flash(unsigned long startsec, unsigned long endsec);


    void InitSystemInfo(); //�ڶ������У�ͬ������
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
	void InitSystemMoreInfo(); //�ڶ������У�ͬ������
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

