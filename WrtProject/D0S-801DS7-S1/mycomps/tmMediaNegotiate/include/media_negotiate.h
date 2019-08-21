/* #############################################################################
 */

/*  2012-12-05
 *
 *  Description              :
 *
 *    媒体协商 ，为兼容801A和801D平台，以及双向视频设计
 *  
 *    1.呼叫发起端在CALL指令后带媒体协商数据，数据格式为：
 *   
 *    [本机视频编码能力][本机视频解码能力][本机音频编码能力][本机音频解码能力]
 *
 *    2.被叫端在回应空闲指令带媒体协商数据，数据格式为：
 *
 *    [本机视频编码能力][本机视频解码能力][本机音频编码能力][本机音频解码能力]
 *
 *    3.发起端将在分析收到的媒体数据，决定采用的编解码格式
 *
 *    4.如果收到的数据中不带媒体协商数据，则认为对方为801A平台设备，按照默认处理。
 *
 *     例如： 801D分机呼叫801A分机（不具备视频编码能力）
 *
 *  呼叫发起端 
    本机视频编码能力
    local_video_encode.v_type = V_MPEG4_H264;
    local_video_encode.v_resolution = V_VGA;
    local_video_encode.v_bitrate = BITRATE_LEV8;
    local_video_encode.v_frame_rate = 25;
    local_video_encode.v_port = 20000;
    local_video_encode.v_res0 = 0;
    local_video_encode.v_res1 = 0;
    local_video_encode.v_res2 = 0;
    local_video_encode.v_res3 = 0;
    
    本机视频解码能力
    local_video_decode.v_type = V_MPEG4_H264;
    local_video_decode.v_resolution = V_VGA;
    local_video_decode.v_bitrate = BITRATE_LEV8;
    local_video_decode.v_frame_rate = 25;
    local_video_decode.v_port = 20000;
    local_video_decode.v_res0 = 0;
    local_video_decode.v_res1 = 0;
    local_video_decode.v_res2 = 0;
    local_video_decode.v_res3 = 0;
    
    
    本机音频编码能力
    local_audio_encode.a_type = A_G711;
    local_audio_encode.a_channel = A_CHANNEL_1;
    local_audio_encode.a_sample_bit = A_BIT_16;
    local_audio_encode.a_sample_rate = A_16K;
    local_audio_encode.ms_per_pak = 20;    //每个包多少ms的数据,默认20ms
    local_audio_encode.a_port = 15004;
    local_audio_encode.a_res0 = 0;
    local_audio_encode.a_res1 = 0;
    local_audio_encode.a_res2 = 0;

    本机音频解码能力
    local_audio_decode.a_type = A_G711;
    local_audio_decode.a_channel = A_CHANNEL_1;
    local_audio_decode.a_sample_bit = A_BIT_16;
    local_audio_decode.a_sample_rate = A_16K;
    local_audio_decode.ms_per_pak = 20;    //每个包多少ms的数据,默认20ms
    local_audio_decode.a_port = 15004;
    local_audio_decode.a_res0 = 0;
    local_audio_decode.a_res1 = 0;
    local_audio_decode.a_res2 = 0;

*音频部分协商主要根据 被叫的local_audio_decode能力和主叫的local_audio_encode来协商
*对主叫的local_audio_decode、被叫的local_audio_encode可填0
    
 *								  ^	
 *   801A分机将不带媒体数据		  |
 *								  |
 *   呼叫发起端应采用801A默认参数初始化音视频编解码器: 关闭视频编码、关闭视频解码、开启音频编解码
 *								  |
 *								  |
 */ //							  |	
//								  |
/*								  |
2013-7-10 增加说明 媒体协商流程	  |
#define MEDIA_NEGOTIATE           |     0x1190
#define MEDIA_NEGOTIATE_ACK       |     0x1191
								  |
===============================================================================
								  |
			 呼叫端               |      被叫端
				|				  |
				|                 |
			 发起呼叫------------------>收到呼叫
			 					  |			|
			 					  |			|
			 收到空闲<------------------回复空闲
			 	|				  |
				|				  |
		  发送媒体协商			  |
	 主叫端媒体处理能力结构 ------|
		 (40+128字节)----0x1190---------->收到媒体协商
		 								媒体协商处理
		 					NEGOTIATION_RESULT(local_media、remote_media)
		 											|               |
		 											|				|
		 								        协商结构 	   发送至主叫端
		 								  被叫音视频初始化使用   (40+44字节)
		 								  			|				 |
		 收到协商结果<----------0x1191---------------|----------------|
			   |									|	
			   |									|		
	  起动音视频开始通话<------------------->启动音视频开始通话

协商规则:
1、音频打包时间 协商为本地enc打包时间
2、视频编码率 为对方解码的编码率
3、音视频接收端口号 为对方的本地编码端口号
4、视频编码类型 为本地编码和对端解码协商出双方都支持的类型，优先H264
5、视频解码类型 为本地解码和对端编码协商出双方都支持的类型，优先H264
6、视频编码分辨率和对方视频解码分辨率 为本地编码分辨率
7、音频采样通道数 为本地读和对方写协商出双方都支持的通道数，优先单声道
8、音频采样率 为本地读和对方写协商出双方都支持的通道数，优先8000
9、音频采样位宽 为本地读和对方写协商出双方都支持的通道数，优先8bit
10、音频编解码格式 协商本地enc和对端dec都支持的通道数，优先G729、次优先G711
================================================================================

注:如果被叫端不支持媒体协商那么主叫端按801A旧平台参数启动音视频

示例协商结果:

用于被叫端音视频初始化
local_media.v_enc_type = V_H264;
local_media.v_dec_type = V_MPEG4;
local_media.v_enc_bitrate = 256*1000;//被叫发送码流256K
local_media.a_packtime = 20;//20ms
local_media.a_samplechannel = A_CHANNEL_2;//2
local_media.a_samplerate    = A_16K;//16000
local_media.a_samplebit     = A_BIT_16;//16
local_media.a_enc_type = A_G711;
local_media.a_dec_type = A_G711;
local_media.v_recvport = 20000;
local_media.a_recvport = 15004;
local_media.v_enc_resolution_x = 352;
local_media.v_enc_resolution_y = 288;
local_media.v_dec_resolution_x = 640;
local_media.v_dec_resolution_y = 480;

发送至主叫端用于主叫端音视频初始化
remote_media.v_enc_type = V_MPEG4;
remote_media.v_dec_type = V_H264;
remote_media.v_enc_bitrate = 600*1000;//主叫发送码流600K
remote_media.a_packtime = 20;
remote_media.a_samplechannel = A_CHANNEL_2;//2
remote_media.a_samplerate    = A_16K;//16000
remote_media.a_samplebit     = A_BIT_16;//16
remote_media.a_enc_type = A_G711;
remote_media.a_dec_type = A_G711;
remote_media.v_recvport = 20000;
remote_media.a_recvport = 15004;
remote_media.v_enc_resolution_x = 640;
remote_media.v_enc_resolution_y = 480;
remote_media.v_dec_resolution_x = 352;
remote_media.v_dec_resolution_y = 288;
*/

#ifndef  _MEDIA_NEGOTIATE_H_
#define  _MEDIA_NEGOTIATE_H_

#if	defined(__cplusplus)
extern	"C"	{
#endif	/* defined(__cplusplus) */

#define DOOR_A 0x0101
#define DOOR_B 0x0102
#define DOOR_C 0x0103
#define DOOR_D 0x0104

#define ROOM_A 0x0201
#define ROOM_B 0x0202
#define ROOM_C 0x0203
#define ROOM_D 0x0204
#define ROOM_755D 0x0205
#define ROOM_745D 0x0206

#define MANAGER_D 0x0301

#define EXTEND_8K	0x0401

#define SMALL_DOOR_B 0x0501
#define SMALL_DOOR_D 0x0502

typedef enum
{
    V_801A=0,
    V_NONE,
    V_MPEG4,       //is used by 801A
    V_H264,
    V_MPEG4_H264,
}V_TYEP;

#define min(a,b)  (((a) < (b)) ? (a) : (b))

typedef struct{
    int v_enc_type;
    int v_dec_type;
    int v_enc_bitrate;
    int a_packtime;
    int a_samplechannel;//音频读写 channel、bit、rate使用同一参数
    int a_samplebit;
    int a_samplerate;
    int a_enc_type;
    int a_dec_type;
    int v_recvport;
    int a_recvport;
    int v_sendport;
    int a_sendport;
    unsigned short v_dec_resolution_w;
    unsigned short v_dec_resolution_h;
    unsigned short v_enc_resolution_w;
    unsigned short v_enc_resolution_h;
    unsigned int remote_platform_type;
}NEGOTIATE_RESULT;//52 byte

typedef enum
{
    V_QQCIF_P=0,//88  X 72
    V_QQCIF_N,  //88  X 60
    V_SQCIF,    //128 X 96
    V_QCIF_P,   //176 X 144
    V_QCIF_N,   //176 X 120
    V_CIF_P,    //352 X 288  is used by 801A
    V_CIF_N,    //352 X 240
    V_2CIF_P,   //704 X 288
    V_2CIF_N,   //704 X 240    
    V_H_D1_P,   //352 X 576
    V_H_D1_N,   //352 X 480
    V_D1_P,     //704 X 576
    V_D1_N,     //704 X 480
    V_F_D1_P,   //720 X 576
    V_F_D1_N,   //720 X 480
    V_VGA,      //640 X 480  is used by 801D
    V_QVGA,     //320 X 240
    V_QQVGA,    //160 X 120    
    V_SVGA,     //800 X 600    
    V_XVGA,     //1024X 768    
    V_WAGA,     //1440X 900    
    V_SXGA,     //1280X 1024
    V_WSXGA,    //1680X 1050
    V_UXGA,     //1600X 1200
    V_WUXGA,    //1920X 1200
    V_QXGA,     //2048X 1536
    V_MPEG_HD,  //1920X 1080
    V_720P,     //1280X 720
    V_4K,
    V_8K,
}V_RESOLUTION;

typedef enum
{
    BITRATE_LEV0=0,  //64kbps
    BITRATE_LEV1,    //128kbps
    BITRATE_LEV2,    //256k   128x2
    BITRATE_LEV3,    //384k   128x3
    BITRATE_LEV4,    //512k   128x4
    BITRATE_LEV5,    //640k   128x5
    BITRATE_LEV6,    //768k   128x6
    BITRATE_LEV7,    //896k   128x7
    BITRATE_LEV8,    //1024k  128x8    is used by 801A
    BITRATE_LEV9,    //1536k  1.5M
    BITRATE_LEV10,   //2048   2M
}V_BITRATE;

typedef struct
{
    V_TYEP v_type;//4
    V_RESOLUTION v_resolution;//4
    V_BITRATE v_bitrate;//4
    int v_frame_rate;//4
    unsigned short v_port;
    unsigned short v_res0;//4
    int v_res1;//4
    int v_res2;//4
    int v_res3;//4
}VIDEO_INFO;//32

typedef enum
{
    A_TYPE_801A=0,
    A_NONE,
    A_PCM,
    A_G711,       //is used by 801A
    A_G729,
    A_G711_G729,
}A_TYPE;

typedef enum
{
    A_CHANNEL_801A=0,
    A_CHANNEL_1,
    A_CHANNEL_2,//is used by 801A
    A_CHANNEL_1_2,
    A_CHANNEL_4,
    A_CHANNEL_6,
    A_CHANNEL_8,
}A_CHANNEL;

typedef enum
{
    A_BIT_801A=0,
    A_BIT_8,
    A_BIT_16,    //is used by 801A
    A_BIT_8_16,
}A_SAMPLE_BIT;

typedef enum
{
    A_RATE_801A=0,
    A_8K,
    A_16K,      //is used by 801A
    A_8K_16K,
    A_22K,
    A_44_1K,
    A_48K,
}A_SAMPLE_RATE;

typedef struct
{
    A_TYPE a_type;
    A_CHANNEL a_channel;
    A_SAMPLE_BIT a_sample_bit;
    A_SAMPLE_RATE a_sample_rate;
    int ms_per_pak;    //每个包多少ms的数据,默认20ms
    unsigned short a_port;
    unsigned short a_res0;
    int a_res1;
    int a_res2;
}AUDIO_INFO;

typedef struct
{
    VIDEO_INFO local_video_encode;
    VIDEO_INFO local_video_decode;
    AUDIO_INFO local_audio_encode;
    AUDIO_INFO local_audio_decode;
    unsigned int platform_type;
    unsigned int reseve1;
    unsigned int reseve2;
    unsigned int reseve3;
}MEDIA_DATA;

void Init_Local_MediaInfo();
void Get_local_mediainfo(char *buf);
void Get_negotiate_result_of_remote(char *buf);
void media_negotiate_process(char *buf,int buflen);
void Set_local_mediaInfo(char *buf);
void Reset_negotiate_result();
void Reset_negotiate_audioresult();
void Reset_negotiate_videodecresult();
void Reset_negotiate_videoencresult();

void printMediaInfo();
void printRemoteMediaInfo();
int get_remote_platform_type();
int get_venc_type();
int get_vdec_type();
int get_venc_bitrate();
int get_venc_width();
int get_venc_height();
int get_vdec_width();
int get_vdec_height();
int get_vrecv_port();
int get_arecv_port();
int get_vsend_port();
int get_asend_port();
int get_a_rate();
int get_a_bit();
int get_a_channel();
int get_a_packtime();
int get_aenc_type();
int get_adec_type();

void set_vdec_type(int type);
void set_venc_type(int type);
void set_adec_type(int type);
void set_aenc_type(int type);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* end of _MEDIA_NEGOTIATE_H_*/

