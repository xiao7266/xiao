#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <SDL.h>
#include "audiovideo.h"
#include "Handler.h"
#include "public_menu_res.h"
#include "window.h"




T_JIAJU_TABLE g_JIAJU_TABLE[MAX_JIAJU_TABLE_COUNT];

int     g_cur_select_room_type = 0;   //��ǰѡ��ķ������
int     g_cur_select_room = 0;        //��ǰѡ��ķ���
int     g_cur_select_elec_type = 0;   //��ǰѡ��ĵ������
int     g_cur_select_elec = 0 ;       //��ǰѡ��ĵ���

int     g_create_set_type = 0;

MENU_ITEM    g_ROOMTYPE[MAX_ROOM_SORT]={{"����",0,1},{"����",1,2},{"����",2,3},{"�鷿",3,4},{"����",4,5},{"������",5,6},{"��̨",6,7},{"�Զ���",7,8}};
MENU_ITEM    g_ROOMNAME[MAX_ROOM_SORT][MAX_ROOM_NAME]={
        {{"������",0,1},{"����һ",1,2},{"���Ҷ�",2,3},{"������",3,4},{"������",4,5},{"������",5,6},{"������",6,7},{"������",7,8},{"���Ұ�",8,9},{"���Ҿ�",9,10}},
        {{"����",0,1},{"����һ",1,2},{"������",2,3},{"������",3,4},{"������",4,5},{"������",5,6},{"������",6,7},{"������",7,8},{"������",8,9},{"������",9,10}},
        {{"����",0,1},{"����һ",1,2},{"������",2,3},{"������",3,4},{"������",4,5},{"������",5,6},{"������",6,7},{"������",7,8},{"������",8,9},{"������",9,10}},
        {{"�鷿",0,1},{"�鷿һ",1,2},{"�鷿��",2,3},{"�鷿��",3,4},{"�鷿��",4,5},{"�鷿��",5,6},{"�鷿��",6,7},{"�鷿��",7,8},{"�鷿��",8,9},{"�鷿��",9,10}},
        {{"����",0,1},{"����һ",1,2},{"������",2,3},{"������",3,4},{"������",4,5},{"������",5,6},{"������",6,7},{"������",7,8},{"������",8,9},{"������",9,10}},
        {{"������",0,1},{"������һ",1,2},{"�������",2,3},{"��������",3,4},{"��������",4,5},{"��������",5,6},{"��������",6,7},{"��������",7,8},{"�������",8,9},{"�������",9,10}},
        {{"��̨",0,1},{"��̨һ",1,2},{"��̨��",2,3},{"��̨��",3,4},{"��̨��",4,5},{"��̨��",5,6},{"��̨��",6,7},{"��̨��",7,8},{"��̨��",8,9},{"��̨��",9,10}},
        {{"�Զ���һ",0,1},{"�Զ����",1,2},{"�Զ�����",2,3},{"�Զ�����",3,4},{"�Զ�����",4,5},{"�Զ�����",5,6},{"�Զ�����",6,7},{"�Զ����",7,8},{"�Զ����",8,9},{"�Զ���ʮ",9,10}}
};

MENU_ITEM    g_ELECTYPE[MAX_ELEC_SORT]={{"�ƹ�",0 ,1},{"����",1, 2},{"����",2, 3},{"�յ�",3, 4},{"DVD",4, 5},{"����",5,6},{"��Ӧ��",6,7},{"�Զ���",7, 8}};

MENU_ITEM    g_ELECTNAME[MAX_ELEC_SORT][MAX_ELEC_NAME]={
        {{"�ƹ�һ",0, 1},{"�ƹ��",1, 2},{"�ƹ���",2, 3},{"�ƹ���",3, 4},{"�ƹ���",4, 5},{"�ƹ���",5,6},{"�ƹ���",6, 7},{"�ƹ��",7, 8},{"�ƹ��",8, 9},{"�ƹ�ʮ",9, 10}},
        {{"����һ",0, 1},{"������",1, 2},{"������",2, 3},{"������",3, 4},{"������",4, 5},{"������",5,6},{"������",6, 7},{"������",7, 8},{"������",8, 9},{"����ʮ",9, 10}},
        {{"����һ",0, 1},{"���Ӷ�",1, 2},{"������",2, 3},{"������",3, 4},{"������",4, 5},{"������",5,6},{"������",6, 7},{"���Ӱ�",7, 8},{"���Ӿ�",8, 9},{"����ʮ",9, 10}},
        {{"�յ�һ",0, 1},{"�յ���",1, 2},{"�յ���",2, 3},{"�յ���",3, 4},{"�յ���",4, 5},{"�յ���",5,6},{"�յ���",6, 7},{"�յ���",7, 8},{"�յ���",8, 9},{"�յ�ʮ",9, 10}},
        {{"DVDһ",0, 1}, {"DVD��",1, 2}, {"DVD��",2, 3},{"DVD��",3, 4},{"DVD��",4, 5},{"DVD��",5,6},{"DVD��",6, 7},{"DVD��",7, 8},{"DVD��",8, 9},{"DVDʮ",9, 10}},
        {{"����һ",0, 1},{"������",1, 2},{"������",2, 3},{"������",3, 4},{"������",4, 5},{"������",5,6},{"������",6, 7},{"������",7, 8},{"������",8, 9},{"����ʮ",9, 10}},
        {{"�¶ȸ�Ӧ��",0, 1},{"ʪ�ȸ�Ӧ��",1, 2},{"�ƶ���Ӧ��",2, 3},{"���ո�Ӧ��",3, 4},{"�����Ӧ��",4, 5},{"һ����̼��Ӧ��",5,6},{"������̼��Ӧ��",6, 7},{"������Ӧ��",7, 8},{"������Ӧ��",8, 9},{"������Ӧ��",9, 10}},
        {{"�Զ���һ",0, 1},{"�Զ����",1, 2},{"�Զ�����",2, 3},{"�Զ�����",3, 4},{"�Զ�����",4, 5},{"�Զ�����",5,6},{"�Զ�����",6, 7},{"�Զ����",7, 8},{"�Զ����",8, 9},{"�Զ���ʮ",9, 10}}   
};

WRT_Rect rc_configmenuico[15]={
        {308,71,370,48},   //�м�ѡ���ѡ�񷿼����
        {224,55,80,80},    //�ϼ�ͷ
        {683,55,80,80},    //�¼�ͷ

        {308,181,370,48},   //ѡ�񷿼�  ѡ���2
        {224,165,80,80},   //�ϼ�ͷ
        {683,165,80,80},   //�¼�ͷ

        {308,291,370,48},   //ѡ�񷿼�  ѡ���2
        {224,275,80,80},   //�ϼ�ͷ
        {683,275,80,80},   //�¼�ͷ

        {308,401,370,48},   //ѡ�񷿼�  ѡ���2
        {224,385,80,80},   //�ϼ�ͷ
        {683,385,80,80},   //�¼�ͷ    

        {780,531,111,69},              //ȷ��   
        {900,531,111,69},              //����
        {6,531,111,69}                 //����Ļ         
};
static void DrawSelectFrame(WRT_Rect rt1)
{
        WRT_Rect rt;
        rt = rt1;
        DrawRect(rt,2,rgb_white);
        DeflateRect(&rt,4);
        FillRect(rgb_white,rt);
}

static void   DrawROOMTYPE(){
        int xoffset = 0;
        //JpegDisp(rc_configmenuico[0].x,rc_configmenuico[0].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
        DrawSelectFrame(rc_configmenuico[0]);


        xoffset = (rc_configmenuico[0].w - GetTextWidth(LT(g_ROOMTYPE[g_cur_select_room_type].name),0))/2;
        if(xoffset < 0)
                xoffset = 0;
        xoffset += rc_configmenuico[0].x;
        DrawText(LT(g_ROOMTYPE[g_cur_select_room_type].name),xoffset,rc_configmenuico[0].y+rc_configmenuico[0].h-15,rgb_black);         

        // JpegDisp(rc_configmenuico[3].x,rc_configmenuico[3].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
        DrawSelectFrame(rc_configmenuico[3]);

        xoffset = (rc_configmenuico[3].w - GetTextWidth(LT(g_ROOMNAME[g_cur_select_room_type][g_cur_select_room].name),0))/2;
        if(xoffset < 0)
                xoffset = 0;
        xoffset += rc_configmenuico[3].x;
        DrawText(LT(g_ROOMNAME[g_cur_select_room_type][g_cur_select_room].name),xoffset,rc_configmenuico[3].y+rc_configmenuico[3].h-15,rgb_black);           
}

static void DrawELECTYPE(){
        int xoffset = 0;
        // JpegDisp(rc_configmenuico[6].x,rc_configmenuico[6].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));   
        DrawSelectFrame(rc_configmenuico[6]);


        xoffset = (rc_configmenuico[6].w - GetTextWidth(LT(g_ELECTYPE[g_cur_select_elec_type].name),0))/2;
        if(xoffset < 0)
                xoffset = 0;
        xoffset += rc_configmenuico[6].x;
        DrawText(LT(g_ELECTYPE[g_cur_select_elec_type].name),xoffset,rc_configmenuico[6].y+rc_configmenuico[6].h-15,rgb_black);         

        // JpegDisp(rc_configmenuico[9].x,rc_configmenuico[9].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
        DrawSelectFrame(rc_configmenuico[9]);

        xoffset = (rc_configmenuico[9].w - GetTextWidth(LT(g_ELECTNAME[g_cur_select_elec_type][g_cur_select_elec].name),0))/2;
        if(xoffset < 0)
                xoffset = 0;
        xoffset += rc_configmenuico[9].x;
        DrawText(LT(g_ELECTNAME[g_cur_select_elec_type][g_cur_select_elec].name),xoffset,rc_configmenuico[9].y+rc_configmenuico[9].h-15,rgb_black);           
}

static void DrawROOMNAME(){
        int xoffset = 0;
        //JpegDisp(rc_configmenuico[3].x,rc_configmenuico[3].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
        DrawSelectFrame(rc_configmenuico[3]);

        xoffset = (rc_configmenuico[3].w - GetTextWidth(LT(g_ROOMNAME[g_cur_select_room_type][g_cur_select_room].name),0))/2;
        if(xoffset < 0)
                xoffset = 0;
        xoffset += rc_configmenuico[3].x;
        DrawText(LT(g_ROOMNAME[g_cur_select_room_type][g_cur_select_room].name),xoffset,rc_configmenuico[3].y+rc_configmenuico[3].h-15,rgb_black);                 
}
static void DrawELECTNAME(){
        int xoffset = 0;
        //JpegDisp(rc_configmenuico[9].x,rc_configmenuico[9].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
        DrawSelectFrame(rc_configmenuico[9]);

        xoffset = (rc_configmenuico[9].w - GetTextWidth(LT(g_ELECTNAME[g_cur_select_elec_type][g_cur_select_elec].name),0))/2;
        if(xoffset < 0)
                xoffset = 0;
        xoffset += rc_configmenuico[9].x;
        DrawText(LT(g_ELECTNAME[g_cur_select_elec_type][g_cur_select_elec].name),xoffset,rc_configmenuico[9].y+rc_configmenuico[9].h-15,rgb_black);           

}

void CreateConfigMain(int type){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = ConfigMainEventHandler;
        pWindow->NotifyReDraw = DrawConfigMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        g_create_set_type = type;
        
        g_cur_select_room_type = 0;
        g_cur_select_room = 0;
        g_cur_select_elec_type =0;
        g_cur_select_elec = 0;
                
        WindowAddToWindowList(pWindow);    
        pWindow->NotifyReDraw2(pWindow);

       
}
void DrawConfigMain(){
       
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        DrawText(LT("��ѡ�񷿼����"),rc_configmenuico[1].x-200,rc_configmenuico[1].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[0].x,rc_configmenuico[0].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        DrawSelectFrame(rc_configmenuico[0]);

        JpegDisp(rc_configmenuico[1].x,rc_configmenuico[1].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[2].x,rc_configmenuico[2].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));

        DrawText(LT("��ѡ�񷿼�"),rc_configmenuico[4].x-200,rc_configmenuico[4].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[3].x,rc_configmenuico[3].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        DrawSelectFrame(rc_configmenuico[3]);

        JpegDisp(rc_configmenuico[4].x,rc_configmenuico[4].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[5].x,rc_configmenuico[5].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));

        DrawText(LT("��ѡ��������"),rc_configmenuico[7].x-200,rc_configmenuico[7].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[6].x,rc_configmenuico[6].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        DrawSelectFrame(rc_configmenuico[6]);

        JpegDisp(rc_configmenuico[7].x,rc_configmenuico[7].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[8].x,rc_configmenuico[8].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));  

        DrawText(LT("��ѡ�����"),rc_configmenuico[10].x-200,rc_configmenuico[10].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[9].x,rc_configmenuico[9].y,bt_xuanzekuang,sizeof(bt_xuanzekuang)); 
        DrawSelectFrame(rc_configmenuico[9]);

        JpegDisp(rc_configmenuico[10].x,rc_configmenuico[10].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[11].x,rc_configmenuico[11].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));        

        JpegDisp(rc_configmenuico[12].x,rc_configmenuico[12].y,menu_ok1,sizeof(menu_ok1));    
        JpegDisp(rc_configmenuico[13].x,rc_configmenuico[13].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_configmenuico[14].x,rc_configmenuico[14].y,bt_guanping1,sizeof(bt_guanping1));

        DrawROOMTYPE();   
        DrawELECTYPE(); 
}
int ConfigMainEventHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        for(index =0 ;index<15;index++){
                if(IsInside(rc_configmenuico[index],xy)){
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
                        pos_x = rc_configmenuico[index].x;
                        pos_y = rc_configmenuico[index].y;              
                        switch(index){
                case 1:  //��������
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                g_cur_select_room_type--;
                                if(g_cur_select_room_type < 0)
                                        g_cur_select_room_type=(MAX_ROOM_SORT-1);
                                DrawROOMTYPE();
                        }                                	
                        break;   
                case 2:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));    
                                g_cur_select_room_type++;
                                if(g_cur_select_room_type > (MAX_ROOM_SORT-1))
                                        g_cur_select_room_type=0;
                                DrawROOMTYPE();                                                       
                        }                  
                        break;
                        
                case 4://����ѡ��
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                g_cur_select_room--;
                                if(g_cur_select_room < 0)
                                        g_cur_select_room=(MAX_ROOM_NAME-1);
                                DrawROOMNAME();
                        }    
                        break;             
                case 5:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));    
                                g_cur_select_room++;
                                if(g_cur_select_room > (MAX_ROOM_NAME-1))
                                        g_cur_select_room=0;
                                DrawROOMNAME();                                                     
                        }                 
                        break;
                        
                case 7://�������ѡ��
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                g_cur_select_elec_type--;
                                if(g_cur_select_elec_type < 0)
                                        g_cur_select_elec_type=(MAX_ELEC_SORT-1);
                                DrawELECTYPE();
                        }
                        break;                 
                case 8:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));    
                                g_cur_select_elec_type++;
                                if(g_cur_select_elec_type > (MAX_ELEC_SORT-1))
                                        g_cur_select_elec_type=0;
                                DrawELECTYPE();                                                       
                        }                 
                        break;
                        
                case 10: //����ѡ��   
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_shangjiantou2,sizeof(bt_shangjiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
                                g_cur_select_elec--;
                                if(g_cur_select_elec < 0)
                                        g_cur_select_elec=(MAX_ELEC_NAME-1);
                                DrawELECTNAME();
                        }  
                        break;               
                case 11:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_xiajiantou2,sizeof(bt_xiajiantou2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_xiajiantou1,sizeof(bt_xiajiantou1));    
                                g_cur_select_elec++;
                                if(g_cur_select_elec > (MAX_ELEC_NAME-1))
                                        g_cur_select_elec=0;
                                DrawELECTNAME();                                                       
                        }                 
                        break;
                case 12://ȷ��
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                if(g_create_set_type == 0)
                                        CreateSetWin(0);
                                else if(g_create_set_type == 1){
                                        CreateSceneControlwin(0);
                                }else if(g_create_set_type == 2){
                                        CreateSceneControlwin(1);
                                }
                        }                       
                        break;                         	
                case 13://����
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }                   	
                        break;
                case 14://����Ļ
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
///////////////////////////////////////////////////////
///////����RF����
//�豸���Ͷ���


/////////////////////////////////////////////////////////////////////
//                                                                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////
//extern MYSYSINFO  g_mysysinfo;
static char jiaju_path[]="/home/wrt/jiaju.txt";
//static unsigned long g_findjiaju_tid;
typedef struct JIAJUDEFAULT{
        unsigned char roomsort;
        unsigned char roomname;
        unsigned char elecsort;
        unsigned char elecname;
        unsigned char RFsort;
        unsigned char RFNo;
        unsigned char SubSw;
        unsigned char RIUsort;
        unsigned char RIUNo;
        unsigned char RIU485Addr;
}JiajuDefault;

static JiajuDefault g_JiajuDefault[16]={
        {0x0,0x0,0x2,0x0,0x7,0x21,0x01,0x03,0x81,0x81}, //���ң������ң����ӣ�����һ��
        {0x0,0x0,0x4,0x0,0x7,0x21,0x02,0x03,0x81,0x81}, //���ң������ң�DVD��DVDһ��
        {0x0,0x0,0x3,0x0,0x7,0x21,0x03,0x03,0x81,0x81}, //���ң������ң��յ����յ�һ��	
        {0x0,0x0,0x0,0x0,0x5,0x21,0x01,0x03,0x81,0x81}, //���ң������ң��ƹ⣬�ƹ�һ��		

        {0x1,0x0,0x2,0x0,0x7,0x11,0x01,0x03,0x81,0x81}, //�����������������ӣ�����һ��
        {0x1,0x0,0x4,0x0,0x7,0x11,0x02,0x03,0x81,0x81}, //��������������DVD��DVDһ��
        {0x1,0x0,0x3,0x0,0x7,0x11,0x03,0x03,0x81,0x81}, //���������������յ����յ�һ��	
        {0x1,0x0,0x0,0x0,0x5,0x11,0x01,0x03,0x81,0x81}, //���������������ƹ⣬�ƹ�һ��	

        {0x2,0x0,0x2,0x0,0x7,0x31,0x01,0x03,0x81,0x81}, //�����������������ӣ�����һ��
        {0x2,0x0,0x4,0x0,0x7,0x31,0x02,0x03,0x81,0x81}, //��������������DVD��DVDһ��
        {0x2,0x0,0x3,0x0,0x7,0x31,0x03,0x03,0x81,0x81}, //���������������յ����յ�һ��	
        {0x2,0x0,0x0,0x0,0x5,0x31,0x01,0x03,0x81,0x81}, //���������������ƹ⣬�ƹ�һ��		

        {0x3,0x0,0x2,0x0,0x7,0x41,0x01,0x03,0x81,0x81}, //�鷿�����鷿�����ӣ�����һ��
        {0x3,0x0,0x4,0x0,0x7,0x41,0x02,0x03,0x81,0x81}, //�鷿�����鷿��DVD��DVDһ��
        {0x3,0x0,0x3,0x0,0x7,0x41,0x03,0x03,0x81,0x81}, //�鷿�����鷿���յ����յ�һ��	
        {0x3,0x0,0x0,0x0,0x5,0x41,0x01,0x03,0x81,0x81}, //�鷿�����鷿���ƹ⣬�ƹ�һ��				

};

void SetJiajuDefault(){
        int i =0;
        int index = -1;
        int j = 0;
        memset(g_JIAJU_TABLE,0,sizeof(g_JIAJU_TABLE));
        /*
        for(i = 0; i < MAX_JIAJU_TABLE_COUNT;i++){
        if(g_JIAJU_TABLE[i].valid == 1)
        return;
        }
        */
        for(i =0; i< 16;i++){
                if((index =get_idle_jiaju_table()) != -1){
                        memcpy(g_JIAJU_TABLE[index].SysID,pSystemInfo->mySysInfo.sysID,4);
                        g_JIAJU_TABLE[index].valid = 1;
                        g_JIAJU_TABLE[index].JiajuID.roomsort = g_JiajuDefault[i].roomsort;
                        g_JIAJU_TABLE[index].JiajuID.roomname = g_JiajuDefault[i].roomname;
                        g_JIAJU_TABLE[index].JiajuID.elecsort = g_JiajuDefault[i].elecsort;
                        g_JIAJU_TABLE[index].JiajuID.elecname = g_JiajuDefault[i].elecname;
                        g_JIAJU_TABLE[index].RfID.RFsort =  g_JiajuDefault[i].RFsort;
                        g_JIAJU_TABLE[index].RfID.RFNo =  g_JiajuDefault[i].RFNo;
                        g_JIAJU_TABLE[index].RfID.SubSw =  g_JiajuDefault[i].SubSw;
                        g_JIAJU_TABLE[index].RiuID.RIUsort = g_JiajuDefault[i].RIUsort;
                        g_JIAJU_TABLE[index].RiuID.RIUNo  =  g_JiajuDefault[i].RIUNo;
                        g_JIAJU_TABLE[index].RiuID.RIU485Addr  =  g_JiajuDefault[i].RIU485Addr;
                        if(g_JiajuDefault[i].RFsort == 0x06){
                                for(j=0; j<MAX_IR_MODE; j++){
                                        g_JIAJU_TABLE[index].IrMode[j].IrSort =  g_JiajuDefault[i].SubSw;
                                        g_JIAJU_TABLE[index].IrMode[j].IrIndex = j;
                                }              	  
                        }

                }		
        }
        save_jiaju_table_to_flash();	
}

int clear_jiaju_table(int param)
{
        CreateDialogWin2(LT("�����������..."),MB_NONE,NULL,NULL);
        memset(g_JIAJU_TABLE,0,sizeof(g_JIAJU_TABLE));
        for(int i=0; i< MAX_JIAJU_TABLE_COUNT;i++)
                memcpy(g_JIAJU_TABLE[i].SysID,pSystemInfo->mySysInfo.sysID,4);
        save_jiaju_table_to_flash();
        CloseWindow();
        CreateDialogWin2(LT("��ճɹ�!"),MB_NONE,NULL,NULL);
        SDL_Delay(1000);
        CloseWindow();
        return 0;
}

void init_jiaju_table(){
        memset(g_JIAJU_TABLE,0,sizeof(g_JIAJU_TABLE));
        for(int i=0; i< MAX_JIAJU_TABLE_COUNT;i++)
                memcpy(g_JIAJU_TABLE[i].SysID,pSystemInfo->mySysInfo.sysID,4);
        //��flash�л�þ���ļҾ����ñ�
#ifdef TEST_FFS    
        if(pSystemInfo->mySysInfo.isusedffs == 1)	{ 
                FILE* file= NULL;
                int size1 = sizeof(g_JIAJU_TABLE);
                file = fopen(jiaju_path,"rb");
                if(file != NULL){
                        long size = fread((void*)g_JIAJU_TABLE,1,size1,file);
                        if(size == size1){
                                fclose(file);  
                                file = NULL;         
                                return;
                        }
                        fclose(file);
                        file = NULL; 
                }  
                
                WRT_DEBUG("�Ķ�%s error  = %d \n",jiaju_path,errno);   
        }
#endif   

       //
       //t_create("ts10", 124, 0x1000, 0x1000, 0, &test_tid);
       //t_start(test_tid, 0, (void (*)())test_task, NULL);


}
int  get_idle_jiaju_table(){
        for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++)
                if(g_JIAJU_TABLE[i].valid == 0)
                        return i;
        return -1;
}
int  get_jiaju(T_ELEC_ID* JiajuID){
        for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++)
                if(g_JIAJU_TABLE[i].valid == 1){
                        if(g_JIAJU_TABLE[i].JiajuID.roomsort == JiajuID->roomsort &&
                                g_JIAJU_TABLE[i].JiajuID.roomname == JiajuID->roomname &&
                                g_JIAJU_TABLE[i].JiajuID.elecsort == JiajuID->elecsort &&
                                g_JIAJU_TABLE[i].JiajuID.elecname == JiajuID->elecname ){
                                        return i;
                        }

                }

                return -1;       
}
/*2009-7-29 14:16:56*/
unsigned char* Get_jiaju_table_file(int *len){
        unsigned char* tmpbuf = NULL;
        *len = sizeof(g_JIAJU_TABLE);
        tmpbuf = (unsigned char*)ZENMALLOC(*len);
        if(tmpbuf == NULL){
                *len =0;
                return NULL;
        }
        memcpy(tmpbuf,(void*)g_JIAJU_TABLE,*len);
        return tmpbuf;
}

//2009-7-29 14:31:17
int  Set_jiaju_table_file(unsigned char* buf,int buflen){
        WRT_DEBUG("����Ҿ������ļ�buflen = 0x%x,%d ",buflen,sizeof(g_JIAJU_TABLE));
        if(buflen  == sizeof(g_JIAJU_TABLE)){
                memcpy(g_JIAJU_TABLE,buf,buflen);
                return save_jiaju_table_to_flash();

        }
        return 0;
}



bool save_jiaju_table_to_flash(){
#ifdef TEST_FFS   
        if(pSystemInfo->mySysInfo.isusedffs == 1)	{ 
                FILE* file=NULL;
                int size1 = sizeof(g_JIAJU_TABLE);
                file = fopen(jiaju_path,"wb");
                if(file != NULL){
                        long write_size = fwrite((void*)g_JIAJU_TABLE,1,size1,file);
                        if(write_size == size1){
                                fclose(file);           
                                return true;
                        }
                        WRT_DEBUG("errno = %d  write_size=%d ",errno,write_size);
                        fclose(file);
                        unlink(jiaju_path);
                }   
                WRT_DEBUG("errno = %d ",errno);
        }
#endif  
        return false;   
}

int isganyinqi()
{
	for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x06){
                		if(g_JIAJU_TABLE[i].JiajuID.elecname == 0x0 || g_JIAJU_TABLE[i].JiajuID.elecname  == 0x01 || g_JIAJU_TABLE[i].JiajuID.elecname == 0x06){ //ֻ���¶Ⱥ�ʪ��
                			return 1;
                		}
                	}
                }
        }
        return 0;
         
                			
                			
}

int isdelayjack()
{
	for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x05){ //����ǲ���
                		WRT_DEBUG("���� 0x%x",g_JIAJU_TABLE[i].RfID.RFsort);
                		if(g_JIAJU_TABLE[i].RfID.RFsort == 0xb){ //�����Cbus	
                			return 1;
                		}
                	}
                }
        }
        return 0;
}



int isguanlianfangqu(int zone)
{
	WRT_MESSAGE("isguanlianfangqu 0x%x",zone);
	for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x05){ //����ǲ���
                		WRT_DEBUG("���� 0x%x",g_JIAJU_TABLE[i].RfID.RFsort);
                		if(g_JIAJU_TABLE[i].RfID.RFsort == 0xb){ //�����Cbus
                			WRT_DEBUG("cbus 0x%x",g_JIAJU_TABLE[i].RiuID.RIU485Addr);
                			if(g_JIAJU_TABLE[i].RiuID.RIU485Addr != 0x0){                				
                				if(g_JIAJU_TABLE[i].RiuID.RIU485Addr < 0x5 || g_JIAJU_TABLE[i].RiuID.RIU485Addr > 0x8){
                					WRT_WARNING("ֻ�ܹ���5-8����zone = 0x%x",g_JIAJU_TABLE[i].RiuID.RIU485Addr);
                				        continue;
                			        }                			        
                				if(g_JIAJU_TABLE[i].RiuID.RIU485Addr == zone){
                					WRT_DEBUG("��������g_JIAJU_TABLE[i].RiuID.RIU485Addr =0x%x,zone = 0x%x",g_JIAJU_TABLE[i].RiuID.RIU485Addr,zone);
                					if(Jiaju_GetStatus(&g_JIAJU_TABLE[i]) == 0x1){
                						WRT_DEBUG("���������Ĳ����Ǵ򿪵�");	
                					        return 1;
                					}else{
                						WRT_DEBUG("���������Ĳ����ǹرյĻ���״̬δ֪");	
                						return 0;
                					}
                				}
                					  
                			}
                		}
                	}
                }
        }
        return 0;	
}

void FindAndViewJiajuStatus()
{

	unsigned long msg[4];
	for(int i=0;i<MAX_JIAJU_TABLE_COUNT;i++){
                if(g_JIAJU_TABLE[i].valid == 1){
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x06){
                		if(g_JIAJU_TABLE[i].JiajuID.elecname == 0x0 || g_JIAJU_TABLE[i].JiajuID.elecname  == 0x01 || g_JIAJU_TABLE[i].JiajuID.elecname == 0x06){ //ֻ���¶Ⱥ�ʪ��
                			
                			msg[0] = g_JIAJU_TABLE[i].RfID.RFNo;
                			msg[1] = g_JIAJU_TABLE[i].RfID.SubSw;
                			msg[2]= g_JIAJU_TABLE[i].JiajuID.elecname;
                			msg[3] =0;
                			
                			send_pool_msg(msg);
                			
                			
                		}
                		
                	}
                }
        }
}


