#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "safe_menu_res.h"

#include "window.h"

#include "RoomGpioUtil.h"
#include "c_interface.h"

#include "CmdId.h"
#include "enet/enet.h"


//从char[0]到char[15]分别为1-16防区状态(是布防还是撤防状态)
static void SendLogToCenter(unsigned int id)//id == 6 arm,id == 7 disalarm
{ 
   char tmpbuf[57]; 
   memset(tmpbuf,0,57);//16个防区
   short cmd = 0; 
   char buf[16]; 
   char id_c = 0; 
   id_c = id; 
   memset(buf,0,16); 
   buf[0] = pSystemInfo->mySysInfo.safestatus[16];
   buf[1] = pSystemInfo->mySysInfo.safestatus[17];
   for(int i=2;i<16;i++) 
      buf[i] = pSystemInfo->mySysInfo.safestatus[i];
   int len = 57;
   strncpy(tmpbuf,"WRTI",4);
   memcpy(tmpbuf+4,&len,sizeof(int));
   cmd = DEV_SEND_DEBUGLOG;
   memcpy(tmpbuf+8,&cmd,2);
   strncpy(tmpbuf+10,(const char*)pSystemInfo->LocalSetting.privateinfo.LocalID,15); 
   memcpy(tmpbuf+40,&id_c,1);
   memcpy(tmpbuf+41,buf,16);

   App_Enet_Send2(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,20302,tmpbuf,len); 
}

static int Get_safe_status_for_playalarm()
{
	if(pSystemInfo->mySysInfo.cur_scene == 255)
	{
		for(int i=2;i<16;i++){
			if(pSystemInfo->mySysInfo.safestatus[i] == 0x01){
				return 1;
			}
	    }
    }else
    {
    	for(int i=2;i<16;i++){
			if(get_security_status(i)== 0x01){
				return 1;
			}
	    }
    }
    return 0;
}
extern bool g_have_msg;
extern bool g_isstartdelay;
extern int g_cur_select_scene_mode; //配置时挑选的情景。
extern bool     g_have_defend ;   //是否设置设防
static const WRT_Rect rc_safemenuico[6]={
        {326,43,132,160},   //全部布防
        {565,43,132,160},   //全部撤防
        {150,240,750,128},    //防区区域
        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

static bool m_isSetSecurity[16] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //保存零时设置的防区
//bool        g_curSetSafeArea[16]; //当前设置的安全防区
static int  m_cur_safe_type = 0;
extern int g_chefang_err_cnt;

#ifdef WRT_MORE_ROOM
void sync_all_zone()
{
        //当触控分机修改了防区布撤防状态，同步防区状态
        unsigned short zonestate = 0;
        int i = 0;
        for(i = 2; i<16;i++)
        {
                if( pSystemInfo->mySysInfo.safestatus[i] == 0x01)
                        zonestate |= (1<<i); 
        }
        if(zonestate == 0){
                if(g_have_msg == false){
                        if(IsNewPhoto()){
                                open_green_led(1);
                        }else
                                open_green_led(0);
                }else
                        open_green_led(g_have_msg);
        }
        //tm_wkafter(100);
        sync_zone(0,(unsigned char*)&zonestate,2);                       

}
#endif

void SendAlarmLogToCenter(int value)
{
	if(value)
		SendLogToCenter(6);
	else
		SendLogToCenter(7);
}

void start_defend(bool value){//2009-7-21
        int i =0;
        for(i =0; i<16;i++){
                if(pSystemInfo->mySysInfo.safestatus[i+16] == 1) //判断防区是否启用，没有启用返回没有布防。
                        break;
        }
        if(i == 16)
                return;
        unsigned long event[4];
        check_alarm2();
        if(value == true)
        {
        	if(Get_safe_status_for_playalarm()==1)
        	{
		        event[0] = CUSTOM_CMD_WARNING_TIMER;
		        event[1] = event[2] = event[3] = 0;
		        wrthost_send_cmd(event);
		        g_isstartdelay = true;
		        SDL_Delay(100);
		        StartAlaramPromptVocie();
	        }
        }else
        {
        	g_isstartdelay = false;
        }
        
        g_have_defend = true;

        if(value)
        	SendLogToCenter(6);
        else
        	SendLogToCenter(7);

		g_chefang_err_cnt = 0;
		tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);
}

int   GetIsStartDefendStatus(){
        int i =0;
        for(i =0; i<16;i++){
                if(pSystemInfo->mySysInfo.safestatus[i+16] == 1) //判断防区是否启用，没有启用返回没有布防。
                        break;
        }
        if(i == 16)
    	{
    		tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);
            return 0;
    	}
        return 1;
}

int  scene_is_relation_zone(int mode){
	int i =0;
	for(i = 0; i<MAX_SCENE_MODE;i++){
		if(g_SCENE_MODE_TABLE[i].scene ==  pSystemInfo->mySysInfo.scenenumber[mode])
			return g_SCENE_MODE_TABLE[i].relation;
	}
	return 0;
}


bool get_security_status(int index){
        if(index > 15)
                return false;
        if(pSystemInfo->mySysInfo.safestatus[index+16] == 0) //判断防区是否启用，没有启用返回没有布防。
                return false;
        //WRT_DEBUG("g_cur_set_scene_mode value is %d-%d\n",pSystemInfo->mySysInfo.cur_scene,scene_is_relation_zone(pSystemInfo->mySysInfo.cur_scene));

        if(scene_is_relation_zone(pSystemInfo->mySysInfo.cur_scene) == 1 )
        	return get_cur_scenemode_safe_status(pSystemInfo->mySysInfo.cur_scene,index);
        else
        	return pSystemInfo->mySysInfo.safestatus[index];

}

void set_security_status(int index,bool status){
        if(index > 15)
                return;

        if(pSystemInfo->mySysInfo.safestatus[index+16] == 0) //判断防区是否启用，没有启用则返回
                return;
        if(index == 0 || index == 1)
        {
        	pSystemInfo->mySysInfo.safestatus[index]  = 1; 
            return;
        }
        if(scene_is_relation_zone(g_cur_set_scene_mode) == 1)
          	set_cur_scenemode_safe_status(g_cur_set_scene_mode,index,status);
        else
          	pSystemInfo->mySysInfo.safestatus[index]  = status;               
}

bool isenablesafe(){
        int i,j;
        /*
        for(i =0; i<15;i++){
        for(j =1;j<16;j++){
        if(pSystemInfo->mySysInfo.safestatus[i+16] != pSystemInfo->mySysInfo.safestatus[j+16]){
        return false;
        }
        }
        }
        return true;
        */
        for(i =0; i< 8;i++)
                if(pSystemInfo->mySysInfo.safestatus[i+16] == 0x01)
                        return true;
        return false;
}




void test_alarm_for_factory(){
        int i =0;

        //test_extern_device();

        for(i=0; i<32;i++){
                pSystemInfo->mySysInfo.safestatus[i] = 1;
        }
#ifdef WRT_MORE_ROOM
        //当触控分机修改了防区布撤防状态，同步防区状态
        sync_all_zone();
#endif
		pSystemInfo->mySysInfo.cur_scene  = 255;
        pSystemInfo->mySysInfo.alarmtime  = 1;
        pSystemInfo->mySysInfo.alarmtime1 = 1;
        unsigned long event[4];
        check_alarm2();//清空之前的报警信息，重新开始布防。
        SDL_Delay(100);
        event[0] = CUSTOM_CMD_WARNING_TIMER;
        event[1] = event[2] = event[3] = 0;
        wrthost_send_cmd(event);
        g_isstartdelay = true;
        g_have_defend = true;
        WRT_DEBUG("已经开始布防");
        SDL_Delay(100);
        StartAlaramPromptVocie();
        CloseAllWindows();
        
}

void  password_chefang()
{
	int i =0;
	int issave = 0;
	char str[128];	
	for(i=0;i<16;i++)
		m_isSetSecurity[i]=false;
	m_isSetSecurity[0] = true;
	m_isSetSecurity[1] = true;	
	sprintf(str,"%s",LT("全部撤防"));
	add_log(str);
    //撤防取消防区布撤防配置，如有配置情景，则取消情景对应防区配置
	for(i=0;i<16;i++){
		//pSystemInfo->mySysInfo.safestatus[i] = m_isSetSecurity[i];
		set_security_status(i,m_isSetSecurity[i]);
		if(m_isSetSecurity[i]== true){
			sprintf(str,"%d %s",i+1,LT("防区布防"));
			add_log(str);

		}
	}
	WRT_DEBUG("撤防处理");
	CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
	
	ClearViewSafeStatus();
	if(pSystemInfo->mySysInfo.cur_scene != 255)
	{
		chefang_jiaju();
	}
	
	UpdateSystemInfo();
	
	save_scene_mode_table_to_flash();
	
	check_alarm(0);
	
	CloseAllWindows();
	
	mp3stop();

	SendLogToCenter(7);
	tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);

	g_chefang_err_cnt = 0;
}

void  remote_chefang()
{
	int i =0;
	int issave = 0;
	char str[128];
	for(i=0;i<16;i++)
		m_isSetSecurity[i]=false;
	m_isSetSecurity[0] = true;
	m_isSetSecurity[1] = true;
    sprintf(str,"%s",LT("全部撤防"));
    add_log(str);
    //撤防取消防区布撤防配置，如有配置情景，则取消情景对应防区配置
	for(i=2;i<16;i++){
		//pSystemInfo->mySysInfo.safestatus[i] = m_isSetSecurity[i];
		set_security_status(i,m_isSetSecurity[i]);
		if(m_isSetSecurity[i]== true){
			sprintf(str,"%d %s",i+1,LT("防区布防"));
			add_log(str);
		}
	}
	//
	WRT_DEBUG("撤防处理");

	ClearViewSafeStatus();
	UpdateSystemInfo();
	check_alarm(0);

	SendLogToCenter(7);
	tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);
	g_chefang_err_cnt = 0;
}

int  ok_safe(int param){
		pSystemInfo->mySysInfo.cur_scene = 255;
		UpdateSystemInfo();

        if(m_cur_safe_type == 0){

                int i;
                int issave = 1;
                int ret = 0;
                char str[128];
                for(i=2;i<16;i++){
                    pSystemInfo->mySysInfo.safestatus[i] = m_isSetSecurity[i];
                    issave = 1;
                    if(pSystemInfo->mySysInfo.safestatus[i] == true){
                            sprintf(str,"%d %s",i+1,LT("防区布防"));
                            add_log(str);
                            ret = 1;
                    }
                }

                if(issave){
                        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                        save_localhost_config_to_flash();
                        UpdateSystemInfo(); 
                }
                if(ret == 1 ){//重新开始布防
#ifdef WRT_MORE_ROOM
                        close_all_channel();
#endif
			
                        unsigned long event[4];
                        
                        check_alarm2();//清空之前的报警信息，重新开始布防。
                        SDL_Delay(100);
                        event[0] = CUSTOM_CMD_WARNING_TIMER;
                        event[1] = event[2] = event[3] = 0;
                        wrthost_send_cmd(event);
                        g_isstartdelay = true;
                        g_have_defend = true;
                        WRT_DEBUG("已经开始布防");
                        SDL_Delay(100);
                        StartAlaramPromptVocie();
                        CloseAllWindows();

#ifdef WRT_MORE_ROOM
                        //当触控分机修改了防区布撤防状态，同步防区状态
                        sync_all_zone();
#endif
                }else{
#ifdef WRT_MORE_ROOM
                        //当触控分机修改了防区布撤防状态，同步防区状态
                        sync_all_zone();
#endif
                        check_alarm(0);
                        CloseAllWindows();

                }

        }else{
                int i =0;
                for( i=0; i<16;i++){
                        if(g_cur_select_scene_mode == SCENE_ZAIJIA){ //在家模式
                                set_cur_scenemode_safe_status(g_cur_select_scene_mode,i,0);
                        }else if(g_cur_select_scene_mode == SCENE_LIJIA){
                                set_cur_scenemode_safe_status(g_cur_select_scene_mode,i,1);
                        }else
                                set_cur_scenemode_safe_status(g_cur_select_scene_mode,i,m_isSetSecurity[i]);
                }
                CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
                save_scene_mode_table_to_flash();
                
                CloseTopWindow();
        }
        Window* p = FindWindow(SAFEWIN);
        if(p){
                DestroyWindow(p);
                p = NULL;
        }
        if(Get_safe_status_for_playalarm() == 0)
        {
        	mp3stop();
        	SendLogToCenter(7);
        }else

        SendLogToCenter(6);
        tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);
		
        return 0;
}

int Reset_safe(int param){
	int i =0;
	for(i=2;i<16;i++)
		m_isSetSecurity[i]=false;
	return 0;                
}

static int safehandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_safemenuico[2].x;
        int starty = rc_safemenuico[2].y;
        for(i=0; i<1;i++)
                for(j=0; j< 8;j++){
                        tempx = startx+(j)*96;
                        tempy = starty+(i)*96;
                        if(x > tempx && x < (tempx+48) && y > tempy  && y < (tempy+48)){
                                count = (i*8)+j;
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
                                if(pSystemInfo->mySysInfo.safestatus[count+16] == 0) //如果防区没有启动，则不需要处理
                                        break;
                                if(status == 1){
                                        StartButtonVocie();
                                }else{
                                        if(count == 0 || count == 1)
                                                break;
                                        if(m_isSetSecurity[count] == true ){
                                                JpegDisp(tempx,tempy, ico_fangquguan,sizeof(ico_fangquguan));
                                                m_isSetSecurity[count] = false;
                                        }else{
                                                JpegDisp(tempx,tempy, ico_fangqukai,sizeof(ico_fangqukai));
                                                m_isSetSecurity[count] = true;
                                        }
                                }
                                break;
                        }
                }
                return ret;
}

void  CreateSafeWin(int type){
        int i =0;
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SafeEventHandler;
        pWindow->NotifyReDraw = DrawSafeMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        m_cur_safe_type = type;

        if(m_cur_safe_type == 0){
                for(i=0;i<16;i++)
                        m_isSetSecurity[i] = pSystemInfo->mySysInfo.safestatus[i];//如果非在家模式，则查看的是，该模式下安防设置，如果为在家模式则，
                //查看的是单独控制的防区。pSystemInfo->mySysInfo.safestatus[i];
        }else{
                for(i=0;i<16;i++){
                        m_isSetSecurity[i] = get_cur_scenemode_safe_status(g_cur_select_scene_mode,i);
                }
        }
        
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}
void DrawSafeMain(){
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

        int i=0;
        int j=0;
        int count = 0;
        int startx = rc_safemenuico[2].x;
        int starty = rc_safemenuico[2].y;
        int tempx  = 0;
        int tempy = 0;
        char tmptxt[32];
        for(i=0;i<1;i++){
                for(j=0; j<8;j++){
                        count = i*8+j;
                        tempx = startx+j*96;
                        tempy = starty+i*96;
                        if(pSystemInfo->mySysInfo.safestatus[16+count] == 0){
                                JpegDisp(tempx,tempy,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
                        }else{
                                if(m_isSetSecurity[count])
                                        JpegDisp(tempx,tempy,ico_fangqukai,sizeof(ico_fangqukai));
                                else
                                        JpegDisp(tempx,tempy,ico_fangquguan,sizeof(ico_fangquguan));
                        }
                        sprintf(tmptxt,"%s%d",LT("防区"),count+1);
                        DrawText_16(tmptxt,tempx,tempy+70,rgb_white);

                }
        }
        JpegDisp(rc_safemenuico[0].x,rc_safemenuico[0].y,menu_quanbubufang1,sizeof(menu_quanbubufang1));
        JpegDisp(rc_safemenuico[1].x,rc_safemenuico[1].y,menu_quanbuchefang1,sizeof(menu_quanbuchefang1));
        JpegDisp(rc_safemenuico[3].x,rc_safemenuico[3].y,menu_ok1,sizeof(menu_ok1));
        JpegDisp(rc_safemenuico[4].x,rc_safemenuico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_safemenuico[5].x,rc_safemenuico[5].y,bt_guanping1,sizeof(bt_guanping1));
        int xoffset = rc_safemenuico[0].x +(rc_safemenuico[0].w - GetTextWidth(LT("全部布防"),0))/2;
        int yoffset = rc_safemenuico[0].y+rc_safemenuico[0].h;
        DrawText(LT("全部布防"),xoffset,yoffset,rgb_white);

        xoffset = rc_safemenuico[1].x +(rc_safemenuico[1].w - GetTextWidth(LT("全部撤防"),0))/2;
        yoffset = rc_safemenuico[1].y+rc_safemenuico[1].h;
        DrawText(LT("全部撤防"),xoffset,yoffset,rgb_white);
}

int  SafeEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_safemenuico[index],xy)){
                        pos_x = rc_safemenuico[index].x;
                        pos_y = rc_safemenuico[index].y;
                        ret = 1;
                        switch(index){
                  case 0://全部布防
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_quanbubufang2,sizeof(menu_quanbubufang2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_quanbubufang1,sizeof(menu_quanbubufang1));
                                  int i =0;
                                  for(i=0;i<16;i++){
                                          if(pSystemInfo->mySysInfo.safestatus[16+i] == 1)//如果防区没有启动则不处理
                                                  m_isSetSecurity[i]=true;
                                  }
                                  //ReDrawCurrentWindow();
                                  g_isUpdated = 0;
                                  DrawSafeMain();
                                  update_rect(0,0,1024,531);
                                  g_isUpdated = 1;
                                  CreateDialogWin2(LT("确定重置防区!"),MB_OK|MB_CANCEL,ok_safe,Reset_safe);

                          }
                          break;
                  case 1://全部撤防
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_quanbuchefang2,sizeof(menu_quanbuchefang2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_quanbuchefang1,sizeof(menu_quanbuchefang1));
                                  int i =0;
                                  for(i=0;i<16;i++)
                                          m_isSetSecurity[i]=false;
                                  m_isSetSecurity[0] = true;
                                  m_isSetSecurity[1] = true;
                                  //ReDrawCurrentWindow();
                                  g_isUpdated = 0;
                                  DrawSafeMain();
								  update_rect(0,0,1024,531);
								  g_isUpdated = 1;
                                  CreateDialogWin2(LT("确定重置防区!"),MB_OK|MB_CANCEL,ok_safe,Reset_safe);
                          }
                          break;
                  case 3://确定
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                  CreateDialogWin2(LT("确定重置防区!"),MB_OK|MB_CANCEL,ok_safe,Reset_safe);
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
                  case 2:
                          //进入防区区域
                          ret = safehandler(x,y,status);//enablesafehandler(x,y,status);
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }
        return ret;
}

/*
void Set_Safe_Status(bool bsafe[16]){
for(int i=0;i<16;i++){
g_curSetSafeArea[i] = bsafe[i];
}
}
*/
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
static int enablesafehandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int startx = rc_safemenuico[2].x;
        int starty = rc_safemenuico[2].y;
        //printf("存在bug 第二行的按键位置有错\n");
        for(i=0; i<1;i++)
                for(j=0; j< 8;j++){
                        tempx = startx+(j)*96;
                        tempy = starty+(i)*96;
                        if(x > tempx && x < (tempx+48) && y > tempy  && y < (tempy+48)){
                                count = (i*8)+j;
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
                                }else{
                                        if(m_isSetSecurity[count] == true){
                                                JpegDisp(tempx,tempy, ico_fangquguan,sizeof(ico_fangquguan));
                                                m_isSetSecurity[count] = false;
                                        }else{
                                                JpegDisp(tempx,tempy, ico_fangqukai,sizeof(ico_fangqukai));
                                                m_isSetSecurity[count] = true;
                                        }
                                        
                                }
                                break;
                        }
                }
                return ret;
}

void CreateEnableSafeWin(){
        int i ;
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = EnableSafeEventHandler;
        pWindow->NotifyReDraw = DrawEnableSafeMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        for(i =0; i<16; i++)
                m_isSetSecurity[i] =  pSystemInfo->mySysInfo.safestatus[16+i];
        pWindow->NotifyReDraw2(pWindow);
}

void DrawEnableSafeMain(){
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

        int i=0;
        int j=0;
        int count = 0;
        WRT_Rect curclear;
        curclear.x = 227;
        curclear.y = 63;
        curclear.w = 345;
        curclear.h = 139;
        //FillRect(BACK_COLOR,curclear);
        char* text = LT("请选择需要启动的防区");
        char  tmptxt[10];
        DrawText(text,(1024-GetTextWidth(text,0))/2,100,rgb_white);
        int startx = rc_safemenuico[2].x;
        int starty = rc_safemenuico[2].y;
        int tempx  = 0;
        int tempy = 0;

        for(i=0;i<1;i++){
                for(j=0; j<8;j++){
                        count = i*8+j;
                        tempx = startx+j*96;
                        tempy = starty+i*96;
                        if(m_isSetSecurity[count])
                                JpegDisp(tempx,tempy,ico_fangqukai,sizeof(ico_fangqukai));
                        else
                                JpegDisp(tempx,tempy,ico_fangquguan,sizeof(ico_fangquguan));
                        sprintf(tmptxt,"%s%d",LT("防区"),count+1);
                        DrawText_16(tmptxt,tempx,tempy+70,rgb_white);
                }
        }
        JpegDisp(rc_safemenuico[3].x,rc_safemenuico[3].y,menu_ok1,sizeof(menu_ok1));
        JpegDisp(rc_safemenuico[4].x,rc_safemenuico[4].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_safemenuico[5].x,rc_safemenuico[5].y,bt_guanping1,sizeof(bt_guanping1));

}
int EnableSafeEventHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<6;index++){
                if(IsInside(rc_safemenuico[index],xy)){
                        pos_x = rc_safemenuico[index].x;
                        pos_y = rc_safemenuico[index].y;
                        ret = 1;
                        switch(index){
                    case 3: //确定
                            if(status == 1){
                                    StartButtonVocie();
                                    JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                            }else{
                                    JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));
                                    CreateDialogWin2(LT("确定启用选定防区!"),MB_OK|MB_CANCEL,enable_safe,NULL);
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
                    case 5: /*关屏*/
                            if(status == 1){ //按下
                                    StartButtonVocie();
                                    JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                            }else{
                                    JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                    screen_close();
                                    CloseAllWindows();
                            }
                            break;
                    case 2:
                            //进入防区区域
                            ret = enablesafehandler(x,y,status);
                            break;
                    default:
                            break;
                        }
                        break;
                }
        }
        return ret;
}

int  enable_safe(int param){
        int i=0;
        int j= 0;
        int prestatus = 0;
        char str[128];
        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
        
        for(i=0; i< 16;i++){
                prestatus = pSystemInfo->mySysInfo.safestatus[i+16] ;
                pSystemInfo->mySysInfo.safestatus[i+16] = m_isSetSecurity[i];
                if(m_isSetSecurity[i] == 1){
                        if(i == 0 || i== 1)
                                pSystemInfo->mySysInfo.safestatus[i]= 1; //只要防区1，2,启用，则常态布防。
                        sprintf(str,"%s %d",LT("启用防区"),i+1);
                        add_log(str);
                }


                if((prestatus == 1) && prestatus != pSystemInfo->mySysInfo.safestatus[i+16]){ //如果防区，由启用改变为没有启动，则修改相应的防区设置。
#if HAVE_JIAJU_SAFE
                        pSystemInfo->mySysInfo.safestatus[i] = 0; //修改单独布防撤防区域
                        sprintf(str,"%s %d",LT("禁用防区"),i+1);
                        add_log(str);
                        for(j =0; j<MAX_SCENE_MODE ;j++){ //修改所对应的模式下的防区。
                                set_cur_scenemode_safe_status(j,i,0);
                        }
#else
                        pSystemInfo->mySysInfo.safestatus[i] = 0; //修改单独布防撤防区域
                        sprintf(str,"%s %d",LT("禁用防区"),i+1);
                        add_log(str);
#endif
                }

        }
        save_localhost_config_to_flash();
        if(Get_safe_status_for_playalarm() == 0)
        {
        	mp3stop();
        }
        UpdateSystemInfo();
        CloseWindow();
		g_chefang_err_cnt = 0;
        return 0;

}

///////////////////////////////////////////////////////
//
//
static WRT_Rect rcsafe2[4]={
        {326,145,132,160},             //防区启用
        {565,145,132,160},             //布撤防

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

void CreateSafe2Win(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = Safe2EventHandle;
        pWindow->NotifyReDraw = DrawSafe2Main;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);
}

void DrawSafe2Main(){
        int xoffset = 0;
        WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);
		
		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

        JpegDisp(rcsafe2[0].x,rcsafe2[0].y,menu_qiyongfangqu1,sizeof(menu_qiyongfangqu1));
        JpegDisp(rcsafe2[1].x,rcsafe2[1].y,menu_buchefang1,sizeof(menu_buchefang1));

        JpegDisp(rcsafe2[2].x,rcsafe2[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcsafe2[3].x,rcsafe2[3].y,bt_guanping1,sizeof(bt_guanping1));

        DrawEnText(LT("防区启用"),rcsafe2[0].w+xoffset,rcsafe2[0].h,rcsafe2[0].x,rcsafe2[0].y);
        DrawEnText(LT("布/撤防"),rcsafe2[1].w+xoffset,rcsafe2[1].h,rcsafe2[1].x,rcsafe2[1].y);
}

int Safe2EventHandle(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rcsafe2[index],xy)){
                        pos_x = rcsafe2[index].x;
                        pos_y = rcsafe2[index].y;
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
                  case 0://启用防区
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_qiyongfangqu2,sizeof(menu_qiyongfangqu2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_qiyongfangqu1,sizeof(menu_qiyongfangqu1));
                                  CreateEnableSafeWin();
                          }
                          break;
                  case 1://布撤防
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_buchefang2,sizeof(menu_buchefang2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_buchefang1,sizeof(menu_buchefang1));
                                  CreateSafeWin(0);

                          }
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
                  default:
                          break;
                        }
                        break;
                }
        }
        return ret;
}

///////////////////////////////////////////////////////
//
static WRT_Rect rcsafe3[4]={
        {189,149,132,160},             //防区启用
        {428,149,132,160},             //布撤防

        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

static int g_isviewstatus[16]={0};

void SetViewSafeStatus(int index){
        g_isviewstatus[index] = 1;
}

int GetViewSafeStatus(int index){
        return g_isviewstatus[index];
}
void ClearViewSafeStatus(){
        for(int i =0 ; i<16;i++)
                g_isviewstatus[i] = 0;
				//	chefang_reset_zoneProbe();
}

void ViewSafeSatus(int isupdate){
        char text[256];
        WRT_Rect rt;
        int offset = 40;
        rt.x = 0;
        rt.y = 30;
        rt.w = 290;
        rt.h = 390;
		
	if(isupdate == 1)
		g_isUpdated = 0;
        FillRect(BACK_COLOR,rt);


        rt.x = 10;
        rt.y = 30;
        rt.w = 20;
        rt.h = 20;
        int color = rgb_black;

        for(int i=0; i< 8;i++){
        		memset(text,0,256);
                if( pSystemInfo->mySysInfo.safestatus[16+i] == 0 ){ //如果防区没有启动则不处理
                        sprintf(text,"%s %d",LT("禁用防区"),i+1);
                        color = rgb_black;
                }else{

                        if(get_security_status(i) && g_have_defend){
                                if(GetViewSafeStatus(i)){
                                        color = rgb_red;
                                        sprintf(text,"%d%s",i+1,LT("防区报警"));
                                }else{
                                        color = rgb_yellow;
                                        sprintf(text,"%d %s",i+1,LT("防区布防"));
                                }
                        }else{
                                if(i > 1){
                                        color = rgb_green;
                                        sprintf(text,"%d %s",i+1,LT("防区未布防"));
                                }else{
                                        if(GetViewSafeStatus(i)){
                                                color = rgb_red;
                                                sprintf(text,"%d%s",i+1,LT("防区报警"));
                                        }else{
                                                color = rgb_yellow;
                                                sprintf(text,"%s %d",LT("防区布防"),i+1);
                                        }
                                }
                        }
                }
                rt.y += offset;
                DrawText(text,50,rt.y+rt.h,color);
                tDraw3dOutset(rt.x,rt.y,rt.w,rt.h);
                WRT_Rect tmp = rt;
                DeflateRect(&tmp,1);
                FillRect(color,tmp);
        }
		if(isupdate == 1){
		     update_rect(0,0,800,425);
		     g_isUpdated = 1;
		}
}

void CreateViewSafeStatusWin(){
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
	        return ;
	pWindow->EventHandler = ViewSafeStatusHandler;
	pWindow->NotifyReDraw = DrawSafeStatusMain;
	pWindow->valid = 1;
	pWindow->type = SAFEWIN;
	WRT_DEBUG("CreateViewSafeStatusWin pWindow = %x",pWindow);
	//WindowAddToWindowList(pWindow);
	if(WindowInsertToWindowList(pWindow) == 1)
	        pWindow->NotifyReDraw2(pWindow);
}
void DrawSafeStatusMain(){
        
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

	ViewSafeSatus(0);
	tDraw3dOutset(rcsafe3[1].x,rcsafe3[1].y,rcsafe3[1].w,rcsafe3[1].h);
	Color_FillRect(rcsafe3[1],0,0,255);
	DrawTextToRect(rcsafe3[1],LT("撤  防"));
	JpegDisp(rcsafe3[2].x,rcsafe3[2].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rcsafe3[3].x,rcsafe3[3].y,bt_guanping1,sizeof(bt_guanping1));
}
int  ViewSafeStatusHandler(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rcsafe3[index],xy)){
                        pos_x = rcsafe3[index].x;
                        pos_y = rcsafe3[index].y;
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
                  case 0://
                          break;
                  case 1://
                          if(status == 1){
                                  StartButtonVocie();
				  				  g_isUpdated = 0;
                                  tDraw3dInset(rcsafe3[1].x,rcsafe3[1].y,rcsafe3[1].w,rcsafe3[1].h);
                                  Color_FillRect(rcsafe3[1],255,0,0);
                                  DrawTextToRect(rcsafe3[1],LT("撤  防"));
								  update_rect(rcsafe3[1].x,rcsafe3[1].y,rcsafe3[1].w,rcsafe3[1].h);
								  g_isUpdated = 1;
                          }else{
                          	CreatePasswordWin(SET_CHEFANG_PWD_WIN);
                          	/*
                                  if(g_cur_set_scene_mode == SCENE_ZAIJIA){
                                          //CloseWindow();
                                          CreatePasswordWin(SET_CHEFANG_PWD_WIN);
                                  }else{
                                          //CloseWindow();
                                          CreatePasswordWin(SET_CHEFANG_STATUS);
                                  }
                                 */
                          }
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
                  default:
                          break;
                        }
                        break;
                }
        }
        return ret;
}
/////////////////////////////////////////////////////////////////////
//
//远程控制及自拍开关设置窗口
////////////////////////////////////////////////////////////////////
static WRT_Rect rcRemote[4]={
        {500,30,48,48},
        {500,80,48,48},
        {900,531,111,69},              //返回
        {6,531,111,69}                 //关屏幕
};

void CreateRemoteControl(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = RemoteControlHandle;
        pWindow->NotifyReDraw = DrawRemoteControlMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}
void DrawRemoteControlMain(){
		WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);

		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
 
        DrawText(LT("是否启用远程控制防区和家居"),30,rcRemote[0].y+rcRemote[0].h-20,rgb_white);
        DrawText(LT("是否启用自动抓拍"),30,rcRemote[1].y+rcRemote[1].h-20,rgb_white);
        if(pSystemInfo->mySysInfo.isremote == 1)
                JpegDisp(rcRemote[0].x,rcRemote[0].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
                JpegDisp(rcRemote[0].x,rcRemote[0].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));

        if(pSystemInfo->mySysInfo.isautocap == 1)
                JpegDisp(rcRemote[1].x,rcRemote[1].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
                JpegDisp(rcRemote[1].x,rcRemote[1].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));

        JpegDisp(rcRemote[2].x,rcRemote[2].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rcRemote[3].x,rcRemote[3].y,bt_guanping1,sizeof(bt_guanping1));
}

int    RemoteControlHandle(int x,int y,int status){
        WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<4;index++){
                if(IsInside(rcRemote[index],xy)){
                        pos_x = rcRemote[index].x;
                        pos_y = rcRemote[index].y;
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
                  case 0://
                          if(status == 1)
                                  StartButtonVocie();
                          else{
                                  pSystemInfo->mySysInfo.isremote = !pSystemInfo->mySysInfo.isremote;
                                  if(pSystemInfo->mySysInfo.isremote == 1)
                                          JpegDisp(rcRemote[0].x,rcRemote[0].y,ico_fangqukai,sizeof(ico_fangqukai));
                                  else
                                          JpegDisp(rcRemote[0].x,rcRemote[0].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));

                                  FlushSystemInfo();
                          }
                          break;
                  case 1://
                          if(status == 1)
                                  StartButtonVocie();
                          else{
                                  pSystemInfo->mySysInfo.isautocap = !pSystemInfo->mySysInfo.isautocap;
                                  if(pSystemInfo->mySysInfo.isautocap == 1)
                                          JpegDisp(rcRemote[1].x,rcRemote[1].y,ico_fangqukai,sizeof(ico_fangqukai));
                                  else
                                          JpegDisp(rcRemote[1].x,rcRemote[1].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
                                  FlushSystemInfo();
                          }
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
                  default:
                          break;
                        }
                        break;
                }
        }
        return ret;
}


