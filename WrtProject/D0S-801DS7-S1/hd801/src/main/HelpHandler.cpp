#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "window.h"
#include "c_interface.h"
#include "info_list_handler.h"


static int  m_help_page = 1;
static int  m_cur_help_page = 1;

static unsigned char* bimage[16] ={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static  int   bimagesize[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int image_count = 0;

/*帮助浏览*/
static const WRT_Rect rc_helpbrowser[4] = 
{
        {660,531,111,69},           /*上翻*/
        {780,531,111,69},            /* 下翻*/

        {900,531,111,69},              //返回
        {4,531,111,69}                 //关屏幕			
};	


int read_image(){

        char tmpname[10];
        int i=0;
        int res=0;
        for(i =0; i< 6;i++){
                if(bimage[i]  != NULL){
                        res++;
                        continue;
                }
                memset(tmpname,0,10);
                sprintf(tmpname,"pic%d",i);

                bimagesize[i] = 0;	

                bimage[i] = (unsigned char*)read_tmppic(&bimagesize[i],tmpname);
                WRT_DEBUG("bimage[i] = 0x%x ,%d",bimage[i],bimagesize[i]);
                if(bimage[i] != NULL)
                        res++;
        }
        image_count = res;
        return res;
}


void free_image(){
        int i =0;
        for(i =0; i< 16;i++){
                if(bimage[i] != NULL){
                        ZENFREE(bimage[i]);
                        bimage[i] = NULL;
                        bimagesize[i] = 0;
                }
        }
}

static void DrawHelpInfo(){
        WRT_Rect curbox;
        char text[72];
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 1024;
        curbox.h = 531; 

        FillRect(0xb2136,curbox); 
 
       // SDL_Delay(20);   
        if(image_count == 0){         
                DrawText_16(LT("暂无帮助信息，正请求中心配置..."),10,70,rgb_white);
        }
        else{
                if(bimage[m_cur_help_page-1] != NULL){
                        JpegDispCenter(0,0,bimage[m_cur_help_page-1],bimagesize[m_cur_help_page-1],1024);
                        //JpegDisp(0,0,bimage[m_cur_help_page-1],bimagesize[m_cur_help_page-1]);
                }
                DrawStatusbar(m_help_page,m_cur_help_page);
        }



}

void CreateHelpWin(){
        int i,len;
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = HelpEventHandler;
        pWindow->NotifyReDraw = DrawHelpMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        free_image();
        image_count = read_alltempimage(bimage,bimagesize);
        i = len =0;
        for(i=0;i<image_count;i++)
                len +=bimagesize[i];
        // if(image_count == 0){
        unsigned long tmpevent[4];
        tmpevent[0] = ROOM_INQHELPINFO;
        tmpevent[1] = len;
        tmpevent[2] = tmpevent[3] =0;
        wrthost_send_cmd(tmpevent);
        //}
        m_cur_help_page = 1;
        if(image_count > 1){
                m_help_page = image_count;
        }else
                m_help_page = 1;
        pWindow->NotifyReDraw2(pWindow);    	
}
void DrawHelpMain(){
        WRT_DEBUG("DrawHelpMain \n");
        if(image_count == 0){
                image_count = read_alltempimage(bimage,bimagesize);
                if(image_count > 1)
                        m_help_page = image_count;
        } 		
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));  
        if(m_help_page > 1)
                JpegDisp(rc_helpbrowser[0].x,rc_helpbrowser[0].y,menu_shangfan1,sizeof(menu_shangfan1));

        if(m_help_page > 1)
                JpegDisp(rc_helpbrowser[1].x,rc_helpbrowser[1].y,menu_xiafan1,sizeof(menu_xiafan1));          
        JpegDisp(rc_helpbrowser[2].x,rc_helpbrowser[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_helpbrowser[3].x,rc_helpbrowser[3].y,bt_guanping1,sizeof(bt_guanping1));
        DrawHelpInfo();
}
int HelpEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rc_helpbrowser[index],xy)){
                        pos_x = rc_helpbrowser[index].x;
                        pos_y = rc_helpbrowser[index].y;
                        ret = 1;
                        switch(index){
                case 0: /*上翻*/  
                        if(m_help_page == 1)        
                                break;                  	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));                     	       
                                m_cur_help_page--;
                                if(m_cur_help_page < 1)
                                        m_cur_help_page = 1;
                                DrawHelpInfo();  
                        }                 
                        break;                
                case 1:/*下翻*/
                        if(m_help_page == 1)        
                                break;                  	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                  	
                                m_cur_help_page++;
                                if(m_cur_help_page > m_help_page)
                                        m_cur_help_page =  m_help_page; 
                                DrawHelpInfo();  
                        }                                                                                         
                        break;                                    
                case 2: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                unsigned long tmp_event[4];
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));  
                                CloseTopWindow();
                                WRT_DEBUG("send CUSTOM_CMD_CANCEL");
                                tmp_event[0] = CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_INQHELPINFO;
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