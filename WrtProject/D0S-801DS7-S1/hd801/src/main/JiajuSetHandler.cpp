#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "Handler.h"
#include "audiovideo.h"

#include "public_menu_res.h"
#include "window.h"
#include "c_interface.h"
#include "jiaju_menu_res.h"


extern bool     g_isstartdelay;
extern bool     g_have_defend;
extern bool     g_is_leave ;

#if HAVE_SCREEN_KEEP
extern void screenkeep_init();
//extern int gHaveScreenkeep;
#endif

extern unsigned char ico_fangqukai[1431];
extern unsigned char ico_fangquguan[1160];

/*
static MENU_ITEM g_SCENE_MODE[MAX_SCENE_MODE] = {
        {"会客", 0, 0},
        {"就餐 ", 1, 1},
        {"影院 ", 2, 2},
        {"就寝 ", 3, 3},
        {"温馨 ", 4, 5},
        {"在家", 5, 6},
        {"离家", 6, 7}
};
*/
T_SCENE_MODE_TABLE   g_SCENE_MODE_TABLE[MAX_SCENE_MODE];
int g_cur_select_scene_mode = -1;

#if 0
extern  WRT_Rect rc_jiajuemenuico[4];

void CreateSceneSetSub(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SceneSetSubEventHandler;
        pWindow->NotifyReDraw = DrawSceneSetSubMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawSceneSetSubMain(){
        JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(rc_jiajuemenuico[0].x,rc_jiajuemenuico[0].y,qingjing_peizhi1,sizeof(qingjing_peizhi1));
        JpegDisp(rc_jiajuemenuico[1].x,rc_jiajuemenuico[1].y,anfang_peizhi1,sizeof(anfang_peizhi1));
        JpegDisp(rc_jiajuemenuico[2].x,rc_jiajuemenuico[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_jiajuemenuico[3].x,rc_jiajuemenuico[3].y,bt_guanping1,sizeof(bt_guanping1));
}

int SceneSetSubEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<4;index++){
                if(IsInside(rc_jiajuemenuico[index],xy)){
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
                        pos_x = rc_jiajuemenuico[index].x;
                        pos_y = rc_jiajuemenuico[index].y;
                        switch(index){
                case 0: //情景控制
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,qingjing_peizhi2,sizeof(qingjing_peizhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,qingjing_peizhi1,sizeof(qingjing_peizhi1));
                                CreateConfigMain(1);
                        }
                        break;
                case 1://安防控制
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,anfang_peizhi2,sizeof(anfang_peizhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,anfang_peizhi1,sizeof(anfang_peizhi1));
                                CreateSafeWin(1); //安防窗口
                        }
                        break;
                case 2://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 3://关屏幕
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
#endif

/////////////////////////////////////////////////////////////////
//
/*
        {150,57,132,160},
        {324,57,132,160},
        {498,57,132,160},

        {63,236,132,160},
        {238,236,132,160},
        {418,236,132,160},
        {586,236,132,160},
        
        
        {30,145,132,160},    //情景控制配置
        {180,145,132,160},   //室内控制配置
        {330,145,132,160},  //情景定时控制
        {480,145,132,160},   //远程控制开/关
        {630,145,132,160},   //家居对码     
        
        {200,57,132,160},    //情景控制配置
        {409,57,132,160},   //室内控制配置
        {113,236,132,160},  //情景定时控制
        {306,236,132,160},   //远程控制开/关
        {509,236,132,160},   //家居对码           
*/        
//
//

        
static const WRT_Rect rc_jiajusetmenuico[7]={
        {150,145,132,160},    //情景控制配置
        {350,145,132,160},   //室内控制配置
        {550,145,132,160},  //情景定时控制
        {750,145,132,160},   //远程控制开/关
        {0,0,0,0},   //家居对码            

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

void CreateHomeSetWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = HomeSetEventHandler;
        pWindow->NotifyReDraw = DrawHomeSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

static void DrawRemote()
{
        WRT_Rect rt;
        int xoffset = 0; //21
        rt.x = rc_jiajusetmenuico[2].x;
        rt.y  = rc_jiajusetmenuico[2].h+rc_jiajusetmenuico[2].y-24;
        rt.w = rc_jiajusetmenuico[2].w;
        rt.h = 50;
        //if(GetCurrentLanguage() == ENGLISH)
        	FillRect(BACK_COLOR,rt);
        if(pSystemInfo->mySysInfo.isremote == 0){
                JpegDisp(rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y,remote_close1,sizeof(remote_close1));
                DrawEnText(LT("远程关"),rc_jiajusetmenuico[2].w  ,rc_jiajusetmenuico[2].h,rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y);
        }else{
                JpegDisp(rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y,remote_open1,sizeof(remote_open1));
                DrawEnText(LT("远程开"),rc_jiajusetmenuico[2].w ,rc_jiajusetmenuico[2].h,rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y);
        }
}
void DrawHomeSetMain(){
        int xoffset = 0; //21
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        
        JpegDisp(rc_jiajusetmenuico[0].x,rc_jiajusetmenuico[0].y,qingjingkongzhi_peizhi1,sizeof(qingjingkongzhi_peizhi1));

        JpegDisp(rc_jiajusetmenuico[1].x,rc_jiajusetmenuico[1].y,qingjingtimer1,sizeof(qingjingtimer1));
        JpegDisp(rc_jiajusetmenuico[3].x,rc_jiajusetmenuico[3].y,jiaju_duima1,sizeof(jiaju_duima1));

        if(pSystemInfo->mySysInfo.isremote == 0){
                JpegDisp(rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y,remote_close1,sizeof(remote_close1));
                DrawEnText(LT("远程关"),rc_jiajusetmenuico[2].w ,rc_jiajusetmenuico[2].h,rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y);
        }else{
                JpegDisp(rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y,remote_open1,sizeof(remote_open1));
                DrawEnText(LT("远程开"),rc_jiajusetmenuico[2].w ,rc_jiajusetmenuico[2].h,rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y);
        }

        DrawEnText(LT("情景配置"),rc_jiajusetmenuico[0].w + xoffset,rc_jiajusetmenuico[0].h,rc_jiajusetmenuico[0].x,rc_jiajusetmenuico[0].y);
        DrawEnText(LT("定时配置"),rc_jiajusetmenuico[1].w + xoffset,rc_jiajusetmenuico[1].h,rc_jiajusetmenuico[1].x,rc_jiajusetmenuico[1].y);
        DrawEnText(LT("智联对码"),rc_jiajusetmenuico[3].w + xoffset,rc_jiajusetmenuico[3].h,rc_jiajusetmenuico[3].x,rc_jiajusetmenuico[3].y);

        JpegDisp(rc_jiajusetmenuico[5].x,rc_jiajusetmenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_jiajusetmenuico[6].x,rc_jiajusetmenuico[6].y,bt_guanping1,sizeof(bt_guanping1));
}

int HomeSetEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<7;index++){
                if(IsInside(rc_jiajusetmenuico[index],xy)){
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
                        pos_x = rc_jiajusetmenuico[index].x;
                        pos_y = rc_jiajusetmenuico[index].y;
                        switch(index){
                case 0: //情景控制
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,qingjingkongzhi_peizhi2,sizeof(qingjingkongzhi_peizhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,qingjingkongzhi_peizhi1,sizeof(qingjingkongzhi_peizhi1));
                                //CreateSceneSetWin2();
                                CreateSceneWin2();
                        }
                        break;
                case 4://室内控制
                	break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,shineikongzhi_peizhi2,sizeof(shineikongzhi_peizhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,shineikongzhi_peizhi1,sizeof(shineikongzhi_peizhi1));
                                //CreateRoomControlSetWin();
                                CreateConfigMain(0); //add.2008.5.27. by ljw .
                                //CreateHostSetWin();
                                if(GetJiajuCount() != 0)
                                        CreateDialogWin2(LT("清空旧的配置表?"),MB_OK|MB_CANCEL,clear_jiaju_table,NULL);
                        }
                        break;
                case 1: //定时配置
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,qingjingtimer2,sizeof(qingjingtimer2));
                        }else{
                                JpegDisp(pos_x,pos_y,qingjingtimer1,sizeof(qingjingtimer1));
                                CreateTimerScene();
                        }
                        break;
                case 2://远程开关
                        if(status == 1){
                                StartButtonVocie();
                                /*
                                
                                if(pSystemInfo->mySysInfo.isremote == 0)
                                        JpegDisp(pos_x,pos_y,remote_close2,sizeof(remote_close2));
                                else
                                        JpegDisp(pos_x,pos_y,remote_open2,sizeof(remote_open2));
                        }else{
                                if(pSystemInfo->mySysInfo.isremote == 0)
                                        JpegDisp(pos_x,pos_y,remote_open1,sizeof(remote_open1));
                                else
                                        JpegDisp(pos_x,pos_y,remote_close1,sizeof(remote_close1));
                                */
#if 1
                                unsigned long tmpevent[4];
                                if(pSystemInfo->mySysInfo.isremote != 0)
                                        pSystemInfo->mySysInfo.isremote = 0;
                                else
                                        pSystemInfo->mySysInfo.isremote = 1;
                                g_isUpdated = 0;
                                DrawRemote();
                                update_rect(rc_jiajusetmenuico[2].x,rc_jiajusetmenuico[2].y,rc_jiajusetmenuico[2].w,rc_jiajusetmenuico[2].h+20);
                                g_isUpdated = 1;

                                tmpevent[0] = ROOM_REMOTE_SWITCH;
                                tmpevent[1] = 1;
                                tmpevent[2]  =pSystemInfo->mySysInfo.isremote;
                                wrthost_send_cmd(tmpevent);
                                save_localhost_config_to_flash();
#endif
                        }
                        break;
                case 3://家居对码
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,jiaju_duima2,sizeof(jiaju_duima2));
                        }else{
                                JpegDisp(pos_x,pos_y,jiaju_duima1,sizeof(jiaju_duima1));
                                CreateHostSetWin();
                        }
                        break;
                case 5://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 6://关屏幕
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


///////////////////////////////////////////////////////////////
//情景配置菜单1
#if 0
static const WRT_Rect rc_scenesetmenuico[9]={
        {150,57,132,160},
        {324,57,132,160},
        {498,57,132,160},

        {63,236,132,160},
        {238,236,132,160},
        {418,236,132,160},
        {586,236,132,160},

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};
void CreateSceneSetWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SceneSetEventHandler;
        pWindow->NotifyReDraw = DrawSceneSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}
void DrawSceneSetMain(){
        int xoffset = 21;
        JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(rc_scenesetmenuico[0].x,rc_scenesetmenuico[0].y,menu_huike1,sizeof(menu_huike1));
        JpegDisp(rc_scenesetmenuico[1].x,rc_scenesetmenuico[1].y,menu_jiucan1,sizeof(menu_jiucan1));
        JpegDisp(rc_scenesetmenuico[2].x,rc_scenesetmenuico[2].y,menu_yingyuan1,sizeof(menu_yingyuan1));

        JpegDisp(rc_scenesetmenuico[3].x,rc_scenesetmenuico[3].y,menu_jiuqin1,sizeof(menu_jiuqin1));
        JpegDisp(rc_scenesetmenuico[4].x,rc_scenesetmenuico[4].y,menu_wenxin1,sizeof(menu_wenxin1));


        JpegDisp(rc_scenesetmenuico[5].x,rc_scenesetmenuico[5].y,bt_zaijia1,sizeof(bt_zaijia1));
        JpegDisp(rc_scenesetmenuico[6].x,rc_scenesetmenuico[6].y,bt_lijia1,sizeof(bt_lijia1));

        JpegDisp(rc_scenesetmenuico[7].x,rc_scenesetmenuico[7].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_scenesetmenuico[8].x,rc_scenesetmenuico[8].y,bt_guanping1,sizeof(bt_guanping1));

        DrawEnText(LT2("Guest"),rc_scenesetmenuico[0].w+xoffset,rc_scenesetmenuico[0].h,rc_scenesetmenuico[0].x,rc_scenesetmenuico[0].y);
        DrawEnText(LT2("Dinner"),rc_scenesetmenuico[1].w+xoffset,rc_scenesetmenuico[1].h,rc_scenesetmenuico[1].x,rc_scenesetmenuico[1].y);
        DrawEnText(LT2("Cinema"),rc_scenesetmenuico[2].w+xoffset,rc_scenesetmenuico[2].h,rc_scenesetmenuico[2].x,rc_scenesetmenuico[2].y);
        DrawEnText(LT2("Sleeping"),rc_scenesetmenuico[3].w+xoffset,rc_scenesetmenuico[3].h,rc_scenesetmenuico[3].x,rc_scenesetmenuico[3].y);
        DrawEnText(LT2("Warm"),rc_scenesetmenuico[4].w+xoffset,rc_scenesetmenuico[4].h,rc_scenesetmenuico[4].x,rc_scenesetmenuico[4].y);
        DrawEnText(LT2("Home"),rc_scenesetmenuico[5].w+xoffset,rc_scenesetmenuico[5].h,rc_scenesetmenuico[5].x,rc_scenesetmenuico[5].y);
        DrawEnText(LT2("Outside"),rc_scenesetmenuico[6].w+xoffset,rc_scenesetmenuico[6].h,rc_scenesetmenuico[6].x,rc_scenesetmenuico[6].y);

}

int clear_scene_mode(int param){
        CreateDialogWin2(LT("正在清空情景配置..."),MB_NONE,NULL,NULL);
        T_SCENE_MODE_TABLE tmptable;
        memcpy(&tmptable,&g_SCENE_MODE_TABLE[g_cur_select_scene_mode],sizeof(T_SCENE_MODE_TABLE));
        memset(&g_SCENE_MODE_TABLE[g_cur_select_scene_mode],0,sizeof(T_SCENE_MODE_TABLE));
        if(!save_scene_mode_table_to_flash()){ //保存失败，恢复原来的数据
                CloseWindow();
                CreateDialogWin2(LT("清空失败!"),MB_OK,NULL,NULL);
                memcpy(&g_SCENE_MODE_TABLE[g_cur_select_scene_mode],&tmptable,sizeof(T_SCENE_MODE_TABLE));
               // SDL_Delay(2000);
               SDL_Delay(2000);
                CloseWindow();
        }else{
                Jiaju_SetScene(pSystemInfo->mySysInfo.sysID,g_cur_select_scene_mode,1);
                CloseWindow();
                CreateDialogWin2(LT("清空成功!"),MB_NONE,NULL,NULL);
                SDL_Delay(2000);
                CloseWindow();

        }
        return 0;
}
int SceneSetEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<9;index++){
                if(IsInside(rc_scenesetmenuico[index],xy)){
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
                        pos_x = rc_scenesetmenuico[index].x;
                        pos_y = rc_scenesetmenuico[index].y;
                        switch(index){
                case 0: //会客
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_huike2,sizeof(menu_huike2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_huike1,sizeof(menu_huike1));
                                g_cur_select_scene_mode = 0;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'会客'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);

                        }
                        break;
                case 1://就餐
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jiucan2,sizeof(menu_jiucan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_jiucan1,sizeof(menu_jiucan1));
                                g_cur_select_scene_mode = 1;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'就餐'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);
                        }
                        break;
                case 2://影院
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_yingyuan2,sizeof(menu_yingyuan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_yingyuan1,sizeof(menu_yingyuan1));
                                g_cur_select_scene_mode = 2;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'影院'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);
                        }
                        break;
                case 3://就寝
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jiuqin2,sizeof(menu_jiuqin2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_jiuqin1,sizeof(menu_jiuqin1));
                                g_cur_select_scene_mode = 3;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'就寝'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);
                        }
                        break;
                case 4://温馨
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_wenxin2,sizeof(menu_wenxin2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_wenxin1,sizeof(menu_wenxin1));
                                g_cur_select_scene_mode = 4;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'温馨'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);
                        }
                        break;
                case 5://在家
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_zaijia2,sizeof(bt_zaijia2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_zaijia1,sizeof(bt_zaijia1));
                                g_cur_select_scene_mode = 5;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'在家'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);
                        }
                        break;
                case 6://离家
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_lijia2,sizeof(bt_lijia2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_lijia1,sizeof(bt_lijia1));
                                g_cur_select_scene_mode = 6;
#if HAVE_JIAJU_SAFE
                                CreateSceneSetSub();
#else
                                CreateConfigMain(1);
#endif
                                CreateDialogWin2(LT("清空旧的'离家'情景配置?"),MB_OK | MB_CANCEL,clear_scene_mode,NULL);
                        }
                        break;
                case 7://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 8://关屏幕
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
#endif
///////////////////////////////////////////////////////
//


/////////////////////////////////////////////////////////////
/*
void CreateRoomControlSetWin(){
CreateSceneControlwin(1);
}
void DrawRoomControlSetMain(){
}
int RoomControlSetEventHandler(int x,int y,int status){
return SceneControlEventHandler(x,y,status);
}
*/
//////////////////////////////////////////////////////////////////////
//
#define MAX_INPUT_BITS 3

extern char  g_number[12];




MENU_ITEM    g_DEVTYPE[MAX_DEV_SORT]={{"电脑类", 0, 1},
{"智能遥控器", 1, 2},
{"智能终端", 2, 3},
{"智能插座", 3, 4},
{"智能开关", 4, 5},
{"智能插座和开关", 5, 6},
{"智能红外转发器", 6, 7},
{"智能窗帘控制器", 7, 8},
{"智能灯光控制器", 8, 9},
{"宠物喂养器", 9, 10},
{"ECB设备",10,11}
};
//空调模式
static MENU_ITEM g_AIR_MODE[MAX_AIR_MODE] = {
        {"关", 0, C_CLOSE},
        {"开", 1, C_OPEN},
        {"22度", 2, C_KEY2},
        {"25度", 3, C_KEY5},
        {"27度", 4, C_KEY7},
        {"28度", 5, C_KEY8},
        {"29度", 6, C_KEY9},
        {"自定义1", 7, 8},
        {"自定义2", 8, 9},
        {"自定义3", 9, 10}
};
//窗帘模式
static MENU_ITEM g_CURTAIN_MODE[MAX_AIR_MODE] = {
        {"关", 0, C_CLOSE},
        {"开", 1, C_OPEN},
        {"停", 2, C_STOP},
        {"自定义1",3,4},
        {"自定义2",4,5},
        {"自定义3",5,6},
        {"自定义4",6,7},
        {"自定义5",7,8},
        {"自定义6",8,9},
        {"自定义7",9,10}
};
//其他模式
static MENU_ITEM g_SWITCH_MODE[MAX_AIR_MODE] = {
        {"关", 0, C_CLOSE},
        {"开", 1, C_OPEN},
        {"解锁", 2, C_UNLOCK},
        {"锁定", 3, C_LOCK},
        {"自定义1",4,5},
        {"自定义2",5,6},
        {"自定义3",6,7},
        {"自定义4",7,8},
        {"自定义5",8,9},
        {"自定义6",9,10}
};

static MENU_ITEM g_TV_MODE[MAX_AIR_MODE]={
        {"关",0,C_CLOSE},
        {"开",1,C_OPEN},
        {"频道+",2,C_CHADD},
        {"频道-",3,C_CHSUB},
        {"音量+",4,C_VOLADD},
        {"音量-",5,C_VOLSUB},
        {"自定义1",6,7},
        {"自定义2",7,8},
        {"自定义3",8,9},
        {"自定义4",9,10}
};

//可调压模式？？？
MENU_ITEM adjustable_mode[MAX_AIR_MODE] =	{
        {"关", 0, C_CLOSE},
        {"开", 1, C_OPEN},
        {"强", 2, C_UP},
        {"弱", 3, C_DOWN},
        {"解锁", 4, C_UNLOCK},
        {"锁定", 5, C_LOCK},
        {"自定义1",6,7},
        {"自定义2",7,8},
        {"自定义3",8,9},
        {"自定义4",9,10}
};




int        g_cur_select_dev_type = 0;
static int g_cur_select_edit = 1;          //当前挑选的EDIT
static int g_prev_select_edit = 1;         //前一个挑选的EDIT
static int g_MAX_EDIT_COUNT = 5;           //默认最大的编辑框最大的个数
static int g_cur_input_count = 0;    //当前输入的位数不超过MAX_INPUT_BITS
static int g_is_ir_control = 0;

/*
static char g_cur_input_RFtype[12];       //当前输入RF类型
static char g_cur_input_RFnumber[12];     //但前收入RF编码
static char g_cur_input_RFsubnumber[12];    //但前收入RF分类编码
static char g_cur_input_RIUtype[12];        //当前输入的RIU类型
static char g_cur_input_RIUnumber[12];      //当前输入的RIU编码
*/
static char g_cur_input_number[16][12]; //当前输入的数据

static unsigned long* pLampNumberPointer=NULL;
static unsigned long* pLampNumberPointer1=NULL;
static unsigned long* pLampNumberSize=NULL;
static unsigned long* pLampNumberSize1=NULL;
static const WRT_Rect rc_setmenuico[24]={
        {200,30,247,40},               //设备类型
        {136,100,80,40},               //设备地址
        {136,160,80,40},               //设备子地址
        {340,100,80,40},               //RIU类型
        {340,160,80,40},               //RIU编号
        {550,100,80,40},               //RIU485地址
        {550,160,80,40},               //红外类型
                   
        {160,230,80,40},               //操作模式
        {460,230,80,40},
        {160,290,80,40},
        {460,290,80,40},
        {160,350,80,40},
        {460,350,80,40},
        {160,410,80,40},
        {460,410,80,40},
        {160,470,80,40},
        {460,470,80,40},

        {120,5,80,80},                //上箭头
        {450,5,80,80},                //下箭头

        {650,100,300,400},            //数字符号所在区域
        {660,531,111,69},              //学习
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};
static int jiaju_study(){
        T_ELEC_ID tmpjiaju;
        tmpjiaju.roomsort = (unsigned char)g_cur_select_room_type;
        tmpjiaju.roomname = (unsigned char)g_cur_select_room;
        tmpjiaju.elecsort = (unsigned char)g_cur_select_elec_type;
        tmpjiaju.elecname = (unsigned char)g_cur_select_elec;
        int index = get_jiaju(&tmpjiaju);
        if(index == -1){
                index = get_idle_jiaju_table();
                if(index == -1)
                        return -1;
        }
        return Jiaju_SetupIrStudy(&g_JIAJU_TABLE[index],0);
}

static int set_rf_number(){
        int ret =0;
        T_JIAJU_TABLE   tmpJiaju;
        tmpJiaju.valid = 1;
        tmpJiaju.JiajuID.roomsort = (unsigned char)g_cur_select_room_type;
        tmpJiaju.JiajuID.roomname = (unsigned char)g_cur_select_room;
        tmpJiaju.JiajuID.elecsort = (unsigned char)g_cur_select_elec_type;
        tmpJiaju.JiajuID.elecname = (unsigned char)g_cur_select_elec;
        tmpJiaju.RfID.RFsort = (unsigned char)g_DEVTYPE[g_cur_select_dev_type].number;
        tmpJiaju.RfID.RFNo = (unsigned char)(strtoul(g_cur_input_number[0],NULL,16));
        tmpJiaju.RfID.SubSw = (unsigned char)(strtoul(g_cur_input_number[1],NULL,16));
        if(tmpJiaju.RfID.RFsort != 0x0b){
                if(tmpJiaju.RfID.SubSw  ==  0x0)
                        tmpJiaju.RfID.SubSw = 0x0;
                else
                        tmpJiaju.RfID.SubSw -= 0x1;
        }
        tmpJiaju.RiuID.RIUsort =(unsigned char)(strtoul(g_cur_input_number[2],NULL,16));
        tmpJiaju.RiuID.RIUNo  = (unsigned char)(strtoul(g_cur_input_number[3],NULL,16))+0x80;
        tmpJiaju.RiuID.RIU485Addr  = (unsigned char)(strtoul(g_cur_input_number[4],NULL,16))+0x80;
        memcpy(tmpJiaju.SysID, pSystemInfo->mySysInfo.sysID,4);
        CreateDialogWin2(LT("正在设置设备地址..."),MB_NONE,NULL,NULL);
        if(Jiaju_SetRfAddr(&tmpJiaju, tmpJiaju.RfID.RFNo) == 0){
                //Jiaju_SetSysID(&tmpJiaju, tmpJiaju.SysID);
                CloseWindow();
                CreateDialogWin2(LT("设置设备地址成功..."),MB_OK,NULL,NULL);
        }else{
                CloseWindow();
                CreateDialogWin2(LT("设置设备地址失败..."),MB_OK,NULL,NULL);
        }
}

static int save_jiaju_config(){
        T_ELEC_ID tmpjiaju;
        int ishex = 16;
        tmpjiaju.roomsort = (unsigned char)g_cur_select_room_type;
        tmpjiaju.roomname = (unsigned char)g_cur_select_room;
        tmpjiaju.elecsort = (unsigned char)g_cur_select_elec_type;
        tmpjiaju.elecname = (unsigned char)g_cur_select_elec;
        int index = get_jiaju(&tmpjiaju);
        if(index == -1){
                index = get_idle_jiaju_table();
                if(index == -1)
                        return -1;
        }
        if(g_cur_select_dev_type == 10)
                ishex = 10;

        g_JIAJU_TABLE[index].valid = 1;
        g_JIAJU_TABLE[index].JiajuID.roomsort = (unsigned char)g_cur_select_room_type;
        g_JIAJU_TABLE[index].JiajuID.roomname = (unsigned char)g_cur_select_room;
        g_JIAJU_TABLE[index].JiajuID.elecsort = (unsigned char)g_cur_select_elec_type;
        g_JIAJU_TABLE[index].JiajuID.elecname = (unsigned char)g_cur_select_elec;
        g_JIAJU_TABLE[index].RfID.RFsort = (unsigned char)g_DEVTYPE[g_cur_select_dev_type].number;
        g_JIAJU_TABLE[index].RfID.RFNo = (unsigned char)(strtoul(g_cur_input_number[0],NULL,ishex));
        g_JIAJU_TABLE[index].RfID.SubSw = (unsigned char)(strtoul(g_cur_input_number[1],NULL,ishex));
        if(g_JIAJU_TABLE[index].RfID.RFsort  != 0xb){
                if(g_JIAJU_TABLE[index].RfID.SubSw   ==  0x0)
                        g_JIAJU_TABLE[index].RfID.SubSw = 0x0;
                else
                        g_JIAJU_TABLE[index].RfID.SubSw -= 0x1;
        }

        g_JIAJU_TABLE[index].RiuID.RIUsort =(unsigned char)(strtoul(g_cur_input_number[2],NULL,ishex));
        if(g_cur_select_dev_type == 10){
                  g_JIAJU_TABLE[index].RiuID.RIUNo = (unsigned char)(g_is_ir_control & 0x000000ff);
                  g_JIAJU_TABLE[index].RiuID.RIU485Addr = (unsigned char)(strtoul(g_cur_input_number[4],NULL,ishex));
        }else{
                g_JIAJU_TABLE[index].RiuID.RIUNo  = (unsigned char)(strtoul(g_cur_input_number[3],NULL,ishex))+0x80; //2009-8-14 16:41:26
                g_JIAJU_TABLE[index].RiuID.RIU485Addr  = (unsigned char)(strtoul(g_cur_input_number[4],NULL,ishex))+0x80; //2009-8-14 16:41:31
        }
        memcpy(g_JIAJU_TABLE[index].SysID, pSystemInfo->mySysInfo.sysID,4);
        WRT_DEBUG("Index = %d--%d ",index,g_JIAJU_TABLE[index].RfID.RFsort);
        if(g_cur_select_dev_type == 6 || (g_is_ir_control == 1)){
                int i =0;
                if(g_cur_select_elec_type == 0){//灯光
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }else if(g_cur_select_elec_type == 1){//窗帘
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }else if(g_cur_select_elec_type == 2){//电视
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }else if(g_cur_select_elec_type == 3){//空调
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }else if(g_cur_select_elec_type == 4){//DVD
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }else if(g_cur_select_elec_type == 5){//插座
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }else if(g_cur_select_elec_type == 7){//自定义
                        for(i=0; i<MAX_IR_MODE;i++){
                                g_JIAJU_TABLE[index].IrMode[i].IrSort = g_JIAJU_TABLE[index].RfID.SubSw;//(unsigned char)(strtoul(g_cur_input_number[5],NULL,16));
                                g_JIAJU_TABLE[index].IrMode[i].IrIndex = (unsigned char)(strtoul(g_cur_input_number[i+6],NULL,ishex));
                        }
                }

        }
        save_jiaju_table_to_flash();
        return 0;
}

unsigned char get_select_control_index(T_JIAJU_TABLE* pJiaju,unsigned char cmdmode)
{
        int i ;
        unsigned char status = -1;
        int elec_type = pJiaju->JiajuID.elecsort;
        if(elec_type == 0){//灯光
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(adjustable_mode[i].number == cmdmode){
                                status = (unsigned char)(adjustable_mode[i].id & 0x000000ff);
                                break;
                        }
                }
                
        }else if(elec_type == 1){ //窗帘
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(g_CURTAIN_MODE[i].number == cmdmode){
                                status = (unsigned char) (g_CURTAIN_MODE[i].id & 0x000000ff);
                                break;
                        }
                }       
        }else if(elec_type == 2){//电视
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(g_TV_MODE[i].number == cmdmode){
                                status = (unsigned char)  (g_TV_MODE[i].id& 0x000000ff);
                                break;
                        }
                } 
        }else if(elec_type == 3){//空调
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(g_AIR_MODE[i].number == cmdmode){
                                status =  (unsigned char)(g_AIR_MODE[i].id & 0x000000ff);
                        }
                } 
        }else if(elec_type == 4){//DVD
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(g_SWITCH_MODE[i].number == cmdmode){
                                status =  (unsigned char)( g_SWITCH_MODE[i].id & 0x000000ff);
                        }
                } 
        }else if(elec_type == 5){ //插座
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(g_SWITCH_MODE[i].number == cmdmode){
                                status =  (unsigned char)( g_SWITCH_MODE[i].id & 0x000000ff);
                        }
                } 
        }else if(elec_type == 7){ //自定义
                for(i = 0; i<MAX_AIR_MODE;i++){
                        if(g_SWITCH_MODE[i].number == cmdmode){
                                status =  (unsigned char)( g_SWITCH_MODE[i].id & 0x000000ff);
                        }
                } 
        }

        return status;
}

unsigned char get_select_control_text(T_JIAJU_TABLE* pJiaju,int index,char* ptext)
{
        unsigned    char  status = 0;
        int elec_type = pJiaju->JiajuID.elecsort;
        if(elec_type == 0){//灯光
                sprintf(ptext,"%s",LT(adjustable_mode[index].name));
                 status = (unsigned char)adjustable_mode[index].number;
        }else if(elec_type == 1){ //窗帘
                sprintf(ptext,"%s",LT(g_CURTAIN_MODE[index].name));
                    status =(unsigned char)g_CURTAIN_MODE[index].number;
        }else if(elec_type == 2){//电视
                sprintf(ptext,"%s",LT(g_TV_MODE[index].name));
                status =(unsigned char)g_TV_MODE[index].number;
        }else if(elec_type == 3){//空调
                sprintf(ptext,"%s",LT(g_AIR_MODE[index].name));
                status =(unsigned char)g_AIR_MODE[index].number;
        }else if(elec_type == 4){//DVD
                sprintf(ptext,"%s",LT(g_SWITCH_MODE[index].name));
                status =(unsigned char)g_SWITCH_MODE[index].number;
        }else if(elec_type == 5){ //插座
                sprintf(ptext,"%s",LT(g_SWITCH_MODE[index].name));
                status =(unsigned char)g_SWITCH_MODE[index].number;
        }else if(elec_type == 7){ //自定义
                sprintf(ptext,"%s",LT(g_SWITCH_MODE[index].name));
                status =(unsigned char)g_SWITCH_MODE[index].number;

        }

        return status;

}



static void DrawSetInfo(){
        int xoffset = 0;
        WRT_Rect rt;
        //  JpegDisp(rc_setmenuico[0].x,rc_setmenuico[0].y,bt_setxuanzekuang,sizeof(bt_setxuanzekuang));
        DrawRect(rc_setmenuico[0],2,rgb_white);
       
        rt = rc_setmenuico[0];
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);

        xoffset = (rc_setmenuico[0].w - GetTextWidth(LT(g_DEVTYPE[g_cur_select_dev_type].name),1))/2;
        if(xoffset < 0)
                xoffset = 1;
        xoffset += rc_setmenuico[0].x;

        DrawText_16(LT(g_DEVTYPE[g_cur_select_dev_type].name),xoffset,rc_setmenuico[0].y+rc_setmenuico[0].h-15,rgb_black);
        WRT_Rect rcbox;

        rcbox.x = 5;
        rcbox.y = rc_setmenuico[0].y+rc_setmenuico[0].h+10;
        rcbox.w = 640-rcbox.x;
        rcbox.h = 531 - rcbox.y ;
        FillRect(BACK_COLOR,rcbox);
          
        DrawText_16(LT("设备类型:"),10,rc_setmenuico[0].y+28,rgb_white);
        DrawText_16(LT("设备地址:"),10,rc_setmenuico[1].y+28,rgb_white);
        DrawText_16(LT("设备子地址:"),10,rc_setmenuico[2].y+28,rgb_white);
        if(g_cur_select_dev_type == 10){ //CBus设备
                DrawText_16(LT("通路地址:"),rc_setmenuico[3].x-90,rc_setmenuico[3].y+28,rgb_white);
                DrawText_16(LT("红外控制"),rc_setmenuico[4].x-90,rc_setmenuico[4].y+28,rgb_white);
                if(g_cur_select_elec_type == 5)
                       DrawText_16(LT("关联防区:"),rc_setmenuico[5].x-100,rc_setmenuico[5].y+28,rgb_white);
                if(g_is_ir_control == 1)
                        JpegDisp(rc_setmenuico[4].x,rc_setmenuico[4].y,ico_fangqukai,sizeof(ico_fangqukai));
                else
                        JpegDisp(rc_setmenuico[4].x,rc_setmenuico[4].y,ico_fangquguan,sizeof(ico_fangquguan));
        }else{
                DrawText_16(LT("RIU类型:"),rc_setmenuico[3].x-90,rc_setmenuico[3].y+28,rgb_white);
                DrawText_16(LT("RIU编号:"),rc_setmenuico[4].x-90,rc_setmenuico[4].y+28,rgb_white);
                DrawText_16(LT("RIU地址:"),rc_setmenuico[5].x-100,rc_setmenuico[5].y+28,rgb_white);
        }
        
        rcbox.x = 5;
        rcbox.y = rc_setmenuico[7].y-10;
        rcbox.w = 608-rcbox.x;
        rcbox.h = 521-rcbox.y;

        for(int j=1; j<6;j++){
                if(g_cur_select_dev_type == 10 && j > 3){
                	if(g_cur_select_elec_type == 5){
                		if(j != 5)
                			continue;
                		
                        }else
                        	continue;
                        
                }
                rt = rc_setmenuico[j];
                DrawRect(rt,2,rgb_white);
                DeflateRect(&rt,4);
                FillRect(rgb_white,rt);
                //JpegDisp(rc_setmenuico[j].x,rc_setmenuico[j].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                DrawText_16(g_cur_input_number[j-1],rc_setmenuico[j].x+10,rc_setmenuico[j].y+28,rgb_black);
        }

        if(g_cur_select_dev_type == 6 || (g_cur_select_dev_type == 10 && g_is_ir_control == 1 )){ //红外智能控制器或者CBus设备
                int i=0;
                char tmpname[20];
                //      JpegDisp(rc_setmenuico[20].x,rc_setmenuico[20].y,xuexi_bt1,sizeof(xuexi_bt1)); //2009-7-14
                if(g_cur_select_dev_type == 10 && g_is_ir_control == 1){
                        for(i = 7; i<17;i++){
                                rt = rc_setmenuico[i];
                                DrawRect(rt,2,rgb_white);
                                DeflateRect(&rt,4);
                                FillRect(rgb_white,rt);
                                //pegDisp(rc_setmenuico[i].x,rc_setmenuico[i].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                                DrawText_16(g_cur_input_number[i-1],rc_setmenuico[i].x+10,rc_setmenuico[i].y+28,rgb_black);

                        }
                }else{
                        for(i = 6; i<17;i++){
                                //JpegDisp(rc_setmenuico[i].x,rc_setmenuico[i].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                                rt = rc_setmenuico[i];
                                DrawRect(rt,2,rgb_white);
                                DeflateRect(&rt,4);
                                FillRect(rgb_white,rt);
                                DrawText_16(g_cur_input_number[i-1],rc_setmenuico[i].x+10,rc_setmenuico[i].y+28,rgb_black);

                        }
                        DrawText_16(LT("红外类型:"),rc_setmenuico[6].x-100,rc_setmenuico[6].y+28,rgb_white);
                }

                if(g_cur_select_elec_type == 0){//灯光
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(adjustable_mode[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }

                }else if(g_cur_select_elec_type == 1){ //窗帘
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(g_CURTAIN_MODE[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }
                }else if(g_cur_select_elec_type == 2){//电视
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(g_TV_MODE[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }
                }else if(g_cur_select_elec_type == 3){//空调
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(g_AIR_MODE[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }

                }else if(g_cur_select_elec_type == 4){//DVD
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(g_SWITCH_MODE[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }
                }else if(g_cur_select_elec_type == 5){ //插座
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(g_SWITCH_MODE[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }
                }else if(g_cur_select_elec_type == 7){ //自定义
                        for(i=0;i<MAX_IR_MODE;i++){
                                sprintf(tmpname,"%s:\n",LT(g_SWITCH_MODE[i].name));
                                DrawText_16(tmpname,rc_setmenuico[i+7].x-140,rc_setmenuico[i+7].y+28,rgb_white);
                        }
                }
                DrawRect(rcbox,1,rgb_blue);

                g_MAX_EDIT_COUNT = 16;
        
        }else{
                g_MAX_EDIT_COUNT = 5;
                FillRect(BACK_COLOR,rcbox);
                rcbox.x = rc_setmenuico[6].x-100;
                rcbox.y = rc_setmenuico[6].y;
                rcbox.w = 100+rc_setmenuico[6].w;
                rcbox.h = 28+rc_setmenuico[6].h;
                FillRect(BACK_COLOR,rcbox);

        }
         DrawRect(rc_setmenuico[g_cur_select_edit],2,rgb_yellow);
}

static int setnumberhandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        WRT_Rect rt;
        int startx = rc_setmenuico[19].x;
        int starty = rc_setmenuico[19].y;
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer1[count],pLampNumberSize1[count]);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer[count],pLampNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_cur_input_count <(MAX_INPUT_BITS)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                g_cur_input_count++;
                                                if(strlen(g_cur_input_number[g_cur_select_edit-1]) < 3){
                                                        strcat(g_cur_input_number[g_cur_select_edit-1],(char*)&c);                         
                                                        DrawText_16(g_cur_input_number[g_cur_select_edit-1],rc_setmenuico[g_cur_select_edit].x+10,rc_setmenuico[g_cur_select_edit].y+28,rgb_black);
                                                       // update_rect(rc_setmenuico[g_cur_select_edit].x,rc_setmenuico[g_cur_select_edit].y,rc_setmenuico[g_cur_select_edit].w,rc_setmenuico[g_cur_select_edit].h);
                                                       // g_isUpdated = 1;
                                                }

                                        }else if(count  == 9){ //*
                                                g_cur_input_count--;
                                                if(g_cur_input_count < 0)
                                                        g_cur_input_count = 0;
                                                g_cur_input_number[g_cur_select_edit-1][g_cur_input_count]='\0';
                                                rt = rc_setmenuico[g_cur_select_edit];
                                                DeflateRect(&rt,4);
                                                g_isUpdated = 0;
                                                FillRect(rgb_white,rt);
                                                //JpegDisp(rc_setmenuico[g_cur_select_edit].x,rc_setmenuico[g_cur_select_edit].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                                                //DrawRect(rc_setmenuico[g_cur_select_edit],2,rgb_yellow);
                                                DrawText_16(g_cur_input_number[g_cur_select_edit-1],rc_setmenuico[g_cur_select_edit].x+10,rc_setmenuico[g_cur_select_edit].y+28,rgb_black);
                                                update_rect(rt.x,rt.y,rt.w,rt.h);
                                                g_isUpdated = 1;
                                        }else if(count == 11){//# //确认
                                                if(g_cur_select_edit == 1){ //设置RF地址
                                                        set_rf_number();
                                                        break;
                                                }
                                                g_cur_input_count = 0;
                                                g_prev_select_edit = g_cur_select_edit;
                                                g_cur_select_edit++;
                                                if(g_cur_select_edit > g_MAX_EDIT_COUNT)
                                                        g_cur_select_edit = 1;
                                                rt = rc_setmenuico[g_prev_select_edit];
                                                g_isUpdated = 0;
                                                DrawRect(rt,2,rgb_white);
                                                DeflateRect(&rt,4);
                                                FillRect(rgb_white,rt);
                                                //JpegDisp(rc_setmenuico[g_prev_select_edit].x,rc_setmenuico[g_prev_select_edit].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                                                DrawText_16(g_cur_input_number[g_prev_select_edit-1],rc_setmenuico[g_prev_select_edit].x+10,rc_setmenuico[g_prev_select_edit].y+28,rgb_black);
                                                update_rect(rc_setmenuico[g_prev_select_edit].x,rc_setmenuico[g_prev_select_edit].y,rc_setmenuico[g_prev_select_edit].w,rc_setmenuico[g_prev_select_edit].h);
                                                 rt = rc_setmenuico[g_cur_select_edit];
                                                g_isUpdated = 0;
                                                DrawRect(rt,2,rgb_yellow);
                                                DeflateRect(&rt,4);
                                                FillRect(rgb_white,rt);
                                                //JpegDisp(rc_setmenuico[g_cur_select_edit].x,rc_setmenuico[g_cur_select_edit].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                                                //DrawRect(rc_setmenuico[g_cur_select_edit],2,rgb_yellow);
                                                DrawText_16(g_cur_input_number[g_cur_select_edit-1],rc_setmenuico[g_cur_select_edit].x+10,rc_setmenuico[g_cur_select_edit].y+28,rgb_black);
                                                update_rect(rc_setmenuico[g_cur_select_edit].x,rc_setmenuico[g_cur_select_edit].y,rc_setmenuico[g_cur_select_edit].w,rc_setmenuico[g_cur_select_edit].h);
                                                g_isUpdated = 1;

                                        }
                                }//else status == 0
                                break;
                        }//if(x > tempx && x < (tempx+76) && y > tempy  && y < (tempy+72)
                }//for
                return ret;
}




void CreateSetWin(int set_type){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SetEventHandler;
        pWindow->NotifyReDraw = DrawSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        //  g_cur_control_type = set_type;
        g_cur_input_count = 0;
        g_is_ir_control = 0;
        memset(g_cur_input_number,0,16*12*sizeof(char));
        T_ELEC_ID tmpjiaju;
        tmpjiaju.roomsort = (unsigned char)g_cur_select_room_type;
        tmpjiaju.roomname = (unsigned char)g_cur_select_room;
        tmpjiaju.elecsort = (unsigned char)g_cur_select_elec_type;
        tmpjiaju.elecname = (unsigned char)g_cur_select_elec;
        int index = get_jiaju(&tmpjiaju);
        if(index == -1){
                g_cur_select_dev_type =0;
                g_MAX_EDIT_COUNT = 5;
                for(int i=0; i<10;i++){
                        // sprintf(g_cur_input_number[i+6],"%x",g_JIAJU_TABLE[index].IrMode[i].IrIndex);
                        sprintf(g_cur_input_number[i+6],"%x",i);
                }

        }else{
                int i=0;
                g_cur_select_dev_type = 0;
                for(i=0;i<MAX_DEV_SORT;i++){
                        if(g_DEVTYPE[i].number == (int)g_JIAJU_TABLE[index].RfID.RFsort){
                                g_cur_select_dev_type =  i;
                                break;
                        }

                }
                if(g_cur_select_dev_type == 10){ //CBus设备。
                        sprintf(g_cur_input_number[0],"%d",g_JIAJU_TABLE[index].RfID.RFNo); //设备地址
                        sprintf(g_cur_input_number[1],"%d",g_JIAJU_TABLE[index].RfID.SubSw);//设备子地址
                        sprintf(g_cur_input_number[2],"%d",g_JIAJU_TABLE[index].RiuID.RIUsort);//通路地址
                        if(g_cur_select_elec_type == 5) //插座
                        	sprintf(g_cur_input_number[4],"%d",g_JIAJU_TABLE[index].RiuID.RIU485Addr); //表示关联的防区号，0默认为不关联。只能关联4-8防区
                        if(g_JIAJU_TABLE[index].RiuID.RIUNo == 0x01){ //表示存在红外
                                g_is_ir_control = 1;
                                for(int i=0; i<10;i++){
                                       sprintf(g_cur_input_number[i+6],"%d",g_JIAJU_TABLE[index].IrMode[i].IrIndex);
                                       // sprintf(g_cur_input_number[i+6],"%x",i);
                                }
                        }

                }else{

                        sprintf(g_cur_input_number[0],"%x",g_JIAJU_TABLE[index].RfID.RFNo);  
                         sprintf(g_cur_input_number[1],"%x",g_JIAJU_TABLE[index].RfID.SubSw+0x01);
                        sprintf(g_cur_input_number[2],"%x",g_JIAJU_TABLE[index].RiuID.RIUsort);
                        sprintf(g_cur_input_number[3],"%x",g_JIAJU_TABLE[index].RiuID.RIUNo-0x80);
                        sprintf(g_cur_input_number[4],"%x",g_JIAJU_TABLE[index].RiuID.RIU485Addr-0x80);
                        if(g_cur_select_dev_type == 6){
                                g_MAX_EDIT_COUNT = 16;
                                //sprintf(g_cur_input_number[5],"%x",g_JIAJU_TABLE[index].IrMode[0].IrSort);

                        }

                        sprintf(g_cur_input_number[5],"%x",g_JIAJU_TABLE[index].RfID.SubSw+0x01);
                        for(int i=0; i<10;i++){
                                // sprintf(g_cur_input_number[i+6],"%x",g_JIAJU_TABLE[index].IrMode[i].IrIndex);
                                sprintf(g_cur_input_number[i+6],"%x",i);
                        }
                }



        }

        GetNumberJpgPointer(&pLampNumberPointer,&pLampNumberPointer1);
        GetNumberJpegSizePointer(&pLampNumberSize,&pLampNumberSize1);
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}
void DrawSetMain(){
        WRT_Rect rt;
        int i;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        
        DrawRect(rc_setmenuico[0],2,rgb_white);
        rt = rc_setmenuico[0];
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);
       
        for(i =1;i<4;i++)
        {
                rt = rc_setmenuico[i];
                DrawRect(rt,2,rgb_white);
                DeflateRect(&rt,4);
                FillRect(rgb_white,rt);
        }
        
        if(g_cur_select_dev_type != 10){
                for(i = 4; i<6;i++){
                        rt = rc_setmenuico[i];
                        DrawRect(rt,2,rgb_white);
                        DeflateRect(&rt,4);
                        FillRect(rgb_white,rt);
                }
        }else{
        	if(g_cur_select_elec_type == 5){ //如果是插座
        		rt = rc_setmenuico[5];
                        DrawRect(rt,2,rgb_white);
                        DeflateRect(&rt,4);
                        FillRect(rgb_white,rt);
        	}
        }

        JpegDisp(rc_setmenuico[17].x,rc_setmenuico[17].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_setmenuico[18].x,rc_setmenuico[18].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));

        
        JpegDisp(rc_setmenuico[22].x,rc_setmenuico[22].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_setmenuico[23].x,rc_setmenuico[23].y,bt_guanping1,sizeof(bt_guanping1));
        int j =0;
        int startx = rc_setmenuico[19].x;
        int starty = rc_setmenuico[19].y;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pLampNumberPointer[(i*3)+j],pLampNumberSize[(i*3)+j]);
                }
                DrawText_16(LT("设备类型:"),10,rc_setmenuico[0].y+28,rgb_white);
                DrawText_16(LT("设备地址:"),10,rc_setmenuico[1].y+28,rgb_white);
                DrawText_16(LT("设备子地址:"),10,rc_setmenuico[2].y+28,rgb_white);
                if(g_cur_select_dev_type == 10){ //CBus设备
                        DrawText_16(LT("通路地址:"),rc_setmenuico[3].x-90,rc_setmenuico[3].y+28,rgb_white);
                        DrawText_16(LT("红外控制"),rc_setmenuico[4].x-90,rc_setmenuico[4].y+28,rgb_white);
                        if(g_cur_select_elec_type == 5){
                        	DrawText_16(LT("关联防区:"),rc_setmenuico[5].x-100,rc_setmenuico[5].y+28,rgb_white);
                        	//JpegDisp(rc_setmenuico[5].x,rc_setmenuico[5].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                        }
                        if(g_is_ir_control == 1)
                                JpegDisp(rc_setmenuico[4].x,rc_setmenuico[4].y,ico_fangqukai,sizeof(ico_fangqukai));
                        else
                                JpegDisp(rc_setmenuico[4].x,rc_setmenuico[4].y,ico_fangquguan,sizeof(ico_fangquguan));
                }else{
                        DrawText_16(LT("RIU类型:"),rc_setmenuico[3].x-90,rc_setmenuico[3].y+28,rgb_white);
                        DrawText_16(LT("RIU编号:"),rc_setmenuico[4].x-90,rc_setmenuico[4].y+28,rgb_white);
                        DrawText_16(LT("RIU地址:"),rc_setmenuico[5].x-100,rc_setmenuico[5].y+28,rgb_white);
                }


                g_cur_select_edit = 1;
                g_prev_select_edit = 1;

                DrawSetInfo();
               
                g_cur_input_count = strlen(g_cur_input_number[g_cur_select_edit-1]);
                WRT_Rect curbox;
                curbox.x = 135;
                curbox.y = 531;
                curbox.w = 578;
                curbox.h = 55;
	        JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
	             
		JpegDisp(rc_setmenuico[21].x,rc_setmenuico[21].y,menu_ok1,sizeof(menu_ok1));            
                char tmptext[100];  //,按'#'键红外学习!
                if(GetCurrentLanguage() == ENGLISH){
                        sprintf(tmptext,"set '%s'",LT(g_ROOMNAME[g_cur_select_room_type][g_cur_select_room].name));
                        DrawText_16(tmptext,curbox.x,curbox.y+35,rgb_red);

                        sprintf(tmptext,"-'%s'",LT(g_ELECTNAME[g_cur_select_elec_type][g_cur_select_elec].name));
                        DrawText_16(tmptext,curbox.x,curbox.y+55,rgb_red);
                }else{
                        sprintf(tmptext,"请配置'%s'的'%s'!\n",LT(g_ROOMNAME[g_cur_select_room_type][g_cur_select_room].name),LT(g_ELECTNAME[g_cur_select_elec_type][g_cur_select_elec].name));
                        DrawText_16(tmptext,curbox.x,curbox.y+45,rgb_red);
                }
}
int SetEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        WRT_Rect rt;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<24;index++){
                if(IsInside(rc_setmenuico[index],xy)){
                        ret = 1;
                        pos_x = rc_setmenuico[index].x;
                        pos_y = rc_setmenuico[index].y;
                        switch(index){
                case 0: //RF类型选择
                        break;
                case 17: // 上箭头
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                g_cur_select_dev_type--;
                                if(g_cur_select_dev_type < 0)
                                        g_cur_select_dev_type = (MAX_DEV_SORT-1);
				g_isUpdated = 0;
				g_cur_select_edit = 1;
                                g_prev_select_edit = 1;
                                DrawSetInfo();
				update_rect(0,0,640,531);
				g_isUpdated  = 1;
                        }
                        break;
                case 18://房间下箭头
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));
                                g_cur_select_dev_type++;
                                if(g_cur_select_dev_type > (MAX_DEV_SORT-1))
                                        g_cur_select_dev_type = 0;
				g_isUpdated = 0;
				g_cur_select_edit = 1;
                                g_prev_select_edit = 1;
                                DrawSetInfo();
				update_rect(0,0,640,531);
				g_isUpdated  = 1;
                        }
                        break;
                case 19://数字区域
                        ret = setnumberhandler(x,y,status);
                        break;
                case 20: //红外学习
                        break; //2009-7-14
                        if(g_cur_select_dev_type == 6){
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,xuexi_bt2,sizeof(xuexi_bt2));
                                }else{
                                        JpegDisp(pos_x,pos_y,xuexi_bt1,sizeof(xuexi_bt1));
                                        CreateDialogWin2(LT("红外学习中..."),MB_NONE,NULL,NULL);
                                        if(jiaju_study() == 0){
                                                CloseWindow();
                                                CreateDialogWin2(LT("学习红外成功"),MB_OK,NULL,NULL);
                                        }else{
                                                CloseWindow();
                                                CreateDialogWin2(LT("学习红外失败"),MB_OK,NULL,NULL);
                                        }
                                }
                        }
                        break;
                case 21://确定
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                if( save_jiaju_config() == 0){
                                        CloseWindow();
                                        CreateDialogWin2(LT("配置成功"),MB_OK,NULL,NULL);
                                }else{
                                        CloseWindow();
                                        CreateDialogWin2(LT("配置失败"),MB_OK,NULL,NULL);
                                }
                        }
                        break;
                case 22://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 23://关屏幕
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                default:
                        if(status != 1)
                                break;
                        if((g_cur_select_dev_type != 6) ){
                                if(g_is_ir_control == 0){
                                        if(index < 1 || index > 5){
                                                break;
                                        }
                                }
                        }
                        if(g_cur_select_dev_type == 10){
                                if(index  == 5 && g_cur_select_elec_type !=5)
                                        break;
                                if(index == 6)
                                        break;
                                if(index  == 4)
                                {
                                        g_is_ir_control =!g_is_ir_control;
                                        //if(g_is_ir_control == 1)
                                         //        JpegDisp(rc_setmenuico[4].x,rc_setmenuico[4].y,ico_select_2,sizeof(ico_select_2));
                                        //else
                                         //        JpegDisp(rc_setmenuico[4].x,rc_setmenuico[4].y,ico_select_1,sizeof(ico_select_1));
                                        g_isUpdated = 0;
                                        g_cur_select_edit = 1;
                                        g_prev_select_edit = 1;
                                        DrawSetInfo();
                                        update_rect(0,0,640,531);
                                        g_isUpdated  = 1;										
										
                                        break;
                                }
                        }
                        g_prev_select_edit = g_cur_select_edit;
                        g_cur_select_edit = index;
                        g_cur_input_count = strlen(g_cur_input_number[g_cur_select_edit-1]);
                       
                        g_isUpdated = 0;
                        rt = rc_setmenuico[g_prev_select_edit];
                        DrawRect(rt,2,rgb_white);
                        DeflateRect(&rt,4);
                        FillRect(rgb_white,rt);
                        //JpegDisp(rc_setmenuico[g_prev_select_edit].x,rc_setmenuico[g_prev_select_edit].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                        DrawText_16(g_cur_input_number[g_prev_select_edit-1],rc_setmenuico[g_prev_select_edit].x+10,rc_setmenuico[g_prev_select_edit].y+28,rgb_black);
                        update_rect(rc_setmenuico[g_prev_select_edit].x,rc_setmenuico[g_prev_select_edit].y,rc_setmenuico[g_prev_select_edit].w,rc_setmenuico[g_prev_select_edit].h);
                        
                        rt = rc_setmenuico[g_cur_select_edit];
                        DrawRect(rt,2,rgb_white);
                        DeflateRect(&rt,4);
                        FillRect(rgb_white,rt);
                        DrawRect(rc_setmenuico[g_cur_select_edit],2,rgb_yellow);
                        DrawText_16(g_cur_input_number[g_cur_select_edit-1],rc_setmenuico[g_cur_select_edit].x+10,rc_setmenuico[g_cur_select_edit].y+28,rgb_black);
                        update_rect(rc_setmenuico[g_cur_select_edit].x,rc_setmenuico[g_cur_select_edit].y,rc_setmenuico[g_cur_select_edit].w,rc_setmenuico[g_cur_select_edit].h);
                        g_isUpdated = 1;
                        break;
                        }
                        break;
                }
        }
        return ret;
}

///////////////////////////////////////////////
//

static int g_cur_select_mode_name =0; //当前挑选的子模式（空调具体的模式等等）
static int g_scene_type = 0;

static int g_is_once = 0;    



static const WRT_Rect rc_scenecontrolico[10]={
        {228,131,360,48},
        {228,231,360,48},
        {144,115,64,64},
        {603,115,64,64},
        {144,215,64,64},
        {603,215,64,64},
        {353,313,96,60},               //安防
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

int send_scene_mode(int mode){ //采用广播模式
        if(mode < 0 || mode > 9)
                return -1;
         WRT_DEBUG("控制的情景模式%d ,%d",mode,pSystemInfo->mySysInfo.scenenumber[mode]);
         control_scene((int)pSystemInfo->mySysInfo.scenenumber[mode]);     
         pSystemInfo->mySysInfo.cur_scene = mode;
         UpdateSystemInfo();
         g_cur_set_scene_mode = pSystemInfo->mySysInfo.cur_scene;
        //int ret = Jiaju_SetScene(pSystemInfo->mySysInfo.sysID,pSystemInfo->mySysInfo.scenenumber[mode],0);
        return 0;
}

int get_cur_scenemode_safe_status(int mode,int index){
        return g_SCENE_MODE_TABLE[mode].safestatus[index];
}

int set_cur_scenemode_safe_status(int mode,int index,int status){
        if(index == 0 || index == 1)
                return 1;
        //printf("g_cur_select_scene_mode =%d,index = %d,status = %d \n",g_cur_select_scene_mode,index,status);
        g_SCENE_MODE_TABLE[mode].safestatus[index]=status;
        return 1;
}

static int save_scene_mode(){
#if 0	
        T_ELEC_ID   JiajuID;
        int         index =-1;
        int         scene_index = -1;
        int         status_index = -1;
        int         i=0;
        unsigned    char  status = 0;
        int         sort1 = -1;
        unsigned char  status1 = 0;
        JiajuID.roomsort = (unsigned char)g_cur_select_room_type;
        JiajuID.roomname = (unsigned char)g_cur_select_room;
        JiajuID.elecsort = (unsigned char)g_cur_select_elec_type;
        JiajuID.elecname = (unsigned char)g_cur_select_elec;
        index = get_jiaju(&JiajuID);
        if(index == -1)
                return -1;
        for(i=0; i< MAX_JIAJU_TABLE_COUNT;i++){
                if(  g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[i].sort == index &&
                        g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[i].valid == 1){
                                scene_index = i;
                                break;
                }
        }
        if(scene_index == -1){
                scene_index = get_idle_scene_table(g_cur_select_scene_mode);
                if(scene_index == -1)
                        return -1;
        }

        if(g_cur_select_elec_type == 0){//灯光
                //status_index=g_SWITCH_MODE[g_cur_select_mode_name].id;
                status = (unsigned char)adjustable_mode[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 1){ //窗帘
                //status_index=g_CURTAIN_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_CURTAIN_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 2){ //电视
                // status_index=g_TV_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_TV_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 3){ //空调
                //status_index=g_AIR_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_AIR_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 4){ //DVD
                //status_index=g_SWITCH_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_SWITCH_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 5){//插座
                status = (unsigned char)g_SWITCH_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 7){//自定义
                status = (unsigned char)g_SWITCH_MODE[g_cur_select_mode_name].number;
        }

        /*  //写死了红外的索引，暂时没有使用
        if(g_JIAJU_TABLE[index].RfID.RFsort == 6){ //红外设备,存红外的索引值
        status = g_cur_select_mode_name;//(unsigned char)g_JIAJU_TABLE[index].IrMode[status_index].IrIndex;
        }
        */

        //如果是修改，先保存原来的状态
        if( g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].valid == 1){
                sort1 = g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].sort;
                status1 = g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].status;
        }

        g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].sort = (unsigned char)index; //家居表索引
        g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].status = (unsigned char)status;
        g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].valid = 1;

        //存16防区的状态。
        //将配置发送给底层
        if(save_scene_mode_table_to_flash()){
                //pSystemInfo->mySysInfo.cenemode[g_cur_select_scene_mode] = 0;
                WRT_DEBUG("g_cur_select_mode_name = %d,status = %d ",g_cur_select_mode_name,status);
                return Jiaju_SetupScene(&g_JIAJU_TABLE[index],g_cur_select_scene_mode,status);
        }else{ //如果保存失败，则回复原来的设置。
                if(sort1 != -1){
                        g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].sort = sort1;
                        g_SCENE_MODE_TABLE[g_cur_select_scene_mode].scene_table[scene_index].status  = status1;
                }
        }
        
#endif        
        return -1;


}

int send_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec,unsigned char status){
        T_ELEC_ID   JiajuID;
        int         index =-1;
        unsigned char status1=0;
        JiajuID.roomsort = (unsigned char)roomtype;
        JiajuID.roomname = (unsigned char)room;
        JiajuID.elecsort = (unsigned char)electtype;
        JiajuID.elecname = (unsigned char)elec;
        index = get_jiaju(&JiajuID);
        if(index == -1){
                WRT_DEBUG("没有找到该家居配置信息index == -1");
                return -2;
        }
        if(electtype == 0){//灯光
                status1 = (unsigned char)adjustable_mode[status].number;
        }else if(electtype == 1){ //窗帘
                status1 =(unsigned char)g_CURTAIN_MODE[status].number;
        }else if(electtype == 2){ //电视
                status1 =(unsigned char)g_TV_MODE[status].number;
        }else if(electtype == 3){ //空调
                status1 =(unsigned char)g_AIR_MODE[status].number;
        }else if(electtype == 4){ //DVD
                status1 =(unsigned char)g_SWITCH_MODE[status].number;
        }else if(electtype == 5){//插座
                status1 = (unsigned char)g_SWITCH_MODE[status].number;
        }else if(electtype == 6){//自定义
                status1 = (unsigned char)g_SWITCH_MODE[status].number;
        }

        /*
        if(g_JIAJU_TABLE[index].RfID.RFsort == 0x7){ //红外设备
        status1 = status;
        }
        */

        return  Jiaju_SetStatus(&g_JIAJU_TABLE[index], status1);
}

unsigned char* get_jiaju_config_buf_for_center(int* len){
        unsigned char* jiaju_buf;
        unsigned char* p = 0;
        int i =0,j=0;
        int buflen = 0;
        jiaju_buf = (unsigned char*)ZENMALLOC(sizeof(g_JIAJU_TABLE)+16);
        for(i=0;i<16;i++){
#if HAVE_JIAJU_SAFE
                *(jiaju_buf+i) = (unsigned char)get_cur_scenemode_safe_status(g_cur_set_scene_mode,i);//pSystemInfo->mySysInfo.safestatus[i];
#else
                *(jiaju_buf+i) = (unsigned char)pSystemInfo->mySysInfo.safestatus[i];
#endif
        }
        p = jiaju_buf+16;
        for(i =0; i< MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                        *(p+j*5) = (unsigned char)g_JIAJU_TABLE[i].JiajuID.roomsort+0x01;
                        *(p+1+j*5) = (unsigned char)g_JIAJU_TABLE[i].JiajuID.roomname+0x01;
                        *(p+2+j*5) = (unsigned char)g_JIAJU_TABLE[i].JiajuID.elecsort+0x01;
                        *(p+3+j*5) = (unsigned char)g_JIAJU_TABLE[i].JiajuID.elecname+0x01;
                        *(p+4+j*5) = 0xfd;//(unsigned char)Jiaju_GetStatus(&g_JIAJU_TABLE[i]); 不查状态，直接返回状态未明
                        j++;
                }
        }
        *len = j*5+16;
        return jiaju_buf;
}

int get_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec){
        T_ELEC_ID   JiajuID;
        int         index =-1;

        JiajuID.roomsort = (unsigned char)roomtype;
        JiajuID.roomname = (unsigned char)room;
        JiajuID.elecsort = (unsigned char)electtype;
        JiajuID.elecname = (unsigned char)elec;
        index = get_jiaju(&JiajuID);
        if(index == -1){
                WRT_DEBUG("没有找到该家居配置信息index == -1");
                return -2;
        }
        return  Jiaju_GetStatus(&g_JIAJU_TABLE[index]);
}


int  send_jiaju_status(){
        //1:获得当前的电器，房间的索引
        //2:从家居配置表中，获得配置好的，记录
        //3:将记录，打包发送给底层
        T_ELEC_ID   JiajuID;
        int         index =-1;
        int         ret = 0;
        int         scene_index = -1;
        int         status_index = -1;
        unsigned    char  status = 0;
        JiajuID.roomsort = (unsigned char)g_cur_select_room_type;
        JiajuID.roomname = (unsigned char)g_cur_select_room;
        JiajuID.elecsort = (unsigned char)g_cur_select_elec_type;
        JiajuID.elecname = (unsigned char)g_cur_select_elec;
        index = get_jiaju(&JiajuID);
        if(index == -1){
               WRT_DEBUG("没有找到该家居配置信息index == -1");
                return -1;
        }

        if(g_cur_select_elec_type == 0){//灯光
                status_index=adjustable_mode[g_cur_select_mode_name].id;
                status = (unsigned char)adjustable_mode[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 1){ //窗帘
                status_index=g_CURTAIN_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_CURTAIN_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 2){ //电视
                status_index=g_TV_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_TV_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 3){ //空调
                status_index=g_AIR_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_AIR_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 4){ //DVD
                status_index=g_SWITCH_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_SWITCH_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 5){//插座
                status_index=g_SWITCH_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_SWITCH_MODE[g_cur_select_mode_name].number;
        }else if(g_cur_select_elec_type == 7){//自定义
                status_index=g_SWITCH_MODE[g_cur_select_mode_name].id;
                status =(unsigned char)g_SWITCH_MODE[g_cur_select_mode_name].number;
        }
        /*
        if(g_JIAJU_TABLE[index].RfID.RFsort == 6){ //红外设备
        status = status_index;//(unsigned char)g_JIAJU_TABLE[index].IrMode[status_index].IrIndex;
        }
        */
        WRT_DEBUG("g_cur_select_mode_name = %d,status = %d ",g_cur_select_mode_name,status);
        ret = Jiaju_SetStatus(&g_JIAJU_TABLE[index], status);
        if(ret == -3)
                ret = 0;
        return  ret;

}

static void DrawSceneControlText(){
        int xoffset =0;
        //JpegDisp(rc_scenecontrolico[0].x,rc_scenecontrolico[0].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));
        WRT_Rect rt;
        rt = rc_scenecontrolico[0];
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,3);
        FillRect(BACK_COLOR,rt);
        if(g_cur_select_elec_type == 0){//灯光
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(adjustable_mode[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(adjustable_mode[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }else if(g_cur_select_elec_type == 1){ //窗帘
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(g_CURTAIN_MODE[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(g_CURTAIN_MODE[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }else if(g_cur_select_elec_type == 2){ //电视
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(g_TV_MODE[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(g_TV_MODE[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }else if(g_cur_select_elec_type == 3){ //空调
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(g_AIR_MODE[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(g_AIR_MODE[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }else if(g_cur_select_elec_type == 4){ //DVD
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(g_SWITCH_MODE[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(g_SWITCH_MODE[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }else if(g_cur_select_elec_type == 5){ //插座
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(g_SWITCH_MODE[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(g_SWITCH_MODE[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }else if(g_cur_select_elec_type == 7){ //自定义
                xoffset = (rc_scenecontrolico[0].w - GetTextWidth(LT(g_SWITCH_MODE[g_cur_select_mode_name].name),0))/2;
                if(xoffset < 0)
                        xoffset = 0;
                xoffset += rc_scenecontrolico[0].x;
                DrawText(LT(g_SWITCH_MODE[g_cur_select_mode_name].name),xoffset,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);
        }
        //DrawText(g_room_type[g_cur_sceneroom_type].name,rc_scenecontrolico[0].x+rc_scenecontrolico[0].w/3,rc_scenecontrolico[0].y+rc_scenecontrolico[0].h-15,rgb_white);

}

void CreateSceneControlwin(int type){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SceneControlEventHandler;
        pWindow->NotifyReDraw = DrawSceneControlMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        g_cur_select_mode_name =0;
        g_scene_type = type;
#if 0
        //只有当g_scene_type == 1的时候，需要获得当前家居的状态
        if(g_scene_type == 1){
                T_ELEC_ID   JiajuID;
                int         index =-1;
                JiajuID.roomsort = (unsigned char)g_cur_select_room_type;
                JiajuID.roomname = (unsigned char)g_cur_select_room;
                JiajuID.elecsort = (unsigned char)g_cur_select_elec_type;
                JiajuID.elecname = (unsigned char)g_cur_select_elec;
                index = get_jiaju(&JiajuID);
                if(index != -1){
                        unsigned char status;
                        int rc =0;
                        rc = Jiaju_GetStatus(&g_JIAJU_TABLE[index]);
                        if(rc !=-1)
                                g_cur_select_mode_name = rc;
                }

        }
#endif
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawSceneControlMain(){
	WRT_Rect rt;
			rt.x = 0;
			rt.y = 0;
			rt.w =1024;
			rt.h = 530;
			FillRect(0xb2136,rt);
	
			JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        //JpegDisp(rc_scenecontrolico[0].x,rc_scenecontrolico[0].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));
        
        rt = rc_scenecontrolico[0];
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,3);
        FillRect(BACK_COLOR,rt);

        JpegDisp(rc_scenecontrolico[2].x,rc_scenecontrolico[2].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_scenecontrolico[3].x,rc_scenecontrolico[3].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));;
        // if(g_scene_type == 0)
        //  JpegDisp(rc_scenecontrolico[6].x,rc_scenecontrolico[6].y,bt_sceneanfang1,sizeof(bt_sceneanfang1));
        JpegDisp(rc_scenecontrolico[7].x,rc_scenecontrolico[7].y,menu_ok1,sizeof(menu_ok1));
        JpegDisp(rc_scenecontrolico[8].x,rc_scenecontrolico[8].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_scenecontrolico[9].x,rc_scenecontrolico[9].y,bt_guanping1,sizeof(bt_guanping1));
        DrawSceneControlText();
}

int  SceneControlEventHandler(int x,int y ,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<10;index++){
                if(IsInside(rc_scenecontrolico[index],xy)){
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
                        pos_x = rc_scenecontrolico[index].x;
                        pos_y = rc_scenecontrolico[index].y;
                        switch(index){
                case 2: //房间 上箭头
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));

                                g_cur_select_mode_name--;
                                if(g_cur_select_mode_name < 0)
                                        g_cur_select_mode_name=(MAX_IR_MODE-1);
                                DrawSceneControlText();
                        }
                        break;
                case 3://房间下箭头
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));
                                g_cur_select_mode_name++;
                                if(g_cur_select_mode_name > (MAX_IR_MODE-1))
                                        g_cur_select_mode_name=0;
                                DrawSceneControlText();
                        }
                        break;
                case 6://安防
                        if(g_scene_type == 1)
                                break;
#if 0
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_sceneanfang2,sizeof(bt_sceneanfang2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_sceneanfang1,sizeof(bt_sceneanfang1));
                                CreateSafeWin();
                        }
#endif
                        break;
                case 7://确定
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                if(g_scene_type ==0){
                                        CreateDialogWin2(LT("配置情景模式中..."),MB_NONE,NULL,NULL);
                                        if(save_scene_mode() == 0){
                                                CloseWindow();
                                                CreateDialogWin2(LT("配置成功"),MB_OK,NULL,NULL);
                                        }else{
                                                CloseWindow();
                                                CreateDialogWin2(LT("配置失败"),MB_OK,NULL,NULL);
                                        }
                                }else if(g_scene_type == 1){
                                        //将每一个电器的控制命令发送给底层
                                        CreateDialogWin2(LT("控制家居中..."),MB_NONE,NULL,NULL);
                                        if(send_jiaju_status() == 0){
                                                CloseWindow();
                                                CreateDialogWin2(LT("控制成功"),MB_OK,NULL,NULL);
                                        }else{
                                                CloseWindow();
                                                CreateDialogWin2(LT("控制失败"),MB_OK,NULL,NULL);
                                        }
                                }

                        }
                        break;
                case 8://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 9://关屏幕
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
///////////////////////////////////////////////////////////////////////////////////
//
//
/////////////////////////////////////////////////////////////////////////////////
#define MAX_LOCALHOST    4

static int g_cur_select_edit2 = 0;  //当前选择的编辑框
static int g_prev_select_edit2 = 0;  //前一个边界框
static char g_cur_input_number2[9][20] ;
static char g_cur_input_count2 = 0;



static WRT_Rect rc_hostsetmenuico[13]={
        {231,50,268,36},   //家居对码
        {231,100,268,36},   //RIU地址
        {231,150,268,36},   //手机号码1：

        {231,200,268,36},   //手机号码2：
        {231,250,268,36},   //手机号码3：
        {231,300,268,36},   //手机号码4：

        {231,350,268,36},   //手机号码5：
        {231,400,268,36},   //手机号码6：
        {231,450,268,36},   //手机号码7：

        {650,100,300,400},            //数字符号所在区域
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};


static void SetDefaultHostInfo()
{

        int i = 0;
        int len = 0;
        char phonenumber[16];
        for(i = 0; i<5;i++){

                memset(phonenumber,0,16);
                
                memcpy(phonenumber,pSystemInfo->mySysInfo.phonenumber[i],15);
                memset(g_cur_input_number2[i+4],0,20);
                len = strlen(phonenumber);
                if( len > 0){
                        strncpy(g_cur_input_number2[i+4],phonenumber,len);
                }

        }
}

static void Drawlocalhostinfo2(){
	char pwd[16];
	memset(pwd,0,16);
        for(int i=0;i<9;i++){
        	if(i== 1){
        		memset(pwd,'*',strlen(g_cur_input_number2[i]));
        		DrawText_16(pwd,rc_hostsetmenuico[i].x+5,rc_hostsetmenuico[i].y+28,rgb_black);
        	}else if(i == 3)
        	{
        		if(g_cur_input_number2[i] == NULL)
        			;
        		else
        			DrawText_16(g_cur_input_number2[i],rc_hostsetmenuico[i].x+5,rc_hostsetmenuico[i].y+28,rgb_black);
        	}else
                	DrawText_16(g_cur_input_number2[i],rc_hostsetmenuico[i].x+5,rc_hostsetmenuico[i].y+28,rgb_black);
        }
}
static void Drawlocalhostinfo(){
        WRT_Rect tmprect;
        char chtext[50];
        int index;

        for(int i = 0 ;i < 9;i++){
                DrawRect(rc_hostsetmenuico[i],2,rgb_white);
                tmprect  = rc_hostsetmenuico[i];
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
        }


        DrawText_16(LT("智联网关ID:"),10,rc_hostsetmenuico[0].y+25,rgb_white);

        DrawText_16(LT("智联对码:"),10,rc_hostsetmenuico[1].y+25,rgb_white);
       // DrawRect(rc_hostsetmenuico[1],2,rgb_white);

        memset(chtext,0,50);
        index = 1;
        sprintf(chtext,"%s",LT("智联网关IP:"));
        DrawText_16(chtext,10,rc_hostsetmenuico[2].y+25,rgb_white);
       // DrawRect(rc_hostsetmenuico[2],2,rgb_white);

        //memset(chtext,0,50);
        //index = 1;
        //sprintf(chtext,"%s%d:",LT("分机ID"),index);
        DrawText_16(LT("分机ID:"),10,rc_hostsetmenuico[3].y+25,rgb_white);
        //DrawRect(rc_hostsetmenuico[3],2,rgb_white);

        memset(chtext,0,50);
        index = 1;
        sprintf(chtext,"%s%d:",LT("手机号码"),index);
        DrawText_16(chtext,10,rc_hostsetmenuico[4].y+25,rgb_white);
        //DrawRect(rc_hostsetmenuico[4],2,rgb_white);

        memset(chtext,0,50);
        index = 2;
        sprintf(chtext,"%s%d:",LT("手机号码"),index);
        DrawText_16(chtext,10,rc_hostsetmenuico[5].y+25,rgb_white);
       // DrawRect(rc_hostsetmenuico[5],2,rgb_white);

        memset(chtext,0,50);
        index = 3;
        sprintf(chtext,"%s%d:",LT("手机号码"),index);
        DrawText_16(chtext,10,rc_hostsetmenuico[6].y+25,rgb_white);
       // DrawRect(rc_hostsetmenuico[6],2,rgb_white);

        memset(chtext,0,50);
        index = 4;
        sprintf(chtext,"%s%d:",LT("手机号码"),index);
        DrawText_16(chtext,10,rc_hostsetmenuico[7].y+25,rgb_white);
        //DrawRect(rc_hostsetmenuico[7],2,rgb_white);

        memset(chtext,0,50);
        index = 5;
        sprintf(chtext,"%s%d:",LT("手机号码"),index);
        DrawText_16(chtext,10,rc_hostsetmenuico[8].y+25,rgb_white);
       // DrawRect(rc_hostsetmenuico[8],2,rgb_white);

        Drawlocalhostinfo2();
}

static void save_localhost_config(){
    int i=0;
    int j;
    memset(pSystemInfo->mySysInfo.sysID,0,16);
    memset(pSystemInfo->mySysInfo.gatewaypwd,0,16);
    strcpy(pSystemInfo->mySysInfo.sysID,g_cur_input_number2[0]);
    strcpy(pSystemInfo->mySysInfo.gatewaypwd,g_cur_input_number2[1]);
    if(strlen(g_cur_input_number2[2]) == 0)
    	pSystemInfo->mySysInfo.gatewayip = 0;
    else{
    	pSystemInfo->mySysInfo.gatewayip = inet_addr(g_cur_input_number2[2]);
    	
    }
    memset(pSystemInfo->mySysInfo.gatewayroomid,'0',15);
    i = strlen(g_cur_input_number2[3]);
    if(i>=15)
    {
   		memcpy(pSystemInfo->mySysInfo.gatewayroomid,g_cur_input_number2[3],15);
   		pSystemInfo->mySysInfo.gatewayroomid[15]='\0';
   	}
   	else
   	{
   		memcpy(pSystemInfo->mySysInfo.gatewayroomid,g_cur_input_number2[3],i);
   		pSystemInfo->mySysInfo.gatewayroomid[15]='\0';
   	}
       	
	/*
	unsigned long tmp = (unsigned long)strtoul(g_cur_input_number2[0],NULL,16);
	WRT_DEBUG("tmp = %x ,%s",tmp,g_cur_input_number2[0]);
	pSystemInfo->mySysInfo.sysID[0] = (tmp >>24)& 0xff;
	pSystemInfo->mySysInfo.sysID[1] = (tmp >>16)& 0xff;
	pSystemInfo->mySysInfo.sysID[2] = (tmp >>8)& 0xff;
	pSystemInfo->mySysInfo.sysID[3] = tmp & 0xff;
	WRT_DEBUG("%x %x %x %x",pSystemInfo->mySysInfo.sysID[0],pSystemInfo->mySysInfo.sysID[1],pSystemInfo->mySysInfo.sysID[2],pSystemInfo->mySysInfo.sysID[3]);
	for(i =0; i< MAX_JIAJU_TABLE_COUNT;i++){
	if(g_JIAJU_TABLE[i].valid == 1){
	memcpy(g_JIAJU_TABLE[i].SysID, pSystemInfo->mySysInfo.sysID,4);
	}
	}
	*/

	for(i = 0;i <5;i++)
	{
		int len = strlen(g_cur_input_number2[i+4]);
		if( len > 0){
			memset(pSystemInfo->mySysInfo.phonenumber[i],0,15);
			strncpy(pSystemInfo->mySysInfo.phonenumber[i],g_cur_input_number2[i+4],len);
			WRT_DEBUG("phone = %s ",pSystemInfo->mySysInfo.phonenumber[i]);
		}
		if(len == 0)
			memset(pSystemInfo->mySysInfo.phonenumber[i],0,15);
	}
	
	save_jiaju_table_to_flash();
	save_localhost_config_to_flash();
	connect_net_jiaju_gateway();
	
}

static int setnumberhandler2(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_hostsetmenuico[9].x;
        int starty = rc_hostsetmenuico[9].y;
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer1[count],pLampNumberSize1[count]);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer[count],pLampNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_cur_input_count2 <(17)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                g_cur_input_count2++;
                                                if(strlen(g_cur_input_number2[g_cur_select_edit2]) < 17){
                                                        strcat(g_cur_input_number2[g_cur_select_edit2],(char*)&c);
                                                        if(g_cur_select_edit2 == 1){ // 密码
                                                		char pwd[16];
                                                		memset(pwd,0,16);
                                                		memset(pwd,'*',strlen(g_cur_input_number2[g_cur_select_edit2]));
                                                		DrawText_16(pwd,rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
                                                	}else
						        					DrawText_16(g_cur_input_number2[g_cur_select_edit2],rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
                                                }

                                        }else if(count == 11){
											c[0] = '.';
											g_cur_input_count2++;
											if(strlen(g_cur_input_number2[g_cur_select_edit2]) < 17){
										        strcat(g_cur_input_number2[g_cur_select_edit2],(char*)&c);
										        if(g_cur_select_edit2 == 1){ // 密码
				                                                		char pwd[16];
				                                                		memset(pwd,0,16);
				                                                		memset(pwd,'*',strlen(g_cur_input_number2[g_cur_select_edit2]));
				                                                		DrawText_16(pwd,rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
				                                                	}else
										        	DrawText_16(g_cur_input_number2[g_cur_select_edit2],rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
											}
                                        }else if(count  == 9){ //*
                                                g_cur_input_count2--;
                                                if(g_cur_input_count2 < 0)
                                                        g_cur_input_count2 = 0;
                                                g_cur_input_number2[g_cur_select_edit2][g_cur_input_count2]='\0';

                                                WRT_Rect  rc1 = rc_hostsetmenuico[g_prev_select_edit2];
                                                DeflateRect(&rc1,4);
                                                g_isUpdated = 0;
                                                FillRect(rgb_white,rc1);
                                                DrawRect(rc_hostsetmenuico[g_cur_select_edit2],2,rgb_yellow);
                                                if(g_cur_select_edit2 == 1){ // 密码
                                                	char pwd[16];
                                                	memset(pwd,0,16);
                                                	memset(pwd,'*',strlen(g_cur_input_number2[g_cur_select_edit2]));
                                                	DrawText_16(pwd,rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
                                                }else
                                                	DrawText_16(g_cur_input_number2[g_cur_select_edit2],rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
                                                update_rect(rc1.x,rc1.y,rc1.w,rc1.h);
                                                g_isUpdated = 1;
                                        }
                                }//else status == 0
                                break;
                        }//if(x > tempx && x < (tempx+76) && y > tempy  && y < (tempy+72)
                }//for
                return ret;
}

//家居对码
void CreateHostSetWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = HostSetEventHandler;
        pWindow->NotifyReDraw = DrawHostSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        g_prev_select_edit2 = 0;
        g_cur_select_edit2 = 0;
        g_cur_input_count2 = 0;
        memset(g_cur_input_number2,0,9*20);
        strcpy(g_cur_input_number2[0],pSystemInfo->mySysInfo.sysID);
        strcpy(g_cur_input_number2[1],pSystemInfo->mySysInfo.gatewaypwd);
        strcpy(g_cur_input_number2[3],pSystemInfo->mySysInfo.gatewayroomid);
        
        toinet_addr(pSystemInfo->mySysInfo.gatewayip,(unsigned char*)g_cur_input_number2[2]);
        //sprintf(g_cur_input_number2[0],"%02x%02x%02x%02x",pSystemInfo->mySysInfo.sysID[0],pSystemInfo->mySysInfo.sysID[1],pSystemInfo->mySysInfo.sysID[2],pSystemInfo->mySysInfo.sysID[3]);
        g_cur_input_count2 = strlen(g_cur_input_number2[0]);
        SetDefaultHostInfo();
        WRT_DEBUG("g_cur_input_count2 =  %d ",g_cur_input_count2);
        GetNumberJpgPointer(&pLampNumberPointer,&pLampNumberPointer1);
        GetNumberJpegSizePointer(&pLampNumberSize,&pLampNumberSize1);
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawHostSetMain(){
		WRT_DEBUG("DrawHostSetMain\n");
		//JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        int i =0;
        int j =0;
        int startx = rc_hostsetmenuico[9].x;
        int starty = rc_hostsetmenuico[9].y;
        for(i=0; i<4;i++)
        {
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pLampNumberPointer[(i*3)+j],pLampNumberSize[(i*3)+j]);
                }
        }
        Drawlocalhostinfo();
        DrawRect(rc_hostsetmenuico[g_cur_select_edit2],2,rgb_yellow);

        // Drawlocalhostinfo2();

        JpegDisp(rc_hostsetmenuico[10].x,rc_hostsetmenuico[10].y,menu_ok1,sizeof(menu_ok1));
        JpegDisp(rc_hostsetmenuico[11].x,rc_hostsetmenuico[11].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_hostsetmenuico[12].x,rc_hostsetmenuico[12].y,bt_guanping1,sizeof(bt_guanping1));


}

int HostSetEventHandler(int x,int y ,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<13;index++){
                if(IsInside(rc_hostsetmenuico[index],xy)){
                        ret = 1;
                        pos_x = rc_hostsetmenuico[index].x;
                        pos_y = rc_hostsetmenuico[index].y;
                        switch(index)
                        {
                        case 9:
                                ret = setnumberhandler2(x,y,status);
                                break;

                        case 10://确定
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                        save_localhost_config();
                                        CloseWindow();
                                        CreateDialogWin2(LT("保存设置成功!"),MB_OK,NULL,NULL);
                                        //需要重新发起连接。连接网关。
                                }
                                break;
                        case 11://返回
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                        CloseTopWindow();
                                }
                                break;
                        case 12://关屏幕
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


                                g_prev_select_edit2 = g_cur_select_edit2;
                                g_cur_select_edit2 = index;
                                g_cur_input_count2 =0;
                                WRT_Rect  rc1 = rc_hostsetmenuico[g_prev_select_edit2];

                                DeflateRect(&rc1,4);
                                g_isUpdated = 0;
                                FillRect(rgb_white,rc1);
                                DrawRect(rc_hostsetmenuico[g_prev_select_edit2],2,rgb_white);
                                
                                //  JpegDisp(rc_hostsetmenuico[g_prev_select_edit2].x,rc_hostsetmenuico[g_prev_select_edit2].y,bt_setxuanzekuang,sizeof(bt_setxuanzekuang));
                                if(g_prev_select_edit2 == 1){
                                	char pwd[16];
                                	memset(pwd,0,16);
                                	memset(pwd,'*',strlen(g_cur_input_number2[g_prev_select_edit2]));
                                	DrawText_16(pwd,rc_hostsetmenuico[g_prev_select_edit2].x+5,rc_hostsetmenuico[g_prev_select_edit2].y+28,rgb_black);
                                }else
                                	DrawText_16(g_cur_input_number2[g_prev_select_edit2],rc_hostsetmenuico[g_prev_select_edit2].x+5,rc_hostsetmenuico[g_prev_select_edit2].y+28,rgb_black);
                                g_isUpdated = 1;
                                update_rect(rc_hostsetmenuico[g_prev_select_edit2].x,rc_hostsetmenuico[g_prev_select_edit2].y,rc_hostsetmenuico[g_prev_select_edit2].w,rc_hostsetmenuico[g_prev_select_edit2].h);

                                rc1 = rc_hostsetmenuico[g_cur_select_edit2];

                                DeflateRect(&rc1,4);
                                g_isUpdated = 0;
                                FillRect(rgb_white,rc1);
                                //  DrawRect(rc_hostsetmenuico[g_cur_select_edit2],2,rgb_white);
                                // JpegDisp(rc_hostsetmenuico[g_cur_select_edit2].x,rc_hostsetmenuico[g_cur_select_edit2].y,bt_setxuanzekuang,sizeof(bt_setxuanzekuang));
                                DrawRect(rc_hostsetmenuico[g_cur_select_edit2],2,rgb_yellow);
                                if(g_cur_select_edit2 == 1){
									char pwd[16];
                                	memset(pwd,0,16);
                                	memset(pwd,'*',strlen(g_cur_input_number2[g_prev_select_edit2]));
                                	DrawText_16(pwd,rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
                                }else
                                	DrawText_16(g_cur_input_number2[g_cur_select_edit2],rc_hostsetmenuico[g_cur_select_edit2].x+5,rc_hostsetmenuico[g_cur_select_edit2].y+28,rgb_black);
                                g_isUpdated = 1;
                                update_rect(rc_hostsetmenuico[g_cur_select_edit2].x,rc_hostsetmenuico[g_cur_select_edit2].y,rc_hostsetmenuico[g_cur_select_edit2].w,rc_hostsetmenuico[g_cur_select_edit2].h);
                                g_cur_input_count2 = strlen(g_cur_input_number2[g_cur_select_edit2]);
                                break;
                        }
                        break;
                }
        }
        return ret;
}



/////////////////////////////////////////////////////////
static char scene_mode_path[]="/home/wrt/scene.txt";
void init_scene_mode_table(){
        int i,j;
        memset(g_SCENE_MODE_TABLE,0,sizeof(g_SCENE_MODE_TABLE));
#if 0
        for( i=0;i<MAX_SCENE_MODE;i++){
                for( j =0; j<16;j++)
                        if(j<4)
                                g_SCENE_MODE_TABLE[i].safestatus[j] = 1;
                        else
                                g_SCENE_MODE_TABLE[i].safestatus[j] = 0;
        }
#endif
        //从flash中读出情景模式配置表
#ifdef TEST_FFS
        if(pSystemInfo->mySysInfo.isusedffs == 1)	{
                FILE* file=NULL;
                int size1 = sizeof(g_SCENE_MODE_TABLE);
                file = fopen(scene_mode_path,"rb");
                if(file){
                        long size = fread((void*)g_SCENE_MODE_TABLE,1,size1,file);
                        if(size == size1){
                                fclose(file);
                                /*
                                for(i = 0; i<MAX_SCENE_MODE;i++){
                                	  g_SCENE_MODE_TABLE[i].scene = i;
                                }*/
                                /*
                                for(i =0;i < 16 ;i++){
                                        g_SCENE_MODE_TABLE[SCENE_LIJIA].safestatus[i] = 1;
                                        g_SCENE_MODE_TABLE[SCENE_ZAIJIA].safestatus[i] = 0;
                                        if(i<2){
                                                int  j=0;
                                                for(j=0; j< 7;j++)
                                                        g_SCENE_MODE_TABLE[j].safestatus[i] = 1;

                                        }
                                }
                                */
                                return;
                        }
                        fclose(file);
                }else{
                	//默认的数据
		  			for( i=0;i<MAX_SCENE_MODE;i++){
		  				g_SCENE_MODE_TABLE[i].scene = i+1;
		  				g_SCENE_MODE_TABLE[i].relation = 0;
                		for( j =0; j<16;j++)
                	                g_SCENE_MODE_TABLE[i].safestatus[j] = 0;
                	}
                }
                file = NULL;
                WRT_DEBUG("阅读情景模式表：%s error = %d ",scene_mode_path,errno);
        }
#endif
        return ;
}
int  get_idle_scene_table(int mode){
        if(mode < 0 || mode > (MAX_SCENE_MODE-1))
                return -1;
/*               
        for(int i =0; i< MAX_JIAJU_TABLE_COUNT;i++)
                if(g_SCENE_MODE_TABLE[mode].scene_table[i].valid == 0)
                        return i;
*/
	for(int i=0; i<MAX_SCENE_MODE;i++)
		if(g_SCENE_MODE_TABLE[i].scene == mode)
			return i;
        return -1;
}



bool  save_scene_mode_table_to_flash(){
#ifdef TEST_FFS
        if(pSystemInfo->mySysInfo.isusedffs == 1)	{
                FILE* file=NULL;
                int size1 = sizeof(g_SCENE_MODE_TABLE);
                file = fopen(scene_mode_path,"wb");
                if(file != NULL){
                        long write_size = fwrite((void*)g_SCENE_MODE_TABLE,1,size1,file);
                        if(write_size == size1){
                                fclose(file);
                                file = NULL;
                                system("sync");
                                return true;
                        }
                        WRT_DEBUG("errno = %d  write_size=%d ",errno,write_size);
                        fclose(file);
                        file = NULL;
                        unlink(scene_mode_path);
                }
                WRT_DEBUG("errno = %d ",errno);
        }
#endif
        return false;

}


//------------------------------------
static int g_jackcount = 0;
static WRT_Rect rtDelay[10]={
        {650,100,300,400},            //数字符号所在区域
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {6,531,111,69},                 //关屏幕	
        
        {32,70,100,36},
        {32,130,100,36},
        {32,190,100,36},
        {32,250,100,36},
        {32,310,100,36},
        {32,370,100,36}
};

void send_delay_jack(int def_value)
{
	int i;
	int count  = 0;
	for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x05){ //如果是插座
                		if(g_JIAJU_TABLE[i].RfID.RFsort == 0xb){ //如果是Cbus	
                			
                			if(def_value ==1){
                				Jiaju_SetCbusStatus2(&g_JIAJU_TABLE[i],0x05,0);
                			}else{ 
                				if(g_jackcount == count)
                					break;         
                				Jiaju_SetCbusStatus2(&g_JIAJU_TABLE[i],0x05,atoi(g_cur_input_number[count]));
                				count++;
                			}
                			
                					
                		}
                	}
                }
        }
       
}

static int leave_delay_ok()
{
        int i;
        g_is_leave = 1; //2009-7-23
        g_have_defend = 1;
        unsigned long event[4]={0};
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
               StartAlaramPromptVocie();
       }
       //逐条发送延时控制。
       send_delay_jack(0);
        CloseAllWindows();	
	return 0;
}

static int setnumberhandler3(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rtDelay[0].x;
        int starty = rtDelay[0].y;
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer1[count],pLampNumberSize1[count]);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer[count],pLampNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_cur_input_count <(MAX_INPUT_BITS)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                g_cur_input_count++;
                                                if(strlen(g_cur_input_number[g_cur_select_edit-4]) < 3){
                                                        strcat(g_cur_input_number[g_cur_select_edit-4],(char*)&c);
                                                        DrawText_16(g_cur_input_number[g_cur_select_edit-4],rtDelay[g_cur_select_edit].x+5,rtDelay[g_cur_select_edit].y+28,rgb_black);
                                                }

                                        }else if(count  == 9){ //*
                                                g_cur_input_count--;
                                                if(g_cur_input_count < 0)
                                                        g_cur_input_count = 0;
                                                g_cur_input_number[g_cur_select_edit-4][g_cur_input_count]='\0';
                                                WRT_Rect tmprect;
                                                tmprect = rtDelay[g_cur_select_edit];
                                                DeflateRect(&tmprect,4);
                                                g_isUpdated = 0;
                                                FillRect(rgb_white,tmprect);
                                                //JpegDisp(rtDelay[g_cur_select_edit].x,rtDelay[g_cur_select_edit].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                                                DrawRect(rtDelay[g_cur_select_edit],2,rgb_yellow);
                                                DrawText_16(g_cur_input_number[g_cur_select_edit-4],rtDelay[g_cur_select_edit].x+5,rtDelay[g_cur_select_edit].y+28,rgb_black);
                                                update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                                g_isUpdated = 1;
                                        }else if(count == 11){//# //确认

                                        }
                                }//else status == 0
                                break;
                        }//if(x > tempx && x < (tempx+76) && y > tempy  && y < (tempy+72)
                }//for
                return ret;
}
static void CloseDelayWin()
{
        g_cur_input_count = 0;
        g_cur_select_edit = 1;
        g_prev_select_edit = 1;
        g_jackcount = 0;
        memset(g_cur_input_number,0,16*12*sizeof(char));	
}
void CreateLeaveDelayWin()
{
 	Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        g_cur_input_count = 0;
        g_cur_select_edit = 4;
        g_prev_select_edit = 4;
        g_jackcount = 0;
        memset(g_cur_input_number,0,16*12*sizeof(char));
        strcpy(g_cur_input_number[0],"0");
        strcpy(g_cur_input_number[1],"0");
        strcpy(g_cur_input_number[2],"0");
        strcpy(g_cur_input_number[3],"0");
        strcpy(g_cur_input_number[4],"0");
        strcpy(g_cur_input_number[5],"0");
                        
        pWindow->EventHandler = LeaveDelayEventHandler;
        pWindow->NotifyReDraw = DrawLeaveDelayWin;
        pWindow->CloseWindow = CloseDelayWin;
        pWindow->valid = 1;
        pWindow->type = CHILD;	
        GetNumberJpgPointer(&pLampNumberPointer,&pLampNumberPointer1);
        GetNumberJpegSizePointer(&pLampNumberSize,&pLampNumberSize1);
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
	
}
void  DrawLeaveDelayWin()
{
 
 	int i,j;
 	int count = 0;
 	WRT_Rect curbox,tmprect;
 	
 	char tmp[100];
        JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
 
        int startx = rtDelay[0].x;
        int starty = rtDelay[0].y;
        for(i=0; i<4;i++){
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pLampNumberPointer[(i*3)+j],pLampNumberSize[(i*3)+j]);
                }
        }
	for( i=0;i<MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x05){ //如果是插座           
                		if(g_JIAJU_TABLE[i].RfID.RFsort == 0xb){ //如果是Cbus
                			if(count == 6)
                				break;
                			memset(tmp,0,100);
                			sprintf(tmp,"%s %s %s",LT(g_ROOMNAME[g_JIAJU_TABLE[i].JiajuID.roomsort][g_JIAJU_TABLE[i].JiajuID.roomname].name),
                				LT(g_ELECTNAME[g_JIAJU_TABLE[i].JiajuID.elecsort][g_JIAJU_TABLE[i].JiajuID.elecname].name),LT("延时时间(分钟):"));

                			starty = rtDelay[count+4].y+26;
                			if(GetCurrentLanguage() == ENGLISH){
                				startx = 5;
                				DrawText_16(tmp,startx,starty,rgb_white);
                				startx += GetTextWidth(tmp,1);
                			}else{
                				startx = 32;
                				DrawText(tmp,startx,starty,rgb_white);
                				startx += GetTextWidth(tmp,0);
                			}
                			startx += 10;
                			starty = rtDelay[count+4].y;
                			
                			//JpegDisp(startx,starty,bt_setsmalledit,sizeof(bt_setsmalledit));
                			rtDelay[count+4].x = startx;
                			rtDelay[count+4].y  = starty;
                			tmprect = rtDelay[count+4];
                			DeflateRect(&tmprect,4);
                			DrawRect(rtDelay[count+4],2,rgb_white);
                			FillRect(rgb_white,tmprect);
                			if(g_cur_select_edit == (count+4)){
                				 DrawRect(rtDelay[count+4],2,rgb_yellow);
                				 g_cur_input_count = strlen(g_cur_input_number[count]);
                			}                			
                			DrawText_16(g_cur_input_number[count],rtDelay[count+4].x+5,rtDelay[count+4].y+28,rgb_black);
                			count++;
					g_jackcount = count;
                			
                			
                		}
                	}
                }
        }

	
        JpegDisp(rtDelay[2].x,rtDelay[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rtDelay[3].x,rtDelay[3].y,bt_guanping1,sizeof(bt_guanping1));	
        
        //请设置插座延时关闭的时间,按'确认'键确认
       
        curbox.x = 135;
        curbox.y = 531;
        curbox.w = 578;
        curbox.h = 55;

	JpegDisp(130,curbox.y,menu_sub_back4,sizeof(menu_sub_back4));
 
	JpegDisp(rtDelay[1].x,rtDelay[1].y,menu_ok1,sizeof(menu_ok1));         
        DrawText_16(LT("请设置插座延时关闭的时间,按'确认'键确认"),curbox.x,curbox.y+45,rgb_red);
        
}
int  LeaveDelayEventHandler(int x,int y,int status)
{
       int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<10;index++){
                if(IsInside(rtDelay[index],xy)){
                        ret = 1;
                        pos_x = rtDelay[index].x;
                        pos_y = rtDelay[index].y;
                        switch(index){
                case 0: //输入框
                	ret = setnumberhandler3(x,y,status);
                        break;

                case 1://确定
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                leave_delay_ok();
                                
                        }
                        break;
                case 2://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 3://关屏幕
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                default:
                	if(status != 1)
                		break;
                        if((index -4) > g_jackcount)
                        	break;
                        WRT_Rect tmprect;
                        g_prev_select_edit = g_cur_select_edit;
                        g_cur_select_edit = index;
                        g_cur_input_count = strlen(g_cur_input_number[g_cur_select_edit-4]);
			
			WRT_MESSAGE("g_prev_select_edit = %d %s ",g_prev_select_edit,g_cur_input_number[g_prev_select_edit-4]);
			tmprect = rtDelay[g_prev_select_edit];
			g_isUpdated = 0;
			DrawRect(tmprect,2,rgb_white);
			DeflateRect(&tmprect,4);
			FillRect(rgb_white,tmprect);
                        DrawText_16(g_cur_input_number[g_prev_select_edit-4],rtDelay[g_prev_select_edit].x+5,rtDelay[g_prev_select_edit].y+28,rgb_black);
                        update_rect(rtDelay[g_prev_select_edit].x,rtDelay[g_prev_select_edit].y,rtDelay[g_prev_select_edit].w,rtDelay[g_prev_select_edit].h);
                        
			tmprect = rtDelay[g_cur_select_edit];
			DeflateRect(&tmprect,4);
			FillRect(rgb_white,tmprect);
                        //JpegDisp(rtDelay[g_cur_select_edit].x,rtDelay[g_cur_select_edit].y,bt_setsmalledit,sizeof(bt_setsmalledit));
                        DrawRect(rtDelay[g_cur_select_edit],2,rgb_yellow);
                        DrawText_16(g_cur_input_number[g_cur_select_edit-4],rtDelay[g_cur_select_edit].x+5,rtDelay[g_cur_select_edit].y+28,rgb_black);
                        update_rect(rtDelay[g_cur_select_edit].x,rtDelay[g_cur_select_edit].y,rtDelay[g_cur_select_edit].w,rtDelay[g_cur_select_edit].h);
                        g_isUpdated = 1;
                        
                        break;
                	}
        	}
	}
        return ret;	
}


//情景配置菜单2，只配置情景号。	
//------------------------------------------------------------------------------
static WRT_Rect rtScenerc[14] ={
	{250,90,100,45},
	{250,150,100,45},
	{30,240,650,228},    //防区区域
	{0,0,0,0},
	{0,0,0,0},
	{0,0,0,0},
	{0,0,0,0},
	{0,0,0,0},
	{0,0,0,0,},
	{0,0,0,0}, 
	
 	{700,100,300,400},            //数字符号所在区域
        {900,531,111,69},              //返回
        {6,531,111,69},                 //关屏幕	
         {780,531,111,69},              //确认
	
};
static unsigned char g_input_scene_number[3]={'\0'};
static int g_set_scene_mode = -1;
static int g_relation = 0;
static int g_safestate[16];

void CloseSceneWindow()
{
	
        g_cur_input_count2 = 0;
        g_set_scene_mode = -1;
        g_relation  =0;
        memset(g_safestate,0,sizeof(g_safestate));

        memset(g_input_scene_number,0,3);
        
}

void chefang_jiaju()
{
	int i;
	for(i =2;i <16;i++){
		g_SCENE_MODE_TABLE[g_cur_set_scene_mode].safestatus[i]=0;
	}
}

static void save_scene_data()
{
	int i;
	pSystemInfo->mySysInfo.scenenumber[g_set_scene_mode] = atoi((char*)g_input_scene_number);
	g_SCENE_MODE_TABLE[g_set_scene_mode].relation = g_relation;
	for(i =0;i <16;i++){
	    g_SCENE_MODE_TABLE[g_set_scene_mode].safestatus[i]=g_safestate[i];
	}
	g_SCENE_MODE_TABLE[g_set_scene_mode].scene = pSystemInfo->mySysInfo.scenenumber[g_set_scene_mode];      
	UpdateSystemInfo();
	save_scene_mode_table_to_flash();
}

void CreateSceneSetWin2(int scene_mode){
	int i;
    Window* pWindow = (Window*)New_Window();
    if(pWindow == NULL)
            return ;
    pWindow->EventHandler = SceneSetEventHandler2;
    pWindow->NotifyReDraw = DrawSceneSetMain2;
    pWindow->valid = 1;
    pWindow->CloseWindow = CloseSceneWindow;
    pWindow->type = CHILD;
    WindowAddToWindowList(pWindow);
    g_cur_input_count2 = 0;
    memset(g_input_scene_number,0,3);
    memset(g_safestate,0,sizeof(g_safestate));
    g_set_scene_mode = scene_mode;
    g_relation = g_SCENE_MODE_TABLE[g_set_scene_mode].relation;
    for(i =0;i <8;i++){
    	    g_safestate[i] = g_SCENE_MODE_TABLE[g_set_scene_mode].safestatus[i];
    }

    sprintf((char*)g_input_scene_number,"%d", g_SCENE_MODE_TABLE[g_set_scene_mode].scene);
    GetNumberJpgPointer(&pLampNumberPointer,&pLampNumberPointer1);
    GetNumberJpegSizePointer(&pLampNumberSize,&pLampNumberSize1);        
    pWindow->NotifyReDraw2(pWindow);
}
void Drawrelationzone()
{
	 int startx = rtScenerc[2].x;
        int starty = rtScenerc[2].y;
        int tempx  = 0;
        int tempy = 0;
        int count = 0;
        int i,j;
        char tmptxt[10];
        for(i=0;i<1;i++){
                for(j=0; j<8;j++){
                        count = i*8+j;
                        tempx = startx+j*80;
                        tempy = starty+i*106;
                        if(pSystemInfo->mySysInfo.safestatus[16+count] == 0){
                                JpegDisp(tempx,tempy,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
                        }else{
                                if(g_safestate[count] == 1)
                                        JpegDisp(tempx,tempy,ico_fangqukai,sizeof(ico_fangqukai));
                                else
                                        JpegDisp(tempx,tempy,ico_fangquguan,sizeof(ico_fangquguan));
                        }
                        sprintf(tmptxt,"%s%d",LT("防区"),count+1);
                        DrawText_16(tmptxt,tempx,tempy+70,rgb_white);

                }
        }
}
void DrawSceneSetMain2()
{
	char* name;
	char number[10]={0};
	WRT_Rect tmprect;
	WRT_Rect rt;
	int x;
        int i =0;
        int j =0;
        int startx = rtScenerc[10].x;
        int starty = rtScenerc[10].y;
        
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        for(i=0; i<4;i++)
        {
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pLampNumberPointer[(i*3)+j],pLampNumberSize[(i*3)+j]);
                }
        }

        
        
        JpegDisp(rtScenerc[11].x,rtScenerc[11].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rtScenerc[12].x,rtScenerc[12].y,bt_guanping1,sizeof(bt_guanping1));
        JpegDisp(rtScenerc[13].x,rtScenerc[13].y,menu_ok1,sizeof(menu_ok1));
        name = (char*)ZENMALLOC(50);
        if(name == NULL)
        	return;
        memset(name,0,50);
 
        switch(g_set_scene_mode)
        {
       	case	SCENE_HUIKE:
        	sprintf(name,"'%s' %s :",LT("会客"),LT("情景编号"));
        	break;
        case 	SCENE_JIUCAN:
        	sprintf(name,"'%s' %s :",LT("就餐"),LT("情景编号"));
        	break;
        case  	SCENE_YINGYUAN:
        	sprintf(name,"'%s' %s :",LT("影院"),LT("情景编号"));
        	break;
        case 	SCENE_JIUQIN:
        	sprintf(name,"'%s' %s :",LT("就寝"),LT("情景编号"));
        	break;
        case 	SCENE_WENXIN:
        	sprintf(name,"'%s' %s :",LT("温馨"),LT("情景编号"));
        	break;
        case 	SCENE_ZAIJIA:
        	sprintf(name,"'%s' %s :",LT("在家"),LT("情景编号"));
        	break;
        case 	SCENE_LIJIA:
        	sprintf(name,"'%s' %s :",LT("离家"),LT("情景编号"));
        	break;
        case 	SCENE_YEQI:
        	sprintf(name,"'%s' %s :",LT("夜起"),LT("情景编号"));
        	break;
        case 	SCENE_CHENQI:
        	sprintf(name,"'%s' %s :",LT("晨起"),LT("情景编号"));
        	break;
        case 	SCENE_USERDEFINE:
        	sprintf(name,"'%s' %s :",LT("自定义"),LT("情景编号"));
        	break;
        default:
        	memset(name,0,50);
        	break;
        }

        x = 10;
		DrawText(name,x,rtScenerc[0].y + 30,rgb_white);
        DrawRect(rtScenerc[0],2,rgb_white);
        tmprect = rtScenerc[0];
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
        DrawTextToRect2(tmprect,(char*)g_input_scene_number,rgb_black,1);
        ZENFREE(name);
      
        //显示是否关联防区。
        DrawText(LT("关联防区:"),10 ,rtScenerc[1].y+30,rgb_white);
        if(g_relation == 1){
        	JpegDisp(rtScenerc[1].x,rtScenerc[1].y,ico_fangqukai,sizeof(ico_fangqukai));
        	Drawrelationzone();
        }else{
        	JpegDisp(rtScenerc[1].x,rtScenerc[1].y,ico_fangquguan,sizeof(ico_fangquguan));
		FillRect(0xb2136,rtScenerc[2]);
        }
 
    
        	
}
static int setnumberhandler4(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rtScenerc[10].x;
        int starty = rtScenerc[10].y;
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
                                        g_rc_status[count+11] = true;
                                else{
                                        if(g_rc_status[count+11] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count+11] = false;
                                }
                                
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer1[count],pLampNumberSize1[count]);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pLampNumberPointer[count],pLampNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_cur_input_count2 <(2)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                if(strlen((char*)g_input_scene_number) == 0)
                                                	if(count == 10)
                                                		return 0;
                                                if(strlen((char*)g_input_scene_number) < 3){
                                                	g_cur_input_count2++;
                                                        strcat((char*)g_input_scene_number,(char*)&c);
													WRT_Rect  rc1 = rtScenerc[0];
                                                	DeflateRect(&rc1,4);
                                                	g_isUpdated = 0;
                                                	
                                                	FillRect(rgb_white,rc1);
                                                	DrawRect(rtScenerc[0],2,rgb_yellow);                                                        
                                                        DrawTextToRect2(rtScenerc[0],(char*)g_input_scene_number,rgb_black,1);
                                                        update_rect(rc1.x,rc1.y,rc1.w,rc1.h);
                                                        g_isUpdated = 1;
                                                        //g_cur_input_count2 = strlen((char*)g_input_scene_number);
                                                }

                                        }else if(count == 9){

                                                g_cur_input_count2--;
                                                if(g_cur_input_count2 < 0)
                                                        g_cur_input_count2 = 0;
                                                g_input_scene_number[g_cur_input_count2]='\0';

                                                WRT_Rect  rc1 = rtScenerc[0];
                                                DeflateRect(&rc1,4);
                                                g_isUpdated = 0;
                                                FillRect(rgb_white,rc1);
                                                DrawRect(rtScenerc[0],2,rgb_yellow);
                                                DrawTextToRect2(rtScenerc[0],(char*)g_input_scene_number,rgb_black,1);
                                                update_rect(rc1.x,rc1.y,rc1.w,rc1.h);
                                                g_isUpdated = 1;
                                                g_cur_input_count2 = strlen((char*)g_input_scene_number);
                                        }                                       
                                }//else status == 0
                                break;
                        }//if(x > tempx && x < (tempx+76) && y > tempy  && y < (tempy+72)
                }//for
                return ret;
}

static int relationsafehandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rtScenerc[2].x;
        int starty = rtScenerc[2].y;
        for(i=0; i<1;i++)
                for(j=0; j< 8;j++){
                        tempx = startx+(j)*80;
                        tempy = starty+(i)*106;
                        if(x > tempx && x < (tempx+48) && y > tempy  && y < (tempy+48)){
                                count = (i*8)+j;
                                ret =1;
                                /*
                                if(status == 1)
                                        g_rc_status[count] = true;
                                else{
                                        if(g_rc_status[count] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count] = false;
                                }
                                */
                                if(pSystemInfo->mySysInfo.safestatus[count+16] == 0) //如果防区没有启动，则不需要处理
                                        break;
                                if(status == 1){
                                        StartButtonVocie();
                                }else{
                                		if(count == 0 || count == 1)
                                			break;
                                        if(g_safestate[count] == 1 ){
                                                JpegDisp(tempx,tempy, ico_fangquguan,sizeof(ico_fangquguan));
                                                g_safestate[count] = 0;
                                        }else{
                                                JpegDisp(tempx,tempy, ico_fangqukai,sizeof(ico_fangqukai));
                                                g_safestate[count] = 1;
                                        }
                                }
                                break;
                        }
                }
                return ret;
}
int SceneSetEventHandler2(int x,int y,int status)
{
       int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        
        for(index =0 ;index<14;index++){
                if(IsInside(rtScenerc[index],xy)){
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
                        pos_x = rtScenerc[index].x;
                        pos_y = rtScenerc[index].y;
                        
                        switch(index){
                        case 10://数字区域
                                
                        	ret = setnumberhandler4(x,y,status);
                        	break;
                        case 11:
                        	if(status == 1){
                        	        StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                                }else{
                                	JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                	CloseTopWindow();
                                }
                        	break;
                        case 12:
                        	if(status == 1){
                        	         StartButtonVocie();
                        		JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        	}else{	
                        		JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                        		screen_close();
                        	}
                        	break;
                        case 13:
                        	if(status == 1){
                        	         StartButtonVocie();
                        		JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        	}else{	
                        		JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                        		CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                        		SDL_Delay(1000);
                        		save_scene_data();
                        		CloseTopWindow();
                        	}
                        	break;
                        case 2: //防区选择。
                        	if(g_relation == 0)
                        		break;
                        	
                        	ret = relationsafehandler(x,y,status);
                        	break;
                        case 1: //关联防区
                        	if(status == 1){
                        	         StartButtonVocie();
                        	}else{
                        	    	if(g_relation == 1){
        					JpegDisp(rtScenerc[1].x,rtScenerc[1].y,ico_fangquguan,sizeof(ico_fangquguan));
        					FillRect(0xb2136,rtScenerc[2]);
        					g_relation = 0;
       			 		}else{
        					JpegDisp(rtScenerc[1].x,rtScenerc[1].y,ico_fangqukai,sizeof(ico_fangqukai));
        					g_relation = 1;
        					g_isUpdated = 0;
        					g_safestate[0] = 1;
        					g_safestate[1] = 1;
						Drawrelationzone();
						g_isUpdated = 1;
						update_rect(rtScenerc[2].x,rtScenerc[2].y,rtScenerc[2].w,rtScenerc[2].h);
						
        				}
                        	}
                        	break;
                        default:
                        	if(status == 1){
                        		g_cur_input_count2 = strlen((char*)g_input_scene_number);
                        		DrawRect(rtScenerc[0],2,rgb_white);
                        		DrawRect(rtScenerc[0],2,rgb_yellow);
                        	}
                        	break;
                        }
                }
        }
        return ret;	
}
