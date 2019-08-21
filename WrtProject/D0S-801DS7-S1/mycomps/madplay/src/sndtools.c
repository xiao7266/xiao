#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>       //sem_t
#include <dirent.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>

#include <linux/soundcard.h>

#define VAR_STATIC
#define MIXER_DEVICE	"/dev/mixer"


#include "sndtools.h"

#define _MODIFY_MIXER  //20100211

//int devrecfd = 0;
//int devplayfd = 0;
//int mixer_fd = 0;
//int SpkVolume[5] = {10, 25, 40, 55, 65};
int SpkVolume[6] = {0,60, 70, 80, 90, 100};
int MicVolume[6] = {0,60, 70, 80, 90, 100};
void SoundSetSpeakerVolume(unsigned char volume);
void SoundSetMicVolume(unsigned char volume);
void SoundSetSpeakerMute(void);
#ifdef _MODIFY_MIXER
void mixer_init(void);
#endif


// 等待回放结束
static void SyncPlay(void);

static int abuf_size= 0;

static int devrecfd = 0;
static int devplayfd = 0;
static int mixer_fd = 0;
//---------------------------------------------------------------------------
/*
* Open Sound device
* Return 1 if success, else return 0.
*/
int OpenSnd(/* add by new version */int nWhich)
{
        int status;   // 系统调用的返回值
        int setting;
        if(nWhich == 1)
        {
                printf("open devrecfd = %d\n", devrecfd);
                if(devrecfd > 0)
                {
                        close(devrecfd);
                }
                devrecfd = 0;
                if(devrecfd == 0)
                {
                        devrecfd = open ("/dev/dsp", O_RDONLY);//, 0);//open("/dev/dsp", O_RDWR);
                        if(devrecfd < 0)
                        {
                                devrecfd = 0;
                                return 0;
                        }
                        setting = 0x0010000A;//0x00040009;//0x0008000B;//0x0002000C;//0x0004000A;//0x0008000B;//0x00040009;

                        status = ioctl(devrecfd, SNDCTL_DSP_SETFRAGMENT, (char *)&setting);
                        if (status == -1) {
                                perror("ioctl buffer size");
                                return -1;
                        }
                }
                /*    // Open Mixer device
                if((mixer_fd = open(MIXER_DEVICE, O_RDONLY, 0)) != -1)
                {
                int enable = 1;
                ioctl(mixer_fd, MIXER_WRITE(SOUND_ONOFF_MIN), &enable);
                int enablemute = 1;
                ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_SPEAKER), &enablemute);
                }
                else
                {
                printf("mix_fd open fail\n");
                }   */   
        }
        else
        {
                printf("open devplayfd = %d\n", devplayfd);
                if(devplayfd > 0)
                {
                        close(devplayfd);
                }
                devplayfd = 0;
                if(devplayfd == 0)
                {
                        devplayfd = open ("/dev/dsp1", O_WRONLY);//, 0);//open("/dev/dsp", O_RDWR);
                        if(devplayfd < 0)
                        {
                                printf("打开错误\n");
                                devplayfd = 0;
                                return 0;
                        }

                        setting =0x0010000A;// 0x0002000C;//0x00040009;//0x0008000B;//0x0002000C;//0x0004000A;//0x0008000B;//0x00040009;

                        status = ioctl(devplayfd, SNDCTL_DSP_SETFRAGMENT, (char *)&setting);
                        if (status == -1) {
                                perror("ioctl buffer size");
                                return -1;
                        }
                }
#ifdef _MODIFY_MIXER
                mixer_init();
#else
                if(mixer_fd > 0)
                {
                        close(mixer_fd);
                        mixer_fd = 0;
                }
                /* Open Mixer device */
                if((mixer_fd = open(MIXER_DEVICE, O_RDONLY, 0)) != -1)     // O_RDONLY    O_WRONLY
                {
                        int enable = 1;
                        ioctl(mixer_fd, MIXER_WRITE(SOUND_ONOFF_MIN), &enable);
                        int enablemute = 1;
                        ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_SPEAKER), &enablemute);
                }
                else
                {
                        printf("mix_fd open fail\n");
                }
#endif 
                SoundSetSpeakerVolume(3); //设置音量
                SoundSetMicVolume(3);//LocalCfg.SpkVolume); //设置MIC音量     
        }

        return 1;
}
//---------------------------------------------------------------------------
#ifdef _MODIFY_MIXER
void mixer_init(void)
{
#if 1
        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }
        /* Open Mixer device */
        if ((mixer_fd = open(MIXER_DEVICE, O_RDONLY)) > 0)
        {
                //int enable = 1;
                //ioctl(mixer_fd, MIXER_WRITE(SOUND_ONOFF_MIN), &enable);
                int enablemute = 1;
                ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_PCM), &enablemute);      //SOUND_MIXER_SPEAKER
                enablemute = 1;
                ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_MIC), &enablemute);
        }
        else
        {
                printf("mix_fd open fail\n");
        }
        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }
#endif
}
#endif
//---------------------------------------------------------------------------
void SoundSetSpeakerVolume(unsigned char volume)
{
#if 1
        int vol, sys_vol, devs;
        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }
        if(volume > 4)
                printf("SoundSetSpeakerVolume  volume = %d\n", volume);
        /* Open Mixer device */
        if ((mixer_fd = open(MIXER_DEVICE, O_RDONLY)) > 0)
        {
                ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &devs);
                printf("SoundSetSpeakerVolume devs = 0x%X, SOUND_MIXER_PCM = 0x%X, SOUND_ONOFF_MIN = 0x%X, SOUND_MIXER_SPEAKER = 0x%X\n",
                        devs, SOUND_MIXER_PCM, SOUND_ONOFF_MIN, SOUND_MIXER_SPEAKER);
                if (devs & (1 << SOUND_MIXER_PCM))
                {
                        sys_vol = SpkVolume[volume];
                        vol = (((unsigned short)sys_vol) << 8) | sys_vol;
                        printf("write speaker vol = %d\n", vol);
                        ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_PCM), &vol);

                        //ioctl(mixer_fd, MIXER_READ(SOUND_MIXER_PCM), &vol);
                        //printf("read speaker vol = %d\n", vol);
                }
        }
        else
                printf("mixer_fd = %d\n", mixer_fd);
        if(mixer_fd > 0)
        {
                close(mixer_fd);
               mixer_fd = 0;
        }
#endif
}
//---------------------------------------------------------------------------
void SoundSetMicVolume(unsigned char volume)
{
#if 1
        int vol, sys_vol, devs;

        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }
        printf("SoundSetMicVolume  volume = %d\n", volume);
        /* Open Mixer device */
        if ((mixer_fd = open(MIXER_DEVICE, O_RDONLY)) > 0)
        {
                //  if(mixer_fd > 0)
                //   {
                ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &devs);
                if (devs & (1 << SOUND_MIXER_MIC))                       // SOUND_MIXER_MIC
                {		
                        sys_vol = MicVolume[volume];
                        vol = (((unsigned short)sys_vol) << 8) | sys_vol;
                        printf("write mic vol = %d, SOUND_MIXER_MIC = 0x%X\n", vol, SOUND_MIXER_MIC);
                        ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_MIC), &vol);

                        ioctl(mixer_fd, MIXER_READ(SOUND_MIXER_MIC), &vol);
                        printf("read mic vol = %d\n", vol);
                }
        }
        else
                printf("mixer_fd = %d\n", mixer_fd);
        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }
#endif
}
//---------------------------------------------------------------------------
void SoundSetSpeakerMute(void)
{
#if 1
        int vol, sys_vol, devs;
        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }  
        /* Open Mixer device */
        if ((mixer_fd = open(MIXER_DEVICE, O_RDONLY)) > 0)
        {
                //  if(mixer_fd > 0)
                //   {
                ioctl(mixer_fd, SOUND_MIXER_READ_DEVMASK, &devs); 
                if (devs & (1 << SOUND_MIXER_PCM))
                {
                        sys_vol = 0;
                        vol = (((unsigned short)sys_vol) << 8) | sys_vol;
                        printf("write speaker vol = %d\n", vol);
                        ioctl(mixer_fd, MIXER_WRITE(SOUND_MIXER_PCM), &vol);

                        //ioctl(mixer_fd, MIXER_READ(SOUND_MIXER_PCM), &vol);
                        //printf("read speaker vol = %d\n", vol);
                }
        }
        else
                printf("mixer_fd = %d\n", mixer_fd);
        if(mixer_fd > 0)
        {
                close(mixer_fd);
                mixer_fd = 0;
        }
#endif
}
//---------------------------------------------------------------------------
/*
* Close Sound device
* return 1 if success, else return 0.
*/
int CloseSnd(/* add by new version */int nWhich)
{
        int status;
        if(nWhich == 1)
        {
                printf("close devrecfd = %d\n", devrecfd);
                if(devrecfd > 0)
                        close(devrecfd);
                devrecfd = 0;
                printf("devrecfd closed\n");
        }
        else
        {
                printf("close devplayfd = %d\n", devplayfd);
                // 等待回放结束
                SyncPlay();
                if(devplayfd > 0)
                        close(devplayfd);
                devplayfd = 0;

                printf("devplayfd closed\n");

#ifndef _MODIFY_MIXER
                if (mixer_fd > 0)
                        close(mixer_fd);
                mixer_fd = 0;

                printf("mixer_fd closed\n");
#endif
        }
        return 1;
}

//---------------------------------------------------------------------------
int SoundUmount(void)
{
        devrecfd = 0;
        devplayfd = 0;
        mixer_fd = 0;
}
//---------------------------------------------------------------------------
/*
* Set Record an Playback format
* return 1 if success, else return 0.
* bits -- FMT8BITS(8bits), FMT16BITS(16bits)
* hz -- FMT8K(8000HZ), FMT16K(16000HZ), FMT22K(22000HZ), FMT44K(44000HZ)
chn -- MONO 1 STERO 2
*/
int SetFormat(int nWhich, int bits, int hz, int chn)
{
        int samplesize;
        int tmp;
        int dsp_stereo;
        int setting;

        //  int fd;	// 声音设备的文件描述符
        int arg;	// 用于ioctl调用的参数
        int status;   // 系统调用的返回值
        int devfd;
        if(nWhich == AUDIORECORDDEVICE)
                devfd = devrecfd;
        else
                devfd = devplayfd;

        if(devfd <= 0)
        {
                printf("devfd small than 0\n");
                return -1;
        }

        //  if(nWhich == AUDIODSP)
        //   {
        //    printf("SetFormat 1\n");
        //    usleep(1000*1000);
        // 设置采样时的量化位数       FIC8120只支持16位
        status = ioctl(devfd, SOUND_PCM_READ_BITS, &arg);
        if(arg != bits)
        {
                arg = bits;
                status = ioctl(devfd, SOUND_PCM_WRITE_BITS, &arg);
                if (status == -1)
                {
                        printf("SOUND_PCM_WRITE_BITS ioctl failed\n");
                        //ResetMachine();
                }
                if (arg != bits)
                        printf("unable to set sample size\n");
        }
        //    printf("SetFormat 2\n");
        //    usleep(1000*1000);
        // 设置采样时的声道数目
        status = ioctl(devfd, SOUND_PCM_READ_CHANNELS, &arg);
        if(arg != chn)
        {
                arg = chn;
                status = ioctl(devfd, SOUND_PCM_WRITE_CHANNELS, &arg);
                if (status == -1)
                {
                        printf("SOUND_PCM_WRITE_CHANNELS ioctl failed");
                }
                if (arg != chn)
                        printf("unable to set number of channels");
        }
        //printf("SetFormat 3\n");
        // 设置采样时的采样频率
        status = ioctl(devfd, SOUND_PCM_READ_RATE, &arg);
        if(arg != hz)
        {
                arg = hz;
                status = ioctl(devfd, SOUND_PCM_WRITE_RATE, &arg);
                if (status == -1)
                {
                        printf("SOUND_PCM_WRITE_RATE ioctl failed");
                }
        }
         abuf_size = AUIDOBLKSIZE;
#if 0
        //printf("SetFormat 4\n");
       

        status = ioctl(devfd, SNDCTL_DSP_SAMPLESIZE, &abuf_size);
        if (status == -1) {
                printf("ioctl buffer size");
                return -1;
        }

#endif

        ioctl(devfd, SNDCTL_DSP_GETBLKSIZE, &abuf_size);

        printf("abuf_size= %d\n",abuf_size);

        if (abuf_size < 4 || abuf_size > 65536)
        {
                //   if (abuf_size == -1)
                printf ( "Invalid audio buffers size %d\n", nWhich);
        }


        return 1;
}

//---------------------------------------------------------------------------
// 等待回放结束
static void SyncPlay(void)
{
        int status;
        if(devplayfd > 0)
        {
                status = ioctl(devplayfd, SOUND_PCM_SYNC, 0);
                if (status == -1)
                        printf("SOUND_PCM_SYNC ioctl failed");
        }
}

//---------------------------------------------------------------------------
/*
* Record
* return numbers of byte for read.
*/
int RecordAudio(char *buf, int size)
{
        int status;
        //  int i;
        status = 0;
        if(devrecfd > 0)
                status=read(devrecfd, buf, size);
        //  for(i=0; i<size; i++)
        //    printf("0x%X, ", buf[i]);
        return status;
}

//---------------------------------------------------------------------------
/* 
* Playback
* return numbers of byte for write.
*/
int PlayAudio(char *buf, int size)
{
        int status;
        status = 0;
        if(devplayfd > 0){
                while(size){
                        status = write(devplayfd, buf, size);
                        if(status == -1){
                                if (errno == EINTR)
                                        continue;
                               else
                                        return -1;
                        }
                        buf += status;
                        size -= status;
                }
        }
        //printf("size = %d\n", size);
        //CheckWbStatus(); //检测 WB IO 状态
        return 0;
}
