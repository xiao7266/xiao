#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "SDL.h"
#include "wrt_log.h"
#include "zenmalloc.h"
#include "wrt_audio.h"
#include "mi_ao.h"
#include "mi_ai.h"

static MI_AUDIO_DEV g_AoDevId = 0;
static MI_AO_CHN g_AoChn = 0;

static MI_AUDIO_DEV g_AiDevId = 0;
static MI_AI_CHN g_AiChn = 0;

static int Sstar_SampleRate2Enum(unsigned int samples_rate)
{
    switch (samples_rate)
    {
        case 8000:
            return E_MI_AUDIO_SAMPLE_RATE_8000;
        case 11025:
            return E_MI_AUDIO_SAMPLE_RATE_11025;
        case 16000:
            return E_MI_AUDIO_SAMPLE_RATE_16000;
        case 22050:
            return E_MI_AUDIO_SAMPLE_RATE_22050;
        case 32000:
            return E_MI_AUDIO_SAMPLE_RATE_32000;
        case 44100:
            return E_MI_AUDIO_SAMPLE_RATE_44100;
        case 48000:
            return E_MI_AUDIO_SAMPLE_RATE_48000;
        default:
            printf("Error: Unsupported samplerate(%d)...\n");
            return -1;
    }
}

static FILE *fpsave = NULL;
int OpenAudio(int type,unsigned int channel,unsigned int samples_rate,unsigned int bits,int framesize)
{
    MI_S32 s32Ret = MI_SUCCESS;

    //Ao
    MI_AUDIO_Attr_t stAoSetAttr;
    MI_SYS_ChnPort_t stAoChn0OutputPort0;
    if (TYPE_PLAY == type)
    {
        //set ao attr
        memset(&stAoSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
        stAoSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
        stAoSetAttr.eSamplerate = Sstar_SampleRate2Enum(samples_rate);
        if (1 == channel)
        {
            stAoSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
            stAoSetAttr.u32ChnCnt = 1;
        }
        else
        {
            stAoSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
            stAoSetAttr.u32ChnCnt = 2;
        }
        stAoSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
        
        stAoSetAttr.u32FrmNum = 6;
        stAoSetAttr.u32PtNumPerFrm = 2048;

        //set ao output port depth
        memset(&stAoChn0OutputPort0, 0, sizeof(MI_SYS_ChnPort_t));
        stAoChn0OutputPort0.eModId = E_MI_MODULE_ID_AO;
        stAoChn0OutputPort0.u32DevId = g_AoDevId;
        stAoChn0OutputPort0.u32ChnId = g_AoChn;
        stAoChn0OutputPort0.u32PortId = 0;
        s32Ret = MI_AO_SetPubAttr(g_AoDevId, &stAoSetAttr);
        s32Ret |= MI_AO_Enable(g_AoDevId);
        s32Ret |= MI_AO_EnableChn(g_AoDevId, g_AoChn);
        if (MI_SUCCESS != s32Ret)
        {
            WRT_ERROR("Stop audio fail! ret=0x%x\n", s32Ret);
            return -1;
        }
    }
    else if (TYPE_CAPTURE == type)
    {
        MI_S32 s32Ret = MI_SUCCESS, i;
        //Ai
        MI_AUDIO_DEV AiDevId = g_AiDevId;//1
        MI_AI_CHN AiChn = g_AiChn;
        MI_AUDIO_Attr_t stAiSetAttr;
        MI_SYS_ChnPort_t stAiChn0OutputPort0;
        MI_AI_VqeConfig_t stAiVqeConfig;
        MI_U32 u32AecSupfreq[6] = {20,40,60,80,100,120};
        MI_U32 u32AecSupIntensity[7] = {4,4,4,4,6,6,6};
        MI_S16 s16CompressionRatioInput[5] = {-70, -60, -30, 0, 0};
        MI_S16 s16CompressionRatioOutput[5] = {-70, -45, -18, 0, 0};

        //set ai attr
        memset(&stAiSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
        stAiSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
        stAiSetAttr.eSamplerate = Sstar_SampleRate2Enum(samples_rate);;
        stAiSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
        if (1 == channel)
        {
            stAiSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
            stAiSetAttr.u32ChnCnt = 1;
        }
        else
        {
            stAiSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
            stAiSetAttr.u32ChnCnt = 1;
        }
        stAiSetAttr.u32FrmNum = 16;
        stAiSetAttr.u32PtNumPerFrm = 384;

        //set ai output port depth
        memset(&stAiChn0OutputPort0, 0, sizeof(MI_SYS_ChnPort_t));
        stAiChn0OutputPort0.eModId = E_MI_MODULE_ID_AI;
        stAiChn0OutputPort0.u32DevId = AiDevId;
        stAiChn0OutputPort0.u32ChnId = AiChn;
        stAiChn0OutputPort0.u32PortId = 0;

        //ai vqe
        memset(&stAiVqeConfig, 0, sizeof(MI_AI_VqeConfig_t));
        stAiVqeConfig.bHpfOpen = FALSE;
        stAiVqeConfig.bAnrOpen = FALSE;
        stAiVqeConfig.bAgcOpen = TRUE;
        stAiVqeConfig.bEqOpen = FALSE;
        stAiVqeConfig.bAecOpen = TRUE;

        stAiVqeConfig.stAecCfg.bComfortNoiseEnable = FALSE;
        memcpy(stAiVqeConfig.stAecCfg.u32AecSupfreq, u32AecSupfreq, sizeof(u32AecSupfreq));
        memcpy(stAiVqeConfig.stAecCfg.u32AecSupIntensity,  u32AecSupIntensity, sizeof(u32AecSupIntensity));

        stAiVqeConfig.s32FrameSample = 128;
        stAiVqeConfig.s32WorkSampleRate = E_MI_AUDIO_SAMPLE_RATE_16000;

        //Hpf
        stAiVqeConfig.stHpfCfg.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER;
        stAiVqeConfig.stHpfCfg.eHpfFreq = E_MI_AUDIO_HPF_FREQ_120;

        //Anr
        stAiVqeConfig.stAnrCfg.eMode= E_MI_AUDIO_ALGORITHM_MODE_USER;
        stAiVqeConfig.stAnrCfg.eNrSpeed = E_MI_AUDIO_NR_SPEED_LOW;
        stAiVqeConfig.stAnrCfg.u32NrIntensity = 5;            //[0, 30]
        stAiVqeConfig.stAnrCfg.u32NrSmoothLevel = 10;          //[0, 10]

        //Agc
        stAiVqeConfig.stAgcCfg.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER;
        stAiVqeConfig.stAgcCfg.s32NoiseGateDb = -50;           //[-80, 0], NoiseGateDb disable when value = -80
        stAiVqeConfig.stAgcCfg.s32TargetLevelDb =   0;       //[-80, 0]
        stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainInit = 1;  //[-20, 30]
        stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMax =  15; //[0, 30]
        stAiVqeConfig.stAgcCfg.stAgcGainInfo.s32GainMin = -5; //[-20, 30]
        stAiVqeConfig.stAgcCfg.u32AttackTime = 1;              //[1, 20]
        memcpy(stAiVqeConfig.stAgcCfg.s16Compression_ratio_input, s16CompressionRatioInput, sizeof(s16CompressionRatioInput));
        memcpy(stAiVqeConfig.stAgcCfg.s16Compression_ratio_output, s16CompressionRatioOutput, sizeof(s16CompressionRatioOutput));
        stAiVqeConfig.stAgcCfg.u32DropGainMax = 60;            //[0, 60]
        stAiVqeConfig.stAgcCfg.u32NoiseGateAttenuationDb = 10;  //[0, 100]
        stAiVqeConfig.stAgcCfg.u32ReleaseTime = 10;             //[1, 20]

        //Eq
        stAiVqeConfig.stEqCfg.eMode = E_MI_AUDIO_ALGORITHM_MODE_USER;
        for (i = 0; i < sizeof(stAiVqeConfig.stEqCfg.s16EqGainDb) / sizeof(stAiVqeConfig.stEqCfg.s16EqGainDb[0]); i++)
        {
           stAiVqeConfig.stEqCfg.s16EqGainDb[i] = 5;
        }

        stAiVqeConfig.u32ChnNum = 1;

        MI_AI_SetPubAttr(AiDevId, &stAiSetAttr);
        MI_AI_Enable(AiDevId);
        MI_AI_EnableChn(AiDevId, AiChn);

        MI_AI_SetVqeVolume(AiDevId, 0, 9);

        s32Ret = MI_AI_SetVqeAttr(AiDevId, AiChn, 0, 0, &stAiVqeConfig);
        if (s32Ret != MI_SUCCESS)
        {
            WRT_ERROR("%#x\n", s32Ret);
        }
        MI_AI_EnableVqe(AiDevId, AiChn);

        MI_SYS_SetChnOutputPortDepth(&stAiChn0OutputPort0,4,8);
    }
    if (fpsave == NULL)
    {
        fpsave = fopen("test.pcm", "w+");
        printf("open audio\n");
    }
    return 0;
}

void PlayAudio(unsigned char* buffer, int len)
{
    MI_AUDIO_Frame_t stAudioFrame;

    MI_S32 s32Ret = 0;

    if ((NULL != buffer) && (len > 0))
    {
        buffer[len] = '\0';
        stAudioFrame.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
        stAudioFrame.apVirAddr[0] = buffer;
        stAudioFrame.u32Len = len;
        s32Ret = MI_AO_SendFrame(g_AoDevId, g_AoChn, &stAudioFrame, -1);
        //printf("send audio data to ao device, len = %d\n", len);
    }
    else
    {
        printf("Audio Error: Play audio buffer is NULL or len <= 0\n");
    }
}

void StopAudio(int type)
{
    MI_AO_ChnState_t stChnState;
    MI_S32 s32Ret = -1;
    if (TYPE_PLAY == type)
    {
        playretry:
        s32Ret = MI_AO_QueryChnStat(g_AoDevId, g_AoChn, &stChnState);
        if (stChnState.u32ChnBusyNum < 2048 * 2)
        {
            s32Ret = MI_AO_DisableChn(g_AoDevId, g_AoChn);
            s32Ret |= MI_AO_Disable(g_AoDevId);
            if (MI_SUCCESS != s32Ret)
            {
                WRT_ERROR("Stop audio fail! ret=0x%x\n", s32Ret);
                return;
            }
            printf("stop audio\n");
        }
        else
        {
            usleep(10*1000);
            goto playretry;
        }
    }
    else if (TYPE_CAPTURE == type)
    {
        MI_AUDIO_DEV AiDevId = g_AiDevId;
        MI_AI_CHN AiChn = g_AiChn;

        MI_AI_DisableVqe(AiDevId, AiChn);
        MI_AI_DisableChn(AiDevId, AiChn);
        MI_AI_Disable(AiDevId);
    }
    //if (fpsave != NULL)
    //{
    //    fclose(fpsave);
    //    fpsave = NULL;
    //    printf("close audio file\n");
    //}
}

int GetAudio(unsigned char* buf, int *len)
{
    MI_AUDIO_DEV AiDevId = 0;
    MI_AI_CHN AiChn = 0;
    MI_AUDIO_Frame_t stAudioFrame;
    MI_AUDIO_AecFrame_t stAecFrm;
    if (MI_SUCCESS == MI_AI_GetFrame(AiDevId, AiChn, &stAudioFrame, &stAecFrm, 128))//1024 / 8000 = 128ms
    {
        if (0 == stAudioFrame.u32Len)
        {
            usleep(10 * 1000); //try again
            //continue;
        }
        memcpy(buf, stAudioFrame.apVirAddr[0], stAudioFrame.u32Len);
        //if (fpsave)
        //{
        //    fwrite(buf, 1, stAudioFrame.u32Len, fpsave);
        //}
        *len = stAudioFrame.u32Len;
        MI_AI_ReleaseFrame(AiDevId,  AiChn, &stAudioFrame, NULL);
    }

    return *len;
}

void SetAlsaVolume(int type,int level)
{
}

int  GetAlsaVolume(int type)
{
    return 0;
}

void open_alsa_dev()
{
}
