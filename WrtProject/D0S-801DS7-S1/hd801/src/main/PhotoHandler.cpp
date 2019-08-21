#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h> 
#include <pthread.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <sys/wait.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <errno.h>
#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"
#include "window.h"
#include "c_interface.h"

//extern  WRT_Rect rc_infobrowsermenuico[17];
extern unsigned char sd_beifen_1[21651];
extern unsigned char sd_beifen_2[21437];
extern unsigned char usb_beifen_1[20894];
extern unsigned char usb_beifen_2[20993];
static char photodir[]="/home/wrt/pic/picture";
static char videodir[]="/home/wrt/pic/video";

static WRT_Rect rc_infobrowsermenuico[19]=
{   
        {32,50,664,40},
        {32,90,664,40},
        {32,130,664,40},
        {32,170,664,40},
        {32,210,664,40},
        {32,250,664,40},
        {32,290,664,40},
        {32,330,664,40},
        {32,370,664,40},   

        {900,40,104,76},           /*清空*/ 
        {900,120,104,76},            /*锁定*/  
        {900,200,104,76},            /*删除*/
        {900,280,104,76},            /*打开*/         
        {660,531,111,69},           /*上翻*/
        {780,531,111,69},            /* 下翻*/

        {900,531,111,69},              //返回
        {6,531,111,69},                 //关屏幕		
        {900,360,104,76},           /*U盘*/
        {900,440,104,76},            /*SD卡*/
}; 

static int m_photo_page = 1;
static int m_cur_photo_page = 1;
static int m_cur_photo_select = -1;
static int m_pre_photo_select = -1;

static int m_video_page = 1;
static int m_cur_video_page = 1;
static int m_cur_video_select = -1;
static int m_pre_video_select = -1;

static int m_usb_flag = 0;
static int m_sd_flag = 0;

static bool g_photo_lock_status = false;
int g_hasplay = 0;

#ifdef WRT_MORE_ROOM
static bool g_more_room_read_photo_end = false;
#endif

/*
static  const WRT_Rect rc_infobrowsermenuico[16]=
{
{32,50,695,86},
{32,90,695,126},
{32,130,695,166},
{32,170,695,206},
{32,210,695,246},
{32,250,695,286},
{32,290,695,326},
{32,330,695,366},
{32,370,695,406},

{39,426,159,474},
{448,426,560,474},
{576,426,688,474},
{741,230,800,287},

{741,312,798,392},
{741,397,800,480},
{741, 40,800,120}
};*/
extern void mDirIsExist(char *dirname);

static void extern_device_check()
{
	DIR *dirp;
	struct dirent * ptr;
	int count = 0;
	dirp = opendir("/mnt/sda");
	if(dirp != NULL)
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count >2) // . .. 2个文件夹
		{
			m_usb_flag=1;
			
		}
		
	}
	if(dirp != NULL)
		closedir(dirp);
	ptr = NULL;
	dirp = NULL;

	
	count = 0;
	dirp = opendir("/mnt/mmc");
	if(dirp != NULL)
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count >2) // . .. 2个文件夹
		{
			m_sd_flag=1;
			
		}
	}
	if(dirp != NULL)
		closedir(dirp);
	ptr = NULL;
	dirp = NULL;
}

static int Copy_PicDataToUDisk(int param)
{
	int count = -1;
	char cpbuf[256];
	count  = (m_cur_photo_page-1)*9+m_cur_photo_select;
	if(count < 0)
		return -1;
	PHOTOINFO* curinfo = get_select_photo(count);
	mDirIsExist("/mnt/sda/picture");
	sprintf(cpbuf,"cp %s/%s /mnt/sda/picture",photodir,curinfo->name);

	system(cpbuf);
	system("sync");
}

static int Copy_PicDataToSDisk(int param)
{
	int count = -1;
	char cpbuf[256];
	count  = (m_cur_photo_page-1)*9+m_cur_photo_select;
	if(count < 0)
		return -1;
	PHOTOINFO* curinfo = get_select_photo(count);
	mDirIsExist("/mnt/mmc/picture");
	sprintf(cpbuf,"cp %s/%s /mnt/mmc/picture",photodir,curinfo->name);

	system(cpbuf);
	system("sync");
}

static int Copy_VidDataToUDisk(int param)
{
	int count = -1;
	char cpbuf[256];
	count  = (m_cur_video_page-1)*9+m_cur_video_select;
	if(count < 0)
		return -1;
	PHOTOINFO* curinfo = get_select_video(count);
	mDirIsExist("/mnt/sda/video");
	sprintf(cpbuf,"cp %s/%s /mnt/sda/video",videodir,curinfo->name);

	system(cpbuf);
	system("sync");
}

static int Copy_VidDataToSDisk(int param)
{
	int count = -1;
	char cpbuf[256];
	count  = (m_cur_video_page-1)*9+m_cur_video_select;
	if(count < 0)
		return -1;
	PHOTOINFO* curinfo = get_select_video(count);
	mDirIsExist("/mnt/mmc/video");
	sprintf(cpbuf,"cp %s/%s /mnt/mmc/video",videodir,curinfo->name);

	system(cpbuf);
	system("sync");
}

static void DrawPhotoVodCurSelect(int selected){


#ifdef HAVE_PLAY_JPEG

        if(g_hasplay == 0){
                unsigned long event[4];
                event[0] =CUSTOM_CMD_PLAY_JPEG_FRAME;
                event[1] = selected;
                event[2] = event[3] = 0;
                wrthost_send_cmd(event);
                g_hasplay =  1;

        }

#endif

}


static void DrawPhotoCurSelect(int param)
{
        int count = 0;
        long size = 0;
        PHOTOINFO* pTemp = 0;
        WRT_Rect curbox;
        char  tmpname[256];
        char  tmpname1[256];
        int str_len = 0;
        int xoffset = 0;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 889;
        curbox.h = 530;
        if(m_cur_photo_select == -1){
                CloseTopWindow();
                return;
        }
        count = (m_cur_photo_page-1)*9+m_cur_photo_select;
        pTemp = get_select_photo(count);
#ifdef WRT_MORE_ROOM
        WRT_DEBUG("param = %d,pTemp->idle = %d,pTemp = %x,read = %d",param,pTemp->idle,pTemp,pTemp->read & 0xffff);
        if((param  == 1) && pTemp && pTemp->idle == 1 && ((pTemp->read & 0xffff) == 1))
        {
        	FillRect(0xb2136,curbox); 
                DrawText(LT("没有图片..."),80,77,rgb_white);
                g_more_room_read_photo_end = true;
                return;
        }
#endif
        if(pTemp && pTemp->idle == 1){

                //
                unsigned char* paddr = get_photo_addr((char*)pTemp->name,&size);
                //pTemp->read = 1;
                set_photo_read(pTemp,1);
                if(size > 0){

        				FillRect(0xb2136,curbox); 

                        SDL_Delay(20);
                        
                        if(strstr((char*)pTemp->name,"vod") != NULL)
                                DrawPhotoVodCurSelect(count);
                        else
                                JpegDispZoom(0,0,paddr,size,0.6,0.8);

                        memset(tmpname,0,sizeof(tmpname));
                        int addr = (pTemp->read & 0xffff0000 ) >> 16;
                        if(addr  == 0){
                                sprintf(tmpname,"%s_%s",LT("门口机"),(char*)pTemp->name);
                        }else if(addr == 1){
                                sprintf(tmpname,"%s_%s",LT("小门口机"),(char*)pTemp->name);
                        }else if(addr == 2){
                                sprintf(tmpname,"%s_%s",LT("围墙机"),(char*)pTemp->name);
                        }else if(addr == 4){
                        		sprintf(tmpname,"%s_%s",LT("数字小门口机"),(char*)(pTemp->name));    
                        }else{
                                sprintf(tmpname,"%s",(char*)pTemp->name);
                        }
                        str_len = GetTextWidth(tmpname,1);
                        xoffset = (888 - str_len)/2;
                        if(xoffset < 0){
                                str_len =  strlen(tmpname);
                                int first_text_len = str_len;
                                while(first_text_len > 0){
                                        if((first_text_len >= 2)&& tmpname[first_text_len-1] >=0xA0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(tmpname1,0,256);
                                        memcpy(tmpname1,tmpname,first_text_len);
                                        tmpname1[first_text_len+1]='\0';
                                        str_len = GetTextWidth(tmpname1,0);
                                        if( (str_len) < 300){ //430
                                                break;
                                        }
                                }  
                                xoffset = (888 - str_len)/2;
                                DrawText_16((char*)tmpname1,xoffset,380,rgb_white);
                                DrawText_16((char*)(tmpname+first_text_len),xoffset,410,rgb_white);
                        }else
                                DrawText_16((char*)(tmpname),xoffset,450,rgb_white);
                        if(pTemp->lock){
                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));
                                g_photo_lock_status = true;
                                JpegDisp(40,50,ico_lock,sizeof(ico_lock));
                        }else{
                                JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
                                g_photo_lock_status = false;
                                curbox.x = 40;
                                curbox.y = 50;
                                curbox.w = 28;
                                curbox.h = 28;
                                FillRect(BACK_COLOR,curbox);
                        }
                }
        }else{

        		FillRect(0xb2136,curbox); 
        		//tm_wkafter(100);
                DrawText(LT("没有图片..."),80,77,rgb_white);

        }
        DrawStatusbar(m_photo_page,m_cur_photo_page);
        WRT_DEBUG(" DrawPhotoCurSelect ");
}
static void DrawPhotoInfo(){
        WRT_xy xy;
        WRT_Rect curbox;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 889;
        curbox.h = 530;
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
        if(m_cur_photo_page !=1 && m_cur_photo_page>0)
                JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

        if(m_photo_page > 1 && m_photo_page != m_cur_photo_page )
                JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));
        if(get_photo_count() == 0){

        		FillRect(0xb2136,curbox); 
 
                DrawText(LT("没有图片..."),80,77,rgb_white);
                return;
        }
        if(m_cur_photo_select == -1){

       	 		FillRect(0xb2136,curbox); 
  
                SDL_Delay(20);
        }
        PHOTOINFO* pTemp = 0;
        char tmpname[256] ;
        char tmpname1[256];
        int i;
        int count = (m_cur_photo_page-1)*9;
        i = 0;
        int offset = 40;
        for(i = 0;i<9;i++){
                pTemp = get_select_photo(count+i);
                if(pTemp && pTemp->idle == 1){

                        int addr = (pTemp->read & 0xffff0000 ) >> 16;
                        memset(tmpname,0,sizeof(tmpname));
                        if(addr  == 0){
                                sprintf(tmpname,"%s_%s",LT("门口机"),(char*)pTemp->name);
                        }else if(addr == 1){
                                sprintf(tmpname,"%s_%s",LT("小门口机"),(char*)pTemp->name);
                        }else if(addr == 2){
                                sprintf(tmpname,"%s_%s",LT("围墙机"),(char*)pTemp->name);
                        }else if(addr == 4){
                        	sprintf(tmpname,"%s_%s",LT("数字小门口机"),(char*)(pTemp->name));    
                        }else{
                                sprintf(tmpname,"%s",(char*)pTemp->name);
                        }
                        int str_len = GetTextWidth(tmpname,0);
                        if(str_len > 540)
                        {
                                int first_text_len = strlen(tmpname);

                                while(first_text_len > 0){
                                        if((first_text_len >= 2)&& tmpname[first_text_len-1] >=0xA0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(tmpname1,0,256);
                                        memcpy(tmpname1,tmpname,first_text_len);
                                        tmpname1[first_text_len+1]='\0';
                                        str_len = GetTextWidth(tmpname1,0);
                                        if( (str_len) < 480){ //430
                                                break;
                                        }
                                }
                                memset(tmpname,0,256);
                                strcpy(tmpname,tmpname1);
                                strcat(tmpname,"...");
                        }
                        if(m_cur_photo_select == i){
                                DrawText((char*)tmpname,130,50+i*offset+23,rgb_yellow);
                                JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2));
                                if(pTemp->lock){

                                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));
                                        g_photo_lock_status = true;
                                }else{

                                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
                                        g_photo_lock_status = false;
                                }
                        }else{
                                DrawText((char*)tmpname,130,50+i*offset+23,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                        }
                        if(m_pre_photo_select == i && m_pre_photo_select != m_cur_photo_select){
                                DrawText((char*)tmpname,130,50+i*offset+23,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                        }
                        if((pTemp->read & 0xffff) == 1)
                                JpegDisp(100,50+i*offset,ico_tupian2,sizeof(ico_tupian2));
                        else
                                JpegDisp(100,50+i*offset,ico_tupian1,sizeof(ico_tupian1));

                        if(pTemp->lock){
                                JpegDisp(40,50+i*offset,ico_lock,sizeof(ico_lock));
                        }
                        else{
                                curbox.x = 40;
                                curbox.y = 50+i*offset;
                                curbox.w = 28;
                                curbox.h = 28;
                                FillRect(BACK_COLOR,curbox);

                        }
                }else
                        break;
        }
        DrawStatusbar(m_photo_page,m_cur_photo_page);


}

void CreatePhotoWin(){
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = PhotoEventHandler;
	pWindow->NotifyReDraw = DrawPhotoMain;
	pWindow->valid = 1;
	pWindow->type = CHILD;
	WindowAddToWindowList(pWindow);
	int size = get_photo_count();
	m_photo_page =size /9;
	
	if(size%9)
		m_photo_page +=1;
	if(m_photo_page == 0)
		m_photo_page = 1;
		
	m_cur_photo_page = 1;
	m_cur_photo_select = -1;
	m_pre_photo_select = -1;
	m_usb_flag = 0;
	m_sd_flag = 0;
	pWindow->NotifyReDraw2(pWindow);
}

void DrawPhotoMain(){
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	extern_device_check();

	JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
	JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_dakai1,sizeof(bt_dakai1));

	if(m_usb_flag ==1)
	{
		JpegDisp(rc_infobrowsermenuico[17].x,rc_infobrowsermenuico[17].y,usb_beifen_1,sizeof(usb_beifen_1)); 
	}
	if(m_sd_flag == 1)
	{
		JpegDisp(rc_infobrowsermenuico[18].x,rc_infobrowsermenuico[18].y,sd_beifen_1,sizeof(sd_beifen_1)); 
	}

	g_photo_lock_status = false;

	DrawPhotoInfo();
}

int  PhotoEventHandler(int x,int y,int status){
        int res =0;
        WRT_xy xy;
        int index =0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        for(index=0;index<19;index++){
                if(IsInside(rc_infobrowsermenuico[index],xy)){
                        pos_x = rc_infobrowsermenuico[index].x;
                        pos_y = rc_infobrowsermenuico[index].y;
                        res  =1;
                        switch(index){
                case 12:/*阅读*/

                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_dakai2,sizeof(bt_dakai2));
                        }else{

                                        JpegDisp(pos_x,pos_y,bt_dakai1,sizeof(bt_dakai1));
                                CreatePhotoSubWin();
                        }

                        break;
                case 13:/*上翻*/
                        if(m_cur_photo_page == 1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                m_cur_photo_page--;
                                if(m_cur_photo_page < 1)
                                        m_cur_photo_page  = 1;
                                m_cur_photo_select = -1;
                                m_pre_photo_select = -1;
                                g_isUpdated = 0;
                                DrawPhotoInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 14:/*下翻*/
                        if(m_photo_page == m_cur_photo_page)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
                                m_cur_photo_page++;
                                if(m_cur_photo_page > m_photo_page)
                                        m_cur_photo_page  = m_photo_page;
                                m_cur_photo_select = -1;
                                m_pre_photo_select =  -1;
                                g_isUpdated = 0;
                                DrawPhotoInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);         
                        }
                        break;
                case 9:/*清空*/
                        if(get_photo_count() == 0)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));
                                CreateDialogWin2(LT("你确定要清空所有图片"),MB_OK|MB_CANCEL,clear_photo,NULL);
                        }
                        break;
                case 10:/*删除*/
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew));
                                PHOTOINFO* curinfo = get_select_photo((m_cur_photo_page-1)*9+m_cur_photo_select);
                                if(curinfo->lock)
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);
                                else
                                        CreateDialogWin2(LT("你确定要删除该图片"),MB_OK|MB_CANCEL,del_cur_photo,NULL);
                        }
                        break;
                case 11:/*锁定*/
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                if(g_photo_lock_status){

                                		JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                }else{

                                		JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                                }
                        }else{
                                if(g_photo_lock_status){

                                        JpegDisp(pos_x,pos_y,bt_jiesuo1,sizeof(bt_jiesuo1));
                                        lock_cur_photo(false);
                                }else{

                                        JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));
                                        lock_cur_photo(true);
                                }
                                g_isUpdated = 0;
                                DrawPhotoInfo();//重写锁显示的那一块
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);         
                        }
                        break;
                case 17:/*拷贝到U盘*/
                		if(m_usb_flag == 0)
                			break;
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,usb_beifen_2,sizeof(usb_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,usb_beifen_1,sizeof(usb_beifen_1));
							CreateDialogWin2(LT("拷贝数据到U盘..."),MB_OK|MB_CANCEL,Copy_PicDataToUDisk,NULL);
						}
                        break;
                case 18:/*拷贝到sd卡*/
                		if(m_sd_flag == 0)
                			break;
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,sd_beifen_2,sizeof(sd_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,sd_beifen_1,sizeof(sd_beifen_1));
							CreateDialogWin2(LT("拷贝数据到SD卡..."),MB_OK|MB_CANCEL,Copy_PicDataToSDisk,NULL);
						}
                        break;
                case 15: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                m_cur_photo_select = -1;
                                m_pre_photo_select = -1;
                                CloseTopWindow();
                                save_photo();
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
                        if(get_photo_count() == 0)
                                break;
                        if((index+(m_cur_photo_page-1)*9) < get_photo_count()){
                                if(status == 0){
                                        m_pre_photo_select = m_cur_photo_select;
                                        m_cur_photo_select = index;
                                        if(m_pre_photo_select != m_cur_photo_select){
                                        		g_isUpdated =  0;
                                                DrawPhotoInfo();
                                                g_isUpdated = 1;
                                				update_rect(0,0,1024,600);       
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
#ifdef WRT_MORE_ROOM
void more_room_set_read_photo_pos()
{
        m_cur_photo_page = 1;
        m_cur_photo_select = 0;
        g_more_room_read_photo_end = false;
}
int  more_room_read_next_photo()
{
        if(g_more_room_read_photo_end == true){

                if(IsNewPhoto() == 0)
                {
                        close_red_led();
                }
                save_photo();
                return 0;
        }
        m_cur_photo_select++;
        if((m_cur_photo_select +(m_cur_photo_page-1)*9) > get_photo_count()){
                if(IsNewPhoto() == 0)
                {
                        close_red_led();
                }
                save_photo();
                return 0;
        }
        WRT_DEBUG("DrawPhotoCurSelect");
        DrawPhotoCurSelect(1);
        return 1;
}
#endif

void CreatePhotoSubWin(){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        g_hasplay = 0;
        pWindow->EventHandler = PhotoSubEventHandler;
        pWindow->NotifyReDraw = DrawPhotoSubMain;
        pWindow->valid = 1;
#ifdef WRT_MORE_ROOM
        pWindow->type = PHOTO_READ;
#else
        pWindow->type = CHILD;
#endif
        WindowAddToWindowList(pWindow);
        pWindow->NotifyReDraw2(pWindow);

}
void DrawPhotoSubMain(){
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 

	JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));


	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
	//JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_dakai1,sizeof(bt_dakai1));

	extern_device_check();
	if(m_usb_flag ==1)
	{
		JpegDisp(rc_infobrowsermenuico[17].x,rc_infobrowsermenuico[17].y,usb_beifen_1,sizeof(usb_beifen_1)); 
	}
	if(m_sd_flag == 1)
	{
		JpegDisp(rc_infobrowsermenuico[18].x,rc_infobrowsermenuico[18].y,sd_beifen_1,sizeof(sd_beifen_1)); 
	}

	if(get_photo_count() >  1)
	    JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

	if(get_photo_count() > 1)
	    JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));

	JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));
	g_photo_lock_status  = false;
	DrawPhotoCurSelect(0);
	WRT_DEBUG("DrawPhotoSubMain ");

}

int  PhotoSubEventHandler(int x,int y,int status){
        int ret=0;
        WRT_xy xy;
        int index =0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        for(index=0;index<19;index++){
                if(IsInside(rc_infobrowsermenuico[index],xy)){
                        pos_x = rc_infobrowsermenuico[index].x;
                        pos_y = rc_infobrowsermenuico[index].y;
                        ret = 1;
                        switch(index){
                case 12:
                	break;
                case 13:/*上翻*/
                        if(get_photo_count() < 2)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);
                                }
#endif
                                m_cur_photo_select --;
                                if(m_cur_photo_select < 0)
                                        m_cur_photo_select = 0;
                                
                                g_isUpdated =0;   
                                DrawPhotoCurSelect(0);
								g_isUpdated = 1;
                                update_rect(0,0,1024,600);                                         
                        }
                        break;
                case 14:/*下翻*/
                        if(get_photo_count() < 2)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);



                                }
#endif
                                m_cur_photo_select++;
                                if((m_cur_photo_select+(m_cur_photo_page-1)*9) > get_photo_count())
                                        m_cur_photo_select-= 1;
                                g_isUpdated = 0;
                                DrawPhotoCurSelect(0);
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);        
                        }
                        break;
                case 9:/*清空*/
                        if(get_photo_count() ==0)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{
 
                                        JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);


                                }
#endif
                                CreateDialogWin2(LT("你确定要清空所有图片"),MB_OK|MB_CANCEL,clear_photo,NULL);
                        }
                        break;
                case 10:/*删除*/
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                        JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                        }else{
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);

                                }
#endif

                                        JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew));
                                PHOTOINFO* curinfo = get_select_photo((m_cur_photo_page-1)*9+m_cur_photo_select);
                                if(curinfo->lock)
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);
                                else
                                        CreateDialogWin2(LT("你确定要删除该图片"),MB_OK|MB_CANCEL,del_cur_photo,NULL);
                        }
                        break;
                case 11:/*锁定*/
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                if(g_photo_lock_status){

                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                }else{

                                                JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                                }
                        }else{
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);



                                }
#endif

                                if(g_photo_lock_status){

                                                JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                        lock_cur_photo(false);
                                }else{

                                                JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));
                                        lock_cur_photo(true);
                                }
                                g_isUpdated = 0;
                                DrawPhotoCurSelect(0);
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);       
                        }
                        break;
                case 17:/*拷贝到U盘*/
                		if(m_usb_flag == 0)
                			break;
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,usb_beifen_2,sizeof(usb_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,usb_beifen_1,sizeof(usb_beifen_1));
							CreateDialogWin2(LT("拷贝数据到U盘..."),MB_OK|MB_CANCEL,Copy_PicDataToUDisk,NULL);
						}
                        break;
                case 18:/*拷贝到sd卡*/
                		if(m_sd_flag == 0)
                			break;
                        if(get_photo_count() == 0)
                                break;
                        if(m_cur_photo_select == -1)
                                break;
                        if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,sd_beifen_2,sizeof(sd_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,sd_beifen_1,sizeof(sd_beifen_1));
							CreateDialogWin2(LT("拷贝数据到SD卡..."),MB_OK|MB_CANCEL,Copy_PicDataToSDisk,NULL);
						}
                        break;
                case 15: /*返回*/

                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);

                                }
#endif
                                m_cur_photo_select = -1;
                                m_pre_photo_select = -1;
                                CloseTopWindow();
                                save_photo();
                        }

                        break;
                case 16:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
#ifdef HAVE_PLAY_JPEG
                                {
                                        unsigned long event[4];
                                        g_hasplay = 0;
                                        event[0] =CUSTOM_CMD_STOP_JPEG_FRAME;
                                        event[1] = event[2] = event[3] = 0;
                                        wrthost_send_cmd(event);

                                }
#endif

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

int del_cur_photo(int param){
        PHOTOINFO* curinfo = get_select_photo((m_cur_photo_page-1)*9+m_cur_photo_select);
        if(curinfo == NULL)
                return 0;
        if(curinfo->lock)
                return 0;
        del_photo(curinfo);
        int size = get_photo_count();
        m_photo_page =size /9;
        if(size%9)
                m_photo_page +=1;
        if(m_photo_page == 0)
                m_photo_page = 1;
        if(m_cur_photo_page > m_photo_page)
                m_cur_photo_page = m_photo_page;
        m_cur_photo_select = -1;
        m_pre_photo_select = -1;
	return 0;                
}
int clear_photo(int param){
        m_cur_photo_select = -1;
        m_pre_photo_select = -1;
        del_all_photo();
        m_cur_photo_page = 1;
        m_photo_page = 1;
        return 0;
}
void lock_cur_photo(bool islock){
        PHOTOINFO* curinfo = get_select_photo((m_cur_photo_page-1)*9+m_cur_photo_select);
        if(curinfo == NULL)
                return;
        if(islock)
                lock_photo(curinfo,1);
        else
                lock_photo(curinfo,0);
        //   m_cur_photo_select = -1;
        //   m_pre_photo_select = -1;
        // ReDrawCurrentWindow();
}



//////////////////////////////////


static bool g_video_lock_status = false;
static int m_isexit = 0;

pthread_t video_thread = -1;
int g_isrecplay_kit = 0;
static void DrawVideoInfo(){
        WRT_xy xy;
        WRT_Rect curbox;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 889;
        curbox.h = 530;
        JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
        if(m_cur_video_page !=1 && m_cur_video_page>0)
                JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,menu_shangfan1,sizeof(menu_shangfan1));

        if(m_video_page > 1 && m_video_page != m_cur_photo_page )
                JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,menu_xiafan1,sizeof(menu_xiafan1));

        JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));
        if(get_video_count() == 0){

        		FillRect(0xb2136,curbox); 
 
                DrawText(LT("没有视频"),80,77,rgb_white);
                return;
        }
        if(m_cur_video_select == -1){

       	 		FillRect(0xb2136,curbox); 
  
                SDL_Delay(20);
        }
        PHOTOINFO* pTemp = 0;
        char tmpname[256] ;
        char tmpname1[256];
        int i;
        int count = (m_cur_video_page-1)*9;
        i = 0;
        int offset = 40;
        for(i = 0;i<9;i++){
                pTemp = get_select_video(count+i);
                if(pTemp && pTemp->idle == 1){

                        int addr = (pTemp->read & 0xffff0000 ) >> 16;
                        memset(tmpname,0,sizeof(tmpname));
                        if(addr  == 0){
                                sprintf(tmpname,"%s_%s",LT("门口机"),(char*)pTemp->name);
                        }else if(addr == 1){
                                sprintf(tmpname,"%s_%s",LT("小门口机"),(char*)pTemp->name);
                        }else if(addr == 2){
                                sprintf(tmpname,"%s_%s",LT("围墙机"),(char*)pTemp->name);
                        }else if(addr == 3){
                                sprintf(tmpname,"%s_%s",LT("住户"),(char*)pTemp->name);
                        }else if(addr == 4){
                        		sprintf(tmpname,"%s_%s",LT("数字小门口机"),(char*)(pTemp->name));    
                        }else{
                                sprintf(tmpname,"%s",(char*)pTemp->name);
                        }
                        int str_len = GetTextWidth(tmpname,0);
                        if(str_len > 540)
                        {
                                int first_text_len = strlen(tmpname);

                                while(first_text_len > 0){
                                        if((first_text_len >= 2)&& tmpname[first_text_len-1] >=0xA0)
                                                first_text_len -=2;
                                        else
                                                first_text_len --;
                                        memset(tmpname1,0,256);
                                        memcpy(tmpname1,tmpname,first_text_len);
                                        tmpname1[first_text_len+1]='\0';
                                        str_len = GetTextWidth(tmpname1,0);
                                        if( (str_len) < 480){ //430
                                                break;
                                        }
                                }
                                memset(tmpname,0,256);
                                strcpy(tmpname,tmpname1);
                                strcat(tmpname,"...");
                        }
                        if(m_cur_video_select == i){
                                DrawText((char*)tmpname,140,50+i*offset+23,rgb_yellow);
                                JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2));
                                if(pTemp->lock){
                                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_jiesuo1,sizeof(bt_jiesuo1));
                                        g_video_lock_status = true;
                                }else{
                                        JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
                                        g_video_lock_status = false;
                                }
                        }else{
                                DrawText((char*)tmpname,140,50+i*offset+23,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                        }
                        if(m_pre_video_select == i && m_pre_video_select != m_cur_video_select){
                                DrawText((char*)tmpname,140,50+i*offset+23,rgb_white);
                                JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
                        }
                        if((pTemp->read & 0xffff) == 1)
                                JpegDisp(100,50+i*offset,ico_shipin2,sizeof(ico_shipin2));
                        else
                                JpegDisp(100,50+i*offset,ico_shipin1,sizeof(ico_shipin1));

                        if(pTemp->lock){
                                JpegDisp(40,50+i*offset,ico_lock,sizeof(ico_lock));
                        }
                        else{
                                curbox.x = 40;
                                curbox.y = 50+i*offset;
                                curbox.w = 28;
                                curbox.h = 28;
                                FillRect(BACK_COLOR,curbox);
                        }
                }else
                        break;
        }
        DrawStatusbar(m_video_page,m_cur_video_page);


}

void CreateVideoWin(){
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = VideoEventHandler;
	pWindow->NotifyReDraw = DrawVideoMain;
    pWindow->valid = 1;
    pWindow->type = CHILD;
    WindowAddToWindowList(pWindow);
	int size = get_video_count();
	m_video_page =size /9;
	if(size%9)
		m_video_page +=1;
	if(m_video_page == 0)
		m_video_page = 1;
	m_cur_video_page = 1;
	m_cur_video_select = -1;
	m_pre_video_select = -1;
	m_usb_flag = 0;
    m_sd_flag = 0;
    pWindow->NotifyReDraw2(pWindow);
}
void DrawVideoMain(){
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchunew,sizeof(bt_shanchunew));
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_suoding1,sizeof(bt_suoding1));
	JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_dakai1,sizeof(bt_dakai1));

	extern_device_check();
	if(m_usb_flag ==1)
	{
		JpegDisp(rc_infobrowsermenuico[17].x,rc_infobrowsermenuico[17].y,usb_beifen_1,sizeof(usb_beifen_1)); 
	}
	if(m_sd_flag == 1)
	{
		JpegDisp(rc_infobrowsermenuico[18].x,rc_infobrowsermenuico[18].y,sd_beifen_1,sizeof(sd_beifen_1)); 
	}

	g_video_lock_status = false;
        
	DrawVideoInfo();
}
int  VideoEventHandler(int x,int y,int status){
        int res =0;
        WRT_xy xy;
        int index =0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        for(index=0;index<19;index++){
                if(IsInside(rc_infobrowsermenuico[index],xy)){
                        pos_x = rc_infobrowsermenuico[index].x;
                        pos_y = rc_infobrowsermenuico[index].y;
                        res  =1;
                        switch(index){
                case 12:/*阅读*/

                        if(get_video_count() == 0)
                                break;
                        if(m_cur_video_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_dakai2,sizeof(bt_dakai2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_dakai1,sizeof(bt_dakai1));
                                CreateVideoSubWin();
                        }

                        break;
                case 13:/*上翻*/
                        if(m_cur_video_page == 1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shangfan2,sizeof(menu_shangfan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shangfan1,sizeof(menu_shangfan1));
                                m_cur_video_page--;
                                if(m_cur_video_page < 1)
                                        m_cur_video_page  = 1;
                                m_cur_video_select = -1;
                                m_pre_video_select = -1;
                                g_isUpdated = 0;
                                DrawVideoInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);      
                        }
                        break;
                case 14:/*下翻*/
                        if(m_video_page == m_cur_video_page)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_xiafan2,sizeof(menu_xiafan2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_xiafan1,sizeof(menu_xiafan1));
                                m_cur_video_page++;
                                if(m_cur_video_page > m_video_page)
                                        m_cur_video_page  = m_video_page;
                                m_cur_video_select = -1;
                                m_pre_video_select =  -1;
                                g_isUpdated = 0;
                                DrawVideoInfo();
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);         
                        }
                        break;
                case 9:/*清空*/
                        if(get_video_count() == 0)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));
                                CreateDialogWin2(LT("你确定要清空所有录像"),MB_OK|MB_CANCEL,clear_video,NULL);
                        }
                        break;
                case 10:/*删除*/
                        if(get_video_count() == 0)
                                break;
                        if(m_cur_video_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();

                                JpegDisp(pos_x,pos_y,bt_shanchunew2,sizeof(bt_shanchunew2));
                        }else{

                                JpegDisp(pos_x,pos_y,bt_shanchunew,sizeof(bt_shanchunew));
                                PHOTOINFO* curinfo = get_select_video((m_cur_video_page-1)*9+m_cur_video_select);
                                if(curinfo->lock)
                                        CreateDialogWin2(LT("锁定状态,不能删除"),MB_OK,NULL,NULL);
                                else
                                        CreateDialogWin2(LT("你确定要删除该录像"),MB_OK|MB_CANCEL,del_cur_video,NULL);
                        }
                        break;
                case 11:/*锁定*/
                        if(get_video_count() == 0)
                                break;
                        if(m_cur_video_select == -1)
                                break;
                        if(status == 1){
                                StartButtonVocie();
                                if(g_video_lock_status){

                                		JpegDisp(pos_x,pos_y,bt_jiesuo2,sizeof(bt_jiesuo2));
                                }else{
                                		JpegDisp(pos_x,pos_y,bt_suoding2,sizeof(bt_suoding2));
                                }
                        }else{
                                if(g_video_lock_status){

                                        JpegDisp(pos_x,pos_y,bt_jiesuo1,sizeof(bt_jiesuo1));
                                        lock_cur_video(false);
                                }else{

                                        JpegDisp(pos_x,pos_y,bt_suoding1,sizeof(bt_suoding1));
                                        lock_cur_video(true);
                                }
                                g_isUpdated = 0;
                                DrawVideoInfo();//重写锁显示的那一块
                                g_isUpdated = 1;
                                update_rect(0,0,1024,600);         
                        }
                        break;
                case 17:/*拷贝到U盘*/
                		if(m_usb_flag == 0)
                			break;
                        if(get_video_count() == 0)
                                break;
                        if(m_cur_video_select == -1)
                                break;
                        if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,usb_beifen_2,sizeof(usb_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,usb_beifen_1,sizeof(usb_beifen_1));
							CreateDialogWin2(LT("拷贝数据到U盘..."),MB_OK|MB_CANCEL,Copy_VidDataToUDisk,NULL);
						}
                        break;
                case 18:/*拷贝到sd卡*/
                		if(m_sd_flag == 0)
                			break;
                        if(get_video_count() == 0)
                                break;
                        if(m_cur_video_select == -1)
                                break;
                        if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,sd_beifen_2,sizeof(sd_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,sd_beifen_1,sizeof(sd_beifen_1));
							CreateDialogWin2(LT("拷贝数据到SD卡..."),MB_OK|MB_CANCEL,Copy_VidDataToSDisk,NULL);
						}
                        break;
                case 15: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
                                m_cur_video_select = -1;
                                m_pre_video_select = -1;
                                CloseTopWindow();
                                save_video();
                        }

                        break;
                case 16:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close();
                                save_video();
                                CloseAllWindows();
                        }
                        break;
                default:
                        if(get_video_count() == 0)
                                break;
                        if((index+(m_cur_video_page-1)*9) < get_video_count()){
                                if(status == 0){
                                        m_pre_video_select = m_cur_video_select;
                                        m_cur_video_select = index;
                                        if(m_pre_video_select != m_cur_video_select){
                                        		g_isUpdated =  0;
                                                DrawVideoInfo();
                                                g_isUpdated = 1;
                                				update_rect(0,0,1024,600);       
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

static void exit_mplayer2()
{
	system("killall -9 mplayer");
	if(video_thread > 0)
	{
		if(pthread_cancel(video_thread) ==0)
			printf("video_mplayer_thread cancel success\n");
		else
			printf("video_mplayer_thread cancel fail\n");
	}
}

extern int g_playRecVideo;

static void CloseVideoSubWin()
{
	m_isexit = 1;
	g_playRecVideo = 0;
	g_isrecplay_kit = 1;

	SDL_Delay(100);
	printf("CloseVideoSubWin\n");
	//exit_mplayer2();
}

void *Mplayer_videoplay_thread_fun(void *args)
{
	char fullname[256];
	int pid = -1;
	int count = (m_cur_video_page-1)*9+m_cur_video_select;
	PHOTOINFO* pTemp = 0;
	if(count < 0)
		return NULL;
    pTemp = get_select_video(count);
	set_video_read(pTemp,1);
	if(pTemp == NULL)
		return NULL;
    sprintf(fullname,"/home/wrt/pic/video/%s",pTemp->name);
	pid = fork();
	if(pid<0)
	{
		printf("fork error!\n");
	}
	else if(pid == 0)
	{
        //SetVolume(pSystemInfo->mySysInfo.ringvolume-20);
		execl("/usr/bin/mplayer","mplayer","-slave","-quiet","-fs",fullname);
	}
	else
	{
		printf("father process\n");
	}
	wait(NULL);
	if(m_isexit == 1)
	{
	}else
	{
		CloseTopWindow();
	}
	return NULL;
}

void CreateVideoSubWin(){
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return;
	pWindow->EventHandler = VideoSubEventHandler;
	pWindow->NotifyReDraw = DrawVideoSubMain;
	pWindow->valid = 1;
	pWindow->CloseWindow = CloseVideoSubWin;
	m_isexit = 0;
		g_isrecplay_kit = 0;
	pWindow->type = PLAY_RECVIDEO;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

extern int  PlayRecvideo(void *arg);
void video_rec_autoclose()
{
	CloseTopWindow();
}
void DrawVideoSubMain(){
	int ret = 0;
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 600;
	//FillRect(rgb_black,rt);
	
		char fullname[256];
		int pid = -1;
	
		int count = (m_cur_video_page-1)*9+m_cur_video_select;
		PHOTOINFO* pTemp = 0;
		if(count < 0)
			return;
		pTemp = get_select_video(count);
		set_video_read(pTemp,1);
	
		if(pTemp == NULL)
			return;
		sprintf(fullname,"/home/wrt/pic/video/%s",pTemp->name);
		PlayRecvideo(fullname);
		/*
		pthread_attr_t attr;
	
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&video_thread, &attr,Mplayer_videoplay_thread_fun,NULL);
		if(video_thread < 0)
		{
			printf("create video mplayer thread failed\n");
		}
		pthread_attr_destroy(&attr);
		*/

}

int  VideoSubEventHandler(int x,int y,int status){
        int ret=0;
        WRT_xy xy;
        int index =0;
        xy.x = x;
        xy.y = y;
        int pos_x,pos_y;
        StartButtonVocie();
		if(status == 1)
			;
		else
		{
			CloseTopWindow();
			SDL_Delay(100);
			m_cur_video_select = -1;
	        m_pre_video_select = -1;
	        g_isUpdated = 0;
            DrawVideoInfo();
            g_isUpdated = 1;
            update_rect(0,0,1024,600);   
	        save_video();
		}
        return ret;
}

int del_cur_video(int param){
        PHOTOINFO* curinfo = get_select_video((m_cur_video_page-1)*9+m_cur_video_select);
        if(curinfo == NULL)
                return 0;
        if(curinfo->lock)
                return 0;
        del_video(curinfo);
        int size = get_video_count();
        m_video_page =size /9;
        if(size%9)
                m_video_page +=1;
        if(m_video_page == 0)
                m_video_page = 1;
        if(m_cur_video_page > m_video_page)
                m_cur_video_page = m_video_page;
        m_cur_video_select = -1;
        m_pre_video_select = -1;
	return 0;                
}
int clear_video(int param){
        m_cur_video_select = -1;
        m_pre_video_select = -1;
        del_all_video();
        m_cur_video_page = 1;
        m_video_page = 1;
        return 0;
}
void lock_cur_video(bool islock){
        PHOTOINFO* curinfo = get_select_video((m_cur_video_page-1)*9+m_cur_video_select);
        if(curinfo == NULL)
                return;
        if(islock)
                lock_video(curinfo,1);
        else
                lock_video(curinfo,0);
        //   m_cur_photo_select = -1;
        //   m_pre_photo_select = -1;
        // ReDrawCurrentWindow();
}

