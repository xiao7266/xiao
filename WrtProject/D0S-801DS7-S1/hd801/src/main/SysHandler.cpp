#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/vfs.h> 
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "sys_menu_res.h"
#include "window.h"

static const WRT_Rect rc_sysmenuico[6]={
        {326,149,132,160},             //系统信息
        {565,149,132,160},             //本机状态
        {660,531,111,69},              /*上翻*/ //470
        {780,531,111,69},               /* 下翻*/	//574
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕	
};

//////////////////////////////////////////////////////////
void CreateSysWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = SysEventHandler;
        pWindow->NotifyReDraw = DrawSysMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);
}

void DrawSysMain(){
        int xoffset = 0;

        WRT_Rect rt;
        rt.x = 0;
        rt.y = 0;
        rt.w =1024;
        rt.h = 530;
        FillSingleRect(0xb2136,rt);

        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(rc_sysmenuico[0].x,rc_sysmenuico[0].y,menu_xitongxinxi1,sizeof(menu_xitongxinxi1));
        JpegDisp(rc_sysmenuico[1].x,rc_sysmenuico[1].y,menu_xitongzhuangtai1,sizeof(menu_xitongzhuangtai1));
        JpegDisp(rc_sysmenuico[4].x,rc_sysmenuico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_sysmenuico[5].x,rc_sysmenuico[5].y,bt_guanping1,sizeof(bt_guanping1)); 
        DrawEnText(LT("系统信息"),rc_sysmenuico[0].w+xoffset,rc_sysmenuico[0].h,rc_sysmenuico[0].x,rc_sysmenuico[0].y);
        DrawEnText(LT("系统状态"),rc_sysmenuico[1].w+xoffset,rc_sysmenuico[1].h,rc_sysmenuico[1].x,rc_sysmenuico[1].y);
}
int SysEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_sysmenuico[index],xy)){
                        pos_x = rc_sysmenuico[index].x;
                        pos_y = rc_sysmenuico[index].y;
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
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_xitongxinxi2,sizeof(menu_xitongxinxi2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_xitongxinxi1,sizeof(menu_xitongxinxi1));
                                  CreateSysInfoWin();                      
                          }
                          break;  
                  case 1:
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_xitongzhuangtai2,sizeof(menu_xitongzhuangtai2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_xitongzhuangtai1,sizeof(menu_xitongzhuangtai1)); 
                                  CreateSysStatusWin();                     
                          }            	  
                          break;      
                  case 4: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                  CloseTopWindow();                   
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
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

static void DrawSysInfo(){            
        char text[40]={'\0'};
        unsigned char ip[16] = {0};
        int pos_x=30,pos_y=80;
        int offset = 30;
        unsigned short year = 0;
        memcpy(ip,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        sprintf(text,"ID:    %s",ip);
        DrawText(text,pos_x,pos_y,rgb_white);

        memset(text,'\0',40);
        sprintf(text,"SN:    %s",pSystemInfo->BootInfo.SN);
        pos_y +=offset;
        DrawText(text,pos_x,pos_y,rgb_white);

        memset(text,'\0',40);
        memset(ip,0,16);
        toinet_addr((pSystemInfo->LocalSetting.privateinfo.LocalIP),ip);
        sprintf(text,"%s IP:    %s",LT("本机"),ip);
        pos_y +=offset;
        DrawText(text,pos_x,pos_y,rgb_white);

        memset(text,'\0',40);
        memset(ip,0,16);
        toinet_addr((pSystemInfo->LocalSetting.privateinfo.SubMaskIP),ip);
        sprintf(text,"%s:  %s",LT("子网掩码"),ip);
        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);


        memset(text,'\0',40);
        memset(ip,0,16);
        toinet_addr((pSystemInfo->LocalSetting.privateinfo.GateWayIP),ip);
        sprintf(text,"%s:      %s",LT("网关"),ip);
        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);

        memset(text,'\0',40);
        memset(ip,0,16);
        toinet_addr((pSystemInfo->LocalSetting.publicroom.Door[0].IP),ip);    
        sprintf(text,"%s IP:    %s",LT("门口机"),ip);
        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);


        memset(text,'\0',40);
        memset(ip,0,16);
        toinet_addr((pSystemInfo->LocalSetting.publicinfo.CenterMajorIP),ip);    
        sprintf(text,"%s IP:    %s",LT("中心"),ip);
        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);

        memset(text,'\0',40);
        memset(ip,0,16);
        toinet_addr((pSystemInfo->LocalSetting.publicinfo.CenterWarningIP),ip);        
        sprintf(text,"%s IP:  %s",LT("中心报警"),ip);
        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);

        memset(text,'\0',40);
        memset(ip,0,16);
        sprintf(text,"MAC:   %02x:%02x:%02x:%02x:%02x:%02x",pSystemInfo->BootInfo.MAC[0],
                pSystemInfo->BootInfo.MAC[1],pSystemInfo->BootInfo.MAC[2],pSystemInfo->BootInfo.MAC[3],
                pSystemInfo->BootInfo.MAC[4],pSystemInfo->BootInfo.MAC[5]);
        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);
 
        memset(text,'\0',40);
        memcpy(&year,pSystemInfo->BootInfo.Version+4,2);
        sprintf(text,"%s: %d.%02d.%02d.%04d%02d%02d",LT("本机软件版本"),pSystemInfo->BootInfo.Version[1],pSystemInfo->BootInfo.Version[2],pSystemInfo->BootInfo.Version[3],year,
                pSystemInfo->BootInfo.Version[6],pSystemInfo->BootInfo.Version[7]);

        pos_y +=offset;
        DrawText(text,pos_x, pos_y,rgb_white);

}

void CreateSysInfoWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = SysInfoEventHandler;
        pWindow->NotifyReDraw = DrawSysInfoMain;
        pWindow->valid = 1; 
        pWindow->type = CHILD;

        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);
}
void DrawSysInfoMain(){
        WRT_Rect rt;
        rt.x = 0;
        rt.y = 0;
        rt.w =1024;
        rt.h = 530;
        FillSingleRect(0xb2136,rt);

        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        JpegDisp(rc_sysmenuico[4].x,rc_sysmenuico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_sysmenuico[5].x,rc_sysmenuico[5].y,bt_guanping1,sizeof(bt_guanping1)); 
        DrawSysInfo();	
}
int SysInfoEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_sysmenuico[index],xy)){
                        pos_x = rc_sysmenuico[index].x;
                        pos_y = rc_sysmenuico[index].y;
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
                case 4: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                CloseTopWindow();                   
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

//////////////////////////////
//系统状态
//////////////////////////////

static int g_cur_view_page = 0;

static void DrawSysStatusInfo(){
	WRT_Rect curbox;
	long _size;
	float fsize = 0;
	struct statfs vbuf;        
	curbox.x = 0;
	curbox.y = 0;
	curbox.w = 800;
	curbox.h = 415;  

	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillSingleRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
 
	JpegDisp(rc_sysmenuico[4].x,rc_sysmenuico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_sysmenuico[5].x,rc_sysmenuico[5].y,bt_guanping1,sizeof(bt_guanping1)); 

	statfs("/",&vbuf);

	_size = (vbuf.f_bsize * vbuf.f_bfree);

	if(g_cur_view_page == 0){
		DrawText(LT("本机状态: 活动"),50,70,rgb_white);

		char text[60];
		fsize = (float)_size/(1024*1024);
		sprintf(text,"%s: %.2fM",LT("可用的Flash空间"),fsize);
		WRT_DEBUG("text = %s fsize = %f",text,fsize);
		DrawText(text,50,100,rgb_white);
		
		_size = (vbuf.f_bsize * vbuf.f_blocks) - _size;
		fsize = (float)_size/(1024*1024);
		sprintf(text,"%s: %.2fM",LT("已使用的Flash空间"),fsize);
		WRT_DEBUG("text = %s fsize = %f",text,fsize);
		DrawText(text,50,130,rgb_white);
		
		if(pSystemInfo->mySysInfo.isbuttonvoice  == 0)
			sprintf(text,"1: %s",LT("本机按键音关闭"));
		else
			sprintf(text,"1: %s",LT("本机按键音启用"));
		DrawText(text,50,160,rgb_white);    

		if(pSystemInfo->mySysInfo.isjiaju == 0)
			sprintf(text,"2: %s",LT("本机智联模块关闭"));
		else
			sprintf(text,"2: %s",LT("本机智联模块启用"));
		DrawText(text,50,190,rgb_white);    

		if(pSystemInfo->mySysInfo.iskeepscreen  == 0)
			sprintf(text,"3: %s",LT("本机屏幕保护关闭"));
		else
			sprintf(text,"3: %s",LT("本机屏幕保护启用"));
		DrawText(text,50,220,rgb_white);  

		if(pSystemInfo->mySysInfo.isagent == 0)
			sprintf(text,"4: %s",LT("本机未托管"));
		else
			sprintf(text,"4: %s",LT("本机已托管"));	
		DrawText(text,50,250,rgb_white);  

		if(pSystemInfo->mySysInfo.isavoid == 0)
			sprintf(text,"5: %s",LT("本机未设置免打扰"));
		else
			sprintf(text,"5: %s",LT("本机已设置免打扰"));	
		DrawText(text,50,280,rgb_white);   

		if(pSystemInfo->mySysInfo.isautocap == 0)
			sprintf(text,"6: %s",LT("本机自动抓拍关闭"));
		else
			sprintf(text,"6: %s",LT("本机自动抓拍启用"));	
		DrawText(text,50,310,rgb_white);

		if(pSystemInfo->mySysInfo.isremote == 0)
			sprintf(text,"7: %s",LT("本机远程控制关闭"));
		else
			sprintf(text,"7: %s",LT("本机远程控制启用"));	
		DrawText(text,50,340,rgb_white);        

		sprintf(text,"8: %s %d-%d-%d-%d-%d",LT("呼叫格式"),pSystemInfo->mySysInfo.idrule[0],pSystemInfo->mySysInfo.idrule[1]  ,pSystemInfo->mySysInfo.idrule[2]  
			,pSystemInfo->mySysInfo.idrule[3]  ,pSystemInfo->mySysInfo.idrule[4],pSystemInfo->mySysInfo.idrule[5] );  
		DrawText(text,50,370,rgb_white);
		
		return;
	}
}

void CreateSysStatusWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        g_cur_view_page = 0;
        pWindow->EventHandler = SysStatusEventHandler;
        pWindow->NotifyReDraw = DrawSysStatusMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;

        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);
}

void DrawSysStatusMain(){
        DrawSysStatusInfo();
}

int SysStatusEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_sysmenuico[index],xy)){
                        pos_x = rc_sysmenuico[index].x;
                        pos_y = rc_sysmenuico[index].y;
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
                case 2://上翻
                		break;
                        if(GetCurrentLanguage() == ENGLISH)
                                break;
                        if(g_cur_view_page ==0)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                g_cur_view_page--;       
                                if(g_cur_view_page < 0)
                                        g_cur_view_page = 8;
                                g_isUpdated = 0;
                                DrawSysStatusInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);
                        }                    
                        break;
                case 3://下翻
                		break;
                        if(GetCurrentLanguage() == ENGLISH)
                                break;
                        if(g_cur_view_page==1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));  
                                g_cur_view_page++;
                                if(g_cur_view_page > 8)
                                        g_cur_view_page = 0; 
                                g_isUpdated = 0; 
                                DrawSysStatusInfo();   
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);                                
                        }                       
                        break;
                case 4:/*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                CloseTopWindow();                   
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

