#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"

#include "window.h"




#define MAX_PWD_BITS 9            //8位密码 ，最后一位‘\0'

//extern bool g_rc_status[42];
extern  char g_number[12];
#ifdef __cplusplus
extern "C" {
#endif	
        void mcu_reset_sys(unsigned char param); //smalldoor_handler.cpp
#ifdef __cplusplus
}
#endif

static int pws_type = 0;
static unsigned long* pNumberPointer=NULL;
static unsigned long* pNumberPointer1=NULL;
static unsigned long* pNumberSize=NULL;
static unsigned long* pNumberSize1=NULL;
static const WRT_Rect rc_pwdmenuico[4]={
        {75,210,292,44}, //密码框输入框所在区域
        {650,100,300,400},    //数字符号所在区域
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕				
};

static const WRT_Rect rc_setpwdmenuico[4]={
        {75,125,292,44}, //密码框输入框所在区域 // 下一个68
        {650,100,300,400},    //数字符号所在区域
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕				
};

static char g_inputpassword[MAX_PWD_BITS];       /*当前输入的密码*/
static char g_newpassword[MAX_PWD_BITS];         /*二次确认密码*/
static char g_password[MAX_PWD_BITS]="88888888";  /*密码*/
char        g_debugpassword[MAX_PWD_BITS]="20090101";   //调试密码
static char g_diplaypwd[MAX_PWD_BITS];
static int  g_inputpwdcount = 0;
static int  g_inputstatus = 1;    //更改密码时状态

int g_chefang_err_cnt = 0;
static SDL_TimerID m_chefang_err_id = 0;

static Uint32 g_chefang_error_process (Uint32 interval,void* pvoid)
{
	printf("g_chefang_error_process\n");
	g_chefang_err_cnt = 0;
	
	if(m_chefang_err_id != 0) {
		SDL_RemoveTimer(m_chefang_err_id);
	}
	
	m_chefang_err_id = 0;
}

static void Process_Chefang_Error()
{
	if(m_chefang_err_id != 0) {
		SDL_RemoveTimer(m_chefang_err_id);
	}
	printf("启动输入错误计时\n");
	m_chefang_err_id = 0;
	
	m_chefang_err_id = SDL_AddTimer(pSystemInfo->mySysInfo.chengfangtime*60*1000,g_chefang_error_process,NULL);
}

static int passwordhandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_pwdmenuico[1].x;
        int starty = rc_pwdmenuico[1].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0;i<MAX_PWD_BITS;i++)
                g_diplaypwd[i]='*';
        for(i=0; i<4;i++)
                for(j=0; j< 3;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){    	   	
                                count = (i*3)+j;
                                ret =1;
                                if(status == 1)
                                        g_rc_status[count] = true;
                                else{
                                        if(g_rc_status[count] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count] = false;
                                }		    	   	  
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(tempx,tempy, (unsigned char*)pNumberPointer1[count],pNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pNumberPointer[count],pNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_inputpwdcount < (MAX_PWD_BITS-1)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                strcat(g_inputpassword,(char*)&c);
                                                g_inputpwdcount++;
                                                g_diplaypwd[g_inputpwdcount]='\0';
                                                DrawText(g_diplaypwd,rc_pwdmenuico[0].x+10,rc_pwdmenuico[0].y+32,rgb_black);
                                        }else if(count  == 9){ //*
                                                WRT_Rect tmprect;
                                                g_inputpassword[g_inputpwdcount-1] = '\0';
                                                tmprect = rc_pwdmenuico[0];
                                                DeflateRect(&tmprect,4);
                                                //g_isUpdated = 0;
                                                FillRect(rgb_white,tmprect);
                                                //JpegDisp(rc_pwdmenuico[0].x,rc_pwdmenuico[0].y,mima_input,sizeof(mima_input));
                                                g_inputpwdcount--;
                                                if(g_inputpwdcount < 0)
                                                        g_inputpwdcount = 0;
                                                g_diplaypwd[g_inputpwdcount]='\0';
                                                DrawText(g_diplaypwd,rc_pwdmenuico[0].x+10,rc_pwdmenuico[0].y+32,rgb_black);
                                                //update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                                //g_isUpdated = 1;
                                        }else if(count == 11){//# //确认
                                        		if(pws_type == SET_CHEFANG_STATUS || pws_type == SET_CHEFANG_PWD_WIN)
                                    			{
                                    				if(pSystemInfo->mySysInfo.chengfangcnt == 0)
														pSystemInfo->mySysInfo.chengfangcnt = 3;

													
                                    				if(g_chefang_err_cnt >= pSystemInfo->mySysInfo.chengfangcnt)
                                					{
                                						char errbuf[128];
														if(pSystemInfo->mySysInfo.chengfangtime <= 0)
															pSystemInfo->mySysInfo.chengfangtime = 5;
														if(GetCurrentLanguage() == ENGLISH)
															sprintf(errbuf,"Password error,please try again after %d min",pSystemInfo->mySysInfo.chengfangtime);
														else
															sprintf(errbuf,"输入密码错太多，请%d分钟后再试",pSystemInfo->mySysInfo.chengfangtime);
														CreateDialogWin2(errbuf,MB_OK,NULL,NULL);
														if(g_chefang_err_cnt == pSystemInfo->mySysInfo.chengfangcnt)
															Process_Chefang_Error();

														return -1;
                                					}
													
                                    			}//如果是撤防则看统计错误次数
                                                if(strcmp(g_inputpassword,g_password) == 0){

                                                        if(pws_type == SET_CHEFANG_STATUS){
                                                                Window* p = FindWindow(SAFEWIN);
                                                                if(p){
                                                                        DestroyWindow(p);
                                                                        p = NULL;
                                                                }
                                                                cur_set_leave_home(false);
                                                                CloseTopWindow();
																g_chefang_err_cnt = 0;
                                                        }else if(pws_type == SET_SAFE_PWD_WIN){
                                                                CloseWindow();
																g_chefang_err_cnt = 0;
#ifdef WRT_MORE_ROOM
                                                                CreateSafeWin(0);
#else
                                                                CreateSafe2Win();
                                                               // password_chefang();
#endif
                                                                //CreateSafeWin(0);
                                                        }else if(pws_type == SET_JIAJU_PWD_WIN){
                                                                CloseWindow();
                                                                CreateHomeSetWin();                              
                                                        }else if(SET_DEFAULT_CONFIG_PWD_WIN == pws_type){ //恢复默认设置
                                                                CreateDialogWin2(LT("恢复出厂设置"),MB_NONE,NULL,NULL);
                                                                SetDefaultConfig(pSystemInfo);
                                                                SDL_Delay(2000);
                                                                UpdateSystemInfo();
                                                                CloseWindow();
                                                                CreateDialogWin2(LT("已经恢复出厂设置,5秒后重启"),MB_NONE,NULL,NULL);	
                                                                SDL_Delay(5000);
                                                                mcu_reset_sys(1);
                                                                SDL_Delay(5000);
                                                                Reset_sys();

                                                        }else if(SET_CHEFANG_PWD_WIN == pws_type){
                                                        	password_chefang();
															g_chefang_err_cnt = 0;
                                                        }else
                                                    	{
                                                    		if(pws_type == SET_CHEFANG_PWD_WIN || SET_CHEFANG_STATUS)
																g_chefang_err_cnt++;
															printf("1111111\n");
                                                            CreateDialogWin2(LT("密码错误!!!"),MB_OK,NULL,NULL);
                                                    	}


                                                }
                                                else if(strcmp(g_debugpassword,g_inputpassword) == 0){
                                                        if(pws_type == SET_NET_CONFIG_WIN){
                                                            CloseWindow();
                                                            CreateProjectSetupWin(); 
                                                        }else{
                                                    		if(pws_type == SET_CHEFANG_PWD_WIN || SET_CHEFANG_STATUS)
																g_chefang_err_cnt++;
															printf("11111112222\n");
                                                            CreateDialogWin2(LT("密码错误!!!"),MB_OK,NULL,NULL);
                                                        }
                                                }else
                                            	{
                                            		if(pws_type == SET_CHEFANG_PWD_WIN || SET_CHEFANG_STATUS)
														g_chefang_err_cnt++;
													printf("11111113333\n");
                                                    CreateDialogWin2(LT("密码错误!!!"),MB_OK,NULL,NULL);
                                            	}
                                        }

                                }
                                break;
                        }
                }
                return ret;
}

void  CreatePasswordWin(int type){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = PasswordEventhandler;
        pWindow->NotifyReDraw = DrawPasswordMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        pws_type = type;
        //memcpy(g_password,pSystemInfo->mySysInfo.password,8);

        GetNumberJpgPointer(&pNumberPointer,&pNumberPointer1);
        GetNumberJpegSizePointer(&pNumberSize,&pNumberSize1);
        pWindow->NotifyReDraw2(pWindow);
}
void  DrawPasswordMain(){
        modifypwd(); //2009-7-14
        
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillSingleRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

        JpegDisp(rc_pwdmenuico[2].x,rc_pwdmenuico[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_pwdmenuico[3].x,rc_pwdmenuico[3].y,bt_guanping1,sizeof(bt_guanping1)); 
        int i =0;
        int j =0;
        int startx = rc_pwdmenuico[1].x;
        int starty = rc_pwdmenuico[1].y;
        WRT_Rect tmprect;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pNumberPointer[(i*3)+j],pNumberSize[(i*3)+j]);
                }
                tmprect = rc_pwdmenuico[0];
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                //JpegDisp(rc_pwdmenuico[0].x,rc_pwdmenuico[0].y,mima_input,sizeof(mima_input));
                if(pws_type == SET_DEFAULT_CONFIG_PWD_WIN){
                        int yoffset = 80;
                        if(GetCurrentLanguage() == CHINESE){
                                yoffset = 160;
                                DrawText_16(LT("本操作会导致丢失个人资料和数据,"),rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-120,rgb_white);
                                DrawText_16(LT("而且所有设置将恢复到出厂设置."),rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-80,rgb_white);
                        }
                        DrawText_16(LT("恢复出厂设置."),rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-yoffset,rgb_white);
                        DrawText_16(LT("您要继续吗？请输入密码"),rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-40,rgb_white);
                        DrawText_16(LT("按'#'确认"),rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-10,rgb_white);
                        // DrawText_16(LT("请输入'1234',再按'#'",rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-10,rgb_white);   
                }else if(SET_CHEFANG_STATUS == pws_type){
                        char* tmp  = LT("离家");
                        char tmptxt[256];
#if HAVE_JIAJU_SAFE			
                        switch(g_cur_set_scene_mode){
                        
				            case SCENE_LIJIA:
				                    tmp = LT("离家");
				                    break;
				            case SCENE_WENXIN:
				                    tmp = LT("温馨");
				                    break;
				            case SCENE_HUIKE:
				                    tmp = LT("会客");
				                    break;
				            case SCENE_JIUCAN:
				                    tmp = LT("就餐");
				                    break;
				            case SCENE_YINGYUAN:
				                    tmp=LT("影院");
				                    break;
				            case SCENE_JIUQIN:
				                    tmp = LT("就寝");
				                    break;
                        }
#endif			

                        sprintf(tmptxt,"请输入密码解除'%s'状态,按'#'确认",tmp);
                        if(GetCurrentLanguage() == ENGLISH)
                                sprintf(tmptxt,"Input password",tmp);
                        DrawText_16(tmptxt,rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-10,rgb_white);
                }else
                        DrawText(LT("请输入密码:"),rc_pwdmenuico[0].x,rc_pwdmenuico[0].y-10,rgb_white);
		                WRT_Rect curbox;
		                curbox.x = 135;
		                curbox.y = 531;
		                curbox.w = 578;
		                curbox.h = 55;
		                //FillSingleRect(0xb2136,curbox);
			        JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));            
		                DrawText_16(LT("提示:按'*'退格，按'#'确认"),curbox.x,curbox.y+45,rgb_black);	
		                memset(g_inputpassword,0,MAX_PWD_BITS);
		                g_inputpwdcount = 0;

}

int   PasswordEventhandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rc_pwdmenuico[index],xy)){
                        pos_x = rc_pwdmenuico[index].x;
                        pos_y = rc_pwdmenuico[index].y;
                        ret = 1;         
                        switch(index){     
                  case 0:
                          break;        
                  case 2: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                  //if(SET_CHEFANG_STATUS !=  pws_type)          
                                  CloseTopWindow();                   
                          }
                          break;
                  case 3:/*关屏*/
                          if(status == 1){ //按下
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                  screen_close(); 
                                  CloseAllWindows();
                          }
                          break;
                  case 1:
                          //进入密码区域
                          ret = passwordhandler(x,y,status);
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }      
        return ret;  	
}
///////////////////////////////////////////////////////////////////////////////////
//
static int cancel_pwd_error(int param){
        CloseAllWindows();
        return 0;
}
static int setpasswordhandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int tmpY =  0;
        WRT_Rect tmprect;
        int startx = rc_pwdmenuico[1].x;
        int starty = rc_pwdmenuico[1].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0;i<MAX_PWD_BITS;i++)
                g_diplaypwd[i]='*';
        for(i=0; i<4;i++)
                for(j=0; j< 3;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){    	   	
                                count = (i*3)+j;
                                ret =1;
                                if(status == 1)
                                        g_rc_status[count] = true;
                                else{
                                        if(g_rc_status[count] == false){
                                                ret = 0;
                                                return ret;
                                        }
                                        g_rc_status[count] = false;
                                }		    	   	  
                                if(status == 1){
                                        StartButtonVocie();
                                        JpegDisp(tempx,tempy, (unsigned char*)pNumberPointer1[count],pNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pNumberPointer[count],pNumberSize[count]);
                                        if( (count != 9 && count != 11) && (g_inputpwdcount < (MAX_PWD_BITS-1)) ){ //9 *，11 #
                                                c[0] = g_number[count];
                                                strcat(g_inputpassword,(char*)&c);
                                                g_inputpwdcount++;
                                                g_diplaypwd[g_inputpwdcount]='\0';
                                                tmpY=rc_setpwdmenuico[0].y+(g_inputstatus-1)*80;	    	
                                                DrawText(g_diplaypwd,rc_pwdmenuico[0].x+10,tmpY+32,rgb_black);
                                        }else if(count  == 9){ //*
                                                g_inputpassword[g_inputpwdcount-1] = '\0';
                                                g_inputpwdcount--;
                                                if(g_inputpwdcount < 0)
                                                        g_inputpwdcount = 0;
                                                g_diplaypwd[g_inputpwdcount]='\0';	    	      		 
                                                tmpY=rc_setpwdmenuico[0].y+(g_inputstatus-1)*80;	
                                                tmprect =  rc_setpwdmenuico[0];
                                                tmprect.y = tmpY;
                                                DeflateRect(&tmprect,4);
                                                g_isUpdated = 0;
                                                FillRect(rgb_white,tmprect);
                                                
                                                DrawText(g_diplaypwd,rc_pwdmenuico[0].x+10,tmpY+32,rgb_black);
                                                update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                                g_isUpdated = 1;
                                        }else if(count == 11){//# //确认
                                                if(g_inputstatus == 1){
                                                        if(strcmp(g_inputpassword,g_password)!=0)
                                                                /*if(strcmp(g_inputpassword,g_debugpassword) != 0)*/{
                                                                        CreateDialogWin2(LT("旧密码错误，再次输入？"),MB_OK|MB_CANCEL,NULL,cancel_pwd_error);
                                                                        break;
                                                        }
                                                        memset(g_inputpassword,0,MAX_PWD_BITS);
                                                        g_diplaypwd[g_inputpwdcount]='\0';
                                                        tmpY=rc_setpwdmenuico[0].y + (g_inputstatus-1)*80;
                                                        tmprect = rc_setpwdmenuico[0];
                                                        tmprect.y = tmpY;
                                                        g_isUpdated = 0;
                                                        DrawRect(tmprect,2,rgb_white);
                                                        DeflateRect(&tmprect,4);
                                                        FillRect(rgb_white,tmprect);
                                                        
                                                        DrawText(g_diplaypwd,rc_pwdmenuico[0].x+10,tmpY+32,rgb_black); 	    	      			  
                                                        g_inputpwdcount=0;
                                                        update_rect(rc_setpwdmenuico[0].x,tmpY,rc_setpwdmenuico[0].w,rc_setpwdmenuico[0].h);
                                                        g_isUpdated = 1;
                                                }
                                                if(g_inputstatus  == 2){
                                                        /*
                                                        if(strlen(g_inputpassword) < 8){
                                                        CreateDialogWin2("请输入8位数的密码!",MB_OK,NULL,NULL);
                                                        break;
                                                        } 
                                                        */                           
                                                        strcpy(g_newpassword,g_inputpassword);
                                                        memset(g_inputpassword,0,MAX_PWD_BITS);
                                                        g_diplaypwd[g_inputpwdcount]='\0';
                                                        tmpY=rc_setpwdmenuico[0].y + (g_inputstatus-1)*80;
                                                        tmprect = rc_setpwdmenuico[0];
                                                        tmprect.y = tmpY;
                                                        g_isUpdated = 0;
                                                        DrawRect(tmprect,2,rgb_white);
                                                        DeflateRect(&tmprect,4);
                                                        FillRect(rgb_white,tmprect);
                                                        //JpegDisp(rc_setpwdmenuico[0].x,tmpY,mima_input,sizeof(mima_input));
                                                        DrawText(g_diplaypwd,rc_pwdmenuico[0].x+10,tmpY+32,rgb_black); 	    	      			  
                                                        g_inputpwdcount=0;
                                                         update_rect(rc_setpwdmenuico[0].x,tmpY,rc_setpwdmenuico[0].w,rc_setpwdmenuico[0].h);
                                                        g_isUpdated = 1;	
                                                        /*
                                                        if(strlen(g_newpassword) < 8){
                                                        }
                                                        */
                                                } 
                                                if(g_inputstatus == 3 ){
                                                        if(strcmp(g_newpassword ,g_inputpassword) !=0 )
                                                                CreateDialogWin2(LT("两次输入新密码不同，再次输入？"),MB_OK|MB_CANCEL,NULL,cancel_pwd_error);
                                                        else{
                                                                ConfirmResetPwd(0);
                                                                CreateDialogWin2(LT("修改密码成功！"),MB_OK,NULL,NULL);
                                                        }
                                                        break;
                                                }
                                                g_inputstatus++;
                                                tmpY=rc_setpwdmenuico[0].y + (g_inputstatus-1)*80;
                                                tmprect = rc_setpwdmenuico[0];
                                                tmprect.y = tmpY;
                                                g_isUpdated = 0;
                                                DrawRect(tmprect,2,rgb_yellow);
                                                DeflateRect(&tmprect,4);
                                                FillRect(rgb_white,tmprect);
                                                update_rect(rc_setpwdmenuico[0].x,tmpY,rc_setpwdmenuico[0].w,rc_setpwdmenuico[0].h);
                                                g_isUpdated = 1;                                                        
                                               // JpegDisp(rc_setpwdmenuico[0].x,tmpY,mima_input2,sizeof(mima_input2));

                                        }
                                }
                                break;
                        }
                }
                return ret;
}

void CreateSetPasswordWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = SetPasswordEventHandler;
        pWindow->NotifyReDraw = DrawSetPasswordMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        GetNumberJpgPointer(&pNumberPointer,&pNumberPointer1);
        GetNumberJpegSizePointer(&pNumberSize,&pNumberSize1);
        pWindow->NotifyReDraw2(pWindow);

        // memcpy(g_password,pSystemInfo->mySysInfo.password,8);
}
void DrawSetPasswordMain(){
        modifypwd(); //2009-7-14
        
		WRT_Rect rt;
				rt.x = 0;
				rt.y = 0;
				rt.w =1024;
				rt.h = 530;
				FillSingleRect(0xb2136,rt);
		
				JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
        //JpegDisp(0,0,menu_sub_back,sizeof(menu_sub_back));   
        JpegDisp(rc_setpwdmenuico[2].x,rc_setpwdmenuico[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_setpwdmenuico[3].x,rc_setpwdmenuico[3].y,bt_guanping1,sizeof(bt_guanping1)); 
        int i =0; 
        int j =0;
        int startx = rc_setpwdmenuico[1].x;
        int starty = rc_setpwdmenuico[1].y;
        WRT_Rect tmprect;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pNumberPointer[(i*3)+j],pNumberSize[(i*3)+j]);
                }
                tmprect  = rc_setpwdmenuico[0];
                DrawRect(tmprect,2,rgb_yellow);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                DrawText(LT("请输入旧密码:"),rc_setpwdmenuico[0].x,rc_setpwdmenuico[0].y-10,rgb_white);
                
                tmprect  = rc_setpwdmenuico[0];
                tmprect.y += 80;
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);    
                DrawText(LT("请输入新密码:"),rc_setpwdmenuico[0].x,rc_setpwdmenuico[0].y+70,rgb_white);

                tmprect  = rc_setpwdmenuico[0];
                tmprect.y += 160;
                DrawRect(tmprect,2,rgb_white);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);                  
                DrawText(LT("请再次输入新密码:"),rc_setpwdmenuico[0].x,rc_setpwdmenuico[0].y+150,rgb_white);
                WRT_Rect curbox;
                curbox.x = 135;
                curbox.y = 531;
                curbox.w = 578;
                curbox.h = 55;
	        JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
	                       
        SDL_Delay(20);
                DrawText_16(LT("提示:按'*'退格，按'#'确认"),curbox.x,curbox.y+45,rgb_black);	
                memset(g_inputpassword,0,MAX_PWD_BITS);
                memset(g_newpassword,0,MAX_PWD_BITS);
                g_inputpwdcount = 0;	 
                g_inputstatus = 1;	
}
int  SetPasswordEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int tmpret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rc_setpwdmenuico[index],xy)){
                        pos_x = rc_setpwdmenuico[index].x;
                        pos_y = rc_setpwdmenuico[index].y;
                        ret = 1;         
                        switch(index){     
                  case 0:
                          break;        
                  case 2: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                  CloseTopWindow();                   
                          }
                          break;
                  case 3:/*关屏*/
                          if(status == 1){ //按下
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                  screen_close(); 
                                  CloseAllWindows();
                          }
                          break;
                  case 1:
                          //进入数字区域
                          ret = setpasswordhandler(x,y,status);	  
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }      
        return ret;  	
}

int ConfirmResetPwd(int param){
        memset(pSystemInfo->mySysInfo.password,0,8);
        memcpy(pSystemInfo->mySysInfo.password,g_newpassword,8);
        //  memset(g_password,0,MAX_PWD_BITS);
        //  strcpy(g_password,g_newpassword);
        FlushSystemInfo();

#ifdef WRT_MORE_ROOM
        //当触控分机修改了防区布撤防状态，同步防区状态
        sync_pwd(0,g_newpassword,8);                       
#endif
	return 0;
}

void modifypwd(){
        if(pSystemInfo->mySysInfo.password[0] != 0xff)
                memcpy(g_password,pSystemInfo->mySysInfo.password,8);
}
