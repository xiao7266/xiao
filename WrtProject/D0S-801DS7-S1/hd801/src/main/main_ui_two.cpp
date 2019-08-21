#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "c_interface.h"
#include "ctrlcmd.h"
#include "audiovideo.h"
#include "window.h"
#include "idtoiptable.h"
#include "ui_two_res.h"
#include "public_menu_res.h"



#include "image.h"
#include "jpeg.h"

#include <sys/stat.h>
#include <unistd.h>



extern bool     g_net_is_ok;
extern bool     g_isstartdelay;
extern bool     g_is_have_jiaju ;

extern WRT_Rect  rc_ui2_ico[6];
bool g_is_replace_main_ui[3] = {0};



static WRT_Rect  rc_main_ui2[11]=
{
        {127,50,150,73},
        {282,50,150,73},
        {437,50,150,73},
        {592,50,150,73},
        {747,50,150,73},

        {127,477,150,73},
        {282,477,150,73},
        {437,477,150,73},
        {592,477,150,73},
        {747,477,150,73},

        {949,0,62,59}

};

void CreateMainUI2()
{
        Window* pWindow = New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = HandleMainUI2Event;
        pWindow->NotifyReDraw = DrawMainUI2;
        pWindow->valid = 1;
        pWindow->type = ROOT;
        WindowAddToWindowList(pWindow);
#if  HAVEKEYBOARD
        InitAlphabetAndNumber();  //为call room 服务
#endif
        pWindow->NotifyReDraw2(pWindow);
        for(int i =0 ; i< 42;i++)
                g_rc_status[i] = false;
}

void UpdateReplaceJpeg(int index)
{
	static unsigned char* g_replacejpgbuf[3] = {NULL};
	static int g_replace_size[3] = {0};
	
	if(g_replacejpgbuf[index] == NULL){
		switch(index){
		case 0:
			g_replacejpgbuf[index] = get_main_ui_replace_jpg("main280",&g_replace_size[index]);
			break;
		case 1:
			g_replacejpgbuf[index] = get_main_ui_replace_jpg("main961",&g_replace_size[index]);
			break;
		case 2:
			g_replacejpgbuf[index] = get_main_ui_replace_jpg("main962",&g_replace_size[index]);
			break;
		}
	}
	
        if(g_is_replace_main_ui[index] == 1){
        	if(g_replacejpgbuf[index] != NULL)
        		ZENFREE(g_replacejpgbuf[index]);
        	g_replacejpgbuf[index] = NULL;
        	g_replace_size[index] = 0;
        	switch(index)
        	{
        		case 0:
        			g_replacejpgbuf[index] = get_main_ui_replace_jpg("main280",&g_replace_size[index]);
        			break;
        		case 1:
        			g_replacejpgbuf[index] = get_main_ui_replace_jpg("main961",&g_replace_size[index]);
        			break;
        		case 2:
        			g_replacejpgbuf[index] = get_main_ui_replace_jpg("main962",&g_replace_size[index]);
        			break;
        	}
        	
        	g_is_replace_main_ui[index] = 0;
        }
        
        switch(index)
        {
			case 0:
				if(g_replacejpgbuf[0] != NULL)
					JpegDisp(0,rc_main_ui2[0].y+rc_main_ui2[0].h+2,g_replacejpgbuf[0],g_replace_size[0]);
				else
					JpegDisp(0,rc_main_ui2[0].y+rc_main_ui2[0].h+2,main_center_back_820,sizeof(main_center_back_820));
				break;
			case 1:
				if(g_replacejpgbuf[1] != NULL)
					JpegDisp(0,0,g_replacejpgbuf[1],g_replace_size[1]);
				break;
			case 2:
				if(g_replacejpgbuf[2] != NULL)
					JpegDisp(0,rc_main_ui2[5].y,g_replacejpgbuf[2],g_replace_size[2]);			
				break;
	}
	
}

void ReUpdateUI2(int index)
{
		

	if(index == 0){
#if 1//保存主界面显示时间的背景		
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
		
		SDL_BlitSurface(g_pScreenSurface,&srt,tmp_g_pScreenSurface,&rt);
	
#endif        
		UpdateTimeIcon();
		UpdateAVIcon();
		UpdateNetIcon();
		UpdateMsgIcon();
		UpdateAlarmIcon(0);
		DrawHuanjinValue(0,0);
		DrawWeatherResult();	
	}
}

void DrawMainUI2()
{
	int yoffset = 0;
	int i,j;
	WRT_Rect rc;


	g_is_have_jiaju = pSystemInfo->mySysInfo.isjiaju;
	JpegDisp(0,0,mainback_820,sizeof(mainback_820));

	rc.x = 0;
	rc.y =0;
	rc.w = 1024;
	rc.h = 600;
/*	
	FillRect(0x272b63,rc);//主背景
	 
	rc.x = 0;
	rc.y = rc_main_ui2[0].y+rc_main_ui2[0].h;
	rc.w = 1024;
	rc.h = 2;
	FillRect(0xd9fef6,rc);//上线
	
	rc.x = 0;
	rc.y = rc_main_ui2[5].y-2;
	rc.w = 1024;
	rc.h = 2;
	FillRect(0xd9fef6,rc);//下线
	
	for(i=0;i<2;i++)
		for(j =0;j<4;j++)
		{
			if(i ==0 )
				rc.y = 50;
			else
				rc.y = 477;
							
			rc.x = rc_main_ui2[0+j].x+rc_main_ui2[0+j].w;
			//rc.w = rc_main_ui2[1+j].x-rc_main_ui2[0+j].x;
			rc.h = rc_main_ui2[0].h;
			//FillRect(0x808080,rc);
			
			JpegDisp(rc.x,rc.y,ui2_line,sizeof(ui2_line));
		}//小图标间隙

*/
		
        rc = rc_main_ui2[0];
        rc.y -= 35;
        rc.h = 24;
        DrawTextToRect(rc,LT("在家"),rgb_white);
        rc = rc_main_ui2[1];
        rc.y -= 35;
        rc.h = 24;
        DrawTextToRect(rc,LT("离家"),rgb_white);
        rc = rc_main_ui2[2];
        rc.y -= 35;
        rc.h = 24;
        DrawTextToRect(rc,LT("维修"),rgb_white);
        rc = rc_main_ui2[3];
        rc.y -= 35;
        rc.h = 24;
        DrawTextToRect(rc,LT("电梯"),rgb_white);
                
        rc = rc_main_ui2[4];
        rc.y -= 35;
        rc.h = 24;
        DrawTextToRect(rc,LT("智联"),rgb_white);

        rc = rc_main_ui2[5];
        rc.y = rc.y + rc.h+5;
        rc.h = 24;
        DrawTextToRect(rc,LT("服务"),rgb_white);
        rc = rc_main_ui2[6];
        rc.y = rc.y + rc.h+5;
        rc.h = 24;
        DrawTextToRect(rc,LT("设置"),rgb_white);
        rc = rc_main_ui2[7];
        rc.y = rc.y + rc.h+5;
        rc.h = 24;
        DrawTextToRect(rc,LT("图文"),rgb_white);
        rc = rc_main_ui2[8];
        rc.y = rc.y + rc.h+5;
        rc.h = 24;
        DrawTextToRect(rc,LT("监视"),rgb_white);
        rc = rc_main_ui2[9];
        rc.y = rc.y + rc.h+5;
        rc.h = 24;
        DrawTextToRect(rc,LT("呼叫"),rgb_white);
        
        JpegDisp(rc_main_ui2[0].x,rc_main_ui2[0].y,zaijia1_820,sizeof(zaijia1_820));        
		
        JpegDisp(rc_main_ui2[1].x,rc_main_ui2[1].y,lijia1_820,sizeof(lijia1_820));
        JpegDisp(rc_main_ui2[2].x,rc_main_ui2[2].y,weixiu1_820,sizeof(weixiu1_820));
        JpegDisp(rc_main_ui2[3].x,rc_main_ui2[3].y,dianti1_820,sizeof(dianti1_820));
        if(g_is_have_jiaju)
       		JpegDisp(rc_main_ui2[4].x,rc_main_ui2[4].y,jiaju1_820,sizeof(jiaju1_820));
        else
        	JpegDisp(rc_main_ui2[4].x,rc_main_ui2[4].y,jiaju3_820,sizeof(jiaju3_820));     

        JpegDisp(rc_main_ui2[5].x,rc_main_ui2[5].y,fuwu1_820,sizeof(fuwu1_820));
        JpegDisp(rc_main_ui2[6].x,rc_main_ui2[6].y,shezhi1_820,sizeof(shezhi1_820));
        JpegDisp(rc_main_ui2[7].x,rc_main_ui2[7].y,xinxi1_820,sizeof(xinxi1_820));
        JpegDisp(rc_main_ui2[8].x,rc_main_ui2[8].y,jianshi1_820,sizeof(jianshi1_820));
        JpegDisp(rc_main_ui2[9].x,rc_main_ui2[9].y,hujiao1_820,sizeof(hujiao1_820));
        JpegDisp(rc_main_ui2[10].x,rc_main_ui2[10].y,close1_820,sizeof(close1_820));

        UpdateReplaceJpeg(0);
        UpdateReplaceJpeg(1);
        UpdateReplaceJpeg(2);
#if 1//保存主界面显示时间的背景
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
	
	SDL_BlitSurface(g_pScreenSurface,&srt,tmp_g_pScreenSurface,&rt);
#endif   

	UpdateTimeIcon();
	UpdateAVIcon();
	
	UpdateMsgIcon();
	UpdateAlarmIcon(0);
	DrawHuanjinValue(0,0);
	DrawWeatherResult();
}



int   HandleMainUI2Event(int x,int y,int status)
{
        int ret = 0;
        WRT_xy xy;


        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
		
        unsigned long tmp_event[4];
        for(index =0 ;index<11;index++){
                if(IsInside(rc_main_ui2[index],xy)){
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
                        pos_x = rc_main_ui2[index].x;
                        pos_y = rc_main_ui2[index].y;

                        switch(index)
                        {
                        case 0: //在家-OK
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,zaijia2_820,sizeof(zaijia2_820));
                                }else{
										JpegDisp(pos_x,pos_y,zaijia1_820,sizeof(zaijia1_820));
				                                         //send_scene_mode(SCENE_ZAIJIA);//
										CreatePasswordWin(SET_CHEFANG_STATUS);
										//ipc_start();
										g_net_is_ok = true;
										//start_defend(false);
                                }
                                
                                break;
                        case 1://离家 -对话框字显示不正常
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,lijia2_820,sizeof(lijia2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,lijia1_820,sizeof(lijia1_820));
                                        
										CreateDialogWin2(LT("离家状态下,防区将设防,电源将关闭.确定要选择离家模式吗?"),MB_OK|MB_CANCEL,cur_set_leave_home,NULL);
                                        send_scene_mode(SCENE_LIJIA);
                                        //ipc_stop();
                                        g_net_is_ok = true;
                                        //start_defend(true);
                                }
                                break;
                        case 2://维修 - OK
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,weixiu2_820,sizeof(weixiu2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,weixiu1_820,sizeof(weixiu1_820));
                                        CreateMaintainWin();
                                        g_net_is_ok = true;
                                }
                                break;                              
                        case 3://电梯
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,dianti2_820,sizeof(dianti2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,dianti1_820,sizeof(dianti1_820));
                                        
                                        tmp_event[0] = ROOM_CALL_LIFT;
                                        tmp_event[1] =  pSystemInfo->LocalSetting.publicroom.Door[0].IP;
                                        tmp_event[2] = tmp_event[3]=0;
                                        wrthost_send_cmd(tmp_event);
                                       g_net_is_ok = true;
                                       
                                }
                                break;
                      case 4://家居
                      	        if(!g_is_have_jiaju)    
                                	break;
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,jiaju2_820,sizeof(jiaju2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,jiaju1_820,sizeof(jiaju1_820));
                                        //ClearRect(0,0,1024,600);
                                         //CreateJiajuWin();
                                         CreateSceneWin();
                                         g_net_is_ok = true;
                                }
                                break;  
                        case 5://服务
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,fuwu2_820,sizeof(fuwu2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,fuwu1_820,sizeof(fuwu1_820));
                                        ClearRect(0,0,1024,600);
										CreateServiceWin();
										g_net_is_ok = true;
                                }
                                break;
                        case 6://设置
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,shezhi2_820,sizeof(shezhi2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,shezhi1_820,sizeof(shezhi1_820));
                                        
                                        CreateSetupWin();
                                        g_net_is_ok = true;
                                }

                                break;
                        case 7: //图文
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,xinxi2_820,sizeof(xinxi2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,xinxi1_820,sizeof(xinxi1_820));
                                        CreateInfoWin();
                                        g_net_is_ok = true;
                                }
                                break;
                        case 9: //呼叫
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,hujiao2_820,sizeof(hujiao2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,hujiao1_820,sizeof(hujiao1_820));
                                        CreateCallWin();
                                        g_net_is_ok = true;
                                }
                                break;

                        case 8://监视
                                if(status == 1){ //按下
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,jianshi2_820,sizeof(jianshi2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,jianshi1_820,sizeof(jianshi1_820));
                                        CreatetMonitorWin();
                                        g_net_is_ok = true;
                                }
                                break;
                        case 10: //关屏
                                if(status == 1){ //按下
                                		//int len;
                                		//savefbtofile33(&len,90);
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,close2_820,sizeof(close2_820));
                                }else{
                                        JpegDisp(pos_x,pos_y,close1_820,sizeof(close1_820));
                                        screen_close();
                                        g_net_is_ok = true;
                                }
                                break;
                        default:
                                break;
                        }
                        break;
                }
        }
        if(ret == 0){ //处理小图标
        	 for(index =0 ;index<6;index++){
                	if(IsInside(rc_ui2_ico[index],xy)){
                	 	ret = 1;
                	 	WRT_Rect tmprect =rc_ui2_ico[index];
                         	if(status == 1)
                               	 	g_rc_status[index] = true;
                        	else{
                                	if(g_rc_status[index] == false){
                                        	ret = 0;
                                        	return ret;
                                	}
                                	g_rc_status[index] = false;
                       		}               	 	
				//DeflateRect(&tmprect,1);
                	 	switch(index)
                	 	{
                	 		case 0:
                	 			break;
                	 		case 1: //安防
                	 			if(status == 1){
                	 				StartButtonVocie();
                	 				DrawRect(tmprect,16,0x808080);
                	 			}else{
                	 				UpdateAlarmIcon();
                	 			 	CreatePasswordWin(SET_SAFE_PWD_WIN);
                	 			 	g_net_is_ok = true;
                	 			}
                	 			break;
                	 		case 2://报警日志
								if(status == 1){
									StartButtonVocie();
                	 				DrawRect(tmprect,16,0x808080);
                	 			}else{
                	 				UpdateAlarmIcon();
                	 				CreateWarningLog();
                	 				g_net_is_ok = true;
                	 			}
                	 			break;
                	 		case 3://本机设置
								if(status == 1){
									StartButtonVocie();
                	 				DrawRect(tmprect,16,0x808080);
                	 				
                	 			}else{ 
                	 				UpdateAVIcon();
                	 				CreateLocalSetWin();
                	 				g_net_is_ok = true;
                	 			}
                	 			break;
                	 		case 4://短信
								if(status == 1){
									StartButtonVocie();
                	 				DrawRect(tmprect,16,0x808080);
                	 			}else{  
                	 				UpdateMsgIcon();
                	 				CreateMsgWin();
                	 				g_net_is_ok = true;
                	 			}
                	 			break;
                	 		case 5://留言
								if(status == 1){
									StartButtonVocie();
                	 				DrawRect(tmprect,16,0x808080);
                	 			}else{ 
                	 				UpdateMsgIcon();
                	 				CreateVoiceWin();
                	 				g_net_is_ok = true;
                	 			}
                	 			break;
                	 		default:
                	 			break;	
                			 		
                	 	}
                	}
                }
        }
        return ret;
}
