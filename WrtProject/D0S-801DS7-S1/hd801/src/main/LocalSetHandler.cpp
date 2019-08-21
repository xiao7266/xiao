#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "localset_menu_res.h"

#include "window.h"
#include "c_interface.h"
#include "wrt_audio.h"



//extern int    g_localid; //cmdhandler.cpp
//extern bool g_rc_status[42];
//extern bool  g_have_faze;
extern unsigned char projectconfig1[4040];
extern unsigned char projectconfig2[4166];

#if HAVE_SCREEN_KEEP
//extern void screenkeep_init();
//extern int gHaveScreenkeep;
#endif

static const WRT_Rect rc_localsetmenuico[10]={
		{108,83, 132,160},	  //密码
		{326,83, 132,160},	 //安防
		{555,83,132,160},  //铃声
		{779,83,132,160},	//家居
        {108,305,132,160},   //免打扰
        {326,305,132,160},  //延时设置
        {555,305,132,160},  //网络设置
        {779,305,132,160},  //恢复出厂设置

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕	
};


void CreateLocalSetWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = LockSetEventHandler;
        pWindow->NotifyReDraw = DrawLocalSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);
}
static void DrawTemp()
{
        WRT_Rect rt;
        int xoffset = 0;
        rt.x = rc_localsetmenuico[1].x;
        rt.y = rc_localsetmenuico[1].y+rc_localsetmenuico[1].h - 24;
        rt.w = rc_localsetmenuico[1].w;
        rt.h = 48;

        if(pSystemInfo->mySysInfo.isagent == 1)
                DrawEnText(LT("取消托管"),rc_localsetmenuico[1].w+xoffset,rc_localsetmenuico[1].h,rc_localsetmenuico[1].x,rc_localsetmenuico[1].y);
        else
                DrawEnText(LT("托管"),rc_localsetmenuico[1].w+xoffset,rc_localsetmenuico[1].h,rc_localsetmenuico[1].x,rc_localsetmenuico[1].y);

}

static void DrawTemp1()
{
	WRT_Rect rt;
	int xoffset = 0;
	rt.x = rc_localsetmenuico[4].x;
	rt.y = rc_localsetmenuico[4].y+rc_localsetmenuico[4].h - 24;
	rt.w = rc_localsetmenuico[4].w;
	rt.h = 48;

	if(pSystemInfo->mySysInfo.isavoid == 1)
		DrawEnText(LT("取消免打扰"),rc_localsetmenuico[4].w+xoffset,rc_localsetmenuico[4].h,rc_localsetmenuico[4].x,rc_localsetmenuico[4].y);
	else
		DrawEnText(LT("免打扰"),rc_localsetmenuico[4].w+xoffset,rc_localsetmenuico[4].h,rc_localsetmenuico[4].x,rc_localsetmenuico[4].y);
}

void DrawLocalSetMain(){
        int xoffset = 0;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
 
        JpegDisp(rc_localsetmenuico[0].x,rc_localsetmenuico[0].y,menu_fujiagongneng1,sizeof(menu_fujiagongneng1));

        if(pSystemInfo->mySysInfo.isagent == 1)
                JpegDisp(rc_localsetmenuico[1].x,rc_localsetmenuico[1].y,menu_quxiaotuoguan1,sizeof(menu_quxiaotuoguan1));
        else
                JpegDisp(rc_localsetmenuico[1].x,rc_localsetmenuico[1].y,menu_tuoguan1,sizeof(menu_tuoguan1));

        JpegDisp(rc_localsetmenuico[2].x,rc_localsetmenuico[2].y,menu_colormanage1,sizeof(menu_colormanage1));  

        JpegDisp(rc_localsetmenuico[3].x,rc_localsetmenuico[3].y,menu_yinliangset1,sizeof(menu_yinliangset1));
        if(pSystemInfo->mySysInfo.isavoid == 1)
                JpegDisp(rc_localsetmenuico[4].x,rc_localsetmenuico[4].y,menu_quxiaomiandarao1,sizeof(menu_quxiaomiandarao1));
        else
                JpegDisp(rc_localsetmenuico[4].x,rc_localsetmenuico[4].y,menu_miandarao1,sizeof(menu_miandarao1));
        JpegDisp(rc_localsetmenuico[5].x,rc_localsetmenuico[5].y,menu_xianglingshijian1,sizeof(menu_xianglingshijian1)); 
        JpegDisp(rc_localsetmenuico[6].x,rc_localsetmenuico[6].y,projectconfig1,sizeof(projectconfig1));   
        JpegDisp(rc_localsetmenuico[7].x,rc_localsetmenuico[7].y,menu_chuchangshezhi1,sizeof(menu_chuchangshezhi1));
        JpegDisp(rc_localsetmenuico[8].x,rc_localsetmenuico[8].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_localsetmenuico[9].x,rc_localsetmenuico[9].y,bt_guanping1,sizeof(bt_guanping1)); 

        DrawEnText(LT("附加功能"),rc_localsetmenuico[0].w+xoffset,rc_localsetmenuico[0].h,rc_localsetmenuico[0].x,rc_localsetmenuico[0].y);
        DrawEnText(LT("色彩管理"),rc_localsetmenuico[2].w+xoffset,rc_localsetmenuico[2].h,rc_localsetmenuico[2].x,rc_localsetmenuico[2].y);
        DrawEnText(LT("音量调节"),rc_localsetmenuico[3].w+xoffset,rc_localsetmenuico[3].h,rc_localsetmenuico[3].x,rc_localsetmenuico[3].y);
        DrawEnText(LT("延时设置"),rc_localsetmenuico[5].w+xoffset,rc_localsetmenuico[5].h,rc_localsetmenuico[5].x,rc_localsetmenuico[5].y);
        DrawEnText(LT("工程配置"),rc_localsetmenuico[6].w+xoffset,rc_localsetmenuico[6].h,rc_localsetmenuico[6].x,rc_localsetmenuico[6].y);
        DrawEnText(LT("恢复出厂设置"),rc_localsetmenuico[7].w+21,rc_localsetmenuico[7].h,rc_localsetmenuico[7].x,rc_localsetmenuico[7].y);

        DrawTemp();
        DrawTemp1();

}
int LockSetEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<10;index++){
                if(IsInside(rc_localsetmenuico[index],xy)){
                        pos_x = rc_localsetmenuico[index].x;
                        pos_y = rc_localsetmenuico[index].y;
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
                  case 0://附加功能
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_fujiagongneng2,sizeof(menu_fujiagongneng2));
                                                          
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_fujiagongneng1,sizeof(menu_fujiagongneng1));
                                  CreateExtraFunction();
                          }
                          break;
                  case 1://托管
                          if(status == 1){
                                  StartButtonVocie();
                                  if(pSystemInfo->mySysInfo.isagent == 1)
                                          JpegDisp(pos_x,pos_y,menu_quxiaotuoguan2,sizeof(menu_quxiaotuoguan2));
                                  else
                                          JpegDisp(pos_x,pos_y,menu_tuoguan2,sizeof(menu_tuoguan2));
                          }else{
                                  if(pSystemInfo->mySysInfo.isagent == 1)
                                          JpegDisp(pos_x,pos_y,menu_quxiaotuoguan1,sizeof(menu_quxiaotuoguan1));    
                                  else
                                          JpegDisp(pos_x,pos_y,menu_tuoguan1,sizeof(menu_tuoguan1));
                                  // CreateTypeSet(CONTRAST_SET_WIN); 
                                  if(pSystemInfo->mySysInfo.isagent == true) //如果已经设置托管
                                          CreateDialogWin2(LT("已经设置托管,是否取消托管设置?"),MB_OK|MB_CANCEL,CancelAgent,NULL); 
                                  else{                 
                                          CreateAvoidHarryWin(1);
                                          if(pSystemInfo->mySysInfo.isavoid == true){
                                                  CreateDialogWin2(LT("您已经设置了免打扰,设置托管时,将强制取消免打扰!"),MB_OK,NULL,NULL);
                                          }                		      
                                  }
                          }            	  
                          break;      
                  case 2://颜色管理 
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_colormanage2,sizeof(menu_colormanage2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_colormanage1,sizeof(menu_colormanage1));
                                  CreateColorManager();
                                  break;
                          }                 	 	
                          break;
                  case 3://音量  
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_yinliangset2,sizeof(menu_yinliangset2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_yinliangset1,sizeof(menu_yinliangset1));
                                  CreateTypeSet(VOLUME_SET_WIN); 
                                  // CreateVoiceControl();
                          }                               	 	
                          break;
                  case 4://免打扰
                          if(status == 1){
                                  StartButtonVocie();
                                  if(pSystemInfo->mySysInfo.isavoid == 1)
                                          JpegDisp(pos_x,pos_y,menu_quxiaomiandarao2,sizeof(menu_quxiaomiandarao2));
                                  else
                                          JpegDisp(pos_x,pos_y,menu_miandarao2,sizeof(menu_miandarao2));
                          }else{
                                  if(pSystemInfo->mySysInfo.isavoid == 1)
                                          JpegDisp(pos_x,pos_y,menu_quxiaomiandarao1,sizeof(menu_quxiaomiandarao1));
                                  else
                                          JpegDisp(pos_x,pos_y,menu_miandarao1,sizeof(menu_miandarao1));
                                  //if(g_mysysinfo->isavoid == false){
                                  //    printf("zzzzzzzzzzzzz\n");
                                  //    CreateDialogWin("是否重新设置免打扰时间?否,则采用默认的免打扰时间为12小时!",SET_CANCEL_AVOID); 
                                  //  }

                                  if(pSystemInfo->mySysInfo.isavoid == true) //如果已经设置免打扰
                                          CreateDialogWin2(LT("已经设置免打扰,是否取消免打扰设置?"),MB_OK|MB_CANCEL,CancelAvoid,NULL); 
                                  else{
                                          CreateAvoidHarryWin(0);
                                          if(pSystemInfo->mySysInfo.isagent == true){
                                                  CreateDialogWin2(LT("您已经设置了托管,设置免打扰时,将强制取消托管!"),MB_OK,NULL,NULL);
                                          }
                                  }    


                          }                   	 	
                          break;
                  case 5://延时设置
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_xianglingshijian2,sizeof(menu_xianglingshijian2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_xianglingshijian1,sizeof(menu_xianglingshijian1));  
                                  CreateRingTimeSetWin();                
                          }                   	 	
                          break;
                  case 6://网络设置
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,projectconfig2,sizeof(projectconfig2));
                          }else{
                                  JpegDisp(pos_x,pos_y,projectconfig1,sizeof(projectconfig1));  
                                  // CreateRingTimeSetWin();   
                                  CreatePasswordWin(SET_NET_CONFIG_WIN);             
                          }             	      
                          break;
                  case 7://恢复出厂设置
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_chuchangshezhi2,sizeof(menu_chuchangshezhi2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_chuchangshezhi1,sizeof(menu_chuchangshezhi1));  
                                  //CreateRingTimeSetWin(); 
                                  CreatePasswordWin(SET_DEFAULT_CONFIG_PWD_WIN);
                          }             	    
                          break;
                  case 8: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                  CloseTopWindow(); 

                          }
                          break;
                  case 9:/*关屏*/
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
//////////////////////////////////////////////////////////////////////////////
//
#define MAX_TIME_BITS 4

static unsigned long* pTimeNumberPointer=NULL;
static unsigned long* pTimeNumberPointer1=NULL;
static unsigned long* pTimeNumberSize=NULL;
static unsigned long* pTimeNumberSize1=NULL;

static char  g_timechar[MAX_TIME_BITS];
static int   g_timeinputcount = 0;
extern char  g_number[12];
static int   g_curselecttime = 0;
static int   g_preselecttime = 0;
//static int   g_curringtime = 30;



static const WRT_Rect rc_ringtimemenuico[7]={
        {100,80,292,50},        //输入框所在区域
        {100,200,292,50},        //输入框所在区域	
        {100,320,292,50},        //输入框所在区域  
        {100,440,292,50},

        {650,100,300,400},    //数字符号所在区域
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕				
};


static int ringtimehandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_ringtimemenuico[4].x;
        int starty = rc_ringtimemenuico[4].y; 
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pTimeNumberPointer1[count],pTimeNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pTimeNumberPointer[count],pTimeNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_timeinputcount <(MAX_TIME_BITS-1)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                strcat(g_timechar,(char*)&c);
                                                g_timeinputcount++;
                                                DrawText(g_timechar,rc_ringtimemenuico[g_curselecttime].x+20,rc_ringtimemenuico[g_curselecttime].y+32,rgb_black);
                                        }else if(count  == 9){ //*
                                                WRT_Rect tmprect;
                                                g_timechar[g_timeinputcount-1] = '\0';
                                                tmprect = rc_ringtimemenuico[g_curselecttime];
                                                DeflateRect(&tmprect,4);
                                                g_isUpdated = 0;
                                                FillSingleRect(rgb_white,tmprect);
                                                //JpegDisp(rc_ringtimemenuico[g_curselecttime].x,rc_ringtimemenuico[g_curselecttime].y,mima_input2,sizeof(mima_input2));
                                                g_timeinputcount--;
                                                if(g_timeinputcount < 0)
                                                        g_timeinputcount = 0;
                                                DrawText(g_timechar,rc_ringtimemenuico[g_curselecttime].x+20,rc_ringtimemenuico[g_curselecttime].y+32,rgb_black);
                                                WRT_DEBUG("ddddddddd %s",g_timechar);
                                                update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                                g_isUpdated = 1;
                                                
                                        }else if(count == 11){//# //确认

                                                int times = atoi(g_timechar);//转换字符串为数字以做判断
                                                if(g_curselecttime == 0){
                                                        if(times > 60 || times < 35){
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                                CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);	
                                                        }else{
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                pSystemInfo->mySysInfo.ringtimes = times;
                                                                if(pSystemInfo->mySysInfo.ringtimes < 35)
                                                                        pSystemInfo->mySysInfo.ringtimes = 35;
                                                                if(pSystemInfo->mySysInfo.ringtimes > 60)
                                                                        pSystemInfo->mySysInfo.ringtimes = 60;
                                                                SetRingtimes(pSystemInfo->mySysInfo.ringtimes);
                                                                //ReDrawCurrentWindow();
                                                                save_localhost_config_to_flash();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("响铃时间设置完毕!"),MB_OK,NULL,NULL);
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                        }
                                                }else if(g_curselecttime == 1){
                                                        if(times < 1 || times > 255){
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                                CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);	
                                                        }else{
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                pSystemInfo->mySysInfo.alarmtime = times;
                                                                //ReDrawCurrentWindow();
                                                                save_localhost_config_to_flash();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("布防延时时间设置完毕!"),MB_OK,NULL,NULL);
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                        }	                      	
                                                }else if(g_curselecttime == 2){
                                                        if(times < 1 || times > 255){
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                                CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);	

                                                        }else{
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                pSystemInfo->mySysInfo.alarmtime1  = times;
                                                                save_localhost_config_to_flash();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("上报延时时间设置完毕!"),MB_OK,NULL,NULL);
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                        }	                      	
                                                }else{
                                                        if(times < 30 || times > 300){
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                                CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);	
                                                        }else{
                                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                                pSystemInfo->mySysInfo.alarmvoicetime  = times;
                                                                save_localhost_config_to_flash();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("报警声持续时间设置完毕!"),MB_OK,NULL,NULL);
                                                                memset(g_timechar,0,MAX_TIME_BITS);
                                                        }	
                                                }
                                        }
                                }
                                break;
                        }
                }
                return ret;
}

static void updateinputtime(int index){
        int i =0; 
        char tmptext[30];
        WRT_Rect tmprect;
        g_isUpdated = 0;
        tmprect = rc_ringtimemenuico[g_preselecttime];
        DrawRect(tmprect,2,rgb_white);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
        
        //JpegDisp(rc_ringtimemenuico[g_preselecttime].x,rc_ringtimemenuico[g_preselecttime].y,mima_input,sizeof(mima_input));
        tmprect = rc_ringtimemenuico[g_curselecttime];
        DrawRect(tmprect,2,rgb_yellow);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
        //JpegDisp(rc_ringtimemenuico[g_curselecttime].x,rc_ringtimemenuico[g_curselecttime].y,mima_input2,sizeof(mima_input2));
        if(g_preselecttime == 0){
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.ringtimes);
        }else if(g_preselecttime == 1){
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmtime);
        }else if(g_preselecttime == 2){
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmtime1);
        }else
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmvoicetime);

        DrawText(tmptext,rc_ringtimemenuico[g_preselecttime].x+20,rc_ringtimemenuico[g_preselecttime].y+32,rgb_black);


        if(g_curselecttime == 0){
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.ringtimes);
        }else if(g_curselecttime == 1){
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmtime);
        }else if(g_curselecttime ==2){
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmtime1);
        }else
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmvoicetime);

        DrawText(tmptext,rc_ringtimemenuico[g_curselecttime].x+20,rc_ringtimemenuico[g_curselecttime].y+32,rgb_black);  
       
        memset(g_timechar,0,MAX_TIME_BITS);
        if(index == 0){
                sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.ringtimes);
        }else if(index == 1){
                sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.alarmtime);
        }else if(index == 2){
                sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.alarmtime1);
        }else if(index ==3){
                sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.alarmvoicetime);
        }
        g_timeinputcount = strlen(g_timechar);
        update_rect(rc_ringtimemenuico[g_preselecttime].x,rc_ringtimemenuico[g_preselecttime].y,rc_ringtimemenuico[g_preselecttime].w,rc_ringtimemenuico[g_preselecttime].h);
        update_rect(rc_ringtimemenuico[g_curselecttime].x,rc_ringtimemenuico[g_curselecttime].y,rc_ringtimemenuico[g_curselecttime].w,rc_ringtimemenuico[g_curselecttime].h);
        g_isUpdated = 1;
}

void CreateRingTimeSetWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = RingTimeSetEventHandler;
        pWindow->NotifyReDraw = RingTimeSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        GetNumberJpgPointer(&pTimeNumberPointer,&pTimeNumberPointer1);
        GetNumberJpegSizePointer(&pTimeNumberSize,&pTimeNumberSize1);
        pWindow->NotifyReDraw2(pWindow);
}

void RingTimeSetMain(){
        WRT_Rect tmprect;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillSingleRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));   
        JpegDisp(rc_ringtimemenuico[5].x,rc_ringtimemenuico[5].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_ringtimemenuico[6].x,rc_ringtimemenuico[6].y,bt_guanping1,sizeof(bt_guanping1)); 
        int i =0; 
        int j =0;
        int startx = rc_ringtimemenuico[4].x;
        int starty = rc_ringtimemenuico[4].y;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pTimeNumberPointer[(i*3)+j],pTimeNumberSize[(i*3)+j]);
                }
                tmprect = rc_ringtimemenuico[0];
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                //JpegDisp(rc_ringtimemenuico[0].x,rc_ringtimemenuico[0].y,mima_input,sizeof(mima_input));
                DrawText(LT("请输入响铃持续时间(35-60)秒:"),30,rc_ringtimemenuico[0].y-10,rgb_white);

                tmprect = rc_ringtimemenuico[1];
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
               // JpegDisp(rc_ringtimemenuico[1].x,rc_ringtimemenuico[1].y,mima_input,sizeof(mima_input));
                DrawText(LT("请输入布防延时时间(1-255)秒:"),30,rc_ringtimemenuico[1].y-10,rgb_white);
                
                tmprect = rc_ringtimemenuico[2];
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                //JpegDisp(rc_ringtimemenuico[2].x,rc_ringtimemenuico[2].y,mima_input,sizeof(mima_input));
                DrawText(LT("请输入上报延时时间(1-255)秒:"),30,rc_ringtimemenuico[2].y-10,rgb_white);

                tmprect = rc_ringtimemenuico[3];
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                //JpegDisp(rc_ringtimemenuico[3].x,rc_ringtimemenuico[3].y,mima_input,sizeof(mima_input));
                DrawText(LT("请输入报警声持续时间(30-300)秒:"),30,rc_ringtimemenuico[3].y-10,rgb_white);	 
                tmprect = rc_ringtimemenuico[g_curselecttime];
                DrawRect(tmprect,2,rgb_yellow);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                //JpegDisp(rc_ringtimemenuico[g_curselecttime].x,rc_ringtimemenuico[g_curselecttime].y,mima_input2,sizeof(mima_input2));	  	  

                char tmptext[30];
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.ringtimes);
                //g_111 = pSystemInfo->mySysInfo.ringtimes - 35;
                DrawText(tmptext,rc_ringtimemenuico[0].x+20,rc_ringtimemenuico[0].y+32,rgb_black);

                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmtime);
                DrawText(tmptext,rc_ringtimemenuico[1].x+20,rc_ringtimemenuico[1].y+32,rgb_black);
				//g_222 = pSystemInfo->mySysInfo.alarmtime-1;
                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmtime1);
                //g_333 = pSystemInfo->mySysInfo.alarmtime1;
                DrawText(tmptext,rc_ringtimemenuico[2].x+20,rc_ringtimemenuico[2].y+32,rgb_black);	 

                sprintf(tmptext,"%d",pSystemInfo->mySysInfo.alarmvoicetime);
                DrawText(tmptext,rc_ringtimemenuico[3].x+20,rc_ringtimemenuico[3].y+32,rgb_black);	
                // sprintf(tmptext,"当前铃声持续时间为:%d秒",pSystemInfo->mySysInfo.ringtimes);
                // DrawText(tmptext,rc_ringtimemenuico[0].x,rc_ringtimemenuico[0].y+rc_ringtimemenuico[0].h+32,rgb_yellow);

                WRT_Rect curbox;
                curbox.x = 135;
                curbox.y = 531;
                curbox.w = 578;
                curbox.h = 55;
		        JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
	        
                DrawText_16(LT("提示:按'*'退格，按'#'确认"),curbox.x,curbox.y+45,rgb_black);	
                memset(g_timechar,0,MAX_TIME_BITS);
                if(g_curselecttime == 0){
                        sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.ringtimes);
                }else if(g_curselecttime == 1){
                        sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.alarmtime);
                }else if(g_curselecttime == 2){
                        sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.alarmtime1);
                }else if(g_curselecttime == 3){
                        sprintf(g_timechar,"%d",pSystemInfo->mySysInfo.alarmvoicetime);
                }
                g_timeinputcount = strlen(g_timechar);

                //g_curringtime = GetRingtimes();
}
int RingTimeSetEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<7;index++){
                if(IsInside(rc_ringtimemenuico[index],xy)){
                        pos_x = rc_ringtimemenuico[index].x;
                        pos_y = rc_ringtimemenuico[index].y;
                        ret = 1;         
                        switch(index){     
                  case 0: 
                          g_preselecttime = g_curselecttime;      
                          g_curselecttime = 0;              	  	  
                          updateinputtime(0);
                          break; 
                  case 1:
                          g_preselecttime = g_curselecttime;   
                          g_curselecttime = 1;
                          updateinputtime(1);
                          break;
                  case 2:
                          g_preselecttime = g_curselecttime;   
                          g_curselecttime = 2;     	  	  
                          updateinputtime(2);
                          break;  
                  case 3:
                          g_preselecttime = g_curselecttime;   
                          g_curselecttime = 3;     	  	  
                          updateinputtime(3);
                          break;
                  case 5: /*返回*/
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
                  case 4:
                          //进入密码区域
                          ret = ringtimehandler(x,y,status);
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }      
        return ret;  	
}

//////////////////////////////////////////////////////////////////////
//

static const WRT_Rect rc_harrytimemenuico[5]={
        {75,180,200,40},        //输入框所在区域
        {75,254,200,40},       //输入框2所在区域
        {650,100,300,400},    //数字符号所在区域
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕				
};

static char g_Hourchar[3]={'\0','\0','\0'};
static char g_Minchar[3]={'\0','\0','\0'};
static char g_HourMinInputcount=0;
static char g_HourMinInputStatus =1;
static unsigned long  g_hour=0;
static unsigned long  g_min=0;
static int agent_type = 0; //默认创建的是免打扰window

static void DrawSelectEdit(){
        if(g_HourMinInputStatus == 1){ //选择小时输入框
                WRT_Rect tmprect;
                tmprect = rc_harrytimemenuico[0];
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                DrawText(g_Hourchar,rc_harrytimemenuico[0].x+10,rc_harrytimemenuico[0].y+26,rgb_black);
                DrawRect(rc_harrytimemenuico[0],2,rgb_yellow);
                DrawRect(rc_harrytimemenuico[1],2,rgb_white);
                
        }else if(g_HourMinInputStatus == 2){
                WRT_Rect tmprect;
                tmprect = rc_harrytimemenuico[1];
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);                
                DrawText(g_Minchar,rc_harrytimemenuico[1].x+10,rc_harrytimemenuico[1].y+26,rgb_black);
                DrawRect(rc_harrytimemenuico[1],2,rgb_yellow);   
                DrawRect(rc_harrytimemenuico[0],2,rgb_white);

        }
}

static int avoidharryhandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_harrytimemenuico[2].x;
        int starty = rc_harrytimemenuico[2].y; 
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pTimeNumberPointer1[count],pTimeNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pTimeNumberPointer[count],pTimeNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_HourMinInputcount <(3-1)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                if(g_HourMinInputStatus == 1){
                                                        strcat(g_Hourchar,(char*)&c);
                                                        DrawText(g_Hourchar,rc_harrytimemenuico[0].x+10,rc_harrytimemenuico[0].y+26,rgb_black);
                                                }else if (g_HourMinInputStatus == 2){
                                                        strcat(g_Minchar,(char*)&c);
                                                        DrawText(g_Minchar,rc_harrytimemenuico[1].x+10,rc_harrytimemenuico[1].y+26,rgb_black);	    	      	    	
                                                }
                                                int times1 = atoi(g_Hourchar);
                                                int times2 = atoi(g_Minchar);
                                                if(times1 > 99){
                                                        CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);
                                                        break;
                                                }
                                                if(times2 > 59){
                                                        CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);
                                                        break;
                                                }
                                                g_HourMinInputcount++;	    	      	    
                                        }else if(count  == 9){ //*	    	      		 
                                                g_HourMinInputcount--;
                                                if(g_HourMinInputcount < 0)
                                                        g_HourMinInputcount = 0;
                                                if(g_HourMinInputStatus == 1){
                                                        g_Hourchar[g_HourMinInputcount] = '\0';
                                                        WRT_Rect rt = rc_harrytimemenuico[0];
                                                        DeflateRect(&rt,4);
                                                        g_isUpdated = 0;
                                                        FillRect(rgb_white,rt);
                                                        DrawText(g_Hourchar,rc_harrytimemenuico[0].x+10,rc_harrytimemenuico[0].y+26,rgb_black);
                                                        DrawRect(rc_harrytimemenuico[0],2,rgb_yellow);
                                                        update_rect(0,0,500,400);
                                                        g_isUpdated = 1;
                                                }else{
                                                        g_Minchar[g_HourMinInputcount] = '\0';
                                                        WRT_Rect rt = rc_harrytimemenuico[1];
                                                        DeflateRect(&rt,4);
                                                        g_isUpdated = 0;
                                                        FillRect(rgb_white,rt);
                                                        DrawText(g_Minchar,rc_harrytimemenuico[1].x+10,rc_harrytimemenuico[1].y+26,rgb_black);	                         
                                                        DrawRect(rc_harrytimemenuico[1],2,rgb_yellow);
                                                        update_rect(0,0,500,400);
                                                        g_isUpdated = 1;
                                                }

                                        }else if(count == 11){//# //确认
                                                int times1 = atoi(g_Hourchar);
                                                int times2 = atoi(g_Minchar);
                                                if(times1 > 99){
                                                        CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);
                                                        break;
                                                }
                                                if(times2 > 59){
                                                        CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);
                                                        break;
                                                }
                                                g_hour = times1;
                                                g_min = times2;
                                                if(g_hour == 0 && g_min == 0){
                                                        g_hour = 12; 
                                                        g_min = 0;
                                                }
                                                if(agent_type == 0){   //免打扰 


                                                        pSystemInfo->mySysInfo.isavoid  = true;
                                                        //OpenAndCloseRing(pSystemInfo->mySysInfo.isavoid); //将时间传递进入。
                                                        SetAvoidtimes(g_hour,g_min);
                                                        pSystemInfo->mySysInfo.avoidtimes =g_hour*3600+g_min*60;
							WRT_MESSAGE(" pSystemInfo->mySysInfo.avoidtimes = %d\n", pSystemInfo->mySysInfo.avoidtimes);
                                                        pSystemInfo->mySysInfo.isagent = false;//强制取消托管
                                                        SetAgenttimes(0,0);
                                                        pSystemInfo->mySysInfo.agenttimes = 43200;

                                                }else{
                                                        pSystemInfo->mySysInfo.isagent  = true; 
                                                        SetAgenttimes(g_hour,g_min);
                                                        printf("设置托管\n");
                                                        pSystemInfo->mySysInfo.agenttimes =g_hour*3600+g_min*60;

                                                        pSystemInfo->mySysInfo.isavoid = false;//强制取消免打扰
                                                        SetAvoidtimes(0,0);
                                                        pSystemInfo->mySysInfo.avoidtimes = 43200;


                                                }
                                                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                                save_localhost_config_to_flash();
                                                CloseWindow();
                                                if(pSystemInfo->mySysInfo.isagent  == true)
                                                        CreateDialogWin2(LT("设置托管成功!"),MB_OK,NULL,NULL);

                                                if(pSystemInfo->mySysInfo.isavoid == true)
                                                        CreateDialogWin2(LT("设置免打扰成功"),MB_OK,NULL,NULL);	    	      		      
                                        }
                                }
                                break;
                        }
                }
                return ret;
}

void CreateAvoidHarryWin(int type){
        agent_type = type;
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = AvoidHarryEventHandler;
        pWindow->NotifyReDraw = DrawAvoidHarryMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        GetNumberJpgPointer(&pTimeNumberPointer,&pTimeNumberPointer1);
        GetNumberJpegSizePointer(&pTimeNumberSize,&pTimeNumberSize1);
        pWindow->NotifyReDraw2(pWindow);	
}
void DrawAvoidHarryMain(){
        WRT_Rect tmprect;
        
		WRT_Rect rt;
				rt.x = 0;
				rt.y = 0;
				rt.w = 1024;
				rt.h = 530;
		FillRect(0xb2136,rt);
		
				JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));   
        JpegDisp(rc_harrytimemenuico[3].x,rc_harrytimemenuico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_harrytimemenuico[4].x,rc_harrytimemenuico[4].y,bt_guanping1,sizeof(bt_guanping1)); 
        int i =0; 
        int j =0;
        int startx = rc_harrytimemenuico[2].x;
        int starty = rc_harrytimemenuico[2].y;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pTimeNumberPointer[(i*3)+j],pTimeNumberSize[(i*3)+j]);
                }
                tmprect = rc_harrytimemenuico[0];
                DeflateRect(&tmprect,4);
                DrawRect(rc_harrytimemenuico[0],2,rgb_white);
                FillRect(rgb_white,tmprect);
                
                tmprect = rc_harrytimemenuico[1];
                DeflateRect(&tmprect,4);
                DrawRect(rc_harrytimemenuico[1],2,rgb_white);
                FillRect(rgb_white,tmprect);
                if(agent_type == 0)
                        DrawText_16(LT("请输入免打扰持续时间:"),rc_harrytimemenuico[0].x,rc_harrytimemenuico[0].y-20,rgb_white);
                else
                        DrawText_16(LT("请输入托管的持续时间:"),rc_harrytimemenuico[0].x,rc_harrytimemenuico[0].y-20,rgb_white);
                DrawText(LT("小时"),rc_harrytimemenuico[0].x+rc_harrytimemenuico[0].w+8,rc_harrytimemenuico[0].y+rc_harrytimemenuico[0].h-8,rgb_white);
                DrawText(LT("分钟"),rc_harrytimemenuico[1].x+rc_harrytimemenuico[1].w+8,rc_harrytimemenuico[1].y+rc_harrytimemenuico[1].h-8,rgb_white);
                DrawText(LT("(提示:默认为12小时,时间不能超过99小时,59分)"),rc_harrytimemenuico[0].x-50,50,rgb_red);
                WRT_Rect curbox;
                curbox.x = 135;
                curbox.y = 531;
                curbox.w = 578;
                curbox.h = 55;
	        	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));//清下面状态栏的jpg
	        
                if(agent_type == 0)
                        GetAvoidtimes(&g_hour,&g_min);
                else
                        GetAgenttimes(&g_hour,&g_min);
                if(g_hour == 0 && g_min == 0){
                        g_hour = 12;
                        g_min = 0;
                }

                DrawText_16(LT("提示:按'*'退格，按'#'确认"),curbox.x,curbox.y+45,rgb_black);	
                char tmptext[30];
                //sprintf(tmptext,"持续时间为：%d小时%d分钟\n",g_hour,g_min);
               // if(GetCurrentLanguage() == ENGLISH){
                        sprintf(tmptext,"%s: %d %s %d %s",LT("持续时间为"),g_hour,LT("小时"),g_min,LT("分钟"));
                //}

                DrawText(tmptext,rc_harrytimemenuico[1].x,rc_harrytimemenuico[1].y+rc_harrytimemenuico[1].h+32,rgb_red);
                memset(g_Hourchar,0,3);
                memset(g_Minchar,0,3);
                g_HourMinInputcount=0;
                g_HourMinInputStatus =1;
                DrawSelectEdit();


}

int AvoidHarryEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<5;index++){
                if(IsInside(rc_harrytimemenuico[index],xy)){
                        pos_x = rc_harrytimemenuico[index].x;
                        pos_y = rc_harrytimemenuico[index].y;
                        ret = 1;         
                        switch(index){     
                case 0:
                        g_HourMinInputStatus = 1;
                        g_HourMinInputcount = strlen(g_Hourchar);
                        g_isUpdated = 0;
                        DrawSelectEdit();
                        update_rect(0,0,500,400);
                        g_isUpdated = 1;
                        break; 
                case 1:
                        g_HourMinInputStatus = 2;
                        g_HourMinInputcount =strlen(g_Minchar);
                        g_isUpdated = 0;
                        DrawSelectEdit();
                        update_rect(0,0,500,400);
                        
                        g_isUpdated = 1;
                        break;        
                case 3: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                CloseTopWindow();                 
                        }
                        break;
                case 4:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close(); 
                                CloseAllWindows();
                        }
                        break;
                case 2:
                        ret = avoidharryhandler(x,y,status);
                        break;
                default:
                        break;
                        }
                        break;
                }
        }      
        return ret;  	
}

int CancelAvoid(int param){

        pSystemInfo->mySysInfo.isavoid = false;
        pSystemInfo->mySysInfo.avoidtimes = 43200; //恢复默认的免打扰时间
        //      OpenAndCloseRing(pSystemInfo->mySysInfo.isavoid); //将时间传递进入。
        SetAvoidtimes(0,0);  
        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);    
        save_localhost_config_to_flash();
        CloseWindow();
        CreateDialogWin2(LT("取消免打扰设置成功!"),MB_NONE,NULL,NULL);
        SDL_Delay(2000);
        CloseWindow();
        return 0;

}

int CancelAgent(int param){
        pSystemInfo->mySysInfo.isagent = false;
        pSystemInfo->mySysInfo.agenttimes = 43200; //默认12小时托管
        SetAgenttimes(0,0);  
        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);    
        save_localhost_config_to_flash();
        CloseWindow();
        CreateDialogWin2(LT("取消托管设置成功!"),MB_NONE,NULL,NULL);     
        SDL_Delay(2000);
        CloseWindow();
        return 0;

}
////////////////////////////////////////////////////////////
//
static int win_type =0;
static WRT_Rect rc_typemenuico[5]={
        {150,164,100,100},
        {460,164,100,100},
        {380,195,72,75},
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕		
};

static WRT_Rect  rcVolume[17] = {
        {450,100,210,40},
        {450,160,210,40},
        {450,220,210,40},
        {450,280,210,40},
        {450,340,210,40},
        {380,100,60,40},//+
        {670,100,60,40},//-          
        {380,160,60,40},//+
        {670,160,60,40},//-         
        {380,220,60,40},//+
        {670,220,60,40},//-	
        {380,280,60,40},//+
        {670,280,60,40},//-		

        {380,340,60,40},//+
        {670,340,60,40},//-			

        {900,531,111,69},
        {6,531,111,69}
};
static int g_issave = 0;	
static int g_Bass = 10;
static int g_Treble =3;
static int g_volume = 40;	
static int g_talkvolume = 40;
static int g_alarmvolume = 40;
static int g_micvolume = 40;
static void Draw_volume(){
        int i =0;
        WRT_Rect tmp;
        char ttext[10];
        tDraw3dInset(rcVolume[0].x,rcVolume[0].y,rcVolume[0].w,rcVolume[0].h);
        tmp = rcVolume[0];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);
        tmp.w = (g_volume)*2.1*2;// 5个等级X4.2
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",g_volume*2);
        DrawTextToRect(rcVolume[0],ttext);//画第一个音量

        tDraw3dInset(rcVolume[1].x,rcVolume[1].y,rcVolume[1].w,rcVolume[1].h);
        tmp = rcVolume[1];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);
        tmp.w = g_talkvolume*2.1*2;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",g_talkvolume*2);
        DrawTextToRect(rcVolume[1],ttext);

        tDraw3dInset(rcVolume[2].x,rcVolume[2].y,rcVolume[2].w,rcVolume[2].h);
        tmp = rcVolume[2];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);	
        tmp.w = g_alarmvolume*2.1*2;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",g_alarmvolume*2);
        DrawTextToRect(rcVolume[2],ttext);

		tDraw3dInset(rcVolume[3].x,rcVolume[3].y,rcVolume[3].w,rcVolume[3].h);
	    tmp = rcVolume[3];
	    DeflateRect(&tmp,1);
	    FillRect(WRT0RGB(255,255,255),tmp);	
	    tmp.w = g_micvolume*2.1;
	    FillRect(WRT0RGB(0,128,255),tmp);
	    sprintf(ttext,"%d",g_micvolume);
	    DrawTextToRect(rcVolume[3],ttext);	
/*
        tDraw3dInset(rcVolume[3].x,rcVolume[3].y,rcVolume[3].w,rcVolume[3].h);
        tmp = rcVolume[3];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);	
        tmp.w = g_talkvolume*2.1+1;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",g_talkvolume);
        DrawTextToRect(rcVolume[3],ttext);	

        tDraw3dInset(rcVolume[4].x,rcVolume[4].y,rcVolume[4].w,rcVolume[4].h);
        tmp = rcVolume[4];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);	
        tmp.w = g_alarmvolume*2.1+1;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",g_alarmvolume);
        DrawTextToRect(rcVolume[4],ttext);		
*/
   
		tDraw3dOutset(rcVolume[5].x,rcVolume[5].y,rcVolume[5].w,rcVolume[5].h);
		tDraw3dOutset(rcVolume[6].x,rcVolume[6].y,rcVolume[6].w,rcVolume[6].h);
		tDraw3dOutset(rcVolume[7].x,rcVolume[7].y,rcVolume[7].w,rcVolume[7].h);
		tDraw3dOutset(rcVolume[8].x,rcVolume[8].y,rcVolume[8].w,rcVolume[8].h);
		tDraw3dOutset(rcVolume[9].x,rcVolume[9].y,rcVolume[9].w,rcVolume[9].h);	
		tDraw3dOutset(rcVolume[10].x,rcVolume[10].y,rcVolume[10].w,rcVolume[10].h);

		tDraw3dOutset(rcVolume[11].x,rcVolume[11].y,rcVolume[11].w,rcVolume[11].h);		
		tDraw3dOutset(rcVolume[12].x,rcVolume[12].y,rcVolume[12].w,rcVolume[12].h);			
/*
        tDraw3dOutset(rcVolume[13].x,rcVolume[13].y,rcVolume[13].w,rcVolume[13].h);
        tDraw3dOutset(rcVolume[14].x,rcVolume[14].y,rcVolume[14].w,rcVolume[14].h);		
        */
	for(i =5;i<13;i++){
                Color_FillRect(rcVolume[i],0,0,255);
                if(i%2 )
                        DrawTextToRect(rcVolume[i],"-");
		else	
                        DrawTextToRect(rcVolume[i],"+");
        }	
}

static void Draw_Valume1(int index,int isupdate){
        int itmp;
        int itmp2 =0;  
        int itmp3 = 0;
        WRT_Rect tmp;
        char ttext[10];	  
        itmp = index;
        if(!(itmp%2)){ //+
                if(itmp == 6 ){ 
                        itmp2 = 0;   
                        g_volume +=10;  
                        if(g_volume >= 50)
                                g_volume= 50; 
                        itmp3 = g_volume*2;
                        pSystemInfo->mySysInfo.ringvolume = g_volume+40 ;
                        SetAlsaVolume(TYPE_PLAY,pSystemInfo->mySysInfo.ringvolume);

                }else if(itmp == 8 ){
                        itmp2= 1;
                        g_talkvolume +=10;  
                        if(g_talkvolume >= 50)
                                g_talkvolume= 50; 
                        itmp3 = g_talkvolume*2;	 
                        pSystemInfo->mySysInfo.talkvolume = g_talkvolume + 40 ;   		

                }else if(itmp == 10){
                        itmp2 = 2;
                        g_alarmvolume +=10;  
                        if(g_alarmvolume >= 50)
                                g_alarmvolume= 50; 
                        itmp3 = g_alarmvolume*2;	 
                        pSystemInfo->mySysInfo.alarmvolume  	= g_alarmvolume + 40;   		   	 
                }else if(itmp == 12)
                {
                		itmp2 = 3;
                        g_micvolume+=20;  
                        if(g_micvolume >= 100)
                                g_micvolume= 100; 
                        itmp3 = g_micvolume;
                        pSystemInfo->mySysInfo.micvolume = g_micvolume;
                } 

        }else{//-
                if(itmp == 5){ 
                        itmp2 = 0;   
                        g_volume -=10;  
                        if(g_volume <= 0)
                                g_volume = 10; 
                        itmp3 =  g_volume*2;
                        pSystemInfo->mySysInfo.ringvolume = g_volume + 40;
                        SetAlsaVolume(TYPE_PLAY,pSystemInfo->mySysInfo.ringvolume);

                }else if(itmp == 7){
                        itmp2= 1;
                        g_talkvolume -=10;  
                        if(g_talkvolume <= 0)
                                g_talkvolume= 10; 
                        itmp3 = g_talkvolume*2;	 
                        pSystemInfo->mySysInfo.talkvolume = g_talkvolume + 40 ;
                        //tmVideoDisp_SetContrast(pSystemInfo->mySysInfo.contrast);   	  
                }else if(itmp == 9){
                        itmp2 = 2;
                        g_alarmvolume -=10;  
                        if(g_alarmvolume <= 0)
                                g_alarmvolume= 10; 
                        itmp3 = g_alarmvolume*2;	 
                        pSystemInfo->mySysInfo.alarmvolume  	= g_alarmvolume+40; 
                        // tmVideoDisp_SetSaturation(pSystemInfo->mySysInfo.saturation); 	  	   	 
                }else if(itmp == 11){
                		itmp2 = 3;
                        g_micvolume-=20;  
                        if(g_micvolume <= 0)
                                g_micvolume= 0; 
                        itmp3 = g_micvolume;
                        pSystemInfo->mySysInfo.micvolume = g_micvolume;
                } 	  	
        }
	if(isupdate == 1) 
		g_isUpdated = 0;
        tDraw3dInset(rcVolume[itmp2].x,rcVolume[itmp2].y,rcVolume[itmp2].w,rcVolume[itmp2].h);
        tmp = rcVolume[itmp2];
        DeflateRect(&tmp,1);

        FillRect(WRT0RGB(255,255,255),tmp);	  
        tmp.w = itmp3*2.1;
        
        if(tmp.w == 0)
                tmp.w = 1;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",itmp3);
        DrawTextToRect(rcVolume[itmp2],ttext);	  	
        g_issave =1;  	
	if(isupdate == 1){
			update_rect(rcVolume[itmp2].x,rcVolume[itmp2].y,rcVolume[itmp2].w,rcVolume[itmp2].h);
		     g_isUpdated = 1;		
	}
}

void CreateTypeSet(int type){

        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = TypeSetEventHandler;
        pWindow->NotifyReDraw = DrawTypeSetMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        g_issave = 0;
        g_volume =pSystemInfo->mySysInfo.ringvolume-40;
        g_talkvolume = pSystemInfo->mySysInfo.talkvolume-40;
        g_alarmvolume = pSystemInfo->mySysInfo.alarmvolume-40;
        g_micvolume   = pSystemInfo->mySysInfo.micvolume;
        WindowAddToWindowList(pWindow);  	
        pWindow->NotifyReDraw2(pWindow);
}

void DrawTypeSetMain(){
	WRT_Rect curbox;
	int xoffset = 0;
	curbox.x = 0;
	curbox.y = 0;
	curbox.w = 800;
	curbox.h = 422;    

	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	xoffset = rcVolume[5].x-230 ;
	DrawText(LT("铃声音量"),xoffset,130,rgb_white);
	DrawText(LT("通话音量"),xoffset,190,rgb_white);
	DrawText(LT("报警音量"),xoffset,250,rgb_white);
	DrawText(LT("麦克风音量"),xoffset,310,rgb_white);

	Draw_volume();
	JpegDisp(rcVolume[15].x,rcVolume[15].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rcVolume[16].x,rcVolume[16].y,bt_guanping1,sizeof(bt_guanping1));  
}

int TypeSetEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =5;index<17;index++){
                if(IsInside(rcVolume[index],xy)){
                        pos_x = rcVolume[index].x;
                        pos_y = rcVolume[index].y;
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
                        switch(index)
                        {         
                case 15: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));  
                                if(g_issave == 1){ 
                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                        save_localhost_config_to_flash();
                                        CloseWindow();
                                        CreateDialogWin2(LT("保存设置成功!"),MB_NONE,NULL,NULL);
                                        g_issave = 0;
                                        SDL_Delay(1000);
                                        CloseWindow();
                                }
                                CloseTopWindow();                   
                        }
                        break;
                case 16:/*关屏*/
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
                	if(index >12) //5 6 7 8 9 10
                		break;
                        if(status == 1){
                                StartButtonVocie();
								WRT_Rect uprect = rcVolume[index];
								g_isUpdated = 0;
				                tDraw3dInset(rcVolume[index].x,rcVolume[index].y,rcVolume[index].w,rcVolume[index].h);
								update_rect(uprect.x,uprect.y,uprect.w,uprect.h);

				                Color_FillRect(rcVolume[index],255,0,0);	
								if(index%2 )
								        DrawTextToRect(rcVolume[index],"-");
								else
								     	DrawTextToRect(rcVolume[index],"+");   
								//uprect.x -= 4;
								//uprect.y -= 4;
								//uprect.w +=4;
								//uprect.h +=4;
								update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
								g_isUpdated = 1;
                        }else{
								
								WRT_Rect uprect = rcVolume[index];
								g_isUpdated = 0;
				                                tDraw3dOutset(rcVolume[index].x,rcVolume[index].y,rcVolume[index].w,rcVolume[index].h);
								update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
				                                Color_FillRect(rcVolume[index],0,0,255);	
				                                if(index%2 )
				                                        DrawTextToRect(rcVolume[index],"-");
				                                else
				                                        DrawTextToRect(rcVolume[index],"+"); 
				  
								//uprect.x -= 4;
								//uprect.y -= 4;
								//uprect.w +=4;
								//uprect.h +=4;
								update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
								g_isUpdated = 1;
									
									
								Draw_Valume1(index,1);
								//update_rect(rcVolume[0].x,rcVolume[0].y,rcVolume[0].w,rcVolume[0].h);
								//update_rect(rcVolume[1].x,rcVolume[1].y,rcVolume[1].w,rcVolume[1].h);
								//update_rect(rcVolume[2].x,rcVolume[2].y,rcVolume[2].w,rcVolume[2].h);
								//update_rect(rcVolume[3].x,rcVolume[3].y,rcVolume[3].w,rcVolume[3].h);

                        }
                        break;
                }
                break;
                }
        }      
        return ret;  	

}	

int g_videobright = 0;
int g_videocontrast = 0;
int g_videosaturation = 0;

static WRT_Rect  rcColorManage[11] = {
        {420,120,200,40},
        {420,180,200,40},
        {420,240,200,40},
        {340,120,60,40},//-
        {640,120,60,40},//+          
        {340,180,60,40},//-
        {640,180,60,40},//+         
        {340,240,60,40},//-
        {640,240,60,40},//+	

        {900,531,111,69},
        {6,531,111,69}
};




static void Draw_Color(){
        int i =0;
        WRT_Rect tmp;
        char ttext[10];
        tDraw3dInset(rcColorManage[0].x,rcColorManage[0].y,rcColorManage[0].w,rcColorManage[0].h);
        tmp = rcColorManage[0];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);
        tmp.w = pSystemInfo->mySysInfo.bright*2;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",pSystemInfo->mySysInfo.bright);
        DrawTextToRect(rcColorManage[0],ttext);

        tDraw3dInset(rcColorManage[1].x,rcColorManage[1].y,rcColorManage[1].w,rcColorManage[1].h);
        tmp = rcColorManage[1];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);
        tmp.w = pSystemInfo->mySysInfo.contrast*2;	
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",pSystemInfo->mySysInfo.contrast);
        DrawTextToRect(rcColorManage[1],ttext);

        tDraw3dInset(rcColorManage[2].x,rcColorManage[2].y,rcColorManage[2].w,rcColorManage[2].h);
        tmp = rcColorManage[2];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);
        tmp.w = pSystemInfo->mySysInfo.saturation*2;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",pSystemInfo->mySysInfo.saturation);
        DrawTextToRect(rcColorManage[2],ttext);

        tDraw3dOutset(rcColorManage[3].x,rcColorManage[3].y,rcColorManage[3].w,rcColorManage[3].h);
        tDraw3dOutset(rcColorManage[4].x,rcColorManage[4].y,rcColorManage[4].w,rcColorManage[4].h);
        tDraw3dOutset(rcColorManage[5].x,rcColorManage[5].y,rcColorManage[5].w,rcColorManage[5].h);
        tDraw3dOutset(rcColorManage[6].x,rcColorManage[6].y,rcColorManage[6].w,rcColorManage[6].h);
        tDraw3dOutset(rcColorManage[7].x,rcColorManage[7].y,rcColorManage[7].w,rcColorManage[7].h);
        tDraw3dOutset(rcColorManage[8].x,rcColorManage[8].y,rcColorManage[8].w,rcColorManage[8].h);	
        for(i =3;i<9;i++){
                Color_FillRect(rcColorManage[i],0,0,255);	
                if(i%2 == 0)
                {
                        DrawTextToRect(rcColorManage[i],"+");
                }
                else
                {		
                        DrawTextToRect(rcColorManage[i],"-");
                 
                }
        }
}

static void Draw_Color1(int index,int isupdate){
        int itmp;
        int itmp2 =0;  
        int itmp3 = 0;
        WRT_Rect tmp;
        char ttext[10];	  
        itmp = index;
        if(!(itmp%2)){ //+
                if(itmp == 3 || itmp == 4){ 
                        itmp2 = 0;   
                        pSystemInfo->mySysInfo.bright +=1;  
                        if(pSystemInfo->mySysInfo.bright > 100)
                                pSystemInfo->mySysInfo.bright = 100; 
                        itmp3 =    pSystemInfo->mySysInfo.bright;
                       // tmVideoDisp_SetBrightness(pSystemInfo->mySysInfo.bright);

                }else if(itmp == 5 || itmp == 6){
                        itmp2= 1;
                        pSystemInfo->mySysInfo.contrast +=1;   
                        if(pSystemInfo->mySysInfo.contrast > 100)
                                pSystemInfo->mySysInfo.contrast = 100;
                        itmp3 =    pSystemInfo->mySysInfo.contrast;	  	
                       // tmVideoDisp_SetContrast(pSystemInfo->mySysInfo.contrast);   	  
                }else{
                        itmp2 = 2;
                        pSystemInfo->mySysInfo.saturation +=1;   
                        if(pSystemInfo->mySysInfo.saturation > 100)
                                pSystemInfo->mySysInfo.saturation = 100;
                        itmp3 =    pSystemInfo->mySysInfo.saturation;	 
                      //  tmVideoDisp_SetSaturation(pSystemInfo->mySysInfo.saturation); 	  	   	 
                } 

        }else{//-
                if(itmp == 3 || itmp == 4){ 
                        itmp2 = 0;   
                        pSystemInfo->mySysInfo.bright -=1;  
                        if(pSystemInfo->mySysInfo.bright < 0)
                                pSystemInfo->mySysInfo.bright =0; 
                        itmp3 =    pSystemInfo->mySysInfo.bright;
                       // tmVideoDisp_SetBrightness(pSystemInfo->mySysInfo.bright);	  	   	    
                }else if(itmp == 5 || itmp == 6){
                        itmp2= 1;
                        pSystemInfo->mySysInfo.contrast -=1; 
                        if(pSystemInfo->mySysInfo.contrast < 0)
                                pSystemInfo->mySysInfo.contrast = 0;  
                        itmp3 =    pSystemInfo->mySysInfo.contrast;	  
                       // tmVideoDisp_SetContrast(pSystemInfo->mySysInfo.contrast); 	   	  
                }else{
                        itmp2 = 2;
                        pSystemInfo->mySysInfo.saturation -=1;   
                        if(pSystemInfo->mySysInfo.saturation < 0)
                                pSystemInfo->mySysInfo.saturation = 0;
                        itmp3 =    pSystemInfo->mySysInfo.saturation;	
                      //  tmVideoDisp_SetSaturation(pSystemInfo->mySysInfo.saturation);  	  	   	 
                } 	  	
        }
        g_videobright = pSystemInfo->mySysInfo.bright;
        g_videocontrast = pSystemInfo->mySysInfo.contrast;
        g_videosaturation = pSystemInfo->mySysInfo.saturation;
		if(isupdate == 1)
		    g_isUpdated = 0;
        tDraw3dInset(rcColorManage[itmp2].x,rcColorManage[itmp2].y,rcColorManage[itmp2].w,rcColorManage[itmp2].h);
        tmp = rcColorManage[itmp2];
        DeflateRect(&tmp,1);
        FillRect(WRT0RGB(255,255,255),tmp);	  
        tmp.w = itmp3*2;
        FillRect(WRT0RGB(0,128,255),tmp);
        sprintf(ttext,"%d",itmp3);
        DrawTextToRect(rcColorManage[itmp2],ttext);	  	
        g_issave =1;  
		if(isupdate == 1){
			update_rect(rcColorManage[itmp2].x,rcColorManage[itmp2].y,rcColorManage[itmp2].w,rcColorManage[itmp2].h);
		    g_isUpdated = 1;
		}
}

void CreateColorManager(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;  
        pWindow->EventHandler = ColorManagerEventHandler;
        pWindow->NotifyReDraw = DrawColorManager;
        pWindow->valid = 1;
        g_issave =0;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        pWindow->NotifyReDraw2(pWindow);	
}
void DrawColorManager()
{
        WRT_Rect curbox;
        int xoffset = 0;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 800;
        curbox.h = 422;  
        
		WRT_Rect rt;
				rt.x = 0;
				rt.y = 0;
				rt.w =1024;
				rt.h = 530;
		FillRect(0xb2136,rt);
		
				JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
       // JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));    
       // FillRect(BACK_COLOR,curbox);
        xoffset = rcColorManage[3].x -200;
        DrawText(LT("视频亮度"),xoffset,150,rgb_white);

        xoffset = rcColorManage[3].x-200 ;
        DrawText(LT("视频对比度"),xoffset,210,rgb_white);

        xoffset = rcColorManage[3].x -200;
        DrawText(LT("视频饱和度"),xoffset,270,rgb_white);
        Draw_Color();
        JpegDisp(rcColorManage[9].x,rcColorManage[9].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcColorManage[10].x,rcColorManage[10].y,bt_guanping1,sizeof(bt_guanping1)); 
        

  	    
}
int ColorManagerEventHandler(int x,int y,int stauts){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =3;index<11;index++){
                if(IsInside(rcColorManage[index],xy)){
                        pos_x = rcColorManage[index].x;
                        pos_y = rcColorManage[index].y;
                        ret = 1; 
                        if(stauts == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }		                    
                        switch(index){         
                case 9: /*返回*/
                        if(stauts == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));  
                                if(g_issave == 1){ 
                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                        save_localhost_config_to_flash();
                                        CloseWindow();
                                        CreateDialogWin2(LT("保存设置成功!"),MB_NONE,NULL,NULL);
                                        g_issave = 0;
                                        SDL_Delay(1000);
                                        CloseWindow();
                                }
                                CloseTopWindow();                   
                        }
                        break;
                case 10:/*关屏*/
                        if(stauts == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                if(g_issave == 1){ 
                                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                        save_localhost_config_to_flash();
                                        CloseWindow();
                                        CreateDialogWin2(LT("保存设置成功!"),MB_NONE,NULL,NULL);
                                        g_issave = 0;
                                        SDL_Delay(1000);
                                        CloseWindow();
                                }
                                screen_close(); 
                                CloseAllWindows();
                        }
                        break;
                default:
                        if(stauts == 1){
                                StartButtonVocie();
								g_isUpdated = 0;
								WRT_Rect uprect = rcColorManage[index];
                                tDraw3dInset(rcColorManage[index].x,rcColorManage[index].y,rcColorManage[index].w,rcColorManage[index].h);
								update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
                                Color_FillRect(rcColorManage[index],255,0,0);	
                                if(index%2 == 0)
                                        DrawTextToRect(rcColorManage[index],"+");
                                else
                                        DrawTextToRect(rcColorManage[index],"-");  
   
				uprect.x -= 4;
				uprect.y -= 4;
				uprect.w += 4;
				uprect.h += 4;
				update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
				g_isUpdated = 1;
                        }else{
				g_isUpdated = 0;
				WRT_Rect uprect = rcColorManage[index];
                                tDraw3dOutset(rcColorManage[index].x,rcColorManage[index].y,rcColorManage[index].w,rcColorManage[index].h);
								update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
                                Color_FillRect(rcColorManage[index],0,0,255);	
                                if(index%2 == 0)
                                        DrawTextToRect(rcColorManage[index],"+");
                                else
                                        DrawTextToRect(rcColorManage[index],"-");                	   	
                               
				
				
				uprect.x -= 4;
				uprect.y -= 4;
				uprect.w +=4;
				uprect.h +=4;
				update_rect(uprect.x,uprect.y,uprect.w,uprect.h);
				g_isUpdated = 1;
								
				Draw_Color1(index,1);	
															
                        }
                        break;
                        }
                        break;
                }
        }      
        return ret;  	

}

static WRT_Rect rcVoiceControl[4] ={	
        {189,149,132,160},          
        {428,149,132,160},  
        {900,531,111,69},
        {6,531,111,69}
};

void CreateVoiceControl(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = VoiceControlEventHandle;
        pWindow->NotifyReDraw = DrawVoiceControlMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        pWindow->NotifyReDraw2(pWindow);
}
void DrawVoiceControlMain(){
        JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));  
        if(GetButtonVoiceStatus())
                JpegDisp(rcVoiceControl[0].x,rcVoiceControl[0].y,menu_buttonvoiceopen1,sizeof(menu_buttonvoiceopen1));
        else
                JpegDisp(rcVoiceControl[0].x,rcVoiceControl[0].y,menu_buttonvoiceclose1,sizeof(menu_buttonvoiceclose1));
        JpegDisp(rcVoiceControl[1].x,rcVoiceControl[1].y,menu_yinliangset1,sizeof(menu_yinliangset1));         
        JpegDisp(rcVoiceControl[2].x,rcVoiceControl[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcVoiceControl[3].x,rcVoiceControl[3].y,bt_guanping1,sizeof(bt_guanping1));    
}
int VoiceControlEventHandle(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rcVoiceControl[index],xy)){
                        pos_x = rcVoiceControl[index].x;
                        pos_y = rcVoiceControl[index].y;
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
                 case 0://按键伴音           	 	
                         if(status == 1){
                                 StartButtonVocie();
                                 if(GetButtonVoiceStatus())
                                         JpegDisp(pos_x,pos_y,menu_buttonvoiceopen2,sizeof(menu_buttonvoiceopen2));
                                 else
                                         JpegDisp(pos_x,pos_y,menu_buttonvoiceclose2,sizeof(menu_buttonvoiceclose2));
                         }else{
                                 if(GetButtonVoiceStatus()){
                                         OpenAndCloseButtonVoice(false);//关闭按键音
                                         pSystemInfo->mySysInfo.isbuttonvoice = false;
                                         JpegDisp(pos_x,pos_y,menu_buttonvoiceclose1,sizeof(menu_buttonvoiceclose1));
                                 }else{
                                         OpenAndCloseButtonVoice(true);//打开按键音
                                         pSystemInfo->mySysInfo.isbuttonvoice = true;	      
                                         JpegDisp(pos_x,pos_y,menu_buttonvoiceopen1,sizeof(menu_buttonvoiceopen1));
                                 }

                                 CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                 save_localhost_config_to_flash(); 
                                 CloseTopWindow();

                         }                   	 	
                         break;
                 case 1://音量

                         if(status == 1){
                                 StartButtonVocie();
                                 JpegDisp(pos_x,pos_y,menu_yinliangset2,sizeof(menu_yinliangset2));
                         }else{
                                 JpegDisp(pos_x,pos_y,menu_yinliangset1,sizeof(menu_yinliangset1));    
                                 CreateTypeSet(VOLUME_SET_WIN);              
                         }                                  	 	
                         break;            	           
                 case 2: /*返回*/
                         if(status == 1){
                                 StartButtonVocie();
                                 JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                         }else{
                                 JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));  
                                 CloseTopWindow();                   
                         }
                         break;
                 case 3:/*关屏*/
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

////////////////////////////////////////////////////////////////////////
//2009-10-27
static const WRT_Rect rcExtra[9]={

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

static const WRT_Rect rcExtra_Ico[9]={
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
int g_iseffect = 0;

static void UpdateExtraFun(int index,int start){
        if(start)
                JpegDisp(rcExtra_Ico[index].x,rcExtra_Ico[index].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
                JpegDisp(rcExtra_Ico[index].x,rcExtra_Ico[index].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
}

void CreateExtraFunction(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = ExtraFunctionEventHandler;
        pWindow->NotifyReDraw = DrawExtraFunction;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);      
        pWindow->NotifyReDraw2(pWindow);
}
void DrawExtraFunction(){
	WRT_Rect rt;
			rt.x = 0;
			rt.y = 0;
			rt.w =1024;
			rt.h = 530;
	FillRect(0xb2136,rt);
	
			JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back)); 
        JpegDisp(rcExtra[7].x,rcExtra[7].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcExtra[8].x,rcExtra[8].y,bt_guanping1,sizeof(bt_guanping1));   
        DrawText(LT("是否启动自动抓拍"),  rcExtra[0].x,rcExtra[0].y,rgb_white);    
        DrawText(LT("是否启动按键提示音"),  rcExtra[1].x,rcExtra[1].y,rgb_white);
        DrawText(LT("是否启动屏保"),  rcExtra[2].x,rcExtra[2].y,rgb_white);
       // DrawText(LT("是否挂接模拟分机"),  rcExtra[3].x,rcExtra[3].y,rgb_white);

        if(pSystemInfo->mySysInfo.isenglish == 0)
                DrawText("语言:英文",  rcExtra[3].x,rcExtra[3].y,rgb_white);
        else{
                DrawText("Language:Chinese(Simplified)",  rcExtra[3].x,rcExtra[3].y,rgb_white);
        }
        if(pSystemInfo->mySysInfo.isuseui == 0x1){
        	DrawText(LT("时尚风格"),rcExtra[4].x,rcExtra[4].y,rgb_white);
        }else{
        	DrawText(LT("默认风格"),rcExtra[4].x,rcExtra[4].y,rgb_white);
        }
		//DrawText(LT("访客呼叫优先"),rcExtra[5].x,rcExtra[5].y,rgb_white);
        if(pSystemInfo->mySysInfo.isautocap == 1)		
                JpegDisp(rcExtra_Ico[0].x,rcExtra_Ico[0].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
                JpegDisp(rcExtra_Ico[0].x,rcExtra_Ico[0].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));   

        if(GetButtonVoiceStatus())		
                JpegDisp(rcExtra_Ico[1].x,rcExtra_Ico[1].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
                JpegDisp(rcExtra_Ico[1].x,rcExtra_Ico[1].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));   

        if(pSystemInfo->mySysInfo.iskeepscreen == 1)
                JpegDisp(rcExtra_Ico[2].x,rcExtra_Ico[2].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
                JpegDisp(rcExtra_Ico[2].x,rcExtra_Ico[2].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));  

        //if(pSystemInfo->mySysInfo.isnvo == 1)
        //        JpegDisp(rcExtra_Ico[3].x,rcExtra_Ico[3].y,ico_fangqukai,sizeof(ico_fangqukai));
        //else
        //        JpegDisp(rcExtra_Ico[3].x,rcExtra_Ico[3].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));    
//#ifndef THREE_UI                      
		JpegDisp(rcExtra_Ico[3].x,rcExtra_Ico[3].y,ico_fangquguan,sizeof(ico_fangquguan));

        JpegDisp(rcExtra_Ico[4].x,rcExtra_Ico[4].y,ico_fangquguan,sizeof(ico_fangquguan));                
//#endif        
	//JpegDisp(rcExtra_Ico[5].x,rcExtra_Ico[5].y,ico_fangquguan,sizeof(ico_fangquguan));
	
		//if(pSystemInfo->mySysInfo.doorcallproir)		
		//	JpegDisp(rcExtra_Ico[5].x,rcExtra_Ico[5].y,ico_fangqukai,sizeof(ico_fangqukai));
	    //else
		//	JpegDisp(rcExtra_Ico[5].x,rcExtra_Ico[5].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));   

}
int   ExtraFunctionEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;


        for(index =0;index<9;index++){
                if(IsInside(rcExtra_Ico[index],xy)){
                        pos_x = rcExtra_Ico[index].x;
                        pos_y = rcExtra_Ico[index].y;
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
                 case 0://自动抓拍
                         if(status == 1){
                                 StartButtonVocie();
                                 break;
                         }
                         if(pSystemInfo->mySysInfo.isautocap == 1){
                                 pSystemInfo->mySysInfo.isautocap = 0;
                                 UpdateExtraFun(0,0);
                         }else{
                                 pSystemInfo->mySysInfo.isautocap = 1;
                                 UpdateExtraFun(0,1);
                         }
                         CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                         
                         save_localhost_config_to_flash(); 
                         SDL_Delay(500);
                         CloseTopWindow();                                	 	
                         break;
                 case 1://按键音
                         if(status == 1){
                                 StartButtonVocie();
                                 SDL_Delay(150);
                                 break;
                         }                   
                         if(GetButtonVoiceStatus()){
                                 OpenAndCloseButtonVoice(false);//关闭按键音
                                 pSystemInfo->mySysInfo.isbuttonvoice = false;
                                 UpdateExtraFun(1,0);
                         }else{
                                 OpenAndCloseButtonVoice(true);//打开按键音
                                 pSystemInfo->mySysInfo.isbuttonvoice = true;	      
                                 UpdateExtraFun(1,1);
                         }
                         CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                         save_localhost_config_to_flash(); 
                         SDL_Delay(500);
                         CloseTopWindow();                                	 	
                         break;    
                 case 2: //屏保
                         if(status == 1){
                                 StartButtonVocie();
                                 SDL_Delay(150);
                                 break;
                         }                    
                         if(pSystemInfo->mySysInfo.iskeepscreen == 0){
                                 pSystemInfo->mySysInfo.iskeepscreen =1;
                                 screenkeep_init();
                                 screenkeep_start();
                                 UpdateExtraFun(2,1);

                         }else{
                                 pSystemInfo->mySysInfo.iskeepscreen =0;
                                 screenkeep_stop();
                                 UpdateExtraFun(2,0);
                         }  
                         CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                         save_localhost_config_to_flash();
                         SDL_Delay(500);
                         CloseTopWindow();                                                           
                         break; 
                 case 3:
//#ifdef THREE_UI     //暂时去掉中英文切换 
//				break;
//#endif

                         if(status == 1){
                                 StartButtonVocie();
                                 SDL_Delay(200);
                                 UpdateExtraFun(3,1);
                                 break;
                         } 

                         if(pSystemInfo->mySysInfo.isenglish == 0){

                                 UpdateExtraFun(3,1);
                                 CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                 pSystemInfo->mySysInfo.isenglish = 1;
                                 SetSysInfoToEnglish();

                         }else{

                                 UpdateExtraFun(3,1);
                                 CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                 pSystemInfo->mySysInfo.isenglish = 0;
                                 SetSysInfoToChinese();
                         }                        
                         //save_localhost_config_to_flash(); 
                         UpdateSystemInfo();
                         CloseTopWindow();   
                         break;   
                case 4:
                	if(status == 1){
                                 StartButtonVocie();
                                 SDL_Delay(200);
                                 UpdateExtraFun(4,1);
                                 break;
                         } 
                         if(pSystemInfo->mySysInfo.isuseui == 0x1){
                                 UpdateExtraFun(4,1);
                                 CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                 pSystemInfo->mySysInfo.isuseui = 0x2;
                         }else{
                                 UpdateExtraFun(4,1);
                                 CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                 pSystemInfo->mySysInfo.isuseui = 0x1;
                                 
                         }
                         UpdateSystemInfo();  
                         SwitchSkin(pSystemInfo->mySysInfo.isuseui);//切换皮肤
                         SDL_Delay(500);
                         CloseAllWindows();
                         
                                                  
                	break;  
                 case 5://door call proir
                 		break;
                         if(status == 1){
                             StartButtonVocie();
                             SDL_Delay(150);
                             break;
                         }                   
                         if(pSystemInfo->mySysInfo.doorcallproir == 1){
                             pSystemInfo->mySysInfo.doorcallproir = 0;
                             UpdateExtraFun(5,0);
                         }else{
                             pSystemInfo->mySysInfo.doorcallproir = 1;	      
                             UpdateExtraFun(5,1);
                         }
                         CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                         UpdateSystemInfo();
                         SDL_Delay(500);
                         CloseTopWindow();                                	 	
                	break;  
                 case 6:
                        #if 0
                        if(status == 1){
                                  
                                 StartButtonVocie();
                                 break;                                
                        }
                        if(pSystemInfo->mySysInfo.effect == 1){
                                pSystemInfo->mySysInfo.effect  = 0;
                                UpdateExtraFun(6,1);
                        }else{
                                pSystemInfo->mySysInfo.effect  = 1;
                                
                                UpdateExtraFun(6,0);
                        }
                        g_iseffect = pSystemInfo->mySysInfo.effect;
                        save_localhost_config_to_flash(); 
                        ReDrawCurrentWindow();
						#endif
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
