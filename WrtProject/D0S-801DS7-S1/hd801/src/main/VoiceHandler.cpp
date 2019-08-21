#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "voice_ico_res.h"

#include "window.h"


extern  WRT_Rect rc_infobrowsermenuico[17];

/*
static const WRT_Rect rc_voicebrowsermenuico[16]={
{32,50,695,86},
{32,90,695,126},
{32,130,695,166},
{32,170,695,206},
{32,210,695,246},
{32,250,695,286},
{32,290,695,326},
{32,330,695,366},
{32,370,695,406},   

{39,426,159,474},                
{448,426,560,474},           
{576,426,688,474},           
{741,230,800,287},                   

{741,312,798,392},           
{741,397,800,480},         
{741, 40,800,120}            
};*/
static int m_voice_page = 1;
static int m_cur_voice_page = 1;
static int m_cur_voice_select = -1;
static int m_pre_voice_select = -1;
static bool g_voice_lock_status = false;
static bool g_is_player_voice = false;

static void DrawVoiceInfo(){
        WRT_xy xy;  
        WRT_Rect curbox;
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 889;
        curbox.h = 530;  
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        if(m_cur_voice_page != 1 &&m_cur_voice_page>0)
                JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

        if(m_voice_page > 1 && m_voice_page != m_cur_voice_page)
                JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));   

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));

        if(get_voice_count() == 0){

        	FillRect(0xb2136,curbox); 

                DrawText(LT("没有语音留言..."), 80,77,rgb_white);
                return;
        }
        
        VOICEINFO* pTemp = 0;
        int count = (m_cur_voice_page-1)*9;
        int  i = 0;
        int offset = 40;
        char text[255] = {0};
        char text1[255] = {0};
        int textwidth =0;
        int first_text_len =0;

        if(m_cur_voice_select == -1){

        	FillRect(0xb2136,curbox); 

        }

        for( i =0;i<9;i++){
                pTemp = get_select_voice(count+i);
                if(pTemp != NULL && pTemp->idle == 1){
                        memset(text,0,255);
                        int addr = (pTemp->read & 0xffff0000) >> 16;
                        if(addr == 0)
                                sprintf(text,"%s_%s",LT("门口机"),(char*)(pTemp->name));    
                        else if(addr == 1){
                                sprintf(text,"%s_%s",LT("小门口机"),(char*)(pTemp->name));    
                        }else if(addr == 2){
                                sprintf(text,"%s_%s",LT("围墙机"),(char*)(pTemp->name));    
                        }else if(addr == 3){
                                sprintf(text,"%s_%s",LT("家庭留言"),(char*)(pTemp->name));    
                        }else if(addr == 4){
                        	sprintf(text,"%s_%s",LT("数字小门口机"),(char*)(pTemp->name));    
                        }else{
                                sprintf(text,"%s",(char*)(pTemp->name));    
                        }
                        textwidth = GetTextWidth(text,0);
                        if(textwidth > 540)
                        {
                                first_text_len = strlen(text);
                                while(first_text_len > 0){
                                        if((first_text_len >= 2)&& text[first_text_len-1] >=0xA0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(text1,0,255);
                                        memcpy(text1,text,first_text_len);
                                        text1[first_text_len+1]='\0';
                                        textwidth = GetTextWidth(text1,0);
                                        if( (textwidth) < 480){ //430
                                                break;
                                        }
                                }    
                                memset(text,0,255);
                                strcpy(text,text1);
                                strcat(text,"...");
                        }
                        if(m_cur_voice_select == i){
                                DrawText(text,110,50+i*offset+23,rgb_yellow);                  
                                if(pTemp->lock){

                                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1)); 
                                        g_voice_lock_status = true;                                     
                                } else{

                                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));       
                                        g_voice_lock_status = false;
                                }
                                JpegDisp(40,50+i*offset,ico_select_2,sizeof(ico_select_2));               
                        }else{
                                DrawText(text,110,50+i*offset+23,rgb_white); 
                                JpegDisp(40,50+i*offset,ico_select_1,sizeof(ico_select_1));                  
                        }
                        if(m_pre_voice_select == i && m_pre_voice_select != m_cur_voice_select){
                                DrawText(text,110,50+i*offset+23,rgb_white);

                                JpegDisp(40,50+i*offset,ico_select_1,sizeof(ico_select_1));                    
                        }
                        if((pTemp->read & 0xffff) == 0)
                        	JpegDisp(70,50+i*offset,ico_voice1,sizeof(ico_voice1));
                        else
                        	JpegDisp(70,50+i*offset,ico_voice2,sizeof(ico_voice2));
                        if(pTemp->lock){
                                JpegDisp(10,50+i*offset,ico_lock,sizeof(ico_lock));
                        }else{
                                curbox.x = 10;
                                curbox.y = 50+i*offset;
                                curbox.w = 28;
                                curbox.h = 28;
                                FillRect(BACK_COLOR,curbox);
                        }   

                }else
                        break;

        }
		//留言最大数目是6条，所以无上下翻页
        //if(m_cur_voice_select == -1){
                DrawStatusbar(m_voice_page,m_cur_voice_page);
        //}

}

static void DrawVoiceCurSelect(){
        int count = 0;
        if(m_cur_voice_select == -1){
                CloseTopWindow();
                return;
        }
        count =(m_cur_voice_page-1)*9+m_cur_voice_select ;
        char text[255];
        char text1[255];
        int str_len =0;
        int xoffset =0;

        VOICEINFO* curvoice = get_select_voice(count);
        if(curvoice && curvoice->idle == 1){
                //播放声音。。。。。。
                long size =0;
                unsigned char* voice_data = 0;
                if((curvoice->read & 0xffff)  == 0)
                        JpegDisp(10,50,ico_voice1,sizeof(ico_voice1));
                else
                        JpegDisp(10,50,ico_voice2,sizeof(ico_voice2));

                int addr = (curvoice->read & 0xffff0000) >> 16;
                memset(text,0,255);
                if(addr == 0)
                        sprintf(text,"%s_%s",LT("门口机"),(char*)(curvoice->name));    
                else if(addr == 1){
                        sprintf(text,"%s_%s",LT("小门口机"),(char*)(curvoice->name));    
                }else if(addr == 2){
                        sprintf(text,"%s_%s",LT("围墙机"),(char*)(curvoice->name));    
                }else if(addr == 3){
                        sprintf(text,"%s_%s",LT("家庭留言"),(char*)(curvoice->name));    
                }else if(addr == 4){
                        	sprintf(text,"%s_%s",LT("数字小门口机"),(char*)(curvoice->name));    
                }else{
                        sprintf(text,"%s",(char*)(curvoice->name));    
                } 
                str_len = GetTextWidth(text,1);
                xoffset = (640 - str_len)/2;
                if(xoffset < 0){
                        str_len =  strlen(text);
                        int first_text_len = str_len;
                        while(first_text_len > 0){
                                if((first_text_len >= 2)&& text[first_text_len-1] >=0xA0)
                                        first_text_len -=2;
                                else
                                        first_text_len --;
                                memset(text1,0,255);
                                memcpy(text1,text,first_text_len);
                                text1[first_text_len+1]='\0';
                                str_len = GetTextWidth(text1,0);
                                if( (str_len) < 300){ //430
                                        break;
                                }
                        }  
                        xoffset = (640 - str_len)/2;
                        DrawText((char*)text1,xoffset,114,rgb_white);
                        DrawText((char*)(text+first_text_len),xoffset,144,rgb_white);
                }else
                        DrawText((char*)(text),xoffset,114,rgb_white);
                /*
                if(curvoice->lock){
                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));
                g_voice_lock_status = true;
                }else{
                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
                g_voice_lock_status = false;
                }
                */ 
                if(curvoice->lock){
                        JpegDisp(50,50,ico_lock,sizeof(ico_lock));
                }else{
                        WRT_Rect curbox;
                        curbox.x = 50;
                        curbox.y = 50;
                        curbox.w = 28;
                        curbox.h = 28;
                //        FillRect(BACK_COLOR,curbox);
                }  

                if(curvoice->addr != NULL && curvoice->len > 0)
                {
                        voice_data = curvoice->addr;
                        size = curvoice->len;
                }else{          
                        voice_data  = get_voice_addr((char*)curvoice->name,&size);
                        curvoice->addr = voice_data;
                        curvoice->len = size;
                        WRT_DEBUG("voice_data = %x ,%d",voice_data,size);
                }
                g_is_player_voice = true;
                
                char tmpbuf[72];
                sprintf(tmpbuf,"/home/wrt/voice/%s",curvoice->name);

                //leaveplay((char*)voice_data,size,0);
                mp3stop();
                SDL_Delay(500);
				set_voice_read(curvoice,1);
				save_voice();
                leaveplay((char*)tmpbuf,size,0);
                WRT_DEBUG("播放声音\n");
                
                //system(tmpbuf);
                //ZENFREE(voice_data);
        }
}

void CloseVoiceWin(){
        free_mem_all_voice();
}
void CreateVoiceWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = VoiceEventHandler;
        pWindow->NotifyReDraw = DrawVocieMain;
        pWindow->CloseWindow = CloseVoiceWin;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        int size = get_voice_count();
        m_voice_page =size /9;
        if(size%9)
                m_voice_page +=1;
        if(m_voice_page == 0)
                m_voice_page = 1;
        m_cur_voice_page =1;
        m_cur_voice_select = -1;
        m_pre_voice_select = -1;     

        pWindow->NotifyReDraw2(pWindow);    
}
void DrawVocieMain(){

		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

        JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

	    JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
	    JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
	    JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
	    JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));
          
        g_voice_lock_status = false;
        DrawVoiceInfo();
}

int  VoiceEventHandler(int x,int y,int status){
        int res =0;
        WRT_xy xy;
        int index =0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        for(index=0;index<17;index++){
                if(IsInside(rc_infobrowsermenuico[index],xy)){
                        pos_x = rc_infobrowsermenuico[index].x;
                        pos_y = rc_infobrowsermenuico[index].y;
                        res  =1;
                        switch(index){
                case 12:/*播放*/
                        if(get_voice_count() == 0)
                                break;    
                        if(m_cur_voice_select == -1)
                                break;  
                        if(g_is_player_voice == true){
                                leavestop();
                                g_is_player_voice = false;
                        }        
                        if(status == 1){
                                StartButtonVocie();
								SDL_Delay(400);
                                JpegDisp(pos_x,pos_y,bt_bofang2,sizeof(bt_bofang2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_bofang1,sizeof(bt_bofang1));
                                CreateVoiceSubWin();            
                        }

                        break;
                case 13:/*上翻*/
                        if(m_cur_voice_page == 1)        
                                break;                                     	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }
                                m_cur_voice_page--;                     
                                if(m_cur_voice_page < 1)
                                        m_cur_voice_page  = 1;
                                m_cur_voice_select = -1;
                                m_pre_voice_select = -1;
                                g_isUpdated = 0;
                                DrawVoiceInfo();
								g_isUpdated = 1;
                                update_rect(0,0,1024,600);                                
                        }
                        break;
                case 14:/*下翻*/  
                        if(m_cur_voice_page == m_voice_page)        
                                break;  
                                                                                      	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1)); 
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                   		               	          
                                m_cur_voice_page++;
                                if(m_cur_voice_page > m_voice_page)
                                        m_cur_voice_page  = m_voice_page;
                                m_cur_voice_select = -1;
                                m_pre_voice_select =  -1;
                                g_isUpdated = 0;
                                DrawVoiceInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 9:/*清空*/     
                        if(get_voice_count() == 0)        
                                break; 

                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{

                                        JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));  
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                 		
                                CreateDialogWin2(LT("你确定要清空所有留言"),MB_OK|MB_CANCEL,clear_voice,NULL);     
                        }                                         
                        break;
                case 10:/*删除*/
                        if(get_voice_count() == 0)        
                                break;                  	
                        if(m_cur_voice_select == -1)
                                break;                      
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew)); 
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }
                                VOICEINFO* curinfo = get_select_voice((m_cur_voice_page-1)*9+m_cur_voice_select);
                                if((curinfo->read & 0xffff) == 0)
                                {
                                	CreateDialogWin2(LT("您还未阅读,确定删除?"),MB_OK|MB_CANCEL,del_cur_voice,NULL);
                                	break;
                                }
                                if(curinfo->lock){
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);
                                }else                 		
                                        CreateDialogWin2(LT("你确定要删除该留言"),MB_OK|MB_CANCEL,del_cur_voice,NULL);    
                        }                    
                        break;
                case 11:/*锁定*/
                        if(get_voice_count() == 0)        
                                break;                  	
                        if(m_cur_voice_select == -1)
                                break;                     
                        if(status == 1){
                                StartButtonVocie();


                                        if(g_voice_lock_status)
                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                        else
                                                JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));

                        }else{

                                        if(g_voice_lock_status){
                                                JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));
                                                lock_cur_voice(false);
                                        }else{
                                                JpegDisp(pos_x,pos_y,bt_jiesuo1,sizeof(bt_jiesuo1));   
                                                lock_cur_voice(true); 
                                        }

                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                  	  
                                g_isUpdated = 0;
                                DrawVoiceInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }                 	  
                        break;
                case 15: /*返回*/

                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));    
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                   		                	
                                m_cur_voice_select = -1;
                                m_pre_voice_select = -1;  
                                CloseTopWindow(); 

                        }

                        break;
                case 16:/*关屏*/

                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                          
                                screen_close(); 
                                CloseAllWindows();
                        }
                        break;
                default:
                        if(get_voice_count() == 0)        
                                break;                 	
                        if((index+(m_cur_voice_page-1)*9) < get_voice_count()){
                                if(status == 0){
                                        m_pre_voice_select = m_cur_voice_select;
                                        m_cur_voice_select = index;
                                        if(m_cur_voice_select != m_pre_voice_select){
                                        	g_isUpdated = 0;
                                                DrawVoiceInfo();
                                                g_isUpdated = 1;
                                				update_rect(0,0,1024,600);      
                                                StartButtonVocie();
                                        }
                                }
                        }
                        break;                                                
                        }
                        break;
                }
        }    
        return res;  
}

void CreateVoiceSubWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = VoiceSubEventHandler;
        pWindow->NotifyReDraw = DrawVoiceSubMain;
        pWindow->valid = 1;
        pWindow->type = LEAVE_CHILD;   
        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);        
}
void DrawVoiceSubMain(){
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

		//JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
//		JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));
		//   if(get_voice_count() == 0){

//		JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
//		JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));
//		JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
//		JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));

        /* 	
        }else{
        JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
        JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));
        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
        JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang3,sizeof(bt_bofang3));
        }
        */
        
//        if(m_voice_page > 1)
//                JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

//        if(m_voice_page > 1)
//               JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));   

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));          
        g_voice_lock_status = false;
        DrawVoiceCurSelect();
}

int  VoiceSubEventHandler(int x,int y,int status){
        int ret=0;
        WRT_xy xy;
        int index =0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        for(index=0;index<17;index++){
                if(IsInside(rc_infobrowsermenuico[index],xy)){
                        pos_x = rc_infobrowsermenuico[index].x;
                        pos_y = rc_infobrowsermenuico[index].y;
                        ret = 1;
                        switch(index){
                case 13:/*上翻*/
                	break;
                        if(get_voice_count() == 0)        
                                break; 
                        if(m_voice_page == 1)//根据音乐的第几个来判断
                                break;                                                                  	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                 		 
                                m_cur_voice_select --;
                                if(m_cur_voice_select < 0)
                                        m_cur_voice_select = 0;
                                DrawVoiceCurSelect();
                        }
                        break;
                case 14:/*下翻*/ 
                	break;
                        if(get_voice_count() == 0)        
                                break;  
                        if(m_voice_page == 1)
                                break;  
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1)); 
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                		               	          
                                m_cur_voice_select++;
                                if((m_cur_voice_select+(m_cur_voice_page-1)*9) > get_voice_count())
                                        m_cur_voice_select--;
                                DrawVoiceCurSelect();
                        }
                        break;            	            
                case 9:/*清空*/ 
                        /*    
                        if(get_voice_count() == 0)        
                        break;   
                        if(g_is_player_voice == true){
                        leavestop();
                        g_is_player_voice = false;
                        }                        
                        if(status == 1){
                        StartButtonVocie();
                        JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{
                        JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));   
                        CreateDialogWin2("你确定要清空所有留言",MB_OK|MB_CANCEL,clear_voice,NULL);   
                        }*/                                         
                        break;
                case 10:/*删除*/
                        /*
                        if(get_voice_count() == 0)        
                        break;                  	
                        if(m_cur_voice_select == -1)
                        break;
                        if(g_is_player_voice == true){
                        leavestop();
                        g_is_player_voice = false;
                        }                      
                        if(status == 1){
                        StartButtonVocie();
                        JpegDisp(pos_x,pos_y,bt_shanchu2,sizeof(bt_shanchu2));
                        }else{
                        JpegDisp(pos_x,pos_y,bt_shanchu1,sizeof(bt_shanchu1));  
                        CreateDialogWin2("你确定要删除该留言",MB_OK|MB_CANCEL,del_cur_voice,NULL);     
                        } */                   
                        break;
                case 11:/*锁定*/
                        /*
                        if(get_voice_count() == 0)        
                        break;                  	
                        if(m_cur_voice_select == -1)
                        break;
                        if(status == 1){
                        StartButtonVocie();
                        if(g_voice_lock_status)
                        JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                        else
                        JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                        }else{
                        if(g_voice_lock_status){
                        JpegDisp(pos_x,pos_y,bt_jiesuo1,sizeof(bt_jiesuo1));
                        lock_cur_voice(false);
                        }else{
                        JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));   
                        lock_cur_voice(true); 
                        }
                        // DrawVoiceCurSelect();
                        } */              	  
                        break;
                case 15: /*返回*/      
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));   
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                		                 	
                                m_cur_voice_select = -1;
                                m_pre_voice_select = -1;  
                                CloseTopWindow(); 
                                save_voice();                
                        }

                        break;
                case 16:/*关屏*/

                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1)); 
                                if(g_is_player_voice == true){
                                        leavestop();
                                        g_is_player_voice = false;
                                }                         
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



int  del_cur_voice(int param){
        VOICEINFO* curinfo = get_select_voice((m_cur_voice_page-1)*9+m_cur_voice_select);
        if(curinfo == NULL)
                return 0;
        del_voice(curinfo);
        int size = get_voice_count();
        m_voice_page =size /9;
        if(size%9)
                m_voice_page +=1;
        if(m_voice_page == 0)
                m_voice_page = 1;
        if(m_cur_voice_page > m_voice_page)
                m_cur_voice_page = m_voice_page;
        m_cur_voice_select = -1;
		return 0;                	
}
int  clear_voice(int param){
        m_cur_voice_select = -1;
        m_pre_voice_select = -1;
        del_all_voice();
        m_cur_voice_page = 1;
        m_voice_page = 1;	
        return 0;
}
void lock_cur_voice(bool islock){  
        VOICEINFO* curinfo = get_select_voice((m_cur_voice_page-1)*9+m_cur_voice_select);
        if(curinfo == NULL)
                return;
        lock_voice(curinfo,islock);
        // ReDrawCurrentWindow();	
}

