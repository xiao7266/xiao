#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"

#include "window.h"

extern bool g_have_warning;

extern  WRT_Rect rc_infobrowsermenuico[17]; 

static int m_log_page = 1;
static int m_cur_log_page = 1;
static int m_cur_log_select = -1;
static int m_pre_log_select = -1;
static bool g_log_lock_status = false;

static void DrawLogInfo(){
        WRT_xy xy;  
        WRT_Rect curbox;
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 889;
        curbox.h = 530;  
        
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
        if(m_cur_log_page ==1)
        {
        	;
        }
        else
        {
        	JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));
        } 

        if(m_log_page > 1  && m_log_page != m_cur_log_page)
                JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));   

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));
        if(get_log_count() == 0){

                FillRect(0xb2136,curbox); 

                DrawText(LT("没有任何报警日志..."), 80,77,rgb_white);
                return;
        }
         

        int count = (m_cur_log_page-1)*9;
        int  i = 0;
        int offset = 40;
        // char text[42] = {0};//将来可该改成静态的，使（log,log,使用同一内存）;
        char* text1 = (char*)ZENMALLOC(400); //将来可该改成静态的，使（log,log,使用同一内存）;
        char* text2 = (char*)ZENMALLOC(400);
        if(text1 == NULL || text2 == NULL)
        {
                if(text1)
                        ZENFREE(text1);
                if(text2)
                        ZENFREE(text2);

                FillRect(0xb2136,curbox); 

                DrawText(LT("程序出现错误..."), 80,77,rgb_white);
                return;


        }
        int textlen = 0;
        long start =0;
        long tmpstart = 0;
        int islock = 0;
        int isread = 0;
        LOGINFO* pLog= 0;
        if(m_cur_log_select == -1){

                FillRect(0xb2136,curbox); 

                SDL_Delay(20);
        }

        i=0;
        for(i=0;i<9;i++){
                pLog = get_select_log(count+i);
                if(pLog && pLog->idle == 1){  

                        memset(text1,0,400);
                        memcpy(text1,pLog->addr,pLog->len);
                        islock = pLog->lock;
                        isread = pLog->read;
                        if(strlen(text1) == 0)
                                continue;
                        unsigned char* pText = (unsigned char*)text1;
                        int first_text_len = strlen(text1);
                        int str_width = GetTextWidth(text1,1);
                        if(str_width  >  620){
                                while(first_text_len > 0){
                                        if((first_text_len >= 2)&& pText[first_text_len-1] >=0xA0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(text2,0,400);
                                        memcpy(text2,pText,first_text_len);
                                        str_width = GetTextWidth(text2,0);
                                        if( str_width < 620){
                                                break;
                                        }
                                }
                        }else{
                                memset(text2,0,400);
                                memcpy(text2,pText,first_text_len);
                        }
                        /*
                        if(first_text_len >40){ //40个字符
                        while(first_text_len > 0){
                        if((first_text_len >= 2)&& pText[first_text_len-1] >=0xA0)
                        first_text_len -=2;
                        else
                        first_text_len --;
                        if( (first_text_len *12) < 600){
                        break;
                        }
                        }
                        memcpy(text,text1,first_text_len);
                        text[first_text_len]='\0';
                        }else{
                        memcpy(text,text1,first_text_len);
                        text[first_text_len] = '\0';
                        } 
                        */
                        if(m_pre_log_select == i && m_pre_log_select != m_cur_log_select){
                                DrawText_16(text2,70,50+i*offset+22,rgb_white);         
                        }
                        if(m_cur_log_select == i){
                                DrawText_16(text2,70,50+i*offset+22,rgb_yellow);
                                if(islock){

                                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1)); 
                                        g_log_lock_status = true;                                     
                                }else{

                                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1)); 
                                        g_log_lock_status = false;                                                  	
                                }                                                
                        }else{
                                DrawText_16(text2,70,50+i*offset+22,rgb_white);                  
                        }         
                        JpegDisp(40,50+i*offset,bt_ico_baojing1,sizeof(bt_ico_baojing1));
                        if(islock){
                                JpegDisp(10,50+i*offset,ico_lock,sizeof(ico_lock));
                        }else{
                                curbox.x = 10;
                                curbox.y = 50+i*offset;
                                curbox.w = curbox.h = 28;
                                FillRect(0xb2136,curbox);
                        }
                }else
                        break;

        }
        ZENFREE(text1);
        ZENFREE(text2);
        //if(m_cur_log_select == -1){
                DrawStatusbar(m_log_page,m_cur_log_page);
        //}

}

void DrawStatusbar(int npages,int npage)
{
        char text[50];

        JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
		JpegDisp(300,531,menu_sub_back4,sizeof(menu_sub_back4));

        memset(text,0,50);
        if(GetCurrentLanguage() == ENGLISH)
                sprintf(text,"Page %d ,total %d ",npage,npages);
        else
                sprintf(text,"当前第 %d 页,总 %d 页",npage,npages);
        DrawText_16(text,180,576,rgb_black);
}

void CreateWarningLog(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = WarningLogEventHandler;
        pWindow->NotifyReDraw = DrawWarningLogMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        int size = get_log_count();
        m_log_page =size /9;
        if(size%9)
                m_log_page +=1;       
        if(m_log_page == 0)
                m_log_page = 1;  
        m_cur_log_page = 1;
        pWindow->NotifyReDraw2(pWindow);       
}
void DrawWarningLogMain(){

		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);
		g_have_warning = false;
        JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

        JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
        JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
        //JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_dakai1,sizeof(bt_dakai1));
      
        g_log_lock_status = false;
        DrawLogInfo();
}

int  WarningLogEventHandler(int x,int y,int status){
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
                        switch(index)
                        {
                        case 12:/*阅读*/
                                break;
                                if(get_log_count() == 0)
                                        break;                    
                                if(status == 1){
                                        StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_dakai2,sizeof(bt_dakai2));
                                }else{

                                        JpegDisp(pos_x,pos_y,bt_dakai1,sizeof(bt_dakai1));          
                                }

                                break;
                        case 13:/*上翻*/
                                if(m_cur_log_page == 1)
                                        break;
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                        m_cur_log_page--;
                                        if(m_cur_log_page < 1)
                                                m_cur_log_page  = 1;
                                        m_cur_log_select = -1;
                                        m_pre_log_select = -1;
                                        g_isUpdated = 0;
                                        DrawLogInfo();
                                        g_isUpdated = 1;
                                        update_rect(0,0,1024,600);
                                }
                                break;
                        case 14:/*下翻*/  
                                if(m_log_page == m_cur_log_page)
                                        break;
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                                }else{
                                        JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                	          
                                        m_cur_log_page++;
                                        if(m_cur_log_page > m_log_page)
                                                m_cur_log_page  = m_log_page;
                                        m_cur_log_select = -1;
                                        m_pre_log_select =  -1;
                                        g_isUpdated = 0;
                                        DrawLogInfo();
                                        g_isUpdated = 1;
                                        update_rect(0,0,1024,600);
                                }
                                break;
                        case 9:/*清空*/     
                                if(get_log_count() == 0)        
                                        break;   
                                if(status == 1){
                                        StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                                }else{

                                        JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));  
                                        CreateDialogWin2(LT("你确定要清空所有日志"),MB_OK|MB_CANCEL,clear_log,NULL);     
                                }                                         
                                break;
                        case 10:/*删除*/
                                if(get_log_count() == 0)        
                                        break;                  	
                                if(m_cur_log_select == -1)
                                        break;
                                if(status == 1){
                                        StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                                }else{

                                        JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew));
                                        if(g_log_lock_status)
                                                CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);
                                        else
                                                CreateDialogWin2(LT("你确定要删除该日志"),MB_OK|MB_CANCEL,del_cur_log,NULL);     
                                }                    
                                break;
                        case 11:/*锁定*/
                                if(get_log_count() == 0)        
                                        break;                  	
                                if(m_cur_log_select == -1)
                                        break;
                                if(status == 1){
                                        StartButtonVocie();

                                        if(g_log_lock_status)
                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                        else
                                                JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));

                                }else{

                                        if(g_log_lock_status){

                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                                lock_cur_log(false);
                                        }else{

                                                JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));   
                                                lock_cur_log(true); 
                                        }
                                }                	  
                                break;
                        case 15: /*返回*/
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));                   	
                                        m_cur_log_select = -1;
                                        m_pre_log_select = -1;  
                                        CloseTopWindow();   
                                        save_log();              
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
                                if(get_log_count() == 0)        
                                        break;                 	
                                if((index+(m_cur_log_page-1)*9) < get_log_count()){
                                        if(status == 0){
                                                m_pre_log_select = m_cur_log_select;
                                                m_cur_log_select = index;
                                                if(m_cur_log_select != m_pre_log_select){
                                                        g_isUpdated = 0;
                                                        DrawLogInfo();
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


int del_cur_log(int param){
        int count = (m_cur_log_page-1)*9+m_cur_log_select;
        LOGINFO*  pLog = get_select_log(count);
        if(pLog)
                del_log(pLog);
        m_cur_log_select = -1;
        m_pre_log_select = -1;
        int size = get_log_count();
        m_log_page =size /9;
        if(size%9)
                m_log_page +=1;
        if(m_log_page == 0)
                m_log_page = 1;
        if(m_cur_log_page > m_log_page)
                m_cur_log_page = m_log_page;	

        return 0;
}

int clear_log(int param){
        m_cur_log_select = -1;
        m_pre_log_select = -1;
        del_all_log();
        m_cur_log_page = 1;
        m_log_page = 1;  

        return 0; 	
}
int lock_cur_log(bool islock){
        int count =(m_cur_log_page-1)*9+m_cur_log_select;
        LOGINFO* pLog  = get_select_log(count);
        if(pLog)
                lock_log(pLog,islock);
        //  ReDrawCurrentWindow();
        g_isUpdated = 0;
        DrawLogInfo();
        g_isUpdated = 1;
        update_rect(0,0,1024,600);
        return 0;
}
