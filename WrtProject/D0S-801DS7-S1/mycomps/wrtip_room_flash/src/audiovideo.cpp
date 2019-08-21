
#include <sys/socket.h>
#include <net/if.h>
#include <semaphore.h>       //sem_t
#include <dirent.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/vfs.h>
#include <netinet/if_ether.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
//#include <linux/videodev.h>
#include <errno.h>
#include <linux/fb.h> 
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <semaphore.h>       //sem_t
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>

#include "public.h"
//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//
#include "wrt_audiostream.h"
#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "button_sound.h"
#include "leave_zh.h"
#include "leave_en.h"
#include "audiovideo.h"
#include "cmd_handler.h"
#include "RoomGpioUtil.h"
#include "info_list_handler.h"
#include "smalldoor_handler.h"
#include "wrt_log.h"
#include "g711.h"
#include "wrt_audio.h"
#include "jpeg.h"
#include <madplay.h>

#include "media_negotiate.h"

#define USE_MYJITTER 1
#define USE_AUDIO_MEDIASTREAM 0 //1 使用新的媒体架构来处理音频
#define USE_OLD_AUDIO_INTERFACE 1

#ifdef USE_MYJITTER
#include "myjitter.h"
#endif
#define BUTTON_VOICE_PATH "/home/wrt/default/button.mp3"
#define BUTTON_VOICE_PATH1 "/home/wrt/default/newbutton.mp3"

typedef unsigned long UInt32;
typedef unsigned int UInt;

#include "videocommon.h"

#include "tmAppport.h"


#define closesocket close
#define ioctlsocket ioctl


static SDL_mutex* g_button_lock = NULL;

#define CAPTURE_PHOTO   1
#define CANCEL_CAPPHOTO 2
#define CANCEL_CAPFRAME 3

#define AUDIOSTART 1
#define AUDIOSTOP  -1

#define  STARTCAPAUDIO  1
#define  STOPCAPAUDIO   0

#define  LEAVE_START  1 //留言开始
#define  LEAVE_CANCEL 2 //留言取消
#define  LEAVE_STOP   3 //留言结束


#ifdef __cplusplus
extern "C"{
#endif

#define ENGLISH 0x01
int iptv_image_write_jpeg_420(unsigned char *y,unsigned char *u,
    unsigned char *v,int width,int height,
    unsigned char *outputbuf,int quality,int destwidth);
int GetCurrentLanguage();
int gb2uni( const unsigned char *gbstr, unsigned short *unibuf, int buflen );
int ucs2ToUtf8(const unsigned short *ucs, unsigned char *cbuf, int cbuf_len);        

unsigned char* savefbtofile(int* buflen,int quality);

#ifdef __cplusplus
}
#endif

extern char doorname[256];
int g_copy_yuv_flag = 0;
unsigned char g_copy_yuv_buf[152064];
//#define  PLAYSERVERPRI  63
//CPlayer *pplayer=NULL;
int VideoFrameNumber; //why? libplayer.a
int InitDns = 0;
//int video_state = 0;
static CWRTMsgQueue g_ai_msg_queue;
static CWRTMsgQueue g_ao_msg_queue;
static CWRTMsgQueue g_leave_msg_queue;
static CWRTMsgQueue g_capture_msg_queue;
static CWRTMsgQueue g_mp3_msg_queue;
static CWRTMsgQueue g_phone_video_msg_queue;
static CWRTMsgQueue g_phone_audio_msg_queue;
static SDL_sem*  g_ai_msg_sem = NULL;
static SDL_sem*  g_ao_msg_sem = NULL;
static SDL_sem*  g_leave_msg_sem = NULL;
static SDL_sem*  g_capture_msg_sem = NULL;
static SDL_sem*  g_mp3_msg_sem = NULL;
static SDL_sem*  g_phone_video_msg_sem = NULL;
static SDL_sem*  g_phone_audio_msg_sem = NULL;

static SDL_mutex* g_play_mutex  = NULL;
static SDL_mutex* g_capture_mutex = NULL;


//unsigned long aistart_qid, aostart_qid,  mp3start_qid,leavestart_qid;
//unsigned long tid_mp3player,tid_leaveplayer;
//unsigned long tid_capture,qid_capture;
//unsigned long tid_phonevideo,qid_phonevideo;
//static unsigned long g_capture_video_lock = -1;
//static unsigned long g_play_lock = -1;
//static    int ai_instance=1;
//static WAV_HDR g_hdr;
extern CCmdHandler*    m_pCmdHandler;
extern T_SYSTEMINFO* pSystemInfo;
extern int g_phone_connect_index; //2010.10.13

extern int g_holdon_flag;
extern int g_have_cap_photo;
//extern UInt32  TicksToMillisec(UInt32 timeInTicks);
/*
extern   void  SendStopWarningTimer();
extern    void SendStartWarningTimer();
*/
extern int gInputFlag;
//
static unsigned char* picturebuf = NULL;

static int  g_photo_quality = 90;

//int InitAudioOutOK;
static bool m_startButtonVocie=false;//是否开启按键声音
static bool m_close_ring = false;//是否关闭铃声

static bool m_istalk1 = false;
static bool m_istalk2 = false;
static bool m_iscapleave = false;

static unsigned long gRecvAudioHandle;
static unsigned long gAITHandle;
static int AudioOutInstatce;
static int AudioDeviceInst;

static bool IsLeaveWord = false; //是否留言
static bool IsSmalldoorLeave = false;
static int   g_playstatus = 0;

static bool  gPlayIsStart = false;

static int g_ptime = 40;
static int g_frame_size  = g_ptime*8*2;
static int g_rtp_packetlen = g_frame_size/2 +12;
static int g_arate = 8000;


static int g_fd_mplayer_fifo = -1;

	

typedef enum AudioOutMode {
        AudioOutModeMp3        = 0x1,
        AudioOutModeTalk       = 0x2,
        AudioOutModeLeave      = 0x3,
        AudioOutModeStop       = 0x4

} AudioOutMode_t;

AudioOutMode_t aomode;


#ifndef USE_MYJITTER

#pragma pack(1)
typedef struct wrt_rtp_header
{

        uint16_t cc:4;
        uint16_t extbit:1;
        uint16_t padbit:1;
        uint16_t version:2;
        uint16_t paytype:7;
        uint16_t markbit:1;
        uint16_t seq_number;
        uint32_t timestamp;
        uint32_t ssrc;
} wrt_rtp_header_t;

#pragma pack()

#endif

static int AudioRecv(void *pParam);
static int AIThread(void *pParam);
static int mp3playertask(void* pvoid);
static int JVideoProc(void *data);
static int leaveplayertask(void* pvoid);

extern "C" void playaudio(unsigned int audioinstatce,unsigned char* data,int len);


static void enter_video_lock()
{
	if(g_capture_mutex)
	        SDL_LockMutex(g_capture_mutex);
		
}

static void exit_video_lock()
{
	if(g_capture_mutex  )
		SDL_UnlockMutex(g_capture_mutex);
}

uint32_t tm_getticks(uint32_t* hi,uint32_t* li)
{
        uint32_t ticks;
        /*
        struct timeval now;
        gettimeofday(&now, NULL);
        ticks=(now.tv_sec)*1000+(now.tv_usec)/1000;
        */
        ticks = SDL_GetTicks();
        *li = ticks;
        return ticks;
}
uint32_t tm_get(uint32_t* date,uint32_t *time1,uint32_t* ticks)
{
        struct tm* curr_tm;
        time_t t;	
        uint32_t cur_date = 0;
        uint32_t cur_time = 0;
        time(&t);
        curr_tm = localtime(&t);
        cur_date |= (((curr_tm->tm_year + 1900) << 16)& 0xffff0000);
        cur_date |= (((curr_tm->tm_mon+1) << 8) & 0x0000ff00 );
        cur_date |= (curr_tm->tm_mday & 0x000000ff);


        cur_time |= (curr_tm->tm_hour << 16)& 0xffff0000;
        cur_time |= (curr_tm->tm_min << 8) & 0x0000ff00;
        cur_time |= (curr_tm->tm_sec & 0x000000ff);


        if(date != (uint32_t*)NULL)
                *date = cur_date;
        if(time1 != (uint32_t*)NULL)
                *time1 = cur_time;
        if(ticks != (uint32_t*)NULL)
                *ticks = 0;
        return 0;
}
uint32_t tm_set(uint32_t date,uint32_t time1,uint32_t ticks)
{
        struct tm curr_tm;
        time_t t;
        int rt;
	
        curr_tm.tm_year = (date >> 16)& 0x0000ffff ;
        curr_tm.tm_year -= 1900;
        curr_tm.tm_mon = (date >> 8) &  0x000000ff ;
        curr_tm.tm_mon -= 1;
        curr_tm.tm_mday = date & 0x000000ff;
        curr_tm.tm_hour = (time1 >> 16) & 0x0000ffff;
        curr_tm.tm_min = (time1>> 8) & 0x000000ff;
        curr_tm.tm_sec = (time1) & 0x000000ff;
        curr_tm.tm_isdst = -1;

        t=mktime(&curr_tm);
        rt = stime(&t);

        return 0;
}

#ifdef HAVE_SUB_PHONE
//static unsigned long g_phoneAudiohandle;
//static unsigned long g_phone_qid = 0xffffffff;

int  phone_audio_task(void* pvoid);

static int connect_phone_client(unsigned long ipaddr,int _port){

        struct sockaddr_in addr;
        int tmp_sock = -1;
        fd_set writefd;
        struct timeval timeout;    
        int max_fd = -1; 
        int flag = 1;
        int rc =0;
        tmp_sock  = socket(AF_INET,SOCK_STREAM, 0);
        if(tmp_sock < 0){
                WRT_DEBUG("创建socket failed ");              
                return -1;
        }          
        memset(&addr,0,sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;      
        addr.sin_addr.s_addr = ipaddr;
        addr.sin_port   = _port;

        ioctlsocket(tmp_sock,FIONBIO,(char*)&flag);
        if(connect(tmp_sock,(struct sockaddr*)&addr,sizeof(struct sockaddr_in)) < 0){
                if(errno  != EINPROGRESS){           
                        closesocket(tmp_sock);
                        tmp_sock = -1;  
                        return tmp_sock;      
                }
        }  


        FD_ZERO(&writefd);
        if(max_fd < tmp_sock)
                max_fd = tmp_sock;
        FD_SET(tmp_sock,&writefd);   
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        rc = select(max_fd+1,NULL,&writefd,NULL,&timeout);
        if(rc > 0){      
                if(FD_ISSET(tmp_sock,&writefd)){
                        WRT_MESSAGE("connect ok  ipaddr = 0x%x",ipaddr);
                        return tmp_sock;
                }  
        }else{
                WRT_DEBUG("connect error 0x%x",ipaddr);
        }

        closesocket(tmp_sock);
        tmp_sock = -1;
        return tmp_sock;       
}



int send_phone_video(unsigned long ip,short port,unsigned char* y,unsigned char* u,unsigned char* v,unsigned char* outbuf)
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
        int exitflag = 0;
        int isloop = 0;
        int count = 3;
        int timeoutcount = 0;
        fd_set writefd;
        struct timeval timeout;  
        CWRTMsg* pMsg = NULL;

	
        sd = connect_phone_client(ip,port);
  
        WRT_DEBUG("-----------------");
        if(sd != -1){
                FD_ZERO(&writefd);
                FD_SET(sd, &writefd);	
        }
        timeout.tv_sec = 1;
        timeout.tv_usec = 0; //2009.4.8. modify 300MS              
        while(1)
        {

loop:
                memset(msg ,0,sizeof(msg));
                SDL_SemTryWait(g_phone_video_msg_sem);
                pMsg  = g_phone_video_msg_queue.get_message();
                if(pMsg){
                        if(pMsg->get_value() == MODULE_EXIT){
                                pMsg->free_message();
                                ZENFREE(pMsg);
                                pMsg = NULL;
                                return -1;
                        }else if(pMsg->get_value() == MODULE_MSG){
                                uint32_t msglen = 0;
                                const void* pt_msg = NULL;
                                // q_receive(qid_phonevideo,Q_NOWAIT,0,msg);
                                pt_msg = pMsg->get_message(msglen);
                                if(pt_msg && msglen > 0){
                                        memcpy(msg,pt_msg,sizeof(msg));                               
                                        if(msg[0] == CANCEL_CAPPHOTO && msg[1] == 3){
                                                WRT_MESSAGE("停止手机视频,行号%d",__LINE__);
                                                pMsg->free_message();
                                                ZENFREE(pMsg);
                                                pMsg = NULL;
                                                return 0;
                                        }
                                }else{
                                        pMsg->free_message();
                                        ZENFREE(pMsg);
                                        pMsg = NULL;

                                }
                        }else{
                                pMsg->free_message();
                                ZENFREE(pMsg);
                                pMsg = NULL;
                        }

                }
                if(sd == -1){
                	WRT_DEBUG("无法连接上手机ip = %x port =%d,1秒后尝试重新连接",ip,(unsigned short)port);
                	SDL_Delay(1000);
                        sd = connect_phone_client(ip,port);
                        if(sd != -1){
                        	timeoutcount = 0;
                                FD_ZERO(&writefd);
                                FD_SET(sd, &writefd);
                        }else
	                	continue;
                }                
                //if(pplayer == NULL)
                //        continue;
                enter_video_lock();
                //pplayer->CapturePic(y,u,v);
                exit_video_lock();

                unsigned long outbuflen = iptv_image_write_jpeg_420(y,u,v,352,288,outbuf+40,g_photo_quality,352);
                if(outbuflen == 0)
                        continue;

                int sendlen = 0;
                int ttlen = 0;

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
                        	timeoutcount++;
                        	if(timeoutcount > 3){
                        		timeoutcount = 0;
                        		closesocket(sd);
                        		sd = -1;
                        		WRT_DEBUG("超时关闭socket");
                        	}
                                WRT_DEBUG("发送手机视频select超时0x%x sd = %d",errno,sd);
                                break;
                        }else if(result < 0){
                                WRT_DEBUG("发送手机视频select错误0x%x sd = %d",errno,sd);
                                exitflag = 1;
                                break;
                        }
                        if(FD_ISSET(sd, &writefd))
                        {

                                result = send(sd,(char*)outbuf+sendlen,ttlen-sendlen,0);
                                if(result < 0){
                                        WRT_DEBUG("发送手机视频send错误0x%x",errno);
                                        if(errno == 0x5020)
                                        {
                                                closesocket(sd);
                                                sd = -1;
                                                goto loop;
                                        }else{
                                        	exitflag = 1;
                                        	break;
                                        }
                                }else{
                                	sendlen  +=  result;
                                	if(sendlen == ttlen)
                                        	break;
                                }

                        }
                }

                if(exitflag == 1)
                        break;
                SDL_Delay(80);
        }
end:
        WRT_MESSAGE("关闭发送手机视频网络%d",sd);
        closesocket(sd);
        sd = -1;
}

static int send_phone_video_task(void* pvoid)
{
        const int width = 352;
        const int height = 288;	
        CWRTMsg* pMsg = NULL; 
        unsigned char* picturebuf1 = NULL;
        unsigned char * y = (unsigned char*)ZENMALLOC(width*height);
        unsigned char * u = (unsigned char*)ZENMALLOC(width*height/4);
        unsigned char * v = (unsigned char*)ZENMALLOC(width*height/4);
        picturebuf1 = (unsigned char*)ZENMALLOC(100*1024);
        unsigned long RMsgBuff[4];
        WRT_DEBUG("send_phone_video_task begin\n");
        g_phone_video_msg_sem = SDL_CreateSemaphore(0);
        assert(g_phone_video_msg_sem != NULL);
        while(1){
                memset(RMsgBuff,0,sizeof(RMsgBuff));
                WRT_DEBUG("send_phone_video_task WAIT\n");
                SDL_SemWait(g_phone_video_msg_sem);
                pMsg = g_phone_video_msg_queue.get_message();
                if(pMsg){
                        if(pMsg->get_value() == MODULE_EXIT){
                                pMsg->free_message();
                                ZENFREE(pMsg);
                                pMsg = NULL;
                                break;
                        }else if(pMsg->get_value() == MODULE_MSG){
                                uint32_t msglen = 0;
                                const void* pt_msg= NULL;
                                pt_msg = pMsg->get_message(msglen);
                                if(pt_msg && msglen > 0){
                                        memcpy(RMsgBuff,pt_msg,sizeof(RMsgBuff));
                                        WRT_DEBUG("send_phone_video_task wait ok%d %d",RMsgBuff[0],RMsgBuff[1]);
                                        if(RMsgBuff[0] == CAPTURE_PHOTO ){
#ifdef HAVE_SUB_PHONE                        
                                                if(RMsgBuff[1] == 3)
                                                {
                                                        unsigned long sendip = RMsgBuff[2];
                                                        short sendport= htons(RMsgBuff[3]);
                                                        if(send_phone_video(sendip,sendport,y,u,v,picturebuf1) < 0)
                                                                break;

                                                }
#endif                	
                                        }
                                }
                        }else{
                                pMsg->free_message();
                                ZENFREE(pMsg);
                        }
                }
        }
        ZENFREE(y);
        ZENFREE(u);
        ZENFREE(v);
        ZENFREE(picturebuf1);
        g_phone_video_msg_queue.release();
        SDL_DestroySemaphore(g_phone_video_msg_sem);
        g_phone_video_msg_sem = NULL;

        return 0;
}


#endif


//

/*
void SetWaveHeader(){
memset(&g_hdr,0,sizeof(WAV_HDR));
memcpy(&g_hdr,"RIFF",4);
memcpy(g_hdr.wID,"WAVE",4);
memcpy(g_hdr.fId,"fmt ",4);
g_hdr.pcm_header_len = 0x12;
g_hdr.wFormatTag = 0x0007;
g_hdr.nChannels = 2;
g_hdr.nSamplesPerSec = 16000;
g_hdr.nAvgBytesPerSec = 64000;
g_hdr.nBlockAlign = 4;
g_hdr.nBitsPerSample = 16;
memcpy(g_hdr.dId,"data",4);
}
*/
void SetPhotoQuality(int quality){
        if(quality < 0 || quality > 100)
                return;
        g_photo_quality = quality;
        pSystemInfo->mySysInfo.photoqualit = g_photo_quality;
}

int GetPhotoQuailty(){
        return g_photo_quality;
}

void CapturePhoto(){
        unsigned long msg[4];
        memset(msg, 0, 16);
        msg[0] = CAPTURE_PHOTO;
        g_capture_msg_queue.send_message(MODULE_MSG,msg,sizeof(msg),g_capture_msg_sem);
}


void CancelCapturePhoto1(){
        unsigned long msg[4];
        memset(msg, 0, 16);
        msg[0] = CANCEL_CAPPHOTO;
        msg[1]=1;
        g_capture_msg_queue.send_message(MODULE_MSG,msg,sizeof(msg),g_capture_msg_sem);
}


void CapturePhoto1(){
        if(pSystemInfo->mySysInfo.isautocap == 0)
                return;
        unsigned long msg[4];
        memset(msg, 0, 16);
        msg[0] = CAPTURE_PHOTO;
        msg[1]=1;
         g_capture_msg_queue.send_message(MODULE_MSG,msg,sizeof(msg),g_capture_msg_sem);
}



 int parse_msg(CWRTMsg* pMsg,unsigned long* RMsg){
        int ret = 0;
        if(pMsg == NULL)
                return 0;
        if(pMsg->get_value() == MODULE_EXIT){
                pMsg->free_message();
                ZENFREE(pMsg);
                pMsg = NULL;
                ret = -1;
               
        }else if(pMsg->get_value() == MODULE_MSG){
                uint32_t msglen = 0;
                const void* pt_msg = NULL;
                pt_msg = pMsg->get_message(msglen);
                if(pt_msg && msglen > 0){
                        memcpy(RMsg,pt_msg,sizeof(unsigned long)*4);
                        ret = 1;
                }
                pMsg->free_message();
                ZENFREE(pMsg);
                pMsg = NULL;
        }else{
                pMsg->free_message();
                ZENFREE(pMsg);
                pMsg = NULL;
        }
        return ret;
}


static int capture_photo_task(void* pvoid){
        unsigned long RMsgBuff[4];
        CWRTMsg* pMsg = NULL;
        g_capture_msg_sem = SDL_CreateSemaphore(0);
        assert(g_capture_msg_sem != NULL);
        int outbuflen = 0;
        int ret = 0;
        unsigned char* outbuf = NULL;      
        while(1){
        		g_copy_yuv_flag = 0;
                memset(RMsgBuff,0,sizeof(RMsgBuff));
                outbuflen = 0;
                outbuf = NULL;
                SDL_SemWait(g_capture_msg_sem);
                pMsg = g_capture_msg_queue.get_message();
                if(pMsg){
                        ret = parse_msg(pMsg,RMsgBuff);
                        if(ret == 0)
                                continue;
                        if(ret < 0)
                                break;

                }
                //q_receive(qid_capture,Q_WAIT,0,RMsgBuff);
                if(RMsgBuff[0] == CAPTURE_PHOTO ){
                        if(RMsgBuff[1] == 1){
                                SDL_Delay(4000); //等待5秒后自动抓拍一张。
                                if(SDL_SemTryWait(g_capture_msg_sem) == 0){
                                        pMsg = g_capture_msg_queue.get_message();
                                        if(pMsg){
                                                ret = parse_msg(pMsg,RMsgBuff);
                                                if(ret > 0){
                                                        if(RMsgBuff[0] == CANCEL_CAPPHOTO)
                                                                continue;
                                                }else if(ret < 0){
                                                        break;
                                                }
                                        }
                                }
                        }
                    g_copy_yuv_flag = 2;//让解码拷贝yuvbuf
                    int waitcnt = 0;
					while(g_copy_yuv_flag != 3)
					{
						usleep(10*1000);
						waitcnt++;
						if(waitcnt >= 200)
							continue;
                    }
                    outbuf = savefbtofile(&outbuflen,g_photo_quality);
					if(outbuf && (outbuflen > 0)){
					        unsigned char* sendbuf = (unsigned char*)ZENMALLOC(outbuflen);
					        if(sendbuf){
					                int itmp = 0;
					                if(m_pCmdHandler){
					                        if( m_pCmdHandler->GetIsDoorCall() == 0)
					                                itmp = 0;
					                        else
					                                itmp = 1;
					                }
					                memcpy(sendbuf,outbuf,outbuflen);
					                send_voice_photo_to_center(itmp,ROOM_SEND_PHOTO_VOICE,sendbuf,outbuflen);
					        }
					}
					if(outbuf != NULL)
					ZENFREE(outbuf);
					outbuf = NULL;
                }else if(RMsgBuff[0] == CANCEL_CAPPHOTO){
                        continue;
                }

                SDL_Delay(1000);
        }
        g_capture_msg_queue.release();
        SDL_DestroySemaphore(g_capture_msg_sem);
        g_capture_msg_sem = NULL;
}

void JStartVideoProc(void)
{


	g_capture_mutex = SDL_CreateMutex();
	assert(g_capture_mutex != NULL);

        SDL_CreateThread(capture_photo_task,NULL);
        //SDL_CreateThread(send_phone_video_task,NULL);

        WRT_DEBUG("JStartVideoProc ok");      

}


extern void DirIsExist(char *dirname);
/****************************************************
抓拍图片:
此处ZENMALLOC	分配的内存，由外部释放。
这里不用释放。by ljw 2012-11-14 16:00:45
*****************************************************/

int g_cap_flag = 0;

unsigned char* savefbtofile(int* buflen,int quality)   
{  

	uint32_t day,Month,years, dates,times;
	uint32_t min,second,Hour;
	long _size,fsize;
	struct statfs vbuf;
	statfs("/",&vbuf);
	_size = (vbuf.f_bsize * vbuf.f_bfree);
	fsize = (float)_size/(1024*1024);
	if(fsize < 15)
	{
		*buflen = 0;
		return NULL;
	}
	char timename[50];
	tm_get(&dates,&times,NULL);
	day= dates&0xff;
	Month = (dates >> 8) &0xff;
	years = (dates>>16) & 0xffff;
	Hour = (times >> 16)& 0XFFFF;
	second = (times & 0xff);
	min = (times >> 8) &0xff;
   	DirIsExist("/home/wrt/pic/picture");

	int i = 0;
	unsigned char* outputbuf = (unsigned char* )ZENMALLOC(352*288*3);
	if(outputbuf == NULL){
		WRT_DEBUG("ZENMALLOC FAILED");

		return NULL;
	}
	char inputbuf2[152064];
	memcpy(inputbuf2,g_copy_yuv_buf,101376);

	int j = 101376;

	for(i=101376;i<152064;i=i+2)
	{
		inputbuf2[j] = g_copy_yuv_buf[i];
		j++;
	}
	j = 126720;
	for(i=101377;i<152064;i=i+2)
	{
		inputbuf2[j] = g_copy_yuv_buf[i];
		j++;
	}
	int outbuflen = 0;

	outbuflen = iptv_image_write_jpeg_420((unsigned char *)inputbuf2,(unsigned char *)inputbuf2+101376,(unsigned char *)inputbuf2+126720,352,288,(unsigned char *)outputbuf,80,352);
	
	if(outbuflen <= 0)
	{
		ZENFREE(outputbuf);
		return NULL;
	}
	int itmp = 0;
	if(m_pCmdHandler)
		itmp = m_pCmdHandler->GetIsDoorCall();

	if(outputbuf && (outbuflen > 0)){
		unsigned char* sendbuf = (unsigned char*)ZENMALLOC(outbuflen);
		if(sendbuf){
				memcpy(sendbuf,outputbuf,outbuflen);
				send_voice_photo_to_center(itmp,ROOM_SEND_PHOTO_VOICE,sendbuf,outbuflen); //小门口机
		}
	}

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
	sprintf((char*)pTemp->name,"pic_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".jpg");
	pTemp->read  = 0;
	pTemp->read |= ((itmp <<16) & 0XFFFF0000);
	add_photo(pTemp,outputbuf,outbuflen);
 	system("sync");
 	if(buflen)
 		*buflen = outbuflen;
 	return outputbuf;
}

void SetSendIPAndPort(unsigned long ip,short port){
        /*
        sendip = ip;
        sendport = port;
        printf("send ip= %x \n",sendip);
        */
}
	/*
	memset(&pic,0,sizeof(pic));
	pic.width=1024;
	pic.height = 600;
	pic.data = (unsigned char *)tmpbuf;
	outbuflen = iptv_image_write_jpeg(&pic,outputbuf,quality,16);
  */

static int video_state = 1;//默认播放
static int g_decode_vol_sys = 0;
static pid_t mplayer_pid ;
static  SDL_Thread *pVideoThread = NULL;


void PauseAndPlayVideo(int status){

	if(status == 0 && video_state == 1){
		int tmpstate = 0;
		WRT_MESSAGE("stop video play");

		video_state = 0;

		Stopplayvideo();

		WRT_MESSAGE("exit video thread, %d \n",tmpstate);


	}else if(status == 1 && video_state == 0){
		video_state = 1;
		WRT_MESSAGE("start video play");
		Startplayvideo(NULL);
	}
}

int  GetPlayVideoStatus()
{
        return video_state;
}


void PausePlayVideo(int status){
	
	PauseAndPlayVideo(status);
/*
        if(m_pCmdHandler){
                unsigned long event[4];
                event[0] = STATUS_PAUSE_VIDEO;
                event[1] = status;
                event[2]=event[3] = 0;
                m_pCmdHandler->CallDisplayCallBack(event);
        }
*/
}

SDL_Thread * mp3thread;
static int mp3_status = 0;

void InitAudioDevice(void)
{
    //开启音频接收任务

    int  retVal;

    unsigned long msg[4];
    IsLeaveWord = false;
    IsSmalldoorLeave = false;
    aomode = AudioOutModeStop;

    g_play_mutex = SDL_CreateMutex();
    assert(g_play_mutex != NULL);

#ifdef HAVE_SUB_PHONE
	SDL_CreateThread(phone_audio_task,NULL);
#endif
	//SDL_CreateThread(AudioRecv,NULL);
#if USE_OLD_AUDIO_INTERFACE
	SDL_CreateThread(AudioRecv,NULL);
	SDL_CreateThread(AIThread,NULL);
#endif
	mp3thread = SDL_CreateThread(mp3playertask,NULL);
	//SDL_CreateThread(leaveplayertask,NULL);
 
    SDL_Delay(200);
        WRT_DEBUG("InitAudioDevice ok");
	
        //  RoomGpioUtilSetDoorAudioOnA();
        //   SetWaveHeader();
      //  wrt_test_echo_delay();

}

void SetVolume(int volume)
{
	SetAlsaVolume(TYPE_PLAY,volume);
}
void SetCapVolume(int volume)
{
	SetAlsaVolume(TYPE_CAPTURE,volume);
}

void SetBass(int bass){
}
void SetTreble(int treble){
}
static void sigchld_handler(int signo)
{
	pid_t pid;
	int state = 0;
	pid = waitpid(-1,&state,WNOHANG);
	WRT_MESSAGE("exit child process   %d \n",pid);
}
static int JVideoProc(void *data)
{
        //struct SystemInfo systeminfo;
	system("killall  -9 mplayer");
	//char *newargv[] = { NULL, " -vc", "on28190", "-geometry","100:100","-x","352","-y","288","cnxt://192.168.1.32:video=20000"};
	//char *newenviron[] = { NULL };
        //execve("/mnt/sda/mplayer", newargv,newenviron);
        //启动一个进程，并等待进程结束
        signal(SIGCHLD,sigchld_handler);
        system("/mnt/sda/mplayer -vc on28190 -geometry 100:100 -x 352 -y 288 cnxt://192.168.1.32:video=20000"); //
        WRT_MESSAGE("mplayer exit = \n");
        return 0;
		 	        
#if 0        
        char name[100];
        char temp[32];
        int maxloop;
        int i;
        void *p;
        int playnumber=0;
        char *user="dig";

        CPlayer *player = new CPlayer;
        pplayer = player;

        //GetSystemInfo(&systeminfo);

        strcpy(name,"udp://192.168.1.68:20000");
        player->InitPlayer("192.168.1.200",0);
        player->SetPicReverse(0);
        player->InitAudioVolume(0x34b);
        maxloop = 1;

        InitDns = 1;//	printf("InitDns=1\n");

        // PausePlayVideo(0);

        player->StartPlay(name, maxloop);

        //printf("after startplay \n");

        delete player;
        player = NULL;
        pplayer = NULL;
#endif        
	return 1;

}

static char g_audiobuffer[2048];
//decode g711 ulaw to raw data
void play_audio(unsigned int audioinstatce,unsigned char* data,int len)
{
    int i ;
    //char* buf = NULL;
    int count = 0;
    int ret =0;
    int totallen =0;
    //buf = (char*)ZENMALLOC(len*2);
    //memset(buf,0,len*2);
    memset(g_audiobuffer, 0, 2048);
    short *buf16 = (short *)g_audiobuffer;
    for (i=0; i < len; i++)
    {
        buf16[i] = ulaw2linear(data[i]);
    }
    totallen = len*2;
    PlayAudio((unsigned char*)g_audiobuffer,len*2);
    gPlayIsStart  = true;
    //ZENFREE(buf);
    //buf = NULL;
}

static int AudioRecv(void *pParam)
{
    int result;
    int sd;
    struct sockaddr_in from,s_in,to;
    socklen_t fromlen;
    char *buf;
    char *recvbuf;
    int recv_buf_size;
    int reuse;
    int i;
    unsigned long msg[4];
    int size =0;
    unsigned char * leavebuf =0;
    int leavelen = 0;
    VOICEINFO* pTemp = NULL;
    CWRTMsg* pMsg = NULL;
    int ret = 0;
    uint32_t  times,dates;
    unsigned long day,Month,years ;
    unsigned long min,second,Hour;

    unsigned long divert_ip;
    short divert_port;

    bool    b_isleave = false;

    unsigned long ticks1, ticks2;
    ticks1 = ticks2 = 0;
    recvbuf = (char *)ZENMALLOC(1600);

    if (recvbuf == NULL)
    {
        WRT_ERROR("alloc audio recv buffer fail!");

        return 0;
    }

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        WRT_ERROR("create audio socket fail 0x%x",errno);
        ZENFREE(recvbuf);
        return 0;
    }

    recv_buf_size = 32768;
    if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&recv_buf_size, sizeof(int)) != 0)
    {
        WRT_ERROR("set audio socket  SO_RCVBUF error=0x%x ",errno);
        ZENFREE(recvbuf);
        closesocket(sd);

        return 0;
    }

    reuse = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) != 0)
    {
        WRT_ERROR("set audio socket SO_REUSEADDR errno=0x%x \n",errno);
        ZENFREE(recvbuf);
        closesocket(sd);

        return 0;
    }
    /*
    reuse = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, (char *) &reuse, sizeof(reuse)) != 0)
    {
    WRT_ERROR("设置音频接收SOCKET SO_REUSEPORT errno=0x%x \n",errno);
    ZENFREE(recvbuf);
    closesocket(sd);
    t_delete(0);
    return;
    }
    */
    s_in.sin_family      = AF_INET;
    s_in.sin_addr.s_addr = INADDR_ANY;
    s_in.sin_port        = htons(15004);

    if (bind(sd, (struct sockaddr *) &s_in, sizeof(s_in)) != 0)
    {
        WRT_ERROR("set audio socket bind errno=0x%x \n",errno);
        ZENFREE(recvbuf);
        closesocket(sd);

        return 0;
    }

    //added 2007-9-24
    int mode = 1;
    ioctlsocket(sd, FIONBIO, (char *)&mode);

    struct timeval timeout;

    fd_set readfd;

    g_ao_msg_sem = SDL_CreateSemaphore(0);
    assert(g_ao_msg_sem != NULL);

    while (1)
    {
        memset(msg,0,sizeof(msg));
        divert_ip = 0;
        divert_port = 0;
        SDL_SemWait(g_ao_msg_sem);
        pMsg = g_ao_msg_queue.get_message();
        if(pMsg){
            ret = parse_msg(pMsg,msg);
            if(ret < 0)
                break;
            if(ret == 0)
                continue;
        }
        if(msg[0] != AUDIOSTART)
        {
            continue;
        }
        SetVolume(pSystemInfo->mySysInfo.talkvolume);
        divert_ip = msg[1];
        divert_port = (msg[2] & 0x0000ffff);
        WRT_MESSAGE("start recv audio divert_ip =0x%x,g_frame_size=%d",divert_ip,g_frame_size);
        SDL_LockMutex(g_play_mutex);
        //if(m_pCmdHandler && m_pCmdHandler->GetIsSimulateTalk())
        // RoomGpioUtilSetSpkOff();
        // else
        // RoomGpioUtilSetSpkOn();//打开扬声器
        while (1)
        {
            if (aomode == AudioOutModeStop) //Confirm audio play already stop
                break;
            SDL_Delay(20);
        }

        //初始化音频播放模块
        if (m_pCmdHandler->GetCurrentSysStatus() == SYS_CALLSIP || m_pCmdHandler->GetCurrentSysStatus() == SYS_SIPCALL)
        {
            if (OpenAudio(TYPE_PLAY, 1, g_arate, 16, 0)  == -1)
            {
                WRT_MESSAGE("TYPE_PLAY failed \n");
                SDL_UnlockMutex(g_play_mutex);
                continue;
            }

            WRT_MESSAGE("Talk ao init: %d %d %d %d",1,g_arate,16,g_frame_size);

#ifdef USE_MYJITTER
            InitMyJitter();
#endif
        }
        else
        {
            if (OpenAudio(TYPE_PLAY, 2, 16000, 16, 0) == -1)
            {
                WRT_MESSAGE("TYPE_PLAY failed \n");
                SDL_UnlockMutex(g_play_mutex);
                continue;
            }
            WRT_MESSAGE("Ao init %d %d %d %d", 2, 1600, 16, 768);
        }
        aomode = AudioOutModeTalk;
        WRT_MESSAGE("Start recv talk data");
        //启动数据接收任务
        while(1)
        {
            timeout.tv_sec = 0;
#ifdef USE_MYJITTER        
            if(m_pCmdHandler->GetCurrentSysStatus() == SYS_CALLSIP || m_pCmdHandler->GetCurrentSysStatus() == SYS_SIPCALL)
            { //如果呼叫SIP客户端，则接受数据为RTP包
                timeout.tv_usec = 20000;       
            }
            else
            {                  
                timeout.tv_usec = 300000; //2009.4.8. modify 300MS        
            }
#else
            timeout.tv_usec = 7000; //2009.4.8. modify 300MS
#endif  
            //SDL_Delay(1000);
            memset(msg,0,sizeof(msg));
            if (SDL_SemTryWait(g_ao_msg_sem) == 0)
            {
                pMsg = g_ao_msg_queue.get_message();
                if (pMsg)
                {
                    ret = parse_msg(pMsg,msg);
                    if (ret < 0)
                        break;
                    if (ret == 0)
                        continue;
                }
                if (msg[0] == AUDIOSTOP)
                {
                    break;
                }
                else if (msg[0] == LEAVE_STOP)
                {
                    if (b_isleave == true)
                    {
                        tm_get(&dates,&times,NULL);
                        day= dates&0xff;
                        Month = (dates >> 8) &0xff;
                        years = (dates>>16) & 0xffff;
                        Hour = (times >> 16)& 0XFFFF;
                        second = (times & 0xff);
                        min = (times >> 8) &0xff;
#ifdef SEND_TO_CENTER  //2009-7-14
                        if (leavebuf && (leavelen > 0))
                        {
                            unsigned char* sendbuf = (unsigned char*)ZENMALLOC(leavelen);
                            if (sendbuf)
                            {
                                memcpy(sendbuf,leavebuf,leavelen);
                                send_voice_photo_to_center(3,ROOM_SEND_PHOTO_VOICE,sendbuf,	leavelen);
                            }
                        }
#endif
                        if(pTemp){
                            int itmp = 0;
                            if(m_pCmdHandler)
                                itmp = m_pCmdHandler->GetIsDoorCall();
                            pTemp->read  |= (itmp << 16) & 0xffff0000;
                            sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".voc");
                            add_voice(pTemp,(unsigned char*)leavebuf,leavelen);
                            WRT_MESSAGE("增加留言名称%s ，行号%d",pTemp->name,__LINE__);
                        }

                        b_isleave = false;
                        leavebuf = 0;
                        leavelen = 0;
                        pTemp = 0;
                    }
                }//
                else if (msg[0] == LEAVE_CANCEL)
                {
                    b_isleave = false;
                    if(leavebuf)
                        ZENFREE(leavebuf);
                    leavebuf = 0;
                    leavelen = 0;
                    if(pTemp)
                        memset(pTemp,0,sizeof(VOICEINFO));
                }
                else if (msg[0] == LEAVE_START)
                {
                    b_isleave = true;
                    if ((pTemp = get_idle_voice()) == NULL)
                    {
                        b_isleave = false;
                    }
                    else
                    {
                        size = 1*1024*1024; //1M memory flash???
                        if(leavebuf == NULL)
                            leavebuf  = (unsigned char*)ZENMALLOC(size);
                    }
                }
            }
            if (m_pCmdHandler->GetCurrentSysStatus() == SYS_IDLE)
            {
                WRT_DEBUG("Cur system error %d",m_pCmdHandler->GetCurrentSysStatus());
                break;
            }

            FD_ZERO(&readfd);
            FD_SET(sd, &readfd);
            result = select(sd+1, &readfd, NULL, NULL, &timeout);
            if(result < 0)
            {
                printf("select recv timeout \n");
                continue;
            }

            if(FD_ISSET(sd, &readfd))
            {
                fromlen = sizeof(struct sockaddr);
                result = recvfrom(sd, recvbuf, 1600, 0, (struct sockaddr *)&from, &fromlen);
                // printf("recvfrom  %d \n",result);
                if (result > 0)
                {
                    if (m_pCmdHandler->GetCurrentSysStatus() == SYS_CALLSIP || m_pCmdHandler->GetCurrentSysStatus() == SYS_SIPCALL)
                    { //如果呼叫SIP客户端，则接受数据为RTP包
                        if (result == g_rtp_packetlen)
                        {
#ifdef USE_MYJITTER
                            unsigned char* rtpbuf = (unsigned char*)ZENMALLOC(g_rtp_packetlen);
                            if (NULL != rtpbuf)
                            {
                                memcpy(rtpbuf,recvbuf,g_rtp_packetlen);
                                wrt_rtp_header_t* p = (wrt_rtp_header_t*)rtpbuf;
                                p->seq_number = ntohs(p->seq_number);
                                p->timestamp  =  ntohl(p->timestamp);
                                PutMyJitter(p);                                       
                            }                                
#else
                            unsigned char* tempbuf = NULL;
                            tempbuf = (unsigned char*)(recvbuf+12);
                            play_audio(AudioOutInstatce, tempbuf, result-12);
                            continue;
#endif
                        }
                    }
                    else if (result == 384)
                    {
                        if (divert_ip != 0 && divert_port != 0)
                        {
                            to.sin_addr.s_addr = divert_ip;
                            to.sin_port = htons(divert_port);
                            to.sin_family = AF_INET;
                            int rrr = sendto(sd,recvbuf,result,0,(struct sockaddr*)&to,sizeof(struct sockaddr_in));

                            continue; //转发数据目的地址。
                        }
                        if (b_isleave == true)
                        {

                            if ((leavelen +384) <= size)
                            {
                                memcpy(leavebuf+leavelen,recvbuf,384);
                                leavelen +=384;
                            }
                            else
                            {
                                tm_get(&dates,&times,NULL);
                                day= dates&0xff;
                                Month = (dates >> 8) &0xff;
                                years = (dates>>16) & 0xffff;
                                Hour = (times >> 16)& 0XFFFF;
                                second = (times & 0xff);
                                min = (times >> 8) &0xff;
#ifdef SEND_TO_CENTER   //2009-7-14
                                if (leavebuf && (leavelen > 0))
                                {
                                    unsigned char* sendbuf = (unsigned char*)ZENMALLOC(leavelen);
                                    if (sendbuf)
                                    {
                                        memcpy(sendbuf,leavebuf,leavelen);
                                        send_voice_photo_to_center(3,ROOM_SEND_PHOTO_VOICE,sendbuf,leavelen); //小门口机
                                    }
                                }
#endif
                                if (pTemp)
                                {
                                    int itmp = 0;
                                    if(m_pCmdHandler)
                                        itmp = m_pCmdHandler->GetIsDoorCall();
                                    pTemp->read  |= (itmp << 16) & 0xffff0000;
                                    sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".voc");
                                    WRT_MESSAGE("Add leave voice %s, line: %d",pTemp->name,__LINE__);
                                    add_voice(pTemp,(unsigned char*)leavebuf,leavelen);
                                }
                                b_isleave = false;
                                leavebuf = 0;
                                leavelen = 0;
                                pTemp  =0;
                            }
                        }
                        play_audio(AudioOutInstatce,(unsigned char*)recvbuf,result);
                    }
                    else if (result == 512) //测试用于A5S-801D A8S-801D
                    {
                        play_audio(AudioOutInstatce, (unsigned char*)recvbuf, result);
                    }
                }
            }

#ifdef USE_MYJITTER
            wrt_rtp_header_t * p1 = GetMyJitter();
            if(p1 != NULL)
            {                    
                play_audio(AudioOutInstatce,(unsigned char*)((unsigned char*)p1+12),g_frame_size/2);
                ZENFREE(p1);
                p1 = NULL;   
            }                      
#endif            
        }

        //AudioOutExit(AudioOutInstatce);
        SDL_Delay(50);
        StopAudio(TYPE_PLAY);
        SDL_Delay(50);

        aomode = AudioOutModeStop;
        RoomGpioUtilSetSpkOff();//关闭扬声器
        WRT_MESSAGE("Stop recv talk data");
        gPlayIsStart = false;
#ifdef USE_MYJITTER
        ExitMyJitter();
#endif  
        SDL_UnlockMutex(g_play_mutex);
    }

    closesocket(sd);
    ZENFREE(recvbuf);
    g_ao_msg_queue.release();
    SDL_DestroySemaphore(g_ao_msg_sem);
    g_ao_msg_sem = NULL;
    return 0;
}


int g_audio_packsize = 1;
static int AIThread(void *pParam)
{
    int err;
    char* buf;
    short *buf16;
    char *outbuf;
    int i;
    int result;
    int sd;
    struct sockaddr_in to,s_in;
    int send_buf_size;
    int reuse;
    unsigned long msg[4];
    uint32_t  times,dates;
    unsigned long day, Month, years;
    unsigned long min, second, Hour;
    unsigned long sendip;
    short         sendport;
    int size =0;
    unsigned char * leavebuf =0;
    int leavelen = 0;
    VOICEINFO* pTemp = NULL;
    CWRTMsg* pMsg = NULL;

    bool b_isleave = false;
    int  isrecord  = 0;  //2009-7-15

    char* rtpbuf = NULL; //by ljw 2010.3.29
    unsigned long timeindex = 0; //by ljw 2010-5-4 8:52:19
    wrt_rtp_header_t    rtpheader;
    struct timeval timeaudio; //for schedule send audio by ljw 2010-5-6 9:03:08 
    UInt32 m_timestamp;
    uint32_t m_captimestamp;
    int irecordlen =0;
    int record1,record2 ;

    outbuf = (char *)ZENMALLOC(1460);
    if (outbuf == NULL)
    {
        WRT_ERROR("AIThread:alloc send buffer fail");
        return 0;
    }
    rtpbuf = (char*)ZENMALLOC(1460);
    if(rtpbuf == NULL)
    {
        WRT_ERROR("AIThread:alloc rtp buffer fail");
        ZENFREE(outbuf);
        return 0;
    }
#if USE_AUDIO_MEDIASTREAM

#else 
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        WRT_ERROR("AIThread:Create socket fail 0x%x",errno);
        ZENFREE(outbuf);
        ZENFREE(rtpbuf);
        return 0;
    }

    send_buf_size = 32768;
    if (setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char *)&send_buf_size, sizeof(int)) != 0)
    {
        WRT_ERROR("AIThread:Set SOCKET  SO_SNDBUF error=0x%x",errno);
        ZENFREE(outbuf);
        ZENFREE(rtpbuf);
        closesocket(sd);
        return 0;
    }

    reuse = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) != 0)
    {
        WRT_ERROR("AIThread:SOCKET  SO_REUSEADDR error=0x%x",errno);
        ZENFREE(outbuf);
        ZENFREE(rtpbuf);
        closesocket(sd);
        return 0;
    }
    /*
    reuse = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEPORT, (char *) &reuse, sizeof(reuse)) != 0)
    {
    WRT_ERROR("设置音频发送SOCKET  SO_REUSEPORT error=0x%x",errno);
    ZENFREE(outbuf);
    closesocket(sd);
    t_delete(0);
    return;
    }
    */
    s_in.sin_family      = AF_INET;
    s_in.sin_addr.s_addr = INADDR_ANY;
    s_in.sin_port        = htons(15003);

    if (bind(sd, (struct sockaddr *) &s_in, sizeof(s_in)) != 0)
    {
        WRT_ERROR("AIThread:SOCKET  bind error=0x%x",errno);
        ZENFREE(outbuf);
        ZENFREE(rtpbuf);
        closesocket(sd);
        return 0;
    }
#endif

    g_ai_msg_sem = SDL_CreateSemaphore(0);
    assert(g_ai_msg_sem);

    while (1)
    {
        memset(msg,0,sizeof(msg));
        sendip = 0;
        sendport = 0;
        SDL_SemWait(g_ai_msg_sem);
        pMsg = g_ai_msg_queue.get_message();
        if (pMsg)
        {
            err = parse_msg(pMsg,msg);
            if(err < 0)
                break;
            if(err == 0)
                continue;
        }

        //q_receive(aistart_qid,Q_WAIT, 0, msg);
        if (msg[0] != AUDIOSTART)
        {
#if USE_AUDIO_MEDIASTREAM

            stop_wrt_audiostream();
            //uninit_wrt_audiostream();
#endif
            continue;
        }
        SetVolume(pSystemInfo->mySysInfo.talkvolume);
#if USE_AUDIO_MEDIASTREAM


#else
        if (m_pCmdHandler && m_pCmdHandler->GetIsSimulateTalk())
        {
            RoomGpioUtilSetMicOff();
            WRT_MESSAGE("close mic");
        }
        else
        {
            RoomGpioUtilSetMicOn(); //启动MIC
            WRT_MESSAGE("open mic");
        }
        int pcmbufsize;
        if(g_audio_packsize == 1)
            pcmbufsize = 768;//12MS
        else if(g_audio_packsize == 2)
            pcmbufsize = 1536;//24MS
        else if(g_audio_packsize == 3)
            pcmbufsize = 2560;//40ms
#endif
        sendip = msg[1];
        sendport = htons(msg[2]); 
#if USE_AUDIO_MEDIASTREAM
        set_audio_param(16000,1,12,250,1);
        start_wrt_audiostream(sendip,sendport,15004,1);
        continue;
#else
        while(1)
        {//需要确保play已经初始化,因为ALSA OPEN接口线程不安全。会导致同时open。导致open失败。
            if(aomode == AudioOutModeTalk)
                break;
            SDL_Delay(20);
        }
#endif

#if USE_AUDIO_MEDIASTREAM

#else
        //by ljw 2010.3.29
        if (m_pCmdHandler->GetCurrentSysStatus() == SYS_CALLSIP || m_pCmdHandler->GetCurrentSysStatus() == SYS_SIPCALL)
        {

            //ai_instance = AudioInInit(1,g_arate,0,16,g_frame_size);
            // SetFormat(AUDIORECORDDEVICE,16,g_arate,1);
            if(OpenAudio(TYPE_CAPTURE,1,g_arate,16,0) == -1)
            {
                WRT_MESSAGE("TYPE_CAPTURE init failed \n");
                continue;
            }

            pcmbufsize  = g_frame_size;
            timeindex = 0;
            m_timestamp =0;                     
            WRT_MESSAGE("sip ai init %d %d %d %d\n",1,g_arate,16,g_frame_size);
        }
        else
        {
            // ai_instance = AudioInInit(2,16000,0,16,768);
            //SetFormat(AUDIORECORDDEVICE,16,16000,2);
            pcmbufsize  = 1536;//768;
            WRT_MESSAGE("ai init %d %d %d %d\n",2,16000,16,pcmbufsize);
            if (OpenAudio(TYPE_CAPTURE,2,16000,16,0) == -1){ //最后一个参数决定了包的大小
                WRT_MESSAGE("TYPE_CAPTURE init failed \n");
                continue;
            }
        }
        //end

        WRT_MESSAGE("开始捕捉音频数据");
        size = 0;
        leavebuf = NULL;
        leavelen = 0;
        pTemp = 0;
        buf = (char*)ZENMALLOC(pcmbufsize);
        //SDL_Delay(64);//延时64MS使得录音缓冲中有足够的数据
        record1 = record2 =0;
        int count =0;
        while(1)//通话或者录音中，每次采集768字节做G711压缩
        {
            memset(msg,0,sizeof(msg));
            if(SDL_SemTryWait(g_ai_msg_sem) == 0)
            {
                pMsg = g_ai_msg_queue.get_message();
                if(pMsg)
                {
                    err = parse_msg(pMsg,msg);
                    if(err < 0)
                        break;
                    if(err == 0)
                        continue;
                }
                if(msg[0] == AUDIOSTOP)
                {
                    if(b_isleave == true){
                        if(leavelen > 0){
                            tm_get(&dates,&times,NULL);
                            day= dates&0xff;
                            Month = (dates >> 8) &0xff;
                            years = (dates>>16) & 0xffff;
                            Hour = (times >> 16)& 0XFFFF;
                            second = (times & 0xff);
                            min = (times >> 8) &0xff;
#ifdef SEND_TO_CENTER //2009-7-14
                            if(leavebuf && (leavelen > 0)){
                                unsigned char* sendbuf = (unsigned char*)ZENMALLOC(leavelen);
                                if(sendbuf){
                                    memcpy(sendbuf,leavebuf,leavelen);
                                    send_voice_photo_to_center(isrecord?4:3,ROOM_SEND_PHOTO_VOICE,sendbuf,leavelen); //小门口机
                                }
                            }
#endif
                            sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".voc");
                            int itmp = 1;
                            itmp = isrecord?3:1;
                            pTemp->read |= (itmp << 16) & 0xffff0000;
                            add_voice(pTemp,(unsigned char*)leavebuf,leavelen);
                            WRT_MESSAGE("增加留言名称%s ，行号%d",pTemp->name,__LINE__);
                        }
                        if(m_pCmdHandler ){
                            unsigned long event[4];
                            if(isrecord == 0)
                            {
                                event[0] = ROOM_HANGUP;
                            }
                            else
                            {
                                event[0] = CUSTOM_CMD_ROOM_STOP_RECORD; //停止录音2009-7-15
                            }
                            event[1] = 4;
                            event[2] = event[3]=0;
                            m_pCmdHandler->Send_cmd(event);
                        }
                        b_isleave = false;
                        isrecord = 0;
                        leavebuf = 0;
                        leavelen = 0;
                    }
                    printf("AUDIOSTOP cap audio\n");
                    break;
                } 
                else if (msg[0] == LEAVE_STOP)
                {
                    if(b_isleave == true)
                    {
                        tm_get(&dates,&times,NULL);
                        day= dates&0xff;
                        Month = (dates >> 8) &0xff;
                        years = (dates>>16) & 0xffff;
                        Hour = (times >> 16)& 0XFFFF;
                        second = (times & 0xff);
                        min = (times >> 8) &0xff;
#ifdef SEND_TO_CENTER //2009-7-14
                        if(leavebuf && (leavelen > 0))
                        {
                            unsigned char* sendbuf = (unsigned char*)ZENMALLOC(leavelen);
                            if(sendbuf){
                                memcpy(sendbuf,leavebuf,leavelen);
                                send_voice_photo_to_center(isrecord?4:3,ROOM_SEND_PHOTO_VOICE,sendbuf,leavelen);
                            }
                        }
#endif
                        if (pTemp)
                        {
                            sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".voc");
                            int itmp = 1;
                            itmp = isrecord?3:1;
                            pTemp->read  |= (itmp << 16) & 0xffff0000;
                            add_voice(pTemp,(unsigned char*)leavebuf,leavelen);
                            WRT_MESSAGE("add voice name:%s,line:%d",pTemp->name,__LINE__);
                        }
                        b_isleave = false;
                        isrecord = 0;
                        leavebuf = 0;
                        leavelen = 0;
                        pTemp = 0;
                    }
                }
                else if(msg[0] == LEAVE_CANCEL)
                {
                    b_isleave = false;
                    isrecord = 0;
                    if(leavebuf)
                        ZENFREE(leavebuf);
                    leavebuf = 0;
                    leavelen = 0;
                    if(pTemp)
                        memset(pTemp,0,sizeof(VOICEINFO));
                }
                else if(msg[0] == LEAVE_START)
                {
                    b_isleave = true;
                    isrecord = msg[1];
                    if((pTemp = get_idle_voice()) == NULL)
                    {
                        b_isleave = false;
                        if(m_pCmdHandler)
                        {
                            unsigned long event[4];
                            if(isrecord == 0)
                                event[0] = ROOM_HANGUP;
                            else
                                event[0] = CUSTOM_CMD_ROOM_STOP_RECORD;//停止录音2009-7-15
                            event[1] = 5;
                            event[2] = event[3]=0;
                            m_pCmdHandler->Send_cmd(event);
                        }
                        isrecord = 0;
                    }
                    else
                    {
                        size  = 1*1024*1024;
                        if(leavebuf == NULL)
                            leavebuf  = (unsigned char*)ZENMALLOC(size);
                        WRT_MESSAGE("start leave voice");
                    }
                    }
            }
            tm_getticks(NULL,&m_captimestamp);

            irecordlen = pcmbufsize;
            record1 =  0;
            record2 = 0;
            //printf("begin............pcmbufsize = %d\n",pcmbufsize);
            while (record1 != pcmbufsize)
            {
                record2 = GetAudio((unsigned char*)(buf+record1), &irecordlen);
                if(record2 == 0)
                {
                    //printf("&\n");
                    break;
                }
                record1 += record2;
                irecordlen -= record2;
            }
            if (record2 == 0 && record1 != pcmbufsize)
            {
                //printf("record2 = %d,record1 = %d \n",record2,record1);
                SDL_Delay(11);
                continue;
            }
            // printf("@");
            m_timestamp = m_captimestamp*8;           

            //音频压缩
            buf16 = (short *)buf;
            for (i=0; i<record1/2; i++)
            {
                outbuf[i] = (char)linear2ulaw(buf16[i]);
            }
            //回收缓冲区
            // ZENFREE(buf);
            // buf = NULL;
            // AudioInBufClear(ai_instance,(unsigned char *)buf);

            if(b_isleave == false )
            {
                if(sendip == 0 || sendport == 0)
                continue;
                //by ljw 2010.3.29.
                int sendsize = pcmbufsize/2;
                char* rtpsendbuf = outbuf;

                if(m_pCmdHandler->GetCurrentSysStatus() == SYS_CALLSIP || m_pCmdHandler->GetCurrentSysStatus() == SYS_SIPCALL){ //如果呼叫SIP客户端，则接受数据为RTP包
                    // printf("call sip send rtp packet %x,%d,header =%d\n",sendip,sendport,sizeof(rtpheader));
                    memset(&rtpheader,0,sizeof(rtpheader));
                    rtpheader.paytype=0;
                    rtpheader.version =2;

                    timeindex++;
                    rtpheader.timestamp = m_timestamp;
                    rtpheader.seq_number = timeindex;
                    rtpheader.timestamp = htonl(rtpheader.timestamp);
                    rtpheader.seq_number = htons(rtpheader.seq_number);

                    memset(rtpbuf,0,g_rtp_packetlen);
                    memcpy(rtpbuf,&rtpheader,sizeof(rtpheader));
                    memcpy(rtpbuf+sizeof(rtpheader),outbuf,g_frame_size/2);
                    sendsize = g_rtp_packetlen;
                    rtpsendbuf = (char*)rtpbuf;


                    //continue;
                }
                else
                {
                    sendsize =pcmbufsize/2;
                    rtpsendbuf = outbuf;
                }
                //endif
                to.sin_family      = AF_INET;
                to.sin_addr.s_addr = sendip;
                to.sin_port        = sendport;
                result = 0;//sendsize;
                count++;

                if (count > 20)
                {
                    result = sendto(sd, rtpsendbuf,sendsize,0,(struct sockaddr *)&to,sizeof(to));
                    if (result != sendsize)
                    {
                        WRT_WARNING("send audio udp packet 0x%x",errno);
                    }
                }
                else
                {
                    ;//printf("count xxxxxx %d\n",count);
                }

            }
            else
            {
                if((leavelen +pcmbufsize/2) <= size)
                {
                    memcpy(leavebuf+leavelen,outbuf,pcmbufsize/2);
                    leavelen +=pcmbufsize/2;
                }
                else
                {
                    if (leavelen > 0)
                    {
                        tm_get(&dates,&times,NULL);
                        day= dates&0xff;
                        Month = (dates >> 8) &0xff;
                        years = (dates>>16) & 0xffff;
                        Hour = (times >> 16)& 0XFFFF;
                        second = (times & 0xff);
                        min = (times >> 8) &0xff;
#ifdef SEND_TO_CENTER  //2009-7-14
                        if(leavebuf && (leavelen > 0)){
                            unsigned char* sendbuf = (unsigned char*)ZENMALLOC(leavelen);
                            if(sendbuf){
                                memcpy(sendbuf,leavebuf,leavelen);
                                send_voice_photo_to_center(isrecord?4:3,ROOM_SEND_PHOTO_VOICE,sendbuf,leavelen);
                            }
                        }
#endif
                        int itmp = 1;
                        itmp = isrecord?3:1; //3本机，1小门口机
                        pTemp->read  |= (itmp << 16) & 0xffff0000;
                        sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".voc");
                        add_voice(pTemp,(unsigned char*)leavebuf,leavelen);
                        WRT_MESSAGE("Add leave voice name%s,line=%d",pTemp->name,__LINE__);
                    }
                    if(m_pCmdHandler){
                        unsigned long event[4];
                        if(isrecord == 0)
                            event[0] = ROOM_HANGUP;
                        else
                            event[1] = CUSTOM_CMD_ROOM_STOP_RECORD;//停止录音2009-7-15
                        event[1] = 6;
                        event[2] = event[3]=0;
                        m_pCmdHandler->Send_cmd(event);
                    }

                    b_isleave = false;
                    isrecord = 0;
                    leavebuf = 0;
                    leavelen = 0;
                }
            }//else
            SDL_Delay(11);
        }//while
        RoomGpioUtilSetMicOff();//关闭MIC
        StopAudio(TYPE_CAPTURE);
        WRT_MESSAGE("Stop AI thread");
        ZENFREE(buf);
        buf = NULL;
        if(err < 0)
        break;
#endif
    }//while

    closesocket(sd);
    ZENFREE(outbuf);
    ZENFREE(rtpbuf);
    g_ai_msg_queue.release();
    SDL_DestroySemaphore(g_ai_msg_sem);
    g_ai_msg_sem = NULL;

    return 0;
}

static int  leaveplayertask(void* pvoid){
	int m_cnt =0;
	while(1)
	{
		if(mp3_status == 1)
		{
			m_cnt++;
			SDL_Delay(20);
		}else if(mp3_status == 2)
		{
			m_cnt = 0;
		}else if(mp3_status == 0)
		{
			SDL_Delay(1000);
		}

		if(m_cnt == 1000)
		{
			printf("音频写被塞住，尝试重新启动音频线程\n");
			m_cnt = 0;
			mp3_status = 0;
			
			printf("音频重启成功\n");
		}
	}
#if 0
        int	Status;
        int     size;
        char *buf;
        int     BufSize=0;
        CWRTMsg* pMsg = NULL;


        int     err = 0;
        int     layer;
        int     first_head;
        int     m_AudioOutInstatce;
        int     loop  = 1;
        unsigned long RMsgBuff[4];
        int     result=384;
        int     i =0;
        int    isplayer = 0;
        uint32_t  times,dates;
        unsigned long day,Month,years ;
        unsigned long min,second,Hour;
        g_leave_msg_sem = SDL_CreateSemaphore(0);
        assert(g_leave_msg_sem);
        while(1)
        {


                memset(RMsgBuff,0,sizeof(unsigned long)*4);
                SDL_SemWait(g_leave_msg_sem);
                pMsg = g_leave_msg_queue.get_message();
                if(pMsg){
                        err = parse_msg(pMsg,RMsgBuff);
                        if(err < 0)
                                break;
                        if(err == 0)
                                continue;

                }
                if(RMsgBuff[2] != AUDIOSTART)
                {
                        continue;
                }
                SDL_LockMutex(g_play_mutex);
                

                RoomGpioUtilSetSpkOn();//打开扬声器
                RoomGpioUtilSetDoorAudioOff(); //25,26,1,1
                
                SetVolume(pSystemInfo->mySysInfo.ringvolume);

                buf = ( char *)RMsgBuff[0];
                size = (int)RMsgBuff[1];
                loop = RMsgBuff[3];
                WRT_MESSAGE("开始播放留言size:%d",size);
                //改回来。
                // SetFormat(AUDIOPLAYDEVICE,16,16000,2);
                while(1){//需要确保play已经初始化,因为ALSA OPEN接口线程不安全。会导致同时open。导致open失败。
						if(aomode == AudioOutModeStop)
								break;
						SDL_Delay(20);
				}
                if(OpenAudio(TYPE_PLAY,2,16000,16,1) == NULL){
                	WRT_MESSAGE("TYPE_PLAY init failed");
                	SDL_UnlockMutex(g_play_mutex);
                	continue;
                	
                }

               // m_AudioOutInstatce =AudioOutInit(2,16000,16,768);//AudioOutInit(1,8000,16,320); //AudioOutInit(2,16000,16,768);
                aomode = AudioOutModeLeave;
                BufSize  = 10*result;
                while(1)
                {
                        memset(RMsgBuff,0,sizeof(unsigned long)*4);
                        if(SDL_SemTryWait(g_leave_msg_sem) == 0)
                        {
                                pMsg = g_leave_msg_queue.get_message();
                                if(pMsg){
                                        err = parse_msg(pMsg,RMsgBuff);
                                        if(err < 0)
                                                break;
                                        if(err == 0)
                                                continue;
                                }
                                if(RMsgBuff[2] == AUDIOSTOP){
                                        break;
                                }
                        }

						play_audio(m_AudioOutInstatce,(unsigned char*)(buf+BufSize),result);
                         
                        if((BufSize+result) >=size){
                                  break;
                        }
                        BufSize += result;
                        //SDL_Delay(10);
                }

                //WaitAudioOutPlay(); //等待播放缓冲里面的数据完全播放完毕
                /*
                * 退出音频输出模块
                */
                
                SDL_Delay(50);
                StopAudio(TYPE_PLAY);
                SDL_Delay(50);
               // AudioOutExit(m_AudioOutInstatce);
                aomode = AudioOutModeStop;
                BufSize =0;
                RoomGpioUtilSetSpkOff();//关闭扬声器

                if(m_pCmdHandler){
                        unsigned long event[4];
                        event[0] = STATUS_LEAVE_PLAY_OVER;
                        event[1] = 0;
                        event[2]=event[3] = 0;
                        m_pCmdHandler->CallDisplayCallBack(event);
                }
                SDL_UnlockMutex(g_play_mutex);
                WRT_MESSAGE("留言播放完毕");
                if(err < 0)
                        break;
        }
        g_leave_msg_queue.release();
        SDL_DestroySemaphore(g_leave_msg_sem);
        g_leave_msg_sem = NULL;
        return 0;
#endif
}

#if 1

int GetPlayMp3Status(){
        return g_playstatus;
}
#if 0
static int mp3playertask(void* pvoid)
{
	int err = 0;
	char cmd_path[MAX_FILE_LEN];
        CWRTMsg* pMsg = NULL;
        unsigned long RMsgBuff[4];
	char gbbuf[MAX_FILE_LEN];
	char unibuf[MAX_FILE_LEN];
        g_mp3_msg_sem = SDL_CreateSemaphore(0);
        assert(g_mp3_msg_sem);
        while(1)
        {
                memset(RMsgBuff,0,sizeof(RMsgBuff));
                SDL_SemWait(g_mp3_msg_sem);
                pMsg = g_mp3_msg_queue.get_message();
                if(pMsg){
                        err = parse_msg(pMsg,RMsgBuff);
                        if(err < 0)
                                break;
                        if(err == 0)
                                continue;
                }
                if(RMsgBuff[2] != AUDIOSTART)
                {
                        continue;
                }
       		unlink("/dev/my_mplayer_fifo");
		mkfifo("/dev/my_mplayer_fifo",O_CREAT |0666);                
	 	g_fd_mplayer_fifo = open("/dev/my_mplayer_fifo",O_RDWR);
		if(g_fd_mplayer_fifo == -1)
			WRT_DEBUG("open /dev/my_mplayer_fifo failed \n");
                g_playstatus = RMsgBuff[3];
                memset(cmd_path,0,MAX_FILE_LEN);
                memset(gbbuf,0,MAX_FILE_LEN);
                strcpy(gbbuf,(char*)RMsgBuff[0]);
                memset(unibuf,0,MAX_FILE_LEN);
		gb2uni((const unsigned char*)gbbuf,(unsigned short*)unibuf,MAX_FILE_LEN);  
		memset(gbbuf,0,MAX_FILE_LEN);       		
                ucs2ToUtf8((const unsigned short*)unibuf,(unsigned char*)gbbuf,MAX_FILE_LEN);
                WRT_DEBUG("gbbuf = %s \n",gbbuf);
                sprintf(cmd_path,"mplayer-rc4 %s -slave -quiet  -loop 0 -input file=/dev/my_mplayer_fifo",gbbuf);
		//execlp("mplayer","mplayer","-slave","-quiet","-afm","ffmpeg" ,"-loop","0","-input","file=/dev/my_mplayer_fifo",gbbuf,NULL);
		system(cmd_path);
		WRT_DEBUG("===================================system return %d \n",errno);
		close(g_fd_mplayer_fifo);
		//WRT_DEBUG("execlp return %d \n",errno);
		g_fd_mplayer_fifo = -1;
        }
        g_mp3_msg_queue.release();
        SDL_DestroySemaphore(g_mp3_msg_sem);
        g_mp3_msg_sem = NULL;
        return 0;

}
#else

static unsigned char* read_mp3_file(char* filename,int *buflen){
	FILE* file = NULL;
	char gbbuf[MAX_FILE_LEN];
	char unibuf[MAX_FILE_LEN];			
        memset(gbbuf,0,MAX_FILE_LEN);
   		strcpy(gbbuf,(char*)filename);
        memset(unibuf,0,MAX_FILE_LEN);
        if(strstr(filename,"/zh/") != NULL ||strstr(filename,"/en/") != NULL){
		gb2uni((const unsigned char*)gbbuf,(unsigned short*)unibuf,MAX_FILE_LEN);  
		memset(gbbuf,0,MAX_FILE_LEN);       		
        	ucs2ToUtf8((const unsigned short*)unibuf,(unsigned char*)gbbuf,MAX_FILE_LEN);
        }

		file = fopen(gbbuf,"rb");
		WRT_DEBUG("read_mp3_file %s \n",gbbuf);	
		if(file){
			int readsize  = 0;
			unsigned char* buffer = NULL;
			fseek(file,0,SEEK_END);
			int len = ftell(file);
			fseek(file,0,SEEK_SET);
			*buflen = len;
			buffer = (unsigned char*)ZENMALLOC(len);
			if(buffer == NULL){
				fclose(file);
				return NULL;
			}
			memset(buffer,0,len);
			len = 0;
			while( !feof( file )){
				readsize = fread(buffer+len,sizeof(char),(*buflen-len),file);
				if(ferror(file)){
					WRT_DEBUG("read file %s ,error,%x",filename,errno);
					ZENFREE(buffer);
					buffer = NULL;
					break;
				}
				len += readsize;
				if(len == *buflen)
					break;
			}
			fclose(file);
			return buffer;
		}else
		{
			WRT_DEBUG("fopen  failed");
			
		}
	return NULL;
}
static int m_cnt=0;

static int mp3playertask(void* pvoid)
{
        int err = 0;
        int tkstart =0 ;
        int tkend = 0;
        int Status=0;
        int size;
        unsigned char *inbuf = (unsigned char *)ZENMALLOC(4096);
        unsigned  char *outbuf = (unsigned char *)ZENMALLOC(40960);
        unsigned char *tempbuf = (unsigned char *)ZENMALLOC(4096);

        unsigned int framesize=0;
        unsigned char *mp3buf=0;
        unsigned char    *buf=0;
        CWRTMsg* pMsg = NULL;
        int     mp3totallen=0;
        int     IsBufFree=0;
        int     freq=0;
        int     stereo=0;
        int     layer=0;
        int     first_head=0;
        int     m_AudioOutInstatce;
        int     samples=0;
        int     version=0;
        int     loop  = 1;
        int     icopy = 0; //已经从mp3buf里面COPY出来的数据长度
        int     iunprocess = 0;  //inbuf里面未处理的数据的长度
        unsigned long RMsgBuff[4];
        int     replay = 0;
        int     isbreak = 0;

        unsigned long     nTime = 180000; //180秒
        uint32_t timeshi,timeslo;
        UInt32   u32time1,u32time2;
        int    isbuttonvoice = 0;
        g_button_lock = SDL_CreateMutex();
        g_mp3_msg_sem = SDL_CreateSemaphore(0);
        assert(g_mp3_msg_sem);
        while(1)
        {
                memset(RMsgBuff,0,sizeof(RMsgBuff));
                isbuttonvoice = 0;
                SDL_SemWait(g_mp3_msg_sem);//有sem 
                pMsg = g_mp3_msg_queue.get_message();
                if(pMsg){
                        err = parse_msg(pMsg,RMsgBuff);
                        if(err < 0)
                                break;
                        if(err == 0)
                                continue;
                }
                if(RMsgBuff[2] != AUDIOSTART)//如果不是播放，继续
                {
                        continue;
                }
replaymp3:
                SDL_LockMutex(g_play_mutex);
                IsBufFree  = 0;
                if(RMsgBuff[1] == 0){
                	mp3buf = read_mp3_file((char*)RMsgBuff[0],&mp3totallen);
                	if(mp3buf == NULL){
                 		SDL_UnlockMutex(g_play_mutex);
                		continue;
                	}
                	IsBufFree = 1;
                }else{
                	mp3buf = ( unsigned char*)RMsgBuff[0];
                	mp3totallen = RMsgBuff[1];
                }
                if(mp3buf == NULL){
                	SDL_UnlockMutex(g_play_mutex);
                	continue;
                }
                
//replaymp3:
                WRT_MESSAGE("start play MP3");
                tm_getticks(&timeshi,&timeslo);
                u32time1 = timeslo;

                //WRT_DEBUG("MP3 TOTALLEN = %d",mp3totallen);
                loop = RMsgBuff[3];
                if(RMsgBuff[3] == 9)//9次循环
                {
                        isbuttonvoice = 1;
                        SDL_LockMutex(g_button_lock);
                }
                else
                {
                        isbuttonvoice = 0;
                }
                nTime = 180000;
                g_playstatus = loop;
                
				
				if(aomode == AudioOutModeTalk || aomode == AudioOutModeLeave)
                {
                	SDL_UnlockMutex(g_play_mutex);
                	SDL_UnlockMutex(g_button_lock);
                	continue;
                }
				aomode = AudioOutModeMp3;
			
				if(loop == 5){ //小门口机留言声
						//SetVolume(0);
						loop = 0;
				}else if(loop == 9)
				{
					int volume = GetAlsaVolume(TYPE_PLAY);
					//printf("volume %d\n",volume);
					volume +=1; 
					if(volume == 65)//68
						;//
					else
					{
						SetVolume(65);
					}
				}else if(loop == 10)
				{
					SetVolume(10);
				}
				else if(loop == 6){ //延时布防声
						loop = 1;
						
						int volume = GetAlsaVolume(TYPE_PLAY);
						volume +=1; 
						if(volume != pSystemInfo->mySysInfo.ringvolume-10)
							SetVolume(pSystemInfo->mySysInfo.ringvolume-10);
						nTime = pSystemInfo->mySysInfo.alarmtime * 1000;
				}else if (loop == 7){
						loop = 1;
						
						//int volume = GetAlsaVolume(TYPE_PLAY);
						//volume +=1; 
						//if(volume != pSystemInfo->mySysInfo.alarmvolume)
							SetVolume(pSystemInfo->mySysInfo.alarmvolume+16);
						nTime = pSystemInfo->mySysInfo.alarmvoicetime * 1000; //报警持续时间.
				}else{
						if(loop == 3 || loop == 2)
								loop = 1;
						else
								loop = 0;
						RoomGpioUtilSetDoorAudioOff();//25,26,1,1，(关闭小门口，分机)
						int volume = GetAlsaVolume(TYPE_PLAY);
						volume +=1;
						if(volume != pSystemInfo->mySysInfo.ringvolume)
							SetVolume(pSystemInfo->mySysInfo.ringvolume);
						
				}
                /*
		                * 初始化MP3播放器
		                */
                MpegAudioInit();

                /* 
	                * 从MP3数据中找到第一个有效的MP3头
	                */

                if(mp3totallen > 4096)
                        size = 4096;
                else
                        size = mp3totallen;

                memcpy(inbuf,mp3buf,size);

                Status = MpegAudioFindHead(mp3buf,mp3totallen,&framesize); //返回值是有效MP3头的偏移量
                first_head = Status;
                Status = 0;

                /*
	                * 得到MP3的频率
	                */
                freq = MpegAudioGetFrequency();

                /*
	                * 查询MP3的声道数,1为双声道，0为单声道
	                */
                stereo = MpegAudioIsstereo();

                /*
		                * 查询MP3的层数
		                */
                layer = MpegAudioGetLayer();

                version = MpegAudioGetVersion();

                //int mData = 0;
                //mData = (stereo==1?2:1)*freq*2/1000;

                /*
	                * 初始化音频输出模块,输入参数分别为通道数，频率，数据位数，输出块大小
	                * MP3的帧长是1152
	                */

                samples = version==0 ? 1152:576;
				printf("Init audio play freq=%d,stereo=%d,layer=%d,version=%d,framesize=%d,nTime=%d\n",freq,(stereo==1?2:1),layer,version,framesize,nTime);
				//struct timeval tv1;
				//
				//struct timeval tv2;
				//struct timezone tz;
				//int diff = 0;
				//int err;
				//err = gettimeofday(&tv1, &tz);
				//printf("11111\n");

				if(isbuttonvoice != 1)
				{
					printf("Not button voice\n");
	                if(OpenAudio(TYPE_PLAY,(stereo==1?2:1),freq,16,1) == -1){//1是阻塞式写，0是非阻塞式写
	                	WRT_MESSAGE("mp3player task TYPE_PLAY init failed");
	                	if(mp3buf){
	                		ZENFREE(mp3buf);
	                		mp3buf = NULL;
	                	}
	                	SDL_UnlockMutex(g_play_mutex);
	                	SDL_UnlockMutex(g_button_lock);
	                	continue;
	                }
	                StopAudio(TYPE_PLAY);
                	SDL_Delay(20);
                }
                
                if(OpenAudio(TYPE_PLAY,(stereo==1?2:1),freq,16,1) == -1){//1是阻塞式写，0是非阻塞式写
                	WRT_MESSAGE("mp3player task TYPE_PLAY init failed");
                	if(mp3buf){
                		ZENFREE(mp3buf);
                		mp3buf = NULL;
                	}
                	SDL_UnlockMutex(g_play_mutex);
                	SDL_UnlockMutex(g_button_lock);
                	continue;
                }
                mp3_status = 0;
                //printf("22222\n");
				
                WRT_DEBUG("Open audio device success!");

                if(size < 4096){
                        icopy  =  size-first_head; //已经COPY的数据
                        memcpy(inbuf,mp3buf+first_head,icopy); //将数据重新移动到inbuf起始位置
                        iunprocess = icopy; //当前inbuf中未处理数据的长度
                }else{
                        icopy  = size;//拷贝4096个字节
                        memcpy(inbuf,mp3buf+first_head,icopy);//将数据去掉head后，重新移动到inbuf的起始位置
                        iunprocess = icopy;//当前inbuf中未处理数据的长度
                }
                
				//上面拷贝了mp3头后面的第一个包。
                /*
	                * 获取MP3数据，循环播放
	                */
                while(1)
                {
                        memset(RMsgBuff,0,sizeof(RMsgBuff));
                        if(SDL_SemTryWait(g_mp3_msg_sem) == 0)
                        {
                                pMsg = g_mp3_msg_queue.get_message();
                                if(pMsg){
                                        err = parse_msg(pMsg,RMsgBuff);
                                        if(err < 0)
                                                break;
                                        if(err == 0)
                                                continue;
                                }
                                if(RMsgBuff[2] == AUDIOSTOP)
                                {
                                		//printf("3333\n");
                                        WRT_MESSAGE("Recv mp3 play message");
                                        break;
                                }
                                if(RMsgBuff[2] == AUDIOSTART){
                                        if(RMsgBuff[3] == 2 || RMsgBuff[3] == 3 || RMsgBuff[3] == 5 || RMsgBuff[3] == 8){
                                                replay = 1;
                                                WRT_MESSAGE("Replay mp3 0x%x",RMsgBuff[3]);
                                                break;
                                        }
                                }

                        }
                        tm_getticks(&timeshi,&timeslo);
                        u32time2 =  timeslo;
                        u32time2 -= u32time1;
                        if(u32time2 > nTime){
                                WRT_MESSAGE("time over, stop mp3 play");
                                break;
                        }
                        if(err < 0)
                                break;

                        /*
                        * 获取Mp3数据，让缓冲区数据保持4096字节
                        */
                        if(Status != 0){ //表示inbuf里面的数据已经解码完毕，需要重新COPY数据到inbuf里面
                        	
                                int itmp1 = iunprocess-Status; //剩下的数据  ，Status 表示size里面已经解码的数据长度
                                int itmp2 = mp3totallen-first_head-icopy; //剩下未COPY得数据
                                //memset(tempbuf,0,4096);
                                //memcpy(tempbuf,inbuf-Status,itmp1);
                                //memset(inbuf,0,4096);
                                //memset(outbuf,0,40960);
                                memcpy(inbuf,inbuf-Status,itmp1);//拷贝inbuf剩余数据到inbuf首地址
                                if( itmp2 > Status){ //如果剩下的未COPY数据里面还有一个Status,则copy
                                		
                                        memcpy(inbuf-itmp1,mp3buf+first_head+icopy,Status);
                                        icopy += Status;
                                }else { //如果剩下的数据，不够Status;则判断剩下的数据足够多少frame ,就copy多少帧的数据
                                        int itmp3 = itmp2+itmp1; //总未解码的数据
                                        itmp3 =(int)( itmp3/framesize); //有多少帧
                                        if(itmp3 > 0){
                                                itmp3 *= framesize; //算出需要COPY的数据
                                                memcpy(inbuf-itmp1,mp3buf+first_head+icopy,itmp3-itmp1);
                                                icopy += (itmp3-itmp1);
                                                iunprocess = itmp3;
                                        }else{
                                                if(loop == 1 || loop == 2){
                                                		
                                                        if(size < 4096){
                                                                memset(inbuf,0,4096);
                                                                memset(outbuf,0,40960);
                                                                icopy  =  size-first_head;
                                                                memcpy(inbuf,mp3buf+first_head,icopy);
                                                                iunprocess = icopy;
                                                        }else{
                                                                memset(inbuf,0,4096);
                                                                memset(outbuf,0,40960);
                                                                icopy  = size;
                                                                memcpy(inbuf,mp3buf+first_head,icopy);
                                                                iunprocess = icopy;
                                                        }
                                                        gInputFlag = 0;
                                                        if(loop == 2)
                                                                SDL_Delay(2000);
                                                        else
                                                                SDL_Delay(1000);
                                                }else{
                                                        WRT_MESSAGE("Mp3 play complete!");
                                                        break;
                                                }
                                        }
                                }

                        }
                        /*
                        * 对缓冲区数据进行解码，返回值为-1，则解码出错；返回值>0为消耗的缓冲区长度；
                        * 返回值=0，则数据未使用.
                        */
                        int playlen = 0;
                        Status=MpegAudioDecoder(outbuf,inbuf,iunprocess,&playlen);
                        if (Status == -1)
                        {
                                WRT_WARNING("MP3 decode error");
                                break;
                        }
                        /*
                        * 将解码得到的数据输出，音频输出模块内部有缓冲区，当内部缓冲区满，则应该阻塞，
                        * 等待解码缓冲区有空间区域。解码需要足够快，以保证内部缓冲区不为空。AudioOutGetBuf
                        * 为非阻塞式的等待.
                        */
                        //printf("mp3 %s Dec %d %d\n",RMsgBuff[0],playlen);
                        if(playlen == 0)
                        {
                        	continue;
                        }
                        mp3_status = 1;
                        PlayAudio((unsigned char*)outbuf,playlen);
                        //usleep(30*1000);
                        mp3_status = 2;

                        //SDL_Delay(playlen/mData);
						//if(isbuttonvoice == 1)
                        //	SDL_Delay(12);
                }
                

                /*
                * MP3播放模块退出
                */
				//printf("write over %d\n",m_cnt++);
                MpegAudioClose();
                
                StopAudio(TYPE_PLAY);
                SDL_Delay(20);
                //if (!gettimeofday(&tv2, &tz) && !err) {
				//	diff = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);
				///} 
				//printf("open alsa device time %d\n",diff);
                if(isbuttonvoice == 1)
                {
                	SDL_UnlockMutex(g_button_lock);
                }
                SDL_UnlockMutex(g_play_mutex);
                
                
                if(replay == 1){
                        replay = 0;
                        goto replaymp3;
                }

                aomode = AudioOutModeStop;
                
                if((IsBufFree== 1)&& mp3buf){
                	ZENFREE(mp3buf);
                	mp3buf = NULL;
                }
                
                // SetMainSpkStatus(0);//打开主分机spk
                WRT_DEBUG("mp3 play end");
                
                if(err < 0)
                        break;
        }
        ZENFREE(inbuf);
        ZENFREE(outbuf);
        ZENFREE(tempbuf);
        g_mp3_msg_queue.release();
        SDL_DestroySemaphore(g_mp3_msg_sem);
        g_mp3_msg_sem = NULL;
        SDL_DestroyMutex(g_button_lock);
        g_button_lock = NULL;
        return 0;

}

#endif
#endif

int GetsmalldoorLeaveStatus()
{
        return IsSmalldoorLeave;
}

int cancelsmalldoorleave()
{
        if(IsSmalldoorLeave){
                unsigned long msg[4];
                memset(msg,0,16);
                msg[0] = LEAVE_CANCEL;
                IsSmalldoorLeave = false;
                return g_ai_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ai_msg_sem);
                //return q_send(aistart_qid,msg);
        }
        return -1;
}

int startsmalldoorleave(){
        if(!IsSmalldoorLeave){
                unsigned long msg[4];
                memset(msg,0,16);
                msg[0] = LEAVE_START;
                IsSmalldoorLeave = true;
                return g_ai_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ai_msg_sem);
        }
        return -1;
}


int stopsmalldoorleave(){
        if(IsSmalldoorLeave){
                unsigned long msg[4];
                memset(msg,0,16);
                msg[0] = LEAVE_STOP;
                IsSmalldoorLeave = false;
                RoomGpioUtilSetDoorAudioOff();
                return g_ai_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ai_msg_sem);
        }
        return -1;
}
VOICEINFO* pTemp = NULL;

void start_record(){
	while(1){
		if(aomode == AudioOutModeStop)
				break;
		SDL_Delay(20);
	}
	SDL_Delay(50);
	SetCapVolume(90);
	aomode = AudioOutModeLeave;
	set_audio_param(16000,1,20,0,1);

	pTemp = NULL;
	if((pTemp = get_idle_voice()) == NULL){
			if(m_pCmdHandler){
					unsigned long event[4];
					event[0] = CUSTOM_CMD_ROOM_STOP_RECORD;//停止录音2009-7-15
					event[1] = 5;
					event[2] = event[3]=0;
					m_pCmdHandler->Send_cmd(event);
			}
	}else{
		system("rm -f /home/wrt/tmp.wav");
		start_wrt_local_leave("/home/wrt/tmp.wav");
	}

}

void stop_record(){

	unsigned char *leavebuf;
	int size = 0;
	struct stat buf;
	stop_wrt_local_leave();
	SDL_Delay(50);
	aomode = AudioOutModeStop;
	stat("/home/wrt/tmp.wav",&buf);
	size  = buf.st_size;
	if(size == 0)
		return;
	uint32_t  times,dates;
	unsigned long day,Month,years ;
	unsigned long min,second,Hour;
	leavebuf = (unsigned char *)ZENMALLOC(size+1);
	
	if(leavebuf == NULL)
	{
		printf("申请留言保存BUF失败\n");
		return;
	}
		
	FILE *fp = NULL;
	fp = fopen("/home/wrt/tmp.wav","r");
	fread(leavebuf,size,1,fp);

	tm_get(&dates,&times,NULL);
	day= dates&0xff;
	Month = (dates >> 8) &0xff;
	years = (dates>>16) & 0xffff;
	Hour = (times >> 16)& 0XFFFF;
	second = (times & 0xff);
	min = (times >> 8) &0xff;
	

	int itmp = 3;//local
	pTemp->read  |= (itmp << 16) & 0xffff0000;
	
	sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".wav");
	
	add_voice(pTemp,(unsigned char*)leavebuf,size);
	pTemp = NULL;
}

static char m_buf[256];
static int m_isPlayleaveword = 0;

static int LeavePlayByMplyaer(void *pParam)
{
	int pid =-1;
	pid = fork();
	
	if(pid<0)
	{
		printf("fork error!\n");
	}
	else if(pid == 0)
	{
		printf("留言播放开始! %s",m_buf);
		execl("/usr/bin/mplayer","mplayer","-slave","-quiet",m_buf);//,"-really-quiet"
	}else
	{
		printf("father process\n");
	}
	wait(NULL);
	
	WRT_DEBUG("留言播放完毕!");
	if(m_pCmdHandler){
            unsigned long event[4];
            event[0] = STATUS_LEAVE_PLAY_OVER;
            event[1] = 0;
            event[2]=event[3] = 0;
            m_pCmdHandler->CallDisplayCallBack(event);
    }
    m_isPlayleaveword = 0;
}

int leaveplay(char* buf,int buflen,int loop){
    return 0;
	leavestop();
	talktostop();
	mp3stop();
	m_isPlayleaveword = 1;
	SDL_Delay(50);//确保 其它音频线程设备释放完成
	while(1){
		if(aomode == AudioOutModeStop)
				break;
		SDL_Delay(20);
	}
	int volume = GetAlsaVolume(TYPE_PLAY);
	volume +=1; 
	if(volume != pSystemInfo->mySysInfo.talkvolume)
		SetVolume(pSystemInfo->mySysInfo.talkvolume);
	memset(m_buf,0,256);
	memcpy(m_buf,buf,strlen(buf));
	
	SDL_CreateThread(LeavePlayByMplyaer,NULL);
}

int leavestop(){
    return 0;
    if(m_isPlayleaveword == 1)
        system("killall -9 mplayer");
    m_isPlayleaveword = 0;
}

int mp3play(char *mp3buf, int mp3buflen,int loop)
{
    return 0;
	//if(g_fd_mplayer_fifo == -1){
        	unsigned long msg[4];
        	memset(msg, 0, 16);
        	msg[0] = (unsigned long)mp3buf;
        	msg[1] = mp3buflen;
        	msg[2] = AUDIOSTART;
        	msg[3] = loop;
        	WRT_DEBUG("mp3play:%s.",mp3buf);
        	g_mp3_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_mp3_msg_sem);
	/*}else{

			char cmdbuf[MAX_FILE_LEN];
			char gbbuf[MAX_FILE_LEN];
			char unibuf[MAX_FILE_LEN];			
			int ret = 0;
			memset(cmdbuf,0,MAX_FILE_LEN);
               		 memset(gbbuf,0,MAX_FILE_LEN);
                	strcpy(gbbuf,(char*)mp3buf);
                	memset(unibuf,0,MAX_FILE_LEN);
			gb2uni((const unsigned char*)gbbuf,(unsigned short*)unibuf,MAX_FILE_LEN);  
			memset(gbbuf,0,MAX_FILE_LEN);       		
               		ucs2ToUtf8((const unsigned short*)unibuf,(unsigned char*)gbbuf,MAX_FILE_LEN);
                	WRT_DEBUG("mplayer loadfile gbbuf = %s \n",gbbuf);			
			sprintf(cmdbuf,"loadfile %s \n",gbbuf);
        		ret = write(g_fd_mplayer_fifo,cmdbuf,strlen(cmdbuf));
        		WRT_DEBUG("write ret  = %d ,errno = %x \n",ret,errno);
			
	}
	
    */   
        return 1;

}

int mp3stop()
{
    return 0;
        // SendStopWarning();
        unsigned long msg[4];
        memset(msg, 0, 16);
        msg[2] = AUDIOSTOP;
        WRT_MESSAGE("mp3stop");
        g_mp3_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_mp3_msg_sem);
        //SDL_Delay(5000);
        /*
        if(g_fd_mplayer_fifo != -1){
        	int ret;
        	while(1){
        		ret = write(g_fd_mplayer_fifo,"quit\n",strlen("quit\n"));

        		WRT_DEBUG("===============write [quit] ret  = %d ,errno = %x g_fd_mplayer_fifo =%d\n",ret,errno,g_fd_mplayer_fifo);
        		if(errno == EINTR)
        			continue;
        		else
        			break;
        	}
        	while(1){
        		if(g_fd_mplayer_fifo == -1)
        			break;
        		SDL_Delay(500);
        	}
        	SDL_Delay(500);
        }
       */
        
        /*
        int ret=0;

        ret = system("killall -15 mplayer");
        WRT_DEBUG("mp3stop: result=%d.errno=%x",ret,errno);  
   
        SDL_Delay(2000);
        ret = system("killall -9 mplayer");
         WRT_DEBUG("mp3stop1: result=%d.errno=%x",ret,errno);  
	*/
        return 1;
}

int talkstart()
{
    printf("talkstart\n");
    //return 0;
#if USE_OLD_AUDIO_INTERFACE
        mp3stop();
        leavestop();
        unsigned long msg[4];
        int retval;
        memset(msg, 0, 16);
        msg[0] = AUDIOSTART;
         retval = g_ao_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ao_msg_sem);
        return retval;
#endif
        return 0;
}

int talkstop()
{
    //return 0;
#if USE_OLD_AUDIO_INTERFACE
        unsigned long msg[4];
        memset(msg, 0, 16);
        msg[0] = AUDIOSTOP;
        g_ao_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ao_msg_sem);
        return 1;
#endif
        return 0;
}

static Uint32 voltimer_callback(Uint32 Interval,void* pvoid)
{
	int remote_platform = get_remote_platform_type();

	if(get_aenc_type() >= A_PCM || get_adec_type() >= A_PCM)
	{
		if(remote_platform == DOOR_B)
			SetVolume(pSystemInfo->mySysInfo.talkvolume-15);
		else
		{
			if(pSystemInfo->mySysInfo.talkvolume > 85)
			{
				SetVolume(87);
			}
			else
			{
				if(pSystemInfo->mySysInfo.talkvolume == 80)
					SetVolume(78);
				else
					SetVolume(pSystemInfo->mySysInfo.talkvolume);
			}
		}
	}else
	{
		if(pSystemInfo->mySysInfo.talkvolume > 85)
			SetVolume(85);
		else
		{
			if(pSystemInfo->mySysInfo.talkvolume == 80)
				SetVolume(78);
			else
				SetVolume(pSystemInfo->mySysInfo.talkvolume);
		}
	}
	
	return 0;
}

int talktostart(unsigned long ip,short port)
{
    //return 0;
    printf("talktostart\n");
#if USE_OLD_AUDIO_INTERFACE
        unsigned long msg[4];
        memset(msg, 0, 16);
        msg[0] = AUDIOSTART;
        msg[1] = ip;
        msg[2] = port;
        return g_ai_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ai_msg_sem);
#endif
	mp3stop();
	//system("amixer cset numid=10,iface=MIXER,name='ADC PGA Gain' 1");

	SDL_Delay(100);

	//  struct timeval tv1;
	//	struct timeval tv2;
	//	struct timezone tz;
	//	int diff = 0;
	//	int err;
	//	err = gettimeofday(&tv1, &tz);
	aomode = AudioOutModeTalk;
	
	SetVolume(70);
	int remote_platform = get_remote_platform_type();

	if(get_aenc_type() >= A_PCM || get_adec_type() >= A_PCM)
	{
		if((remote_platform == ROOM_745D)
			|| (remote_platform == EXTEND_8K)
		)
		{
			set_audio_param(8000,1,12,203,3);
		}else if(remote_platform == ROOM_755D)
			set_audio_param(16000,1,12,203,3);//3表示D平台分机呼叫
		else
			set_audio_param(16000,1,12,203,3);//3表示D平台分机呼叫

		if(g_holdon_flag == 2)//外挂分机摘机
		{
			SetMainMicSpkOFF();
			SetSubMicSpkON();
			if((remote_platform == DOOR_B) ||
				(remote_platform == SMALL_DOOR_B))
			{
				SetCapVolume(pSystemInfo->mySysInfo.micvolume-28);
			}else
				SetCapVolume(pSystemInfo->mySysInfo.micvolume-23);
		}
		else
		{
			SetSubMicSpkOFF();
			SetMainMicSpkON();
			SetCapVolume(pSystemInfo->mySysInfo.micvolume-10);
		}
	}
	else
	{
		//if(strcmp(doorname,"门口机")==0)
			set_audio_param(16000,1,12,203,3);//1表示非D平台分机呼叫
		//else
		//{
		//	printf("非门口机\n");
		//	set_audio_param(16000,1,20,220,3);
		//}
		if(g_holdon_flag == 2)//外挂分机摘机
		{
			SetMainMicSpkOFF();
			SetSubMicSpkON();
			SetCapVolume(pSystemInfo->mySysInfo.micvolume-28);
		}
		else
		{
			SetSubMicSpkOFF();
			SetMainMicSpkON();
			SetCapVolume(pSystemInfo->mySysInfo.micvolume-16);
		}
	}

	start_wrt_local_leave("/home/wrt/tmp.wav");
	stop_wrt_local_leave();
	SDL_Delay(100);
	//float softgain = 2.0;//
	//set_wrt_player_db_gain(&softgain);

	
	if(get_aenc_type() >= A_PCM || get_adec_type() >= A_PCM)
	{
		if(remote_platform == DOOR_B)
			SetVolume(pSystemInfo->mySysInfo.talkvolume-10);
		else
		{
			if(pSystemInfo->mySysInfo.talkvolume > 85)
			{
				SetVolume(87);
			}
			else
			{
				if(pSystemInfo->mySysInfo.talkvolume == 80)
					SetVolume(78);
				else
					SetVolume(pSystemInfo->mySysInfo.talkvolume);
			}
		}
	}else
	{
		if(pSystemInfo->mySysInfo.talkvolume > 85)
			SetVolume(85);
		else
		{
			if(pSystemInfo->mySysInfo.talkvolume == 80)
				SetVolume(78);
			else
				SetVolume(pSystemInfo->mySysInfo.talkvolume);
		}
	}

	start_wrt_audiostream(ip,port,15004,1);
	//SDL_TimerID volumetimer = 0;
	//volumetimer = SDL_AddTimer(1500,voltimer_callback,NULL);

	///if (!gettimeofday(&tv2, &tz) && !err) {
	//			diff = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);
	//		}//
	//printf("diff %d\n",diff);
	return 0;
}

int talktostop()
{
    //return 0;

#if USE_OLD_AUDIO_INTERFACE
	unsigned long msg[4];
	memset(msg, 0, 16);
	msg[0] = AUDIOSTOP;
	return g_ai_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_ai_msg_sem);
#endif
	stop_wrt_audiostream();

	SetSubMicSpkOFF();
	SetMainMicSpkON();
	
	//if(get_adec_type() >= A_NONE || get_aenc_type() >= A_NONE)
	//	Reset_negotiate_audioresult();

	SDL_Delay(50);
	aomode = AudioOutModeStop;
	return 0;
}

int talkLeaveCancel(void){

	if(IsLeaveWord){
		stop_wrt_remote_leave();
		SDL_Delay(20);
		IsLeaveWord = false;
		if(pTemp)
		{
			memset(pTemp,0,sizeof(VOICEINFO));
			pTemp = NULL;
		}
	}
	aomode = AudioOutModeStop;
	return 0;
}

int talkLeaveStart(void)
{
	mp3stop();
	SDL_Delay(50);
	
	while(1){
		if(aomode == AudioOutModeStop)
			break;
		SDL_Delay(20);
	}
	
	aomode == AudioOutModeLeave;

    if(!IsLeaveWord){
    
        if((pTemp = get_idle_voice()) == NULL){
			;
		}else{
            system("rm -f /home/wrt/tmp.wav");
            system("sync");
            set_audio_param(16000,2,12,250,3);
            start_wrt_remote_leave("/home/wrt/tmp.wav",15004);
            IsLeaveWord = true;
		}
    }
    return 0;
}

int talkLeaveStop(void)
{
        if(IsLeaveWord){
                IsLeaveWord = false;
                if(pTemp == NULL)
                	return 0;
                unsigned char *leavebuf;
				int size= 0;
				struct stat buf;
				stop_wrt_remote_leave();
				SDL_Delay(200);
				system("sync");
				stat("/home/wrt/tmp.wav",&buf);
				size  = buf.st_size;
				if(size == 0)
					return 0;
				uint32_t  times,dates;
				unsigned long day,Month,years ;
				unsigned long min,second,Hour;
				leavebuf = (unsigned char *)ZENMALLOC(size+1);
				if(leavebuf == NULL)
					return 0;
				FILE *fp = NULL;
				fp = fopen("/home/wrt/tmp.wav","r");
				fread(leavebuf,size,1,fp);
				
				tm_get(&dates,&times,NULL);
				day= dates&0xff;
				Month = (dates >> 8) &0xff;
				years = (dates>>16) & 0xffff;
				Hour = (times >> 16)& 0XFFFF;
				second = (times & 0xff);
				min = (times >> 8) &0xff;
//	 测试是否是此泄漏内存
				if(leavebuf && (size > 0)){
					unsigned char* sendbuf = (unsigned char*)ZENMALLOC(size);
					if(sendbuf){
							memcpy(sendbuf,leavebuf,size);
							send_voice_photo_to_center(3,ROOM_SEND_PHOTO_VOICE,sendbuf,size); //小门口机
//					}
					}
				}
				int itmp = 0;
				if(m_pCmdHandler)//door
						itmp = m_pCmdHandler->GetIsDoorCall();
				pTemp->read  |= (itmp << 16) & 0xffff0000;
				sprintf((char*)pTemp->name,"voc_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".wav");
				add_voice(pTemp,(unsigned char*)leavebuf,size);
				pTemp = NULL;
        }

        aomode == AudioOutModeStop;
        return 0;
}

int GetLeaveStatus(void){
        return IsLeaveWord;
}

int g_button_test = 0;


void StartButtonVocie(){
		
#ifndef WRT_MORE_ROOM
        if(m_startButtonVocie){
            	mp3play((char*)BUTTON_VOICE_PATH,0,9);
        }
#endif
}

void StartButtonVocie2()
{
	mp3play((char*)BUTTON_VOICE_PATH,0,10);
}

#define TEST_MP3 0
void starsmalldoorleavervoice(){

        unsigned long msg[4];
#if TEST_MP3
        unsigned char* addr = NULL;
        unsigned long addrlen = 0;
#endif
        memset(msg, 0, 16);
#if TEST_MP3
        if(m_pCmdHandler)
                addr = m_pCmdHandler->GetTestRingAddr(MSG_RING,&addrlen);
        if(addr == NULL){
                msg[0] = (unsigned long)leavevoice;
                msg[1] = sizeof(leavevoice);
        }else{
                msg[0] = (unsigned long)addr;
                msg[1] = addrlen ;
        }
#else

        if(GetCurrentLanguage() == ENGLISH){
                msg[0] = (unsigned long)leave_en;
                msg[1] = sizeof(leave_en);
        }else{
                msg[0] = (unsigned long)leave_zh;
                msg[1] = sizeof(leave_zh);
        }
#endif
        msg[2] = AUDIOSTART;
        msg[3] = 5;
        g_mp3_msg_queue.send_message(MODULE_MSG,(void*)msg,sizeof(msg),g_mp3_msg_sem);
        return ;
}

void stopsmalldoorleavevoice(){
        mp3stop();
}

void StartAlaramPromptVocie(){
        mp3stop();
        SDL_Delay(50);
        mp3play((char*)"/home/wrt/default/alarm.mp3",0,6);
}

void StopAlaramPromptVoice(){
        mp3stop();
}

void OpenAndCloseButtonVoice(bool isopen){
        m_startButtonVocie = isopen;
}

bool GetButtonVoiceStatus(){
        return m_startButtonVocie;
}

void OpenAndCloseRing(bool isopen){
        m_close_ring = isopen;
}

int start_smalldoor_audio(int issimulate){
        SDL_LockMutex(g_play_mutex);
        RoomGpioUtilSetSpkOn();//打开扬声器
        RoomGpioUtilSetMicOn();//启动MIC
        if(issimulate == 1){
        	RoomGpioUtilSetMicOff();//关闭MIC
        	RoomGpioUtilSetSpkOff();//关闭扬声器        	
            RoomGpioUtilSetDoorAudioOnD();
        }else
            RoomGpioUtilSetDoorAudioOnA();//打开模拟音频通道
        aomode = AudioOutModeTalk;
        return 0;
}

int stop_smalldoor_audio(int issimulate){
	if(aomode != AudioOutModeTalk)
		return 0;
    RoomGpioUtilSetMicOff();//关闭MIC
    RoomGpioUtilSetSpkOff();//关闭扬声器
    RoomGpioUtilSetDoorAudioOff();
    aomode = AudioOutModeStop;
    SDL_UnlockMutex(g_play_mutex);
    return 0;
}

int SetSipCallParam()
{
        if(g_ptime == 40){
                g_ptime = 20;
                g_frame_size = g_ptime*8*4;
                g_rtp_packetlen = g_frame_size/2 +12;
                g_arate = 16000;
                return 1;
        }else{
                g_ptime = 40;
                g_frame_size = g_ptime*8*2;
                g_rtp_packetlen = g_frame_size/2+12;
                g_arate = 8000;
                return 0;
        }
}

#ifdef HAVE_SUB_PHONE
int phone_audio_task(void* pvoid)
{
        int result;
        int sd;
        int err =0;
        struct sockaddr_in from,s_in,to;
        socklen_t fromlen;
        char *buf;
        char *recvbuf;
        int recv_buf_size;
        int reuse;
        unsigned long sendip;
        short    sendport;
        unsigned long msg[4];
        CWRTMsg* pMsg = NULL;
        int size =0;
        recvbuf = (char *)ZENMALLOC(1600);


        if (recvbuf == NULL)
        {
                WRT_ERROR("创建接收手机音频缓冲失败");
               
                return 0;
        }

        sd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sd < 0)
        {
                WRT_ERROR("创建接收手机音频SOCKET失败0x%x",errno);
                ZENFREE(recvbuf);
              
                return 0;
        }

        recv_buf_size = 32768;
        if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&recv_buf_size, sizeof(int)) != 0)
        {
                WRT_ERROR("配置接收手机音频SOCKET SO_RCVBUF error=0x%x",errno);
                ZENFREE(recvbuf);
                closesocket(sd);

                return 0;
        }

        reuse = 1;
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) != 0)
        {
                WRT_ERROR("配置接收手机音频SOCKET SO_REUSEADDR error=0x%x",errno);
                ZENFREE(recvbuf);
                closesocket(sd);
                return 0;
        }

        s_in.sin_family      = AF_INET;
        s_in.sin_addr.s_addr = INADDR_ANY;
        s_in.sin_port        = htons(15002);

        if (bind(sd, (struct sockaddr *) &s_in, sizeof(s_in)) != 0)
        {
                WRT_ERROR("配置接收手机音频SOCKET bind error=0x%x",errno);
                ZENFREE(recvbuf);
                closesocket(sd);
                return 0;
        }

        //added 2007-9-24
        int mode = 1;
        ioctlsocket(sd, FIONBIO, (char *)&mode);

        struct timeval timeout;

        fd_set readfd;
        g_phone_audio_msg_sem = SDL_CreateSemaphore(0);
        assert(g_phone_audio_msg_sem);

        while(1)
        {
                memset(msg,0,sizeof(msg));
                sendip = 0;
                sendport =  0;
                SDL_SemWait(g_phone_audio_msg_sem);
                pMsg = g_phone_audio_msg_queue.get_message();
                if(pMsg){
                        err = parse_msg(pMsg,msg);
                        if(err < 0)
                                break;
                        if(err == 0)
                                continue;
                }

                if(msg[0] != AUDIOSTART)
                {
                        continue;
                }
                if(err < 0)
                        break;
                WRT_MESSAGE("开始接收手机音频数据%d 0x%x:%d",msg[0],msg[1],msg[2]);
                sendip = msg[1];
                sendport = htons(msg[2]);
                if(sendip == 0 || sendport == 0)
                        continue;
                while(1)
                {

                        memset(msg ,0,sizeof(msg));
                        if(SDL_SemTryWait(g_phone_audio_msg_sem) == 0){
                                pMsg = g_phone_audio_msg_queue.get_message();
                                if(pMsg){
                                        err = parse_msg(pMsg,msg);
                                        if(err < 0)
                                                break;
                                        if(err == 0)
                                                continue;
                                }
                                if(msg[0] == AUDIOSTOP)
                                        break;
                        }
                        FD_ZERO(&readfd);
                        FD_SET(sd, &readfd);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = 300000; //2009.4.8. modify 300MS
                        result = select(sd+1, &readfd, NULL, NULL, &timeout);
                        if(result <= 0)
                        {
                                continue;
                        }
                        if(FD_ISSET(sd, &readfd))
                        {

                                fromlen = sizeof(struct sockaddr);
                                result = recvfrom(sd,recvbuf,1600,0,(struct sockaddr *)&from,&fromlen);
                                if (result > 0)
                                {
                                        to.sin_family      = AF_INET;
                                        //to.sin_addr.s_addr = htonl(0xc0a80144);
                                        to.sin_addr.s_addr = sendip;//htonl(sendip);
                                        to.sin_port        = sendport;//htons(sendport);
                                        //result = 384;
                                        result = sendto(sd,recvbuf,result,0,(struct sockaddr *)&to,sizeof(to));
                                        if (result != 384)
                                        {
                                                printf("phone auido sendto,error=%x\n",errno);
                                                WRT_DEBUG("发送手机音频数据到地址0x%x:%d错误0x%x",sendip,ntohs(sendport),errno);
                                        }
                                        // printf("sendto ip =%x,port  = %d len = %d",sendip,sendport,result);

                                }
                        }
                }
                if(err < 0)
                        break;
                WRT_MESSAGE("停止接收手机音频数据");
        }
        
        closesocket(sd);
        ZENFREE(recvbuf);
        g_phone_audio_msg_queue.release();
        SDL_DestroySemaphore(g_phone_audio_msg_sem);
        g_phone_audio_msg_sem = NULL;

       return 0;
}


int start_talkto_phone(unsigned long ip,unsigned long port)
{
        unsigned long t_msg[4] = {0};
        t_msg[0] = AUDIOSTART;
        t_msg[1] = ip;
        t_msg[2] = port;
        t_msg[3] = 0;
        return g_ao_msg_queue.send_message(MODULE_MSG,(void*)t_msg,sizeof(t_msg),g_ao_msg_sem);
}

int start_phone_audio(unsigned long ip ,unsigned long port)
{
        unsigned long t_msg[4]={0};
        t_msg[0] = AUDIOSTART;
        t_msg[1] = ip;
        t_msg[2] = port;
        return g_phone_audio_msg_queue.send_message(MODULE_MSG,(void*)t_msg,sizeof(t_msg),g_phone_audio_msg_sem);
        //return q_send(g_phone_qid,t_msg);
}
int stop_phone_audio()
{
        unsigned long t_msg[4]={0};
        t_msg[0] = AUDIOSTOP;
        t_msg[1] = 0;
        t_msg[2] = 0;
        return g_phone_audio_msg_queue.send_message(MODULE_MSG,(void*)t_msg,sizeof(t_msg),g_phone_audio_msg_sem);
}

int start_phone_video(unsigned long ip,unsigned long port)
{
        unsigned long t_msg[4] = {0};
        t_msg[0] = CAPTURE_PHOTO;
        t_msg[1] = 3;
        t_msg[2] = ip;
        t_msg[3] = port;
        int ret = g_phone_video_msg_queue.send_message(MODULE_MSG,(void*)t_msg,sizeof(t_msg),g_phone_video_msg_sem);

         WRT_DEBUG("start_phone_video 9x%x :%d ret=%d",ip,port,ret);
        return 1;
}
int stop_phone_video()
{
        unsigned long t_msg[4] = {0};
        t_msg[0] = CANCEL_CAPPHOTO;
        t_msg[1] = 3;
        t_msg[2] = 0;
        t_msg[3] = 0;
        WRT_DEBUG("stop_phone_video");
        return  g_phone_video_msg_queue.send_message(MODULE_MSG,(void*)t_msg,sizeof(t_msg),g_phone_video_msg_sem);
}
#endif
                       
