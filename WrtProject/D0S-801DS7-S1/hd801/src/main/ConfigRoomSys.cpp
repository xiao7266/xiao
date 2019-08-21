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

int     g_cur_select_room_type = 0;   //当前选择的房间类别
int     g_cur_select_room = 0;        //当前选择的房间
int     g_cur_select_elec_type = 0;   //但前选择的电器类别
int     g_cur_select_elec = 0 ;       //当前选择的电器

int     g_create_set_type = 0;

MENU_ITEM    g_ROOMTYPE[MAX_ROOM_SORT]={{"卧室",0,1},{"客厅",1,2},{"餐厅",2,3},{"书房",3,4},{"厨房",4,5},{"卫生间",5,6},{"阳台",6,7},{"自定义",7,8}};
MENU_ITEM    g_ROOMNAME[MAX_ROOM_SORT][MAX_ROOM_NAME]={
        {{"主卧室",0,1},{"卧室一",1,2},{"卧室二",2,3},{"卧室三",3,4},{"卧室四",4,5},{"卧室五",5,6},{"卧室六",6,7},{"卧室七",7,8},{"卧室八",8,9},{"卧室九",9,10}},
        {{"客厅",0,1},{"客厅一",1,2},{"客厅二",2,3},{"客厅三",3,4},{"客厅四",4,5},{"客厅五",5,6},{"客厅六",6,7},{"客厅七",7,8},{"客厅八",8,9},{"客厅九",9,10}},
        {{"餐厅",0,1},{"餐厅一",1,2},{"餐厅二",2,3},{"餐厅三",3,4},{"餐厅四",4,5},{"餐厅五",5,6},{"餐厅六",6,7},{"餐厅七",7,8},{"餐厅八",8,9},{"餐厅九",9,10}},
        {{"书房",0,1},{"书房一",1,2},{"书房二",2,3},{"书房三",3,4},{"书房四",4,5},{"书房五",5,6},{"书房六",6,7},{"书房七",7,8},{"书房八",8,9},{"书房九",9,10}},
        {{"厨房",0,1},{"厨房一",1,2},{"厨房二",2,3},{"厨房三",3,4},{"厨房四",4,5},{"厨房五",5,6},{"厨房六",6,7},{"厨房七",7,8},{"厨房八",8,9},{"厨房九",9,10}},
        {{"卫生间",0,1},{"卫生间一",1,2},{"卫生间二",2,3},{"卫生间三",3,4},{"卫生间四",4,5},{"卫生间五",5,6},{"卫生间六",6,7},{"卫生间七",7,8},{"卫生间八",8,9},{"卫生间九",9,10}},
        {{"阳台",0,1},{"阳台一",1,2},{"阳台二",2,3},{"阳台三",3,4},{"阳台四",4,5},{"阳台五",5,6},{"阳台六",6,7},{"阳台七",7,8},{"阳台八",8,9},{"阳台九",9,10}},
        {{"自定义一",0,1},{"自定义二",1,2},{"自定义三",2,3},{"自定义四",3,4},{"自定义五",4,5},{"自定义六",5,6},{"自定义七",6,7},{"自定义八",7,8},{"自定义九",8,9},{"自定义十",9,10}}
};

MENU_ITEM    g_ELECTYPE[MAX_ELEC_SORT]={{"灯光",0 ,1},{"窗帘",1, 2},{"电视",2, 3},{"空调",3, 4},{"DVD",4, 5},{"插座",5,6},{"感应器",6,7},{"自定义",7, 8}};

MENU_ITEM    g_ELECTNAME[MAX_ELEC_SORT][MAX_ELEC_NAME]={
        {{"灯光一",0, 1},{"灯光二",1, 2},{"灯光三",2, 3},{"灯光四",3, 4},{"灯光五",4, 5},{"灯光六",5,6},{"灯光七",6, 7},{"灯光八",7, 8},{"灯光九",8, 9},{"灯光十",9, 10}},
        {{"窗帘一",0, 1},{"窗帘二",1, 2},{"窗帘三",2, 3},{"窗帘四",3, 4},{"窗帘五",4, 5},{"窗帘六",5,6},{"窗帘七",6, 7},{"窗帘八",7, 8},{"窗帘九",8, 9},{"窗帘十",9, 10}},
        {{"电视一",0, 1},{"电视二",1, 2},{"电视三",2, 3},{"电视四",3, 4},{"电视五",4, 5},{"电视六",5,6},{"电视七",6, 7},{"电视八",7, 8},{"电视九",8, 9},{"电视十",9, 10}},
        {{"空调一",0, 1},{"空调二",1, 2},{"空调三",2, 3},{"空调四",3, 4},{"空调五",4, 5},{"空调六",5,6},{"空调七",6, 7},{"空调八",7, 8},{"空调九",8, 9},{"空调十",9, 10}},
        {{"DVD一",0, 1}, {"DVD二",1, 2}, {"DVD三",2, 3},{"DVD四",3, 4},{"DVD五",4, 5},{"DVD六",5,6},{"DVD七",6, 7},{"DVD八",7, 8},{"DVD九",8, 9},{"DVD十",9, 10}},
        {{"插座一",0, 1},{"插座二",1, 2},{"插座三",2, 3},{"插座四",3, 4},{"插座五",4, 5},{"插座六",5,6},{"插座七",6, 7},{"插座八",7, 8},{"插座九",8, 9},{"插座十",9, 10}},
        {{"温度感应器",0, 1},{"湿度感应器",1, 2},{"移动感应器",2, 3},{"光照感应器",3, 4},{"烟雾感应器",4, 5},{"一氧化碳感应器",5,6},{"二氧化碳感应器",6, 7},{"风力感应器",7, 8},{"雨量感应器",8, 9},{"其他感应器",9, 10}},
        {{"自定义一",0, 1},{"自定义二",1, 2},{"自定义三",2, 3},{"自定义四",3, 4},{"自定义五",4, 5},{"自定义六",5,6},{"自定义七",6, 7},{"自定义八",7, 8},{"自定义九",8, 9},{"自定义十",9, 10}}   
};

WRT_Rect rc_configmenuico[15]={
        {308,71,370,48},   //中间选择框选择房间类别
        {224,55,80,80},    //上箭头
        {683,55,80,80},    //下箭头

        {308,181,370,48},   //选择房间  选择框2
        {224,165,80,80},   //上箭头
        {683,165,80,80},   //下箭头

        {308,291,370,48},   //选择房间  选择框2
        {224,275,80,80},   //上箭头
        {683,275,80,80},   //下箭头

        {308,401,370,48},   //选择房间  选择框2
        {224,385,80,80},   //上箭头
        {683,385,80,80},   //下箭头    

        {780,531,111,69},              //确认   
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕         
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
        DrawText(LT("请选择房间类别"),rc_configmenuico[1].x-200,rc_configmenuico[1].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[0].x,rc_configmenuico[0].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        DrawSelectFrame(rc_configmenuico[0]);

        JpegDisp(rc_configmenuico[1].x,rc_configmenuico[1].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[2].x,rc_configmenuico[2].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));

        DrawText(LT("请选择房间"),rc_configmenuico[4].x-200,rc_configmenuico[4].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[3].x,rc_configmenuico[3].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        DrawSelectFrame(rc_configmenuico[3]);

        JpegDisp(rc_configmenuico[4].x,rc_configmenuico[4].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[5].x,rc_configmenuico[5].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));

        DrawText(LT("请选择电器类别"),rc_configmenuico[7].x-200,rc_configmenuico[7].y+50,rgb_white);
        //JpegDisp(rc_configmenuico[6].x,rc_configmenuico[6].y,bt_xuanzekuang,sizeof(bt_xuanzekuang));  
        DrawSelectFrame(rc_configmenuico[6]);

        JpegDisp(rc_configmenuico[7].x,rc_configmenuico[7].y,bt_shangjiantou1,sizeof(bt_shangjiantou1));
        JpegDisp(rc_configmenuico[8].x,rc_configmenuico[8].y,bt_xiajiantou1,sizeof(bt_xiajiantou1));  

        DrawText(LT("请选择电器"),rc_configmenuico[10].x-200,rc_configmenuico[10].y+50,rgb_white);
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
                case 1:  //房间类型
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
                        
                case 4://房间选择
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
                        
                case 7://电器类别选择
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
                        
                case 10: //电器选择   
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
                case 12://确定
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
                case 13://返回
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                CloseTopWindow();
                        }                   	
                        break;
                case 14://关屏幕
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
///////////////////////////////////////////////////////
///////增加RF窗口
//设备类型定义


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
        {0x0,0x0,0x2,0x0,0x7,0x21,0x01,0x03,0x81,0x81}, //卧室，主卧室，电视，电视一，
        {0x0,0x0,0x4,0x0,0x7,0x21,0x02,0x03,0x81,0x81}, //卧室，主卧室，DVD，DVD一，
        {0x0,0x0,0x3,0x0,0x7,0x21,0x03,0x03,0x81,0x81}, //卧室，主卧室，空调，空调一，	
        {0x0,0x0,0x0,0x0,0x5,0x21,0x01,0x03,0x81,0x81}, //卧室，主卧室，灯光，灯光一，		

        {0x1,0x0,0x2,0x0,0x7,0x11,0x01,0x03,0x81,0x81}, //客厅，主客厅，电视，电视一，
        {0x1,0x0,0x4,0x0,0x7,0x11,0x02,0x03,0x81,0x81}, //客厅，主客厅，DVD，DVD一，
        {0x1,0x0,0x3,0x0,0x7,0x11,0x03,0x03,0x81,0x81}, //客厅，主客厅，空调，空调一，	
        {0x1,0x0,0x0,0x0,0x5,0x11,0x01,0x03,0x81,0x81}, //客厅，主客厅，灯光，灯光一，	

        {0x2,0x0,0x2,0x0,0x7,0x31,0x01,0x03,0x81,0x81}, //餐厅，主餐厅，电视，电视一，
        {0x2,0x0,0x4,0x0,0x7,0x31,0x02,0x03,0x81,0x81}, //餐厅，主餐厅，DVD，DVD一，
        {0x2,0x0,0x3,0x0,0x7,0x31,0x03,0x03,0x81,0x81}, //餐厅，主餐厅，空调，空调一，	
        {0x2,0x0,0x0,0x0,0x5,0x31,0x01,0x03,0x81,0x81}, //餐厅，主餐厅，灯光，灯光一，		

        {0x3,0x0,0x2,0x0,0x7,0x41,0x01,0x03,0x81,0x81}, //书房，主书房，电视，电视一，
        {0x3,0x0,0x4,0x0,0x7,0x41,0x02,0x03,0x81,0x81}, //书房，主书房，DVD，DVD一，
        {0x3,0x0,0x3,0x0,0x7,0x41,0x03,0x03,0x81,0x81}, //书房，主书房，空调，空调一，	
        {0x3,0x0,0x0,0x0,0x5,0x41,0x01,0x03,0x81,0x81}, //书房，主书房，灯光，灯光一，				

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
        CreateDialogWin2(LT("正在清空配置..."),MB_NONE,NULL,NULL);
        memset(g_JIAJU_TABLE,0,sizeof(g_JIAJU_TABLE));
        for(int i=0; i< MAX_JIAJU_TABLE_COUNT;i++)
                memcpy(g_JIAJU_TABLE[i].SysID,pSystemInfo->mySysInfo.sysID,4);
        save_jiaju_table_to_flash();
        CloseWindow();
        CreateDialogWin2(LT("清空成功!"),MB_NONE,NULL,NULL);
        SDL_Delay(1000);
        CloseWindow();
        return 0;
}

void init_jiaju_table(){
        memset(g_JIAJU_TABLE,0,sizeof(g_JIAJU_TABLE));
        for(int i=0; i< MAX_JIAJU_TABLE_COUNT;i++)
                memcpy(g_JIAJU_TABLE[i].SysID,pSystemInfo->mySysInfo.sysID,4);
        //从flash中获得具体的家居配置表
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
                
                WRT_DEBUG("阅读%s error  = %d \n",jiaju_path,errno);   
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
        WRT_DEBUG("保存家居配置文件buflen = 0x%x,%d ",buflen,sizeof(g_JIAJU_TABLE));
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
                		if(g_JIAJU_TABLE[i].JiajuID.elecname == 0x0 || g_JIAJU_TABLE[i].JiajuID.elecname  == 0x01 || g_JIAJU_TABLE[i].JiajuID.elecname == 0x06){ //只查温度和湿度
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
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x05){ //如果是插座
                		WRT_DEBUG("插座 0x%x",g_JIAJU_TABLE[i].RfID.RFsort);
                		if(g_JIAJU_TABLE[i].RfID.RFsort == 0xb){ //如果是Cbus	
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
                	if(g_JIAJU_TABLE[i].JiajuID.elecsort == 0x05){ //如果是插座
                		WRT_DEBUG("插座 0x%x",g_JIAJU_TABLE[i].RfID.RFsort);
                		if(g_JIAJU_TABLE[i].RfID.RFsort == 0xb){ //如果是Cbus
                			WRT_DEBUG("cbus 0x%x",g_JIAJU_TABLE[i].RiuID.RIU485Addr);
                			if(g_JIAJU_TABLE[i].RiuID.RIU485Addr != 0x0){                				
                				if(g_JIAJU_TABLE[i].RiuID.RIU485Addr < 0x5 || g_JIAJU_TABLE[i].RiuID.RIU485Addr > 0x8){
                					WRT_WARNING("只能关联5-8防区zone = 0x%x",g_JIAJU_TABLE[i].RiuID.RIU485Addr);
                				        continue;
                			        }                			        
                				if(g_JIAJU_TABLE[i].RiuID.RIU485Addr == zone){
                					WRT_DEBUG("关联防区g_JIAJU_TABLE[i].RiuID.RIU485Addr =0x%x,zone = 0x%x",g_JIAJU_TABLE[i].RiuID.RIU485Addr,zone);
                					if(Jiaju_GetStatus(&g_JIAJU_TABLE[i]) == 0x1){
                						WRT_DEBUG("关联防区的插座是打开的");	
                					        return 1;
                					}else{
                						WRT_DEBUG("关联防区的插座是关闭的或者状态未知");	
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
                		if(g_JIAJU_TABLE[i].JiajuID.elecname == 0x0 || g_JIAJU_TABLE[i].JiajuID.elecname  == 0x01 || g_JIAJU_TABLE[i].JiajuID.elecname == 0x06){ //只查温度和湿度
                			
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


