#ifndef __VIDEOCOMMON__H_
#define __VIDEOCOMMON__H_\

#define _RTP_HEAD_SIZE  12
#define MP4VNUM 200
#define _WRT_VIDEO_PACK_SIZE  1460
#define MULTITTL 5
#define MAXPACKNUM  100     //帧最大数据包数量
#define VIDEOMAX 720*480
#define MPEG4_ENC_DATA_HEAD_LEN 18
#define P_Debug printf
#define UDPVIDEOPACKMAXSIZE 256000

#define _SUB_HOST_WATCH        0x0020   //开始监视              分机->主机
#define _SUB_HOST_WATCHEND     0x0021   //结束监视 
#define _HOST_SUB_DOOR_WATCHEND     0x0108   //主机结束监视     主机->分机
#define _HOST_BUSY               0X0103
#define _HOST_MONTACK               0X0102
#define _SUB_HOST_WATCHEND     0x0021   //结束监视   

#define WRT_VIDEO_RECV_PORT 20000

	// 消息头
	struct TALK_TMSG_HEADER
	   {
		char Head[4];
		int Length; 		// 长度
		unsigned short Order;		//命令字
		char Src_Desc[15];					 //本机描述
		char Dest_Desc[15]; 				  //目标描述
	   }__attribute__ ((packed));

	struct TimeStamp1{
		unsigned int OldCurrVideo;	   //上一次当前视频时间
		unsigned int CurrVideo;
		unsigned int OldCurrAudio;	   //上一次当前音频时间
		unsigned int CurrAudio;
	};
   
	//同步播放结构
	struct _SYNC
	{
		pthread_cond_t cond;		//同步线程条件变量
		pthread_condattr_t cond_attr;
		pthread_mutex_t lock;		//互斥锁
		pthread_mutex_t audio_rec_lock;//[VPLAYNUM];//音频录制互斥锁
		pthread_mutex_t audio_play_lock;//[VPLAYNUM];//音频播放互斥锁
		int video_capture_lock_flag;		 //视频捕获互斥锁 标志
		pthread_mutex_t video_capture_lock;
		pthread_mutex_t video_rec_lock;//[VPLAYNUM];//视频录制互斥锁
		pthread_mutex_t video_play_lock;//[VPLAYNUM];//视频播放互斥锁
		unsigned int count;		//计数
		uint8_t isDecodeVideo; 	//视频已解码一帧  解码线程-->同步线程
		uint8_t isPlayVideo;		//视频已播放一帧  播放线程-->同步线程
		uint8_t isDecodeAudio; 	//音频已解码一帧  解码线程-->同步线程
		uint8_t isPlayAudio;		//音频已播放一帧  播放线程-->同步线程
	};
	
	//视频接收缓冲  未解码
	struct tempvideobuf1
	{
		//	int iput;					  // 环形缓冲区的当前放入位置
		//	int iget;					  // 缓冲区的当前取出位置
		//	int n;						  // 环形缓冲区中的元素总数量
		uint32_t timestamp;  //时间戳
		uint32_t frameno; 	  //帧序号
		short TotalPackage;	  //总包数
		uint8_t CurrPackage[MAXPACKNUM]; //当前包   1 已接收	0 未接收
		int Len;				  //帧数据长度
		uint8_t isFull;				   //该帧已接收完全
		unsigned char *buffer;//[VIDEOMAX];
		unsigned char frame_flag; 			//帧标志 音频帧 I帧 P帧  
	};

	//视频接收缓冲 链表
	typedef struct node2{
				  struct tempvideobuf1 Content;
				  struct node2 *llink, *rlink;
	}TempVideoNode1;
   
	struct _wrt_video_buf1	  //[视频流结构体]
	{
		unsigned short DataType;
		unsigned short Frameno;
		unsigned int timestamp;  //时间戳
		unsigned short seq_no;
		unsigned char *buff;	   //[视频流的大小]
		unsigned char CurrPackage[MAXPACKNUM]; //当前包	1 已接收  0 未接收
		unsigned int  CurrPackseq[MAXPACKNUM];
		int size;
		int TotalPackage; 	 //增加这一个
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
	 
//通话数据结构
struct talkdata1
{
	//   unsigned char Order;	   //操作命令
	char HostAddr[20];		 //主叫方地址
	unsigned char HostIP[4]; //主叫方IP地址
	char AssiAddr[20];		 //被叫方地址
	unsigned char AssiIP[4]; //被叫方IP地址
	unsigned int timestamp;  //时间戳
	unsigned short DataType;		  //数据类型
	unsigned short Frameno; 		  //帧序号
	unsigned int Framelen;			  //帧数据长度	  
	unsigned short TotalPackage;	  //总包数
	unsigned short CurrPackage; 	  //当前包数
	unsigned short Datalen; 		  //数据长度
	unsigned short PackLen; 	  //数据包大小

#ifdef _WRT_SUPPORT  //慧锐通
	 unsigned int curr_timestamp;  //时间戳
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
//添加转发视频到手机接口。
void init_phone_video();
int start_phone_video2(unsigned long ip,int index);
int stop_phone_video2(int index);

///
//////////////////////////////////////////////////////////


#endif //
