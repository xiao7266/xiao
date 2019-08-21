#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "ring_ico_res.h"
#include "window.h"
#include "c_interface.h"


//extern bool     g_rc_status[42]; 
extern  WRT_Rect rc_infobrowsermenuico[17];


bool isdownload = false;

enum{
        ITEM_CENTERCALL=1,                   /*���ĺ�������*/
        ITEM_WALLCALL,                     /*Χǽ����������*/
        ITEM_DOORCALL,                      /*�ſڻ���������*/
        ITME_LITTLEDOORCALL,               /*С�ſڻ���������*/
        ITEM_ROOMCALL,                     /*����ͨ����*/
        ITEM_WARING,                       /*��������*/
        ITEM_MSG,                          /*����Ϣ����*/
        ITEM_ALARM_CLOCK                    /*��������*/
};
static int  m_CurrentCallRingtem = ITEM_CENTERCALL;

static int m_ring_page = 1;
static int m_cur_ring_page = 1;
static int m_cur_ring_select = -1;
static int m_pre_ring_select = -1;
static int m_ring_totalcount = 0;

static RINGLIST* pdownring = NULL;

static void DrawRingInfo(){

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        if(m_cur_ring_page!=1 && m_cur_ring_page>0)
                JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

        if(m_ring_page > 1 && m_cur_ring_page!=m_ring_page)
                JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));   

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));  

        if(GetFirstRingElement() == NULL){
                DrawText(LT("���ڲ�ѯ..."),80,77,rgb_white);
                return;
        }

        RINGLIST* pTemp = GetFirstRingElement();
        WRT_xy xy;  
        WRT_Rect curbox;
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 889;
        curbox.h = 530;//�߶� 
        if(m_cur_ring_select == -1){
        	FillRect(0xb2136,curbox); 
		}

        int count = (m_cur_ring_page-1)*9;
        int  i = 0;
        int offset = 42;
        char text[72]={0};
        int tmpid = count;

        for( i=0;i< count;i++)
                pTemp = pTemp->next;
        if(pTemp == NULL)
                return;
        for(i =0; i<9; i++){
                if(pTemp != NULL){
                        tmpid++;
                        memset(text,0,72);            
                        sprintf(text,"%d.%s",tmpid,pTemp->name);
                        if(m_cur_ring_select == i){
                                DrawText(text,130,50+(i)*offset+24,rgb_yellow);
                                JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2)); 
                        }else{
                                DrawText(text,130,50+(i)*offset+24,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1)); 
                        }
                        if(m_pre_ring_select == i && m_pre_ring_select != m_cur_ring_select){
                                DrawText(text,130,50+(i)*offset+24,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                        }
                        JpegDisp(100,50+i*offset,ico_lingsheng,sizeof(ico_lingsheng));
                }else
                        break;
                pTemp=pTemp->next;
        }     

        DrawStatusbar(m_ring_page,m_cur_ring_page);
}

void CreateRingDownWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        m_cur_ring_page = 1; 
        m_ring_page = 1;
        pWindow->EventHandler = RingDownEventHandler;
        pWindow->NotifyReDraw = DrawRingDownMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        DelAllRingFromList(); 
        pWindow->NotifyReDraw2(pWindow);     
        unsigned long tmp_event[4];
        tmp_event[0] = ROOM_BROWSERING;
        tmp_event[1] = tmp_event[2] = tmp_event[3] = 0;
        wrthost_send_cmd(tmp_event);

}
void DrawRingDownMain(){
        m_ring_totalcount = GetRingListCount();
        int size = m_ring_totalcount/9;
        if(m_ring_totalcount%9)
                size +=1;
        if(size == 0)
                size = 1;
        m_ring_page = size;	
        if(m_cur_ring_page > m_ring_page)
                m_cur_ring_page = 1;
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

        JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));
        //JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
        //JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));

        JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,menu_lingshengxiazai1,sizeof(menu_lingshengxiazai1));    
        JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,menu_lingshengshiting1,sizeof(menu_lingshengshiting1));

        DrawRingInfo();
}

int RingDownEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        xy.x = x;
        xy.y = y;
        unsigned long tmp[4];
        int pos_x,pos_y;
        for(index =0;index<17;index++){
                if(IsInside(rc_infobrowsermenuico[index],xy)){
                        pos_x  = rc_infobrowsermenuico[index].x;
                        pos_y  = rc_infobrowsermenuico[index].y;
                        ret = 1;
                        switch(index){
                            	  	
                  case 9://����
                          if(m_ring_totalcount == 0)        
                                  break;    
                          if(m_cur_ring_select == -1)
                                  break;
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_lingshengshiting2,sizeof(menu_lingshengshiting2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_lingshengshiting1,sizeof(menu_lingshengshiting1));    
                                  RINGLIST* pTemp = GetSelectRingElement(m_cur_ring_select+(m_cur_ring_page-1)*9);
                                  if(pTemp  == NULL)
                                          break;
                                  if(pTemp->isdown){

                                          CreateDialogWin2(LT("�����Ѿ����أ��뵽��������������!"),MB_OK,NULL,NULL);                  
                                  }else{
                                          if(pTemp->addr != NULL && pTemp->len > 0){
                                                  SDL_Delay(1000);
                                                  WRT_DEBUG("mp3play = 8");
                                                  mp3stop();
                                                  mp3play((char*)pTemp->addr,pTemp->len,8); 
                                                  CreateDialogWin2(LT("���Ҫ���أ��밴���ذ�ť!"),MB_OK,downring_ok,NULL);
                                                  break;
                                          } 

                                          int tmplen = strlen(pTemp->name);
                                          char* pName = (char*)ZENMALLOC(tmplen+1);
                                          memset(pName,0,tmplen+1);
                                          strcpy(pName,pTemp->name);             
                                          isdownload =false;
                                          tmp[0] = ROOM_RINGDOWN;
                                          tmp[1] = (unsigned long)pName;
                                          tmp[2] = strlen(pName);
                                          tmp[3] = 0;
                                          wrthost_send_cmd(tmp);
                                          CreateDialogWin2(LT("���Եȣ����ڻ���..."),MB_CANCEL,NULL,cancel_listenring);
                                  }  
                          }
                          break;            	  	
                  case 10: /*����*/
                          if(m_ring_totalcount == 0)        
                                  break;    
                          if(m_cur_ring_select == -1)
                                  break;             	
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_lingshengxiazai2,sizeof(menu_lingshengxiazai2));
                          }else{

                                  JpegDisp(pos_x,pos_y,menu_lingshengxiazai1,sizeof(menu_lingshengxiazai1));    
                                  RINGLIST* pTemp = GetSelectRingElement(m_cur_ring_select+(m_cur_ring_page-1)*9);
                                  if(pTemp  == NULL)
                                          break;
                                  if(pTemp->isdown){
                                  	  	  g_isUpdated = 0;
                                          DrawRingInfo();
                                          g_isUpdated = 1;
                                	  	  update_rect(0,0,1024,600);      
                                          CreateDialogWin2(LT("�������Ѿ�����"),MB_OK,NULL,NULL); 

                                          //���棿
                                  }else{   
                                          pdownring = pTemp;
                                          CreateDialogWin2(LT("ȷ��Ҫ����!"),MB_OK|MB_CANCEL,download_ring,NULL);                       
                                  }  
                          }
                          break;
                  case 13: /*�Ϸ�*/  
                          if(m_cur_ring_page == 1)        
                                  break;                	
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));                	          
                                  m_cur_ring_page--;
                                  if(m_cur_ring_page < 1)
                                          m_cur_ring_page  = 1;
                                  m_pre_ring_select = -1;
                                  m_cur_ring_select = -1;                        
                                  g_isUpdated = 0;
                                  DrawRingInfo();  
                                  g_isUpdated = 1;
                                  update_rect(0,0,1024,600);      
                          }                      

                          break;
                  case 14:/*�·�*/
                          if(m_cur_ring_page == m_ring_page)
                                  break;                 	
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                          }else{  
                                  JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));               	
                                  m_cur_ring_page++;
                                  if(m_cur_ring_page > m_ring_page)
                                          m_cur_ring_page  = m_ring_page;
                                  m_pre_ring_select = -1;
                                  m_cur_ring_select = -1;                        
                                  g_isUpdated = 0;
                                  DrawRingInfo();             
                                  g_isUpdated = 1;
                                  update_rect(0,0,1024,600);          
                          }
                          break;
                  case 15: /*����*/
                          if(status == 1){
                          			//mp3stop(); 
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));                    	
                                  DelAllRingFromList(); 
                                  if(wrthost_sysidle())
                                          mp3stop();                                 
                                  m_ring_totalcount  = 0;
                                  m_pre_ring_select = -1;
                                  m_cur_ring_select = -1;
                                  m_cur_ring_page = 1;
                                  tmp[0] = CUSTOM_CMD_CANCEL;
                                  tmp[1] = ROOM_BROWSERING;
                                  tmp[2] = tmp[3] = 0;
                                  wrthost_send_cmd(tmp);
                                  CloseTopWindow();  
                          }                       
                          break;
                  case 16:/*����*/
                          if(status == 1){ //����
                          		//mp3stop(); 
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1)); 
                                  if(wrthost_sysidle())
                                          mp3stop();                           
                                  tmp[0] = CUSTOM_CMD_CANCEL;
                                  tmp[1] = ROOM_BROWSERING;
                                  tmp[2] = tmp[3] = 0;
                                  wrthost_send_cmd(tmp);                        
                                  DelAllRingFromList(); 
                                  screen_close(); 
                                  CloseAllWindows();
                          }
                          break;
                  default:
                          if(m_ring_totalcount== 0)        
                                  break;      
                          if((index + (m_cur_ring_page-1)*9) < m_ring_totalcount){

                                  if(status == 0){    	
                                          m_pre_ring_select = m_cur_ring_select;
                                          m_cur_ring_select = index;
                                          g_isUpdated = 0;
                                          DrawRingInfo();
                                          g_isUpdated = 1;
                                	 	  update_rect(0,0,1024,600);      
                                  }else
                                          StartButtonVocie();
                          }

                          break;
                        }
                        break;
                }
        } 
        return ret;   
}

int download_ring(int param){
        if(pdownring== 0)
                return 0;
        unsigned long tmp[4];	
        RINGLIST* pTemp = pdownring;
        if(pTemp->addr != NULL && pTemp->len != 0){
                //��ʾ�Ѿ����������б��ֻ��Ҫ���浹���ص��б���
                CreateDialogWin2(LT("���ڱ�������..."),MB_NONE,NULL,NULL);
                if(AddRingToDownList(pTemp->name,pTemp->len,pTemp->addr) == 1){
                        pTemp->addr = NULL;
                        pTemp->len = 0;
                        pTemp->isdown = 1;
                        CloseWindow();
                        CreateDialogWin2(LT("�������!"),MB_NONE,NULL,NULL);
                        SDL_Delay(2000);
                        CloseWindow();
                }else{
                        pTemp->addr = NULL;
                        pTemp->len = 0;
                        CloseWindow();
                        CreateDialogWin2(LT("��������ʧ��!"),MB_NONE,NULL,NULL);
                        SDL_Delay(2000);
                        CloseWindow(); 

                }
                return 0;
        } 
        int tmplen = strlen(pTemp->name);   
        char* pName = (char*)ZENMALLOC(tmplen+1);
        memset(pName,0,tmplen+1);
        strcpy(pName,pTemp->name);   
        isdownload =true;
        WRT_DEBUG("pName = %s ",pName);              
        tmp[0] = ROOM_RINGDOWN;
        tmp[1] = (unsigned long)pName;
        tmp[2] = strlen(pName);
        tmp[3] = 0;
        wrthost_send_cmd(tmp);	
        return 0;

}

int cancel_listenring(int param){
        unsigned long tmp[4];	
        tmp[0] = CUSTOM_CMD_CANCEL;
        tmp[1] = ROOM_RINGDOWN;
        tmp[2] = 0;
        tmp[3] = 0;
        wrthost_send_cmd(tmp);
        return 0;

}

//////////////////////////////////////////////////////////////////
//�������ص���������

static int m_download_page = 1;
static int m_cur_download_page = 1;
static int m_cur_download_select = -1;
static int m_pre_download_select = -1;
static bool g_download_lock_status = false;
static int g_cur_lock_status = 0;

char* str_find_pos(char* str,int c){
	char* ptemp = str;
	char* ret_str = NULL;
	while(1){
		ret_str = strchr(ptemp,c);
		if(ret_str != NULL){
			ptemp = ret_str+1;
		}
		else
			break;
	}
	return ptemp;
}

static void DrawDownLoadInfo(){
        WRT_xy xy;  
        WRT_Rect curbox;
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 889;
        curbox.h = 530;   
        if(get_downring_count() == 0){

        	FillRect(0xb2136,curbox); 
 
                DrawText(LT("û�������ص�����!"), 80,77,rgb_white);

                return;
        }
        init_downring();
        DOWNRINGINFO* pTemp = 0;//�������β
        int count = (m_cur_download_page-1)*9;
        int  i = 0;
        int offset = 40;
        char text[72] = {0};
        int textlen = 0;
        // printf("m_cur_download_select = %d \n",m_cur_download_select);
        
        if(m_cur_download_select == -1){
        	FillRect(0xb2136,curbox); 
        }
     
        for( i =0;i<9;i++){
                pTemp = get_select_downring(count+i);
                if(pTemp != NULL && pTemp->idle == 1){
                        memset(text,0,72);
                       // printf("Name %s\n",(char*)str_find_pos((pTemp->name),'//'));
                        //sprintf(text,"%s...",(int)(pTemp->id),(char*)pTemp->buf);
                        strcpy(text,(char*)str_find_pos((pTemp->name),'//'));                 
                        if(m_cur_download_select == i){
                                JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2));   
                                DrawText(text,130,50+i*offset+23,rgb_yellow);  

                                //if(pTemp->lock){
                                //         JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));                      
                                //}else{

                                //         JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));                 
                                //}
                                g_download_lock_status = g_cur_lock_status = pTemp->lock;                
                        }else{
                                DrawText(text,130,50+i*offset+23,rgb_white); 
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));  

                        }
                        if(m_pre_download_select == i){
                                DrawText(text,130,50+i*offset+23,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));          
                        }
                        JpegDisp(100,50+i*offset,ico_lingsheng,sizeof(ico_lingsheng));
                        if(pTemp->lock){
                                JpegDisp(40,50+i*offset,ico_lock,sizeof(ico_lock));
                        }else{
                                curbox.x = 40;
                                curbox.y = 50+i*offset;
                                curbox.w = curbox.h = 28;
                                FillRect(0xb2136,curbox);
                                // tm_wkafter(20);             
                        }   

                }else
                        break;

        }

        if(m_cur_download_select == -1){
                DrawStatusbar(m_download_page,m_cur_download_page);
        }
}

/////////////////////////////////////////////////////////////////////////////
//

int IsDelCurRing(){
        int count = 0;
        count =(m_cur_download_page-1)*9+m_cur_download_select ;
        DOWNRINGINFO* curinfo = get_select_downring(count);
        if(strstr(curinfo->name,"/zh") != 0 ||strstr(curinfo->name,"/en"))
                return 0;  
        return 1;
}

static void DrawDownloadCurSelect(){
        int count = 0;
        count =(m_cur_download_page-1)*9+m_cur_download_select ;
        char text[72];
        DOWNRINGINFO* curinfo = get_select_downring(count);
        if(curinfo && curinfo->idle == 1){
                //��������������������
                //long size =0;
                //printf("��������\n");
                 //mp3play((char*)curinfo->ring_path,0,0);
                //ZENFREE(voice_data);
                //����ǰ��������Ϣ����������
                //m_CurrentCallRingtem ��ǰ��������
                // if(wrtip_
                unsigned char* addr=0;
                long size =0;
                strcpy(pSystemInfo->mySysInfo.ringname[m_CurrentCallRingtem-1],curinfo->name);
                wrtip_set_ring(m_CurrentCallRingtem-1,curinfo->name,0);
                save_localhost_config_to_flash();
        }
}

int set_cur_ring(int param){
        DrawDownloadCurSelect();
        if(wrthost_sysidle())
                mp3stop();  
        return 0;
        //mp3stop();//ֹͣ����MP3��
}
int cancel_set_cur_ring(int param){
        if(wrthost_sysidle())
                mp3stop();  
        return 0;
        //mp3stop();//ֹͣ����MP3��
}

void CloseDownLoadRingWin(){
        free_mem_all_downring();
}

void CreateDownLoadRingWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = DownLoadRingEventHandler;
        pWindow->NotifyReDraw = DrawDownLoadRingMain;
        pWindow->CloseWindow = CloseDownLoadRingWin;
        pWindow->valid = 1;
        pWindow->type = CHILD; 
        WindowAddToWindowList(pWindow); 
        
        init_downring();
        int size = get_downring_count();
        m_download_page =size /9;
        if(size%9)
                m_download_page +=1;
        if(m_download_page == 0)
                m_download_page = 1;
        m_cur_download_page =1 ;
        m_cur_download_select = -1;
        m_pre_download_select = -1; 
        WRT_MESSAGE("size = %d,m_download_page = %d \n",size,m_download_page);
        pWindow->NotifyReDraw2(pWindow);     
}
void DrawDownLoadRingMain(){
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

        JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
        JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));
        //JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));    
        JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));

        if(m_cur_download_page >1)
                JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

        if(m_cur_download_page < m_download_page)
                JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));   

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));          
        g_download_lock_status = false;

        DrawDownLoadInfo();
}
int DownLoadRingEventHandler(int x,int y,int status){
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
                case 12:
                        if(get_downring_count() == 0)
                                break;    
                        if(m_cur_download_select == -1)
                                break;                
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_bofang2,sizeof(bt_bofang2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_bofang1,sizeof(bt_bofang1));
                                DOWNRINGINFO* pTemp = get_select_downring ((m_cur_download_page-1)*9+m_cur_download_select);
                                if(pTemp){
                                        char pTmpText[70]                   	     ;

                                        switch(m_CurrentCallRingtem)
                                        {
                                        case ITEM_CENTERCALL:
                                                // pTmpText ="ȷ��Ҫ���õ�ǰ����Ϊ'���ĺ�������'��?";
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("���ĺ�������"));
                                                break;
                                        case ITEM_WALLCALL:      
                                                // pTmpText ="ȷ��Ҫ���õ�ǰ����Ϊ'Χǽ����������'��?"; 
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("Χǽ����������"));
                                                break;
                                        case ITEM_DOORCALL:
                                                // pTmpText ="ȷ��Ҫ���õ�ǰ����Ϊ'�ſڻ���������'��?";
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("�ſڻ���������"));
                                                break;
                                        case ITME_LITTLEDOORCALL:
                                                //pTmpText ="ȷ��Ҫ���õ�ǰ����Ϊ'С�ſڻ���������'��?";
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("С�ſڻ���������"));
                                                break;
                                        case ITEM_ROOMCALL:
                                                // pTmpText ="ȷ��Ҫ���õ�ǰ����Ϊ'����ͨ����'��?";
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("����ͨ����"));
                                                break;
                                        case ITEM_WARING:
                                                //pTmpText = "ȷ��Ҫ���õ�ǰ����Ϊ'��������'��?";
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("��������"));
                                                break;
                                        case ITEM_MSG:
                                                //pTmpText = "ȷ��Ҫ���õ�ǰ����Ϊ'��������'��?";
                                                sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("��������"));
                                                break;
                                        case ITEM_ALARM_CLOCK:
                                        	sprintf(pTmpText,"%s'%s'",LT("ȷ��Ҫ���õ�ǰ����Ϊ"),LT("��������"));
                                        	break;                            	    
                                        default:
                                                break;
                                        }                    	 
                                        CreateDialogWin2(pTmpText,MB_OK|MB_CANCEL,set_cur_ring,cancel_set_cur_ring);

                                        if(pTemp->name){ 
                                                SDL_Delay(200);
                                                if(wrthost_sysidle())
                                                        mp3stop();  
                                                mp3play((char*)pTemp->name,0,8);
                                        }
                                }
                        }                
                        break;
                case 13:/*�Ϸ�*/
                        if(get_downring_count() == 0)        
                                break;  
                        if(m_cur_download_page == 1)
                                break;                	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                m_cur_download_page--;
                                if(m_cur_download_page < 1)
                                        m_cur_download_page  = 1;
                                m_cur_download_select = -1;
                                m_pre_download_select = -1;
                                g_isUpdated  = 0 ;
                                DrawDownLoadRingMain();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 14:/*�·�*/  
                        if(get_downring_count() == 0)        
                                break;  
                        if(m_download_page == m_cur_download_page)
                                break;                                       	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                	          
                                m_cur_download_page++;
                                if(m_cur_download_page > m_download_page)
                                        m_cur_download_page  = m_download_page;
                                m_cur_download_select = -1;
                                m_pre_download_select =  -1;
                                g_isUpdated = 0;
                                DrawDownLoadRingMain();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 9:/*���*/     
                        if(get_downring_count() == 0)        
                                break;   
                        if(status == 1){
                                StartButtonVocie();
                                SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));  
                                CreateDialogWin2(LT("��ȷ��Ҫ�����������"),MB_OK|MB_CANCEL,clear_download,NULL);     
                        }          
                        
                        break;
                case 10:/*ɾ��*/
                        if(get_downring_count() == 0)        
                                break;                  	
                        if(m_cur_download_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
								SDL_Delay(200);
                                JpegDisp(pos_x,pos_y,bt_shanchu2,sizeof(bt_shanchu2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_shanchu1,sizeof(bt_shanchu1)); 
                                if(IsDelCurRing()){
                                        if(g_cur_lock_status == 0)
                                                CreateDialogWin2(LT("��ȷ��Ҫɾ��������"),MB_OK|MB_CANCEL,del_cur_download,NULL);    
                                        else
                                                CreateDialogWin2(LT("����״̬,����ɾ��"),MB_OK,NULL,NULL);
                                }else{
                                        CreateDialogWin2(LT("Ĭ������,����ɾ��"),MB_OK,NULL,NULL);
                                }
                                //init_downring();
                                //DrawDownLoadRingMain();
                        }                    
                        break;
                case 11:/*����*/
                		break;
                		
                        if(get_downring_count() == 0)        
                                break;                  	
                        if(m_cur_download_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                if(g_download_lock_status){
                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                }else{
                                                JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                                }
                        }else{
                                if(g_download_lock_status){
                                                JpegDisp(pos_x,pos_y,bt_jiesuo1,sizeof(bt_jiesuo1));
                                        lock_cur_download(false);
                                        //printf("����\n");
                                }else{
                                                JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));   
                                        lock_cur_download(true); 
                                        // printf("����\n");
                                }
                        }                 	  
                        break;
                case 15: /*����*/
                        if(status == 1){
                        		if(wrthost_sysidle())
                                        mp3stop(); 
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));                   	
                                m_cur_download_select = -1;
                                m_pre_download_select = -1;  
                                //if(wrthost_sysidle())
                                //        mp3stop(); 
                                CloseTopWindow();  
                                save_downring();               
                        }

                        break;
                case 16:/*����*/
                        if(status == 1){ //����
                        		//mp3stop();
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
                                screen_close(); 
                                if(wrthost_sysidle())
                                        mp3stop();                         
                                CloseAllWindows();
                        }
                        break;
                default:
                        if(get_downring_count() == 0)        
                                break;   

                        if((index+(m_cur_download_page-1)*9) < get_downring_count()){
                                if(status != 1){
                                        m_pre_download_select = m_cur_download_select;
                                        m_cur_download_select = index;
                                        if(m_cur_download_select != m_pre_download_select){
                                        	g_isUpdated = 0;
                                            DrawDownLoadInfo();
                                            g_isUpdated = 1;
                            				update_rect(0,0,1024,600);   
                            				SDL_Delay(150);
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

void lock_cur_download(bool islock){
        DOWNRINGINFO* curinfo = get_select_downring((m_cur_download_page-1)*9+m_cur_download_select);
        if(curinfo != NULL)
        {
                lock_downring(curinfo,islock);	
        }
        // ReDrawCurrentWindow();
        g_isUpdated = 0;
        DrawDownLoadInfo();
        g_isUpdated = 1;
		update_rect(0,0,1024,600);      
}

int  del_cur_download(int param){
        int i =0;
        DOWNRINGINFO* curinfo = get_select_downring((m_cur_download_page-1)*9+m_cur_download_select);
        if(curinfo == NULL)
                return 0 ;
        if(curinfo->lock)
                return 0;

        for(i=0;i<8;i++){
                if(strcmp(pSystemInfo->mySysInfo.ringname[i],curinfo->name) == 0){
                        memset(pSystemInfo->mySysInfo.ringname[i],0,256);
                        wrtip_set_ring(i,0,0); //�ָ�Ĭ�ϵ�
                        //   printf("go default\n");
                }
        }
        

        del_downring(curinfo);
        init_downring();
        int size = get_downring_count();
        m_download_page =size /9;
        if(size%9)
                m_download_page +=1;
        if(m_download_page == 0)
                m_download_page = 1;
        if(m_cur_download_page > m_download_page)
                m_cur_download_page = m_download_page;
        
	return 0;                
}
int clear_download(int param){
        int i =0;
        for(i=0;i<8;i++){
                memset(pSystemInfo->mySysInfo.ringname[i],0,256);
                wrtip_set_ring(i,0,0); //�ָ�Ĭ�ϵ�

        }    
        m_cur_download_select = -1;
        m_pre_download_select = -1;
        del_all_downring();
        m_cur_download_page = 1;
        m_download_page = 1;
        init_downring();
        DrawDownLoadRingMain();
	return 0;        	
}

//////////////////////////////////////////////////////////////////////////////////////
//��������



static const WRT_Rect rc_callringmenuico[6]={
        {296,181,80,80},                //�ϼ�ͷ
        {650,181,80,80},                //�¼�ͷ
        {385,197,260,48},               //ѡ���
        {780,531,111,69},              //ȷ��
        {900,531,111,69},              //����       
        {6,531,111,69}                 //����Ļ			
};
static void DrawCallRingItemInfo(){
        int x1=0;
        int y1 = 0;
        WRT_Rect rt;
        rt = rc_callringmenuico[2];
        // x1 = rc_callringmenuico[2].x+rc_callringmenuico[2].w/4;
        y1 = rc_callringmenuico[2].y+rc_callringmenuico[2].h-15;
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);
        DrawRect(rc_callringmenuico[2],2,rgb_white);

        //JpegDisp(rc_callringmenuico[2].x,rc_callringmenuico[2].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));
        switch(m_CurrentCallRingtem){
        case ITEM_CENTERCALL:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("���ĺ�������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
    
                DrawText(LT("���ĺ�������"),x1,y1,rgb_black);

                break;
        case ITEM_WALLCALL:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("Χǽ����������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("Χǽ����������"),x1,y1,rgb_black);
                break;
        case ITEM_DOORCALL:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("�ſڻ���������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("�ſڻ���������"),x1,y1,rgb_black);
                break;
        case ITME_LITTLEDOORCALL:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("С�ſڻ���������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("С�ſڻ���������"),x1,y1,rgb_black);
                break;
        case ITEM_ROOMCALL:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("����ͨ����"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("����ͨ����"),x1,y1,rgb_black);
                break;
        case ITEM_WARING:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("��������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("��������"),x1,y1,rgb_black);
                break;
        case ITEM_MSG:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("��������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("��������"),x1,y1,rgb_black);
                break;
        case ITEM_ALARM_CLOCK:
                x1 = (rc_callringmenuico[2].w - GetTextWidth(LT("��������"),0))/2;
                if(x1 < 0)
                        x1 = 0;
                x1 += rc_callringmenuico[2].x;
                DrawText(LT("��������"),x1,y1,rgb_black);      	
        	break;
        default:
                break;
        }
}

void CreateRingWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = RingEventHandler;
        pWindow->NotifyReDraw = DrawRingMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow); 
}
void DrawRingMain(){
        WRT_Rect rt;

		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        JpegDisp(rc_callringmenuico[0].x,rc_callringmenuico[0].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));   
        JpegDisp(rc_callringmenuico[1].x,rc_callringmenuico[1].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));  
        // JpegDisp(rc_callringmenuico[2].x,rc_callringmenuico[2].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
		rt = rc_callringmenuico[2];
		DeflateRect(&rt,4);
        DrawRect(rc_callringmenuico[2],2,rgb_white);  
        FillRect(rgb_white,rt);
       
        JpegDisp(rc_callringmenuico[4].x,rc_callringmenuico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_callringmenuico[5].x,rc_callringmenuico[5].y,bt_guanping1,sizeof(bt_guanping1)); 
        DrawText(LT("��Ŀ"),rc_callringmenuico[0].x- GetTextWidth(LT("��Ŀ"),0) - 10,rc_callringmenuico[0].y+rc_callringmenuico[0].h-30,rgb_white);

        m_CurrentCallRingtem = ITEM_CENTERCALL;
        DrawCallRingItemInfo();
        WRT_Rect curbox;
        curbox.x = 135;
        curbox.y = 531;
        curbox.w = 578;
        curbox.h = 55;
		JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));    
		DrawText_16(LT("��������"), curbox.x,curbox.y+45,rgb_black);
		JpegDisp(rc_callringmenuico[3].x,rc_callringmenuico[3].y,menu_ok1,sizeof(menu_ok1));          
     
}
int RingEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_callringmenuico[index],xy)){
                        pos_x = rc_callringmenuico[index].x;
                        pos_y = rc_callringmenuico[index].y;
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
                case 0: /*�Ϸ�*/               	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));                     	       
                                m_CurrentCallRingtem--;
                                if(m_CurrentCallRingtem < ITEM_CENTERCALL)
                                        m_CurrentCallRingtem = ITEM_ALARM_CLOCK; 
                                DrawCallRingItemInfo();
                        }                 
                        break;                
                case 1:/*�·�*/          	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));                  	
                                m_CurrentCallRingtem++;
                                if(m_CurrentCallRingtem > ITEM_ALARM_CLOCK)
                                        m_CurrentCallRingtem = ITEM_CENTERCALL;
                                DrawCallRingItemInfo();
                        }                                                                                         
                        break;   
                case 3://ȷ��
                        if(status == 1){
                        		
                                StartButtonVocie();
                                SDL_Delay(300);
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1)); 
                                CreateDownLoadRingWin();
                        }  
                        break;                                  
                case 4: /*����*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));                   	
                                CloseTopWindow();
                        }
                        break;
                case 5:/*����*/
                        if(status == 1){ //����
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
