#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "window.h"
#include "c_interface.h"


#ifdef THREE_UI
#include "jiaju_menu_res2.h"
#endif

unsigned  char g_isctrlelec = 0; //用来控制是否丢弃触控消息


int g_cur_set_scene_mode=-1;


WRT_Rect rc_jiajuemenuico[4]={
        {326,145,132,160},
        {565,145,132,160},
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕         
};

void CreateJiajuWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = JiajuEventHandler;
        pWindow->NotifyReDraw = DrawJiajuMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);      
        pWindow->NotifyReDraw2(pWindow);        
}

void DrawJiajuMain(){
        int xoffset = 0;
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(rc_jiajuemenuico[0].x,rc_jiajuemenuico[0].y,menu_qingjingkongzhi1,sizeof(menu_qingjingkongzhi1));
        JpegDisp(rc_jiajuemenuico[1].x,rc_jiajuemenuico[1].y,menu_shineikongzhi1,sizeof(menu_shineikongzhi1));
        JpegDisp(rc_jiajuemenuico[2].x,rc_jiajuemenuico[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_jiajuemenuico[3].x,rc_jiajuemenuico[3].y,bt_guanping1,sizeof(bt_guanping1));
        DrawEnText(LT("情景控制"),rc_jiajuemenuico[0].w+xoffset,rc_jiajuemenuico[0].h,rc_jiajuemenuico[0].x,rc_jiajuemenuico[0].y);
        DrawEnText(LT("室内控制"),rc_jiajuemenuico[1].w+xoffset,rc_jiajuemenuico[1].h,rc_jiajuemenuico[1].x,rc_jiajuemenuico[1].y);
}

int JiajuEventHandler(int x,int y,int status){
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
                                JpegDisp(pos_x,pos_y,menu_qingjingkongzhi2,sizeof(menu_qingjingkongzhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_qingjingkongzhi1,sizeof(menu_qingjingkongzhi1));
                                CreateSceneWin();
                        }                                                                   
                        break;
                case 1://室内控制
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shineikongzhi2,sizeof(menu_shineikongzhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shineikongzhi1,sizeof(menu_shineikongzhi1));           
                               // CreateConfigMain(2); 
                              	 CreateShortcutRoomControl2(); //2010-11-19

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

////////////////////////////////////////////////////////////
//情景控制

static const WRT_Rect rc_scenemenuico[12]={
        {86,50,132,160},
        {266,50,132,160},
        {446,50,132,160},
        {626,50,132,160},
        {806,50,132,160},
        
        {86,295,132,160},
        {266,295,132,160},
        {446,295,132,160},
        {626,295,132,160},
        {806,295,132,160},
                
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕         
};

void CreateSceneWin2()
{
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = SceneEventHandler2;
        pWindow->NotifyReDraw = DrawSceneMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);      
        pWindow->NotifyReDraw2(pWindow);	
}

int SceneEventHandler2(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        int rc = 0;
      
        xy.x = x;
        xy.y = y;

        for(index =0 ;index<12;index++){
                if(IsInside(rc_scenemenuico[index],xy)){
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
                        pos_x = rc_scenemenuico[index].x;
                        pos_y = rc_scenemenuico[index].y;              
                        switch(index){
                case 0: //会客
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_huike2,sizeof(menu_huike2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_huike1,sizeof(menu_huike1));
							CreateSceneSetWin2(SCENE_HUIKE);
                        }                                                                   
                        break;
                case 1://就餐
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jiucan2,sizeof(menu_jiucan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_jiucan1,sizeof(menu_jiucan1)); 
                               CreateSceneSetWin2(SCENE_JIUCAN);
                        }                       
                        break;  
                case 2://影院
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_yingyuan2,sizeof(menu_yingyuan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_yingyuan1,sizeof(menu_yingyuan1));                              
                                CreateSceneSetWin2(SCENE_YINGYUAN);

                        }                       
                        break;  
                case 3://就寝
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jiuqin2,sizeof(menu_jiuqin2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_jiuqin1,sizeof(menu_jiuqin1));                                
                                CreateSceneSetWin2(SCENE_JIUQIN);
                             
                        }                       
                        break;  
                case 4://温馨
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_wenxin2,sizeof(menu_wenxin2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_wenxin1,sizeof(menu_wenxin1));                               
                                CreateSceneSetWin2(SCENE_WENXIN);
                                    
                        }                       
                        break;   
                case 5:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_zaijia2,sizeof(bt_zaijia2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_zaijia1,sizeof(bt_zaijia1));        	
                                CreateSceneSetWin2(SCENE_ZAIJIA);
			}
                	break;
                case 6:
			if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_lijia2,sizeof(bt_lijia2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_lijia1,sizeof(bt_lijia1)); 
                                CreateSceneSetWin2(SCENE_LIJIA);
                	
                        }
                	break;
                case 7:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_yeqi2,sizeof(menu_yeqi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_yeqi1,sizeof(menu_yeqi1));             	
                                CreateSceneSetWin2(SCENE_YEQI);
                   	
                        }
                	break;
                case 8:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_chenqi2,sizeof(menu_chenqi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_chenqi1,sizeof(menu_chenqi1));               	
                                CreateSceneSetWin2(SCENE_CHENQI);

                        }                	
                	break;
                case 9:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_zidingyi2,sizeof(menu_zidingyi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_zidingyi1,sizeof(menu_zidingyi1));                   	
                                  CreateSceneSetWin2(SCENE_USERDEFINE);
                 	
                        }
                	break;                                                                    
                case 10://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }                       
                        break;
                case 11://关屏幕
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

void CreateSceneWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = SceneEventHandler;
        pWindow->NotifyReDraw = DrawSceneMain;
        pWindow->valid = 1;
        pWindow->type = JIAJU_CONTROL;
        WindowAddToWindowList(pWindow);      
        pWindow->NotifyReDraw2(pWindow);
}

static void DrawSceneStatus(char* text)
{
	 JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
	 JpegDisp(300,531,menu_sub_back4,sizeof(menu_sub_back4));
 	 DrawText(text,180,576,rgb_white);
 	 update_rect(130,531,600,60);	
}

void DrawSceneMain(){
        int xoffset = 0;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        
        JpegDisp(rc_scenemenuico[0].x,rc_scenemenuico[0].y,menu_huike1,sizeof(menu_huike1));
        JpegDisp(rc_scenemenuico[1].x,rc_scenemenuico[1].y,menu_jiucan1,sizeof(menu_jiucan1));
        JpegDisp(rc_scenemenuico[2].x,rc_scenemenuico[2].y,menu_yingyuan1,sizeof(menu_yingyuan1));
        JpegDisp(rc_scenemenuico[3].x,rc_scenemenuico[3].y,menu_jiuqin1,sizeof(menu_jiuqin1));
        JpegDisp(rc_scenemenuico[4].x,rc_scenemenuico[4].y,menu_wenxin1,sizeof(menu_wenxin1));
        

        JpegDisp(rc_scenemenuico[5].x,rc_scenemenuico[5].y,bt_zaijia1,sizeof(bt_zaijia1));
        JpegDisp(rc_scenemenuico[6].x,rc_scenemenuico[6].y,bt_lijia1,sizeof(bt_lijia1));
        JpegDisp(rc_scenemenuico[7].x,rc_scenemenuico[7].y,menu_yeqi1,sizeof(menu_yeqi1));
        JpegDisp(rc_scenemenuico[8].x,rc_scenemenuico[8].y,menu_chenqi1,sizeof(menu_chenqi1));
        JpegDisp(rc_scenemenuico[9].x,rc_scenemenuico[9].y,menu_zidingyi1,sizeof(menu_zidingyi1));        
          
        JpegDisp(rc_scenemenuico[10].x,rc_scenemenuico[10].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_scenemenuico[11].x,rc_scenemenuico[11].y,bt_guanping1,sizeof(bt_guanping1));  

        DrawEnText(LT("会客模式"),rc_scenemenuico[0].w+xoffset,rc_scenemenuico[0].h,rc_scenemenuico[0].x,rc_scenemenuico[0].y);
        DrawEnText(LT("就餐模式"),rc_scenemenuico[1].w+xoffset,rc_scenemenuico[1].h,rc_scenemenuico[1].x,rc_scenemenuico[1].y);
        DrawEnText(LT("影院模式"),rc_scenemenuico[2].w+xoffset,rc_scenemenuico[2].h,rc_scenemenuico[2].x,rc_scenemenuico[2].y);
        DrawEnText(LT("就寝模式"),rc_scenemenuico[3].w+xoffset,rc_scenemenuico[3].h,rc_scenemenuico[3].x,rc_scenemenuico[3].y);
        DrawEnText(LT("温馨模式"),rc_scenemenuico[4].w+xoffset,rc_scenemenuico[4].h,rc_scenemenuico[4].x,rc_scenemenuico[4].y);
        
        DrawEnText(LT("在家模式"),rc_scenemenuico[5].w+xoffset,rc_scenemenuico[5].h,rc_scenemenuico[5].x,rc_scenemenuico[5].y);
        DrawEnText(LT("离家模式"),rc_scenemenuico[6].w+xoffset,rc_scenemenuico[6].h,rc_scenemenuico[6].x,rc_scenemenuico[6].y);
        DrawEnText(LT("夜起模式"),rc_scenemenuico[7].w+xoffset,rc_scenemenuico[7].h,rc_scenemenuico[7].x,rc_scenemenuico[7].y);
        DrawEnText(LT("晨起模式"),rc_scenemenuico[8].w+xoffset,rc_scenemenuico[8].h,rc_scenemenuico[8].x,rc_scenemenuico[8].y);
        DrawEnText(LT("自定义模式"),rc_scenemenuico[9].w+xoffset,rc_scenemenuico[9].h,rc_scenemenuico[9].x,rc_scenemenuico[9].y);
}
int SceneEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        int rc = 0;
      
        xy.x = x;
        xy.y = y;

        for(index =0 ;index<12;index++){
                if(IsInside(rc_scenemenuico[index],xy)){
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
                        pos_x = rc_scenemenuico[index].x;
                        pos_y = rc_scenemenuico[index].y;              
                        switch(index){
                case 0: //会客
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_huike2,sizeof(menu_huike2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_huike1,sizeof(menu_huike1));

                                g_cur_set_scene_mode = SCENE_HUIKE;
                                g_isctrlelec = 1;
                                DrawSceneStatus(LT("正在控制'会客'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);                             
                                if(rc == 0){
#if HAVE_JIAJU_SAFE         
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)                       	
                                	start_defend(true); //启动防区布防 2009-7-21
#endif                                           
                                        ;

                                }else
                                        ;
                                g_isctrlelec = 0;
          


                        }                                                                   
                        break;
                case 1://就餐
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_jiucan2,sizeof(menu_jiucan2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_jiucan1,sizeof(menu_jiucan1)); 
                                
                                g_cur_set_scene_mode = SCENE_JIUCAN;
                                g_isctrlelec = 1;
                                DrawSceneStatus(LT("正在控制'就餐'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                           
                                if(rc == 0){
#if HAVE_JIAJU_SAFE                      
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)             	
                                        	start_defend(true); //启动防区布防    2009-7-21  
#endif                                                                   
                                        ;//DrawSceneStatus(LT("控制成功"));
                                }else
                                        ;//DrawSceneStatus(LT("控制失败"));
                                g_isctrlelec = 0;
                        }                       
                        break;  
                case 2://影院
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_yingyuan2,sizeof(menu_yingyuan2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_yingyuan1,sizeof(menu_yingyuan1));  
                              
                                g_isctrlelec = 1;                                
                                g_cur_set_scene_mode = SCENE_YINGYUAN;
                                DrawSceneStatus(LT("正在控制'影院'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                          
                                if(rc == 0){  
#if HAVE_JIAJU_SAFE                  
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)                 	 
                                        	start_defend(true); //启动防区布防    2009-7-21  									
#endif                                        
                                        ;
                                }else
                                        ;
                                g_isctrlelec = 0;

                        }                       
                        break;  
                case 3://就寝
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_jiuqin2,sizeof(menu_jiuqin2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_jiuqin1,sizeof(menu_jiuqin1)); 
                               
                                g_isctrlelec = 1;                                
                                g_cur_set_scene_mode  = SCENE_JIUQIN;
                                DrawSceneStatus(LT("正在控制'就寝'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                             
                                if(rc == 0){
#if HAVE_JIAJU_SAFE                       
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)            	
                                        	start_defend(true); //启动防区布防    2009-7-21  									 
#endif                                        
                                        ;//DrawSceneStatus(LT("控制成功"));
                                }else
                                        ;//DrawSceneStatus(LT("控制失败"));
                                g_isctrlelec = 0;                             
                        }                       
                        break;  
                case 4://温馨
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_wenxin2,sizeof(menu_wenxin2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_wenxin1,sizeof(menu_wenxin1)); 
                  
                                g_isctrlelec = 1;                                  
                                g_cur_set_scene_mode = SCENE_WENXIN;
                                DrawSceneStatus(LT("正在控制'温馨'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                               
                                if(rc == 0){
#if HAVE_JIAJU_SAFE        
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)                           	
                                        	start_defend(true); //启动防区布防    2009-7-21                                  
#endif                                        
                                        ;
                                }else
                                        ;
                                g_isctrlelec = 0;      
                        }                       
                        break;   
                case 5:
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,bt_zaijia2,sizeof(bt_zaijia2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,bt_zaijia1,sizeof(bt_zaijia1));  
                                g_isctrlelec = 1;               	
                                g_cur_set_scene_mode = SCENE_ZAIJIA;
                                DrawSceneStatus(LT("正在控制'在家'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                           
                                if(rc == 0){   
#if HAVE_JIAJU_SAFE                     
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)              	
                                		start_defend(true);                             
#endif                                	
                                        ;
                                }else
                                        ;

                                g_isctrlelec = 0;                                         
               		}
                	break;
                case 6:
						if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,bt_lijia2,sizeof(bt_lijia2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,bt_lijia1,sizeof(bt_lijia1)); 
                                g_isctrlelec = 1;   
                                g_cur_set_scene_mode = SCENE_LIJIA;
                                DrawSceneStatus(LT("正在控制'离家'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                             
                                if(rc == 0){ 
#if HAVE_JIAJU_SAFE                       
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)            	
                                		start_defend(true);                             
#endif                                  	                              
                                        ;
                                }else
                                        ;
                                g_isctrlelec = 0;                     	
                        }
                	break;
                case 7:
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_yeqi2,sizeof(menu_yeqi2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_yeqi1,sizeof(menu_yeqi1));
                                g_isctrlelec = 1;                 	
                                g_cur_set_scene_mode = SCENE_YEQI;
                                DrawSceneStatus(LT("正在控制'夜起'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);                             
                                if(rc == 0){  
#if HAVE_JIAJU_SAFE                         
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)          	
                                		start_defend(true);                             
#endif                                  	                             
                                        ;
                                }else
                                        ;
                                g_isctrlelec = 0;                   	
                        }
                	break;
                case 8:
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_chenqi2,sizeof(menu_chenqi2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_chenqi1,sizeof(menu_chenqi1));    
                                g_isctrlelec = 1;             	
                                g_cur_set_scene_mode = SCENE_CHENQI;
                                DrawSceneStatus(LT("正在控制'晨起'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                            
                                if(rc == 0){     
#if HAVE_JIAJU_SAFE                        
								if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)           	
                                		start_defend(true);                             
#endif                                  	                          
                                        ;
                                }else
                                        ;
                                g_isctrlelec = 0;
                        }                	
                	break;
                case 9:
                        if(status == 1){
                                StartButtonVocie();
                                
                                JpegDisp(pos_x,pos_y,menu_zidingyi2,sizeof(menu_zidingyi2));
                        }else{
                        		SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,menu_zidingyi1,sizeof(menu_zidingyi1));                   	
                                g_isctrlelec = 1;
                                g_cur_set_scene_mode = SCENE_USERDEFINE;
                                DrawSceneStatus(LT("正在控制'自定义'情景..."));
                                rc = send_scene_mode(g_cur_set_scene_mode);
                            
                                if(rc == 0){     
#if HAVE_JIAJU_SAFE                            
						if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)       	
                                		start_defend(true);                             
#endif                                  	                          
                                        ;
                                }else
                                        ;
                                g_isctrlelec = 0;                   	
                        }
                	break;                                                                    
                case 10://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }                       
                        break;
                case 11://关屏幕
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
        pSystemInfo->mySysInfo.cur_scene = g_cur_set_scene_mode;
        UpdateSystemInfo();
        return ret;                 
}

int GetCurSceneMode(){
        return g_cur_set_scene_mode;
}

////////////////////////////////////////////////////////////////
//室内控制快捷操作
int GetJiajuCount()
{
        int icount = 0;
        int i ;
        T_JIAJU_TABLE* pJiaju = g_JIAJU_TABLE;
        for(i =0; i<MAX_JIAJU_TABLE_COUNT;i++,pJiaju++){
                if(pJiaju->valid == 1){
                        icount++;
                }
        }
        return icount;
}





#define MAX_ELEC_COUNT 12

static unsigned char g_pre_select_room_index = 12; //当前页中前一个挑选的房间的索引
static unsigned char g_cur_select_room_index = 12;//当前页中当前挑选的房间的索引
static unsigned char g_cur_room_page = 0; //当前第几页
static unsigned char g_room_total_page = 0; //4个房间一页，总页数
static int g_room_total_count = 0; //房间总数
static unsigned char  g_cur_select_room_elec = 0 ; //当前挑选电器
static unsigned char  g_pre_select_room_elec = 0;//前一个挑选的电器
static unsigned char g_cur_room_elec_count = 0; //当前页电器总数

static unsigned char g_pre_select_ctrl_button = 16;
static unsigned char g_cur_select_ctrl_button = 16;

static unsigned char g_ctrl_status[6];

static unsigned int g_jiaju_text_offset = 15;

static unsigned int g_cur_status = 0xfd;

static unsigned char g_room[MAX_JIAJU_TABLE_COUNT][2];
static unsigned char g_room_elec[MAX_ELEC_COUNT]; //每一页分3行4列，总12个电器，


static void InitShortCut()
{
	g_pre_select_room_index = 12;
	g_cur_select_room_index = 12;
	g_cur_room_page = 0;
	g_room_total_page = 0;
	g_room_total_count = 0;
	g_cur_select_room_elec = 0;
	g_pre_select_room_elec  = 0;
	g_cur_room_elec_count = 0;
	g_cur_select_ctrl_button = 16;
	g_pre_select_ctrl_button = 16;
	
	memset(g_room,0xff,sizeof(g_room));
	memset(g_room_elec,0xff,sizeof(g_room_elec));
	memset(g_ctrl_status,0xff,sizeof(g_ctrl_status));
}
	

static int GetRoomCount(unsigned char roomsort,unsigned char roomname)
{
	int i = 0;
	int ret = 0;
	for(i =0 ;i<MAX_JIAJU_TABLE_COUNT;i++)
	{
		if(g_room[i][0] == roomsort && g_room[i][1] == roomname){
			ret = 1;
	                break;
	        }
	}
	if(ret == 0)
	{
		for(i =0 ;i<MAX_JIAJU_TABLE_COUNT;i++)
		{
			if(g_room[i][0] == 0xff && g_room[i][1] == 0xff){
				g_room[i][0] = roomsort;
				g_room[i][1] = roomname;
	                	break;
	        	}
		}
		return i+1;
		
	}else{
	        for(i =0 ;i<MAX_JIAJU_TABLE_COUNT;i++)
		{
			if(g_room[i][0] == 0xff && g_room[i][1] == 0xff){
	                	break;
	        	}
		}
		return i;	
	}
	return 0;
}

static int GetJiajuRoomCount()
{
        int icount = 0;
        int i ;
        memset(g_room,0xff,sizeof(g_room));
        T_JIAJU_TABLE* pJiaju = g_JIAJU_TABLE;
        for(i =0; i<MAX_JIAJU_TABLE_COUNT;i++,pJiaju++){
                if(pJiaju->valid == 1){
                        icount = GetRoomCount(pJiaju->JiajuID.roomsort,pJiaju->JiajuID.roomname);
                }
        }
        return icount;	
}

static const WRT_Rect rc_roomcontrolico2[27]={

	{80,120,130,100},
	{210,120,130,100},
	{340,120,130,100},
	{470,120,130,100},
	{80,240,130,100},
	{210,240,130,100},
	{340,240,130,100},
	{470,240,130,100},
	{80,360,130,100},
	{210,360,130,100},
	{340,360,130,100},
	{470,360,130,100},
	
	{193,38,168,48}, //房间位置
	{365,38,168,48},
	{0,0,0,0},
	{0,0,0,0},	
	
        {774,284,91,55},   //控制button;
        {877,284,91,55}, 
        {774,350,91,55},   
        {877,350,91,55}, 
        {774,416,91,55}, 
        {877,416,91,55},  
          
         
        

	{107,38,66,49}, //上箭头
	{537,38,66,49}, //下箭头
	{763,64,217,57},
     
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕                       
};

static void DrawCurPageRoomName()
{
	int itmp = (g_cur_room_page-1)*2;
	int i = itmp;
	int j = 0;
	unsigned char x ,y;
	x =y = 0;
	
	for(i = itmp;i<(itmp+2);i++){
		JpegDisp(rc_roomcontrolico2[(i-itmp)+12].x,rc_roomcontrolico2[(i-itmp)+12].y,menu_jiaju_fangjian1,sizeof(menu_jiaju_fangjian1));		
		if((g_room[i][0] != 0xff) &&(g_room[i][1] != 0xff)){
			x = g_room[i][0];
			y = g_room[i][1];
			if(j == 0){
				g_pre_select_room_index = g_cur_select_room_index;
				g_cur_select_room_index = 12;
				JpegDisp(rc_roomcontrolico2[j+12].x,rc_roomcontrolico2[j+12].y,menu_jiaju_fangjian2,sizeof(menu_jiaju_fangjian2));
			}
			DrawTextToRect3(rc_roomcontrolico2[j+12],LT(g_ROOMNAME[x][y].name),rgb_white,1);
			j++;
		}
	}
}

static void DrawCurSelectRoomName()
{
	int itmp = (g_cur_room_page-1)*2;
	int i = itmp;
	int j = 0;
	unsigned char x ,y;
	x =y = 0;
	i += (g_cur_select_room_index - 12);
	if((g_room[i][0] != 0xff) &&(g_room[i][1] != 0xff)){
		x = g_room[i][0];
		y = g_room[i][1];
		DrawTextToRect3(rc_roomcontrolico2[g_cur_select_room_index],LT(g_ROOMNAME[x][y].name),rgb_white,1);
	}
	
	i = (g_pre_select_room_index - 12)+itmp;
	if((g_room[i][0] != 0xff) &&(g_room[i][1] != 0xff)){
		x = g_room[i][0];
		y = g_room[i][1];
		DrawTextToRect3(rc_roomcontrolico2[g_pre_select_room_index],LT(g_ROOMNAME[x][y].name),rgb_white,1);
	}	

}

static int AddElecIndexToList(int index)
{
	int i = 0;
	for(i = 0; i< MAX_ELEC_COUNT; i++)
	{
		if(g_room_elec[i] == 0xff){
			g_room_elec[i] = (unsigned char)(index & 0xff);
			
			return i+1;
		}
	}
	if(i == MAX_ELEC_COUNT)
		return MAX_ELEC_COUNT;
	return 0;
}

static void GetCurRoomElecIndex() 
{
	int itmp = (g_cur_room_page-1)*2;
	int i = itmp;
	int j = 0;
	unsigned char x ,y;
	x =y = 0;
	i += (g_cur_select_room_index - 12);
	memset(g_room_elec,0xff,sizeof(g_room_elec));
	if((g_room[i][0] != 0xff) &&(g_room[i][1] != 0xff)){
		x = g_room[i][0]; 
		y = g_room[i][1];	
 		T_JIAJU_TABLE* pJiaju = g_JIAJU_TABLE;
       		 for(i =0; i<MAX_JIAJU_TABLE_COUNT;i++,pJiaju++){
                	if((pJiaju->valid == 1) 
                		&& (pJiaju->JiajuID.roomsort == x) 
                		&& (pJiaju->JiajuID.roomname == y) ){
                		if((g_cur_room_elec_count = AddElecIndexToList(i)) == 0) //如果一个房间超过12个电器暂不显示。
                			return;
                	}
       		 }		
	}	
}

static unsigned char* SelectImage(T_JIAJU_TABLE* pJiaju,unsigned int* size,int status)
{
	if(pJiaju == NULL)
		return NULL;
	switch(pJiaju->JiajuID.elecsort)
	{
		case 0:
			if(status == 1){
				*size = sizeof(menu_jiaju_light1);
				return menu_jiaju_light1;
			}else{
				*size = sizeof(menu_jiaju_light2);
				return menu_jiaju_light2;				
			}
			break;
		case 1:
			if(status == 1){
				*size = sizeof(menu_jiaju_curtian1);
				return menu_jiaju_curtian1;
			}else{
				*size = sizeof(menu_jiaju_curtian2);
				return menu_jiaju_curtian2;				
			}			
			break;
		case 2:
			if(status == 1){
				*size = sizeof(menu_jiaju_tv1);
				return menu_jiaju_tv1;
			}else{
				*size = sizeof(menu_jiaju_tv2);
				return menu_jiaju_tv2;				
			}			
			break;
		case 3:
			if(status == 1){
				*size = sizeof(menu_jiaju_air1);
				return menu_jiaju_air1;
			}else{
				*size = sizeof(menu_jiaju_air2);
				return menu_jiaju_air2;				
			}			
			break;
		case 4:
			if(status == 1){
				*size = sizeof(menu_jiaju_dvd1);
				return menu_jiaju_dvd1;
			}else{
				*size = sizeof(menu_jiaju_dvd2);
				return menu_jiaju_dvd2;				
			}			
			break;
		case 5:
			if(status == 1){
				*size = sizeof(menu_jiaju_jack1);
				return menu_jiaju_jack1;
			}else{
				*size = sizeof(menu_jiaju_jack2);
				return menu_jiaju_jack2;				
			}			
			break;
		case 6:
			if(status == 1){
				*size = sizeof(menu_jiaju_inductor1);
				return menu_jiaju_inductor1;
			}else{
				*size = sizeof(menu_jiaju_inductor2);
				return menu_jiaju_inductor2;				
			}			
			break;
		case 7:
			if(status == 1){
				*size = sizeof(menu_jiaju_user1);
				return menu_jiaju_user1;
			}else{
				*size = sizeof(menu_jiaju_user2);
				return menu_jiaju_user2;				
			}			
			break;
	}
	*size = 0;
	return NULL;
}

static void DrawCurRoomElec()
{
	T_JIAJU_TABLE* pJiaju = g_JIAJU_TABLE;
	int i = 0;
	unsigned char* tmpimage = NULL;
	unsigned int size =0;
	WRT_Rect rt;
	GetCurRoomElecIndex();
	JpegDisp(80,112,menu_jiaju_back2,sizeof(menu_jiaju_back2));
	for(i = 0; i < MAX_ELEC_COUNT; i++){
		if(g_room_elec[i] != 0xff){
			pJiaju = &g_JIAJU_TABLE[g_room_elec[i]];
			if(pJiaju->valid == 0)
				continue;
			tmpimage = NULL;
			size = 0;
			tmpimage = SelectImage(pJiaju,&size,1);
			if(tmpimage){	
				JpegDispRectCenter(rc_roomcontrolico2[i],tmpimage,size);
				rt.x = rc_roomcontrolico2[i].x;
				rt.y = rc_roomcontrolico2[i].y+rc_roomcontrolico2[i].h-g_jiaju_text_offset;
				rt.w = rc_roomcontrolico2[i].w;
				rt.h = 20;
				DrawTextToRect3(rt,LT(g_ELECTNAME[pJiaju->JiajuID.elecsort][pJiaju->JiajuID.elecname].name),rgb_black,1);
			}
		}
	}
}

static void GetCurElecStatusText(char* text,int ret)
{
	if(ret == 0){
		if(g_cur_status == 0xffffffff || g_cur_status == 0xfd ){ //状态未知
			sprintf(text,"%s %s",LT("当前状态:"),LT("未知"));
		}else if(g_cur_status == 0){
			sprintf(text,"%s %s",LT("当前状态:"),LT("关"));
		}else if(g_cur_status == 1){
			sprintf(text,"%s %s",LT("当前状态:"),LT("开"));
		}
		return;
	}
	if(g_cur_status == 0xffffffff ||  g_cur_status == 0xfd ){
		sprintf(text,"%s %s",LT("当前状态:"),LT("未知"));
		return;
		
	}
	
	ret -= 0x1;
	if(ret == 0x0){ //温度
		sprintf(text,"%s %d℃",LT("当前状态:"),g_cur_status);
	}else if( ret == 0x01){ //湿度
		sprintf(text,"%s %d%%",LT("当前状态:"),g_cur_status);
	}else if(ret == 0x05){//空气
		sprintf(text,"%s %dPPM",LT("当前状态:"),g_cur_status);
	}else if (ret == 0x06){//空气
		sprintf(text,"%s %dPPM",LT("当前状态:"),g_cur_status);
	}else if (ret == 0x7){//风力
	}else if(ret == 0x8){//雨量
	}
		
		
	
}

static void  UpdateElecStatus(int ret,unsigned long color)
{
	WRT_Rect rt;
	char text[50];
	memset(text,0,50);
	GetCurElecStatusText(text,ret);
	JpegDisp(763,64,menu_jiaju_back3,sizeof(menu_jiaju_back3));
	rt.x = 763;
	rt.y= 64;
	rt.w = 217;
	rt.h = 30;
	DrawTextToRect3(rt,text,color,1);	
}

static void UpdateSelectElec(int update)
{
	unsigned char* pTmpImage = NULL;
	unsigned int size = 0;

	
	WRT_Rect rt;
	int i = g_room_elec[g_pre_select_room_elec];
	T_JIAJU_TABLE* pJiaju = &g_JIAJU_TABLE[i];
	if(pJiaju->valid == 0)
		return;
	pTmpImage = SelectImage(pJiaju,&size,1);
	if(pTmpImage){
		JpegDispRectCenter(rc_roomcontrolico2[g_pre_select_room_elec],pTmpImage,size);
		rt.x = rc_roomcontrolico2[g_pre_select_room_elec].x;
		rt.y = rc_roomcontrolico2[g_pre_select_room_elec].y+rc_roomcontrolico2[g_pre_select_room_elec].h - g_jiaju_text_offset;
		rt.w = rc_roomcontrolico2[g_pre_select_room_elec].w;
		rt.h = 20;		
		DrawTextToRect3(rt,LT(g_ELECTNAME[pJiaju->JiajuID.elecsort][pJiaju->JiajuID.elecname].name),rgb_black,1);	
	}
	
	
	i = g_room_elec[g_cur_select_room_elec];
	pJiaju = &g_JIAJU_TABLE[i];
	if(pJiaju->valid == 0)
		return;
	pTmpImage = NULL;
	size = 0;
	pTmpImage = SelectImage(pJiaju,&size,2);
	if(pTmpImage){
		JpegDispRectCenter(rc_roomcontrolico2[g_cur_select_room_elec],pTmpImage,size);
		rt.x = rc_roomcontrolico2[g_cur_select_room_elec].x;
		rt.y = rc_roomcontrolico2[g_cur_select_room_elec].y+rc_roomcontrolico2[g_cur_select_room_elec].h - g_jiaju_text_offset;
		rt.w = rc_roomcontrolico2[g_cur_select_room_elec].w;
		rt.h = 20;			
		DrawTextToRect3(rt,LT(g_ELECTNAME[pJiaju->JiajuID.elecsort][pJiaju->JiajuID.elecname].name),0x00ff8000,1);
	}
	/*
	JpegDisp(110,424,menu_sub_back4,sizeof(menu_sub_back4));
 	DrawText(LT("获取状态中..."),150,460,rgb_white);
	if(pJiaju->JiajuID.elecsort == 6){
		g_cur_status = Jiaju_QueryStatus(pJiaju->RfID.RFNo,pJiaju->RfID.SubSw,pJiaju->JiajuID.elecname);
		UpdateElecStatus(pJiaju->JiajuID.elecname+0x1);
	}else{
		g_cur_status = Jiaju_GetStatus(pJiaju);
		UpdateElecStatus(0);
	}
	JpegDisp(110,424,menu_sub_back4,sizeof(menu_sub_back4));
	*/
}

static void DisplayAfterInit()
{
	T_JIAJU_TABLE* pJiaju;
	int i=0;
	g_isctrlelec = 1;
	i = g_room_elec[g_cur_select_room_elec];
	pJiaju = &g_JIAJU_TABLE[i];
	
	if(pJiaju->valid == 0){
		g_isctrlelec = 0;
		return;
	}
	
	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));;
 	DrawText(LT("获取状态中..."),150,570,rgb_white);
	if(pJiaju->JiajuID.elecsort == 6){
		g_cur_status = Jiaju_QueryStatus(pJiaju->RfID.RFNo,pJiaju->RfID.SubSw,pJiaju->JiajuID.elecname);
		UpdateElecStatus(pJiaju->JiajuID.elecname+0x1,rgb_white);
	}else{
		g_cur_status = Jiaju_GetStatus(pJiaju);
		UpdateElecStatus(0,rgb_white);
	}
	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));;
	WRT_DEBUG("DisplayAfterInit g_cur_status =%x",g_cur_status);
	g_isctrlelec = 0;
		
}

static void DrawControlButton(int update)
{
        int i = 0;
        char text[64];
        WRT_Rect rt;
        i = g_room_elec[g_cur_select_room_elec];
        T_JIAJU_TABLE* pJiaju = &g_JIAJU_TABLE[i];

        if(pJiaju == NULL){
		for(i = 0; i<6;i++){
		        rt = rc_roomcontrolico2[i+16];
		        DeflateRect(&rt,4);
		        FillRect(rgb_black,rt);
			//JpegDispRectCenter(rc_roomcontrolico2[i+16],menu_jiaju_button1,sizeof(menu_jiaju_button1));  
        	}

                return;
        }
        if(pJiaju->valid != 1){
		for(i = 0; i<6;i++){
			//JpegDispRectCenter(rc_roomcontrolico2[i+16],menu_jiaju_button1,sizeof(menu_jiaju_button1));  
			 rt = rc_roomcontrolico2[i+16];
		        DeflateRect(&rt,4);
		        FillRect(rgb_black,rt);
        	} 
	
        	return;        
	}        
	if(pJiaju->JiajuID.elecsort == 6){ //感应器，查询状态显示出来。
		for(i = 0; i<6;i++){
			//JpegDispRectCenter(rc_roomcontrolico2[i+16],menu_jiaju_button1,sizeof(menu_jiaju_button1));  
			 rt = rc_roomcontrolico2[i+16];
		        DeflateRect(&rt,4);
		        FillRect(rgb_black,rt);
        	}

		return ;
	}
	memset(g_ctrl_status,0xff,sizeof(g_ctrl_status));	
        for(i = 0; i<6;i++)
        {
                memset(text,0,64);
                g_ctrl_status[i] = get_select_control_text(pJiaju,i,text);
                //JpegDispRectCenter(rc_roomcontrolico2[i+16],menu_jiaju_button1,sizeof(menu_jiaju_button1)); 
                rt = rc_roomcontrolico2[i+16];
		DeflateRect(&rt,4);
		FillRect(rgb_black,rt);              
                DrawTextToRect3(rc_roomcontrolico2[i+16],text,rgb_white,0);
        }
        JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));

        return;
}

static int UpdateControlButton(int update)
{
        int i = 0;
        char text[64];
        WRT_Rect rt;
        i = g_room_elec[g_cur_select_room_elec];
        T_JIAJU_TABLE* pJiaju = &g_JIAJU_TABLE[i];
        if(pJiaju == NULL)
                return 1;
        if(pJiaju->valid != 1)
        	return 1;                
	if(pJiaju->JiajuID.elecsort == 6){ //感应器，查询状态显示出来。
		return 1;
	}
	if(update == 1)
                        g_isUpdated = 0;
	i = g_pre_select_ctrl_button - 16;
                memset(text,0,64);
	get_select_control_text(pJiaju,i,text);
	rt = rc_roomcontrolico2[i+16];
	DeflateRect(&rt,4);
	FillRect(rgb_black,rt);  
	//JpegDispRectCenter(rc_roomcontrolico2[i+16],menu_jiaju_button1,sizeof(menu_jiaju_button1));               
	DrawTextToRect3(rc_roomcontrolico2[i+16],text,rgb_white,0);
		
	i = g_cur_select_ctrl_button - 16;
                memset(text,0,64);
	get_select_control_text(pJiaju,i,text);
	rt = rc_roomcontrolico2[i+16];
	DeflateRect(&rt,4);
	FillRect(rgb_white,rt);  	
	//JpegDispRectCenter(rc_roomcontrolico2[i+16],menu_jiaju_button2,sizeof(menu_jiaju_button2));               
	DrawTextToRect3(rc_roomcontrolico2[i+16],text,rgb_black,0);
	if(update == 1){
	        update_rect(775,284,193,188);
	        g_isUpdated = 1;
	        
	}
        return 0;
}

static void ProcessCtrlButton(int index)
{
	int i = 0;
	g_isctrlelec = 1;
        i = g_room_elec[g_cur_select_room_elec];
        T_JIAJU_TABLE* pJiaju = &g_JIAJU_TABLE[i];
        if(pJiaju == NULL){
        	g_isctrlelec = 0;
                return;
        }
        if(pJiaju->valid != 1){
        	g_isctrlelec = 0;
        	return;        
        }        
	if(pJiaju->JiajuID.elecsort == 6){ //感应器，查询状态显示出来。
		g_isctrlelec = 0;
		return ;
	}             
 	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
 	DrawText(LT("控制家居中..."),150,570,rgb_white);
	if(Jiaju_SetStatus(pJiaju, g_ctrl_status[index-16]) == 0){
		JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
		if(g_ctrl_status[index-16] == C_CLOSE)
			g_cur_status = 0;
		else if(g_ctrl_status[index-16] == C_OPEN)
			g_cur_status = 1;
		DrawText(LT("控制成功"),150,570,rgb_white);
		UpdateElecStatus(0,rgb_white);
	}else{
		JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
		DrawText(LT("控制失败"),150,570,rgb_white);
		g_cur_status = 0x2;
		UpdateElecStatus(0,rgb_white);
	}
	g_isctrlelec = 0;
}

void CreateShortcutRoomControl2()
{
	
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;  
		InitShortCut();                
        int itemp = GetJiajuRoomCount(); 
        g_room_total_count = itemp;
        g_room_total_page =  itemp / 2;
        if((itemp %2) > 0){
        	g_room_total_page++;
        }
        if(g_room_total_page == 0)
        	g_room_total_page = 1;
        g_cur_room_page = 1;

        pWindow->EventHandler = ShortcutRoomControlEventHandler2;
        pWindow->NotifyReDraw = DrawShortcutRoomControl2;
        pWindow->DisplayAfterInit = NULL;//DisplayAfterInit;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}


void DrawShortcutRoomControl2()
{

	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	//JpegDisp(0,0,menu_jiaju_back,sizeof(menu_jiaju_back));
	JpegDisp(80,112,menu_jiaju_back2,sizeof(menu_jiaju_back2));
	JpegDisp(763,64,menu_jiaju_back3,sizeof(menu_jiaju_back3));
	JpegDisp(rc_roomcontrolico2[12].x-4,rc_roomcontrolico2[12].y,menu_jiaju_back4,sizeof(menu_jiaju_back4));
	JpegDisp(rc_roomcontrolico2[23].x-4,rc_roomcontrolico2[23].y,menu_jiaju_back4,sizeof(menu_jiaju_back4));
	JpegDisp(rc_roomcontrolico2[13].x-4,rc_roomcontrolico2[13].y,menu_jiaju_back4,sizeof(menu_jiaju_back4));
	
	JpegDisp(rc_roomcontrolico2[22].x,rc_roomcontrolico2[22].y,menu_jiaju_shangfan1,sizeof(menu_jiaju_shangfan1));
	JpegDisp(rc_roomcontrolico2[23].x,rc_roomcontrolico2[23].y,menu_jiaju_xiafan1,sizeof(menu_jiaju_xiafan1));
	
    JpegDisp(rc_roomcontrolico2[25].x,rc_roomcontrolico2[25].y,bt_fanhui1,sizeof(bt_fanhui1));
    JpegDisp(rc_roomcontrolico2[26].x,rc_roomcontrolico2[26].y,bt_guanping1,sizeof(bt_guanping1)); 
    DrawCurPageRoomName();
    DrawCurRoomElec();      
    DrawControlButton(0);
    UpdateSelectElec(0);


}
int ShortcutRoomControlEventHandler2(int x,int y,int status )
{
        int ret = 0;
        int i =0;
        WRT_xy xy;
         
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;

        for(index =0 ;index<27;index++){
                if(IsInside(rc_roomcontrolico2[index],xy)){
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
                        pos_x = rc_roomcontrolico2[index].x;
                        pos_y = rc_roomcontrolico2[index].y;    

                        switch(index)
                        {
			case 13:
                        case 14:
                        case 12:
                        case 15:
                        	if(((g_cur_room_page-1)*2 + (index-12)+1) > g_room_total_count)
                        		break;
				if(status == 1){
                                        StartButtonVocie();
                                        g_pre_select_room_index = g_cur_select_room_index;
                                        g_cur_select_room_index = index;
                                        g_isUpdated = 0;
                                        JpegDisp(rc_roomcontrolico2[g_pre_select_room_index].x,rc_roomcontrolico2[g_pre_select_room_index].y,menu_jiaju_fangjian1,sizeof(menu_jiaju_fangjian1));
                                        JpegDisp(pos_x,pos_y,menu_jiaju_fangjian2,sizeof(menu_jiaju_fangjian2));
                                        g_pre_select_room_elec = g_cur_select_room_elec = 0;
                                       
                                        DrawCurSelectRoomName();
                                        DrawCurRoomElec();
                                        update_rect(0,0,1024,600);
                                        g_isUpdated = 1;

                                }else{
                                      //  JpegDisp(pos_x,pos_y,menu_jiaju_fangjian1,sizeof(menu_jiaju_fangjian1));
                                }  
                                   
                        	break;
                        case 16: 
                        case 17:
                        case 18:
                        case 19:
                        case 20:
                        case 21:
				if(status == 1){					
                                        //StartButtonVocie();
                                 //       JpegDisp(pos_x,pos_y,menu_jiaju_button2,sizeof(menu_jiaju_button2));
                               // }else{
                                       // JpegDisp(pos_x,pos_y,menu_jiaju_button1,sizeof(menu_jiaju_button1));
                                        g_pre_select_ctrl_button = g_cur_select_ctrl_button;
                                        g_cur_select_ctrl_button = index;
                                        if(UpdateControlButton(1) == 0){
                                        	StartButtonVocie();
                                        	ProcessCtrlButton(index);
                                        }
                                }                          	
                        	break;
                        case 22: 
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_jiaju_shangfan2,sizeof(menu_jiaju_shangfan2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_jiaju_shangfan1,sizeof(menu_jiaju_shangfan1));
                                        g_cur_room_page--;
                                        if(g_cur_room_page < 1)
                                        	g_cur_room_page = 1;
                                        g_pre_select_room_elec = g_cur_select_room_elec = 0;
                                        
                                        g_isUpdated = 0;
                                        DrawCurPageRoomName();
                                        DrawCurRoomElec();
                                        update_rect(80,0,700,531);
                                        g_isUpdated = 1;

                                }  
                                                          	
                                break;   
                        case 23:


                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_jiaju_xiafan2,sizeof(menu_jiaju_xiafan2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_jiaju_xiafan1,sizeof(menu_jiaju_xiafan1));
                                        g_cur_room_page++;
                                        if(g_cur_room_page > g_room_total_page)
                                        	g_cur_room_page = 1;
                                        g_pre_select_room_elec = g_cur_select_room_elec = 0;                                  	
                                        g_isUpdated = 0;
                                        DrawCurPageRoomName(); 
                                        DrawCurRoomElec(); 
                                        update_rect(80,0,700,531);
                                        g_isUpdated = 1;
                                                                            
                                }  
                                          
                                break;
                        case 24:
                        	if(status == 1){
                                        StartButtonVocie();
                                        UpdateElecStatus(0,rgb_yellow);
                                }else{
                                	DisplayAfterInit();
                                	
                                }
                        	break;
                        case 25: //返回
                        	
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                        CloseTopWindow();
                                }                       

                                break;
                        case 26: //关屏
                                if(status == 1){ 
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
                                        screen_close(); 
                                        CloseAllWindows();
                                }
                                break;
                        case 0:
                        case 1:
                        case 2:
                        case 3:
                        case 4:
                        case 5:
                        case 6:
                        case 7:
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                        	if((index + 1) > g_cur_room_elec_count)
                        		break;                          		
                        	if(status == 1){
                        	                WRT_Rect rt;
                        		g_pre_select_room_elec = g_cur_select_room_elec;
                        		g_cur_select_room_elec = index;
                        		StartButtonVocie();
                        		g_isUpdated = 0;
                        		DrawControlButton(0);
                        		update_rect(775,284,193,188);
                        		UpdateSelectElec(0);
                                                update_rect(80,112,595,385);
                        		g_isUpdated =1;              		
                        		//DisplayAfterInit();

                        		
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
