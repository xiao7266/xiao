#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "window.h"
#include "c_interface.h"
#include "idtoiptable.h"

extern  WRT_Rect rc_infobrowsermenuico[17];

////////////////////////////////////////////////////
//   [in] id  输入参数
//   [out] ip  查找到数据存入ip
//   [in,out]  num 传入分配ip的个数，输出num得到的ip个数
//extern void get_ipbyid(char* id,unsigned long* ip,int* num);

static int m_telephone_page = 1;
static int m_cur_telephone_page = 1;
static int m_cur_telephone_select = -1;
static int m_pre_telephone_select = -1;
static bool g_telephone_lock_status = false;
static bool m_telephone_lock = false;


static void call_cur_select(){
        if(m_cur_telephone_select == -1)
                return;
        int  count =(m_cur_telephone_page-1)*9+m_cur_telephone_select ;

        TELEPHONE* pTele = get_select_telephone(count);
        if(pTele == 0)
                return;
        char tempid[16];

        unsigned long ip[4];
        int num =4;
        unsigned long event[4];
        event[0] = CUSTOM_CMD_CALL_ROOM;
        event[1] = 0;
        event[2] = 0;
        event[3] =0;
        memset(ip,0,sizeof(ip)); 
        //按规则解析room_number 
        memset(tempid,0,16);
        memcpy(tempid,pTele->room_number,15);
        parse_inputcallrule(tempid);
        get_ipbyid(tempid,ip,&num);
        wrthost_set_peer_room_ip(ip);
        wrthost_send_cmd(event);  
        WRT_DEBUG("呼叫分机根据ID号 IP个数%d",num);
        CreateCallRoomSubWin();    
}

static void DrawTelephoneInfo(){
        WRT_xy xy;  
        WRT_Rect curbox;
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 889;
        curbox.h = 530;  
        
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
		JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
    	JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));
		if(m_cur_telephone_page != 1 && m_cur_telephone_page>0 )
				JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));
		
		
		if( m_telephone_page>1 && m_telephone_page!=m_cur_telephone_page  )
		{
			JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));
		}
        if(get_telephone_count() == 0){

        		FillRect(0xb2136,curbox); 

                DrawText(LT("没有任何联系人..."), 80,77,rgb_white);
                return;
        }
        int count = (m_cur_telephone_page-1)*9;
        int  i = 0;
        int offset = 40;
        char text[42] = {0};//将来可该改成静态的，使（log,log,使用同一内存）;
        int textlen = 0;
        long start =0;
        long tmpstart = 0;
        int islock = 0;
        int isread = 0;
        
        
        FillRect(0xb2136,curbox);
		
        TELEPHONE* pTele=0;

        /*    start = GetDumpLogStart(false);
        tmpstart = start;
        for(i=0;i<count;i++){
        if(get_log(start,text1,&islock,&isread) == 0)
        return;
        if(strlen(text1) == 0)
        i--;
        if(tmpstart == start)
        return;
        }*/    
        i=0;
        for(i=0;i<9;i++){
                pTele = get_select_telephone(count+i);
                if(pTele == 0)
                        break; 
                if(pTele->idle == 0)
                        continue;
				
				memset(text,0,42); 
				strcpy(text,pTele->room_number); 
#if 0
				if(strlen(pTele->name) != 0)   //如果存在人名 
				{ 
					strcpy(text+13,"   ");	//先加三个空格 
					strcpy(text+16,pTele->name);  //然后添加人名 
				} 
#endif
				islock = pTele->lock;

                if(m_pre_telephone_select == i && m_pre_telephone_select != m_cur_telephone_select){
                        DrawText(text,130,50+i*offset+25,rgb_white);
                        JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                }
                if(m_cur_telephone_select == i){
                        DrawText(text,130,50+i*offset+25,rgb_yellow); 
                        JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2)); 
                        if(islock){

                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1)); 
                                g_telephone_lock_status = true;                                     
                        }else{

                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));        
                                g_telephone_lock_status = false;
                        }
                        m_telephone_lock = islock;                                              
                }else{
                        DrawText(text,130,50+i*offset+25,rgb_white);  
                        JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));                  
                }                
                /* if(isread)
	                JpegDisp(100,50+i*offset,read_msg_ico_2,sizeof(read_msg_ico_2));
	                else if(isread == 0)
	                JpegDisp(100,50+i*offset,read_msg_ico_1,sizeof(read_msg_ico_1));*/
                if(islock)
                        JpegDisp(40,50+i*offset,ico_lock,sizeof(ico_lock));  
                else{
                        curbox.x = 40;
                        curbox.y = 50+i*offset;
                        curbox.w = 28;
                        curbox.h = 28;
                        FillRect(BACK_COLOR,curbox);    
                    } 

        }
		//JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4)); 
		JpegDisp(300,531,menu_sub_back4,sizeof(menu_sub_back4)); 

        //if(m_cur_telephone_select == -1){
                DrawStatusbar(m_telephone_page,m_cur_telephone_page);
        // }

}


void CreateTelephoneWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = TelephoneEventHandler;
        pWindow->NotifyReDraw = DrawTelephoneMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        int size = get_telephone_count();
        m_telephone_page =size /9;

        if(size%9)
                m_telephone_page +=1;
        if(m_telephone_page == 0)
                m_telephone_page = 1;

        WRT_DEBUG("size = %d ,m_telephone_page = %d",size,m_telephone_page);
        m_cur_telephone_page= 1;      
        pWindow->NotifyReDraw2(pWindow);       
}

void DrawTelephoneMain(){

	WRT_Rect rt;

    rt.x = 0;
    rt.y = 0;
    rt.w =1024;
    rt.h = 530;
    FillRect(0xb2136,rt);

    //JpegDisp(530,0,menu_sub_back,sizeof(menu_sub_back));
    JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));


    JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
    JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));
    JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
    JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_hujiao1,sizeof(bt_hujiao1));
        
    //JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
    //JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));          
    g_telephone_lock_status = false;
	m_cur_telephone_select = -1;
    DrawTelephoneInfo();
}

int  TelephoneEventHandler(int x,int y,int status)
{
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
                case 12:/*呼叫*/
                        if(get_telephone_count() == 0)
                                break;                    
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_hujiao2,sizeof(bt_hujiao2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_hujiao1,sizeof(bt_hujiao1));   
                                call_cur_select();       
                        }

                        break;
                case 13:/*上翻*/
                        if(m_cur_telephone_page ==1)  
                        {
                        		printf("下翻无效\n");
                        		break; 
                        }
                                                 	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                m_cur_telephone_page--;
                                if(m_cur_telephone_page < 1)
                                        m_cur_telephone_page  = 1;
                                //m_cur_telephone_page = -1;
                                m_pre_telephone_select = -1;
								m_cur_telephone_select = -1;
                                g_isUpdated  = 0;
                                DrawTelephoneInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 14:/*下翻*/  
                        if(m_telephone_page == m_cur_telephone_page) 
                        {
                        		printf("下翻无效\n");
                                break;    
                        }
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                	          
                                m_cur_telephone_page++;
                                if(m_cur_telephone_page > m_telephone_page)
                                        m_cur_telephone_page  = m_telephone_page;
                                //m_cur_telephone_page = -1;
                                m_pre_telephone_select =  -1;
								m_cur_telephone_select = -1;
                                g_isUpdated = 0;
                                DrawTelephoneInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 9:/*清空*/     
                        if(get_telephone_count() == 0)        
                                break;   
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));  
                                CreateDialogWin2(LT("你确定要清空联系人簿"),MB_OK|MB_CANCEL,clear_telephone,NULL);     
                        }                                         
                        break;
                case 10:/*删除*/
                        if(get_telephone_count() == 0)        
                                break;                  	
                        if(m_cur_telephone_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_shanchu2,sizeof(bt_shanchu2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_shanchu1,sizeof(bt_shanchu1));
                                if(g_telephone_lock_status){
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);
                                }else{
                                        CreateDialogWin2(LT("你确定要删除该联系人"),MB_OK|MB_CANCEL,del_cur_telephone,NULL);     
                                }                    
                        }
                        break;

                case 11:/*锁定*/   //这个显示锁和解锁 需要重写一些为好
                        if(get_telephone_count() == 0)        
                                break;                  	
                        if(m_cur_telephone_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                if(g_telephone_lock_status)
                                        JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                else
                                        JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));

                        }else{

                                if(g_telephone_lock_status){
                                        JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));   
                                        lock_cur_telephone(false); 
                                }else{
                                        JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));   
                                        lock_cur_telephone(true);                                         
                                }

                                g_isUpdated = 0;
                                //DrawTelephoneInfo();
                                DrawTelephoneInfo();
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
                                m_cur_telephone_select = -1;
                                m_pre_telephone_select = -1;  
                                CloseTopWindow();     
                                save_telephone();            
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
                        if(get_telephone_count() == 0)        
                                break;                 	
                        if((index+(m_cur_telephone_page-1)*9) < get_telephone_count()){
                                if(status == 0){
                                        m_pre_telephone_select = m_cur_telephone_select;
                                        m_cur_telephone_select = index;
                                        g_isUpdated = 0;
                                        DrawTelephoneInfo();
                                        g_isUpdated = 1;
                                        update_rect(0,0,1024,600);      
                                        StartButtonVocie();
                                }
                        }
                        break;                                                
                        }
                        break;
                }
        }    
        return res;
}


int del_cur_telephone(int param){
        int count = (m_cur_telephone_page-1)*9+m_cur_telephone_select;
        TELEPHONE* pTele = get_select_telephone(count);
        if(pTele)
                del_telephone(pTele);
        m_cur_telephone_select = -1;
        m_pre_telephone_select = -1;
        int size = get_telephone_count();
        m_telephone_page =size /9;
        if(size%9)
                m_telephone_page +=1;
        if(m_telephone_page == 0)
                m_telephone_page = 1;
        if(m_cur_telephone_page > m_telephone_page)
                m_cur_telephone_page = m_telephone_page;
        return 0;                	
}
int clear_telephone(int param){
        m_cur_telephone_select = -1;
        m_pre_telephone_select = -1;
        del_all_telephone();
        m_cur_telephone_page = 1;
        m_telephone_page = 1;
        return 0;   	
}
void lock_cur_telephone(bool islock){
        int count =(m_cur_telephone_page-1)*9+m_cur_telephone_select;
        TELEPHONE* pTele = get_select_telephone(count);
        if(pTele)
                lock_telephone(pTele,islock);
        // ReDrawCurrentWindow();
}

