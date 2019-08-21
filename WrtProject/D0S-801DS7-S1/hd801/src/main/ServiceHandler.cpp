#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "service_menu_res.h"
#include "window.h"

#include "c_interface.h"
#include "sip_data_sources.h"


#define TE_MAINTAIN 0
#define ENTERTAINMENT_ENABLE 0

#if TE_MAINTAIN
#include "maintain_res.h"
#endif

extern unsigned char meituyule_2[29621];
extern unsigned char meituyule_1[30210];


static const WRT_Rect rc_servicemenuico[10]={
        {108,83, 132,160},   //维修求助
        {326,83, 132,160},   //费用查询
        {555,83,132,160},    //铃声下载
		{779,83,132,160},    //便民服务

        {108,305,132,160},   //家庭留言
        {326,305,132,160},   //服务平台
        {555,305,132,160},   //闹钟
 #if ENTERTAINMENT_ENABLE
        {779,305,132,160},  //娱乐 
 #endif
	/*
        {125,60,132,160},    //维修求助
        {325,60,132,160},   //费用查询
        {525,60,132,160},  //铃声下载

        {125,230,132,160},   //便民服务
        {325,230,132,160},   //家庭留言
        {525,230,132,160},   //服务平台
	*/
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

void CreateServiceWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = ServiceEventHandler;
        pWindow->NotifyReDraw = DrawServiceMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}

void DrawServiceMain(){
        int xoffset = 0;
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        JpegDisp(rc_servicemenuico[0].x,rc_servicemenuico[0].y,menu_weixiuqiuzhu1,sizeof(menu_weixiuqiuzhu1));
        JpegDisp(rc_servicemenuico[1].x,rc_servicemenuico[1].y,menu_feiyongchaxun1,sizeof(menu_feiyongchaxun1));
        JpegDisp(rc_servicemenuico[2].x,rc_servicemenuico[2].y,service_lingshengxiazai1,sizeof(service_lingshengxiazai1));
        JpegDisp(rc_servicemenuico[3].x,rc_servicemenuico[3].y,menu_bianminfuwu1,sizeof(menu_bianminfuwu1));
        JpegDisp(rc_servicemenuico[4].x,rc_servicemenuico[4].y,jiatingliuyan1,sizeof(jiatingliuyan1));
        JpegDisp(rc_servicemenuico[5].x,rc_servicemenuico[5].y,menu_sipservice1,sizeof(menu_sipservice1));
        JpegDisp(rc_servicemenuico[6].x,rc_servicemenuico[6].y,menu_naozhong1,sizeof(menu_naozhong1));
 #if ENTERTAINMENT_ENABLE      
        JpegDisp(rc_servicemenuico[7].x,rc_servicemenuico[7].y,meituyule_1,sizeof(meituyule_1));
        
        JpegDisp(rc_servicemenuico[8].x,rc_servicemenuico[8].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_servicemenuico[9].x,rc_servicemenuico[9].y,bt_guanping1,sizeof(bt_guanping1));
#else
		JpegDisp(rc_servicemenuico[7].x,rc_servicemenuico[7].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_servicemenuico[8].x,rc_servicemenuico[8].y,bt_guanping1,sizeof(bt_guanping1));
		
#endif
        DrawEnText(LT("维修"),rc_servicemenuico[0].w+xoffset,rc_servicemenuico[0].h,rc_servicemenuico[0].x,rc_servicemenuico[0].y);
        DrawEnText(LT("费用查询"),rc_servicemenuico[1].w+xoffset,rc_servicemenuico[1].h,rc_servicemenuico[1].x,rc_servicemenuico[1].y);
        DrawEnText(LT("铃声下载"),rc_servicemenuico[2].w+xoffset,rc_servicemenuico[2].h,rc_servicemenuico[2].x,rc_servicemenuico[2].y);
        DrawEnText(LT("便民服务"),rc_servicemenuico[3].w+xoffset,rc_servicemenuico[3].h,rc_servicemenuico[3].x,rc_servicemenuico[3].y);
        DrawEnText(LT("家庭留言"),rc_servicemenuico[4].w+xoffset,rc_servicemenuico[4].h,rc_servicemenuico[4].x,rc_servicemenuico[4].y);
        DrawEnText(LT("服务中心"),rc_servicemenuico[5].w+xoffset,rc_servicemenuico[5].h,rc_servicemenuico[5].x,rc_servicemenuico[5].y);
		DrawEnText(LT("闹钟"),rc_servicemenuico[6].w+xoffset,rc_servicemenuico[6].h,rc_servicemenuico[6].x,rc_servicemenuico[6].y);
#if ENTERTAINMENT_ENABLE		
		DrawEnText(LT("媒体娱乐"),rc_servicemenuico[7].w+xoffset,rc_servicemenuico[7].h,rc_servicemenuico[7].x,rc_servicemenuico[7].y);
#endif		
}

int  ServiceEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<10;index++){
                if(IsInside(rc_servicemenuico[index],xy)){
                        pos_x = rc_servicemenuico[index].x;
                        pos_y = rc_servicemenuico[index].y;
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
                  case 0://维修求助
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_weixiuqiuzhu2,sizeof(menu_weixiuqiuzhu2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_weixiuqiuzhu1,sizeof(menu_weixiuqiuzhu1));
#if TE_MAINTAIN
                                  CreateMaintainWin2();
#else
                                  CreateMaintainWin();
#endif
                                  //CreateRecordWin();
                                  //CreateTimerScene();
                          }
                          break;
                  case 1: //费用查询
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_feiyongchaxun2,sizeof(menu_feiyongchaxun2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_feiyongchaxun1,sizeof(menu_feiyongchaxun1));
                                  CreateFeeWin();
                          }
                          break;
                  case 2://铃声下载
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,service_lingshengxiazai2,sizeof(service_lingshengxiazai2));
                          }else{
                                  JpegDisp(pos_x,pos_y,service_lingshengxiazai1,sizeof(service_lingshengxiazai1));
                                  CreateRingDownWin();
                          }
                          break;
                  case 3://便民服务
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_bianminfuwu2,sizeof(menu_bianminfuwu2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_bianminfuwu1,sizeof(menu_bianminfuwu1));
                                  CreatePeopleInfoWin();
                          }
                          break;
                  case 4://家庭留言
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,jiatingliuyan2,sizeof(jiatingliuyan2));
                          }else{
                                  JpegDisp(pos_x,pos_y,jiatingliuyan1,sizeof(jiatingliuyan1));
                                  CreateRecordWin();
                          }
                          break;
                  case 5:// 服务中心
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_sipservice2,sizeof(menu_sipservice2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_sipservice1,sizeof(menu_sipservice1));
                                  CreateOursServices();//2015-1-19
                                 
                          }                        
                          break;
                  case 6://闹钟
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_naozhong2,sizeof(menu_naozhong2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_naozhong1,sizeof(menu_naozhong1));
                                 SetIsShortCutAlarmclock();
                                 CreateTimerScene();
                                 
                          }                  	
                  	break;   
#if ENTERTAINMENT_ENABLE
                  case 7: /*娱乐*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,meituyule_2,sizeof(meituyule_2));
                          }else{
                                  JpegDisp(pos_x,pos_y,meituyule_1,sizeof(meituyule_1));
				  				  CreateMutilmediaMain();
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
#else
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
				
#endif                         
                
                  default:
                          break;
                        }
                        break;
                }
        }
        return ret;
}

//////////////////////////////////////////////////////
// 维修

enum{
        ITEM_POWERMAINTAIN=0,                     /*电源维修*/
        ITEM_WATERMAINTAIN,                   /*水管维修*/
        ITEM_HOUSEMAINTAIN,                    /*厨卫维修*/
        ITEM_GASEMAINTAIN                      /*煤气维修*/

};
static int  m_CurrentRepairItem = ITEM_POWERMAINTAIN;
static int  m_MaxRepairItem = 0;

static const WRT_Rect rc_Maintainico[6]={
        {296,181,80,80},                //上箭头
        {650,181,80,80},                //下箭头
        {385,197,260,48},               //选择框
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {4,531,111,69}                 //关屏幕
};

#if TE_MAINTAIN

static const WRT_Rect rc_Maintainico2[6]={
/*
        {178,40,132,160},             //供点
        {488,40,132,160},             //供水
        {178,240,132,160},             //厨卫家居
        {488,240,132,160},             //供气
*/
        {64,145,132,160},             //供电
        {238,145,132,160},             //供水
        {412,145,132,160},             //厨卫家居
        {586,145,132,160},             //供气

        {900,531,111,69},              //返回
        {4,531,111,69}                 //关屏幕
};

static void DrawMaintainItemInfo2()
{
	int x1 = 0;
	int y1 = 0;
	if(m_MaxRepairItem > 0 && m_MaxRepairItem < 5){
		for(int i =0 ;i< m_MaxRepairItem;i++)
		{
			x1 = (rc_Maintainico2[i].w - GetTextWidth(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[i],0))/2;
			if(x1 < 0)
				x1 = 0;
			x1 += rc_Maintainico2[i].x;
			y1 = rc_Maintainico2[i].y +rc_Maintainico2[i].h;
			if(GetCurrentLanguage()== ENGLISH)
				DrawText(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[i],x1,y1,rgb_white);

		}

	}else{

		y1 = rc_Maintainico2[1].y + rc_Maintainico2[1].h ;
		x1 = (rc_Maintainico2[1].w - GetTextWidth(LT("供水系统故障"),0))/2;
		if(x1 < 0)
			x1 = 0;
		x1 += rc_Maintainico2[1].x;
		DrawText(LT("供水系统故障"),x1,y1,rgb_white);

		y1 = rc_Maintainico2[0].y  + rc_Maintainico2[0].h ;
		x1 = (rc_Maintainico2[0].w - GetTextWidth(LT("供电系统故障"),0))/2;
		if(x1 < 0)
			x1 = 0;
		x1 += rc_Maintainico2[0].x;
		DrawText(LT("供电系统故障"),x1,y1,rgb_white);


		y1 = rc_Maintainico2[3].y + rc_Maintainico2[3].h ;
		x1 = (rc_Maintainico2[3].w - GetTextWidth(LT("供气系统故障"),0))/2;
		if(x1 < 0)
			x1 = 0;
		x1 += rc_Maintainico2[3].x;
		DrawText(LT("供气系统故障"),x1,y1,rgb_white);

		y1 = rc_Maintainico2[2].y  + rc_Maintainico2[2].h ;
		x1 = (rc_Maintainico2[2].w - GetTextWidth(LT("厨卫家居故障"),0))/2;
		if(x1 < 0)
			x1 = 0;
		x1 += rc_Maintainico2[2].x;
		DrawText(LT("厨卫家居故障"),x1,y1,rgb_white);

	}
}

#endif

static void DrawMaintainItemInfo(){
        int x1=0;
        int y1 = 0;
        WRT_Rect rt;

        y1 = rc_Maintainico[2].y+rc_Maintainico[2].h-15;
        rt = rc_Maintainico[2];
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);

        if(m_MaxRepairItem > 0){
                x1 = (rc_Maintainico[2].w - GetTextWidth(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[m_CurrentRepairItem],0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_Maintainico[2].x;

	            DrawText(pSystemInfo->LocalSetting.publicroom.NameOfRepaire[m_CurrentRepairItem],x1,y1,rgb_black);
         
        }else{
                switch(m_CurrentRepairItem){
                
		            case ITEM_WATERMAINTAIN:
		                    x1 = (rc_Maintainico[2].w - GetTextWidth(LT("供水系统故障"),0))/2;
		                    if(x1 < 0)
		                            x1 = 0;
		                    x1 += rc_Maintainico[2].x;

		                	DrawText(LT("供水系统故障"),x1,y1,rgb_black);
			                 
		                    break;
		                    
		            case ITEM_POWERMAINTAIN:
		                    x1 = (rc_Maintainico[2].w - GetTextWidth(LT("供电系统故障"),0))/2;
		                    if(x1 < 0)
		                            x1 = 0;
		                    x1 += rc_Maintainico[2].x;

			                DrawText(LT("供电系统故障"),x1,y1,rgb_black);

		                    break;
		                    
		            case ITEM_GASEMAINTAIN:
		                    x1 = (rc_Maintainico[2].w - GetTextWidth(LT("供气系统故障"),0))/2;
		                    if(x1 < 0)
		                            x1 = 0;
		                    x1 += rc_Maintainico[2].x;
		                  
		                	DrawText(LT("供气系统故障"),x1,y1,rgb_black);
		              
		                    break;
		                    
		            case ITEM_HOUSEMAINTAIN:
		                    x1 = (rc_Maintainico[2].w - GetTextWidth(LT("厨卫家居故障"),0))/2;
		                    if(x1 < 0)
		                            x1 = 0;
		                    x1 += rc_Maintainico[2].x;
		                  
			            	DrawText(LT("厨卫家居故障"),x1,y1,rgb_black);
			             
		                    break;
		                    
		            default:
                   			break;
                }
        }
}

void CreateMaintainWin2()
{
#if TE_MAINTAIN	
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = MaintainEventHandler2;
        pWindow->NotifyReDraw = DrawMaintainMain2;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
#endif        
}


void CreateMaintainWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = MaintainEventHandler;
        pWindow->NotifyReDraw = DrawMaintainMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawMaintainMain2()
{
#if TE_MAINTAIN	
        int xoffset = 21;
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
		
        JpegDisp(rc_Maintainico2[0].x,rc_Maintainico2[0].y,gongdiansys_1,sizeof(gongdiansys_1));
        JpegDisp(rc_Maintainico2[1].x,rc_Maintainico2[1].y,gongshuisys_1,sizeof(gongshuisys_1));
        JpegDisp(rc_Maintainico2[2].x,rc_Maintainico2[2].y,chuweisys_1,sizeof(chuweisys_1));
        JpegDisp(rc_Maintainico2[3].x,rc_Maintainico2[3].y,gongqisys_1,sizeof(gongqisys_1));
  
        JpegDisp(rc_Maintainico2[4].x,rc_Maintainico2[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_Maintainico2[5].x,rc_Maintainico2[5].y,bt_guanping1,sizeof(bt_guanping1));

        DrawEnText(LT("供电系统故障"),rc_Maintainico2[0].w+xoffset,rc_Maintainico2[0].h,rc_Maintainico2[0].x,rc_Maintainico2[0].y);
        DrawEnText(LT("供水系统故障"),rc_Maintainico2[1].w+xoffset,rc_Maintainico2[1].h,rc_Maintainico2[1].x,rc_Maintainico2[1].y);
        DrawEnText(LT("厨卫家居故障"),rc_Maintainico2[2].w+xoffset,rc_Maintainico2[2].h,rc_Maintainico2[2].x,rc_Maintainico2[2].y);
        DrawEnText(LT("供气系统故障"),rc_Maintainico2[3].w+xoffset,rc_Maintainico2[3].h,rc_Maintainico2[3].x,rc_Maintainico2[3].y);
#endif        
}
void DrawMaintainMain(){
        WRT_Rect rt;
        
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        JpegDisp(rc_Maintainico[0].x,rc_Maintainico[0].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_Maintainico[1].x,rc_Maintainico[1].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));

        rt = rc_Maintainico[2];//文字框
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);

        JpegDisp(rc_Maintainico[3].x,rc_Maintainico[3].y,menu_ok1,sizeof(menu_ok1));
        JpegDisp(rc_Maintainico[4].x,rc_Maintainico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_Maintainico[5].x,rc_Maintainico[5].y,bt_guanping1,sizeof(bt_guanping1));
        DrawText(LT("项目"),rc_Maintainico[0].x - GetTextWidth(LT("项目"),0) - 10,rc_Maintainico[0].y+
                rc_Maintainico[0].h-30,rgb_white);

        if(pSystemInfo->LocalSetting.publicroom.NumOfRepaire > 0)
        {
                m_MaxRepairItem = pSystemInfo->LocalSetting.publicroom.NumOfRepaire;
                m_CurrentRepairItem = 0;
        }
        else
        {
                m_CurrentRepairItem = ITEM_POWERMAINTAIN;
        }
        DrawMaintainItemInfo();
}

int MaintainEventHandler2(int x,int y,int status)
{
#if TE_MAINTAIN	
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_Maintainico2[index],xy)){
                        pos_x = rc_Maintainico2[index].x;
                        pos_y = rc_Maintainico2[index].y;
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
                        case 0: //供电
                                if(status == 1)
                                {
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,gongdiansys_2,sizeof(gongdiansys_2));
                                }else{
                                        JpegDisp(pos_x,pos_y,gongdiansys_1,sizeof(gongdiansys_1));
                                        tmp_event[0] = ROOM_REPAIR;
                                        tmp_event[1] = ITEM_POWERMAINTAIN+1;
                                        tmp_event[2] = tmp_event[3] = 0;
                                        wrthost_send_cmd(tmp_event);
                                }
                                break;
                        case 1://供水
                                if(status == 1)
                                {
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,gongshuisys_2,sizeof(gongshuisys_2));
                                }else{
                                        JpegDisp(pos_x,pos_y,gongshuisys_1,sizeof(gongshuisys_1));
                                        tmp_event[0] = ROOM_REPAIR;
                                        tmp_event[1] = ITEM_WATERMAINTAIN+1;
                                        tmp_event[2] = tmp_event[3] = 0;
                                        wrthost_send_cmd(tmp_event);
                                }
                                break;
                        case 2://厨卫
                                if(status == 1)
                                {
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,chuweisys_2,sizeof(chuweisys_2));
                                }else{
                                        JpegDisp(pos_x,pos_y,chuweisys_1,sizeof(chuweisys_1));
                                        tmp_event[0] = ROOM_REPAIR;
                                        tmp_event[1] = ITEM_HOUSEMAINTAIN+1;
                                        tmp_event[2] = tmp_event[3] = 0;
                                        wrthost_send_cmd(tmp_event);
                                }
                                break;
                        case 3://供气
                                if(status == 1)
                                {
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,gongqisys_2,sizeof(gongqisys_2));
                                }else{
                                        JpegDisp(pos_x,pos_y,gongqisys_1,sizeof(gongqisys_1));
                                        tmp_event[0] = ROOM_REPAIR;
                                        tmp_event[1] = ITEM_GASEMAINTAIN+1;
                                        tmp_event[2] = tmp_event[3] = 0;
                                        wrthost_send_cmd(tmp_event);
                                }
                                break;
                        case 4: /*返回*/
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                        tmp_event[0]=CUSTOM_CMD_CANCEL;
                                        tmp_event[1] = ROOM_REPAIR;
                                        tmp_event[2] =tmp_event[3] =0;
                                        wrthost_send_cmd(tmp_event);
                                        CloseTopWindow();
                                }
                                break;
                        case 5:/*关屏*/
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                        tmp_event[0]=CUSTOM_CMD_CANCEL;
                                        tmp_event[1] = ROOM_REPAIR;
                                        tmp_event[2] =tmp_event[3] =0;
                                        wrthost_send_cmd(tmp_event);
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
#endif        
}

int  MaintainEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_Maintainico[index],xy)){
                        pos_x = rc_Maintainico[index].x;
                        pos_y = rc_Maintainico[index].y;
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
                case 0: /*上翻*/
                        if(status == 1){
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                                StartButtonVocie();
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                m_CurrentRepairItem--;
                                if(m_MaxRepairItem > 0){
                                        if(m_CurrentRepairItem < 0)
                                                m_CurrentRepairItem = m_MaxRepairItem-1; //从尾开始
                                }else{
                                        if(m_CurrentRepairItem <ITEM_POWERMAINTAIN)
                                                m_CurrentRepairItem = ITEM_GASEMAINTAIN;
                                }
                                DrawMaintainItemInfo();
                        }
                        break;
                case 1:/*下翻*/
                        if(status == 1){
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                                StartButtonVocie();
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));
                                m_CurrentRepairItem++;
                                if(m_MaxRepairItem > 0){
                                        if(m_CurrentRepairItem >= m_MaxRepairItem)
                                                m_CurrentRepairItem = 0; //从头开始
                                }
                                else{
                                        if(m_CurrentRepairItem > ITEM_GASEMAINTAIN)
                                                m_CurrentRepairItem = ITEM_POWERMAINTAIN;
                                }
                                DrawMaintainItemInfo();
                        }
                        break;
                case 3://确定
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                tmp_event[0] = ROOM_REPAIR;
                                tmp_event[1] = m_CurrentRepairItem+1;
                                tmp_event[2] = tmp_event[3] = 0;
                                wrthost_send_cmd(tmp_event);

                        }
                        break;
                case 4: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                tmp_event[0]=CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_REPAIR;
                                tmp_event[2] =tmp_event[3] =0;
                                wrthost_send_cmd(tmp_event);
                                CloseTopWindow();
                        }
                        break;
                case 5:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                tmp_event[0]=CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_REPAIR;
                                tmp_event[2] =tmp_event[3] =0;
                                wrthost_send_cmd(tmp_event);
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


//////////////////////////////////////////////////////////////////////////
//便民服务

static const WRT_Rect rc_docinfo[4]={
        {660,531,111,69},              /*上翻*/
        {780,531,111,69},               /* 下翻*/


        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};
static int m_cur_doc_page = 1;
static int m_doc_page = 1;
static char* pCurText = 0;

static void CloseDocWindow(){
        pCurText = 0;
        m_doc_page = 1;
        m_cur_doc_page = 1;
}

static void DrawDocInfo(){
        int txtlen;
        char* pDocInfoText =0;
        pDocInfoText = GetDocInfo(&txtlen);
        if(pDocInfoText == 0 && txtlen == 0){
                DrawText(LT("正在查询..."),200,80,rgb_white);
                return;
        }
        WRT_Rect curbox={0,0,889,530};
        FillRect(BACK_COLOR,curbox);
        int offset = 30;
        char text[100];
        int  i=0,j=0;
        int textlen = txtlen+1;
        char* pText = ( char*)pDocInfoText;
        if(pCurText == NULL)
                pCurText = pText;
        char* pTmp1 = NULL;
        char* pTmp2 = (char*)pText;
        int x =0;
        if(m_doc_page > 1){
                int page = 0;
                while(1){
                        if(x>12){
                                x = 0;
                                page++;
                        }
                        if(page  == (m_cur_doc_page-1)){
                                pCurText = pTmp1;
                                if(pCurText == NULL)
                                        pCurText = pText;
                                break;
                        }

                        if((pTmp1 = strstr((char*)pTmp2,"\r\n")) != NULL){
                                x++;
                                pTmp1 +=2;
                                pTmp2 = pTmp1;
                        }else{
                                /* if(pTmp2){
                                x++;
                                if(x > 12){
                                x = 0;
                                page++;
                                }
                                }*/
                                break;
                        }
                }

        }else{
                while(1){
                        if(x>12){
                                x = 0;
                                m_doc_page++;
                        }
                        if((pTmp1 = strstr((char*)pTmp2,"\r\n")) != NULL){
                                x++;
                                pTmp1 +=2;
                                pTmp2 = pTmp1;
                        }else{
                                /*if(pTmp2){
                                x++;
                                if(x > 12){
                                x = 0;
                                m_doc_page++;
                                }
                                }*/
                                break;
                        }

                }
        }
        pTmp1 = NULL;
        x = 0;
        pTmp2 =  (char*)pCurText;
        while(1){
                if(x > 12){
                        pCurText = pTmp1;
                        break;
                }
                if((pTmp1 = strstr((char*)pTmp2,"\r\n")) != NULL){
                        j = (pTmp1 - pTmp2);
                        memcpy(text,pTmp2,j );
                        text[j] = '\0';
                        DrawText_16(text,20,x*offset+50,rgb_white);  //可以修改成自己换行
                        x++;
                        pTmp1 +=2;
                        pTmp2 = pTmp1;
                }else{
                        if(pTmp2){
                                DrawText_16(pTmp2,20,x*offset+50,rgb_white);
                                x++;
                        }
                        break;
                }
        }

        DrawStatusbar(m_doc_page,m_cur_doc_page);
}
void CreatePeopleInfoWin(){
        unsigned long tmp_event[4];
        FreeDocInfo();
        tmp_event[0] = ROOM_INQSERVICEINFO;
        tmp_event[1] = tmp_event[2] = tmp_event[3] = 0;
        
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = PeopleInfoEventHandler;
        pWindow->NotifyReDraw = DrawPeopleInfoMain;
        pWindow->CloseWindow = CloseDocWindow;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        m_cur_doc_page = 1;
        m_doc_page = 1;
        pWindow->NotifyReDraw2(pWindow);
        wrthost_send_cmd(tmp_event);

}
void DrawPeopleInfoMain(){
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        JpegDisp(rc_docinfo[2].x,rc_docinfo[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_docinfo[3].x,rc_docinfo[3].y,bt_guanping1,sizeof(bt_guanping1));
        DrawDocInfo();
        if( m_doc_page > 1){
                JpegDisp(rc_docinfo[0].x,rc_docinfo[0].y,menu_shangfan1,sizeof(menu_shangfan1));
                JpegDisp(rc_docinfo[1].x,rc_docinfo[1].y,menu_xiafan1,sizeof(menu_xiafan1));
        }

}
int  PeopleInfoEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rc_docinfo[index],xy)){
                        pos_x = rc_docinfo[index].x;
                        pos_y = rc_docinfo[index].y;
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
                case 0:
                        if(m_doc_page < 2){
                                break;
                        }
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                m_cur_doc_page--;
                                if(m_cur_doc_page < 1)
                                        m_cur_doc_page = 1;
                                DrawDocInfo();
                        }
                        break;
                case 1:
                        if(m_doc_page < 2){
                                break;
                        }
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
                                m_cur_doc_page++;
                                if(m_cur_doc_page > m_doc_page)
                                        m_cur_doc_page = m_doc_page;
                                DrawDocInfo();
                        }
                        break;
                case 2: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                                tmp_event[0] = CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_INQSERVICEINFO;
                                tmp_event[2] = tmp_event[3] = 0;
                                wrthost_send_cmd(tmp_event);
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
                                tmp_event[0] = CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_INQSERVICEINFO;
                                tmp_event[2] = tmp_event[3] = 0;
                                wrthost_send_cmd(tmp_event);
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

////////////////////////////////////////////////////////
//                                                    //
//定时情景处理2009-7-21                              //
////////////////////////////////////////////////////////
static WRT_Rect rcsceneTimer[13]={
        {70,50,80,80},   //上箭头
        {440,50,80,80},   //下箭头
        {154,66,280,48},   //选择框
        {154,130,100,40},
        {330,130,100,40},
        {240,200,140,40},
        {390,200,140,40},
        {90,200,140,40},//闹钟框
        {0,0,0,0},
        {0,0,0,0},
        {650,100,300,400},    //数字符号所在区域

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};
typedef struct _tagsceneTimer
{
        int  hour;
        int  min;
        int  scene;
        int  valid;
}SCENETIMER;



#define  SCENETIMER_START        0x01
#define  ONE_SCENE               0x02
#define  TWO_SCENE               0x04
#define  THREE_SCENE             0x08
#define  FOUR_SCENE              0x10
#define  FIVE_SCENE              0x20
#define  SIX_SCENE               0x40
#define  ALARM_CLOCK             0x80
#define  CLOCKTIMER_START        0x100

extern char g_number[12];

static  char g_scenename[11][12] = {"会客模式","就餐模式","影院模式","就寝模式","温馨模式","在家模式","离家模式","夜起模式","晨起模式","自定义模式","闹钟"};
static SCENETIMER  g_scenetimer[6]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
static SCENETIMER  g_clocktimer[6]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
//static unsigned long g_scenetimertid=0xffffffff;
static SDL_TimerID g_sceneTimerid =0;
static SDL_mutex* g_scenetimerlock=NULL;
static SDL_TimerID g_clockTimerid = 0;

static   unsigned char g_is_only_alarmclock=0;
static  int  g_curselectscene = 0;
static char g_input[2][5];
static int  g_count =0;
static int g_curselectedit = 0;
static int g_isrun = 0;
static int g_isrun2 = 0;
static unsigned long* pIPNumberPointer=NULL;
static unsigned long* pIPNumberPointer1=NULL;
static unsigned long* pIPNumberSize=NULL;
static unsigned long* pIPNumberSize1=NULL;

static char* scenetimerpath = "/home/wrt/scenetimer.txt";
static char* clocktimerpath = "/home/wrt/clocktimer.txt";

void delete_scene_and_clock()
{
	system("rm /home/wrt/scenetimer.txt");
	system("rm /home/wrt/clocktimer.txt");
       // _unlink(scenetimerpath);
       // _unlink(clocktimerpath);
}
static bool save_scenetimer_table_to_flash(){
#ifdef TEST_FFS
        if(pSystemInfo->mySysInfo.isusedffs == 1)	{
                FILE* file= NULL;
                int size1 = sizeof(g_scenetimer);
                file = fopen(scenetimerpath,"wb");
                if(file != NULL){
                        long write_size = fwrite((void*)g_scenetimer,1,size1,file);
                        if(write_size == size1){
                                fclose(file);
                                system("sync");
                                return true;
                        }
                        WRT_DEBUG("errno = %d  write_size=%d ",errno,write_size);
                        fclose(file);
                        unlink(scenetimerpath);
                }
                WRT_DEBUG("save scenetimerpath = %s,errno = %d ",scenetimerpath,errno);
        }
#endif
        return false;
}
static bool save_clocktimer_table_to_flash(){
#ifdef TEST_FFS
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE* file=NULL;
                int size1 = sizeof(g_clocktimer);
                file = fopen(clocktimerpath,"wb");
                if(file != NULL){
                        long write_size = fwrite((void*)g_clocktimer,1,size1,file);
                        if(write_size == size1){
                                fclose(file);
                                system("sync");
                                return true;
                        }
                        WRT_DEBUG("errno = %d  write_size=%d ",errno,write_size);
                        fclose(file);
                        unlink(clocktimerpath);
                }
                WRT_DEBUG("save scenetimerpath = %s,errno = %d ",clocktimerpath,errno);
        }
#endif
        return false;
}

static void printftime(unsigned long dates,unsigned long times){
        unsigned long day,Month,years;
        unsigned long min,second,Hour;
        char timename[50];
        day= dates&0xff;
        Month = (dates >> 8) &0xff;
        years = (dates>>16) & 0xffff;
        Hour = (times >> 16)& 0XFFFF;
        second = (times & 0xff);
        min = (times >> 8) &0xff;
        sprintf(timename,"%04d/%02d/%02d--%02d:%02d:%02d\n",years,Month,day,Hour,min,second);
        WRT_DEBUG(timename);
}

static Uint32 scene_start(Uint32 Interval,void* pvoid){
	int i = 0;
	time_t _t;
	struct tm* ptm; 
	time(&_t);
	ptm = localtime(&_t);

	
	EnterSceneTimerLock();
	for(i =0; i<6;i++){
	        if(g_scenetimer[i].valid == 1 &&
	        	g_scenetimer[i].hour == ptm->tm_hour &&
	        	g_scenetimer[i].min == ptm->tm_min
	        	)
	        {
				if(g_cur_set_scene_mode != g_scenetimer[i].scene)
				{
					//g_cur_set_scene_mode = g_scenetimer[i].scene;
					//printf("???ˉ i %d %d %s \n",g_scenetimer[i].scene,i,g_scenename[g_scenetimer[i].scene]);
					int rc = send_scene_mode(g_scenetimer[i].scene);
	                                 
	                if(rc == 0){
#if HAVE_JIAJU_SAFE                       
					if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)            	
	                        	start_defend(true); //???ˉ·à??2?·à    2009-7-21  									 
#endif                                        
	                        ;//DrawSceneStatus(LT("????3é1|"));
	                }else
	                        ;
                }
                         ReDrawCurrentWindow2(2); //刷新主界面。刷新防区等。     
	        }
	}
	ExitSceneTimerLock();	
	
	return Interval;
}

static Uint32 clock_start(Uint32 Interval,void* pvoid){
	int i = 0;
	time_t _t;
	struct tm* ptm; 
	time(&_t);
	ptm = localtime(&_t);
	
	//printf("闹钟\n");
	EnterSceneTimerLock();
	for(i =0; i<6;i++){
	        if(g_clocktimer[i].valid == 1 &&
	        	g_clocktimer[i].hour == ptm->tm_hour &&
	        	g_clocktimer[i].min == ptm->tm_min &&
	        	g_clocktimer[i].scene == 10
	        	)
	        {
	         	 process_alarm_clock();   
	        }
	}
	ExitSceneTimerLock();	
	
	return Interval;
}

static void scene_timer_start()
{
	EnterSceneTimerLock();
 	int i =0;
	int isok = 0;
	for(i =0; i<6;i++){
	        if(g_scenetimer[i].valid == 1){
	                isok  = 1;
	                break;
	        }
	}
	ExitSceneTimerLock();	
	if(isok == 1){
		g_sceneTimerid = SDL_AddTimer(30000,scene_start,NULL);
	        g_isrun = 1;
	        ReDrawCurrentWindow();
	}
	WRT_MESSAGE("SCENETIMER_START OK ");
}

static void clock_timer_start()
{
	int i =0;
	int isok = 0;
	EnterSceneTimerLock();
	for(i =0; i<6;i++){
	        if(g_clocktimer[i].valid == 1){
	                if(g_clocktimer[i].scene == 10){
	                	 isok  = 1;
	                	 break;
	                }
	        }
	}
	ExitSceneTimerLock();
	
	if(isok == 1){
		g_clockTimerid = SDL_AddTimer(60000,clock_start,NULL);
	        g_isrun2 = 1;
	        ReDrawCurrentWindow();
	}
	WRT_MESSAGE("CLOCKTIMER_START OK");	
}


void InitsceneTimer(){
        g_scenetimerlock = SDL_CreateMutex();
       
       // SDL_CreateThread(sceneTimerTask,NULL);

        //从flash中获得具体的家居配置表
#ifdef TEST_FFS
        if(pSystemInfo->mySysInfo.isusedffs == 1)	{
                FILE* file=NULL;
                int size1 = sizeof(g_scenetimer);
                file = fopen(scenetimerpath,"rb");
                if(file != NULL){
                        long size = fread((void*)g_scenetimer,1,size1,file);
                        if(size == size1){
                                fclose(file);
                                file = NULL;
                              //  return;
                        }
                }
                if(file != NULL){
                	fclose(file);
                	file = NULL;
        	}
                WRT_DEBUG("scenetimerpath = %s,errno = %d ",scenetimerpath,errno);
        }
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE* file= NULL;
                int size1 = sizeof(g_clocktimer);
                file = fopen(clocktimerpath,"rb");
                if(file != NULL){
                        long size = fread((void*)g_clocktimer,1,size1,file);
                        if(size == size1){
                                fclose(file);
                                file = NULL;
                                return;
                        }
                }
                if(file != NULL){
                	fclose(file);
                	file = NULL;
        	}
                WRT_DEBUG("scenetimerpath = %s,errno = %d ",clocktimerpath,errno);
        }                
#endif
}

void SetIsShortCutAlarmclock()
{
	g_is_only_alarmclock = 1;
}
int StartClockTimer()
{
        int i=0;
        uint32_t dates,times;
        int year;
        tm_get(&dates,&times,0);
        year = ((dates  >>  16) & 0xffff);
        if(year == 0){
 //               WRT_DEBUG("未设置日期时间");
                return 2;
        }
        EnterSceneTimerLock();
        for(i =0; i<6;i++)
                if(g_clocktimer[i].valid ==1)
                        break;
        if(i == 6){
                ExitSceneTimerLock();
                return 1;
        }
        ExitSceneTimerLock();
        
        clock_timer_start();

        return 0;	
}
void EnterSceneTimerLock(){
        if(g_scenetimerlock != NULL)
                SDL_LockMutex(g_scenetimerlock);
}
void ExitSceneTimerLock(){
        if(g_scenetimerlock != NULL)
                SDL_UnlockMutex(g_scenetimerlock);
}

int StartSceneTimer(){
        int i=0;
        uint32_t dates,times;
        int year;
        tm_get(&dates,&times,0);
        year = ((dates  >>  16) & 0xffff);
        if(year == 0){
         //       WRT_DEBUG("未设置日期时间");
                return 2;
        }
        EnterSceneTimerLock();
        for(i =0; i<6;i++)
                if(g_scenetimer[i].valid ==1)
                        break;
        if(i == 6){
                ExitSceneTimerLock();
                return 1;
        }
        ExitSceneTimerLock();
        
      	scene_timer_start();
        return 0;
}

int add_clockTimer(int hour,int min,int scene)
{
        int i=0;
        int ret =0;
        EnterSceneTimerLock();
        for(i =0;i<6;i++){
                if(g_clocktimer[i].valid == 0){
                        g_clocktimer[i].valid = 1;
                        g_clocktimer[i].hour = hour;
                        g_clocktimer[i].min = min;
                        g_clocktimer[i].scene = scene;
                        ret = 1;
                        break;
                }
        }
        ExitSceneTimerLock();
        return ret;	
}


void del_clockTimer()
{
        EnterSceneTimerLock();
        memset(g_clocktimer,0,sizeof(g_clocktimer));
        ExitSceneTimerLock();	
}

int add_sceneTimer(int hour,int min,int scene){

        int i=0;
        int ret =0;
        EnterSceneTimerLock();
        for(i =0;i<6;i++){
                if(g_scenetimer[i].valid == 0){
                        g_scenetimer[i].valid = 1;
                        g_scenetimer[i].hour = hour;
                        g_scenetimer[i].min = min;
                        g_scenetimer[i].scene = scene;
                        ret = 1;
                        break;
                }
        }
        ExitSceneTimerLock();
        return ret;
}

void del_sceneTimer(){
        EnterSceneTimerLock();
        memset(g_scenetimer,0,sizeof(g_scenetimer));
        ExitSceneTimerLock();
}
static void updateviewsceneTimer(int isupdate){
        WRT_Rect rctmp;
        rctmp.x = 0;
        rctmp.y = 250;
        rctmp.w = 500;
        rctmp.h = 250;
        if(isupdate ==1)
        	g_isUpdated = 0;
        FillRect(BACK_COLOR,rctmp);
        char tmpbuf[50];
        int i;
        int offset = 30;
        for(i=0; i<6;i++){
                if(g_scenetimer[i].valid == 1){
                        if(GetCurrentLanguage() == ENGLISH)
                        {
                                sprintf(tmpbuf,"%d:'%s' from %d:%02d",i+1,LT(g_scenename[g_scenetimer[i].scene]),g_scenetimer[i].hour,g_scenetimer[i].min);
                        }else{
                                sprintf(tmpbuf,"%d: 每天%d时%d分执行'%s'",i+1,g_scenetimer[i].hour,g_scenetimer[i].min,LT(g_scenename[g_scenetimer[i].scene]));
                        }
                        DrawText_16(tmpbuf,10,265+offset*i,rgb_white);
                }
        }
        if(isupdate == 1){
        	update_rect(rctmp.x,rctmp.y,rctmp.w,rctmp.h);
        	g_isUpdated = 1;
        }

}
static void updateviewclockTimer(){
        WRT_Rect rctmp;
        rctmp.x = 0;
        rctmp.y = 250;
        rctmp.w = 500;
        rctmp.h = 250;
        
        FillSingleRect(0xb2136,rctmp);
        char tmpbuf[50];
        int i;
        int offset = 30;
        for(i=0; i<6;i++){
                if(g_clocktimer[i].valid == 1){
                        if(GetCurrentLanguage() == ENGLISH)
                        {
                                sprintf(tmpbuf,"%d:'%s' from %d:%02d",i+1,LT(g_scenename[g_clocktimer[i].scene]),g_clocktimer[i].hour,g_clocktimer[i].min);
                        }else{
                                sprintf(tmpbuf,"%d: 每天%d时%d分执行'%s'",i+1,g_clocktimer[i].hour,g_clocktimer[i].min,LT(g_scenename[g_clocktimer[i].scene]));
                        }
                        //DrawText_16(tmpbuf,10,265+offset*i,rgb_white);
                        DrawText(tmpbuf,10,265+offset*i,rgb_white,16);
                }
        }
}

static void updateButton1(int index,int isupdate){
        int xoffset =0;
        tDraw3dInset(rcsceneTimer[index].x,rcsceneTimer[index].y,rcsceneTimer[index].w,rcsceneTimer[index].h);
        if(isupdate == 1)
        	update_rect(rcsceneTimer[index].x,rcsceneTimer[index].y,rcsceneTimer[index].w,rcsceneTimer[index].h);
        Color_FillRect(rcsceneTimer[index],255,0,0);
        if(index == 5){
                xoffset = (rcsceneTimer[5].w - GetTextWidth(LT("全部取消"),1))/2;
                if(xoffset < 0)
                        xoffset +=8;
                xoffset += rcsceneTimer[5].x;
                DrawText_16(LT("全部取消"),xoffset,rcsceneTimer[5].y+rcsceneTimer[5].h-10,rgb_black);
        }else if(index == 6){
                xoffset = (rcsceneTimer[6].w - GetTextWidth(LT("增加"),1))/2;
                if(xoffset < 0)
                        xoffset +=10;
                xoffset += rcsceneTimer[6].x;
                DrawText_16(LT("增加"),xoffset,rcsceneTimer[6].y+rcsceneTimer[6].h-10,rgb_black);
        }else if(index == 7){
        	int tmprun = g_isrun;
        	if(g_is_only_alarmclock == 0x01)
        		tmprun = g_isrun2;        		        	
                if(tmprun == 0){
                        xoffset = (rcsceneTimer[7].w - GetTextWidth(LT("执行"),1))/2;
                        if(xoffset < 0)
                                xoffset +=10;
                        xoffset += rcsceneTimer[7].x;
                        DrawText_16(LT("执行"),xoffset,rcsceneTimer[7].y+rcsceneTimer[7].h-10,rgb_black);
                }else{
                        xoffset = (rcsceneTimer[7].w - GetTextWidth(LT("停止"),1))/2;
                        if(xoffset < 0)
                                xoffset +=10;
                        xoffset += rcsceneTimer[7].x;
                        DrawText_16(LT("停止"),xoffset,rcsceneTimer[7].y+rcsceneTimer[7].h-10,rgb_black);
                }
        }
}

static void updateButton2(int index,int isupdate){
        int xoffset =0;
        tDraw3dOutset(rcsceneTimer[index].x,rcsceneTimer[index].y,rcsceneTimer[index].w,rcsceneTimer[index].h);
        if(isupdate == 1)
        	update_rect(rcsceneTimer[index].x,rcsceneTimer[index].y,rcsceneTimer[index].w,rcsceneTimer[index].h);
        Color_FillRect(rcsceneTimer[index],0,0,255);
        if(index == 5){
                xoffset = (rcsceneTimer[5].w - GetTextWidth(LT("全部取消"),1))/2;
                if(xoffset < 0)
                        xoffset +=8;
                xoffset += rcsceneTimer[5].x;
                DrawText_16(LT("全部取消"),xoffset,rcsceneTimer[5].y+rcsceneTimer[5].h-10,rgb_black);
        }else if(index == 6){
                xoffset = (rcsceneTimer[6].w - GetTextWidth(LT("增加"),1))/2;
                if(xoffset < 0)
                        xoffset +=10;
                xoffset += rcsceneTimer[6].x;
                DrawText_16(LT("增加"),xoffset,rcsceneTimer[6].y+rcsceneTimer[6].h-10,rgb_black);
        } else if(index == 7){
        	int tmprun = g_isrun;
        	if(g_is_only_alarmclock == 0x01)
        		tmprun = g_isrun2;        		        	
                if(tmprun == 0){
                        xoffset = (rcsceneTimer[7].w - GetTextWidth(LT("执行"),1))/2;
                        if(xoffset < 0)
                                xoffset +=10;
                        xoffset += rcsceneTimer[7].x;
                        DrawText_16(LT("执行"),xoffset,rcsceneTimer[7].y+rcsceneTimer[7].h-10,rgb_black);
                }else{
                        xoffset = (rcsceneTimer[7].w - GetTextWidth(LT("停止"),1))/2;
                        if(xoffset < 0)
                                xoffset +=10;
                        xoffset += rcsceneTimer[7].x;
                        DrawText_16(LT("停止"),xoffset,rcsceneTimer[7].y+rcsceneTimer[7].h-10,rgb_black);
                }
        }
}
static void updateEdit1(int index,int isupdate){ //修改
        WRT_Rect tmprc;
        if(isupdate == 1){
        	g_isUpdated = 0;
        }
        if(isupdate == 1){
       	        update_rect(rcsceneTimer[index+3].x,rcsceneTimer[index+3].y,rcsceneTimer[index+3].w,rcsceneTimer[index+3].h);
        	g_isUpdated = 1;
        }        
        tmprc = rcsceneTimer[index+3];
        DeflateRect(&tmprc,4);
        FillRect(rgb_white,tmprc);
        DrawRect(rcsceneTimer[index+3],2,rgb_yellow);
        DrawText(g_input[index],rcsceneTimer[index+3].x+5,rcsceneTimer[index+3].y+rcsceneTimer[index+3].h-10,rgb_black,16);

        FillSingleRect(rgb_white,tmprc);
        DrawSingleRect(rcsceneTimer[index+3],2,rgb_yellow);
        DrawText(g_input[index],rcsceneTimer[index+3].x+5,rcsceneTimer[index+3].y+rcsceneTimer[index+3].h-10,rgb_black,16);

}
static void updateEdit2(int index,int isupdate){//0 0
        WRT_Rect tmprc;
        if(isupdate == 1){
        	g_isUpdated = 0;
        }
        DrawRect(rcsceneTimer[index+3],2,rgb_white);
        if(isupdate == 1){
       	        update_rect(rcsceneTimer[index+3].x,rcsceneTimer[index+3].y,rcsceneTimer[index+3].w,rcsceneTimer[index+3].h);
        	g_isUpdated = 1;
        }         
        tmprc = rcsceneTimer[index+3];
        DeflateRect(&tmprc,4);
        FillRect(rgb_white,tmprc);
        DrawText_16(g_input[index],rcsceneTimer[index+3].x+5,rcsceneTimer[index+3].y+rcsceneTimer[index+3].h-10,rgb_black);

        FillSingleRect(rgb_white,tmprc);
        DrawText(g_input[index],rcsceneTimer[index+3].x+5,rcsceneTimer[index+3].y+rcsceneTimer[index+3].h-10,rgb_black,16);

}
static void updateArrow(){
        WRT_Rect tmprc;
        int xoffset = 0;
        g_isUpdated == 0;
        
        tmprc = rcsceneTimer[2];
        DeflateRect(&tmprc,4);
        FillRect(rgb_white,tmprc);
        xoffset = (rcsceneTimer[2].w - GetTextWidth(LT(g_scenename[g_curselectscene]),1))/2;
        if(xoffset < 0)
                xoffset = 2;
        xoffset += rcsceneTimer[2].x;
        DrawText_16(LT(g_scenename[g_curselectscene]),xoffset,rcsceneTimer[2].y+rcsceneTimer[2].h-20,rgb_black);
        update_rect(rcsceneTimer[2].x,rcsceneTimer[2].y,rcsceneTimer[2].w,rcsceneTimer[2].h);
        g_isUpdated = 1;
}

void DrawTimerScene(){
        WRT_Rect tmprc;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
	
        JpegDisp(rcsceneTimer[11].x,rcsceneTimer[11].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcsceneTimer[12].x,rcsceneTimer[12].y,bt_guanping1,sizeof(bt_guanping1));
     
        int j =0;
        int i =0;
        int offset = 30;
        int xoffset = 0;
        int startx = rcsceneTimer[10].x;
        int starty = rcsceneTimer[10].y;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pIPNumberPointer[(i*3)+j],pIPNumberSize[(i*3)+j]);
                }
                    WRT_MESSAGE("1--2");
                if(g_is_only_alarmclock != 0x01){
                	DrawText_16(LT("任务计划，每天最多可选择6个情景执行，输入的时间为24小时制"),10,30,rgb_yellow);
                	JpegDisp(rcsceneTimer[0].x,rcsceneTimer[0].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                	JpegDisp(rcsceneTimer[1].x,rcsceneTimer[1].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));
        	}
        	WRT_MESSAGE("1--3");
        	DrawRect(rcsceneTimer[2],2,rgb_white);
        	DrawRect(rcsceneTimer[3],2,rgb_white);
        	DrawRect(rcsceneTimer[4],2,rgb_white);

                WRT_MESSAGE("1--4");
                tmprc = rcsceneTimer[2];
                DeflateRect(&tmprc,4);
                FillRect(rgb_white,tmprc);

                tmprc = rcsceneTimer[3];
                DeflateRect(&tmprc,4);
                FillRect(rgb_white,tmprc);

                tmprc = rcsceneTimer[4];
                DeflateRect(&tmprc,4);
                FillRect(rgb_white,tmprc);
		 WRT_MESSAGE("2--\n");
                xoffset = (rcsceneTimer[2].w - GetTextWidth(LT(g_scenename[g_curselectscene]),1))/2;
                if(xoffset < 0)
                        xoffset = 2;
                xoffset += rcsceneTimer[2].x;
                DrawText_16(LT(g_scenename[g_curselectscene]),xoffset,rcsceneTimer[2].y+rcsceneTimer[2].h-20,rgb_black);
                DrawText_16(LT("时"),rcsceneTimer[3].x+10+rcsceneTimer[3].w, rcsceneTimer[3].y+rcsceneTimer[3].h-10,rgb_white);
                DrawText_16(LT("分"),rcsceneTimer[4].x +10+rcsceneTimer[4].w, rcsceneTimer[4].y+rcsceneTimer[4].h-10,rgb_white);

                tDraw3dOutset(rcsceneTimer[5].x,rcsceneTimer[5].y,rcsceneTimer[5].w,rcsceneTimer[5].h);
                tDraw3dOutset(rcsceneTimer[6].x,rcsceneTimer[6].y,rcsceneTimer[6].w,rcsceneTimer[6].h);
                tDraw3dOutset(rcsceneTimer[7].x,rcsceneTimer[7].y,rcsceneTimer[7].w,rcsceneTimer[7].h);
                Color_FillRect(rcsceneTimer[5],0,0,255);
                Color_FillRect(rcsceneTimer[6],0,0,255);
                Color_FillRect(rcsceneTimer[7],0,0,255);
                xoffset = (rcsceneTimer[5].w - GetTextWidth(LT("全部取消"),1))/2;
                if(xoffset < 0)
                        xoffset = 8;
                xoffset += rcsceneTimer[5].x;
                DrawText_16(LT("全部取消"),xoffset,rcsceneTimer[5].y+rcsceneTimer[5].h-10,rgb_black);
                xoffset = (rcsceneTimer[6].w - GetTextWidth(LT("增加"),1))/2;
                if(xoffset < 0)
                        xoffset = 10;
                xoffset += rcsceneTimer[6].x;
                DrawText_16(LT("增加"),xoffset,rcsceneTimer[6].y+rcsceneTimer[6].h-10,rgb_black);
                xoffset = (rcsceneTimer[7].w - GetTextWidth(LT("执行"),1))/2;
                if(xoffset < 0)
                        xoffset = 10;
                xoffset += rcsceneTimer[7].x;
                DrawText_16(LT("执行"),xoffset,rcsceneTimer[7].y+rcsceneTimer[7].h-10,rgb_black);
                 WRT_MESSAGE("3--\n");
                updateEdit1(0,0);
                updateEdit2(1,0);
                if(g_is_only_alarmclock == 0x01)
                	updateviewclockTimer();
                else
                	updateviewsceneTimer(0);
                updateButton2(7,0);
                 WRT_MESSAGE("4--\n");
                


}

static int SceneNumberhandle(int x,int y,int status){
        int i = 0;
        int j = 0;
        int ret = 0;
        int tempx  = 0;
        int tempy  = 0;
        int count  = 0;
        int startx = rcsceneTimer[10].x;
        int starty = rcsceneTimer[10].y;
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0; i<4;i++)
                for(j=0; j< 3;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){
                                WRT_DEBUG("count = %d ",count);
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
                                        if(count == 9){
                                                g_count--;
                                                if(g_count < 0)
                                                        g_count = 0;
                                                g_input[g_curselectedit][g_count] = '\0';
                                                updateEdit1(g_curselectedit,1);
                                                break;
                                        }
                                        if(count == 11){//#
                                        	break;
                                        }
                                        if(g_count > 1)
                                                break;
                                        c[0] = g_number[count];
                                        strcat(g_input[g_curselectedit],c);
                                        g_count++;
                                        updateEdit1(g_curselectedit,1);
                                        if(g_count == 2){
                                                int itmp = 0;
                                                itmp = atoi(g_input[g_curselectedit]);
                                                if(g_curselectedit == 0 && itmp > 23){
                                                        CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);
                                                        g_input[g_curselectedit][0] = '\0';
                                                        g_count = 0;
                                                }
                                                if(g_curselectedit == 1 && itmp > 59){
                                                        CreateDialogWin2(LT("输入错误，请重新输入!"),MB_OK,NULL,NULL);
                                                        g_input[g_curselectedit][0] = '\0';
                                                        g_count = 0;
                                                }
                                        }

                                        break;
                                }
                        }
                }
                return ret;
}
int TimerSceneHandle(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<13;index++){
                if(IsInside(rcsceneTimer[index],xy)){
                        pos_x = rcsceneTimer[index].x;
                        pos_y = rcsceneTimer[index].y;
                        ret = 1;
                        switch(index){
                case 0:
                	if(g_is_only_alarmclock == 0x1)
                		break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                g_curselectscene--;
                                if(g_curselectscene < SCENE_HUIKE)
                                        g_curselectscene = SCENE_USERDEFINE;
                                updateArrow();
                        }
                        break;
                case 1:
                	if(g_is_only_alarmclock == 0x1)
                		break;                	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));
                                g_curselectscene++;
                                if(g_curselectscene > SCENE_USERDEFINE)
                                        g_curselectscene = SCENE_HUIKE;
                                updateArrow();
                        }
                        break;
                case 3://shi
                        if(status == 1){
                                StartButtonVocie();
                        }else{
                                g_curselectedit =0;
                                g_count= strlen(g_input[g_curselectedit]);
                                updateEdit1(0,1);
                                updateEdit2(1,1);
                        }
                        break;
                case 4://fen
                        if(status == 1){
                                StartButtonVocie();
                        }else{
                                g_curselectedit = 1;
                                g_count= strlen(g_input[g_curselectedit]);
                                updateEdit1(1,1);
                                updateEdit2(0,1);
                        }
                        break;
                case 5: //取消
                        if(status == 1){
                                StartButtonVocie();
                                g_isUpdated = 0;
                                updateButton1(5,1);
                                update_rect(rcsceneTimer[5].x,rcsceneTimer[5].y,rcsceneTimer[5].w,rcsceneTimer[5].h);
                                g_isUpdated = 1;
                        }else{
                        		g_isUpdated = 0;
                                updateButton2(5,1);
								update_rect(rcsceneTimer[5].x,rcsceneTimer[5].y,rcsceneTimer[5].w,rcsceneTimer[5].h);
                                g_isUpdated = 1;
                                if(g_is_only_alarmclock == 0x1){
                                	del_clockTimer();
                                	updateviewclockTimer();
                                	save_clocktimer_table_to_flash();
                                }else{
                                	del_sceneTimer();
                                	updateviewsceneTimer(1);
                                	save_scenetimer_table_to_flash(); //保存到磁盘
                                }
                        }
                        break;
                case 6://增加
                        if(status == 1){
                                StartButtonVocie();
                                g_isUpdated = 0;
                                updateButton1(6,1);
                                update_rect(rcsceneTimer[6].x,rcsceneTimer[6].y,rcsceneTimer[6].w,rcsceneTimer[6].h);
                                g_isUpdated = 1;
                        }else{ 
                                g_isUpdated = 0; 
                                updateButton2(6,1);
                                update_rect(rcsceneTimer[6].x,rcsceneTimer[6].y,rcsceneTimer[6].w,rcsceneTimer[6].h);
                                g_isUpdated = 1;
                                int hour,min;
                                hour = min = 0;
                                hour = atoi(g_input[0]);
                                min = atoi(g_input[1]);
                               
                               if(g_is_only_alarmclock == 0x01){ //如果仅仅是闹钟
                                	add_clockTimer(hour,min,g_curselectscene);
                                	updateviewclockTimer();//需要修改下闹钟框
                                	if(g_clockTimerid != 0)
                                		SDL_RemoveTimer(g_clockTimerid);
                                	g_clockTimerid = 0;
                                	EnterSceneTimerLock();
                                	g_isrun2 = 0;
                                	ExitSceneTimerLock();
                                }else{
                                	add_sceneTimer(hour,min,g_curselectscene); 
                                	updateviewsceneTimer(1);
                                	if(g_sceneTimerid != 0)
                                		SDL_RemoveTimer(g_sceneTimerid);
                                	g_sceneTimerid = 0;
                                	EnterSceneTimerLock();
                                	g_isrun = 0;
                                	ExitSceneTimerLock();
                        	}
                                g_isUpdated = 0;
                                updateButton2(7,1);
                                
                                memset(g_input,0,10);
                                updateEdit1(0,0);
                				updateEdit2(1,1);
                				updateEdit2(0,1);//恢复选择框的状态
                				g_curselectedit =0;//增加用于选框和闹钟个数显示
                			
                                update_rect(rcsceneTimer[7].x,rcsceneTimer[7].y,rcsceneTimer[7].w,rcsceneTimer[7].h);
                                g_isUpdated = 1;
                        }
                        break;
                case 7: //执行
                        if(status == 1){
                                StartButtonVocie();
                                g_isUpdated = 0;
                                updateButton1(7,1);
                                update_rect(rcsceneTimer[7].x,rcsceneTimer[7].y,rcsceneTimer[7].w,rcsceneTimer[7].h);
                                g_isUpdated = 1;
                        }else{
                        		g_isUpdated = 0;
                                updateButton2(7,1);
                                memset(g_input,0,sizeof(g_input));
                                update_rect(rcsceneTimer[7].x,rcsceneTimer[7].y,rcsceneTimer[7].w,rcsceneTimer[7].h);
                                g_isUpdated = 1;
                                if(g_is_only_alarmclock == 0x01){ //如果仅仅是闹钟
                                	if(g_isrun2 == 0){
                                        	int ret = StartClockTimer();
                                        	if(ret == 2)
                                                	CreateDialogWin2(LT("未设置日期，请校对系统日期!"),MB_OK,NULL,NULL);
                                        	if(ret == 1)
                                                	CreateDialogWin2(LT("未增加闹钟!"),MB_OK,NULL,NULL);
                                        	if(ret == 0)
                                                	save_clocktimer_table_to_flash();
                                	}else{
                                		if(g_clockTimerid != 0)
                                			SDL_RemoveTimer(g_clockTimerid);
                                        	g_clockTimerid = 0;
                                        	EnterSceneTimerLock();
                                        	g_isrun2 = 0;
                                        	ExitSceneTimerLock();
                        			g_isUpdated = 0;
                                		updateButton2(7,1);
                                		update_rect(rcsceneTimer[7].x,rcsceneTimer[7].y,rcsceneTimer[7].w,rcsceneTimer[7].h);
                                		g_isUpdated = 1;
                                	}                                	
                        	}else{                                
                                	if(g_isrun == 0){
                                	        int ret = StartSceneTimer();
                                	        if(ret == 2)
                                	                CreateDialogWin2(LT("未设置日期，请校对系统日期!"),MB_OK,NULL,NULL);
                                	        if(ret == 1)
                                	                CreateDialogWin2(LT("未增加情景计划!"),MB_OK,NULL,NULL);
                                	        if(ret == 0)
                                	                save_scenetimer_table_to_flash();
                                	}else{
                                		if(g_sceneTimerid != 0)
                                			SDL_RemoveTimer(g_sceneTimerid);
                                        	g_sceneTimerid = 0;
                                	        EnterSceneTimerLock();
                                	        g_isrun = 0;
                                	        ExitSceneTimerLock();
                        			g_isUpdated = 0;
                                		updateButton2(7,1);
                                		update_rect(rcsceneTimer[7].x,rcsceneTimer[7].y,rcsceneTimer[7].w,rcsceneTimer[7].h);
                                		g_isUpdated = 1;
                                	}
                        	}
                        }
                        break;
                case 11: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                                // Reset_sys();
                        }
                        break;
                case 12:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                case 10:
                        ret = SceneNumberhandle(x,y,status);
                        break;
                default:
                        break;
                        }
                        break;
                }
        }
        return ret;

}

void CloseTimerScene(){
	if(g_is_only_alarmclock == 0x01)
		g_is_only_alarmclock = 0x0;	
}

void CreateTimerScene(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;

        if(pIPNumberPointer == NULL && pIPNumberPointer1 == NULL){
                GetNumberJpgPointer(&pIPNumberPointer,&pIPNumberPointer1);//获得保存起来的数字符号的数据指针
                GetNumberJpegSizePointer(&pIPNumberSize,&pIPNumberSize1);//获得保存起来的数字符号的数据大小
        }
        memset(g_input,0,sizeof(g_input));
        g_count = 0;
        g_curselectedit = 0;
        if(g_is_only_alarmclock == 0x01){
        	g_curselectscene = 10;
        }else
        	g_curselectscene = 0;

        pWindow->EventHandler = TimerSceneHandle;
        pWindow->NotifyReDraw = DrawTimerScene;
        pWindow->CloseWindow  = CloseTimerScene;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);

        pWindow->NotifyReDraw2(pWindow);

}


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                             //
//录音界面 2009-8-3 9:30:23                                                                                      //
/////////////////////////////////////////////////////////////////////////////////////////////
static WRT_Rect rcRecord[6]={
        {361,237,130,104},	  //start
        {531,237,130,104},   //stop
        {378,196,240,39},   //time
        {460,200,200,40},
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};
static int g_isRecordRun = 0;

static void CloseRecordWin(){
        g_isRecordRun =0;
}

void UpdateRecordTime(int time){
        Window* pWindow = (Window*)WindowGetTopVisibleWindow();
        if(pWindow && pWindow->type == RECORD){
                FillRect(0xdedede,rcRecord[2]);
                char tmptext[50];
                sprintf(tmptext,"%s:%d%s",LT("留言倒计时"),time,LT("秒"));
                DrawText(tmptext,rcRecord[2].x,rcRecord[2].y+rcRecord[2].h-10,rgb_black);
                if(time == 0){
                        g_isRecordRun = 0;
                }
        }
}

static void DrawRecordText()
{
        int xoffset = 0;
        int yoffset = 0;
        xoffset = (rcRecord[0].w - GetTextWidth(LT("录音"),1))/2;
        if(xoffset < 0)
                xoffset = 10;
        xoffset += rcRecord[0].x + 25;
        yoffset = rcRecord[0].h + rcRecord[0].y -40;
        DrawText_16(LT("录音"),xoffset,yoffset,rgb_black);
        xoffset = (rcRecord[1].w - GetTextWidth(LT("停止"),1))/2;
        if(xoffset == 0)
                xoffset = 10;
        xoffset += rcRecord[1].x+25;
        DrawText_16(LT("停止"),xoffset,yoffset,rgb_black);
}

static void DrawSingleRecordText()
{
        int xoffset = 0;
        int yoffset = 0;
        xoffset = (rcRecord[0].w - GetTextWidth(LT("录音"),1))/2;
        if(xoffset < 0)
                xoffset = 10;
        xoffset += rcRecord[0].x + 25;
        yoffset = rcRecord[0].h + rcRecord[0].y -40;
        DrawText_16(LT("录音"),xoffset,yoffset,rgb_black);
        xoffset = (rcRecord[1].w - GetTextWidth(LT("停止"),1))/2;
        if(xoffset == 0)
                xoffset = 10;
        xoffset += rcRecord[1].x+25;
        DrawText_16(LT("停止"),xoffset,yoffset,rgb_black);

}


void CreateRecordWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = RecordEventHandle;
        pWindow->NotifyReDraw = RecordMain;
        pWindow->CloseWindow = CloseRecordWin;
        pWindow->valid = 1;
        pWindow->type = RECORD;    //录音窗口
        WindowAddToWindowList(pWindow);
        g_isRecordRun = 0;
        pWindow->NotifyReDraw2(pWindow);
}
void RecordMain(){
        WRT_Rect tmprc;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        JpegDisp(355,189,back_recordface,sizeof(back_recordface));

        JpegDisp(rcRecord[0].x,rcRecord[0].y,menu_recordstart1,sizeof(menu_recordstart1));
        JpegDisp(rcRecord[1].x,rcRecord[1].y,menu_recordstop1,sizeof(menu_recordstop1));

        JpegDisp(rcRecord[4].x,rcRecord[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcRecord[5].x,rcRecord[5].y,bt_guanping1,sizeof(bt_guanping1));
        DrawRecordText();
        UpdateRecordTime(20);
        
}
int   RecordEventHandle(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rcRecord[index],xy)){
                        pos_x = rcRecord[index].x;
                        pos_y = rcRecord[index].y;
                        ret = 1;
                        switch(index){
                case 0://录音
                        if(status == 1){
                                StartButtonVocie();
                                //SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_recordstart2,sizeof(menu_recordstart2));
                                DrawRecordText();
                        }else{
                                if(g_isRecordRun == 0){
                                		mp3stop();
                                        tmp_event[0] = CUSTOM_CMD_ROOM_START_RECORD;
                                        wrthost_send_cmd(tmp_event);
                                        g_isRecordRun = !g_isRecordRun;
                                }
                                JpegDisp(pos_x,pos_y,menu_recordstart1,sizeof(menu_recordstart1));
                                DrawRecordText();
                        }
                        break;
                case 1://停止
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_recordstop2,sizeof(menu_recordstop2));
                                DrawRecordText();
                        }else{

                                if(g_isRecordRun == 1){
                                        tmp_event[0] = CUSTOM_CMD_ROOM_STOP_RECORD;
                                        wrthost_send_cmd(tmp_event);
                                        g_isRecordRun = !g_isRecordRun;
                                }
                                JpegDisp(pos_x,pos_y,menu_recordstop1,sizeof(menu_recordstop1));
                                DrawRecordText();
                        }
                        break;
                case 4: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                if(g_isRecordRun == 1){
                                        tmp_event[0] = CUSTOM_CMD_ROOM_STOP_RECORD;
                                        wrthost_send_cmd(tmp_event);
                                        g_isRecordRun = !g_isRecordRun;
                                }
                                CloseTopWindow();
                                // Reset_sys();
                        }
                        break;
                case 5:/*关屏*/
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

/////////////////////////////////////////////////////////////////////
//
static WRT_Rect rtSip[16]={

        {170,85,175,120},
        {340,85,175,120},
        {510,85,175,120} ,
        {680,85,175,120} ,

        {170,200,175,120},
        {340,200,175,120},
        {510,200,175,120},
        {680,200,175,120},

        {170,315,175,120},
        {340,315,175,120},
        {510,315,175,120},
        {680,315,175,120},

        {900,531,111,69},              //返回
        {6,531,111,69},                //关屏幕
        {660,531,111,69},              /*上翻*/
        {780,531,111,69},               /* 下翻*/


};

static int g_current_select_grid = 0;
static int g_prev_select_grid    = 0;

static int g_current_sip_page = 1;
static int g_total_sip_pages = 1;


static void DrawSipText(WRT_Rect rt,char* text, int len,int color)
{
        int slen = strlen(text);
        if(slen*12 > rt.w){
                unsigned char* txtbuf = (unsigned char*)ZENMALLOC(slen+1);
                if(txtbuf == NULL)
                        return;
                memcpy(txtbuf,text,slen);
                txtbuf[slen] = '\0';
                int rl = slen ;
                while(1){
                        if(txtbuf[rl-1] >= 0xa0){
                                rl-=2;
                        }else
                        	rl--;
                        if((rl < 12)){
                                break;
                        }
                }
                txtbuf[rl]='\0';
                DrawText((char*)txtbuf,rt.x+15,rt.y+40,color);
                memset(txtbuf,0,slen+1);
                memcpy(txtbuf,text+rl,slen-rl);
                txtbuf[slen-rl]='\0';
                DrawText((char*)txtbuf,rt.x+15,rt.y+90,color);
                ZENFREE(txtbuf);

        }else{
                DrawText(text,rt.x+5,rt.y+70,color);
        }

}

static void HandleSipDataSources(int index)
{
        WRT_Rect rtTemp;
        char cTemp[50];
        int totalsize = 0;
        totalsize = getSipDataSourceSize();


        if(totalsize == 0){
                return;
        }
        if(((index +  (g_current_sip_page-1)*12)+1) > totalsize)
                return;

        g_prev_select_grid = g_current_select_grid;
        g_current_select_grid = index;


        if(g_prev_select_grid != 0xffffffff){
                DrawRect(rtSip[g_prev_select_grid],5,rgb_white);
                rtTemp =  rtSip[g_prev_select_grid];
                rtTemp.x +=5;
                rtTemp.y +=5;
                rtTemp.w -=10;
                rtTemp.h -=10;
                FillRect(BACK_COLOR,rtTemp);
                SIP_DATA_SOURCES* pSipData = getSipDataSource(g_prev_select_grid+(g_current_sip_page-1)*12);
                if(pSipData){
                        if(pSipData->type  == 0x0){
                                if(pSipData->len < 21){
                                        memset(cTemp,0,50);
                                        memcpy(cTemp,pSipData->data,pSipData->len);
                                        DrawSipText(rtTemp,(char*)cTemp,pSipData->len,rgb_yellow);
                                }
                        }else if(pSipData->type == 0x01){
                                JpegDisp(rtTemp.x,rtTemp.y,pSipData->data,pSipData->len);
                        }
                }
                DrawRect(rtSip[g_prev_select_grid],5,rgb_green);
        }

        if(g_current_select_grid != 0xffffffff){
                rtTemp =  rtSip[g_current_select_grid];
                rtTemp.x +=5;
                rtTemp.y +=5;
                rtTemp.w -=10;
                rtTemp.h -=10;
                FillRect(0XFFF79642,rtTemp);
                SIP_DATA_SOURCES* pSipData = getSipDataSource(index+(g_current_sip_page-1)*12);
                if(pSipData){
                        if(pSipData->type  == 0x0){
                                if(pSipData->len < 21){
                                        memset(cTemp,0,50);
                                        memcpy(cTemp,pSipData->data,pSipData->len);
                                        DrawSipText(rtTemp,(char*)cTemp,pSipData->len,rgb_yellow);
                                }
                        }else if(pSipData->type == 0x01){
                                JpegDisp(rtTemp.x,rtTemp.y,pSipData->data,pSipData->len);
                        }
                }
                DrawRect(rtSip[g_current_select_grid],5,0xffff6600);
        }

}


static void DrawSipDataSource(int update){
        int index;
        WRT_Rect rtTemp;
        char cTemp[50];
        WRT_Rect curbox;
        if(getSipDataSourceSize() == 0){
                DrawText(LT("正在查询..."),50,70,rgb_white);
                return;
        }
        
        curbox.x = 160;
        curbox.y =75;
        curbox.w = 695;
        curbox.h = 370;
        FillRect(BACK_COLOR,curbox);

        for(index=0; index<12;index++){
                rtTemp = rtSip[index];
                rtTemp.x +=5;
                rtTemp.y +=5;
                rtTemp.w -=10;
                rtTemp.h -=10;
                SIP_DATA_SOURCES* pSipData = getSipDataSource(index+(g_current_sip_page-1)*12);
                if(pSipData){
                        if(pSipData->type  == 0x0){
                                if(pSipData->len < 21){
                                        memset(cTemp,0,50);
                                        memcpy(cTemp,pSipData->data,pSipData->len);

                                        DrawSipText(rtTemp,((char*)cTemp),pSipData->len,rgb_yellow);
                                }
                        }else if(pSipData->type == 0x01){
                                JpegDisp(rtTemp.x,rtTemp.y,pSipData->data,pSipData->len);
                        }
                }
                DrawRect(rtSip[index],5,rgb_green);

        }

        DrawStatusbar(g_total_sip_pages,g_current_sip_page);
      
}

static void process_sip_call(int param)
{
	CloseWindow();
	CreateCallCenterWin();
}

void ClosePepleInfoWin2(){
        unsigned long tmp_event[4];
        delAllSipDataSource();
        tmp_event[0] = CUSTOM_CMD_CANCEL;
        tmp_event[1] = ROOM_INQSERVICEINFO;
        tmp_event[2] = tmp_event[3] = 0;
        wrthost_send_cmd(tmp_event);
}

void CreatePeopleInfoWin2()
{
        unsigned long tmp_event[4];
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        set_default_data();
        pWindow->EventHandler = PeopleInfoEventHandler2;
        pWindow->NotifyReDraw = DrawPeopleInfoMain2;
        pWindow->CloseWindow = ClosePepleInfoWin2;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

        //tmp_event[0] = ROOM_SERVICES_REQUEST;
        //tmp_event[1] = tmp_event[2] = tmp_event[3] = 0;
        //wrthost_send_cmd(tmp_event);

}
void DrawPeopleInfoMain2()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        JpegDisp(rtSip[12].x,rtSip[12].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rtSip[13].x,rtSip[13].y,bt_guanping1,sizeof(bt_guanping1));

        g_current_select_grid = 0xffffffff;
        g_prev_select_grid = 0xffffffff;
        g_current_sip_page = 1;
        g_total_sip_pages = getSipDataSourceSize()/12;
        if( getSipDataSourceSize() % 12)
                g_total_sip_pages++;
        if(g_total_sip_pages == 0)
                g_total_sip_pages = 1;

        DrawSipDataSource(0);

        HandleSipDataSources(0);

        if(g_total_sip_pages > 1){
                JpegDisp(rtSip[14].x,rtSip[14].y,menu_shangfan1,sizeof(menu_shangfan1));
                JpegDisp(rtSip[15].x,rtSip[15].y,menu_xiafan1,sizeof(menu_xiafan1));
        }


}
int  PeopleInfoEventHandler2(int x,int y,int status)
{
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index = 0; index < 16; index++){
                if(IsInside(rtSip[index],xy)){
                        pos_x = rtSip[index].x;
                        pos_y = rtSip[index].y;
                        ret = 1;
                        switch(index){
                case 12://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }
                        break;
                case 13://关屏
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                CloseAllWindows();
                        }
                        break;
                case 14: //上翻
                        if(g_total_sip_pages == 1){
                                break;
                        }
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                g_current_sip_page--;
                                if(g_current_sip_page < 1)
                                        g_current_sip_page = 1;
                                g_isUpdated = 0;
                                DrawSipDataSource(1);
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);
                        }
                        break;
                case 15://下翻
                        if(g_total_sip_pages == 1){
                                break;
                        }
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
                                g_current_sip_page++;
                                if(g_current_sip_page > g_total_sip_pages)
                                        g_current_sip_page = g_total_sip_pages;
								g_isUpdated = 0;                                        
                                DrawSipDataSource(1);
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);;
                        }
                        break;
                default:
                        if(status == 1){
                                StartButtonVocie();
                                HandleSipDataSources(index);
                        }else{

                                if(((index +  (g_current_sip_page-1)*12)+1) > getSipDataSourceSize())
                                        break;
                        }
                        break;
                        }
                }
        }
        return ret;
}

void CreateOursServices(){
        if(pSystemInfo->mySysInfo.hassip == 1){
                CreatePeopleInfoWin2();
        }else{
                CreatePeopleInfoWin();
        }
}

