#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/fb.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <pthread.h>

#include "Handler.h"
#include "c_interface.h"
#include "ctrlcmd.h"
#include "audiovideo.h"
#include "c_interface.h"
#include "main_menu_res.h"
#include "call_menu_res.h"
#include "public_menu_res.h"
#include "A_Z.h"
#include "number_res.h"
#include "dianhua_res.h"
#include "lift_ico_res.h"
#include "window.h"
#include "image.h"
#include "jpeg.h"

#include "tmWRTICaller.h"

#include <dirent.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/stat.h> 

#include "videocommon.h"
#include "RoomGpioUtil.h"
#include "media_negotiate.h"

#include "trvideo.h"
#ifdef USE_DESKTOP
#include "shortcut_res.h"
#endif

#ifdef USED_ZHONGSHAN
#include "zhongshan_res.h"
#endif

#if defined (__cplusplus)
extern "C"{
#endif

        int zenMemUninit();
        int WRT_HAVE_DBG();

#if defined (__cplusplus)
}
#endif


#ifdef USE_TESHU_1
extern unsigned char menu_teshuxitong2[17604];
extern unsigned char menu_teshuxitong1[22885];
#endif

extern unsigned char meituyule_1[30210];
extern unsigned char meituyule_2[29621];
extern int g_have_cap_photo;
#define USE_VOLUME_IN_VIDEO 0

/*
////////////////////////////////////////////////////
//   [in] id  输入参数
//   [out] ip  查找到数据存入ip
//   [in,out]  num 传入分配ip的个数，输出num得到的ip个数
extern void get_ipbyid(char* id,unsigned long* ip,int* num);
extern void get_id(char* p,int len);
*/
#include "idtoiptable.h"
extern int GetViewSafeStatus(int index); //获得防区是否报警

extern unsigned char g_iscapphoto;
extern unsigned int g_send_my_video;


extern int net_test_ping(unsigned long ipaddr);

#define  MAX_ROOM_BITS 17                 //房间号的位数

//extern RoomSystemInfo      roomtest;    //本机所有信息

extern bool     g_isstartdelay;
extern bool m_issreenoff;
extern int g_is_doorcallroom;

extern int                g_isUpdated; //是否即时刷新

unsigned long     event[4];         //命令包数组
bool     g_rc_status[42];           //判断按下的状态，弹起是否和按下在同一区域
bool     g_net_is_ok= false;       //网络是否通
bool     g_have_warning = false;  //是否设置报警
bool     g_have_msg = false;   //是否有msm
bool     g_have_defend = false;   //是否设置设防
bool     g_is_have_jiaju = false;//true; //默认是有家居模块的。
//bool     g_have_faze = false;   //是否设置打扰
bool     g_is_leave = false;    //是否设置在家 //2009-7-23
bool    g_is_unlock = false;

unsigned long g_cap_time = 0;

static uint32_t g_agentticks  = 0;
extern int Get_safe_status();

static int g_monttype = 0;
//字母表
char g_alphabet[26] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q',
'R','S','T','U','V','W','X','Y','Z'};




//数字表
char g_number[12] = {'1','2','3','4','5','6','7','8','9','*','0','#'};

//两种字母表的指针及其大小
static unsigned long  g_alphabetpointer[28];
static unsigned long  g_alphabetsize[28];

static unsigned long  g_alphabetpointer1[28];
static unsigned long  g_alphabetsize1[28];

//两种数字表指针存放地址
static unsigned long  g_numberpointer[12];
static unsigned long  g_numbersize[12];
static unsigned long  g_numberpointer1[12];
static unsigned long  g_numbersize1[12];

//保存用户输入的房间号
static char  g_roomnumber[MAX_ROOM_BITS+1];
static int   g_inputcount = 0; //用户输入计数，如果大于MAX_ROOM_BITS-1,则不继续输入

static const WRT_Rect g_rc = {0,0,1024,600};

//
static unsigned char g_isInitAlphabet = 0x0;


//主界面小图标的位置
static const WRT_xy  rc_ico[6]={
        {668,541},
        {727,541},
        {786,541},
        {845,541},
        {904,541},
		{963,541}
};

WRT_Rect rc_ui2_ico[7]={
	{30,429 ,42,42},
	{80,429 ,42,42},
	{130,429,42,42},
	{180,429,42,42},
	{230,429,42,42},
	{280,429,42,42},
	{330,429,42,42}
};

//主界面位置
static const WRT_Rect rc_mainmenuico[10] = {
        {108,83, 136, 136},	         //呼叫
        {326,83, 136, 136},          //监视
        {555,83, 136, 136},           //图文
        {779,83, 136, 136},        	//在家
        {107,305,136, 136},           //服务
        {326,305,136, 136},          // 家居
        {555,305,136, 136},           // 设置
        {779,305,136, 136},        //帮助
        {6,531,111,69} ,              //关屏
        {0,0,0,0}              //返回
};

//户户界面位置
static const WRT_Rect rc_callroomico[5]={
        {180,50,720,50},                  //输入框的位置
        {15,119,700,400},                //所有字母所在的范围
        {721,119,300,400},             //所有数字所在的范围
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

static int g_is_call_room = 0;
int g_is_monitor_flag = 0;
int g_current_monitor_index = 0;
int g_is_extern_callcenter = 0;
int g_is_inner_callroom = 0;

/*******************************************************************************
*******************************************************************************/
static void CloseCallWin(){
        g_is_call_room = 0;
        g_is_monitor_flag = 0;
	    g_current_monitor_index = 0;
	    g_is_extern_callcenter = 0;
	    g_is_doorcallroom = 0;
	    g_is_inner_callroom = 0;
}


/************************************************************************/
/*话通话时的音量调整界面.....                                           */
/************************************************************************/
static  WRT_Rect rc_volumepos[5]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

static int g_curtalkvolume = 3;

static void updateSetVolume(){
        int i =0;
        WRT_Rect rcbox;
        int volume = 0;
        switch(g_curtalkvolume)
        {
		    case 0:
		            volume = 40;
		            break;
		    case 1:
		            volume = 32;
		            break;
		    case 2:
		            volume = 24;
		            break;
		    case 3:
		            volume = 16;
		            break;
		    case 4:
		            volume = 8;
		            break;
        }
        SetVolume(volume);
        int j =0;
        for(j = 0; j<5; j++){
                OSD_FillRect(rgb_white,rc_volumepos[j]);
                if(j <= g_curtalkvolume){
                        OSD_FillRect(rgb_yellow,rc_volumepos[j]);
                }
        }
}

static void DrawVolumeSize()
{
	rc_volumepos[0].x = 430;
	rc_volumepos[0].y = 450;
	rc_volumepos[0].w = 20;
	rc_volumepos[0].h = 10;

	rc_volumepos[1].x = 470;
	rc_volumepos[1].y = 430;
	rc_volumepos[1].w = 20;
	rc_volumepos[1].h = 30;

	rc_volumepos[2].x = 510;
	rc_volumepos[2].y = 410;
	rc_volumepos[2].w = 20;
	rc_volumepos[2].h = 50;

	rc_volumepos[3].x = 550;
	rc_volumepos[3].y = 390;
	rc_volumepos[3].w = 20;
	rc_volumepos[3].h = 70;

	rc_volumepos[4].x = 590;
	rc_volumepos[4].y = 370;
	rc_volumepos[4].w = 20;
	rc_volumepos[4].h = 90;

	updateSetVolume();
}

static int HandlerVolume(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<5;index++){
                if(IsInside(rc_volumepos[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_volumepos[index].x;
                        pos_y = rc_volumepos[index].y;
                        if(status != 1){
                                g_curtalkvolume = index;
                                updateSetVolume();
                        }
                        break;

                }
        }
        return ret;
}

/****************************************************************
创建主窗口
****************************************************************/

int ismanager(char* input)
{
        char *tempbuf = "00000000";
        if((memcmp(input+2,tempbuf,8) == 0) && (memcpy(input+13,tempbuf,2) == 0))
                return 1;
        return 0;
}

int isdoor(char* input)
{
	char *tempbuf = "000000";
	if(memcmp(input+7,tempbuf,6) == 0)
		return 1;
	return 0;
}

int iswall(char* input){
	char* tmpbuf="00000000";        
	char* tmpbuf2 = (char*)(input+13);        
	if((memcmp(input+2,tmpbuf,8) == 0) && (strcmp(tmpbuf2,"00") != 0) )                
		return 1;        
	return 0;
}

int isDoorStation(char* input,unsigned long* ip,int num){ //屏蔽掉数字门口机
	int i = 0;
	char tmpid[3];
	memset((void*)tmpid,0,3);
	for(i =0; i< num;i++){
		tmpid[0] = *((char*)input+i*16+13);
		tmpid[1] = *((char*)input+i*16+14);
		if(atoi(tmpid) > 31){
			memset((void*)(input+i*16),0,16);
			ip[i] = 0;
		}
	}
	return 0;
}

void parse_inputcallrule(char* input){
	char text[MAX_ROOM_BITS];
	char text1[10];
	char text2[10];
	int  itmp = 0;
	int  i =0;
	int offset =0;
	for(i =0; i< 5; i++)
		if(pSystemInfo->mySysInfo.idrule[i] != 0)
                        break;
	if(i == 5) //如果全部位0，则不用解析
		return;
	memset(text,0,MAX_ROOM_BITS);
	strcpy(text,input);
	memset(input,0,MAX_ROOM_BITS);

	for(i =0; i < 5; i++){
		memset(text1,0,10);
		memset(text2,0,10);
		itmp = 0;

		memcpy(text2,text+offset,pSystemInfo->mySysInfo.idrule[i]);
		//printf("%d  =%d\n",i,pSystemInfo->mySysInfo.idrule[i]);
		//itmp = atoi(text2);
		switch(i){
			case 0:
			sprintf(text1,"%02s",text2);
			break;
			case 1:
			sprintf(text1,"%03s",text2);
			break;
			case 2:
			sprintf(text1,"%02s",text2);
			break;
			case 3:
			sprintf(text1,"%03s",text2);
			break;
			case 4:
			sprintf(text1,"%03s",text2);
			break;
		}

		strcat(input,text1);
		offset += pSystemInfo->mySysInfo.idrule[i];
	}
	for(i=0;i<13;i++)
	{
		if(input[i]==32)
		{
			input[i]=48;//将空值赋值'0'
		}
	}
}

void UpdateAlarmIcon(int update)
{
	if(pSystemInfo->mySysInfo.isuseui == 0x01){

        	if(Get_safe_status())
        	        JpegDisp(rc_ico[3].x,rc_ico[3].y,ico_shefang1,sizeof(ico_shefang1));
        	else
        	        JpegDisp(rc_ico[3].x,rc_ico[3].y,ico_shefang2,sizeof(ico_shefang2));
        	
        	if(g_have_warning)
        	        JpegDisp(rc_ico[4].x,rc_ico[4].y,ico_baojing1,sizeof(ico_baojing1));
        	else
        	        JpegDisp(rc_ico[4].x,rc_ico[4].y,ico_baojing2,sizeof(ico_baojing2));
        	
        	if(update == 1)
        		g_isUpdated = 0;
        	DisplaySafeSatus();
        	if(update == 1){
        		update_rect(530,531,150,55);
        		g_isUpdated = 1;
        	}
	}else{
			if(Get_safe_status())
        	        JpegDisp(rc_ui2_ico[1].x,rc_ui2_ico[1].y,safeicon_open_820,sizeof(safeicon_open_820));
        	else
        	        JpegDisp(rc_ui2_ico[1].x,rc_ui2_ico[1].y,safeicon_close_820,sizeof(safeicon_close_820));
        	
        	if(g_have_warning)
        	        JpegDisp(rc_ui2_ico[2].x,rc_ui2_ico[2].y,warningicon_open_820,sizeof(warningicon_open_820));
        	else
        	        JpegDisp(rc_ui2_ico[2].x,rc_ui2_ico[2].y,warningicon_close_820,sizeof(warningicon_close_820));	
        	if(update == 1)
        		g_isUpdated = 0;
        	DisplaySafeSatus();
        	if(update == 1){
        		update_rect(994,200,30,200);
        		g_isUpdated = 1;
        	}
	}
}

void UpdateMsgIcon()
{
	if(pSystemInfo->mySysInfo.isuseui == 0x01){
        	if(g_have_msg){
        	        JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_xinxi1,sizeof(ico_xinxi1));
        	}else{
        	        if(isnoreadvoice()){
        	                JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_voice,sizeof(ico_voice));
        	        }else
        	                JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_xinxi2,sizeof(ico_xinxi2));
        	}
	}else{
		
        	if(g_have_msg){
        	        JpegDisp(rc_ui2_ico[4].x,rc_ui2_ico[4].y,infoicon_open_820,sizeof(infoicon_open_820));
        	}else{

        	         JpegDisp(rc_ui2_ico[4].x,rc_ui2_ico[4].y,infoicon_close_820,sizeof(infoicon_close_820));
        	}
        	
        	if(isnoreadvoice()){ //是否有留言
        		JpegDisp(rc_ui2_ico[5].x,rc_ui2_ico[5].y,leave_open_820 ,sizeof(leave_open_820));
        	}else{
        		JpegDisp(rc_ui2_ico[5].x,rc_ui2_ico[5].y,leave_close_820 ,sizeof(leave_close_820));
        	}      			
	}
}

/*
函数功能:检测网络是否和交换机等设备联通。
*/
int GetNetStat()
{
	char    buffer[BUFSIZ];
	FILE    *read_fp;
	int     chars_read;
	int     ret;

	memset( buffer, 0, BUFSIZ );
	read_fp = popen("ifconfig eth0 | grep RUNNING", "r");
	if ( read_fp != NULL ) 
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
		if (chars_read > 0) 
		{
			ret = 1;
		}
		else
		{
			ret = -1;
		}
		pclose(read_fp);
	}
	else
	{
		ret = -1;
	}

	return ret;
}
extern int g_is_wifi_status;
extern unsigned char wifi1[1344];
extern unsigned char wifi2[1611];
extern unsigned char uwifi1[17332];
extern unsigned char uwifi2[21732];

void UpdateNetIcon()
{
	if(pSystemInfo->mySysInfo.isuseui == 0x01){
		if( pSystemInfo->mySysInfo.wifi_switch )
		{
	    	if(g_is_wifi_status)
	        	JpegDisp(rc_ico[5].x,rc_ico[5].y,uwifi1,sizeof(uwifi1));
	    	else
	        	JpegDisp(rc_ico[5].x,rc_ico[5].y,uwifi2,sizeof(uwifi2));
		}
    	if(g_net_is_ok)
        	JpegDisp(rc_ico[0].x,rc_ico[0].y,ico_wangluo1,sizeof(ico_wangluo1));
    	else
        	JpegDisp(rc_ico[0].x,rc_ico[0].y,ico_wangluo2,sizeof(ico_wangluo2));
        
    }else{
    	if( pSystemInfo->mySysInfo.wifi_switch )
		{
	    	if(g_is_wifi_status)
	    		JpegDisp(rc_ui2_ico[6].x,rc_ui2_ico[6].y,wifi2,sizeof(wifi2));
	    	else
	    		JpegDisp(rc_ui2_ico[6].x,rc_ui2_ico[6].y,wifi1,sizeof(wifi1));
		}
		
		if(g_net_is_ok)
    		JpegDisp(rc_ui2_ico[0].x,rc_ui2_ico[0].y,neticon_open_820,sizeof(neticon_open_820));
    	else
    		JpegDisp(rc_ui2_ico[0].x,rc_ui2_ico[0].y,neticon_close_820,sizeof(neticon_close_820));
    	
    }
}

void UpdateAVIcon()
{
	if(pSystemInfo->mySysInfo.isuseui == 0x01){
        	if(pSystemInfo->mySysInfo.isavoid == 1)
        	        JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_mianrao1,sizeof(ico_mianrao1));
        	else
        	{
        	        if(pSystemInfo->mySysInfo.isagent == 1)
        	                JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_tuoguan1,sizeof(ico_tuoguan1));
        	        else
        	                JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_mianrao2,sizeof(ico_mianrao2));
        	}
	}else{
        	if(pSystemInfo->mySysInfo.isavoid == 1)
        	        JpegDisp(rc_ui2_ico[3].x,rc_ui2_ico[3].y,nodisurbicon_open_820,sizeof(nodisurbicon_open_820));
        	else
        	{
        	        if(pSystemInfo->mySysInfo.isagent == 1)
        	                JpegDisp(rc_ui2_ico[3].x,rc_ui2_ico[3].y,agent_820,sizeof(agent_820));
        	        else
        	                JpegDisp(rc_ui2_ico[3].x,rc_ui2_ico[3].y,nodisurbicon_close_820,sizeof(nodisurbicon_close_820));
        	}
	}
}

void UpdateTimeIcon()
{
	Window* pWindow = NULL;

	pWindow =  WindowGetTopVisibleWindow();
#if 1
	if(pSystemInfo->mySysInfo.isuseui == 0x02){
		SDL_Rect rt;
		SDL_Rect srt;
		
		rt.x =0;
		srt.x=0;
		rt.y=125;
		srt.y=125;
		rt.w=300;
		rt.h=60;
		srt.w=300;
		srt.h=60;
		
		SDL_BlitSurface(tmp_g_pScreenSurface,&srt,g_pScreenSurface,&rt);
	//	if(g_pScreenSurface)
	//		SDL_UpdateRect(g_pScreenSurface,rt.x,rt.y,rt.w,rt.h);
    }
#endif
	if(pWindow->type != ROOT )
	{
		return;
	}else
	{
		if(m_issreenoff == true)
		{
			return;
		}
		else
		{
	        uint32_t day,Month,years, dates,times;
	        uint32_t min,second,Hour;
	        char timename[50];
	        tm_get(&dates,&times,NULL);
	        day= dates&0xff;
	        Month = (dates >> 8) &0xff;
	        years = (dates>>16) & 0xffff;
	        Hour = (times >> 16)& 0XFFFF;
	        second = (times & 0xff);
	        min = (times >> 8) &0xff;
	        sprintf(timename,"%04d/%02d/%02d %02d:%02d",years,Month,day,Hour,min);
	        WRT_Rect curbox;
	        if(pSystemInfo->mySysInfo.isuseui == 0x01)
	        {
				JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
	        	DrawText_16(timename,150,570,rgb_black);
			}else{//2
                OSD_DrawText(timename,50,155,rgb_white);
			}
			UpdateNetIcon();
		}
	}
}


#ifdef USE_DESKTOP
void CreateDesktop()
{
        AddMainShortcut();
        InitShortcut();

        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = DesktopEventHandle;
        pWindow->NotifyReDraw = DrawDesktop;
        pWindow->valid = 1;
        pWindow->type = ROOT;
        WindowAddToWindowList(pWindow);
#if HAVEKEYBOARD
        InitAlphabetAndNumber();  //为call room 服务
#endif 

        pWindow->NotifyReDraw2(pWindow);
        for(int i =0 ; i< 42;i++)
                g_rc_status[i] = false;
}

void DrawDesktop()
{
        int xoffset = 21;
        SHORTCUTDATA* _shortcut = NULL;
        g_is_have_jiaju = pSystemInfo->mySysInfo.isjiaju;

        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        //JpegDisp(0,0,menu_man_back,sizeof(menu_man_back));
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 


        for(int i = 1; i<9;i++){
                _shortcut = GetCurrentShortcut(i);
                if(_shortcut){
                        JpegDisp(rc_mainmenuico[i-1].x,rc_mainmenuico[i-1].y,_shortcut->icon_addr1,_shortcut->icon_len1);
                        DrawEnText(LT2(_shortcut->name),rc_mainmenuico[i-1].w +xoffset,rc_mainmenuico[i-1].h ,rc_mainmenuico[i-1].x,rc_mainmenuico[i-1].y);
                }
        }
        JpegDisp(rc_mainmenuico[8].x,rc_mainmenuico[8].y,bt_guanping1,sizeof(bt_guanping1));

        if(!g_is_leave){
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_zaijia1,sizeof(menu_zaijia1));
                
        }else{
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_lijia1,sizeof(menu_lijia1));

        }
        if(g_net_is_ok)
                JpegDisp(rc_ico[0].x,rc_ico[0].y,ico_wangluo1,sizeof(ico_wangluo1));
        else
                JpegDisp(rc_ico[0].x,rc_ico[0].y,ico_wangluo2,sizeof(ico_wangluo2));



        if(pSystemInfo->mySysInfo.isavoid == 1)
                JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_mianrao1,sizeof(ico_mianrao1));
        else
        {
                if(pSystemInfo->mySysInfo.isagent == 1)
                        JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_tuoguan1,sizeof(ico_tuoguan1));
                else
                        JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_mianrao2,sizeof(ico_mianrao2));
        }
        
        if(g_have_msg){
                JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_xinxi1,sizeof(ico_xinxi1));
        }else{
                if(isnoreadvoice()){
                        JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_voice,sizeof(ico_voice));
                }else
                        JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_xinxi2,sizeof(ico_xinxi2));
        }

        if(g_have_defend)
                JpegDisp(rc_ico[3].x,rc_ico[3].y,ico_shefang1,sizeof(ico_shefang1));
        else
                JpegDisp(rc_ico[3].x,rc_ico[3].y,ico_shefang2,sizeof(ico_shefang2));

        if(g_have_warning)
                JpegDisp(rc_ico[4].x,rc_ico[4].y,ico_baojing1,sizeof(ico_baojing1));
        else
                JpegDisp(rc_ico[4].x,rc_ico[4].y,ico_baojing2,sizeof(ico_baojing2));

        unsigned long day,Month,years, dates,times;
        unsigned long min,second,Hour;
        char timename[50];
        tm_get(&dates,&times,NULL);
        day= dates&0xff;
        Month = (dates >> 8) &0xff;
        years = (dates>>16) & 0xffff;
        Hour = (times >> 16)& 0XFFFF;
        second = (times & 0xff);
        min = (times >> 8) &0xff;
        sprintf(timename,"%04d/%02d/%02d %02d:%02d",years,Month,day,Hour,min);
        WRT_Rect curbox;
        curbox.x = 150;
        curbox.y = 424;
        curbox.w = 260;
        curbox.h = 55;

//#ifdef THREE_UI
	JpegDisp(130,curbox.y,menu_sub_back4,sizeof(menu_sub_back4));
//#else        
//        FillRect(0xdedede,curbox); 	  
//#endif   

        DrawText_16(timename,150,460,rgb_black);

        DisplaySafeSatus();
}

int DesktopEventHandle(int x,int y,int status)
{
        SHORTCUTDATA* _shortcut = NULL;
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<9;index++){
                if(IsInside(rc_mainmenuico[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_mainmenuico[index].x;
                        pos_y = rc_mainmenuico[index].y;
                        if(index == 3){
#if HAVE_JIAJU_SAFE
                                if(status == 1){
                                        StartButtonVocie();
                                        ViewSecneJpg2();
                                }else{
                                        ViewSecneJpg1();

                                        if(g_cur_set_scene_mode == SCENE_ZAIJIA){
                                                CreateDialogWin2(LT("离家状态下,防区将设防,电源将关闭.确定要选择离家模式吗?"),MB_OK|MB_CANCEL,cur_set_leave_home,NULL);
                                        }else if(g_cur_set_scene_mode  == SCENE_LIJIA){
                                                if(GetIsStartDefendStatus() == 1){
                                                        CreatePasswordWin(SET_CHEFANG_STATUS);
                                                }else{
                                                        cur_set_leave_home(false);
                                                }

                                        }else{
                                                g_cur_set_scene_mode = SCENE_ZAIJIA;
                                                DrawDesktop();
                                        }

                                }
#else
                                if(!g_is_leave){
                                        if(status == 1){ //按下
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menu_zaijia2,sizeof(menu_zaijia2));
                                        }else{
                                                JpegDisp(pos_x,pos_y,menu_zaijia1,sizeof(menu_zaijia1));
                                                //JpegDisp(pos_x,pos_y,menu_lijia1,sizeof(menu_lijia1));
                                                //模式索引为 6;将其发送给底层
                                                CreateDialogWin2(LT("离家状态下,防区将设防,电源将关闭.确定要选择离家模式吗?"),MB_OK|MB_CANCEL,cur_set_leave_home,NULL);
                                        }
                                }else{
                                        if(status == 1){ //按下
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menu_lijia2,sizeof(menu_lijia2));
                                        }else{
                                                JpegDisp(pos_x,pos_y,menu_lijia1,sizeof(menu_lijia1));
                                                // JpegDisp(pos_x,pos_y,menu_zaijia1,sizeof(menu_zaijia1));
                                                // g_is_leave = false;
                                                //创建密码输入Dialog
                                                CreatePasswordWin(SET_CHEFANG_STATUS);
                                        }
                                }
#endif
                        }else if(index == 8){
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                        screen_close();
                                }
                        }else{
                               _shortcut =  GetCurrentShortcut(index+1);
                               if(_shortcut == NULL)
                                       return ret;
                               if(status == 1){ //按下
                                       StartButtonVocie();
                                       JpegDisp(pos_x,pos_y,_shortcut->icon_addr2,_shortcut->icon_len2);
                               }else{
                                       JpegDisp(pos_x,pos_y,_shortcut->icon_addr1,_shortcut->icon_len1);
                                       if(_shortcut->func){
                                                _shortcut->func();
                                                if(_shortcut->efunc)
                                                        _shortcut->efunc();
                                       }else{
                                              // if(_shortcut->func2)
                                              //         _shortcut->func2(1);
                                       }

                               }
                        }
      
                        break;
                }
        }
        return ret;
} 

void AddMainShortcut()
{

        AddDesktopShortcut("Call",menu_hujiao1,sizeof(menu_hujiao1),menu_hujiao2,sizeof(menu_hujiao2),CreateCallWin,NULL,NULL);
        AddDesktopShortcut("Monitor",menu_jianshi1,sizeof(menu_jianshi1),menu_jianshi2,sizeof(menu_jianshi2),CreatetMonitorWin,NULL,NULL);
        AddDesktopShortcut("Messages",menu_tuwen1,sizeof(menu_tuwen1),menu_tuwen2,sizeof(menu_tuwen2),CreateInfoWin,NULL,NULL);
        AddDesktopShortcut("Home",NULL,0,NULL,0,NULL,NULL,NULL);
        AddDesktopShortcut("Service",menu_fuwu1,sizeof(menu_fuwu1),menu_fuwu2,sizeof(menu_fuwu2),CreateServiceWin,NULL,NULL);
        AddDesktopShortcut("Smart Home",menu_jiaju1,sizeof(menu_jiaju1),menu_jiaju2,sizeof(menu_jiaju2),CreateJiajuWin,NULL,NULL);
        AddDesktopShortcut("Settings",menu_shezhi1,sizeof(menu_shezhi1),menu_shezhi2,sizeof(menu_shezhi2),CreateMainWin,NULL,NULL);
        AddDesktopShortcut("Help",menu_bangzhu1,sizeof(menu_bangzhu1),menu_bangzhu2,sizeof(menu_bangzhu2),CreateHelpWin,NULL,NULL);


        AddDesktopShortcut("Guard Center",ico_hujiaozhongxin1,sizeof(ico_hujiaozhongxin1),ico_hujiaozhongxin2,sizeof(ico_hujiaozhongxin2),CreateCallCenterWin,NULL,SendCallEvent);
        AddDesktopShortcut("Tenant",ico_hujiaozhuhu1,sizeof(ico_hujiaozhuhu1),ico_hujiaozhuhu2,sizeof(ico_hujiaozhuhu2),CreateCallRoomWin,NULL,NULL);
        AddDesktopShortcut("Duty Manager",ico_datangjingli1,sizeof(ico_datangjingli1),ico_datangjingli2,sizeof(ico_datangjingli2),CreateCallCenterWin,NULL,SendCallEvent);
        AddDesktopShortcut("Lift",ico_calllift1,sizeof(ico_calllift1),ico_calllift2,sizeof(ico_calllift2),CallLift,NULL,NULL);
        AddDesktopShortcut("Lift 1",ico_baklift1,sizeof(ico_baklift1),ico_baklift2,sizeof(ico_baklift2),CallLift1,NULL,NULL);

}
#endif

void CreateMainWin(){

        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        FindAndViewJiajuStatus(); //2011-5-19 17:11:03
        pWindow->EventHandler = MainEventHandler;
        pWindow->NotifyReDraw = DrawMain;
        pWindow->valid = 1;
#ifndef USE_DESKTOP //  [2011年3月4日 9:31:39 by ljw]
        pWindow->type = ROOT;
        WindowAddToWindowList(pWindow);
#if HAVEKEYBOARD
        InitAlphabetAndNumber();  //为call room 服务
#endif
    
        pWindow->NotifyReDraw2(pWindow);
        for(int i =0 ; i< 42;i++)
                g_rc_status[i] = false;
#else
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
#endif
}


void DisplaySafeSatus(){//显示桌面防区
        WRT_Rect rt,rt1;
        int offset = 40;
        rt.x = 532;
        rt.y = 543;
        rt.w = 20;
        rt.h = 20;
        int color = rgb_black;
        rt1 = rt;
        rt1.y +=25;
		
        for(int i=0; i< 8;i++){
                if( pSystemInfo->mySysInfo.safestatus[16+i] == 0 ){ //如果防区没有启动则不处理
                        color = rgb_black;
                }else{
                        if(get_security_status(i) && Get_safe_status()){//返回是否布防
                                if(GetViewSafeStatus(i))//返回是否报警
                                        color = rgb_red;
                                else
                                        color = rgb_yellow;
                        }else{
                                if(i > 1)
                                	color = rgb_green;
                                else 
                                	color = rgb_yellow;
                        }
                }
                if(i< 4){
					if(pSystemInfo->mySysInfo.isuseui == 0x01){                	
                        	rt.x += 25;
                        	tDraw3dOutset(rt.x,rt.y,rt.w,rt.h);
                        	WRT_Rect tmp = rt;
                        	DeflateRect(&tmp,1);
                        	FillRect(color,tmp);
                	}else{
                		if(color == rgb_black){
                			JpegDisp(960,200+i*25,defence_black_820,sizeof(defence_black_820));
                		}else if(color == rgb_red){
                			JpegDisp(960,200+i*25,defence_red_820,sizeof(defence_red_820));
                		}else if(color == rgb_green){
                			JpegDisp(960,200+i*25,defence_green_820,sizeof(defence_green_820));
                		}else if(color == rgb_yellow){
                			JpegDisp(960,200+i*25,defence_yellow_820,sizeof(defence_yellow_820));
                		}
                	}
                }else{
                	if(pSystemInfo->mySysInfo.isuseui == 0x01){//时尚界面 
                        	rt1.x += 25;
                        	tDraw3dOutset(rt1.x,rt1.y,rt1.w,rt1.h);
	                        WRT_Rect tmp = rt1;
	                        DeflateRect(&tmp,1);
                        	FillRect(color,tmp);
                	}else{
                		if(color == rgb_black){
                			JpegDisp(960,200+i*25,defence_black_820,sizeof(defence_black_820));
                		}else if(color == rgb_red){
                			JpegDisp(960,200+i*25,defence_red_820,sizeof(defence_red_820));
                		}else if(color == rgb_green){
                			JpegDisp(960,200+i*25,defence_green_820,sizeof(defence_green_820));
                		}else if(color == rgb_yellow){
                			JpegDisp(960,200+i*25,defence_yellow_820,sizeof(defence_yellow_820));
                		}
                	}
                }
        }
}

static void ViewSecneJpg1(){
        switch(g_cur_set_scene_mode){
#if HAVE_JIAJU_SAFE
        case SCENE_HUIKE:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_huike1,sizeof(menu_huike1));
                break;
        case SCENE_JIUCAN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_jiucan1,sizeof(menu_jiucan1));
                break;
        case SCENE_YINGYUAN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_yingyuan1,sizeof(menu_yingyuan1));
                break;
        case SCENE_JIUQIN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_jiuqin1,sizeof(menu_jiuqin1));
                break;
        case SCENE_WENXIN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_wenxin1,sizeof(menu_wenxin1));
                break;
#endif
        case SCENE_ZAIJIA:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_zaijia1,sizeof(menu_zaijia1));
                break;
        case SCENE_LIJIA:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_lijia1,sizeof(menu_lijia1));
                break;
        }
}

static void ViewSecneJpg2(){
        switch(g_cur_set_scene_mode){
#if HAVE_JIAJU_SAFE
        case SCENE_HUIKE:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_huike2,sizeof(menu_huike2));
                break;
        case SCENE_JIUCAN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_jiucan2,sizeof(menu_jiucan2));
                break;
        case SCENE_YINGYUAN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_yingyuan2,sizeof(menu_yingyuan2));
                break;
        case SCENE_JIUQIN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_jiuqin2,sizeof(menu_jiuqin2));
                break;
        case SCENE_WENXIN:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_wenxin2,sizeof(menu_wenxin2));
                break;
#endif
        case SCENE_ZAIJIA:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_zaijia2,sizeof(menu_zaijia2));
                break;
        case SCENE_LIJIA:
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_lijia2,sizeof(menu_lijia2));
                break;
        }
}

void DrawMain(){
        //
        int xoffset = 0;
        int yoffset = 24;
        
        g_is_have_jiaju = pSystemInfo->mySysInfo.isjiaju;
#ifdef USE_DESKTOP
        JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
#else
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(0,0,menu_main_back,sizeof(menu_main_back));
#endif
        JpegDisp(rc_mainmenuico[0].x,rc_mainmenuico[0].y,menu_hujiao1,sizeof(menu_hujiao1));
        JpegDisp(rc_mainmenuico[1].x,rc_mainmenuico[1].y,menu_jianshi1,sizeof(menu_jianshi1));
        JpegDisp(rc_mainmenuico[2].x,rc_mainmenuico[2].y,menu_tuwen1,sizeof(menu_tuwen1));
#ifdef USE_DESKTOP
        JpegDisp(rc_mainmenuico[9].x,rc_mainmenuico[9].y,bt_fanhui1,sizeof(bt_fanhui1));
#endif
#if HAVE_JAIJU_SAFE
/*        if(g_cur_set_scene_mode == SCENE_ZAIJIA )//表示在家
                g_have_defend = false;
        else
                g_have_defend = true;
*/
        ViewSecneJpg1();
#else
        //  ViewSecneJpg1();

        if(!g_is_leave){
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_zaijia1,sizeof(menu_zaijia1));
                // g_have_defend = false;
        }else{
                JpegDisp(rc_mainmenuico[3].x,rc_mainmenuico[3].y,menu_lijia1,sizeof(menu_lijia1));
                // g_have_defend = true;
        }
#endif
        JpegDisp(rc_mainmenuico[4].x,rc_mainmenuico[4].y,menu_fuwu1,sizeof(menu_fuwu1));

#ifdef USE_TESHU_1
        JpegDisp(rc_mainmenuico[5].x,rc_mainmenuico[5].y,menu_teshuxitong1,sizeof(menu_teshuxitong1));
#else
        if(g_is_have_jiaju)
                JpegDisp(rc_mainmenuico[5].x,rc_mainmenuico[5].y,menu_jiaju1,sizeof(menu_jiaju1));
        else
                JpegDisp(rc_mainmenuico[5].x,rc_mainmenuico[5].y,menu_jiaju3,sizeof(menu_jiaju3));
#endif

        JpegDisp(rc_mainmenuico[6].x,rc_mainmenuico[6].y,menu_shezhi1,sizeof(menu_shezhi1));
#ifdef USED_ZHONGSHAN
	JpegDisp(rc_mainmenuico[7].x,rc_mainmenuico[7].y,menu_zhongshansip1,sizeof(menu_zhongshansip1));
#else        
        JpegDisp(rc_mainmenuico[7].x,rc_mainmenuico[7].y,menu_bangzhu1,sizeof(menu_bangzhu1));
#endif        
        JpegDisp(rc_mainmenuico[8].x,rc_mainmenuico[8].y,bt_guanping1,sizeof(bt_guanping1));

#ifndef USE_DESKTOP
/*
        WRT_Rect curbox;
        curbox.x = 150;
        curbox.y = 424;
        curbox.w = 260;
        curbox.h = 55;

#ifdef THREE_UI
	JpegDisp(110,422,menu_sub_back3,sizeof(menu_sub_back3));
#else        
        FillRect(0xdedede,curbox); 	  
#endif   
*/
        if(g_net_is_ok)
                JpegDisp(rc_ico[0].x,rc_ico[0].y,ico_wangluo1,sizeof(ico_wangluo1));
        else
                JpegDisp(rc_ico[0].x,rc_ico[0].y,ico_wangluo2,sizeof(ico_wangluo2));

        if(pSystemInfo->mySysInfo.isavoid == 1)
                JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_mianrao1,sizeof(ico_mianrao1));
        else
        {
                if(pSystemInfo->mySysInfo.isagent == 1)
                        JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_tuoguan1,sizeof(ico_tuoguan1));
                else
                        JpegDisp(rc_ico[1].x,rc_ico[1].y,ico_mianrao2,sizeof(ico_mianrao2));
        }

        if(g_have_msg){
#ifdef WRT_MORE_ROOM
                open_red_led();
#endif
                JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_xinxi1,sizeof(ico_xinxi1));
        }else{
#ifdef WRT_MORE_ROOM
                close_red_led();
#endif
                if(isnoreadvoice()){
                        JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_voice,sizeof(ico_voice));
                }else
                        JpegDisp(rc_ico[2].x,rc_ico[2].y,ico_xinxi2,sizeof(ico_xinxi2));
        }
#ifdef WRT_MORE_ROOM
        if(IsNewPhoto()) //如果存在未阅读的新图片，则打开红灯
        { 
                open_red_led();
        }
#endif

        if(g_have_warning)
                JpegDisp(rc_ico[4].x,rc_ico[4].y,ico_baojing1,sizeof(ico_baojing1));
        else
                JpegDisp(rc_ico[4].x,rc_ico[4].y,ico_baojing2,sizeof(ico_baojing2));

#endif //!USE_DESKTOP
        DrawEnText(LT("呼叫"),rc_mainmenuico[0].w +xoffset,rc_mainmenuico[0].h+yoffset ,rc_mainmenuico[0].x,rc_mainmenuico[0].y);


        DrawEnText(LT("监视"),rc_mainmenuico[1].w +xoffset,rc_mainmenuico[1].h+yoffset ,rc_mainmenuico[1].x,rc_mainmenuico[1].y);


        DrawEnText(LT("图文"),rc_mainmenuico[2].w +xoffset,rc_mainmenuico[2].h +yoffset,rc_mainmenuico[2].x,rc_mainmenuico[2].y);

        if(!g_is_leave){
                DrawEnText(LT("在家"),rc_mainmenuico[3].w +xoffset,rc_mainmenuico[3].h+yoffset ,rc_mainmenuico[3].x,rc_mainmenuico[3].y);
        }else{
                DrawEnText(LT("离家"),rc_mainmenuico[3].w +xoffset,rc_mainmenuico[3].h+yoffset ,rc_mainmenuico[3].x,rc_mainmenuico[3].y);
        }

        DrawEnText(LT("服务"),rc_mainmenuico[4].w +xoffset,rc_mainmenuico[4].h +yoffset,rc_mainmenuico[4].x,rc_mainmenuico[4].y);

#ifdef USE_TESHU_1
        DrawEnText(LT("系统"),rc_mainmenuico[5].w+xoffset,rc_mainmenuico[5].h+yoffset,rc_mainmenuico[5].x,rc_mainmenuico[5].y);
#else
        DrawEnText(LT("智联"),rc_mainmenuico[5].w +xoffset,rc_mainmenuico[5].h +yoffset,rc_mainmenuico[5].x,rc_mainmenuico[5].y);
#endif

        DrawEnText(LT("设置"),rc_mainmenuico[6].w +xoffset,rc_mainmenuico[6].h+yoffset ,rc_mainmenuico[6].x,rc_mainmenuico[6].y);

#ifdef USED_ZHONGSHAN
		DrawEnText(LT("服务中心"),rc_mainmenuico[7].w +xoffset,rc_mainmenuico[7].h+yoffset ,rc_mainmenuico[7].x,rc_mainmenuico[7].y);
#else   
        DrawEnText(LT("帮助"),rc_mainmenuico[7].w +xoffset,rc_mainmenuico[7].h+yoffset ,rc_mainmenuico[7].x,rc_mainmenuico[7].y);
#endif        

#ifndef USE_DESKTOP
/*
        uint32_t day,Month,years, dates,times;
        unsigned long min,second,Hour;
        char timename[50];
        tm_get(&dates,&times,NULL);
        day= dates&0xff;
        Month = (dates >> 8) &0xff;
        years = (dates>>16) & 0xffff;
        Hour = (times >> 16)& 0XFFFF;
        second = (times & 0xff);
        min = (times >> 8) &0xff;
        sprintf(timename,"%04d/%02d/%02d %02d:%02d",years,Month,day,Hour,min);

        DrawText_16(timename,150,460,rgb_black);
*/
        DisplaySafeSatus();
        if(Get_safe_status())
                JpegDisp(rc_ico[3].x,rc_ico[3].y,ico_shefang1,sizeof(ico_shefang1));
        else
                JpegDisp(rc_ico[3].x,rc_ico[3].y,ico_shefang2,sizeof(ico_shefang2));
        
#endif //!USE_DESKTOP


	DrawHuanjinValue(0,0);
	DrawHuanjinValue(0,0);
	DrawHuanjinValue(0,0);
	DrawWeatherResult();
	UpdateTimeIcon();

}

int set_leave_delay(int param)
{
	CloseWindow();
	CreateLeaveDelayWin();
	return 1;
}


int set_leave_nodelay(int param)
{
        int i;
        g_is_leave = param; //2009-7-23
        g_have_defend = param;
		g_cur_set_scene_mode = SCENE_LIJIA;

        for(i =0;i<16;i++){
                        //	if(pSystemInfo->mySysInfo.safestatus[16+i] != 0)
#if HAVE_JIAJU_SAFE
               set_cur_scenemode_safe_status(g_cur_set_scene_mode,i,1);
#else
               set_security_status(i,1);
#endif
       }
       CreateDialogWin2(LT("控制'离家'情景..."),MB_NONE,NULL,NULL);
       send_scene_mode(g_cur_set_scene_mode);//离家
       UpdateSystemInfo();

       if(GetIsStartDefendStatus() ==1 ){
               check_alarm2();
               event[0] = CUSTOM_CMD_WARNING_TIMER;
               event[1] = event[2] = event[3] = 0;
               wrthost_send_cmd(event);
               g_isstartdelay = true;
               WRT_DEBUG("已经开始布防");
               SDL_Delay(100);
               StartAlaramPromptVocie();
       }

       //逐条发送插座延时关闭指令。
       send_delay_jack(1);
       CloseWindow();	
       return 0;
}

int cur_set_leave_home(int  param){
        g_is_leave = param; //2009-7-23
        g_have_defend = param;
        unsigned long event[4];
        int i =0;
        //当设置为离家状态时，将模式索引6，发送给底层

        if(param){
                g_cur_set_scene_mode = SCENE_LIJIA;

                for(i =0;i<16;i++){
                        set_security_status(i,1);
                }

                CreateDialogWin2(LT("控制'离家'情景..."),MB_NONE,NULL,NULL);
                send_scene_mode(g_cur_set_scene_mode);//离家
                UpdateSystemInfo();
                CloseWindow();
                SDL_Delay(200);
                if(GetIsStartDefendStatus() ==1 ){
                    check_alarm2();
                    event[0] = CUSTOM_CMD_WARNING_TIMER;
                    event[1] = event[2] = event[3] = 0;
                    wrthost_send_cmd(event);
                    g_isstartdelay = true;
                    WRT_DEBUG("已经开始布防");
                    SDL_Delay(100);
                    StartAlaramPromptVocie();
                    SendAlarmLogToCenter(1);
               }
        }else{
                g_cur_set_scene_mode = SCENE_ZAIJIA;
                
                for(i =0;i<16;i++){
                        set_security_status(i,0);
                }

                CreateDialogWin2(LT("控制'在家'情景..."),MB_NONE,NULL,NULL);
                send_scene_mode(g_cur_set_scene_mode);//在家
                UpdateSystemInfo();
				SDL_Delay(100);
				printf("close false \n");
                CloseWindow();
                check_alarm(0); //先清空，是否还需要重新布防在家防区呢？
				SendAlarmLogToCenter(0);
        }
	return 0;
}

int MainEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        int icount = 9;
        xy.x = x;
        xy.y = y;
#ifdef USE_DESKTOP
        icount = 10;
#endif
        for(index =0 ;index<icount;index++){
                if(IsInside(rc_mainmenuico[index],xy)){
                        ret = 1;
                        
                        pos_x = rc_mainmenuico[index].x;
                        pos_y = rc_mainmenuico[index].y;
                        // if(status == 1)
                        //      StartButtonVocie();
                switch(index){
                case 0: //呼叫
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_hujiao2,sizeof(menu_hujiao2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_hujiao1,sizeof(menu_hujiao1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateCallWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateCallWin();
                              // CreateLeaveDelayWin();
                                //zenMemUninit();
                        }
                        break;
                case 1://监视
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jianshi2,sizeof(menu_jianshi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_jianshi1,sizeof(menu_jianshi1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreatetMonitorWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreatetMonitorWin();

                        }
                        break;
                case 2://图文
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_tuwen2,sizeof(menu_tuwen2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_tuwen1,sizeof(menu_tuwen1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateInfoWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateInfoWin();
                        }
                        break;
                case 3://在家/离家
#if HAVE_JIAJU_SAFE
                        if(status == 1){
                                StartButtonVocie();
                                ViewSecneJpg2();
                        }else{
                                ViewSecneJpg1();

                                if(g_cur_set_scene_mode == SCENE_ZAIJIA){
										printf("zaijia \n");
                                        CreateDialogWin2(LT("离家状态下,防区将设防,电源将关闭.确定要选择离家模式吗?"),MB_OK|MB_CANCEL,cur_set_leave_home,NULL);
                                }else if(g_cur_set_scene_mode  == SCENE_LIJIA){
                                		printf("lijia \n");
                                        if(GetIsStartDefendStatus() == 1){
                                                CreatePasswordWin(SET_CHEFANG_STATUS);
                                        }else{
                                                cur_set_leave_home(false);
												SDL_Delay(500);
												CloseWindow();
                                        }

                                }else{
                                        g_cur_set_scene_mode = SCENE_ZAIJIA;
                                        DrawMain();
                                }

                        }
#else
                        if(!g_is_leave){
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_zaijia2,sizeof(menu_zaijia2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_zaijia1,sizeof(menu_zaijia1));
                                        //JpegDisp(pos_x,pos_y,menu_lijia1,sizeof(menu_lijia1));
                                        //模式索引为 6;将其发送给底层
                                        ClearRect(0,0,1024,600);
                                        if(isdelayjack())
                                        	CreateDialogWin2(LT("是否设置延时?按'确定'设置延时,并且设置离家情景,按'取消'则只设置离家情景."),MB_OK|MB_CANCEL,set_leave_delay,set_leave_nodelay);
                                        else
                                        	CreateDialogWin2(LT("离家状态下,防区将设防,电源将关闭.确定要选择离家模式吗?"),MB_OK|MB_CANCEL,cur_set_leave_home,NULL); 
                                }
                        }else{
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_lijia2,sizeof(menu_lijia2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_lijia1,sizeof(menu_lijia1));
                                        // JpegDisp(pos_x,pos_y,menu_zaijia1,sizeof(menu_zaijia1));
                                        // g_is_leave = false;
                                        //创建密码输入Dialog
                                        
                                        CreatePasswordWin(SET_CHEFANG_STATUS);
                                }
                        }
#endif
                        break;
                case 4://服务
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_fuwu2,sizeof(menu_fuwu2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_fuwu1,sizeof(menu_fuwu1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateServiceWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateServiceWin();
#ifdef TEST_FFS

                                //    test_file();
#endif
                        }
                        break;
                case 5://家居
#ifdef USE_TESHU_1
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_teshuxitong2,sizeof(menu_teshuxitong2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_teshuxitong1,sizeof(menu_teshuxitong1));                                 
                                CreateSysInfoWin();                    
                        } 
#else
                        if(!g_is_have_jiaju)
                                break;
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jiaju2,sizeof(menu_jiaju2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_jiaju1,sizeof(menu_jiaju1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateJiajuWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateSceneWin();

                        }
#endif
                        break;
                case 6://设置
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shezhi2,sizeof(menu_shezhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shezhi1,sizeof(menu_shezhi1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateSetupWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateSetupWin();

                        }
                        break;
                case 7: //帮助
                        if(status == 1){ //按下
                                StartButtonVocie();
#ifdef USED_ZHONGSHAN
				JpegDisp(pos_x,pos_y,menu_zhongshansip2,sizeof(menu_zhongshansip2));
#else                                
                                JpegDisp(pos_x,pos_y,menu_bangzhu2,sizeof(menu_bangzhu2));
#endif                                
                        }else{
#ifdef USED_ZHONGSHAN
								JpegDisp(pos_x,pos_y,menu_zhongshansip1,sizeof(menu_zhongshansip1));
								CreatePeopleInfoWin2();
#else                        	
                                JpegDisp(pos_x,pos_y,menu_bangzhu1,sizeof(menu_bangzhu1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateHelpWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                ClearRect(0,0,1024,600);
                                CreateHelpWin();
                                //CreateMutilmediaMain();
#endif                                
                        }
                        break;
                case 8: //关屏
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                        }
                        break;
#ifdef USE_DESKTOP
                case 9:
                        if(status == 1)
                        {
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));

                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
#endif
                default:
                        break;
                        }
                        break;
                }
        }
        return ret;
}
/****************************************************************
创建第二个主窗口，可以再第一个和第二个主界面之间切换
****************************************************************/
/*
void CreateMainWin2(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;        
        pWindow->type == ROOT;
        pWindow->CloseWindow = NULL;
        pWindow->NotifyReDraw2 = DrawMainWin2;
        pWindow->EventHandler = MainWin2EventHandle;

        InitAlphabetAndNumber();
        g_net_is_ok = false;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
        for(int i =0 ; i< 42;i++)
                g_rc_status[i] = false;  
        
     
}

void DrawMainWin2()
{
}

int MainWin2EventHandle(int x,int y,int status)
{
}
*/



/****************************************************************
创建门口机呼叫分机窗口
****************************************************************/
bool  g_is_kinescope = false;           //是否有录像功能
bool  g_is_talk = false;                //是否按下对讲按钮
bool  g_is_mute  = false;               //是否按下静音按钮
bool  g_is_lock = false;                //开锁按钮是否有效
bool  g_is_rec_video = false;
bool  g_is_hangup = false;
extern int g_cap_flag;
int   g_holdon_flag = 0;//1-分机摘机 2-模拟分机摘机
extern int g_phone_or_externdevice_holdon;
int g_ipcamera_flag = 0;

static const WRT_Rect   rc_callico[7]={
	{866,24,144, 68},        //对讲
	{866,102,144, 68},        //开锁
	{866,180,144,68},        //抓拍
	{866,258,144,68},        //录像
	{866,336,144,68},         //静音
	{866,414,144,68},         //挂机
	{866,492,144,68}          //退出
};

void CreateDoorCallWin(){
	g_is_talk = false;
	g_is_mute = false;
	g_is_lock = false;
	g_is_unlock = false;
	g_is_hangup = false;
    g_holdon_flag = 3;
    g_phone_or_externdevice_holdon = 1;
	g_agentticks = 0;
	g_ipcamera_flag = 0;
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;

	pWindow->EventHandler = DoorCallEventHandler;
	pWindow->NotifyReDraw = DrawDoorCallMain;
	pWindow->valid = 1;
	pWindow->type = CALLED;
	g_is_rec_video = false;
	pWindow->CloseWindow = CloseCallWin;
	WindowAddToWindowList(pWindow);

	pWindow->NotifyReDraw2(pWindow);
	tm_getticks(NULL,(uint32_t *)&g_agentticks);
}

void copy_to_sencod_and_three_fbpage()	 
{  
	return;
	int  i,j;
	long int fb_size;
	char tmpbuf[1228800];
	int d1,d2;
	int fbfd = 0;	
	
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;   
	long int screensize = 0;   
	while(g_cap_flag == 1)
	{
		usleep(100*1000);
	}

	fbfd = open("/dev/fb0", O_RDWR);   
	if (fbfd < 0) {
		perror("open /dev/fb0 failed");
		return;
	}
  
	ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo);  
	ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);	

	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;   
	
	fb_size = finfo.smem_len;

	char *fbp=(char*)mmap(0,fb_size,PROT_READ|PROT_WRITE,MAP_SHARED, fbfd, 0);	
	if (fbp == NULL) {
		perror("mmap failed");
		close(fbfd);
		return;
	}
	
	for(i=0 ;i<600;i++)
	{
		memcpy(fbp+1228800+(i*1024*2)+2*854,fbp+(i*1024*2)+2*854,2*(1024-854));
	}
	for(j=0;j<600;j++)
	{
		memcpy(fbp+2*1228800+(j*1024*2)+2*854,fbp+(j*1024*2)+2*854,2*(1024-854));
	}
	
	munmap(fbp, fb_size);  
	
	close(fbfd);

}

/******************************************
判断是否能够抓拍图片
by ljw 2012-11-14 16:01:31
*******************************************/
static int CanCapPhoto()
{
	DIR *dirp;
	struct dirent * ptr;
	dirp = opendir("/customer/wrt/pic/picture");
	int count = 0;
	if(dirp == NULL)
		return 1;
	while((ptr = readdir(dirp))!=NULL)
	{
		count++;
	}
	closedir(dirp);
	dirp = NULL;
	if(count >200)//测试抓拍张数
	{
		WRT_DEBUG("抓拍失败，抓拍图片张数已超上限");
		return 0;
	}
	return 1;
}

static int CanRecVideo()
{
	DIR *dirp;
	struct dirent * ptr;
	dirp = opendir("/customer/wrt/pic/video");
	int count = 0;
	if(dirp == NULL)
		return 1;
	while((ptr = readdir(dirp))!=NULL)
	{
		count++;
	}
	closedir(dirp);
	dirp = NULL;
	if(count >50)//录像文件个数上限
	{
		WRT_DEBUG("录像失败，录像个数已超上限");
		return 0;
	}	
	return 1;
}

void DrawDoorCallMain(){

    if(pSystemInfo->mySysInfo.screensize == 0x10){
    	OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
    	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
    }else{
    	WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 600;
		FillRect(rgb_colorKey,rt);

		JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
	}

	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
  
	JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang1,sizeof(menu_duijiang1));
	if(g_is_lock)
	        JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo1,sizeof(menu_kaisuo1));
	else
	        JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo3,sizeof(menu_kaisuo3));
	if(CanCapPhoto())
		JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai1,sizeof(menu_zhuapai1));
	else
		JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
	if(CanRecVideo())
		JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang1,sizeof(menu_luxiang1));
	else
		JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));

	JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin4,sizeof(menu_jingyin4));
	JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji1,sizeof(menu_guaji1));
	JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuoguan_add1,sizeof(menu_tuoguan_add1));

#if USE_VOLUME_IN_VIDEO
	DrawVolumeSize();
#endif
}

int  DoorCallEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        uint32_t tmpticks =0;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<7;index++){
                if(IsInside(rc_callico[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_callico[index].x;
                        pos_y = rc_callico[index].y;
                switch(index){
                case 0: //对讲	
                		if(g_is_talk)
                			break;
                		if(g_is_hangup)
                			break;
                        if(status == 1) {
                                if(!g_is_mute)
                                	StartButtonVocie();
                                OSD_JpegDisp(pos_x,pos_y,menu_duijiang2,sizeof(menu_duijiang2));
                                copy_to_sencod_and_three_fbpage();
                        }else{
                                
                          
                                if(!g_is_talk){
                                        event[0] = ROOM_HOLDON;
                                        event[1] = event[2] = event[3]=0;
                                        wrthost_send_cmd(event);
                                        g_is_talk = !g_is_talk;
                                        OSD_JpegDisp(pos_x,pos_y,menu_duijiang3,sizeof(menu_duijiang3));
                                		copy_to_sencod_and_three_fbpage();

                                }
                                if(g_is_lock == false){
                                        g_is_lock = true;

                                        OSD_JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo1,sizeof(menu_kaisuo1));
                                        copy_to_sencod_and_three_fbpage();
                                        
                                }
                        }
                        break;
                case 1://开锁
                		if(g_is_hangup)
                			break;
                        if(!g_is_lock)
                                break;
                        if(status == 1){ //按下
                                if(!g_is_mute)
                                	StartButtonVocie();
   
                                OSD_JpegDisp(pos_x,pos_y,menu_kaisuo2,sizeof(menu_kaisuo2));
                                copy_to_sencod_and_three_fbpage();
      
                        } else{
                                OSD_JpegDisp(pos_x,pos_y,menu_kaisuo1,sizeof(menu_kaisuo1));
                                copy_to_sencod_and_three_fbpage();
                                
                                if(g_is_talk){ //只有在对讲的情况下才能开锁
                                        event[0] = ROOM_UNLOCK;
                                        event[1] = event[2] = event[3]=0;
                                        wrthost_send_cmd(event);
                                        g_is_unlock = true;
                                }
                        }
                        break;
                case 2://抓拍	
                		if(g_is_hangup)
                			break;
                		if(CanCapPhoto() == 0)
                			break;
                        if(status == 1){ //按下
                                if(!g_is_mute)
                                	StartButtonVocie();
                                OSD_JpegDisp(pos_x,pos_y,menu_zhuapai2,sizeof(menu_zhuapai2));
                                copy_to_sencod_and_three_fbpage();

                        }else{
								g_have_cap_photo =1;
                                OSD_JpegDisp(pos_x,pos_y,menu_zhuapai1,sizeof(menu_zhuapai1));
                                copy_to_sencod_and_three_fbpage();
                                SDL_Delay(100);
                                uint32_t newtime;
                                unsigned long result=0;
                                tm_getticks(NULL,(uint32_t *)&newtime);
                                result = newtime - g_cap_time;
                                if(result > 5000 ){
                                        event[0] = CUSTOM_CMD_PHOTO;
                                        event[1] = event[2] = event[3]=0;
                                        wrthost_send_cmd(event);
                                        g_cap_time = newtime;
                                }

                        }
                        break;
                case 3://录像
                		if(g_is_hangup)
                			break;
                        if(CanRecVideo() == 0)
                        	break;
                        if(status == 1){ //按下
                                if(!g_is_mute)
                                	StartButtonVocie();
                                OSD_JpegDisp(pos_x,pos_y,menu_luxiang2,sizeof(menu_luxiang2));
                                copy_to_sencod_and_three_fbpage();
                                
                        }else{
							if(g_is_rec_video == false)
							{
								long _size,fsize;
								struct statfs vbuf;
								statfs("/",&vbuf);
								_size = (vbuf.f_bsize * vbuf.f_bfree);
								fsize = (float)_size/(1024*1024);
								if(fsize < 30)
								{
									OSD_JpegDisp(pos_x,pos_y,menu_luxiang3,sizeof(menu_luxiang3));
	                                copy_to_sencod_and_three_fbpage();
	                                break;
								}else
								{
	                                OSD_JpegDisp(pos_x,pos_y,menu_luxiang4,sizeof(menu_luxiang4));
	                                copy_to_sencod_and_three_fbpage();

	                                event[0] = CUSTOM_CMD_RECORD_JPEG_FRAME;
	                                event[1] = event[2] = event[3]=0;
	                                wrthost_send_cmd(event);
	                                printf("录像\n");
	                                g_is_rec_video = true;
								}
							}else
							{
									OSD_JpegDisp(pos_x,pos_y,menu_luxiang1,sizeof(menu_luxiang1));
	                                copy_to_sencod_and_three_fbpage();

	                                event[0] = CUSTOM_CMD_STOP_JPEG_FRAME;
	                                event[1] = event[2] = event[3]=0;
	                                wrthost_send_cmd(event);
	                                printf("停止录像\n");
	                                g_is_rec_video = false;
		                     }
                        }
                        break;
                case 4://静音
                		if(g_is_hangup)
                			break;
                		if(g_is_mute)
                			break;
                        if(status == 1){//按下
                        	if(!g_is_mute)
                                	StartButtonVocie();
                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin2,sizeof(menu_jingyin2));
                        	copy_to_sencod_and_three_fbpage();
                        }else{
							if(!g_is_mute)
                                	g_is_mute = true;
                            if(g_is_mute)
                            {
                                mp3stop();
	                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin1,sizeof(menu_jingyin1));
	                        }else
	                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin4,sizeof(menu_jingyin4));
	                        	
	                        copy_to_sencod_and_three_fbpage();

	                        
                        }
                        break;
                case 5://挂机
                		
                		if(g_is_hangup)
                			break;
                        if(status == 1){ //按下
                                if(!g_is_mute)
                                	StartButtonVocie();

                                OSD_JpegDisp(pos_x,pos_y,menu_guaji2,sizeof(menu_guaji2));
                                copy_to_sencod_and_three_fbpage();

                        }else{

                                OSD_JpegDisp(pos_x,pos_y,menu_guaji1,sizeof(menu_guaji1));
                                copy_to_sencod_and_three_fbpage();

								SDL_Delay(300);
                                event[0] = ROOM_HANGUP;
                                event[1] = event[2] = event[3]=0;
                                if(g_is_hangup == false)
                                {
                                	wrthost_send_cmd(event);
                                	g_is_hangup = true;
                                }
                                g_is_talk =false;
                        }
                        break;
                case 6://转接
	                	if(g_is_unlock) //如果已经开锁不允许转接
	                		break;
                		if(g_is_hangup)
                			break;
                        tm_getticks(NULL,(uint32_t *)&tmpticks);
                        if((tmpticks - g_agentticks) < 5000)
                        {
                            break;
                        }
                        if(status == 1){ //按下
                                if(!g_is_mute)
                                	StartButtonVocie();

                                OSD_JpegDisp(pos_x,pos_y,menu_tuoguan_add2,sizeof(menu_tuoguan_add2));
                                copy_to_sencod_and_three_fbpage();

                        }else{
                
                                OSD_JpegDisp(pos_x,pos_y,menu_tuoguan_add1,sizeof(menu_tuoguan_add1));
                                copy_to_sencod_and_three_fbpage();

                        		SDL_Delay(300);
                		        event[0] = ROOM_AGENT;
                        		event[1] = event[2] = event[3]=0;
                        		if(g_is_hangup == false)
                        		{
                                	wrthost_send_cmd(event);
	                        		g_is_hangup = true;
                                }
                        }
                        break;
                default:
                        break;
                        }
                        break;
                }
        }
#if USE_VOLUME_IN_VIDEO
        if(ret != 1)
                ret = HandlerVolume(x,y,status);
#endif
        return ret;
}

/****************************************************************
创建中心呼叫分机窗口
****************************************************************/
void CreateCenterCallWin(){
	g_is_talk = false;
	g_is_mute = false;
	g_is_hangup = false;
	g_holdon_flag = 3;
	g_phone_or_externdevice_holdon = 1;
	g_ipcamera_flag = 0;
	//g_is_doorcallroom = 0;
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = CenterCallEventHandler;
	pWindow->NotifyReDraw = DrawCenterCallMain;
	pWindow->valid = 1;
	pWindow->CloseWindow = CloseCallWin;
	pWindow->type = CALLED;
	WindowAddToWindowList(pWindow);

	pWindow->NotifyReDraw2(pWindow);
}

void DrawCenterCallMain(){

    if(pSystemInfo->mySysInfo.screensize == 0x10){
    	OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
    	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
    }else{
    	WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 600;
		FillRect(rgb_black,rt);

		JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
	}

	JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang1,sizeof(menu_duijiang1));
	JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo3,sizeof(menu_kaisuo3));
	JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
	if(g_is_kinescope)
		JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang1,sizeof(menu_luxiang1));
	else
		JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));

	JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin4,sizeof(menu_jingyin4));
	JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji1,sizeof(menu_guaji1));
	JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu3,sizeof(menu_tuichu3));

#if USE_VOLUME_IN_VIDEO
	DrawVolumeSize();
#endif
}

int CenterCallEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<7;index++){
                if(IsInside(rc_callico[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_callico[index].x;
                        pos_y = rc_callico[index].y;
                        switch(index){
                case 0: //对讲
                		if(g_is_talk)
                			break;
                        if(status == 1) {
                                StartButtonVocie();
         
                                        OSD_JpegDisp(pos_x,pos_y,menu_duijiang2,sizeof(menu_duijiang2));
                        }else{

                                        OSD_JpegDisp(pos_x,pos_y,menu_duijiang1,sizeof(menu_duijiang1));
                                if(!g_is_talk){
                                        event[0] = ROOM_HOLDON;
                                        event[1] = event[2] = event[3]=0;
                                        wrthost_send_cmd(event);
                                        g_is_talk = !g_is_talk;

                                }
                        }
                        break;
                case 1://开锁
                        break;
                case 2://抓拍
                        break;
                case 3://录像
                		break;
                        
                case 4://静音
                		if(g_is_mute)
                			break;
                        if(status == 1){ //按下
                                StartButtonVocie();

                                OSD_JpegDisp(pos_x,pos_y,menu_jingyin2,sizeof(menu_jingyin2));
                        }else{
                            if(!g_is_mute)
                            	g_is_mute = true;
                            if(g_is_mute)
                            {
                                mp3stop();
	                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin1,sizeof(menu_jingyin1));
	                        }else
	                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin4,sizeof(menu_jingyin4));
                        }
                        break;
                case 5://挂机
                        if(status == 1){ //按下
                                StartButtonVocie();
                                OSD_JpegDisp(pos_x,pos_y,menu_guaji2,sizeof(menu_guaji2));
                        }else{
                                OSD_JpegDisp(pos_x,pos_y,menu_guaji1,sizeof(menu_guaji1));
                                event[0] = ROOM_HANGUP;
                                event[1] = event[2] = event[3]=0;
                                wrthost_send_cmd(event);
                        }
                        break;
                case 6://退出
/*
                        if(status == 1){ //按下
                                StartButtonVocie();
                                if(GetCurrentLanguage() == ENGLISH)
                                        OSD_JpegDisp(pos_x,pos_y,emenu_tuoguan2,sizeof(emenu_tuoguan2));
                                else
                                        OSD_JpegDisp(pos_x,pos_y,menu_tuoguan_add2,sizeof(menu_tuoguan_add2));
                        }else{
                                if(GetCurrentLanguage() == ENGLISH)
                                        OSD_JpegDisp(pos_x,pos_y,emenu_tuoguan1,sizeof(emenu_tuoguan1));
                                else
                                        OSD_JpegDisp(pos_x,pos_y,menu_tuoguan_add1,sizeof(menu_tuoguan_add1));
                                //if(g_is_talk == 0){
                                event[0] = ROOM_AGENT;
                                event[1] = event[2] = event[3]=0;
                                wrthost_send_cmd(event);
                                //}
                        }
*/
                        break;
                default:
                        break;
                        }
                        break;
                }
        }
#if USE_VOLUME_IN_VIDEO
        if(ret != 1)
                ret = HandlerVolume(x,y,status);
#endif
        return ret;
}
static int g_ishaveagent = 0;

//户户通被叫分机界面
void CreateRoomCallWin(int type){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
	        return ;
        g_agentticks = 0;
        g_ishaveagent = type;
        g_is_hangup = false;
        g_holdon_flag = 3;
        g_phone_or_externdevice_holdon = 1;
        g_ipcamera_flag = 0;

        if(g_ishaveagent != 0x0)
        {
        	tm_getticks(NULL,(uint32_t *)&g_agentticks);
        }
        pWindow->EventHandler = RoomCallEventHandler;
        pWindow->NotifyReDraw = DrawRoomCallMain;
        pWindow->valid = 1;
        pWindow->CloseWindow = CloseCallWin;
        pWindow->type = CALLED;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawRoomCallMain(){

    if(pSystemInfo->mySysInfo.screensize == 0x10){
    	OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
    	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
    }else{
    	WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 600;
		FillRect(rgb_black,rt);

		JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
	}

    JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang1,sizeof(menu_duijiang1));
    JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_novideo1,sizeof(menu_novideo1));
    JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
    
    JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang1,sizeof(menu_luxiang1));

    JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin4,sizeof(menu_jingyin4));
    JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji1,sizeof(menu_guaji1));
    if(g_ishaveagent == 0)
    	JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu3,sizeof(menu_tuichu3));
    else
    	JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuoguan_add1,sizeof(menu_tuoguan_add1));
       
#if USE_VOLUME_IN_VIDEO
        DrawVolumeSize();
#endif
}
//分机呼叫我
int  RoomCallEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        uint32_t tmpticks = 0;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<7;index++){
                if(IsInside(rc_callico[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_callico[index].x;
                        pos_y = rc_callico[index].y;
                        switch(index){
                case 0: //对讲
                		if(g_is_talk)
                			break;
                		if(g_is_hangup)
                			break;
                        if(status == 1) {
                                OSD_JpegDisp(pos_x,pos_y,menu_duijiang2,sizeof(menu_duijiang2));
                                copy_to_sencod_and_three_fbpage();
                        }else{
                                if(!g_is_talk){
                                    event[0] = ROOM_HOLDON;//回复分机摘机
                                    event[1] = event[2] = event[3]=0;
                                    wrthost_send_cmd(event);
                                    g_is_talk = !g_is_talk;
                                    OSD_JpegDisp(pos_x,pos_y,menu_duijiang3,sizeof(menu_duijiang3));
                                    copy_to_sencod_and_three_fbpage();
                                }
                        }
                        break;
                case 1://是否发送视频
                		if(g_is_hangup)
                			break;
                		if(get_venc_type() <= V_NONE)
                			break;
                		if(!g_is_talk)
                			break;//对讲下才支持发送视频
                			
                		if(status == 1){
                        	OSD_JpegDisp(pos_x,pos_y,menu_novideo2,sizeof(menu_novideo2));
                        	copy_to_sencod_and_three_fbpage();
                        }else{
                        	g_send_my_video = !g_send_my_video;
                        	if(g_send_my_video)
                        	{
                        		OSD_JpegDisp(pos_x,pos_y,menu_havevideo1,sizeof(menu_havevideo1));
                        		copy_to_sencod_and_three_fbpage();
                        	}
                        	else
                        	{
                        		OSD_JpegDisp(pos_x,pos_y,menu_novideo1,sizeof(menu_novideo1));
                        		copy_to_sencod_and_three_fbpage();
                        	}
                        }
                        break;
                case 2://抓拍

                        break;
                case 3://录像

                        if(g_is_hangup)
                			break;
                		if(!g_is_talk)
                			break;
                        if(status == 1){ //按下
                            JpegDisp(pos_x,pos_y,menu_luxiang2,sizeof(menu_luxiang2));
                            copy_to_sencod_and_three_fbpage();
                        }else{
                        	
                            if(g_is_rec_video == false)
							{
								long _size,fsize;
								struct statfs vbuf;
								statfs("/",&vbuf);
								_size = (vbuf.f_bsize * vbuf.f_bfree);
								fsize = (float)_size/(1024*1024);
								if(fsize < 30)
								{
									OSD_JpegDisp(pos_x,pos_y,menu_luxiang3,sizeof(menu_luxiang3));
	                                copy_to_sencod_and_three_fbpage();
	                                break;
								}else
								{
	                                OSD_JpegDisp(pos_x,pos_y,menu_luxiang4,sizeof(menu_luxiang4));
	                                copy_to_sencod_and_three_fbpage();

	                                event[0] = CUSTOM_CMD_RECORD_JPEG_FRAME;
	                                event[1] = event[2] = event[3]=0;
	                                wrthost_send_cmd(event);
	                                printf("录像264\n");
	                                g_is_rec_video = true;

								}
							}else
							{
									OSD_JpegDisp(pos_x,pos_y,menu_luxiang1,sizeof(menu_luxiang1));
	                                copy_to_sencod_and_three_fbpage();

	                                event[0] = CUSTOM_CMD_STOP_JPEG_FRAME;
	                                event[1] = event[2] = event[3]=0;
	                                wrthost_send_cmd(event);
	                                printf("停止录像264\n");
	                                g_is_rec_video = false;
		                     }
                        }
                        break;
                case 4://静音
	                	if(g_is_hangup)
	                		break;
	                	if(g_is_mute)
                			break;
                        if(status == 1){ //按下

                                OSD_JpegDisp(pos_x,pos_y,menu_jingyin2,sizeof(menu_jingyin2));
                                copy_to_sencod_and_three_fbpage();
                        }else{
                              if(!g_is_mute)
                                	g_is_mute = true;
	                            if(g_is_mute)
	                            {
	                                mp3stop();
		                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin1,sizeof(menu_jingyin1));
		                        }else
		                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin4,sizeof(menu_jingyin4));
                        }
                        break;
                case 5://挂机
	                	if(g_is_hangup)
	                		break;
                        if(status == 1){ //按下
	                            OSD_JpegDisp(pos_x,pos_y,menu_guaji2,sizeof(menu_guaji2));
	                            copy_to_sencod_and_three_fbpage();
                        }else{

                                OSD_JpegDisp(pos_x,pos_y,menu_guaji1,sizeof(menu_guaji1));
                                copy_to_sencod_and_three_fbpage();
                                SDL_Delay(300);
                                event[0] = ROOM_HANGUP;
                                event[1] = event[2] = event[3]=0;
                                if(g_is_hangup == false)
                                {	
                                	g_is_hangup = true;
	                                wrthost_send_cmd(event);
                                }
                        }
                        break;
                case 6://退出或托管
	                	if(g_is_hangup)
	                		break;
	                	if(g_ishaveagent == 0)
	                		break;
                        tm_getticks(NULL,(uint32_t *)&tmpticks);
                        if((tmpticks - g_agentticks) < 5000)
                                	break;     
                        if(status == 1){ //按下
                                StartButtonVocie();

                                OSD_JpegDisp(pos_x,pos_y,menu_tuoguan_add2,sizeof(menu_tuoguan_add2));
                                copy_to_sencod_and_three_fbpage();
                        }else{
                                OSD_JpegDisp(pos_x,pos_y,menu_tuoguan_add1,sizeof(menu_tuoguan_add1));
                                copy_to_sencod_and_three_fbpage();
                             
                                SDL_Delay(300);
                                event[0] = ROOM_AGENT;
                                event[1] = event[2] = event[3]=0;
                                if(g_is_hangup == false)
                                {
                                	g_is_hangup = true;
	                                wrthost_send_cmd(event);
                                }
                                
                        }
                        break;
                default:
                        break;
                        }
                        break;
                }
        }
#if USE_VOLUME_IN_VIDEO
        if(ret != 1)
                ret = HandlerVolume(x,y,status);
#endif
        return ret;
}

/****************************************************************
创建主动呼叫主窗口
****************************************************************/
static const WRT_Rect rc_active_call[13]={
        {150,175,132,132},              //呼叫中心
        {350,175,132,132},             //呼叫住户
        {550,175,132,132},             //大堂经理
        {750,175,132,132},             //电梯控制

        {900,531,111,69},              //返回
        {6,531,111,69},                 //关屏幕

        {157,175,132,132},             //呼叫中心
        {308,175,132,132},             //呼叫住户
        {459,175,132,132},             //大堂经理
        {610,175,132,132},             //电梯控制
        {761,175,132,132},             // 备用电梯控制
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

void CreateCallWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = CallEventHandler;
        pWindow->NotifyReDraw = DrawCallMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}

void DrawCallMain(){
        int xoffset;
        int yoffset = 24;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        if(pSystemInfo->mySysInfo.bakliftip == 0){
                JpegDisp(rc_active_call[0].x,rc_active_call[0].y,ico_hujiaozhongxin1,sizeof(ico_hujiaozhongxin1));//呼叫中心
                JpegDisp(rc_active_call[1].x,rc_active_call[1].y,ico_hujiaozhuhu1,sizeof(ico_hujiaozhuhu1));//呼叫住户
                JpegDisp(rc_active_call[2].x,rc_active_call[2].y,ico_datangjingli1,sizeof(ico_datangjingli1)); //大堂经理
                JpegDisp(rc_active_call[3].x,rc_active_call[3].y,ico_calllift1,sizeof(ico_calllift1)); //电梯控制
                JpegDisp(rc_active_call[4].x,rc_active_call[4].y,bt_fanhui1,sizeof(bt_fanhui1));//返回
                JpegDisp(rc_active_call[5].x,rc_active_call[5].y,bt_guanping1,sizeof(bt_guanping1));//关屏
        }else{//有备用电梯
                JpegDisp(rc_active_call[6].x,rc_active_call[6].y,ico_hujiaozhongxin1,sizeof(ico_hujiaozhongxin1));
                JpegDisp(rc_active_call[7].x,rc_active_call[7].y,ico_hujiaozhuhu1,sizeof(ico_hujiaozhuhu1));
                JpegDisp(rc_active_call[8].x,rc_active_call[8].y,ico_datangjingli1,sizeof(ico_datangjingli1));
                JpegDisp(rc_active_call[9].x,rc_active_call[9].y,ico_calllift1,sizeof(ico_calllift1)); //电梯控制
                JpegDisp(rc_active_call[10].x,rc_active_call[10].y,ico_calllift1,sizeof(ico_calllift1)); //备用电梯控制
                JpegDisp(rc_active_call[11].x,rc_active_call[11].y,bt_fanhui1,sizeof(bt_fanhui1));

                JpegDisp(rc_active_call[12].x,rc_active_call[12].y,bt_guanping1,sizeof(bt_guanping1));
        }

        xoffset = 0; //21
        if(pSystemInfo->mySysInfo.bakliftip == 0){

                DrawEnText(LT("呼叫中心"),rc_active_call[0].w + xoffset,rc_active_call[0].h+yoffset,rc_active_call[0].x,rc_active_call[0].y);

                DrawEnText(LT("呼叫住户"),rc_active_call[1].w + xoffset,rc_active_call[1].h+yoffset,rc_active_call[1].x,rc_active_call[1].y);
                
                DrawEnText(LT("大堂经理"),rc_active_call[2].w + xoffset,rc_active_call[2].h+yoffset,rc_active_call[2].x,rc_active_call[2].y);

                DrawEnText(LT("召唤电梯"),rc_active_call[3].w + xoffset,rc_active_call[3].h+yoffset,rc_active_call[3].x,rc_active_call[3].y);
        }else{

                DrawEnText(LT("呼叫中心"),rc_active_call[6].w + xoffset,rc_active_call[6].h+yoffset,rc_active_call[6].x,rc_active_call[6].y);

                DrawEnText(LT("呼叫住户"),rc_active_call[7].w + xoffset,rc_active_call[7].h+yoffset,rc_active_call[7].x,rc_active_call[7].y);
                
                DrawEnText(LT("大堂经理"),rc_active_call[8].w + xoffset,rc_active_call[8].h+yoffset,rc_active_call[8].x,rc_active_call[8].y);

                DrawEnText(LT("电梯"),rc_active_call[9].w + xoffset,rc_active_call[9].h+yoffset,rc_active_call[9].x,rc_active_call[9].y); //Lift 1
                DrawEnText(LT("备用电梯1"),rc_active_call[10].w + xoffset,rc_active_call[10].h+yoffset,rc_active_call[10].x,rc_active_call[10].y); //Lift 1
        }
}
/*
static test_memory(){
char* tmp = NULL;
int count = 0;
while(1){
tmp = (char*)malloc(1024);
if(tmp != NULL)
count++;
else
break;
}
printf("还可供分配的内存为%d bytes,%d \n",1024*count,count);
}
*/

void CallLift()
{
     unsigned long tmp_event[4];
     tmp_event[0] = ROOM_CALL_LIFT;
     tmp_event[1] = pSystemInfo->LocalSetting.publicroom.Door[0].IP;
     tmp_event[2] = tmp_event[3]=0;
     wrthost_send_cmd(tmp_event);
}

void CallLift1()
{
     unsigned long tmp_event[4];
     tmp_event[0] = ROOM_CALL_LIFT;
     tmp_event[1] = pSystemInfo->mySysInfo.bakliftip;
     tmp_event[2] = tmp_event[3]=0;
     wrthost_send_cmd(tmp_event);
}

void SendCallEvent()
{
	unsigned long ip[4];
	unsigned long tmp_event[4];
	tmp_event[0] = ROOM_CALLCENTER;
	tmp_event[1] = tmp_event[2] = tmp_event[3]=0;
	memset(ip,0,sizeof(ip));
	if(pSystemInfo){ 
		ip[0] = pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
		ip[1] = pSystemInfo->LocalSetting.publicinfo.ManagerMinorIP;
		ip[2] = pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
		ip[3] = pSystemInfo->LocalSetting.publicinfo.CenterMinorIP;
	}
	wrthost_set_peer_room_ip(ip);
	wrthost_send_cmd(tmp_event);
}

int  CallEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        unsigned long tmp_event[4];
        int maxindex = 6;
        int firstindex = 0;
        if(pSystemInfo->mySysInfo.bakliftip == 0){
                maxindex = 6;
                firstindex = 0;
        }else{
                maxindex = 13;
                firstindex = 6;
        }
        for(index =firstindex ;index<maxindex;index++){
                if(IsInside(rc_active_call[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_active_call[index].x;
                        pos_y = rc_active_call[index].y;
                        switch(index){
                        
                case 6:
                case 0: //呼叫中心

                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,ico_hujiaozhongxin2,sizeof(ico_hujiaozhongxin2));
                        }else{
                                //unsigned long ip[4];
                                JpegDisp(pos_x,pos_y,ico_hujiaozhongxin1,sizeof(ico_hujiaozhongxin1));
                                if(pSystemInfo->mySysInfo.isbomb == USE_BOMB){
                                        CreateDialogWin2(LT("版本太低,请升级"),MB_OK,NULL,NULL);
                                        break;
                                }
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateCallCenterWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateCallCenterWin();//finish
                                SendCallEvent();//呼叫中心事件处理

                        }
                        break;
                        
                case 7:
                case 1://呼叫住户
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,ico_hujiaozhuhu2,sizeof(ico_hujiaozhuhu2));
                        }else{
                                JpegDisp(pos_x,pos_y,ico_hujiaozhuhu1,sizeof(ico_hujiaozhuhu1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateCallRoomWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateCallRoomWin();
                                //test_memory();
                        }
                        break;
                        
                case 2:
                case 8: //大堂经理
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,ico_datangjingli2,sizeof(ico_datangjingli2));
                        }else{
                                unsigned long ip[4];
                                JpegDisp(pos_x,pos_y,ico_datangjingli1,sizeof(ico_datangjingli1));
                                if(pSystemInfo->mySysInfo.isbomb == USE_BOMB){
                                        CreateDialogWin2(LT("版本太低,请升级"),MB_OK,NULL,NULL);
                                        break;
                                }
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CreateCallCenterWin);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CreateCallCenterWin();
                                SendCallEvent();

                        }                
                	break;
                	
                case 9:
                case 3:// 电梯控制
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,ico_calllift2,sizeof(ico_calllift2));
                        }else{

                                JpegDisp(pos_x,pos_y,ico_calllift1,sizeof(ico_calllift1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CallLift);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CallLift();
                                //test_memory();
                        }
                        break;
                        
                case 10://备用电梯
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,ico_calllift2,sizeof(ico_calllift2));
                        }else{

                                JpegDisp(pos_x,pos_y,ico_calllift1,sizeof(ico_calllift1));
#ifdef USE_DESKTOP
                                if(diff_up_down_time() > 3){
                                        SetCurrentHandlerCallback(CallLift1);
                                        CreateSelectDesktopPos();
                                        break;
                                }
#endif
                                CallLift1();
                                //test_memory();
                        }
                        break;
                case 11:
                case 4://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 12:
                case 5://关屏幕
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                default:
                        break;
                        }
                        break;
                }
        }
        return ret;
}

/*****************************************************************
创建呼叫中心主窗口
****************************************************************/
void CreateCallCenterWin(){
        
        g_is_mute = false;
        g_is_hangup = false;
		g_holdon_flag = 3;
		g_phone_or_externdevice_holdon =1;
		g_is_extern_callcenter = 1;
		g_ipcamera_flag = 0;

        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = CallCenterEventHandler;
        pWindow->NotifyReDraw = DrawCallCenterMain;
        pWindow->valid = 1;
        pWindow->CloseWindow = CloseCallWin;
        pWindow->type = CALLING;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawCallCenterMain(){

    if(pSystemInfo->mySysInfo.screensize == 0x10){
    	OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
    	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
    }else{
    	WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 600;
		FillRect(rgb_black,rt);

		JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
	}

    viewcallinfo(LT("正在呼叫中心..."));

    OSD_JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));//对讲
    OSD_JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo3,sizeof(menu_kaisuo3));//开锁
    OSD_JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));//抓拍
    if(g_is_kinescope)
	    OSD_JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang1,sizeof(menu_luxiang1));//录像
    else
	    OSD_JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));

    OSD_JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin4,sizeof(menu_jingyin4));//静音
    OSD_JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji1,sizeof(menu_guaji1));//挂机
    OSD_JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu3,sizeof(menu_tuichu3));//退出

#if USE_VOLUME_IN_VIDEO
    DrawVolumeSize();
#endif
}

int  CallCenterEventHandler(int x,int y,int status)
{
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<7;index++){
                if(IsInside(rc_callico[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_callico[index].x;
                        pos_y = rc_callico[index].y;
                        switch(index){
                case 0: //对讲
                        break;
                case 1://是否发送视频
                		if(g_is_hangup)
                			break;
	                	if(g_is_call_room == 0 || get_venc_type() <= V_NONE)
	                	{
	                		break;
	                	}
                		if(status == 1){ //按下
                        	OSD_JpegDisp(pos_x,pos_y,menu_novideo2,sizeof(menu_novideo2));
                        	copy_to_sencod_and_three_fbpage();
                        }else{

                        	g_send_my_video = !g_send_my_video;
                        	if(g_send_my_video)
                        	{
                        		OSD_JpegDisp(pos_x,pos_y,menu_havevideo1,sizeof(menu_havevideo1));
                        		copy_to_sencod_and_three_fbpage();
                        	}
                        	else
                        	{
                        		OSD_JpegDisp(pos_x,pos_y,menu_novideo1,sizeof(menu_novideo1));
                        		copy_to_sencod_and_three_fbpage();
                        	}
                        }
                        break;
                case 2://抓拍
                        break;
                case 3://录像

                        if(g_is_hangup)
                			break;
                        if(get_venc_type() <= V_NONE)
                			break;
                        if(status == 1){ //按下
                            JpegDisp(pos_x,pos_y,menu_luxiang2,sizeof(menu_luxiang2));
                            copy_to_sencod_and_three_fbpage();
                        }else{
                        	
                            if(g_is_rec_video == false)
							{
								long _size,fsize;
								struct statfs vbuf;
								statfs("/",&vbuf);
								_size = (vbuf.f_bsize * vbuf.f_bfree);
								fsize = (float)_size/(1024*1024);
								if(fsize < 30)
								{
									OSD_JpegDisp(pos_x,pos_y,menu_luxiang3,sizeof(menu_luxiang3));
	                                copy_to_sencod_and_three_fbpage();
	                                break;
								}else
								{
	                                OSD_JpegDisp(pos_x,pos_y,menu_luxiang4,sizeof(menu_luxiang4));
	                                copy_to_sencod_and_three_fbpage();

	                                event[0] = CUSTOM_CMD_RECORD_JPEG_FRAME;
	                                event[1] = event[2] = event[3]=0;
	                                wrthost_send_cmd(event);
	                                printf("录像264\n");
	                                g_is_rec_video = true;
								}
							}else
							{
									OSD_JpegDisp(pos_x,pos_y,menu_luxiang1,sizeof(menu_luxiang1));
	                                copy_to_sencod_and_three_fbpage();

	                                event[0] = CUSTOM_CMD_STOP_JPEG_FRAME;
	                                event[1] = event[2] = event[3]=0;
	                                wrthost_send_cmd(event);
	                                printf("停止录像264\n");
	                                g_is_rec_video = false;
		                     }
                        }
                        break;
                case 4://静音
	                	if(g_is_hangup)
	                		break;
	                	//if(g_is_mute)
                		//	break;
                        if(status == 1){ //按下
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,menu_jingyin2,sizeof(menu_jingyin2));
                                copy_to_sencod_and_three_fbpage();
                        }else{

                               if(!g_is_mute)
                                	g_is_mute = true;
                            if(g_is_mute)
                            {
                                mp3stop();
	                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin1,sizeof(menu_jingyin1));
	                        }else
	                        	OSD_JpegDisp(pos_x,pos_y,menu_jingyin4,sizeof(menu_jingyin4));
                        }
                        break;
                case 5://挂机
                        if(status == 1){ //按下
                                StartButtonVocie();
  
                                JpegDisp(pos_x,pos_y,menu_guaji2,sizeof(menu_guaji2));
                                //copy_to_sencod_and_three_fbpage();
                        }else{
                                JpegDisp(pos_x,pos_y,menu_guaji1,sizeof(menu_guaji1));
                                //copy_to_sencod_and_three_fbpage();
                                SDL_Delay(300);
                                event[0] = ROOM_HANGUP;
                                event[1] = event[2] = event[3]=0;
	                            wrthost_send_cmd(event);
                        }
                        break;
                case 6://退出
                        break;
                default:
                        break;
                        }
                        break;
                }
        }
#if USE_VOLUME_IN_VIDEO
        if(ret != 1){
                ret = HandlerVolume(x,y,status);
        }
#endif
        return ret;
}

/****************************************************************
创建呼叫分机主窗口
****************************************************************/

#if HAVEKEYBOARD	

#define ALPHABET_TABLE(x,a) \
        g_alphabetpointer[x]=(unsigned long)a##01; \
        g_alphabetsize[x] = sizeof(a##01);\
        g_alphabetpointer1[x]=(unsigned long)a##02;\
        g_alphabetsize1[x] = sizeof(a##02);\

#define NUMBER_TABLE(x,y,a) \
        g_numberpointer[x] = (unsigned long)a##y##_1;\
        g_numbersize[x] = sizeof(a##y##_1);\
        g_numberpointer1[x] = (unsigned long)a##y##_2;\
        g_numbersize1[x] = sizeof(a##y##_2);\


void InitAlphabetAndNumber(){
                if(g_isInitAlphabet == 0x01)
                        return;
                ALPHABET_TABLE(0,A)
                ALPHABET_TABLE(1,B)
                ALPHABET_TABLE(2,C)
                ALPHABET_TABLE(3,D)
                ALPHABET_TABLE(4,E)
                ALPHABET_TABLE(5,F)
                ALPHABET_TABLE(6,G)
                ALPHABET_TABLE(7,H)
                ALPHABET_TABLE(8,I)
                ALPHABET_TABLE(9,J)
                ALPHABET_TABLE(10,K)
                ALPHABET_TABLE(11,L)
                ALPHABET_TABLE(12,M)
                ALPHABET_TABLE(13,N)
                ALPHABET_TABLE(14,O)
                ALPHABET_TABLE(15,P)
                ALPHABET_TABLE(16,Q)
                ALPHABET_TABLE(17,R)
                ALPHABET_TABLE(18,S)
                ALPHABET_TABLE(19,T)
                ALPHABET_TABLE(20,U)
                ALPHABET_TABLE(21,V)
                ALPHABET_TABLE(22,W)
                ALPHABET_TABLE(23,X)
                ALPHABET_TABLE(24,Y)
                ALPHABET_TABLE(25,Z)
                ALPHABET_TABLE(26,add_auto)
                ALPHABET_TABLE(27,add_auto)
                //两种数字表指针存放地址
                NUMBER_TABLE(0,1,number)
                NUMBER_TABLE(1,2,number)
                NUMBER_TABLE(2,3,number)
                NUMBER_TABLE(3,4,number)
                NUMBER_TABLE(4,5,number)
                NUMBER_TABLE(5,6,number)
                NUMBER_TABLE(6,7,number)
                NUMBER_TABLE(7,8,number)
                NUMBER_TABLE(8,9,number)
                NUMBER_TABLE(9,11,numberxin)
                NUMBER_TABLE(10,0,number)
                NUMBER_TABLE(11,10,numberjin)
                g_isInitAlphabet = 0x01;
}
#endif

void GetNumberJpgPointer(unsigned long** pointer1,unsigned long** pointer2){
        *pointer1 = g_numberpointer;
        *pointer2 = g_numberpointer1;
}
void GetNumberJpegSizePointer(unsigned long** size1,unsigned long** size2){
        *size2 =  g_numbersize1;
        *size1 = g_numbersize;
}
void GetAlphabetJpgPointer(unsigned long** pointer1,unsigned long** pointer2){ //获得字母表
	*pointer1 = g_alphabetpointer;
	*pointer2 = g_alphabetpointer1;
}
	
void GetAlphabetJpegSizePointer(unsigned long** size1,unsigned long** size2){ //获得字母表的大小
	*size1 = g_alphabetsize;
	*size2 = g_alphabetsize1;
}
void SetAlphabet26and27Key(int keytype){
#if HAVEKEYBOARD	

	if(keytype == 0){
		ALPHABET_TABLE(26,add_auto)
                ALPHABET_TABLE(27,add_auto)
	}else{
		ALPHABET_TABLE(26,add_auto)
                ALPHABET_TABLE(27,add_auto)
	}
#endif
}

static void DrawContactsIcon(int indx,int offset)
{
	WRT_Rect tmprect[4];
	offset = 0;
	tmprect[0].x = rc_callroomico[1].x+5*100 + 2 - offset-10;
	tmprect[0].y = rc_callroomico[1].y + 3*100 -offset;
	tmprect[0].w = 88;
	tmprect[0].h = 38;
	tmprect[2] = tmprect[0];
	tmprect[2].x += 90;
	tmprect[1] = tmprect[0];
	tmprect[1].y += 33;
	tmprect[3] = tmprect[2];
	tmprect[3].x += 10;
	tmprect[3].y += 33;
	
	if(indx == 0){
		DrawTextToRect2(tmprect[0],LT("新 增"),rgb_yellow,1);//0x2f005b
		DrawTextToRect2(tmprect[1],LT("电话本"),rgb_yellow,1);//0x2f005b
	}else{
		DrawTextToRect2(tmprect[2],LT("查 看"),rgb_yellow,1);//0x2f005b
		DrawTextToRect2(tmprect[3],LT("电话本"),rgb_yellow,1);//0x2f005b
	}
}

//字母表的处理
static int alphabethandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int tempx  =0;
        int tempy = 0;
        int ret = 0;
        int count =0;
        int startx = rc_callroomico[1].x;
        int starty = rc_callroomico[1].y;
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0; i<4;i++)
                for(j=0; j< 7;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){
                                count = (i*7)+j;
                                ret  = 1;
                                if(status == 1)
                                        g_rc_status[count] = true;
                                else{
                                        if(g_rc_status[count] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count] = false;
                                }
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(tempx,tempy, (unsigned char*)g_alphabetpointer1[count],g_alphabetsize1[count]);
                                        if(count == 26 || count == 27){
                                                g_isUpdated = 0;
                                                if(count == 26)
                                                        DrawContactsIcon(0,0);
                                                else
                                                        DrawContactsIcon(1,0);
                                                update_rect(tempx,tempy,100,100);
                                                g_isUpdated = 1;
                                        }
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)g_alphabetpointer[count],g_alphabetsize[count]);
                                        if(count == 26 || count == 27){
                                                g_isUpdated = 0;
                                                if(count == 26)
                                                        DrawContactsIcon(0,0);
                                                else
                                                        DrawContactsIcon(1,0);
                                                update_rect(tempx,tempy,100,100);
                                                g_isUpdated = 1;
                                        }
                                        if(count < 26 && g_inputcount < MAX_ROOM_BITS){ //26 位增加电话本，27 查看电话本
                                                c[0] = g_alphabet[count];
                                                strcat(g_roomnumber,(char*)&c);
                                                WRT_Rect tmprect;
                                                tmprect.x = rc_callroomico[0].x;
                                                tmprect.y = rc_callroomico[0].y;
                                                tmprect.w = 720;
                                                tmprect.h = 50;
                                                DeflateRect(&tmprect,4);
                                                FillRect(rgb_white,tmprect);
                                                DrawText(g_roomnumber,rc_callroomico[0].x+20,rc_callroomico[0].y+30,rgb_black);
                                                g_inputcount++;
                                        }else if(count == 26){  //增加电话本
                                                if(strlen(g_roomnumber) == 0)
                                                        break;
                                                SDL_Delay(100);
                                                CreateDialogWin2(LT("确定要增加到地址簿?"),MB_OK|MB_CANCEL,add_telephone_1,NULL);
                                                
                                        }else if(count == 27){  //查看电话本  有BUG  要解决
                                        		SDL_Delay(100);
                                                CreateTelephoneWin();
                                                memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                g_inputcount=0;
                                        }
                                }
                                break;
                        }
                }
                return ret;
}

//0 表示不存在，否则返回房间对应的ip
unsigned long get_sub_room_ip(int num)
{
	unsigned long ip[8];
    char id[16];
    char exid[8][16];
    char tmpbuf[16];
    char text[3];
    int  count = 8;
    int  i = 0;
    memset(ip,0,sizeof(ip));
    memset(id,0,sizeof(id));
    memcpy(id,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
    memset(text,0,3);
    sprintf(text,"%02d",num);
    id[13] = text[0];
    id[14] = text[1];
    if(strncmp(id,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,15) == 0){
            return 0;
    }
    get_ipbyid(id,ip,&count);
    if(count == 0){
            return 0;
    }
    get_id2((char*)exid,sizeof(exid));
    for(i =0; i<count; i++){
            if(strncmp(id,(char*)exid[i],15) == 0){
                    ip[0] = ip[i];
                    return ip[0];
            }
    }
    return 0;
}
static void CallMinorRoom(int num){
        unsigned long ip[8];
        char id[16];
        char exid[8][16];
        char text[3];
        int count = 8;
        int i =0;
        event[0] = CUSTOM_CMD_CALL_ROOM;
        event[1] = 0;
        event[2] = 0;
        event[3] =0;
        //在上面查询IP
        memset(ip,0,sizeof(ip));
        memset(id,0,sizeof(id));
        memcpy(id,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        memset(text,0,3);
        sprintf(text,"%02d",num);
        id[13] = text[0];
        id[14] = text[1];
        WRT_DEBUG("id = %s ",id);
        if(strncmp(id,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,15) == 0){
                CreateDialogWin2(LT("不能自己呼叫自己!"),MB_OK,NULL,NULL);
                return;
        }
        get_ipbyid(id,ip,&count);
        if(count == 0){
                CreateDialogWin2(LT("无此从分机!"),MB_OK,NULL,NULL);
                return;
        }

        get_id2((char*)exid,sizeof(exid));
        for(i =0; i<count; i++){
                if(strncmp(id,(char*)exid[i],15) == 0){
                        ip[0] = ip[i];
                        ip[1] = 0;
                        ip[2] = 0;
                        ip[3] = 0;
                        //wrthost_Set_RoomCallRoom_LocalID(id);
                        wrthost_set_peer_room_ip(ip);
                        wrthost_send_cmd(event);
                        CreateCallRoomSubWin();
                        return;
                }
        }
        CreateDialogWin2(LT("无此从分机!"),MB_OK,NULL,NULL);
        return;
}


int  VideoTestEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        
        if(IsInside(rc_callico[6],xy)){
                ret = 1;
                if(status == 1)
                        g_rc_status[6] = true;
                else{
                        if(g_rc_status[6] == false){
                                ret = 0;
                                return ret;
                        }
                        g_rc_status[6] = false;
                }
                pos_x = rc_callico[6].x;
                pos_y = rc_callico[6].y;
                
                if(status == 1){
                        StartButtonVocie();

                        JpegDisp(pos_x,pos_y,menu_tuichu2,sizeof(menu_tuichu2));
                        copy_to_sencod_and_three_fbpage();
                }else{

                        JpegDisp(pos_x,pos_y,menu_tuichu1,sizeof(menu_tuichu1));
                        copy_to_sencod_and_three_fbpage();

					   Stopsendvideo();
					   SDL_Delay(1000);
					   Stopplayvideo();
					   CloseTopWindow();
					   g_send_my_video = 0;

                       // event[0] = ROOM_STOPMONT;
                       // event[1] = event[2] = event[3]=0;
                       // wrthost_send_cmd(event);

                }
        }
        return ret;
}

void DrawVideoTestMain(){

	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 600;
	FillRect(rgb_black,rt);

	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
	//DrawText(LT("正在连接中..."),110,100,rgb_white);

  
    OSD_JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));
    OSD_JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo3,sizeof(menu_kaisuo3));
    OSD_JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
    OSD_JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));
    OSD_JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin3,sizeof(menu_jingyin3));
    OSD_JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji3,sizeof(menu_guaji3));
    OSD_JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu1,sizeof(menu_tuichu1));


	

    Startplayvideo(NULL);

    SDL_Delay(1000);
    unsigned long ip[4];
	ip[1]=0x0100007F;
	g_send_my_video = 1;
	//Startsendvideo(&ip[1]);
	
}    

void CreateVideoTestWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        g_is_call_room = 1;
        pWindow->EventHandler = VideoTestEventHandler;
        pWindow->NotifyReDraw = DrawVideoTestMain;
        pWindow->valid = 1;
        pWindow->CloseWindow = CloseCallWin;
        pWindow->type = CALLING;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}

static void Videotest(){
        CreateVideoTestWin();
        return;
}

void *pingfunc(void *)
{
	printf("pingfunc\n");
	sleep(5);
	printf("kill\n");
	int ret = system("killall -9 ping");

	//printf("ret == %d\n",ret);
	//if(ret < 0 )
	//	system("echo net error > /home/wrt/cmd.txt");
}

void killpingthread()
{
	int i, j;
	pthread_t pingid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&pingid, &attr, pingfunc, NULL);
	if ( pingid <= 0 ) {
		printf("无法创建pingid处理线程\n");
		return;
	}
	pthread_attr_destroy(&attr);
}

extern int g_audio_packsize;
extern int g_button_test;
extern int g_enc_restart;

/////////////////////////////////////////////end broadcast

//数字表的处理
static int numberhandler(int x,int y,int status){ //finished
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_callroomico[2].x;
        int starty = rc_callroomico[2].y;
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0; i<4;i++)
                for(j=0; j< 3;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){//88 79
                                count = (i*3)+j;
                                ret =1;
                                if(status == 1)
                                        g_rc_status[count] = true;
                                else{
                                        if(g_rc_status[count] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count] = false;
                                }
                                if(status == 1){
                                        
                                        JpegDisp(tempx,tempy, (unsigned char*)g_numberpointer1[count],g_numbersize1[count]);
                                        StartButtonVocie();
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)g_numberpointer[count],g_numbersize[count]);
                                        if( (count != 9 && count != 11) && (g_inputcount < MAX_ROOM_BITS) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                strcat(g_roomnumber,(char*)&c);
                                                WRT_Rect tmprect;
                                                tmprect.x = rc_callroomico[0].x;
                                                tmprect.y = rc_callroomico[0].y;
                                                tmprect.w = 720;
                                                tmprect.h = 50;
                                                DeflateRect(&tmprect,4);
                                                FillRect(rgb_white,tmprect);
                                                DrawText(g_roomnumber,rc_callroomico[0].x+20,rc_callroomico[0].y+30,rgb_black);
                                                g_inputcount++;
                                        }else if(count  == 9){ //*
                                                WRT_Rect tmprect;
                                                g_roomnumber[g_inputcount-1] = '\0';
                                                tmprect.x = rc_callroomico[0].x;
                                                tmprect.y = rc_callroomico[0].y;
                                                tmprect.w = 720;
                                                tmprect.h = 50;
                                                DeflateRect(&tmprect,4);
                                                g_isUpdated = 0;
                                                FillRect(rgb_white,tmprect);
                                                DrawText(g_roomnumber,rc_callroomico[0].x+20,rc_callroomico[0].y+30,rgb_black);
                                                update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                                g_isUpdated = 1;
                                                
                                                g_inputcount--;
                                                if(g_inputcount < 0)
                                                        g_inputcount = 0;
                                        }else if(count == 11){//# //发送呼叫
                                                // printf("g_roomnumber =%s \n",g_roomnumber);
                                                if(strcmp(g_roomnumber,"ISBOMB") == 0){
                                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                        if(pSystemInfo->mySysInfo.isbomb == USE_BOMB)
                                                                pSystemInfo->mySysInfo.isbomb = 0x0;
                                                        else
                                                                pSystemInfo->mySysInfo.isbomb = USE_BOMB;
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("保存设置成功!"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }
												if(strcmp(g_roomnumber,"DISPLAY") == 0){
														CreateDisplayWin();
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														return 1;
												}

												if(strcmp(g_roomnumber,"IFCONFIG") == 0){
														system("ifconfig -a > /home/wrt/cmd.txt");
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"ROUTE") == 0){
														system("route > /home/wrt/cmd.txt");
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"PINGMYSELF") == 0){
														char sysbuf[100];
														char ipbuf[20];
														unsigned long ip =0;
										                get_host_ip(&ip);
														toinet_addr(ip,(unsigned char*)ipbuf);
														sprintf(sysbuf,"ping -c 1 %s > /home/wrt/cmd.txt",ipbuf);
														killpingthread();
														printf("sysbuf = %s\n",sysbuf);
														system(sysbuf);
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"PINGGATEWAY") == 0){
														char sysbuf[100];
														char ipbuf[20];
														unsigned long ip =0;
										                get_host_gateway(&ip);
														toinet_addr(ip,(unsigned char*)ipbuf);
														sprintf(sysbuf,"ping -c 1 %s > /home/wrt/cmd.txt",ipbuf);
														printf("sysbuf = %s\n",sysbuf);
														killpingthread();
														system(sysbuf);
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"PINGCENTER") == 0){
														char sysbuf[100];
														char ipbuf[20];
														unsigned long ip =0;
										                wrthost_get_centerip(&ip);
														toinet_addr(ip,(unsigned char*)ipbuf);
														sprintf(sysbuf,"ping -c 1 %s > /home/wrt/cmd.txt",ipbuf);
														printf("sysbuf = %s\n",sysbuf);
														killpingthread();
														system(sysbuf);
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"RESETNETDOWN") == 0){
														system("ifconfig eth0 down > /home/wrt/cmd.txt");
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"RESETNETUP") == 0){
														system("ifconfig eth0 up > /home/wrt/cmd.txt");
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														CreateDialogWin2("EXEC OK",MB_OK,NULL,NULL);
														return 1;
												}
												if(strcmp(g_roomnumber,"SYSCMD") == 0){
														CreateSysCmdWin();
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
														return 1;
												}
                                                if(pSystemInfo->mySysInfo.isbomb == USE_BOMB){
                                                        CreateDialogWin2(LT("版本太低,请升级"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }

                                                if(strcmp(g_roomnumber,"RULE") == 0){
                                                        CreateCallRuleWin();
                                                        return 1;
                                                }
                                                /*呼叫从分机*/
                                                if(strcmp(g_roomnumber,"R1") == 0){
                                                        CallMinorRoom(1);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                
                                                if(strcmp(g_roomnumber,"R2") == 0){
                                                        //表示想呼叫第二个分机
                                                        CallMinorRoom(2);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"R3") == 0){
                                                        //表示想呼叫第三个分机
                                                        CallMinorRoom(3);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"R4") == 0){
                                                        //表示想呼叫第四个分机
                                                        CallMinorRoom(4);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                #if 0
                                                if(strcmp(g_roomnumber,"VIDEOTEST") == 0)
                                                {
                                                	CreateCameraWin();
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;
                                                    return 1;
                                                }
												#endif
                                                if(strncmp(g_roomnumber,"CHEFANGCNT",10) == 0)
                                                {
                                                	char tmpcnt[4];
													memcpy(tmpcnt,g_roomnumber+10,g_inputcount-10);

													
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;

													CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                   
                                                    pSystemInfo->mySysInfo.chengfangcnt = atoi(tmpcnt);
                                                    UpdateSystemInfo();  
                                                    CloseWindow();  
													
													CreateDialogWin2(LT("保存设置成功!"),MB_OK,NULL,NULL);
                                                    return 1;
                                                }

												if(strncmp(g_roomnumber,"CHEFANGTIME",11) == 0)
                                                {
                                                	char tmpcnt[4];
													memcpy(tmpcnt,g_roomnumber+11,g_inputcount-11);

													
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;

													CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                   
                                                    pSystemInfo->mySysInfo.chengfangtime = atoi(tmpcnt);
                                                    UpdateSystemInfo();  
                                                    CloseWindow();  
													CreateDialogWin2(LT("保存设置成功!"),MB_OK,NULL,NULL);
                                                    return 1;
                                                }
                                                
                                                if(strcmp(g_roomnumber,"LOG1") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        wrt_set_log_level_mask(LOG_MESSAGE);
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"LOG2") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        wrt_set_log_level_mask(LOG_MESSAGE|LOG_DEBUG);
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"LOG3") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        wrt_set_log_level_mask(LOG_MESSAGE|LOG_DEBUG|LOG_WARNING);
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"LOG4") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        wrt_set_log_level_mask(LOG_MESSAGE|LOG_DEBUG|LOG_WARNING|LOG_ERROR);
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"AUDIO1") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_audio_packsize = 1;
                                                    return 1;
                                                }
                                                if(strcmp(g_roomnumber,"AUDIO2") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_audio_packsize = 2;
                                                    return 1;
                                                }
                                                if(strcmp(g_roomnumber,"AUDIO3") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_audio_packsize = 3;
                                                    return 1;
                                                }
                                                if(strcmp(g_roomnumber,"BUTTON") == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_button_test = !g_button_test;
                                                    return 1;
                                                }
                                                if(strcmp(g_roomnumber,"SCREENSIZE7") == 0)
                                                {
                                                	CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                    memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;
                                                    pSystemInfo->mySysInfo.screensize = 0x7;
                                                    UpdateSystemInfo();         
                                                    CloseWindow();                                       	
                                                	CreateDialogWin2(LT("设置显示屏7寸屏"),MB_OK,NULL,NULL);
                                                	return 1;
                                                	
                                                }
                                                if(strcmp(g_roomnumber,"SCREENSIZE10") == 0)
                                                {
                                                	CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                    memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;
                                                    pSystemInfo->mySysInfo.screensize = 0x10;
                                                    UpdateSystemInfo();  
                                                    CloseWindow();                                              	
                                                	CreateDialogWin2(LT("设置显示屏10寸屏"),MB_OK,NULL,NULL);    
                                                	return 1;                                            	
                                                }
                                                #if 0
                                                if(strcmp(g_roomnumber,"ENC2015") == 0)
                                                {
                                                	CreateDialogWin2("Testing...",MB_NONE,NULL,NULL);
                                                    memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;

													wrthost_syssetbusy();

                                                    Encode_Test_Start();
                                                    
                                                    SDL_Delay(1000);
                                                    g_send_my_video = 1;
                                                    SDL_Delay(5000);
                                                    
                                                    Encode_Test_Stop();
                                                    wrthost_syssetidle();
                                                    CloseWindow();
                                                    if(g_enc_restart >= 10)
                                                		CreateDialogWin2("OK!",MB_OK,NULL,NULL);
                                                	else
                                                		CreateDialogWin2("ERROR!",MB_OK,NULL,NULL);
                                                	return 1;                                            	
                                                }
                                                #endif
                                                if(strcmp(g_roomnumber,"HAVEDEBUG" ) == 0){
	                                                    memset(g_roomnumber,0,MAX_ROOM_BITS+1);
	                                                    g_inputcount=0;
	                                                    if(wrt_get_log_remote() == 0){
	                                                    	wrt_set_log_level_mask(LOG_MESSAGE);
	                                                    	wrt_set_log_remote(1);
	                                                            CreateDialogWin2(LT("进入调试模式"),MB_OK,NULL,NULL);
	                                                    }else{
	                                                    	wrt_set_log_level_mask(LOG_END);
	                                                    	wrt_set_log_remote(0);
	                                                            CreateDialogWin2(LT("进入正常模式"),MB_OK,NULL,NULL);
	                                                    }
	                                                    return 1;
                                                }
                                                if(strcmp(g_roomnumber,"HAVESCREEN") == 0){
                                                        if(pSystemInfo->mySysInfo.iskeepscreen == 0){
                                                                pSystemInfo->mySysInfo.iskeepscreen =1;
                                                                screenkeep_init();
                                                                screenkeep_start();
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                //save_localhost_config_to_flash();
                                                                UpdateSystemInfo();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("启动屏幕保护"),MB_OK,NULL,NULL);

                                                        }else{
                                                                pSystemInfo->mySysInfo.iskeepscreen =0;
                                                                screenkeep_stop();
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                
                                                                UpdateSystemInfo();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("取消屏幕保护"),MB_OK,NULL,NULL);
                                                        }

                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"USECNTNEGO") == 0){
                                                        if(pSystemInfo->mySysInfo.usecenternegotiate == 0){
                                                                pSystemInfo->mySysInfo.usecenternegotiate =1;
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                UpdateSystemInfo();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("启动中心协商"),MB_OK,NULL,NULL);
                                                        }else{
                                                                pSystemInfo->mySysInfo.usecenternegotiate =0;
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                UpdateSystemInfo();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("取消中心协商"),MB_OK,NULL,NULL);
                                                        }
														wrti_SetCallerCntNego(pSystemInfo->mySysInfo.usecenternegotiate);

                                                        return 1;
                                                }
												if(strcmp(g_roomnumber,"20141") == 0){
														CreateIPCameraCfgWin();
														memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
                                                    return 1;
                                                }

												if(strcmp(g_roomnumber,"20142") == 0){
														CreatetIPCameraWin();
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
														g_inputcount=0;
                                                    return 1;
                                                }
                                                
                                                if(strcmp(g_roomnumber,"SHOW1") == 0){
                                                        test_alarm_for_factory();
                                                    return 1;
                                                }
                                                
                                                if(strcmp(g_roomnumber,"SHOW2") == 0){
                                                        CreateDialogWin2(LT("正在清除数据，并恢复默认设置"),MB_NONE,NULL,NULL);
                                                        SetDefaultConfig(pSystemInfo);
                                                        SDL_Delay(2000);
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("清除，并恢复默认设置成功"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }
                                                
                                                if(strcmp(g_roomnumber,"SHOW3") == 0){
                                                	CreateSampleTestWin();
                                                    memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;
                                                    return 1;
                                                }
                                                if(strcmp(g_roomnumber,"SETUP2014") == 0){
                                                	CreateIoLevelSetWin();
                                                    memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                    g_inputcount=0;
                                                    return 1;
                                                }
                                                if(strncmp(g_roomnumber,"QUALITY",7) == 0){
                                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                        int quailty = atoi((char*)(g_roomnumber+7));
                                                        SetPhotoQuality(quailty);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("设置抓拍图片质量成功！"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }
                                                
                                                if(strncmp(g_roomnumber,"ISCAP",5) == 0){
                                                	memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        g_iscapphoto = !g_iscapphoto;
                                                        return 1;
                                                }

                                                if(strncmp(g_roomnumber,"FRAME",5) == 0){
                                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                        int rate = atoi((char*)(g_roomnumber+5));
                                                        if(rate > 10 || rate < 5)
                                                                rate = 5;
                                                        pSystemInfo->mySysInfo.framerate = rate;
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("设置录像帧率成功！"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }
                                                if(strncmp(g_roomnumber,"MAXPHOTO",8) == 0){
                                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                        int quailty = atoi((char*)(g_roomnumber+8));
                                                        Set_Max_photo(quailty);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("设置抓拍图片最大数量成功！"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }
                                                if(strncmp(g_roomnumber,"HASSIP",6) == 0){
                                                        if(pSystemInfo->mySysInfo.hassip == 0x01){
                                                                pSystemInfo->mySysInfo.hassip = 0x0;
                                                        }else
                                                                pSystemInfo->mySysInfo.hassip = 0x01;
                                                        CreateDialogWin2(LT("正在启动/停止SIP服务..."),MB_NONE,NULL,NULL);   
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("启动/停止SIP成功！"),MB_OK,NULL,NULL);                                
                                                        return 1;
                                                }
                                                if(strncmp(g_roomnumber,"G711",4) == 0)
                                                {
                                                        int ret = SetSipCallParam();
                                                        if(ret == 1){
                                                                CreateDialogWin2(LT("设置音频参数G711,16K,20ms！"),MB_OK,NULL,NULL); 
                                                        }else
                                                                CreateDialogWin2(LT("设置音频参数G711,8K,40ms！"),MB_OK,NULL,NULL); 
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;

                                                }
                                                if(strncmp(g_roomnumber,"MEM",3) == 0){
                                                        zenMemUninit();
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(strncmp(g_roomnumber,"ISBELL",6) == 0)
                                                {
                                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                        pSystemInfo->mySysInfo.isdoorbell = !pSystemInfo->mySysInfo.isdoorbell;
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        UpdateSystemInfo(); //保存设置成功!
                                                        CloseWindow();
                                                        CreateDialogWin2(LT("保存设置成功!"),MB_OK,NULL,NULL); 
                                                        return 1;
                                                }
                                                if(pSystemInfo->mySysInfo.hassip && (strncmp(g_roomnumber,"SIP",3) == 0 )){
                                                        CreateCallCenterWin();
                                                        unsigned long tmpevent[4] ={0};
                                                        tmpevent[0] = SIP_CALL;
                                                        tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                                                        wrthost_send_cmd(tmpevent);
                                                        return 1;
                                                }
                                                /*
                                                if(strncmp(g_roomnumber,"ID",2)== 0){
                                                CreateDialogWin("正在修改房间号...",-1);
                                                memcpy(pSystemInfo->LocalSetting.privateinfo.LocalID,g_roomnumber+2,15);
                                                //setRoomCfg(&roomtest.LocalRoom);
                                                memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                CloseWindow();
                                                CreateDialogWin("修改房间ID号成功！",0);
                                                g_inputcount=0;
                                                return 1;
                                                }*/
                                                
                                                if(strcmp(g_roomnumber,"28") == 0){
                                                        // CreateNetConfigWin();
                                                        CreateModifyIPWin(0);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
												if(strcmp(g_roomnumber,"38") == 0){
                                                        CreateWifiCfgWin();
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"29") == 0){
                                                        unsigned long ip=0;
                                                        wrthost_get_doorip(&ip);
                                                        net_sence_task(ip);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"30") == 0){
                                                        unsigned long ip=0;
                                                        wrthost_get_centerip(&ip);
                                                        net_sence_task(ip);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;

                                                }
                                                if(strcmp(g_roomnumber,"31") == 0){
                                                        unsigned long ip=0;
                                                        wrthost_get_peer_room_ip(&ip);
                                                        net_sence_task(ip);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;

                                                }
                                                if(strcmp(g_roomnumber,"32") == 0){
                                                        CreateModifyIPWin(1);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }

                                                if(strcmp(g_roomnumber,"IPCCFG") == 0){
                                                        CreateIPCameraCfgWin();
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(pSystemInfo->mySysInfo.isusedffs == 1)	{
                                                        if(strcmp(g_roomnumber,"FORMAT") == 0){
                                                                memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                                g_inputcount=0;
                                                            //    CreateDialogWin2(LT("确定要格式化Flash?"),MB_OK|MB_CANCEL,format_my_ffs,NULL);
                                                                return 1;
                                                        }
                                                }

                                                if(strcmp(g_roomnumber,"RESET2008") == 0){
                                                        Reset_sys();
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        return 1;
                                                }
                                                if(strcmp(g_roomnumber,"NOJIAJU2008") == 0){
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        g_is_have_jiaju = !g_is_have_jiaju;
                                                        pSystemInfo->mySysInfo.isjiaju = g_is_have_jiaju;
                                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                        //save_localhost_config_to_flash();
                                                        UpdateSystemInfo();
                                                        CloseWindow();
                                                        if(g_is_have_jiaju)
                                                                CreateDialogWin2(LT("设置增加智联模块"),MB_OK,NULL,NULL);
                                                        else
                                                                CreateDialogWin2(LT("设置取消智联模块"),MB_OK,NULL,NULL);
                                                        return 1;
                                                }
                                                /* if(g_inputcount < 9){
                                                CreateDialogWin("房间号错误，再次输入?",SET_ROOM_ERROR_TISHI);
                                                return 1;
                                                }*/
                                                /* char* tmp = (char*)ZENMALLOC(MAX_ROOM_BITS+1);
                                                memset(tmp,0,MAX_ROOM_BITS);
                                                // memcpy(tmp,g_roomnumber,MAX_ROOM_BITS);
                                                strcpy(tmp,(char*)g_roomnumber);
                                                event[0] = CUSTOM_CMD_CALL_ROOM;
                                                event[1] = (unsigned long)tmp;
                                                event[2] = strlen(tmp);
                                                event[3] =0;
                                                wrthost_send_cmd(event);
                                                memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                g_inputcount=0;
                                                CreateCallRoomSubWin(); */

                                                {
                                                        unsigned long ip[4];
                                                        int num =4;
                                                        char id[4][16];
                                                        event[0] = CUSTOM_CMD_CALL_ROOM;
                                                        event[1] = 0;
                                                        event[2] = 0;
                                                        event[3] = 0;
                                                        //在上面查询IP
                                                        memset(ip,0,sizeof(ip));
                                                        parse_inputcallrule(g_roomnumber);
                                                        if(ismanager(g_roomnumber))
                                                        {
                                                                CreateDialogWin2(LT("该房号不存在!"),MB_OK,NULL,NULL);                                                                 

                                                                return 1;
                                                        }
                                                        if(isdoor(g_roomnumber))
                                                        {
                                                                CreateDialogWin2(LT("该房号不存在!"),MB_OK,NULL,NULL);                                                                 

                                                                return 1;
                                                        }
                                                        if(iswall(g_roomnumber))
                                                        {
                                                                CreateDialogWin2(LT("该房号不存在!"),MB_OK,NULL,NULL);                                                                 

                                                                return 1;
                                                        }
                                                        if(strncmp(g_roomnumber,(char*)pSystemInfo->LocalSetting.privateinfo.LocalID,13) == 0){
                                                        	CreateDialogWin2(LT("不能自己呼叫自己!"),MB_OK,NULL,NULL);
                                                        	return 1;
                                                        }

                                                        get_ipbyid(g_roomnumber,ip,&num);
                                                        if(num == 0){
                                                                CreateDialogWin2(LT("该房号不存在!"),MB_OK,NULL,NULL);
                                                                return 1;
                                                        }
                                                        for(int i =0; i<4;i++){
                                                                if(ip[i] == pSystemInfo->LocalSetting.privateinfo.LocalIP)
                                                                        ip[i] = 0;
                                                        }
                                                        if(ip[0] == ip[1]&& ip[1] == ip[2] && ip[2] == ip[3] && ip[3] == 0){
                                                                CreateDialogWin2(LT("呼叫的地址不对"),MB_OK,NULL,NULL);
                                                                return 1;
                                                        }
                                                        
                                                        memset(id,0,sizeof(id));
                                                        get_id2((char*)id,sizeof(id));
                                                        isDoorStation((char*)id,ip,num); //屏蔽掉数字小门口机
                                                        for(i = 0; i<num;i++)
                                                        	WRT_DEBUG("查找到ID和IP(%s : %d)",id[i],ip[i]);
                                                        wrthost_set_peer_room_ip(ip);
                                                        wrthost_send_cmd(event);
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
                                                        g_inputcount=0;
                                                        CreateCallRoomSubWin();
                                                        return 1;
                                                }

                                        }
                                }
                                break;
                        }
                }
                return ret;
}

void CreateCallRoomWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = CallRoomEventHandler;
        pWindow->NotifyReDraw = DrawCallRoomMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);

        ALPHABET_TABLE(26,add_auto)
        ALPHABET_TABLE(27,add_auto)

        pWindow->NotifyReDraw2(pWindow);
}
void DrawCallRoomMain(){

        WRT_Rect tmprect;
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        JpegDisp(rc_callroomico[3].x,rc_callroomico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_callroomico[4].x,rc_callroomico[4].y,bt_guanping1,sizeof(bt_guanping1));
        int i =0;
        int j =0;
        
        int startx = rc_callroomico[1].x;
        int starty = rc_callroomico[1].y;

        for(i = 0;i<4;i++){ //左28个字母button
	        for(j =0; j<7;j++){
        		JpegDisp(startx+(j)*100,starty+(i)*100, (unsigned char*)g_alphabetpointer[(i*7)+j],g_alphabetsize[(i*7)+j]);
	        }
        //SDL_Delay(10);
        }
        
        startx = rc_callroomico[2].x;
        starty = rc_callroomico[2].y;
        for(i=0; i<4;i++)//又12个button
	        for(j=0; j<3;j++){
        		JpegDisp(startx+(j)*100,starty+(i)*100, (unsigned char*)g_numberpointer[(i*3)+j],g_numbersize[(i*3)+j]);
        }
        tmprect.x = rc_callroomico[0].x;
        tmprect.y = rc_callroomico[0].y;
        tmprect.w = 720;
        tmprect.h = 50;
        DrawRect(tmprect,2,rgb_white);
        DeflateRect(&tmprect,4);
        DrawRect(tmprect,2,rgb_red);
        FillRect(rgb_white,tmprect);
        //JpegDisp(rc_callroomico[0].x,rc_callroomico[0].y,dianhua_input,sizeof(dianhua_input));
        DrawText(LT("请输入房号"),10,rc_callroomico[0].y+32,rgb_white);
        //{160,50,720,50},
        //JpegDisp(0,0,huhutong_back,sizeof(huhutong_back));
   
   /*
        if(GetCurrentLanguage()== ENGLISH)
        {
                WRT_Rect rt;
                int xoffset =0;
                rt.x = 0;
                rt.y = 424;
                rt.w = 800;
                rt.h = 56;
             //   FillRect(0xdedede,rt);
                rt.x =15;
                rt.y = 50;
                rt.w = 139;
                rt.h = 69;
            //    FillRect(BACK_COLOR,rt);
                xoffset = (rt.w - GetTextWidth(LT("请输入房号"),0))/2;
                DrawText(LT("请输入房号"),rt.x + xoffset, rt.y+rt.h - 40,rgb_white);
                JpegDisp(rc_callroomico[3].x,rc_callroomico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
                JpegDisp(rc_callroomico[4].x,rc_callroomico[4].y,bt_guanping1,sizeof(bt_guanping1));

                int startx = rc_callroomico[1].x;
                int starty = rc_callroomico[1].y;
                startx += 5*100;
                starty += 3*100;
                JpegDisp(startx,starty,add_dianhua_en01,sizeof(add_dianhua_en01));
                startx = rc_callroomico[1].x + 6*100;
               // JpegDisp(startx,starty,view_dianhua_en1,sizeof(view_dianhua_en1));

        }
*/
        DrawContactsIcon(0,0);
        DrawContactsIcon(1,0);
        WRT_Rect curbox;
        char tmptext[100];
        sprintf(tmptext,"%s:",LT("呼叫格式"));
        int itmp =0;
	    if( pSystemInfo->mySysInfo.idrule[0] == 0 &&
            pSystemInfo->mySysInfo.idrule[1] == 0 &&
            pSystemInfo->mySysInfo.idrule[2] == 0 &&
            pSystemInfo->mySysInfo.idrule[3] == 0 &&
            pSystemInfo->mySysInfo.idrule[4] == 0 )
        {
                strcat(tmptext,LT("请输入13位房间编号"));
        }else{
                char ctmp[16];
                for(i=0; i< 5;i++){
                        itmp = pSystemInfo->mySysInfo.idrule[i];
                        memset(ctmp,0,16);
                        switch(i){
               case 0:
                       if(itmp != 0){
                               sprintf(ctmp,"区号%d位-",itmp);
                               if(GetCurrentLanguage() == ENGLISH)
                                       sprintf(ctmp,"G(%d)-",itmp);

                       }
                       break;
               case 1:
                       if(itmp != 0){
                               sprintf(ctmp,"栋号%d位-",itmp);
                               if(GetCurrentLanguage() == ENGLISH)
                                       sprintf(ctmp,"B(%d)-",itmp);
                       }
                       break;
               case 2:
                       if(itmp != 0){
                               sprintf(ctmp,"单元号%d位-",itmp);
                               if(GetCurrentLanguage() == ENGLISH)
                                       sprintf(ctmp,"U(%d)-",itmp);
                       }
                       break;
               case 3:
                       if(itmp != 0){
                               sprintf(ctmp,"层号%d位-",itmp);
                               if(GetCurrentLanguage() == ENGLISH)
                                       sprintf(ctmp,"F(%d)-",itmp);
                       }
                       break;
               case 4:
                       if(itmp != 0){
                               sprintf(ctmp,"房号%d位",itmp);
                               if(GetCurrentLanguage() == ENGLISH)
                                       sprintf(ctmp,"R(%d)",itmp);
                       }
                       break;
                        }
                        strcat(tmptext,ctmp);
                }
        }
        /*
        sprintf(tmptext,"呼叫格式:区号(%d位)栋号(%d位)层号(%d位)单元号(%d位)房间号(%d位)",pSystemInfo->mySysInfo.idrule[0],
        pSystemInfo->mySysInfo.idrule[1],
        pSystemInfo->mySysInfo.idrule[2],
        pSystemInfo->mySysInfo.idrule[3],
        pSystemInfo->mySysInfo.idrule[4]);*/
        curbox.x = 135;
        curbox.y = 531;
        curbox.w = 578;
        curbox.h = 55;
//#ifdef THREE_UI
	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4)); 
//#else        
      //  FillRect(0xdedede,curbox); 	  
//#endif           
        DrawText_16(tmptext,curbox.x,curbox.y+24,rgb_black);
        DrawText_16(LT("按'#'呼叫,按'*'退格!"),curbox.x,curbox.y+48,rgb_black);
        memset(g_roomnumber,0,MAX_ROOM_BITS+1);
        g_inputcount = 0;
}


int  CallRoomEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<5;index++){
                if(IsInside(rc_callroomico[index],xy)){
                        pos_x = rc_callroomico[index].x;
                        pos_y = rc_callroomico[index].y;
                        ret = 1;
                        switch(index){
                  case 0:
                          break;
                  case 3: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                  /*if(g_inputcount > 10){
                                  tmp_event[0]=CUSTOM_CMD_CANCEL;
                                  tmp_event[1] = tmp_event[2] =tmp_event[3] =0;
                                  wrthost_send_cmd(tmp_event);
                                  }*///2010-4-8 9:32:41
                                  CloseTopWindow();
                          }
                          break;
                  case 4:/*关屏*/
                          if(status == 1){ //按下
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                  /* if(g_inputcount > 10){
                                  tmp_event[0] = CUSTOM_CMD_CANCEL;
                                  tmp_event[1] = tmp_event[2] = tmp_event[3] = 0;
                                  wrthost_send_cmd(tmp_event);
                                  }
                                  *///2010-4-8 9:32:28
                                  
                                  CloseAllWindows();
                                  screen_close();
                          }
                          break;
                  case 1:
                          //进入字母区域
                          ret = alphabethandler(x,y,status);
                          break;
                  case 2:
                          //进入数字区域
                          ret = numberhandler(x,y,status);
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }
        return ret;
}

/****************************************************************
创建呼叫分机子窗口
****************************************************************/
void CreateCallRoomSubWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        g_is_call_room = 1;
        g_is_hangup = false;
        g_holdon_flag = 3;
        g_phone_or_externdevice_holdon =1;
        g_is_rec_video = false;
        g_ipcamera_flag = 0;
        g_is_inner_callroom = 1;
        pWindow->EventHandler = CallRoomSubEventHandler;
        pWindow->NotifyReDraw = DrawCallRoomSubMain;
        pWindow->valid = 1;
        pWindow->CloseWindow = CloseCallWin;
        pWindow->type = CALLING;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}
#if 0
extern int g_camera;
extern int CreateStartCameraThread(void *arg);

void DrawCameraMain(){
        
		if(pSystemInfo->mySysInfo.screensize == 0x10){
			WRT_Rect rt;
			rt.x = 0;
			rt.y = 0;
			rt.w =1024;
			rt.h = 600;
			FillRect(rgb_black,rt);
			JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
		}else{
			WRT_Rect rt;
			rt.x = 0;
			rt.y = 0;
			rt.w =1024;
			rt.h = 600;
			FillRect(rgb_black,rt);
		
			JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
			
		}

        JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));
        JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_novideo1,sizeof(menu_novideo1));
        JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
       
        JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang1,sizeof(menu_luxiang1));
 
        JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin4,sizeof(menu_jingyin4));
        JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji1,sizeof(menu_guaji1));
        JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu3,sizeof(menu_tuichu3));
        
}

int  CameraEventHandler(int x,int y,int status)
{
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<7;index++){
                if(IsInside(rc_callico[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_callico[index].x;
                        pos_y = rc_callico[index].y;
	                switch(index){
		                case 0: //对讲
		                        break;
		                case 1://是否发送视频
		                		
		                        break;
		                case 2://抓拍
		                        break;
		                case 3://录像
		                        break;
		                case 4://静音
		                        break;
		                case 5://挂机
		                        if(status == 1){ //按下
		                                StartButtonVocie();
		  
		                                JpegDisp(pos_x,pos_y,menu_guaji2,sizeof(menu_guaji2));
		                                //copy_to_sencod_and_three_fbpage();
		                        }else{
		                                JpegDisp(pos_x,pos_y,menu_guaji1,sizeof(menu_guaji1));
		                                //copy_to_sencod_and_three_fbpage();
		                                SDL_Delay(300);
		                                g_camera = 0;
		                                SDL_Delay(1000);
		                                wrthost_syssetidle();
		                                CloseTopWindow();
		                        }
		                        break;
		                case 6://退出
		                        break;
		                default:
		                        break;
		                        }
                        break;
                }
        }
#if USE_VOLUME_IN_VIDEO
        if(ret != 1){
                ret = HandlerVolume(x,y,status);
        }
#endif
        return ret;
}

void CreateCameraWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        wrthost_syssetbusy();
		SDL_CreateThread(CreateStartCameraThread,NULL);
		SDL_Delay(1000);

        pWindow->EventHandler = CameraEventHandler;
        pWindow->NotifyReDraw = DrawCameraMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}

#endif

void DrawCallRoomSubMain(){
        //FillRect(0xb2136,g_rc); //2009-7-28
        
		if(pSystemInfo->mySysInfo.screensize == 0x10){
			OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
			JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
			//DrawText(LT("正在连接中..."),110,100,rgb_white);
		}else{
			WRT_Rect rt;
			rt.x = 0;
			rt.y = 0;
			rt.w =1024;
			rt.h = 600;
			FillRect(rgb_black,rt);
		
			JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
			//DrawText(LT("正在连接中..."),110,100,rgb_white);
		}

       // JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
       // DrawText(LT("正在呼叫对方..."),110,100,rgb_white);
       viewcallinfo(LT("正在呼叫对方..."));

        JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));
        JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_novideo1,sizeof(menu_novideo1));
        JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
        //if(g_is_kinescope)
                JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang1,sizeof(menu_luxiang1));
        //else
        //        JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));

        JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin4,sizeof(menu_jingyin4));
        JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji1,sizeof(menu_guaji1));
        JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu3,sizeof(menu_tuichu3));
        
#if USE_VOLUME_IN_VIDEO
        DrawVolumeSize(); //话通话时音量调整按钮
#endif
}

int  CallRoomSubEventHandler(int x,int y,int status){
        return CallCenterEventHandler(x,y, status);
}
/////////////////////////////////////////////////////////////////
//
//
//++++++++++++++++++++2014 2 20+++++++++++++++++++++++++++++++++++++++++
//监控配置界面


static const WRT_Rect rc_monitormenuico[7] =
{
        {233,175,132,160},              // 监视点1
        {446,175,132,160},            // 监视点2
        {659,175,132,160},            // 监视点3
        /*
        {32,316,104,388},             // 监视点 4
        {364,70,436,142},             // 监视点5
        {364,152,436,224},             // 监视点6
        {364,234,436,306},             // 监视点7
        {364,316,436,388},            // 监视点8
        */
        {660,531,111,69},           /*上翻*/
        {780,531,111,69},            /* 下翻*/

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

typedef struct _tagTEMP_T_DEV_ITEM
{
        int          type ;                    //用来区分是小门口，还是网络设备
        T_DEV_ITEM   monitorDev;
}TEMP_T_DEV_ITEM;

static int                 m_monitor_page=0;
static int                 m_cur_monitor_page=0;
static int                 m_monitor_totalcount=0;
static TEMP_T_DEV_ITEM*           m_MonitorDev = NULL;
static unsigned long* pIPNumberPointer=NULL;
static unsigned long* pIPNumberPointer1=NULL;
static unsigned long* pIPNumberSize=NULL;
static unsigned long* pIPNumberSize1=NULL;
static unsigned long* pNumberPointer=NULL;
static unsigned long* pNumberPointer1=NULL;
static unsigned long* pNumberSize=NULL;
static unsigned long* pNumberSize1=NULL;

static int havePage2 = 0;
static int currentPage = 1;
static const WRT_Rect rc_ipcip[22]=
{
	{150,80,210,50},//
	{150,150,210,50},//
	{150,220,210,50},//
	{150,290,210,50},//
	{150,360,210,50},//
	{150,430,210,50},//
	
	{400,80,210,50},//
	{400,150,210,50},//
	{400,220,210,50},//
	{400,290,210,50},//
	{400,360,210,50},//
	{400,430,210,50},//

	{650,80,210,50},//
	{650,150,210,50},//
	{650,220,210,50},//
	{650,290,210,50},//
	{650,360,210,50},//
	{650,430,210,50},//

	{660,531,111,69},           /*上翻*/
    {780,531,111,69},            /* 下翻*/
        
	{900,531,111,69},			   //返回
	{6,531,111,69}, 				//关屏幕
};

static const WRT_Rect rc_ipcamera[12]={
    {220,50,100,50},//增加设备号输入框
    {220,120,280,50},//增加ip输入狂
    {220,430 ,100,50},//删除设备号
    
    {390,320,111,69},//增加按键
    {390,420,111,69},//删除按键
    
    {650,100,300,400},//pwd area
    
    {900,531,111,69},//返回
	{6,531,111,69},//关屏幕
	{530,20,69,69},//显示配置
	{220,190,280,50},//onvif认证账号
	{220,260,280,50},//onvif认证密码
	{280,330,48,48}//onvif是否认证
};

typedef struct _ipcamera_Dev
{
	int	seqnumber;//最大支持12
	int	shownumber;//显示设备号
	unsigned long ip;
	int	onvif_auth_flag;
	char onvif_username[16];
	char onvif_passwd[16];
	int rtsp_auth;
	char rtsp_username[16];
	char rtsp_passwd[16];
}IPCAMERA_DEV;

static int                 m_ipcamera_page=0;
static int                 m_cur_ipcamera_page=0;
static int                 m_ipcamera_totalcount=0;

extern int  g_cur_wifi_cfg_index;

static IPCAMERA_DEV  m_ipcameraDev[12];
static char g_adddeviceid[4];
static int  g_addidcount = 0;

static char g_deldeviceid[4];
static int  g_delidcount = 0;

static char g_adddeviceip[17];
static int  g_addipcount = 0;
static int  g_input_select = 0;

char g_Onvif_Authname[16];
int  g_Onvif_Authname_count = 0;

char g_Onvif_Authpasswd[16];
int  g_Onvif_Authpasswd_count = 0;


static int m_Onvif_Auth_flag = 0;

#define ADD_ID	1
#define ADD_IP  2
#define DEL_ID  3
#define ONVIF_AUTHNAME 4
#define ONVIF_AUTHPASSWD 5

typedef struct  _LocalLanguageRes2{
        char* ctext;
        char* etext;
}LocalLanguageRes2;

static LocalLanguageRes2 language[]=
{
	{"新增监控ID:","Add Ipc ID:"},
	{"新增监控IP:","Add Ipc IP:"},
	{"删除监控ID:","Del Ipc ID:"}
};

static LocalLanguageRes2 language2[]=
{
	{"Onvif账号:","Onvif account:"},
	{"Onvif密码:","Onvif password:"},
	{"启用Onvif认证?","Enable onvif auth?"},
};

static char ipcameracfg_path[]="/home/wrt/ipcameracfg";

static void restore_ipcameracfg()
{
	FILE* cfg_fd = NULL;

	if((cfg_fd = fopen(ipcameracfg_path, "wb")) == NULL)
        printf("无法创建配置文件\n");
    else
    {
		memset(m_ipcameraDev,0,sizeof(m_ipcameraDev));
	    for(int i = 0;i<12;i++)
	    {
	    	m_ipcameraDev[i].seqnumber = i;
	    	m_ipcameraDev[i].shownumber = 0;
	    	m_ipcameraDev[i].ip = 0;
	    	m_ipcameraDev[i].onvif_auth_flag= 0;
	    }
	    int ret = fwrite(m_ipcameraDev,1,sizeof(m_ipcameraDev),cfg_fd);
	    if(ret != sizeof(m_ipcameraDev))
	    {
	    	printf("无法创建配置文件\n");
	    	fclose(cfg_fd);
	    	return;
	    }
	    fclose(cfg_fd);
    }
}

static void save_ipcameracfg()
{
	FILE *cfg_fd;
	int ret =0;
	if((cfg_fd = fopen(ipcameracfg_path, "wb")) == NULL)
		printf("无法创建配置文件\n");
	else
	{
		for(int i = 0;i<12;i++)
		{
			printf("%d:%d \n",i,m_ipcameraDev[i].shownumber);
		}
		char* p = (char*)&m_ipcameraDev;
		int totallen = sizeof(m_ipcameraDev);
		
		int writelen = 0;
		int wl = 0;
		while(1){
			wl = fwrite( p+writelen,1, totallen - writelen , cfg_fd);
			if(ferror(cfg_fd)){
				//printf("写配置文件错误:%x\n",errno);
				break;
			}
			writelen += wl;
			
			if(writelen  == totallen)
				break;
		}
		fflush(cfg_fd);

		fclose(cfg_fd);
		cfg_fd = NULL;
		system("sync");
	}
}

void Init_ipcamera_config()
{
	FILE* cfg_fd = NULL;

	if((cfg_fd=fopen(ipcameracfg_path, "rb"))==NULL)
	{
		printf("监控配置文件不存在，创建新文件\n");
		restore_ipcameracfg();
		return;
	}
	else
	{
		int bytes_read;
		int totalen = sizeof(m_ipcameraDev);
		int readlen = 0;
		char* p = (char*)&m_ipcameraDev;
		fseek(cfg_fd,0,SEEK_SET);
		fseek(cfg_fd,0,SEEK_END);
		if(ftell(cfg_fd) != totalen){
			printf("阅读配置文件出错,文件长度不正确\n");
			fclose(cfg_fd);
			cfg_fd = NULL;
			unlink(ipcameracfg_path);

			restore_ipcameracfg();
			return;
		}
		fseek(cfg_fd,0,SEEK_SET);

		struct stat st_buf;
		
		stat(ipcameracfg_path, &st_buf );  //读源文件大小
		if( st_buf.st_size >= sizeof(m_ipcameraDev) )
			totalen = sizeof(m_ipcameraDev);
		else
			totalen = st_buf.st_size;

		while(1){
			bytes_read=fread(p+readlen,1,totalen - readlen, cfg_fd);
			if(ferror(cfg_fd)){
				//printf("阅读配置文件出错，以默认方式设置,errno =%d\n",errno);
				fclose(cfg_fd);
				cfg_fd = NULL;
				unlink(ipcameracfg_path);

				restore_ipcameracfg();
				return;                		
			}

			printf("ipconfig read= %d \n",bytes_read);
			readlen += bytes_read;
			if(readlen == totalen)
				break;
		}
		fclose(cfg_fd);
		cfg_fd = NULL;
	}
}

static void DrawIpcameraCfgInfo()
{
	WRT_Rect rt;
	WRT_Rect tmprect;
	rt.x = 220;
	rt.y = 0;
	rt.w = 255;
	rt.h = 530;
	FillRect(0xb2136,rt);
	
	for(int i=0;i<3;i++)
	{
		tmprect = rc_ipcamera[i];
		DrawRect(tmprect,2,rgb_white);
		DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
	}
	for(int i=9;i<11;i++)
	{
		tmprect = rc_ipcamera[i];
		DrawRect(tmprect,2,rgb_white);
		DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
	}
}

static void DrawCfg()
{
	WRT_Rect rt;
	WRT_Rect tmprect;
	rt.x = 220;
	rt.y = 0;
	rt.w = 255;
	rt.h = 530;
	FillRect(0xb2136,rt);
	
	for(int i=0;i<3;i++)
	{
		tmprect = rc_ipcamera[i];
		if((i+1) == g_input_select)
			DrawRect(tmprect,2,rgb_yellow);
		else
			DrawRect(tmprect,2,rgb_white);
		DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
	}

	for(int i=9;i<11;i++)
	{
		tmprect = rc_ipcamera[i];
		if((i+1) == g_input_select)
			DrawRect(tmprect,2,rgb_yellow);
		else
			DrawRect(tmprect,2,rgb_white);
		DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
	}

	if(GetCurrentLanguage() == ENGLISH)
     	DrawText(language2[2].etext, rc_ipcamera[11].x-240,rc_ipcamera[11].y+36,rgb_white,26);
    else
     	DrawText(language2[2].ctext, rc_ipcamera[11].x-240,rc_ipcamera[11].y+36,rgb_white,26);

    
	JpegDisp(rc_ipcamera[3].x,rc_ipcamera[3].y,montadd1,sizeof(montadd1));
	JpegDisp(rc_ipcamera[4].x,rc_ipcamera[4].y,montdel1,sizeof(montdel1));
	JpegDisp(rc_ipcamera[8].x,rc_ipcamera[8].y,ipcsetting1,sizeof(ipcsetting1));
	
	//printf("%d %d  %d\n",g_addidcount,g_addipcount,g_delidcount);
    if(g_addidcount > 0)
    	DrawText(g_adddeviceid,rc_ipcamera[0].x+10,rc_ipcamera[0].y+32, rgb_black,24);
    if(g_addipcount > 0)
    	DrawText(g_adddeviceip,rc_ipcamera[1].x+10,rc_ipcamera[1].y+32, rgb_black,24);
    if(g_delidcount > 0)
    	DrawText(g_deldeviceid,rc_ipcamera[2].x+10,rc_ipcamera[2].y+32, rgb_black,24);

    if(strlen(g_Onvif_Authname) > 0)
    	DrawText(g_Onvif_Authname,rc_ipcamera[9].x+10,rc_ipcamera[9].y+32, rgb_black,24);
    if(strlen(g_Onvif_Authpasswd) > 0)
    	DrawText(g_Onvif_Authpasswd,rc_ipcamera[10].x+10,rc_ipcamera[10].y+32, rgb_black,24);

    if(m_Onvif_Auth_flag == 1)
    	JpegDisp(rc_ipcamera[11].x,rc_ipcamera[11].y,ico_fangqukai,sizeof(ico_fangqukai));
    else
    	JpegDisp(rc_ipcamera[11].x,rc_ipcamera[11].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
}

/*
g_addidcount = 0;
g_addipcount = 0;
g_delidcount = 0;
memset(g_adddeviceid,0,4);
memset(g_adddeviceip,0,17);
memset(g_deldeviceid,0,4);
*/
static void DrawIPCameraCfgMain()
{
	WRT_Rect rt;
	WRT_Rect tmprect;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	Init_ipcamera_config();

	for(int i=0;i<3;i++)
	{
		tmprect = rc_ipcamera[i];
		DrawRect(tmprect,2,rgb_white);
		DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
		
        if(GetCurrentLanguage() == ENGLISH)
        	DrawText(language[i].etext, rc_ipcamera[i].x-180,rc_ipcamera[i].y+36,rgb_white,26);
        else
        	DrawText(language[i].ctext, rc_ipcamera[i].x-180,rc_ipcamera[i].y+36,rgb_white,26);
	}

 	for(int i=9;i<11;i++)
 	{
 		tmprect = rc_ipcamera[i];
 		DrawRect(tmprect,2,rgb_white);
 		DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
 	}
 
 	for(int i=9;i<11;i++)
 	{
 		if(GetCurrentLanguage() == ENGLISH)
         	DrawText(language2[i-9].etext, rc_ipcamera[i].x-180,rc_ipcamera[i].y+36,rgb_white,26);
         else
         	DrawText(language2[i-9].ctext, rc_ipcamera[i].x-180,rc_ipcamera[i].y+36,rgb_white,26);
 	}
 	
	JpegDisp(rc_ipcamera[11].x,rc_ipcamera[11].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
 	
	JpegDisp(rc_ipcamera[3].x,rc_ipcamera[3].y,montadd1,sizeof(montadd1));
	JpegDisp(rc_ipcamera[4].x,rc_ipcamera[4].y,montdel1,sizeof(montdel1));
	
	
	int i =0;
    int j =0;
    int startx = rc_ipcamera[5].x;
    int starty = rc_ipcamera[5].y;
    for(i=0; i<4;i++)
        for(j=0; j<3;j++){
            JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pNumberPointer[(i*3)+j],pNumberSize[(i*3)+j]);
        }

	DrawCfg();
	
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
	
    JpegDisp(rc_ipcamera[6].x,rc_ipcamera[6].y,bt_fanhui1,sizeof(bt_fanhui1));

    JpegDisp(rc_ipcamera[7].x,rc_ipcamera[7].y,bt_guanping1,sizeof(bt_guanping1));
    JpegDisp(rc_ipcamera[8].x,rc_ipcamera[8].y,ipcsetting1,sizeof(ipcsetting1));

    DrawText_16(LT("提示：按'*'退格，按'#' 输入'.'"),135,531+45,rgb_black);	
}

static int inputhandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_ipcamera[5].x;
        int starty = rc_ipcamera[5].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';

        for(i=0; i<4;i++)
	        for(j=0; j< 3;j++){
                tempx = startx+(j)*100;
                tempy = starty+(i)*100;
                if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){    	   	
                        count = (i*3)+j;
                        ret =1;
                        if(status == 1)
                                g_rc_status[count] = true;
                        else{
                                if(g_rc_status[count] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[count] = false;
                        }		    	   	  
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(tempx,tempy, (unsigned char*)pNumberPointer1[count],pNumberSize1[count]);	
                        }else{
                                JpegDisp(tempx,tempy, (unsigned char*)pNumberPointer[count],pNumberSize[count]);

                                if(g_input_select == ADD_ID)
                                {
                                	if(g_addidcount > 2)
                                	{
                                		g_addidcount = 0;
                                		memset(g_adddeviceid,0,4);
                                		printf("111%d %d  %d\n",g_addidcount,g_addipcount,g_delidcount);
                                		DrawCfg();
                                		break;
                                	}
                                }
                                else if(g_input_select == ADD_IP)
                                {
                                	if(g_addipcount > 14)
                                	{
                                		g_addipcount = 0;
                                		printf("222%d %d  %d\n",g_addidcount,g_addipcount,g_delidcount);
                                		memset(g_adddeviceip,0,17);
                                		DrawCfg();
                                		break;
                                	}
                                }
                                else if(g_input_select == DEL_ID)
                                {
                                	if(g_delidcount > 2)
                                	{
                                		g_delidcount = 0;
                                		memset(g_deldeviceid,0,4);
                                		//printf("333%d %d  %d\n",g_addidcount,g_addipcount,g_delidcount);
                                		DrawCfg();
                                		break;
                                	}
                                }
                                
                                g_isUpdated = 0;
                                if( (count != 9 && count != 11)){ //9 *，11 #
                                        c[0] = g_number[count];
                                        if(g_input_select == ADD_ID)
                                        {
                                        	strcat(g_adddeviceid,(char*)&c);
	                                        g_addidcount++;
	                                        g_adddeviceid[g_addidcount]='\0';

	                                        DrawCfg();
	                                        
                                        }else if(g_input_select == ADD_IP)
                                        {
                                        	strcat(g_adddeviceip,(char*)&c);
	                                        g_addipcount++;
	                                        g_adddeviceip[g_addipcount]='\0';
	                                       // printf("%d %d  %d\n",g_addidcount,g_addipcount,g_delidcount);
	                                        DrawCfg();
                                        }else if(g_input_select == DEL_ID)
                                        {
                                        	strcat(g_deldeviceid,(char*)&c);
	                                        g_delidcount++;
	                                        g_deldeviceid[g_delidcount]='\0';
	                                  
	                                        DrawCfg();
                                        }
                                }else if(count  == 9){ //*
                                        WRT_Rect tmprect;
                                        if(g_input_select == ADD_ID)
                                        {
	                                        g_adddeviceid[g_addidcount-1] = '\0';
	                                        tmprect = rc_ipcamera[0];
	                                        DeflateRect(&tmprect,4);
	                                        
	                                        FillRect(rgb_white,tmprect);
	                                        
	                                        g_addidcount--;
	                                        if(g_addidcount < 0)
	                                                g_addidcount = 0;
	                                        g_adddeviceid[g_addidcount]='\0';

	                                        DrawCfg();
	                                      
                                        }else if(g_input_select == ADD_IP)
                                        {
                                        	g_adddeviceip[g_addipcount-1] = '\0';
	                                        tmprect = rc_ipcamera[1];
	                                        DeflateRect(&tmprect,4);
	                                        
	                                        FillRect(rgb_white,tmprect);
	                                        
	                                        g_addipcount--;
	                                        if(g_addipcount < 0)
	                                                g_addipcount = 0;
	                                        g_adddeviceip[g_addipcount]='\0';
	                                       
	                                        DrawCfg();
	                                        //DrawText(g_adddeviceip,rc_ipcamera[1].x+10,rc_ipcamera[1].y+32,rgb_black);
	                                        
                                        }else if(g_input_select == DEL_ID)
                                        {
                                        	g_deldeviceid[g_delidcount-1] = '\0';
	                                        tmprect = rc_ipcamera[2];
	                                        DeflateRect(&tmprect,4);
	                                        
	                                        FillRect(rgb_white,tmprect);
	                                        
	                                        g_delidcount--;
	                                        if(g_delidcount < 0)
	                                                g_delidcount = 0;
	                                        g_deldeviceid[g_delidcount]='\0';
	                                      //  printf("23:%s\n",g_deldeviceid);
	                                        DrawCfg();
	                                        //DrawText(g_deldeviceid,rc_ipcamera[2].x+10,rc_ipcamera[2].y+32,rgb_black);
	                                        
                                        }
                                }else if(count == 11){//# //.
                                        c[0] = '.';
                                        if(g_input_select == ADD_IP)
                                        {
                                        	strcat(g_adddeviceip,(char*)&c);
	                                        g_addipcount++;
	                                        
	                                        g_adddeviceip[g_addipcount]='\0';
	                                       // printf("32:%s\n",g_adddeviceip);
	                                        DrawCfg();
	                                        //DrawText(g_adddeviceip,rc_ipcamera[1].x+10,rc_ipcamera[1].y+32,rgb_black);
                                        }else
                                        	return 0;
                                }
                                g_isUpdated = 1;
                                update_rect(0,0,600,530);

                        }
                        break;
                }
	        }
	return ret;
}

int  IPCameraCfgEventHandler(int x,int y,int status)
{
	WRT_xy xy;
	int i;
    WRT_Rect rect;
    int index =0;
    int ret = 0;
    int pos_x,pos_y;
    xy.x = x;
    xy.y = y;

    for(index=0;index<12;index++){
            if(IsInside(rc_ipcamera[index],xy)){
                    ret = 1;
                    pos_x = rc_ipcamera[index].x;
                    pos_y = rc_ipcamera[index].y;
                    switch(index){
            case 0: //设备号输入框
	            	if(status == 1)
	            	{
		            	StartButtonVocie();
	            	}else
	            	{
	            		g_input_select = ADD_ID;
	            		g_isUpdated = 0;
	            		DrawCfg();
	            		g_isUpdated = 1;
	            		update_rect(0,0,600,530);
	            	}

			        break;
            case 1: //设备ip输入框
            		if(status == 1)
            		{
            			StartButtonVocie();
            		}else
            		{
		            	g_input_select = ADD_IP;
		            	g_isUpdated = 0;
		            	DrawCfg();
		            	g_isUpdated = 1;
		            	update_rect(0,0,600,530);
	            	}
	            	break;
            case 2: //删除设备号输入框
            		if(status == 1)
            		{
            			StartButtonVocie();
            		}else
            		{
            			g_input_select = DEL_ID;
            			g_isUpdated = 0;
            			DrawCfg();
            			g_isUpdated = 1;
            			update_rect(0,0,600,530);
            		}
	            	
                    break;
                    
            case 3://增加按键
            		if(g_addidcount == 0 || g_addipcount == 0)
            			return 0;
            		if(status == 1)
            		{
            			StartButtonVocie();
						JpegDisp(pos_x,pos_y,montadd2,sizeof(montadd2));
            		}else
            		{
	            		JpegDisp(pos_x,pos_y,montadd1,sizeof(montadd1));
	            		for(i=0;i<12;i++)
	            		{
	            			if(m_ipcameraDev[i].shownumber == atoi(g_adddeviceid))
	            			{
	            				g_input_select = 0;
				            	g_addidcount = 0;
				            	g_addipcount = 0;
			            		memset(g_adddeviceid,0,4);
			            		memset(g_adddeviceip,0,16);

			            		g_Onvif_Authname_count = 0;
			            		g_Onvif_Authpasswd_count = 0;
			            		memset(g_Onvif_Authname,0,16);
			            		memset(g_Onvif_Authpasswd,0,16);
			            		m_Onvif_Auth_flag = 0;
			            		
			            		CreateDialogWin2(LT("设备号已存在!"),MB_NONE,NULL,NULL);
	            				SDL_Delay(500);
	            				CloseTopWindow();
	            				return 0;
	            			}
	            		}
	            		for(i=0;i<12;i++)
	            		{
	            			if(m_ipcameraDev[i].ip == inet_addr(g_adddeviceip))
	            			{
	            				g_input_select = 0;
				            	g_addidcount = 0;
				            	g_addipcount = 0;
			            		memset(g_adddeviceid,0,4);
			            		memset(g_adddeviceip,0,16);

			            		g_Onvif_Authname_count = 0;
			            		g_Onvif_Authpasswd_count = 0;
			            		memset(g_Onvif_Authname,0,16);
			            		memset(g_Onvif_Authpasswd,0,16);
			            		m_Onvif_Auth_flag = 0;
			            		
			            		CreateDialogWin2(LT("IP已存在!"),MB_NONE,NULL,NULL);
	            				SDL_Delay(500);
	            				CloseTopWindow();
	            				return 0;
	            			}
	            			
	            		}
	            		for(i=0;i<12;i++)
	            		{
	            			if(m_ipcameraDev[i].shownumber == 0)
	            			{
	            				if(inet_addr(g_adddeviceip)<=0)
	            				{
	            					g_input_select = 0;
									g_addidcount = 0;
									g_addipcount = 0;
									memset(g_adddeviceid,0,4);
									memset(g_adddeviceip,0,16);

									g_Onvif_Authname_count = 0;
				            		g_Onvif_Authpasswd_count = 0;
				            		memset(g_Onvif_Authname,0,16);
				            		memset(g_Onvif_Authpasswd,0,16);
				            		m_Onvif_Auth_flag = 0;
			            		
									CreateDialogWin2(LT("IP配置错误!"),MB_NONE,NULL,NULL);
									SDL_Delay(500);
									CloseTopWindow();
									return 0;
	            				}
	            				memset(m_ipcameraDev[i].onvif_username,0,16);
				            	memset(m_ipcameraDev[i].onvif_passwd,0,16);
	            				m_ipcameraDev[i].shownumber = atoi(g_adddeviceid);
	            				m_ipcameraDev[i].ip = inet_addr(g_adddeviceip);

	            				m_ipcameraDev[i].onvif_auth_flag = m_Onvif_Auth_flag;
	            				if(m_Onvif_Auth_flag == 1)
	            				{
		            				memcpy(m_ipcameraDev[i].onvif_username,g_Onvif_Authname,strlen(g_Onvif_Authname));
		            				memcpy(m_ipcameraDev[i].onvif_passwd,g_Onvif_Authpasswd,strlen(g_Onvif_Authpasswd));
	            				}

	            				//printf(" test :%d  0x%x\n",m_ipcameraDev[i].shownumber,m_ipcameraDev[i].ip);
	            				//printf("onvifname = %s onvifpass = %s\n",m_ipcameraDev[i].onvif_username,m_ipcameraDev[i].onvif_passwd);

	            				CreateDialogWin2(LT("操作成功!"),MB_NONE,NULL,NULL);
	            				
	            				SDL_Delay(500);
	            				save_ipcameracfg();
	            				CloseTopWindow();
	            				g_input_select = 0;
				            	g_addidcount = 0;
				            	g_addipcount = 0;
			            		memset(g_adddeviceid,0,4);
			            		memset(g_adddeviceip,0,16);

			            		g_Onvif_Authname_count = 0;
			            		g_Onvif_Authpasswd_count = 0;
			            		memset(g_Onvif_Authname,0,16);
			            		memset(g_Onvif_Authpasswd,0,16);
			            		m_Onvif_Auth_flag = 0;
			            		
	            				return 0;
	            			}
	            			if(i == 11)
		            			CreateDialogWin2(LT("设备已达上限!"),MB_OK,NULL,NULL);
	            		}
	            		
	            		g_input_select = 0;
		            	g_addidcount = 0;
		            	g_addipcount = 0;
	            		memset(g_adddeviceid,0,4);
	            		memset(g_adddeviceip,0,16);

	            		g_Onvif_Authname_count = 0;
	            		g_Onvif_Authpasswd_count = 0;
	            		memset(g_Onvif_Authname,0,16);
	            		memset(g_Onvif_Authpasswd,0,16);
	            		m_Onvif_Auth_flag = 0;
	            		
            		}
                    break;
            case 4://删除按键输入框
            		if(g_delidcount == 0)
            			return 0;
            		if(status == 1)
            		{
            			StartButtonVocie();
            			JpegDisp(pos_x,pos_y,montdel2,sizeof(montdel2));
            		}else
            		{
            			for(int i = 0;i<12;i++)
            			{
            				if(atoi(g_deldeviceid) == m_ipcameraDev[i].shownumber)
            				{
            					printf("del %d\n",m_ipcameraDev[i].shownumber);
            					m_ipcameraDev[i].shownumber = 0;
            					m_ipcameraDev[i].ip = 0;
            					sync();
            					save_ipcameracfg();
            					CreateDialogWin2(LT("操作成功!"),MB_NONE,NULL,NULL);
	            				SDL_Delay(500);
	            				CloseTopWindow();
	            				break;
            				}
            				if(i == 11)
            				{
            					CreateDialogWin2(LT("设备不存在!"),MB_NONE,NULL,NULL);
	            				SDL_Delay(500);
	            				CloseTopWindow();
            				}
            			}
	            		JpegDisp(pos_x,pos_y,montdel1,sizeof(montdel1));

	            		g_input_select = 0;
		            	g_delidcount = 0;
	            		memset(g_deldeviceid,0,4);
            		}
            		break;
           	case 5://数字区域
           			ret = inputhandler(x,y,status);
           			break;
            case 6: /*返回*/
            		printf("返回上一界面\n");
                    if(status == 1){
                            StartButtonVocie();
                            JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                    }else{
                            JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                            
                            CloseTopWindow();
                    }
                    break;
            case 7:/*关屏*/
                    if(status == 1){ //按下
                            StartButtonVocie();
                            JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                    }else{
                            JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                            screen_close();
                            CloseAllWindows();
                    }
                    break;
            case 8://显示IPC配置
            		if(status == 1){ //按下
                            StartButtonVocie();
                            JpegDisp(pos_x,pos_y,ipcsetting2,sizeof(ipcsetting2));
                    }else{
                            JpegDisp(pos_x,pos_y,ipcsetting1,sizeof(ipcsetting1));
                            CreateIPCameraDispWin();
                    }
                    break;
            case 9://onvif authname
            	if(m_Onvif_Auth_flag == 0)
            		break;
            		
				if(status == 1){
					StartButtonVocie();
				}else{
					CreateSsidPwdWin(9);//9 == ONVIF_AUTH_NAME
					g_input_select = 10;
				}             	  	
				break;
			case 10://onvif authpasswd
				if(m_Onvif_Auth_flag == 0)
            		break;
            		
				if(status == 1){
					StartButtonVocie();
				}else{
					CreateSsidPwdWin(10);//10 == ONVIF_AUTH_PASSWD
					g_input_select = 11;
				}
				break;
			case 11://onvif auth flag
				if(status == 1){
                    StartButtonVocie();
	            }else{
                    if(m_Onvif_Auth_flag == 1 ){
                            JpegDisp(pos_x,pos_y, ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
                            m_Onvif_Auth_flag = 0;
                    }else{
                            JpegDisp(pos_x,pos_y, ico_fangqukai,sizeof(ico_fangqukai));
                            m_Onvif_Auth_flag = 1;
                    }
                }
            default:
                    break;
            }
    }

    }
    return ret;
    
}

void CreateIPCameraCfgWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
	        return;
	    m_Onvif_Auth_flag = 0;
        pWindow->EventHandler = IPCameraCfgEventHandler;
        pWindow->NotifyReDraw = DrawIPCameraCfgMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        if(pNumberPointer == NULL && pNumberPointer1 == NULL){
                GetNumberJpgPointer(&pNumberPointer,&pNumberPointer1);
                GetNumberJpegSizePointer(&pNumberSize,&pNumberSize1);
        }
        pWindow->NotifyReDraw2(pWindow);
}

void DrawIPCameraSubMain(){

	g_ipcamera_flag =1;
	
	OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
	DrawText(LT("正在连接中..."),110,100,rgb_white);

    OSD_JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));
    OSD_JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo3,sizeof(menu_kaisuo3));
    OSD_JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
    OSD_JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));
    OSD_JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin3,sizeof(menu_jingyin3));
    OSD_JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji3,sizeof(menu_guaji3));
    OSD_JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu1,sizeof(menu_tuichu1));

}    

int  IPCameraSubEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        if(g_is_hangup)
        	return 0;
        if(IsInside(rc_callico[6],xy)){
                ret = 1;
                if(status == 1)
                        g_rc_status[6] = true;
                else{
                        if(g_rc_status[6] == false){
                                ret = 0;
                                return ret;
                        }
                        g_rc_status[6] = false;
                }
                pos_x = rc_callico[6].x;
                pos_y = rc_callico[6].y;
                
                
                if(status == 1){
                        StartButtonVocie();

                                JpegDisp(pos_x,pos_y,menu_tuichu2,sizeof(menu_tuichu2));
                        copy_to_sencod_and_three_fbpage();
                }else{

                                JpegDisp(pos_x,pos_y,menu_tuichu1,sizeof(menu_tuichu1));
                        copy_to_sencod_and_three_fbpage();
						SDL_Delay(300);
                        event[0] = IPCAMERA_MONITOR_END;
                        event[1] = event[2] = event[3]=0;
                        if(g_is_hangup == false)
                        {
                        	wrthost_send_cmd(event);
                        	g_is_hangup = true;
                        }

                }
        }
        return ret;
}

static void CloseIPCameraWin(){
        g_ipcamera_flag = 0;
}

void CreateIPCameraSubWin(){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
	        return ;
	    g_is_hangup = false;
        pWindow->EventHandler = IPCameraSubEventHandler;
        pWindow->NotifyReDraw = DrawIPCameraSubMain;
        pWindow->valid = 1;
        pWindow->type = CALLING;
        pWindow->CloseWindow = CloseIPCameraWin;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

int Get_valid_ipc()
{
	int cnt = 0;
	for(int i = 0;i<12;i++)
	{
		if(m_ipcameraDev[i].shownumber != 0)
			cnt++;
	}
	return cnt;
}

int GetOnvifAuthInfoByIp(unsigned long ip,char *authname,char *authpasswd)
{
	int ret = 0;
	
	for(int i=0;i<12;i++)
	{
		if(m_ipcameraDev[i].ip == ip)
		{
			ret = m_ipcameraDev[i].onvif_auth_flag;

			if(ret == 1)
			{
				strcpy(authname,m_ipcameraDev[i].onvif_username);
				strcpy(authpasswd,m_ipcameraDev[i].onvif_passwd);
			}

			return ret;
		}
	}

	return 0;
}


static void GetIPCameraInfo()
{
	Init_ipcamera_config();
	
	m_ipcamera_totalcount = 0;
	m_ipcamera_page = 1;
	m_cur_ipcamera_page =1;
	for(int i = 0;i<12;i++)
	{
		if(m_ipcameraDev[i].shownumber != 0)
			m_ipcamera_totalcount++;
	}
	m_ipcamera_page = m_ipcamera_totalcount/3;
	if(m_ipcamera_totalcount%3)
		m_ipcamera_page +=1;
	m_cur_ipcamera_page = 1;
	
}

static void DrawIPCameraInfo_main()
{
		int xoffset = 21;

        DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);
        if(m_ipcamera_totalcount == 0)
                return;
        m_cur_ipcamera_page = 1;        
        int count = (m_cur_ipcamera_page-1)*3; //已经显示的个数
        int count2 = 0;
        char text[256];
        int i = 0;
        int displaycount = 3;
        
        if( (count + displaycount ) >= m_ipcamera_totalcount)
                displaycount = m_ipcamera_totalcount - count;
        else
                displaycount = 3;
        
        for(i = 0; i < displaycount; i++){
            count2 = 0;
            for(int j = 0;j<12;j++)
            {
            	if(m_ipcameraDev[j].shownumber != 0)
            		count2++;
            	if(count2 > count)
            	{
            		JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));

            		if(GetCurrentLanguage() == ENGLISH)
            			sprintf(text,"IPC%d",m_ipcameraDev[j].shownumber);
            		else
            			sprintf(text,"监控%d",m_ipcameraDev[j].shownumber);
            		DrawEnText(text,rc_monitormenuico[i].w,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);
            		count++;
            		break;
            	}
            }
        }

        DrawStatusbar(m_ipcamera_page,m_cur_ipcamera_page);
}

static void DrawIPCameraInfo_down()
{
		int xoffset = 21;
        
        int i = 0;

        if(m_ipcamera_totalcount == 0)
                return;
        m_cur_ipcamera_page++;
        if(m_cur_ipcamera_page > m_ipcamera_page)
                m_cur_ipcamera_page  = 1;

		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

		JpegDisp(rc_monitormenuico[3].x,rc_monitormenuico[3].y,menu_shangfan1,sizeof(menu_shangfan1));
		if(m_cur_ipcamera_page<m_ipcamera_page)
			JpegDisp(rc_monitormenuico[4].x,rc_monitormenuico[4].y,menu_xiafan1,sizeof(menu_xiafan1)); 
		else
			;
		  
		DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);

		JpegDisp(rc_monitormenuico[5].x,rc_monitormenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));

		JpegDisp(rc_monitormenuico[6].x,rc_monitormenuico[6].y,bt_guanping1,sizeof(bt_guanping1));
 
        int count = (m_cur_ipcamera_page-1)*3; //已经显示的个数
        int count2 = 0;
        char text[256];
       
        int displaycount = 3;
        

        if( (count + displaycount ) >= m_ipcamera_totalcount)
                displaycount = m_ipcamera_totalcount - count;
        else
                displaycount = 3;
        
        for(i = 0; i < displaycount; i++){
                count2 = 0;
                for(int j = 0;j<12;j++)
                {
                	if(m_ipcameraDev[j].shownumber != 0)
                		count2++;
                	if(count2 > count)
                	{
                		JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));
                		printf("count %d : showid %d",count,m_ipcameraDev[j].shownumber);
                		if(GetCurrentLanguage() == ENGLISH)
                			sprintf(text,"IPC%d",m_ipcameraDev[j].shownumber);
                		else
                			sprintf(text,"监控%d",m_ipcameraDev[j].shownumber);
                		DrawEnText(text,rc_monitormenuico[i].w,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);
                		count++;
                		break;
                	}
                }
                
        }
        DrawStatusbar(m_ipcamera_page,m_cur_ipcamera_page);
}

static void DrawIPCameraInfo_up()
{
		int xoffset = 21;

        if(m_ipcamera_totalcount == 0)
                return;
        m_cur_ipcamera_page--;
        if(m_cur_ipcamera_page <0 )
                m_cur_ipcamera_page  = 1;//m_monitor_page;        

		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
		if(m_cur_ipcamera_page != 1 && m_cur_ipcamera_page>0)
			JpegDisp(rc_monitormenuico[3].x,rc_monitormenuico[3].y,menu_shangfan1,sizeof(menu_shangfan1));

		if(m_ipcamera_page>1 && m_cur_ipcamera_page!=m_ipcamera_page)
			JpegDisp(rc_monitormenuico[4].x,rc_monitormenuico[4].y,menu_xiafan1,sizeof(menu_xiafan1)); 
		
		  
		DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);
		JpegDisp(rc_monitormenuico[5].x,rc_monitormenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));

		JpegDisp(rc_monitormenuico[6].x,rc_monitormenuico[6].y,bt_guanping1,sizeof(bt_guanping1));

        int count = (m_cur_ipcamera_page-1)*3; //已经显示的个数
        int count2 = 0;
        char text[256];
        int i = 0;
        int displaycount = 3;
        

        if( (count + displaycount ) >= m_ipcamera_totalcount)
                displaycount = m_ipcamera_totalcount - count;
        else
                displaycount = 3;
        
        for(i = 0; i < displaycount; i++){
                count2 = 0;
                for(int j = 0;j<12;j++)
                {
                	if(m_ipcameraDev[j].shownumber != 0)
                		count2++;
                	if(count2 > count)
                	{
                		JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));
                		
                		if(GetCurrentLanguage() == ENGLISH)
                			sprintf(text,"IPC%d",m_ipcameraDev[j].shownumber);
                		else
                			sprintf(text,"监控%d",m_ipcameraDev[j].shownumber);
                		DrawEnText(text,rc_monitormenuico[i].w,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);
                		count++;
                		break;
                	}
                }
                
        }
        
	
		DrawStatusbar(m_ipcamera_page,m_cur_ipcamera_page);
        
}

int  IPCameraEventHandler(int x,int y,int status){
        WRT_xy xy;
        WRT_Rect rect;
        int index =0;
        int ret = 0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        
        for(index=0;index<7;index++){
                if(IsInside(rc_monitormenuico[index],xy)){
                        ret = 1;
                        pos_x = rc_monitormenuico[index].x;
                        pos_y = rc_monitormenuico[index].y;
                        switch(index){
                case 3: //上翻
                		if(m_ipcamera_page == 1 || m_cur_ipcamera_page ==1)
						{
							printf("上翻无效\n");
							break;
						}
                           
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                
                                g_isUpdated = 0;
                                DrawIPCameraInfo_up();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);

                        }
                        break;
                case 4: //下翻
                        if(m_cur_ipcamera_page > m_ipcamera_page || m_cur_ipcamera_page == m_ipcamera_page)
                        {
                        	printf("下翻无效\n");
                        	break;
                        }
                                
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
                                
                                g_isUpdated = 0;
                                
                                DrawIPCameraInfo_down();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);

                        }
                        break;
                case 5: /*返回*/
                		printf("返回上一界面\n");
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                
                                CloseTopWindow();
                        }
                        break;
                case 6:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                default:
                        int count = (m_cur_ipcamera_page-1)*3;
                        if((index + count) < m_ipcamera_totalcount){

                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,ico_jianshi2,sizeof(ico_jianshi2));
                                        //ReDrawMonitorName(index,count);
                                }else{
                                        JpegDisp(pos_x,pos_y,ico_jianshi1,sizeof(ico_jianshi1));
                                        int realindex = 0;
										for(int j=0;j<12;j++)
										{
											if(m_ipcameraDev[j].shownumber != 0)
											{
												realindex++;
											}
											//printf("index + count %d\n",index + count);
											if(realindex == (index + count+1))
											{
												break;
											}
										}
										
										//printf("realindex %d 监视IP:0x%x\n",realindex,m_ipcameraDev[(realindex-1)].ip);
                                        if(m_ipcameraDev[(realindex-1)].ip == 0 || m_ipcameraDev[(realindex-1)].ip == pSystemInfo->LocalSetting.privateinfo.LocalIP){
                                            CreateDialogWin2(LT("监视地址不对"),MB_OK,NULL,NULL);
                                            break;
                                        }
                                        
                                        CreateIPCameraSubWin();
                                        
                                        
                                        event[0] = IPCAMERA_MONITOR_START;
                                        event[1] = 0;
                                        event[2] = 0;
                                        event[3] = 0;
                                        wrthost_send_cmd(event);
                                        char tmptext[256];
                                        if(GetCurrentLanguage() == ENGLISH)
				                			sprintf(tmptext,"IPC%d",m_ipcameraDev[(realindex-1)].shownumber);
				                		else
				                			sprintf(tmptext,"监控%d",m_ipcameraDev[(realindex-1)].shownumber);
                                        OSD_DrawText(tmptext,100,35,rgb_white);
                                }
                        }

                        break;
                        }
                        break;
                }
        }
        return ret;
}

void DrawIPCameraMain(){//监视主界面 整界面画

	WRT_Rect rt;
	
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

    if(m_ipcamera_page > 1)
    	JpegDisp(rc_monitormenuico[4].x,rc_monitormenuico[4].y,menu_xiafan1,sizeof(menu_xiafan1));

    JpegDisp(rc_monitormenuico[5].x,rc_monitormenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));

    JpegDisp(rc_monitormenuico[6].x,rc_monitormenuico[6].y,bt_guanping1,sizeof(bt_guanping1));
	

	DrawIPCameraInfo_main();
	

}

void CreatetIPCameraWin(){
    Window* pWindow = New_Window();
    if(pWindow == NULL)
            return ;
    pWindow->EventHandler = IPCameraEventHandler;
    pWindow->NotifyReDraw = DrawIPCameraMain;
    pWindow->valid = 1;
    pWindow->type = CHILD;
    WindowAddToWindowList(pWindow);
    GetIPCameraInfo();
    pWindow->NotifyReDraw2(pWindow);
}

void m_DrawIpcCfg()
{
	char tmpbuf[120];
	WRT_Rect tmprect;
	int istart = 0;
	int jstart = 0;
	int i,j;
	j = 0;
	if(currentPage == 2)
	{
		istart = 6;
	}
	
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 531;
	FillRect(0xb2136,rt);
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

	if(GetCurrentLanguage() == ENGLISH)
		DrawText(language2[0].etext,420,50,rgb_white,26);
	else
		DrawText(language2[0].ctext,420,50,rgb_white,26);

	if(GetCurrentLanguage() == ENGLISH)
		DrawText(language2[1].etext,670,50,rgb_white,26);
	else
		DrawText(language2[1].ctext,670,50,rgb_white,26);

	if(GetCurrentLanguage() == ENGLISH)
		DrawText("Ipcamera ID",rc_ipcip[0].x-130,50,rgb_white,26);
	else
		DrawText("监控ID",rc_ipcip[0].x-80,50,rgb_white,26);

	if(GetCurrentLanguage() == ENGLISH)
		DrawText("Ipcamera IP",rc_ipcip[0].x+30,50,rgb_white,26);
	else
		DrawText("监控IP",rc_ipcip[0].x+50,50,rgb_white,26);
		
	for(i = istart;i<(6+istart);i++)
	{
		if(m_ipcameraDev[i].shownumber != 0)
		{
			tmprect = rc_ipcip[j];
			DrawRect(tmprect,2,rgb_white);
			DeflateRect(&tmprect,4);
        	FillRect(rgb_white,tmprect);
        	memset(tmpbuf,0,120);
	        if(GetCurrentLanguage() == ENGLISH)
	        {
	        	sprintf(tmpbuf,"%d",m_ipcameraDev[i].shownumber);
	        }
	        else
	        {
	        	sprintf(tmpbuf,"%d",m_ipcameraDev[i].shownumber);
	        }
	        DrawText(tmpbuf, rc_ipcip[j].x-70,rc_ipcip[j].y+36,rgb_white,26);
	        memset(tmpbuf,0,120);
	        toinet_addr(m_ipcameraDev[i].ip,(unsigned char*)tmpbuf);
	        DrawText(tmpbuf, rc_ipcip[j].x+10,rc_ipcip[j].y+36,rgb_black,26);

	        tmprect = rc_ipcip[j+6];
	        DrawRect(tmprect,2,rgb_white);
			DeflateRect(&tmprect,4);
        	FillRect(rgb_white,tmprect);

        	DrawText(m_ipcameraDev[i].onvif_username, rc_ipcip[j+6].x+10,rc_ipcip[j+6].y+36,rgb_black,26);

			tmprect = rc_ipcip[j+12];
			DrawRect(tmprect,2,rgb_white);
			DeflateRect(&tmprect,4);
			FillRect(rgb_white,tmprect);

			DrawText(m_ipcameraDev[i].onvif_passwd, rc_ipcip[j+12].x+10,rc_ipcip[j+12].y+36,rgb_black,26);


	        j++;
		}
	}
	
	if(havePage2)
	{
		if(currentPage == 1)
		{
			JpegDisp(rc_ipcip[19].x,rc_ipcip[19].y,menu_xiafan1,sizeof(menu_xiafan1));
		}
		else if(currentPage == 2)
		{
			JpegDisp(rc_ipcip[18].x,rc_ipcip[18].y,menu_shangfan1,sizeof(menu_shangfan1));
		}
	}
	
	JpegDisp(rc_ipcip[20].x,rc_ipcip[20].y,bt_fanhui1,sizeof(bt_fanhui1));
    JpegDisp(rc_ipcip[21].x,rc_ipcip[21].y,bt_guanping1,sizeof(bt_guanping1));

}

int  IPCameraDispEventHandler(int x,int y,int status){
        WRT_xy xy;
        WRT_Rect rect;
        int index =0;
        int ret = 0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        for(index=0;index<22;index++){
            if(IsInside(rc_ipcip[index],xy)){
                    ret = 1;
                    pos_x = rc_ipcip[index].x;
                    pos_y = rc_ipcip[index].y;
                    switch(index){
			            case 20: /*返回*/
			            		printf("返回上一界面\n");
			                    if(status == 1){
			                            StartButtonVocie();
			                            JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
			                    }else{
			                            JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
			                            CloseTopWindow();
			                    }
			                    break;
			            case 21:/*关屏*/
			                    if(status == 1){ //按下
			                            StartButtonVocie();
			                            JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
			                    }else{
			                            JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
			                            screen_close();
			                            CloseAllWindows();
			                    }
			                    break;
			            case 18: //上翻
		                		if(havePage2 > 0 && currentPage == 2)
								{
									if(status == 1){
		                                StartButtonVocie();
		                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
			                        }else{
		                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));

		                                currentPage = 1;
		                                g_isUpdated = 0;
		                                m_DrawIpcCfg();
		                                g_isUpdated = 1;
		                                update_rect(0,0,1024,600);
			                        }
								}
								
		                        break;
		                case 19: //下翻
		                        if(havePage2 > 0 && currentPage == 1)
		                        {
		                        	if(status == 1){
		                                StartButtonVocie();
		                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
			                        }else{
		                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
		                                currentPage = 2;
		                                g_isUpdated = 0;
		                                m_DrawIpcCfg();
		                                g_isUpdated = 1;
		                                update_rect(0,0,1024,600);

			                        }
		                        }
		                                
		                        
		                        break;
			            default:
			                    break;
                    }
                    break;
            }
        }
        return ret;
}


int GetValidIpcamera()
{
	int cnt = 0;
	for(int i=0;i<12;i++)
	{
		if(m_ipcameraDev[i].shownumber != 0)
			cnt++;
	}
	if(cnt > 6)
		return 1;
	else
		return 0;
}

void DrawIPCameraDispMain(){
	m_DrawIpcCfg();
}

void CreateIPCameraDispWin(){
    Window* pWindow = New_Window();
    if(pWindow == NULL)
            return ;
    
    havePage2 = GetValidIpcamera();
    currentPage = 1;
    
    pWindow->EventHandler = IPCameraDispEventHandler;
    pWindow->NotifyReDraw = DrawIPCameraDispMain;
    pWindow->valid = 1;
    pWindow->type = CHILD;
    WindowAddToWindowList(pWindow);
    GetIPCameraInfo();
    pWindow->NotifyReDraw2(pWindow);
}


//========================================================

int GetMonitorIndex(int type,int index)
{
        int  idx = 0;
        if(type == MONITOR_TYPE_DOOR){
                idx = index;
        }else if(type == MONITOR_TYPE_GATE){
                idx = index - pSystemInfo->LocalSetting.publicroom.NumOfDoor;
        }else if(type == MONITOR_TYPE_SMALLDOOR){
                idx = index - (pSystemInfo->LocalSetting.publicroom.NumOfDoor+pSystemInfo->LocalSetting.publicroom.NumOfGate);
        }else if(type == MONITOR_TYPE_OTHER){
                idx = index - (pSystemInfo->LocalSetting.publicroom.NumOfDoor+pSystemInfo->LocalSetting.publicroom.NumOfGate+pSystemInfo->LocalSetting.privateinfo.NumOfSmallDoor);
        }
        return idx;
}
extern int net_test_ping(unsigned long ipaddr);
void Update_holdon_Redraw()
{
    OSD_JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));
    copy_to_sencod_and_three_fbpage();
    if(g_is_lock == false){
        g_is_lock = true;
    OSD_JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo1,sizeof(menu_kaisuo1));
        copy_to_sencod_and_three_fbpage();
    }
}
void Deal_monitor_event(int index)
{
	unsigned long lockevent[4]={0};
	unsigned long ip = 0;
	int ret = 0;
	if(index >= 1)//已经在监视，按开锁键切换
	{
		lockevent[0] = ROOM_STOP_MONT_BYKEY;
        lockevent[1] = lockevent[2] = lockevent[3]=0;
        wrthost_send_cmd(lockevent);
	}
regetip:
	ip = m_MonitorDev[index].monitorDev.IP;
	ret = net_test_ping(ip);
	if(ret == 0)
	{
		WRT_Rect rt;
	    rt.x = 0;
	    rt.y = 0;
	    rt.w = 640;
	    rt.h = 480;
	    //printf("7877777777%d  %d\n",index,g_current_monitor_index);
		if(g_current_monitor_index >= m_monitor_totalcount)
		{
			index = 0;
			g_current_monitor_index = 0;
			//printf("超最大值1 %d\n",index);
		}
		else
		{
		    g_current_monitor_index++;
			index = g_current_monitor_index;
			//printf("网络不通2 %d\n",index);
		}
		goto regetip;
	}
	g_current_monitor_index++;
	//printf("555555555\n");
	//printf("启动监视 %d\n",index);
	wrthost_set_montip(m_MonitorDev[index].monitorDev.IP);
    lockevent[0] = ROOM_STARTMONT;
    lockevent[1] = m_MonitorDev[index ].type;
    lockevent[2] = GetMonitorIndex(lockevent[1],(index));
    lockevent[3] = 0;
    wrthost_send_cmd(lockevent);
   // printf("666666\n");
    WRT_Rect rt;
    rt.x = 0;
    rt.y = 0;
    rt.w = 640;
    rt.h = 480;
    FillRect(rgb_black,rt);
    OSD_DrawText(m_MonitorDev[index].monitorDev.name,100,35,rgb_white);
    DrawText(LT("正在连接中..."),50,70,rgb_white);
}
static void ReleaseMonitorInfo()
{
	if(m_MonitorDev)
		ZENFREE(m_MonitorDev);
	m_MonitorDev = NULL;
	m_cur_monitor_page = 1;
	m_monitor_page = 1;
	m_monitor_totalcount  = 0;
}

static void GetMonitorInfo(){//获取监视对象的信息，这里主要用于创建监视子界面
        if(m_MonitorDev != NULL && m_monitor_totalcount != 0){
                ReleaseMonitorInfo();
        }
        char text1[20] = {0};
        int j=0;
        int i=0;
		m_monitor_totalcount = 0;
        if(pSystemInfo->LocalSetting.publicroom.NumOfDoor > 0)
        {
        m_monitor_totalcount = pSystemInfo->LocalSetting.publicroom.NumOfDoor; //门口机数量
        }
        if(pSystemInfo->LocalSetting.publicroom.NumOfGate > 0)
        {
        m_monitor_totalcount += pSystemInfo->LocalSetting.publicroom.NumOfGate;//围墙机数量
        }
        if(pSystemInfo->LocalSetting.privateinfo.NumOfMONI > 0)
        {
        m_monitor_totalcount += pSystemInfo->LocalSetting.privateinfo.NumOfMONI;//摄像头（自己安装的摄像头)
		}
        if(pSystemInfo->LocalSetting.privateinfo.NumOfSmallDoor > 0)
        {
        m_monitor_totalcount += pSystemInfo->LocalSetting.privateinfo.NumOfSmallDoor; //小门口机
        }
		if(Get_valid_ipc() > 0)
		{
			m_monitor_totalcount += Get_valid_ipc();
		}
        if(m_MonitorDev == NULL)
        {
                m_MonitorDev = (TEMP_T_DEV_ITEM*)ZENMALLOC(m_monitor_totalcount*sizeof(TEMP_T_DEV_ITEM));
                if(m_MonitorDev == NULL)
                {
                        WRT_DEBUG("ZENMALLOC failed moniter dev ");
                        m_MonitorDev = NULL;
                        return;
                }

        }
        if(pSystemInfo->LocalSetting.publicroom.NumOfDoor > 0)
        {
        for(i=0; i< pSystemInfo->LocalSetting.publicroom.NumOfDoor; i++)
        {
                // strcpy(m_MonitorDev[j].name,pSystemInfo->LocalSetting.publicinfo.Door[i].name);
                // m_MonitorDev.IP  = pSystemInfo->LocalSetting.publicinfo.Door[i].IP;
                m_MonitorDev[j].monitorDev = pSystemInfo->LocalSetting.publicroom.Door[i];
                m_MonitorDev[j].type = MONITOR_TYPE_DOOR;
                j++;
	        }
        }
        if(pSystemInfo->LocalSetting.publicroom.NumOfGate > 0)
        {
        for(i=0; i < pSystemInfo->LocalSetting.publicroom.NumOfGate;i++)
        {
                m_MonitorDev[j].monitorDev = pSystemInfo->LocalSetting.publicroom.MainGate[i];
                m_MonitorDev[j].type = MONITOR_TYPE_GATE;
                j++;
	        }
        }
        if(pSystemInfo->LocalSetting.privateinfo.NumOfSmallDoor > 0)
        {
        for(i=0; i< pSystemInfo->LocalSetting.privateinfo.NumOfSmallDoor; i++)
        {
                m_MonitorDev[j].monitorDev = pSystemInfo->LocalSetting.privateinfo.SmallDoor[i];
                m_MonitorDev[j].type = MONITOR_TYPE_SMALLDOOR;
                j++;
	        }
        }
        if(pSystemInfo->LocalSetting.privateinfo.NumOfMONI > 0)
        {
        for(i = 0; i< pSystemInfo->LocalSetting.privateinfo.NumOfMONI; i++)
        {
                m_MonitorDev[j].monitorDev = pSystemInfo->LocalSetting.privateinfo.Moni[i];
                m_MonitorDev[j].type = MONITOR_TYPE_OTHER;
                j++;
	        }
        }
		if(Get_valid_ipc() > 0)
		{
			for(i = 0;i<12;i++)
			{
				if(m_ipcameraDev[i].shownumber != 0)
				{
					m_MonitorDev[j].type = MONITOR_TYPE_IPCAMERA;
					m_MonitorDev[j].monitorDev.IP = m_ipcameraDev[i].ip;
					memset(m_MonitorDev[j].monitorDev.name,0,20);
					if(GetCurrentLanguage() == ENGLISH)
            			sprintf(m_MonitorDev[j].monitorDev.name,"IPC-%d",m_ipcameraDev[i].shownumber);
            		else
            			sprintf(m_MonitorDev[j].monitorDev.name,"监控-%d",m_ipcameraDev[i].shownumber);
            		j++;
            		if(j >= m_monitor_totalcount)
            		{
            			break;
            		}
				}
			}
		}
        m_monitor_page = m_monitor_totalcount/3;
        if(m_monitor_totalcount%3)
                m_monitor_page +=1;
        m_cur_monitor_page = 1;
		//printf("mont_page :%d m_monitor_totalcount :%d\n",m_monitor_page,m_monitor_totalcount);
        /*
        m_monitor_totalcount = roomtest.LocalRoom.NumOfGate +roomtest.LocalRoom.NumOfGroupGate +1;
        if(m_MonitorName == NULL)
        m_MonitorName = (MONITORNAME*)ZENMALLOC(m_monitor_totalcount*sizeof(MONITORNAME));
        memset((void*)m_MonitorName,0,m_monitor_totalcount*sizeof(MONITORNAME));
        if(m_MonitorIP == NULL)
        m_MonitorIP = (unsigned long*)ZENMALLOC(m_monitor_totalcount*sizeof(unsigned long));
        memset((void*)m_MonitorIP,0,m_monitor_totalcount*sizeof(unsigned long));
        int i =0;
        int x = 0;
        strcpy(m_MonitorName[0].name,"门口主机");
        m_MonitorName[0].idle = 1;
        m_MonitorIP[0] = roomtest.LocalRoom.GroupDoorIP;

        if(roomtest.LocalRoom.NumOfGate > 0){
        for(i =0; i< roomtest.LocalRoom.NumOfGate;i++){
        strcpy(m_MonitorName[i+1].name,(char*)roomtest.LocalRoom.NameOfMainGate[i]);
        m_MonitorName[i+1].idle = 1;
        m_MonitorIP[i+1] = roomtest.LocalRoom.MainGateIP[i];
        }
        }
        if(roomtest.LocalRoom.NumOfGroupGate > 0){
        for(i = roomtest.LocalRoom.NumOfGate;i < (m_monitor_totalcount-1);i++){
        strcpy(m_MonitorName[i].name,(char*)roomtest.LocalRoom.NameOfGroupGate[x]);
        m_MonitorName[i].idle = 1;
        m_MonitorIP[i] = roomtest.LocalRoom.GroupGateIP[x];
        x++;
        }
        }
        m_monitor_page = m_monitor_totalcount/3;
        if(m_monitor_totalcount%3)
        m_monitor_page +=1;
        m_cur_monitor_page = 1;
        */
}

void Get_monitor_dev_info()
{
	GetMonitorInfo();
}
static void DrawMonitorInfo()
{
        WRT_Rect curbox;
        int xoffset = 0;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 1024;
        curbox.h = 530;

        FillRect(0xb2136,curbox);
        DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);
        if(m_MonitorDev == NULL)
                return;


        int count = (m_cur_monitor_page-1)*3; //已经显示的个数
        char* text = NULL;
        int i = 0;
        int displaycount = 3;
        if( (count + displaycount ) >= m_monitor_totalcount)
                displaycount = m_monitor_totalcount - count;
        else
                displaycount = 3;
        for(i = 0; i < displaycount; i++){
                JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));
                text = (char*)(m_MonitorDev[count++].monitorDev.name);
                DrawEnText(text,rc_monitormenuico[i].w+xoffset,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);

        }
        DrawStatusbar(m_monitor_page,m_cur_monitor_page);
}

static void DrawMonitorInfo_main()
{

		int xoffset = 21;

        DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);
        if(m_MonitorDev == NULL)
                return;
        m_cur_monitor_page = 1;        
        int count = (m_cur_monitor_page-1)*3; //已经显示的个数
        char* text = NULL;
        int i = 0;
        int displaycount = 3;

        if( (count + displaycount ) >= m_monitor_totalcount)
                displaycount = m_monitor_totalcount - count;
        else
                displaycount = 3;
                
        for(i = 0; i < displaycount; i++){
                JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));
                text = (char*)(m_MonitorDev[count++].monitorDev.name);
                //printf("mont name :%s\n",m_MonitorDev[count++].monitorDev.name);
                DrawEnText(text,rc_monitormenuico[i].w,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);

        }
        DrawStatusbar(m_monitor_page,m_cur_monitor_page);
}

static void DrawMonitorInfo_down()
{

		int xoffset = 21;
        char* text = NULL;
        int i = 0;
        int displaycount = 3;		
      
        if(m_MonitorDev == NULL)
                return;
        m_cur_monitor_page++;
        if(m_cur_monitor_page > m_monitor_page)
                m_cur_monitor_page  = 1;   

		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

		JpegDisp(rc_monitormenuico[3].x,rc_monitormenuico[3].y,menu_shangfan1,sizeof(menu_shangfan1));
		if(m_cur_monitor_page<m_monitor_page)
			JpegDisp(rc_monitormenuico[4].x,rc_monitormenuico[4].y,menu_xiafan1,sizeof(menu_xiafan1)); 
		else
			;
		  
		DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);

		JpegDisp(rc_monitormenuico[5].x,rc_monitormenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));

		JpegDisp(rc_monitormenuico[6].x,rc_monitormenuico[6].y,bt_guanping1,sizeof(bt_guanping1));
 
        int count = (m_cur_monitor_page-1)*3; //已经显示的个数

        if( (count + displaycount ) >= m_monitor_totalcount)
                displaycount = m_monitor_totalcount - count;
        else
                displaycount = 3;
        for(i = 0; i < displaycount; i++){
            JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));
            text = (char*)(m_MonitorDev[count++].monitorDev.name);
            DrawEnText(text,rc_monitormenuico[i].w,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);
        }
		       
        DrawStatusbar(m_monitor_page,m_cur_monitor_page);
}

static void DrawMonitorInfo_up()
{

		int xoffset = 21;

        
        if(m_MonitorDev == NULL)
                return;
        m_cur_monitor_page--;
        if(m_cur_monitor_page <0 )
                m_cur_monitor_page  = 1;//m_monitor_page;        
			
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
		if(m_cur_monitor_page != 1 && m_cur_monitor_page>0)
			JpegDisp(rc_monitormenuico[3].x,rc_monitormenuico[3].y,menu_shangfan1,sizeof(menu_shangfan1));

		if(m_monitor_page>1 && m_cur_monitor_page!=m_monitor_page)
			JpegDisp(rc_monitormenuico[4].x,rc_monitormenuico[4].y,menu_xiafan1,sizeof(menu_xiafan1)); 
		
		  
		DrawText(LT("请选择监视点"),(1024 - GetTextWidth(LT("请选择监视点"),0))/2,70,rgb_white);
		JpegDisp(rc_monitormenuico[5].x,rc_monitormenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));

		JpegDisp(rc_monitormenuico[6].x,rc_monitormenuico[6].y,bt_guanping1,sizeof(bt_guanping1));

        int count = (m_cur_monitor_page-1)*3; //已经显示的个数
        char* text = NULL;
        int i = 0;
        int displaycount = 3;
        if( (count + displaycount ) >= m_monitor_totalcount)
                displaycount = m_monitor_totalcount - count;
        else
                displaycount = 3;
        for(i = 0; i < displaycount; i++){
                JpegDisp(rc_monitormenuico[i].x,rc_monitormenuico[i].y,ico_jianshi1,sizeof(ico_jianshi1));
                text = (char*)(m_MonitorDev[count++].monitorDev.name);
                //printf("mont name :%s %d %d\n",m_MonitorDev[count++].monitorDev.name,m_monitor_page,m_cur_monitor_page);
                DrawEnText(text,rc_monitormenuico[i].w,rc_monitormenuico[i].h,rc_monitormenuico[i].x,rc_monitormenuico[i].y);

        }
	
		DrawStatusbar(m_monitor_page,m_cur_monitor_page);
        
}


static void ReDrawMonitorName(int index,int count)
{
        /*
        char* text = NULL;
        WRT_Rect rt;
        rt = rc_monitormenuico[index];
        rt.y = rt.y+rt.h - 24;
        rt.h = 48;
        FillRect(BACK_COLOR,rt);

        text = (char*)(m_MonitorDev[index+count].monitorDev.name);
        DrawEnText(text,rc_monitormenuico[index].w+21,rc_monitormenuico[index].h,rc_monitormenuico[index].x,rc_monitormenuico[index].y);
        */
}


void CreatetMonitorWin(){
    Window* pWindow = New_Window();
    if(pWindow == NULL)
            return ;
    pWindow->EventHandler = MonitorEventHandler;
    pWindow->NotifyReDraw = DrawMonitorMain;
    pWindow->valid = 1;
    pWindow->type = CHILD;
    WindowAddToWindowList(pWindow);
    GetMonitorInfo();
    pWindow->NotifyReDraw2(pWindow);
}
extern int g_ts_flag;

//extern void wrthost_syssetbusy();
int  TsCalEventHandler(int x,int y,int status)
{
	switch(x)
	{
		default :
			break;
	}
}
extern int g_ts_send;
static void CloseTsCal()
{
	WRT_DEBUG("校屏界面关");
	g_ts_send = 0;
}
void DrawTsCalibrate()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 600;
	g_isUpdated = 0;
	FillRect(rgb_black,rt);
	
	g_isUpdated = 1;
	WRT_DEBUG("ts 创建");
	g_ts_send = 1;
	wrthost_syssetbusy();	
}
void CreateTsCalibrate(){
	Window* pWindow = New_Window();
	if(pWindow == NULL)
			return ;
	pWindow->EventHandler = TsCalEventHandler;
	pWindow->NotifyReDraw = DrawTsCalibrate;
	pWindow->CloseWindow = CloseTsCal;
	pWindow->valid = 1;
	pWindow->type = TS_CAL;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

T_ROOM_MORE_CFG  *p_MorecfgInfo;
//2014-04-01
///////=======================================

static const WRT_Rect rc_samptestico[27] =
{
	{900,531,111,69},			   //返回
	{6,531,111,69},				   //关屏幕 
	
	{80,70,80,80},
	{190,70,80,80},
	{300,70,80,80},
	{410,70,80,80},
	{520,70,80,80},
	{630,70,80,80},
	{740,70,80,80},
	{850,70,80,80},

	{80,200,80,80},
	{190,200,80,80},
	{300,200,80,80},
	{410,200,80,80},
	{520,200,80,80},
	{630,200,80,80},
	{740,200,80,80},
	{850,200,80,80},

	{80,330,80,80},
	{190,330,80,80},
	{300,330,80,80},
	{410,330,80,80},
	{520,330,80,80},
	{630,330,80,80},
	{740,330,80,80},
	{850,330,80,80},

	{780,531,111,69},              //确认
};

int IoLevelArray[18] = {0};
int IoPassArray[18]  = {0};

int IoTestFlag = 0;
int IoTestFlagFirst = 0;
extern volatile UInt8 g_green;
extern volatile UInt8 g_red;
static void CloseSampleTest()
{
	IoTestFlag = 0;
	IoTestFlagFirst = 0;
}

static void DrawTestDown(int rcNum)
{
	WRT_Rect rt;
	int color;
	rt = rc_samptestico[rcNum];
	color = rgb_black;
	FillRect(color,rt);
}

static void DrawTestResult(int Ok ,int rcNum,char *buf)
{
	WRT_Rect rt;
	int color;
	rt = rc_samptestico[rcNum];
	if(Ok)
		color = rgb_green;
	else
		color = rgb_red;

	if(buf != NULL)
	{
		FillRect(color,rt);
		DrawTextToRect(rt,buf,rgb_white);
	}else
		FillRect(rgb_blue,rt);
}

int  SampleTestEventHandler(int x,int y,int status)
{
	int ret = 0;
	WRT_xy xy;
	int pos_x,pos_y;
	int index;
	xy.x = x;
	xy.y = y;

	for(index =0 ;index<27;index++){
		if(IsInside(rc_samptestico[index],xy)){
				ret = 1;
				
				pos_x = rc_samptestico[index].x;
				pos_y = rc_samptestico[index].y;
		
			switch(index){
				case 0://返回
					if(status == 1){
							mp3stop();
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
					}else{
							JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
							CloseTopWindow();
					}
				break;
				
				case 1: //关屏
					
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
					}else{
						JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
						screen_close();
						CloseAllWindows();
					}
				break;
				
				case 20: /*U盘测试*/
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						DrawTestDown(20);
					}else{
						DIR *dirp;
						struct dirent * ptr;
						int count = 0;
			
						dirp = opendir("/mnt/sda");
						if(dirp == NULL)
						{
							DrawTestResult(0,20,LT("失败"));
						}else
						{
							while((ptr = readdir(dirp))!=NULL)
							{
								count++;
							}
							
							if(count <=2) // . .. 2个文件夹
								DrawTestResult(0,20,LT("失败"));
							else{
								DrawTestResult(1,20,LT("成功"));
							}
						}
						if(dirp != NULL)
							closedir(dirp);
						dirp = NULL;
						ptr  = NULL;
						
					}
				break;
				case 21: /*SD卡测试*/
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						DrawTestDown(21);
					}else{
						DIR *dirp;
						struct dirent * ptr;
						int count = 0;
			
						dirp = opendir("/mnt/mmc");
						if(dirp == NULL)
						{
							DrawTestResult(0,21,LT("失败"));
						}else
						{
							while((ptr = readdir(dirp))!=NULL)
							{
								count++;
							}
							
							if(count <=2) // . .. 2个文件夹
								DrawTestResult(0,21,LT("失败"));
							else{
								DrawTestResult(1,21,LT("成功"));
							}
						}
						if(dirp != NULL)
							closedir(dirp);
						dirp = NULL;
						ptr  = NULL;
						
					}
				break;
				case 22: /*摄像头*/
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						DrawTestDown(22);
					}else{
						DrawTestResult(0,22,NULL);
						//CreateCameraWin();
					}
				break;
				case 23: /*LED灯*/
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						DrawTestResult(1,23,LT("测试中"));
					}else{
					
						tmGpioSetPin(CTL_IO_SETLED_RED,1);
				        SDL_Delay(500);
				        tmGpioSetPin(CTL_IO_SETLED_RED,0);
				        SDL_Delay(500);
				        tmGpioSetPin(CTL_IO_SETLED_GREEN,1);
				        SDL_Delay(500);
				        tmGpioSetPin(CTL_IO_SETLED_GREEN,0);
				        SDL_Delay(500);
				        tmGpioSetPin(CTL_IO_SETLED_RED,1);
				        SDL_Delay(500);
				        tmGpioSetPin(CTL_IO_SETLED_RED,0);
				        SDL_Delay(500);
				        tmGpioSetPin(CTL_IO_SETLED_GREEN,1);
				        g_green = 1;
				        g_red = 0;
						DrawTestResult(0,23,NULL);
					}
				break;
				case 24: /*报警铃音*/
					//break;
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						DrawTestDown(24);//DrawTestResult(1,22,LT("测试中"));
					}else{
						
						DrawTestResult(0,24,NULL);
					}
				break;
				case 25: /*探头电压*/
					//break;
					if(status == 1){ //按下
						mp3stop();
						StartButtonVocie();
						DrawTestDown(25);//DrawTestResult(1,23,LT("测试中"));
					}else{
						
						DrawTestResult(0,25,NULL);
					}
				break;

			default:
					break;
			}
			break;
		}
	}
	return ret;
}

void mySampleTest(int isupdate)
{
	int i = 0;
	int color = rgb_blue;
	char tmptxt[32];
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	if(isupdate == 1)
		g_isUpdated = 0;

	for(i = 0;i<18;i++)
	{
		rt = rc_samptestico[i+2];
		if(IoPassArray[i])
			color = rgb_green;
		else
			color = rgb_blue;
		FillRect(color,rt);
	}

	for(i = 0;i<8;i++)
	{
		sprintf(tmptxt,"%s%d",LT("防区"),i+1);
		DrawText_16(tmptxt,rc_samptestico[i+2].x+20,rc_samptestico[i+2].y+100,rgb_white);
		
		sprintf(tmptxt,"%d",IoLevelArray[i]);
		if(IoLevelArray[i] == 1)
			DrawTextToRect(rc_samptestico[i+2],tmptxt,rgb_red);
		else
			DrawTextToRect(rc_samptestico[i+2],tmptxt,rgb_white);
		
	}

	DrawText_16(LT("手柄报警"),rc_samptestico[10].x+10,rc_samptestico[10].y+100,rgb_white);
	DrawText_16(LT("梯控"),rc_samptestico[11].x+20,rc_samptestico[11].y+100,rgb_white);
	DrawText_16(LT("按键报警"),rc_samptestico[12].x+10,rc_samptestico[12].y+100,rgb_white);
	DrawText_16(LT("外接报警"),rc_samptestico[13].x+10,rc_samptestico[13].y+100,rgb_white);
	DrawText_16(LT("手柄开锁"),rc_samptestico[14].x+10,rc_samptestico[14].y+100,rgb_white);
	DrawText_16(LT("手柄摘机"),rc_samptestico[15].x+10,rc_samptestico[15].y+100,rgb_white);
	DrawText_16(LT("门铃"),rc_samptestico[16].x+20,rc_samptestico[16].y+100,rgb_white);

	DrawText_16(LT("呼叫键"),rc_samptestico[17].x+15,rc_samptestico[17].y+100,rgb_white);
	DrawText_16(LT("开锁键"),rc_samptestico[18].x+15,rc_samptestico[18].y+100,rgb_white);
	DrawText_16(LT("对讲键"),rc_samptestico[19].x+15,rc_samptestico[19].y+100,rgb_white);

	for(i = 0;i<10;i++)
	{
		sprintf(tmptxt,"%d",IoLevelArray[i+8]);
		if(IoLevelArray[i+8] == 1)
			DrawTextToRect(rc_samptestico[i+10],tmptxt,rgb_red);
		else
			DrawTextToRect(rc_samptestico[i+10],tmptxt,rgb_white);
	}

	for(i = 0;i<6;i++)
	{
		rt = rc_samptestico[i+20];
		color = rgb_blue;
		FillRect(color,rt);
	}

	DrawText_16(LT("U盘测试"),rc_samptestico[20].x+10,rc_samptestico[20].y+100,rgb_white);
	DrawText_16(LT("SD卡测试"),rc_samptestico[21].x+10,rc_samptestico[21].y+100,rgb_white);
	DrawText_16(LT("摄像头"),rc_samptestico[22].x+20,rc_samptestico[22].y+100,rgb_white);
	DrawText_16(LT("LED灯"),rc_samptestico[23].x+20,rc_samptestico[23].y+100,rgb_white);
	DrawText_16(LT("报警铃音"),rc_samptestico[24].x+10,rc_samptestico[24].y+100,rgb_white);
	DrawText_16(LT("探头复位"),rc_samptestico[25].x+10,rc_samptestico[25].y+100,rgb_white);

	if(isupdate == 1){
		update_rect(0,0,1024,530);
		g_isUpdated = 1;
	}
}

static void DrawSampleTest()
{
	WRT_Rect rt;
	
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	mySampleTest(0);

    JpegDisp(rc_samptestico[0].x,rc_samptestico[0].y,bt_fanhui1,sizeof(bt_fanhui1));
    JpegDisp(rc_samptestico[1].x,rc_samptestico[1].y,bt_guanping1,sizeof(bt_guanping1));
}

void CreateSampleTestWin(){
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;

	pWindow->EventHandler = SampleTestEventHandler;
	pWindow->NotifyReDraw = DrawSampleTest;
	pWindow->CloseWindow  = CloseSampleTest;
	pWindow->valid = 1;
	pWindow->type = DEVICE_TEST;
	
	IoTestFlag = 1;
	IoTestFlagFirst = 1;
	for(int i=0;i<18;i++)
	{
		IoLevelArray[i]=0;
		IoPassArray[i]=0;
	}
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

unsigned long IoValidLevel[18]={0};

static void DrawIoLevel(int level ,int rcNum)
{
	WRT_Rect rt;
	char tmpbuf[32];
	rt = rc_samptestico[rcNum];

	FillRect(rgb_blue,rt);

	sprintf(tmpbuf,"%d",level);
	if(level == 1)
		DrawTextToRect( rt, tmpbuf,rgb_red);
	else
		DrawTextToRect( rt, tmpbuf,rgb_white);
}

static int ok_iolevel(int param)
{
	int i =0;
	for(i = 0;i<18;i++)
		p_MorecfgInfo->validlevel[i] = IoValidLevel[i];//pSystemInfo->mySysInfo.validlevel[i] = IoValidLevel[i];

	write_Morecfg_file();

	CloseTopWindow();
	CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
	SDL_Delay(800);
	CloseTopWindow();
}

static int IoLevelEventHandler(int x,int y,int status)
{
	int ret = 0;
	WRT_xy xy;
	int pos_x,pos_y;
	int index;
	xy.x = x;
	xy.y = y;

	for(index =0 ;index<27;index++){
		if(IsInside(rc_samptestico[index],xy)){
				ret = 1;
				
				pos_x = rc_samptestico[index].x;
				pos_y = rc_samptestico[index].y;

			switch(index){
				case 0://返回
					if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
					}else{
							JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
							CloseTopWindow();
					}
				break;

				case 1: //关屏
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
					}else{
						JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
						screen_close();
						CloseAllWindows();
					}
				break;
				//case 2:
				//case 3:
				//case 4:
				//case 5:
				//case 6:
				//case 7:
				//case 8:
				//case 9:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				
				if(status == 1){ //按下
					StartButtonVocie();
					DrawTestDown(index);
				}else{
					IoValidLevel[index-2] = !IoValidLevel[index-2];
					
					DrawIoLevel(IoValidLevel[index-2],index);
				}
				break;
				
				case 26: //确认
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
					}else{
						JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
						CreateDialogWin2(LT("确定重置电平设置!"),MB_OK|MB_CANCEL,ok_iolevel,NULL);
					}
				default:
					break;
			}
			break;
		}
	}
	return ret;
}

void myIoLevelSet(int isupdate)
{
	int i = 0;
	int color = rgb_blue;
	char tmptxt[32];
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 800;
	rt.h = 300;
	FillRect(0xb2136,rt);

	if(isupdate == 1)
		g_isUpdated = 0;

	for(i = 0;i<18;i++)
	{
		rt = rc_samptestico[i+2];
		color = rgb_blue;
		FillRect(color,rt);
	}

	for(i = 0;i<8;i++)
	{
		sprintf(tmptxt,"%s%d",LT("防区"),i+1);
		DrawText_16(tmptxt,rc_samptestico[i+2].x+20,rc_samptestico[i+2].y+100,rgb_white);
		
		sprintf(tmptxt,"%d",IoValidLevel[i]);
		if(IoValidLevel[i] == 1)
			DrawTextToRect(rc_samptestico[i+2],tmptxt,rgb_red);
		else
			DrawTextToRect(rc_samptestico[i+2],tmptxt,rgb_white);
	}

	DrawText_16(LT("手柄报警"),rc_samptestico[10].x+10,rc_samptestico[10].y+100,rgb_white);
	DrawText_16(LT("梯控"),rc_samptestico[11].x+20,rc_samptestico[11].y+100,rgb_white);
	DrawText_16(LT("按键报警"),rc_samptestico[12].x+10,rc_samptestico[12].y+100,rgb_white);
	DrawText_16(LT("外接报警"),rc_samptestico[13].x+10,rc_samptestico[13].y+100,rgb_white);
	DrawText_16(LT("手柄开锁"),rc_samptestico[14].x+10,rc_samptestico[14].y+100,rgb_white);
	DrawText_16(LT("手柄摘机"),rc_samptestico[15].x+10,rc_samptestico[15].y+100,rgb_white);
	DrawText_16(LT("门铃"),rc_samptestico[16].x+20,rc_samptestico[16].y+100,rgb_white);

	DrawText_16(LT("呼叫键"),rc_samptestico[17].x+15,rc_samptestico[17].y+100,rgb_white);
	DrawText_16(LT("开锁键"),rc_samptestico[18].x+15,rc_samptestico[18].y+100,rgb_white);
	DrawText_16(LT("对讲键"),rc_samptestico[19].x+15,rc_samptestico[19].y+100,rgb_white);

	for(i = 0;i<10;i++)
	{
		sprintf(tmptxt,"%d",IoValidLevel[i+8]);
		
		if(IoValidLevel[i+8] == 1)
			DrawTextToRect(rc_samptestico[i+10],tmptxt,rgb_red);
		else
			DrawTextToRect(rc_samptestico[i+10],tmptxt,rgb_white);
	}

	if(isupdate == 1){
		
		g_isUpdated = 1;
		update_rect(0,0,1024,400);
	}
}

static void DrawIoLevelSet()
{
	WRT_Rect rt;
	
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 600;
	FillRect(0xb2136,rt);
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	myIoLevelSet(0);

    JpegDisp(rc_samptestico[0].x,rc_samptestico[0].y,bt_fanhui1,sizeof(bt_fanhui1));
    JpegDisp(rc_samptestico[1].x,rc_samptestico[1].y,bt_guanping1,sizeof(bt_guanping1));
    JpegDisp(rc_samptestico[26].x,rc_samptestico[26].y,menu_ok1,sizeof(menu_ok1));
    
    DrawText(LT("设置有效电平,1-高电平,0-低电平"),320,420,rgb_white);
    DrawText(LT("保存后,需重启设备生效"),350,470,rgb_white);
}

void CreateIoLevelSetWin(){
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;

	pWindow->EventHandler = IoLevelEventHandler;
	pWindow->NotifyReDraw = DrawIoLevelSet;
	pWindow->valid = 1;
	pWindow->type = CHILD;

	for(int i=0;i<18;i++)
	{
		IoValidLevel[i] = p_MorecfgInfo->validlevel[i];//IoValidLevel[i] = pSystemInfo->mySysInfo.validlevel[i];
	}

	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

extern unsigned char menu_wangluopeizhi2[8916];
extern unsigned char menu_wangluopeizhi1[8594];
extern unsigned char menu_benjishezhi2[8967];
extern unsigned char menu_benjishezhi1[8712];

extern unsigned char serach1[5583];
extern unsigned char serach2[5608];

extern unsigned char wifiset1[6684];
extern unsigned char wifiset2[6923];

static const WRT_Rect rc_projectcfgico[8]={
	{173,83, 132,132}, //网络配置
	{446,83,132,132},  //端口配置
	{719,83,132,132},  //功能开关
	{173,305,132,132},//监控配置
	{446,305,132,132},//无线配置
	{719,305,132,132},//网关搜索
	
	{900,531,111,69},			   //返回
	{6,531,111,69}				   //关屏幕 
};

static int ProjectSetupHandler(int x,int y,int status)
{
	int ret = 0;
	WRT_xy xy;
	int pos_x,pos_y;
	int index;
	xy.x = x;
	xy.y = y;

	for(index =0 ;index<8;index++){
		if(IsInside(rc_projectcfgico[index],xy)){
				ret = 1;
				
				pos_x = rc_projectcfgico[index].x;
				pos_y = rc_projectcfgico[index].y;
		
			switch(index){
				case 0: //网络配置
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,menu_wangluopeizhi2,sizeof(menu_wangluopeizhi2));
					}else{
						JpegDisp(pos_x,pos_y,menu_wangluopeizhi1,sizeof(menu_wangluopeizhi1));
						CreateNetConfigWin();
					}
					break;
					
				case 1: //端口配置
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,protconfig2,sizeof(protconfig2));
					}else{
						JpegDisp(pos_x,pos_y,protconfig1,sizeof(protconfig1));
						CreateIoLevelSetWin();
					}
				
					break;
				case 2: //功能开关
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,menu_benjishezhi2,sizeof(menu_benjishezhi2));
					}else{
						JpegDisp(pos_x,pos_y,menu_benjishezhi1,sizeof(menu_benjishezhi1));
						CreateContorlSwitchWin();
					}
				
					break;
				case 3: //监控配置
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,montipc2,sizeof(montipc2));
					}else{
						JpegDisp(pos_x,pos_y,montipc1,sizeof(montipc1));
						CreateIPCameraCfgWin();
					}
					break;
				case 4: //WIFI配置   or    网关搜索
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,wifiset2,sizeof(wifiset2));
					}else{
						JpegDisp(pos_x,pos_y,wifiset1,sizeof(wifiset1));
						CreateWifiCfgWin();
					}
					break;
					
				case 5: //网关搜索
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,serach2,sizeof(serach2));
					}else{
						JpegDisp(pos_x,pos_y,serach1,sizeof(serach1));
						//CreateSmartBoxSearchWin();
					}
					break;
					
				case 6: //返回
					if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
					}else{
							JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
							CloseTopWindow();
					}
					break;

				case 7: //关屏
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
					}else{
						JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
						screen_close();
						CloseAllWindows();
					}
					break;
					
				default:
				
					break;
			}
			break;
		}
	}
	return ret;
}

static void DrawProjectSetup()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 600;
	FillRect(0xb2136,rt);
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	JpegDisp(rc_projectcfgico[0].x,rc_projectcfgico[0].y,menu_wangluopeizhi1,sizeof(menu_wangluopeizhi1));
	JpegDisp(rc_projectcfgico[1].x,rc_projectcfgico[1].y,protconfig1,sizeof(protconfig1));
	JpegDisp(rc_projectcfgico[2].x,rc_projectcfgico[2].y,menu_benjishezhi1,sizeof(menu_benjishezhi1));
	
	JpegDisp(rc_projectcfgico[3].x,rc_projectcfgico[3].y,montipc1,sizeof(montipc1));
	JpegDisp(rc_projectcfgico[4].x,rc_projectcfgico[4].y,wifiset1,sizeof(wifiset1));
	JpegDisp(rc_projectcfgico[5].x,rc_projectcfgico[5].y,serach1,sizeof(serach1));
	
	JpegDisp(rc_projectcfgico[6].x,rc_projectcfgico[6].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_projectcfgico[7].x,rc_projectcfgico[7].y,bt_guanping1,sizeof(bt_guanping1)); 
	
	DrawEnText(LT("网络配置"),rc_projectcfgico[0].w,rc_projectcfgico[0].h,rc_projectcfgico[0].x,rc_projectcfgico[0].y+36);
	DrawEnText(LT("端口配置"),rc_projectcfgico[1].w,rc_projectcfgico[1].h,rc_projectcfgico[1].x,rc_projectcfgico[1].y+36);
	DrawEnText(LT("功能开关"),rc_projectcfgico[2].w,rc_projectcfgico[2].h,rc_projectcfgico[2].x,rc_projectcfgico[2].y+36);

	DrawEnText(LT("监控配置"),rc_projectcfgico[3].w,rc_projectcfgico[3].h,rc_projectcfgico[3].x,rc_projectcfgico[3].y+36);
	DrawEnText(LT("无线配置"),rc_projectcfgico[4].w,rc_projectcfgico[4].h,rc_projectcfgico[4].x,rc_projectcfgico[4].y+36);
	DrawEnText(LT("搜索网关"),rc_projectcfgico[5].w,rc_projectcfgico[5].h,rc_projectcfgico[5].x,rc_projectcfgico[5].y+36);

}

void CreateProjectSetupWin()
{
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;

	pWindow->EventHandler = ProjectSetupHandler;
	pWindow->NotifyReDraw = DrawProjectSetup;
	pWindow->valid = 1;
	pWindow->type = CHILD;

	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}
/////////////////////////////////////////////////////////////

//2015-6-9
static const WRT_Rect switch_ico[9]={

        {220,70,150,50},    //自动抓拍
        {220,120,150,50},   //按键提示音
        {220,170,150,50},   //屏幕保护
        {220,220,150,50},   //是否有挂接模拟分机
        {220,270,150,50},   //中文
        {220,320,150,50},	//访客呼叫优先
        {220,370,150,50},
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕	
};

static const WRT_Rect switch_ico2[9]={
        {750,40,48,48},
        {750,90,48,48},
        {750,140,48,48},
        {750,190,48,48},
        {750,240,48,48},
        {750,290,48,48},
        {750,340,48,48},

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕	    
};

void DrawControlSwitch()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	JpegDisp(switch_ico[7].x,switch_ico[7].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(switch_ico[8].x,switch_ico[8].y,bt_guanping1,sizeof(bt_guanping1));   
	DrawText(LT("是否启用防拆开关"),  switch_ico[0].x,switch_ico[0].y,rgb_white);    

	DrawText(LT("是否启用WIFI"),  switch_ico[1].x,switch_ico[1].y,rgb_white);  

	if(pSystemInfo->mySysInfo.fangchai_switch == 1)		
		JpegDisp(switch_ico2[0].x,switch_ico2[0].y,ico_fangqukai,sizeof(ico_fangqukai));
	else
		JpegDisp(switch_ico2[0].x,switch_ico2[0].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));

	if(pSystemInfo->mySysInfo.wifi_switch == 1)		
		JpegDisp(switch_ico2[1].x,switch_ico2[1].y,ico_fangqukai,sizeof(ico_fangqukai));
	else
		JpegDisp(switch_ico2[1].x,switch_ico2[1].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
}

static void UpdateExtraFun11(int index,int start){
    if(start)
            JpegDisp(switch_ico2[index].x,switch_ico2[index].y,ico_fangqukai,sizeof(ico_fangqukai));
    else
            JpegDisp(switch_ico2[index].x,switch_ico2[index].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
}

int ControlSwitchEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;


        for(index =0;index<9;index++){
                if(IsInside(switch_ico2[index],xy)){
                        pos_x = switch_ico2[index].x;
                        pos_y = switch_ico2[index].y;
                        ret = 1; 
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }		                    
                        switch(index){
							
                 case 0://防拆开关
                         if(status == 1){
                                 StartButtonVocie();
                                 break;
                         }
                         if(pSystemInfo->mySysInfo.fangchai_switch == 1){
                                 pSystemInfo->mySysInfo.fangchai_switch = 0;
                                 UpdateExtraFun11(0,0);
								 
                         }else{
                                 pSystemInfo->mySysInfo.fangchai_switch = 1;
                                 UpdateExtraFun11(0,1);
                         }
                         CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                         
                         save_localhost_config_to_flash(); 
                         SDL_Delay(500);
                         CloseTopWindow();                                	 	
                         break;
						 
                 case 1://wifi qiyong
                         if(status == 1){
                                 StartButtonVocie();
                                 SDL_Delay(150);
                                 break;
                         }
                         if( pSystemInfo->mySysInfo.wifi_switch == 1){
                                 pSystemInfo->mySysInfo.wifi_switch = 0;
                                 UpdateExtraFun11(1,0);
								 tmGpioSetPin(CTL_IO_SET_USB1_OFF,0);
                         }else{
                                 pSystemInfo->mySysInfo.wifi_switch = 1;
                                 UpdateExtraFun11(1,1);
                         }
                         CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                         save_localhost_config_to_flash();
                         SDL_Delay(500);
                         CloseTopWindow();
                         break;
						 
                 case 7: /*返回*/
                         if(status == 1){
                                 StartButtonVocie();
                                 JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                         }else{
                                 JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));  
                                 CloseTopWindow();                   
                         }
                         break;
                 case 8:/*关屏*/
                         if(status == 1){ //按下
                                 StartButtonVocie();
                                 JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                         }else{
                                 JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                 screen_close(); 
                                 CloseAllWindows();
                         }
                         break;
                 default:
                         break;
                        }
                        break;
                }
        }      
        return ret;  		
}

void CreateContorlSwitchWin()
{
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;

	pWindow->EventHandler = ControlSwitchEventHandler;
	pWindow->NotifyReDraw = DrawControlSwitch;
	pWindow->valid = 1;
	pWindow->type = CHILD;

	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

#if 0
///////////////////////////////////search smartbox 2015 4 20
#include "smart-box.h"
#include "wrt_crypt.h"
static int sockfd = -1;
static int ctrlsockfd = -1;
static const unsigned short group_port 	= 12836;
static const unsigned short tcp_port 	= 8899;
static const char *group_addr 			= "255.255.255.255";
#define ROOM_SEARCH_SMARTBOX	0x06F0
#define SMARTBOX_SEARCH_ACK		0x08F0
#define MAX_BUFFER_SIZE 1204
#define KEY_1 "heX6SW4QpJeJx5iK"
static pthread_t broadcast_thread_id = -1;
static pthread_t tcpctrl_thread_id = -1;
static int search_flag = 0;
static unsigned long  search_ip = -1;
static unsigned char smartgatewayid[16];
struct netPacket
{
    char head[4];//WRTI    
    unsigned short cmd;
	int len;
    char srcid[15];
    char dstid[15];
	unsigned char data[16];
};

//计算md5函数
static int GenerateMD5( const void *data, int data_len, unsigned char md5[32] ){
	const char *md5Head = "$0000000000$";
	int len = 12+data_len+1;
	char *temp_data = (char *)malloc( len );
	memcpy( temp_data, md5Head, 12 );
	memcpy( temp_data+12, data, data_len );
	memcpy( temp_data+12+data_len, "$", 1 );
	if( WRTMD5( (unsigned char *)temp_data, len, md5, 32 ) < 0 ){
		printf("Generate WRTMD5 failed!\n");
		free(temp_data);
		return -1;
	}
	md5[24] = '\0';
	#if 1
//	printf("\tmd5:%s\n", md5);
	#endif	
	free(temp_data);
	return 0;
}

static int sendCmd( int sockfd, unsigned short cmd){
	cmd = htons(cmd);
	char send_buff[256];
	char rc4_buf[256];
	memset( send_buff, 0, sizeof(send_buff) );
	const int cmdlen = 56;
	strncpy( send_buff, "WRTI", 4 );
	memcpy( send_buff+4, &cmd, 2 );
	memcpy( send_buff+6, &cmdlen, 4 );
	memcpy( send_buff+10, "010010100100101", 15 );//区别分机广播和手机广播
	memcpy( send_buff+25, "000000000000000", 15 );
	struct sockaddr_in broadaddr;
	broadaddr.sin_family = AF_INET;
	broadaddr.sin_port = htons( group_port );
	broadaddr.sin_addr.s_addr = inet_addr( group_addr );
	unsigned char md5[32];
	memset( md5, '\0', sizeof(md5) );
	GenerateMD5( send_buff, 40, md5 );
	memcpy( send_buff+40, md5, 16 );
	int ret = WRTRC4Encrypt( KEY_1, (unsigned char *)send_buff, cmdlen, (unsigned char *)rc4_buf );
	if(sendto(sockfd, rc4_buf, cmdlen, 0, (struct sockaddr *)&broadaddr, sizeof(struct sockaddr_in)) < 0){  
		printf("send error !\n");
	}
	printf("send ok\n");
	return 0;
}

static int RecvBroadcastMessage( const void *buf, int len,struct sockaddr_in addr){
	const netPacket *ptr = (const netPacket *)buf;
	unsigned char md5[32];
	memset( md5, '\0', sizeof(md5) );
	if( strncmp( ptr->head, "WRTI", 4 ) != 0 ){
		printf("head\n");
		return -1;
	}
	
//	printf("len:%d\n", *((int *)(((char *)buf)+6)));
	#if 1
	GenerateMD5( buf, 54, md5 );
	if( memcmp( md5, buf+54, 16 ) ){
		printf("md5\n");
		return -1;
	}
	#endif
	const unsigned short cmd = ntohs(ptr->cmd);
//	printf("%s->cmd:0x%x\n", __FUNCTION__, cmd);
	switch( cmd ){
		case GATEWAY_ACK_0x08F0:{
			
			memset(smartgatewayid,0,16);
			memcpy(smartgatewayid,buf+44,10);

			unsigned long value = *(unsigned long *)(buf + 40);
			if(value>=1 && value<=15)
			{
				printf("sub smartbox value == %d search_flag = %d\n",value,search_flag);
				break;
			}else
				{
					search_flag = 1;
					printf("main smartbox value == %d search_flag = %d\n",value,search_flag);
				}
			
			search_ip = addr.sin_addr.s_addr;
			
			//printf("old smartgatewayid =%s  old ip = 0x%x\n",smartgatewayid,pSystemInfo->mySysInfo.gatewayip);
			if(strncmp((const char *)smartgatewayid,(const char *)pSystemInfo->mySysInfo.smartgatewayid,10) == 0)
			{
				if(pSystemInfo->mySysInfo.gatewayip != search_ip)
				{
					//printf("update smartgatewayip =0x%x\n",search_ip);
					pSystemInfo->mySysInfo.gatewayip = search_ip;
					UpdateSystemInfo();
				}
			}
			
			//printf("recv smartbox ack......ip=0x%x\n",search_ip);
			break;
		}
		default:
			break;
	}

	
	return 0;
}

static void *RecvBroadcastThread(void *arg){
	struct sockaddr_in peeraddr;
	socklen_t socklen = sizeof(peeraddr);
	char recmsg[MAX_BUFFER_SIZE];
	char decmsg[MAX_BUFFER_SIZE];
	int nread = -1;
	const netPacket *ptr = NULL;
	unsigned short cmd = 0;
	while(1){
		memset( recmsg, 0, MAX_BUFFER_SIZE );
		nread = recvfrom( sockfd, recmsg, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&peeraddr, &socklen );  
		if(nread < 0){
			printf("recvfrom err in udptask!/n");  
			if(sockfd)
				continue;
		}else{
			if( peeraddr.sin_addr.s_addr ==  pSystemInfo->LocalSetting.privateinfo.LocalIP)
			{	
				;//printf("recv local----------%d-----from----smart-gateway\n",nread);
				continue;
			}else
				;//printf("recv remote----------%d-----from----smart-gateway\n",nread);
			if(WRTRC4Decrypt( KEY_1, (unsigned char *)recmsg, nread, (unsigned char *)decmsg ) < 0){
				printf("WRTRC4Decrypt failed!\n");
				continue;
			}
			//printf("0x%x\n",peeraddr.sin_addr.s_addr);
			if( nread != 70 ){
				//printf("error packet!\n");
				continue;
			}
			for(int i = 0;i<nread;i++)
			{
				printf("buf[%d]=%x \t",i,decmsg[i]);
			}
			printf("\n");
			RecvBroadcastMessage( decmsg, nread,peeraddr);
		}
	}
	return NULL;
}

void RoomSendSearchPack()
{
	search_flag = 0;
	search_ip = 0;
	sendCmd(sockfd,ROOM_SEARCH_SMARTBOX);
}

void InitBroadcastProcess(){
	printf("\t%s\n",__FUNCTION__);
	struct sockaddr_in peeraddr;
	struct in_addr ia;
	char recmsg[MAX_BUFFER_SIZE];
	unsigned int socklen, n;
	struct hostent *group;
	struct ip_mreq mreq;
	sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
	if( sockfd < 0 ){
		printf("socket creating err!/n");
		exit(1);
	}
	socklen = sizeof(struct sockaddr_in);
	memset( &peeraddr, 0, socklen );
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons( group_port );
	peeraddr.sin_addr.s_addr = htonl( INADDR_ANY );
	if( bind(sockfd, (struct sockaddr *)&peeraddr, sizeof(struct sockaddr_in)) == -1){
		printf("Bind error/n");
		exit(0);
	}
#ifdef GROUP
	memset( &mreq, 0, sizeof(struct ip_mreq) );
	mreq.imr_multiaddr.s_addr = inet_addr( group_addr );	 
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);    
	if(setsockopt(sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0){	
	   perror("setsockopt");	
	   exit(1);    
	}	
#endif
	int on = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on)) < 0){	
	   perror("setsockopt");	
	   exit(1);    
	}		
	pthread_attr_t attr;
	int ret;
	pthread_attr_init( &attr );
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
	ret = pthread_create( &broadcast_thread_id, &attr, RecvBroadcastThread, NULL );
	pthread_attr_destroy( &attr );
	if( ret!=0 ){
		printf( "Create fifo recv pthread error!\n" );
	}
}

static const WRT_Rect rc_searchico[6]={
	{150,175,132,132},			//search button
	{0,0,0,0},
	{0,0,0,0},
	{500,200,200,50},			//
	{900,531,111,69},			   //返回
	{6,531,111,69}				   //关屏幕 
};
int search_ok(int param){
	int i =0;
	pSystemInfo->mySysInfo.gatewayip = search_ip;
	memset(pSystemInfo->mySysInfo.smartgatewayid,0,12);
	memcpy(pSystemInfo->mySysInfo.smartgatewayid,smartgatewayid,10);
	UpdateSystemInfo();
	return 0;                
}
int search_err(int param){
	return 0;                
}
static int SearchSmartBoxHandler(int x,int y,int status)
{
	int ret = 0;
	WRT_xy xy;
	int pos_x,pos_y;
	int index;
	xy.x = x;
	xy.y = y;
	unsigned char disip[16];
	for(index =0 ;index<6;index++){
		if(IsInside(rc_searchico[index],xy)){
				ret = 1;
				pos_x = rc_searchico[index].x;
				pos_y = rc_searchico[index].y;
			switch(index){
				case 0: //网络配置
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,menu_wangluopeizhi2,sizeof(menu_wangluopeizhi2));
					}else{
						JpegDisp(pos_x,pos_y,menu_wangluopeizhi1,sizeof(menu_wangluopeizhi1));
						CreateDialogWin2(LT("搜索中..."),MB_NONE,NULL,NULL);
						RoomSendSearchPack();
						SDL_Delay(2000);
						CloseTopWindow();
						printf("button serach_flag = %d\n",search_flag);
						if(search_flag)
						{
							toinet_addr(search_ip,disip);
							CreateDialogWin2((char *)disip,MB_OK|MB_CANCEL,search_ok,search_err);
						}else
							CreateDialogWin2(LT("搜索失败"),MB_OK,NULL,NULL);
					}
					break;
				case 1: //端口配置
					break;
				case 2: //监控配置
					break;
				case 3: //WIFI配置   or    网关搜索
					break;
				case 4: //返回
					if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
					}else{
							JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
							CloseTopWindow();
					}
					break;
				case 5: //关屏
					if(status == 1){ //按下
						StartButtonVocie();
						JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
					}else{
						JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
						screen_close();
						CloseAllWindows();
					}
					break;
				default:
					break;
			}
			break;
		}
	}
	return ret;
}

static void DrawSearchSmartBox()
{
	char smartboxip[16];
	char text[16];
	WRT_Rect rt;
	WRT_Rect temp;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 600;
	FillRect(0xb2136,rt);
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
	JpegDisp(rc_searchico[0].x,rc_searchico[0].y,menu_wangluopeizhi1,sizeof(menu_wangluopeizhi1));
	DrawEnText(LT("搜索网关"),rc_searchico[0].w,rc_searchico[0].h,rc_searchico[0].x,rc_searchico[0].y+36);
	toinet_addr(pSystemInfo->mySysInfo.gatewayip,(unsigned char*)smartboxip);
	temp = rc_searchico[3];
 	int color = rgb_white;
	DrawRect(temp,2,color);
	DeflateRect(&temp,4);
	FillRect(rgb_white,temp);
	DrawText(smartboxip,rc_searchico[3].x+15,rc_searchico[3].y+36,rgb_black);

	memset(text,0,16);
	sprintf(text,"Key:%02x%02x%02x%02x%02x%02x",pSystemInfo->BootInfo.MAC[0],
                pSystemInfo->BootInfo.MAC[1],pSystemInfo->BootInfo.MAC[2],pSystemInfo->BootInfo.MAC[3],
                pSystemInfo->BootInfo.MAC[4],pSystemInfo->BootInfo.MAC[5]);
	DrawEnText(text,rc_searchico[0].w,rc_searchico[0].h,rc_searchico[0].x+300,rc_searchico[0].y+96);
	JpegDisp(rc_searchico[4].x,rc_searchico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_searchico[5].x,rc_searchico[5].y,bt_guanping1,sizeof(bt_guanping1)); 
	
}

void CreateSmartBoxSearchWin()
{
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = SearchSmartBoxHandler;
	pWindow->NotifyReDraw = DrawSearchSmartBox;
	pWindow->valid = 1;
	pWindow->type = CHILD;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}
#endif
//======================================
//		2014-3-31
//======================================

void DrawMonitorMain(){//监视主界面 整界面画

	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

    if(m_monitor_page > 1)
    	JpegDisp(rc_monitormenuico[4].x,rc_monitormenuico[4].y,menu_xiafan1,sizeof(menu_xiafan1));

    JpegDisp(rc_monitormenuico[5].x,rc_monitormenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));

    JpegDisp(rc_monitormenuico[6].x,rc_monitormenuico[6].y,bt_guanping1,sizeof(bt_guanping1));

	DrawMonitorInfo_main();

}

extern char g_url[100];
int  MonitorEventHandler(int x,int y,int status){
        WRT_xy xy;
        WRT_Rect rect;
        int index =0;
        int ret = 0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        
        for(index=0;index<7;index++){
                if(IsInside(rc_monitormenuico[index],xy)){
                        ret = 1;
                        pos_x = rc_monitormenuico[index].x;
                        pos_y = rc_monitormenuico[index].y;
                        switch(index){
                case 3: //上翻
                		if(m_monitor_page == 1 || m_cur_monitor_page ==1)
						{
							printf("上翻无效\n");
							break;
						}
                           
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                
                                g_isUpdated = 0;
                                DrawMonitorInfo_up();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);

                        }
                        break;
                case 4: //下翻
                        if(m_cur_monitor_page > m_monitor_page || m_cur_monitor_page == m_monitor_page)
                        {
                        	printf("下翻无效\n");
                        	break;
                        }
                                
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
                                
                                g_isUpdated = 0;
                                
                                DrawMonitorInfo_down();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);

                        }
                        break;
                case 5: /*返回*/
                		printf("返回上一界面\n");
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                ReleaseMonitorInfo();
                                CloseTopWindow();
                        }
                        break;
                case 6:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                default:
                        int count = (m_cur_monitor_page-1)*3;
                        if((index + count) < m_monitor_totalcount){

                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,ico_jianshi2,sizeof(ico_jianshi2));
                                }else{
                                        JpegDisp(pos_x,pos_y,ico_jianshi1,sizeof(ico_jianshi1));
                                        
                                        if(m_MonitorDev[(index + count)].monitorDev.IP == 0 || m_MonitorDev[(index + count)].monitorDev.IP == pSystemInfo->LocalSetting.privateinfo.LocalIP){
                                            CreateDialogWin2(LT("监视地址不对"),MB_OK,NULL,NULL);
                                            break;
                                        }
                                        if(pSystemInfo->mySysInfo.isbomb == USE_BOMB){
                                            CreateDialogWin2(LT("版本太低,请升级"),MB_OK,NULL,NULL);
                                            break;
                                        }
										if(m_MonitorDev[(index + count) ].type == MONITOR_TYPE_IPCAMERA)
										{
										#if 0
	                                        CreateMonitorSubWin(1);
											g_current_monitor_index = (index + count+1);
											int onvifret ;
											unsigned char ipcip[256];
											int  Onvif_Auth_Enable = 0;
											char Onvif_Auth_Name[16];
											char Onvif_Auth_Passwd[16];
											
											toinet_addr(m_MonitorDev[(index + count)].monitorDev.IP,ipcip);
											Onvif_Auth_Enable = GetOnvifAuthInfoByIp(m_MonitorDev[(index + count)].monitorDev.IP,Onvif_Auth_Name,Onvif_Auth_Passwd);
											onvifret = Wrt_set_monitor_camera_ip((char *)ipcip,Onvif_Auth_Enable,Onvif_Auth_Name,Onvif_Auth_Passwd,g_url);
											//printf("authflag = %d || authname = %s || authpasswd = %s || g_url = %s\n",Onvif_Auth_Enable,Onvif_Auth_Name,Onvif_Auth_Passwd,g_url);

											if(onvifret == 0)
											{
												event[0] = IPCAMERA_MONITOR_START;
												event[1] = 0;
												event[2] = 0;
												event[3] = 0;
												wrthost_send_cmd(event);
											}else
											{
												SDL_Delay(1000);
												CloseTopWindow();
												return 0;
											}
                                            #endif
										}else
										{
	                                        CreateMonitorSubWin(0);
                                        	g_current_monitor_index = (index + count+1);
                                        
                                        	wrthost_set_montip(m_MonitorDev[(index + count)].monitorDev.IP);
                                        	
                                        	event[0] = ROOM_STARTMONT;
                                        	event[1] = m_MonitorDev[(index + count) ].type;
                                        	event[2] = GetMonitorIndex(event[1],(index+count));
                                        	event[3] = 0;
                                        	wrthost_send_cmd(event);
                                        }
                                        OSD_DrawText(m_MonitorDev[(index + count)].monitorDev.name,100,35,rgb_white);
										
                                }
                        }
                        break;
                        }
                        break;
                }
        }
        return ret;
}

void CreateMonitorSubWin(int type){
        Window* pWindow = New_Window();
        if(pWindow == NULL)
	        return ;
	    g_is_hangup = false;
	    g_is_monitor_flag = 1;
	    g_ipcamera_flag = type ;
	    g_monttype = type;
	    g_current_monitor_index = 0;
	    g_phone_or_externdevice_holdon = 1;
        pWindow->EventHandler = MonitorSubEventHandler;
        pWindow->NotifyReDraw = DrawMonitorSubMain;
        pWindow->valid = 1;
        pWindow->type = CALLING;
        pWindow->CloseWindow = CloseCallWin;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawMonitorSubMain(){
	
	//OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));//监视界面的框框

	

    if(pSystemInfo->mySysInfo.screensize == 0x10 || g_ipcamera_flag){
    	OSD_JpegDisp(0,0,menu_call_back10,sizeof(menu_call_back10));
    	JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
    	DrawText(LT("正在连接中..."),110,100,rgb_white);
    }else{
    	WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 600;
		FillRect(rgb_black,rt);

		JpegDisp(854,0,menu_call_back10_2,sizeof(menu_call_back10_2));
    	//DrawText(LT("正在连接中..."),110,100,rgb_white);
	}
  
    OSD_JpegDisp(rc_callico[0].x,rc_callico[0].y,menu_duijiang3,sizeof(menu_duijiang3));
    OSD_JpegDisp(rc_callico[1].x,rc_callico[1].y,menu_kaisuo3,sizeof(menu_kaisuo3));
    OSD_JpegDisp(rc_callico[2].x,rc_callico[2].y,menu_zhuapai3,sizeof(menu_zhuapai3));
    OSD_JpegDisp(rc_callico[3].x,rc_callico[3].y,menu_luxiang3,sizeof(menu_luxiang3));
    OSD_JpegDisp(rc_callico[4].x,rc_callico[4].y,menu_jingyin3,sizeof(menu_jingyin3));
    OSD_JpegDisp(rc_callico[5].x,rc_callico[5].y,menu_guaji3,sizeof(menu_guaji3));
    OSD_JpegDisp(rc_callico[6].x,rc_callico[6].y,menu_tuichu1,sizeof(menu_tuichu1));
    
}    

//事件处理，单个画元素
int  MonitorSubEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        if(g_is_hangup)
        	return 0;
        if(IsInside(rc_callico[6],xy)){
                ret = 1;
                if(status == 1)
                        g_rc_status[6] = true;
                else{
                        if(g_rc_status[6] == false){
                                ret = 0;
                                return ret;
                        }
                        g_rc_status[6] = false;
                }
                pos_x = rc_callico[6].x;
                pos_y = rc_callico[6].y;
                
                
                if(status == 1){
                        StartButtonVocie();

                        JpegDisp(pos_x,pos_y,menu_tuichu2,sizeof(menu_tuichu2));
                        
                }else{

                        JpegDisp(pos_x,pos_y,menu_tuichu1,sizeof(menu_tuichu1));
                        
						SDL_Delay(300);
						if(g_monttype == 0)
                        event[0] = ROOM_STOPMONT;
                        else
                        	event[0] = IPCAMERA_MONITOR_END;
                        event[1] = event[2] = event[3]=0;
                        if(g_is_hangup == false)
                        {
                        	wrthost_send_cmd(event);
                        	g_is_hangup = true;
                        }

                }
        }
        return ret;
}


//////////////////////////////////////////
int add_telephone_1(int param){
        TELEPHONE* pTemp = get_idle_telephone();
        if(pTemp){
                strcpy(pTemp->room_number,g_roomnumber);
                add_telephone(pTemp);
        }
        
        return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//
static WRT_Rect rcRule[9]={
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {650,100,300,400},    //数字符号所在区域
        {780,531,111,69},
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

static int inputcallrule[5]={0};

static int g_cur_select_edit = 0;
static int g_pre_select_edit = 0;



#define SCREEN_WIDTH 400;
#define SCREEN_HEIGHT 400;

static void updateCallRuleWin(){
        char text[10];
        int xpos=0;
        int ypos=0;
        WRT_Rect tmprect;
        memset(text,0,10);
        switch(g_cur_select_edit){
    case 0:
            if(inputcallrule[g_cur_select_edit] > 2)
                    inputcallrule[g_cur_select_edit] = 2;
            break;
    case 1:
            if(inputcallrule[g_cur_select_edit] > 3)
                    inputcallrule[g_cur_select_edit] = 3;
            break;
    case 2:
            if(inputcallrule[g_cur_select_edit] > 2)
                    inputcallrule[g_cur_select_edit] = 2;
            break;
    case 3:
            if(inputcallrule[g_cur_select_edit] > 3)
                    inputcallrule[g_cur_select_edit] = 3;
            break;
    case 4:
            if(inputcallrule[g_cur_select_edit] > 3)
                    inputcallrule[g_cur_select_edit] = 3;
            break;
        }
        sprintf(text,"%d",inputcallrule[g_cur_select_edit]);
        DrawRect(rcRule[g_pre_select_edit],2,rgb_white);
        DrawRect(rcRule[g_cur_select_edit],2,rgb_yellow);
        xpos = rcRule[g_cur_select_edit].x+5;
        ypos = rcRule[g_cur_select_edit].y+rcRule[g_cur_select_edit].h-10;
        tmprect = rcRule[g_cur_select_edit];
        DeflateRect(&tmprect,2);
        FillRect(BACK_COLOR,tmprect);
        DrawText(text,xpos,ypos,rgb_white);
}


static int rulehandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int tmpY =  0;
        char* pTemp = 0;
        int offset = 0;
        int offset2 = 0;
        int startx = rcRule[5].x;
        int starty = rcRule[5].y;
        char c[2];
        c[0] = '\0';
        c[1] = '\0';

        for(i=0; i<4;i++)
                for(j=0; j< 3;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){
                                count = (i*3)+j;
                                ret =1;
                                if(status == 1)
                                        g_rc_status[count] = true;
                                else{
                                        if(g_rc_status[count] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count] = false;
                                }
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer1[count],pIPNumberSize1[count]);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer[count],pIPNumberSize[count]);

                                        if(count != 9 && count != 11){
                                                if(g_inputcount > 1)
                                                        break;
                                                c[0] = g_number[count];
                                                strcpy(g_roomnumber,c);
                                                // g_inputcount++;
                                                inputcallrule[g_cur_select_edit] = 0;
                                                inputcallrule[g_cur_select_edit] = atoi(g_roomnumber);
                                                if(g_inputcount == 0){
                                                        updateCallRuleWin();
                                                        g_pre_select_edit = g_cur_select_edit;
                                                        g_cur_select_edit ++;
                                                        if(g_cur_select_edit > 4)
                                                                g_cur_select_edit = 0;
                                                        memset(g_roomnumber,0,MAX_ROOM_BITS);
                                                        sprintf(g_roomnumber,"%d",inputcallrule[g_cur_select_edit]);
                                                        // g_inputcount =strlen(g_curinput);
                                                        g_inputcount = 0;
                                                }
                                                g_isUpdated = 0;
                                                updateCallRuleWin();
                                                update_rect(rcRule[g_pre_select_edit].x,rcRule[g_pre_select_edit].y,rcRule[g_pre_select_edit].w,rcRule[g_pre_select_edit].h);
                                                update_rect(rcRule[g_cur_select_edit].x,rcRule[g_cur_select_edit].y,rcRule[g_cur_select_edit].w,rcRule[g_cur_select_edit].h);
                                                g_isUpdated = 1;
                                        }
                                        break;
                                }
                        }
                }
                return ret;
}




void CreateCallRuleWin(){
        g_inputcount = 0;
        memset(g_roomnumber,0,MAX_ROOM_BITS);
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        g_pre_select_edit = 0;
        g_cur_select_edit = 0;
        pWindow->EventHandler = CallRuleEventHandler;
        pWindow->NotifyReDraw = DrawCallRuleMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        if(pIPNumberPointer == NULL && pIPNumberPointer1 == NULL){
                GetNumberJpgPointer(&pIPNumberPointer,&pIPNumberPointer1);
                GetNumberJpegSizePointer(&pIPNumberSize,&pIPNumberSize1);
        }
        if(pSystemInfo){
                memcpy(inputcallrule,pSystemInfo->mySysInfo.idrule,sizeof(inputcallrule));
        }
        pWindow->NotifyReDraw2(pWindow);
}

void DrawCallRuleMain()
{
        WRT_Rect curbox;
        unsigned long day,Month,years, dates,times;
        unsigned long min,second,Hour;
        char text[10];
        int i=0;
        int xpos = 0;
        int ypos = 0;
        char* text1 = LT("区号位数(0-2):");
        char* text2 = LT("栋号位数(0-3):");
        char* text3 = LT("单元号位数(0-2):");
        char* text4 = LT("层号位数(0-3):");
        char* text5 = LT("房间号位数(0-3):");
        xpos =(400 - GetTextWidth(text3,0))/2;
        if(xpos < 0)
                xpos = 10;
                
				WRT_Rect rt;
				rt.x = 0;
				rt.y = 0;
				rt.w =1024;
				rt.h = 530;
				FillRect(0xb2136,rt);
				
				JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));

        DrawText(text1,xpos,120,rgb_white);
        DrawText(text2,xpos,170,rgb_white);
        DrawText(text3,xpos,220,rgb_white);
        DrawText(text4,xpos,270,rgb_white);
        DrawText(text5,xpos,320,rgb_white);
        rcRule[0].x = xpos + GetTextWidth(text3,0)+30;
        rcRule[0].y = 90;
        rcRule[0].w = 100;
        rcRule[0].h = 40;
        rcRule[1] = rcRule[0];
        rcRule[1].y = 140;
        rcRule[2] = rcRule[1];
        rcRule[2].y = 190;
        rcRule[3] = rcRule[1];
        rcRule[3].y = 240;
        rcRule[4] = rcRule[1];
        rcRule[4].y = 290;

        DrawRect(rcRule[0],2,rgb_white);
        DrawRect(rcRule[1],2,rgb_white);
        DrawRect(rcRule[2],2,rgb_white);
        DrawRect(rcRule[3],2,rgb_white);
        DrawRect(rcRule[4],2,rgb_white);
        for(i =0; i<5; i++){
                memset(text,0,10);
                sprintf(text,"%d",inputcallrule[i]);
                xpos = rcRule[i].x+5;
                ypos = rcRule[i].y+rcRule[i].h-10;
                DrawText(text,xpos,ypos,rgb_white);
        }

        JpegDisp(rcRule[6].x,rcRule[6].y,menu_ok1,sizeof(menu_ok1));
        JpegDisp(rcRule[7].x,rcRule[7].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcRule[8].x,rcRule[8].y,bt_guanping1,sizeof(bt_guanping1));
        int j =0;
        int startx = rcRule[5].x;
        int starty = rcRule[5].y;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pIPNumberPointer[(i*3)+j],pIPNumberSize[(i*3)+j]);
                }
                updateCallRuleWin();


}

int  CallRuleEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<9;index++){
                if(IsInside(rcRule[index],xy)){
                        pos_x = rcRule[index].x;
                        pos_y = rcRule[index].y;
                        ret = 1;
                        switch(index){
            case 5:
                    //数字表
                    ret = rulehandler(x,y,status);
                    break;
            case 6://确定
                    if(status == 1){
                            StartButtonVocie();
                            JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                    }else{
                            JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                            int xpos,ypos;
                            char text[10];
                            int i;
                            for(i =0; i<5; i++){
					                memset(text,0,10);
					                sprintf(text,"%d",inputcallrule[i]);
					                xpos = rcRule[i].x+5;
					                ypos = rcRule[i].y+rcRule[i].h-10;
					                DrawText(text,xpos,ypos,rgb_white);
					        }
                            CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                            if(pSystemInfo){
                                    memcpy(pSystemInfo->mySysInfo.idrule,inputcallrule,sizeof(inputcallrule));
                                    save_localhost_config_to_flash();
                            }
                            CloseWindow();
                            CreateDialogWin2(LT("设置完毕!"),MB_OK,NULL,NULL);

                    }
                    break;
            case 7: /*返回*/
                    if(status == 1){
                            StartButtonVocie();
                            JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                    }else{
                            JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                            CloseTopWindow();
                    }
                    break;
            case 8:/*关屏*/
                    if(status == 1){ //按下
                            StartButtonVocie();
                            JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                    }else{
                            JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                            screen_close();
                            CloseAllWindows();
                    }
                    break;
            default:
                    WRT_DEBUG("g_cur_select_edit = %d,index = %d ",g_cur_select_edit,index);
                    g_pre_select_edit = g_cur_select_edit;
                    g_cur_select_edit = index;
                    g_isUpdated = 0;
                    updateCallRuleWin();
                    update_rect(rcRule[g_pre_select_edit].x,rcRule[g_pre_select_edit].y,rcRule[g_pre_select_edit].w,rcRule[g_pre_select_edit].h);
                    update_rect(rcRule[g_cur_select_edit].x,rcRule[g_cur_select_edit].y,rcRule[g_cur_select_edit].w,rcRule[g_cur_select_edit].h);
                    g_isUpdated = 1;
                    break;
                        }
                        break;
                }
        }
        return ret;
}

static const WRT_Rect rc_syscmdico[1]={
        {0,0,1024,600},              // 监视点1
};

static void DrawSysCmd()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 600;
	FillRect(0xb2136,rt);
	FILE* file = NULL;
    int size1 = 0;
	int size;
    struct stat sbuf;
    if(stat("/home/wrt/cmd.txt",&sbuf) < 0)
            return;
    size1 = sbuf.st_size;
    if(size1 > ((2*1024*1024)+4))
            return;
    unsigned char* tmpbuf = (unsigned char*)ZENMALLOC(size1);
    if(tmpbuf == NULL)
            return ;
    file = fopen("/home/wrt/cmd.txt","rb");
    if(file != NULL){
            size = fread(tmpbuf,1,size1,file);
            if(size != size1){
                    ZENFREE(tmpbuf);
                    fclose(file);
                    return ;
            }
            fclose(file);
            file = NULL;
    }
	if(tmpbuf)
	{
		int cmdlen = size/100;
		int molen = size%100;
		printf("cmdlen = %d\n",cmdlen);
		int yoffset = 0;
		char showbuf[100];
		while(cmdlen--)
		{
			memset(showbuf,0,100);
			memcpy(showbuf,tmpbuf+yoffset*100,100);
			printf("showbuf %s\n",showbuf);
			yoffset++;
			DrawText_16((char *)showbuf, 10,10+yoffset*30,rgb_white);
			usleep(10*1000);
		}
		memset(showbuf,0,100);
		memcpy(showbuf,tmpbuf+yoffset*100,molen);
		printf("showbuf %s\n",showbuf);
		yoffset++;
		DrawText_16((char *)showbuf, 10,10+yoffset*30,rgb_white);
		ZENFREE(tmpbuf);
	}
}
static int SysCmdHandler(int x,int y,int status){
	int ret = 0;
	WRT_xy xy;
	int pos_x,pos_y;
	int index;
	xy.x = x;
	xy.y = y;
	for(index =0 ;index<1;index++){
		if(IsInside(rc_syscmdico[index],xy)){
			ret = 1;
			pos_x = rc_syscmdico[index].x;
			pos_y = rc_syscmdico[index].y;
			switch(index){
				case 0: //网络配置
					if(status == 1){ //按下
						StartButtonVocie();
					}else{
						CloseTopWindow();
					}
					break;
				default:
					break;
			}
			break;
		}
	}
	return ret;
}
void CreateSysCmdWin()
{
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = SysCmdHandler;
	pWindow->NotifyReDraw = DrawSysCmd;
	pWindow->valid = 1;
	pWindow->type = CHILD;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}
static void DrawDisplay()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 600;
	FillRect(0xb2136,rt);
	int yoffset = 0;
	DrawText_16((char *)"IFCONFIG:  执行ifconfig -a", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"PINGMYSELF:  执行ping本机", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"PINGGATEWAY:  执行ping网关", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"PINGCENTER:  执行ping中心", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"RESETNETDOWN:  down网络", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"RESETNETUP:  up网络", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"SYSCMD:  查看执行测试结果", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"RULE:  呼叫格式", 100,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"ROUTE:  路由", 100,100+(yoffset++)*30,rgb_white);
	yoffset = 0;
	DrawText_16((char *)"SHOW1:  老版快速测试", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"SHOW2:  清楚数据，恢复出厂",500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"SHOW3:  新版快速测试", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"RESET2008:  重启分机", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"28:  IP配置界面", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"29:  主门口机", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"30:  中心", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"32:  时间配置", 500,100+(yoffset++)*30,rgb_white);
	DrawText_16((char *)"NOJIAJU2008:  家居开关", 500,100+(yoffset++)*30,rgb_white);
}
static int DisplayHandler(int x,int y,int status){
	int ret = 0;
	WRT_xy xy;
	int pos_x,pos_y;
	int index;
	xy.x = x;
	xy.y = y;
	for(index =0 ;index<1;index++){
		if(IsInside(rc_syscmdico[index],xy)){
			ret = 1;
			pos_x = rc_syscmdico[index].x;
			pos_y = rc_syscmdico[index].y;
			switch(index){
				case 0: //网络配置
					if(status == 1){ //按下
						StartButtonVocie();
					}else{
						CloseTopWindow();
					}
					break;
				default:
					break;
			}
			break;
		}
	}
	return ret;
}
void CreateDisplayWin()
{
	Window* pWindow = New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = DisplayHandler;
	pWindow->NotifyReDraw = DrawDisplay;
	pWindow->valid = 1;
	pWindow->type = CHILD;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

#ifdef USE_DESKTOP
static FuncEnter  g_curhandlefunc = NULL;
static const int g_shortcutx = 200;
static const int g_shortcuty = 150;
static WRT_Rect rcDesktopico[10]={
        {12,40,68,60},
        {112,40,68,60},
        {212,40,68,60},
        {312,40,68,60},

        {12,112,68,60},
        {112,112,68,60},
        {212,112,68,60},
        {312,112,68,60},

        {358,9,21,21}, //close
        {10,10,120,20} //text
       
};

void DrawTipInfo(char* name)
{
        WRT_Rect rctip;
        rctip.x = 20+g_shortcutx;
        rctip.y = 183+g_shortcuty;
        rctip.w = 300;
        rctip.h = 20;
        FillRect(0x00A1A1A1,rctip);
        DrawText_16(name, rctip.x, rctip.y + 20,rgb_black);
}

void CloseDesktopPos()
{
        g_curhandlefunc = NULL;
}

void CreateSelectDesktopPos()
{

        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SelectDesktopEventHandler;
        pWindow->NotifyReDraw = DrawSelectDesktopPos;
        pWindow->CloseWindow = CloseDesktopPos;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}
void DrawSelectDesktopPos()
{
        int i = 0;
        JpegDisp(g_shortcutx,g_shortcuty,shortcut_back,sizeof(shortcut_back));
        JpegDisp(rcDesktopico[8].x + g_shortcutx,rcDesktopico[8].y + g_shortcuty,shortcut_close1,sizeof(shortcut_close1));
        for(i = 0; i<8 ;i++)
        {
                JpegDisp(rcDesktopico[i].x+g_shortcutx,rcDesktopico[i].y + g_shortcuty,shortcut_up,sizeof(shortcut_up));
        }
        DrawText_16(LT("请选择桌面位置"),rcDesktopico[9].x+g_shortcutx,rcDesktopico[9].y+10+g_shortcuty,rgb_black);
}

int SelectDesktopEventHandler(int x,int y,int status)
{
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x-g_shortcutx;
        xy.y = y-g_shortcuty;
        for(index =0;index<10;index++){
                if(IsInside(rcDesktopico[index],xy)){
                        pos_x = rcDesktopico[index].x;
                        pos_y = rcDesktopico[index].y;
                        ret = 1;
                        switch(index)
                        {
                        case 8:/*关屏*/
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x + g_shortcutx,pos_y + g_shortcuty,shortcut_close2,sizeof(shortcut_close2));
                                }else{
                                        JpegDisp(pos_x + g_shortcutx,pos_y + g_shortcuty,shortcut_close1,sizeof(shortcut_close1));
                                        CloseTopWindow();
                                }
                                break;
                        default:
                                if(index > 7 )
                                        break;
                                if(index == 3)
                                        break;
                                if(index == 6)
                                        break;
                                if(status == 1)
                                {
                                        JpegDisp(pos_x+g_shortcutx,pos_y + g_shortcuty,shortcut_down,sizeof(shortcut_down));
                                }else{
                                        //处理，添加快捷键。
                                         JpegDisp(pos_x+g_shortcutx,pos_y + g_shortcuty,shortcut_up,sizeof(shortcut_up));
                                         SHORTCUTDATA* _shortcut = FindShortcut2(g_curhandlefunc);
                                         if(_shortcut)
                                         {
                                                 if(IsCurrentShortcut(_shortcut) == 1){
                                                        DrawTipInfo(LT("已存在"));
                                                        break;
                                                 }
                                                ReplaceCurrentShortcut(index+1,_shortcut);
                                         }else{
                                                 DrawTipInfo(LT("配置失败"));
                                                 break;
                                         }
                                         CloseAllWindows();
                                }
                                break;
                        }
                        break;
                }
        }
        return ret;
}
void SetCurrentHandlerCallback(FuncEnter _func)
{
	g_curhandlefunc = _func;
}
#endif
