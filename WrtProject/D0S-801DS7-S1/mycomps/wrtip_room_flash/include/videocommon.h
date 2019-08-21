#ifndef __VIDEOCOMMON__H_
#define __VIDEOCOMMON__H_\

#define _RTP_HEAD_SIZE  12
#define MP4VNUM 200
#define _WRT_VIDEO_PACK_SIZE  1460
#define MULTITTL 5
#define MAXPACKNUM  100     //֡������ݰ�����
#define VIDEOMAX 720*480
#define MPEG4_ENC_DATA_HEAD_LEN 18
#define P_Debug printf
#define UDPVIDEOPACKMAXSIZE 256000

#define _SUB_HOST_WATCH        0x0020   //��ʼ����              �ֻ�->����
#define _SUB_HOST_WATCHEND     0x0021   //�������� 
#define _HOST_SUB_DOOR_WATCHEND     0x0108   //������������     ����->�ֻ�
#define _HOST_BUSY               0X0103
#define _HOST_MONTACK               0X0102
#define _SUB_HOST_WATCHEND     0x0021   //��������   

#define WRT_VIDEO_RECV_PORT 20000

	// ��Ϣͷ
	struct TALK_TMSG_HEADER
	   {
		char Head[4];
		int Length; 		// ����
		unsigned short Order;		//������
		char Src_Desc[15];					 //��������
		char Dest_Desc[15]; 				  //Ŀ������
	   }__attribute__ ((packed));

	struct TimeStamp1{
		unsigned int OldCurrVideo;	   //��һ�ε�ǰ��Ƶʱ��
		unsigned int CurrVideo;
		unsigned int OldCurrAudio;	   //��һ�ε�ǰ��Ƶʱ��
		unsigned int CurrAudio;
	};
   
	//ͬ�����Žṹ
	struct _SYNC
	{
		pthread_cond_t cond;		//ͬ���߳���������
		pthread_condattr_t cond_attr;
		pthread_mutex_t lock;		//������
		pthread_mutex_t audio_rec_lock;//[VPLAYNUM];//��Ƶ¼�ƻ�����
		pthread_mutex_t audio_play_lock;//[VPLAYNUM];//��Ƶ���Ż�����
		int video_capture_lock_flag;		 //��Ƶ���񻥳��� ��־
		pthread_mutex_t video_capture_lock;
		pthread_mutex_t video_rec_lock;//[VPLAYNUM];//��Ƶ¼�ƻ�����
		pthread_mutex_t video_play_lock;//[VPLAYNUM];//��Ƶ���Ż�����
		unsigned int count;		//����
		uint8_t isDecodeVideo; 	//��Ƶ�ѽ���һ֡  �����߳�-->ͬ���߳�
		uint8_t isPlayVideo;		//��Ƶ�Ѳ���һ֡  �����߳�-->ͬ���߳�
		uint8_t isDecodeAudio; 	//��Ƶ�ѽ���һ֡  �����߳�-->ͬ���߳�
		uint8_t isPlayAudio;		//��Ƶ�Ѳ���һ֡  �����߳�-->ͬ���߳�
	};
	
	//��Ƶ���ջ���  δ����
	struct tempvideobuf1
	{
		//	int iput;					  // ���λ������ĵ�ǰ����λ��
		//	int iget;					  // �������ĵ�ǰȡ��λ��
		//	int n;						  // ���λ������е�Ԫ��������
		uint32_t timestamp;  //ʱ���
		uint32_t frameno; 	  //֡���
		short TotalPackage;	  //�ܰ���
		uint8_t CurrPackage[MAXPACKNUM]; //��ǰ��   1 �ѽ���	0 δ����
		int Len;				  //֡���ݳ���
		uint8_t isFull;				   //��֡�ѽ�����ȫ
		unsigned char *buffer;//[VIDEOMAX];
		unsigned char frame_flag; 			//֡��־ ��Ƶ֡ I֡ P֡  
	};

	//��Ƶ���ջ��� ����
	typedef struct node2{
				  struct tempvideobuf1 Content;
				  struct node2 *llink, *rlink;
	}TempVideoNode1;
   
	struct _wrt_video_buf1	  //[��Ƶ���ṹ��]
	{
		unsigned short DataType;
		unsigned short Frameno;
		unsigned int timestamp;  //ʱ���
		unsigned short seq_no;
		unsigned char *buff;	   //[��Ƶ���Ĵ�С]
		unsigned char CurrPackage[MAXPACKNUM]; //��ǰ��	1 �ѽ���  0 δ����
		unsigned int  CurrPackseq[MAXPACKNUM];
		int size;
		int TotalPackage; 	 //������һ��
		int Mark;
	}; 
	 
	 typedef struct _RTP_header 
	 {
		 /* byte 0 */
#if (BYTE_ORDER == LITTLE_ENDIAN)
		 unsigned char csrc_len:4;	 /* expect 0 */
		 unsigned char extension:1;  /* expect 1, see RTP_OP below */
		 unsigned char padding:1;  /* expect 0 */
		 unsigned char version:2;  /* expect 2 */
#elif (BYTE_ORDER == BIG_ENDIAN)
		 unsigned char version:2;
		 unsigned char padding:1;
		 unsigned char extension:1;
		 unsigned char csrc_len:4;
#else
#error Neither big nor little
#endif
		 /* byte 1 */
#if (BYTE_ORDER == LITTLE_ENDIAN)
		 unsigned char payload:7;/* RTP_PAYLOAD_RTSP */
		 unsigned char marker:1;/* expect 1 */
#elif (BYTE_ORDER == BIG_ENDIAN)
		 unsigned char marker:1;
		 unsigned char payload:7;
#endif
		 /* bytes 2, 3 */
		 unsigned short seq_no;
		 /* bytes 4-7 */
		 unsigned int timestamp;
		 /* bytes 8-11 */
		 unsigned int ssrc;/* stream number is used here. */
	 } RTP_header;//12
	 
//ͨ�����ݽṹ
struct talkdata1
{
	//   unsigned char Order;	   //��������
	char HostAddr[20];		 //���з���ַ
	unsigned char HostIP[4]; //���з�IP��ַ
	char AssiAddr[20];		 //���з���ַ
	unsigned char AssiIP[4]; //���з�IP��ַ
	unsigned int timestamp;  //ʱ���
	unsigned short DataType;		  //��������
	unsigned short Frameno; 		  //֡���
	unsigned int Framelen;			  //֡���ݳ���	  
	unsigned short TotalPackage;	  //�ܰ���
	unsigned short CurrPackage; 	  //��ǰ����
	unsigned short Datalen; 		  //���ݳ���
	unsigned short PackLen; 	  //���ݰ���С

#ifdef _WRT_SUPPORT  //����ͨ
	 unsigned int curr_timestamp;  //ʱ���
#endif
}__attribute__ ((packed));

/////////////////////FUNCTION//////////////////////////////
///
int InitUdpSocket(short lPort);
void Talk_CreateUdpVideoRcvThread();
void* Talk_UdpVideoRcvThread(void *arg);

void Talk_RecvVideoUpDown_Func(unsigned char *recv_buf, char *cFromIP, int length);

/////////////////////////////////////////////////
/////////////////////////////////////////////////
TempVideoNode1 * init_videonode(void);
int creat_videonode(TempVideoNode1 *h, struct _wrt_video_buf1 __wrt_video_buf);
int add_videonode(TempVideoNode1 *p, struct talkdata1 talkdata,
   unsigned char *r_buf , int r_length);
int length_videonode(TempVideoNode1 *h);
int delete_videonode(TempVideoNode1 *p);
int delete_lost_videonode(TempVideoNode1 *h, uint32_t currframeno, uint32_t currtimestamp);
int delete_all_videonode(TempVideoNode1 *h);
TempVideoNode1 * find_videonode(TempVideoNode1 *h, int currframeno, int currpackage);
TempVideoNode1 * search_videonode(TempVideoNode1 *h);
int free_videonode(TempVideoNode1 *h);
//void video_play_deal_thread_func(void);
void *video_play_deal_thread_func(void *arg);
int Tcpsendcmd(char *remoteip,int remoteport,int cmd);
void _Wrt_Monitor(int sock);
void *tcp_process_thread_func(void *arg);
void Stopplayvideo();
void Stopsendvideo();

void Encode_Test_Start();
void Encode_Test_Stop();

//void Startplayvideo();

int Startplayvideo(void *data);
int  Startsendvideo(void * arg);
void StopVideoDevice();
void OpenVideoDevice();
void InitVideoDevice();
void UnInitVideoDevice();

////////////////////////
//by ljw added 2012-11-21 9:54:48
//���ת����Ƶ���ֻ��ӿڡ�
void init_phone_video();
int start_phone_video2(unsigned long ip,int index);
int stop_phone_video2(int index);

///
//////////////////////////////////////////////////////////


#endif //
