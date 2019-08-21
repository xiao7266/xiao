/************************************************************************************
�޸ļ�¼��
2008.4.7  ���  �������Զ��˳���ʱ�����OSD�ĵڶ��㡣 (ProcessEvent)
2008.6.23          �����Ļ��������(main.cpp,Main_program.cpp)
2008.6.18          ���ʱ�䴦��.(modifyipandmask.cpp)
2008.6.23          ���������Ź�(main.cpp)
2008.11.12         �յ�����Ϣʱ��û��ˢ��������BUG.
2008.12.24         �����������������˵��������ͨ��ʱ������������ȡ�
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>	

#include "wrtfont.h"
#include "SDL_draw.h"
#include "Handler.h"
#include "audiovideo.h"
#include "main_program.h"
#include "c_interface.h"
#include "jpeg.h"
//#include "zlib.h"
#include "info_list_handler.h"
#include "window.h"
#include "public_menu_res.h"
#include "c_interface.h"
#include "weather.h"
#include "audiovideo.h"
#include "warningtask.h"
#include "RoomGpioUtil.h"
	

#define MYFIFO  "/tmp/myfifo"	/* �����ܵ��ļ���*/

#define MAX_BUFFER_SIZE		PIPE_BUF 	/*��������limits.h��*/
static int g_fifo_fd = -1;

//#include "SDL_rotozoom.h"


#ifdef WRT_MORE_ROOM
#include "RoomGpioUtil.h"
extern unsigned char doorbell[46080];
#endif
char doorname[256];


#define USE_ENCODING 0

#if USE_ENCODING
#include "mp4live_output.h"
#endif



//idtoip
/*
extern void  get_ipbyid(char* id,unsigned long* ip,int* num);
extern void get_id(char* p,int len);
*/
#include "idtoiptable.h"

extern void StopTimer(int type);

#ifdef __cplusplus
extern "C"{
#endif
	//smalldoor_handler.cpp
        void start_smalldoor_video();
        void send_voice_photo_to_center(int type,int cmd,unsigned char* buf,int buflen);

#ifdef __cplusplus
}
#endif

#if HAVE_SCREEN_KEEP
#include "buffer1.h"
#include "buffer2.h"


unsigned char*  ImagePointer[6]={0};
unsigned long   ImageSize[6] = {0};
int             gScreenkeepcount = 2;
extern void screenkeep_init();
#endif

#define S_RESET() system("reboot -f")

extern unsigned long get_sub_room_ip(int num);
extern int g_localid;

extern CMainProcess*          g_MainProcess;
extern bool     g_have_msg;
//extern bool     g_net_is_ok;
extern bool     g_is_talk;
extern bool     g_is_lock;
extern bool     isdownload;                 //true������������false������������
extern bool     g_have_warning ;
extern bool     g_have_defend ; //�Ƿ񲼷�������
extern bool     g_is_leave;    //�Ƿ������ڼ� //2009-7-23
extern int      g_hasplay;
T_SYSTEMINFO      *pSystemInfo;
extern int Get_safe_status();

bool                m_issreenoff;                          //��Ļ�Ƿ�ر�
bool                g_ishandler=false;
bool                g_isstartdelay = false;
bool                g_isstarttime = false;

unsigned long  subroomipaddr[8] = {0};

#ifdef WRT_MORE_ROOM
bool               g_ishangon = false;
bool               g_isadjust = false;

int                g_ismontsmall = -1;

#if 0
unsigned long g_adjustticks1 = 0;
unsigned long g_adjustticks2 = 0;
#endif

#endif

//��ǰ����״̬
static int g_wendu_value = 0;
static int g_shidu_value = 0;
static int g_kongqi_value = 0;


#define             HANDL_ALARM_VOICE      0x300

int                g_isUpdated = 1; //�Ƿ�����ˢ����ʾ��1:yes,0:no

static uint32_t g_timeup = 0;
static uint32_t g_timedown = 0;

static WRT_Rect g_caprt = {0,422,400,56};

unsigned char g_iscapphoto = 0;

static unsigned long g_weather_result[4]={0};


static void set_up_time()
{
        tm_getticks(NULL,&g_timeup);
}

static void set_down_time()
{
        tm_getticks(NULL,&g_timedown);
}

int diff_up_down_time()
{
        return (g_timeup - g_timedown)/1000;
}
//////////////////////////////////////////////////////////
//
//
static unsigned long  g_dates = 0; //�������ڣ�������ص�ǰ�����Ƿ�ı䡣
static int isweek(unsigned long iyear,unsigned long imon,unsigned long iday){
        struct tm time_str;
        time_t curtime;
        time_str.tm_year = iyear-1900;
        time_str.tm_mon = imon-1;
        time_str.tm_mday = iday;
        time_str.tm_hour = 0;
        time_str.tm_min = 0;
        time_str.tm_sec = 1;
        time_str.tm_isdst = -1;
        if((curtime =mktime(&time_str)) == -1)
                return 0;
        WRT_DEBUG("��������%d ",time_str.tm_wday);
        time(&curtime); 
        if(time_str.tm_wday == 0)
                return 0;
        if(time_str.tm_wday == 6)
                return 0;
        return 1;
}

int    IsGetweather()
{
        uint32_t dates,times;
        int Hour = 0;
        int Min  = 0;
        tm_get(&dates,&times,NULL);
        Hour = (times >> 16)& 0XFFFF;
        Min  = (times>> 8) & 0x000000ff;
        if(Hour == 12 || Hour == 18 || Hour == 6){
        	if(Min == 1)
                return 1;
        }
        return 0;
}

int   datescomp(){
        uint32_t dates,times;
        int iyear,imon,iday;
        int iyear1,imon1,iday1;
        tm_get(&dates,&times,NULL);

        //old date
        iyear = (g_dates >> 16) & 0xffff;
        imon = (g_dates >> 8) & 0xff;
        iday = (g_dates )&0xff;
        // new date
        iyear1  = (dates >> 16) & 0xffff;
        imon1 = (dates >> 8) & 0xff;
        iday1   = (dates )&0xff;
        if(iyear1 > iyear){
                g_dates = dates;
                return isweek(iyear1,imon1,iday1);
        }
        if(imon1 > imon){
                g_dates = dates;
                return isweek(iyear1,imon1,iday1);
        }
        if(iday1 > iday){
                g_dates = dates;
                return isweek(iyear1,imon1,iday1);
        }
        return 0;
}

static int free_flash_size(int param)
{
    CreateDialogWin2(LT("�ͷŴ洢�ռ�..."),MB_NONE,NULL,NULL);
    free_flash();
    CloseWindow();
    return 0;
}

static void SendCapJpg()
{
	
	unsigned char* tmprgb = (unsigned char*)g_pScreenSurface->pixels;//YUV����
	unsigned char* outputbuf = (unsigned char* )ZENMALLOC(800*480*3);
	if(outputbuf == NULL){
		WRT_DEBUG("ZENMALLOC FAILED");
		return;
	}
		
	struct image_data  pic;
	int outbuflen  = 0;
	memset(&pic,0,sizeof(pic));
	pic.width=800;
	pic.height = 480;
	pic.data = tmprgb;
	WRT_DEBUG("iptv_image_write_jpeg");
	outbuflen = iptv_image_write_jpeg(&pic,outputbuf,50,32);
	WRT_MESSAGE("outbuflen = %d,tmprgb = %x,outputbuf=%x",outbuflen,tmprgb,outputbuf);
	if(outbuflen > 0){
		send_voice_photo_to_center(2,ROOM_SEND_PHOTO_VOICE,outputbuf,outbuflen);
	}
	
}
//////////////////////////////////////////////////////
//
//
#include <vector>
#include <algorithm>
using namespace std;
typedef vector<int>  ALARMVECTOR;

#define  ALARM_ADD     0x1
#define  ALARM_HANDLE  0x2
#define  ALARM_CANCEL  0x4

static ALARMVECTOR g_alarmvector;
static  SDL_mutex  *g_alarmmutex = NULL;


//��¼ǰ������ʱ�����ľ�����������ʱ�䣬���ǰ��������һֱ�����������ʱ�䣬2�����ڲ���Ӧ��
static unsigned long g_alarmtime[2] = {0,0};
void init_alarm_handle();
void uninit_alarm_handle();




//////////////////////////////////////////////////////////

void init_localhost_config();

void SetSysInfoToEnglish(){

        memset(pSystemInfo->LocalSetting.privateinfo.Moni[0].name,0,20);
        memset(pSystemInfo->LocalSetting.privateinfo.Moni[1].name,0,20);
        memset(pSystemInfo->LocalSetting.privateinfo.Moni[2].name,0,20);

        memset(pSystemInfo->LocalSetting.privateinfo.SmallDoor[0].name,0,20);
        memset(pSystemInfo->LocalSetting.publicroom.MainGate[0].name,0,20);
        memset(pSystemInfo->LocalSetting.publicroom.Door[0].name,0,20);

        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[0],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[1],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[2],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[3],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[0],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[1],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[2],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[3],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[4],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[5],0,20);

        strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[0].name,LT("����ͷһ"));
        strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[1].name, LT("����ͷ��"));
        strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[2].name, LT("����ͷ��"));
        strcpy(pSystemInfo->LocalSetting.privateinfo.SmallDoor[0].name , LT("С�ſڻ�"));
        strcpy(pSystemInfo->LocalSetting.publicroom.MainGate[0].name,LT("��Χǽ��"));
        strcpy(pSystemInfo->LocalSetting.publicroom.Door[0].name,LT("���ſڻ�"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[0],LT("����ϵͳ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[1],LT("��ˮϵͳ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[2],LT("�����Ҿӹ���"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[3],LT("����ϵͳ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[0],LT("������ˮ��"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[1],LT("���"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[2],LT("ȼ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[3],LT("�����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[4],LT("����ˮ��"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[5],LT("ȡů��"));
		memset(pSystemInfo->mySysInfo.ringname,0,2048);

        free_all_ring_cache();
        set_english_ring();   
        /*
        long len =0;
        int i =0;
        unsigned char* addr =0;
        char* ctmp = NULL;
        for(i=0;i<8;i++){ //��������
                addr = 0;
                len = 0;
                ctmp = pSystemInfo->mySysInfo.ringname[i];

                if(ctmp[0] != 0xff && strlen(ctmp) > 0){
                        wrtip_default_set_ring();
                }else{
                        wrtip_set_ring(i,0,0);
                        memset(pSystemInfo->mySysInfo.ringname[i],0,256);
                }
        }
        SDL_Delay(2000);
        */
        wrtip_default_set_ring();
        UpdateSystemInfo();
}

void SetSysInfoToChinese()
{

        memset(pSystemInfo->LocalSetting.privateinfo.Moni[0].name,0,20);
        memset(pSystemInfo->LocalSetting.privateinfo.Moni[1].name,0,20);
        memset(pSystemInfo->LocalSetting.privateinfo.Moni[2].name,0,20);

        memset(pSystemInfo->LocalSetting.privateinfo.SmallDoor[0].name,0,20);
        memset(pSystemInfo->LocalSetting.publicroom.MainGate[0].name,0,20);
        memset(pSystemInfo->LocalSetting.publicroom.Door[0].name,0,20);

        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[0],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[1],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[2],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[3],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[0],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[1],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[2],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[3],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[4],0,20);
        memset(pSystemInfo->LocalSetting.publicroom.NameOfFee[5],0,20);
        strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[0].name,LT("����ͷһ"));
        strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[1].name, LT("����ͷ��"));
        strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[2].name, LT("����ͷ��"));
        strcpy(pSystemInfo->LocalSetting.privateinfo.SmallDoor[0].name , LT("С�ſڻ�"));
        strcpy(pSystemInfo->LocalSetting.publicroom.MainGate[0].name,LT("��Χǽ��"));
        strcpy(pSystemInfo->LocalSetting.publicroom.Door[0].name,LT("���ſڻ�"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[0],LT("����ϵͳ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[1],LT("��ˮϵͳ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[2],LT("�����Ҿӹ���"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[3],LT("����ϵͳ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[0],LT("������ˮ��"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[1],LT("���"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[2],LT("ȼ����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[3],LT("�����"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[4],LT("����ˮ��"));
        strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[5],LT("ȡů��"));
		memset(pSystemInfo->mySysInfo.ringname,0,2048);

        free_all_ring_cache();
        set_chinese_ring();
        
        wrtip_default_set_ring();
        SDL_Delay(2000);
        UpdateSystemInfo();   //���±���ͷ�ļ�
}

void SetDefaultConfig(T_SYSTEMINFO* p){
        if(p == NULL)
                return;
        int i =0;

        /*if(p->mySysInfo.TouchScreenPara[0] == 0xff){ 
        memcpy(p->mySysInfo.TouchScreenPara, tscreendata1, 24);
        }*/
		memset(p->mySysInfo.ringname,0,2048);
		
       	memset(p->mySysInfo.password,0,8);
        memset(p->mySysInfo.superpassword,0,8);
        memcpy(p->mySysInfo.password,"88888888",8);
        memcpy(p->mySysInfo.superpassword,"99999999",8);

        p->BootInfo.SN[31] = 0;
        
        /*
        memset(p->mySysInfo.sysID,0,16);
        memset(p->mySysInfo.gatewaypwd,0,16);
        memset(p->mySysInfo.gatewayroomid,0,16);
        p->mySysInfo.gatewayip = 0;
        */ //�ָ����� ����ռҾ�����
        memset(p->mySysInfo.safestatus,0,128);      //32�����Ļ���״̬
        memset(p->mySysInfo.ringname,0,2048);      //����������
#ifdef WRT_MORE_ROOM
        sync_pwd(0,p->mySysInfo.password,8);
        unsigned short zone = 0;
        SDL_Delay(200);
        sync_zone(0,(unsigned char*)&zone,2);
#endif

        p->mySysInfo.NumOfMp3 = 0;
        for(i=0; i< 5;i++){
                p->mySysInfo.Mp3Position[i] = 0;
                p->mySysInfo.Mp3Len[i] = 0;
                p->mySysInfo.idrule[i] = 0;
        }
        p->mySysInfo.bright = 20;
        p->mySysInfo.contrast = 10;
        p->mySysInfo.saturation = 40;
        p->mySysInfo.ringvolume = 80;
        p->mySysInfo.ringtimes = 35;
        p->mySysInfo.avoidtimes = 43200;
        p->mySysInfo.isavoid = 0;
        p->mySysInfo.isbuttonvoice = 0;
        //p->mySysInfo.isjiaju = 0;
        p->mySysInfo.iskeepscreen = 0;
        p->mySysInfo.isusedffs = 1;
        p->mySysInfo.ishttpdown = 0;
        p->mySysInfo.isagent = 0;
        p->mySysInfo.agenttimes = 43200;
        p->mySysInfo.talkvolume = 15;
        p->mySysInfo.alarmvolume = 20;
        p->mySysInfo.talkvolume = 80;
        p->mySysInfo.alarmvolume = 80;
        p->mySysInfo.micvolume = 100;
        p->mySysInfo.isautocap= 0;
        p->mySysInfo.alarmtime = 100;
        p->mySysInfo.alarmtime1 = 40;
        p->mySysInfo.isrecord = 1;
        p->mySysInfo.isremote = 1;
        p->mySysInfo.alarmvoicetime = 300;
        p->mySysInfo.maxphoto = 100;
        p->mySysInfo.photoqualit = 90;
        p->mySysInfo.bakliftip = 0;
        p->mySysInfo.isnvo = 0;
        p->mySysInfo.framerate  = 7;
        p->mySysInfo.hassip = 1;

        p->mySysInfo.isbomb = 0x0;

		p->mySysInfo.isuseui = 0x2;//Ĭ�ϲ��õ�2��UI

		p->mySysInfo.wifi_dhcp = 0;
		p->mySysInfo.fangchai_switch = 0;
		p->mySysInfo.wifi_switch = 0;
		p->mySysInfo.wifiip = 0x0;
		p->mySysInfo.wifigateway = 0x0;
		p->mySysInfo.wifimask = 0x0;
		p->mySysInfo.wifidns = 0x0;
		p->mySysInfo.chengfangcnt = 3;
		p->mySysInfo.chengfangtime = 5;
		memset(p->mySysInfo.wifissid,0,16);
		memset(p->mySysInfo.wifipwd,0,16);

		for(i = 0;i<10;i++){
			p->mySysInfo.scenenumber[i] = i+1;
		}

        strcpy((char*)p->LocalSetting.privateinfo.LocalID, "010010100100101");
        p->LocalSetting.privateinfo.LocalID[15] = '\0';
        p->LocalSetting.privateinfo.LocalIP = 0x2464a8c0;
        p->LocalSetting.privateinfo.SubMaskIP = 0x00ffffff;//0xffffff00;
        p->LocalSetting.privateinfo.GateWayIP = 0xfe64a8c0;//0xc0a80101;

        p->LocalSetting.privateinfo.NumOfMONI = 0; //����ͷ
        p->LocalSetting.privateinfo.Moni[0].IP = 0x2764a8c0;

        p->LocalSetting.privateinfo.Moni[1].IP = 0x2864a8c0;

        p->LocalSetting.privateinfo.Moni[2].IP = 0x2964a8c0;

        p->LocalSetting.privateinfo.NumOfSmallDoor = 1;
        p->LocalSetting.privateinfo.SmallDoor[0].IP = 0x11;
        p->LocalSetting.privateinfo.NumOfSmallDoor = 0;
        p->LocalSetting.privateinfo.SmallDoor[0].IP = 0;
        p->LocalSetting.privateinfo.SmallDoor[0].name[0]='\0';

        p->LocalSetting.publicinfo.CenterMajorIP = 0x0564a8c0;
        p->LocalSetting.publicinfo.CenterMinorIP = 0;
        p->LocalSetting.publicinfo.ManagerMajorIP = 0x0564a8c0;
        p->LocalSetting.publicinfo.ManagerMinorIP = 0;
        p->LocalSetting.publicinfo.CenterWarningIP = 0;
        p->LocalSetting.publicinfo.CenterMenjinIP = 0;
        p->LocalSetting.publicinfo.CenterFileIP = 0;
        p->LocalSetting.publicinfo.CenterNetIP = 0;

        p->LocalSetting.publicroom.NumOfGate = 1;
        p->LocalSetting.publicroom.MainGate[0].IP = 0x2564a8c0;

        p->LocalSetting.publicroom.NumOfDoor = 1;
        p->LocalSetting.publicroom.Door[0].IP = 0x2664a8c0;

        p->LocalSetting.publicroom.NumOfRepaire = 4;

        p->LocalSetting.publicroom.NumOfFee = 6;

		memset(p->mySysInfo.sysID,0,16);
		memset(p->mySysInfo.gatewaypwd,0,16);
		memset(p->mySysInfo.gatewayroomid,0,16);
		p->mySysInfo.gatewayip = 0;
		
		strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[0].name, LT("����ͷһ"));
		strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[1].name, LT("����ͷ��"));
		strcpy(pSystemInfo->LocalSetting.privateinfo.Moni[2].name, LT("����ͷ��"));
		strcpy(pSystemInfo->LocalSetting.privateinfo.SmallDoor[0].name , LT("С�ſڻ�"));
		strcpy(pSystemInfo->LocalSetting.publicroom.MainGate[0].name,LT("��Χǽ��"));
		strcpy(pSystemInfo->LocalSetting.publicroom.Door[0].name,LT("���ſڻ�"));
		
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[0],LT("����ϵͳ����"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[1],LT("��ˮϵͳ����"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[2],LT("�����Ҿӹ���"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[3],LT("����ϵͳ����"));
		
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[0],LT("������ˮ��"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[1],LT("���"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[2],LT("ȼ����"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[3],LT("�����"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[4],LT("����ˮ��"));
		strcpy(pSystemInfo->LocalSetting.publicroom.NameOfFee[5],LT("ȡů��"));

        pSystemInfo->mySysInfo.idrule[0] = 2;
        pSystemInfo->mySysInfo.idrule[1] = 3;
        pSystemInfo->mySysInfo.idrule[2] = 2;
        pSystemInfo->mySysInfo.idrule[3] = 3;
        pSystemInfo->mySysInfo.idrule[4] = 3;

        memset(pSystemInfo->mySysInfo.phonenumber,0,120);

        test_info_for_factory();
        free_all_ring_cache();
        set_chinese_ring();

        FlushSystemInfo();   //���±���ͷ�ļ�
        init_localhost_config(); //�������úø��ֲ���

        WRT_MESSAGE("���üҾӵ�Ĭ�ϲ���\n");
        delete_scene_and_clock();
        //SetJiajuDefault(); //��ʼ��Ĭ�ϼҾӵĲ���
       // SetJiajuDefault(); //��ʼ��Ĭ�ϼҾӵĲ��� 
        //wrtip_default_set_ring();//�ָ�Ĭ�ϰ�����
        //system("rm -rf /home/wrt/pic");
}

/**
*
*/
int GetIsHaveJiaju(){
        return pSystemInfo->mySysInfo.isjiaju;
}

void SetIsHaveJiaju(int status){
        pSystemInfo->mySysInfo.isjiaju = status;
}

void DeflateRect( WRT_Rect* lpRect,int offset ){
        if(lpRect == NULL)
                return;
        lpRect->x +=offset;
        lpRect->y +=offset;
        lpRect->w -=2*offset;
        lpRect->h -=2*offset;

}

int GetCurrentLanguage()
{
        if(pSystemInfo)
                return pSystemInfo->mySysInfo.isenglish;
        return -1;             
}




/**
*�������ƣ�Reset_sys
*���������
*���������
*��    �ܣ���������ϵͳ
*��    �أ�
*/
void Reset_sys(){
        S_RESET();
}


/************************************************************************/
/*                                                                      */
/************************************************************************/
static void flush_rect(WRT_Rect rt)
{
	
	if(g_pScreenSurface)
		SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);

	/*
        if(COPY_Flush(rt,xy) == -1){
        	WRT_MESSAGE("ˢ��ʧ��");
        	SDL_Delay(20);
        	COPY_Flush(rt,xy);
        }
        */	
       // WRT_DEBUG("ˢ�²���(%d,%d,(%d,%d,%d,%d)",xy.x,xy.y,rt.ul.x,rt.ul.y,rt.lr.x,rt.lr.y);
}

void update_rect(int x,int y,int w,int h)
{

#if 1
        if(x > 1024)
                x= 1024;
        if(y > 600)
                y = 600;

        if(x == 0 && y == 0 && w == 0 && h == 0)
        {
                w = 1024;
                h = 600;    
                 
        }
#endif
        if(g_pScreenSurface)
        	SDL_UpdateRect(g_pScreenSurface,x,y,w,h);
        	//SDL_Flip(g_pScreenSurface);
}

void UpdateWindow(int x,int y,int w,int h,int style)
{
	struct timeval tv_begin, tv_end;

	Uint32 start;
	SDL_Rect rt;
	SDL_Rect srt;

	if(w==0||h==0)
	{
		rt.x =0;
		srt.x=0;
		rt.y=0;
		srt.y=0;
		rt.w=1024;
		rt.h=600;
		srt.w=1024;
		srt.h=600;
	}else
	{
		rt.x =x;
		srt.x=x;
		rt.y=y;
		srt.y=y;
		rt.w=w;
		rt.h=h;
		srt.w=w;
		srt.h=h;
	}

	//SDL_BlitSurface(tmp_g_pScreenSurface,&srt,g_pScreenSurface,&rt);

	
    // SDL_Delay(1500);//�ӳٱ�֤����Ŀ�����������������ˢ֡����

	if(g_pScreenSurface)
	{
		SDL_UpdateRect(g_pScreenSurface,x,y,w,h);
		//SDL_Flip(g_pScreenSurface);
	}


}

/**
*�������ƣ�DrawRect
*���������WRT_Rect rect ��������
*          int linewidth ���ʣ��ߣ��Ŀ��
*          int color     ���ʵ���ɫ
*���������
*��    �ܣ�
*��    �أ�
*/
void DrawRect(WRT_Rect rect,int linewidth,int color){

		if(g_pScreenSurface)
			Draw_Rect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h,color);
			
		if(g_isUpdated == 1){
			if(g_pScreenSurface)
					SDL_UpdateRect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h);		
					//SDL_Flip(g_pScreenSurface);
		}

      //  if(tmp_g_pScreenSurface)
      //  	Draw_Rect(tmp_g_pScreenSurface,rect.x,rect.y,rect.w,rect.h,color);
      //  if(g_isUpdated == 1){
	//		if(tmp_g_pScreenSurface)
     //   		SDL_UpdateRect(tmp_g_pScreenSurface,rect.x,rect.y,rect.w,rect.h);              
     //   }
        		
}

void DrawSingleRect(WRT_Rect rect,int linewidth,int color){

        if(g_pScreenSurface)
        	Draw_Rect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h,color);
        if(g_isUpdated == 1){
			if(g_pScreenSurface)
				//SDL_Flip(g_pScreenSurface);
        			SDL_UpdateRect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h);              
        }
        		
}

/**
*�������ƣ�FillRect
*���������int color �����ɫ
*          WRT_Rect rect  ���ľ���λ��
*���������
*��    �ܣ�����������
*��    �أ�
*/
void FillSingleRect(int color,WRT_Rect rect){
		int m_color;
        if(rect.w < 0)
                rect.w = 0;

        if(rect.h < 0)
                rect.h = 0;
		m_color = SDL_MapRGB(g_pScreenSurface->format, (color>>16)&(0x0000ff), (color>>8) &0x0000ff,color&0x0000ff);
        Draw_FillRect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h,m_color);
        if(g_isUpdated == 1)
        {
			if(g_pScreenSurface)
        		SDL_UpdateRect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h);   
        }
        		
}

void FillRect(int color,WRT_Rect rect){

		int m_color;
		if(rect.w < 0)
				rect.w = 0;

		if(rect.h < 0)
				rect.h = 0;
		m_color = SDL_MapRGB(g_pScreenSurface->format, (color>>16)&(0x0000ff), (color>>8) &0x0000ff,color&0x0000ff);
		Draw_FillRect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h,m_color);
		if(g_isUpdated == 1)
		{
			if(g_pScreenSurface)
				SDL_UpdateRect(g_pScreenSurface,rect.x,rect.y,rect.w,rect.h);	
		}

}


void OSD_FillRect(int color,WRT_Rect rect){
       FillRect(color,rect);
}



/**
*�������ƣ�ClearRect
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void ClearRect(int x,int y,int w,int h){
        
}
static void zoomRGB_32(unsigned char* pSrcImg,unsigned char* pDstImg,int nWidth,int nHeight,float xfRate,float yfRate)
{
        int i = 0;
        int j = 0;

        float fX, fY;
        int iStepSrcImg = nWidth;
        int iStepDstImg = nWidth * xfRate;
        int iX, iY;

        unsigned char bUpLeft, bUpRight, bDownLeft, bDownRight;
        unsigned char gUpLeft, gUpRight, gDownLeft, gDownRight;
        unsigned char rUpLeft, rUpRight, rDownLeft, rDownRight;
        unsigned char b, g, r;

        for(i = 0; i < nHeight * xfRate; i++)
        {
                for(j = 0; j < nWidth * xfRate; j++)
                {
                        fX = ((float)j) /xfRate;
                        fY = ((float)i) /xfRate;
                        iX = (int)fX;
                        iY = (int)fY;
                        bUpLeft = pSrcImg[iY * iStepSrcImg * 4 + iX * 4 + 0];
                        bUpRight = pSrcImg[iY * iStepSrcImg * 4 + (iX + 1) * 4 + 0];
                        bDownLeft = pSrcImg[(iY + 1) * iStepSrcImg * 4 + iX * 4 + 0];
                        bDownRight = pSrcImg[(iY + 1) * iStepSrcImg * 4 + (iX + 1) * 4 + 0];

                        gUpLeft = pSrcImg[iY * iStepSrcImg * 4 + iX * 4 + 1];
                        gUpRight = pSrcImg[iY * iStepSrcImg * 4 + (iX + 1) * 4 + 1];
                        gDownLeft = pSrcImg[(iY + 1) * iStepSrcImg *4 + iX * 4 + 1];
                        gDownRight = pSrcImg[(iY + 1) * iStepSrcImg * 4 + (iX + 1) * 4 + 1];
                        rUpLeft = pSrcImg[iY * iStepSrcImg * 4 + iX * 4 + 2];
                        rUpRight = pSrcImg[iY * iStepSrcImg * 4 + (iX + 1) * 4 + 2];
                        rDownLeft = pSrcImg[(iY + 1) * iStepSrcImg * 4 + iX * 4 + 2];
                        rDownRight = pSrcImg[(iY + 1) * iStepSrcImg * 4 + (iX + 1) * 4 + 2];

                        b = bUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + bUpRight * (fX - iX) * (iY + 1 - fY)
                                + bDownLeft * (iX + 1 - fX) * (fY - iY) + bDownRight * (fX - iX) * (fY - iY);
                        g = gUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + gUpRight * (fX - iX) * (iY + 1 - fY)
                                + gDownLeft * (iX + 1 - fX) * (fY - iY) + gDownRight * (fX - iX) * (fY - iY);
                        r = rUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + rUpRight * (fX - iX) * (iY + 1 - fY)
                                + rDownLeft * (iX + 1 - fX) * (fY - iY) + rDownRight * (fX - iX) * (fY - iY);
                        if(iY >= 0 && iY <= nHeight * 2 && iX >= 0 && iX <= nWidth * 2)
                        {
                                pDstImg[i * iStepDstImg * 4 + j * 4 + 0] = b;        //B
                                pDstImg[i * iStepDstImg * 4 + j * 4 + 1] = g;        //G
                                pDstImg[i * iStepDstImg * 4 + j * 4 + 2] = r;        //R
                                pDstImg[i * iStepDstImg * 4 + j * 4 + 3] = 0x0;   //A
                        }
                }
        }
}

static void zoomRGB_24(unsigned char* pSrcImg,unsigned char* pDstImg,int nWidth,int nHeight,float fRate)
{
        int i = 0;
        int j = 0;

        float fX, fY;
        int iStepSrcImg = nWidth;//ԭʼ���
        int iStepDstImg = nWidth * fRate;//��������
        int DstHeight,DstWidth;
        DstHeight = (int)nHeight * fRate;
        DstWidth  = (int)nWidth * fRate;
	
        int iX, iY;
        unsigned char tmp[4096*4096*2];

        unsigned char bUpLeft, bUpRight, bDownLeft, bDownRight;
        unsigned char gUpLeft, gUpRight, gDownLeft, gDownRight;
        unsigned char rUpLeft, rUpRight, rDownLeft, rDownRight;
        unsigned char b, g, r;

        for(i = 0; i < DstHeight; i++)//���ź�ĸ߶�
        {
        	//printf("ixj:%d,%d iXxiY:%d,%d\n",j,i,iX,iY);
                for(j = 0; j < DstWidth; j++)//���ź�Ŀ��
                {
                        fX = ((float)j) /fRate;
                        fY = ((float)i) /fRate;
                        iX = (int)fX;
                        iY = (int)fY;

                        #if 0
                        bUpLeft = pSrcImg[iY * iStepSrcImg * 3 + iX * 3 + 0];
                        bUpRight = pSrcImg[iY * iStepSrcImg * 3 + (iX + 1) * 3 + 0];
                        bDownLeft = pSrcImg[(iY + 1) * iStepSrcImg * 3 + iX * 3 + 0];
                        bDownRight = pSrcImg[(iY + 1) * iStepSrcImg * 3 + (iX + 1) * 3 + 0];

                        gUpLeft = pSrcImg[iY * iStepSrcImg * 3 + iX * 3 + 1];
                        gUpRight = pSrcImg[iY * iStepSrcImg * 3 + (iX + 1) * 3 + 1];
                        gDownLeft = pSrcImg[(iY + 1) * iStepSrcImg * 3 + iX * 3 + 1];
                        gDownRight = pSrcImg[(iY + 1) * iStepSrcImg * 3 + (iX + 1) * 3 + 1];
                        rUpLeft = pSrcImg[iY * iStepSrcImg * 3 + iX * 3 + 2];
                        rUpRight = pSrcImg[iY * iStepSrcImg * 3 + (iX + 1) * 3 + 2];
                        rDownLeft = pSrcImg[(iY + 1) * iStepSrcImg * 3 + iX * 3 + 2];
                        rDownRight = pSrcImg[(iY + 1) * iStepSrcImg * 3 + (iX + 1) * 3 + 2];

                        b = bUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + bUpRight * (fX - iX) * (iY + 1 - fY)
                                + bDownLeft * (iX + 1 - fX) * (fY - iY) + bDownRight * (fX - iX) * (fY - iY);
                        g = gUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + gUpRight * (fX - iX) * (iY + 1 - fY)
                                + gDownLeft * (iX + 1 - fX) * (fY - iY) + gDownRight * (fX - iX) * (fY - iY);
                        r = rUpLeft * (iX + 1 - fX) * (iY + 1 - fY) + rUpRight * (fX - iX) * (iY + 1 - fY)
                                + rDownLeft * (iX + 1 - fX) * (fY - iY) + rDownRight * (fX - iX) * (fY - iY);
                        #endif
                        //b = pSrcImg[iY * iStepSrcImg*3  + iX*3  + 0];
                        //g = pSrcImg[iY * iStepSrcImg*3  + iX*3  + 1];
                        //r = pSrcImg[iY * iStepSrcImg*3  + iX*3  + 2];
                        if(iY >= 0 && iY <= nHeight * 2 && iX >= 0 && iX <= nWidth * 2)
                        {
                            pDstImg[i * iStepDstImg * 3 + j * 3 + 0] = pSrcImg[iY * iStepSrcImg*3  + iX*3  + 0];        //B
                            pDstImg[i * iStepDstImg * 3 + j * 3 + 1] = pSrcImg[iY * iStepSrcImg*3  + iX*3  + 1];        //G
                            pDstImg[i * iStepDstImg * 3 + j * 3 + 2] = pSrcImg[iY * iStepSrcImg*3  + iX*3  + 2];        //R
                        }
                }
        }
}

void JpegDispRectCenter(WRT_Rect rt,unsigned char* buf,int buflen)
{

        if(buf == NULL || buflen == 0){
                return;
        }
        struct image_data *picdata;
        uint32_t width;
        uint32_t height;
      
        int  x,y;
        x = rt.x;
        y = rt.y;
        picdata = iptv_image_open((unsigned char *)buf, buflen,"jpeg");

        if (picdata != NULL)
        {

                width = picdata->width;
                height = picdata->height;
                if(width > 0 && width  < 801 && height > 0 && height < 423){
                        x += (rt.w - width)/2;
                        y += (rt.h - height)/2;
                        //tmFillBmp(picdata->data, xy, width, height);//������JPEG������ù̶��ڴ棬���Բ����ͷ�data��
                        SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(picdata->data,width,height,24,width*3,
                        	  0xFF0000, 0x00FF00, 0x0000FF,0);
                       if(pSurface){
                       		SDL_Rect rt;
                       		rt.x =x;
                       		rt.y = y;
                       		rt.w = width;
                       		rt.h = height;
                       		SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
                       		SDL_FreeSurface(pSurface);
                       		pSurface = NULL;
                	}
                        if(g_isUpdated == 1)
                        {
                                SDL_UpdateRect(g_pScreenSurface,x,y,width,height);
                        }
                        					  
                }
                ZENFREE(picdata);
                picdata = NULL;
        }else
        {
                WRT_WARNING("jpeg error");
        }	
}

void JpegDispCenter(int x,int y,unsigned char* buf,int buflen,int scwidth){
	if(buf == NULL || buflen == 0){
                return;
        }
        struct image_data *picdata;
        SDL_Surface* pSurface;
        uint32_t width;
        uint32_t height;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
	int start;

	pSurface = iptv_image_jpeg_read_sdl((unsigned char *)buf, buflen);

        if (pSurface != NULL)
        {
                width = pSurface->w;
                height = pSurface->h;
                rt.x += (scwidth -width)/2;
                if(rt.x < 0)
                	rt.x = 0;
                rt.y += (500-height)/2;
                if(rt.y < 0)
                	rt.y = 0;
                if(pSurface){
                        srt.x = 0;
                        srt.y = 0; 
                	
                	
            		rt.w = width;
            		rt.h = height;
                	
                	srt.w = rt.w;
                	srt.h = rt.h;                     
                	SDL_BlitSurface(pSurface,&srt,g_pScreenSurface,&rt);
                	SDL_FreeSurface(pSurface);
                	pSurface = NULL;   
                }
               
                
                if(g_isUpdated == 1)
                {
                        SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
                       // SDL_Flip(g_pScreenSurface);
                       
                }	
           
        }else{
        	WRT_MESSAGE("������� buflen = %d",buflen);
        }
}

void JpegDispZoom2(int x,int y, unsigned char* buf,int buflen)
{
        if(buf == NULL || buflen == 0){
                return;
        }
        /*
        unsigned long ticks = SDL_GetTicks();
        SDL_Surface* pImageSurface =  iptv_image_jpeg_read_sdl((unsigned char *)buf, buflen);
     
        if(pImageSurface){
        	double d_angle=0.0;
        	double d_fx = 1.0;
        	double d_fy = 1.0;
        	double d_smooth = 0;
        	float fscaling = (float)880.0/(float)530.0;
		SDL_Surface *rotozoom_picture;
		SDL_Rect dest;        	
		float tmpfscaling = (float)pImageSurface->w/(float)pImageSurface->h;
        			
		if( tmpfscaling >= fscaling )
		{
			fscaling = (float)880.0/(float)pImageSurface->w;
		}else
		{
			fscaling = (float)530.0/(float)pImageSurface->h;
		}  
		 d_fx = d_fy = fscaling;  
        	 WRT_DEBUG("pImageSurface %x,%d,%d,%f,%f\n",pImageSurface,pImageSurface->w,pImageSurface->h,d_fx,d_fy);
		if ((rotozoom_picture=rotozoomSurfaceXY (pImageSurface, d_angle, d_fx, d_fy, d_smooth))!=NULL) {
			dest.x = x;;
			dest.y = y;
			dest.w = rotozoom_picture->w;
			dest.h = rotozoom_picture->h;
			if ( SDL_BlitSurface(rotozoom_picture, NULL, g_pScreenSurface, &dest) < 0 ) {
				fprintf(stderr, "Blit failed: %s\n", SDL_GetError());
				return;
			}
			SDL_FreeSurface(rotozoom_picture);
		}
		
                if(g_isUpdated == 1)
                {
                         SDL_Flip(g_pScreenSurface);
                }
                SDL_FreeSurface(pImageSurface);	
        	
        }
        WRT_DEBUG("TIME:%d",SDL_GetTicks() - ticks);
        */
        
        struct image_data *picdata;
        static int gzoombuflen = 0;
        static unsigned char* g_ZoomBuf = NULL;
        uint32_t width;
        uint32_t height;
        float fscaling = (float)880.0/(float)530.0;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
     
        picdata = iptv_image_open((unsigned char *)buf, buflen,"jpeg");
        
        if (picdata != NULL)
        {
        		WRT_MESSAGE("%d,%d",picdata->width ,picdata->height);
        		if(picdata->width >0&& picdata->width<880 &&picdata->height >0 && picdata->height<530){
        			WRT_DEBUG("cant resize pic");
        		}else{
        			float tmpfscaling = (float)picdata->width/(float)picdata->height;
        			WRT_MESSAGE("tmpfscaling = %f",tmpfscaling);
        			if( tmpfscaling >= fscaling )
	        		{
	        			fscaling = (float)880.0/(float)picdata->width;
	        		}else
	        		{
	        			fscaling = (float)530.0/(float)picdata->height;
	        		}
        		}
        		if(fscaling* picdata->width <= 880 && fscaling *picdata->height <=530)
        		{
        			width = picdata->width*fscaling;
        			height = picdata->height*fscaling;
        		}
        		if(g_ZoomBuf == NULL)
        		{
        			g_ZoomBuf = (unsigned char*)ZENMALLOC((width)*(height)*3);
                    		if(g_ZoomBuf == NULL){
                            		printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
                            		return ;
                    		}
        		}else{
        			int len = ((width)*(height)*3);
                    		if(len > gzoombuflen){
                            		free(g_ZoomBuf);
                            		g_ZoomBuf = NULL;
                            		g_ZoomBuf = (unsigned char*)ZENMALLOC(len);
                            		if(g_ZoomBuf == NULL){
                                   		 printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
                                    		return ;
                            		}
                           	 	gzoombuflen = len;
                    		}
        		}
		WRT_MESSAGE("��ʼ%f",fscaling);
                zoomRGB_24(picdata->data,g_ZoomBuf, picdata->width,picdata->height,fscaling);
                WRT_MESSAGE("end%f",fscaling);
                SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(picdata->data,width,height,24,width*3,
                        	  0xFF0000, 0x00FF00, 0x0000FF, 0);
	         if(pSurface){ 
	           		rt.w = width;
	           		rt.h = height;
	           		srt.x = 0;
	           		srt.y = 0;
	           		srt.w = width;
	           		srt.h = height;
	           		rt.x  += (880-width)/2;
	           		rt.y  += (530-height)/2;
	           		if(rt.x < 0)
	           			rt.x = 0;
	           		if(rt.y < 0)
	           			rt.y = 0;
	           		WRT_MESSAGE("srt(%d,%d,%d,%d),dst(%d,%d,%d,%d)",srt.x,srt.y,srt.w,srt.h,
	           			rt.x,rt.y,rt.w,rt.h);
	           		SDL_BlitSurface(pSurface,&srt,g_pScreenSurface,&rt);
	           		SDL_FreeSurface(pSurface);
	          		pSurface = NULL;
	    	   }
                
                if(g_isUpdated == 1)
                {
                	SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
                         //SDL_Flip(g_pScreenSurface);
                }
                free(picdata);
        }
        
}

void JpegDispZoom(int x,int y, unsigned char* buf,int buflen,float xscaling,float yscaling)
{
		int time;
		struct timeval tv_begin, tv_end;
        if(buf == NULL || buflen == 0){
                return;
        }
        gettimeofday(&tv_begin, NULL);
        struct image_data *picdata;
        static int gzoombuflen = 0;
        static unsigned char* g_ZoomBuf = NULL;//
        uint32_t width;
        uint32_t height;
        float fscaling;
        float t;
        int tt;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
        picdata = iptv_image_open((unsigned char *)buf, buflen,"jpeg");
        
		//gettimeofday(&tv_end, NULL);
		//time = (1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec)/1000;
		//printf("decode time1 :%d \n",time);
        //printf("JpegDispZoom 2\n");
        if (picdata != NULL)
        {
#if 1
			printf("pic width:%d  height:%d\n",picdata->width,picdata->height);
        	if(picdata->width >0&& picdata->width<880 &&picdata->height >0 && picdata->height<530)
    		{
    			//�����������
    			width = picdata->width;
                height = picdata->height;
                if(picdata->width == 352 && picdata->height == 288)
	    		{
	    			{
					width = 528;
					height = 432;
					rt.x = (880-width)/2;
	    			rt.y = (530-height)/2;
					fscaling = 1.5;
	                if(g_ZoomBuf == NULL){
	                        g_ZoomBuf = (unsigned char*)malloc((width)*(height)*3);
	                        if(g_ZoomBuf == NULL){
	                                printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
	                                return ;
	                        }
	                }else{
	                        int len = ((width)*(height)*3);
	                        if(len > gzoombuflen){
	                                free(g_ZoomBuf);
	                                g_ZoomBuf = NULL;
	                                g_ZoomBuf = (unsigned char*)malloc(len);
	                                if(g_ZoomBuf == NULL){
	                                        printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
	                                        return ;
	                                }
	                                gzoombuflen = len;
	                        }
	                }
	                
	                // zoomRGB_32(picdata->data,g_ZoomBuf, picdata->width,picdata->height,xscaling,xscaling);
	                zoomRGB_24(picdata->data,g_ZoomBuf, picdata->width,picdata->height,fscaling);
	                // printf("g_ZoomBuf create\n");
	                SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(g_ZoomBuf,width,height,24,width*3,
	                        	  0xFF0000, 0x00FF00, 0x0000FF, 0);
	                        //tmFillBmp(picdata->data, xy, width, height);//������JPEG������ù̶��ڴ棬���Բ����ͷ�data��
	                       if(pSurface){ 
	                       		rt.w = width;
	                       		rt.h = height;
	                       		//srt.x = 0;
	                       		//srt.y = 0;
	                       		///srt.w = width;
	                       		//srt.h = height;
	                       		if(rt.x>=0&&rt.y>=0&&rt.w>=0&&rt.h>=0)
	                       			SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
	                       		SDL_FreeSurface(pSurface);
	                       		free(g_ZoomBuf);
	                       		g_ZoomBuf = NULL;
	                      		pSurface = NULL;
	                	}
				  }
	    		}else{ 
						SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(picdata->data,width,height,24,width*3,
                          		 0xFF0000, 0x00FF00, 0x0000FF,0);
                        if(pSurface){
                              
                        	rt.x = (880-width)/2;
	    					rt.y = (530-height)/2;
                        	rt.w = width;
                       		rt.h = height;
                       		if(rt.x>=0&&rt.y>=0&&rt.w>=0&&rt.h>=0)
                        		SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
                        	//SDL_BlitSurface(pSurface,&srt,tmp_g_pScreenSurface,&rt);
                        	SDL_FreeSurface(pSurface);
                        	pSurface = NULL;   
                        }
                 }
    		}
    		else{
    			//�������
    			//ȡһλС������Ϊ���ŵ�λ
    			
    			float pic_ratio = (float)picdata->width/(float)picdata->height;
    			float bak_ratio = (float)880/(float)530;
    			if(pic_ratio>= bak_ratio)
        		{
        			fscaling = (float)880/picdata->width;
        			t = fscaling*1000000;
        			tt = (int)t/100000;
        			fscaling = (float)tt/10;
        			printf("fscaling1 %f\n",fscaling);
        		}else
        		{
        			fscaling = (float)530/picdata->height;
        			t = fscaling*1000000;
        			tt = (int)t/100000;
        			fscaling = (float)tt/10;
        			printf("fscaling2 %f\n",fscaling);
        		}
        		if(fscaling* picdata->width <= 880 && fscaling *picdata->height <=530)
	    		{
	    			width  = picdata->width*fscaling;
	    			height = picdata->height*fscaling;
	    		}
    			printf("pic dst width:%d  height:%d\n",width,height);
	    		rt.x = (880-width)/2;
	    		rt.y = (530-height)/2;
#endif
                if(g_ZoomBuf == NULL){
                        g_ZoomBuf = (unsigned char*)malloc((width)*(height)*3);
                        if(g_ZoomBuf == NULL){
                                printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
                                return ;
                        }
                }else{
                        int len = ((width)*(height)*3);
                        if(len > gzoombuflen){
                                free(g_ZoomBuf);
                                g_ZoomBuf = NULL;
                                g_ZoomBuf = (unsigned char*)malloc(len);
                                if(g_ZoomBuf == NULL){
                                        printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
                                        return ;
                                }
                                gzoombuflen = len;
                        }
                }
                
                // zoomRGB_32(picdata->data,g_ZoomBuf, picdata->width,picdata->height,xscaling,xscaling);
                zoomRGB_24(picdata->data,g_ZoomBuf, picdata->width,picdata->height,fscaling);
               // printf("g_ZoomBuf create\n");
               SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(g_ZoomBuf,width,height,24,width*3,
                        	  0xFF0000, 0x00FF00, 0x0000FF, 0);
                        //tmFillBmp(picdata->data, xy, width, height);//������JPEG������ù̶��ڴ棬���Բ����ͷ�data��
                       if(pSurface){ 
                       		rt.w = width;
                       		rt.h = height;
                       		//srt.x = 0;
                       		//srt.y = 0;
                       		///srt.w = width;
                       		//srt.h = height;
                       		if(rt.x>=0&&rt.y>=0&&rt.w>=0&&rt.h>=0)
                       			SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
                       		SDL_FreeSurface(pSurface);
                       		free(g_ZoomBuf);
                       		g_ZoomBuf = NULL;
                      		pSurface = NULL;
                	}
			  }

			if(g_isUpdated == 1)
			{
				SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,width,height);
			}
			free(picdata);
			picdata = NULL;
        }
}

void JpegFileDispZoom(int x,int y,FILE *fp)
{
        struct image_data *picdata;
        static int gzoombuflen = 0;
        static unsigned char* g_ZoomBuf = NULL;//
        uint32_t width;
        uint32_t height;
        float fscaling;
        float t;
        int tt;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
        //CapturePhotoSave();
        picdata = iptv_image_open2(fp);
        printf("JpegDispZoom 2\n");
        if (picdata != NULL)
        {
#if 1
			printf("pic width:%d  height:%d\n",picdata->width,picdata->height);
        	if(picdata->width >0&& picdata->width<880 &&picdata->height >0 && picdata->height<530)
    		{
    			//�����������
    			width = picdata->width;
                height = picdata->height;
                if(width > 0 && width  < 880 && height > 0 && height < 530){ 
						SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(picdata->data,width,height,24,width*3,
                          		 0xFF0000, 0x00FF00, 0x0000FF,0);
                        if(pSurface){
                              
                        	rt.x = (880-width)/2;
	    					rt.y = (530-height)/2;
                        	rt.w = width;
                       		rt.h = height;
                       		if(rt.x>=0&&rt.y>=0&&rt.w>=0&&rt.h>=0)
                        		SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
                        	//SDL_BlitSurface(pSurface,&srt,tmp_g_pScreenSurface,&rt);
                        	SDL_FreeSurface(pSurface);
                        	pSurface = NULL;   
                        }
                 }
    		}else
    		{
    			//�������
    			//ȡһλС������Ϊ���ŵ�λ
    			
    			float pic_ratio = (float)picdata->width/(float)picdata->height;//ͼƬ��߱���
    			float bak_ratio = (float)880/(float)530;//������ʾ֧�ֱ���
    			if(pic_ratio>= bak_ratio)
        		{
        			fscaling = (float)880/picdata->width;
        			t = fscaling*1000000;
        			tt = (int)t/100000;
        			fscaling = (float)tt/10;
        			printf("fscaling1 %f\n",fscaling);
        		}else
        		{
        			fscaling = (float)530/picdata->height;
        			t = fscaling*1000000;
        			tt = (int)t/100000;
        			fscaling = (float)tt/10;
        			printf("fscaling2 %f\n",fscaling);
        		}
        		if(fscaling* picdata->width <= 880 && fscaling *picdata->height <=530)
	    		{
	    			width = picdata->width*fscaling;
	    			if(width%2 == 0)
	    				;
	    			else width +=1;
	    			
	    			height = picdata->height*fscaling;
	    			if(height %2 == 0)
	    				;
	    			else height +=1;
	    		}
    			printf("pic dst width:%d  height:%d\n",width,height);
	    		rt.x = (880-width)/2;
	    		rt.y = (530-height)/2;
#endif
                if(g_ZoomBuf == NULL){
                        g_ZoomBuf = (unsigned char*)malloc((width)*(height)*3);
                        if(g_ZoomBuf == NULL){
                                printf("ZENMALLOC FAILED %s %d w:%d h:%d",__FILE__,__LINE__,width,height);
                                return ;
                        }
                }else{
                        int len = ((width)*(height)*3);
                        if(len > gzoombuflen){
                                free(g_ZoomBuf);
                                g_ZoomBuf = NULL;
                                g_ZoomBuf = (unsigned char*)malloc(len);
                                if(g_ZoomBuf == NULL){
                                        printf("ZENMALLOC FAILED %s %d",__FILE__,__LINE__);
                                        return ;
                                }
                                gzoombuflen = len;
                        }
                }
               zoomRGB_24(picdata->data,g_ZoomBuf, picdata->width,picdata->height,fscaling);
               SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(g_ZoomBuf,width,height,24,width*3,
                        	  0xFF0000, 0x00FF00, 0x0000FF, 0);
                       if(pSurface){ 
                       		rt.w = width;
                       		rt.h = height;
                       		if(rt.x>=0&&rt.y>=0&&rt.w>=0&&rt.h>=0)
                       			SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
                       		SDL_FreeSurface(pSurface);
                       		free(g_ZoomBuf);
                       		g_ZoomBuf = NULL;
                      		pSurface = NULL;
                	}
			  }

                if(g_isUpdated == 1)
                {
                	SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,width,height);
                	//SDL_Flip(g_pScreenSurface);
                }
                free(picdata);
                picdata = NULL;
        }
}

//int tmFillBmpEx(UInt8* buf_bmp,tmXY_t xy, UInt32 Width, UInt32 Height, UInt32 hEx);
void JpegDisp2(int x,int y,int dw,int dh,unsigned char* buf,int buflen){
        if(buf == NULL || buflen == 0){
                return;
        }
        struct image_data *picdata;
        uint32_t width;
        uint32_t height;
        SDL_Rect rt;
        rt.x = x;
        rt.y = y;
        picdata = iptv_image_open((unsigned char *)buf, buflen,"jpeg");
        if (picdata != NULL)
        {
                width = picdata->width;
                height = picdata->height;
                if(width > 0 && width  < 801 && height > 0 && height < 481){
                        dw = abs((int)(width-dw));
                        dh = abs((int)(height-dh));
                        WRT_DEBUG("dw = %d ,dh = %d ",dw,dh);
                        rt.w = dw;
                        rt.h = dh;
                        SDL_Surface* pSurface =  SDL_CreateRGBSurfaceFrom(picdata->data,width,height,32,width*4,
                        	   0x00ff0000 , 0x0000ff00, 0x000000ff, 0xff000000);
                        //tmFillBmpEx(picdata->data,xy,width,height,dw,dh);
                         if(pSurface){
                       		SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
                       		SDL_FreeSurface(pSurface);
                       		pSurface = NULL;
                	}  
                       // COPY_tmFillBmpEx(picdata->data, xy, width, height,dw,dh);
                        if(g_isUpdated == 1)
                        {
 			 	SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
                        }
                        							
                        // tmFillBmp(picdata->data, xy, width, height);//������JPEG������ù̶��ڴ棬���Բ����ͷ�data��
                }
                ZENFREE(picdata);
                picdata = NULL;
        }	
}

void Bmp32Disp(int x,int y,int w,int h,unsigned char* buf,int size)
{
	if(buf == NULL){
                return;
        }
        SDL_Rect rt;
        rt.x = x;
        rt.y = y;

        SDL_Surface* pSurface =SDL_CreateRGBSurfaceFrom((void *)buf,w,h,32,w*4,0xFF000000, 0x00FF0000, 0x0000FF00,0x000000FF);

    	if(pSurface){
    		rt.w = pSurface->w;
    		rt.h = pSurface->h;
    		SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);//16
    		SDL_FreeSurface(pSurface);
    	}
        if(g_isUpdated == 1)
        {
        	SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
              // SDL_Flip(g_pScreenSurface);
        }
}

void OSD_Bmp32Disp(int x,int y,int w,int h,unsigned char* buf,int size)
{
	if(buf == NULL){
		return;
	}
	Bmp32Disp(x,y,w,h,buf,size);
}

/**
*�������ƣ�JpegDisp
*���������
*���������
*��    �ܣ�
*��    �أ�
*/

void JpegDispFile(int x,int y,struct image_data *picdata){
       
        //struct image_data *picdata;
        uint32_t width;
        uint32_t height;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
		int start;
		//FILE *fd1;
		//struct timeval tv_begin, tv_end;
		//	gettimeofday(&tv_begin, NULL);

		//  picdata = iptv_image_open((unsigned char *)buf, buflen,"jpeg");
		//  gettimeofday(&tv_end, NULL);
		//	start = (1000000 * (tv_end.tv_sec - tv_begin.tv_sec) + tv_end.tv_usec - tv_begin.tv_usec)/1000;
		//	printf("mmm decode time :%d  %d  %d\n",start,picdata->width,picdata->height);

        if (picdata != NULL)
        {
                width = picdata->width;
                height = picdata->height;
                if(width > 0 && width  < 1025 && height > 0 && height < 601){
                 
                              //0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF
						SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(picdata->data,width,height,24,width*3,
                          		 0xFF0000, 0x00FF00, 0x0000FF,0);
                        //   printf("iptv_image_open %d,%d, %d, %d %x \n",x,y,width,height,pSurface);
                        if(pSurface){
                                srt.x = 0;
                                srt.y = 0; 
                        	if(height == 160){
                        		rt.w = width;
                        		rt.h =  height - 30;
                        	}else{
                        		rt.w = width;
                        		rt.h = height;
                        	}
                        	srt.w = rt.w;
                        	srt.h = rt.h;    
                        	g_isUpdated = 1;
                        	SDL_BlitSurface(pSurface,&srt,g_pScreenSurface,&rt);
                        	//SDL_BlitSurface(pSurface,&srt,tmp_g_pScreenSurface,&rt);
                        	SDL_FreeSurface(pSurface);
                        	pSurface = NULL;   
                        }
                       
                        if(g_isUpdated == 1)
                        {
                                SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
                               	//SDL_Flip();
                        }
                        					
                }
                free(picdata);
                picdata = NULL;
        }else{
        	;//WRT_MESSAGE("������� buflen = %d",buflen);
        }
}	

void JpegDisp(int x,int y,unsigned char* buf,int buflen){
        if(buf == NULL || buflen == 0){
                return;
        }
        struct image_data *picdata;
        SDL_Surface* pSurface;
        uint32_t width;
        uint32_t height;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
		int start;

		pSurface = iptv_image_jpeg_read_sdl((unsigned char *)buf, buflen);

        if (pSurface != NULL)
        {
                width = pSurface->w;
                height = pSurface->h;
                
                if(pSurface){
                        srt.x = 0;
                        srt.y = 0; 
                	if(height == 160){
                		rt.w = width;
                		rt.h =  height - 30;
                	}else{
                		rt.w = width;
                		rt.h = height;
                	}
                	srt.w = rt.w;
                	srt.h = rt.h;                     
                	SDL_BlitSurface(pSurface,&srt,g_pScreenSurface,&rt);
                	//SDL_BlitSurface(pSurface,&srt,tmp_g_pScreenSurface,&rt);
                	SDL_FreeSurface(pSurface);
                	pSurface = NULL;   
                }
               
                
                if(g_isUpdated == 1)
                {
                        SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
                       // SDL_Flip(g_pScreenSurface);
                }	
           
        }else{
        	WRT_MESSAGE("������� buflen = %d",buflen);
        }
}

void JpegSingleDisp(int x,int y,unsigned char* buf,int buflen){
        if(buf == NULL || buflen == 0){
                return;
        }
        struct image_data *picdata;
        uint32_t width;
        uint32_t height;
        SDL_Rect rt;
        SDL_Rect srt;
        rt.x = x;
        rt.y = y;
        picdata = iptv_image_open((unsigned char *)buf, buflen,"jpeg");
   
        if (picdata != NULL)
        {
                width = picdata->width;
                height = picdata->height;
                if(width > 0 && width  < 1025 && height > 0 && height < 601){ 
						SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(picdata->data,width,height,24,width*3,
                          		 0xFF0000, 0x00FF00, 0x0000FF,0);
                        if(pSurface){
                                srt.x = 0;
                                srt.y = 0; 
                        	if(height == 160){
                        		rt.w = width;
                        		rt.h =  height - 30;
                        	}else{
                        		rt.w = width;
                        		rt.h = height;
                        	}
                        	srt.w = rt.w;
                        	srt.h = rt.h;                     
                        	SDL_BlitSurface(pSurface,&srt,g_pScreenSurface,&rt);
                        	//SDL_BlitSurface(pSurface,&srt,tmp_g_pScreenSurface,&rt);
                        	SDL_FreeSurface(pSurface);
                        	pSurface = NULL;   
                        }
						
                        SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
                    	//SDL_Flip(g_pScreenSurface);
                
                }
                free(picdata);
                picdata = NULL;
        }else{
			WRT_MESSAGE("������� buflen = %d",buflen);
        }
}

/**
*�������ƣ�OSD_JpegDisp
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void OSD_JpegDisp(int x,int y,unsigned char* buf,int buflen)
{
	if(buf == NULL || buflen == 0)
	    return;
	JpegDisp(x,y,buf,buflen);
}

/**
*�������ƣ�OSD_DrawText
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void OSD_DrawText(char* text,int x,int y,int color){
	if(text == NULL)
		return;
	DrawText(text,x,y,color,24);
}

void OSD_DrawText16(char* text,int x,int y,int color){
	if(text == NULL)
		return;
	DrawText(text,x,y,color,16);
}

/**
*�������ƣ�DrawText
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void DrawText(char* text,int x,int y,int color,int size){
        if(text == NULL)
                return;
       	SDL_Rect rt;
       	SDL_Color clr;
       	clr.r = (color & 0x00ff0000) >> 16;
       	clr.g = (color & 0x0000ff00) >> 8;
       	clr.b = (color & 0x000000ff);
       	rt.x = x;
       	rt.y = y - size;
       	if(rt.y < 0)
       	        rt.y =  0;
       	//rt.x += 20;
       	SDL_Surface* pSurface = WRT_RenderTextBlended(text,clr,size);
        if(pSurface){
       	   rt.h = pSurface->h;
       	   rt.w = pSurface->w;
       	   SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
       	   //SDL_BlitSurface(pSurface,NULL,tmp_g_pScreenSurface,&rt);
       	   SDL_FreeSurface(pSurface);
       	   pSurface = NULL;
		}
        if(g_isUpdated == 1)
        {
	    	SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
            //SDL_Flip(g_pScreenSurface);    
        }	
          
}

void DrawSingleText(char* text,int x,int y,int color,int size){
        if(text == NULL)
                return;
       	SDL_Rect rt;
       	SDL_Color clr;
       	clr.r = (color & 0x00ff0000) >> 16;
       	clr.g = (color & 0x0000ff00) >> 8;
       	clr.b = (color & 0x000000ff);
       	rt.x = x;
       	rt.y = y - size;
       	if(rt.y < 0)
       	        rt.y =  0;
       	//rt.x += 20;
       	SDL_Surface* pSurface = WRT_RenderTextBlended(text,clr,size);
        if(pSurface){
			rt.h = pSurface->h;
			rt.w = pSurface->w;//
			SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&rt);
			//SDL_BlitSurface(pSurface,NULL,tmp_g_pScreenSurface,&rt);
			SDL_FreeSurface(pSurface);
			pSurface = NULL;
		}

	SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
	//SDL_Flip(g_pScreenSurface);
          
}

void DrawText_monospaced_font(char* text,int x,int y,int color){
	if(text == NULL)
		return;

	DrawText(text,x,y,color,24); 
}

/**
*�������ƣ�DrawEnText
*���������
*���������
*��    �ܣ�
*��    �أ�
**/
void DrawEnText(char* text,int w,int h,int x,int y){
        if(text == NULL)
                return;

        int xoffset = 0;
      
        int textwidth = 0;
        char chText[50];
        char chText2[50];

        memset(chText,0,50);
        memset(chText2,0,50);
        strcpy(chText,text);
        
        textwidth = GetTextWidth(chText,0);
        xoffset = (w - textwidth )/2;
       
       
#if 0          
        if(xoffset < 0)
        {
                char* pTemp = strrchr(chText,' ');
                char* pTemp2 = NULL;
                if(pTemp)
                {
                        pTemp2 = pTemp+1;
                        *pTemp ='\0';
                        pTemp = chText;
                }else{
                        if(GetCurrentLanguage() == CHINESE){
                                int   first_text_len = strlen(chText);
                                int   str_len = 0;
                                while(first_text_len > 0){
                                        str_len = GetTextWidth((char*)chText,0);
                                        if(str_len > w){
                                                if((first_text_len >=2 ) && chText[first_text_len-1] >= 0xa0)
                                                        first_text_len -=2;
                                                else
                                                        first_text_len --;
                                                memset(chText2,0,50);
                                                memcpy(chText2,chText,first_text_len);
                                                str_len = GetTextWidth((char*)chText2,0);
                                                if((str_len)< w) //540
                                                        break;
                                        }else{
                                                memset(chText2,0,50);
                                                memcpy(chText2,chText,first_text_len);
                                                break;
                                        }
                                }
                                pTemp = chText2;
                                pTemp2 = (chText+first_text_len);
                        }else{
                                pTemp = chText;
                        }
                }         
                xoffset  = (w - GetTextWidth(chText,0) )/2;
                DrawText(pTemp,x+xoffset,y+h,WRT0RGB(255,255,255));

                xoffset = (w - GetTextWidth(pTemp2,0) )/2;
                DrawText(pTemp2,x+xoffset,y+h+24,WRT0RGB(255,255,255));

        }else{
#endif
                DrawText(chText,x+xoffset,y+h,WRT0RGB(255,255,255));
                
//        }
}



/**
*�������ƣ�DrawText_16
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void DrawText_16(char* text,int x,int y,int color){
        if(text == NULL)
                return;
      DrawText(text,x,y,color,16);
        		
}

/**
*�������ƣ�DrawText_16
*���������
*���������
*��    �ܣ�
*��    �أ�
*/

void DrawText_14(char* text,int x,int y,int color)
{
        if(text == NULL)
                return;
	DrawText(text,x,y,color,12);
        		
}

/**
*�������ƣ�GetTextWidth
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
int GetTextWidth(char* text,int is16)
{
	int w,h;
	w = h = 0;
        if(text == NULL)
                return 0;
        if(is16 == 1){
                WRT_TextSize(text,&w,&h,16);
                return w;
        }
        if(is16 == 2){
        	WRT_TextSize(text,&w,&h,16);
                return w;
        }
        if(is16 == 3){
        	WRT_TextSize(text,&w,&h,16);
                return w;
        }
        WRT_TextSize(text,&w,&h,24);
        return w;
}

/////////////////////////////////////////////////////////////////
void tDraw3dBox(int x,int y,int w,int h,unsigned long crTop,unsigned long  crBottom)
{
        WRT_Rect tmp;
        tmp.x = x;
        tmp.y = y;
        tmp.w = 1;
        tmp.h = h;
        FillRect(crTop,tmp);

        tmp.x = x;
        tmp.y = y;
        tmp.w = w;
        tmp.h = 1;
        FillRect(crTop,tmp);

        tmp.x = x+w;
        tmp.y = y;
        tmp.w = 1;
        tmp.h = h;
        FillRect(crBottom,tmp);

        tmp.x = x;
        tmp.y = y+h;
        tmp.w = w;
        tmp.h = 1;
        FillRect(crBottom,tmp);

}

//�������
void Color_FillRect(WRT_Rect rc,int r,int g,int b){
        WRT_Rect tmp = rc;
        WRT_Rect tmp1;
        unsigned long color = 0;
        int i = 0;
        DeflateRect(&tmp,1);

        for(i =0; i<rc.h-2;i++){
                tmp1.x = tmp.x;
                tmp1.y = tmp.y+i;
                tmp1.w = tmp.w;
                tmp1.h = 1;
                if(rc.h > 40 && rc.h < 255)
                        color = WRT0RGB(r?r:255-i,g?g:255-i,b?b:255-i);
                else if(rc.h > 255)
                        color = WRT0RGB(r,g,b);
                else
                        color = WRT0RGB(r?r:255-i*6,g?g:255-i*6,b?b:255-i*6);
                FillRect(color,tmp1);
        }
}

void DrawTextToRect4(WRT_Rect rc, char* text,unsigned long color)
{
        int x,y;
        x = rc.x+(rc.w - GetTextWidth(text,3))/2;
        y = rc.y+rc.h/2+8;
        DrawText_monospaced_font(text,x,y,color);
}

void DrawTextToRect3(WRT_Rect rc, char* text,unsigned long color,int is16)
{
        int x,y;
        x = rc.x+(rc.w - GetTextWidth(text,2))/2;
        y = rc.y+rc.h/2+8;
        DrawText_14(text,x,y,color);
}
void DrawTextToRect2(WRT_Rect rc, char* text,unsigned long color,int is16)
{
        int x,y;
        x = rc.x+(rc.w - GetTextWidth(text,1))/2;
        y = rc.y+rc.h/2+8;
        DrawText_16(text,x,y,color);
}

void DrawTextToRect(WRT_Rect rc, char* text,unsigned long color){
        int x,y;
        x = rc.x+(rc.w - GetTextWidth(text,0))/2;
        y = rc.y+rc.h/2+12;
        DrawText(text,x,y,color);
}

void tDraw3dInset(int x,int y,int w,int h)
{
        tDraw3dBox(x, y, w, h,WRT0RGB(162, 141, 104),WRT0RGB(234, 230, 221));
        ++x; ++y; w -= 2; h -= 2;
        tDraw3dBox( x, y, w, h,rgb_black,WRT0RGB(213, 204, 187));
}

void tDraw3dOutset(int x,int y,int w,int h){
        tDraw3dBox(x, y, w, h,WRT0RGB(213, 204, 187),WRT0RGB(0, 0, 0));
        ++x; ++y; w -= 2; h -= 2;
        tDraw3dBox( x, y, w, h,WRT0RGB(234, 230, 221),WRT0RGB(162, 141, 104));
}
/////////////////////////////////////////////////////////////////

/**
*�������ƣ�IsInside
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
int IsInside(WRT_Rect rect, WRT_xy xy){
	if((xy.x >rect.x)&&(xy.x < (rect.x+rect.w))&&(xy.y > rect.y)&&(xy.y < (rect.y+rect.h))){
		return 1;
	}else{
		return 0;
	}
}

/**
*�������ƣ�screen_open
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void screen_open(){

        if(m_issreenoff){
                m_issreenoff = false;
                tmGpioSetPin(CTL_IO_SETBACKLIGHT_POWER_ON,1);
                
        }
}

/**
*�������ƣ�screen_close
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void screen_close(){

	if(m_issreenoff == true)
		return;
        m_issreenoff = true;
        tmGpioSetPin(CTL_IO_SETBACKLIGHT_POWER_OFF,0);
}

void SwitchSkin(int skin)
{
	Window* pWindow = GetRootWindow();
	if(pWindow){
		if(skin == 0x01){
			pWindow->EventHandler = MainEventHandler;
			pWindow->NotifyReDraw = DrawMain;
		}else{
			pWindow->EventHandler = HandleMainUI2Event;
			pWindow->NotifyReDraw = DrawMainUI2;
		}
	}
}

/**
*�������ƣ�ReDrawCurrentWindow
*���������
*���������
*��    �ܣ�
*��    �أ�
*/
void ReDrawCurrentWindow(){
        if(g_MainProcess)
                g_MainProcess->NoitfyReDraw(1);
}

void ReDrawCurrentWindow2(int type){
        if(g_MainProcess)
                g_MainProcess->NoitfyReDraw(type);
}

static void DecodeRoom(unsigned char room[16]){
        char text[50] = {'\0'};
        char group[3] = {'\0'};  //���ź�2λ
        char temp1[4] = {'\0'};  //¥����3λ
        char temp2[3] = {'\0'};  //��Ԫ��2λ
        char temp3[4] = {'\0'};  //���3λ
        char temp4[4] = {'\0'};  //����3λ
        char temp5[3] = {'\0'};  //����2λ
        sscanf((char*)room,"%2s%3s%2s%3s%3s%2s",group,temp1,temp2,temp3,temp4,temp5);
        if(strncmp((char*)room,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0)
        {
                sprintf(text,"%s�ŷֻ�",temp5);
                if(GetCurrentLanguage() == ENGLISH)
	                sprintf(text,"N(%s)",temp5);
        }else{
                sprintf(text,"%s��%s��%s��Ԫ%s��%s��",group,temp1,temp2,temp3,temp4,temp5);
                if(GetCurrentLanguage() == ENGLISH)
	                sprintf(text,"G(%s) B(%s) U(%s) F(%s) R(%s)",group,temp1,temp2,temp3,temp4);

        }
        //DrawText(text,30,70,rgb_white);
        viewcallinfo(text);
}

void toinet_addr(unsigned long ip_addr, unsigned char *localip){
        int    i,j;
        unsigned char *ip;

        localip[0] = '0';localip[1] = '.';
        localip[2] = '0';localip[3] = '.';
        localip[4] = '0';localip[5] = '.';
        localip[6] = '0';localip[7] = '\0';

        i = 0;
        ip = (unsigned char *)(&ip_addr);

        for(j = 0; j < 4; j ++)
        {
                if(ip[j]/100)
                        localip[i++] = ip[j]/100+0x30;

                if(ip[j]%100/10)
                        localip[i++] = ip[j]%100/10+0x30;
                else if(ip[j]/100)
                        localip[i++] = ip[j]%100/10+0x30;

                localip[i++] = ip[j]%10+0x30;
                if (j < 3)
                        localip[i++] = '.';
                else
                        localip[i++] = '\0';
        }
}


int GetkeepscreenStatus(){
        return pSystemInfo->mySysInfo.iskeepscreen;
}


void SetkeepscreenStatus(int status){
        pSystemInfo->mySysInfo.iskeepscreen =  status;
#if HAVE_SCREEN_KEEP
        screenkeep_init();
#endif
}




#if HAVE_SCREEN_KEEP

static unsigned char* get_screenkeep_pic_data(char* url,int* size){
        FILE *fp;
        int ws;
        unsigned long   total=0;
        unsigned long len = 0;
        unsigned char* read_data = NULL;
        printf("url = %s\n",url);
        if ((fp = fopen(url, "rb"))!=NULL)
        {
                fseek(fp,0,2);
                len = ftell(fp);
                fseek(fp,0,0);
                read_data = (unsigned char*)ZENMALLOC(len);
                if(read_data == NULL){
                        fclose(fp);
                        fp = 0;
                        return 0;
                }
                do
                {
                        if ((ws  = fread(read_data+total,1,len-total,fp))>0){
                                total+=ws;
                        }

                }while(ws > 0);
                fclose(fp);
                fp = NULL;
                *size = len;
        }
        else
        {
                printf("open url faield \n");
                return NULL;
        }
        return read_data;
}

void init_screenkeep(){

        if(pSystemInfo->mySysInfo.iskeepscreen == 0)
                return;

        if(ImagePointer[0] != 0 && ImagePointer[0] != buffer1 ){
                for(int i=0;i<6;i++){
                        if(ImagePointer[i])
                                ZENFREE(ImagePointer[i]);
                        ImagePointer[i] = 0;
                        ImageSize[i] = 0;
                }
        }

        int size =0;
        char url[100];
        char tmpurl[100];
        unsigned long ip1;
        char ip2[16];
        int ret = 0;
        wrthost_get_centerip(&ip1);
        toinet_addr(ip1,(unsigned char*)ip2);
        sprintf(url, "http://%s",ip2);
        sprintf(tmpurl,"%s/%s",url,"pic.html");
        WRT_DEBUG("read_all_screensave_image");
        ret = read_all_screensave_image(ImagePointer,ImageSize);
        gScreenkeepcount = ret;
        //html_text = "count@pic1.jpg@pic2.jpg@pic3.jpg@.....";
        // unsigned char* html_text = get_screenkeep_pic_data(tmpurl,&size);
        /*        if(ret ){

        char* tmp  = (char*)html_text;
        char* tmp1 = strchr((char*)tmp,'@');
        if(tmp1){
        char* tmp2 = tmp1+1;
        *tmp1 = '\0';
        int count = atoi(tmp);
        if(count > 0 && count < 7){
        for(int i =0; i< count;i++){
        char* tmp3 = strchr(tmp2,'@');
        if(tmp3){
        char* tmp4 = tmp3+1;
        *tmp3 = '\0';
        char* pic = tmp2;
        memset(tmpurl,0,100);
        sprintf(tmpurl,"%s/%s",url,pic);
        size = 0;
        ImagePointer[i] = get_screenkeep_pic_data(tmpurl,&size);
        ImageSize[i]= size;
        tmp2= tmp4;
        }
        }
        gScreenkeepcount = count;
        }
        }
        ZENFREE(html_text);
        html_text = NULL;

        }else{
        */
        if(ret == 0){
                ImagePointer[0] = buffer1;
                ImageSize[0] = sizeof(buffer1);

                ImagePointer[1] = buffer2;
                ImageSize[1] = sizeof(buffer2);

                ImagePointer[2] =0;
                ImageSize[2] = 0;

                ImagePointer[3] = 0;
                ImageSize[3] = 0;

                ImagePointer[4] = 0;
                ImageSize[4] = 0;

                ImagePointer[5] = 0;
                ImageSize[5] = 0;
                gScreenkeepcount = 2;
        }


}

void display_screenkeep_image(int index){

        JpegDisp(0,0,ImagePointer[index],ImageSize[index]);
        
#if 0
        switch(index){
    case 0:
            JpegDisp(0,0,buffer1,sizeof(buffer1));
            break;
    case 1:
            JpegDisp(0,0,buffer2,sizeof(buffer2));
            break;
    case 2:
            JpegDisp(0,0,buffer3,sizeof(buffer3));
            break;
    case 3:
            JpegDisp(0,0,buffer4,sizeof(buffer4));
            break;
    case 4:
            JpegDisp(0,0,buffer5,sizeof(buffer5));
            break;
    case 5:
            JpegDisp(0,0,buffer6,sizeof(buffer6));
            break;
    default:
            break;
        }
#endif

}
#endif


#ifdef TEST_FFS
static bool save_file(char* name,unsigned char* addr,long len){
        FILE* file = 0;
        file = fopen(name,"ab");
        if(file){
                fseek(file,0,SEEK_SET);
                long write_size = fwrite(addr,1,len,file);
                if(write_size == len){
                        // write_size = fwrite("0",1,1,file);
                        //if(write_size == 1){
                        fclose(file);
                        printf("write ok write_size = %d \n",write_size);
                        return true;
                        //}
                }
                printf("errno = %d  write_size=%d \n",errno,write_size);
                fclose(file);
                unlink(name);
        }
        printf("errno = %d \n",errno);

        return false;
}


void test_file(){
#if 0
        FILE* file=0;
        int len;
        int len1;
        int i;
        len1 = sizeof(buffer3);
        char filepath[200];
        while(i<10){
                sprintf(filepath,"/norffs0/root/%d.txt",i+1);
                save_file(filepath,buffer3,sizeof(buffer3));
                i++;
        }

        SDL_Delay(1000);
        i=0;
        unsigned char* tmpbuf = (unsigned char*)ZENMALLOC(len1);
        while(i<10){
                sprintf(filepath,"/norffs0/root/%d.txt",i+1);
                file = fopen(filepath,"r");

                if(file){
                        len = fread(tmpbuf,1,len1,file);
                        printf("len = %d \n",len);
                        if(len != len1){
                                printf("read file %d error code,%x\n",i,errno);
                        }
                        fclose(file);
                        file = 0;
                        if(len == len1){
                                for(int j=0; j<len1;j++){
                                        if(tmpbuf[j] != buffer3[j]){
                                                printf("crc file %x error %x\n",tmpbuf[j],buffer3[j]);
                                                break;
                                        }
                                }

                        }

                }
                i++;
        }
#endif
#if 0
        printf("over........\n");

        SDL_Delay(1000);
        i=0;
        while(i<10){
                sprintf(filepath,"/norffs0/root/%d.txt",i+1);
                unlink(filepath);
                i++;
        }
        printf("del over........\n");
#endif
}
#endif

//static char loclhost_config_path[] ="/norffs0/root/config.txt";

bool save_localhost_config_to_flash(){
        FlushSystemInfo();
        return true;
}
extern int g_videobright;
extern int g_videocontrast;
extern int g_videosaturation;

void init_localhost_config(){
        //���úø������ò�����������
        long len =0;
        int i =0;
        int j = 0;
        unsigned char* addr =0;
        char* ctmp = NULL;
        long h,m;
        //��ѯ�Լ�����չ�ֻ�,���·����ַ���ʱ�򣬻��ٴβ�ѯһ�Ρ�
        unsigned long exip[4];
        char exid[4][16];
        char tmpid[16];
        char tmp[3]={'\0','\0','\0'};
        int num = 4;
        int count =0;
        unsigned long activeip = 0;
        memset(tmpid,0,16);

        for(i = 1; i<9; i++)
        {
    		subroomipaddr[j] = get_sub_room_ip(i);
    		j++;
                        }
        if(g_localid != 0x01)
        	activeip = subroomipaddr[0];

        Init_ipcamera_config();

        for(i=0;i<8;i++){ //��������
                addr = 0;
                len = 0;
                ctmp = pSystemInfo->mySysInfo.ringname[i];

                if(ctmp[0] != 0xff && strlen(ctmp) > 0){
                        wrtip_set_ring(i,ctmp,strlen(ctmp));
                }else{
                		memset(pSystemInfo->mySysInfo.ringname[i],0,256);
                        wrtip_set_ring(i,0,0);
                }
        }

        if(pSystemInfo->mySysInfo.ringtimes == 0 || pSystemInfo->mySysInfo.ringtimes == 0xffffffff)
                pSystemInfo->mySysInfo.ringtimes = 40;

        SetRingtimes(pSystemInfo->mySysInfo.ringtimes); //��������ʱ��,/��������ֻ�������ͬ��

        if(pSystemInfo->mySysInfo.avoidtimes == 0 || pSystemInfo->mySysInfo.avoidtimes == 0xffffffff)
                pSystemInfo->mySysInfo.avoidtimes = 43200; //Ĭ����12Сʱ
        if(pSystemInfo->mySysInfo.agenttimes == 0 || pSystemInfo->mySysInfo.agenttimes == 0xffffffff)
                pSystemInfo->mySysInfo.agenttimes = 43200;

        h = pSystemInfo->mySysInfo.avoidtimes/3600;  //���������ʱ��
        m = (pSystemInfo->mySysInfo.avoidtimes%3600)/60;

        if(pSystemInfo->mySysInfo.isavoid == 1 ){ //��������ֻ�������ͬ��
                SetAvoidtimes(h,m);
        }else{
                pSystemInfo->mySysInfo.isavoid  =0;
                SetAvoidtimes(0,0);
        }

        if(pSystemInfo->mySysInfo.alarmtime == 0xffffffff)
                pSystemInfo->mySysInfo.alarmtime = 100;
        if(pSystemInfo->mySysInfo.alarmtime1 == 0xffffffff)
                pSystemInfo->mySysInfo.alarmtime1 = 40;

        if(pSystemInfo->mySysInfo.talkvolume == 0xffffffff || pSystemInfo->mySysInfo.talkvolume == 0)
                pSystemInfo->mySysInfo.talkvolume = 100;
        //SetVolume(pSystemInfo->mySysInfo.talkvolume);

        if(pSystemInfo->mySysInfo.alarmvolume == 0xffffffff  || pSystemInfo->mySysInfo.alarmvolume == 0)
                pSystemInfo->mySysInfo.alarmvolume = 100;

        if(pSystemInfo->mySysInfo.isagent == 1){
                h = pSystemInfo->mySysInfo.agenttimes/3600;
                m = (pSystemInfo->mySysInfo.agenttimes%3600)/60;
                SetAgenttimes(h,m);
        }else{
                pSystemInfo->mySysInfo.isagent = 0;
                SetAgenttimes(0,0); //��������ֻ�������ͬ��
        }

        if(pSystemInfo->mySysInfo.ringvolume == 0xffffffff|| pSystemInfo->mySysInfo.ringvolume ==0 )
                pSystemInfo->mySysInfo.ringvolume = 100;
        SetVolume(pSystemInfo->mySysInfo.ringvolume);


        if(pSystemInfo->mySysInfo.isbuttonvoice == 0xffffffff){
                pSystemInfo->mySysInfo.isbuttonvoice = 0;
                OpenAndCloseButtonVoice(0);
        }else{
                OpenAndCloseButtonVoice(pSystemInfo->mySysInfo.isbuttonvoice); //�����Ƿ��а�����
        }

        if(pSystemInfo->mySysInfo.isrecord == 0xffffffff)
                pSystemInfo->mySysInfo.isrecord = 1;
        if(pSystemInfo->mySysInfo.isremote == 0xffffffff)
                pSystemInfo->mySysInfo.isremote = 1;
        if(pSystemInfo->mySysInfo.alarmvoicetime == 0xffffffff)
                pSystemInfo->mySysInfo.alarmvoicetime = 300; //��

        if(pSystemInfo->mySysInfo.isautocap > 1 || pSystemInfo->mySysInfo.isautocap < 0){
                pSystemInfo->mySysInfo.isautocap  = 0; //Ĭ������������Զ�ץ�ĵ�
        }
        
        if(pSystemInfo->mySysInfo.bright >= 0 && pSystemInfo->mySysInfo.bright <= 100){
//            
        }else{
                pSystemInfo->mySysInfo.bright =  20;
        }

        if(pSystemInfo->mySysInfo.contrast >= 0 && pSystemInfo->mySysInfo.contrast <= 100){
              
        }else{
                pSystemInfo->mySysInfo.contrast = 10;
        }

        if(pSystemInfo->mySysInfo.saturation >= 0 && pSystemInfo->mySysInfo.saturation <= 100){
              
        }else{
                pSystemInfo->mySysInfo.saturation = 40;
        }
   
		g_videobright = pSystemInfo->mySysInfo.bright;
	    g_videocontrast = pSystemInfo->mySysInfo.contrast;
	    g_videosaturation = pSystemInfo->mySysInfo.saturation;

        if(pSystemInfo->mySysInfo.safestatus[0] == false) //���ð���״̬
                pSystemInfo->mySysInfo.safestatus[0] = pSystemInfo->mySysInfo.safestatus[1] =true;

        if(pSystemInfo->mySysInfo.isjiaju == 0xffffffff)
                pSystemInfo->mySysInfo.isjiaju = 0;
        if(pSystemInfo->mySysInfo.iskeepscreen == 0xffffffff)
                pSystemInfo->mySysInfo.iskeepscreen = 0;
        if(pSystemInfo->mySysInfo.ishttpdown == 0xffffffff)
                pSystemInfo->mySysInfo.ishttpdown = 0;
        if(pSystemInfo->mySysInfo.NumOfMp3 == 0xffffffff){
                pSystemInfo->mySysInfo.NumOfMp3 = 0;
                pSystemInfo->mySysInfo.Mp3Position[0] = 0;
                pSystemInfo->mySysInfo.Mp3Position[1] = 0;
                pSystemInfo->mySysInfo.Mp3Position[2] = 0;
                pSystemInfo->mySysInfo.Mp3Position[3] = 0;
                pSystemInfo->mySysInfo.Mp3Position[4] = 0;
                pSystemInfo->mySysInfo.Mp3Len[0] = 0;
                pSystemInfo->mySysInfo.Mp3Len[1] = 0;
                pSystemInfo->mySysInfo.Mp3Len[2] = 0;
                pSystemInfo->mySysInfo.Mp3Len[3] = 0;
                pSystemInfo->mySysInfo.Mp3Len[4] = 0;
        }

        if(pSystemInfo->mySysInfo.idrule[0] == 0xffffffff){
                pSystemInfo->mySysInfo.idrule[0]  =2;
                pSystemInfo->mySysInfo.idrule[1]  =3;
                pSystemInfo->mySysInfo.idrule[2]  =2;
                pSystemInfo->mySysInfo.idrule[3]  =3;
                pSystemInfo->mySysInfo.idrule[4]  =3;
        }
        if(activeip != 0){ //�ӷֻ�����ʱ�����ֻ�����ͬ��
                wrthost_req_sync(activeip);
        }

        if(pSystemInfo->LocalSetting.privateinfo.LocalIP!= 0 && pSystemInfo->LocalSetting.privateinfo.LocalIP!= 0xffffffff)
                set_host_ip(pSystemInfo->LocalSetting.privateinfo.LocalIP);
        if(pSystemInfo->LocalSetting.privateinfo.SubMaskIP != 0 &&
                pSystemInfo->LocalSetting.privateinfo.SubMaskIP != 0xffffffff)
                set_host_mask(pSystemInfo->LocalSetting.privateinfo.SubMaskIP);
        if(pSystemInfo->LocalSetting.privateinfo.GateWayIP != 0 &&
                pSystemInfo->LocalSetting.privateinfo.GateWayIP != 0xffffffff )
                set_host_gateway(pSystemInfo->LocalSetting.privateinfo.GateWayIP);
        wrthost_set_centerip(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP) ;//ֻ�������ĵ�ַ
        wrthost_set_managerip(pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP);    //ֻ�����������ַ

        if(pSystemInfo->mySysInfo.maxphoto != 0xffff && pSystemInfo->mySysInfo.maxphoto  < 100 &&  pSystemInfo->mySysInfo.maxphoto  > 0){
                Set_Max_photo(pSystemInfo->mySysInfo.maxphoto);
        }else
                pSystemInfo->mySysInfo.maxphoto = 100;

        if(pSystemInfo->mySysInfo.photoqualit != 0xffff && pSystemInfo->mySysInfo.photoqualit  < 100 &&  pSystemInfo->mySysInfo.photoqualit  > 0x0){
                SetPhotoQuality(pSystemInfo->mySysInfo.photoqualit);
        }else
                pSystemInfo->mySysInfo.photoqualit = 100;

        if(pSystemInfo->mySysInfo.bakliftip == 0xffffffff)
                pSystemInfo->mySysInfo.bakliftip = 0;

        if(pSystemInfo->mySysInfo.isnvo == 0xff) //�Ƿ�ҽӷǿ��ӷֻ���
                pSystemInfo->mySysInfo.isnvo = 0;

        if(pSystemInfo->mySysInfo.framerate > 10 || pSystemInfo->mySysInfo.framerate < 5)
                pSystemInfo->mySysInfo.framerate  = 7;

        if(pSystemInfo->mySysInfo.hassip == 0xff)
                pSystemInfo->mySysInfo.hassip = 0;

        if(pSystemInfo->mySysInfo.isenglish != 0x0 && pSystemInfo->mySysInfo.isenglish != 0x1)
                pSystemInfo->mySysInfo.isenglish = 0; //���İ�  
        if(pSystemInfo->mySysInfo.phonenumber[0][0] == 0xff)
                memset(pSystemInfo->mySysInfo.phonenumber,0,120);
        if(pSystemInfo->mySysInfo.isbomb != 0x1)
                pSystemInfo->mySysInfo.isbomb = 0x0;

#ifdef USE_TESHU_2
        pSystemInfo->mySysInfo.isdoorbell = 0x1;
#else
        if(pSystemInfo->mySysInfo.isdoorbell != 0x0 && pSystemInfo->mySysInfo.isdoorbell != 0x1)
                pSystemInfo->mySysInfo.isdoorbell = 0x0;
#endif       

#ifdef WRT_MORE_ROOM
        for(i = 0; i<16;i++)
                pSystemInfo->mySysInfo.safestatus[i+16] = 0x1;
#endif

        pSystemInfo->mySysInfo.isnvo = 0x1; //�汾3.32.00 Ĭ����֧��ģ��ֻ���
/*
        if(pSystemInfo->mySysInfo.screensize != 0x10 && pSystemInfo->mySysInfo.screensize != 0x7)
#ifdef IS10ROOM
        	pSystemInfo->mySysInfo.screensize = 0x10;
#else        	
                pSystemInfo->mySysInfo.screensize = 0x7; //Ĭ�ϲ���7����Ļ��
#endif  
*/
/*                
#ifdef IS10ROOM
        pSystemInfo->mySysInfo.screensize = 0x10;
#endif 
*/
	if(pSystemInfo->mySysInfo.isuseui != 0x1 && pSystemInfo->mySysInfo.isuseui != 0x2)
		pSystemInfo->mySysInfo.isuseui = 0x2;
	if(pSystemInfo->mySysInfo.scenenumber[0] == 0xff ||pSystemInfo->mySysInfo.scenenumber[0] == 0){
		for(i =0; i<16 ;i++){
			pSystemInfo->mySysInfo.scenenumber[i] = i+1;
		}
	}
	if(pSystemInfo->mySysInfo.gatewayip !=0 && pSystemInfo->mySysInfo.gatewayip  != 0xffffffff)
		connect_net_jiaju_gateway(); //�����������ӵ�����Ҿ����� 
}

//[2011-5-16 11:59:15]
static void jiaju_result_callback(unsigned long tmpmsg[4])
{
        unsigned long msg[4];
        msg[0] = STATUS_CHECK_JIAJU;
        msg[1] = tmpmsg[0];//1 SUCC 0 FAIL
        msg[2] = tmpmsg[1];
        msg[3] = 1;
        WRT_DEBUG("�Ҿӽ���ص�����");
      
        if(g_MainProcess){

#if HAVE_USED_SDL 
                g_MainProcess->Send_msg(msg,sizeof(unsigned long)*4);
#else

                g_MainProcess->Send_msg(msg);
#endif
        }
      
        WRT_DEBUG("�Ҿӷ�����%d,%d ",tmpmsg[0],tmpmsg[1]);
}

static int alarm_clock_close(int param)
{
        if(wrthost_sysidle()){
                mp3stop();
        }
        return 0;	
}

void DrawHuanjinValue(int type,int res)
{
	Window* pWindow = NULL;
	pWindow =  WindowGetTopVisibleWindow();
	if(pWindow->type != ROOT )
		return;
	if(m_issreenoff == true)
		return;	
        if(type == 0 && res == 0)
        {
                if(!isganyinqi()) //���û�����ø�Ӧ��
                        return;

        }
        
        WRT_Rect rt;	
        char tmp[20];
        int offset = 10;
        if(pSystemInfo->mySysInfo.isuseui == 0x01){
        	rt.x = 450;
        	rt.y = 20;
        	rt.h = 30;
        	rt.w = 200;
        	FillRect(BACK_COLOR,rt);
		}else{
				rt.x = 450;
                rt.y = 100;
                rt.w = 200;
                rt.h = 30;                              
                ClearRect(rt.x,rt.y,rt.w,rt.h);
		}
        memset(tmp,0,20);
        rt.w = 0;
        if(type == RESULT_WENDU){
                g_wendu_value = res;
        }else if(type == RESULT_SHIDU){
                g_shidu_value = res;
        }else if(type == RESULT_KONGQI){
                g_kongqi_value = res;
        }
        
        if(g_wendu_value > 0){
                sprintf(tmp,"%d��",g_wendu_value);
                WRT_DEBUG("��ѯ������ֵ:%s",tmp);
                rt.w = GetTextWidth(tmp,1);
                if(pSystemInfo->mySysInfo.isuseui == 0x01){
                	FillRect(BACK_COLOR,rt);
                	DrawTextToRect2(rt,tmp,rgb_white,1);
                }else{
                	
                	ClearRect(rt.x,rt.y,rt.w,rt.h);
                	
                	OSD_DrawText(tmp,rt.x,rt.y+25,rgb_white);
                }
        }
        if(g_shidu_value > 0){
                rt.x += rt.w + offset;
                sprintf(tmp,"%d%%",g_shidu_value);
                rt.w = GetTextWidth(tmp,1);
                if(pSystemInfo->mySysInfo.isuseui == 0x01){
                	FillRect(BACK_COLOR,rt);
                	DrawTextToRect2(rt,tmp,rgb_white,1);
                }else{
                	ClearRect(rt.x,rt.y,rt.w,rt.h);
                	
                	OSD_DrawText(tmp,rt.x,rt.y+25,rgb_white);
                }
        }

        if(g_kongqi_value > 0){        
                rt.x += rt.w + offset;
                sprintf(tmp,"%dPPM",g_kongqi_value);
                rt.w = GetTextWidth(tmp,1);
                if(pSystemInfo->mySysInfo.isuseui == 0x01){
                	FillRect(BACK_COLOR,rt);
                	DrawTextToRect2(rt,tmp,rgb_white,1);
                }else{
                	ClearRect(rt.x,rt.y,rt.w,rt.h);

                	OSD_DrawText(tmp,rt.x,rt.y+25,rgb_white);
                }                                 
        }
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

///////////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////////
int CMainProcess::ThreadMain(void){
        uint32_t len = 0;
        int status = -1;
        int ret = 0;
        long pos[3];
        unsigned long       event[4];
        unsigned char* buf = 0;
        while(1)
        {
            int rc;
            if (m_bstart) {
                    rc = SDL_SemWait(m_myMsgQueueSemaphore);//���͵�ʱ��post
            } else {
                    rc = SDL_SemWait(m_myMsgQueueSemaphore);
            }
            if (rc == -1) {
                    break;
            }
            if(rc == 0){
                    CWRTMsg* pMsg = m_myMsgQueue.get_message();
                    if (pMsg != NULL) {
						printf("ThreadMain:get_message:value = %d\r\n",pMsg->get_value());
                        switch (pMsg->get_value()) 
                        {
                    		case MSG_NODE_GUI_POS://ͼ�������Ӧ����,����ǰ������¼�
                            {
                                uint32_t x=0,y=0;
                                WRT_xy xy;
#ifdef WRT_MORE_ROOM
                                unsigned char cid = 0;
#endif
                                buf = (unsigned char*)pMsg->get_message(len);
                                if(buf == NULL)
                                        break;
                                memcpy(pos,buf,sizeof(long)*3);
                                //WRT_DEBUG("x %d  y %d  status  %d",pos[0],pos[1],pos[2]);
                                if(pos[2] == 1)//pos[0]x pos[1]y pos[2]status
                                {
                                        set_down_time();
                                }else{
                                        set_up_time();
                                }
                                xy.x = pos[0];
                                xy.y = pos[1];
                                if(g_iscapphoto == 1){
                                	if((pos[2] == 1) && IsInside(g_caprt,xy)){
                                		SendCapJpg();
                                	}
                        		}
                        		//printf("x %d  y %d  status  %d\n",pos[0],pos[1],pos[2]);
#ifdef WRT_MORE_ROOM
                                cid  = pos[2] & 0x000000ff;
                                pos[2] = (pos[2] >> 16) & 0x0000ffff;

                                printf("recv pos = %d,%d ,%d -cid = %d\n",pos[0],pos[1],pos[2],cid);

#if 0                                         
                                if(pos[2] == 0)
                                {
                                        tm_getticks(NULL,&g_adjustticks2);
                                        if(g_adjustticks1 != 0){
                                                printf("time(%x,%x),diff(%x)\n",g_adjustticks1,g_adjustticks2,g_adjustticks2 - g_adjustticks1);
                                                if((long)(g_adjustticks2 - g_adjustticks1) > 5000)
                                                {
                                                        g_adjustticks1  = g_adjustticks2 = 0;
                                                        screen_adjust(0);
                                                }
                                                g_adjustticks1  = g_adjustticks2 = 0;
                                        }

                                }else if(pos[2] == 1)
                                {
                                        tm_getticks(NULL,&g_adjustticks1);
                                        g_adjustticks2 = g_adjustticks1;
                                }
#endif                                         


                                if(wrthost_get_sys_status() == SYS_IDLE)
                                {
                                    if(pos[2] == 0x00 &&  (get_screen_state(cid) == 0) )
                                    {

                                        open_screen_power(cid,1,1); //����Ļ����ϵͳ����ʱ��

                                    }

                                }else
                                {
                                    if(GetTopWindowID() != 0){
                                        if(GetTopWindowID() != cid){

                                            if(pos[2] == 0x00)
                                                    set_sub_room_ap_busy(cid);
                                            break;
                                        }

                                    }
                                }



#endif
                                Window* pWindow = WindowGetTopVisibleWindow();
                                if(pWindow && pWindow->EventHandler){
#ifdef WRT_MORE_ROOM
                                        SetTopWindowID(cid); //����Ǳ��ж������ڣ�����ҪԤ������ID��
#endif				
                                        ret = pWindow->EventHandler(pos[0],pos[1],pos[2]);
                                        if(ret == 1 && pos[2] == 1 && g_ishandler == false){
                                                g_ishandler = true;
                                                break;
                                        }
                                        if(g_ishandler == true && ret == 0 && pos[2] == 0){
                                                pWindow->NotifyReDraw2(pWindow);
                                                g_ishandler = false;

                                        }else{
                                                g_ishandler = false;

                                        }
                                }
#ifdef WRT_MORE_ROOM
                                SetTopWindowID(cid); //�����������ж������ڣ�����Ҫ��������ID.
#endif
                            }
                            break;
                            
                    case MSG_NODE_VIEW_SCEENKEEP:
                    	{
                        	buf = (unsigned char*)pMsg->get_message(len);
                        	if(buf == NULL)
                                	break;
                        	int index = *(unsigned int*)buf;

                        	if(wrthost_sysidle())
                                	display_screenkeep_image(index);
                        }
                        break;
                        
                    case MSG_NODE_NOTIFY_REDRAW://
                    {
                            buf = (unsigned char*)pMsg->get_message(len);
                            if(buf == NULL)
                                    break;
							long wintype = *(unsigned long*)buf;
                            Window* pWindow = WindowGetTopVisibleWindow();
                            WRT_MESSAGE("wintype = %d %d ",wintype,pWindow->type);
                            if(wintype < 12 ){
                                if(pWindow && pWindow->type == ROOT)
                                {
                                    switch(wintype)
                                    {
                                    case 0:
                                    case 1:
                                    case 2:
                                            pWindow->NotifyReDraw2(pWindow);
                                            break;
                                    case 3:
                                            UpdateAlarmIcon(1);
                                            break;
                                    case 4:
                                            UpdateMsgIcon();
                                            break;
                                    case 5:
                                            UpdateNetIcon();
                                            break;
                                    case 6:
                                            UpdateAVIcon();
                                            break;
                                    case 7:
                                            UpdateTimeIcon();
                                            break;
 				    				case 9:
                                    case 10:
                                    case 11:
                                    	UpdateReplaceJpeg(wintype-9);
                                    	ReUpdateUI2(wintype-9);
                                    	break;                                            
                                    }

                                }else{
                                        
                                    if(pWindow && (pWindow->type == 1) && pWindow->NotifyReDraw)
                                            pWindow->NotifyReDraw2(pWindow);
                                }
                            }else if(wintype == 12){
                                if(pWindow && pWindow->type != ROOT){
                                    if(pWindow->type == SAFEWIN)
                                        pWindow->NotifyReDraw2(pWindow);  
                                    else if(pWindow->type != CALLED && pWindow->type != CALLING && pWindow->type != MUTILMEDIA ){
                                        CloseAllWindows();
                                            
                                    }
                                           
                                }else{
                                   pWindow->NotifyReDraw2(pWindow);  
                                }
                            }else{
                               if(pWindow && pWindow->NotifyReDraw)
                                   pWindow->NotifyReDraw2(pWindow);
                            }
                    }
                    break;

                    case MSG_NODE_STATUS:
                    {	
                        unsigned long tmpevent[4];
                        memset(tmpevent,0,sizeof(unsigned long)*4);
                        buf = (unsigned char*)pMsg->get_message(len);
                        if(buf == NULL)
                        	break;
                        memcpy(tmpevent,buf,sizeof(unsigned long)*4);
                        ProcessEvent(tmpevent);//�����¼�

                    }
                    break;
                    
                    case MSG_NODE_STOP_THREAD://����process
	                    m_bstart = false;
	                    pMsg->free_message();
	                    ZENFREE(pMsg);
	                    return 0;

	                //	break;
	                    
                    case MSG_NODE_START://��ʼ��ϵͳ
                        Init();
                        m_bstart = true;
                        break;
                        
                    case MSG_NODE_STOP:
                        break;
                        
                    case MSG_KEY: //�յ���ֻ��İ�����Ϣ �ͷ����ı���Ϣ
                        break;
                        
                    default:
                        break;
                        
                        }
                }
                    
                if(buf != NULL){
                        ZENFREE(buf);
                }
                if(pMsg != NULL)
                        ZENFREE(pMsg);
                buf = NULL;
                pMsg = NULL;
            }
    	}
}

int CMainProcess::FlashThreadMain(void){
        return 0;
}

#if 1
static  unsigned long test_tid;

void test_task(){
		FILE *fp =NULL;
        SDL_Delay(30000);
        unsigned long tmpmsg[4];
        int count =0;
        printf("����test_task \n");

        fp = fopen("cnt.txt","w");
        if(fp == NULL)
        	return;
        //char buf[16];
        while(1){
        		//memset(buf,0,16);
        		//fp = fopen("cnt.txt","a+");
                tmpmsg[0] = ROOM_STARTMONT ;
                tmpmsg[1] = 0;
                tmpmsg[2] = 0;
                tmpmsg[3] = 0;
                //wrthost_set_peer_LocalID("020020100000001");
                wrthost_set_montip(0x3801a8c0);
                printf("����\n");
                wrthost_send_cmd(tmpmsg);

                SDL_Delay(5000);
                tmpmsg[0] = ROOM_STOPMONT ;
                tmpmsg[1] = 0;
                tmpmsg[2] = 0;
                tmpmsg[3] = 0;
                printf("ֹͣ\n");
                wrthost_send_cmd(tmpmsg);
                SDL_Delay(5000);
                count++;
                printf("���ӵĴ���Ϊ��%d \n",count);
                //sprintf(buf,"-%d",count);
				//int ret = fwrite(buf,1,strlen(buf),fp);
				//printf("%d:%s\n",ret,buf);
				//fclose(fp);
        }
}

#endif

void CMainProcess::UnInit()
{
	uninit_alarm_handle();
}
		
void CMainProcess::Init(){

        unsigned long tmpevent[4];
        int i,j;
        unsigned long tmpip[4];
        unsigned char buf[2]={0,0};
        tmpip[0] = pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
        tmpip[1] = pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
        tmpip[2] = pSystemInfo->LocalSetting.publicinfo.CenterWarningIP;
        tmpip[3] = pSystemInfo->LocalSetting.publicinfo.CenterMinorIP;
        j =0;
        for(j=0;j<3;j++)
                for(i=j+1;i<4;i++){
                        if(tmpip[j] == tmpip[i])
                                tmpip[i] = 0;
                }
                //WRT_MESSAGE("������ַ��0x%x,0x%x,0x%x,0x%x",tmpip[0],tmpip[1],tmpip[2],tmpip[3]);
                //WarningInit(0,tmpip[0],
                //        tmpip[1],
                //        tmpip[2],
                //        tmpip[3],
                //        0);
                m_issreenoff = false;
                
                WindowManagerInit();
				

                //wrthost_init();
                
                wrthost_set_displaycallback((display)disp_callback);

                init_alarm_handle();

                //init_jiaju_table();

                init_scene_mode_table();

                InitsceneTimer();

                //init_pool_jiaju(jiaju_result_callback);
                //init_net_jiaju(NULL);

				

                init_net_jiaju(jiaju_result_callback);


                SDL_Delay(1000);
                init_localhost_config();
                g_have_msg = isnoreadmsg();



				if(pSystemInfo->mySysInfo.isuseui == 0x02){
			
				
                	CreateMainUI2();//822ע��
                	
                }else{
                	CreateMainWin(); //822ע��
                			//CreateMainUI2(0);
                }





#ifdef USE_DESKTOP //  [2011��3��4�� 16:13:15 by ljw]
                CreateDesktop();
               // CreateMainWin(); 
#endif
               // tmRxFilterDisableVideoAudioPort();//�ر���Ƶ����Ƶ�˿ڡ��ſ������˿�
               
                tmpevent[0] = ROOM_GETTIME;
                tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                wrthost_send_cmd(tmpevent);
                
                tmpevent[0] = ROOM_GET_WEATHER; //��ȡ����Ԥ��
                tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                wrthost_send_cmd(tmpevent);
#if 0
				if (access(MYFIFO, F_OK) == -1) 
				{
					if ((mkfifo(MYFIFO, 0777) < 0) && (errno != EEXIST))
					{
						printf("Cannot create fifo file\n");
						exit(1);
					}
				}
				
				/* ��ֻд������ʽ��FIFO�ܵ� */
				g_fifo_fd = -1;
				g_fifo_fd = open(MYFIFO, O_RDWR);
				if (g_fifo_fd == -1)
				{
					printf("Open fifo file error\n");
				}
#endif
#if USE_ENCODING
                AV_Server_Init();

                AV_Server_Start();

                AV_Server_Pause();

                AV_Server_Restart();
#endif


               // tmGpio485Send((char*)buf,1);//��һ����մ������ݡ�
#if 0
                SDL_Delay(20*1000);

                char* p = 0;
                int count = 0;
                while(1){

                        p = (char*)malloc(1024);
                        if(p == 0)
                                break;
                        memset(p,1,1024);
                        count++;
                        for(int i=0; i< 1024;i++)
                                if(p[i] !=1){
                                        printf("error i = %d count =%d \n",i,count);
                                }

                }
                printf("����ʹ�õ��ڴ�Ϊ��%d \n",count*1024); //==32M
                // zenMemUninit();
#endif

}

void CMainProcess::disp_callback(unsigned long event[4]){
        if(g_MainProcess == NULL)
                return;
#if HAVE_USED_SDL
        g_MainProcess->Send_msg(event,sizeof(unsigned long)*4);
#else

        g_MainProcess->Send_msg(event);
#endif
}

void process_alarm_clock()
{
        unsigned long msg[4];
        msg[0] = STATUS_ALARM_CLOCK;
        msg[1] = msg[2] = msg[3] = 0;
#if HAVE_USED_SDL
        g_MainProcess->Send_msg(msg,sizeof(unsigned long)*4);
#else
        g_MainProcess->Send_msg(msg);
#endif	
}

#if 1
static int add_alarm_to_vector(int type){
        int ret =0;
        ALARMVECTOR::iterator itpos;
       SDL_LockMutex(g_alarmmutex);

        itpos = find(g_alarmvector.begin(),g_alarmvector.end(),type);
        if(itpos == g_alarmvector.end()){
                ret = 1;
                g_alarmvector.push_back(type);
        }
       SDL_UnlockMutex(g_alarmmutex);
        return ret;
}

static void alarm_to_center(int type){
        char str[256];
        int ret =0;
        
        //unsigned long tmpevent[4]={0};
        //tmpevent[0] = HANDL_ALARM_VOICE;
        //g_MainProcess->Send_msg(tmpevent,sizeof(unsigned long)*4);

        ret = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,type);
        if(ret == 0)
                sprintf(str,"%d %s!",type,LT("���������ɹ�"));
        else
                sprintf(str,"%d %s!",type,LT("��������ʧ��"));
        add_log(str);
}

//������������ձ������е����ݡ�
void  check_alarm(int isstop){
        SDL_LockMutex(g_alarmmutex);
        if(g_alarmvector.empty()){

                g_have_warning = false;
                g_isstarttime = false;
                if(wrthost_sysidle())
                        mp3stop();
                StopTimer(TIME_WARNING_EVENT_STOP);
                ClearViewSafeStatus();
                Window* p = FindWindow(SAFEWIN);
                if(p){
                        DestroyWindow(p);
                        p = NULL;
                }
                g_is_leave = false;
                if(Get_safe_status())
                	g_have_defend = true;
                else
                	g_have_defend = false;
                SDL_UnlockMutex(g_alarmmutex);
                return;
        }
        ALARMVECTOR::iterator it;
/*
        for(it = g_alarmvector.begin();it!= g_alarmvector.end();){
                if(!get_security_status((*it)-1)) {//�������
                        it = g_alarmvector.erase(it);
                }else{
                        WRT_DEBUG("�����Ǹ�δ�������� %d\n",*it);
                        ++it;
                }
        }
*/
		g_alarmvector.clear();
	
        if(g_alarmvector.empty()){
                g_have_warning = false;
                g_isstarttime = false;
                
                if(wrthost_sysidle())
                        mp3stop();
                StopTimer(TIME_WARNING_EVENT_STOP);
                ClearViewSafeStatus();
                
                Window* p = FindWindow(SAFEWIN);
                if(p){
                        DestroyWindow(p);
                        p = NULL;
                }
                
                g_is_leave = false;
                if(Get_safe_status())
                	g_have_defend = true;
                else
                	g_have_defend = false;

        }

        SDL_UnlockMutex(g_alarmmutex);
}
//Ϊ������������ ��������б������е�����
void  check_alarm2(){
        SDL_LockMutex(g_alarmmutex);
        if(!g_alarmvector.empty())
                g_alarmvector.clear();
        g_have_warning = false;
        g_isstarttime = false;
        if(wrthost_sysidle())
                mp3stop();
        //StopTimer(TIME_WARNING_EVENT_STOP);
        ClearViewSafeStatus();
        SDL_UnlockMutex(g_alarmmutex);
}

static void clear_alarm(){
        SDL_LockMutex(g_alarmmutex);
        if(g_alarmvector.empty()){
                SDL_UnlockMutex(g_alarmmutex);
                return;
        }
        g_alarmvector.clear();
        SDL_UnlockMutex(g_alarmmutex);
}

static void handle_alarm(){
        SDL_LockMutex(g_alarmmutex);
        if(g_alarmvector.empty()){
                SDL_UnlockMutex(g_alarmmutex);
                return;
        }
        ALARMVECTOR::iterator it;
        for(it = g_alarmvector.begin();it!= g_alarmvector.end(); it++){
                if(get_security_status((*it)-1)) {//���û�г������������ı���
                		if(*it == 3 || *it == 4)
                		{
                			wrtip_play_voice(WARNING_RING);
                		}
                        alarm_to_center(*it);
                }
        }
        g_alarmvector.clear(); //�ϲ��ϱ����ģ�Ȼ�����
        SDL_UnlockMutex(g_alarmmutex);
}
#endif
static void realtimealarm(int type){
        SetViewSafeStatus(type-1);
        Window* pWindow = WindowGetTopVisibleWindow();
        if(pWindow->type == TS_CAL)
            return;
		tmGpioSetPin(CTL_IO_SET_ALARMOUT_ON,1);
        if(pWindow &&pWindow->type == SAFEWIN){//������ǳ�������
                ViewSafeSatus(1);
        }else{
                Window* pWindow2 = NULL;
                pWindow2 = FindWindow(SAFEWIN);
                if(pWindow2 == NULL){
					if(pWindow->type == MUTILMEDIA || pWindow->type == DIALOG || pWindow->type == PLAY_RECVIDEO)
                		CloseWindow();
                    CreateViewSafeStatusWin();
                }else{
                        if(pWindow &&pWindow->type == ROOT)
                                DisplaySafeSatus();

                }
        }
        pWindow = WindowGetTopVisibleWindow();
        if(pWindow){
        	if(pWindow->type == MUTILMEDIA)
        		CloseTopWindow();	
        	pWindow = WindowGetTopVisibleWindow();
            if(pWindow->type != CALLED || pWindow->type != CALLING){
            		mp3stop();
                    wrtip_play_voice(WARNING_RING);
            }
        }

        char str[256];
        int ret =0;
        ret = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,type);
        if(ret == 0)
                sprintf(str,"%d %s!",type,LT("���������ɹ�"));
        else
                sprintf(str,"%d %s!",type,LT("��������ʧ��"));
        add_log(str);
        g_have_warning = true;
}

Uint32 handle_alarm_callback(Uint32 interval,void* pvoid)
{
	handle_alarm();
	return 0;
}

Uint32 cancel_alarm_callback(Uint32 interval,void* pvoid)
{
	clear_alarm();
	return 0;
}
int downring_ok(int param){
        if(wrthost_sysidle())
                mp3stop();
        return 0;
}

void DrawWeatherResult()//��������ʾ
{
	if(g_weather_result[0] == 0)
		return;
	Window* pWindow = NULL;
	pWindow =  WindowGetTopVisibleWindow();
	if(pWindow->type != ROOT )
		return;
	if(m_issreenoff == true)
		return;
		
	unsigned char* weatheriocn = NULL;
	int size = 0;
	char weathertext[50];
	memset(weathertext,0,50);
	sprintf(weathertext,"%d~%d��",g_weather_result[1],g_weather_result[2]);
	switch(g_weather_result[0]){
		case 1: //��
			weatheriocn = weather_qing;
			size = sizeof(weather_qing);
			break;
		case 2: //��
			weatheriocn = weather_yin;
			size = sizeof(weather_yin);
			break;
		case 3: //����
			weatheriocn = weather_duoyun;
			size = sizeof(weather_duoyun);
			break;
		case 4://��
			weatheriocn = weather_yu;
			size = sizeof(weather_yu);
			break;
		case 5: //ѩ
			weatheriocn = weather_xue;
			size = sizeof(weather_xue);
			break;
		default:
			return;
	}
	WRT_Rect rt;
	if(pSystemInfo->mySysInfo.isuseui == 0x01){
        	rt.x = 650;
        	rt.y = 15;
        	rt.h = 38;
        	rt.w = GetTextWidth(weathertext,1)+50;
        	FillRect(BACK_COLOR,rt);
        	Bmp32Disp(650,15,40,38,weatheriocn,size);	    
        	rt.y = 20;
        	rt.h = 30;
        	rt.x += 50;
        	rt.w -= 50;
			DrawTextToRect2(rt,weathertext,rgb_white,1);        	    	
        }else{
        	rt.x = 850;
        	rt.y = 124;
        	rt.h = 38;
        	rt.w = GetTextWidth(weathertext,1)+50;
        	OSD_Bmp32Disp(rt.x,rt.y,40,38,weatheriocn,size);  
        	
        	rt.y  = 150;
        	rt.h  = 30;
        	rt.x  = 900;
        	rt.w -= 50;
			OSD_DrawText(weathertext,rt.x,rt.y,rgb_white);           	      	
        }
}

static void ReInitWarningLib()
{
	        int i,j;
                unsigned long tmpip[4];
                tmpip[0] = pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
                tmpip[1] = pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
                tmpip[2] = pSystemInfo->LocalSetting.publicinfo.CenterWarningIP;
                tmpip[3] = pSystemInfo->LocalSetting.publicinfo.CenterMinorIP;
                j =0;
                for(j=0;j<3;j++)
                        for(i=j+1;i<4;i++){
                                if(tmpip[j] == tmpip[i])
                                        tmpip[i] = 0;
                        }
                WRT_DEBUG("�޸ı�����ַ%x,%x,%x,%x\n",tmpip[0],tmpip[1],tmpip[2],tmpip[3]);
                WarningInit(2,tmpip[0],
                                tmpip[1],
                                tmpip[2],
                                tmpip[3],
                                0);
}
//�޸ı����н��� ��ʼͨ�����������
void viewcallinfo(char* text){
	WRT_Rect rt;
	rt.x = 110;
	rt.y = 86;
	rt.w = 400;
	rt.h = 50;
	if(g_isUpdated == 0){
		g_isUpdated = 1;
		FillRect(rgb_colorKey,rt);
		DrawText(text,rt.x,rt.y+24,rgb_white);
		g_isUpdated = 0;
	}else{
		FillRect(rgb_colorKey,rt);
		DrawText(text,rt.x,rt.y+24,rgb_white);		
	}
}

extern void exit_audio_play2();
extern int g_is_doorcallroom;
extern void Get_monitor_dev_info();
extern void Deal_monitor_event(int index);
extern int g_is_monitor_flag;
extern int g_current_monitor_index;
extern bool  g_is_hangup;
extern int g_is_extern_callcenter;
extern int g_is_inner_callroom;

extern void Update_holdon_Redraw();
static int m_last_lock_time = 0;
//extern void RoomSendSearchPack();

void CMainProcess::ProcessEvent(unsigned long event[4]){
        int isvideo = 0;
        int nwrite =0;
        char buff[MAX_BUFFER_SIZE];
        Window* pWindow3;
        

        WRT_DEBUG("�յ��ײ���Ϣ%d %d  %d  %d",event[0],event[1],event[2],event[3]);
printf("%s %lu :event[0]=0x%x\r\n",__FUNCTION__,__LINE__,event[0]);
        switch(event[0])
        {
        case STATUS_WEATHER_RESULT: //����Ԥ�����
        	g_weather_result[0] = event[1];
        	g_weather_result[1] = event[2];
        	g_weather_result[2] = event[3];
        	
        	if(m_issreenoff == true)
				return;
				
			pWindow3 = WindowGetTopVisibleWindow();
			if(pWindow3->type == ROOT)
				NoitfyReDraw(1);//�޸�:������-20130731 �������´���
        	//DrawWeatherResult();
        	break;
        case STATUS_DOOR_UNLOCK:  //�ſڻ������ɹ�
                break;
        case STATUS_SIP_CALL_ROOM:
                {
                        unsigned char* pSrcid  = (unsigned char*)event[1];
                        if(m_issreenoff == true){
                        	screenkeep_stop(); 
                        	screen_open();
                        	m_issreenoff = false;
                        }
                        pWindow3 = WindowGetTopVisibleWindow();
                        if(pWindow3->type == CALLING || pWindow3->type == MUTILMEDIA ){
                                CloseWindow();
                        }
                        CreateCenterCallWin();
                        if(pSrcid){
                                char cTmp[20];
                                sprintf(cTmp,"%s������...\n",(char*)pSrcid);
                                if(GetCurrentLanguage() == ENGLISH)
                                    sprintf(cTmp,"%s calling-in\n",(char*)pSrcid);
                               
                                viewcallinfo(cTmp);
                                ZENFREE(pSrcid);
                                pSrcid = NULL;
                        }else
                               viewcallinfo(LT("SIP�ͻ��˺�����..."));
                }
                break;
        case STATUS_CENTER_CALLROOM:   /*�յ����ĺ��зֻ�����Ϣ*/
#ifdef HAVE_PLAY_JPEG
                if(g_hasplay){
                        unsigned long tmpevent[4];
                        g_hasplay = 0;
                        tmpevent[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                        tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                        wrthost_send_cmd(tmpevent);

                }
#endif
                isvideo = event[1];
                if(isvideo == 1){
                        printf(" STATUS_CENTER_CALLROOM no tmVideoDisp_Start(800,480,0);\n");
                       
                }
#ifdef WRT_MORE_ROOM
                //�յ����д��ӷֻ���Ļ
                open_screen_power(0,1,1);//wyx
#endif
               if(m_issreenoff == true){
                        	screenkeep_stop(); 
                        	screen_open();
                            m_issreenoff = false;
                }
                pWindow3 = WindowGetTopVisibleWindow();
                g_is_doorcallroom =1;
                printf("���Ŀ�ʼ����  %d\n",g_is_doorcallroom);
                if(pWindow3->type == CALLING || pWindow3->type == MUTILMEDIA || pWindow3->type == DIALOG){
                        CloseWindow();
                }

                if(pWindow3->type == PLAY_RECVIDEO)
                {
                	CloseWindow();
                }
                
                CreateCenterCallWin();
                
                
                viewcallinfo(LT("���ĺ�����..."));

                break;

        case STATUS_DOOR_CALLROOM:  //�ſڻ����зֻ�
#ifdef HAVE_PLAY_JPEG
                if(g_hasplay){
                        unsigned long tmpevent[4];
                        g_hasplay = 0;
                        tmpevent[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                        tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                        wrthost_send_cmd(tmpevent);

                }
#endif
                isvideo  = event[1];

                if(isvideo == 1){
                        WRT_DEBUG(" STATUS_DOOR_CALLROOM tmVideoDisp_Start(800,480,0)");
                }
                screenkeep_stop();//�޸�ԭ��:���������иպ��ڷֻ�Ҫ�������ʱ������������ʱ��
				if(m_issreenoff == true){
                    	 
                    	screen_open();
                        m_issreenoff = false;
                }
#ifdef WRT_MORE_ROOM
                //�յ����д��ӷֻ���Ļ
                open_screen_power(0,1,1);
#endif
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == CALLING  ||  pWindow3->type == DIALOG){
                        CloseWindow();
                }
                g_is_doorcallroom = 1;
                printf("������ʼ����  %d\n",g_is_doorcallroom);
                if(pWindow3->type == MUTILMEDIA)
                {
					CloseWindow();
                }
                
                if(pWindow3->type == PLAY_RECVIDEO)
                {
                	CloseWindow();
                }
                
                CreateDoorCallWin();
                memset(doorname,0,256);
                if(event[2] == 0){
                       viewcallinfo(LT("�ſڻ�����"));
                       if(GetCurrentLanguage() == ENGLISH)
                       		memcpy(doorname,"Gate_Station",strlen("Gate_Station"));
                       else
                       		memcpy(doorname,"�ſڻ�",strlen("�ſڻ�"));
                       wrtip_play_voice(2);
                }else if(event[2] == 1){
                        viewcallinfo(LT("С�ſڻ�����"));
                        if(GetCurrentLanguage() == ENGLISH)
                        	memcpy(doorname,"Door_Station",strlen("Door_Station"));
                        else
                        	memcpy(doorname,"С�ſڻ�",strlen("С�ſڻ�"));
                        wrtip_play_voice(3);
                }else if(event[2] == 3){
                	 char tmpbuf[256];
                	 memset(tmpbuf,0,256);
                	 sprintf(tmpbuf,"%s%d",LT("����С�ſڻ�����"),event[3]);
                	 if(GetCurrentLanguage() == ENGLISH)
                	 	sprintf(doorname,"DS_Door_Station%d",event[3]);
                	 else
                	 	sprintf(doorname,"����С�ſڻ�%d",event[3]);
                	 viewcallinfo(tmpbuf);
                	 wrtip_play_voice(3);
                }else
                {
                	viewcallinfo(LT("Χǽ������"));
                	if(GetCurrentLanguage() == ENGLISH)
                		memcpy(doorname,"Entrance_Station",strlen("Entrance_Station"));
                	else
                		memcpy(doorname,"Χǽ��",strlen("Χǽ��"));
                	wrtip_play_voice(1);
                }
                break;
        case STATUS_DOOR_HANGUP:    //�ſڻ��һ�
                g_is_talk = false;
#ifdef WRT_MORE_ROOM
                if(!g_ishangon){
                        open_screen_power(0,0,0);                       
                }else
                        open_screen_power(GetTopWindowID(),0,0);
                g_ishangon = false;
#endif
                WRT_DEBUG("�ſڻ��һ�");
                CloseTopWindow();

#ifdef WRT_MORE_ROOM
                CloseAllWindows();
#endif

                break;
        case STATUS_CENTER_HANGUP: //���Ĺһ�
                g_is_talk = false;
#ifdef WRT_MORE_ROOM
                if(!g_ishangon){
                        open_screen_power(0,0,0);
                }else
                        open_screen_power(GetTopWindowID(),0,0);
                g_ishangon = false;
#endif
                CloseTopWindow();
#ifdef WRT_MORE_ROOM
                CloseAllWindows();
#endif

                break;
        case STATUS_PHONE_OR_EXTERNDEVICE_HOLDON:
        		printf("STATUS_PHONE_OR_EXTERNDEVICE_HOLDON\n");
        		SDL_Delay(2000);
        		CloseAllWindows();
        		//screen_close();
        		break;
        case STATUS_DOOR_BUSY://�ſڻ���æ


#ifdef WRT_MORE_ROOM
                set_sub_room_ap_busy(GetTopWindowID());
                open_screen_power(GetTopWindowID(),0,0);
                CloseTopWindow();
                CreateDialogWin2(LT("�Է���æ!"),MB_NONE,NULL,NULL);
                SDL_Delay(2000);
                g_mont_door = 0;
                CloseAllWindows();
#else
                CloseTopWindow();
                CreateDialogWin2(LT("�Է���æ!"),MB_OK,NULL,NULL);
#endif
                break;
        case STATUS_CENTER_BUSY://���ķ�æ
#ifdef WRT_MORE_ROOM
                set_sub_room_ap_busy(GetTopWindowID());
                open_screen_power(GetTopWindowID(),0,0);
                CloseTopWindow();
                CreateDialogWin2(LT("���ķ�æ!"),MB_NONE,NULL,NULL);
                SDL_Delay(2000);
                CloseAllWindows();

#else
                CloseTopWindow();
                CreateDialogWin2(LT("���ķ�æ!"),MB_OK,NULL,NULL);
#endif
                break;
        case STATUS_CENTER_HOLDON:  //����ժ��
                {
                        pWindow3 = WindowGetTopVisibleWindow();
                        if(pWindow3->type == CALLING )
                        	viewcallinfo(LT("��ʼͨ��..."));
                }
                break;
        case STATUS_SIP_CONNECTING:
                if(event[1] == 1){
                        WRT_Rect rect;
                        rect.x = 50;
                        rect.y = 20; 
                        rect.w = 600;
                        rect.h = 100;
                        FillRect(BACK_COLOR,rect);
                        viewcallinfo(LT("������..."));
                }
                break;
        case STATUS_SIP_START_TALK:
                if(event[1] == 1){
                        WRT_Rect rect;
                        rect.x = 50;
                        rect.y = 20;
                        rect.w = 600;
                        rect.h = 100;
                        FillRect(BACK_COLOR,rect);
                        viewcallinfo(LT("��ʼͨ��..."));
                }
                break;
        case STATUS_SIP_BUSY:
                {
                        CloseTopWindow();
                        CreateDialogWin2(LT("�Է��һ�����æµ"),MB_OK,NULL,NULL);
                }
                break;
		case STATUS_DIVERT_ROOM_TO_ROOM:
				CloseAllWindows();
				SDL_Delay(500);
				CreateCallRoomSubWin();
				break;
		case STATUS_DIVERT_ROOM_TO_CENTER:
				CloseAllWindows();
				SDL_Delay(500);
				CreateCallCenterWin();
				break;
        case STATUS_ROOM_STOPMONT: //����ֹͣ����
            CloseTopWindow();
            WRT_DEBUG("����ֹͣ����");
#ifdef WRT_MORE_ROOM
                open_screen_power(GetTopWindowID(),0,0);
#endif
               
#ifdef WRT_MORE_ROOM
                g_mont_door = 0;
                CloseAllWindows();
#endif
                break;
        case STATUS_ROOMTALKROOM://�������ֻ������� ,����ͨ
        	{
#ifdef HAVE_PLAY_JPEG
                if(g_hasplay){
                        unsigned long tmpevent[4];
                        g_hasplay = 0;
                        tmpevent[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                        tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                        wrthost_send_cmd(tmpevent);

                }
#endif
                unsigned  char roomid[16]={0};
                isvideo =event[1];
                if(isvideo == 1){
                        //tmVideoDisp_Start(800,480,0);
                }
                if(event[2] != 0){
                        memcpy(roomid,(unsigned char*)event[2],15);
                        ZENFREE((unsigned char*)event[2]);
                }
				screenkeep_stop();

				if(m_issreenoff == true){
                    	screen_open();
                            m_issreenoff = false;
                }
#ifdef WRT_MORE_ROOM
                //�յ����д��ӷֻ���Ļ
                open_screen_power(0,1,1);
#endif
                pWindow3 = WindowGetTopVisibleWindow();

                g_is_doorcallroom =1;
                printf("�ֻ���ʼ����  %d\n",g_is_doorcallroom);
                if(pWindow3->type == CALLING || pWindow3->type == MUTILMEDIA || pWindow3->type == DIALOG){
                        CloseWindow();
                }
                if(pWindow3->type == PLAY_RECVIDEO)
                {
                	CloseWindow();
                }
                if(strncmp((char*)roomid,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0)
                        CreateRoomCallWin(0);
                else                	
                        CreateRoomCallWin(1);
                DecodeRoom(roomid);
		}
                break;
        case STATUS_ROOM_BUSY: //�ֻ�æµ
#ifdef WRT_MORE_ROOM
                set_sub_room_ap_busy(GetTopWindowID());
                open_screen_power(GetTopWindowID(),0,0);
#endif
                if(event[1] == 1){
                        pWindow3 = WindowGetTopVisibleWindow();
                        if(pWindow3->type == CALLED )
                                break;
                        CloseTopWindow();
                        CreateDialogWin2(LT("������æ!"),MB_OK,NULL,NULL);
                }else{
                        CloseTopWindow();
                        CreateDialogWin2(LT("�ֻ���æ!"),MB_OK,NULL,NULL);
                }

                break;
        case STATUS_NET_EXCEPTION://��ʾ������ʾ�������쳣����δ����

                WRT_MESSAGE("STATUS_NET_EXCEPTION = %d ",event[1]);
				printf("[net connected failed]\n");
                if(event[1] == 0){
//                        g_net_is_ok = false;
                        pWindow3 = WindowGetTopVisibleWindow();
#ifdef WRT_MORE_ROOM 
                        unsigned char wid = GetTopWindowID();
#endif                        
                        if(pWindow3 && (pWindow3->type == CALLED || pWindow3->type == CALLING || pWindow3->type == MUTILMEDIA)){
                                CloseAllWindows();
                        }
                        pWindow3 = WindowGetTopVisibleWindow();
                        if(pWindow3 && pWindow3->type == ROOT){
                          	if(pSystemInfo->mySysInfo.isuseui == 0x1){
                                	WRT_Rect curbox;
                                	curbox.x = 130;
                                	curbox.y = 531;
                                	curbox.w = 300;
                                	curbox.h = 55;
#ifdef THREE_UI
                                	JpegDisp(curbox.x,curbox.y,menu_sub_back4,sizeof(menu_sub_back4));
#else        
                                	FillRect(0xdedede,curbox); 	  
#endif                                
                                	//JpegDisp(110,435,ico_info,sizeof(ico_info));
                                	DrawText_16(LT("�����������ü�����!"),150,curbox.y+40,rgb_red);

							}else{
								// ������ͼ���X. ˢ������ͼ��
								UpdateNetIcon();
								UpdateTimeIcon();
							}
                        }else{
                                CreateDialogWin2(LT("���粻ͨ!"),MB_OK,NULL,NULL);
                        }
#ifdef WRT_MORE_ROOM
                        SDL_Delay(2000);
                        open_screen_power(wid,0,0);
                        CloseAllWindows();

#endif                        
                }else if(event[1] == 3){
#ifdef WRT_MORE_ROOM 
                        unsigned char wid = GetTopWindowID();
#endif                       	
                       // CloseAllWindows();
                        CloseTopWindow();
                        pWindow3 = WindowGetTopVisibleWindow();

                        if(pWindow3 && pWindow3->type == ROOT){
                        	if(pSystemInfo->mySysInfo.isuseui == 0x01){
                                WRT_Rect curbox;
                                curbox.x = 130;
                                curbox.y = 531;
                                curbox.w = 300;
                                curbox.h = 55;
#ifdef THREE_UI
                                JpegDisp(curbox.x,curbox.y,menu_sub_back4,sizeof(menu_sub_back4));
#else
                                FillRect(0xdedede,curbox); 	  
#endif                                
                                //JpegDisp(110,435,ico_info,sizeof(ico_info));

                                DrawText_16(LT("�����������ü�����!"),150,curbox.y+40,rgb_red);

#ifdef WRT_MORE_ROOM
                                SDL_Delay(2000);
                                open_screen_power(wid,0,0);

#endif
							}else{ 
								// ������ͼ���X. ˢ������ͼ��
								UpdateNetIcon();
								UpdateTimeIcon();
							}
                        }

                }else{
//                        g_net_is_ok = true;
                        CloseAllWindows();
                       // CloseTopWindow();
                        Window* pWindow2 = WindowGetTopVisibleWindow();
                        if(pWindow2->type == ROOT){
							UpdateTimeIcon();
                        }
                }
                DrawWeatherResult();
                DrawHuanjinValue(0,0);
                break;
        case STATUS_ROOM_WARING_OK: //����ok
                break;
        case STATUS_GET_RING_ERROR: //��ȡ�����б� ERROR����������������
                break;
        case STATUS_REQDOCINFO_OVER: //��ñ�����Ϣ���
                if(event[1] == 0 ){
                        NoitfyReDraw(1); 
                }else if(event[1] == 1){
                        CloseWindow();
                        CreateDialogWin2(LT("����δ���÷�������!"),MB_OK,NULL,NULL); 
                }else if (event[1] == 3)
                {			
                        CloseWindow();
                        CreateDialogWin2(LT("������Ϣ�Ѿ�����!"),MB_OK,NULL,NULL);
                }
                break;
        case STATUS_RING_LIST_OVER: //�����б����
                WRT_DEBUG("STATUS_REQDOCINFO_OVER");
                if(event[1] == 0)
                        NoitfyReDraw(1);
                else if(event[1] == 3){
                        CloseWindow();
                        CreateDialogWin2(LT("������Ϣ�Ѿ�����!"),MB_OK,NULL,NULL);
                }else{
                        WRT_Rect curbox;
                        curbox.x = 0;
                        curbox.y = 0;
                        curbox.w = 889;
                        curbox.h = 531;
                        FillRect(0xb2136,curbox);
                        DrawText_16(LT("���޿ɹ����ص�����,�밴�����ء����ص���һ��!"),10,77,rgb_white);
                        //CloseAllWindows();
                        update_rect(curbox.x,curbox.y,curbox.w,curbox.h);
                }
                break;
        case STATUS_RING_DOWNLOAD://�����������
        	{
                RINGLIST* pring = (RINGLIST*)event[1];
                WRT_DEBUG("pring =%x isdownload =%d",pring,isdownload);
                if(pring == 0 && (int)event[2] == -2 && (int)event[3] == -2){
                        //if(isdownload == false)
                        CloseWindow();
                        CreateDialogWin2(LT("��������ʧ��!"),MB_OK,NULL,NULL);
                        break;
                }
                if(pring == 0 && (int)event[2] == -1 && (int)event[3] == -1){
                        CreateDialogWin2(LT("����������..."),MB_OK,NULL,NULL);
                        break;
                }

                if(isdownload){
                        CloseWindow();
                        CreateDialogWin2(LT("���ڱ�������..."),MB_NONE,NULL,NULL);
                        if(AddRingToDownList(pring->name,pring->len,pring->addr) == 1){
                                pring->addr = 0;
                                pring->len = 0;
                                pring->isdown = 1;
                                CloseWindow();
                                CreateDialogWin2(LT("�������!"),MB_OK,NULL,NULL);
                        }else{
                                pring->addr = 0;
                                pring->len = 0;
                                CloseWindow();
                                CreateDialogWin2(LT("��������ʧ��!"),MB_OK,NULL,NULL);
                        }
                        pring->addr = 0;
                        pring->len = 0;
                }else{
                        CloseWindow();
                        SDL_Delay(500);
                        CloseWindow();
                        SDL_Delay(500);
                        CreateDialogWin2(LT("���Ҫ���أ��밴���ذ�ť!"),MB_OK,downring_ok,NULL);
                        SDL_Delay(200); //��ʱ200MS���ſ�ʼ����
                        mp3stop();
                        mp3play((char*)pring->addr,pring->len,8);                        
                }
        	}
                break;
        case STATUS_REQUEST_REPAIR: //����ά���Ѿ�����
                CreateDialogWin2(LT("ά�������Ѿ�����!"),MB_OK,NULL,NULL);
                break;
        case STATUS_PUBLIC_TEXT_INFO: //�ı��������ݵ���
                WRT_DEBUG("STATUS_PUBLIC_TEXT_INFO %d",event[1]);
                if(event[1] == 1){
                		screenkeep_stop();
                        init_screenkeep();
                        break;
                }
                if(event[1] >1 && event[1] < 5){ //������ͼ���滻
                	if(m_issreenoff == true){
                		screen_open();
                	}
                	screenkeep_stop();
                	g_is_replace_main_ui[event[1]-2] = 1;
                	ReDrawCurrentWindow2(event[1]+7);
                	break;
                		
                }
                g_have_msg = true;
#ifdef WRT_MORE_ROOM
                open_red_led();
                open_all_channel();
#endif
                ReDrawCurrentWindow2(4);
                break;
        case STATUS_PUBLIC_PIC_INFO: //ͼƬ�������ݵ���
                g_have_msg = true;
#ifdef WRT_MORE_ROOM
                open_red_led();
                open_all_channel();
#endif
                ReDrawCurrentWindow2(4);
                break;
        case STATUS_ROOM_HANGUP: //���Է��һ�,����ʱ�䵽�һ�
                g_is_talk = false;
                //���������н�ʱ���һ�����Ҫ�ر���Ƶ��
                isvideo =event[1];
                if(isvideo == 1){
                        WRT_DEBUG("��ʾ������");
                       // tmVideoDisp_Start(800,480,0);
                }
#ifdef WRT_MORE_ROOM
                SDL_Delay(1000);
                if(!g_ishangon){
                        open_screen_power(0,0,0);
                }else
                        open_screen_power(GetTopWindowID(),0,0);
                g_ishangon = false;
#endif

                CloseTopWindow();


                break;
        case STATUS_FEE_LIST_OVER: //��ѯ���õ����ݽ������
                if(event[1] == 0)
                {
                        NoitfyReDraw(1);
                        //printf("��ѯ���õ����ݽ������\n");
                }
                else{  //û�з����б�
                        //    CloseAllWindows();
                        WRT_Rect curbox;
                        curbox.x = 0;
                        curbox.y = 0;
                        curbox.w = 1024;
                        curbox.h = 531;
                        FillRect(0xb2136,curbox);
                        DrawText_16(LT("���޿ɹ���ѯ������,�밴�����ء����ص���һ��!"),10,77,rgb_white);
                        update_rect(curbox.x,curbox.y,curbox.w,curbox.h);
                }
                break;
                // case STATUS_SET_SYS_INFO: //��������ϵͳ��Ϣ
                //     break;
        case STATUS_DOOR_LEAVEWORD: //����
        printf("%s %lu STATUS_DOOR_LEAVEWORD\r\n",__FUNCTION__,__LINE__);
        		SetVolume(0);//�����ſڻ����Թرձ��ֻ��������
                break;
        case STATUS_DOOR_LEAVEWORD_STOP: //����
        		SetVolume(pSystemInfo->mySysInfo.ringvolume);//�ſڻ������˳���ָ�������������
                break;
        
        case STATUS_START_EMERG:
        		
                g_have_warning= true;
                ReDrawCurrentWindow2(3);
                /*  Window* pWindow1 = WindowGetTopVisibleWindow();
                if(pWindow1->type == ROOT)
                NoitfyReDraw(1);  */
                break;
        case STATUS_ROOM_EMERG: //��������

                if(event[1] == 1){
                        g_have_warning = false;
                } else{
                        g_have_warning = true;
                        
                }
                break;
                // case  STATUS_ROOM_GET_IP:               /*��ȡROOM��ַ*/
                //    break;
        case STATUS_ROOM_CALLING_ROOM:            /*���ں���ROOM*/
                break;
        case STATUS_RECORD_START:
                SetVolume(0);
                break;
        case STATUS_RECORD_STOP:
                SetVolume(pSystemInfo->mySysInfo.ringvolume);
                break;
        case  STATUS_ROOM_TALK_ROOM :             /*��ʼͨ��,�ֻ�ժ��*/
#ifdef HAVE_SUB_PHONE
                if(event[1] == 1)
                {
                        CloseAllWindows();
                        break;
                }
#endif
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == CALLING ){
                      if(event[1] !=2)
                      	viewcallinfo(LT("��ʼͨ��..."));
#ifdef WRT_MORE_ROOM
                        g_ishangon = true;
#endif
                }
                if(pWindow3->type == CALLED){
#ifdef WRT_MORE_ROOM
                        g_ishangon = true;
#endif
                       if(event[1] !=2)
                       		viewcallinfo(LT("��ʼͨ��..."));
                }
#ifdef WRT_MORE_ROOM
                close_other_screen_power(GetTopWindowID());
#endif
                break;
        case STATUS_CONNECTING:          /* ����������*/
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == CALLING)
                       viewcallinfo(LT("����������..."));
                break;
        case STATUS_ROOM_VIEW_TIME: //��ʾʱ�� 
                static int netresetcnt = 0;
				if(netresetcnt++ > 60)
				{
					if(wrthost_get_sys_status() == 0)//SYS_IDLE
					{
						system("ifconfig eth0 down");
						SDL_Delay(100);
						system("ifconfig eth0 up");
						wrthost_config_networking();
						send_free_arp();
						netresetcnt = 0;
					}
				}
                //RoomSendSearchPack();//broadcast
				
                WRT_DEBUG("STATUS_ROOM_VIEW_TIME");
                if(isstartscreenkeep() == 1)
                	break;
                if(datescomp() == 1){
                        StartSceneTimer(); //�����龰�ƻ�����
                        SDL_Delay(1000);
                        StartClockTimer();
                }
                if(IsGetweather()){ //�ж��Ƿ�6��12��18�㣬Ȼ���ȡ����Ԥ����
                        unsigned long weatherevent[4];
                        weatherevent[0] = ROOM_GET_WEATHER; //��ȡ����Ԥ��
                        weatherevent[1] = weatherevent[2] = weatherevent[3] = 0;
                        wrthost_send_cmd(weatherevent);
                }

                if(m_issreenoff == true)//������ǹصģ���ôֱ�ӽ���
                        break;
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == ROOT){
                	WRT_DEBUG("STATUS_ROOM_VIEW_TIME UpdateTimeIcon");
					
					UpdateTimeIcon();
                }

                if(pWindow3->type != CALLING && pWindow3->type != CALLED){
                        if(pWindow3->type == DIALOG){
                                DialogWin* pDlgWin = (DialogWin*)pWindow3;
                                if(pDlgWin->dlgtype & MB_ONLY){
                                        break;
                                }
                        }
                        if(!has_flash_size())
                                CreateDialogWin2(LT("�洢�ռ䲻�����ͷŴ洢�ռ�?"),MB_OK|MB_CANCEL|MB_ONLY,free_flash_size,NULL);
                }

                break;
                //case STATUS_ROOM_GET_IP_ERROR:
                //     break;
        case STATUS_ROOM_SYS_UPDATE: //ϵͳ������
				if(m_issreenoff == true){
                        	screenkeep_stop(); 
                        	screen_open();
                                m_issreenoff = false;
                }
               
                if(event[1] == 1){
                        CloseAllWindows();
                        CreateDialogWin2(LT("�����ɹ�,5s���Զ�����"),MB_NONE,NULL,NULL);
                        SDL_Delay(5000);

                        S_RESET();
                      
                }else if (event[1] == 2){
                        CloseAllWindows();
                        CreateDialogWin2(LT("����Զ�������ֻ�"),MB_NONE,NULL,NULL);
                        SDL_Delay(5000);

                        S_RESET();
                        
                }else if(event[1] == 0){
                        CloseAllWindows();
                        CreateDialogWin2(LT("ϵͳ��������"),MB_NONE,NULL,NULL);
                }else if(event[1] == 10)
                {

                }
                else if(event[1] == -1){
                        CloseAllWindows();
                        CreateDialogWin2(LT("ϵͳ����ʧ��"),MB_OK,NULL,NULL);
				}else if(event[1] == -2){
			 			CloseAllWindows();
                        CreateDialogWin2(LT("�汾һ��,��������"),MB_OK,NULL,NULL);
                }
                break;
        case STATUS_AVOID_TIMES_TIMEOUT:
                if(event[1] == 0){ //���������
                        pSystemInfo->mySysInfo.isavoid = false; //ȡ�������
                        pSystemInfo->mySysInfo.avoidtimes = 43200;
                        SetAvoidtimes(0,0);
                }else if(event[1] == 1){//�������й�
                        pSystemInfo->mySysInfo.isagent = false; //ȡ���й�
                        pSystemInfo->mySysInfo.agenttimes = 43200;
                        SetAgenttimes(0,0);
                }
                save_localhost_config_to_flash();
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == ROOT){
                	WRT_MESSAGE("avoid time out NoitfyReDraw\n");
                        NoitfyReDraw(6);
                }
                break;
        case STATUS_REMOTE_ALARM:
                WRT_DEBUG("STATUS_REMOTE_ALARM  check_alarm event[1]=%d",event[1]);
                if(event[1] == 1){
                		g_have_defend = true;
                        start_defend(true);
                        ReDrawCurrentWindow();

                }else{

						WRT_DEBUG("check_alarm");
                        check_alarm(0);
                        Window* pWindow13 = WindowGetTopVisibleWindow();
                        if(pWindow13)
                        	pWindow13->NotifyReDraw2(pWindow13);
                        //ReDrawCurrentWindow();
                        tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);
                }
                //check_alarm(event[1]);
                break;
        case STATUS_START_WARNING: //�����ӳٻ򱨾��ӳٵ�ʱ�䵽�ˣ���ʼ��������

                if(event[1] == 1){
                        SDL_AddTimer(10,handle_alarm_callback,NULL);
                        g_isstarttime = false;
                        WRT_ERROR("�ϱ�����");//�������ӳ�ʱ���
                }else{
                        SDL_AddTimer(10,cancel_alarm_callback,NULL);
                        g_isstartdelay = false;
                        if(wrthost_sysidle())
                                StopAlaramPromptVoice();
                        WRT_ERROR("��������");//���ڲ����ӳ���
                }
                break;
                
        case STATUS_SAFE_WARNING:

                if(event[1] < 1)
                        break;
                if(pSystemInfo->mySysInfo.safestatus[16+(event[1]-1)] == 0){ //�������û�������򲻴���
                        break;
                }

                if(event[1] == 0x01 || event[1] == 0x02 /*|| event[1] == 0x07  || event[1] == 0x08*/  ){ //1��2��7��8��������ʱ��Ӧ��
					screenkeep_stop();
					if(m_issreenoff == true){
						screen_open();
						m_issreenoff = false;
					}
                        realtimealarm(event[1]);
                        g_have_warning = true;
                        break;
                }
                
                if(Get_safe_status()&& g_isstartdelay == false){
                		
                        if(get_security_status(event[1]-1) ){
								tmGpioSetPin(CTL_IO_SET_ALARMOUT_ON,1);
                        		screenkeep_stop();
								if(m_issreenoff == true){
									screen_open();
									m_issreenoff = false;
								}
                                g_have_warning = true;
                                if(event[1] == 0x7 || event[1] == 0x08){
                                		
                                        realtimealarm(event[1]);
                                        
                                        break;
                                }
                                unsigned long tmpevent[4];
                                if(add_alarm_to_vector(event[1]) == 0)//���뱨���б�
                                        break;
                                //֪ͨ�⿪ʼ����

                                tmpevent[0] = CUSTOM_CMD_WARNING_TIMER;
                                tmpevent[1] = 1;
                                tmpevent[2] = tmpevent[3] =0;
                                if(g_isstarttime == false){
                                        wrthost_send_cmd(tmpevent);
                                        g_isstarttime = true;//�Ѿ��ڱ����ӳٴ�����
                                        WRT_DEBUG("�б�������ʱ����");
                                        SetViewSafeStatus(event[1]-1);
                                }else{
                                        SetViewSafeStatus(event[1]-1);
                                }
                                //���沿�ֻ������ϸ��£������ر�����ʾ
                                Window* pWindow = WindowGetTopVisibleWindow();
                               	if(pWindow->type == TS_CAL)
					            	return;
                                if(pWindow &&pWindow->type == SAFEWIN){
                                        ViewSafeSatus(1);
                                }else{
                                        Window* pWindow2 = NULL;
                                        pWindow2 = FindWindow(SAFEWIN);
                                        if(pWindow2 == NULL){
											if(pWindow->type == MUTILMEDIA || pWindow->type == DIALOG)
            									CloseWindow();
            								if(pWindow->type == PLAY_RECVIDEO)
							                {
							                	CloseWindow();
							                }
                                            //ClearViewSafeStatus();
                                            SetViewSafeStatus(event[1]-1);
                                            CreateViewSafeStatusWin();
                                        }else{
                                            if(pWindow &&pWindow->type == ROOT)
                                                    DisplaySafeSatus();
                                        }
                                }
                                
                                if(event[1] != 0x03 && event[1] != 0x04){//3 4 ���������ű��ر�����
                                        pWindow = WindowGetTopVisibleWindow();
                                        if(pWindow){
                                        	if(pWindow->type == MUTILMEDIA || pWindow->type == PLAY_RECVIDEO)
                                        		CloseTopWindow();
                                        	pWindow = WindowGetTopVisibleWindow();
                                            if(pWindow->type != CALLED || pWindow->type != CALLING){
                                            	wrtip_play_voice(WARNING_RING);
                                            }
                                        }
                                }
                                ReDrawCurrentWindow2(3);
                        }
                }

                break;
        case HANDL_ALARM_VOICE:
                {
                    Window* pWindow = WindowGetTopVisibleWindow();
                    if(pWindow){
                    	if(pWindow->type == MUTILMEDIA || pWindow->type == PLAY_RECVIDEO)
                            CloseTopWindow();
                        pWindow = WindowGetTopVisibleWindow();
                        if(pWindow && (pWindow->type != CALLED || pWindow->type != CALLING)){

										mp3stop();
                            wrtip_play_voice(WARNING_RING);
                        }
                    }
                }
                break;
        case STATUS_PAUSE_VIDEO:
                if(event[2] == 0)
                        PauseAndPlayVideo(event[1]);
                else{
                        start_smalldoor_video();
                }
                break;
        case STATUS_REMOTE_SCENE: //2009-7-23
                g_cur_set_scene_mode = event[1]; //Զ�����õ�ǰ���龰�����������õķ�����

                if(scene_is_relation_zone(g_cur_set_scene_mode) == 1){
                	//�����ǰ�з���������Ϊ��������ô����
                	WRT_DEBUG("ˢ�������й�������");
                	start_defend(true);//���¿�ʼ���� 
                	ReDrawCurrentWindow();
                }else if(g_cur_set_scene_mode == SCENE_ZAIJIA)
                {
                	WRT_DEBUG("ˢ�������޹�������");
                	remote_chefang();
                	ReDrawCurrentWindow();
                }
                
                break;
        case STATUS_RMCORD_TIME:
                UpdateRecordTime(event[1]);
                if(event[1] == 0){
                        pWindow3 = NULL;
                        pWindow3 = WindowGetTopVisibleWindow();
                        if(pWindow3->type == RECORD){
                                WRT_Rect curbox;
                                curbox.x = 130;
                                curbox.y = 531;
                                curbox.w = 300;
                                curbox.h = 55;
#ifdef THREE_UI
                                JpegDisp(curbox.x ,curbox.y,menu_sub_back4,sizeof(menu_sub_back4));
#else        
                                FillRect(0xdedede,curbox); 	  
#endif                                   
                                //JpegDisp(110,435,ico_info,sizeof(ico_info));
                                DrawText_16(LT("����¼�����!"),150,curbox.y+40,rgb_red);
                        }
                }
                break;
        case STATUS_LEAVE_PLAY_OVER:  //���Բ������
                pWindow3 = NULL;
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3 && pWindow3->type == LEAVE_CHILD){
                        WRT_Rect curbox;
                        curbox.x = 130;
                        curbox.y = 531;
                        curbox.w = 300;
                        curbox.h = 55;
#ifdef THREE_UI
                        JpegDisp(curbox.x,curbox.y,menu_sub_back4,sizeof(menu_sub_back4));
#else
                        FillRect(0xdedede,curbox); 	  
#endif
                        DrawText_16(LT("���Բ������!"),150,curbox.y+40,rgb_red);
                }
                break;
#ifdef HAVE_PLAY_JPEG
        case STATUS_PLAY_JPEG:
                {
                        unsigned char* p = NULL;
                        p = (unsigned char* )event[1];
                        unsigned long size = event[2];
                        if(p && size != 0 && g_hasplay == 1){
                                JpegDispCenter(0,0,p,size,640);
                        }
                }
                break;
        case STATUS_DISPLAY_REC:
				pWindow3 = NULL;
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == ROOT)
                	break;
                if(pSystemInfo->mySysInfo.screensize == 0x10){
                        if(m_isdisplayrec){
                                if(wrthost_sysidle()){
                                        m_isdisplayrec = false;
                                        ClearRect(80,77,100,60);
                                        break;
                                }

                                /*
                                WRT_Rect rt;
                                rt.x = 80;
                                rt.y = 77;
                                rt.w = 10;
                                rt.h = 10;
                                OSD_FillRect(0xff0000,rt);
                                */
                                OSD_DrawText16("REC",85,100,0xff0000);
                        }else
                                ClearRect(80,77,100,60);
                                
                        m_isdisplayrec = !m_isdisplayrec;
                        if(event[1] == 1){
                                m_isdisplayrec = false;
                                ClearRect(80,77,100,60);
                        }
                }else{
                        if(m_isdisplayrec){
                                if(wrthost_sysidle()){
                                        m_isdisplayrec = false;
                                        ClearRect(0,0,100,60);
                                        break;
                                }

                                WRT_Rect rt;
                                rt.x = 35;
                                rt.y = 40;
                                rt.w = 10;
                                rt.h = 10;
                                OSD_FillRect(0xff0000,rt);
                                OSD_DrawText16("REC",50,50,0xff0000);
                        }else
                                ClearRect(0,0,100,60);
                        m_isdisplayrec = !m_isdisplayrec;
                        if(event[1] == 1){
                                m_isdisplayrec = false;
                                ClearRect(0,0,100,60);
                        }
                }
                break;
#endif
        case STATUS_DEBUG_MSG:
                if(event[1] == 1)
                        CreateDialogWin2(LT("�ӷֻ�����ͬ��"),MB_OK,NULL,NULL);
                break;
        case STATUS_CAP_NEW_PHONE:
#ifdef WRT_MORE_ROOM
                printf("recv STATUS_CAP_NEW_PHONE");
                open_red_led();
#endif
                break;
        case STATUS_SCREEN_CLOSE:
                screen_close(); 
                break;
        case STATUS_CHECK_JIAJU:
        		WRT_DEBUG("�Ҿ�״̬��� %d  %d",event[1],event[2]);
                if(event[3] == 0){
                        pWindow3 = NULL;
                        pWindow3 = WindowGetTopVisibleWindow();
                        if(pWindow3->type == ROOT){
                                WRT_DEBUG("������Ҿ��¶ȣ�ʪ�ȵ�");
                                FindAndViewJiajuStatus();
                        }

                } if(event[3] == 1){

                		if(event[2]==2)
		                {
		                	
		                	pWindow3 = NULL;
		                    pWindow3 = WindowGetTopVisibleWindow();
		                    if((pWindow3->type == JIAJU_CONTROL) && ( m_issreenoff == false )){
		                    	if(event[1]==1)
		                    	{
									JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
									JpegDisp(300,531,menu_sub_back4,sizeof(menu_sub_back4));
									DrawText(LT("����ͳɹ�"),180,576,rgb_white);
									update_rect(130,531,600,60);
		                    	}else if(event[1] == 0)
								{
									JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
									JpegDisp(300,531,menu_sub_back4,sizeof(menu_sub_back4));
									DrawText(LT("�����ʧ��"),180,576,rgb_white);
									update_rect(130,531,600,60);
								}else if(event[1]==3)
								{
									CreateDialogWin2(LT("���Ƴɹ�"),MB_OK,NULL,NULL);
								}else if(event[1]==4)
								{
									CreateDialogWin2(LT("����ʧ��"),MB_OK,NULL,NULL);
								}
								
		                    }
		                	break;
		                	
		                }
                        //�ػ���ʾ
                        pWindow3 = NULL;
                        pWindow3 = WindowGetTopVisibleWindow();
                        if((pWindow3->type == ROOT) && ( m_issreenoff == false )){
                                DrawHuanjinValue(event[1],event[2]);
                        }
                }
                break;
        case STATUS_ALARM_CLOCK:
        		screenkeep_stop();
				if(m_issreenoff == true){
                	screen_open();
                    m_issreenoff = false;
                }
        		pWindow3 = NULL;
                pWindow3 = WindowGetTopVisibleWindow();
                if(pWindow3->type == TS_CAL)
	            	return;
                if(pWindow3->type == CALLING || pWindow3->type == CALLED) 
                	break;
                if(pWindow3->type == MUTILMEDIA || pWindow3->type == DIALOG|| pWindow3->type == PLAY_RECVIDEO)
                	CloseTopWindow();
                wrtip_play_voice(ALARM_CLOCK_RING);
				

                SetDialogWin2CloseCallback(alarm_clock_close);
                CreateDialogWin2(LT("����������..."),MB_OK,alarm_clock_close,NULL);
                break;
        case STATUS_REMOTE_UPATE_SYSTEM_INFO:
        	ReInitWarningLib();
        	break;
        case STATUS_TS_CAL:
        	CreateTsCalibrate();
        	break;
        case STATUS_IOLEVEL_TEST://������豸���Խ�����ô�ػ�
        	pWindow3 = NULL;
            pWindow3 = WindowGetTopVisibleWindow();
            if(pWindow3->type == DEVICE_TEST) 
            	mySampleTest(1);
            
        	break;
		case STATUS_KEYLOCK_EVENT:
        {
			StartButtonVocie();
			screenkeep_stop();
        	if(m_issreenoff == true) //������״̬���Ǻ���
       	 	{
       	 		screen_open();
				send_free_arp();
				SDL_Delay(100);
			}
        	pWindow3 = NULL;
            pWindow3 = WindowGetTopVisibleWindow();
            if(pWindow3->type == TS_CAL || pWindow3->type == MUTILMEDIA)
            	return;
			if(pWindow3->type == CALLED)
			{
				if(g_is_talk)//����
				{
					unsigned long tmpevent[4];
                    tmpevent[0] = ROOM_UNLOCK;
                    tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
                    wrthost_send_cmd(tmpevent);
					return;
				}
				return;
			}
			if(pWindow3->type == CALLING && g_is_monitor_flag == 0)
			{
				return;
			}
			
			int tmptime = SDL_GetTicks();
			int difftime = tmptime - m_last_lock_time;
			if(difftime > 2000 || difftime < 0)
			{
				m_last_lock_time = tmptime;
				if(g_is_monitor_flag == 0)
				{
					CreateMonitorSubWin(0);
					Get_monitor_dev_info();
				}
				if(g_is_monitor_flag == 1)
				{
					Deal_monitor_event(g_current_monitor_index);
				}
			}
        	}
        	break;
        case STATUS_KEYCALL_EVENT:
			StartButtonVocie();
			screenkeep_stop();
        	if(m_issreenoff == true) //������״̬���Ǻ���
       	 	{
       	 		screen_open();
				send_free_arp();
				SDL_Delay(100);
			}
        	pWindow3 = NULL;
            pWindow3 = WindowGetTopVisibleWindow();
            if(pWindow3->type == TS_CAL || pWindow3->type == MUTILMEDIA)
            	return;
            if( wrthost_get_holdon_device() > PHYKEY_HOLDON)
            	return;
        	if(pWindow3->type == CALLING && g_is_monitor_flag == 1)
        	{
        		int tmptime = SDL_GetTicks();
				int difftime = tmptime - m_last_lock_time;
				if(difftime > 1000 || difftime < 0)
				{
					m_last_lock_time = tmptime;
	        		unsigned long tmpevent[4];
	                tmpevent[0] = ROOM_STOPMONT;
	                tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
	                wrthost_send_cmd(tmpevent);
                }
                return;
        	}
        	
        	if(pWindow3->type == CALLING && (g_is_extern_callcenter == 1 || g_is_inner_callroom == 1))
        	{
        		unsigned long tmpevent[4];
                tmpevent[0] = ROOM_HANGUP;
                tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
                wrthost_send_cmd(tmpevent);
                return;
        	}
        	
        	if(pWindow3->type == CALLED)
        	{
        		if(g_is_talk)
        		{
        			unsigned long tmpevent[4];
	                tmpevent[0] = ROOM_HANGUP;
	                tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
	                if(g_is_hangup == false)
                    {
                    	wrthost_send_cmd(tmpevent);
                    	g_is_hangup = true;
                    }
	                return;
        		}else
        		{
        			unsigned long tmpevent[4];
	                tmpevent[0] = ROOM_HOLDON;
	                tmpevent[1] = tmpevent[2] = tmpevent[3]=0;
	                if(g_is_talk == false)
                    {
                    	wrthost_send_cmd(tmpevent);
                    	g_is_talk = true;
                    }
                    Update_holdon_Redraw();
	                return;
        		}
        	}
        	break;
        case STATUS_KEYHOLD_EVENT:
			StartButtonVocie();
			screenkeep_stop();
			if(m_issreenoff == true) //������״̬���Ǻ���
			{
				screen_open();
				send_free_arp();
				SDL_Delay(100);
			}
			pWindow3 = NULL;
			pWindow3 = WindowGetTopVisibleWindow();
			if(pWindow3->type == TS_CAL || pWindow3->type == MUTILMEDIA)
            	return;
			if(pWindow3->type != CALLED && pWindow3->type != CALLING && wrthost_sysidle())
			{
				CreateCallCenterWin();//finish
	            SendCallEvent();//���������¼�����
            }
        	break;
        case STATUS_SET_IOLEVEL_EVENT:
        	CreateDialogWin2(LT("��������IO��ƽ�ɹ�,������..."),MB_NONE,NULL,NULL);
        	SDL_Delay(3000);
        	Reset_sys();
        	break;
        default:
                break;
        }
}


/**
*��ʼ���ֻ������Ĵ���ģ��
*/

void init_alarm_handle(){
        //����һ��������������
        g_alarmmutex = SDL_CreateMutex();
}

void uninit_alarm_handle(){
	clear_alarm();
	SDL_DestroyMutex(g_alarmmutex);
	g_alarmmutex = NULL;	
}


