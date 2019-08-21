#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "media_negotiate.h"

static MEDIA_DATA local_media_attr;
static NEGOTIATE_RESULT local_media;
static NEGOTIATE_RESULT remote_media;

void Init_Local_MediaInfo()
{
    //本机视频编码能力
    local_media_attr.local_video_encode.v_type = V_NONE;
    local_media_attr.local_video_encode.v_resolution = V_CIF_P;
    local_media_attr.local_video_encode.v_bitrate = 800*1000;
    local_media_attr.local_video_encode.v_frame_rate = 25;
    local_media_attr.local_video_encode.v_port = 20000;
    local_media_attr.local_video_encode.v_res0 = 0;
    local_media_attr.local_video_encode.v_res1 = 0;
    local_media_attr.local_video_encode.v_res2 = 0;
    local_media_attr.local_video_encode.v_res3 = 0;

    //本机视频解码能力
    local_media_attr.local_video_decode.v_type = V_H264;
    local_media_attr.local_video_decode.v_resolution = V_F_D1_P;
    local_media_attr.local_video_decode.v_bitrate = 1*1024*1024;
    local_media_attr.local_video_decode.v_frame_rate = 25;
    local_media_attr.local_video_decode.v_port = 20000;
    local_media_attr.local_video_decode.v_res0 = 0;
    local_media_attr.local_video_decode.v_res1 = 0;
    local_media_attr.local_video_decode.v_res2 = 0;
    local_media_attr.local_video_decode.v_res3 = 0;

    //本机音频编码能力
    local_media_attr.local_audio_encode.a_type = A_G711;
    local_media_attr.local_audio_encode.a_channel = A_CHANNEL_1_2;
    local_media_attr.local_audio_encode.a_sample_bit = A_BIT_16;
    local_media_attr.local_audio_encode.a_sample_rate = A_8K_16K;
    local_media_attr.local_audio_encode.ms_per_pak = 20;	//每个包多少ms的数据,默认20ms
    local_media_attr.local_audio_encode.a_port = 15004;
    local_media_attr.local_audio_encode.a_res0 = 0;
    local_media_attr.local_audio_encode.a_res1 = 0;
    local_media_attr.local_audio_encode.a_res2 = 0;

    //本机音频解码能力
    local_media_attr.local_audio_decode.a_type        = A_G711;
    local_media_attr.local_audio_decode.a_channel     = A_CHANNEL_1_2;
    local_media_attr.local_audio_decode.a_sample_bit  = A_BIT_16;
    local_media_attr.local_audio_decode.a_sample_rate = A_8K_16K;
    local_media_attr.local_audio_decode.ms_per_pak = 20;	//每个包多少ms的数据,默认20ms
    local_media_attr.local_audio_decode.a_port = 15004;
    local_media_attr.local_audio_decode.a_res0 = 0;
    local_media_attr.local_audio_decode.a_res1 = 0;
    local_media_attr.local_audio_decode.a_res2 = 0;
    local_media_attr.platform_type = ROOM_755D;
}

void Get_local_mediainfo(char *buf)
{
    memcpy(buf,&local_media_attr,sizeof(MEDIA_DATA));

    printf("send venc:%d\n",local_media_attr.local_video_encode.v_type);
    printf("send vdec:%d\n",local_media_attr.local_video_decode.v_type);

    printf("send a_enc:%d\n",local_media_attr.local_audio_encode.a_type);
    printf("send a_dec:%d\n",local_media_attr.local_audio_decode.a_type);
}

void Get_negotiate_result_of_remote(char *buf)
{
    memcpy(buf,&remote_media,sizeof(NEGOTIATE_RESULT));
}

void Set_local_mediaInfo(char *buf)
{
    memcpy(&local_media,buf,sizeof(NEGOTIATE_RESULT));
}

void media_negotiate_process(char *buf,int buflen)
{
    int v_enc_type = -1;
    int v_dec_type = -1;
    int local_a_enc_type = -1;
    int local_a_dec_type = -1;
    int remote_a_enc_type = -1;
    int remote_a_dec_type = -1;

    if(buflen < 144)
    {
        memset(&local_media,0,sizeof(NEGOTIATE_RESULT));
        memset(&remote_media,0,sizeof(NEGOTIATE_RESULT));
        return;
    }

    MEDIA_DATA remote_media_type;
    memcpy(&remote_media_type,buf,sizeof(MEDIA_DATA));

    printf("recv venc:%d\n",remote_media_type.local_video_encode.v_type);
    printf("recv vdec:%d\n",remote_media_type.local_video_decode.v_type);

    printf("recv a_enc:%d\n",remote_media_type.local_audio_encode.a_type);
    printf("recv a_dec:%d\n",remote_media_type.local_audio_decode.a_type);

    printf("recv door venc res:%d\n",remote_media_type.local_video_encode.v_resolution);
    printf("recv door vdec res:%d\n",remote_media_type.local_video_decode.v_resolution);
    printf("recv door venc type:%d\n",remote_media_type.local_video_encode.v_type);
    printf("recv door venc type:%d\n",remote_media_type.local_video_decode.v_type);

    local_media.remote_platform_type = remote_media_type.platform_type;
    remote_media.remote_platform_type= local_media_attr.platform_type;

    local_media.a_packtime  = local_media_attr.local_audio_encode.ms_per_pak;
    remote_media.a_packtime = remote_media_type.local_audio_encode.ms_per_pak;

    local_media.v_enc_bitrate  = remote_media_type.local_video_decode.v_bitrate;
    remote_media.v_enc_bitrate = local_media_attr.local_video_decode.v_bitrate;

    local_media.v_recvport  = remote_media_type.local_video_encode.v_port;
    remote_media.v_recvport = local_media_attr.local_video_encode.v_port;

    local_media.a_recvport  = remote_media_type.local_audio_encode.a_port;
    remote_media.a_recvport = local_media_attr.local_audio_encode.a_port;

    local_media.v_sendport  = remote_media_type.local_video_decode.v_port;
    remote_media.v_sendport = local_media_attr.local_video_decode.v_port;

    local_media.a_sendport  = remote_media_type.local_audio_decode.a_port;
    remote_media.a_sendport = local_media_attr.local_audio_decode.a_port;

    v_dec_type = min(local_media_attr.local_video_decode.v_type,remote_media_type.local_video_encode.v_type);
    v_enc_type = min(local_media_attr.local_video_encode.v_type,remote_media_type.local_video_decode.v_type);

    local_media.v_dec_type  = v_dec_type;
    local_media.v_enc_type  = v_enc_type;
    remote_media.v_dec_type = v_enc_type;
    remote_media.v_enc_type = v_dec_type;

    //协商视频编解码分辨率 暂时只判断 V_CIF_P V_VGA V_QVGA V_F_D1_P
    //V_CIF_P V_VGA主要适合我们  V_F_D1_P 预留
    if(local_media_attr.local_video_encode.v_resolution == V_CIF_P)
    {
        remote_media.v_dec_resolution_w = 352;
        remote_media.v_dec_resolution_h = 288;
        local_media.v_enc_resolution_w  = 352;
        local_media.v_enc_resolution_h  = 288;
    }else if(local_media_attr.local_video_encode.v_resolution == V_VGA)
    {
        remote_media.v_dec_resolution_w = 640;
        remote_media.v_dec_resolution_h = 480;
        local_media.v_enc_resolution_w  = 640;
        local_media.v_enc_resolution_h  = 480;
    }else if(local_media_attr.local_video_encode.v_resolution == V_QVGA)
    {
        remote_media.v_dec_resolution_w = 320;
        remote_media.v_dec_resolution_h = 240;
        local_media.v_enc_resolution_w  = 320;
        local_media.v_enc_resolution_h  = 240;
    }else if(local_media_attr.local_video_encode.v_resolution == V_F_D1_P)
    {
        remote_media.v_dec_resolution_w = 720;
        remote_media.v_dec_resolution_h = 576;
        local_media.v_enc_resolution_w  = 720;
        local_media.v_enc_resolution_h  = 576;
    }
    if(remote_media_type.local_video_encode.v_resolution == V_CIF_P)
    {
        local_media.v_dec_resolution_w   = 352;
        local_media.v_dec_resolution_h   = 288;
        remote_media.v_enc_resolution_w  = 352;
        remote_media.v_enc_resolution_h  = 288;
    }else if(remote_media_type.local_video_encode.v_resolution == V_VGA)
    {
        local_media.v_dec_resolution_w   = 640;
        local_media.v_dec_resolution_h   = 480;
        remote_media.v_enc_resolution_w  = 640;
        remote_media.v_enc_resolution_h  = 480;
    }else if(remote_media_type.local_video_encode.v_resolution == V_QVGA)
    {
        local_media.v_dec_resolution_w   = 320;
        local_media.v_dec_resolution_h   = 240;
        remote_media.v_enc_resolution_w  = 320;
        remote_media.v_enc_resolution_h  = 240;
    }else if (remote_media_type.local_video_encode.v_resolution == V_F_D1_P)
    {
        local_media.v_dec_resolution_w   = 720;
        local_media.v_dec_resolution_h   = 576;
        remote_media.v_enc_resolution_w  = 720;
        remote_media.v_enc_resolution_h  = 576;
    }
    else if (remote_media_type.local_video_encode.v_resolution == V_SXGA) //G platform
    {
        local_media.v_dec_resolution_w   = 720;
        local_media.v_dec_resolution_h   = 576;
        remote_media.v_enc_resolution_w  = 720;
        remote_media.v_enc_resolution_h  = 576;
        remote_media.v_dec_type = V_H264;
    }
    else if (remote_media_type.local_video_encode.v_resolution == V_720P) //A3_A4 platform
    {
        local_media.v_dec_resolution_w   = 1280;
        local_media.v_dec_resolution_h   = 720;
        remote_media.v_enc_resolution_w  = 1280;
        remote_media.v_enc_resolution_h  = 720;
        remote_media.v_dec_type = V_H264;
    }
    //协商音频通道数
    if(local_media_attr.local_audio_decode.a_channel == A_CHANNEL_1)
    {
        if(remote_media_type.local_audio_encode.a_channel == A_CHANNEL_1 ||
            remote_media_type.local_audio_encode.a_channel == A_CHANNEL_1_2)
        {
            local_media.a_samplechannel = 1;
            remote_media.a_samplechannel = 1;
        }else
        {
            local_media.a_samplechannel = 0;
            remote_media.a_samplechannel = 0;//协商失败为0
        }
    }
    else if (local_media_attr.local_audio_decode.a_channel == A_CHANNEL_2)
    {
        if (remote_media_type.local_audio_encode.a_channel == A_CHANNEL_2 ||
            remote_media_type.local_audio_encode.a_channel == A_CHANNEL_1_2)
        {
            local_media.a_samplechannel = 2;
            remote_media.a_samplechannel = 2;
        }
        else
        {
            local_media.a_samplechannel  = 0;
            remote_media.a_samplechannel = 0;//协商失败为0
        }
    }
    else if (local_media_attr.local_audio_decode.a_channel == A_CHANNEL_1_2)
    {
        if (remote_media_type.local_audio_encode.a_channel == A_CHANNEL_1_2)
        {
            local_media.a_samplechannel = 1;
            remote_media.a_samplechannel = 1;
        }
        else if (remote_media_type.local_audio_encode.a_channel == A_CHANNEL_2)
        {
            local_media.a_samplechannel = 2;
            remote_media.a_samplechannel = 2;
        }
        else if (remote_media_type.local_audio_encode.a_channel == A_CHANNEL_1)
        {
            local_media.a_samplechannel = 1;
            remote_media.a_samplechannel = 1;
        }
        else
        {
            local_media.a_samplechannel = 0;
            remote_media.a_samplechannel = 0;//协商失败为0
        }
    }

    //协商音频采样率
    if (local_media_attr.local_audio_decode.a_sample_rate == A_8K_16K)
    {
        if (remote_media_type.local_audio_encode.a_sample_rate == A_8K)
        {
            local_media.a_samplerate  = 8000;
            remote_media.a_samplerate = 8000;
        }
        else if (remote_media_type.local_audio_encode.a_sample_rate == A_16K)
        {
            local_media.a_samplerate  = 16000;
            remote_media.a_samplerate = 16000;
        }
        else if (remote_media_type.local_audio_encode.a_sample_rate == A_8K_16K)
        {
            local_media.a_samplerate  = 8000;
            remote_media.a_samplerate = 8000;
        }
        else
        {
            local_media.a_samplerate  = 0;
            remote_media.a_samplerate = 0;
        }
    }
    else if (local_media_attr.local_audio_decode.a_sample_rate == A_8K)
    {
        if (remote_media_type.local_audio_encode.a_sample_rate == A_8K_16K ||
            remote_media_type.local_audio_encode.a_sample_rate == A_8K)
        {
            local_media.a_samplerate  = 8000;
            remote_media.a_samplerate = 8000;
        }
        else
        {
            local_media.a_samplerate  = 0;
            remote_media.a_samplerate = 0;
        }
    }
    else if (local_media_attr.local_audio_decode.a_sample_rate == A_16K)
    {
        if (remote_media_type.local_audio_encode.a_sample_rate == A_8K_16K ||
            remote_media_type.local_audio_encode.a_sample_rate == A_16K)
        {
            local_media.a_samplerate  = 16000;
            remote_media.a_samplerate = 16000;
        }else
        {
            local_media.a_samplerate  = 0;
            remote_media.a_samplerate = 0;
        }
    }
    //协商音频采样位宽
    if (local_media_attr.local_audio_decode.a_sample_bit == A_BIT_8_16)
    {
        if (remote_media_type.local_audio_encode.a_sample_bit == A_BIT_8 ||
            remote_media_type.local_audio_encode.a_sample_bit == A_BIT_8_16)
        {
            local_media.a_samplebit  = 8;
            remote_media.a_samplebit = 8;
        }
        else if (remote_media_type.local_audio_encode.a_sample_bit == A_BIT_16)
        {
            local_media.a_samplebit  = 16;
            remote_media.a_samplebit = 16;
        }
        else
        {
            local_media.a_samplebit  = 0;
            remote_media.a_samplebit = 0;
        }
    }
    else if (local_media_attr.local_audio_decode.a_sample_bit == A_BIT_8)
    {
        if(remote_media_type.local_audio_encode.a_sample_bit == A_BIT_8 ||
            remote_media_type.local_audio_encode.a_sample_bit == A_BIT_8_16)
        {
            local_media.a_samplebit  = 8;
            remote_media.a_samplebit = 8;
        }else
        {
            local_media.a_samplebit  = 0;
            remote_media.a_samplebit = 0;
        }
    }
    else if (local_media_attr.local_audio_decode.a_sample_bit == A_BIT_16)
    {
        if (remote_media_type.local_audio_encode.a_sample_bit == A_BIT_16 ||
            remote_media_type.local_audio_encode.a_sample_bit == A_BIT_8_16)
        {
            local_media.a_samplebit  = 16;
            remote_media.a_samplebit = 16;
        }
        else
        {
            local_media.a_samplebit  = 0;
            remote_media.a_samplebit = 0;
        }
    }
    //协商音频编解码格式
    if (local_media_attr.local_audio_encode.a_type == remote_media_type.local_audio_decode.a_type)
    {
        local_a_enc_type  = local_media_attr.local_audio_encode.a_type;
        remote_a_dec_type = local_media_attr.local_audio_encode.a_type;
    }
    else
    {
        if(local_media_attr.local_audio_encode.a_type == A_NONE)
        {
            local_a_enc_type  = A_NONE;
            remote_a_dec_type = A_NONE;
        }
        else if(local_media_attr.local_audio_encode.a_type == A_G711)
        {
            if(remote_media_type.local_audio_decode.a_type == A_G711_G729)
            {
                local_a_enc_type  = A_G711;
                remote_a_dec_type = A_G711;
            }else if(remote_media_type.local_audio_decode.a_type == A_NONE)
            {
                local_a_enc_type  = A_NONE;
                remote_a_dec_type = A_NONE;
            }else if(remote_media_type.local_audio_decode.a_type == A_G729 ||
                remote_media_type.local_audio_decode.a_type == A_PCM)
            {
                local_a_enc_type  = A_PCM;
                remote_a_dec_type = A_PCM;
            }
        }else if (local_media_attr.local_audio_encode.a_type == A_G729)
        {
            if (remote_media_type.local_audio_decode.a_type == A_G711_G729)
            {
                local_a_enc_type  = A_G729;
                remote_a_dec_type = A_G729;
            }
            else if (remote_media_type.local_audio_decode.a_type == A_NONE)
            {
                local_a_enc_type  = A_NONE;
                remote_a_dec_type = A_NONE;
            }
            else if (remote_media_type.local_audio_decode.a_type == A_G711 ||
                remote_media_type.local_audio_decode.a_type == A_PCM)
            {
                local_a_enc_type  = A_PCM;
                remote_a_dec_type = A_PCM;
            }
        }
        else if (local_media_attr.local_audio_encode.a_type == A_G711_G729)
        {
            if (remote_media_type.local_audio_decode.a_type == A_G729)
            {
                local_a_enc_type  = A_G729;
                remote_a_dec_type = A_G729;
            }
            else if (remote_media_type.local_audio_decode.a_type == A_NONE)
            {
                local_a_enc_type  = A_NONE;
                remote_a_dec_type = A_NONE;
            }
            else if (remote_media_type.local_audio_decode.a_type == A_G711)
            {
                local_a_enc_type  = A_G711;
                remote_a_dec_type = A_G711;
            }
            else if (remote_media_type.local_audio_decode.a_type == A_PCM)
            {
                local_a_enc_type  = A_PCM;
                remote_a_dec_type = A_PCM;
            }
        }
    }

    if (local_media_attr.local_audio_decode.a_type == remote_media_type.local_audio_encode.a_type)
    {
        local_a_dec_type  = local_media_attr.local_audio_decode.a_type;
        remote_a_enc_type = local_media_attr.local_audio_decode.a_type;
    }
    else
    {
        if (remote_media_type.local_audio_encode.a_type == A_NONE)
        {
            local_a_enc_type  = A_NONE;
            remote_a_dec_type = A_NONE;
        }
        else if(remote_media_type.local_audio_encode.a_type == A_G711)
        {
            if(local_media_attr.local_audio_decode.a_type == A_G711_G729)
            {
                local_a_enc_type  = A_G711;
                remote_a_dec_type = A_G711;
            }else if(local_media_attr.local_audio_decode.a_type == A_G729 ||
                local_media_attr.local_audio_decode.a_type == A_PCM)
            {
                local_a_enc_type  = A_PCM;
                remote_a_dec_type = A_PCM;
            }
        }
        else if(remote_media_type.local_audio_encode.a_type == A_G729)
        {
            if (local_media_attr.local_audio_decode.a_type == A_G711_G729)
            {
                local_a_enc_type  = A_G729;
                remote_a_dec_type = A_G729;
            }
            else if (local_media_attr.local_audio_decode.a_type == A_NONE)
            {
                local_a_enc_type  = A_NONE;
                remote_a_dec_type = A_NONE;
            }
            else if (local_media_attr.local_audio_decode.a_type == A_G711 ||
                local_media_attr.local_audio_decode.a_type == A_PCM)
            {
                local_a_enc_type  = A_PCM;
                remote_a_dec_type = A_PCM;
            }
        }else if(remote_media_type.local_audio_encode.a_type == A_G711_G729)
        {
            if(local_media_attr.local_audio_decode.a_type == A_G729)
            {
                local_a_enc_type  = A_G729;
                remote_a_dec_type = A_G729;
            }else if(local_media_attr.local_audio_decode.a_type == A_NONE)
            {
                local_a_enc_type  = A_NONE;
                remote_a_dec_type = A_NONE;
            }else if(local_media_attr.local_audio_decode.a_type == A_G711)
            {
                local_a_enc_type  = A_G711;
                remote_a_dec_type = A_G711;
            }else if(local_media_attr.local_audio_decode.a_type == A_PCM)
            {
                local_a_enc_type  = A_PCM;
                remote_a_dec_type = A_PCM;
            }
        }
    }

    local_media.a_enc_type = local_a_enc_type;
    local_media.a_dec_type = local_a_dec_type;

    remote_media.a_enc_type = remote_a_enc_type;
    remote_media.a_dec_type = remote_a_dec_type;
}

void Reset_negotiate_result()
{
    memset(&local_media,0,sizeof(NEGOTIATE_RESULT));
    memset(&remote_media,0,sizeof(NEGOTIATE_RESULT));
}

void Reset_negotiate_audioresult()
{
    local_media.a_dec_type = 0;
    local_media.a_enc_type = 0;
    local_media.a_packtime = 0;
    local_media.a_recvport = 0;
    local_media.a_samplebit= 0;
    local_media.a_samplechannel = 0;
    local_media.a_samplerate = 0;

    memset(&remote_media,0,sizeof(NEGOTIATE_RESULT));
}

void Reset_negotiate_videodecresult()
{
    local_media.v_dec_resolution_h =0;
    local_media.v_dec_resolution_w =0;
    local_media.v_dec_type = 0;
    local_media.v_recvport =0;
    memset(&remote_media,0,sizeof(NEGOTIATE_RESULT));
}

void Reset_negotiate_videoencresult()
{
    local_media.v_enc_bitrate =0;
    local_media.v_enc_resolution_h =0;
    local_media.v_enc_resolution_w =0;
    local_media.v_enc_type = 0;
    local_media.v_sendport = 0;
    memset(&remote_media,0,sizeof(NEGOTIATE_RESULT));
}

void printMediaInfo()
{
    printf("venc:%d\n",get_venc_type());
    printf("vdec:%d\n",get_vdec_type());
    printf("vencbitrate:%d\n",get_venc_bitrate());
    printf("venc_w:%d\n",get_venc_width());
    printf("venc_h:%d\n",get_venc_height());
    printf("vdec_w:%d\n",get_vdec_width());
    printf("venc_h:%d\n",get_vdec_height());
    printf("v_recvport:%d\n",get_vrecv_port());
    printf("a_recvprt:%d\n",get_arecv_port());
    printf("a_rate:%d\n",get_a_rate());
    printf("a_bit:%d\n",get_a_bit());
    printf("a_channel:%d\n",get_a_channel());
    printf("a_packtime:%d\n",get_a_packtime());
    printf("a_enc:%d\n",get_aenc_type());
    printf("a_dec:%d\n",get_adec_type());
    printf("对方平台类型:%d\n",local_media.remote_platform_type);
}

void printRemoteMediaInfo()
{
    printf("Remote venc:%d\n",remote_media.v_enc_type);
    printf("Remote vdec:%d\n",remote_media.v_dec_type);
    printf("Remote vencbitrate:%d\n",remote_media.v_enc_bitrate);
    printf("Remote venc_w:%d\n",remote_media.v_enc_resolution_w);
    printf("Remote venc_h:%d\n",remote_media.v_enc_resolution_h);
    printf("Remote vdec_w:%d\n",remote_media.v_dec_resolution_w);
    printf("Remote venc_h:%d\n",remote_media.v_dec_resolution_h);
    printf("Remote v_recvport:%d\n",remote_media.v_recvport);
    printf("Remote a_recvprt:%d\n",remote_media.a_recvport);
    printf("Remote a_rate:%d\n",remote_media.a_samplerate);
    printf("Remote a_bit:%d\n",remote_media.a_samplebit);
    printf("Remote a_channel:%d\n",remote_media.a_samplechannel);
    printf("Remote a_packtime:%d\n",remote_media.a_packtime);
    printf("Remote a_enc:%d\n",remote_media.a_enc_type);
    printf("Remote a_dec:%d\n",remote_media.a_dec_type);
    printf("对方平台类型:%d\n",remote_media.remote_platform_type);
}

int get_remote_platform_type()
{
    return local_media.remote_platform_type;
}
int get_venc_type()
{
    return local_media.v_enc_type;
}
int get_vdec_type()
{
    return local_media.v_dec_type;
}
int get_venc_bitrate()
{
    return local_media.v_enc_bitrate;
}
int get_venc_width()
{
    return local_media.v_enc_resolution_w;
}
int get_venc_height()
{
    return local_media.v_enc_resolution_h;
}
int get_vdec_width()
{
    return local_media.v_dec_resolution_w;
}
int get_vdec_height()
{
    return local_media.v_dec_resolution_h;
}
int get_vrecv_port()
{
    return local_media.v_recvport;
}
int get_arecv_port()
{
    return local_media.a_recvport;
}
int get_vsend_port()
{
    return local_media.v_sendport;
}
int get_asend_port()
{
    return local_media.a_sendport;
}

int get_a_rate()
{
    return local_media.a_samplerate;
}
int get_a_bit()
{
    return local_media.a_samplebit;
}
int get_a_channel()
{
    return local_media.a_samplechannel;
}
int get_a_packtime()
{
    return local_media.a_packtime;
}
int get_aenc_type()
{
    return local_media.a_enc_type;
}
int get_adec_type()
{
    return local_media.a_dec_type;
}

void set_vdec_type(int type)
{
    local_media.v_dec_type = type;
}
void set_venc_type(int type)
{
    local_media.v_enc_type = type;
}
void set_adec_type(int type)
{
    local_media.a_dec_type = type;
}
void set_aenc_type(int type)
{
    local_media.a_enc_type = type;
}
