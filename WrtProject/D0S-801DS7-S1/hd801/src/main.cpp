#define TEST_HTTP_DRIVER 0
#define TEST_WEB_SERVER  1
#undef  TEST_WEB_SERVER

#include <stdio.h> 

#include <errno.h> 
#include <dirent.h>  
#include <fcntl.h> 

#include <arpa/inet.h>  //for in_addr     
#include <linux/rtnetlink.h>    //for rtnetlink     
#include <net/if.h> //for IF_NAMESIZ, route_info     
#include <stdlib.h> //for malloc(), free()     
#include <string.h> //for strstr(), memset()     

#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include "wrtfont.h"
#include "c_interface.h"
#include "ctrlcmd.h"    
#include "Handler.h"
#include "audiovideo.h"      
#include "main_program.h"  
#include "jpeg.h"   
#include "image.h"
#include "window.h" 
#include "zenmalloc.h"
#include "RoomGpioUtil.h"
#include "tslib.h"  

#include "fbutils.h"
#include "testutils.h"
#include "wrt_audio.h"
#include "RoomGpioUtil.h"
#include "tmSysConfig.h"
#include "c_interface.h"

#include "wrt_audiostream.h"

#include "sstardisp.h"
#include "mi_disp.h"

#include "wrtTcpShare.h"
//#include "call_menu_res.h"
#ifdef TEST_WEB_SERVER

extern "C" long Web_Init(void);

#endif

#ifdef __cplusplus 
extern "C"{
#endif 
        int WRT_HAVE_DBG(); 
int tcpServerToMainCallback(cJSON *json,cJSON **jsonReturn,char *structBody,char**structBodyReturn);
#ifdef __cplusplus
} 
#endif     

int g_isscreencal = 0;
SDL_Surface* g_pScreenSurface = NULL;
SDL_Surface* tmp_g_pScreenSurface = NULL;
//SDL_Surface* tmp_g_pScreenSurface1 = NULL;

SDL_TimerID  g_screenkeeptimerid = 0;
SDL_TimerID  g_screenkeepplaytimerid = 0;
static int g_index = 0;

Uint32 gstarttick;
Uint32 gendtick; 

extern unsigned char g_isctrlelec ;

//////////////////////////////////////////
struct ICMPheader 
{
        unsigned char	byType;
        unsigned char	byCode;
        unsigned short	nChecksum; 
        unsigned short	nId;
        unsigned short	nSequence;
};

struct IPheader
{
        unsigned char	byVerLen; 
        unsigned char	byTos;
        unsigned short	nTotalLength;
        unsigned short	nId;
        unsigned short	nOffset;
        unsigned char	byTtl;
        unsigned char	byProtocol;
        unsigned short	nChecksum;
        unsigned int	nSrcAddr; 
        unsigned int	nDestAddr; 
};

  
////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// 
//extern RoomSystemInfo roomtest;
extern int                g_isUpdated ;
extern T_SYSTEMINFO    *pSystemInfo;
extern T_ROOM_MORE_CFG  *p_MorecfgInfo;
extern bool            m_issreenoff;
extern bool            g_net_is_ok;
volatile UInt8         ledflag = 0;
volatile UInt8         g_green = 0;
volatile UInt8         g_red = 0;

unsigned long          tid_checktask;        //网络自检任务

#ifdef USE_SOFTWATCHDOG
tmInstance_t           watchdoginst=0;
#endif
CMainProcess*          g_MainProcess=NULL;

extern bool     g_have_warning ;  //是否设置报警
extern bool     g_have_msg ;   //是否有msm
extern bool     g_have_defend;


static char volName[FILENAME_MAX];  // volume name
static bool isDevCreated=false;


unsigned char tscreendata1[24] = {
        0x1,0x18,0x6,0x66,0x6,0x71,0x1,0x16,0x0,0x64,0x0,0x3C,
        0x5,0x3C,0x3,0x24,0x0,0x40,0x55,0x2A,0x0,0x0,0x63,0x4
};

//////////////////////////////////////////////////////////////////////////

static int net_check_task(void* pvoid);

/////////////////////////////////////////////////////////////////////////

//#undef printf 
//#define printf printf

//2015-7-15 修改LED灯使用过程中熄灭问题
static Uint32 GpioControl (Uint32 interval,void* pvoid)
{
	//printf("interval = %d g_have_msg = %d\n",interval,g_have_msg);

        ledflag++;
        if(ledflag == 1){
            if(g_have_warning){
                   // if(g_green == 1){
                   //         tmGpioSetPin(CTL_IO_SETLED_RED,0);
                    //        g_green = 0;
                  //  }
                   // if(g_red == 0){
                            tmGpioSetPin(CTL_IO_SETLED_RED,1);
                            g_red = 1;
                   // }
            }else if(g_have_msg || isnoreadpic()|| isnoreadvideo()||isnoreadvoice()){
                   // if(g_green == 1){ 
                   //         tmGpioSetPin(CTL_IO_SETLED_GREEN,0);
                   //         g_green = 0;
                   // }
                   // if(g_red == 0){
                            tmGpioSetPin(CTL_IO_SETLED_RED,1);
                            g_red = 1;
                  //  }
            }else{   //显示绿灯
            		
                    //if(g_red == 1){
                   //         tmGpioSetPin(CTL_IO_SETLED_RED,0);
                   //         g_red =0;
                  //  }
                  //  if(g_green == 0){
                    	
                            tmGpioSetPin(CTL_IO_SETLED_GREEN,1);
                            g_green = 1;
                 //   }
            }
        }else if(ledflag == 2){
                if(g_have_warning){ //如果有报警显示报警灯闪烁红灯
                      //  if(g_red == 1){
                                tmGpioSetPin(CTL_IO_SETLED_RED,0);
                                g_red = 0;
                     //   }
                }
                ledflag = 0;
        }
	
     	return interval;
}


//01087522371 
static unsigned short CalcChecksum (char *pBuffer, int nLen)
{
        //Checksum for ICMP is calculated in the same way as for
        //IP header

        //This code was taken from: http://www.netfor2.com/ipsum.htm
 
        unsigned short nWord;
        unsigned int nSum = 0;
        int i; 

        //Make 16 bit words out of every two adjacent 8 bit words in the packet
        //and add them up
        for (i = 0; i < nLen; i = i + 2)
        {
                nWord =((pBuffer [i] << 8)& 0xFF00) + (pBuffer [i + 1] & 0xFF);
                nSum = nSum + (unsigned int)nWord;
        }

        //Take only 16 bits out of the 32 bit sum and add up the carries
        while (nSum >> 16)
        {
                nSum = (nSum & 0xFFFF) + (nSum >> 16);
        }

        //One's complement the result
        nSum = ~nSum;

        return ((unsigned short) nSum);
}

static bool ValidateChecksum (char *pBuffer, int nLen)
{
        unsigned short nWord;
        unsigned int nSum = 0;
        int i;

        //Make 16 bit words out of every two adjacent 8 bit words in the packet
        //and add them up
        for (i = 0; i < nLen; i = i + 2)
        {
                nWord =((pBuffer [i] << 8)& 0xFF00) + (pBuffer [i + 1] & 0xFF);
                nSum = nSum + (unsigned int)nWord;
        }

        //Take only 16 bits out of the 32 bit sum and add up the carries
        while (nSum >> 16)
        {
                nSum = (nSum & 0xFFFF) + (nSum >> 16);
        }

        //To validate the checksum on the received message we don't complement the sum
        //of one's complement
        //One's complement the result
        //nSum = ~nSum;

        //The sum of one's complement should be 0xFFFF
        return ((unsigned short)nSum == 0xFFFF);
}

int net_test_ping(unsigned long ipaddr){
        int nCount = 1;
        int pingok = 0;
        int nSequence=0;
        int nMessageSize = 0;
        ICMPheader sendHdr;
        char* pSendBuffer;
        //char* pRecvBuffer;
		int starttime = 0;
		int endtime = 0;
		starttime = SDL_GetTicks();
        int sock;
        sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);	//Create a raw socket which will use ICMP

        sockaddr_in dest;	//Dest address to send the ICMP request
        dest.sin_addr.s_addr = ipaddr;
        dest.sin_family = AF_INET;
        dest.sin_port = rand ();	//Pick a random port

		int retreturn = 0;
        int nResult = 0;
        fd_set fdRead;
        timeval timeInterval = {0, 0};
        timeInterval.tv_usec = 10 * 1000;

		//printf("test ping ===== 0x%x\n",ipaddr);
        sendHdr.nId = htons (rand ());	//Set the transaction Id

        while ( nCount--)
        {
                //Create the message buffer, which is big enough to store the header and the message data
                pSendBuffer = (char*) ZENMALLOC(sizeof (ICMPheader) + nMessageSize);

                sendHdr.byCode = 0;	//Zero for ICMP echo and reply messages
                sendHdr.nSequence = htons (nSequence++);
                sendHdr.byType = 8;	//Eight for ICMP echo message
                sendHdr.nChecksum = 0;	//Checksum is calculated later on

                memcpy (pSendBuffer, &sendHdr, sizeof (ICMPheader));	//Copy the message header in the buffer
                memset (pSendBuffer + sizeof (ICMPheader), 'x', nMessageSize);	//Fill the message with some arbitary value

                //Calculate checksum over ICMP header and message data
                sendHdr.nChecksum = htons (CalcChecksum (pSendBuffer, sizeof (ICMPheader) + nMessageSize));

                //Copy the message header back into the buffer
                memcpy (pSendBuffer, &sendHdr, sizeof (ICMPheader));

                nResult = sendto (sock, pSendBuffer, sizeof (ICMPheader) + nMessageSize, 0, (sockaddr *)&dest, sizeof (sockaddr));

                //Save the time at which the ICMP echo message was sent


                if (nResult <0)
                {

                        printf("send error \n");
						retreturn = 0;
                        ZENFREE(pSendBuffer);
                        //closesocket(sock);
                        break ;
                }

                FD_ZERO (&fdRead);
                FD_SET (sock, &fdRead);

                if ((nResult = select (sock+1, &fdRead, NULL, NULL, &timeInterval))
                        <=0)
                {
                       // printf("select error \n");

                        ZENFREE(pSendBuffer);
                        retreturn = 0;
                        //closesocket(sock);
                        break ;
                }

                if (nResult > 0 && FD_ISSET (sock, &fdRead))
                {
                        //Allocate a large buffer to store the response
                        char *pRecvBuffer = (char*)ZENMALLOC(1500);

                        if ((nResult = recvfrom (sock, pRecvBuffer, 1500, 0, 0, 0))
                                <=0)
                        {
                                printf("recv error \n");

                                ZENFREE(pSendBuffer);
                                ZENFREE(pRecvBuffer);
                                retreturn = 0;
                                //closesocket(sock);
                                break ;
                        }


                        //We got a response so we construct the ICMP header and message out of it
                        ICMPheader recvHdr;
                        char *pICMPbuffer = NULL;

                        //The response includes the IP header as well, so we move 20 bytes ahead to read the ICMP header
                        pICMPbuffer = pRecvBuffer + sizeof(IPheader);

                        //ICMP message length is calculated by subtracting the IP header size from the
                        //total bytes received
                        int nICMPMsgLen = nResult - sizeof(IPheader);

                        //Construct the ICMP header
                        memcpy (&recvHdr, pICMPbuffer, sizeof (recvHdr));

                        //Construct the IP header from the response
                        IPheader ipHdr;
                        memcpy (&ipHdr, pRecvBuffer, sizeof (ipHdr));

                        recvHdr.nId = recvHdr.nId;
                        recvHdr.nSequence = recvHdr.nSequence;
                        recvHdr.nChecksum = ntohs (recvHdr.nChecksum);

                        //Check if the response is an echo reply, transaction ID and sequence number are same
                        //as for the request, and that the checksum is correct
                        if (recvHdr.byType == 0 &&
                                recvHdr.nId == sendHdr.nId &&
                                recvHdr.nSequence == sendHdr.nSequence &&
                                ValidateChecksum (pICMPbuffer, nICMPMsgLen)  &&
                                memcmp (pSendBuffer + sizeof(ICMPheader), pRecvBuffer + sizeof (ICMPheader) + sizeof(IPheader),
                                nResult - sizeof (ICMPheader) - sizeof(IPheader)) == 0)
                        {
                                //All's OK
                               // printf("ok!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                               
                                pingok++;
                        }
                        else
                        {
                        		retreturn = 0;
                                //printf("The echo reply is not correct!\n");
                        }

                        ZENFREE(pRecvBuffer);
                }
                else
                {
                        printf("Request timed out!\n");
                        retreturn = 0;
                }
                ZENFREE(pSendBuffer);
        }
        closesocket(sock);
        if(pingok == 1)
        {
        	retreturn = 1;
        }
        return retreturn;
}


void net_sence_task(unsigned long ipaddr){
        int nCount = 3;
        int pingok = 0;
        int nSequence=0;
        int nMessageSize = 0;
        ICMPheader sendHdr;
        char* pSendBuffer;
        //char* pRecvBuffer;

        int sock;
        sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);	//Create a raw socket which will use ICMP

        sockaddr_in dest;	//Dest address to send the ICMP request
        dest.sin_addr.s_addr = ipaddr;
        dest.sin_family = AF_INET;
        dest.sin_port = rand ();	//Pick a random port

        int nResult = 0;
        fd_set fdRead;
        timeval timeInterval = {0, 0};
        timeInterval.tv_usec = 1000 * 1000;


        sendHdr.nId = htons (rand ());	//Set the transaction Id

        while ( nCount--)
        {
                //Create the message buffer, which is big enough to store the header and the message data
                pSendBuffer = (char*) ZENMALLOC(sizeof (ICMPheader) + nMessageSize);

                sendHdr.byCode = 0;	//Zero for ICMP echo and reply messages
                sendHdr.nSequence = htons (nSequence++);
                sendHdr.byType = 8;	//Eight for ICMP echo message
                sendHdr.nChecksum = 0;	//Checksum is calculated later on

                memcpy (pSendBuffer, &sendHdr, sizeof (ICMPheader));	//Copy the message header in the buffer
                memset (pSendBuffer + sizeof (ICMPheader), 'x', nMessageSize);	//Fill the message with some arbitary value

                //Calculate checksum over ICMP header and message data
                sendHdr.nChecksum = htons (CalcChecksum (pSendBuffer, sizeof (ICMPheader) + nMessageSize));

                //Copy the message header back into the buffer
                memcpy (pSendBuffer, &sendHdr, sizeof (ICMPheader));

                nResult = sendto (sock, pSendBuffer, sizeof (ICMPheader) + nMessageSize, 0, (sockaddr *)&dest, sizeof (sockaddr));

                //Save the time at which the ICMP echo message was sent


                if (nResult <0)
                {

                        printf("send error \n");

                        ZENFREE(pSendBuffer);
                        //closesocket(sock);
                        break ;
                }

                FD_ZERO (&fdRead);
                FD_SET (sock, &fdRead);

                if ((nResult = select (sock+1, &fdRead, NULL, NULL, &timeInterval))
                        <=0)
                {
                        printf("select error \n");

                        ZENFREE(pSendBuffer);
                        //closesocket(sock);
                        break ;
                }

                if (nResult > 0 && FD_ISSET (sock, &fdRead))
                {
                        //Allocate a large buffer to store the response
                        char *pRecvBuffer = (char*)ZENMALLOC(1500);

                        if ((nResult = recvfrom (sock, pRecvBuffer, 1500, 0, 0, 0))
                                <=0)
                        {
                                printf("recv error \n");

                                ZENFREE(pSendBuffer);
                                ZENFREE(pRecvBuffer);
                                //closesocket(sock);
                                break ;
                        }


                        //We got a response so we construct the ICMP header and message out of it
                        ICMPheader recvHdr;
                        char *pICMPbuffer = NULL;

                        //The response includes the IP header as well, so we move 20 bytes ahead to read the ICMP header
                        pICMPbuffer = pRecvBuffer + sizeof(IPheader);

                        //ICMP message length is calculated by subtracting the IP header size from the
                        //total bytes received
                        int nICMPMsgLen = nResult - sizeof(IPheader);

                        //Construct the ICMP header
                        memcpy (&recvHdr, pICMPbuffer, sizeof (recvHdr));

                        //Construct the IP header from the response
                        IPheader ipHdr;
                        memcpy (&ipHdr, pRecvBuffer, sizeof (ipHdr));

                        recvHdr.nId = recvHdr.nId;
                        recvHdr.nSequence = recvHdr.nSequence;
                        recvHdr.nChecksum = ntohs (recvHdr.nChecksum);

                        //Check if the response is an echo reply, transaction ID and sequence number are same
                        //as for the request, and that the checksum is correct
                        if (recvHdr.byType == 0 &&
                                recvHdr.nId == sendHdr.nId &&
                                recvHdr.nSequence == sendHdr.nSequence &&
                                ValidateChecksum (pICMPbuffer, nICMPMsgLen)  &&
                                memcmp (pSendBuffer + sizeof(ICMPheader), pRecvBuffer + sizeof (ICMPheader) + sizeof(IPheader),
                                nResult - sizeof (ICMPheader) - sizeof(IPheader)) == 0)
                        {
                                //All's OK
                                printf("ok!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                                pingok++;
                        }
                        else
                        {
                                printf("The echo reply is not correct!\n");
                        }

                        ZENFREE(pRecvBuffer);
                }
                else
                {
                        printf("Request timed out!\n");
                }
                ZENFREE(pSendBuffer);
        }
        closesocket(sock);
        if(pingok == 3){
                char msg[50];
                memset(msg,0,5);
                sprintf(msg,"ping %x ok!",ipaddr);
                //	CreateDialogWin(msg,0);
                CreateDialogWin2(msg,MB_OK,NULL,NULL);
        }else
        {
                char msg[50];
                memset(msg,0,5);
                sprintf(msg,"ping %x failed!",ipaddr);
                //CreateDialogWin(msg,0);
                CreateDialogWin2(msg,MB_OK,NULL,NULL);

        }
}
#if HAVE_SCREEN_KEEP

//extern int gHaveScreenkeep;
extern int gScreenkeepcount;
extern void init_screenkeep();

#endif

static Uint32 screenkeepplay_callback(Uint32 interval,void* pvoid)
{
	WRT_DEBUG("screenkeepplay_callback");
	if(pSystemInfo->mySysInfo.iskeepscreen){
			
           //	ClearRect(0,0,1024,600);//需清空OSD
           	
           	g_MainProcess->ViewSceenKeep(g_index);
                g_index++;
                if(g_index > (gScreenkeepcount-1))
			g_index = 0;
		return   interval;
	}else{
		if(wrthost_sysidle()){
			screen_close(); 
    		}
    	
    	}
    return interval;
 
}

#if HAVE_SCREEN_KEEP

void screenkeep_init(){
   	init_screenkeep();
}

void screenkeep_stop(){
	gendtick = SDL_GetTicks();
	gstarttick =  gendtick;

	if(pSystemInfo->mySysInfo.iskeepscreen == 0)
		return;	
	if(g_screenkeepplaytimerid != 0){
      		SDL_RemoveTimer(g_screenkeepplaytimerid);
      		send_free_arp();
      		if(g_MainProcess)
 				g_MainProcess->NoitfyReDraw(12);//重新显示屏保之前的界面      
      		g_screenkeepplaytimerid = 0;
      		m_issreenoff = false;
      	}
}

extern void force_to_fb1();

//屏保或者关屏，如果是多媒体播放状态则不关闭或屏保
void screenkeep_start()
{
	if(pSystemInfo->mySysInfo.iskeepscreen == 0)
	{
		Window *pWindow3;
		pWindow3 = WindowGetTopVisibleWindow();
	    if(pWindow3->type == MUTILMEDIA){
	            return;
	    }
		screen_close();
		return;
	}
	if(m_issreenoff == true) 
		return;

	if(g_screenkeepplaytimerid == 0){
		WRT_DEBUG("screenkeep_start");
		Window *pWindow3;
		pWindow3 = WindowGetTopVisibleWindow();
	    if(pWindow3->type == MUTILMEDIA){
	            return;
	    }
		g_screenkeepplaytimerid  = SDL_AddTimer(3*1000,screenkeepplay_callback,NULL);
		m_issreenoff = true;                            
	}
}

#endif

/////////////////////////////////////////////////////////
//

/////////////////////////////////////////////////////////

static int create_macsn_thread()
{
    char snflag;
    //char msn[32];
    //char mmac[6];

    snflag=0;
    getmacsnFileFlag(&snflag);

    if(snflag==0)
    {
        SDL_CreateThread(task_macsnfile,NULL);
    }else
    {
        //getmacsnFileMAC(mmac);
        //getmacsnFileSN(msn);

        //setRoomMAC(mmac);
        //setRoomSN(msn);
        SDL_CreateThread(severce_macsnfile,NULL);
    }
}

static int testmp3(void *p)
{
    char *buf;

    while(1)
    {
        buf = (char *)malloc(1024);
        if(buf != NULL)
            free(buf);
    }
}

void setblackscreen(){
    SDL_Rect curbox;
    SDL_Surface* pSurface = NULL;
    SDL_Color color;
    curbox.x = 0;
    curbox.y = 0;
    curbox.w = 1024; 
    curbox.h = 600;
    g_isUpdated = 0;

    SDL_FillRect(g_pScreenSurface,&curbox,0x00000000);

    SDL_UpdateRect(g_pScreenSurface,0,0,1024,600);

    g_isUpdated = 1;
}
//
void test_view_start_interface(){
    SDL_Rect curbox;
    SDL_Surface* pSurface = NULL;
    SDL_Color color;
    curbox.x = 0;
    curbox.y = 0;
    curbox.w = 1024; 
    curbox.h = 600;
    g_isUpdated = 0;

    SDL_FillRect(g_pScreenSurface,&curbox,0xFF0080ff);

    color.r =  255;
    color.g =  255;
    color.b =  255;
    pSurface = WRT_RenderTextBlended(LT("系统正在启动..."),color,36);
    if(pSurface){
        SDL_Rect dst;
        dst.x = (curbox.w - pSurface->w)/2;
        dst.y = (curbox.h - pSurface->h)/2;
        dst.w = pSurface->w;
        dst.h  = pSurface->h;
        SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&dst);
    }
    SDL_UpdateRect(g_pScreenSurface,0,0,1024,600);
    g_isUpdated = 1;
}

static void setinittime()
{
    unsigned short year  = 2011;
    unsigned short month = 1;
    unsigned short day   = 1;
    unsigned short hour  = 01;
    unsigned short min   = 01;
    unsigned long date =0;
    unsigned long time =0;
    date = ((year&0xffff) << 16)|((month& 0xff)<<8) | (day & 0xff);
    time = ((hour&0xff) << 16) | ((min& 0xff)<<8 | (0 & 0xff));
    tm_set(date,time,0);	
}

static void process_mouse_msg(int x,int y,int status){
    long pos[3];
    if( g_MainProcess && status != -1 && (g_isctrlelec == 0 )){
        pos[0] = x;
        pos[1] = y;
        pos[2] = status;
        g_MainProcess->Send_Pos(pos);
    }
}

extern int g_enc_restart;

int isstartscreenkeep()
{
    gendtick = SDL_GetTicks();
    WRT_DEBUG("gstarttick  %d gendtick %d",gstarttick,gendtick);
    if((gendtick - gstarttick) >= 1800000)
    {
        gstarttick = gendtick;
        return 1;
    }

    if((gendtick - gstarttick) >= 180000){
        gstarttick =  gendtick;
        screenkeep_start();
        return 1;
    }
    return 0;
}

extern int gReadGpio(FILE * fp);
int g_ts_flag = 0;
int g_ts_send = 0;

int Get_safe_status()
{
    if(pSystemInfo->mySysInfo.safestatus[16]==1 || pSystemInfo->mySysInfo.safestatus[17] == 1)
        return 1;

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


extern void test_task();
//extern int encodevideo_task();

void test_show(int num){
        SDL_Rect curbox;
        SDL_Surface* pSurface = NULL;
        SDL_Color color;
        curbox.x = 0;
        curbox.y = 0;
        curbox.w = 1024; 
        curbox.h = 600;
        g_isUpdated = 0;

        SDL_FillRect(g_pScreenSurface,&curbox,rgb_black);

        color.r =  255;
        color.g =  255;
        color.b =  255;
        if(num == 1)
        	pSurface = WRT_RenderTextBlended("内存分配失败",color,36);
        else if(num == 2)
        	pSurface = WRT_RenderTextBlended("编码器启动失败",color,36);
        else if(num == 3)
        	pSurface = WRT_RenderTextBlended("文件不存在",color,36);
        else if(num == 4)
        	pSurface = WRT_RenderTextBlended("文件长度为0",color,36);
        else if(num == 10)
        	pSurface = WRT_RenderTextBlended("测试编码失败",color,36);
        else if(num == 20)
        	pSurface = WRT_RenderTextBlended("测试编码成功",color,36);
        //pSurface = WRT_RenderTextBlended(LT("系统正在启动..."),color,36);
        if(pSurface){
        	SDL_Rect dst;
        	dst.x = (curbox.w - pSurface->w)/2;
        	dst.y = (curbox.h - pSurface->h)/2;
        	dst.w = pSurface->w;
        	dst.h  = pSurface->h;
        	SDL_BlitSurface(pSurface,NULL,g_pScreenSurface,&dst);
        	
        }
        SDL_UpdateRect(g_pScreenSurface,0,0,1024,600);
		        g_isUpdated = 1;
}

//===============================wyx 2014 11 12 
int g_is_config_wifi = 1;//wifi配置界面引用，如果配置界面重新配置，那么此处任务退出重新执行
int g_is_wifi_status = 0;

int GetWifiNetStat()
{
	char    buffer[BUFSIZ];
	FILE    *read_fp;
	int     chars_read;
	int     ret;

	memset( buffer, 0, BUFSIZ );
	read_fp = popen("ifconfig ra0 | grep RUNNING", "r");//ra0 == rt3070sta serial
	if ( read_fp != NULL ) 
	{
		chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
		if (chars_read > 0) 
		{
			ret = 1;
		}
		else
		{
			ret = -1;
		}
		pclose(read_fp);
	}
	else
	{
		ret = -1;
	}

	return ret;
}

struct route_info{     
	u_int dstAddr;     
	u_int srcAddr;     
	u_int gateWay;     
	char ifName[IF_NAMESIZE];     
};  

#define G_READ_SIZE 8192

char msgBuf[G_READ_SIZE];
static unsigned long m_gatewayip = 0;

int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)     
{     
	struct nlmsghdr *nlHdr;     
	int readLen = 0, msgLen = 0;     
	do{     
		//收到内核的应答     
		if((readLen = recv(sockFd, bufPtr, G_READ_SIZE - msgLen, 0)) < 0)     
		{     
			perror("SOCK READ: ");     
			return -1;     
		}     

		nlHdr = (struct nlmsghdr *)bufPtr;     
		//检查header是否有效     
		if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))     
		{     
			perror("Error in recieved packet");     
			return -1;     
		}     

		if(nlHdr->nlmsg_type == NLMSG_DONE)      
		{     
			break;     
		}     
		else     
		{     
			bufPtr += readLen;     
			msgLen += readLen;     
		}     


		if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)      
		{     
			break;     
		}     
	} while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));     
	return msgLen;     
}     

void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)     
{     
	struct rtmsg *rtMsg;     
	struct rtattr *rtAttr;     
	int rtLen;     
	char *tempBuf = NULL;     
	struct in_addr dst;     
	struct in_addr gate;     

	tempBuf = (char *)malloc(100);     
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);     
	// If the route is not for AF_INET or does not belong to main routing table     
	//then return.      
	if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))     
		return;     

	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);     
	rtLen = RTM_PAYLOAD(nlHdr);     
	for(;RTA_OK(rtAttr,rtLen);rtAttr = RTA_NEXT(rtAttr,rtLen)){     
		switch(rtAttr->rta_type) {     
		case RTA_OIF:     
			if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);     
			break;     
		case RTA_GATEWAY:     
			rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);     
			break;     
		case RTA_PREFSRC:     
			rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);     
			break;     
		case RTA_DST:     
			rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);     
			break;     
		}     
	}     
	dst.s_addr = rtInfo->dstAddr;     
	if (strstr((char *)inet_ntoa(dst), "0.0.0.0"))     
	{     
		//printf("oif:%s",rtInfo->ifName);     
		gate.s_addr = rtInfo->gateWay;
		if(strncmp("ra0",rtInfo->ifName,3)==0)
		{
			//printf("ggggg wwww 0x%x\n",rtInfo->gateWay);
			m_gatewayip = rtInfo->gateWay;
		}
		gate.s_addr = rtInfo->srcAddr;     
		//printf("src:%s\n",(char *)inet_ntoa(gate));     
		gate.s_addr = rtInfo->dstAddr;     
		//printf("dst:%s\n",(char *)inet_ntoa(gate));      
	}  
	if(tempBuf)
		free(tempBuf);     
	return;     
}  

int get_gateway()     
{
	struct nlmsghdr *nlMsg;     
	struct rtmsg *rtMsg;     
	struct route_info *rtInfo;     
	     
	m_gatewayip = 0;

	int sock, len, msgSeq = 0;     

	if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)     
	{     
		perror("Socket Creation: ");     
		return -1;     
	}     

	memset(msgBuf, 0, G_READ_SIZE);     

	nlMsg = (struct nlmsghdr *)msgBuf;     
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);     

	nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.     
	nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .     

	nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.     
	nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.     
	nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.     

	if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0){     
		printf("Write To Socket Failed…\n");     
		return -1;     
	}     
	
	if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {     
		printf("Read From Socket Failed…\n");     
		return -1;     
	}     

	rtInfo = (struct route_info *)malloc(sizeof(struct route_info));     
	for(;NLMSG_OK(nlMsg,len);nlMsg = NLMSG_NEXT(nlMsg,len)){     
		memset(rtInfo, 0, sizeof(struct route_info));     
		parseRoutes(nlMsg, rtInfo);     
	}    
	if(rtInfo)
		free(rtInfo);   
	
	close(sock);
	
	return 0;
}  

int tellDHCP(){
	FILE *fp;
	int n;
	fp = fopen("/home/wrt/udhcpc2014" , "r");
	if(fp == NULL)
	{
		printf("quan:-------------fopen error\n");
		return -1;
	}
	fseek( fp , 0 , SEEK_END );
	int file_size;
	file_size = ftell( fp );
	//    printf( "%d\n" , file_size );
	char *tmp;
	fseek( fp , 0 , SEEK_SET);
	tmp =  (char *)malloc( file_size * sizeof( char ) );
	fread( tmp , file_size , sizeof(char) , fp);
	printf("quan:---------------udhcpc result:\n%s\n" , tmp );
	if(strstr(tmp,"failing")==NULL && file_size != 0 && strstr(tmp,"Lease")!=NULL)
	{
		printf("quan:========================================udhcpc succeed\n");
		n = 1;
	}
	else
	{
		printf("quan:========================================udhcpc failed\n");
		n = -2;
	}
	if(fp)
	{
		fflush(fp);
		fclose(fp);
		fp = NULL;
	}
	if(tmp)
		free(tmp);
	return n;
}


int wifi_process(void *p)
{
	int ret = 0;
	char cmdbuf[128];
	unsigned long gatewayip = 0;
	int run_flag = 0;
loopNetstat://检测wifi是否running

	//system("ifconfig eth0 down");//启用wifi需关闭eth0，因为IP会出现相同配置
	if( (pSystemInfo->mySysInfo.wifi_switch == 0) || (strlen((const char *)pSystemInfo->mySysInfo.wifissid)==0))
	{
		usleep(5000*1000);
		goto loopNetstat;
	}

	if(run_flag <= 0)
	{
		tmGpioSetPin(CTL_IO_SET_USB1_OFF,0);
		usleep(500*1000);
		tmGpioSetPin(CTL_IO_SET_USB1_ON,0);
		usleep(1000*1000);
	}
	system("ifconfig ra0 down");
	system("ifconfig ra0 up");
	
	usleep(1000*1000);
	run_flag = GetWifiNetStat();
	
	if(run_flag != 1)
	{
		usleep(2000*1000);
		goto loopNetstat;
	}
	
	system("iwpriv ra0 set NetworkType=Infra");

	memset(cmdbuf,0,128);
	sprintf(cmdbuf,"iwpriv ra0 set SSID=%s",pSystemInfo->mySysInfo.wifissid);
	system(cmdbuf);

	if(strlen((const char*)pSystemInfo->mySysInfo.wifipwd) == 0)
		printf("No password \n");
	else
	{
		system("iwpriv ra0 set AuthMode=WPAPSK");

		system("iwpriv ra0 set EncrypType=AES");

		memset(cmdbuf,0,128);
		sprintf(cmdbuf,"iwpriv ra0 set WPAPSK=%s",pSystemInfo->mySysInfo.wifipwd);
		system(cmdbuf);
	}
	
	memset(cmdbuf,0,128);
	sprintf(cmdbuf,"iwconfig ra0 essid %s",pSystemInfo->mySysInfo.wifissid);
	system(cmdbuf);

	g_is_config_wifi = 1;

	int linkcnt = 0;

	if(pSystemInfo->mySysInfo.wifi_dhcp)
	{
		system("rm -f /home/wrt/udhcpc2014");
		system("udhcpc -i ra0 -a -t 20 -T 3 -n -q > /home/wrt/udhcpc2014");
		if(tellDHCP() < 0)
		{
			goto loopNetstat;
		}
		
	}else
	{
		if(pSystemInfo->mySysInfo.wifiip > 0 && pSystemInfo->mySysInfo.wifiip != 0xFFFFFFFF)
			set_wifi_ip(pSystemInfo->mySysInfo.wifiip);
		if(pSystemInfo->mySysInfo.wifimask > 0 && pSystemInfo->mySysInfo.wifimask != 0xFFFFFFFF)
			set_wifi_netmask(pSystemInfo->mySysInfo.wifimask);
		if(pSystemInfo->mySysInfo.wifigateway > 0 && pSystemInfo->mySysInfo.wifigateway != 0xFFFFFFFF)
			set_wifi_gateway(pSystemInfo->mySysInfo.wifigateway);
		if(pSystemInfo->mySysInfo.wifidns != 0 && pSystemInfo->mySysInfo.wifidns !=0xFFFFFFFF)
		{
			memset(cmdbuf,0,128);

			system("rm  -rf /etc/resolv.conf");
			char sysbuf[64];
			toinet_addr(pSystemInfo->mySysInfo.wifidns,(unsigned char*)cmdbuf);
			//printf("dns  ================================ %s\n",cmdbuf);
			
			sprintf(sysbuf,"echo nameserver %s >> /etc/resolv.conf",cmdbuf);
			system(sysbuf);
		}
	}
	
	while(g_is_config_wifi)
	{
		usleep(2000*1000);
		if(pSystemInfo->mySysInfo.wifi_dhcp)
		{
			get_gateway();
			gatewayip = m_gatewayip;
		}
		else
			gatewayip = pSystemInfo->mySysInfo.wifigateway;
		
		if(net_test_ping(gatewayip) == 1)
		{
			g_is_wifi_status = 1;
			linkcnt = 0;
		}else
		{
			linkcnt++;
			if(linkcnt >= 5)
			{
				memset(cmdbuf,0,128);
				sprintf(cmdbuf,"iwconfig ra0 essid %s",pSystemInfo->mySysInfo.wifissid);
				system(cmdbuf);
				g_is_wifi_status = 0;
				if(linkcnt > 15)
					goto loopNetstat;
				printf("reconnect wifi!\n");
			}
		}
	}

}

void Wifi_Config_Process()
{
	SDL_CreateThread(wifi_process,NULL);
}

//=============================For test video========Start=========================================
#include "mi_vdec.h"
#include "mi_sys.h"
#include "sstarvideo.h"
FILE *g_pStreamFile[32] = {NULL};
#define NALU_PACKET_SIZE            256*1024
//#define ADD_HEADER_ES
#define MI_U32VALUE(pu8Data, index) (pu8Data[index]<<24)|(pu8Data[index+1]<<16)|(pu8Data[index+2]<<8)|(pu8Data[index+3])
static int g_VdecRun = 0;
static pthread_t g_VdeStream_tid;

typedef struct WRT_VideoParam_s
{
    unsigned short u16VideoW;
    unsigned short u16VideoH;
    unsigned short u16X;
    unsigned short u16Y;
    unsigned short u16DispW;
    unsigned short u16DispH;
    int VideoType;
} WRT_VideoParam_t;

extern void WRT_LocalVideoDecodeExit();
extern void WRT_LocalVideoDecodeInit(WRT_VideoParam_t *pstVideoParam);

typedef struct
{
    int startcodeprefix_len;
    unsigned int len;
    unsigned int max_size;
    char *buf;
    unsigned short lost_packets;
} NALU_t;

static int info2 = 0, info3 = 0;

static int FindStartCode2 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 1))
        return 0;
    else
        return 1;
}

static int FindStartCode3 (unsigned char *Buf)
{
    if((Buf[0] != 0) || (Buf[1] != 0) || (Buf[2] != 0) || (Buf[3] != 1))
        return 0;
    else
        return 1;
}

NALU_t *AllocNALU(int buffersize)
{
    NALU_t *n;
    if ((n = (NALU_t*)calloc (1, sizeof (NALU_t))) == NULL)
    {
        printf("AllocNALU: n");
        exit(0);
    }
    n->max_size=buffersize;
    if ((n->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
    {
        free (n);
        printf ("AllocNALU: n->buf");
        exit(0);
    }
    return n;
}

void FreeNALU(NALU_t *n)
{
    if (n)
    {
        if (n->buf)
        {
            free(n->buf);
            n->buf=NULL;
        }
        free (n);
    }
}

int GetAnnexbNALU (NALU_t *nalu, MI_S32 chn)
{
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
        printf ("GetAnnexbNALU: Could not allocate Buf memory\n");
    nalu->startcodeprefix_len=3;
    if (3 != fread (Buf, 1, 3, g_pStreamFile[chn]))
    {
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2 (Buf);
    if(info2 != 1)
    {
        if(1 != fread(Buf+3, 1, 1, g_pStreamFile[chn]))
        {
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3 (Buf);
        if (info3 != 1)
        {
            free(Buf);
            return -1;
        }
        else
        {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }
    else
    {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;
    while (!StartCodeFound)
    {
        if (feof (g_pStreamFile[chn]))
        {
            nalu->len = (pos-1)-nalu->startcodeprefix_len;
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            free(Buf);
            fseek(g_pStreamFile[chn], 0, 0);
            return pos-1;
        }
        Buf[pos++] = fgetc (g_pStreamFile[chn]);
        info3 = FindStartCode3(&Buf[pos-4]);
        if(info3 != 1)
            info2 = FindStartCode2(&Buf[pos-3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }
    rewind = (info3 == 1) ? -4 : -3;
    if (0 != fseek (g_pStreamFile[chn], rewind, SEEK_CUR))
    {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }
    nalu->len = (pos+rewind);
    memcpy (nalu->buf, &Buf[0], nalu->len);
    free(Buf);
    return (pos+rewind);
}

void dump(NALU_t *n)
{
    if (!n)
        return;
    //printf(" len: %d  ", n->len);
    //printf("nal_unit_type: %x\n", n->nal_unit_type);
}

void *_VdecSendStream(void *args)
{
    MI_SYS_ChnPort_t stChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;
    MI_SYS_BUF_HANDLE hSysBuf;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_VDEC_CHN vdecChn = 0;
    MI_S32 s32TimeOutMs = 20, s32ChannelId = 0, s32TempHeight = 0;
    MI_S32 s32Ms = 30;
    MI_BOOL bVdecChnEnable;
    MI_U16 u16Times = 20000;

    MI_S32 s32ReadCnt = 0;
    FILE *readfp = NULL;
    MI_U8 *pu8Buf = NULL;
    MI_S32 s32Len = 0;
    MI_U32 u32FrameLen = 0;
    MI_U64 u64Pts = 0;
    MI_U8 au8Header[32] = {0};
    MI_U32 u32Pos = 0;
    MI_VDEC_ChnStat_t stChnStat;
    MI_VDEC_VideoStream_t stVdecStream;

    MI_U32 u32FpBackLen = 0; // if send stream failed, file pointer back length

    char tname[32];
    memset(tname, 0, 32);

#ifndef ADD_HEADER_ES
    NALU_t *n;
    n = AllocNALU(2000000);
#endif

    vdecChn = 0;
    snprintf(tname, 32, "push_t_%u", vdecChn);

    memset(&stChnPort, 0x0, sizeof(MI_SYS_ChnPort_t));
    stChnPort.eModId = E_MI_MODULE_ID_VDEC;
    stChnPort.u32DevId = 0;
    stChnPort.u32ChnId = vdecChn;//0 1 2 3
    stChnPort.u32PortId = 0;
    if ((access("720P25.h264", F_OK))!=-1)
    {
        readfp = fopen("720P25.h264", "rb"); //ES
        printf("open current dir es file\n");
    }
    else if ((access("/customer/720P25.h264", F_OK))!=-1)
    {
        readfp = fopen("/customer/alsa.conf720P25.h264", "rb"); //ES
        printf("open /customer dir es file\n");
    }
    if (!readfp)
    {
        printf("Open es file failed!\n");
        return NULL;
    }
    else
    {
        g_pStreamFile[vdecChn] = readfp;
    }

    // s32Ms = _stTestParam.stChannelInfo[s32VoChannel].s32PushDataMs;
    // bVdecChnEnable = _stTestParam.stChannelInfo[0].bVdecChnEnable;

    memset(&stBufConf, 0x0, sizeof(MI_SYS_BufConf_t));
    stBufConf.eBufType = E_MI_SYS_BUFDATA_RAW;
    stBufConf.u64TargetPts = 0;
    pu8Buf = (MI_U8 *)malloc(NALU_PACKET_SIZE);

    s32Ret = MI_SYS_SetChnOutputPortDepth(&stChnPort, 0, 3);
    if (MI_SUCCESS != s32Ret)
    {
        printf("MI_SYS_SetChnOutputPortDepth error, %X\n", s32Ret);
        return NULL;
    }

    s32Ms = 30;

    //printf("----------------------%d------------------\n", stChnPort.u32ChnId);
    while (g_VdecRun)
    {
#ifdef ADD_HEADER_ES
        memset(au8Header, 0, 16);
        u32Pos = fseek(readfp, 0, SEEK_CUR);
        s32Len = fread(au8Header, 1, 16, readfp);
        if(s32Len <= 0)
        {
            fseek(readfp, 0, SEEK_SET);
            continue;
        }
        u32FrameLen = MI_U32VALUE(au8Header, 4);
        // printf("vdecChn:%d, u32FrameLen:%d, %d\n", vdecChn, u32FrameLen, NALU_PACKET_SIZE);
        if(u32FrameLen > NALU_PACKET_SIZE)
        {
            fseek(readfp, 0, SEEK_SET);
            continue;
        }
        s32Len = fread(pu8Buf, 1, u32FrameLen, readfp);
        if(s32Len <= 0)
        {
            fseek(readfp, 0, SEEK_SET);
            continue;
        }

        stVdecStream.pu8Addr = pu8Buf;
        stVdecStream.u32Len = s32Len;
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len + 16; //back length
#else
        GetAnnexbNALU(n, vdecChn);
        dump(n);
        stVdecStream.pu8Addr = (MI_U8 *)n->buf;
        stVdecStream.u32Len = n->len;
        stVdecStream.u64PTS = u64Pts;
        stVdecStream.bEndOfFrame = 1;
        stVdecStream.bEndOfStream = 0;

        u32FpBackLen = stVdecStream.u32Len; //back length
#endif

        if(0x00 == stVdecStream.pu8Addr[0] && 0x00 == stVdecStream.pu8Addr[1]
            && 0x00 == stVdecStream.pu8Addr[2] && 0x01 == stVdecStream.pu8Addr[3]
            && 0x65 == stVdecStream.pu8Addr[4] || 0x61 == stVdecStream.pu8Addr[4]
            || 0x26 == stVdecStream.pu8Addr[4] || 0x02 == stVdecStream.pu8Addr[4]
            || 0x41 == stVdecStream.pu8Addr[4])
        {
            usleep(s32Ms * 1000);
        }

        if (MI_SUCCESS != (s32Ret = MI_VDEC_SendStream(vdecChn, &stVdecStream, s32TimeOutMs)))
        {
            //ST_ERR("MI_VDEC_SendStream fail, chn:%d, 0x%X\n", vdecChn, s32Ret);
            fseek(readfp, - u32FpBackLen, SEEK_CUR);
        }

        u64Pts = u64Pts + 33;

        if (0 == (s32ReadCnt++ % 30))
            ;// printf("vdec(%d) push buf cnt (%d)...\n", s32VoChannel, s32ReadCnt)
            ;//printf("###### ==> Chn(%d) push frame(%d) Frame Dec:%d  Len:%d\n", s32VoChannel, s32ReadCnt, stChnStat.u32DecodeStreamFrames, u32Len);
    }
    printf("\n\n");
    usleep(300000);
    if (readfp)
    {
        fclose(readfp);
        readfp = NULL;
    }
    free(pu8Buf);

    printf("End----------------------%d------------------End\n", stChnPort.u32ChnId);

    return NULL;
}

void _LocalVideoDispTest(MI_S32 s32Disp)
{
    WRT_VideoParam_t stVideoParam;
    if (s32Disp)
    {
        stVideoParam.VideoType = E_MI_VDEC_CODEC_TYPE_H264;
        stVideoParam.u16VideoW = 1280;
        stVideoParam.u16VideoH = 720;
        stVideoParam.u16X = 0;
        stVideoParam.u16Y = 0;
        stVideoParam.u16DispW = 1024;
        stVideoParam.u16DispH = 600;
        WRT_LocalVideoDecodeInit(&stVideoParam);
        g_VdecRun = TRUE;
        pthread_create(&g_VdeStream_tid, NULL, _VdecSendStream, NULL);
        printf("Bind ST_LocalCameraDisp...\n");
    }
    else
    {
        g_VdecRun = FALSE;
        pthread_join(g_VdeStream_tid, NULL);
        WRT_LocalVideoDecodeExit();
        printf("UnBind ST_LocalCameraDisp...\n");
    }

    return;
}

//==================================For video test========End============================================
#include "wrt_audio.h"

static int g_AudioRun = FALSE;
static pthread_t g_AudioTest_tid = 0;

void *_AudioTestStream(void *args)
{
    unsigned char buffer[4096];
    int audiolen = 0;

    while (g_AudioRun)
    {
        memset(buffer, 0, 4096);
        audiolen = 0;
        GetAudio(buffer, &audiolen);
        //printf("get len = %d\n", audiolen);
        if (audiolen != 0)
        {
            PlayAudio(buffer, audiolen);
        }
        usleep(10*1000);
    }

    printf("exit audio play thread\n");
}

void _LocalAudioTest(MI_S32 s32Play)
{
    if (s32Play)
    {
        if (OpenAudio(TYPE_PLAY, 1, 16000, 16, 0) == -1)
        {
            printf("open ao device fail\n");
            return;
        }
        if (OpenAudio(TYPE_CAPTURE, 1, 16000, 16, 0) == -1)
        {
            printf("open ai device fail\n");
            return;
        }
        g_AudioRun = TRUE;
        pthread_create(&g_AudioTest_tid, NULL, _AudioTestStream, NULL);
    }
    else
    {
        g_AudioRun = FALSE;
        pthread_join(g_AudioTest_tid, NULL);
        StopAudio(TYPE_PLAY);
        StopAudio(TYPE_CAPTURE);
    }

    return;
}

//==================================For audio test========Start==========================================

//=======================================================================================================
//=======================================
extern void InitBroadcastProcess();
int callbacktest(cJSON *str,cJSON **str2,char *str3,char **str4)
	{
	printf("str=%s\r\n",str3);
	}


int main(int argc ,char* argv[]) 
{
        long pos[3];
        int done = 0;
        int gpioval = 0;
        UInt8 adjustbuffer[30];
        SDL_Event event;
        struct tsdev *ts;
        int ret = 0;
        int enc_test = 0;
        MI_DISP_PubAttr_t stDispPubAttr;
        stDispPubAttr.eIntfType = E_MI_DISP_INTF_LCD;
        stDispPubAttr.eIntfSync = E_MI_DISP_OUTPUT_USER;
        sstar_disp_init(&stDispPubAttr);
#if 0 //test video
        while (1)
        {
            _LocalVideoDispTest(1);
            sleep(20);
            _LocalVideoDispTest(0);
            sleep(3);
        }
#endif 
#if 0 //test audio
    while (1)
    {
        _LocalAudioTest(1);
        sleep(10);
        _LocalAudioTest(0);
        sleep(3);
    }
#endif
        WrtLogCallbacks mycallback;
        //InitGpio(); //初始化GPIO
        //tmGpioSetPin(CTL_IO_SETBACKLIGHT_POWER_ON,1);

        //tmGpioSetPin(CTL_IO_SET_USB1_ON,0);//add by wyx :enable wifi(usb1)
        //tmGpioSetPin(CTL_IO_SET_ALARMOUT_OFF,1);

        //SetSubMicSpkOFF();
        //SetMainMicSpkON();
        setinittime();
       
        //system("amixer cset numid=10,iface=MIXER,name='ADC PGA Gain' 1");
        g_red = 0;
        g_green = 0;
        
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0){
        	printf("error: %s\n",SDL_GetError());
        	return 0;
        }
        g_pScreenSurface = SDL_SetVideoMode(1024,600,32,SDL_SWSURFACE);
        if(g_pScreenSurface == NULL){
        	SDL_Quit();
        	return 0;
        }
        
        tmp_g_pScreenSurface = SDL_CreateRGBSurface(SDL_SWSURFACE,300,200,32,0xFF0000, 0x00FF00, 0x0000FF,0xFF000000);
		if(tmp_g_pScreenSurface == NULL)
		{
			SDL_Quit();
			return 0;
		}
		
		SDL_ShowCursor(0);//不使用光标
        WRT_InitFont("/home/wrt/font.ttf");
        
        zenMemInit(0); //初始化内存模块
        pSystemInfo = NULL;
        InitSystemInfo();
        pSystemInfo = GetSystemInfo();
#if 0
		unsigned short year = 2015;
		memcpy(pSystemInfo->BootInfo.Version+4,&year,2);
		pSystemInfo->BootInfo.Version[1] = 3;
		pSystemInfo->BootInfo.Version[2] = 21;
		pSystemInfo->BootInfo.Version[3] = 00;
		pSystemInfo->BootInfo.Version[6] = 7;
		pSystemInfo->BootInfo.Version[7] = 20;
#endif
		pSystemInfo->LocalSetting.privateinfo.LocalID[15] = '\0';
		p_MorecfgInfo = NULL;
		InitSystemMoreInfo();
		p_MorecfgInfo = GetSystemMoreInfo();
		wrt_log_init();

		wrt_set_log_id((char*)pSystemInfo->LocalSetting.privateinfo.LocalID);	
		wrt_set_log_remote(0); //默认关闭远程服务
		wrt_set_log_level_mask(LOG_ERROR);//(LOG_MESSAGE|LOG_DEBUG|LOG_ERROR);//
		wrt_set_log_server_addr(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,60000);

        g_have_defend = Get_safe_status();
		pSystemInfo->mySysInfo.doorcallproir = 0;
        g_cur_set_scene_mode = pSystemInfo->mySysInfo.cur_scene;

        //WRT_MESSAGE("打开音频设置\n");
        //open_alsa_dev();
		//wrthost_initWatchDog();
      	ts = ts_open("/dev/input/event0",0);
		ts_config(ts);
		struct ts_sample sample;
		int ts_flag = 1;
		int tempx,tempy;

		mycallback.malloc = zenMalloc;
		mycallback.free = zenFree;
		mycallback.realloc = zenRealloc;
		wrt_set_log_memory_adapter(&mycallback);

        pSystemInfo->mySysInfo.isusedffs = 1; //必须使用文件系统
        
        if(argv[1] == NULL)
        	test_view_start_interface();

		//SDL_Delay(000);
		//test_show(enc_test);
		//SDL_Delay(2000);

		//tmGpioSetPin(CTL_IO_SETCMOS_POWER_OFF,0); //关闭本机摄像头
		//tmGpioSetPin(CTL_IO_SETLED_RED,0); //红灯灭
		//tmGpioSetPin(CTL_IO_SETLED_GREEN,1);//绿灯灭

        if(g_MainProcess){
                delete g_MainProcess;
                g_MainProcess = NULL;
        }

        g_MainProcess = new CMainProcess();
        if(g_MainProcess){
        
            g_MainProcess->StartThread();//main_program threadmain

            g_MainProcess->Start();//main_program init
            while(1){
                if(g_MainProcess->IsStart()) //等待mainprocess任务起来
                    break;
                SDL_Delay(1000);
            }
        }

#if HAVE_SCREEN_KEEP     //开机就启动init
		//screenkeep_init(); //加载屏保图static int isadjustscreen = 0;片。
#endif

#ifdef TEST_WEB_SERVER
        //Web_Init();
#endif
//	system("ifconfig eth0 down");
//	system("ifconfig eth0 hw ether 00:0c:0d:11:22:44");
//	system("ifconfig eth0 192.168.1.171");
//	system("ifconfig eth0 up");
	//InitArpSocket();
	//send_free_arp();

	SDL_CreateThread(net_check_task,NULL);
	//SDL_AddTimer(1000,GpioControl,NULL);//定时控制灯闪
	SDL_Delay(200);
   	//create_macsn_thread();//set Mac Thread	

	//InitBroadcastProcess();
	gstarttick = SDL_GetTicks();
	gendtick = SDL_GetTicks();

	unsigned long aaa = 0;
	g_isscreencal = 0;

	unsigned long ip=0;

	int i=0;
	//get_host_gateway(&ip);
	//i = net_test_ping(ip);
	//if(i == 0)
	{
	//	ip = 0;
	//	wrthost_get_managerip(&ip);
	//	i = net_test_ping(ip);
	}
	
	//if(i == 1)
	//	g_net_is_ok = true;
	//else
	//	g_net_is_ok = false;
		
	//UpdateNetIcon();

//2015 1 16 暂时注释掉不是标配的功能
	//Wifi_Config_Process();//wyx 2014-11-12
	int last_x = 0;
	int last_y = 0;

	#if 1//xiao

	initTcp(tcpServerToMainCallback);
	#endif
	while(!done){
		if(g_isscreencal == 1)
		{
			ts = NULL;
			ts = ts_open("/dev/input/event0",0);
			ts_config(ts);
			g_isscreencal = 0;
		}
		
			ret =ts_read(ts,&sample,1);
			
		if(ret <= 0)
			continue;
			
		gstarttick = SDL_GetTicks();

		//printf("x:%d__y:%d__presse:%d\n",sample.x,sample.y,sample.pressure);

		
		if(ts_flag == 1 && sample.pressure >= 1)
		{
			ts_flag = 0;
		}
		
		if(ts_flag == 0 && sample.pressure == 0)
		{
			if(m_issreenoff == true){
				if(sample.pressure  == 0){
					if(m_issreenoff == true) //非屏保状态，是黑屏
					{
						screen_open();
						screenkeep_stop();
						send_free_arp();
						SDL_Delay(300);
						g_MainProcess->NoitfyReDraw(1);
					}
				}
				continue;
			}else
			{
				if(g_ts_send == 0)
				{
					process_mouse_msg(sample.x,sample.y,1);
					SDL_Delay(100);
					process_mouse_msg(sample.x,sample.y,0);
				}
			}
			ts_flag =1;
		}
	}

    if(g_MainProcess){
        delete g_MainProcess;
        g_MainProcess = NULL;
    }

    zenMemUninit();
    SDL_Quit();
    return 0;
}

static int net_check_task(void* pvoid){

        int nCount = 3;
        int pingok = 0;
        int nSequence=0;
        int nMessageSize = 0;
        ICMPheader sendHdr;
        char* pSendBuffer;
        char* pRecvBuffer;

        int sock;
        sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);	//Create a raw socket which will use ICMP

        sockaddr_in dest;//Dest address to send the ICMP request
        dest.sin_addr.s_addr = pSystemInfo->LocalSetting.privateinfo.GateWayIP;
        dest.sin_family = AF_INET;
        dest.sin_port = rand ();	//Pick a random port

        int nResult = 0;
        int count = 10;
        fd_set fdRead;
        timeval timeInterval = {0, 0};
        timeInterval.tv_usec = 5000 * 1000;

        sendHdr.nId = htons (rand ());	//Set the transaction Id
        pSendBuffer = (char*)ZENMALLOC(sizeof (ICMPheader) + nMessageSize);
        pRecvBuffer = (char*)ZENMALLOC(1500);
        while (count--)
        {
                //Create the message buffer, which is big enough to store the header and the message data
                dest.sin_addr.s_addr = pSystemInfo->LocalSetting.privateinfo.GateWayIP;
                dest.sin_family = AF_INET;
                dest.sin_port = rand ();	//Pick a random port

                memset(pSendBuffer,0,sizeof (ICMPheader) + nMessageSize);
                sendHdr.byCode = 0;	//Zero for ICMP echo and reply messages
                sendHdr.nSequence = htons (nSequence++);
                sendHdr.byType = 8;	//Eight for ICMP echo message  //8 - 请求回显
                sendHdr.nChecksum = 0;	//nChecksum is calculated later on

                memcpy (pSendBuffer, &sendHdr, sizeof (ICMPheader));	//Copy the message header in the buffer
                memset (pSendBuffer + sizeof (ICMPheader), 'x', nMessageSize);	//Fill the message with some arbitary value

                //Calculate checksum over ICMP header and message data
                sendHdr.nChecksum = htons (CalcChecksum (pSendBuffer, sizeof (ICMPheader) + nMessageSize));

                //Copy the message header back into the buffer
                memcpy (pSendBuffer, &sendHdr, sizeof (ICMPheader));

                nResult = sendto (sock, pSendBuffer, sizeof (ICMPheader) + nMessageSize, 0, (sockaddr *)&dest, sizeof (sockaddr));

                //Save the time at which the ICMP echo message was sent

                if (nResult <0)
                {
                        //closesocket(sock);
                        SDL_Delay(5000);
                        continue;
                }

                FD_ZERO (&fdRead);//fd_set fdread
                FD_SET (sock, &fdRead);

                if ((nResult = select (sock+1, &fdRead, NULL, NULL, &timeInterval))<=0)
                {
                        //closesocket(sock);
                        SDL_Delay(5000);
                        continue;
                }

                if (nResult > 0 && FD_ISSET (sock, &fdRead))
                {
                        //Allocate a large buffer to store the response
                        memset(pRecvBuffer,0,1500);

                        if ((nResult = recvfrom (sock, pRecvBuffer, 1500, 0, 0, 0)) <= 0)
                        {
                                //closesocket(sock);
                                SDL_Delay(5000);
                                continue;
                        }


                        //We got a response so we construct the ICMP header and message out of it
                        ICMPheader recvHdr;
                        char *pICMPbuffer = NULL;

                        //The response includes the IP header as well, so we move 20 bytes ahead to read the ICMP header
                        pICMPbuffer = pRecvBuffer + sizeof(IPheader);

                        //ICMP message length is calculated by subtracting the IP header size from the
                        //total bytes received
                        int nICMPMsgLen = nResult - sizeof(IPheader);

                        //Construct the ICMP header
                        memcpy (&recvHdr, pICMPbuffer, sizeof (recvHdr));

                        //Construct the IP header from the response
                        IPheader ipHdr;
                        memcpy (&ipHdr, pRecvBuffer, sizeof (ipHdr));

                        recvHdr.nId = recvHdr.nId;
                        recvHdr.nSequence = recvHdr.nSequence;
                        recvHdr.nChecksum = ntohs (recvHdr.nChecksum);

                        //Check if the response is an echo reply, transaction ID and sequence number are same
                        //as for the request, and that the checksum is correct
                        if (recvHdr.byType == 0 &&
                                recvHdr.nId == sendHdr.nId &&
                                recvHdr.nSequence == sendHdr.nSequence &&
                                ValidateChecksum (pICMPbuffer, nICMPMsgLen)  &&
                                memcmp (pSendBuffer + sizeof(ICMPheader), pRecvBuffer + sizeof (ICMPheader) + sizeof(IPheader),
                                nResult - sizeof (ICMPheader) - sizeof(IPheader)) == 0)
                        {
                                //All's OK
                                ReDrawCurrentWindow2(5);
                                break;
                        }
                }
        }
        ZENFREE(pSendBuffer);
        ZENFREE(pRecvBuffer);
        closesocket(sock);
      	return 0;
}

