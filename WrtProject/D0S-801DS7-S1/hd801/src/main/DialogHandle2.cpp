#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "window.h"
#include "dialog_menu_res.h"
#include "public_menu_res.h"

//DIALOG_CALLBACK     gfunc_ok = NULL;
//DIALOG_CALLBACK     gfunc_cancel=NULL;
static DIALOG_CALLBACK gfunc_close = NULL;

extern bool     g_is_leave;
static char    dialog_text[256];
static int    dialog_type =0;

static const  WRT_Rect rc_dialog[3]={
        {392,340,92,56},
        {472,340,92,56},
        {550,340,92,56}
};

static void CloseDialog(){
	if(gfunc_close != NULL)
		gfunc_close(1);
        WRT_DEBUG("关闭dialog  %s ",dialog_text);
}

void SetDialogWin2CloseCallback(DIALOG_CALLBACK func_close)
{
	gfunc_close = func_close;
}


void CreateDialogWin2(char* text,int type ,DIALOG_CALLBACK  func_ok, DIALOG_CALLBACK func_cancel){
        //  Window* pWindow = (Window*)New_Window();
        //  if(pWindow == NULL)
        //  	   return ;
        DialogWin* pdlg = (DialogWin*)New_Dialog();
        if(pdlg == NULL)
                return;
        memset(pdlg->text,0,256);
        strcpy(pdlg->text,text);
        dialog_type = type;   	   
        pdlg->gfunc_ok = func_ok;
        pdlg->baseWin.EventHandler = DialogHandler2;
        pdlg->baseWin.NotifyReDraw = DrawDialogMain2;
        pdlg->baseWin.CloseWindow = CloseDialog;
        pdlg->baseWin.valid = 1;
        pdlg->baseWin.type = DIALOG;
        pdlg->dlgtype = type;
        pdlg->gfunc_cancel = func_cancel;
        pdlg->baseWin.pos.x = 352;
        pdlg->baseWin.pos.y = 180;
        pdlg->baseWin.pos.w = 320;
        pdlg->baseWin.pos.h = 240;
		
        WindowAddToWindowList(&pdlg->baseWin); 
        pdlg->baseWin.NotifyReDraw2(&pdlg->baseWin);  	
        WRT_DEBUG("create dialog 2 %s ",text);
}

static void DrawOKAndCancel(int x,int y,int w,int h,char* strtext,int status){
        if(strtext == NULL)
                return;
        int xoffset1 = 0;
        int yoffset1 = 0;
        xoffset1 = (w - GetTextWidth(strtext,0))/2;

        if(xoffset1 < 0){
                if(status == 1)
                        xoffset1 = 6;
                else
                        xoffset1 = 3;
        }
        if(status == 1)
                xoffset1 += x;
        else
                xoffset1 += x+10;
        if(status == 1)
                yoffset1 = y + h -  30;
        else
                yoffset1 = y + h -  20;
        DrawText(strtext,xoffset1, yoffset1,rgb_black);

}

static void DrawSingleOKAndCancel(int x,int y,int w,int h,char* strtext,int status){
        if(strtext == NULL)
                return;
        int xoffset1 = 0;
        int yoffset1 = 0;
        xoffset1 = (w - GetTextWidth(strtext,0))/2;

        if(xoffset1 < 0){
                if(status == 1)
                        xoffset1 = 6;
                else
                        xoffset1 = 3;
        }
        if(status == 1)
                xoffset1 += x;
        else
                xoffset1 += x+10;
        if(status == 1)
                yoffset1 = y + h -  30;
        else
                yoffset1 = y + h -  20;
        DrawText(strtext,xoffset1, yoffset1,rgb_black);

}

void DrawDialogMain2(){	
        JpegDisp(352,180,menu_dialog_back,sizeof(menu_dialog_back));
        DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
        memset(dialog_text,0,256);
        if(p->baseWin.type ==DIALOG){	
                strcpy(dialog_text,p->text);
                dialog_type = p->dlgtype;
        }

        if((dialog_type & MB_OK) && (dialog_type & MB_CANCEL)){
                JpegDisp(rc_dialog[0].x,rc_dialog[0].y,menuback1,sizeof(menuback1));//menu_queren1
                JpegDisp(rc_dialog[2].x,rc_dialog[2].y,menuback1,sizeof(menuback1)); //menu_quxiao1
                DrawOKAndCancel(rc_dialog[0].x,rc_dialog[0].y,rc_dialog[0].w,rc_dialog[0].h,LT("确认"),0);
                DrawOKAndCancel(rc_dialog[2].x,rc_dialog[2].y,rc_dialog[2].w,rc_dialog[2].h,LT("取消"),0);


        }else if((dialog_type & MB_OK) && !(dialog_type &MB_CANCEL)){
                JpegDisp(rc_dialog[1].x,rc_dialog[1].y,menuback1,sizeof(menuback1));
                DrawOKAndCancel(rc_dialog[1].x,rc_dialog[1].y,rc_dialog[1].w,rc_dialog[1].h,LT("确认"),0);

        }else if((dialog_type & MB_CANCEL) && !(dialog_type & MB_OK)){
                JpegDisp(rc_dialog[1].x,rc_dialog[1].y,menuback1,sizeof(menuback1));
                DrawOKAndCancel(rc_dialog[1].x,rc_dialog[1].y,rc_dialog[1].w,rc_dialog[1].h,LT("取消"),0);
        }
        if(strlen(dialog_text) != 0){
                DrawText_16(LT("用户确认"),362,200,rgb_black);
                int offset = 30;
                char* text=NULL;
                int  i = 0;
                int textlen = strlen(dialog_text);
                unsigned char* pText = (unsigned char*)dialog_text;
                int   first_text_len = textlen;
                int   view_text_len = 0;
                int  str_len = 0;
                str_len = GetTextWidth((char*)pText,1);
                if(str_len < 300){
                        DrawText_16(dialog_text,362,240,rgb_black);//处理 
                        return;
                }
#if 1
                text = (char*)ZENMALLOC(textlen+1);
                memset(text,0,textlen+1);
                while(1){
                	
                        while(first_text_len > 0){
                                str_len = GetTextWidth((char*)pText,0);
                                if(str_len > 310){
                                        if((first_text_len >=2 ) && pText[first_text_len-1] >= 0xa0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(text,0,textlen+1);
                                        memcpy(text,pText,first_text_len);
                                        str_len = GetTextWidth((char*)text,1);
                                        if((str_len)<310) //540
                                                break;
                                }else{
                                        memset(text,0,textlen+1);
                                        memcpy(text,pText,first_text_len);
                                        break;
                                }
                        }
                        if(first_text_len == 0 || first_text_len < 0)
                                break;
                        DrawText_16(text,362,i*offset+240,rgb_black);
                        i++;
                        view_text_len +=(first_text_len);
                        pText = (unsigned char*)dialog_text+view_text_len;
                        first_text_len = textlen - view_text_len;
                }
                ZENFREE(text);
#endif

                /*
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
                */
        }		
}

int DialogHandler2(int x,int y,int status){
        int ret = 0;
        WRT_xy xy;
        int pos_x,pos_y;
        int index;
        int res = 0;
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
                        if((dialog_type & MB_OK) && (dialog_type & MB_CANCEL)){  //OK,CANCEL 0,2
                                if(index == 0){
                                        if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[0].w,rc_dialog[0].h,LT("确认"),1);
                                        }else{
                                        	
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[0].w,rc_dialog[0].h,LT("确认"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){
                                                        /*
                                                        if(strcmp("确定要校正屏幕吗？"	,p->text) == 0){
                                                        CloseWindow();
                                                        if(p->gfunc_ok)
                                                        p->gfunc_ok(1);
                                                        break;
                                                        }
                                                        */
                                                        if(p->gfunc_ok)
                                                               res =  p->gfunc_ok(1);

                                                } 
                                                if(res == 0)                       
                                                	CloseTopWindow();
                                                break;
                                        }
                                }
                                if(index == 2){
                                        if(status == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[2].w,rc_dialog[2].h,LT("取消"),1);

                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[2].w,rc_dialog[2].h,LT("取消"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){	
                                                        if(p->gfunc_cancel)
                                                               res =  p->gfunc_cancel(1);                       	
                                                }                        
						if(res == 0)
                                                	CloseTopWindow();
                                                break;
                                        }
                                }

                        }else if((dialog_type & MB_OK) && !(dialog_type &MB_CANCEL)){  //OK ,1
                                if(index == 1){
                                        if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[1].w,rc_dialog[1].h,LT("确认"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[1].w,rc_dialog[1].h,LT("确认"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){	
                                                        if(p->gfunc_ok)
                                                               res = p->gfunc_ok(1);                       	
                                                }                        
						if(res == 0)
                                                	CloseTopWindow();
                                                break;
                                        }
                                }

                        }else if((dialog_type & MB_CANCEL) && !(dialog_type & MB_OK)){ //CANCEL,1
                                if(index == 1){
                                        if(status == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[1].w,rc_dialog[1].h,LT("取消"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dialog[1].w,rc_dialog[1].h,LT("取消"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){	
                                                        if(p->gfunc_cancel)
                                                                res = p->gfunc_cancel(1);                       	
                                                } 
                                                if(res == 0)  
                                                	CloseTopWindow();
                                                break;
                                        }
                                }
                        }
                        break;
                }

        }
        return ret;
}


//-----------------------------------------------------------------------------------------------------
static const WRT_Rect   rc_dlg3[3]={
        {114,342,92,56},   //确定
        {274,342,92,56},    //取消
        {434,342,92,56}    //呼叫
};

void CreateDialogWin3(char* text,int type,DIALOG_CALLBACK  func_ok, DIALOG_CALLBACK func_cancel,DIALOG_CALLBACK func_call)
{
	DialogWin* pdlg = (DialogWin*)New_Dialog();
        if(pdlg == NULL)
                return;
        memset(pdlg->text,0,256);
        strcpy(pdlg->text,text);
        dialog_type = type;   	   
        pdlg->gfunc_ok = func_ok;
        pdlg->baseWin.EventHandler = DialogHandler3;
        pdlg->baseWin.NotifyReDraw = DrawDialogMain3;
        pdlg->baseWin.CloseWindow = NULL;
        pdlg->baseWin.valid = 1;
        pdlg->baseWin.type = DIALOG;
        pdlg->dlgtype = type;
        pdlg->gfunc_cancel = func_cancel;
        pdlg->gfunc_call = func_call;
        pdlg->baseWin.pos.x = 80;
        pdlg->baseWin.pos.y = 60;
        pdlg->baseWin.pos.w = 480;
        pdlg->baseWin.pos.h = 360;		
        WindowAddToWindowList(&pdlg->baseWin); 
        pdlg->baseWin.NotifyReDraw2(&pdlg->baseWin);  	
        WRT_DEBUG("create dialog 3 %s ",text);
}

void DrawDialogMain3()
{
	 JpegDispZoom(80,60,menu_dialog_back,sizeof(menu_dialog_back),1.5,1.5);
	DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
        memset(dialog_text,0,256);
        if(p->baseWin.type ==DIALOG){	
                strcpy(dialog_text,p->text);
                dialog_type = p->dlgtype;
        }

        if((dialog_type & MB_OK) && (dialog_type & MB_CANCEL) && !(dialog_type & MB_CALL)){
                JpegDisp(rc_dlg3[0].x,rc_dlg3[0].y,menuback1,sizeof(menuback1));//menu_queren1
                JpegDisp(rc_dlg3[2].x,rc_dlg3[2].y,menuback1,sizeof(menuback1)); //menu_quxiao1
                DrawOKAndCancel(rc_dlg3[0].x,rc_dlg3[0].y,rc_dlg3[0].w,rc_dlg3[0].h,LT("确认"),0);
                DrawOKAndCancel(rc_dlg3[2].x,rc_dlg3[2].y,rc_dlg3[2].w,rc_dlg3[2].h,LT("取消"),0);


        }else if((dialog_type & MB_OK) && !(dialog_type &MB_CANCEL) && !(dialog_type & MB_CALL)){
                JpegDisp(rc_dlg3[1].x,rc_dlg3[1].y,menuback1,sizeof(menuback1));
                DrawOKAndCancel(rc_dlg3[1].x,rc_dlg3[1].y,rc_dlg3[1].w,rc_dlg3[1].h,LT("确认"),0);

        }else if((dialog_type & MB_CANCEL) && (dialog_type & MB_OK) && (dialog_type & MB_CALL)){
        	JpegDisp(rc_dlg3[0].x,rc_dlg3[0].y,menuback1,sizeof(menuback1));
                DrawOKAndCancel(rc_dlg3[0].x,rc_dlg3[0].y,rc_dlg3[0].w,rc_dlg3[0].h,LT("确认"),0);
                
                JpegDisp(rc_dlg3[1].x,rc_dlg3[1].y,menuback1,sizeof(menuback1));
                DrawOKAndCancel(rc_dlg3[1].x,rc_dlg3[1].y,rc_dlg3[1].w,rc_dlg3[1].h,LT("取消"),0);
                
                JpegDisp(rc_dlg3[2].x,rc_dlg3[2].y,menuback1,sizeof(menuback1));
                DrawOKAndCancel(rc_dlg3[2].x,rc_dlg3[2].y,rc_dlg3[2].w,rc_dlg3[2].h,LT("呼叫"),0);
        }
        if(strlen(dialog_text) != 0){
                 DrawText_16(LT("用户确认"),90,85,rgb_black);
                 int offset = 30;
                 char* text=NULL;
                 int  i;
                 int textlen = strlen(dialog_text);
                 unsigned char* pText = (unsigned char*)dialog_text;
                 int   first_text_len = textlen;
                 int   view_text_len = 0;
                 int  str_len = 0;
                 str_len = GetTextWidth((char*)pText,1);
                 if(str_len < 440){
                         DrawText_16(dialog_text,110,150,rgb_black);//处理 
                         return;
                 }
                 
                 text = (char*)ZENMALLOC(textlen+1);
                 memset(text,0,textlen+1);
                 while(1){
                         while(first_text_len > 0){
                                 str_len = GetTextWidth((char*)pText,1);
                                 if(str_len > 440){
                                         if((first_text_len >=2 ) && pText[first_text_len-1] >= 0xa0)
                                                 first_text_len -=2;
                                         else
                                                 first_text_len --;
                                         memset(text,0,textlen+1);
                                         memcpy(text,pText,first_text_len);
                                         str_len = GetTextWidth((char*)text,1);
                                         if((str_len)<440) 
                                                 break;
                                 }else{
                                         memset(text,0,textlen+1);
                                         memcpy(text,pText,first_text_len);
                                         break;
                                 }
                         }
                         if(first_text_len == 0 || first_text_len < 0)
                                 break;
                         DrawText(text,110,i*offset+140,rgb_black);
                         i++;
                         view_text_len +=(first_text_len);
                         pText = (unsigned char*)dialog_text+view_text_len;
                         first_text_len = textlen - view_text_len;
                 }
                 ZENFREE(text);
        }
           
}

int  DialogHandler3(int x,int y,int status)
{
	int ret = 0;
        WRT_xy xy;
        int index;
        xy.x = x;
        xy.y = y;
        int ret2 = 0;
        int pos_x,pos_y;
        for(index =0 ;index<3;index++){
                if(IsInside(rc_dlg3[index],xy)){
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
                        pos_x = rc_dlg3[index].x;
                        pos_y = rc_dlg3[index].y;                        
                        switch(index)
                        {
                        case 0://确认
                               if((dialog_type & MB_OK)  && 
                               		((dialog_type & MB_CANCEL) || (dialog_type & MB_CALL))){  
                               	        if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("确认"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("确认"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){
                                                        if(p->gfunc_ok)
                                                                p->gfunc_ok(1);

                                                }                        
                                                CloseTopWindow(); 
                                        }                        
                        	}
                                break;
                        case 1:
                               if((dialog_type & MB_OK)  && 
                                   	!(dialog_type & MB_CANCEL) && 
                                   	!(dialog_type & MB_CALL)){  
                               	        if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("确认"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("确认"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){
                                                        if(p->gfunc_ok)
                                                                p->gfunc_ok(1);

                                                }                        
                                                CloseTopWindow(); 
                                        }
                                        break;                        
                        	}
                        	if(dialog_type & MB_CANCEL){
                              	        if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("取消"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("取消"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){
                                                        if(p->gfunc_cancel)
                                                                p->gfunc_cancel(1);

                                                }                        
                                                CloseTopWindow(); 
                                        }
                                        break;                          		
                        	}
                        	
                                break;
                        case 2: 
                        	if((dialog_type & MB_CANCEL) && !(dialog_type & MB_CALL)){
                              	        if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("取消"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("取消"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){
                                                        if(p->gfunc_cancel)
                                                                p->gfunc_cancel(1);

                                                }                        
                                                CloseTopWindow(); 
                                        }
                                        break;                         		
                        		
                        	} 
                        	if(dialog_type & MB_CALL){
                        	       if(status  == 1){
                                                StartButtonVocie();
                                                JpegDisp(pos_x,pos_y,menuback2,sizeof(menuback2));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("呼叫"),1);
                                        }else{
                                                JpegDisp(pos_x,pos_y,menuback1,sizeof(menuback1));
                                                DrawOKAndCancel(pos_x,pos_y,rc_dlg3[0].w,rc_dlg3[0].h,LT("呼叫"),0);
                                                DialogWin* p = (DialogWin*)WindowGetTopVisibleWindow();
                                                if(p->baseWin.type ==DIALOG){
                                                        if(p->gfunc_call)
                                                                p->gfunc_call(1);

                                                }                        
                                              
                                        }
                                        break; 
                        	}
                        	                             
                                break;
                         default:
                         	break;
                        }


                }
	}

        return ret;
}