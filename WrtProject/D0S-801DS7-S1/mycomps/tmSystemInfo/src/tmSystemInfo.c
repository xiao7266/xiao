/*
* The contents of this file are subject to the Mozilla Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS
* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
* implied. See the License for the specific language governing
* rights and limitations under the License.
*
*
* The Initial Developer of the Original Code is Cisco Systems Inc.
* Portions created by Cisco Systems Inc. are
* Copyright (C) Cisco Systems Inc. 2001.  All Rights Reserved.
*
* Contributor(s):
*              Bill May        wmay@cisco.com
*/
/*
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

#include "tmSysConfig.h"
#include "SDL_thread.h"
#include "CmdId.h"
#define cfg_dir1 "/customer/wrt"
#define cfg_dir2 "/customer/wrt/config"
#define cfg_name "/customer/wrt/config/cfg"
#define cfg_name2 "/customer/wrt/config/morecfg"
#define mac_name  "/customer/wrt/config/macsn"
#define ver_name "/customer/wrt/version"

#define CFG_FILE 0x01
#define PHOTO_FILE 0x02
#define VOICE_FILE 0X04
#define JIAJU_FILE 0X08

#define HEAD_LENGTH	             (4/*协议头WRTI*/+2/*指令*/+4/*指令长度*/)   //指令头长度


static T_SYSTEMINFO  g_SystemInfo;
static T_MACINFO g_macinfo;

static unsigned long  tid_writeflash;
static SDL_sem*   g_writesem = NULL;
static int        g_writetask_exit = 0;
static int        g_iswritefileflag = 0;
static pthread_mutex_t  g_WriteMutext ;

//--------------------------------
//20400
/*mac_file  文件说明
//共7个字节，第一个字节自己为从PC获取MAC的标志为0:需要获取1:不需要
//第二个字节到第七个字节为MAC地址
//
*/

#define CENTERIP g_SystemInfo.LocalSetting.publicinfo.CenterMajorIP
#define LOCALID  g_SystemInfo.LocalSetting.privateinfo.LocalID



void  MacinfoRestoreDefault()
{
    T_MACINFO  *pmac=&g_macinfo;
    memset(pmac,0,sizeof(T_MACINFO));
    write_macsn_file();
}

void write_macsn_file()  //mac_name
{
	FILE *mac_fd;
	int ret =0;

	if((mac_fd = fopen(mac_name, "wb")) == NULL)
	{
		printf("无法创建 MAC文件\n");
	}
	else
	{
		char* p = (char *)&g_macinfo;
		
		int wl=0;
		fseek(mac_fd, 0, SEEK_SET);
		wl = fwrite( p, sizeof(T_MACINFO ) , 1, mac_fd);
		printf("wl = %d \n",wl);
		if(wl != 1)
		{
		    printf("写MAC文件错误:%x\n",errno);
		}

		fflush(mac_fd);
		fclose(mac_fd);
		mac_fd = NULL;   
	}
		
}

void read_macsn_file()
{ 
	DIR *dirp = NULL;
	FILE* mac_fd = NULL;

	if((mac_fd=fopen(mac_name, "rb"))==NULL)
	{
		printf("mac文件不存在，创建新文件1\n");
		//mac文件不存在，创建新文件
		MacinfoRestoreDefault();
	}
	else
	{
		int bytes_read;

		char* p = (char *)&g_macinfo;

		fseek(mac_fd, 0, SEEK_SET);

		bytes_read=fread(p, sizeof(g_macinfo), 1, mac_fd);		
		if(bytes_read != 1){
			fclose(mac_fd);
			mac_fd = NULL;
			unlink(mac_name);
			printf("阅读MAC文件出错，以默认方式设置\n");
			MacinfoRestoreDefault();
			return;
		}
		fclose(mac_fd);
		mac_fd = NULL;
	}
}

void setmacsnFileMAC(char *sMac)
{
	//strncpy(g_macinfo.mac,sMac,6);
	memcpy((char *)&g_macinfo.mac,sMac,6);
	write_macsn_file();
}

void getmacsnFileMAC(char *sMac)
{
	read_macsn_file();
	memcpy(sMac,&g_macinfo.mac,6);
}

void getmacsnFileSN(char *sSn)
{
    read_macsn_file();
    memcpy(sSn,&g_macinfo.sn,32);
}

void setmacsnFileSN(char *sSn)
{
	strncpy((char *)&g_macinfo.sn,sSn,32);
	write_macsn_file();
}

void setmacsnFileFlag(char *flag)
{
	memcpy(&g_macinfo.flag,flag,1);
	write_macsn_file();	 
}
void getmacsnFileFlag(char *flag)
{
	read_macsn_file();
	memcpy(flag,&g_macinfo.flag,1);
}

void setRoomSN(char *sSn)
{
	//strcpy(g_SystemInfo.BootInfo.SN,sSn); //修改，写macsn文件使用strcpy替换了写好的mac
	memcpy(&g_SystemInfo.BootInfo.SN,sSn, 32);
	write_cfg_file();
}

void setRoomMAC(char *sMac)
{
	read_cfg_file();
	memcpy(&g_SystemInfo.BootInfo.MAC,sMac, 6);
	write_cfg_file();
}

void updateDoorMAC(char *sMac)
{
	char cmd[128];
	memset(cmd,0,128);
	system("ifconfig eth0 down");
	sprintf(cmd,"ifconfig eth0 hw ether %x:%x:%x:%x:%x:%x",sMac[0],sMac[1],sMac[2],sMac[3],sMac[4],sMac[5]);
	system(cmd);
	system("ifconfig eth0 up");
}

void updateRoomMACFromCfg()
{
	char sMac[6];
	char cmd[128];

	memcpy(sMac,&g_SystemInfo.BootInfo.MAC, 6);

	memset(cmd,0,128);
	system("ifconfig eth0 down");
	sprintf(cmd,"ifconfig eth0 hw ether %x:%x:%x:%x:%x:%x",sMac[0],sMac[1],sMac[2],sMac[3],sMac[4],sMac[5]);
	system(cmd);
	system("ifconfig eth0 up");
}


int g_rectimer;
static char g_dst_id[16]="";

int task_macsnfile(void *pParam)
{
	printf("task_macsnfile successed!\n");
	struct sockaddr_in s_addr, c_addr;
	int sd;
	int ret; 
	int len;
	int mode=1;    //工作在非阻塞
	
	mNetPack np={0};
	socklen_t addr_len;
	char sn[32],mac[6],flag;

	g_rectimer=0;	
	sd= socket(AF_INET, SOCK_DGRAM, 0);

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(20400);
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY;


	c_addr.sin_family = AF_INET;
	c_addr.sin_port   = htons(20400);
	c_addr.sin_addr.s_addr = CENTERIP;

	if(sd<0 )
	{
		printf("creat UDP socket failed!\n");
	}
	strncpy(np.head,"WRTI",4);
	memcpy(np.srcid,LOCALID,15);
	memcpy(np.dstid,g_dst_id,15);

	np.uCmd = DOOR_REQ_MAC;
	np.len = HEAD_LENGTH;//4+4+2

	ret = bind(sd,(struct sockaddr *)&s_addr,sizeof(struct sockaddr_in));
	if(ret==-1)
	{
		printf(" udp bind port error1!\n");
		return -1;
	}

	ret = sendto(sd,&np,np.len,0,(struct sockaddr *)&c_addr,sizeof(s_addr));
	if(ret==-1)
	{
		printf(" udp send data error1!\n");
		return -1;
	}

	ioctl(sd,FIONBIO,(char*)&mode);

	while(g_rectimer<10*50)  //25S
	{
		len = recvfrom(sd, &np, sizeof(np)-1, 0, (struct sockaddr *)&s_addr, &addr_len);
		//printf("req mac thread receive data\n");
		if(len>0)
		{
			if(np.uCmd==CENTER_SET_MAC)
			{
				printf("1: have receive the CENER_SET_MAC !\n ");

				flag=1;
				memcpy(sn,&np.DATA[0],32);
				memcpy(mac,&np.DATA[32],6);
				setmacsnFileFlag(&flag);
				setmacsnFileMAC(mac);
				setmacsnFileSN(sn);
				setRoomMAC(mac);
				setRoomSN(sn);
				updateDoorMAC(mac);

				strncpy(np.head,"WRTI",4);
				memcpy(np.srcid,LOCALID,15);
				memcpy(np.dstid,g_dst_id,15);
				np.uCmd = DOOR_SET_MAC_ACK;
				np.len = 40+1;
				np.DATA[0]=1;
				ret = sendto(sd,&np,np.len,0,(struct sockaddr *)&c_addr,sizeof(s_addr));

				break;
			}
		}
		else
		{
			g_rectimer++;
			SDL_Delay(50);//50x50x10 
			continue;
		}
		break;
	}
	if(g_rectimer>=10*50)
		printf("1:failed to receive CENER_SET_MAC!\n");
	printf("req mac thread success\n");

	close(sd);
}

int severce_macsnfile(void *pParam)
{
	int m_Socket;
	int len;
	int result;
	socklen_t addr_len;
	fd_set readfd;
	unsigned long maxfd=0;
	struct timeval timeout;
	mNetPack np={0};
	struct sockaddr_in s_addr, c_addr;
	char sn[32],mac[6],flag;
	int ret; 
	

	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	int on = 1;
	ret = setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(20400);
	s_addr.sin_addr.s_addr = INADDR_ANY;


	c_addr.sin_family = AF_INET;
	c_addr.sin_port = htons(20400);
	c_addr.sin_addr.s_addr = CENTERIP;
	printf("severce_macsnfile successed!\n");


	ret = bind(m_Socket, (struct sockaddr *) &s_addr, sizeof(s_addr));
	if(ret == -1)
	{
		printf(" udp bind port error2!\n");
		return -1;
	}
    while(1)
    {	
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;
		FD_ZERO(&readfd);
    	FD_SET(m_Socket, &readfd);
    	maxfd = (m_Socket+1)>maxfd?(m_Socket+1):maxfd;
    	result = select(maxfd,&readfd, NULL,  NULL, &timeout);
    	
    	if(result)
    	{
    		if(FD_ISSET(m_Socket,&readfd))
        	{
				len = recvfrom(m_Socket, &np, sizeof(np)-1, 0, (struct sockaddr *)&s_addr, &addr_len);
				if(len>0)
				{
					printf("have data coming------\n");
					if(np.uCmd==CENTER_SET_MAC)
					{
						printf("2:have receive the CENER_SET_MAC !\n ");


						memcpy(np.srcid,LOCALID,15);
						memcpy(np.dstid,g_dst_id,15);
						flag=1;
						memcpy(sn,&np.DATA[0],32);
						memcpy(mac,&np.DATA[32],6);
						setmacsnFileFlag(&flag);
						setmacsnFileMAC(mac);
						setmacsnFileSN(sn);
						setRoomMAC(mac);
						setRoomSN(sn);
						updateDoorMAC(mac);
						strncpy(np.head,"WRTI",4);
						memcpy(np.srcid,LOCALID,15);
						memcpy(np.dstid,g_dst_id,15);

						np.uCmd = DOOR_SET_MAC_ACK;
						np.len = 40+1; 
						np.DATA[0]=1;
						ret = sendto(m_Socket,&np,np.len,0,(struct sockaddr *)&c_addr,sizeof(s_addr));
					}
				}
				else
				{
					continue;	
				}
			}
		}
	}
}

//=================end of setMac==============================

int write_flash(unsigned short *addr, unsigned long length, unsigned long toaddr)
{
        return 0;
}

void read_flash(unsigned long  srcAddr, unsigned char*  pDst, unsigned long numBytes)
{

}

void erase_flash(unsigned long startsec, unsigned long endsec)
{

}

T_SYSTEMINFO* GetSystemInfo()
{
        return &g_SystemInfo;
}

void FlushSystemInfo()
{
        UpdateSystemInfo();
}

static void write_cfg_file()
{
        FILE *cfg_fd;
        int ret =0;
        if((cfg_fd = fopen(cfg_name, "wb")) == NULL)
                printf("无法创建配置文件\n");
        else
        {
                char* p = (char*)&g_SystemInfo;
                int totallen = sizeof(g_SystemInfo);
                //printf("totallen = %d \n",totallen);
                int writelen = 0;
                int wl = 0;
                while(1){
                	wl = fwrite( p+writelen,1, totallen - writelen , cfg_fd);
                	if(ferror(cfg_fd)){
                		printf("写配置文件错误:%x\n",errno);
                		break;
                	}
                	writelen += wl;
                	if(writelen  == totallen)
                		break;
                }
                fflush(cfg_fd);

                //printf("write cfg over %d \n",writelen);
                fclose(cfg_fd);
                cfg_fd = NULL;
                system("sync");
        }
}

static void read_cfg_file()
{
        DIR *dirp = NULL;
        FILE* cfg_fd = NULL;
        //查找配置文件目录是否存在
        if((dirp = opendir("/customer")) == NULL){
        	if(mkdir("/customer",1) < 0)
        		printf("error to mkdir /home\n");
        }
        if(dirp != NULL){
        	closedir(dirp);
        	dirp = NULL;
        }
        if((dirp=opendir(cfg_dir1)) == NULL)
        {
                if(mkdir(cfg_dir1, 1) < 0)
                        printf("error to mkdir %s\n", cfg_dir1);
        }
        if(dirp != NULL)
        {
                closedir(dirp);
                dirp = NULL;
        }
        //查找配置文件目录是否存在
        if((dirp=opendir(cfg_dir2)) == NULL)
        {
                if(mkdir(cfg_dir2, 1) < 0)
                        printf("error to mkdir %s\n", cfg_dir2);
        }
        if(dirp != NULL)
        {
                closedir(dirp);
                dirp = NULL;
        }
        //打开配置文件
        if((cfg_fd=fopen(cfg_name, "rb"))==NULL)
        {
                printf("配置文件不存在，创建新文件\n");
                if((cfg_fd = fopen(cfg_name, "wb")) == NULL)
                        printf("无法创建配置文件\n");
                else
                {
                        SetDefaultSystemInfo();
                        UpdateSystemInfo();
                }
        }
        else
        {
                int bytes_read;
                int totalen = sizeof(g_SystemInfo);
                int readlen = 0;
                char* p = (char*)&g_SystemInfo;
                fseek(cfg_fd,0,SEEK_SET);
                fseek(cfg_fd,0,SEEK_END);
                if(ftell(cfg_fd) != totalen){
            		printf("阅读配置文件出错,文件长度为0");
                	fclose(cfg_fd);
                	cfg_fd = NULL;
                	unlink(cfg_name);
                	
                	SetDefaultSystemInfo();
                	UpdateSystemInfo();
                	return;
                }
                fseek(cfg_fd,0,SEEK_SET);
	        	while(1){
                	bytes_read=fread(p+readlen,1,totalen - readlen, cfg_fd);
                	if(ferror(cfg_fd)){
                		printf("阅读配置文件出错，以默认方式设置,errno =%d\n",errno);
                        	fclose(cfg_fd);
                        	cfg_fd = NULL;
                        	unlink(cfg_name);
                        	
                        	SetDefaultSystemInfo();
                        	UpdateSystemInfo();
                        	return;                		
                	}
                	
                	//printf("bytes_read = %d \n",bytes_read);
                	readlen += bytes_read;
                	if(readlen == totalen)
                		break;
                }
				updateRoomMACFromCfg();

                fclose(cfg_fd);
                cfg_fd = NULL;
        }

}

void SetDefaultSystemInfo()
{

        int i =0;
        T_SYSTEMINFO* p = NULL;
        p =&g_SystemInfo;

        strcpy(p->mySysInfo.password, "88888888");
        strcpy(p->mySysInfo.superpassword, "99999999");
        
		FILE *fd = NULL;
		if((fd = fopen(ver_name, "rb")) == NULL)
	    {
	        printf("版本文件不存在\n");
	    }else
	    {	
	    	int count=0;
	    	char inbuf[32];
			fseek(fd, 0, SEEK_SET);
			count = fread(inbuf, 16, 1, fd);
			fclose(fd);
			system("rm /customer/wrt/version");
			system("sync");
			memcpy(p->BootInfo.Version, inbuf+1, 10);
			if(inbuf[15]==0x1)
			{
				p->mySysInfo.screensize = 0x10;
			}else
				p->mySysInfo.screensize = 0x07;
		}
		
        p->BootInfo.SN[31] = 0;
        memset(p->mySysInfo.sysID,0,16);
        memset(p->mySysInfo.gatewaypwd,0,16);
        memset(p->mySysInfo.gatewayroomid,0,16);
        p->mySysInfo.gatewayip = 0;
        memset(p->mySysInfo.safestatus,0,128);      //32防区的基本状态
        memset(p->mySysInfo.ringname,0,2048);      //八种铃声的

        p->mySysInfo.NumOfMp3 = 0;
        for(i=0; i< 5;i++){
                p->mySysInfo.Mp3Position[i] = 0;
                p->mySysInfo.Mp3Len[i] = 0;
                p->mySysInfo.idrule[i] = 0;
        }
        p->mySysInfo.bright = 20;
        p->mySysInfo.contrast = 10;
        p->mySysInfo.saturation = 40;
        p->mySysInfo.ringvolume = 80;
        p->mySysInfo.ringtimes = 35;
        p->mySysInfo.avoidtimes = 43200;
        p->mySysInfo.isavoid = 0;
        p->mySysInfo.isbuttonvoice = 0;
        //  p->mySysInfo.isjiaju = 0;
        p->mySysInfo.iskeepscreen = 0;
        p->mySysInfo.isusedffs = 1;
        p->mySysInfo.ishttpdown = 0;
        p->mySysInfo.isagent = 0;
        p->mySysInfo.agenttimes = 43200;
        p->mySysInfo.talkvolume = 80;
        p->mySysInfo.alarmvolume = 80;
        p->mySysInfo.micvolume = 100;
        p->mySysInfo.isautocap= 0;
        p->mySysInfo.alarmtime = 100;
        p->mySysInfo.alarmtime1 = 40;
        p->mySysInfo.isrecord = 1;
        p->mySysInfo.isremote = 1;
        p->mySysInfo.alarmvoicetime = 300;
        p->mySysInfo.maxphoto = 100;
        p->mySysInfo.photoqualit = 90;
        p->mySysInfo.bakliftip = 0;
        p->mySysInfo.isnvo = 0;
        p->mySysInfo.framerate  = 7;
        p->mySysInfo.hassip = 1;
        p->mySysInfo.isenglish = 0;
        p->mySysInfo.isbomb = 0x0;
		p->mySysInfo.isuseui = 0x2; //默认采用第2版UI
		p->BootInfo.program_size = 5*1024*1024;
		p->mySysInfo.cur_scene  = 255;
		p->mySysInfo.doorcallproir = 0;
		p->mySysInfo.usecenternegotiate = 0;
		
        strcpy(p->LocalSetting.privateinfo.LocalID, "010010100100101");
        p->LocalSetting.privateinfo.LocalID[15] = '\0';
		printf("========================localid=%s\r\n",LOCALID);
        p->LocalSetting.privateinfo.LocalIP = 0x2464a8c0;
        p->LocalSetting.privateinfo.SubMaskIP = 0x00ffffff;//0xffffff00;
        p->LocalSetting.privateinfo.GateWayIP = 0xfe64a8c0;//0xc0a80101;

		p->mySysInfo.wifi_dhcp = 0;
		p->mySysInfo.fangchai_switch = 0;
		p->mySysInfo.wifi_switch = 0;
		p->mySysInfo.wifiip = 0x0;
		p->mySysInfo.wifigateway = 0x0;
		p->mySysInfo.wifimask = 0x0;
		p->mySysInfo.wifidns = 0x0;
		p->mySysInfo.chengfangcnt = 3;
		p->mySysInfo.chengfangtime = 5;
		memset(p->mySysInfo.wifissid,0,16);
		memset(p->mySysInfo.wifipwd,0,16);
		
        p->LocalSetting.privateinfo.NumOfMONI = 0; //摄像头
        p->LocalSetting.privateinfo.Moni[0].IP = 0x2764a8c0;
        strcpy(p->LocalSetting.privateinfo.Moni[0].name, "摄像头一");

        p->LocalSetting.privateinfo.Moni[1].IP = 0x2864a8c0;
        strcpy(p->LocalSetting.privateinfo.Moni[1].name, "摄像头二");

        p->LocalSetting.privateinfo.Moni[2].IP = 0x2964a8c0;
        strcpy(p->LocalSetting.privateinfo.Moni[2].name, "摄像头三");

		p->LocalSetting.privateinfo.NumOfSmallDoor = 0;
		p->LocalSetting.privateinfo.SmallDoor[0].IP = 0;
		p->LocalSetting.privateinfo.SmallDoor[0].name[0]='\0';

        p->LocalSetting.publicinfo.CenterMajorIP = 0x0564a8c0;
        p->LocalSetting.publicinfo.CenterMinorIP = 0;
        p->LocalSetting.publicinfo.ManagerMajorIP = 0x0564a8c0;
        p->LocalSetting.publicinfo.ManagerMinorIP = 0;
        p->LocalSetting.publicinfo.CenterWarningIP = 0;
        p->LocalSetting.publicinfo.CenterMenjinIP = 0;
        p->LocalSetting.publicinfo.CenterFileIP = 0;
        p->LocalSetting.publicinfo.CenterNetIP = 0;

        p->LocalSetting.publicroom.NumOfGate = 1;
        p->LocalSetting.publicroom.MainGate[0].IP = 0x2564a8c0;
        strcpy(p->LocalSetting.publicroom.MainGate[0].name,"主围墙机");

        p->LocalSetting.publicroom.NumOfDoor = 1;
        p->LocalSetting.publicroom.Door[0].IP = 0x2664a8c0;
        strcpy(p->LocalSetting.publicroom.Door[0].name,"主门口机");

        p->LocalSetting.publicroom.NumOfRepaire = 4;
        strcpy(p->LocalSetting.publicroom.NameOfRepaire[0],"供电系统故障");
        strcpy(p->LocalSetting.publicroom.NameOfRepaire[1],"供水系统故障");
        strcpy(p->LocalSetting.publicroom.NameOfRepaire[2],"厨卫家居故障");
        strcpy(p->LocalSetting.publicroom.NameOfRepaire[3],"供气系统故障");

        p->LocalSetting.publicroom.NumOfFee = 6;
        strcpy(p->LocalSetting.publicroom.NameOfFee[0],"生活用水费");
        strcpy(p->LocalSetting.publicroom.NameOfFee[1],"电费");
        strcpy(p->LocalSetting.publicroom.NameOfFee[2],"燃气费");
        strcpy(p->LocalSetting.publicroom.NameOfFee[3],"管理费");
        strcpy(p->LocalSetting.publicroom.NameOfFee[4],"饮用水费");
        strcpy(p->LocalSetting.publicroom.NameOfFee[5],"取暖费");
        
        p->mySysInfo.idrule[0] = 2;
        p->mySysInfo.idrule[1] = 3;
        p->mySysInfo.idrule[2] = 2;
        p->mySysInfo.idrule[3] = 3;
        p->mySysInfo.idrule[4] = 3;

        memset(p->mySysInfo.phonenumber,0,120);
}

void UpdateSystemInfo(){
	if(g_writesem){
		EnterMutex();
		g_iswritefileflag |= CFG_FILE;
		SDL_SemPost(g_writesem);
		ExitMutex();
	}
}

static int writeflastask(){
        while(!g_writetask_exit){
                SDL_SemWait(g_writesem);
               // printf(" write cfg file\n");
                EnterMutex();
                if(g_iswritefileflag & CFG_FILE){
                        write_cfg_file();
                        g_iswritefileflag &= ~CFG_FILE;
                }
                ExitMutex();

        }
 	return 0;
}

void InitSystemInfo(){
	int retVal;
	memset(&g_SystemInfo,0,sizeof(T_SYSTEMINFO));
	g_writesem = SDL_CreateSemaphore(0);



	if(pthread_mutex_init(&g_WriteMutext,NULL) != 0)
	{
		printf("g_WriteMutext create error %d ret=%d\n",errno,retVal);
	}

	SDL_CreateThread((int (*)())writeflastask, NULL);
	SDL_Delay(1000);
	read_cfg_file();
	printf("========================localid=%s\r\n",LOCALID);
}

void UninitSystemInfo()
{
        g_writetask_exit = 1;
        UpdateSystemInfo(); //
        SDL_Delay(2000);
        SDL_DestroySemaphore(g_writesem);
        g_writesem = NULL;
        pthread_mutex_destroy(&g_WriteMutext);
}

void EnterMutex(){

        pthread_mutex_lock(&g_WriteMutext);

}

void  ExitMutex(){
        pthread_mutex_unlock(&g_WriteMutext);
}

void printf_sys_info()
{
	T_SYSTEMINFO* pSystemInfo = &g_SystemInfo;
	printf("pSystemInfo->LocalSetting.privateinfo.LocalIP = 0x%08x \n",pSystemInfo->LocalSetting.privateinfo.LocalIP);
	printf("pSystemInfo->LocalSetting.privateinfo.SubMaskIP = 0x%08x \n",pSystemInfo->LocalSetting.privateinfo.SubMaskIP);
	printf("pSystemInfo->LocalSetting.privateinfo.GateWayIP = 0x%08x\n",pSystemInfo->LocalSetting.privateinfo.GateWayIP);
	printf("pSystemInfo->LocalSetting.publicinfo.CenterMajorIP = 0x%08x \n",pSystemInfo->LocalSetting.publicinfo.CenterMajorIP);
}

//add by wyx 2014-5-30 
static T_ROOM_MORE_CFG  g_MorecfgInfo;
static pthread_mutex_t  g_WriteMutext2;
void SetDefaultMoreCfg()
{
	int i = 0;
	memset(&g_MorecfgInfo,0,sizeof(T_ROOM_MORE_CFG));
	for(i=0;i<11;i++)
	{
		g_MorecfgInfo.validlevel[i]=0;
	}
	g_MorecfgInfo.validlevel[11] = 0;
	g_MorecfgInfo.validlevel[12] = 1;
	g_MorecfgInfo.validlevel[13] = 1;
	g_MorecfgInfo.validlevel[14] = 0;
	g_MorecfgInfo.validlevel[15] = 0;
	g_MorecfgInfo.validlevel[16] = 0;
	g_MorecfgInfo.validlevel[17] = 0;
}
void InitSystemMoreInfo(){
	int retVal;
	memset(&g_MorecfgInfo,0,sizeof(T_ROOM_MORE_CFG));
	if(pthread_mutex_init(&g_WriteMutext2,NULL) != 0)
	{
		printf("g_WriteMutext2 create error %d ret=%d\n",errno,retVal);
	}
	read_Morecfg_file();
}
void write_Morecfg_file()
{
	FILE *cfg_fd;
	int ret =0;
	if((cfg_fd = fopen(cfg_name2, "wb")) == NULL)
		printf("无法创建配置文件 morecfg\n");
	else
	{
		char* p = (char*)&g_MorecfgInfo;
		int totallen = sizeof(g_MorecfgInfo);
		int writelen = 0;
		int wl = 0;
		while(1){
			wl = fwrite( p+writelen,1, totallen - writelen , cfg_fd);
			if(ferror(cfg_fd)){
				printf("morecfg:写配置文件错误:%x\n",errno);
				break;
			}
			writelen += wl;
			if(writelen  == totallen)
				break;
		}
		fflush(cfg_fd);
		fclose(cfg_fd);
		cfg_fd = NULL;
		system("sync");
	}
}
static void read_Morecfg_file()
{
        DIR *dirp = NULL;
        FILE* cfg_fd = NULL;
        if((dirp = opendir("/customer")) == NULL){
        	if(mkdir("/customer",1) < 0)
        		printf("error to mkdir /home\n");
        }
        if(dirp != NULL){
        	closedir(dirp);
        	dirp = NULL;
        }
        if((dirp=opendir(cfg_dir1)) == NULL)
        {
                if(mkdir(cfg_dir1, 1) < 0)
                        printf("error to mkdir %s\n", cfg_dir1);
        }
        if(dirp != NULL)
        {
                closedir(dirp);
                dirp = NULL;
        }
        if((dirp=opendir(cfg_dir2)) == NULL)
        {
                if(mkdir(cfg_dir2, 1) < 0)
                        printf("morecfg:error to mkdir %s\n", cfg_dir2);
        }
        if(dirp != NULL)
        {
                closedir(dirp);
                dirp = NULL;
        }
        if((cfg_fd=fopen(cfg_name2, "rb"))==NULL)
        {
                printf("morecfg:配置文件不存在，创建新文件\n");
                if((cfg_fd = fopen(cfg_name2, "wb")) == NULL)
                    printf("morecfg:无法创建配置文件\n");
                else
                {
                    SetDefaultMoreCfg();
                    write_Morecfg_file();
                }
        }
        else
        {
                int bytes_read;
                int totalen = sizeof(g_MorecfgInfo);
                int readlen = 0;
                char* p = (char*)&g_MorecfgInfo;
                fseek(cfg_fd,0,SEEK_SET);
                fseek(cfg_fd,0,SEEK_END);
                if(ftell(cfg_fd) != totalen){
            		printf("morecfg:阅读配置文件出错,文件长度为0");
                	fclose(cfg_fd);
                	cfg_fd = NULL;
                	unlink(cfg_name2);
                	SetDefaultMoreCfg();
                	write_Morecfg_file();
                	return;                	
                }
                fseek(cfg_fd,0,SEEK_SET);
		        while(1){
                	bytes_read=fread(p+readlen,1,totalen - readlen, cfg_fd);
                	if(ferror(cfg_fd)){
                		printf("morecfg:阅读配置文件出错，以默认方式设置,errno =%d\n",errno);
                    	fclose(cfg_fd);
                    	cfg_fd = NULL;
                    	unlink(cfg_name2);
                    	SetDefaultMoreCfg();
                    	write_Morecfg_file();
                    	return;                		
                	}
                	readlen += bytes_read;
                	if(readlen == totalen)
                		break;
                }
                fclose(cfg_fd);
                cfg_fd = NULL;
        }
}
T_ROOM_MORE_CFG * GetSystemMoreInfo()
{
	return &g_MorecfgInfo;
}
void UninitSystemMoreInfo()
{
	pthread_mutex_destroy(&g_WriteMutext2);
}
void EnterMutexMore(){
	pthread_mutex_lock(&g_WriteMutext2);
}
void  ExitMutexMore(){
	pthread_mutex_unlock(&g_WriteMutext2);
}
#if 1 //add by xiao
void systemGetLocalID(char *localID)
{
	printf("systemGetLocalID:%s\r\n",LOCALID);
	memcpy(localID,LOCALID,15);
}
#endif
