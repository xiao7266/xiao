

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h> 
#include "madplay.h"
#include "httpclient.h"
#include "SDL.h"
#include "SDL_thread.h"
#include "wrt_msg_queue.h"
#include "zenmalloc.h"
#include "bell.h"
#include "wrt_audio.h"
#include "wav_parser.h" 
#include "sndwav_common.h" 

//#define SAVE_FILE 1

#if 0
#define MSG_PLAY        0x01
#define MSG_STOP        0x02
#define MSG_PLAY_DATA   0x03
#define MSG_EXIT        0x04

typedef struct _AUDIOFORMAT{
        int nchannels;
        int samples;
        int freq;
        int bits;
}AUDIOFORMAT;

static SDL_Thread* g_pPlayThread = NULL;
static SDL_sem*   g_pPlaySem = NULL;
static CWRTMsgQueue g_PlayQueue;

int playtask_func(void* p)
{

       int rc = 0;
       while(1)
       {
               rc = SDL_SemWait(g_pPlaySem);
               if(rc == 0)
               {
                       CWRTMsg* pMsg = g_PlayQueue.get_message();
                       
                       if(pMsg && (pMsg->get_value() == MSG_PLAY))
                       {
                                AUDIOFORMAT* af = NULL;
                                unsigned int len = sizeof(af);
                                
                                af =(AUDIOFORMAT*)(pMsg->get_message(len));
                                if(af == NULL)
                                {
                                        pMsg->free_message();
                                        ZENFREE(pMsg);
                                        pMsg = NULL;
                                        continue;
                                }
                                
                               //OpenSnd(AUDIODSP1);
                              // SetFormat(AUDIODSP1,af->bits,af->freq,af->nchannels);
                               while(1)
                               {
                                        rc = SDL_SemWait(g_pPlaySem);
                                        if(rc == 0)
                                        {
                                                CWRTMsg* pMsg2 = g_PlayQueue.get_message();
                                                if(pMsg2 && (pMsg2->get_value() == MSG_PLAY_DATA))
                                                {
                                                        unsigned int datalen = 0;
                                                        int iplayedlen = 0;
                                                        unsigned char* data = (unsigned char*)pMsg2->get_message(datalen);
                                                        if(data != NULL && datalen != 0){
                                                               
                                                                       // PlayAudio((char*)(data),datalen);

                                                        }
                                                        pMsg2->free_message();
                                                        ZENFREE(pMsg2);
                                                     //   SDL_Delay(af->samples*1000/af->freq);
                                                        continue;
                                                }else if(pMsg2 && (pMsg2->get_value() == MSG_STOP)){
                                                        pMsg2->free_message();
                                                        ZENFREE(pMsg2);
                                                        break;
                                                }
                                                
                                        }
                               }
                               pMsg->free_message();
                               ZENFREE(pMsg);
                               pMsg = NULL;
                              // CloseSnd(AUDIODSP1);
                               continue;
                                
                       }else if(pMsg != NULL && (pMsg->get_value() == MSG_EXIT) )
                       {
                               pMsg->free_message();
                               ZENFREE(pMsg);
                               pMsg = NULL;
                               break;
                       }else if(pMsg != NULL)
                       {
                               pMsg->free_message();
                               ZENFREE(pMsg);
                               pMsg = NULL;
                               continue;
                       }
               }

       }
       SDL_DestroySemaphore(g_pPlaySem);
       g_pPlaySem = NULL;
       return 0;
}

int CreatePlayTask()
{
    g_pPlaySem = SDL_CreateSemaphore(0);
    g_pPlayThread = SDL_CreateThread(playtask_func,NULL);
}

int DestoryPlayTask()
{
   g_PlayQueue.send_message(MSG_EXIT,g_pPlaySem);
   SDL_Delay(1000);
   SDL_WaitThread(g_pPlayThread, NULL);
   if(g_pPlaySem != NULL)
         SDL_DestroySemaphore(g_pPlaySem);
   g_pPlaySem = NULL;
   g_pPlayThread = NULL;
   while(1){
        CWRTMsg* pMsg = g_PlayQueue.get_message();
        if(pMsg == NULL)
                break;
        pMsg->free_message();
        ZENFREE(pMsg);
        pMsg = NULL;
   }
}

void StartPlay(int nch,int freq,int nsample)
{
        AUDIOFORMAT af;
        af.bits = 16;
        af.freq = freq;
        af.nchannels = nch;
        af.samples = nsample;
        g_PlayQueue.send_message(MSG_PLAY,&af,sizeof(af),g_pPlaySem);
}

void StopPlay()
{
   g_PlayQueue.send_message(MSG_STOP,g_pPlaySem);
}

void PutAudioData(unsigned char* data,int len)
{
        g_PlayQueue.send_message(MSG_PLAY_DATA,data,len,g_pPlaySem);
}
#endif

int main(int argc,char**argv)
{
        int	    Status=0;
        int     size;
        unsigned char *inbuf = (unsigned char *)ZENMALLOC(4096);
        unsigned char *outbuf = (unsigned char *)ZENMALLOC(40960);
  
        unsigned int framesize=0;
        unsigned char *mp3buf=0;
        unsigned char    *buf=0;
        int     mp3totallen=0;
        int     BufSize=0;
        int     freq=0;
        int     stereo=0;
        int     layer=0;
        int     first_head=0;
        int     m_AudioOutInstatce;
        int     samples=0;
        int     version=0;
        int     loop  = 0;
        int     icopy = 0; //已经从mp3buf里面COPY出来的数据长度
        int     iunprocess = 0;  //inbuf里面未处理的数据的长度
        unsigned long RMsgBuff[4];
        int     replay = 0;
        int     isbreak = 0;
        int     fd = 0;
        int ws = 0;
        int total = 0;
        unsigned long     nTime = 180000; //180秒
        int count = 0;
        void* phandle = NULL;
#ifdef SAVE_FILE        
        int file = -1;       
        WAVContainer_t wav; 
#endif 
        SDL_Init(SDL_INIT_TIMER);

        if(argc < 2)
        {
                mp3buf = doorbell;
                mp3totallen = sizeof(doorbell);

        }else{

              

                fd  = HttpClientOpen(argv[1]);
                if(fd < 0)
                        return 0;
                mp3totallen = HttpClientGetContentLength(fd);
                mp3buf = (unsigned char *)ZENMALLOC(mp3totallen);
                if(mp3buf == NULL)
                {
                        HttpClientClose(fd);
                        printf("malloc failed \n");
                        return 0;
                }
                do
                {
                        if ((ws  = HttpClientRead(fd,(char*)(mp3buf+total),mp3totallen-total))>0){
                                total+=ws;
                        }

                }while(ws > 0);
                HttpClientClose(fd);
                fd = -1;
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


        /*
        * 初始化音频输出模块,输入参数分别为通道数，频率，数据位数，输出块大小
        * MP3的帧长是1152
        */

        samples = version==0 ? 1152:576;

       // CreatePlayTask();

        //OpenSnd(AUDIODSP1);

        //SetFormat(AUDIODSP1,FMT16BITS,freq,);
     //    StartPlay((stereo==1?2:1),freq,samples);
        phandle = OpenAudio(TYPE_PLAY,(stereo==1?2:1),freq,16,samples*2*(stereo==1?2:1));
        printf("phandle = %x \n",phandle);
#ifdef SAVE_FILE        
        remove("/mnt/sda/1.wav"); 	
        file = open("/mnt/sda/1.wav",O_WRONLY | O_CREAT, 0644);
        memset(&wav,0,sizeof(WAVContainer_t));
        wav.format.channels = (stereo==1?2:1);
        wav.format.sample_rate = freq;
        wav.format.sample_length = 16;	
        	
        SNDWAV_PrepareWAVParams(&wav,8);
        WAV_WriteHeader(file, &wav);
#endif        
        

        printf("mp3totallen = %d ,freq=%d,stereo=%d,layer=%d,version=%d,framesize=%d\n",mp3totallen,freq,stereo,layer,version,framesize);
       
        if(size < 4096){
                icopy  =  size-first_head; //已经COPY的数据
                memcpy(inbuf,mp3buf+first_head,icopy); //将数据重新移动到inbuf起始位置
                iunprocess = icopy; //当前inbuf中未处理数据的长度
        }else{
                icopy  = size;
                memcpy(inbuf,mp3buf+first_head,icopy);//将数据去掉head后，重新移动到inbuf的起始位置
                iunprocess = icopy;//当前inbuf中未处理数据的长度
        }

        /*
        * 获取MP3数据，循环播放
        */
        while(1)
        {

                int len2 = 0;
                int playlen;
                /*
                * 获取Mp3数据，让缓冲区数据保持4096字节
                */

                if(Status != 0){ //表示inbuf里面的数据已经解码完毕，需要重新COPY数据到inbuf里面
                        int itmp1 = iunprocess-Status; //剩下的数据  ，Status 表示size里面已经解码的数据长度
                        int itmp2 = mp3totallen-first_head-icopy; //剩下未COPY得数据
                        memcpy(inbuf,inbuf-Status,itmp1);
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
                                                printf("开始循环播放\n");

                                        }else{
                                                printf("数据播放完毕\n");
                                                break;
                                        }
                                }
                        }

                }



                /*
                * 对缓冲区数据进行解码，返回值为-1，则解码出错；返回值>0为消耗的缓冲区长度；
                * 返回值=0，则数据未使用.
                */
                Status=MpegAudioDecoder(outbuf,inbuf,iunprocess);
                if (Status == -1)
                {
                        printf("解码错误\n");
                        break;
                }
//                printf("Status = %d \n",Status);

                //memcpy(buf,outbuf,samples*2*(stereo==1?2:1));

                /*
                * 播放解码出来的数据
                */

                playlen = samples*2*(stereo==1?2:1);
 
                PlayAudio((unsigned char*)outbuf,playlen);
                //usleep(((samples*1000/freq)*1000)/2);
               // SDL_Delay(12);
#ifdef SAVE_FILE                
                write(file, outbuf, playlen);
#endif                
		  //usleep(((samples*1000/freq)*1000)/2);
                
//                PutAudioData(outbuf,playlen);                     	
//                usleep(((samples*1000/freq)*1000)/2);
        }

        /*
        * MP3播放模块退出
        */

        MpegAudioClose();
       // SDL_Delay(500);
        /*
        * 退出音频输
	* 出模块
        */
       // CloseSnd(AUDIODSP1);
       StopAudio(TYPE_PLAY);
       // StopPlay();
       // DestoryPlayTask();
        if(argc > 1)
                ZENFREE(mp3buf);
        ZENFREE(inbuf);
        ZENFREE(outbuf);
	zenMemUninit();
#ifdef SAVE_FILE  	
        close(file);
#endif        
        SDL_Quit();
        return 0;

}