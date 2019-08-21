#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "read_msg_res.h"
#include "public_menu_res.h"

#include "window.h"



extern bool g_have_msg;
static int m_msg_page = 1;
static int m_cur_msg_page = 1;
static int m_cur_msg_select = -1;
static int m_pre_msg_select = -1;

static int m_cur_lock = 0;

static bool g_msg_lock_status = false;

static int  g_is_save_msg = 0;

extern int g_isUpdated;

#ifdef WRT_MORE_ROOM
static bool g_more_room_read_msg_end = false;
#endif



WRT_Rect rc_infobrowsermenuico[17]=
{   
        {32,50,664,40},
        {32,90,664,40},
        {32,130,664,40},
        {32,170,664,40},
        {32,210,664,40},
        {32,250,664,40},
        {32,290,664,40},
        {32,330,664,40},
        {32,370,664,40},   

        {900,44,104,76},           /*清空*/ 
        {900,127,104,76},            /*锁定*/  
        {900,217,104,76},            /*删除*/
        {900,309,104,76},            /*打开*/         
        {660,531,111,69},           /*上翻*/
        {780,531,111,69},            /* 下翻*/

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕			
};     


static void DrawPhotoMsgCurSelect(MSGINFO* pMsg){ 
        if(pMsg == NULL)
                return;
        unsigned char* photobuf=NULL;
        int len =0;
        int islock =  islock = pMsg->lock;
        WRT_DEBUG("Photo msg %s \n",pMsg->addr);
        photobuf = get_photo_msg((char*)pMsg->addr,&len);
        if(photobuf == 0 || len == 0)
                return;
        WRT_DEBUG("read Photo msg %x %d \n",photobuf,len);
        JpegDispCenter(0,0,photobuf,len,888);
        ZENFREE(photobuf);
        photobuf = NULL;
        set_msg_read(pMsg,3);
        g_is_save_msg = 1;
        if(islock)
                JpegDisp(40,50,ico_lock,sizeof(ico_lock)); 
        if(islock){

                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));
                g_msg_lock_status = true;
        }else{

                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));         
                g_msg_lock_status = false;
        }
        m_cur_lock = islock;
}

static void DrawMsgCurSelect(int param)
{
        int count = 0;
        long start =0;
        long tmpstart = 0;
        int i =0;
        int textlen = 0;
        int islock=-1;
        int isread = 0;
        int font_width = 12;
        WRT_Rect curbox;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 889;
        curbox.h = 530;    
        if(m_cur_msg_select == -1){
                //FillRect(0xb2136,curbox); 
               // DrawText(LT("没有短消息..."), 80,77,rgb_white);
               // WRT_MESSAGE("没有任何短消息 ...CloseTopWindow");
                CloseTopWindow();
                return;
        }

        if(GetCurrentLanguage() == ENGLISH)
                font_width = 14;

        count =(m_cur_msg_page-1)*9+m_cur_msg_select ;

        char* text1 = (char*)ZENMALLOC(400);
        char* text2 = (char*)ZENMALLOC(400);
        if(text1 == NULL || text2 == NULL){
                CloseTopWindow();
                if(text1)
                        ZENFREE(text1);
                if(text2)
                        ZENFREE(text2);
                return;
        }
        memset(text1,0,400);
        memset(text2,0,400);
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 889;
        curbox.h = 530;
        MSGINFO* pMsg = 0;
        pMsg = get_select_msg(count);
        if(pMsg == 0 || pMsg->idle == 0) {
                ZENFREE(text1);
                ZENFREE(text2);

        	FillRect(0xb2136,curbox); 
 
                DrawText(LT("没有短消息..."),80,77,rgb_white);   
#ifdef WRT_MORE_ROOM
                g_more_room_read_msg_end = true;
#endif
                return;
        }
        if(pMsg->len < 0 || pMsg->len > 400){
                ZENFREE(text1);
                ZENFREE(text2);

       		 FillRect(0xb2136,curbox); 
  
                DrawText(LT("没有短消息..."),80,77,rgb_white);      	
#ifdef WRT_MORE_ROOM
                g_more_room_read_msg_end = true;
#endif
                return; 
        }
#ifdef WRT_MORE_ROOM
        if(param == 1){
                if(pMsg->read == 1 ||  pMsg->read == 3)
                {
                        ZENFREE(text1);
                        ZENFREE(text2);

       			 FillRect(0xb2136,curbox); 
  
                        DrawText(LT("没有短消息..."),80,77,rgb_white);   
                        g_more_room_read_msg_end = true;
                        return;
                }
        }
#endif
        if(pMsg->read == 2 || pMsg->read == 3){
                WRT_DEBUG("(%d,%d,%d,%d)",curbox.x,curbox.y,curbox.w,curbox.h);

        	FillRect(0xb2136,curbox); 
  
                SDL_Delay(20);
                DrawPhotoMsgCurSelect(pMsg);
                ZENFREE(text1);
                ZENFREE(text2);
                return;
        }
        memcpy(text1,pMsg->addr,pMsg->len);
        text1[pMsg->len] = '\0';
        textlen = pMsg->len;
        islock = pMsg->lock;
        if(textlen > 0){

        	FillRect(0xb2136,curbox); 
 
                SDL_Delay(20);
                int i =1; 
                int offset = 40;
                unsigned char* pText = (unsigned char*)text1;
                int   first_text_len = textlen;
                int   view_text_len = 0;
                int str_len = 0;

                while(1){
                        while(first_text_len > 0){
                                str_len = GetTextWidth((char*)pText,0);
                                if(str_len > 640){
                                        if((first_text_len >=2 ) && pText[first_text_len-1] >= 0xa0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(text2,0,400);
                                        memcpy(text2,pText,first_text_len);
                                        str_len = GetTextWidth((char*)text2,0);
                                        if((str_len)<640) //540
                                                break;
                                }else{
                                        memset(text2,0,400);
                                        memcpy(text2,pText,first_text_len);
                                        break;
                                }
                        }
                        if(first_text_len == 0 || first_text_len < 0)
                                break;
                        DrawText(text2,32,i*offset+20,rgb_white);
                        view_text_len +=(first_text_len);
                        first_text_len = textlen - view_text_len;
                        i++;
                        pText = (unsigned char*)text1+view_text_len;
                }
                if(islock)
                        JpegDisp(2,44,ico_lock,sizeof(ico_lock));            

                if(islock){

                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));
                        g_msg_lock_status = true;
                }else{

                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));      
                        g_msg_lock_status = false;
                }
                isread = 1;
                m_cur_lock = islock;

                set_msg_read(pMsg,isread);
                g_is_save_msg = 1;

        } 
        ZENFREE(text1);
        ZENFREE(text2);
}

static void DrawMsgInfo(){
        WRT_Rect curbox;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 889;
        curbox.h = 530;
        
		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
		if(m_cur_msg_page > 1)
				JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));
		
		if(m_msg_page > m_cur_msg_page)
				JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));   
		JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
		JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1)); 

        if(get_msg_count() == 0){

        		FillRect(0xb2136,curbox);  
                DrawText(LT("没有短消息..."), 80,77,rgb_white);
                return;
        }
        int count = (m_cur_msg_page-1)*9;
        int  i = 0;
        int offset = 40;
        //  char text[72] = {0};   //将来可该改成静态的，使（log,msg,使用同一内存）;
        // char text2[10];
        // char text1[400] = {0}; //将来可该改成静态的，使（log,msg,使用同一内存）;
        char* text1 = (char*)ZENMALLOC(400);
        char* text2 = (char*)ZENMALLOC(400);
        if(text1 == NULL || text2 == NULL){
                if(text1)
                        ZENFREE(text1);
                if(text2)
                        ZENFREE(text2);
                return;
        }
        int textlen = 0;
        int isread = -1;
        long start =0;
        long tmpstart = 0;
        int islock = -1;
        MSGINFO* pMsg = 0;
        if(m_cur_msg_select == -1){
        	FillRect(0xb2136,curbox); 
            SDL_Delay(20);
        }

        i=0;
        for(i=0;i<9;i++){
                pMsg = get_select_msg(count+i);
                if(pMsg == 0)
                        break;	
                if(pMsg->idle == 0)
                        continue;
                if(pMsg->len < 0 || pMsg->len > 400)
                        continue;
                memset(text1,0,400);
                islock = -1;
                isread = -1;
                memcpy(text1,pMsg->addr,pMsg->len);
                islock = pMsg->lock;
                isread = pMsg->read;
                textlen  = pMsg->len;
                if(textlen == 0){
                        continue;
                }
                unsigned char* pText = (unsigned char*)text1;
                int first_text_len = textlen;
                int str_len = 0;

                str_len = GetTextWidth((char*) pText,0);
                if(str_len > 540){

                        while(first_text_len > 0){
                                if((first_text_len >= 2)&& pText[first_text_len-1] >=0xA0)
                                        first_text_len -=2;
                                else
                                        first_text_len --;
                                memset(text2,0,400);
                                memcpy(text2,pText,first_text_len);
                                text2[first_text_len+1]='\0';
                                str_len = GetTextWidth(text2,0);
                                if( (str_len) < 540){ //430
                                        break;
                                }
                        }
                }else{
                        memset(text2,0,400);
                        memcpy(text2,pText,textlen);
                }
                // memcpy(text,text1,first_text_len);
                // text[first_text_len]='\0';

                if(m_pre_msg_select == i && m_pre_msg_select != m_cur_msg_select){
                        DrawText(text2,130,50+i*offset+25,rgb_white);
                        JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                }
                if(m_cur_msg_select == i){
                        DrawText(text2,130,50+i*offset+25,rgb_yellow); 
                        JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2)); 
                        if(islock){

                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1)); 
                                g_msg_lock_status = true;                                     
                        }else{

                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));        
                                g_msg_lock_status = false;
                        }
                        m_cur_lock = islock;                                              
                }else{
                        DrawText(text2,130,50+i*offset+25,rgb_white);  
                        JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));                  
                }                
                if(isread == 1)
                        JpegDisp(100,50+i*offset,read_msg_ico_2,sizeof(read_msg_ico_2)); //已读
                else if(isread == 0)
                        JpegDisp(100,50+i*offset,read_msg_ico_1,sizeof(read_msg_ico_1)); //未读
                else if (isread == 2){ //图片消息
                        JpegDisp(100,50+i*offset,ico_tupian1,sizeof(ico_tupian1)); //未读图片消息
                }else if(isread == 3){
                        JpegDisp(100,50+i*offset,ico_tupian2,sizeof(ico_tupian2));//已读图片消息
                }
                if(islock)
                        JpegDisp(40,50+i*offset,ico_lock,sizeof(ico_lock));  
                else{
                        curbox.x = 40;
                        curbox.y = 50+i*offset;
                        curbox.w = 28;
                        curbox.h = 28;
                        FillRect(BACK_COLOR,curbox);
                }

                // sprintf(text2,"%d.",(MAX_MSG-count+i)); 
                //  DrawText(text2,10,78+i*offset,rgb_white);    

        }
        //if(m_cur_msg_select == -1){
                DrawStatusbar(m_msg_page,m_cur_msg_page);
        //}  
        ZENFREE(text1);
        ZENFREE(text2);
}

void CloseMsgWin(){
        g_have_msg = isnoreadmsg(); 
        if(g_is_save_msg){
                save_msg();  
                g_is_save_msg = 0;
        }     	
}

void CreateMsgWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = MsgHandler;
        pWindow->NotifyReDraw = DrawMsgWin;
        pWindow->CloseWindow = CloseMsgWin;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);     
        /*
        char text[100];
        for(int i =0; i< 10;i++){
        memset(text,0,100);
        sprintf(text,"中安伟讯短息测试页面%d",i);
        add_msg(text);
        }*/

        int size = get_msg_count();
        m_msg_page =size /9;
        if(size%9)
                m_msg_page +=1;
        if(m_msg_page == 0){
                m_msg_page = 1;
                m_cur_msg_page = 1;
        }
        m_cur_msg_page = 1;
        m_cur_msg_select = -1;
        m_pre_msg_select = -1;   
        m_cur_lock = 0;
        pWindow->NotifyReDraw2(pWindow);

}

void DrawMsgWin(){

		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		

        JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));


        JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
        JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
        JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_dakai1,sizeof(bt_dakai1));

        
                 
        g_msg_lock_status = false;
        DrawMsgInfo();

}

int MsgHandler(int x,int y,int status){
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
                case 12:/*阅读*/
                        if(get_msg_count() == 0)
                                break;  
                        if(m_cur_msg_select == -1)
                                break;                  
                        if(status == 1){
                                StartButtonVocie();
 
                                JpegDisp(pos_x,pos_y,bt_dakai2,sizeof(bt_dakai2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_dakai1,sizeof(bt_dakai1));
                                CreateReadMsgWin1();              
                        }

                        break;
                case 13:/*上翻*/
                        if(m_cur_msg_page == 1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                m_cur_msg_page--;
                                if(m_cur_msg_page < 1)
                                        m_cur_msg_page  = 1;
                                m_cur_msg_select = -1;
                                m_pre_msg_select = -1;
                                g_isUpdated = 0;
                                DrawMsgInfo();
                                update_rect(0,0,1024,600);   
                                g_isUpdated = 1;
                        }
                        break;
                case 14:/*下翻*/  
                        if(m_msg_page == m_cur_msg_page)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                	          
                                m_cur_msg_page++;
                                if(m_cur_msg_page > m_msg_page)
                                        m_cur_msg_page  = m_msg_page;
                                m_cur_msg_select = -1;
                                m_pre_msg_select =  -1;
                                g_isUpdated = 0;
                                DrawMsgInfo();
                                update_rect(0,0,1024,600);   
                                g_isUpdated = 1;
                        }
                        break;
                case 9:/*清空*/     
                        if(get_msg_count() == 0)        
                                break;   
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{

                                        JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));    
                                CreateDialogWin2(LT("你确定要清空所有的短消息"),MB_OK|MB_CANCEL,clear_msg,NULL); 
                        }                                         

                        break;
                case 10:/*删除*/
                        if(get_msg_count() == 0)        
                                break;                  	
                        if(m_cur_msg_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                        }else{

                                        JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew));   
                                if(m_cur_lock == 0)
                                        CreateDialogWin2(LT("你确定要删除该短消息"),MB_OK|MB_CANCEL,del_cur_msg,NULL); 
                                else
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL); 
                        }                    
                        break;
                case 11:/*锁定*/
                        if(get_msg_count() == 0)        
                                break;                  	
                        if(m_cur_msg_select == -1)
                                break;
                        if(status == 1){

                                StartButtonVocie();

                                if(g_msg_lock_status){

                                        JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                }else{

                                        JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                                }
                        }else{
                                if(g_msg_lock_status){

                                		JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                        lock_cur_msg(false); 
                                }else{

                                		JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));   
                                		lock_cur_msg(true); 
                                }
                                g_isUpdated = 0;
                                DrawMsgInfo();//需要修改锁标志显示问题 参照电话本
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
                                m_cur_msg_select = -1;
                                m_pre_msg_select = -1;  
                                CloseTopWindow();   
                                g_have_msg = isnoreadmsg(); 
                                if(g_is_save_msg){
                                        save_msg();  
                                        g_is_save_msg = 0;
                                }               
                        }

                        break;
                case 16:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
                                screen_close(); 
                                g_have_msg = isnoreadmsg();
                                CloseAllWindows();
                                if(g_is_save_msg){
                                        save_msg();  
                                        g_is_save_msg = 0;
                                }     
                        }
                        break;
                default:
                        if(get_msg_count() == 0)        
                                break;                 	
                        if((index+(m_cur_msg_page-1)*9) < get_msg_count()){
                                if(status == 0){
                                        m_pre_msg_select = m_cur_msg_select;
                                        m_cur_msg_select = index;
                                        if(m_cur_msg_select != m_pre_msg_select){
                                        	g_isUpdated = 0;
                                            DrawMsgInfo();
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

#ifdef WRT_MORE_ROOM

void more_room_set_read_msg_pos()
{
        m_cur_msg_page = 1;
        m_cur_msg_select = 0;
        g_more_room_read_msg_end = false;
}

int  more_room_read_next()
{
        if(g_more_room_read_msg_end == true){
                g_have_msg = isnoreadmsg();
                if(!g_have_msg)
                {
                        close_red_led();
                }
                if(g_is_save_msg){
                        save_msg();  
                        g_is_save_msg = 0;
                }  
                return 0;
        }
        m_cur_msg_select++;
        if((m_cur_msg_select +(m_cur_msg_page-1)*9) > get_msg_count()){
                g_have_msg = isnoreadmsg();
                if(!g_have_msg)
                {
                        close_red_led();
                }
                if(g_is_save_msg){
                        save_msg();  
                        g_is_save_msg = 0;
                }  
                return 0;
        }
        DrawMsgCurSelect(1);
        return 1;

}

#endif

void CreateReadMsgWin1(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = ReadMsg1Handler;
        pWindow->NotifyReDraw = DrawReadMsgWin1;
        pWindow->valid = 1;
#ifdef WRT_MORE_ROOM
        pWindow->type = MSG_READ;
#else
        pWindow->type = CHILD;
#endif
	WindowAddToWindowList(pWindow);      

        pWindow->NotifyReDraw2(pWindow);
      
}
void DrawReadMsgWin1(){//改写成选中了的直接打开信息，上下翻显示上下条信息
	WRT_Rect rt;
			rt.x = 0;
			rt.y = 0;
			rt.w =1024;
			rt.h = 530;
			FillSingleRect(0xb2136,rt);
	
			JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

    JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
    JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
    JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
    //JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_dakai1,sizeof(bt_dakai1));
         
	if(get_msg_count() > 1)
		JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));
	if(get_msg_count() > 1)
		JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));          
        g_msg_lock_status = false;
        DrawMsgCurSelect(0);

}
int  ReadMsg1Handler(int x,int y,int status){
        int ret = 0;
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
                case 13:/*上翻*/
                        if(get_msg_count() == 0)        
                                break;                  	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));                	
                                m_cur_msg_select --;
                                if(m_cur_msg_select < 0)
                                        m_cur_msg_select = 0;
                                g_isUpdated = 0;
                                DrawMsgCurSelect(0);
				g_isUpdated = 1;
                                update_rect(0,0,1024,600);   
                        }
                        break;
                case 14:/*下翻*/
                        if(get_msg_count() == 0)        
                                break;                  	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                	
                                m_cur_msg_select++;
                                if((m_cur_msg_select +(m_cur_msg_page-1)*9) > get_msg_count())
                                        m_cur_msg_select -= 1;
                                g_isUpdated = 0;
                                DrawMsgCurSelect(0);
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);   
                        }
                        break;
                case 9:    /*清空*/
                        if(get_msg_count() == 0)        
                                break;   
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{

                                        JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));   
                                CreateDialogWin2(LT("你确定要清空所有的短消息"),MB_OK|MB_CANCEL,clear_msg,NULL); 
                        }                                         
                        break;
                case 10:/*删除*/
                        if(get_msg_count() == 0)        
                                break;                  	
                        if(m_cur_msg_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                        }else{

                                        JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew)); 
                                if(m_cur_lock == 0)
                                        CreateDialogWin2(LT("你确定要删除该短消息"),MB_OK|MB_CANCEL,del_cur_msg,NULL);   
                                else
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);   
                        }                    
                        break;
                case 11:/*锁定*/
                        if(get_msg_count() == 0)        
                                break;                  	
                        if(m_cur_msg_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                if(g_msg_lock_status){

                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                }else{

                                                JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                                }
                        }else{
                                if(g_msg_lock_status){

                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                        lock_cur_msg(false); 
                                }else{

                                                JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));   
                                        lock_cur_msg(true); 
                                }
                                g_isUpdated = 0;
                                DrawMsgCurSelect(0);
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
                                m_cur_msg_select = -1;
                                m_pre_msg_select = -1;  
                                CloseTopWindow();   
                                if(g_is_save_msg){
                                        save_msg();  
                                        g_is_save_msg = 0;
                                }            
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
                        break;                                                    
                        }
                        break;
                }
        }    
        return ret;
}



int del_cur_msg(int param){
        //  del_msg((m_cur_msg_page-1)*9+m_cur_msg_select);
        int count = (m_cur_msg_page-1)*9+m_cur_msg_select;
        MSGINFO* pMsg = get_select_msg(count);
        if(pMsg)
                del_msg(pMsg);
        m_cur_msg_select = -1;
        m_pre_msg_select = -1;
        int size = get_msg_count();
        m_msg_page =size /9;
        if(size%9)
                m_msg_page +=1;
        if(m_msg_page == 0)
                m_msg_page = 1;
        if(m_cur_msg_page > m_msg_page)
                m_cur_msg_page = m_msg_page;
        /*
        if(size == 0){
        	 m_cur_msg_select = -1;
        	m_pre_msg_select = -1;
        }else{
        	m_cur_msg_select--;
        	m_pre_msg_select = m_cur_msg_select;
        }
        */
              	
        return 0;
}
int clear_msg(int param){
        m_cur_msg_select = -1;
        m_pre_msg_select = -1;
        del_all_msg();
        m_cur_msg_page = 1;
        m_msg_page = 1;
        return 0;  
}
void lock_cur_msg(bool islock){

        int count = 0;
        int i = 0;
        MSGINFO* pMsg = 0;
        count =(m_cur_msg_page-1)*9+m_cur_msg_select;   

        pMsg = get_select_msg(count);
        if(pMsg){
                lock_msg(pMsg,islock);
                g_is_save_msg = 1;
        }

}
