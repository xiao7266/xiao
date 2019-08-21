#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "setup_main_res.h"
#include "window.h"

#include "c_interface.h"


#ifdef USE_TESHU_1
extern unsigned char menu_xitongzhuangtai2[4733];
extern unsigned char menu_xitongzhuangtai1[4540];
#endif
extern unsigned char main_shengji2[25837];
extern unsigned char main_shengji1[26128];
extern unsigned char image_shengji1[26546];
extern unsigned char image_shengji2[26032];
extern unsigned char app_shengji1[28857];
extern unsigned char app_shengji2[28588];
//extern bool     g_rc_status[42]; 

static const WRT_Rect rc_setupmenuico[10]={
        {108,83, 132,160},    //密码
        {326,83, 132,160},   //安防
        {555,83,132,160},  //铃声
        {779,83,132,160},   //家居
        {108,305,132,160},   //系统
        {326,305,132,160},  //时间校准
        {555,305,132,160},  //屏幕校准
        {779,305,132,160},  //本机设置
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕				
};


void CreateSetupWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = SetupEventHandler;
        pWindow->NotifyReDraw = DrawSetupMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);   	
        pWindow->NotifyReDraw2(pWindow);
}

void DrawSetupMain(){
	int xoffset = 0;
	
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));   
        JpegDisp(rc_setupmenuico[0].x,rc_setupmenuico[0].y,menu_mima1,sizeof(menu_mima1));
        JpegDisp(rc_setupmenuico[1].x,rc_setupmenuico[1].y,menu_anfang1,sizeof(menu_anfang1)); 
        JpegDisp(rc_setupmenuico[2].x,rc_setupmenuico[2].y,menu_lingsheng1,sizeof(menu_lingsheng1));
#ifdef USE_TESHU_1
        if(pSystemInfo->mySysInfo.isautocap == 0)//是否自动抓拍   
                JpegDisp(rc_setupmenuico[3].x,rc_setupmenuico[3].y,menu_quxiaozhuapai1,sizeof(menu_quxiaozhuapai1));
        else
                JpegDisp(rc_setupmenuico[3].x,rc_setupmenuico[3].y,menu_zidongzhuapai1,sizeof(menu_zidongzhuapai1));

        JpegDisp(rc_setupmenuico[5].x,rc_setupmenuico[5].y,menu_xitongzhuangtai1,sizeof(menu_xitongzhuangtai1));
#else
	 if(g_is_have_jiaju)
         	JpegDisp(rc_setupmenuico[3].x,rc_setupmenuico[3].y,menu_jiajushezhi1,sizeof(menu_jiajushezhi1)); 
         else
         	JpegDisp(rc_setupmenuico[3].x,rc_setupmenuico[3].y,menu_jiajushezhi3,sizeof(menu_jiajushezhi3)); 


        JpegDisp(rc_setupmenuico[5].x,rc_setupmenuico[5].y,menu_xitong1,sizeof(menu_xitong1));
#endif


        JpegDisp(rc_setupmenuico[4].x,rc_setupmenuico[4].y,menu_shijianjiaozhun1,sizeof(menu_shijianjiaozhun1)); 
        JpegDisp(rc_setupmenuico[6].x,rc_setupmenuico[6].y,main_shengji1,sizeof(main_shengji1));
        JpegDisp(rc_setupmenuico[7].x,rc_setupmenuico[7].y,menu_benjishezhi1,sizeof(menu_benjishezhi1));   	  
        JpegDisp(rc_setupmenuico[8].x,rc_setupmenuico[8].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_setupmenuico[9].x,rc_setupmenuico[9].y,bt_guanping1,sizeof(bt_guanping1)); 

        DrawEnText(LT("密码"),rc_setupmenuico[0].w+xoffset,rc_setupmenuico[0].h,rc_setupmenuico[0].x,rc_setupmenuico[0].y);
        DrawEnText(LT("安防"),rc_setupmenuico[1].w+xoffset,rc_setupmenuico[1].h,rc_setupmenuico[1].x,rc_setupmenuico[1].y);
        DrawEnText(LT("铃声"),rc_setupmenuico[2].w+xoffset,rc_setupmenuico[2].h,rc_setupmenuico[2].x,rc_setupmenuico[2].y);
#ifdef USE_TESHU_1
        DrawEnText(LT("录像抓拍"),rc_setupmenuico[3].w+xoffset,rc_setupmenuico[3].h,rc_setupmenuico[3].x,rc_setupmenuico[3].y);
        DrawEnText(LT("系统状态"),rc_setupmenuico[5].w+xoffset,rc_setupmenuico[5].h,rc_setupmenuico[5].x,rc_setupmenuico[5].y);
#else
        DrawEnText(LT("智联"),rc_setupmenuico[3].w+xoffset,rc_setupmenuico[3].h,rc_setupmenuico[3].x,rc_setupmenuico[3].y);
        DrawEnText(LT("系统"),rc_setupmenuico[5].w+xoffset,rc_setupmenuico[5].h,rc_setupmenuico[5].x,rc_setupmenuico[5].y);
#endif

        DrawEnText(LT("时间校准"),rc_setupmenuico[4].w+xoffset,rc_setupmenuico[4].h,rc_setupmenuico[4].x,rc_setupmenuico[5].y);
        DrawEnText(LT("升级"),rc_setupmenuico[6].w+xoffset,rc_setupmenuico[6].h,rc_setupmenuico[6].x,rc_setupmenuico[6].y);
        DrawEnText(LT("本机设置"),rc_setupmenuico[7].w+xoffset,rc_setupmenuico[7].h,rc_setupmenuico[7].x,rc_setupmenuico[7].y);


        WRT_Rect curbox;
        curbox.x = 135;
        curbox.y = 530;
        curbox.w = 578;
        curbox.h = 55;
		JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4)); 
		

}

int  SetupEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<10;index++){
                if(IsInside(rc_setupmenuico[index],xy)){
                        pos_x = rc_setupmenuico[index].x;
                        pos_y = rc_setupmenuico[index].y;
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
                  case 0://密码 OK
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_mima2,sizeof(menu_mima2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_mima1,sizeof(menu_mima1));  
                                  CreateSetPasswordWin();                     
                          }            	 
                          break; 
                  case 1: //安防 
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_anfang2,sizeof(menu_anfang2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_anfang1,sizeof(menu_anfang1));     
                                  CreatePasswordWin(SET_SAFE_PWD_WIN);
                                  //CreateSafeWin();                  
                          }                 	  
                          break; 
                  case 2://铃声
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_lingsheng2,sizeof(menu_lingsheng2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_lingsheng1,sizeof(menu_lingsheng1)); 
                                  CreateRingWin();                     
                          }              	  
                          break; 
                  case 3://家居设置
#ifdef USE_TESHU_1
                          if(status == 1){
                                  StartButtonVocie();
                                  if(pSystemInfo->mySysInfo.isautocap == 0){
                                          JpegDisp(pos_x,pos_y,menu_quxiaozhuapai2,sizeof(menu_quxiaozhuapai2));
                                  }else{
                                          JpegDisp(pos_x,pos_y,menu_zidongzhuapai2,sizeof(menu_zidongzhuapai2));
                                  }

                          }else{
                                  if(pSystemInfo->mySysInfo.isautocap == 0){
                                          JpegDisp(pos_x,pos_y,menu_zidongzhuapai1,sizeof(menu_zidongzhuapai1));
                                          pSystemInfo->mySysInfo.isautocap = 1;
                                  }else{
                                          JpegDisp(pos_x,pos_y,menu_quxiaozhuapai1,sizeof(menu_quxiaozhuapai1));
                                          pSystemInfo->mySysInfo.isautocap = 0;
                                  }
                                  CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                                  save_localhost_config_to_flash();              		 
                                  CloseTopWindow();

                          }
#else
			  if(!g_is_have_jiaju)
			  	break;
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_jiajushezhi2,sizeof(menu_jiajushezhi2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_jiajushezhi1,sizeof(menu_jiajushezhi1));  
                                  CreatePasswordWin(SET_JIAJU_PWD_WIN);
                          }     
                          // CreateHomeSetWin(); 
#endif


                          break; 
                  case 5://系统
#ifdef USE_TESHU_1
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_xitongzhuangtai2,sizeof(menu_xitongzhuangtai2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_xitongzhuangtai1,sizeof(menu_xitongzhuangtai1));  
                                  CreateSysStatusWin();                      
                          }   
#else
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_xitong2,sizeof(menu_xitong2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_xitong1,sizeof(menu_xitong1));  
                                  CreateSysWin();                     
                          }                  	  
#endif
                          break; 
                  case 4://时间校准
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_shijianjiaozhun2,sizeof(menu_shijianjiaozhun2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_shijianjiaozhun1,sizeof(menu_shijianjiaozhun1));      
                                  CreateDialogWin2(LT("确定和中心时间校准?"),MB_OK|MB_CANCEL,ResetSysTime,NULL);
                          }                   	  
                          break;
                  case 6://升级
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,main_shengji2,sizeof(main_shengji2));
                          }else{
                                  JpegDisp(pos_x,pos_y,main_shengji1,sizeof(main_shengji1)); 
                                  //CreateDialogWin2(LT("确定要校正屏幕吗?"),MB_OK|MB_CANCEL,screen_adjust,NULL);                   
                                  CreateUpgradeWin();
                          }                        	  
                          break;
                  case 7://本机设置
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_benjishezhi2,sizeof(menu_benjishezhi2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_benjishezhi1,sizeof(menu_benjishezhi1)); 
                                  CreateLocalSetWin();                      
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

int  ResetSysTime(int param){
        unsigned long event[4];
        event[0] = ROOM_GETTIME;
        event[1] = event[2] = event[3] = 0;
        wrthost_send_cmd(event);
        WRT_DEBUG("send cmd room_gettime");
        return 0;
}

//////////////////////////////////////////////////////////
static const WRT_Rect rc_upgrademenuico[4]={
	{233,175,132,160}, 
	{659,175,132,160},
    {900,531,111,69},              //返回
    {6,531,111,69}                 //关屏幕		
};

void CreateUpgradeWin()
{
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = UpgradeEventHandler;
        pWindow->NotifyReDraw = DrawUpgradeWin;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);   	
        pWindow->NotifyReDraw2(pWindow);	
}

void DrawUpgradeWin()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);	
    JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));   
        
    JpegDisp(rc_upgrademenuico[0].x,rc_upgrademenuico[0].y,app_shengji1,sizeof(app_shengji1)); 
    JpegDisp(rc_upgrademenuico[1].x,rc_upgrademenuico[1].y,image_shengji1,sizeof(image_shengji1)); 
    JpegDisp(rc_upgrademenuico[2].x,rc_upgrademenuico[2].y,bt_fanhui1,sizeof(bt_fanhui1)); 
    JpegDisp(rc_upgrademenuico[3].x,rc_upgrademenuico[3].y,bt_guanping1,sizeof(bt_guanping1));	
    DrawEnText(LT("应用升级"),rc_upgrademenuico[0].w,rc_upgrademenuico[0].h,rc_upgrademenuico[0].x,rc_upgrademenuico[0].y);
    DrawEnText(LT("镜像升级"),rc_upgrademenuico[1].w,rc_upgrademenuico[1].h,rc_upgrademenuico[1].x,rc_upgrademenuico[1].y);
}
int UpgradeEventHandler(int x,int y,int status)
{
	WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rc_upgrademenuico[index],xy)){
                        pos_x = rc_upgrademenuico[index].x;
                        pos_y = rc_upgrademenuico[index].y;
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
                        case 0:
 			  			if(status == 1){
                          	        StartButtonVocie();

									JpegDisp(rc_upgrademenuico[0].x,rc_upgrademenuico[0].y,app_shengji2,sizeof(app_shengji2));
                          	}else{
                          	        StartButtonVocie();
 
									JpegDisp(rc_upgrademenuico[0].x,rc_upgrademenuico[0].y,app_shengji1,sizeof(app_shengji1));
									unsigned long tmpevent[4] = {0};
									tmpevent[0] = CUSTOM_CMD_LOCAL_UPGRADE;
									wrthost_send_cmd(tmpevent);
                          	}
                          	break;                        	
                        case 1:
 			  				if(status == 1){
                          	        StartButtonVocie();

									JpegDisp(rc_upgrademenuico[1].x,rc_upgrademenuico[1].y,image_shengji2,sizeof(image_shengji2));
                          	}else{
                          			FILE *tempfp = fopen("/mnt/mmc/wrtsystem/system.tar.gz","r");
                          			if(tempfp != NULL)
                          			{
                          				fclose(tempfp);
                          				tempfp = NULL;
                          				system("cp /mnt/mmc/wrtsystem/system.tar.gz /");
                          				system("sync");
                          				CreateDialogWin2(LT("升级成功,5s后自动重启"),MB_NONE,NULL,NULL);
                        				SDL_Delay(5000);
                        				system("reboot -f");
                          			}
                          			
                          			JpegDisp(rc_upgrademenuico[1].x,rc_upgrademenuico[1].y,image_shengji1,sizeof(image_shengji1));
                        			CreateDialogWin2(LT("系统升级失败"),MB_OK,NULL,NULL);
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