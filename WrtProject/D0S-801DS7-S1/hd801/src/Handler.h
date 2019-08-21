#ifndef _HANDLER_H_
#define _HANDLER_H_

#include "SDL.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef __cplusplus
extern "C"{
#endif
#include "wrt_log.h"
#include "info_list_handler.h"   
#include "zenmalloc.h"
        //#include "tmSysCfgRoom.h"
#include "tmSysConfig.h"

#include "tmJiaju.h"

#ifdef WRT_MORE_ROOM
#include "wrt_more_room_handle.h"
#endif



#define   closesocket close
#define   ioctlsocket ioctl


//#define USE_DESKTOP 1

#ifdef USE_DESKTOP
#undef USE_DESKTOP
#endif

#ifdef USE_DESKTOP
#include "Destktop.h"

        void CreateDesktop();
        void DrawDesktop();
        int DesktopEventHandle(int x,int y,int status);

        void AddMainShortcut();
#endif

#define TEST_FFS 1                                     //测试FORMAT
        //#define USED_FFS 1                                     //使用FFS
        /////////////////////////////////////////////////////////////////////
        //config 
#define HAVE_SCREEN_KEEP             1                //是否有屏幕保护
#define HAVE_JIAJU_SAFE              1               //家居是否带安保。
#define HAVEKEYBOARD	             1
        //#undef  HAVE_SCREEN_KEEP 
        ////////////////////////////////////////////////////////////////////
#define rgb_black 0xFF000000
#define rgb_white 0xFFffffff
#define rgb_yellow 0xFFffff00
#define rgb_red   0xFFff0000
#define rgb_blue   0xFF0000ff
#define rgb_green  0xFF00ff00

#define rgb_colorKey 0xFF404040

//#define CHINESE 0x0
//#define ENGLISH 0x1
        extern bool g_rc_status[42];

        typedef struct _tagWRT_Rect{
                int x;
                int y; 
                int w;
                int h;
        }WRT_Rect;

        typedef struct _tagWRT_xy{
                int x;
                int y;
        }WRT_xy;

        typedef struct _MonitorName{
                int  idle ;
                char name[40];
        }MONITORNAME;

        ///////////////////////////////////////////////////////////////////////////////////////////
#define MAX_ROOM_TYPE   5
#define MAX_WIRING_TYPE 5
#define MAX_LAMP        4

        //房间类别('客厅‘，’主卧‘等等)
        /*
                char* name;
                char* name_map;
                int   id;
        }ROOMTYPE;

        //电器类别
        typedef struct _tagRoomWiring{
                char* name;
                int   id;
        }ROOMWIRING;


        typedef struct _tagLAMPINFO{
                char name[10];
                bool state;
        }LAMPINFO;

        typedef struct _tagSETINFO{
                char name[10];              //名字
                char controlname[20];       //控制器名字
                char number[12];            //编号
                char riunumber[12];        //riu编号
        }SETINFO;

        typedef struct _tagTVAIRSETINFO{
                char name[10];              //名字
                char controlname[20];       //控制器名字
                // char number[12];            //编号
                // char riunumber[12];        //riu编号	
                char keynumber[9][12];     //按键编码(从上到下（音量+，音量-，频道+,频道-，av,open/close).
                //如果是空调（从上到下（温度+，温度—，模式+，模式-，强度+，强度-，开/关）
        }TVAIRSETINFO;
        */
        /////////////////////////////////////////////////////////////////////////
        
        extern SDL_Surface*  g_pScreenSurface;
        extern SDL_Surface*  tmp_g_pScreenSurface;
//        extern SDL_Surface*  tmp_g_pScreenSurface1;
        
        
typedef unsigned char UInt8;        
typedef unsigned long UInt32;


        /////////////////////////////////////////////////////////////////////////
        //为Dialog service
#define DEL_MESSAGE                    1         /*当前是要删除短消息*/
#define DEL_LOG                        2         /*当前是要删除报警信息*/
#define DEL_PHOTO                      3         /*当前是要删除/抓拍图片*/
#define DEL_SOUND_LEAVER               4         /*当前是要删除语音留言*/
#define DEL_RING                       5         /*当前是要删除已下载铃声*/
#define CLEAR_MESSAGE                  6         /*当前是要清空短消息*/    
#define CLEAR_LOG                      7         /*当前是要清空报警信息*/  
#define CLEAR_PHOTO                    8         /*当前是要清空抓拍图片*/  
#define CLEAR_SOUND_LEAVER             9         /*当前是要清空语音留言*/  
#define CLEAR_RING                     10        /*当前是要清空已下载铃声*/
#define CLEAR_TELEPHONE                20
#define DEL_TELEPHONE                  21 


#define SET_LEAVE_HOME                 11        /*当前是要设置离家状态*/
#define SET_PWD_ERROR_CUE              12        /*设置提示密码输入错误的窗口*/
#define SET_ROOM_ERROR_TISHI           13        /*当输入房间号错误是，所设置的提示窗口*/
#define SET_PWD_RESET                  14        /*当前是否重置密码*/
#define  SET_RESET_TIME                 15        /*当前重置时间*/
#define SET_UPDATE_SYS                  16        /*当前系统升级成功*/
#define SET_SAFE_RESET                  17       /*确认重置防区*/
#define SET_RING_DOWNLOAD               18        /*确认铃声是否下载*/
#define SET_NET_CONFIG                  19        /*确认是否设置网络配置*/
#define SET_ADD_TELEPHONE               22        /*确认增加地址薄*/
#ifdef TEST_FFS
#define SET_FORMAT                      23        /*确认是否格式化*/
#endif

#define SET_RING                        24        /*确认是否设置铃声*/
#define SET_CLEAR_OLD_SCENE             25        /*确认是否清空以前的情景配置*/
        //#define SET_SAVE_RING                   26        /*确认是否保存试听的铃声。
#define SET_CANCEL_AVOID                27          /*确认是否取消免打扰*/
#define SET_CANCEL_AGENT                28          /*确认是否取消托管*/
#define SET_ROOM_NUMBER                 29          /*确认要修改分机编号*/
#define SET_TOUCH_SREEN                 30         /*确定是否进入触摸程序窗口*/

#define SET_ENABLE_SAFE                 31         /*确定要启用选定的防区*/
#define SET_IS_SCREENAJUEST             32         /*确认是否校准屏幕*/

        //////////////////////////////////////////////////////////////
        //PasswordHandler.cpp 为那个状态所创建密码输入框
#define SET_CHEFANG_STATUS             42        /*离家模式变位在家模式创建的密码输入窗口 CreatePasswordWin*/
#define SET_SAFE_PWD_WIN               43        /*安防密码窗口*/
#define SET_JIAJU_PWD_WIN              44         /*家居密码窗口*/
#define SET_DEFAULT_CONFIG_PWD_WIN     45         /*恢复出厂设置密码窗口*/
#define SET_NET_CONFIG_WIN             46         /*设置网络配置窗口*/
#define SET_CHEFANG_PWD_WIN            47         /*撤防密码窗口*/


		///////////////////////////////////////////////////////////////////////////
		///////////////////////
		//mutilmeida.cpp
		//
		void CreateMutilmediaMain();
		void CreateMutilmediaSub();
		void CreatePhotoWindow();
		void CreateMutilmediaVideo();
		void CreateMutilmediaAudio();
		void CreateMutilmediaPic();
		void CreateMplayerVideo();
		void CreateTsCalibrate();
		void CreateVideoTestWin();
		////////////////////////////////////////////////////////////
		//////////////////////////////////

		/////arp.c
		int InitArpSocket(void);
		void CloseArpSocket(void);
		int send_free_arp(void);
		/////
		
        //////////////////////////////////////////////////////////////////
        //localsethandler.cpp
#define  VOLUME_SET_WIN                    100         //音量设置窗口
#define  BRIGHT_SET_WIN                    101         //亮度设置窗口
#define  CONTRAST_SET_WIN                  102         //对比度设置窗口



        //////////////////////////////////////////////////////////////////
        //safehandler.cpp
        //extern bool  g_curSetSafeArea[16];        //当前设置的情景模式安防区域
        extern int   g_cur_set_scene_mode;        //当前设置的情景模式，在具体设置模式的时候使用。
	void chefang_jiaju();
        extern bool  g_is_have_jiaju;             //是否有家居模块,需要保存到flash
	extern bool g_is_replace_main_ui[3];          //中心是否有下载图片替换主界面。

        ////////////////////////////////////////////////////////////////////
        //osd draw function main_program.cpp

#define WRTARGB(a,r,g,b)	((unsigned long)((((unsigned long)(unsigned char)(r))<<16)|\
        (((unsigned)(unsigned char)(g))<<8))|\
        (((unsigned long)(unsigned char)(b))|\
        (((unsigned long)(unsigned char)(a))<<24)))
#define WRTRGB(r,g,b)	WRTARGB(255,(r),(g),(b))		/* rgb full alpha*/
#define WRT0RGB(r,g,b)	WRTARGB(0,(r),(g),(b))		/* rgb no alpha*/

#define WRTLT(x,y) GetLocalText(x,y)
#define LT(x) WRTLT(x,0)
#define LT2(x)  WRTLT(x,1)

#define LT3(x) GetLocalText2(x)

	//void wyxdug(int n)
	//{
	//	printf("seq :%d\n");
	//}
	extern int g_isUpdated;
        void UpdateWindow(int x,int y,int w,int h,int style);
        void update_rect(int x,int y,int w,int h);
		
        void ClearRect(int x,int y,int w,int h);

        void FillRect(int color,WRT_Rect rect);
		void FillSingleRect(int color,WRT_Rect rect);
        void OSD_FillRect(int color,WRT_Rect rect);

        void DrawRect(WRT_Rect rect,int linewidth,int color);
        void DrawSingleRect(WRT_Rect rect,int linewidth,int color);
        void Bmp32Disp(int x,int y,int w,int h,unsigned char* buf,int size);
        void OSD_Bmp32Disp(int x,int y,int w,int h,unsigned char* buf,int size);

        void JpegDispCenter(int x,int y,unsigned char* buf,int buflen,int scwidth);
        
        void JpegDispRectCenter(WRT_Rect rt,unsigned char* buf,int buflen);

        void JpegDisp(int x,int y,unsigned char* buf,int buflen);
        void JpegDispFile(int x,int y,struct image_data *picdata);
        void JpegSingleDisp(int x,int y,unsigned char* buf,int buflen);
        
        void JpegDisp2(int x,int y,int dw,int dh,unsigned char* buf,int buflen);

        void OSD_JpegDisp(int x,int y,unsigned char* buf,int buflen);
        
        void JpegDispZoom(int x,int y, unsigned char* buf,int buflen,float xscaling,float yscaling);
		void JpegFileDispZoom(int x,int y,FILE *fp);
        int  GetTextWidth(char* text,int is16);

        void DrawText(char* text,int x,int y,int color,int size=24);//24号宋体
        
		void DrawSingleText(char* text,int x,int y,int color,int size=24);
        void DrawText_monospaced_font(char* text,int x,int y,int color);
        void DrawTextToRect4(WRT_Rect rc, char* text,unsigned long color);

        void DrawText_16(char* text,int x,int y,int color); // 16号字体
        void OSD_DrawText16(char* text,int x,int y,int color);

        void OSD_DrawText(char* text,int x,int y,int color);

        void tDraw3dBox(int x,int y,int w,int h,unsigned long crTop,unsigned long  crBottom);

        void Color_FillRect(WRT_Rect rc,int r,int g,int b); //渐变颜色

        void DrawTextToRect(WRT_Rect rc, char* text ,unsigned long color=0x00000000);
        void DrawTextToRect2(WRT_Rect rc, char* text,unsigned long color,int is16);
        void DrawTextToRect3(WRT_Rect rc, char* text,unsigned long color,int is16);

        void tDraw3dInset(int x,int y,int w,int h);

        void tDraw3dOutset(int x,int y,int w,int h);

        int  IsInside(WRT_Rect rect, WRT_xy xy);

        void screen_close();
        void screen_open();

        void ReDrawCurrentWindow();

        void ReDrawCurrentWindow2(int type);

        void toinet_addr(unsigned long ip_addr, unsigned char *localip);

        void Reset_sys();

        void net_sence_task(unsigned long ipaddr); //ping 

        void DeflateRect( WRT_Rect* lpRect,int offset );

        int screen_adjust(int param);

        void SetDefaultConfig(T_SYSTEMINFO* p);


        void yes_screen_adjust();

        void no_screen_adjust();


        void check_alarm(int isstop);
        void check_alarm2();

        void DisplaySafeSatus();

        //2010.7.20 ,语言支持函数
        int GetCurrentLanguage(); 

        void SetSysInfoToEnglish();
        void SetSysInfoToChinese();

        char* GetLocalText(char* text,int ise);
        char* GetLocalText2(char* text);
        void DrawEnText(char* text,int w,int h,int x,int y);

        void DrawText_14(char* text,int x,int y,int color);

         int diff_up_down_time();
         
         //[2011-5-16 12:30:19]
         void DrawHuanjinValue(int type,int res);
         
         //2011-8-11 11:05:04
         void DrawWeatherResult();
         
         //2012-10-26 16:33:48
          void viewcallinfo(char* text);

        ///////////////////////////////////////////////////////////////////

#if HAVE_SCREEN_KEEP
        void screenkeep_init();
        void screenkeep_stop();
        void screenkeep_start();
        void display_screenkeep_image(int index);
#endif
	int isstartscreenkeep();

        ///////////////////////////////////////////////////////////////////
        //Mainhandler.cpp ,已完成
        void InitAlphabetAndNumber();
		
		        //2:刷桌面全部
        //3：刷桌面报警图标
        //4：刷桌面短消息
        //5:刷桌面网络图标
        //6:刷新桌面托管免扰图标
        //7：刷新桌面时间。
        //8:刷新桌面的中心图片
       //9:刷新右上角
       //10:刷新左下角
        void UpdateAlarmIcon(int update = 1);
        void UpdateMsgIcon();
        void UpdateNetIcon();
        void UpdateAVIcon();
        void UpdateTimeIcon();
        void UpdateReplaceJpeg(int index); 
        void ReUpdateUI2(int index) ;//调用UpdateReplaceJpeg 后，需要重新刷新UI2其他元素。
        
        int set_leave_delay(int param);
        int set_leave_nodelay(int param);
        
        void SwitchSkin(int skin);

        void CreateMainWin();
        void DrawMain();
        int MainEventHandler(int x,int y,int status);

        void CreateDoorCallWin();
        void DrawDoorCallMain();
        int  DoorCallEventHandler(int x,int y,int status);

        void CreateCenterCallWin();
        void DrawCenterCallMain();
        int  CenterCallEventHandler(int x,int y,int status);

        void CreateRoomCallWin(int type);
        void DrawRoomCallMain();
        int  RoomCallEventHandler(int x,int y,int status);

        void CreateCallWin();
        void DrawCallMain();
        int  CallEventHandler(int x,int y,int status);
        void CallLift();
        void CallLift1();

        void CreateCallCenterWin();
        void DrawCallCenterMain();
        int  CallCenterEventHandler(int x,int y,int status);
        void SendCallEvent();
        
		void CreateCameraWin();
		
        void CreateCallRoomWin();
        void DrawCallRoomMain();
        int  CallRoomEventHandler(int x,int y,int status);

        void CreateCallRoomSubWin();
        void DrawCallRoomSubMain();
        int  CallRoomSubEventHandler(int x,int y,int status);


        void CreatetMonitorWin();
        void DrawMonitorMain();
        int  MonitorEventHandler(int x,int y,int status);

        void CreateMonitorSubWin(int type);
        void DrawMonitorSubMain();
        int  MonitorSubEventHandler(int x,int y,int status);

        int cur_set_leave_home(int  param);

        void CreateCallRuleWin();
        void DrawCallRuleMain();
        int  CallRuleEventHandler(int x,int y,int status);

        void parse_inputcallrule(char* input);

#ifdef USE_DESKTOP
        void CreateSelectDesktopPos();
        void DrawSelectDesktopPos();
        int SelectDesktopEventHandler(int x,int y,int status);
        void SetCurrentHandlerCallback(FuncEnter _func);
       
#endif

	void CreateLeaveDelayWin();
	void  DrawLeaveDelayWin();
	int  LeaveDelayEventHandler(int x,int y,int status);
	
	
        ///////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////
        //阅读短消息 MsgHandler.cpp
        ////////////////////////////////////////////////////////////
        void CreateMsgWin();
        void DrawMsgWin();
        int MsgHandler(int x,int y,int status);

        void CreateReadMsgWin1();
        void DrawReadMsgWin1();
        int  ReadMsg1Handler(int x,int y,int status);

        int del_cur_msg(int param);
        int clear_msg(int param);
        void lock_cur_msg(bool islock);

#ifdef WRT_MORE_ROOM
        void more_room_set_read_msg_pos();
        int  more_room_read_next();
#endif
        ///////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////////////
        //photo_texthandler.cpp 已完成
        void CreateInfoWin();
        void DrawInfoMain();
        int  InfoEventHandler(int x,int y,int status);

        void DrawStatusbar(int npages,int npage);
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////
        //LogHandler.cpp  已完成
        void CreateWarningLog();
        void DrawWarningLogMain();
        int  WarningLogEventHandler(int x,int y,int status);

        int del_cur_log(int param);
        int clear_log(int param);
        int lock_cur_log(bool islock);
        ////////////////////////////////////////


        /////////////////////////////////////////////////////
        //voiceHanlder.cpp 已完成
        void CreateVoiceWin();
        void DrawVocieMain();
        int  VoiceEventHandler(int x,int y,int status);

        void CreateVoiceSubWin();
        void DrawVoiceSubMain();
        int  VoiceSubEventHandler(int x,int y,int status);


        int del_cur_voice(int param);
        int clear_voice(int param);
        void lock_cur_voice(bool islock);
        //////////////////////////////////////////////////////


        ///////////////////////////////////////////////////////////
        //PhotoHandler.cpp 已完成
        void CreatePhotoWin();
        void DrawPhotoMain();
        int  PhotoEventHandler(int x,int y,int status);

        void CreatePhotoSubWin();
        void DrawPhotoSubMain();
        int  PhotoSubEventHandler(int x,int y,int status);

        int del_cur_photo(int param);
        int clear_photo(int param);
        void lock_cur_photo(bool islock);

		void CreateVideoWin();
        void DrawVideoMain();
        int  VideoEventHandler(int x,int y,int status);
        void CreateVideoSubWin();
        void DrawVideoSubMain();
        int  VideoSubEventHandler(int x,int y,int status);
        int del_cur_video(int param);
        int clear_video(int param);
        void lock_cur_video(bool islock);
#ifdef WRT_MORE_ROOM
        void more_room_set_read_photo_pos();
        int  more_room_read_next_photo();
#endif
        ///////////////////////////////////////////////////////////

        ////////////////////////////////////////////////
        //RingHandler.cpp 已完成

        void CreateRingWin();
        void DrawRingMain();
        int RingEventHandler(int x,int y,int status);

        void CreateRingDownWin();
        void DrawRingDownMain();
        int RingDownEventHandler(int x,int y,int status);

        void CreateDownLoadRingWin();
        void DrawDownLoadRingMain();
        int DownLoadRingEventHandler(int x,int y,int status);

        int del_cur_download(int param);
        int clear_download(int param);
        void lock_cur_download(bool islock);

        int download_ring(int param);
        int cancel_listenring(int param);

        int set_cur_ring(int param);
        int cancel_set_cur_ring(int param);
        ////////////////////////////////////////////////

        /////////////////////////////////////////////////////////
        //DialogHandler.cpp 已完成
        void CreateDialogWin(char* text,int type);
        void DrawDialogMain();
        int  DialogHandler(int x,int y,int status);


        //DialogHandler2.cpp
#define MB_NONE    1
#define MB_OK      1<<1
#define MB_CANCEL  1<<2
#define MB_CALL    1<<3
#define MB_ONLY    1<<4

        typedef int (*DIALOG_CALLBACK)(int param);

	
	void SetDialogWin2CloseCallback(DIALOG_CALLBACK func_close);
        void CreateDialogWin2(char* text,int type ,DIALOG_CALLBACK  func_ok, DIALOG_CALLBACK func_cancel);
        void DrawDialogMain2();
        int  DialogHandler2(int x,int y,int status);
        
        void CreateDialogWin3(char* text,int type,DIALOG_CALLBACK  func_ok, DIALOG_CALLBACK func_cancel,DIALOG_CALLBACK func_call);
        void DrawDialogMain3();
        int  DialogHandler3(int x,int y,int status);
        ///////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////
        //feehandler.cpp   部分
        void CreateFeeSubWin();
        void DrawFeeSubMain();
        int FeeSubEventHandler(int x,int y,int status);

        void CreateFeeWin();
        void DrawFeeMain();
        int  FeeEventHandler(int x,int y,int status);
        ///////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////
        //jiajuhandler.cpp
        void CreateJiajuWin();
        void DrawJiajuMain();
        int JiajuEventHandler(int x,int y,int status);

        void CreateSceneWin();
        void DrawSceneMain();
        int SceneEventHandler(int x,int y,int status);
        void CreateSceneWin2();
        int SceneEventHandler2(int x,int y,int status);

        void CreateRoomControl();
        void DrawRoomControl();
        int RoomControlEventHandler(int x,int y,int status);

        void CreateLampControlWin();
        void DrawLampControlWin();
        int LampControlEventHandler(int x,int y,int status);

        void CreateJackControlWin();
        void DrawJackControlWin();
        int JackControlEventHandler(int x,int y,int status);

        void CreatePurdahControlWin();
        void DrawPurdahControlWin();
        int PurdahControlEventHandler(int x,int y,int status);

        void CreateTVControlWin();
        void DrawTVControlWin();
        int TVControlEventHandler(int x,int y,int status);

        void CreateAirControlWin();
        void DrawAirControlWin();
        int AirControlEventHandler(int x,int y,int status);

        //extern int g_cur_set_scene_mode;
        ///////////////////////////////////////////////////////////////
        //家居控制设置JiajuSetHandler.cpp
        void CreateHomeSetWin();
        void DrawHomeSetMain();
        int HomeSetEventHandler(int x,int y,int status);

        void CreateSceneSetWin();
        void DrawSceneSetMain();
        int SceneSetEventHandler(int x,int y,int status);
        
        void CreateSceneSetWin2(int scene_mode);
        void DrawSceneSetMain2();
        int SceneSetEventHandler2(int x,int y,int status);

        void CreateSceneControlwin(int type);
        void DrawSceneControlMain();
        int  SceneControlEventHandler(int x,int y ,int status);

        /*
        void CreateRoomControlSetWin();
        void DrawRoomControlSetMain();
        int RoomControlSetEventHandler(int x,int y,int status);
        */

        void CreateSetWin(int set_type);
        void DrawSetMain();
        int  SetEventHandler(int x,int y,int status);


        void CreateSceneSetSub();
        void DrawSceneSetSubMain();
        int SceneSetSubEventHandler(int x, int y, int status);


        int get_cur_scenemode_safe_status(int mode,int index);
        int set_cur_scenemode_safe_status(int mode,int index,int status);
        unsigned char get_select_control_index(T_JIAJU_TABLE* pJiaju,unsigned char cmdmode);
        /*
        void CreateTVSetWin();
        void DrawTVSetMain();
        int  TVSetEventHandler(int x,int y,int status);

        void CreateAirSetWin();
        void DrawAirSetMain();
        int  AirSetEventHandler(int x,int y,int status);
        */
        /*
        void CreateAtHomeWin();
        void DrawAtHomeMain();
        int  AtHomeEventHandler(int x,int y,int status);

        void CreateLeaveHomeWin();
        void DrawLeaveHomeMain();
        int  LeaveHomeEventHandler(int x,int y,int status);
        */
        ////////////////////////////////////////////////////////////////
        //ServiceHandler.cpp
        void CreateServiceWin();
        void DrawServiceMain();
        int  ServiceEventHandler(int x,int y,int status);


        //维修
        void CreateMaintainWin();
        void DrawMaintainMain();
        int  MaintainEventHandler(int x,int y,int status);

        void CreateMaintainWin2();
        void DrawMaintainMain2();
        int  MaintainEventHandler2(int x,int y,int status);

        //便民服务
        void CreatePeopleInfoWin();
        void DrawPeopleInfoMain();
        int  PeopleInfoEventHandler(int x,int y,int status);

        void CreatePeopleInfoWin2();
        void DrawPeopleInfoMain2();
        int  PeopleInfoEventHandler2(int x,int y,int status);

        void CreateOursServices();

        //家庭留言

        void CreateRecordWin();
        void RecordMain();
        int   RecordEventHandle(int x,int y,int status);
        void UpdateRecordTime(int time);

        //情景任务计划
        void SetIsShortCutAlarmclock();
        void CreateTimerScene();
        int TimerSceneHandle(int x,int y,int status);
        void DrawTimerScene();

        void InitsceneTimer();
        void EnterSceneTimerLock();
        void ExitSceneTimerLock();

        int StartSceneTimer();
        int StartClockTimer();
        void delete_scene_and_clock();

        ///////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////
        //setuphandler.cpp
        void CreateSetupWin();
        void DrawSetupMain();
        int  SetupEventHandler(int x,int y,int status);

        int ResetSysTime(int param);
        
        void CreateUpgradeWin();
        void DrawUpgradeWin();
        int  UpgradeEventHandler(int x,int y,int status);
        



        ///////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////
        //PasswordHandler.cpp
        void  CreatePasswordWin(int type);
        void  DrawPasswordMain();
        int   PasswordEventhandler(int x,int y,int status);

        void CreateSetPasswordWin();
        void DrawSetPasswordMain();
        int  SetPasswordEventHandler(int x,int y,int status);

        int ConfirmResetPwd(int param);

        void modifypwd();
        ///////////////////////////////////////////////////////////////////


        ///////////////////////////////////////////////////////////////////
        //SafeHandler.cpp
        void  CreateSafeWin(int type=0);
        void DrawSafeMain();
        int  SafeEventHandler(int x,int y,int status);

        int Reset_safe(int param);
        bool get_security_status(int index);
        int ok_safe(int param);
        void set_security_status(int index,bool status);
        bool isenablesafe();


        void CreateSafe2Win();
        void DrawSafe2Main();
        int Safe2EventHandle(int x,int y,int status);


        void CreateEnableSafeWin();
        void DrawEnableSafeMain();
        int EnableSafeEventHandler(int x,int y,int status);
        int enable_safe(int param);


        void CreateViewSafeStatusWin();
        void DrawSafeStatusMain();
        int  ViewSafeStatusHandler(int x,int y,int status);
        void SetViewSafeStatus(int index);
        void ClearViewSafeStatus();
        void ViewSafeSatus(int isupdate);

        void CreateRemoteControl();
        void DrawRemoteControlMain();
        int    RemoteControlHandle(int x,int y,int status);

        int   GetIsStartDefendStatus();
		void CreateSysCmdWin();
        //void Set_Safe_Status(bool bsafe[16]);
        
        void  password_chefang();
        void  remote_chefang();

#ifdef WRT_MORE_ROOM
        void sync_all_zone();
#endif

        ///////////////////////////////////////////////////////////////////


        ///////////////////////////////////////////////////
        //HelpHandler.cpp
        void CreateHelpWin();
        void DrawHelpMain();
        int HelpEventHandler(int x,int y,int status);
        ///////////////////////////////////////////////////

        ////////////////////////////////////////////////////
        //SysHandler.cpp
        void CreateSysWin();
        void DrawSysMain();
        int SysEventHandler(int x,int y,int status);

        //系统信息
        void CreateSysInfoWin();
        void DrawSysInfoMain();
        int SysInfoEventHandler(int x,int y,int status);

        //系统状态
        void CreateSysStatusWin();
        void DrawSysStatusMain();
        int SysStatusEventHandler(int x,int y,int status);
        ////////////////////////////////////////////////////


        ////////////////////////////////////////////
        //MainHandler.cpp
        void GetNumberJpgPointer(unsigned long** pointer1,unsigned long** pointer2);//获得保存起来的数字符号的数据指针
        void GetNumberJpegSizePointer(unsigned long** size1,unsigned long** size2);//获得保存起来的数字符号的数据大小
        
        void GetAlphabetJpgPointer(unsigned long** pointer1,unsigned long** pointer2); //获得字母表
        void GetAlphabetJpegSizePointer(unsigned long** size1,unsigned long** size2); //获得字母表的大小
        void SetAlphabet26and27Key(int keytype);

        int add_telephone_1(int param);
        ////////////////////////////////////////////

        ///////////////////////////////////////////
        //LocaSetHandler.cpp
        void CreateLocalSetWin();
        void DrawLocalSetMain();
        int LockSetEventHandler(int x,int y,int status);

        //响铃时间设置
        void CreateRingTimeSetWin();
        void RingTimeSetMain();
        int RingTimeSetEventHandler(int x,int y,int status);

        //免打扰设置
        void CreateAvoidHarryWin(int type);
        void DrawAvoidHarryMain();
        int AvoidHarryEventHandler(int x,int y,int status);

        //音量调节
        void CreateTypeSet(int type);
        void DrawTypeSetMain();
        int TypeSetEventHandler(int x,int y,int status);

        void CreateVoiceControl();
        void DrawVoiceControlMain();
        int VoiceControlEventHandle(int x,int y,int status);

        int CancelAvoid(int param); //取消免打扰
        int CancelAgent(int param); //取消托管

        void CreateColorManager();
        void DrawColorManager();
        int ColorManagerEventHandler(int x,int y,int stauts);


        void CreateExtraFunction();
        void DrawExtraFunction();
        int   ExtraFunctionEventHandler(int x,int y,int status);
        //////////////////////////////////////////

        //////////////////////////////////////////
        //modifyipandmask.cpp
        void CreateModifyIPWin(int type);
        void DrawModifyIPMain();
        int ModifyIPEventHandler(int x,int y,int status);

        int modify_net_set(int param);

        int modify_room_number(int param);

        void CreateNetConfigWin();
        void DrawNetConfigWin();
        int NetConfigEventHandler(int x,int y,int status);


        //////////////////////////////////////
        //telephoneHandler.cpp
        int clear_telephone(int param);
        void CreateTelephoneWin();
        void DrawTelephoneMain();
        int  TelephoneEventHandler(int x,int y,int status);
        int del_cur_telephone(int param);
        void lock_cur_telephone(bool islock);
        int add_telephone_1(int param);


        ///////////////////////////////////////
        //



        //////////////////////////////////////////
        //配置家居系统
        //configRoomSys.cpp


#define BACK_COLOR   0XB2136

#define MAX_DEV_SORT  11    //设备种类
#define MAX_ROOM_SORT  8    //最大的房间类别
#define MAX_ROOM_NAME  10   //每个房间类型中最大房间数量
#define MAX_ELEC_SORT  8     //最大的电器类型
#define MAX_ELEC_NAME  10   //每个类型中，最大的电器数量

#define MAX_AIR_MODE   10   //最大的空调模式
#define MAX_JIAJU_TABLE_COUNT  128  //最大的家居表的大小

#define MAX_SCENE_MODE 10         //情景模式的个数


#define SCENE_HUIKE       0 //会客
#define SCENE_JIUCAN      1 //就餐
#define SCENE_YINGYUAN    2 //影院
#define SCENE_JIUQIN      3 //就寝
#define SCENE_WENXIN      4 //温馨
#define SCENE_ZAIJIA      5 //在家
#define SCENE_LIJIA       6 //离家

#define SCENE_YEQI        7 //夜起
#define SCENE_CHENQI      8//晨起
#define SCENE_USERDEFINE  9//自定义

//#define SCENE_UNKNOW	  10 //单独防区

        typedef struct _tagSceneTable{
                int sort;             //家居配置表中，家居的索引
                int status;           //每个家居的操作
                int valid;            //是否有配置（效）
        }T_SCENE_TABLE;

        typedef struct _tagSceneModeTable{
               // T_SCENE_TABLE  scene_table[MAX_JIAJU_TABLE_COUNT];
                int scene;
                int relation;
                unsigned char safestatus[16] ;       //每个模式下，16防区的安防状态           
        }T_SCENE_MODE_TABLE;


        typedef struct _tagMENU_ITEM{
                char *name; //菜单名称
                int   id;   //对应的菜单ID
                int  number;//对应编号
        }MENU_ITEM;








        extern MENU_ITEM    g_DEVTYPE[MAX_DEV_SORT];

        extern MENU_ITEM    g_ROOMTYPE[MAX_ROOM_SORT];
        extern MENU_ITEM    g_ROOMNAME[MAX_ROOM_SORT][MAX_ROOM_NAME];
        extern MENU_ITEM    g_ELECTYPE[MAX_ELEC_SORT];
        extern MENU_ITEM    g_ELECTNAME[MAX_ELEC_SORT][MAX_ELEC_NAME];

        extern T_JIAJU_TABLE   g_JIAJU_TABLE[MAX_JIAJU_TABLE_COUNT];
        extern T_SCENE_MODE_TABLE   g_SCENE_MODE_TABLE[MAX_SCENE_MODE];

        extern int     g_cur_select_room_type; //当前所选择的房间类别
        extern int     g_cur_select_room;        //当前选择的房间
        extern int     g_cur_select_elec_type;   //但前选择的电器类别
        extern int     g_cur_select_elec ;       //当前选择的电器
        extern int     g_cur_select_dev_type;     //当前选择的RF设备类型
        extern int     g_cur_select_scene_mode;   //当前选择的情景模式,配置的时候使用

        extern T_SYSTEMINFO*  pSystemInfo;

        void CreateConfigMain(int type);
        void DrawConfigMain();
        int ConfigMainEventHandler(int x,int y,int status);

        void init_jiaju_table();      /*初始化家居表，保存128个家居表，也就是可以配置128个电器*/
        int  get_idle_jiaju_table();    /*获得空闲的家居表*/
        int  get_jiaju(T_ELEC_ID* JiajuID);     /*通过家居参数，从家居表中查找需要的家居数据*/

        int clear_jiaju_table(int param);
        int GetJiajuCount();


        void init_scene_mode_table();     /*初始化情景模式表*/
        int  get_idle_scene_table(int mode);/*获得空闲的情景模式*/

        int send_scene_mode(int mode);   /*通过485发送情景模式的索引号*/
        
        int  scene_is_relation_zone(int mode);//判断当前情景是否关联防区。

        unsigned char get_select_control_text(T_JIAJU_TABLE* pJiaju,int index,char* ptext);



        int send_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec,unsigned char status);
        unsigned char* get_jiaju_config_buf_for_center(int* len);
        int get_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec);
        int send_scene_mode(int mode);
        void SetJiajuDefault();
        int clear_scene_mode(int param);

        int GetCurSceneMode();

        unsigned char* Get_jiaju_table_file(int *len); //2009-7-29 14:17:05
        int Set_jiaju_table_file(unsigned char* buf,int buflen);//2009-7-29 14:31:08
        
        void FindAndViewJiajuStatus();

        //////////////////////////////////////////////////2010-11-19
        //
#ifdef THREE_UI
        void CreateShortcutRoomControl2();
        void DrawShortcutRoomControl2();
        int ShortcutRoomControlEventHandler2(int x,int y,int status );
#else        
        void CreateShortcutRoomControl();
        void DrawShortcutRoomControl();
        int ShortcutRoomControlEventHandler(int x,int y,int status );
#endif        

        ///////////////////////////////////////////////////////////////////////
        void CreateHostSetWin();
        void DrawHostSetMain();
        int HostSetEventHandler(int x,int y ,int status);

        void init_localhost_config();
        bool  save_scene_mode_table_to_flash();
        bool save_jiaju_table_to_flash();
        bool save_localhost_config_to_flash();

        int GetIsHaveJiaju();

        void start_defend(bool value);
        
        void process_alarm_clock();
        
        int isganyinqi();
        
        int isguanlianfangqu(int zone);
        int isdelayjack();
        
        void send_delay_jack(int def_value);

        ///////////////////////////////////////////////////////////////////
        //
#ifdef TEST_FFS
  //      void init_my_ffs();
 //       void test_file();
//        int format_my_ffs(int param);
#endif

        int downring_ok(int param);
        ////////////////////////////////////
        //
   //     unsigned long inet_addr(const char *cp);


        //for factory
        void test_alarm_for_factory();//safehandler.cpp


//  [2010年12月29日 9:38:48 by ljw]

        void CreateMainUI2();
        void DrawMainUI2();
        int   HandleMainUI2Event(int x,int y,int status);

//2014-3-27 增加IO已经设备简易测试
		void CreateSampleTestWin();
		void mySampleTest(int isupdate);
//2014-3-28  增加IO有效电平设置
		void CreateIoLevelSetWin();
		void myIoLevelSet(int isupdate);
//2014-3-31 add setup
		void CreateProjectSetupWin();
//2014-4-1 add ipcamera
		void CreateIPCameraCfgWin();
		void CreatetIPCameraWin();
		void Init_ipcamera_config();
//2014-6-24 add sub room config [wyx]
		void CreateSubRoomCfgWin();
		
//2014-07-22
		void CreateIPCameraDispWin();

//2014-11-11
		void CreateWifiCfgWin();
		void CreateSsidPwdWin(int type);
		int set_wifi_netmask(unsigned long netmask);
		int set_wifi_ip(unsigned long ipaddr);
		int set_wifi_gateway(unsigned long gatewayip);
		void Wifi_Config_Process();
		int GetWifiNetStat();
//2015-2-4	发送防区布撤防日志到中心
		void SendAlarmLogToCenter(int value);
//2015-6-9 
		void CreateContorlSwitchWin();
//2015-6-8
		void CreateDisplayWin();
		void CreateSmartBoxSearchWin();
#ifdef __cplusplus
}
#endif

#endif // _HANDLER_H_