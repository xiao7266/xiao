#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/if.h>
#include <linux/route.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "Handler.h"
#include "audiovideo.h"
#include "public_menu_res.h"

#include "window.h"

#include "c_interface.h"
#include "net_config_menu_res.h"
#include "warningtask.h"

//#if defined(__cplusplus)
//extern "C"
//{
//#endif  
//        unsigned long inet_addr(const char *cp);
///
//#if defined(__cplusplus)
//}
//#endif  

#define USE_LIFT_IP 1

//extern RoomSystemInfo roomtest;

#define MODIFY_IP 1
#define MODIFY_MASK 2
#define MODIFY_GATEWAY 3
#define MODIFY_CENTERIP 4
#define MODIFY_ROOMIP   5
#define MODIFY_DOORIP   6
#define MODIFY_MANAGERIP 7   //托管IP
#define MODIFY_GARAGEIP  8   //车库IP
#define MODIFY_BAKMANAGERIP 9

//extern bool g_rc_status[42];
extern int g_localid ;  //cmdhandler.cpp
extern char g_number[12];
extern char g_alphabet[26];

static unsigned long* pIPNumberPointer=NULL;
static unsigned long* pIPNumberPointer1=NULL;
static unsigned long* pIPNumberSize=NULL;
static unsigned long* pIPNumberSize1=NULL;

static unsigned long* pIPAlphabetPointer = NULL;
static unsigned long* pIPAlphabetPointer1 = NULL;
static unsigned long* pIPAlphabetSize = NULL;
static unsigned long* pIPAlphabetSize1 = NULL;

static char modifyip[16];
static char modifymask[16];
static char modifygateway[16];
static char modifycenterip[16];
static char modifyroomip[16];
static char modifydoorip[16];
static char modifymanagerip[16];
static char modifygarageip[16];
static char modifymanagerip2[16];

static int g_cur_modify_index = MODIFY_IP;
static int g_cur_input_point = 0;

static int g_cur_window_type = 0;
static int max_input_count = 3;

static const WRT_Rect rc_setipmenuico[13]={
        {237,50,200,44}, //密码框输入框所在区域 // 下一个45
        {237,100,200,44}, //密码框输入框所在区域 // 下一个45 //70
        {237,150,200,44}, //密码框输入框所在区域 // 下一个45
        {237,200,200,44}, //密码框输入框所在区域 // 下一个45
        {237,250,200,44}, //密码框输入框所在区域 // 下一个45
        {237,300,200,44}, //密码框输入框所在区域 // 下一个45
        {650,100,300,400},    //数字符号所在区域539 452


        {780,531,111,69},              //确认
        {900,531,111,69},              //返回
        {6,531,111,69},                 //关屏幕				

        {237,350,200,44}, //密码框输入框所在区域 // 下一个45
        {237,400,200,44}, //密码框输入框所在区域 // 下一个45
        {237,450,200,44} //密码框输入框所在区域 // 下一个45
};

static int modify_netconfig(){
        int isresetwaringlib =0;
        unsigned long ip  = inet_addr(modifyip);
        unsigned long ip2 = 0;
        unsigned long tmpip[4] = {0};
        get_host_ip(&ip2);
        if(ip != ip2){
                int ret = set_host_ip(ip);
                if(ret != 1){
                        return -1;
                }
        }

        ip  = inet_addr(modifymask);
        ip2 = 0;
        get_host_mask(&ip2);
        if(ip != ip2){
                int ret = set_host_mask(ip);
                if(ret != 1){
                        return -1;
                }
        }

        ip  = inet_addr(modifygateway);
        ip2 = 0;
        get_host_gateway(&ip2);
        if(ip != ip2){
                int ret = set_host_gateway(ip);
                if(ret != 1){
                        return -1;
                }
        }

        ip  = inet_addr(modifycenterip);
        WRT_DEBUG("modifycenterip = %s ,ip=%x",modifycenterip,ip);
        ip2 = 0;
        wrthost_get_centerip(&ip2);
        WRT_DEBUG("ip2 %x ",ip2);
        if(ip != ip2){
                wrthost_set_centerip(ip);
                isresetwaringlib = 1;
                wrt_set_log_server_addr(ip,60000); //重置日志发送地址。
        }	

        ip  = inet_addr(modifyroomip);
        if(pSystemInfo->mySysInfo.hassip == 0x01){
                if(pSystemInfo->mySysInfo.sipproxyip != ip)
                        pSystemInfo->mySysInfo.sipproxyip = ip;

        }    
        ip = inet_addr(modifymanagerip2);
        if( pSystemInfo->LocalSetting.publicinfo.ManagerMinorIP  != ip)
                pSystemInfo->LocalSetting.publicinfo.ManagerMinorIP  = ip;
        //tmpip[0] = ip;
        //tmpip[1] = 0;
        //tmpip[2] = 0;
        //tmpip[3] = 0;
        //wrthost_get_peer_room_ip(&ip2);
        //if(ip != ip2){
        //     wrthost_set_peer_room_ip(tmpip);
        //	}	

        ip  = inet_addr(modifydoorip);
        ip2 = 0;
        wrthost_get_doorip(&ip2);
        if(ip != ip2){
                wrthost_set_doorip(ip);
        }	

        ip  = inet_addr(modifymanagerip);
        ip2 = 0;
        wrthost_get_managerip(&ip2);
        if(ip != ip2){
                wrthost_set_managerip(ip);
                isresetwaringlib = 1;
        }
#if USE_LIFT_IP
        ip  = inet_addr(modifygarageip);
        ip2 = pSystemInfo->mySysInfo.bakliftip;
        if(ip2 != ip){
                pSystemInfo->mySysInfo.bakliftip = ip;
        }
#else
        ip  = inet_addr(modifygarageip);
        ip2 = 0;
        wrthost_get_garageip(&ip2);
        if(ip != ip2){
                wrthost_set_garageip(ip);
        }
#endif	
        save_modify();
        if(isresetwaringlib == 1){
                int i,j;
                unsigned long tmpip[4];
                tmpip[0] = pSystemInfo->LocalSetting.publicinfo.CenterMajorIP;
                tmpip[1] = pSystemInfo->LocalSetting.publicinfo.ManagerMajorIP;
                tmpip[2] = pSystemInfo->LocalSetting.publicinfo.CenterWarningIP;
                tmpip[3] = pSystemInfo->LocalSetting.publicinfo.CenterMinorIP;
                j =0;
                for(j=0;j<3;j++)
                        for(i=j+1;i<4;i++){
                                if(tmpip[j] == tmpip[i])
                                        tmpip[i] = 0;
                        }
                        WRT_DEBUG("修改报警地址%x,%x,%x,%x\n",tmpip[0],tmpip[1],tmpip[2],tmpip[3]);
                        WarningInit(2,tmpip[0],
                               tmpip[1],
                               tmpip[2],
                               tmpip[3],
                                0);
        }
	
        return 1;	
}

static int modifytime(){

        unsigned short year  = (unsigned short)atoi(modifyip);
        unsigned short month = (unsigned short)atoi(modifymask);
        unsigned short day   = (unsigned short)atoi(modifygateway);
        unsigned short hour  = (unsigned short)atoi(modifycenterip);
        unsigned short min   = (unsigned short)atoi(modifyroomip);
        unsigned short sec   = (unsigned short)atoi(modifydoorip);

        unsigned long date =0;
        unsigned long time =0;
        date = ((year&0xffff) << 16)|((month& 0xff)<<8) | (day & 0xff);
        time = ((hour&0xffff) << 16) | ((min& 0xff)<<8) |(sec & 0xff);
        return tm_set(date,time,0);	   
        
}

static int setiphandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int tmpY =  0;
        char* pTemp = 0;
        int offset = 0;
        int offset2 = 0;
        int startx = rc_setipmenuico[6].x;
        int starty = rc_setipmenuico[6].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';

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
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer1[count],pIPNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer[count],pIPNumberSize[count]);
                                        switch(g_cur_modify_index)
                                        {
                        case MODIFY_IP:
                                pTemp = (char*)modifyip;
                                offset = 32;
                                offset2 = 0;
                                break;
                        case MODIFY_MASK:
                                pTemp = (char*)modifymask;
                                offset = 32+42;
                                offset2 = 1;
                                break;
                        case MODIFY_GATEWAY:
                                pTemp = (char*)modifygateway;
                                offset = 32+88;
                                offset2 = 2;
                                break;
                        case MODIFY_CENTERIP:
                                pTemp = (char*)modifycenterip;
                                offset = 32+130;
                                offset2 = 3;
                                break;

                        case MODIFY_ROOMIP:
                                if(pSystemInfo->mySysInfo.hassip == 0x0)
                                        break;
                                pTemp = (char*)modifyroomip;
                                offset = 32+172;
                                offset2 = 4;
                                break;

                        case MODIFY_DOORIP:
                                pTemp = (char*)modifydoorip;
                                offset = 32+214;
                                offset2 = 5;
                                break;
                        case MODIFY_MANAGERIP:
                                pTemp = (char*)modifymanagerip;
                                offset = 32+256;
                                offset2 = 10;
                                break;
#if USE_LIFT_IP	    	      	        
                        case MODIFY_GARAGEIP:
                                pTemp = (char*)modifygarageip;
                                offset = 32+298;    
                                offset2 = 11;
                                break;
#endif	    	      	        
                        case MODIFY_BAKMANAGERIP:
                                pTemp = (char*)modifymanagerip2;
                                offset = 32+340;    
                                offset2 = 12;
                                break;
                        default:
                                return ret;
                               }
                                        int tmplen = strlen(pTemp);

                                        if(count != 9 && count != 11){  
                                                if( tmplen > 15)     
                                                        break;                   	 	 
                                                c[0] = g_number[count];             	     
                                                strcat(pTemp,(char*)&c);
                                        }
                                        if(count == 9 && (tmplen -1) >=0 ){
                                                pTemp[tmplen-1] ='\0';
                                                g_cur_input_point--;
                                        }
                                        if(count == 11){
                                                if( tmplen > 15)     
                                                        break;                    	 	
                                                if(g_cur_input_point < max_input_count){
                                                        c[0] = '.';
                                                        strcat(pTemp,(char*)&c);
                                                        g_cur_input_point++;
                                                }
                                        }
                                        WRT_Rect tmprect;
                                        tmprect = rc_setipmenuico[offset2];
                                        g_isUpdated = 0;
                                        DrawRect(tmprect,2,rgb_yellow);
                                        DeflateRect(&tmprect,4);
                                        FillRect(rgb_white,tmprect);
                                       // JpegDisp(rc_setipmenuico[offset2].x,rc_setipmenuico[offset2].y,mima_input2,sizeof(mima_input2));                		 
                                        DrawText_16(pTemp,rc_setipmenuico[0].x+10,rc_setipmenuico[offset2].y+32,rgb_black);	    	      	
                                        update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                        g_isUpdated = 1;                                                                            
                                }
                                break;
                        }
                }
                return ret;
}

//=======================================wyx 2014-11-11


static const WRT_Rect rc_setipmenuico_wifi[13]={
	{237,50,200,44}, //密码框输入框所在区域 // 下一个45
	{237,100,200,44}, //密码框输入框所在区域 // 下一个45 //70
	{237,150,200,44}, //密码框输入框所在区域 // 下一个45
	{237,200,200,44}, //密码框输入框所在区域 // 下一个45
	{237,250,200,44}, //密码框输入框所在区域 // 下一个45
	{237,300,200,44}, //密码框输入框所在区域 // 下一个45
	{650,100,300,400},	  //数字符号所在区域539 452


	{780,531,111,69},			   //确认
	{900,531,111,69},			   //返回
	{6,531,111,69}, 				//关屏幕				

	{237,350,200,44}, //密码框输入框所在区域 // 下一个45
	{237,400,200,44}, //密码框输入框所在区域 // 下一个45
	{237,450,48,48},
};


char wifi_ip[16];
char wifi_mask[16];
char wifi_gateway[16];
char wifi_dns[16];
char wifi_ssid[17];
char wifi_pwd[17];
char wifi_authmode[8];
char wifi_encryptype[8];

#define WIFI_IP 		1
#define WIFI_MASK 		2
#define WIFI_GATEWAY	3
#define WIFI_DNS 		4
#define WIFI_SSID   	5
#define WIFI_PWD   		6
#define WIFI_AUTHMODE   7
#define WIFI_ENCRYPTYPE 8

#define ONVIF_AUTH_NAME 9
#define ONVIF_AUTH_PASSWD 10

int  g_cur_wifi_cfg_index = 0;
extern int g_is_config_wifi;

int set_wifi_ip(unsigned long ipaddr)
{
	int tmp_sock = -1;
	struct ifreq T_ifreq;
	tmp_sock  = socket( AF_INET ,SOCK_DGRAM, 0);
	WRT_DEBUG("ModifyHostIp%x\n",ipaddr);
	if(tmp_sock < 0){
		WRT_WARNING("创建socket failed ");
		return -1;
	}
	if(ipaddr != 0){

		strcpy(T_ifreq.ifr_name,"ra0");
		((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_family = AF_INET;
		((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_addr.s_addr =ipaddr; //htonl(ipaddr);
		if(0 !=ioctlsocket(tmp_sock,SIOCSIFADDR,(char*)&T_ifreq)){
			WRT_DEBUG("修改IP地址失败new ipaddr = 0x%x",ipaddr);
			closesocket(tmp_sock);
			return -1;
		}
	}

	closesocket(tmp_sock);

	return 1;	  

}

int set_wifi_netmask(unsigned long netmask)
{
	int tmp_sock = -1;
	struct ifreq T_ifreq;
	tmp_sock  = socket( AF_INET ,SOCK_DGRAM, 0);
	WRT_DEBUG("ModifyHostMask%x\n",netmask);
	if(tmp_sock < 0){
		WRT_WARNING("创建socket failed ");
		return -1;
	}

	if(netmask != 0){
		memset(&T_ifreq,0,sizeof(struct ifreq));
		strcpy(T_ifreq.ifr_name,"ra0");
		((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_family = AF_INET;
		((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_addr.s_addr =netmask;// htonl(netmask);
		if(0 != ioctlsocket(tmp_sock,SIOCSIFNETMASK,(char*)&T_ifreq)){
			WRT_DEBUG("修改网络掩码失败new mask = 0x%x %d",netmask,errno);
			closesocket(tmp_sock);
			return -1;
		}
	}
	closesocket(tmp_sock);

	return 1;      
}

int set_wifi_gateway(unsigned long gatewayip)
{
	struct in_addr in;
	int ret;
	char cmd[256];
	in.s_addr = gatewayip;
	char* ip = inet_ntoa(in);
	memset(cmd,0,256);
	sprintf(cmd,"route add default gateway %s",ip);
	if(system(cmd) < 0){
		WRT_DEBUG("cmd =%s 失败",cmd);
		return -1;
	}
	WRT_DEBUG("cmd =%s ok",cmd);
	return 1;
}

static int modify_wificonfig()
{
	unsigned long ip  = inet_addr(wifi_ip);
	
	pSystemInfo->mySysInfo.wifiip = ip;
	int ret = set_wifi_ip(ip);
	
	if(ret != 1){
		return -1;
	}
	
	ip = 0;
	ip  = inet_addr(wifi_mask);
	

	pSystemInfo->mySysInfo.wifimask = ip;
	ret = set_wifi_netmask(ip);
	if(ret != 1){
		return -1;
	}
	
	ip = 0;
	ip  = inet_addr(wifi_gateway);
	pSystemInfo->mySysInfo.wifigateway = ip;
	ret = set_wifi_gateway(ip);
	if(ret != 1){
		return -1;
	}
	
	pSystemInfo->mySysInfo.wifidns = inet_addr(wifi_dns);

	memcpy(pSystemInfo->mySysInfo.wifissid,wifi_ssid,16);
	memcpy(pSystemInfo->mySysInfo.wifipwd,wifi_pwd,16);

	memcpy(pSystemInfo->mySysInfo.wifiauthmode,wifi_authmode,8);
	memcpy(pSystemInfo->mySysInfo.wifiencryptype,wifi_encryptype,8);

	save_modify();
	
	return 1;
}

int modify_wifi_set(int param){

	CloseTopWindow();
	g_is_config_wifi = 0;
	SDL_Delay(200);
	CreateDialogWin2(LT("修改配置中..."),MB_NONE,NULL,NULL);
	SDL_Delay(1000);
	if(modify_wificonfig() == -1){
		CloseTopWindow();
		CreateDialogWin2(LT("修改网络配置失败"),MB_NONE,NULL,NULL);
		SDL_Delay(1000);
		CloseTopWindow();
	}else{
		CloseTopWindow();
		//send_free_arp();
		CreateDialogWin2(LT("修改网络配置成功"),MB_NONE,NULL,NULL);	
		SDL_Delay(1000);
		Wifi_Config_Process();
		CloseTopWindow();
	}

	return 0;
}

static int wifisetiphandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int tmpY =  0;
        char* pTemp = 0;
        int offset = 0;
        int offset2 = 0;
        int startx = rc_setipmenuico_wifi[6].x;
        int starty = rc_setipmenuico_wifi[6].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';

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
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer1[count],pIPNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer[count],pIPNumberSize[count]);
                                        switch(g_cur_wifi_cfg_index)
                                        {
                        case WIFI_IP:
                                pTemp = (char*)wifi_ip;
                                offset = 32;
                                offset2 = 0;
                                break;
                        case WIFI_MASK:
                                pTemp = (char*)wifi_mask;
                                offset = 32+42;
                                offset2 = 1;
                                break;
                        case WIFI_GATEWAY:
                                pTemp = (char*)wifi_gateway;
                                offset = 32+88;
                                offset2 = 2;
                                break;
                        case WIFI_DNS:
                                pTemp = (char*)wifi_dns;
                                offset = 32+130;
                                offset2 = 3;
                                break;

                        case WIFI_SSID:
                        case WIFI_PWD:
                        case WIFI_AUTHMODE:
                        case WIFI_ENCRYPTYPE:
                        		break;
                        		
                        default:
                                return ret;
                               }
                                        int tmplen = strlen(pTemp);

                                        if(count != 9 && count != 11){  
                                                if( tmplen > 15)     
                                                        break;                   	 	 
                                                c[0] = g_number[count];             	     
                                                strcat(pTemp,(char*)&c);
                                        }
                                        if(count == 9 && (tmplen -1) >=0 ){
                                                pTemp[tmplen-1] ='\0';
                                                g_cur_input_point--;
                                        }
                                        if(count == 11){
                                                if( tmplen > 15)     
                                                        break;                    	 	
                                                if(g_cur_input_point < max_input_count){
                                                        c[0] = '.';
                                                        strcat(pTemp,(char*)&c);
                                                        g_cur_input_point++;
                                                }
                                        }
                                        WRT_Rect tmprect;
                                        tmprect = rc_setipmenuico_wifi[offset2];
                                        g_isUpdated = 0;
                                        DrawRect(tmprect,2,rgb_yellow);
                                        DeflateRect(&tmprect,4);
                                        FillRect(rgb_white,tmprect);
                                        DrawText_16(pTemp,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[offset2].y+32,rgb_black);	    	      	
                                        update_rect(tmprect.x,tmprect.y,tmprect.w,tmprect.h);
                                        g_isUpdated = 1;                                                                            
                                }
                                break;
                        }
                }
                return ret;
}

static void DrawWifiCfgInfo(){

        char name[20];
        int xoffset = 100;
        int yoffset = 0;
        WRT_Rect tmprect;
        unsigned long color = rgb_white;
        
        tmprect = rc_setipmenuico_wifi[0];
        
        if(g_cur_wifi_cfg_index == WIFI_IP)
                color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);

		if(GetCurrentLanguage() == ENGLISH)
		    sprintf(name,"WIFI Ip");
		else
			sprintf(name,"无线IP");
        
        DrawText_16(name,xoffset,rc_setipmenuico_wifi[0].y+rc_setipmenuico_wifi[0].h-15+yoffset,rgb_white);

        tmprect = rc_setipmenuico_wifi[1];
        if(g_cur_wifi_cfg_index == WIFI_MASK)
                color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
         
        
        if(GetCurrentLanguage() == ENGLISH)
		    sprintf(name,"Netmask");
		else
			sprintf(name,"子网掩码");
        DrawText_16(name,xoffset,rc_setipmenuico_wifi[1].y+rc_setipmenuico_wifi[1].h-15+yoffset,rgb_white);

        tmprect = rc_setipmenuico_wifi[2];
        if(g_cur_wifi_cfg_index == WIFI_GATEWAY)
                 color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);                
        
        
        if(GetCurrentLanguage() == ENGLISH)
		    sprintf(name,"GateWay");
		else
			sprintf(name,"无线网关");
        DrawText_16(name,xoffset,rc_setipmenuico_wifi[2].y+rc_setipmenuico_wifi[2].h-15+yoffset,rgb_white);

        tmprect = rc_setipmenuico_wifi[3];
        if(g_cur_wifi_cfg_index == WIFI_DNS)
                color = rgb_yellow;
        else
               color = rgb_white;
               
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);               
        
        sprintf(name,"DNS");
        
        DrawText_16(name,xoffset,rc_setipmenuico_wifi[3].y+rc_setipmenuico_wifi[3].h-15+yoffset,rgb_white);	  

        tmprect = rc_setipmenuico_wifi[4];
        if(g_cur_wifi_cfg_index == WIFI_SSID)
                 color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);               

		if(GetCurrentLanguage() == ENGLISH)
		    sprintf(name,"SSID");
		else
			sprintf(name,"无线名称");
    	
        color = rgb_white;
         
        DrawText_16(name,xoffset,rc_setipmenuico_wifi[4].y+rc_setipmenuico_wifi[4].h-15+yoffset,color);	

        tmprect = rc_setipmenuico_wifi[5];
        if(g_cur_wifi_cfg_index == WIFI_PWD)
               color = rgb_yellow;
        else
               color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);   
		
        
		if(GetCurrentLanguage() == ENGLISH)
		    sprintf(name,"Password");
		else
			sprintf(name,"无线密码");
		
        DrawText_16(name,xoffset,rc_setipmenuico_wifi[5].y+rc_setipmenuico_wifi[5].h-15+yoffset,rgb_white);	


        DrawText_16(wifi_ip,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[0].y+32+yoffset,rgb_black);
        DrawText_16(wifi_mask,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[1].y+32+yoffset,rgb_black);
        DrawText_16(wifi_gateway,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[2].y+32+yoffset,rgb_black);
        DrawText_16(wifi_dns,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[3].y+32+yoffset,rgb_black);
        DrawText_16(wifi_ssid,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[4].y+32+yoffset,rgb_black);
        DrawText_16(wifi_pwd,rc_setipmenuico_wifi[0].x+10,rc_setipmenuico_wifi[5].y+32+yoffset,rgb_black);

		if(GetCurrentLanguage() == ENGLISH)
			sprintf(name,"DHCP");
		else
			sprintf(name,"动态获取IP");

        DrawText_16(name,xoffset,rc_setipmenuico_wifi[12].y+32,rgb_white);
		

        if(pSystemInfo->mySysInfo.wifi_dhcp == 1)		
            JpegDisp(rc_setipmenuico_wifi[12].x,rc_setipmenuico_wifi[12].y,ico_fangqukai,sizeof(ico_fangqukai));
        else
            JpegDisp(rc_setipmenuico_wifi[12].x,rc_setipmenuico_wifi[12].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));

}

int WifiCfgEventHandler(int x,int y ,int status){
        WRT_xy xy;
        int ret = 0;
        int tmpret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
       
        for(index =0;index<13;index++){
                if(IsInside(rc_setipmenuico_wifi[index],xy)){
                        pos_x = rc_setipmenuico_wifi[index].x;
                        pos_y = rc_setipmenuico_wifi[index].y;
                        ret = 1;         
                        switch(index){     
                  case 0:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
								g_cur_wifi_cfg_index = WIFI_IP; 
								g_cur_input_point = 0;
								g_isUpdated = 0;
								DrawWifiCfgInfo();
								g_isUpdated = 1;
								update_rect(0,0,529,531);
                          }
                          break;
                  case 1:
                          if(status == 1){
	                          StartButtonVocie();
                          }else{
								g_cur_wifi_cfg_index = WIFI_MASK; 
								g_cur_input_point =0;
								g_isUpdated = 0;
								DrawWifiCfgInfo();
								g_isUpdated = 1;
								update_rect(0,0,529,531);
                          }
                          break;
                  case 2:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_wifi_cfg_index = WIFI_GATEWAY;
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawWifiCfgInfo();
                                  g_isUpdated = 1;
        			   			update_rect(0,0,529,531);
                          }
                          break;
                  case 3:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_wifi_cfg_index = WIFI_DNS; 
                                  g_cur_input_point =0;
                                  g_isUpdated = 0;
                                  DrawWifiCfgInfo();
                                  g_isUpdated = 1;
        			  			  update_rect(0,0,529,531);    
                          }             	  	
                          break;   

                  case 4:
                          if(status == 1){
	                          StartButtonVocie();
                          }else{
	                          g_cur_wifi_cfg_index = WIFI_SSID;
				  			  CreateSsidPwdWin(0);
                          }             	  	
                          break; 

                  case 5:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_wifi_cfg_index = WIFI_PWD; 
        			  			 CreateSsidPwdWin(1);
                          }             	  	
                          break;
                  case 10:
				  		break;
                  		if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_wifi_cfg_index = WIFI_AUTHMODE; 
        			  			 CreateSsidPwdWin(2);
                          }
                  		break;
                  case 11:
				  		break;
                  		if(status == 1){
                                 StartButtonVocie();
                          }else{
                                 g_cur_wifi_cfg_index = WIFI_ENCRYPTYPE;
        			  			 CreateSsidPwdWin(3);
                          }
                  		break;
                  case 12:
                  		if(status == 1){
                                 StartButtonVocie();
                          }else{
                          		pSystemInfo->mySysInfo.wifi_dhcp = !pSystemInfo->mySysInfo.wifi_dhcp;
                          		UpdateSystemInfo();
                          		
                                if(pSystemInfo->mySysInfo.wifi_dhcp == 1)	
						            JpegDisp(rc_setipmenuico_wifi[12].x,rc_setipmenuico_wifi[12].y,ico_fangqukai,sizeof(ico_fangqukai));
						        else
						            JpegDisp(rc_setipmenuico_wifi[12].x,rc_setipmenuico_wifi[12].y,ico_fangquwuxiao,sizeof(ico_fangquwuxiao));
                          }
                  		break;
                  case 7://确认
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));  
                                  
                                  CreateDialogWin2(LT("确定要修改网络配置？"),MB_OK|MB_CANCEL,modify_wifi_set,NULL);
                                  
                                  g_cur_wifi_cfg_index = WIFI_IP; 
                                  g_cur_input_point = 0;     
                          }            	  	
                          break;
                  case 8: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                  CloseTopWindow();
                          }
                          break;
                  case 9:/*关屏*/
                          if(status == 1){ //按下
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                  screen_close(); 
                                  CloseAllWindows();
                          }
                          break;

                  case 6:
                          //进入数字区域
                          ret = wifisetiphandler(x,y,status);	  
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }  

        return ret;  		
}

void DrawWifiCfgMain()
{
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

	JpegDisp(rc_setipmenuico_wifi[8].x,rc_setipmenuico_wifi[8].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_setipmenuico_wifi[9].x,rc_setipmenuico_wifi[9].y,bt_guanping1,sizeof(bt_guanping1)); 
	int i =0; 
	int j =0;
	int startx = rc_setipmenuico_wifi[6].x;
	int starty = rc_setipmenuico_wifi[6].y;
	for(i=0; i<4;i++)
		for(j=0; j<3;j++){
			JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pIPNumberPointer[(i*3)+j],pIPNumberSize[(i*3)+j]);
		}

	WRT_Rect curbox;
	curbox.x = 135;
	curbox.y = 531;
	curbox.w = 578;
	curbox.h = 55;
	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4)); 
	JpegDisp(rc_setipmenuico_wifi[7].x,rc_setipmenuico_wifi[7].y,menu_ok1,sizeof(menu_ok1));                 

	DrawText_16(LT("提示：按'*'退格，按'#' 输入'.'"),curbox.x,curbox.y+45,rgb_black);	
	DrawWifiCfgInfo();
	
}

void CreateWifiCfgWin()
{
	Window* pWindow = (Window*)New_Window();
    if(pWindow == NULL)
            return ;  
    pWindow->EventHandler = WifiCfgEventHandler;
    pWindow->NotifyReDraw = DrawWifiCfgMain;
    pWindow->valid = 1;
    pWindow->type = CHILD;
    WindowAddToWindowList(pWindow);
    if(pIPNumberPointer == NULL && pIPNumberPointer1 == NULL){
            GetNumberJpgPointer(&pIPNumberPointer,&pIPNumberPointer1);
            GetNumberJpegSizePointer(&pIPNumberSize,&pIPNumberSize1);
    }
    
	memset(wifi_ip,0,16);
	memset(wifi_mask,0,16);
	memset(wifi_gateway,0,16);
	memset(wifi_dns,0,16);
	memset(wifi_pwd,0,17);
	memset(wifi_ssid,0,17);
	memset(wifi_authmode,0,8);
	memset(wifi_encryptype,0,8);
	
    toinet_addr(pSystemInfo->mySysInfo.wifiip,(unsigned char*)wifi_ip);
    toinet_addr(pSystemInfo->mySysInfo.wifimask,(unsigned char*)wifi_mask);
    toinet_addr(pSystemInfo->mySysInfo.wifigateway,(unsigned char*)wifi_gateway);
    toinet_addr(pSystemInfo->mySysInfo.wifidns,(unsigned char*)wifi_dns);
    
	memcpy(wifi_ssid,pSystemInfo->mySysInfo.wifissid,16);
	memcpy(wifi_pwd,pSystemInfo->mySysInfo.wifipwd,16);
	memcpy(wifi_authmode,pSystemInfo->mySysInfo.wifiauthmode,8);
	memcpy(wifi_encryptype,pSystemInfo->mySysInfo.wifiencryptype,8);
	
	//memset(pSystemInfo->mySysInfo.wifipwd,0,28);
	//memset(wifi_pwd,0,16);
    g_cur_wifi_cfg_index = WIFI_IP;

    pWindow->NotifyReDraw2(pWindow);	
}

//=======================================================
//=========================================================

void CreateModifyIPWin(int type){
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;  
        pWindow->EventHandler = ModifyIPEventHandler;
        pWindow->NotifyReDraw = DrawModifyIPMain;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        g_cur_window_type = type;
        WindowAddToWindowList(pWindow);
        if(pIPNumberPointer == NULL && pIPNumberPointer1 == NULL){
                GetNumberJpgPointer(&pIPNumberPointer,&pIPNumberPointer1);
                GetNumberJpegSizePointer(&pIPNumberSize,&pIPNumberSize1);
        }
		memset(modifyip,0,16);
		memset(modifymask,0,16);  
		memset(modifygateway,0,16);  
		memset(modifycenterip,0,16);  
		memset(modifyroomip,0,16);  
		memset(modifydoorip,0,16);  
		memset(modifymanagerip2,0,16);
        if(g_cur_window_type == 0){
                unsigned long ip =0;
                get_host_ip(&ip);
                toinet_addr(ip,(unsigned char*)modifyip);

                ip =0;
                get_host_mask(&ip);
                toinet_addr(ip,(unsigned char*)modifymask);

                ip =0;
                get_host_gateway(&ip);
                toinet_addr(ip,(unsigned char*)modifygateway);

                ip =0;
                wrthost_get_centerip(&ip);
                toinet_addr(ip,(unsigned char*)modifycenterip);

                if(pSystemInfo->mySysInfo.hassip == 0x01){    
                        ip =0;
                        ip = pSystemInfo->mySysInfo.sipproxyip;

                        toinet_addr(ip,(unsigned char*)modifyroomip);
                }

                ip =0;
                wrthost_get_doorip(&ip);
                toinet_addr(ip,(unsigned char*)modifydoorip);

                ip =0;
                wrthost_get_managerip(&ip);
                toinet_addr(ip,(unsigned char*)modifymanagerip);

                ip = 0;
                ip =  pSystemInfo->LocalSetting.publicinfo.ManagerMinorIP;
                toinet_addr(ip,(unsigned char*)modifymanagerip2);

#if USE_LIFT_IP      
                ip = 0;
                if(pSystemInfo)
                        ip = pSystemInfo->mySysInfo.bakliftip;
                toinet_addr(ip,(unsigned char*)modifygarageip);
#endif      

                max_input_count = 3;
        } else{
                memset(modifyip,0,16);
                memset(modifymask,0,16);  
                memset(modifygateway,0,16);  
                memset(modifycenterip,0,16);  
                memset(modifyroomip,0,16);  
                memset(modifydoorip,0,16);  
                memset(modifymanagerip2,0,16);
                max_input_count = 4;
        }     
        g_cur_modify_index = MODIFY_IP;

        pWindow->NotifyReDraw2(pWindow);	
}

static void DrawDebugInfo(){

        char name[20];
        int xoffset = 5;
        int yoffset = 0;
        WRT_Rect tmprect;
        unsigned long color = rgb_white;
        if(g_cur_window_type != 0){
                xoffset = 30;
                yoffset = 0;
        }
        tmprect = rc_setipmenuico[0];
        
        if(g_cur_modify_index == MODIFY_IP)
                color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
        if(g_cur_window_type == 0){  
                sprintf(name,"IP:");
        }else
                sprintf(name,"%s:",LT("年"));
        DrawText_16(name,xoffset,rc_setipmenuico[0].y+rc_setipmenuico[0].h-15+yoffset,rgb_white);

         tmprect = rc_setipmenuico[1];
        if(g_cur_modify_index == MODIFY_MASK)
                color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);
        if(g_cur_window_type == 0){  
                sprintf(name,"%s:",LT("子网掩码"));
        }else
                sprintf(name,"%s:",LT("月"));	      
        DrawText_16(name,xoffset,rc_setipmenuico[1].y+rc_setipmenuico[1].h-15+yoffset,rgb_white);

        tmprect = rc_setipmenuico[2];
        if(g_cur_modify_index == MODIFY_GATEWAY)
                 color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);                
        if(g_cur_window_type == 0){  
                sprintf(name,"%s:",LT("网关"));
        }else
                sprintf(name,"%s:",LT("日"));		      
        DrawText_16(name,xoffset,rc_setipmenuico[2].y+rc_setipmenuico[2].h-15+yoffset,rgb_white);

        tmprect = rc_setipmenuico[3];
        if(g_cur_modify_index == MODIFY_CENTERIP)
                color = rgb_yellow;
        else
               color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);               
        if(g_cur_window_type == 0){
                sprintf(name,"%s IP:",LT("中心"));
        }else
                sprintf(name,"%s:",LT("时"));
        DrawText_16(name,xoffset,rc_setipmenuico[3].y+rc_setipmenuico[3].h-15+yoffset,rgb_white);	  

        tmprect = rc_setipmenuico[4];
        if(g_cur_modify_index == MODIFY_ROOMIP)
                 color = rgb_yellow;
        else
                color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);               
        if(g_cur_window_type == 0){ 
                if(pSystemInfo->mySysInfo.hassip == 0x01){
                		if(pSystemInfo->mySysInfo.isenglish)
                        	sprintf(name,"SIP Server IP:");
                        else
                        	sprintf(name,"SIP 服务器 IP:");
                        color = rgb_white;
                }else{ 
                        sprintf(name,"%s IP:",LT("房间"));
                        color = 0x00c0c0c0;
                }
        }else{
                sprintf(name,"%s:",LT("分"));	
                color = rgb_white;	      
        }
        DrawText_16(name,xoffset,rc_setipmenuico[4].y+rc_setipmenuico[4].h-15+yoffset,color);	

        tmprect = rc_setipmenuico[5];
        if(g_cur_modify_index == MODIFY_DOORIP)
               color = rgb_yellow;
        else
               color = rgb_white;
        DrawRect(tmprect,2,color);
        DeflateRect(&tmprect,4);
        FillRect(rgb_white,tmprect);               
        if(g_cur_window_type == 0){  
                sprintf(name,"%s IP:",LT("门口机"));
        }else
                sprintf(name,"%s:",LT("秒"));		      
        DrawText_16(name,xoffset,rc_setipmenuico[5].y+rc_setipmenuico[5].h-15+yoffset,rgb_white);	

        if(g_cur_window_type == 0){
                sprintf(name,"%s IP:",LT("管理机"));		      
                DrawText_16(name,xoffset,rc_setipmenuico[10].y+rc_setipmenuico[10].h-15+yoffset,rgb_white);	
#if USE_LIFT_IP
                sprintf(name,"%s IP:",LT("备用电梯"));			
                DrawText_16(name,xoffset,rc_setipmenuico[11].y+rc_setipmenuico[11].h-15+yoffset,rgb_white);	
#else		
                sprintf(name,"车库IP:");			
                DrawText_16(name,xoffset,rc_setipmenuico[11].y+rc_setipmenuico[11].h-15+yoffset,0x00c0c0c0);	
#endif	

                sprintf(name,"%s IP2:",LT("管理机"));		      
                DrawText_16(name,xoffset,rc_setipmenuico[12].y+rc_setipmenuico[12].h-15+yoffset,rgb_white);
                tmprect = rc_setipmenuico[10];
                if(g_cur_modify_index == MODIFY_MANAGERIP){
                         color = rgb_yellow;	        
                }else{
                       color = rgb_white;
                }
                DrawRect(tmprect,2,color);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                
                tmprect = rc_setipmenuico[11];                	    
                if(g_cur_modify_index == MODIFY_GARAGEIP){
                         color = rgb_yellow;	        
                }else{
                        color = rgb_white;
                }
                DrawRect(tmprect,2,color);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
                 
                 tmprect = rc_setipmenuico[12];              	
                if(g_cur_modify_index == MODIFY_BAKMANAGERIP)
                {   
                         color = rgb_yellow;	        
                }else{
                       color = rgb_white;
                }
                DrawRect(tmprect,2,color);
                DeflateRect(&tmprect,4);
                FillRect(rgb_white,tmprect);
        }

        DrawText_16(modifyip,rc_setipmenuico[0].x+10,rc_setipmenuico[0].y+32+yoffset,rgb_black);
        DrawText_16(modifymask,rc_setipmenuico[0].x+10,rc_setipmenuico[1].y+32+yoffset,rgb_black);
        DrawText_16(modifygateway,rc_setipmenuico[0].x+10,rc_setipmenuico[2].y+32+yoffset,rgb_black);
        DrawText_16(modifycenterip,rc_setipmenuico[0].x+10,rc_setipmenuico[3].y+32+yoffset,rgb_black);
        DrawText_16(modifyroomip,rc_setipmenuico[0].x+10,rc_setipmenuico[4].y+32+yoffset,rgb_black);
        DrawText_16(modifydoorip,rc_setipmenuico[0].x+10,rc_setipmenuico[5].y+32+yoffset,rgb_black);
        if(g_cur_window_type == 0){
                DrawText_16(modifymanagerip,rc_setipmenuico[0].x+10,rc_setipmenuico[10].y+32+yoffset,rgb_black);
                DrawText_16(modifygarageip,rc_setipmenuico[0].x+10,rc_setipmenuico[11].y+32+yoffset,rgb_black);          	  	
                DrawText_16(modifymanagerip2,rc_setipmenuico[0].x+10,rc_setipmenuico[12].y+32+yoffset,rgb_black);
        }
        
}
void DrawModifyIPMain(){
          
		  WRT_Rect rt;
		  rt.x = 0;
		  rt.y = 0;
		  rt.w =1024;
		  rt.h = 530;
		  FillRect(0xb2136,rt);
  
		  JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

        JpegDisp(rc_setipmenuico[8].x,rc_setipmenuico[8].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_setipmenuico[9].x,rc_setipmenuico[9].y,bt_guanping1,sizeof(bt_guanping1)); 
        int i =0; 
        int j =0;
        int startx = rc_setipmenuico[6].x;
        int starty = rc_setipmenuico[6].y;
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pIPNumberPointer[(i*3)+j],pIPNumberSize[(i*3)+j]);
                }


                WRT_Rect curbox;
                curbox.x = 135;
                curbox.y = 531;
                curbox.w = 578;
                curbox.h = 55;
	        	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4)); 
                JpegDisp(rc_setipmenuico[7].x,rc_setipmenuico[7].y,menu_ok1,sizeof(menu_ok1));                 
                //FillRect(0xdedede,curbox); 
                DrawText_16(LT("提示：按'*'退格，按'#' 输入'.'"),curbox.x,curbox.y+45,rgb_black);	
                DrawDebugInfo();
}

int ModifyIPEventHandler(int x,int y ,int status){
        WRT_xy xy;
        int ret = 0;
        int tmpret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
       
        for(index =0;index<13;index++){
                if(IsInside(rc_setipmenuico[index],xy)){
                        pos_x = rc_setipmenuico[index].x;
                        pos_y = rc_setipmenuico[index].y;
                        ret = 1;         
                        switch(index){     
                  case 0:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_IP; 
                                  g_cur_input_point = 0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			   			update_rect(0,0,529,531);    
                          }
                          break;
                  case 1:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_MASK; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			   				update_rect(0,0,529,531);    
                          }            	  	  
                          break;
                  case 2:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_GATEWAY; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			   			update_rect(0,0,529,531);    
                          }             	  	
                          break;
                  case 3:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_CENTERIP; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			  			 update_rect(0,0,529,531);    
                          }             	  	
                          break;   

                  case 4:
                          if(pSystemInfo->mySysInfo.hassip == 0x0)
                                  break;
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_ROOMIP; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			  			 update_rect(0,0,529,531);    
                          }             	  	
                          break; 

                  case 5:
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_DOORIP; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			  			 update_rect(0,0,529,531);    
                          }             	  	
                          break;            	 	         	  	  
                  case 7://确认
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,menu_ok2,sizeof(menu_ok2));
                          }else{
                                  JpegDisp(pos_x,pos_y,menu_ok1,sizeof(menu_ok1));  
                                  if(g_cur_window_type == 0)         
                                          CreateDialogWin2(LT("确定要修改网络配置？"),MB_OK|MB_CANCEL,modify_net_set,NULL);
                                  else
                                          CreateDialogWin2("确定要修改时间？",MB_OK|MB_CANCEL,modify_net_set,NULL);
                                  g_cur_modify_index = MODIFY_IP; 
                                  g_cur_input_point = 0;     
                          }            	  	
                          break;      
                  case 8: /*返回*/
                          if(status == 1){
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                  CloseTopWindow();                   
                          }
                          break;
                  case 9:/*关屏*/
                          if(status == 1){ //按下
                                  StartButtonVocie();
                                  JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                          }else{
                                  JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                  screen_close(); 
                                  CloseAllWindows();
                          }
                          break;
                  case 10:
                          if(g_cur_window_type != 0)
                                  break;
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_MANAGERIP; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        			   			update_rect(0,0,529,531);    
                          }                        
                          break;
#if USE_LIFT_IP                    
                  case 11:
                          if(g_cur_window_type != 0)
                                  break;
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_GARAGEIP; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                  g_isUpdated = 1;
        						  update_rect(0,0,529,531);    
                          }
#endif            	  	                       
                          break;
                  case 12:
                          if(g_cur_window_type != 0)
                                  break;
                          if(status == 1){
                                  StartButtonVocie();
                          }else{
                                  g_cur_modify_index = MODIFY_BAKMANAGERIP; 
                                  g_cur_input_point =0;
                                   g_isUpdated = 0;
                                  DrawDebugInfo();
                                   g_isUpdated = 1;
        			   			update_rect(0,0,529,531);    
                          }                        
                          break;
                  case 6:
                          //进入数字区域
                          ret = setiphandler(x,y,status);	  
                          break;
                  default:
                          break;
                        }
                        break;
                }
        }  

        return ret;  		
}

int modify_net_set(int param){

        if(g_cur_window_type == 0){
        		CloseTopWindow();
        		SDL_Delay(200);
                CreateDialogWin2(LT("修改配置中..."),MB_NONE,NULL,NULL);
                SDL_Delay(1000);
                if(modify_netconfig() == -1){
                        CloseTopWindow();
                        CreateDialogWin2(LT("修改网络配置失败"),MB_NONE,NULL,NULL);
                        SDL_Delay(1000);
                        CloseTopWindow();
                }else{
                        CloseTopWindow();
                        send_free_arp();
                        CreateDialogWin2(LT("修改网络配置成功"),MB_NONE,NULL,NULL);	
                        SDL_Delay(1000);
                        CloseTopWindow();            
                }
        }else{
        		CloseTopWindow();
        		SDL_Delay(200);
                CreateDialogWin2("修改时间中...",MB_NONE,NULL,NULL);
                SDL_Delay(1000);
                if(modifytime() != 0){
                        CloseTopWindow();
                        CreateDialogWin2("修改时间失败",MB_NONE,NULL,NULL);
                        SDL_Delay(1000);
                        CloseTopWindow();              
                }else{
                        CloseTopWindow();
                        CreateDialogWin2("修改时间成功",MB_NONE,NULL,NULL);	
                        SDL_Delay(1000);
                        CloseWindow();              
                }
        }
        return 0;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

static  WRT_Rect rc_setnetconfigmenuico[11]={
        {15,60,720,50},   //区号 // {100,70,56,36},   //区号
        {0,0,0,0},   //栋号
        {0,0,0,0},   //单元号
        {0,0,0,0},    //层号
        {0,0,0,0},   //房间号
        {0,0,0,0},   //分机编号
        {0,0,0,0}, //自动配置
        {15,119,700,400},                //所有字母所在的范围
        {721,119,300,400},             //所有数字所在的范围

        {900,531,111,69},              //返回
        {6,531,111,69},                 //关屏幕				
};

static int g_cur_select_edit = 0;
static int g_pre_select_edit = 0;
static char g_input_number[6][4];
static char g_input_number2[6][4];
static const int g_input_max_count[6] = {2,3,2,3,3,2};
static int g_cur_input_count = 0;
static  char g_editcontent[100];
static  char g_editcontent1[6][4]={"__","___","__","___","___","__"};
static  char g_postion_name[6][10]={"区","栋","单元","层","房","编号"};

static const char  g_sprintfcmd[6][5]={"%02s","%03s","%02s","%03s","%03s","%02s"};

static void SetEditRect()
{
	if(GetCurrentLanguage() == ENGLISH){
	    rc_setnetconfigmenuico[1].x = 95;	
	    rc_setnetconfigmenuico[1].y = 64;
	    rc_setnetconfigmenuico[1].w = 50;
	    rc_setnetconfigmenuico[1].h = 42;
	    
	    rc_setnetconfigmenuico[2] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[2].x = 189;
	    rc_setnetconfigmenuico[2].w = 62;
	    
	    rc_setnetconfigmenuico[3] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[3].x = 293;
	    rc_setnetconfigmenuico[3].w = 50;
	    
	    rc_setnetconfigmenuico[4] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[4].x = 386;
	    rc_setnetconfigmenuico[4].w = 62;
	    
	    rc_setnetconfigmenuico[5] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[5].x = 489;
	    rc_setnetconfigmenuico[5].w = 62;
	    
	    rc_setnetconfigmenuico[6] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[6].x = 594;
	    rc_setnetconfigmenuico[6].w = 50;
		
	}else{
	    rc_setnetconfigmenuico[1].x = 39;	
	    rc_setnetconfigmenuico[1].y = 64;
	    rc_setnetconfigmenuico[1].w = 50;
	    rc_setnetconfigmenuico[1].h = 42;
	    
	    rc_setnetconfigmenuico[2] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[2].x = 142;
	    rc_setnetconfigmenuico[2].w = 62;
	    
	    rc_setnetconfigmenuico[3] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[3].x = 257;
	    rc_setnetconfigmenuico[3].w = 50;
	    
	    rc_setnetconfigmenuico[4] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[4].x = 385;
	    rc_setnetconfigmenuico[4].w = 62;
	    
	    rc_setnetconfigmenuico[5] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[5].x = 499;
	    rc_setnetconfigmenuico[5].w = 62;
	    
	    rc_setnetconfigmenuico[6] = rc_setnetconfigmenuico[1];
	    rc_setnetconfigmenuico[6].x = 614;
	    rc_setnetconfigmenuico[6].w = 50;
	    	    
	}
	
}

static void SetEditContent(char* editcontent,char* editcontent1,int editpos){
	           int pos = 0;
	           int i = 0;
                   int result = editpos*2;
                   char* pDest= NULL;
                   int tmpcount = 0;
                   char* pSrc = editcontent;
                   for(i = 0; i<result;i++){
                   	   pDest = strchr(pSrc,' ');
                   	   if(pDest == NULL)
                   	   	break;
                   	   pSrc = pDest + 1;
                   	
                   }
                   
                   if(pDest == NULL){
                   	pos = 0;
                   }else
                   	pos = pDest - editcontent +1;
                   switch(editpos)
                   {
                   	case 0:
                   		editcontent1[pos]='_';
                   		editcontent1[pos+1]='_';
      
                   		break;
                   	case 1:
                   		editcontent1[pos]='_';
                   		editcontent1[pos+1]='_';
                   		editcontent1[pos+2]='_';
                   		break;
                   	case 2:
                   		editcontent1[pos]='_';
                   		editcontent1[pos+1]='_';
                   		break;
                   	case 3:
                   		editcontent1[pos]='_';
                   		editcontent1[pos+1]='_';
                   		editcontent1[pos+2]='_';
                   		break;
                   	case 4:
                   		editcontent1[pos]='_';
                   		editcontent1[pos+1]='_';
                   		editcontent1[pos+2]='_';
                   		break;
                   	case 5:
                   		editcontent1[pos]='_';
                   		editcontent1[pos+1]='_';
                   		break;
                   }              	
}
static void DrawEditViewContent(int update)
{
		WRT_Rect tmprect;
        memset(g_editcontent,0,100);
        sprintf(g_editcontent,"00 %s 000 %s 00 %s 000 %s 000 %s 00 %s",LT("区"),LT("栋"),LT("单元"),LT("层"),LT("房"),LT("编号"));

        tmprect.x = rc_setnetconfigmenuico[0].x;
        tmprect.y = rc_setnetconfigmenuico[0].y;
        tmprect.w = 800;
        tmprect.h = 50;
        DrawRect(tmprect,2,rgb_white);
        DeflateRect(&tmprect,4);
        //DrawRect(tmprect,4,rgb_black);
        FillRect(rgb_white,tmprect);
        tmprect.x += 10;
       

       	for(int i = 0;i<6;i++){
       	    	FillRect(rgb_white,rc_setnetconfigmenuico[i+1]);
       	    	DrawTextToRect(rc_setnetconfigmenuico[i+1],g_input_number[i],rgb_black);
       	}
       	for(int j=0;j<6;j++)
       	{
       		DrawText(LT(g_postion_name[j]),rc_setnetconfigmenuico[j+1].x+rc_setnetconfigmenuico[j+1].w+20,rc_setnetconfigmenuico[1].h+rc_setnetconfigmenuico[1].y-10,rgb_black,24);
       	}

        DrawTextToRect(rc_setnetconfigmenuico[g_cur_select_edit+1],g_editcontent1[g_cur_select_edit],rgb_red);
          
}

static void DrawSettingsIcon(int indx,int offset)
{
	
	WRT_Rect tmprect[2];
	offset = 0;
	tmprect[0].x = rc_setnetconfigmenuico[7].x+5*100 + 2 - offset;
	tmprect[0].y = rc_setnetconfigmenuico[7].y + 3*100 -offset;
	tmprect[0].w = 88;
	tmprect[0].h = 79;
	tmprect[1] = tmprect[0];
	tmprect[1].x += 100;
	if(indx == 0)
	{
		//DrawText(LT("手动配置"),tmprect[0].x,tmprect[0].y+40,rgb_yellow,16);	
		DrawTextToRect2(tmprect[0],LT("手动配置"),rgb_yellow,1);
	}
	else
	{
		//DrawText(LT("自动配置"),tmprect[1].x,tmprect[1].y+40,rgb_yellow,16);
		DrawTextToRect2(tmprect[1],LT("自动配置"),rgb_yellow,1);

	}
	
}
static int setnetconfighandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int tmpY =  0;
        char* pTemp = 0;
        int offset = 0;
        int offset2 = 0;
        int startx = rc_setnetconfigmenuico[8].x;
        int starty = rc_setnetconfigmenuico[8].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';

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
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer1[count],pIPNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer[count],pIPNumberSize[count]);
                                        if(count != 9 && count != 11){  
                                                g_cur_input_count --;
                                                if(g_cur_input_count > (-1)){        	 	 
                                                	char tmp[4]={'\0'};
                                                        c[0] = g_number[count];  
                                                        strcat(g_input_number2[g_cur_select_edit],c);
                                                        sprintf(g_input_number[g_cur_select_edit],g_sprintfcmd[g_cur_select_edit],g_input_number2[g_cur_select_edit]);
							if(g_cur_input_count == 0){
                                                       	
                                                		g_pre_select_edit = g_cur_select_edit;
                                                	
                                                		g_cur_select_edit ++;
                                                		if(g_cur_select_edit > 5)
                                                			g_cur_select_edit = 0; 
                                                        	g_cur_input_count = g_input_max_count[g_cur_select_edit];
                                                        	memset(g_input_number2[g_cur_select_edit],0,4);
                                                	}
                                                }
                                                g_isUpdated = 0;
                                                DrawEditViewContent(1);
                                                update_rect(rc_setnetconfigmenuico[0].x,rc_setnetconfigmenuico[0].y,rc_setnetconfigmenuico[0].w,rc_setnetconfigmenuico[0].h);
                                                g_isUpdated = 1;
                                        }
                                        if(count == 9){ //*
                                        	
                                                g_cur_input_count = g_input_max_count[g_cur_select_edit];
                                                memset(g_input_number2[g_cur_select_edit],0,4);
                                                sprintf(g_input_number[g_cur_select_edit],g_sprintfcmd[g_cur_select_edit],g_input_number2[g_cur_select_edit]);
 						g_isUpdated = 0;
                                                DrawEditViewContent(1);
                                                update_rect(rc_setnetconfigmenuico[0].x,rc_setnetconfigmenuico[0].y,rc_setnetconfigmenuico[0].w,rc_setnetconfigmenuico[0].h);
                                                g_isUpdated = 1;
                                                
                                        }
                                        if(count == 11){//#
                                                CreateDialogWin2(LT("你确定要修改房间编号？"),MB_OK|MB_CANCEL,modify_room_number,NULL);
                                        }
                                }
                                break;
                        }
                }
                return ret;
}
//字母表的处理
static int alphabethandler_modifyid(int x,int y,int status){
        int i= 0;
        int j =0;
        int tempx  =0;
        int tempy = 0;
        int ret = 0;
        int count =0;
        int startx = rc_setnetconfigmenuico[7].x;
        int starty = rc_setnetconfigmenuico[7].y;
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0; i<4;i++)
                for(j=0; j< 7;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){
                                count = (i*7)+j;
                                ret  = 1;
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPAlphabetPointer1[count],pIPAlphabetSize1[count]);
                                        if(count == 26)
                                        	DrawSettingsIcon(0,3);
                                        if(count == 27)
                                        	DrawSettingsIcon(1,3);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPAlphabetPointer[count],pIPAlphabetSize[count]);
                                        if(count < 26){ 
                                                g_cur_input_count --;
                                                if(g_cur_input_count > (-1)){        	 	 
                                                	char tmp[4]={'\0'};
                                                        c[0] = g_alphabet[count];  
                                                        strcat(g_input_number2[g_cur_select_edit],c);
                                                        sprintf(g_input_number[g_cur_select_edit],g_sprintfcmd[g_cur_select_edit],g_input_number2[g_cur_select_edit]);
                                                        
                                                       
                                                        if(g_cur_input_count == 0){
                                                       	
                                                		g_pre_select_edit = g_cur_select_edit;
                                                	
                                                		g_cur_select_edit ++;
                                                		if(g_cur_select_edit > 5)
                                                			g_cur_select_edit = 0; 
                                                        	g_cur_input_count = g_input_max_count[g_cur_select_edit];
                                                        	memset(g_input_number2[g_cur_select_edit],0,4);
                                                	}
                                                }
                                                g_isUpdated = 0;
                                                DrawEditViewContent(1);
                                                update_rect(rc_setnetconfigmenuico[0].x,rc_setnetconfigmenuico[0].y,rc_setnetconfigmenuico[0].w,rc_setnetconfigmenuico[0].h);
                                                g_isUpdated = 1;
                                                
                                        }else if(count == 26){ //手动配置 
                                        	 DrawSettingsIcon(0,0);
                                        	 CreateModifyIPWin(0);   
                                              
                                        }else if(count == 27){//由于自动配置会从未配置此ID的中心下载错误数据所以删除
                                               DrawSettingsIcon(1,0);
                                        }
                                }
                                break;
                        }
                }
                return ret;
}

int modify_room_number(int param){
        int i = 0;
        int z = 0;
        int j[6] = {2,5,7,10,13,15};
        int i_len = 0;
        int offset = 2;
        char tmpID[16];
        char *x;
        int  localnumber = 0;
        memset(tmpID,0,16);
        CreateDialogWin2(LT("正在保存数据..."),MB_NONE,NULL,NULL);
        /*
        for(z =0 ;z<6;z++){
                offset = strlen(g_input_number[z]);
                for(i= 0;i<offset; i++){
                        tmpID[--j[z]] = g_input_number[z][offset-(1+i)];          
                }

        }
        */
        sprintf(tmpID,"%02s%03s%02s%03s%03s%02s",g_input_number[0],g_input_number[1],g_input_number[2],g_input_number[3],g_input_number[4],g_input_number[5]);
        localnumber = atoi(g_input_number[5]);
        if(localnumber != g_localid)
                g_localid = localnumber;
 
/*
   	x = tmpID;
  	 while (*x != '\0')
     		 *x = (char)_toupper((int) *(x++));
*/                
        memcpy(pSystemInfo->LocalSetting.privateinfo.LocalID,tmpID,15);
        save_localhost_config_to_flash();
        UpdateSystemInfo(); // 因为要重启，所以必须立即保存
        CloseWindow();
        CreateDialogWin2(LT("修改房间号成功,5秒后系统重启!"),MB_NONE,NULL,NULL);
        SDL_Delay(4000);
#ifdef WRT_MORE_ROOM
        open_screen_power(0,0,0);
        SDL_Delay(1000);
#endif
        Reset_sys();
        return 0;

}

void CreateNetConfigWin(){
        g_cur_select_edit = 0;
        g_pre_select_edit = 0;
       // g_input_max_count = 2;
        g_cur_input_count =0;
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;   
        pWindow->EventHandler = NetConfigEventHandler;
        pWindow->NotifyReDraw = DrawNetConfigWin;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        if(pIPNumberPointer == NULL && pIPNumberPointer1 == NULL){
                GetNumberJpgPointer(&pIPNumberPointer,&pIPNumberPointer1);
                GetNumberJpegSizePointer(&pIPNumberSize,&pIPNumberSize1);
        }
        if(pIPAlphabetPointer == NULL && pIPAlphabetPointer1 == NULL){
        	GetAlphabetJpgPointer(&pIPAlphabetPointer,&pIPAlphabetPointer1);
        	GetAlphabetJpegSizePointer(&pIPAlphabetSize,&pIPAlphabetSize1);
        }
        SetAlphabet26and27Key(0);
        SetEditRect();    
        memset(g_input_number2,0,24);
        pWindow->NotifyReDraw2(pWindow);	
}


void DrawNetConfigWin(){
        int i =0; 
        int j =0;
        
        int startx = rc_setnetconfigmenuico[7].x;
        int starty = rc_setnetconfigmenuico[7].y;  
        WRT_Rect rt;
		rt.x = 0;
		rt.y = 0;
		rt.w =1024;
		rt.h = 530;
		FillRect(0xb2136,rt);
		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));  
		
        JpegDisp(rc_setnetconfigmenuico[9].x,rc_setnetconfigmenuico[9].y,bt_fanhui1,sizeof(bt_fanhui1));
        JpegDisp(rc_setnetconfigmenuico[10].x,rc_setnetconfigmenuico[10].y,bt_guanping1,sizeof(bt_guanping1)); 
       // JpegDisp(rc_setnetconfigmenuico[6].x,rc_setnetconfigmenuico[6].y,menu_zidongpeizhi1,sizeof(menu_zidongpeizhi1));
      //  JpegDisp(rc_setnetconfigmenuico[7].x,rc_setnetconfigmenuico[7].y,menu_shoudongpeizhi1,sizeof(menu_shoudongpeizhi1)); 

      //  DrawEnText(LT2("Automatic"),rc_setnetconfigmenuico[6].w + 21,rc_setnetconfigmenuico[6].h,rc_setnetconfigmenuico[6].x,rc_setnetconfigmenuico[6].y);
     //   DrawEnText(LT2("Manual"),rc_setnetconfigmenuico[7].w + 21,rc_setnetconfigmenuico[7].h,rc_setnetconfigmenuico[7].x,rc_setnetconfigmenuico[7].y);
        
       for(i = 0;i<4;i++){
        for(j =0; j<7;j++){
        JpegDisp(startx+(j)*100,starty+(i)*100, (unsigned char*)pIPAlphabetPointer[(i*7)+j],pIPAlphabetSize[(i*7)+j]);
        }
        //tm_wkafter(10);
        }
        startx = rc_setnetconfigmenuico[8].x;
        starty = rc_setnetconfigmenuico[8].y;         
        for(i=0; i<4;i++)
                for(j=0; j<3;j++){
                        JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pIPNumberPointer[(i*3)+j],pIPNumberSize[(i*3)+j]);
                }

                memset(g_input_number,0,24);
                memset(g_input_number2,0,24);
                g_input_number[0][0] = pSystemInfo->LocalSetting.privateinfo.LocalID[0];
                g_input_number[0][1] = pSystemInfo->LocalSetting.privateinfo.LocalID[1];

                g_input_number[1][0] = pSystemInfo->LocalSetting.privateinfo.LocalID[2];
                g_input_number[1][1] = pSystemInfo->LocalSetting.privateinfo.LocalID[3];
                g_input_number[1][2] = pSystemInfo->LocalSetting.privateinfo.LocalID[4];

                g_input_number[2][0] = pSystemInfo->LocalSetting.privateinfo.LocalID[5];
                g_input_number[2][1] = pSystemInfo->LocalSetting.privateinfo.LocalID[6];

                g_input_number[3][0] = pSystemInfo->LocalSetting.privateinfo.LocalID[7];
                g_input_number[3][1] = pSystemInfo->LocalSetting.privateinfo.LocalID[8];
                g_input_number[3][2] = pSystemInfo->LocalSetting.privateinfo.LocalID[9];

                g_input_number[4][0] = pSystemInfo->LocalSetting.privateinfo.LocalID[10];
                g_input_number[4][1] = pSystemInfo->LocalSetting.privateinfo.LocalID[11];
                g_input_number[4][2] = pSystemInfo->LocalSetting.privateinfo.LocalID[12];    

                g_input_number[5][0] = pSystemInfo->LocalSetting.privateinfo.LocalID[13];
                g_input_number[5][1] = pSystemInfo->LocalSetting.privateinfo.LocalID[14];



                DrawText_16(LT("请输入15位房间编号(2位区号-3位栋号-2位单元号-3位层号-3位房号-2位分机编号)"),10,rc_setnetconfigmenuico[0].y-15,rgb_red);
                g_cur_input_count = g_input_max_count[g_cur_select_edit];
				DrawEditViewContent(0);
                DrawSettingsIcon(0,0);
                DrawSettingsIcon(1,0);
                
                /*
                DrawText_16(LT("区号:"),rc_setnetconfigmenuico[0].x-85,rc_setnetconfigmenuico[0].y+20,rgb_white);
                DrawRect(rc_setnetconfigmenuico[0],2,rgb_white);
                g_cur_input_count = strlen(g_input_number[0]);
                //DrawText_16(g_input_number[0],rc_setnetconfigmenuico[0]+3,rc_setnetconfigmenuico[0].y+25,rgb_white);

                DrawText_16(LT("栋号:"),rc_setnetconfigmenuico[1].x-70,rc_setnetconfigmenuico[1].y+20,rgb_white);
                DrawRect(rc_setnetconfigmenuico[1],2,rgb_white);


                DrawText_16(LT("单元号:"),rc_setnetconfigmenuico[2].x-85,rc_setnetconfigmenuico[2].y+20,rgb_white);
                DrawRect(rc_setnetconfigmenuico[2],2,rgb_white);


                DrawText_16(LT("层号:"),rc_setnetconfigmenuico[3].x-85,rc_setnetconfigmenuico[3].y+20,rgb_white);
                DrawRect(rc_setnetconfigmenuico[3],2,rgb_white);

                DrawText_16(LT("房间号:"),rc_setnetconfigmenuico[4].x-70,rc_setnetconfigmenuico[4].y+20,rgb_white);
                DrawRect(rc_setnetconfigmenuico[4],2,rgb_white);

                DrawText_16(LT("分机编号:"),rc_setnetconfigmenuico[5].x-85,rc_setnetconfigmenuico[5].y+20,rgb_white);
                DrawRect(rc_setnetconfigmenuico[5],2,rgb_white);

                DrawRect(rc_setnetconfigmenuico[g_pre_select_edit],2,rgb_white);
                DrawRect(rc_setnetconfigmenuico[g_cur_select_edit],2,rgb_yellow);

                for(i= 0; i< 6;i++){
                        DrawText_16(g_input_number[i],rc_setnetconfigmenuico[i].x+3,rc_setnetconfigmenuico[i].y+25,rgb_white);
                }
                */

                WRT_Rect curbox;
                curbox.x = 135;
                curbox.y = 531;
                curbox.w = 578;
                curbox.h = 55;
	        	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));
	        	
                DrawText_16(LT("提示:按'*'退格，按'#'确认"),curbox.x,curbox.y+45,rgb_black);	       
}

int  NetConfigEventHandler(int x,int y ,int status){
        WRT_xy xy;
        int ret = 0;
        int tmpret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<11;index++){
                if(IsInside(rc_setnetconfigmenuico[index],xy)){
                        pos_x = rc_setnetconfigmenuico[index].x;
                        pos_y = rc_setnetconfigmenuico[index].y;
                        ret = 1;         
                        switch(index){  
                        	/*
                case 6:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_zidongpeizhi2,sizeof(menu_zidongpeizhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_zidongpeizhi1,sizeof(menu_zidongpeizhi1));   
                                unsigned long tmpevent[4];
                                tmpevent[0] = ROOM_ASKFOR_SYSINFO;
                                tmpevent[1] = tmpevent[2] = tmpevent[3] = 0;
                                wrthost_send_cmd(tmpevent);
                                CreateDialogWin2(LT("正在向中心请求配置...."),MB_NONE,NULL,NULL);
                        }                    
                        break;
                case 7:
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,menu_shoudongpeizhi2,sizeof(menu_shoudongpeizhi2));
                        }else{
                                JpegDisp(pos_x,pos_y,menu_shoudongpeizhi1,sizeof(menu_shoudongpeizhi1));  
                                CreateModifyIPWin(0);                           
                        }                    
                        break;    
                 */   
                case 9: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                CloseTopWindow();                   
                        }
                        break;
                case 10:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close(); 
                                CloseAllWindows();
                        }
                        break;
                case 0:
                	break;
                	if(status == 1){
                		WRT_Rect tmprect = rc_setnetconfigmenuico[0];
                		DrawRect(tmprect,2,rgb_yellow);
                	}else{
                		WRT_Rect tmprect = rc_setnetconfigmenuico[0];
                		DrawRect(tmprect,2,rgb_white);
                	}
                	break;
                case 8://数字
                	 ret = setnetconfighandler(x,y,status);	  
                        break;
                case 7://字母
                	ret = alphabethandler_modifyid(x,y,status);
                	break;
                default:
                	g_cur_select_edit = index - 1;
                	g_cur_input_count = g_input_max_count[g_cur_select_edit]; 
                	memset(g_input_number2[g_cur_select_edit],0,4);
                    g_isUpdated = 0;
                    DrawEditViewContent(1);
                    update_rect(rc_setnetconfigmenuico[0].x,rc_setnetconfigmenuico[0].y,rc_setnetconfigmenuico[0].w,rc_setnetconfigmenuico[0].h);
                    g_isUpdated = 1;
                	             																																																																																										g_isUpdated = 1;
                	break;
                }
                /*
                case 8:
                        //进入数字区域
                        ret = setnetconfighandler(x,y,status);	  
                        break;
                default:
                        if(index >= 0 && index < 6){
                                g_cur_input_count = strlen(g_input_number[index]);
                                switch(index){
                case 0:
                        g_input_max_count = 2;
                        break;
                case 1:
                        g_input_max_count = 3;
                        break;
                case 2:
                        g_input_max_count = 2;
                        break;
                case 3:
                        g_input_max_count = 3;
                        break;
                case 4:
                        g_input_max_count = 3;
                        break;
                case 5:
                        g_input_max_count = 2;
                        break;
                                }

                                g_pre_select_edit = g_cur_select_edit;
                                g_cur_select_edit = index;
                                DrawRect(rc_setnetconfigmenuico[g_pre_select_edit],2,rgb_white);
                                DrawRect(rc_setnetconfigmenuico[g_cur_select_edit],2,rgb_yellow);
                        }
                        break;
                        }
                  
                        break;
                */                        
                }
        }      
        return ret;  		    
}


//==========================2014-11-11
//=======================================================
int g_ssid_pwd_flag = 0;//0==ssid 1== pwd
int g_is_daxie_flag = 0;
//字母表
char g_alphabet_2[26] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q',
'r','s','t','u','v','w','x','y','z'};
extern char g_Onvif_Authname[16];
extern int  g_Onvif_Authname_count;

extern char g_Onvif_Authpasswd[16];
extern int  g_Onvif_Authpasswd_count;

static void DrawWifiSettingsIcon(int index,int offset)
{
	
	WRT_Rect tmprect[2];
	offset = 0;
	tmprect[0].x = rc_setnetconfigmenuico[7].x+5*100 + 2 - offset;
	tmprect[0].y = rc_setnetconfigmenuico[7].y + 3*100 -offset;
	tmprect[0].w = 88;
	tmprect[0].h = 79;
	tmprect[1] = tmprect[0];
	tmprect[1].x += 100;
	if(index == 0)
	{
		if(GetCurrentLanguage() == ENGLISH)
			DrawTextToRect2(tmprect[0],"Cap",rgb_yellow,1);
		else
			DrawTextToRect2(tmprect[0],"大写",rgb_yellow,1);
		DrawTextToRect2(tmprect[1],"-",rgb_yellow,1);
	}
	else if(index == 1)
	{
		if(g_is_daxie_flag)
			DrawTextToRect2(tmprect[1],"-",rgb_yellow,1);
		else
			DrawTextToRect2(tmprect[1],"_",rgb_yellow,1);

	}else if(index == 2)
	{
		if(GetCurrentLanguage() == ENGLISH)
			DrawTextToRect2(tmprect[0],"Lower",rgb_yellow,1);
		else
			DrawTextToRect2(tmprect[0],"小写",rgb_yellow,1);
		DrawTextToRect2(tmprect[1],"_",rgb_yellow,1);
		
	}
}

void DrawSsidPwdEdit()
{
	WRT_Rect tmprect;
    memset(g_editcontent,0,100);

    tmprect.x = rc_setnetconfigmenuico[0].x;
    tmprect.y = rc_setnetconfigmenuico[0].y;
    tmprect.w = 800;
    tmprect.h = 50;
    DrawRect(tmprect,2,rgb_white);
    DeflateRect(&tmprect,4);
    FillRect(rgb_white,tmprect);

    if(g_ssid_pwd_flag == 0)
	{
		DrawText(wifi_ssid,rc_setnetconfigmenuico[0].x+20,rc_setnetconfigmenuico[0].y+36,rgb_black,24);
		//printf("ssid %s\n",wifi_ssid);
	}else if(g_ssid_pwd_flag == 1)
	{
		DrawText(wifi_pwd,rc_setnetconfigmenuico[0].x+20,rc_setnetconfigmenuico[0].y+36,rgb_black,24);
		//printf("pwd %s\n",wifi_pwd);
	}else if(g_ssid_pwd_flag == 2)
	{
		DrawText(wifi_authmode,rc_setnetconfigmenuico[0].x+20,rc_setnetconfigmenuico[0].y+36,rgb_black,24);
		//printf("authmode %s\n",wifi_authmode);
	}else if(g_ssid_pwd_flag == 3)
	{
		DrawText(wifi_encryptype,rc_setnetconfigmenuico[0].x+20,rc_setnetconfigmenuico[0].y+36,rgb_black,24);
		//printf("encryptype %s\n",wifi_encryptype);
	}else if(g_ssid_pwd_flag == ONVIF_AUTH_NAME)
	{
		DrawText(g_Onvif_Authname,rc_setnetconfigmenuico[0].x+20,rc_setnetconfigmenuico[0].y+36,rgb_black,24);
		//printf("g_Onvif_Authname %s\n",g_Onvif_Authname);
	}else if(g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
	{
		DrawText(g_Onvif_Authpasswd,rc_setnetconfigmenuico[0].x+20,rc_setnetconfigmenuico[0].y+36,rgb_black,24);
		//printf("g_Onvif_Authpasswd %s\n",g_Onvif_Authpasswd);
	}
}

static int setssidpwdhandler(int x,int y,int status){
        int i= 0;
        int j =0;
        int ret = 0;
        int tempx  =0;
        int tempy = 0;
        int count =0;
        int tmpY =  0;
        char* pTemp = 0;
        int offset = 0;
        int offset2 = 0;
        int startx = rc_setnetconfigmenuico[8].x;
        int starty = rc_setnetconfigmenuico[8].y; 
        char c[2];
        c[0] = '\0';
        c[1] = '\0';

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
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer1[count],pIPNumberSize1[count]);	
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPNumberPointer[count],pIPNumberSize[count]);
                                        if(count != 9 && count != 11){ 
                                        	if(g_ssid_pwd_flag == 0 || g_ssid_pwd_flag == 1 || \
                                        		g_ssid_pwd_flag == ONVIF_AUTH_NAME || g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
                                        	{
	                                            if(g_cur_input_count < 16){        	 	 
	                                            	char c[2];
	                                            	c[0] = '\0';
	                                            	c[1] = '\0';
	                                                c[0] = g_number[count];
	                                                if(g_ssid_pwd_flag == 1)
	                                                    strcat(wifi_pwd,c);
	                                                else if(g_ssid_pwd_flag == 0)
	                                                	strcat(wifi_ssid,c);
	                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_NAME)
	                                                	strcat(g_Onvif_Authname,c);
	                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
	                                                	strcat(g_Onvif_Authpasswd,c);
	                                                g_cur_input_count++;
	                                            }
                                            }else if(g_ssid_pwd_flag == 2 || g_ssid_pwd_flag == 3)
                                            {
                                            	if(g_cur_input_count < 8){        	 	 
	                                            	char c[2];
	                                            	c[0] = '\0';
	                                            	c[1] = '\0';
	                                                c[0] = g_number[count];
	                                                if(g_ssid_pwd_flag == 2)
	                                                    strcat(wifi_authmode,c);
	                                                else if(g_ssid_pwd_flag == 3)
	                                                	strcat(wifi_encryptype,c);
	                                                g_cur_input_count++;
	                                            }
                                            }
                                            
                                            g_isUpdated = 0;
                                            DrawSsidPwdEdit();
                                            update_rect(rc_setnetconfigmenuico[0].x,rc_setnetconfigmenuico[0].y,rc_setnetconfigmenuico[0].w,rc_setnetconfigmenuico[0].h);
                                            g_isUpdated = 1;
                                        }
                                        if(count == 9){ //*
                                    		if(g_cur_input_count >= 1)
                                    		{
                                                if(g_ssid_pwd_flag == 1)
                                                	wifi_pwd[g_cur_input_count-1] = '\0';
                                                else if(g_ssid_pwd_flag == 0)
                                                	wifi_ssid[g_cur_input_count-1] = '\0';
                                                else if(g_ssid_pwd_flag == 2)
                                                	wifi_authmode[g_cur_input_count-1] = '\0';
                                                else if(g_ssid_pwd_flag == 3)
                                                	wifi_encryptype[g_cur_input_count-1] = '\0';
                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_NAME)
	                                                g_Onvif_Authname[g_cur_input_count-1] = '\0';
                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
	                                                g_Onvif_Authpasswd[g_cur_input_count-1] = '\0';
	                                                
                                                g_cur_input_count--;
                                                
                                                //printf("222 g_len = %d  %s %s\n",g_cur_input_count,wifi_ssid,wifi_pwd);
 												g_isUpdated = 0;
                                                DrawSsidPwdEdit();
                                                update_rect(0,0,1024,150);	
                                                g_isUpdated = 1;
                                            }
                                        }
                                        //if(count == 11){//#
                                        //        CreateDialogWin2(LT("你确定要修改房间编号？"),MB_OK|MB_CANCEL,modify_room_number,NULL);
                                        //}
                                }
                                break;
                        }
                }
                return ret;
}
//字母表的处理
static int alphabethandler_ssidpwd(int x,int y,int status){
        int i= 0;
        int j =0;
        int tempx  =0;
        int tempy = 0;
        int ret = 0;
        int count =0;
        int startx = rc_setnetconfigmenuico[7].x;
        int starty = rc_setnetconfigmenuico[7].y;
        char c[2];
        c[0] = '\0';
        c[1] = '\0';
        for(i=0; i<4;i++)
                for(j=0; j< 7;j++){
                        tempx = startx+(j)*100;
                        tempy = starty+(i)*100;
                        if(x > tempx && x < (tempx+88) && y > tempy  && y < (tempy+79)){
                                count = (i*7)+j;
                                ret  = 1;
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
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPAlphabetPointer1[count],pIPAlphabetSize1[count]);
										
                                        if(count == 26)
                                        	DrawWifiSettingsIcon(0,3);
                                        if(count == 27)
                                        	DrawWifiSettingsIcon(1,3);
                                }else{
                                        JpegDisp(tempx,tempy, (unsigned char*)pIPAlphabetPointer[count],pIPAlphabetSize[count]);

										
										
                                        if(count < 26){
                                        	if(g_ssid_pwd_flag == 0 || g_ssid_pwd_flag == 1 || \
                                        		g_ssid_pwd_flag == ONVIF_AUTH_NAME || g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
                                        	{
                                                if(g_cur_input_count < 16){
	                                            	char c[2];
	                                            	c[0] = '\0';
	                                            	c[1] = '\0';
	                                            	
	                                            	if(g_is_daxie_flag == 1)
	                                            		c[0] = g_alphabet[count];
	                                            	else
	                                                	c[0] = g_alphabet_2[count];
	                                                	
	                                                if(g_ssid_pwd_flag == 1)
	                                                    strcat(wifi_pwd,c);
	                                                else if(g_ssid_pwd_flag == 0)
	                                                	strcat(wifi_ssid,c);
	                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_NAME)
	                                                	strcat(g_Onvif_Authname,c);
	                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
	                                                	strcat(g_Onvif_Authpasswd,c);
	                                                
	                                                g_cur_input_count++;
	                                                //printf("AAA len = %d %s %s\n",g_cur_input_count,wifi_ssid,wifi_pwd);
                                           		 }
                                           	}else if(g_ssid_pwd_flag == 2 || g_ssid_pwd_flag == 3)
                                           	{
                                           		if(g_cur_input_count < 8){
	                                            	char c[2];
	                                            	c[0] = '\0';
	                                            	c[1] = '\0';
	                                                if(g_is_daxie_flag == 1)
	                                            		c[0] = g_alphabet[count];
	                                            	else
	                                                	c[0] = g_alphabet_2[count];
	                                                if(g_ssid_pwd_flag == 2)
	                                                    strcat(wifi_authmode,c);
	                                                else if(g_ssid_pwd_flag == 3)
	                                                	strcat(wifi_encryptype,c);
	                                                g_cur_input_count++;
	                                                //printf("AAA222 len = %d %s %s\n",g_cur_input_count,wifi_ssid,wifi_pwd);
                                           		 }
                                           	}
                                            g_isUpdated = 0;
                                            DrawSsidPwdEdit();
                                            update_rect(0,0,1024,150);	
                                            g_isUpdated = 1;
                                        }else if(count == 26){//大小写切换

                                        	g_is_daxie_flag = !g_is_daxie_flag;

											if(count == 26)
												JpegDisp(tempx+100,tempy, (unsigned char*)pIPAlphabetPointer[count+1],pIPAlphabetSize[count+1]);
											
                                        	if(g_is_daxie_flag)
                                        		DrawWifiSettingsIcon(0,0);
                                        	else
                                        		DrawWifiSettingsIcon(2,0);

                                            g_isUpdated = 0;
                                            DrawSsidPwdEdit();
                                            update_rect(0,0,1024,150);	
                                            g_isUpdated = 1;
                                              
                                        }else if(count == 27){
                                             DrawWifiSettingsIcon(1,0);
                                             if(g_ssid_pwd_flag == 0 || g_ssid_pwd_flag == 1|| \
                                        		g_ssid_pwd_flag == ONVIF_AUTH_NAME || g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
                                        	 {
                                                if(g_cur_input_count < 16){
	                                            	char c[2];
	                                            	c[0] = '\0';
	                                            	c[1] = '\0';
													if(g_is_daxie_flag)
	                                                	c[0] = '-';
													else
														c[0] = '_';
	                                                if(g_ssid_pwd_flag == 1)
	                                                    strcat(wifi_pwd,c);
	                                                else if(g_ssid_pwd_flag == 0)
	                                                	strcat(wifi_ssid,c);
	                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_NAME)
	                                                	strcat(g_Onvif_Authname,c);
	                                                else if(g_ssid_pwd_flag == ONVIF_AUTH_PASSWD)
	                                                	strcat(g_Onvif_Authpasswd,c);
	                                                	
	                                                g_cur_input_count++;
	                                                //printf("AAA len = %d %s %s\n",g_cur_input_count,wifi_ssid,wifi_pwd);
                                           		 }
                                           	}else if(g_ssid_pwd_flag == 2 || g_ssid_pwd_flag == 3)
                                           	{
                                           		if(g_cur_input_count < 8){
	                                            	char c[2];
	                                            	c[0] = '\0';
	                                            	c[1] = '\0';
	                                                if(g_is_daxie_flag)
	                                                	c[0] = '-';
													else
														c[0] = '_';
	                                                if(g_ssid_pwd_flag == 2)
	                                                    strcat(wifi_authmode,c);
	                                                else if(g_ssid_pwd_flag == 3)
	                                                	strcat(wifi_encryptype,c);
	                                                g_cur_input_count++;
	                                                //printf("AAA222 len = %d %s %s\n",g_cur_input_count,wifi_ssid,wifi_pwd);
                                           		 }
                                           	}
                                            g_isUpdated = 0;
                                            DrawSsidPwdEdit();
                                            update_rect(0,0,1024,150);	
                                            g_isUpdated = 1;
                                        }
                                }
                                break;
                        }
                }
                return ret;
}

void DrawSsidPwdWin(){
	int i =0; 
	int j =0;

	int startx = rc_setnetconfigmenuico[7].x;
	int starty = rc_setnetconfigmenuico[7].y;  
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w = 1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));  

	JpegDisp(rc_setnetconfigmenuico[9].x,rc_setnetconfigmenuico[9].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_setnetconfigmenuico[10].x,rc_setnetconfigmenuico[10].y,bt_guanping1,sizeof(bt_guanping1)); 

	for(i = 0;i<4;i++){
		for(j =0; j<7;j++){
			JpegDisp(startx+(j)*100,starty+(i)*100, (unsigned char*)pIPAlphabetPointer[(i*7)+j],pIPAlphabetSize[(i*7)+j]);
		}
	}
	startx = rc_setnetconfigmenuico[8].x;
	starty = rc_setnetconfigmenuico[8].y;         
	for(i=0; i<4;i++)
		for(j=0; j<3;j++){
			JpegDisp(startx+(j)*(100),starty+(i)*(100), (unsigned char*)pIPNumberPointer[(i*3)+j],pIPNumberSize[(i*3)+j]);
		}

	memset(g_input_number,0,24);
	memset(g_input_number2,0,24);

	DrawSsidPwdEdit();
	DrawWifiSettingsIcon(0,0);
	DrawWifiSettingsIcon(1,0);

	WRT_Rect curbox;
	curbox.x = 135;
	curbox.y = 531;
	curbox.w = 578;
	curbox.h = 55;
	JpegDisp(130,531,menu_sub_back4,sizeof(menu_sub_back4));

	DrawText_16(LT("提示:按'*'退格，按'#'确认"),curbox.x,curbox.y+45,rgb_black);	       
}

int  SsidPwdEventHandler(int x,int y ,int status){
        WRT_xy xy;
        int ret = 0;
        int tmpret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<11;index++){
                if(IsInside(rc_setnetconfigmenuico[index],xy)){
                        pos_x = rc_setnetconfigmenuico[index].x;
                        pos_y = rc_setnetconfigmenuico[index].y;
                        ret = 1;         
                        switch(index){  
                        	
                case 9: /*返回*/
                        if(status == 1){
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));           
                                CloseTopWindow();                   
                        }
                        break;
                case 10:/*关屏*/
                        if(status == 1){ //按下
                                StartButtonVocie();
                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
                        }else{
                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
                                screen_close(); 
                                CloseAllWindows();
                        }
                        break;
                case 0:
                	break;
                	if(status == 1){
                		WRT_Rect tmprect = rc_setnetconfigmenuico[0];
                		DrawRect(tmprect,2,rgb_yellow);
                	}else{
                		WRT_Rect tmprect = rc_setnetconfigmenuico[0];
                		DrawRect(tmprect,2,rgb_white);
                	}
                	break;
                case 8://数字
                	 ret = setssidpwdhandler(x,y,status);	  
                        break;
                case 7://字母
                	ret = alphabethandler_ssidpwd(x,y,status);
                	break;
                default:
                	g_cur_select_edit = 0;
                	
                    g_isUpdated = 0;
                    DrawSsidPwdEdit();
                    g_isUpdated = 1;
                	update_rect(0,0,1024,150);																																																																																									g_isUpdated = 1;
                	break;
                }
                                     
                }
        }      
        return ret;  		    
}

void CreateSsidPwdWin(int type){
        g_cur_select_edit = 0;
        g_pre_select_edit = 0;
        g_cur_input_count = 0;
        g_ssid_pwd_flag = type;
       	g_is_daxie_flag = 1;
        if(type == 0)
        	g_cur_input_count = strlen(wifi_ssid);
        else if(type == 1)
        	g_cur_input_count = strlen(wifi_pwd);
        else if(type == 2)
        	g_cur_input_count = strlen(wifi_authmode);
        else if(type == 3)
        	g_cur_input_count = strlen(wifi_encryptype);
        else if(type == ONVIF_AUTH_NAME)
        	g_cur_input_count = strlen(g_Onvif_Authname);
        else if(type == ONVIF_AUTH_PASSWD)
        	g_cur_input_count = strlen(g_Onvif_Authpasswd);

        
        Window* pWindow = (Window*)New_Window();
        if(pWindow == NULL)
                return ;
        pWindow->EventHandler = SsidPwdEventHandler;
        pWindow->NotifyReDraw = DrawSsidPwdWin;
        pWindow->valid = 1;
        pWindow->type = CHILD;
        WindowAddToWindowList(pWindow);  	
        if(pIPNumberPointer == NULL && pIPNumberPointer1 == NULL){
                GetNumberJpgPointer(&pIPNumberPointer,&pIPNumberPointer1);
                GetNumberJpegSizePointer(&pIPNumberSize,&pIPNumberSize1);
        }
        if(pIPAlphabetPointer == NULL && pIPAlphabetPointer1 == NULL){
        	GetAlphabetJpgPointer(&pIPAlphabetPointer,&pIPAlphabetPointer1);
        	GetAlphabetJpegSizePointer(&pIPAlphabetSize,&pIPAlphabetSize1);
        }
        SetAlphabet26and27Key(0);
        
        pWindow->NotifyReDraw2(pWindow);
}

//======================================================
