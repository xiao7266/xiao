#ifndef WRT_AUDIOSTREAM_H
#define WRT_AUDIOSTREAM_H

#ifdef __cplusplus
extern "C"{
#endif


/****************************************************************************
//demo
init_wrt_audiostream(0);
.........
.........
set_audio_param(16000,1,12,180,3);
start_wrt_audiostream(0x2001a8c0,15004,15004,1);
.........
start_wrt_record_audiostream("/home/wrt/ring/1.wav");
.........
stop_wrt_record_audiostream();
.........
.........
stop_wrt_audiostream();


//本地录音
set_audio_param(16000,2,12,0);
start_wrt_local_leave("/home/wrt/ring/1.wav");
...................
stop_wrt_local_leave();
...................
uninit_wrt_audiostream();
*****************************************************************************/

/*
*函数名：init_wrt_audiostream
*输入参数：debug ,debug =1 表示打开debug模式。
*输出参数：无
*功能：初始化mediastreamer2媒体库。（需要libmediastreamer2.a 和libortp.a)
*还需要扩展消回声库libtmWebRTCAec.a libspeexdsp.a
*/
void init_wrt_audiostream(int debug);

/*
*函数名：uninit_wrt_audiostream
*输入参数：无
*输出参数：无
*功能：退出mediastreamer2媒体库
*/
void uninit_wrt_audiostream();

/*
*函数名：set_audio_param
*输入参数：rate,采样率，默认16000
		nchannel,声道，默认2
		pttime,采样的时间，一次采样多久的采样点。默认12ms.单位毫秒。
		ectime,回声消除，时间参数。（音频参数为：1.8000，16.20）。效果最好。
		默认的的位率为16bits.
		echomode,消回声模式,1,2,3
		#define ONLY_SPEEX 1
		#define ONLY_WEBEC 2
		#define SPEEX_AND_WEBEC 3
*输出参数：无
*功能：设置音频参数
*/
void set_audio_param(int rate,int nchannel,int pttime,int ectime,int echomode);

/*
*函数名：start_wrt_audiostream
*输入参数： 	destip,udp发送音频的目的地址
		destport,udp发送音频的目的地址的端口
		localport，本地接收音频的端口。
		ec,是否采用软件回声消除。
		（如果参数为16000，2，16，12）。请不要配置该项。
		
*输出参数：无
*返回：0
*功能：启动音频流媒体
*/
int start_wrt_audiostream(unsigned long destip,short destport,short localport,int ec);

/*
*函数名：stop_wrt_audiostream
*输入参数： 无
		
*输出参数：无
*返回：0
*功能：停止音频流媒体
*/
int stop_wrt_audiostream();

/*
*函数名：start_wrt_record_audiostream
*输入参数： file 音频文件的路径和文件名。
		
*输出参数：无
*返回：0
*功能：启动录音（录制对方的留言）。
*/
int  start_wrt_record_audiostream(const char* file);

/*
*函数名：stop_wrt_record_audiostream
*输入参数： 无
		
*输出参数：无
*返回：0
*功能：停止录音（停止录制对方的留言）。
*/
int  stop_wrt_record_audiostream();

/*
*函数名：start_wrt_local_leave
*输入参数： file 音频文件的路径和文件名

*输出参数：无
*返回：0
*功能：开始录制本地留言。
*/
int start_wrt_local_leave(const char* file);

/*
*函数名：stop_wrt_local_leave
*输入参数： 
		
*输出参数：无
*功能：停止录制本地留言。
*/
void stop_wrt_local_leave();


void set_wrt_player_db_gain(float* db); //可以不设置，默认是2.0
void set_wrt_recorder_db_gain(float* db); //可以不设置，默认是2.0
	

/*
*函数名：start_wrt_remote_leave
*输入参数： file 音频文件的路径和文件名，以及本地监听的端口
		
*输出参数：无
*功能：启动录制主机留言。
*/
int start_wrt_remote_leave(const char* file,int localport);


/*
*函数名：stop_wrt_remote_leave
*输入参数： 
		
*输出参数：无
*功能：停止录制主机留言。
*/
int stop_wrt_remote_leave();

/*
分机转发主机音频到手机或5-8从分机
*/
//srcip:主叫ip
//srcport:主叫端接收音频端口号
//destip:转发至设备ip
//destport:转发至设备接收音频端口号
//localport1:接收主机音频端口号
//localport2:接收转发设备发送至主分机的音频端口号
int start_wrt_subroomaudio(unsigned long srcip,short srcport,unsigned long destip,short destport,short localport1,short localport2);

/*
停止转发主机音频到手机或5-8从分机
*/
int stop_wrt_subroomaudio();

int wrt_test_echo_delay();

#ifdef __cplusplus
}
#endif

#endif
