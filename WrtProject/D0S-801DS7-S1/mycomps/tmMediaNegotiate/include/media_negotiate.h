/* #############################################################################
 */

/*  2012-12-05
 *
 *  Description              :
 *
 *    ý��Э�� ��Ϊ����801A��801Dƽ̨���Լ�˫����Ƶ���
 *  
 *    1.���з������CALLָ����ý��Э�����ݣ����ݸ�ʽΪ��
 *   
 *    [������Ƶ��������][������Ƶ��������][������Ƶ��������][������Ƶ��������]
 *
 *    2.���ж��ڻ�Ӧ����ָ���ý��Э�����ݣ����ݸ�ʽΪ��
 *
 *    [������Ƶ��������][������Ƶ��������][������Ƶ��������][������Ƶ��������]
 *
 *    3.����˽��ڷ����յ���ý�����ݣ��������õı�����ʽ
 *
 *    4.����յ��������в���ý��Э�����ݣ�����Ϊ�Է�Ϊ801Aƽ̨�豸������Ĭ�ϴ���
 *
 *     ���磺 801D�ֻ�����801A�ֻ������߱���Ƶ����������
 *
 *  ���з���� 
    ������Ƶ��������
    local_video_encode.v_type = V_MPEG4_H264;
    local_video_encode.v_resolution = V_VGA;
    local_video_encode.v_bitrate = BITRATE_LEV8;
    local_video_encode.v_frame_rate = 25;
    local_video_encode.v_port = 20000;
    local_video_encode.v_res0 = 0;
    local_video_encode.v_res1 = 0;
    local_video_encode.v_res2 = 0;
    local_video_encode.v_res3 = 0;
    
    ������Ƶ��������
    local_video_decode.v_type = V_MPEG4_H264;
    local_video_decode.v_resolution = V_VGA;
    local_video_decode.v_bitrate = BITRATE_LEV8;
    local_video_decode.v_frame_rate = 25;
    local_video_decode.v_port = 20000;
    local_video_decode.v_res0 = 0;
    local_video_decode.v_res1 = 0;
    local_video_decode.v_res2 = 0;
    local_video_decode.v_res3 = 0;
    
    
    ������Ƶ��������
    local_audio_encode.a_type = A_G711;
    local_audio_encode.a_channel = A_CHANNEL_1;
    local_audio_encode.a_sample_bit = A_BIT_16;
    local_audio_encode.a_sample_rate = A_16K;
    local_audio_encode.ms_per_pak = 20;    //ÿ��������ms������,Ĭ��20ms
    local_audio_encode.a_port = 15004;
    local_audio_encode.a_res0 = 0;
    local_audio_encode.a_res1 = 0;
    local_audio_encode.a_res2 = 0;

    ������Ƶ��������
    local_audio_decode.a_type = A_G711;
    local_audio_decode.a_channel = A_CHANNEL_1;
    local_audio_decode.a_sample_bit = A_BIT_16;
    local_audio_decode.a_sample_rate = A_16K;
    local_audio_decode.ms_per_pak = 20;    //ÿ��������ms������,Ĭ��20ms
    local_audio_decode.a_port = 15004;
    local_audio_decode.a_res0 = 0;
    local_audio_decode.a_res1 = 0;
    local_audio_decode.a_res2 = 0;

*��Ƶ����Э����Ҫ���� ���е�local_audio_decode���������е�local_audio_encode��Э��
*�����е�local_audio_decode�����е�local_audio_encode����0
    
 *								  ^	
 *   801A�ֻ�������ý������		  |
 *								  |
 *   ���з����Ӧ����801AĬ�ϲ�����ʼ������Ƶ�������: �ر���Ƶ���롢�ر���Ƶ���롢������Ƶ�����
 *								  |
 *								  |
 */ //							  |	
//								  |
/*								  |
2013-7-10 ����˵�� ý��Э������	  |
#define MEDIA_NEGOTIATE           |     0x1190
#define MEDIA_NEGOTIATE_ACK       |     0x1191
								  |
===============================================================================
								  |
			 ���ж�               |      ���ж�
				|				  |
				|                 |
			 �������------------------>�յ�����
			 					  |			|
			 					  |			|
			 �յ�����<------------------�ظ�����
			 	|				  |
				|				  |
		  ����ý��Э��			  |
	 ���ж�ý�崦�������ṹ ------|
		 (40+128�ֽ�)----0x1190---------->�յ�ý��Э��
		 								ý��Э�̴���
		 					NEGOTIATION_RESULT(local_media��remote_media)
		 											|               |
		 											|				|
		 								        Э�̽ṹ 	   ���������ж�
		 								  ��������Ƶ��ʼ��ʹ��   (40+44�ֽ�)
		 								  			|				 |
		 �յ�Э�̽��<----------0x1191---------------|----------------|
			   |									|	
			   |									|		
	  ������Ƶ��ʼͨ��<------------------->��������Ƶ��ʼͨ��

Э�̹���:
1����Ƶ���ʱ�� Э��Ϊ����enc���ʱ��
2����Ƶ������ Ϊ�Է�����ı�����
3������Ƶ���ն˿ں� Ϊ�Է��ı��ر���˿ں�
4����Ƶ�������� Ϊ���ر���ͶԶ˽���Э�̳�˫����֧�ֵ����ͣ�����H264
5����Ƶ�������� Ϊ���ؽ���ͶԶ˱���Э�̳�˫����֧�ֵ����ͣ�����H264
6����Ƶ����ֱ��ʺͶԷ���Ƶ����ֱ��� Ϊ���ر���ֱ���
7����Ƶ����ͨ���� Ϊ���ض��ͶԷ�дЭ�̳�˫����֧�ֵ�ͨ���������ȵ�����
8����Ƶ������ Ϊ���ض��ͶԷ�дЭ�̳�˫����֧�ֵ�ͨ����������8000
9����Ƶ����λ�� Ϊ���ض��ͶԷ�дЭ�̳�˫����֧�ֵ�ͨ����������8bit
10����Ƶ������ʽ Э�̱���enc�ͶԶ�dec��֧�ֵ�ͨ����������G729��������G711
================================================================================

ע:������ж˲�֧��ý��Э����ô���ж˰�801A��ƽ̨������������Ƶ

ʾ��Э�̽��:

���ڱ��ж�����Ƶ��ʼ��
local_media.v_enc_type = V_H264;
local_media.v_dec_type = V_MPEG4;
local_media.v_enc_bitrate = 256*1000;//���з�������256K
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

���������ж��������ж�����Ƶ��ʼ��
remote_media.v_enc_type = V_MPEG4;
remote_media.v_dec_type = V_H264;
remote_media.v_enc_bitrate = 600*1000;//���з�������600K
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
    int a_samplechannel;//��Ƶ��д channel��bit��rateʹ��ͬһ����
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
    int ms_per_pak;    //ÿ��������ms������,Ĭ��20ms
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

