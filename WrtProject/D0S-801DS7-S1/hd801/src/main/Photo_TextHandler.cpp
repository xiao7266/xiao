#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "photo_text_res.h"

#include "window.h"

#define REC_PATH "/home/wrt/pic"
extern int g_rec_flag;
extern int g_have_rec_video ;
extern int g_have_cap_photo ;

#define PIC_INFO_ENABLE 0
//extern bool g_rc_status[9];
extern char *m_DevName;
WRT_Rect rc_photo_text[6] ={
        {150,175,132,160},             //������¼
        {350,175,132,160},             //��������
#if  PIC_INFO_ENABLE
        {550,175,132,160},             //ͼƬ        
		{750,175,132,160},			   //����
#else		
		{550,175,132,160},			   //����
#endif
		
        {900,531,111,69},              //����
        {6,531,111,69}                 //����Ļ	
};

int  InfoEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<6;index++){
                if(IsInside(rc_photo_text[index],xy)){
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
                        pos_x = rc_photo_text[index].x;
                        pos_y = rc_photo_text[index].y;      
		
                        switch(index){
                case 0: //������¼
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_baojingjilu2,sizeof(menu_baojingjilu2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_baojingjilu1,sizeof(menu_baojingjilu1));
                                CreateWarningLog();
                        }               	                                            
                        break;
                case 1://��������
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_yuyinxinxiang2,sizeof(menu_yuyinxinxiang2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_yuyinxinxiang1,sizeof(menu_yuyinxinxiang1));  
                                CreateVoiceWin();              			
                        }                   	
                        break;
#if  PIC_INFO_ENABLE
					
                case 2://ͼƬ
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_tupian2,sizeof(menu_tupian2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_tupian1,sizeof(menu_tupian1));
                        		m_DevName = REC_PATH;
                        		g_rec_flag =1;
                                CreateMutilmediaSub();
                                g_have_rec_video = 0;
								g_have_cap_photo = 0;
                        }                   	
                        break;          

                case 3://����
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_duanxin2,sizeof(menu_duanxin2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_duanxin1,sizeof(menu_duanxin1));
                                CreateMsgWin();
                        }                   	
                        break;                	
                case 4://����
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }                   	
                        break;
                case 5://����Ļ
                        if(status == 1){ //����
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
                                screen_close(); 
                                CloseAllWindows();
                        }
                        break;
#else
				case 2://����
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_duanxin2,sizeof(menu_duanxin2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_duanxin1,sizeof(menu_duanxin1));
                                CreateMsgWin();
                        }                   	
                        break;                	
                case 3://����
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }                   	
                        break;
                case 4://����Ļ
                        if(status == 1){ //����
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

void CreateInfoWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;  
        pWindow->EventHandler = InfoEventHandler;
        pWindow->NotifyReDraw = DrawInfoMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);        
        pWindow->NotifyReDraw2(pWindow);
}



void DrawInfoMain(){
        int xoffset = 0;
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        JpegDisp(rc_photo_text[0].x,rc_photo_text[0].y,menu_baojingjilu1,sizeof(menu_baojingjilu1));
        JpegDisp(rc_photo_text[1].x,rc_photo_text[1].y,menu_yuyinxinxiang1,sizeof(menu_yuyinxinxiang1));
#if PIC_INFO_ENABLE		
        JpegDisp(rc_photo_text[2].x,rc_photo_text[2].y,menu_tupian1,sizeof(menu_tupian1));
		JpegDisp(rc_photo_text[3].x,rc_photo_text[3].y,menu_duanxin1,sizeof(menu_duanxin1));	        
        JpegDisp(rc_photo_text[4].x,rc_photo_text[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_photo_text[5].x,rc_photo_text[5].y,bt_guanping1,sizeof(bt_guanping1));
#else
		JpegDisp(rc_photo_text[2].x,rc_photo_text[2].y,menu_duanxin1,sizeof(menu_duanxin1));	        
        JpegDisp(rc_photo_text[3].x,rc_photo_text[3].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_photo_text[4].x,rc_photo_text[4].y,bt_guanping1,sizeof(bt_guanping1));
#endif		


        DrawEnText(LT("������¼"),rc_photo_text[0].w,rc_photo_text[0].h,rc_photo_text[0].x,rc_photo_text[0].y);
        DrawEnText(LT("��������"),rc_photo_text[1].w,rc_photo_text[1].h,rc_photo_text[1].x,rc_photo_text[1].y);
#if PIC_INFO_ENABLE

        DrawEnText(LT("ͼƬ"),rc_photo_text[2].w,rc_photo_text[2].h,rc_photo_text[2].x,rc_photo_text[2].y);
		DrawEnText(LT("����Ϣ"),rc_photo_text[3].w,rc_photo_text[3].h,rc_photo_text[3].x,rc_photo_text[3].y);
#else		
		DrawEnText(LT("����Ϣ"),rc_photo_text[2].w,rc_photo_text[2].h,rc_photo_text[2].x,rc_photo_text[2].y);
#endif
}


