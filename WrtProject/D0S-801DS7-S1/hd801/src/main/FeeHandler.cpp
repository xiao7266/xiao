#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"

#include "public_menu_res.h"
#include "window.h"
#include "c_interface.h"
//extern bool     g_rc_status[42]; 

enum{	
        ITEM_WATERFEE=0,                         /*������ˮ�Ѳ�ѯ*/	                                            
        ITEM_POWERFEE,                           /*��Ѳ�ѯ*/
        ITEM_GASEFEE,                            /*���Ѳ�ѯ*/ 
        ITEM_MANAGEFEE,                          /*�����*/ 
        ITEM_TABLEWATERFEE,                      /*����ˮ*/
        ITEM_WARMFEE                             /*ȡů��*/
};

static int  m_MaxQueryItem = 0;
static int  m_CurrentQueryItem = ITEM_WATERFEE;
static int  m_fee_page = 1;
static int  m_cur_fee_page = 1;
static int  m_fee_totalcount = 0;
static int  fee_flag;
/*�������*/
static const WRT_Rect rc_feebrowsermenuico[4] = 
{
        {660,531,111,69},           /*�Ϸ�*/
        {780,531,111,69},            /* �·�*/

        {900,531,111,69},              //����
        {6,531,111,69}                 //����Ļ			
};	      

static const WRT_Rect rc_feechaxunico[6]={
        {296,181,80,80},                //�ϼ�ͷ
        {650,181,80,80},                //�¼�ͷ
        {385,197,260,48},               //ѡ���
        {780,531,111,69},              //ȷ��
        {900,531,111,69},              //����       
        {6,531,111,69}                 //����Ļ			
};


static void DrawFeeInfo(){
        char text[72]={0};
        int offset = 40;
        char* text1 = NULL;
        if(m_MaxQueryItem > 0){
        		WRT_DEBUG("fee MaxQueryItem");
                text1 = pSystemInfo->LocalSetting.publicroom.NameOfFee[m_CurrentQueryItem];
        }else{
                if(m_CurrentQueryItem == ITEM_POWERFEE)
                        text1 = LT("���");
                else if(m_CurrentQueryItem == ITEM_WATERFEE)
                        text1 = LT("������ˮ��");
                else if(m_CurrentQueryItem == ITEM_GASEFEE)
                        text1 = LT("ȼ����");
                else if(m_CurrentQueryItem == ITEM_MANAGEFEE)
                {
                        text1 = LT("�����");
                }else if(m_CurrentQueryItem == ITEM_TABLEWATERFEE){
                        text1 = LT("����ˮ��");
                }else if(m_CurrentQueryItem == ITEM_WARMFEE){
                        text1 = LT("ȡů��");
                }else{
                        DrawText(LT("û���б�..."),80,77,rgb_white);
                        return;
                }
        }
        FEELIST* pTemp = GetFirstFeeElement();
        if(pTemp == NULL){
                DrawText(LT("���ڲ�ѯ..."),80,77,rgb_white);
                return;
        }
        WRT_Rect curbox;
        curbox.x = 0;  
        curbox.y = 0;  
        curbox.w = 1024;
        curbox.h = 530; 

        FillRect(0xb2136,curbox); 
    
        //SDL_Delay(1000);             

        int count = (m_cur_fee_page-1)*12;
        int  i = 0;
        for( i=0;i< count;i++)
                pTemp = pTemp->next;
        if(pTemp != NULL){
                for( i =0;i<9;i++){
                        if(pTemp != NULL){
                                memset(text,0,72);
                                sprintf(text,"%s",pTemp->buf);
                                DrawText(text,130,50+i*offset,rgb_white);
                        }else 
                                break;
                        pTemp = pTemp->next;
                }
        }
        DrawStatusbar(m_fee_page,m_cur_fee_page);
}

static  void DrawItemInfo(){
        int x1=0;
        int y1 = 0;
        WRT_Rect  rt;
        y1 = rc_feechaxunico[2].y+rc_feechaxunico[2].h-15;
        // JpegDisp(rc_feechaxunico[2].x,rc_feechaxunico[2].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));
        rt = rc_feechaxunico[2];
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);

        if(m_MaxQueryItem > 0){
                x1 = (rc_feechaxunico[2].w - GetTextWidth(pSystemInfo->LocalSetting.publicroom.NameOfFee[m_CurrentQueryItem],0))/2;
                if(x1 < 0) 
                        x1 = 0;
                x1 += rc_feechaxunico[2].x;
                
                	DrawText(pSystemInfo->LocalSetting.publicroom.NameOfFee[m_CurrentQueryItem],x1,y1,rgb_black);
                
                
        }else{

                switch(m_CurrentQueryItem){
             	case ITEM_POWERFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("���"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                     DrawText(LT("���"),x1,y1,rgb_black);
                    
  	                	//DrawText("���",x1,y1,rgb_black);
  	                
                     break;
             case ITEM_WATERFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("������ˮ��"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                    
  	                	DrawText(LT("������ˮ��"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_GASEFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("ú����"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                    
  	                	DrawText(LT("ú����"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_MANAGEFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("�����"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                     
  	                	FillSingleRect(rgb_white,rt);
  	                	DrawText(LT("�����"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_TABLEWATERFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("����ˮ��"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                     
  	                	DrawText(LT("����ˮ��"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_WARMFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("ȡů��"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                    
  	                	DrawText(LT("ȡů��"),x1,y1,rgb_black);
  	                
                     break;
             default:
                     break;
                }
        }
}

//////////////////////////////////////////////////////////////////////////

void CreateFeeWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = FeeEventHandler;
        pWindow->NotifyReDraw = DrawFeeMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);     
}
void DrawFeeMain(){
        WRT_Rect rt;

		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        JpegDisp(rc_feechaxunico[0].x,rc_feechaxunico[0].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));   
        JpegDisp(rc_feechaxunico[1].x,rc_feechaxunico[1].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));  
        //JpegDisp(rc_feechaxunico[2].x,rc_feechaxunico[2].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        rt = rc_feechaxunico[2];
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);

        JpegDisp(rc_feechaxunico[3].x,rc_feechaxunico[3].y,menu_ok1,sizeof(menu_ok1)); 
        JpegDisp(rc_feechaxunico[4].x,rc_feechaxunico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_feechaxunico[5].x,rc_feechaxunico[5].y,bt_guanping1,sizeof(bt_guanping1)); 

        DrawText(LT("��Ŀ"),rc_feechaxunico[0].x - GetTextWidth(LT("��Ŀ"),0) - 10,rc_feechaxunico[0].y+rc_feechaxunico[0].h-30,rgb_white);
        if(pSystemInfo->LocalSetting.publicroom.NumOfFee > 0){
                m_MaxQueryItem = pSystemInfo->LocalSetting.publicroom.NumOfFee;
                m_CurrentQueryItem = 0;
        }
        else
                m_CurrentQueryItem = ITEM_WATERFEE;
        fee_flag =1;
        DrawItemInfo();
}
int  FeeEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_feechaxunico[index],xy)){
                        pos_x = rc_feechaxunico[index].x;
                        pos_y = rc_feechaxunico[index].y;
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
                                m_CurrentQueryItem--;                   		 
                                if(m_MaxQueryItem > 0){ 
                                        if(m_CurrentQueryItem < 0)
                                                m_CurrentQueryItem = m_MaxQueryItem - 1; //��β����ʼ
                                }else{
                                        if(m_CurrentQueryItem < ITEM_WATERFEE)
                                                m_CurrentQueryItem = ITEM_WARMFEE; 
                                }
                                DrawItemInfo();
                        }                 
                        break;                
                case 1:/*�·�*/          	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));                  	
                                m_CurrentQueryItem++;
                                if(m_MaxQueryItem > 0){
                                        if(m_CurrentQueryItem  >= m_MaxQueryItem)
                                                m_CurrentQueryItem = 0; //��ͷ��ʼ
                                }else{
                                        if(m_CurrentQueryItem > ITEM_WARMFEE)
                                                m_CurrentQueryItem = ITEM_WATERFEE; 
                                }
                                DrawItemInfo();
                        }                                                                                         
                        break;   
                case 3://ȷ��
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1)); 
                                CreateFeeSubWin();
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


void CreateFeeSubWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = FeeSubEventHandler;
        pWindow->NotifyReDraw = DrawFeeSubMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;   
        WindowAddToWindowList(pWindow);  
        pWindow->NotifyReDraw2(pWindow);
        unsigned long tmp_event[4];
        tmp_event[0] = ROOM_INQFEE;
        tmp_event[1] = m_CurrentQueryItem+1; //��1��
        tmp_event[2] = tmp_event[3] = 0;
        wrthost_send_cmd(tmp_event); 
       // SYS_REQUESTFEE;
}

void DrawFeeSubMain(){
        m_fee_totalcount = GetFeeListCount();
        int size = m_fee_totalcount/9;
        if(m_fee_totalcount%9)
                size +=1;
        if(size == 0)
                size = 1;
        m_fee_page = size;	
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));
        WRT_Rect rt;

		rt.x = 0;
		rt.y = 0;
		rt.w = 1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        if(m_fee_page > 1){
        	JpegDisp(rc_feebrowsermenuico[0].x,rc_feebrowsermenuico[0].y,menu_shangfan1,sizeof(menu_shangfan1));
        	JpegDisp(rc_feebrowsermenuico[1].x,rc_feebrowsermenuico[1].y,menu_xiafan1,sizeof(menu_xiafan1));   
        }
       
        JpegDisp(rc_feebrowsermenuico[2].x,rc_feebrowsermenuico[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_feebrowsermenuico[3].x,rc_feebrowsermenuico[3].y,bt_guanping1,sizeof(bt_guanping1));             		
        DrawFeeInfo();
}
int FeeSubEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        unsigned long tmp_event[4] ;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rc_feebrowsermenuico[index],xy)){
                        pos_x = rc_feebrowsermenuico[index].x;
                        pos_y = rc_feebrowsermenuico[index].y;
                        ret = 1;
                        switch(index){
                case 0: /*�Ϸ�*/  
                        if(m_fee_totalcount == 0)        
                                break;  
                        if(m_fee_page < 2)
                        	break;                	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));                     	       
                                m_cur_fee_page--;
                                if(m_cur_fee_page < 1)
                                        m_cur_fee_page = 1;
                                DrawFeeInfo();  
                        }                 
                        break;                
                case 1:/*�·�*/
                        if(m_fee_totalcount == 0)        
                                break;  
						if(m_fee_page < 2)
                        	break;                                                  	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));                  	
                                m_cur_fee_page++;
                                if(m_cur_fee_page > m_fee_page)
                                        m_cur_fee_page =  m_fee_page; 
                                DrawFeeInfo();  
                        }                                                                                         
                        break;                                    
                case 2: /*����*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));                   	
                                DelAllFeeFromList();
                                tmp_event[0] = CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_INQFEE;
                                tmp_event[2] =tmp_event[3] =0;
                                wrthost_send_cmd(tmp_event);   
                                CloseTopWindow();                   
                        }
                        break;
                case 3:/*����*/
                        if(status == 1){ //����
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
                                tmp_event[0] = CUSTOM_CMD_CANCEL;
                                tmp_event[1] = ROOM_INQFEE;
                                tmp_event[2] = tmp_event[3] = 0;
                                wrthost_send_cmd(tmp_event);                        
                                DelAllRingFromList(); 
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

