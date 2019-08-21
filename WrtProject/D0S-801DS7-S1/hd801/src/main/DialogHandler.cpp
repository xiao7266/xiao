#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audiovideo.h"
#include "Handler.h"
#include "window.h"
#include "tmvideodisp.h"
#include "dialog_menu_res.h"
#include "public_menu_res.h"

//extern bool     g_rc_status[42]; //用来判断当弹起的位置和按下的位置不一样时的处理状态

static char  dialog_text[256];
static int   dialog_type =0;

static const  WRT_Rect rc_dialog[3]={
        {280,280,92,56},
        {360,280,92,56},
        {440,280,92,56}
};

static void No_Button(){
        switch(dialog_type){
        case SET_PWD_ERROR_CUE:
        case SET_ROOM_ERROR_TISHI:
                CloseAllWindows();
                return;
        case SET_SAFE_RESET:
                Reset_safe();
                break;
        case SET_RING:
                cancel_set_cur_ring();
                break;
        case SET_IS_SCREENAJUEST:
                no_screen_adjust();
                break;
        default:
                break;

        }
        CloseTopWindow();
}

static void Yes_Button(){
        switch(dialog_type){
     case CLEAR_MESSAGE:
             clear_msg();
             break;
     case DEL_MESSAGE:
             del_cur_msg();
             break;
     case CLEAR_PHOTO:
             clear_photo();
             break;
     case DEL_PHOTO:
             del_cur_photo();
             break;
     case CLEAR_LOG:
             clear_log();
             break;
     case DEL_LOG:
             del_cur_log();
             break;
     case CLEAR_SOUND_LEAVER:
             clear_voice();
             break;
     case DEL_SOUND_LEAVER:
             del_cur_voice();
             break;
     case DEL_RING:
             del_cur_download();
             break;
     case CLEAR_RING:
             clear_download();
             break;
     case SET_LEAVE_HOME:
             cur_set_leave_home(true);
             break;
     case SET_PWD_RESET:
             ConfirmResetPwd();
             break;
     case SET_RESET_TIME:
             ResetSysTime();
             break;
     case SET_UPDATE_SYS:
             Reset_sys();
             break;
     case SET_RING_DOWNLOAD:
             download_ring();
             break;
     case SET_NET_CONFIG:
             modify_net_set();
             break;
     case CLEAR_TELEPHONE:
             clear_telephone();
             break;
     case DEL_TELEPHONE:
             del_cur_telephone();
             break;
     case SET_ADD_TELEPHONE:
             add_telephone_1();
             break;
     case SET_SAFE_RESET:
             ok_safe();
             break;
     case SET_FORMAT:
            // format_my_ffs();
             break;
     case SET_RING:
             set_cur_ring();
             break;
     case SET_CLEAR_OLD_SCENE:
             clear_scene_mode();
             break;
     case SET_CANCEL_AVOID:
             CancelAvoid();
             break;
     case SET_CANCEL_AGENT:
             CancelAgent();
             break;
     case SET_ROOM_NUMBER:
             modify_room_number();
             break;
     case SET_TOUCH_SREEN:
             screen_adjust();
             break;
     case SET_ENABLE_SAFE:
             enable_safe();
             break;
     case SET_IS_SCREENAJUEST:
             yes_screen_adjust();
             break;
     default:
             break;
        }
}

void CreateDialogWin(char* text,int type){
        memset(dialog_text,0,256);
        strcpy(dialog_text,text);
        dialog_type = type;
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = DialogHandler;
        pWindow->NotifyReDraw = DrawDialogMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        pWindow->pos.x = 240;
        pWindow->pos.y = 120;
        pWindow->pos.w = 320;
        pWindow->pos.h = 240;        	
        WindowAddToWindowList(pWindow);      	   
        pWindow->NotifyReDraw2(pWindow);	        
}

void DrawDialogMain(){
        JpegDisp(240,120,menu_dialog_back,sizeof(menu_dialog_back));
        if(dialog_type != -1){
                if(dialog_type == 0 ||dialog_type == SET_UPDATE_SYS){
                        JpegDisp(360,280,menu_queren1,sizeof(menu_queren1));
                }else if(dialog_type == -2){
                        JpegDisp(360,280,menu_quxiao1,sizeof(menu_quxiao1));
                }else{
                        JpegDisp(280,280,menu_queren1,sizeof(menu_queren1));
                        JpegDisp(440,280,menu_quxiao1,sizeof(menu_quxiao1));
                }
        }

        if(strlen(dialog_text) != 0){
                DrawText(LT("用户确认"),250,140,rgb_black);
                //  DrawText(dialog_text,250,186,rgb_black);
                int offset = 40;
                char text[40];
                int  i;
                int textlen = strlen(dialog_text);
                unsigned char* pText = (unsigned char*)dialog_text;
                int   first_text_len = textlen;
                int   view_text_len = 0;
                if(first_text_len > 33){
                        while(1){
                                while(first_text_len > 0){
                                        if((first_text_len >=2 ) && pText[first_text_len-1] >= 0xa0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        if((first_text_len * 8)<260)
                                                break;
                                }
                                memcpy(text,pText,first_text_len);
                                text[first_text_len]='\0';
                                DrawText_16(text,250,i*offset+186,rgb_black);

                                i++;
                                view_text_len +=(first_text_len);
                                pText = (unsigned char*)dialog_text+view_text_len;
                                first_text_len = textlen - view_text_len;
                                if(first_text_len <=24){
                                        memcpy(text,pText,first_text_len);
                                        text[first_text_len]='\0';
                                        DrawText_16(text,250,i*offset+186,rgb_black);
                                        break; 
                                }
                        }

                }else{
                        DrawText_16(dialog_text,250,186,rgb_black);//处理 
                }      

        }	 
}
int  DialogHandler(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        xy.x = x;
        xy.y = y;
        if(dialog_type == -1)
                return 1;
        for(index = 0; index < 3;index++){
                if(IsInside(rc_dialog[index],xy)){
                        ret = 1;
                        if(status == 1)
                                g_rc_status[6] = true;
                        else{
                                if(g_rc_status[6] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[6] = false;
                        }
                        pos_x = rc_dialog[index].x;
                        pos_y = rc_dialog[index].y;
                        switch(index){
                case 0://确定
                        if(dialog_type == 0)
                                break;
                        if(dialog_type == -2)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_queren2,sizeof(menu_queren2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_queren1,sizeof(menu_queren1));

                                Yes_Button();
                                CloseTopWindow();

                        }
                        break; 
                case 1:
                        if(dialog_type !=SET_UPDATE_SYS ){
                                if(dialog_type > 0)
                                        break;
                        }

                        if(status == 1){
                                StartButtonVocie();
                                if(dialog_type == 0 ||dialog_type ==SET_UPDATE_SYS)
                                        JpegDisp(pos_x,pos_y,menu_queren2,sizeof(menu_queren2));
                                else
                                        JpegDisp(pos_x,pos_y,menu_quxiao2,sizeof(menu_quxiao2));
                        }else{
                                if(dialog_type == 0 || dialog_type ==SET_UPDATE_SYS){
                                        JpegDisp(pos_x,pos_y,menu_queren1,sizeof(menu_queren1));
                                        Yes_Button();
                                        CloseTopWindow();

                                }else{
                                        JpegDisp(pos_x,pos_y,menu_quxiao1,sizeof(menu_quxiao1));

                                        cancel_listenring();
                                        CloseTopWindow();
                                }
                        }     
                        break;       		
                case 2://取消
                        if(dialog_type == 0)
                                break;
                        if(dialog_type == -2)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_quxiao2,sizeof(menu_quxiao2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_quxiao1,sizeof(menu_quxiao1));
                                No_Button();
                                /*
                                if(dialog_type == SET_PWD_ERROR_CUE || dialog_type == SET_ROOM_ERROR_TISHI)
                                CloseAllWindows();
                                else{

                                if(dialog_type == SET_SAFE_RESET)
                                Reset_safe();
                                CloseTopWindow();

                                }
                                */
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