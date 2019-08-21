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

#define TEST_FFS 1                                     //����FORMAT
        //#define USED_FFS 1                                     //ʹ��FFS
        /////////////////////////////////////////////////////////////////////
        //config 
#define HAVE_SCREEN_KEEP             1                //�Ƿ�����Ļ����
#define HAVE_JIAJU_SAFE              1               //�Ҿ��Ƿ��������
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

        //�������('�������������ԡ��ȵ�)
        /*
                char* name;
                char* name_map;
                int   id;
        }ROOMTYPE;

        //�������
        typedef struct _tagRoomWiring{
                char* name;
                int   id;
        }ROOMWIRING;


        typedef struct _tagLAMPINFO{
                char name[10];
                bool state;
        }LAMPINFO;

        typedef struct _tagSETINFO{
                char name[10];              //����
                char controlname[20];       //����������
                char number[12];            //���
                char riunumber[12];        //riu���
        }SETINFO;

        typedef struct _tagTVAIRSETINFO{
                char name[10];              //����
                char controlname[20];       //����������
                // char number[12];            //���
                // char riunumber[12];        //riu���	
                char keynumber[9][12];     //��������(���ϵ��£�����+������-��Ƶ��+,Ƶ��-��av,open/close).
                //����ǿյ������ϵ��£��¶�+���¶ȡ���ģʽ+��ģʽ-��ǿ��+��ǿ��-����/�أ�
        }TVAIRSETINFO;
        */
        /////////////////////////////////////////////////////////////////////////
        
        extern SDL_Surface*  g_pScreenSurface;
        extern SDL_Surface*  tmp_g_pScreenSurface;
//        extern SDL_Surface*  tmp_g_pScreenSurface1;
        
        
typedef unsigned char UInt8;        
typedef unsigned long UInt32;


        /////////////////////////////////////////////////////////////////////////
        //ΪDialog service
#define DEL_MESSAGE                    1         /*��ǰ��Ҫɾ������Ϣ*/
#define DEL_LOG                        2         /*��ǰ��Ҫɾ��������Ϣ*/
#define DEL_PHOTO                      3         /*��ǰ��Ҫɾ��/ץ��ͼƬ*/
#define DEL_SOUND_LEAVER               4         /*��ǰ��Ҫɾ����������*/
#define DEL_RING                       5         /*��ǰ��Ҫɾ������������*/
#define CLEAR_MESSAGE                  6         /*��ǰ��Ҫ��ն���Ϣ*/    
#define CLEAR_LOG                      7         /*��ǰ��Ҫ��ձ�����Ϣ*/  
#define CLEAR_PHOTO                    8         /*��ǰ��Ҫ���ץ��ͼƬ*/  
#define CLEAR_SOUND_LEAVER             9         /*��ǰ��Ҫ�����������*/  
#define CLEAR_RING                     10        /*��ǰ��Ҫ�������������*/
#define CLEAR_TELEPHONE                20
#define DEL_TELEPHONE                  21 


#define SET_LEAVE_HOME                 11        /*��ǰ��Ҫ�������״̬*/
#define SET_PWD_ERROR_CUE              12        /*������ʾ�����������Ĵ���*/
#define SET_ROOM_ERROR_TISHI           13        /*�����뷿��Ŵ����ǣ������õ���ʾ����*/
#define SET_PWD_RESET                  14        /*��ǰ�Ƿ���������*/
#define  SET_RESET_TIME                 15        /*��ǰ����ʱ��*/
#define SET_UPDATE_SYS                  16        /*��ǰϵͳ�����ɹ�*/
#define SET_SAFE_RESET                  17       /*ȷ�����÷���*/
#define SET_RING_DOWNLOAD               18        /*ȷ�������Ƿ�����*/
#define SET_NET_CONFIG                  19        /*ȷ���Ƿ�������������*/
#define SET_ADD_TELEPHONE               22        /*ȷ�����ӵ�ַ��*/
#ifdef TEST_FFS
#define SET_FORMAT                      23        /*ȷ���Ƿ��ʽ��*/
#endif

#define SET_RING                        24        /*ȷ���Ƿ���������*/
#define SET_CLEAR_OLD_SCENE             25        /*ȷ���Ƿ������ǰ���龰����*/
        //#define SET_SAVE_RING                   26        /*ȷ���Ƿ񱣴�������������
#define SET_CANCEL_AVOID                27          /*ȷ���Ƿ�ȡ�������*/
#define SET_CANCEL_AGENT                28          /*ȷ���Ƿ�ȡ���й�*/
#define SET_ROOM_NUMBER                 29          /*ȷ��Ҫ�޸ķֻ����*/
#define SET_TOUCH_SREEN                 30         /*ȷ���Ƿ���봥�����򴰿�*/

#define SET_ENABLE_SAFE                 31         /*ȷ��Ҫ����ѡ���ķ���*/
#define SET_IS_SCREENAJUEST             32         /*ȷ���Ƿ�У׼��Ļ*/

        //////////////////////////////////////////////////////////////
        //PasswordHandler.cpp Ϊ�Ǹ�״̬���������������
#define SET_CHEFANG_STATUS             42        /*���ģʽ��λ�ڼ�ģʽ�������������봰�� CreatePasswordWin*/
#define SET_SAFE_PWD_WIN               43        /*�������봰��*/
#define SET_JIAJU_PWD_WIN              44         /*�Ҿ����봰��*/
#define SET_DEFAULT_CONFIG_PWD_WIN     45         /*�ָ������������봰��*/
#define SET_NET_CONFIG_WIN             46         /*�����������ô���*/
#define SET_CHEFANG_PWD_WIN            47         /*�������봰��*/


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
#define  VOLUME_SET_WIN                    100         //�������ô���
#define  BRIGHT_SET_WIN                    101         //�������ô���
#define  CONTRAST_SET_WIN                  102         //�Աȶ����ô���



        //////////////////////////////////////////////////////////////////
        //safehandler.cpp
        //extern bool  g_curSetSafeArea[16];        //��ǰ���õ��龰ģʽ��������
        extern int   g_cur_set_scene_mode;        //��ǰ���õ��龰ģʽ���ھ�������ģʽ��ʱ��ʹ�á�
	void chefang_jiaju();
        extern bool  g_is_have_jiaju;             //�Ƿ��мҾ�ģ��,��Ҫ���浽flash
	extern bool g_is_replace_main_ui[3];          //�����Ƿ�������ͼƬ�滻�����档

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

        void DrawText(char* text,int x,int y,int color,int size=24);//24������
        
		void DrawSingleText(char* text,int x,int y,int color,int size=24);
        void DrawText_monospaced_font(char* text,int x,int y,int color);
        void DrawTextToRect4(WRT_Rect rc, char* text,unsigned long color);

        void DrawText_16(char* text,int x,int y,int color); // 16������
        void OSD_DrawText16(char* text,int x,int y,int color);

        void OSD_DrawText(char* text,int x,int y,int color);

        void tDraw3dBox(int x,int y,int w,int h,unsigned long crTop,unsigned long  crBottom);

        void Color_FillRect(WRT_Rect rc,int r,int g,int b); //������ɫ

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

        //2010.7.20 ,����֧�ֺ���
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
        //Mainhandler.cpp ,�����
        void InitAlphabetAndNumber();
		
		        //2:ˢ����ȫ��
        //3��ˢ���汨��ͼ��
        //4��ˢ�������Ϣ
        //5:ˢ��������ͼ��
        //6:ˢ�������й�����ͼ��
        //7��ˢ������ʱ�䡣
        //8:ˢ�����������ͼƬ
       //9:ˢ�����Ͻ�
       //10:ˢ�����½�
        void UpdateAlarmIcon(int update = 1);
        void UpdateMsgIcon();
        void UpdateNetIcon();
        void UpdateAVIcon();
        void UpdateTimeIcon();
        void UpdateReplaceJpeg(int index); 
        void ReUpdateUI2(int index) ;//����UpdateReplaceJpeg ����Ҫ����ˢ��UI2����Ԫ�ء�
        
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
        //�Ķ�����Ϣ MsgHandler.cpp
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
        //photo_texthandler.cpp �����
        void CreateInfoWin();
        void DrawInfoMain();
        int  InfoEventHandler(int x,int y,int status);

        void DrawStatusbar(int npages,int npage);
        ///////////////////////////////////////////////////////////////////

        ///////////////////////////////////
        //LogHandler.cpp  �����
        void CreateWarningLog();
        void DrawWarningLogMain();
        int  WarningLogEventHandler(int x,int y,int status);

        int del_cur_log(int param);
        int clear_log(int param);
        int lock_cur_log(bool islock);
        ////////////////////////////////////////


        /////////////////////////////////////////////////////
        //voiceHanlder.cpp �����
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
        //PhotoHandler.cpp �����
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
        //RingHandler.cpp �����

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
        //DialogHandler.cpp �����
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
        //feehandler.cpp   ����
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
        //�Ҿӿ�������JiajuSetHandler.cpp
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


        //ά��
        void CreateMaintainWin();
        void DrawMaintainMain();
        int  MaintainEventHandler(int x,int y,int status);

        void CreateMaintainWin2();
        void DrawMaintainMain2();
        int  MaintainEventHandler2(int x,int y,int status);

        //�������
        void CreatePeopleInfoWin();
        void DrawPeopleInfoMain();
        int  PeopleInfoEventHandler(int x,int y,int status);

        void CreatePeopleInfoWin2();
        void DrawPeopleInfoMain2();
        int  PeopleInfoEventHandler2(int x,int y,int status);

        void CreateOursServices();

        //��ͥ����

        void CreateRecordWin();
        void RecordMain();
        int   RecordEventHandle(int x,int y,int status);
        void UpdateRecordTime(int time);

        //�龰����ƻ�
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

        //ϵͳ��Ϣ
        void CreateSysInfoWin();
        void DrawSysInfoMain();
        int SysInfoEventHandler(int x,int y,int status);

        //ϵͳ״̬
        void CreateSysStatusWin();
        void DrawSysStatusMain();
        int SysStatusEventHandler(int x,int y,int status);
        ////////////////////////////////////////////////////


        ////////////////////////////////////////////
        //MainHandler.cpp
        void GetNumberJpgPointer(unsigned long** pointer1,unsigned long** pointer2);//��ñ������������ַ��ŵ�����ָ��
        void GetNumberJpegSizePointer(unsigned long** size1,unsigned long** size2);//��ñ������������ַ��ŵ����ݴ�С
        
        void GetAlphabetJpgPointer(unsigned long** pointer1,unsigned long** pointer2); //�����ĸ��
        void GetAlphabetJpegSizePointer(unsigned long** size1,unsigned long** size2); //�����ĸ��Ĵ�С
        void SetAlphabet26and27Key(int keytype);

        int add_telephone_1(int param);
        ////////////////////////////////////////////

        ///////////////////////////////////////////
        //LocaSetHandler.cpp
        void CreateLocalSetWin();
        void DrawLocalSetMain();
        int LockSetEventHandler(int x,int y,int status);

        //����ʱ������
        void CreateRingTimeSetWin();
        void RingTimeSetMain();
        int RingTimeSetEventHandler(int x,int y,int status);

        //���������
        void CreateAvoidHarryWin(int type);
        void DrawAvoidHarryMain();
        int AvoidHarryEventHandler(int x,int y,int status);

        //��������
        void CreateTypeSet(int type);
        void DrawTypeSetMain();
        int TypeSetEventHandler(int x,int y,int status);

        void CreateVoiceControl();
        void DrawVoiceControlMain();
        int VoiceControlEventHandle(int x,int y,int status);

        int CancelAvoid(int param); //ȡ�������
        int CancelAgent(int param); //ȡ���й�

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
        //���üҾ�ϵͳ
        //configRoomSys.cpp


#define BACK_COLOR   0XB2136

#define MAX_DEV_SORT  11    //�豸����
#define MAX_ROOM_SORT  8    //���ķ������
#define MAX_ROOM_NAME  10   //ÿ��������������󷿼�����
#define MAX_ELEC_SORT  8     //���ĵ�������
#define MAX_ELEC_NAME  10   //ÿ�������У����ĵ�������

#define MAX_AIR_MODE   10   //���Ŀյ�ģʽ
#define MAX_JIAJU_TABLE_COUNT  128  //���ļҾӱ�Ĵ�С

#define MAX_SCENE_MODE 10         //�龰ģʽ�ĸ���


#define SCENE_HUIKE       0 //���
#define SCENE_JIUCAN      1 //�Ͳ�
#define SCENE_YINGYUAN    2 //ӰԺ
#define SCENE_JIUQIN      3 //����
#define SCENE_WENXIN      4 //��ܰ
#define SCENE_ZAIJIA      5 //�ڼ�
#define SCENE_LIJIA       6 //���

#define SCENE_YEQI        7 //ҹ��
#define SCENE_CHENQI      8//����
#define SCENE_USERDEFINE  9//�Զ���

//#define SCENE_UNKNOW	  10 //��������

        typedef struct _tagSceneTable{
                int sort;             //�Ҿ����ñ��У��Ҿӵ�����
                int status;           //ÿ���ҾӵĲ���
                int valid;            //�Ƿ������ã�Ч��
        }T_SCENE_TABLE;

        typedef struct _tagSceneModeTable{
               // T_SCENE_TABLE  scene_table[MAX_JIAJU_TABLE_COUNT];
                int scene;
                int relation;
                unsigned char safestatus[16] ;       //ÿ��ģʽ�£�16�����İ���״̬           
        }T_SCENE_MODE_TABLE;


        typedef struct _tagMENU_ITEM{
                char *name; //�˵�����
                int   id;   //��Ӧ�Ĳ˵�ID
                int  number;//��Ӧ���
        }MENU_ITEM;








        extern MENU_ITEM    g_DEVTYPE[MAX_DEV_SORT];

        extern MENU_ITEM    g_ROOMTYPE[MAX_ROOM_SORT];
        extern MENU_ITEM    g_ROOMNAME[MAX_ROOM_SORT][MAX_ROOM_NAME];
        extern MENU_ITEM    g_ELECTYPE[MAX_ELEC_SORT];
        extern MENU_ITEM    g_ELECTNAME[MAX_ELEC_SORT][MAX_ELEC_NAME];

        extern T_JIAJU_TABLE   g_JIAJU_TABLE[MAX_JIAJU_TABLE_COUNT];
        extern T_SCENE_MODE_TABLE   g_SCENE_MODE_TABLE[MAX_SCENE_MODE];

        extern int     g_cur_select_room_type; //��ǰ��ѡ��ķ������
        extern int     g_cur_select_room;        //��ǰѡ��ķ���
        extern int     g_cur_select_elec_type;   //��ǰѡ��ĵ������
        extern int     g_cur_select_elec ;       //��ǰѡ��ĵ���
        extern int     g_cur_select_dev_type;     //��ǰѡ���RF�豸����
        extern int     g_cur_select_scene_mode;   //��ǰѡ����龰ģʽ,���õ�ʱ��ʹ��

        extern T_SYSTEMINFO*  pSystemInfo;

        void CreateConfigMain(int type);
        void DrawConfigMain();
        int ConfigMainEventHandler(int x,int y,int status);

        void init_jiaju_table();      /*��ʼ���Ҿӱ�����128���Ҿӱ�Ҳ���ǿ�������128������*/
        int  get_idle_jiaju_table();    /*��ÿ��еļҾӱ�*/
        int  get_jiaju(T_ELEC_ID* JiajuID);     /*ͨ���ҾӲ������ӼҾӱ��в�����Ҫ�ļҾ�����*/

        int clear_jiaju_table(int param);
        int GetJiajuCount();


        void init_scene_mode_table();     /*��ʼ���龰ģʽ��*/
        int  get_idle_scene_table(int mode);/*��ÿ��е��龰ģʽ*/

        int send_scene_mode(int mode);   /*ͨ��485�����龰ģʽ��������*/
        
        int  scene_is_relation_zone(int mode);//�жϵ�ǰ�龰�Ƿ����������

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


//  [2010��12��29�� 9:38:48 by ljw]

        void CreateMainUI2();
        void DrawMainUI2();
        int   HandleMainUI2Event(int x,int y,int status);

//2014-3-27 ����IO�Ѿ��豸���ײ���
		void CreateSampleTestWin();
		void mySampleTest(int isupdate);
//2014-3-28  ����IO��Ч��ƽ����
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
//2015-2-4	���ͷ�����������־������
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