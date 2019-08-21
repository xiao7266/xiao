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
        ITEM_WATERFEE=0,                         /*生活用水费查询*/	                                            
        ITEM_POWERFEE,                           /*电费查询*/
        ITEM_GASEFEE,                            /*气费查询*/ 
        ITEM_MANAGEFEE,                          /*管理费*/ 
        ITEM_TABLEWATERFEE,                      /*引用水*/
        ITEM_WARMFEE                             /*取暖费*/
};

static int  m_MaxQueryItem = 0;
static int  m_CurrentQueryItem = ITEM_WATERFEE;
static int  m_fee_page = 1;
static int  m_cur_fee_page = 1;
static int  m_fee_totalcount = 0;
static int  fee_flag;
/*费用浏览*/
static const WRT_Rect rc_feebrowsermenuico[4] = 
{
        {660,531,111,69},           /*上翻*/
        {780,531,111,69},            /* 下翻*/

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕			
};	      

static const WRT_Rect rc_feechaxunico[6]={
        {296,181,80,80},                //上箭头
        {650,181,80,80},                //下箭头
        {385,197,260,48},               //选择框
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回       
        {6,531,111,69}                 //关屏幕			
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
                        text1 = LT("电费");
                else if(m_CurrentQueryItem == ITEM_WATERFEE)
                        text1 = LT("生活用水费");
                else if(m_CurrentQueryItem == ITEM_GASEFEE)
                        text1 = LT("燃气费");
                else if(m_CurrentQueryItem == ITEM_MANAGEFEE)
                {
                        text1 = LT("管理费");
                }else if(m_CurrentQueryItem == ITEM_TABLEWATERFEE){
                        text1 = LT("饮用水费");
                }else if(m_CurrentQueryItem == ITEM_WARMFEE){
                        text1 = LT("取暖费");
                }else{
                        DrawText(LT("没有列表..."),80,77,rgb_white);
                        return;
                }
        }
        FEELIST* pTemp = GetFirstFeeElement();
        if(pTemp == NULL){
                DrawText(LT("正在查询..."),80,77,rgb_white);
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
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("电费"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                     DrawText(LT("电费"),x1,y1,rgb_black);
                    
  	                	//DrawText("电费",x1,y1,rgb_black);
  	                
                     break;
             case ITEM_WATERFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("生活用水费"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                    
  	                	DrawText(LT("生活用水费"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_GASEFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("煤气费"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                    
  	                	DrawText(LT("煤气费"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_MANAGEFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("管理费"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                     
  	                	FillSingleRect(rgb_white,rt);
  	                	DrawText(LT("管理费"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_TABLEWATERFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("饮用水费"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                     
  	                	DrawText(LT("饮用水费"),x1,y1,rgb_black);
  	                
                     break;
             case ITEM_WARMFEE:
                     x1 = (rc_feechaxunico[2].w - GetTextWidth(LT("取暖费"),0))/2;
                     if(x1 < 0) 
                             x1 = 0;
                     x1 += rc_feechaxunico[2].x;
                    
  	                	DrawText(LT("取暖费"),x1,y1,rgb_black);
  	                
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

        DrawText(LT("项目"),rc_feechaxunico[0].x - GetTextWidth(LT("项目"),0) - 10,rc_feechaxunico[0].y+rc_feechaxunico[0].h-30,rgb_white);
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
                case 0: /*上翻*/               	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));  
                                m_CurrentQueryItem--;                   		 
                                if(m_MaxQueryItem > 0){ 
                                        if(m_CurrentQueryItem < 0)
                                                m_CurrentQueryItem = m_MaxQueryItem - 1; //从尾部开始
                                }else{
                                        if(m_CurrentQueryItem < ITEM_WATERFEE)
                                                m_CurrentQueryItem = ITEM_WARMFEE; 
                                }
                                DrawItemInfo();
                        }                 
                        break;                
                case 1:/*下翻*/          	
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));                  	
                                m_CurrentQueryItem++;
                                if(m_MaxQueryItem > 0){
                                        if(m_CurrentQueryItem  >= m_MaxQueryItem)
                                                m_CurrentQueryItem = 0; //从头开始
                                }else{
                                        if(m_CurrentQueryItem > ITEM_WARMFEE)
                                                m_CurrentQueryItem = ITEM_WATERFEE; 
                                }
                                DrawItemInfo();
                        }                                                                                         
                        break;   
                case 3://确定
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1)); 
                                CreateFeeSubWin();
                        }           
                        break;                         
                case 4: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));                   	
                                CloseTopWindow();                   
                        }
                        break;
                case 5:/*关屏*/
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
        tmp_event[1] = m_CurrentQueryItem+1; //加1了
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
                case 0: /*上翻*/  
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
                case 1:/*下翻*/
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
                case 2: /*返回*/
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
                case 3:/*关屏*/
                        if(status == 1){ //按下
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

