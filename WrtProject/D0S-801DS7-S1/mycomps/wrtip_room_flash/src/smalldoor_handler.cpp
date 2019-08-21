//#include <tmNxTypes.h>
//#include <pna.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include <fcntl.h>      
#include <termios.h>   
#include "public.h"
//#include <tmosal.h>
#include "wrt_log.h"
//#include "tmVideoDisp.h"
#include "cmd_handler.h"
#include "sock_handler.h"
#include "audiovideo.h"
//#include "zlib.h"
#include "info_list_handler.h"
//#include "tmGpio485.h"
#include "smalldoor_handler.h"
//#include "tmdlAo.h"
#include "RoomGpioUtil.h"
#include "zenmalloc.h"
//#include "tmdlUartGpio.h"
//#include "tmUartSupport.h"
#include "tmSysConfig.h"
#include "warningtask.h"
#include "enet/enet.h"



//#include <AudioInLib.h>
//#include <AudioOutLib.h>
#include "bell.h"

#define closesocket close
#define ioctlsocket ioctl

extern T_SYSTEMINFO* pSystemInfo;
extern CCmdHandler*    m_pCmdHandler;
extern CSocketManager*   m_pSocketManager;

static int g_isalarm = 0;

static int g_serial_fd = -1;

#if 0
#if defined(__cplusplus)
extern "C" {
#endif



        /********************************************************************/
        //-------------------------------------------------------------------------
        //
        //     函数申明
        //
        //-------------------------------------------------------------------------
        //

        tmErrorCode_t tmVideoCap_Init();

        tmErrorCode_t tmVideoCap_Close();

        tmErrorCode_t tmVideoCap_Start();

        tmErrorCode_t  tmVideoCap_Stop();

        tmErrorCode_t tmVideoCap_Restart();

        tmErrorCode_t tmVideoCap_GetData(void **id,unsigned char **Y,int *Y_size,
                unsigned char **U,int *U_size,
                unsigned char **V,int *V_size);

        tmErrorCode_t tmVideoCap_ReturnData(void *id);

        tmErrorCode_t Vcodec_GetData(int inst,void **id,unsigned char **Y,int *Y_size,
                unsigned char **U,int *U_size,
                unsigned char **V,int *V_size);

        tmErrorCode_t Vcodec_ReturnData(int inst,void *id,unsigned char *Y,unsigned char *U,
                unsigned char *V);

        int iptv_image_write_jpeg_420(unsigned char *y,unsigned char *u,unsigned char *v,int width,int height,unsigned char *outputbuf,int quality,int destwidth);

        void smprint();

#if defined(__cplusplus)
}
#endif

#endif

/////////////////////////////////////////////////////////////////////////
//小门口事件
#define SMALLDOOR_START_VIDEO_EVENT         0x01
#define SMALLDOOR_STOP_VIDEO_EVENT          0x02

#define SMALLDOOR_START_AUDIO_EVENT         0x03
#define SMALLDOOR_STOP_AUDIO_EVENT          0x04

#define SMALLDOOR_AUTO_CAPPHOTO             0X05
#define SMALLDOOR_CANCEL_AUTO_CAPPHOTO      0X06
#define SMALLDOOR_CAPPHOTO                  0x07

#define SMALLDOOR_VIDEO_TO_PHONE_START      0X08
#define SMALLDOOR_VIDEO_TO_PHONE_STOP       0X09 


//#define SMALLDOOR_SEND_PHOTO_EVENT          0X08

/////////////////////////////////////////////////////////////////////////
void ProcessSmallCmd(unsigned char* recvbuffer);
void send_smalldoor_cmd_485(unsigned short event[4]);
void send_485_msg(unsigned long msg[4]);

static CWRTMsgQueue    g_sps_msg_queue;
static CWRTMsgQueue    g_smalldoor_msg_queue;
static CWRTMsgQueue    g_sdsc_msg_queue;
static CWRTMsgQueue    g_sdsc2_msg_queue;
static SDL_sem*        g_sps_msg_sem = NULL;
static SDL_sem*        g_smalldoor_msg_sem = NULL;
static SDL_sem*        g_sdsc_msg_sem = NULL;
static SDL_sem*        g_sdsc2_msg_sem = NULL;
/*
static unsigned long       g_sps_qid = -1;
static unsigned long       g_sps_tid = -1;

static unsigned long       g_smalldoor_qid = -1 ;                //出来小门口任务的消息队列
static unsigned long       g_smalldoor_tid = -1;                   //小门口机任务的任务ID
static unsigned long       g_smalldoor_video_tid = -1;             //小门口机视频处理任务


//  [2011年3月21日 8:44:43 by ljw]
static unsigned long       g_smalldoor_send_cmd_tid = -1;
static unsigned long       g_sdsc_qid1 = -1;
static unsigned long       g_sdsc_qid2 = -1;
*/
//static unsigned long       g_smalldoor_audio_tid =-1;
extern int GetPhotoQuailty();//audiovideo.cpp
//extern void relase_jpeg_frame(JPEGFRAME* p); //audiovideo.cpp

int  g_alarmtype = 0;

extern CCmdHandler*      m_pCmdHandler;
extern int               g_localid;
#ifdef HAVE_SUB_PHONE
extern int g_phone_connect_index;
#endif

static bool               g_bsmalldoor_video=false;


static int capture_smalldoor_photo_frame(unsigned char* picturebuf,unsigned char* y,unsigned char* u,unsigned char* v,JPEGFRAME** frameheader){
        unsigned long tmpevent[4];
        tmpevent[0] = STATUS_DISPLAY_REC;
        tmpevent[1] = 0 ;
        tmpevent[2]= tmpevent[3] = 0;
        if(m_pCmdHandler)
                m_pCmdHandler->CallDisplayCallBack(tmpevent);
        unsigned char* outbuf = picturebuf;
        unsigned long outbuflen = 0;// = iptv_image_write_jpeg_420(y,u,v,480,240,outbuf,GetPhotoQuailty(),480); 
        if(outbuflen == 0)
	        return 0;
        if(*frameheader == NULL){

                *frameheader = (JPEGFRAME*)ZENMALLOC(sizeof(JPEGFRAME));
                if((*frameheader) == NULL)
                        return 0;
                memset((*frameheader),0,sizeof(JPEGFRAME));
                (*frameheader)->data = (unsigned char*)ZENMALLOC(outbuflen);
                if((*frameheader)->data == NULL){
                        ZENFREE((*frameheader));
                        *frameheader = NULL;
                        return 0;
                }
                (*frameheader)->size = outbuflen;
                memcpy((*frameheader)->data,outbuf,outbuflen);
                WRT_MESSAGE("分配小门口机frameheader  = %x",*frameheader);
        }else{
                JPEGFRAME *p,*p1,*p2;
                p = (JPEGFRAME*)ZENMALLOC(sizeof(JPEGFRAME));
                memset(p,0,sizeof(JPEGFRAME));
                p->data = (unsigned char*)ZENMALLOC(outbuflen);
                if(p->data == NULL){
                        ZENFREE(p);
                        return 0;
                }
                p->size = outbuflen;
                memcpy(p->data,outbuf,outbuflen);

                p1 = (*frameheader);
                while(p1){
	                p2 = p1;
	                p1 = p1->next;
                }
                p2->next = p;
        }

        return outbuflen;

}


static void capture_smalldoor_photo(unsigned char* picturebuf,unsigned char* y,unsigned char* u,unsigned char* v,int issend){
        uint32_t  times,dates;
        uint32_t day,Month,years;
        uint32_t min,second,Hour;
        PHOTOINFO* pTemp =0;
        pTemp =  get_idle_photo();
        if(pTemp == 0){
                int i =1;
                while(1){
                        pTemp = get_select_photo(MAX_PHOTO-i);
                        if(pTemp){
                                if(pTemp->idle == 1 && pTemp->lock == 0)
                                        break;
                        }
                        i++;
                }
                if(pTemp)
                        del_photo(pTemp);
                pTemp =  get_idle_photo();
        }
        if(pTemp){

                unsigned char* outbuf = picturebuf;/*get_idle_photo_addr();*///(unsigned char*)ZENMALLOC(100*1024);
                unsigned long outbuflen = 0;//iptv_image_write_jpeg_420(y,u,v,480,240,outbuf,GetPhotoQuailty(),480);//480,240
                WRT_MESSAGE("抓拍小门口机视频size:%d",outbuflen);
                if(pTemp && outbuflen <= (100*1024)){
                        tm_get(&dates,&times,NULL);
                        day= dates&0xff;
                        Month = (dates >> 8) &0xff;
                        years = (dates>>16) & 0xffff;
                        Hour = (times >> 16)& 0XFFFF;
                        second = (times & 0xff);
                        min = (times >> 8) &0xff;
                        sprintf((char*)pTemp->name,"pic_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".jpg");
                        unsigned char* picbuf = (unsigned char*)ZENMALLOC(outbuflen);

                        if(picbuf){
                                memcpy(picbuf,outbuf,outbuflen);
                                pTemp->read |= ((1 << 16) & 0xffff0000);
                                add_photo(pTemp,picbuf,outbuflen);
                                unsigned long tmpevent[4];
                                tmpevent[0] = STATUS_CAP_NEW_PHONE;
                                tmpevent[1] = 0 ;
                                tmpevent[2]= tmpevent[3] = 0;
                                if(m_pCmdHandler){
                                        m_pCmdHandler->CallDisplayCallBack(tmpevent);
                                }
                        }
                        if(issend){
                                unsigned char* sendbuf = (unsigned char*)ZENMALLOC(outbuflen);
                                if(sendbuf){
                                        //  unsigned long evt[4];
                                        memcpy(sendbuf,outbuf,outbuflen);
                                        send_voice_photo_to_center(2,ROOM_SEND_PHOTO_VOICE,sendbuf,outbuflen);
                                        //  evt[0] = SMALLDOOR_SEND_PHOTO_EVENT;
                                        //  evt[1] = (unsigned long)sendbuf;
                                        //  evt[2] = outbuflen;
                                        //  q_send(g_sps_qid,evt);

                                }
                        }

                }
        }
}

//2009-7-14
void send_voice_photo_to_center(int type,int cmd,unsigned char* buf,int buflen){
        unsigned long msg[4];
        memset(msg,0,sizeof(msg));
        msg[0] = cmd;
        msg[1] = (unsigned long)buf;
        msg[2] = buflen;
        msg[3] = type;
        g_sps_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_sps_msg_sem);
       
}

void send_control_cmd_to_center(unsigned long event[4]){
       g_sps_msg_queue.send_message(MODULE_MSG,(void*)event,sizeof(event),g_sps_msg_sem);
}

int smalldoor_photo_send_task(void* pvoid){
        unsigned long msg[4];
        int err = 0;
        CWRTMsg* pMsg = NULL;
        g_sps_msg_sem = SDL_CreateSemaphore(0);
        assert(g_sps_msg_sem);
        while(1){
                memset(msg,0,sizeof(msg));
                if(SDL_SemWait(g_sps_msg_sem) == 0){
                	pMsg = g_sps_msg_queue.get_message();
                	if(pMsg){
                		err = parse_msg(pMsg,msg);
                		if(err < 0)
                			break;
                		if(err == 0)
                			continue;	
                	}

                        if(msg[0] == ROOM_SEND_PHOTO_VOICE){
                                unsigned char* packbuf = NULL;
                                unsigned char* tmp = (unsigned char*)msg[1];
                                if(tmp == NULL)
                                        continue;
                                int cmdlen = 41+msg[2];
                                packbuf = (unsigned char*)ZENMALLOC(cmdlen);
                                if(packbuf == NULL){
                                        ZENFREE(tmp);
                                        continue;
                                }

                                short cmd = ROOM_SEND_PHOTO_VOICE;
                                unsigned char uc = msg[3]& 0xff;
                                cmd =htons(cmd);
                                memset(packbuf,0,cmdlen);
                                strncpy((char*)packbuf,"WRTI",4);
                                memcpy(packbuf+4,&cmdlen,4);
                                memcpy(packbuf+8,&cmd,2);
                                memcpy(packbuf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                memcpy(packbuf+25,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                memcpy(packbuf+40,&uc,1);
                                memcpy(packbuf+41,(void*)tmp,msg[2]);
                                ZENFREE(tmp);
                                m_pSocketManager->SendResultToCenter(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,packbuf,cmdlen);
                                ZENFREE(packbuf);
                                packbuf = NULL;
                                continue;
                        }
                        if(msg[0] == ROOM_REMOTE_SWITCH){
                                int cmdlen = 42;
                                unsigned char packbuf[42];
                                short cmd = ROOM_REMOTE_SWITCH;
                                unsigned char uc = msg[1]& 0xff;
                                unsigned char uc1 = msg[2]& 0xff;
                                cmd =htons(cmd);

                                memset(packbuf,0,cmdlen);
                                strncpy((char*)packbuf,"WRTI",4);
                                memcpy(packbuf+4,&cmdlen,4);
                                memcpy(packbuf+8,&cmd,2);
                                memcpy(packbuf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                memcpy(packbuf+25,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                memcpy(packbuf+40,&uc,1);
                                memcpy(packbuf+41,&uc1,1);
                                m_pSocketManager->SendResultToCenter(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,packbuf,cmdlen);
                                continue;
                        }
                        if(msg[0] == ROOM_SEND_COLOR_MSG){
                                int cmdlen = 40;
                                unsigned char packbuf[40];
                                short cmd = ROOM_SEND_COLOR_MSG;
                                cmd =htons(cmd);
                                memset(packbuf,0,cmdlen);
                                strncpy((char*)packbuf,"WRTI",4);
                                memcpy(packbuf+4,&cmdlen,4);
                                memcpy(packbuf+8,&cmd,2);
                                memcpy(packbuf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                                m_pSocketManager->SendResultToCenter(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,packbuf,cmdlen);
                                continue;
                               // memcpy(packbuf+25,pSystemInfo->LocalSetting.privateinfo.LocalID,15);                        	
                        }
                }
        }
        g_sps_msg_queue.release();
        SDL_DestroySemaphore(g_sps_msg_sem);
        g_sps_msg_sem = NULL;
}

#ifdef HAVE_SUB_PHONE
static int connect_phone(unsigned long ip,short port)
{
        int result;
        int sd;
        struct sockaddr_in from,s_in,to;
        int fromlen;
        char *buf;
        int reuse;
        unsigned long sendip;
        short    sendport;
        unsigned long msg[4];
        int size =0;
        int flag = 1;

        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd < 0)
        {
                WRT_WARNING("创建连接手机SOCKET失败0x%x",errno);
                return -1;
        }



        int mode = 1;
        ioctlsocket(sd, FIONBIO, (char *)&mode);
        struct timeval timeout;

        fd_set writefd;

        to.sin_family      = AF_INET;
        to.sin_addr.s_addr = ip;
        to.sin_port        = htons(port);
        if(connect(sd,(struct sockaddr*)&to,sizeof(struct sockaddr_in)) < 0)
        {

                if(errno  != EINPROGRESS){
                        WRT_DEBUG("链接地址 %x:%d 失败0x%x",ip,port,errno);
                        closesocket(sd);

                        return -1;
                }
        }
        timeout.tv_sec = 3;
        timeout.tv_usec =0;
        FD_ZERO(&writefd);
        FD_SET(sd,&writefd);
        if((result = select(sd+1,NULL,&writefd,NULL,&timeout)) > 0){
                if(FD_ISSET(sd,&writefd))
                {
                        WRT_DEBUG("链接地址 %x:%d 成功 ",ip,port);
                        return sd;
                }
        }
        WRT_DEBUG("链接地址 %x:%d select失败0x%x result = %d",ip,port,errno,result);
        closesocket(sd);
        sd = -1;

        return sd;
}

static int send_sm_video_phone(int sd,unsigned char* pitcurebuf,unsigned char* y,unsigned char* u,unsigned char* v)
{

       unsigned char* outbuf = pitcurebuf;/*get_idle_photo_addr();*///(unsigned char*)ZENMALLOC(100*1024);
        unsigned long outbuflen = 0;//iptv_image_write_jpeg_420(y,u,v,480,288,outbuf+40,GetPhotoQuailty(),352);//480,240

        struct timeval timeout;
        fd_set writefd;
		fd_set readfd;
        int sendlen =0;
        int result =0;
        int ttlen = 0;
		char buf[2];

        FD_ZERO(&writefd);
        FD_SET(sd,&writefd);
		FD_SET(sd,&readfd);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
		

        strncpy((char*)outbuf,"WRTI",4);
        ttlen = outbuflen + 40;
        memcpy((char*)outbuf+4,&ttlen,4);
        short tcmd = htons(0x0019);
        memcpy((char*)outbuf+8,&tcmd,2);
        strncpy((char*)outbuf+10,"000000000000000",15);
        memcpy((char*)outbuf+25,pSystemInfo->LocalSetting.privateinfo.LocalID,15);

        while(1){
                result = select(sd+1, NULL, &writefd, NULL, &timeout);
                if(result == 0)
                {
                        WRT_DEBUG("发送小门口机视频select 超时0x%x,sd = %d",errno,sd);
                        break;
                }else if(result < 0)
                {
                       WRT_DEBUG("发送小门口机视频select  错误0x%x sd=%d",errno,sd);
                        return -1;
                }
                if(FD_ISSET(sd, &writefd))
                {

                        result = send(sd,(char*)outbuf+sendlen,ttlen-sendlen,0);
                        if(result < 0){
                                WRT_DEBUG("发送小门口机视频 Send 错误 0x%x",errno);
                                return -1;
                        }
                        sendlen  +=  result;
                        if(sendlen == ttlen)
                                break;
                }

        }
        return 0;
}
#endif

int smalldoor_warningtest_task(void *pvoid)
{
	int i =1;
	char ch;
	while(1)
	{
		ch = getchar();
		printf("报警测试 %d\n",ch-48);
#if 1
		unsigned long tmpevent[4];
		tmpevent[0] = STATUS_SAFE_WARNING;
		tmpevent[1] = ch-48;
		tmpevent[2] = tmpevent[3] = 0;
		if(m_pCmdHandler)
			m_pCmdHandler->CallDisplayCallBack(tmpevent);
#endif
			i++;

	}
}

int smalldoor_video_task(void* pvoid){
        bool isplay=false;
        unsigned long evt[4];
        int tmpcount = 0;
        int i,j,l,m;
        uint32_t starttime =0;
        int istime = 0;
        bool  bcapphoto = false;
        int issend=0;
        int err = 0;
        CWRTMsg* pMsg = NULL;
        unsigned char* picutre_buf = (unsigned char*)ZENMALLOC(100*1024);
        unsigned long rc = 0;
        int framenum = pSystemInfo->mySysInfo.framerate * CAP_FRAME_TIME;
        bool bcapphotoframe = false;
        JPEGFRAME* frameheader = NULL;
        int  frametotalsize  = 0;
        int  frametime = 0;
        int  frametotalnum = 0;
        uint32_t frameticks1,frameticks2;
#ifdef HAVE_SUB_PHONE
        int sd = -1;
        int intervealvedframe = 0;
        unsigned long phoneip=0;
        short phoneport = 0;        
#endif
        if(framenum > 60 || framenum < 30)
                framenum  = 30;
         g_smalldoor_msg_sem = SDL_CreateSemaphore(0);
         assert(g_smalldoor_msg_sem);

        while(1){
                evt[0] = evt[1] = evt[2] = evt[3] = 0;
                issend = 0;
                if(isplay == false){
                        WRT_MESSAGE("小门口视频任务等待");
                        //q_receive(g_smalldoor_qid,Q_WAIT,0,evt);
                        err =SDL_SemWait(g_smalldoor_msg_sem);
			
                }else{
                	err = SDL_SemTryWait(g_smalldoor_msg_sem);
                }
                if(err == 0){
                         pMsg = g_smalldoor_msg_queue.get_message();
                        if(pMsg){
                        	err = parse_msg(pMsg,evt);
                        	if(err < 0)
                        		break;
                        	if(err == 0)
                        		continue;
                        	
                        }
                }

                if(evt[0] == SMALLDOOR_START_VIDEO_EVENT){

                        if(isplay == true){

                                continue;
                        }
                        isplay = true;

                        //  tm_wkafter(100);
                        WRT_MESSAGE("抓捕小门口机视频");
                        if(pSystemInfo->mySysInfo.screensize == 0x10)
                        	;
				//tmVideoDisp_SetMode(1);
                        if(pSystemInfo->mySysInfo.screensize == 0x7)
							;//tmVideoDisp_SetMode(0);

                        //  tmVideoDisp_Start(480, 288, 1);
                        //  tm_wkafter(100);

                }else if(evt[0] == SMALLDOOR_STOP_VIDEO_EVENT){
                        unsigned long tmpevent[4];
                        isplay = false;
                        //relase_jpeg_frame(frameheader);
                        frameheader = NULL;
                        tmpevent[0] = STATUS_DISPLAY_REC;
                        tmpevent[1] = 1 ;
                        tmpevent[2]= tmpevent[3] = 0;
                        if(m_pCmdHandler)
                                m_pCmdHandler->CallDisplayCallBack(tmpevent);
                        WRT_MESSAGE("停止小门口机视频");
#ifdef HAVE_SUB_PHONE
                        if(sd != -1)
                        {
                                closesocket(sd);
                                sd = -1;
                        }
#endif
                        continue;
                }else if(evt[0] == SMALLDOOR_CAPPHOTO){
                        if(evt[1] == 2 ){
                                if(bcapphotoframe == false){

                                        bcapphotoframe = true;
                                        framenum = pSystemInfo->mySysInfo.framerate * CAP_FRAME_TIME;
                                        if(framenum > 60 || framenum < 30)
                                                framenum  = 30;
                                        frametotalsize = 0;
                                        frametotalnum  = framenum;
                                        frametime =  (CAP_FRAME_TIME*1000)/framenum;
                                        frameticks2 = frameticks1 = 0;
                                        tm_getticks(NULL,&frameticks1);
                                        WRT_MESSAGE("开始录制小门口机视频");
                                }
                        }else
                                bcapphoto = true;
                        continue;
                }else if(evt[0] == SMALLDOOR_AUTO_CAPPHOTO){
                        istime = 1;
                        tm_getticks(NULL,&starttime);
                        continue;
                }else if(evt[0] == SMALLDOOR_VIDEO_TO_PHONE_START)
                {
#ifdef HAVE_SUB_PHONE	
                        WRT_MESSAGE("启动小门机视频发送到手机");
                        if(sd != -1)
                        {
                                closesocket(sd);
                                sd = -1;
                        }
                        phoneip = evt[1];
                        phoneport = evt[2];
                        sd = connect_phone(evt[1],evt[2]);
                        if(sd == -1)
                        {
                                WRT_DEBUG("小门机视频connect to phone %08x:%d failed \n",evt[1],evt[2]);

                        }
#endif
                }else if(evt[0] == SMALLDOOR_VIDEO_TO_PHONE_STOP)
                {
#ifdef HAVE_SUB_PHONE
                        WRT_MESSAGE("停止小门机视频发送到手机");
                        if(sd != -1)
                        {
                        	phoneip = phoneport=0;
                                closesocket(sd);
                                sd = -1;
                        }
#endif
                }

                if(istime == 1){
                        uint32_t nowtime = 0;
                        tm_getticks(NULL,&nowtime);
                        if((nowtime - starttime) > 5000){
                                if(evt[0] != SMALLDOOR_CANCEL_AUTO_CAPPHOTO){
                                        bcapphoto = true;
                                        istime = 0;
                                        issend =1;
                                }
                        }
                }
                if(evt[0] == SMALLDOOR_CANCEL_AUTO_CAPPHOTO){
                        bcapphoto = false;
                        istime = 0;
                }
                if(isplay == true){

#if 0
                        tmErrorCode_t err;
                        unsigned char *Y,*U,*V, *yuv;
                        int Y_size,U_size,V_size;
                        void *id;
                        err = Vcodec_GetData(NULL,&id,&Y,&Y_size,&U,&U_size,&V,&V_size);

                        if (err == TM_OK) {
                                yuv = tmVideoDisp_Lock();//p

                                if(yuv == NULL){
                                        Vcodec_ReturnData(NULL,id,Y,U,V);
                                        WRT_WARNING("tmVideoDisp_Lock == NULL");
                                        tm_wkafter(35);
                                        continue;
                                }


                                memcpy(yuv, Y, 138240);
                                memcpy(yuv+138240, U, 34560);
                                memcpy(yuv+138240+34560, V, 34560);
                                
				//if(g_phone_connect_index == -1)
                                tmVideoDisp_Config(352, 288, yuv, yuv+138240, yuv+138240+34560, 480, 240);//

                                if(bcapphoto == true){
                                        capture_smalldoor_photo(picutre_buf,yuv,yuv+138240,yuv+138240+34560,issend);
                                        bcapphoto = false;
                                }
                                if(bcapphotoframe == true){
                                        tm_getticks(NULL,&frameticks2);
                                        if((frameticks2 - frameticks1) > frametime ){
                                                frameticks1 = frameticks2;
                                                frametotalsize += capture_smalldoor_photo_frame(picutre_buf,yuv,yuv+138240,yuv+138240+34560,&frameheader);
                                                framenum--;
                                                if(framenum == 0){
                                                        bcapphotoframe = false;
                                                        WRT_MESSAGE("add_vod,frameheader  = %x size = %d,frametime = %d,framenum = %d ",frameheader,frametotalsize,frametime,frametotalnum);
                                                        add_vod(frameheader,frametotalsize,frametotalnum,1);
                                                        relase_jpeg_frame(frameheader);
                                                        frameheader = NULL;
                                                        unsigned long tmpevent[4];
                                                        tmpevent[0] = STATUS_DISPLAY_REC;
                                                        tmpevent[1] = 1 ;
                                                        tmpevent[2]= tmpevent[3] = 0;
                                                        if(m_pCmdHandler)
                                                                m_pCmdHandler->CallDisplayCallBack(tmpevent);
                                                }
                                        }

                                }
#ifdef HAVE_SUB_PHONE
                                if(sd != -1)
                                {

                                        int ret = send_sm_video_phone(sd,picutre_buf,yuv,yuv+138240,yuv+138240+34560);
                                        if(ret == -1)
                                        {
                                                WRT_DEBUG("send smalldoor video to phone Failed ");
                                                closesocket(sd);
                                                sd = -1;
                                                if(phoneip !=0 && phoneport !=0){
                                                        sd = connect_phone(phoneip,phoneport);
                                                        if(sd == -1)
                                                        {
                                                                WRT_DEBUG("重连接connect to phone %08x:%d failed ",evt[1],evt[2]);
                                                                        
                                                        }
                                                }
                                        }else if(ret == -2)
						sd = -1;
										

                                }

#endif
                                SDL_Delay(35);

                                err = Vcodec_ReturnData(NULL,id,Y,U,V);

                                if (err != TM_OK) {
                                        WRT_DEBUG("tmVideoCap_ReturnData err,return code=%x",err);
                                }

                        }
#else

                        SDL_Delay(35);
#endif
                }
        }
        g_smalldoor_msg_queue.release();
        SDL_DestroySemaphore(g_smalldoor_msg_sem);
        g_smalldoor_msg_sem = NULL;
        return 0;
}



/*******************************************************************
函数名称：smalldoor_handler_task
函数功能：小门口监听任务
输入参数：
输出参数：
返回值：
********************************************************************/
int smalldoor_handler_task(void* pvoid){
	
#if 0	
        unsigned char* recvbuffer;
        UInt32 bufferlen = 0;
        recvbuffer = (unsigned char*)ZENMALLOC(100);
        bufferlen = 100;
        WRT_MESSAGE("485启动");
        tmGpio485Open();
        while(1){
                memset(recvbuffer,0,100);
                bufferlen = 100;
                tmGpio485Recv_NearDoor((char*)recvbuffer,&bufferlen);
                if(0xF5 == *recvbuffer || 0xFA == *recvbuffer){
                        ProcessSmallCmd(recvbuffer);
                }
        }
        tmGpio485Close();
        ZENFREE(recvbuffer);
        recvbuffer = NULL;
        t_delete(0);
#endif        
       return 0;
}

int smalldoor_handler_send_cmd_task(void* pvoid)
{

        int ret =0;
        unsigned long t_msg[4];
        unsigned long t_msg2[4];
        unsigned short scmd = 0;
        CWRTMsg* pMsg = NULL;
        int count = 3;
	g_sdsc_msg_sem = SDL_CreateSemaphore(0);
	g_sdsc2_msg_sem = SDL_CreateSemaphore(0);
	assert(g_sdsc_msg_sem);
	assert(g_sdsc2_msg_sem);
        WRT_MESSAGE("小门口机处理任务启动");
        while(1)
        {
                memset(t_msg,0,sizeof(t_msg));
                count = 3;
                SDL_SemWait(g_sdsc_msg_sem);
                pMsg = g_sdsc_msg_queue.get_message();
                if(pMsg){
                	ret = parse_msg(pMsg,t_msg);
                	if(ret < 0)
                		break;
                	if(ret == 0)
                		continue;
                }

                 
                SDL_Delay(350);
                do{
                	//清空队列g_sdsc_qid2，
						g_sdsc2_msg_queue.release();
                        send_485_msg(t_msg);
                        if(t_msg[2] == 0x0) //目标ID为0，则表是广播。
                                break;
                        if(t_msg[3]& 0x02) //表示该指令不需要等待应答。
                                break;


                        memset(t_msg2,0,sizeof(t_msg2));
                        ret = SDL_SemWaitTimeout(g_sdsc2_msg_sem,1500);
                        if(ret == 0)
                        {
                        	 pMsg = g_sdsc2_msg_queue.get_message();
                		if(pMsg){
                			ret = parse_msg(pMsg,t_msg);
                			if(ret < 0)
                				break;
                			if(ret == 0)
                				continue;
                		}
                                unsigned short cmd1 = t_msg[0] & 0x0000ffff;
                                if(cmd1 == (t_msg2[0]&0x0000ffff))
                                {
                                        
                                        break;
                                }
                        }

                        count --; 
                }while(count != 0);
                if(count == 0) //发送3次还没有收到消息，采用默认行为。
                {
                      
                        unsigned long event[4];
                        WRT_MESSAGE("没有收到应答采用默认行为 %x ",t_msg[0]&0x0000ffff);
                        if(ROOM_START_MONT_SMALLDOOR == (t_msg[0]&0x0000ffff)){
                                event[0] = ROOM_STOP_MONT_SMALLDOOR;
                                event[1] = event[2] = event[3] = 0;
                                m_pCmdHandler->Send_cmd(event);
                        }
                        if(ROOM_HANGUP_SMALLDOOR  == (t_msg[0]&0x0000ffff)){
                                event[0] = SMALLDOOR_ACK_HANGUP;
                                event[1] = 2;
                                event[2] = event[3] = 0;
                                m_pCmdHandler->Send_cmd(event);
                        }
                        if(ROOM_STOP_MONT_SMALLDOOR == (t_msg[0]&0x0000ffff)){
                                event[0] = SMALLDOOR_ACK_STOPMONT;
                                event[1] = event[2] = event[3] = 0;
                                m_pCmdHandler->Send_cmd(event);
                        }
                        if(ROOM_SMALLDOOR_LEAVE == (t_msg[0]&0x0000ffff)){
                                WRT_MESSAGE("ROOM_SMALLDOOR_LEAVE --ROOM_HANGUP");
                                event[0] = ROOM_HANGUP;
                                event[1] = 7;
                                event[2] = event[3] = 0;
                                m_pCmdHandler->Send_cmd(event);
                        }
                        if(ROOM_HOLDON_SMALLDOOR == (t_msg[0]&0x0000ffff))
                        {
                                event[0] = SMALLDOOR_ACK_HOLDON;
                                event[1] = 0;
                                event[2] = event[3] = 0;
                                m_pCmdHandler->Send_cmd(event);	
                        }

                }
        }
        g_sdsc_msg_queue.release();
        g_sdsc2_msg_queue.release();
        SDL_DestroySemaphore(g_sdsc_msg_sem);
        SDL_DestroySemaphore(g_sdsc2_msg_sem);
        g_sdsc_msg_sem = NULL;
        g_sdsc2_msg_sem = NULL;
        
        return 0;
}


void init_smalldoor(){

#if 0
        tmVideoCap_Init();
        tmVideoCap_Start();
#endif
	//SDL_CreateThread(smalldoor_handler_task,NULL);
	//SDL_CreateThread(smalldoor_video_task,NULL);
	//SDL_CreateThread(smalldoor_warningtest_task,NULL);
	SDL_CreateThread(smalldoor_photo_send_task,NULL);
	//SDL_CreateThread(smalldoor_handler_send_cmd_task,NULL);

}

static void SendAlarmToCenter2(int type,int mode)
{
	//int isstart = 0;
	unsigned long alarmtype = 0;
	alarmtype = mode;
        alarmtype = alarmtype << 16;
        alarmtype |= type;
        /*
	if(type == 106){//欠压
		isstart = 1;
		g_alarmtype = alarmtype;
	}else if(type == 107){//欠压恢复
		isstart = 0;
		g_alarmtype = 0;
	}else if(type == 111){ //电网停电
		isstart = 0;
	}else if(type == 112)//电网恢复
		isstart = 0;
        //if(m_pCmdHandler)
        //		m_pCmdHandler->lowtension_timer(isstart);
        */
        WRT_DEBUG("报警 %d --电源模块%d,alarmtype = %x\n",type,mode,alarmtype);
        SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,alarmtype); 
        // 2011-9-29 10:26:09,添加简易语音提示 “嘀两声".
        if(type == 106 || type == 107){
        	if(m_pCmdHandler)
        		m_pCmdHandler->playvoice2(WARNING_RING,0);	
        }
}

/****************************************************************************************
帧头（1）	长度（1）	目标ID（1）	源ID（1）	参数（n）	校验（1）
****************************************************************************************/
void ProcessSmallCmd(unsigned char* recvbuffer){
        unsigned char result=0;
        unsigned short cmd = 0;
        unsigned char  srcid = -1;

        int len = *(unsigned char*)(recvbuffer+1);
        int destid = *(unsigned char*)(recvbuffer+2); //表示小门口机包中的目标ID。
        srcid = *(unsigned char*)(recvbuffer+3);
        //如果destid和分机的ID相同才需要处理,如果destid == 0x0,则表示是广播的数据，都需要处理。
        WRT_MESSAGE("接收到485包destid = 0x%x,srcid = 0x%x ",destid,srcid);


        if((g_localid != destid) && ((destid != 0x0))){
                WRT_DEBUG("跳过485包目的地址不对g_localid = 0x%x,destid = 0x%x,srcid = 0x%x ",g_localid,destid,srcid);
                return;
        }


        unsigned char crc    = *(unsigned char*)(recvbuffer+(len-1));
        for(int i=0; i<(len-1);i++){
                result +=recvbuffer[i];
        }
        if(crc != result){
        	unsigned char result1 = (crc + result) & 0xff;
        	if(result1 != 0){
                	WRT_DEBUG("485包crc error crc = 0x%x,result=0x%x result1=0x%x",crc,result,result1);
                	return;
                }
        }
        int cmdlen = len-5;

        cmd = *(unsigned short*)(recvbuffer+4);
        cmd = ntohs(cmd);
        if(cmd ==  SIMULATE_ALARM){ //如果模拟分机报警。
                unsigned short tmpmsg[4];
                tmpmsg[0] = ROOM_ACK_SIMULATE_ALARM;
                tmpmsg[1] = 0x0;
                tmpmsg[2] = 0x0;
                tmpmsg[3] = 0x0 | 0x02;
                send_smalldoor_cmd(tmpmsg);
                int ret = 0;
                ret = SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,0);
                if(ret == 0){
                        if(pSystemInfo->mySysInfo.isenglish == 0x01)
                                add_log("Emergency alarm Succeeded");
                        else
                                add_log("紧急报警成功");
                }else{
                        if(pSystemInfo->mySysInfo.isenglish == 0x01)
                                add_log("Emergency alarm Failed");
                        else
                                add_log("紧急报警失败");
                }
                return;
        }
        if((cmd == SIMULATE_HOLDON) || (cmd == SIMULATE_HANGUP) ||(cmd == SIMULATE_UNLOCK ) ){ //如果是模拟分机发送来的数据，之处理摘机
                if(g_localid != 0x01) //非主分机，不处理。
                        return;

        }
        if(cmd == ROOM_ALARM_1){
        	SendAlarmToCenter2(106,srcid);
        	return;
        }else if(cmd == ROOM_ALARM_2){
        	SendAlarmToCenter2(107,srcid);
        	return;        	
        }
        else if(cmd == ROOM_ALARM_3){
        	SendAlarmToCenter2(111,srcid);
        	return;        	
        }
        else if(cmd == ROOM_ALARM_4){
        	SendAlarmToCenter2(112,srcid);
        	return;        	
        }

        //直接将命令转给命令处理任务处理
        if(m_pCmdHandler && !m_pCmdHandler->isupdate()){
                unsigned long event[4];
                event[0] = cmd;
                event[2] = cmdlen-2;
                if(event[2] >0){
                        unsigned char* tmpbuf = (unsigned char*)ZENMALLOC(cmdlen-2);
                        if(tmpbuf){
                                memcpy(tmpbuf,recvbuffer+6,event[2]);
                                event[1] = (unsigned long)tmpbuf;
                        }
                }else
                        event[1] = 0;
                event[3]=*(recvbuffer+3);//保存小门口ID
                WRT_MESSAGE("收到485包event[0] = 0x%x,0x%x,0x%x,event[3] = 0x%x  *recvbuffer=0x%x",cmd,event[1],event[2],event[3],*recvbuffer);
                if((*recvbuffer == 0xF5) && (event[0] == SMALLDOOR_ACK_OPENLOCK ||
                        event[0] == SMALLDOOR_ACK_HOLDON ||
                        event[0] == SMALLDOOR_ACK_HANGUP ||
                        event[0] == SMALLDOOR_IDLE ||
                        event[0] == SMALLDOOR_ACK_STOPMONT ||
                        event[0] == SMALLDOOR_ACK_LEAVE))
                        g_sdsc2_msg_queue.send_message(MODULE_MSG,event,sizeof(event),g_sdsc2_msg_sem);
                       // q_send(g_sdsc_qid2,event); //通知发送任务不需要重发数据了。

                m_pCmdHandler->Send_cmd2(event);

        }

}

void start_auto_cap_smalldoor_photo(){
        unsigned long evt[4];
        if(pSystemInfo->mySysInfo.isautocap == 1){
                evt[0] = SMALLDOOR_AUTO_CAPPHOTO;
                evt[1] = evt[2] = evt[3] =0;
                g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);
               
        }
}
void stop_auto_cap_smalldoor_photo(){
        unsigned long evt[4];
        if(pSystemInfo->mySysInfo.isautocap == 1){
                evt[0] = SMALLDOOR_CANCEL_AUTO_CAPPHOTO;
                evt[1] = evt[2] = evt[3] =0;
               g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);
        }
}

int start_smalldoor_video(){
        int ret = 0;

        if(g_bsmalldoor_video == true)
                stop_smalldoor_video();
        unsigned long evt[4];
        evt[0] = SMALLDOOR_START_VIDEO_EVENT;
        evt[1] = evt[2] = evt[3] =0;
        g_bsmalldoor_video = true;
        g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);

#ifdef HAVE_SUB_PHONE
        if(g_phone_connect_index != -1)
        {
                WRT_MESSAGE("start_or_stop_smvideo_phone 0x%x",m_pSocketManager->GetIPAddrByIndex(g_phone_connect_index));
                start_or_stop_smvideo_phone(m_pSocketManager->GetIPAddrByIndex(g_phone_connect_index),15005);
        }

#endif
        return ret;
}

int stop_smalldoor_video(){
        if(g_bsmalldoor_video == false){
                WRT_DEBUG("小门口机视频已经停止");
                return 0;
        }
        int ret =0;
        unsigned long evt[4];
#ifdef HAVE_SUB_PHONE
        if(g_phone_connect_index != -1)
        {
                start_or_stop_smvideo_phone(0,0);
        }
#endif
        evt[0] = SMALLDOOR_STOP_VIDEO_EVENT;
        evt[1] = evt[2] = evt[3] =0;
        g_bsmalldoor_video = false;
        //tm_wkafter(100);
       g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);
        SDL_Delay(200);

        return ret;
}


void send_485_msg(unsigned long msg[4])
{
        unsigned short u_msg[4];
        u_msg[0] = msg[0] & 0x0000ffff;
        u_msg[1] = msg[1] & 0x0000ffff;
        u_msg[2] = msg[2] & 0x0000ffff;
        u_msg[3] = msg[3] & 0x0000ffff;
        send_smalldoor_cmd_485(u_msg);

} 

//event[3] & MASK
//MASK 0:无数据，。
//MASK:0x1:有数据。
//mask:0x2:需要应答。
void send_smalldoor_cmd(unsigned short event[4]){

        unsigned long msg[4];
        msg[0] = event[0];
        msg[1] = event[1];
        msg[2] = event[2];
        msg[3] = event[3];
        g_sdsc_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_sdsc_msg_sem);
       

}

void send_smalldoor_cmd_485(unsigned short event[4]){
        unsigned char buf[9];
        unsigned short cmd = event[0];
        unsigned char crc=0;
        int i =0;
        int len = 0;
        buf[0] = 0xFA;
        buf[2] = event[2];//目标ID。
        buf[3] = (unsigned char)g_localid;    //源ID
        cmd = htons(cmd);
        memcpy(buf+4,&cmd,sizeof(unsigned short));
        if((event[3] & 0x01) == 0x01){//是否带有数据。
                buf[6] = event[1];
                len = 8;

        }else
                len =7;
        buf[1] = len;
        for(i=0;i<(len-1);i++)
                crc +=buf[i];
        buf[len-1] = crc;
      //  int ret = tmGpio485Send((char*)buf,len);
        WRT_MESSAGE("发送485包:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
}

void cap_smalldoor_photo(){
        unsigned long evt[4];
        evt[0] = SMALLDOOR_CAPPHOTO;
        evt[1] = evt[2] = evt[3] =0;
        g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);
}

void cap_smalldoor_photo_frame(){
        unsigned long evt[4];
        evt[0] = SMALLDOOR_CAPPHOTO;
        evt[1] = 2;
        evt[2] = evt[3] = 0;
       g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);
}



////////////////////////////////////////////////////////////////////////////////////
//
//防区报警的处理
////////////////////////////////////////////////////////////////////////////////////


static  unsigned char  ss_id=0;
static unsigned long g_dsp_mcu_qid = -1;
static unsigned long g_dsp_mcu_tid = -1;

void SendAlarmToCenter(unsigned char type)
{ 
        int alarmtype = 0;
        int start = 0;
        if(type == 0){
                alarmtype = 106;
                g_alarmtype = 106;
        }else{
                alarmtype = 107;
                g_alarmtype = 0;
        }
        if(alarmtype == 106){//欠压
        	g_isalarm = 1;
        }
        if(alarmtype == 107){
        	if(g_isalarm ==0){
        		return ;
        	}
        	g_isalarm ++;
        	if(g_isalarm > 2){
        		WRT_DEBUG("屏蔽掉107 报警\n");
        		g_isalarm = 2;
        		return;
        	}
        }
        if(alarmtype == 106)
        	start = 1;
        if(m_pCmdHandler)
        		m_pCmdHandler->lowtension_timer(start);
        WRT_DEBUG("低压报警 %d --%d\n",type,alarmtype);
        SendWarning((char*)pSystemInfo->LocalSetting.privateinfo.LocalID,alarmtype); 
        // 2011-9-29 10:26:09,添加简易语音提示 “嘀两声".
        if(m_pCmdHandler)
        	m_pCmdHandler->playvoice2(WARNING_RING,0);

        /*       char tmpbuf[44];  
        int alarmtype = 0;
        int cmdlen = 44;
        short cmd = 0;
        if(type == 0){
        alarmtype = 107;
        }else{
        alarmtype = 106;
        }
        memset(tmpbuf,0,44);
        strncpy(tmpbuf,"WRTI",4);
        memcpy(tmpbuf+4,&cmdlen,sizeof(int));
        cmd = htons(ROOM_SEND_ALARM);
        memcpy(tmpbuf+8,&cmd,2);
        memcpy(tmpbuf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
        memcpy(tmpbuf+40,&alarmtype,4);
        App_Enet_Send2(pSystemInfo->LocalSetting.publicinfo.CenterMajorIP,0,tmpbuf,cmdlen);    
        */        
}

//返回0 表示成功  -1 表示失败
static int SendSerialCmd(int fd,char* buffer,int len)
{
	int ret = 0;
	int wsize = 0;	
	if(fd == -1)
		return -1;
	
	while(1){
		ret = write(fd,buffer,len-wsize);
		if(ret > 0){
			wsize += ret;
			if(wsize == len)
				break;//写入完成
		}else{
		 	WRT_DEBUG("写串口数据失败Error:%d",errno);
		 	return -1;	
		}	
	}
	return 0;
}


int ReadSerialData(int fd,char *data,int datalen)
{
    int recvlen,ret;
    fd_set fs_read;
    struct timeval tv_timeout;
    if(fd == -1)
    	return -1;
    
    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);
    tv_timeout.tv_sec=5;
    tv_timeout.tv_usec=0;
    ret = 0;
    recvlen =0;
    ret=select(fd+1,&fs_read,NULL,NULL,&tv_timeout);
    WRT_DEBUG("ReadSerialData RET = %d error =%d ",ret,errno);
    if(ret)
    {
    	if(FD_ISSET(fd,&fs_read)){
    		
        	recvlen=read(fd,data,datalen);
        	WRT_DEBUG("read serail %d ",recvlen);
        	
        }
    }
    else
    {
        return -1;
    }
    return recvlen;
}

extern int IoPassArray[18];
extern int IoTestFlag;

static int mcu_dsp(void* pvoid){

        unsigned char* recvbuffer;
        unsigned long time1;
        int bufferlen = 0;
        int rsize = 0;
        int ret = 0;  
        int flags = 0;      
        struct  termios Opt;
		WRT_DEBUG("串口任务启动");
        recvbuffer = (unsigned char*)ZENMALLOC(100);
        if(recvbuffer == NULL){
                WRT_MESSAGE("分配窗口通信内存失败");
                return 0;
        }
		g_serial_fd = open("/dev/ttyS1",O_RDWR);
        if(g_serial_fd == -1){
        	WRT_DEBUG("打开设备/dev/ttyS1失败Error:%d",errno);
 			ZENFREE(recvbuffer);
        	recvbuffer = NULL;        	
        	return 0;
        }
        tcgetattr(g_serial_fd, &Opt);
		tcflush(g_serial_fd,TCIFLUSH);
		cfsetispeed(&Opt,B2400);/*设置为19200Bps*/
		cfsetospeed(&Opt,B2400);
		Opt.c_cflag |= CS8;                          //设置数据位
		Opt.c_cflag &= ~PARENB;
		Opt.c_oflag &= ~(OPOST);
		Opt.c_cflag &= ~CSTOPB;
		Opt.c_lflag &= ~(ICANON|ISIG|ECHO|IEXTEN);
		Opt.c_iflag &= ~(INPCK|BRKINT|ICRNL|ISTRIP|IXON);
		Opt.c_cc[VMIN] = 0;
		Opt.c_cc[VTIME] = 0;
		if (tcsetattr(g_serial_fd,TCSANOW,&Opt) != 0)//装载初始化参数
		{
	  	 	close(g_serial_fd);
	  	 	g_serial_fd = -1;
	 		ZENFREE(recvbuffer);
	        recvbuffer = NULL; 	 	
	  	 	return 0;
		}
		fcntl(g_serial_fd, F_SETFL,0);
        while(1){
        		SDL_Delay(500);
                memset(recvbuffer,0,100);
                bufferlen = 100;
                if( read(g_serial_fd,(char*)recvbuffer,bufferlen) > 0){
                	//WRT_DEBUG("*recvbuffer = %x %x %x %x %x %x %x %x %x %x",recvbuffer[0],recvbuffer[1],recvbuffer[2],recvbuffer[3],
                	// recvbuffer[4],recvbuffer[5],recvbuffer[6],recvbuffer[7],recvbuffer[8],recvbuffer[9]);
                	if(0xF5 == *recvbuffer){
                	        unsigned char crc = 0;
                	        unsigned char tmpcrc = 0;
                	        unsigned char len = *(unsigned char*)(recvbuffer+1);
                	        crc = *(recvbuffer+(len-1));
                	        for(int i =0; i<(len-1);i++)
                	                tmpcrc += recvbuffer[i];
                	        if(tmpcrc != crc){
                	                WRT_DEBUG("防区接收数据校验出错！");
                	                continue;
                	        }
                	        unsigned short cmd = *(unsigned short*)(recvbuffer+2);
                	        cmd = ntohs(cmd);
                	
                	        if(0x802 == cmd){
                	                unsigned long tmpevent[4];
                	                tmpevent[0] = STATUS_SAFE_WARNING;
                	                tmpevent[1] = *(unsigned char*)(recvbuffer+4);
                	                tmpevent[2]= tmpevent[3] = 0;

                	                if(IoTestFlag)//factory test
                	                {
	                	                IoPassArray[tmpevent[1]-1]  = 1;
	                	                unsigned long event2[4];
										event2[0] = STATUS_IOLEVEL_TEST;
										event2[1] = *(unsigned char*)(recvbuffer+4);
										event2[2] = event2[3] = 0;
										if(m_pCmdHandler)
                	                        m_pCmdHandler->CallDisplayCallBack(event2);
                	                }else
                	                {
                	                	if(m_pCmdHandler)
	        	                        	m_pCmdHandler->CallDisplayCallBack(tmpevent);
	        	                    }
	        	                    
                	                unsigned char sendbuf[6];
                	                unsigned short cmd = 0x802;
                	                sendbuf[0] = 0XFA;
                	                sendbuf[1] = 6;
                	                cmd = htons(cmd);
                	                memcpy(sendbuf+2,&cmd,sizeof(unsigned short));
                	                sendbuf[4]=tmpevent[1];
                	                sendbuf[5] = sendbuf[0]+sendbuf[1]+sendbuf[2]+sendbuf[3]+sendbuf[4];
                	                SendSerialCmd(g_serial_fd,(char*)sendbuf,6);
                	                continue;
                	        }
                	        if(0x801  == cmd){
                	                unsigned char sendbuf[6];
                	                unsigned short cmd = 0x801;
                	                sendbuf[0] = 0XFA;
                	                sendbuf[1] = 6;
                	                cmd = htons(cmd);
                	                memcpy(sendbuf+2,&cmd,sizeof(unsigned short));
                	                sendbuf[4]=0;
                	                sendbuf[5] = sendbuf[0]+sendbuf[1]+sendbuf[2]+sendbuf[3]+sendbuf[4];
                	                SendSerialCmd(g_serial_fd,(char*)sendbuf,6);
                	        }
                	        if(0x806 == cmd){
                	                unsigned char sendbuf[6];
                	                unsigned short cmd = 0x806;
                	                sendbuf[0] = 0XFA;
                	                sendbuf[1] = 6;
                	                cmd = htons(cmd);
                	                memcpy(sendbuf+2,&cmd,sizeof(unsigned short));
                	                sendbuf[4]= recvbuffer[4];
                	                sendbuf[5] = sendbuf[0]+sendbuf[1]+sendbuf[2]+sendbuf[3]+sendbuf[4];
                	                SendSerialCmd(g_serial_fd,(char*)sendbuf,6);	
                	                  
                	                SendAlarmToCenter(sendbuf[4]);              
                	        }
                	        //ProcessSmallCmd(recvbuffer);
                	}
        	}
        }

      	close(g_serial_fd);
      	g_serial_fd = 0;
        ZENFREE(recvbuffer);
        recvbuffer = NULL;
        return 0;
}

void init_16safe(){
	SDL_CreateThread(mcu_dsp,NULL);
}

void send_dsp_to_mcu(unsigned short param){
	unsigned char sendbuf[5];
	unsigned short cmd = param;
	sendbuf[0] = 0XFA;
	sendbuf[1] = 5;
	cmd = htons(cmd);
	memcpy(sendbuf+2,&cmd,sizeof(unsigned short));
	sendbuf[4] = sendbuf[0]+sendbuf[1]+sendbuf[2]+sendbuf[3];
	//  int ret = tmUartSupportSend(1,sendbuf,5);
}

void mcu_reset_sys(unsigned char param){
	
	unsigned char sendbuf[6];
	unsigned short cmd = 0x801;
	sendbuf[0] = 0XFA;
	sendbuf[1] = 6;
	cmd = htons(cmd);
	memcpy(sendbuf+2,&cmd,sizeof(unsigned short));
	sendbuf[4] = param;
	sendbuf[5] = sendbuf[0]+sendbuf[1]+sendbuf[2]+sendbuf[3]+sendbuf[4];

	//  tmUartSupportSend(1,sendbuf,6);
}

void start_or_stop_smvideo_phone(unsigned long ip,short port)
{
	unsigned long evt[4];
	if(ip != 0 && port != 0){
		evt[0] = SMALLDOOR_VIDEO_TO_PHONE_START;
		evt[1] = ip;
		evt[2] = port;
		evt[3] = 0;
	}else{
		evt[0] = SMALLDOOR_VIDEO_TO_PHONE_STOP;
		evt[1] = 0;
		evt[2] = 0;
		evt[3] = 0;
	}
	g_smalldoor_msg_queue.send_message(MODULE_MSG,(void*)evt,sizeof(evt),g_smalldoor_msg_sem);
}

