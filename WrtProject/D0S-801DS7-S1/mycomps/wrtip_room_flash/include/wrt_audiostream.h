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


//����¼��
set_audio_param(16000,2,12,0);
start_wrt_local_leave("/home/wrt/ring/1.wav");
...................
stop_wrt_local_leave();
...................
uninit_wrt_audiostream();
*****************************************************************************/

/*
*��������init_wrt_audiostream
*���������debug ,debug =1 ��ʾ��debugģʽ��
*�����������
*���ܣ���ʼ��mediastreamer2ý��⡣����Ҫlibmediastreamer2.a ��libortp.a)
*����Ҫ��չ��������libtmWebRTCAec.a libspeexdsp.a
*/
void init_wrt_audiostream(int debug);

/*
*��������uninit_wrt_audiostream
*�����������
*�����������
*���ܣ��˳�mediastreamer2ý���
*/
void uninit_wrt_audiostream();

/*
*��������set_audio_param
*���������rate,�����ʣ�Ĭ��16000
		nchannel,������Ĭ��2
		pttime,������ʱ�䣬һ�β�����õĲ����㡣Ĭ��12ms.��λ���롣
		ectime,����������ʱ�����������Ƶ����Ϊ��1.8000��16.20����Ч����á�
		Ĭ�ϵĵ�λ��Ϊ16bits.
		echomode,������ģʽ,1,2,3
		#define ONLY_SPEEX 1
		#define ONLY_WEBEC 2
		#define SPEEX_AND_WEBEC 3
*�����������
*���ܣ�������Ƶ����
*/
void set_audio_param(int rate,int nchannel,int pttime,int ectime,int echomode);

/*
*��������start_wrt_audiostream
*��������� 	destip,udp������Ƶ��Ŀ�ĵ�ַ
		destport,udp������Ƶ��Ŀ�ĵ�ַ�Ķ˿�
		localport�����ؽ�����Ƶ�Ķ˿ڡ�
		ec,�Ƿ�����������������
		���������Ϊ16000��2��16��12�����벻Ҫ���ø��
		
*�����������
*���أ�0
*���ܣ�������Ƶ��ý��
*/
int start_wrt_audiostream(unsigned long destip,short destport,short localport,int ec);

/*
*��������stop_wrt_audiostream
*��������� ��
		
*�����������
*���أ�0
*���ܣ�ֹͣ��Ƶ��ý��
*/
int stop_wrt_audiostream();

/*
*��������start_wrt_record_audiostream
*��������� file ��Ƶ�ļ���·�����ļ�����
		
*�����������
*���أ�0
*���ܣ�����¼����¼�ƶԷ������ԣ���
*/
int  start_wrt_record_audiostream(const char* file);

/*
*��������stop_wrt_record_audiostream
*��������� ��
		
*�����������
*���أ�0
*���ܣ�ֹͣ¼����ֹͣ¼�ƶԷ������ԣ���
*/
int  stop_wrt_record_audiostream();

/*
*��������start_wrt_local_leave
*��������� file ��Ƶ�ļ���·�����ļ���

*�����������
*���أ�0
*���ܣ���ʼ¼�Ʊ������ԡ�
*/
int start_wrt_local_leave(const char* file);

/*
*��������stop_wrt_local_leave
*��������� 
		
*�����������
*���ܣ�ֹͣ¼�Ʊ������ԡ�
*/
void stop_wrt_local_leave();


void set_wrt_player_db_gain(float* db); //���Բ����ã�Ĭ����2.0
void set_wrt_recorder_db_gain(float* db); //���Բ����ã�Ĭ����2.0
	

/*
*��������start_wrt_remote_leave
*��������� file ��Ƶ�ļ���·�����ļ������Լ����ؼ����Ķ˿�
		
*�����������
*���ܣ�����¼���������ԡ�
*/
int start_wrt_remote_leave(const char* file,int localport);


/*
*��������stop_wrt_remote_leave
*��������� 
		
*�����������
*���ܣ�ֹͣ¼���������ԡ�
*/
int stop_wrt_remote_leave();

/*
�ֻ�ת��������Ƶ���ֻ���5-8�ӷֻ�
*/
//srcip:����ip
//srcport:���ж˽�����Ƶ�˿ں�
//destip:ת�����豸ip
//destport:ת�����豸������Ƶ�˿ں�
//localport1:����������Ƶ�˿ں�
//localport2:����ת���豸���������ֻ�����Ƶ�˿ں�
int start_wrt_subroomaudio(unsigned long srcip,short srcport,unsigned long destip,short destport,short localport1,short localport2);

/*
ֹͣת��������Ƶ���ֻ���5-8�ӷֻ�
*/
int stop_wrt_subroomaudio();

int wrt_test_echo_delay();

#ifdef __cplusplus
}
#endif

#endif
