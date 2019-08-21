
#include <sys/socket.h>
#include <net/if.h>
#include <semaphore.h>       //sem_t
#include <dirent.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/vfs.h>
#include <netinet/if_ether.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <linux/fb.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <semaphore.h>       //sem_t
#include <malloc.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/wait.h>
#include "videocommon.h"

#include "info_list_handler.h"
#include "audiovideo.h"
#include "tmSysConfig.h"
#include "cmd_handler.h"

#include "wrt_audiostream.h"
#include "media_negotiate.h"

#include "jpeg.h"
#include "trvideo.h"

#include "sstardisp.h"
#include "sstarvideo.h"
#include "sstarcommon.h"

#ifdef __cplusplus
extern "C"{
#endif
#include "list.h"
#ifdef __cplusplus
}
#endif

typedef enum
{
    E_VIDEO_DISPLAY_NA = 0x0,
    E_VIDEO_DISPLAY_INIT,
    E_VIDEO_DISPLAY_DEINIT
} WRT_VideoDispStat_e;

typedef struct WRT_VideoParam_s
{
    unsigned short u16VideoW;
    unsigned short u16VideoH;
    unsigned short u16X;
    unsigned short u16Y;
    unsigned short u16DispW;
    unsigned short u16DispH;
    int VideoType;
} WRT_VideoParam_t;
extern char doorname[256];

FILE* fd;
FILE* sfd;

extern CCmdHandler*    m_pCmdHandler;

int DeltaLen;
int video_play_flag= 0;
int video_rcv_flag = 0;
int videoisplay = E_VIDEO_DISPLAY_DEINIT;
int video_exit =0;
unsigned short Curr_Frameno = 1;
unsigned int TotalPackage = 0;
//int temp_video_n;
int videorec2playsem_flag = 0;
struct tm *curr_tm_t;
struct TimeStamp1 TimeStamp;  //接收时间与播放时间，同步用
sem_t videoplaysem;
sem_t videorec2playsem;

extern T_SYSTEMINFO* pSystemInfo;

struct _SYNC sync_s;
int g_dec_mode = 0; //default decode h264
extern int g_enc_mode;
extern int g_send_run;
int temp_video_n;      //视频接收缓冲个数
int Talk_m_VideoSocket;
int Talk_tcpSocket;

int Talk_LocalVideoPort =20000;  //视频UDP端口

pthread_t video_play_deal_thread;	   //视频播放数据处理线程
pthread_t talk_udpvideorcvid;//udp接收数据并处理线程
pthread_t tcp_thread;
pthread_t play_recvideo_thread;


pthread_t talk_udpvideosendid;


int g_have_cap_photo = 0;
int g_have_rec_video = 0;
int g_video_rec_flag;
int SndBufLen=1024*128;
int RcvBufLen=1024*128;
TempVideoNode1 *TempVideoNode_h;    //视频接收缓冲列表
struct _wrt_video_buf1 _wrt_video_buf;

/******************************************************/
//by ljw added 2012-11-21 9:46:25
#define MAX_PHONE 4
static unsigned long g_video_phone_addr[MAX_PHONE]={0};
////////////////////////////////////////////////////////////

void WRT_LocalVideoDecodeInit(WRT_VideoParam_t *pstVideoParam)
{
    MI_S32 s32Layer = 0;
    MI_S32 s32Chn = 0;
    MI_S32 s32VdecChn = 0;
    MI_U16 u16Vdec2DispW = 0;
    MI_U16 u16Vdec2DispH = 0;
    WRT_DispChnParam_t stDispParam;
    if ((pstVideoParam->u16DispW > pstVideoParam->u16VideoW) ||
        (pstVideoParam->u16DispH > pstVideoParam->u16VideoH))
    {
        u16Vdec2DispW = pstVideoParam->u16VideoW;
        u16Vdec2DispH = pstVideoParam->u16VideoH;
    }
    else
    {
        u16Vdec2DispW = pstVideoParam->u16DispW;
        u16Vdec2DispH = pstVideoParam->u16DispH;
    }
    stDispParam.u16SrcVideoW = u16Vdec2DispW;
    stDispParam.u16SrcVideoH = u16Vdec2DispH;
    stDispParam.u16X = pstVideoParam->u16X;
    stDispParam.u16Y = pstVideoParam->u16Y;
    stDispParam.u16Width = pstVideoParam->u16DispW;
    stDispParam.u16Height = pstVideoParam->u16DispH;
    WRT_EnableDispChn(s32Layer, s32Chn, &stDispParam);
    printf("beal xxxxx111\n");
    WRT_CreateVdecChannel(s32VdecChn, pstVideoParam->VideoType,
        pstVideoParam->u16VideoW, pstVideoParam->u16VideoH,
        u16Vdec2DispW, u16Vdec2DispH);
    printf("beal xxxxx1222\n");

    WRT_ModuleBind(E_MI_MODULE_ID_VDEC, 0, s32VdecChn, 0,
        E_MI_MODULE_ID_DISP, 0, s32Layer, s32Chn);
    printf("beal xxxxx1333\n");

    return;
}

void WRT_LocalVideoDecodeExit()
{
    MI_S32 s32Layer = 0;
    MI_S32 s32Chn = 0;
    MI_S32 s32VdecChn = 0;
    WRT_ModuleUnBind(E_MI_MODULE_ID_VDEC, 0, s32VdecChn, 0,
        E_MI_MODULE_ID_DISP, 0, s32Layer, s32Chn);
    WRT_DiableDispChn(s32Layer, s32Chn);

    WRT_DestroyVdecChannel(s32VdecChn);
}

void videoexit_1();
void videoexit_2();

int InitUdpSocket(short lPort)//lport ==20000 video
{
    struct sockaddr_in s_addr;
    int  nZero=0;
    int  iLen;
    int m_Socket;
    int  nYes;
    int ret;
    int rc;
    int ttl; //设置TTL值
    printf("InitUdpSocket\n");
    /* 创建 socket , 关键在于这个 SOCK_DGRAM */

    if ((m_Socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)   //[创建socket]
    {
        P_Debug("Create socket error\r");    //[调试信息]
        return 0;
    }
    else
        P_Debug("create socket.\n");

    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    /* 设置地址和端口信息 */
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(lPort);
    s_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr(LocalIP);//INADDR_ANY;

    iLen=sizeof(nZero);           //  SO_SNDBUF

    nZero=SndBufLen;       //128K
    setsockopt(m_Socket,SOL_SOCKET,SO_SNDBUF,(char*)&nZero,sizeof((char*)&nZero));

    nZero=RcvBufLen;       //128K
    setsockopt(m_Socket,SOL_SOCKET,SO_RCVBUF,(char*)&nZero,sizeof((char*)&nZero));

    int on = 1;
    ret = setsockopt( m_Socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    ttl = MULTITTL; //设置TTL值
    rc = setsockopt(m_Socket, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));

    /* 绑定地址和端口信息 */
    if ((bind(m_Socket, (struct sockaddr *) &s_addr, sizeof(s_addr))) == -1)
    {
        P_Debug("bind error m_Socket = %d error =%x\n", m_Socket,errno);
        return 0;
    }
    else
        P_Debug("bind address to socket.\n\r");

    if (lPort == Talk_LocalVideoPort)
    {
        Talk_m_VideoSocket = m_Socket;
        //创建UDP接收线程
        Talk_CreateUdpVideoRcvThread();
    }

    return 1;
}

static int m_last_seq = 0;
int g_phone_or_externdevice_holdon = 1;
extern int g_exit_status;
extern int g_is_monitor_flag;
extern int   g_holdon_flag;

//2014 2 24 增加用于分机有手机接入
//当手机摘机，分机退出视频显示，只负责转发。
// 1-初始值 0-手机摘机或者手柄摘机 2-主分机退出视频
int g_phone_mont = 0;
//当手机监视，用于区别是分机主动监视还是手机监视
//手机监视，分机只转发视频，但视频初始化还是正常初始化并退出

void Talk_RecvVideoUpDown_Func(unsigned char *recv_buf, char *cFromIP, int length)  //对讲视频数据
{
    int i,j;
    int isAddrOK;
    int sendlength;
    int tmp;
    int isFull;
    struct talkdata1 talkdata;
    RTP_header r_header;
    int CurrPackage;

    //帧序号
    memcpy(&r_header, recv_buf, _RTP_HEAD_SIZE);
    unsigned short tmp_seq_no;
    tmp_seq_no = r_header.seq_no << 8;

    tmp_seq_no |= ((r_header.seq_no & 0xff00) >> 8);
    r_header.seq_no = tmp_seq_no;

    unsigned int tmp_timestamp;
    tmp_timestamp = ((r_header.timestamp & 0x000000FF) << 24) + ((r_header.timestamp & 0x0000FF00)<< 8) +((r_header.timestamp  & 0x00FF0000) >> 8) + ((r_header.timestamp & 0xFF000000) >> 24);
    r_header.timestamp = tmp_timestamp;

    //printf("seq :%d   ts :%d\n",r_header.seq_no,r_header.timestamp);
    if(r_header.version != 2)
        printf("r_header.version = %d ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n", r_header.version);

    //printf("4 _wrt_video_buf.size = %d, r_header.timestamp = %d\n", _wrt_video_buf.size, r_header.timestamp);

    talkdata.DataType = 2;
    talkdata.Frameno = Curr_Frameno;
    talkdata.TotalPackage;
    talkdata.timestamp = r_header.timestamp;


    if(temp_video_n >= MP4VNUM)
    {
        temp_video_n = MP4VNUM;
    }
    else
    {
        if(videorec2playsem_flag == 0)
        {
            P_Debug("VideoDeal_Func videorec2playsem_flag1 = 0 ----------\n");
            return;
        }

        //锁定videoplaybuf[vpbuf_iget].buffer
        pthread_mutex_lock(&sync_s.video_play_lock);      
        isFull = 0;
        //   printf("talkdata.timestamp=%d  _wrt_video_buf.timestamp=%d _wrt_video_buf.TotalPackage=%d\n",talkdata.timestamp,_wrt_video_buf.timestamp,_wrt_video_buf.TotalPackage);      

        if(_wrt_video_buf.timestamp != talkdata.timestamp)
        {
            if(_wrt_video_buf.size > 0)
            {
                _wrt_video_buf.DataType = talkdata.DataType;
                _wrt_video_buf.Frameno = talkdata.Frameno;
                isFull = 1;//前一个包是否有效
                for(i=0; i<_wrt_video_buf.TotalPackage; i++)
                {
                    if(_wrt_video_buf.CurrPackage[i] == 0)
                    {
                        isFull = 0;
                        printf("lost some package,drop it %d~~\n",length);
                        break;
                    }
                }
                for(i = 1;i<_wrt_video_buf.TotalPackage;i++)
                {
                    if((_wrt_video_buf.CurrPackseq[i-1]+1)!= _wrt_video_buf.CurrPackseq[i])
                    {
                        isFull = 0;
                        printf("seq error discard\n");
                        break;
                    }
                }
                if(_wrt_video_buf.size > UDPVIDEOPACKMAXSIZE)
                    printf("_wrt_video_buf.size = %d, > UDPVIDEOPACKMAXSIZE\n", _wrt_video_buf.size);
                else
                {
                    //printf("_wrt_video_buf.size = %d, Curr_Frameno = %d, length = %d\n", _wrt_video_buf.size, Curr_Frameno, length);
                    //
                    if (isFull == 1)
                    {
                        if(_wrt_video_buf.Mark == 1)
                        {
                            unsigned int headcheck;
                            //memcpy(&headcheck,&_wrt_video_buf.buff,3);
                            //printf("%d\n",headcheck);
                            if(g_dec_mode == 1)
                            {

                                if(_wrt_video_buf.buff[0] == 0x00 && _wrt_video_buf.buff[1] == 0x00 && _wrt_video_buf.buff[2] == 0x01)
                                    creat_videonode(TempVideoNode_h, _wrt_video_buf);
                                else
                                    printf("MP4 start head error\n");
                            }else if(g_dec_mode == 0)
                            {
                                if(_wrt_video_buf.buff[0] == 0x00 && _wrt_video_buf.buff[1] == 0x00 && _wrt_video_buf.buff[2] == 0x00&& _wrt_video_buf.buff[3] == 0x01)
                                    creat_videonode(TempVideoNode_h, _wrt_video_buf);
                                else
                                    printf("H264 start head error\n");
                            }
                        }
                        else
                        {
                            printf("_wrt_video_buf.Frameno = %d, data damage\n", _wrt_video_buf.Frameno);
                            isFull = 0;
                        }
                    }
                }   
                Curr_Frameno ++;
            }
            _wrt_video_buf.size = 0;
            _wrt_video_buf.timestamp = talkdata.timestamp;
            _wrt_video_buf.seq_no = r_header.seq_no;
            _wrt_video_buf.Mark = 0;

            for (i=0;i<MAXPACKNUM;i++)
            {
                _wrt_video_buf.CurrPackage[i] = 0;
            }

            if ((length - _RTP_HEAD_SIZE) > _WRT_VIDEO_PACK_SIZE)
            {
                printf("(length - _RTP_HEAD_SIZE) = %d , > _WRT_VIDEO_PACK_SIZE\n", (length - _RTP_HEAD_SIZE));
                return;  //大包，丢弃
            }
            else 
            {
                //g_len=g_len+length-12;
                memcpy(_wrt_video_buf.buff, recv_buf + _RTP_HEAD_SIZE, length - _RTP_HEAD_SIZE);   
                //fwrite(recv_buf + _RTP_HEAD_SIZE,length - _RTP_HEAD_SIZE,1,fd);
            }

            CurrPackage = r_header.seq_no - _wrt_video_buf.seq_no;
            _wrt_video_buf.CurrPackage[CurrPackage] = 1;
            _wrt_video_buf.CurrPackseq[CurrPackage] = r_header.seq_no;
            _wrt_video_buf.TotalPackage = CurrPackage + 1;
            _wrt_video_buf.size += (length - _RTP_HEAD_SIZE);

            if(r_header.marker ==1)  
            {
                _wrt_video_buf.Mark =r_header.marker;  
            }
        }
        else
        {
            CurrPackage = r_header.seq_no - _wrt_video_buf.seq_no;

            if (CurrPackage < 0)
            {
                printf("5.1 %d, r_header.seq_no = %d, _wrt_video_buf.seq_no = %d\n", CurrPackage, r_header.seq_no, _wrt_video_buf.seq_no);
                CurrPackage = CurrPackage + 65536;
                printf("CurrPackage = %d\n", CurrPackage);
            }

            if((_wrt_video_buf.size + (length - _RTP_HEAD_SIZE)) <= UDPVIDEOPACKMAXSIZE)
            {
                // g_len=g_len+length-12;
                //memcpy(_wrt_video_buf.buff + (CurrPackage)*_WRT_VIDEO_PACK_SIZE, recv_buf + _RTP_HEAD_SIZE, length - _RTP_HEAD_SIZE);
                memcpy(_wrt_video_buf.buff + _wrt_video_buf.size, recv_buf + _RTP_HEAD_SIZE, length - _RTP_HEAD_SIZE);
                //fwrite(recv_buf + _RTP_HEAD_SIZE,length - _RTP_HEAD_SIZE,1,fd);
            }  
            else
            {
                for(i=0;i<MAXPACKNUM;i++)
                {
                    _wrt_video_buf.CurrPackage[i] = 0;  
                }
            }

            if ((length - _RTP_HEAD_SIZE) > _WRT_VIDEO_PACK_SIZE)
                printf("(length - _RTP_HEAD_SIZE) = %d , > _WRT_VIDEO_PACK_SIZE\n", (length - _RTP_HEAD_SIZE));

            _wrt_video_buf.CurrPackage[CurrPackage] = 1;
            _wrt_video_buf.CurrPackseq[CurrPackage] = r_header.seq_no;
            _wrt_video_buf.size += (length - _RTP_HEAD_SIZE);

            if(_wrt_video_buf.TotalPackage < CurrPackage + 1)
            {
                _wrt_video_buf.TotalPackage = CurrPackage + 1;
            }
            //printf("pos = %d, _wrt_video_buf.size = %d\n", (r_header.seq_no - _wrt_video_buf.seq_no)*_WRT_VIDEO_PACK_SIZE, _wrt_video_buf.size);           

            if(r_header.marker ==1)
            {
                _wrt_video_buf.Mark =r_header.marker;
            }
        }
#if 1
        if (isFull == 1)
        {
            TimeStamp.OldCurrVideo = TimeStamp.CurrVideo; //上一次当前视频时间
            TimeStamp.CurrVideo = talkdata.timestamp;

            temp_video_n = length_videonode(TempVideoNode_h);
            //printf("video_n %d\n",temp_video_n);
            if (g_dec_mode == 1)
            {
                if (temp_video_n >= 3)   //3帧 40*3 120
                {
                    if(videorec2playsem_flag == 1)
                        sem_post(&videorec2playsem);
                    else
                        P_Debug("VideoDeal_Func videorec2playsem_flag2 = %d-\n",videorec2playsem_flag);
                }
            }
            else if(g_dec_mode == 0)
            {
                if(temp_video_n >= 3)
                {
                    if(videorec2playsem_flag == 1)
                        sem_post(&videorec2playsem);
                    else
                        P_Debug("VideoDeal_Func videorec2playsem_flag2 = %d-\n",videorec2playsem_flag);
                }
            }
        }
#endif
    //解锁
    pthread_mutex_unlock(&sync_s.video_play_lock);
    }
}

void Talk_CreateUdpVideoRcvThread()
{
    pthread_attr_t attr;
    int i, ret;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(&talk_udpvideorcvid, &attr, Talk_UdpVideoRcvThread, NULL);
    if(talk_udpvideorcvid <= 0)
    {
        printf("create recv thread error\n");
        return;
    }
    pthread_attr_destroy(&attr);

    if(ret!=0){
        P_Debug ("Create wrt video pthread error!\n");
    }
}

/*************************************************************************
*by ljw added 2012-11-21 9:51:08
*转发视频到手机
*************************************************************************/
static void sendvideo_to_phone(void* buffer,int len){
	int i =0;
	if(Talk_m_VideoSocket == -1)
		return;
	for(i = 0; i<MAX_PHONE;i++){
		if(g_video_phone_addr[i] != 0 && g_video_phone_addr[i] != 0xffffffff){
			 struct sockaddr_in to;
			  to.sin_family      = AF_INET;
              to.sin_addr.s_addr = g_video_phone_addr[i];
              to.sin_port        = htons(20000);
              sendto(Talk_m_VideoSocket,buffer,len,0,(struct sockaddr *)&to,sizeof(to));
		}
	}
}

void* Talk_UdpVideoRcvThread(void *arg)  //UDP接收线程函数
{
    /* 循环接收数据 */
    int sendlength;
    char FromIP[20];
    int newframeno;//帧序
    int currpackage;
    int i,j;
    int sub;
    struct sockaddr_in c_addr;
    socklen_t addr_len;
    int len;
    int tmp;
    unsigned char buff[UDPVIDEOPACKMAXSIZE];

    int isAddrOK;
    int result;
    fd_set readfd;
    unsigned long maxfd=0;
    struct timeval timeout;

    addr_len = sizeof(c_addr);
    video_rcv_flag =1 ;
    while (video_rcv_flag)
    {
        len = 0;
        if (Talk_m_VideoSocket < 0)
        {
            len = 0;
            usleep(10*1000);
            continue;
            //printf("socket is closed\n");
        }
        else
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = 40000;
            FD_ZERO(&readfd);
            FD_SET(Talk_m_VideoSocket, &readfd);
            maxfd = (Talk_m_VideoSocket+1)>maxfd?(Talk_m_VideoSocket+1):maxfd;
            result = select(maxfd,&readfd, NULL,  NULL, &timeout);
            if (result)
            {
                if (FD_ISSET(Talk_m_VideoSocket,&readfd))
                {
                    len = recvfrom(Talk_m_VideoSocket, buff, sizeof(buff) - 1, 0,(struct sockaddr *) &c_addr, &addr_len);
                }
            }
            result = 0;
        }
        //fwrite(buff,len,1,fd);

        if (len < UDPVIDEOPACKMAXSIZE && len>0)
        {
            buff[len] = '\0';
            strcpy(FromIP, inet_ntoa(c_addr.sin_addr));
            //printf("rcvfrom %s:%d:len = %d\n\r",
            //        inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port), len);
            if (g_is_monitor_flag == 0)
            {
                sendvideo_to_phone(buff,len);
            }
            if (g_phone_or_externdevice_holdon == 1 && g_phone_mont == 0)//手机没摘机，且不是手机监视
            {
                Talk_RecvVideoUpDown_Func(buff, FromIP, len);
            }
            else if (g_phone_or_externdevice_holdon == 0)//手机摘机
            {
                videoexit_1();
                videoexit_2();
                g_phone_or_externdevice_holdon = 2;//手机通话中
            }
            else
            {
                ;
            }
        }
        else
        {
            //printf("Udp video pack size larger than %d\n", UDPVIDEOPACKMAXSIZE);
            buff[len] = '\0';
        }
    }

    video_exit++;
}

//---------------------------------------------------------------------------
TempVideoNode1 * init_videonode(void) //初始化单链表的函数
{
    TempVideoNode1 *h; // *h保存表头结点的指针，*p指向当前结点的前一个结点，*s指向当前结点
    if((h=(TempVideoNode1 *)malloc(sizeof(TempVideoNode1)))==NULL) //分配空间并检测
    {
        printf("can not alloc memory\n");
        return NULL;
    }
    h->llink=NULL; //左链域
    h->rlink=NULL; //右链域
    return(h);
}

//---------------------------------------------------------------------------
//函数名称：creat
//功能描述：在链表尾部添加数据
//返回类型：无返回值
//函数参数： h:单链表头指针
int creat_videonode(TempVideoNode1 *h, struct _wrt_video_buf1 __wrt_video_buf)
{
	TempVideoNode1 *t;
	TempVideoNode1 *p;
	int j;
	int DataOk;
	int ret;
	if(h == NULL)
	return 0;
	t=h;
	//  t=h->next;
	while(t->rlink!=NULL)    //循环，直到t指向空
		t=t->rlink;   //t指向下一结点
	DataOk = 1;

	if(t&&(DataOk == 1))
	{
		if(__wrt_video_buf.size > 200000)
		{
			printf("__wrt_video_buf.size too large\n");
			return 0;
		}
		//尾插法建立链表
		if((p=(TempVideoNode1 *)malloc(sizeof(TempVideoNode1)))==NULL) //生成新结点s，并分配内存空间
		{
			printf("can not alloc memory\n");
			return 0;
		}
		
		p->Content.buffer=(unsigned char *)malloc(__wrt_video_buf.size);
		if(p->Content.buffer == NULL)
		{
			printf("can not alloc memory\n");
			return 0;
		}
		p->Content.isFull = 0;

		p->Content.frame_flag = __wrt_video_buf.DataType;
		p->Content.frameno = __wrt_video_buf.Frameno;
		p->Content.TotalPackage = 10;
		p->Content.timestamp = __wrt_video_buf.timestamp;

		p->Content.Len = __wrt_video_buf.size;
		memcpy(p->Content.buffer,__wrt_video_buf.buff, 
			__wrt_video_buf.size);// r_length - DeltaLen);
		p->Content.isFull = 1;
		p->rlink=NULL;    //p的指针域为空
		p->llink=t;
		t->rlink=p;       //p的next指向这个结点
		//    t=p;             //t指向这个结点
		return p->Content.isFull;
	}
}

//---------------------------------------------------------------------------
//函数名称：add
//功能描述：在链表尾部添加数据
//返回类型：无返回值
//函数参数： h:单链表头指针
int add_videonode(TempVideoNode1 *p, struct talkdata1 talkdata,
      unsigned char *r_buf , int r_length)
{
	int j;
	int DataOk;

	DataOk = 1;
	if((talkdata.DataType < 1) || (talkdata.DataType > 5))
		DataOk = 0;
	if(talkdata.Framelen > VIDEOMAX)
		DataOk = 0;
	if(talkdata.CurrPackage > talkdata.TotalPackage)
		DataOk = 0;
	if(talkdata.CurrPackage <= 0)
		DataOk = 0;
	if(talkdata.TotalPackage <= 0)
		DataOk = 0;
	if(DataOk == 1)
	{
		p->Content.frame_flag = talkdata.DataType;
		p->Content.frameno = talkdata.Frameno;
		p->Content.TotalPackage = talkdata.TotalPackage;
		p->Content.timestamp = talkdata.timestamp;
		p->Content.CurrPackage[talkdata.CurrPackage - 1] = 1;
		//      if(talkdata.CurrPackage == p->Content.TotalPackage)
		//        p->Content.Len =  (talkdata.CurrPackage - 1) * talkdata.PackLen + r_length - DeltaLen;
		//      p->Content.Len = talkdata.Framelen;
		memcpy(p->Content.buffer + (talkdata.CurrPackage - 1) * talkdata.PackLen,
		r_buf + DeltaLen, talkdata.Datalen);// r_length - DeltaLen);
		p->Content.isFull = 1;
		for(j=0; j< p->Content.TotalPackage; j++)
			if(p->Content.CurrPackage[j] == 0)
			{
				p->Content.isFull = 0;
				break;
			}

		return p->Content.isFull;
	}
}

//---------------------------------------------------------------------------
//函数名称：length
//功能描述：求单链表长度
//返回类型：无返回值
//函数参数：h:单链表头指针
int length_videonode(TempVideoNode1 *h)
{
	TempVideoNode1 *p;
	int i=0;         //记录链表长度
	if(h == NULL)
		return 0;
	p=h->rlink;
	while(p!=NULL)    //循环，直到p指向空
	{
		i=i+1;
		p=p->rlink;   //p指向下一结点
	}
	return i;
	//    printf(" %d",i); //输出p所指接点的数据域
}

//---------------------------------------------------------------------------
//函数名称：delete_
//功能描述：删除函数
//返回类型：整型
//函数参数：h:单链表头指针 i:要删除的位置
int delete_videonode(TempVideoNode1 *p)
{
	if(p != NULL)
	{
		//不为最后一个结点
		if(p->rlink != NULL)
		{
			(p->rlink)->llink=p->llink;
			(p->llink)->rlink=p->rlink;
			if(p->Content.buffer)
			{
				// if(Local._Decoce_Use_Acwd == 1)  //MJPEG 使用ACWD
				//   sfree(p->Content.buffer);
				//else
				free(p->Content.buffer);
                p->Content.buffer = NULL;
			}
			
			if(p)
				free(p);
            p = NULL;
		}
		else
		{
			(p->llink)->rlink=p->rlink;
			if(p->Content.buffer)
			{
				// if(Local._Decoce_Use_Acwd == 1)  //MJPEG 使用ACWD
				//   sfree(p->Content.buffer);
				// else
				free(p->Content.buffer);
                p->Content.buffer = NULL;
			}
			if(p)
				free(p);
            p = NULL;
		}
		return(1);
	}
	else
		printf("video delete null\n");
	return(0);
}

//---------------------------------------------------------------------------
int delete_all_videonode(TempVideoNode1 *h)
{
    return 0;
    TempVideoNode1 *p,*q;
    p = NULL;
    q = NULL;
    if(h == NULL)
        return 0;
    if (NULL == h->rlink)
    {
        printf("video list is NULL!\n");
        return 0;
    }
    else
    {
        p = h->rlink;        //此时p为首结点
    }
    while (NULL != p)   //找到要删除结点的位置
    {
        //不为最后一个结点
        q = p;
        if ((p->rlink != NULL) && (p->llink != NULL))
        {
            (p->rlink)->llink=p->llink;
            (p->llink)->rlink=p->rlink;
        }
        else
            (p->llink)->rlink=p->rlink;
        if (q != NULL)
        {
            if (q->Content.buffer != NULL)
            {         
                free(q->Content.buffer);
                p->Content.buffer = NULL;
            }
            free(q);
        }

        q = NULL;
        if (p->rlink != NULL)
        {
            p = p->rlink;
        }
        else
        {
            p = NULL;
        }
    }
}
//---------------------------------------------------------------------------
int delete_lost_videonode(TempVideoNode1 *h, uint32_t currframeno, uint32_t currtimestamp) //删除不全帧
{
	TempVideoNode1 *p,*q;
	if(h == NULL)
		return 0;
	p=h->rlink;        //此时p为首结点
	while(p != NULL)   //找到要删除结点的位置
	{
		//不为最后一个结点
		q = p;
		if(p->rlink != NULL)
		{
			//        if(p->Content.frameno < currframeno) //进入循环，直到p为空，或找到x
			if(p->Content.timestamp < currtimestamp)
			{
				(p->rlink)->llink=p->llink;
				(p->llink)->rlink=p->rlink;
				p = p->llink;
				if(q->Content.buffer)
				{
					// if(Local._Decoce_Use_Acwd == 1)  //MJPEG 使用ACWD
					//    sfree(q->Content.buffer);
					// else
					free(q->Content.buffer);
                    p->Content.buffer = NULL;
				}
				if(q)
					free(q);
                q = NULL;
				if(temp_video_n > 0)
					temp_video_n --;
			}
		}
		else
		{
			//        if(p->Content.frameno < currframeno) //进入循环，直到p为空，或找到x
			if(p->Content.timestamp < currtimestamp)
			{
				(p->llink)->rlink=p->rlink;
				p = p->llink;
				if(q->Content.buffer)
				{
					// if(Local._Decoce_Use_Acwd == 1)  //MJPEG 使用ACWD
					//   sfree(q->Content.buffer);
					// else
					free(q->Content.buffer);
                    p->Content.buffer = NULL;
				}
				if(q)
					free(q);
                q = NULL;
				if(temp_video_n > 0)
					temp_video_n --;
			}
		}
		p = p->rlink;
	}
	return 1;
}
//---------------------------------------------------------------------------
//函数名称：find_
//功能描述：查找函数
//返回类型：整型
//函数参数：h:单链表头指针 x:要查找的值
//查找该帧该包是否已存在


TempVideoNode1 * find_videonode(TempVideoNode1 *h, int currframeno, int currpackage)
{
	TempVideoNode1 *p;
	int PackIsExist; //数据包已接收标志
	int FrameIsNew;  //数据包是否是新帧的开始
	if(h == NULL)
		return 0;
	p=h->rlink;//此时p为首结点
	PackIsExist = 0;
	FrameIsNew = 1;
	while(p!=NULL)
	{
		if(p->Content.frameno == currframeno) //进入循环，直到p为空，或找到x
		{
			FrameIsNew = 0;
			if(p->Content.CurrPackage[currpackage - 1] == 1)
			{
#ifdef _DEBUG
				printf("video pack exist %d, p->Content.timestamp = %d\n", currframeno, p->Content.timestamp);
#endif
				PackIsExist = 1;
			}
			break;
		}
		p=p->rlink;   //s指向p的下一结点
	}
	if(p!=NULL)
		return p;
	else
		return NULL;
}


//---------------------------------------------------------------------------
//函数名称：find_
//功能描述：查找函数
//返回类型：整型
//函数参数：h:单链表头指针 x:要查找的值
//查找最老的帧
TempVideoNode1 * search_videonode(TempVideoNode1 *h)
{
	TempVideoNode1 *p;
	TempVideoNode1 *tem_p;

	if(h == NULL)
		return 0;
	tem_p = NULL;
	p=h->rlink;    //此时p为首结点
	//先取得tem_p, 不能直接取第一个，因为若第一个为不全帧，则会死循环
	while(p!=NULL)
	{
		if(p->Content.isFull == 1)
		{
			tem_p = p;
			break;
		}
		p=p->rlink;   //s指向p的下一结点
	}

	if(tem_p != NULL)
		while(p!=NULL)
		{
			if(p->Content.isFull == 1)
				if(p->Content.timestamp < tem_p->Content.timestamp) //进入循环，直到p为空，或找到x
				{
					tem_p = p;
				}
			p=p->rlink;   //s指向p的下一结点
		}

	return tem_p;

}
//---------------------------------------------------------------------------
int free_videonode(TempVideoNode1 *h)
{
	TempVideoNode1 *p,*t;
	int i=0;         //记录链表长度
	if(h == NULL)
		return 0;
	p=h->rlink;
	while(p!=NULL)    //循环，直到p指向空
	{
		i=i+1;
		t = p;
		p=p->rlink;   //p指向下一结点
		free(t);
	}
	return i;
}

void mDirIsExist(char *dirname)
{
	DIR *dirp;
	char cmd[1024];
	
	dirp = opendir(dirname);
	if(dirp == NULL)
	{
		int ret = mkdir(dirname,S_IRWXU);
		if(ret == 0)
			printf("create success\n");
	}
	if(dirp != NULL)
		closedir(dirp);
	dirp = NULL;
}

void openrecfile()
{
	uint32_t  times,dates;
	long _size,fsize;
	struct statfs vbuf;
	unsigned long day,Month,years ;
    unsigned long min,second,Hour;
    char filename[256];
	tm_get(&dates,&times,NULL);
    day= dates&0xff;
	Month = ((dates >> 8) &0xff);
	years = ((dates>>16) & 0xffff);
	Hour =  (times >> 16)& 0XFFFF;
	second =(times & 0xff);
	min =   (times >> 8) &0xff;
	mDirIsExist("/home/wrt/pic/video");
	if(g_dec_mode == 1)
    	sprintf((char*)filename,"/home/wrt/pic/video/%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".m4v");
    else if(g_dec_mode == 0)
    	sprintf((char*)filename,"/home/wrt/pic/video/%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".264");
	statfs("/",&vbuf);
	_size = (vbuf.f_bsize * vbuf.f_bfree);
	fsize = (float)_size/(1024*1024);
	if(fsize < 35)
	{
		g_video_rec_flag =0 ;
		//return;
	}
	
	if(g_video_rec_flag == 1)
	{
		sfd = NULL;
		sfd = fopen(filename,"wb");
		PHOTOINFO* pTemp =0;
	    pTemp =  get_idle_video();
	    if(pTemp == 0){
	            int i =1;
	            while(1){
	                    pTemp = get_select_video(MAX_VIDEO-i);
	                    if(pTemp){
	                            if(pTemp->idle == 1 && pTemp->lock == 0)
	                                    break;
	                    }
	                    i++;
	            }
	            if(pTemp)
	                    del_video(pTemp);
	            pTemp =  get_idle_video();
	    }
	    int itmp = 0;
	    
			if(m_pCmdHandler)
				itmp = m_pCmdHandler->GetIsDoorCall();
		
		if(g_dec_mode == 1)
			sprintf((char*)pTemp->name,"%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".m4v");
		else
			sprintf((char*)pTemp->name,"%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".264");
		pTemp->read  = 0;
		pTemp->read |= ((itmp <<16) & 0XFFFF0000);
		add_video(pTemp,NULL,0);
	}
}

char mp4EncHead[29] = {0x00,0x00,0x01,0xb0,0xf3,0x00,0x00,0x01,0xb5,0x09,0x00,0x00,0x01,\
			0x00,0x00,0x00,0x01,0x20,0x00,0x86,0xc4,0x00,0x7b,0x0c,0x2c,0x10,0x90,0x51,0x8f};
extern unsigned char g_copy_yuv_buf[152064];
extern int g_copy_yuv_flag;

extern int g_ipcamera_flag;
int g_playRecVideo = 0;
int m_isPlayvideo = 0;
extern int    g_localid ;
extern int g_ipcamera_flag;
//MPEG4解码线程
void *video_play_deal_thread_func(void *arg)
{
    int i, j, k;//, sub;
    int tmp;
    int m_rec_flag = 0;
    int m_len =0;
    int decode_cnt =0;
    int start_decode_flag = 0;
    TempVideoNode1 * tmp_videonode;
    int discard = 0;
    int *yuvFrameVirAddr;
    unsigned int yuvFramePhyAddr;
    WRT_VideoParam_t stVideoParam;

    uint32_t dellostframeno;
    uint32_t dellosttimestamp;

    //static FILE *yuvfp = NULL;
    //yuvfp = fopen("test.yuv","wb");

    g_video_rec_flag = 0;

    int checkcnt = 0;
    while (checkcnt < 6)
    {
        checkcnt++;
        if (get_vdec_type() == 0)
        {
            usleep(100*1000); //100ms
            continue;
        }
        else
        {
            break;
        }
    }

    printf("beal 11xxxxx6  vdec=%d\n", get_vdec_type());
    printf("beal 22xxxxx6  W=%d\n", get_vdec_width());
    printf("beal 33xxxxx6  h=%d\n", get_vdec_height());
    printf("beal 44xxxxx6  W=%d\n", get_venc_width());
    printf("beal 55xxxxx6  h=%d\n", get_venc_height());

    stVideoParam.VideoType = E_MI_VDEC_CODEC_TYPE_H264;
    stVideoParam.u16VideoW = get_vdec_width();
    stVideoParam.u16VideoH = get_vdec_height();
    stVideoParam.u16X = 0;
    stVideoParam.u16Y = 0;
    stVideoParam.u16DispW = 854; //1024x600 video disp area
    stVideoParam.u16DispH = 600;

    WRT_LocalVideoDecodeInit(&stVideoParam);

    if (get_vdec_type() >= V_H264)
    {
        g_dec_mode = 0;
    }
    else if (get_vdec_type() == V_NONE)
        return NULL;
    else
        g_dec_mode = 1;

    while(video_play_flag)
    {
        //等待采集线程有数据的信号, 测试用
        //等待UDP接收线程有数据的信号

        dellostframeno = 0;
        dellosttimestamp = 0;
        if(temp_video_n <= 2)
        {
            usleep(20000);
            if(video_play_flag)
                continue;
        }
        else
        {
            if(g_dec_mode)
            {
                if(temp_video_n > 5)
                    usleep(10000);
                else
                    usleep(18000);
            }
            else
            {   
                if(temp_video_n > 5)
                    usleep(10000);
                else
                    usleep(18000);
            }
        }
        if(temp_video_n > 0 && video_play_flag)
        {
            pthread_mutex_lock(&sync_s.video_play_lock);
            //查找最老的帧
            tmp_videonode = (TempVideoNode1 *)search_videonode(TempVideoNode_h);
            if(tmp_videonode == NULL)
            temp_video_n = 0;

            if((tmp_videonode != NULL)&&(tmp_videonode->Content.isFull == 1))
            {
                if(temp_video_n > 0)
                    temp_video_n --;
                dellostframeno = tmp_videonode->Content.frameno;
                dellosttimestamp = tmp_videonode->Content.timestamp;
                //printf("buffer =0x%x\n", tmp_videonode->Content.buffer[4]&0x7);
                //if (0 == start_decode_flag)
                //{
                //    if (0x7 == tmp_videonode->Content.buffer[4]&0x7)
                //    {
                //       WRT_SendVdecFrame(0, tmp_videonode->Content.buffer, tmp_videonode->Content.Len);
                //        start_decode_flag = 1;
                //    }
                //}
                //else
                {
                    WRT_SendVdecFrame(0, tmp_videonode->Content.buffer, tmp_videonode->Content.Len);
                }
                decode_cnt++;
                delete_videonode(tmp_videonode);

                //删除不全帧
                delete_lost_videonode(TempVideoNode_h, dellostframeno, dellosttimestamp);
            }
            pthread_mutex_unlock(&sync_s.video_play_lock);
        }

    }
    //printf("video decode thread  exit!!!!!!!!!!!!!!!!!!!!!!\n");
    video_exit++;
}

void video_start_thread_func()
{
    int i, j;
    pthread_attr_t attr;
    int EncodeType;
    int width, height;

    _wrt_video_buf.size = 0;
    _wrt_video_buf.timestamp = 110;
    _wrt_video_buf.seq_no = 0;
    _wrt_video_buf.buff =(unsigned char *)malloc(UDPVIDEOPACKMAXSIZE+1);
    if(_wrt_video_buf.buff == NULL)
        return;
    DeltaLen = 9 + sizeof(struct talkdata1);

    if(TempVideoNode_h == NULL)
        TempVideoNode_h = (TempVideoNode1 *)init_videonode();   //[视频流初始化]

    temp_video_n = 0;
    video_play_flag =1;
    //视频接收缓冲链表
    delete_all_videonode(TempVideoNode_h);

    videorec2playsem_flag = 1;

    if(pthread_mutex_init (&sync_s.video_play_lock, NULL) == -1)
        printf("cannot init sync_s.video_play_lock\n");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&video_play_deal_thread, &attr, video_play_deal_thread_func, NULL);
    if ( video_play_deal_thread <= 0 ) {
        printf("%s:%d:Create video play thread fail\n", __FUNCTION__, __LINE__);
        return;
    }
    pthread_attr_destroy(&attr);
}

typedef struct
{
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    unsigned max_size;            //! Nal Unit Buffer size
    int forbidden_bit;            //! should be always FALSE
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    int nal_unit_type;            //! NALU_TYPE_xxxx
    char *buf;                    //! contains the first byte followed by the EBSP
    unsigned short lost_packets;  //! true, if packet loss is detected
} H264Header;

static FILE *playfp = NULL;
static int info2;
static int info3;

H264Header *h264Frame = NULL;
char *filebuf = NULL;

static int findH264Header2 (unsigned char *buffer)
{
	if(buffer[0]!=0 || buffer[1]!=0 || buffer[2] !=1) 
		return 0;
	else 
		return 1;
}

static int findH264Heade3 (unsigned char *buffer)
{
    if(buffer[0]!=0 || buffer[1]!=0 || buffer[2] !=0 || buffer[3] !=1)
        return 0;
    else
        return 1;
}

static H264Header *allocH264Header(int buffersize)
{
	H264Header *header;

	if ((header = (H264Header*)calloc (1, sizeof (H264Header))) == NULL)
	{
		printf("Malloc memory failed\n");
		exit(0);
	}

	header->max_size=buffersize;

	if ((header->buf = (char*)calloc (buffersize, sizeof (char))) == NULL)
	{
		free(header);
		printf("Malloc memory failed\n");
		exit(0);
	}

	return header;
}

static void freeH264Header(H264Header * header)
{
    if(header)
    {
        if(header->buf)
        {
            free(header->buf);
            header->buf = NULL;
        }
        header->max_size = 0;
        free(header);
    }
}

static int bufseq = 0;
struct stat sbuf;

static int getNextH264Block(int mediatype,H264Header *nalu)
{
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *buf;
	
	int lastseq = 0;
	
	if(bufseq >0)
	{
		if(mediatype)//1 mp4
			lastseq = bufseq -3;
		else
			lastseq = bufseq -4;
	}

loop:
	if(mediatype)//mp4
	{
		nalu->startcodeprefix_len=3;

		info2 = findH264Header2 ((unsigned char *)&filebuf[bufseq]);
		if (info2 != 1)
		{
			;
		}
		else
		{
			 bufseq+= 3;
		}
		StartCodeFound = 0;
		info2 = 0;

		while (!StartCodeFound)
		{
			bufseq++;
			if(bufseq >= sbuf.st_size)
				return -10;
			info2 = findH264Header2((unsigned char *)&filebuf[bufseq-3]);

			StartCodeFound = (info2 == 1);
		}

		nalu->len = (bufseq -lastseq-3);
		
		memcpy (nalu->buf, filebuf+lastseq, nalu->len);

		return (bufseq -lastseq-3);
	}else
	{
		nalu->startcodeprefix_len=4;

		info3 = findH264Heade3 ((unsigned char *)&filebuf[bufseq]);
		if (info3 != 1)
		{
			;
		}
		else
		{
			 bufseq+= 4;
			 nalu->startcodeprefix_len = 4;
		}
		
		StartCodeFound = 0;
		info3 = 0;

		while (!StartCodeFound)
		{
			bufseq++;
			if(bufseq >= sbuf.st_size)
				return -10;
			info3 = findH264Heade3((unsigned char *)&filebuf[bufseq-4]);

			StartCodeFound = (info3 == 1);
		}

		nalu->len = (bufseq -lastseq-4);
		
		memcpy (nalu->buf, filebuf+lastseq, nalu->len);

		return (bufseq -lastseq-4);
	}
}

static int myCheckExtName(char *filename, char *ExtName)//检查文件扩展名
{
	int i;
	int dirlen;
	char extname[20];
	dirlen = strlen(filename);
	strcpy(extname, "");
	
	for(i=dirlen; i>=0; i--)
	{
		if(filename[i] == '.')
		{
			if((dirlen - i) < 20)
			{
				memcpy(extname, filename + i + 1, dirlen - i);//拷贝后缀名
				extname[dirlen - i] = '\0';
			}
			break;
		}
	}	
	if(strcmp(ExtName, extname)==0)
		return 1;
	else
		return 0;
}

extern int g_isrecplay_kit;
extern void video_rec_autoclose();

void* Play_recVideoThread(void *arg)
{
}

int  PlayRecvideo(void *arg)
{
	m_isPlayvideo = 1;
	char *filename = (char *)calloc(1,256);
	
	memset(filename,0,256);
	memcpy(filename,arg,strlen((const char *)arg));

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&play_recvideo_thread, &attr, Play_recVideoThread, filename);
	if ( play_recvideo_thread <= 0 ) {
		printf("无法创建视频录像播放数据处理线程\n");

		return -1;
	}
	pthread_attr_destroy(&attr);

}

void Talk_CreateUdpVideoSendThread(char *dstip)
{

}
static int m_sendvideo = 0;

int  Startsendvideo(void * arg)
{
/*
	InitVideoDevice();
	if(get_venc_type() >= V_H264)
		g_enc_mode = 0;
	else
		g_enc_mode = 1;
	g_send_run  = 1;
	Talk_CreateUdpVideoSendThread((char *)arg);
*/
}

void Stopsendvideo()
{
/*
	g_send_run = 0;
	if(talk_udpvideosendid > 0)
	{
		pthread_join(talk_udpvideosendid,NULL);
		printf("video send  cancel success\n");
	}
		
	sleep(1);
	g_enc_mode = 0;
	UnInitVideoDevice();
*/
}

int  Startplayvideo(void *arg)
{
    //return 0;
    int cnt = 3;

    //system("killall -9 openRTSP");
    while (cnt--)
    {

        if (videoisplay == E_VIDEO_DISPLAY_DEINIT)
        {
            video_exit = 0;

            if (g_phone_mont != 1) //1. Phone monitor
                video_start_thread_func(); //Non phone monitor

            InitUdpSocket(WRT_VIDEO_RECV_PORT);

            usleep(300*1000);
            videoisplay = E_VIDEO_DISPLAY_INIT;
            return 0;
        }
        usleep(200*1000);
    }

}

void videoexit_1()
{
    if(g_phone_mont == 1)
        return;
    if(g_phone_or_externdevice_holdon != 2)
    {
        video_play_flag = 0;
        temp_video_n = 0;
        if(video_play_deal_thread > 0)
        {
            pthread_join(video_play_deal_thread,NULL);
            printf("video_play_deal_func cancel success\n");
            video_play_deal_thread = 0;
        }else
            printf("video_play_deal_func is exit\n");
    }
}

void videoexit_2()
{
    if(g_phone_mont == 1)
        return;
    if(g_phone_or_externdevice_holdon != 2)
    {
        video_play_flag=0;
        videorec2playsem_flag = 0;

        pthread_mutex_destroy(&sync_s.video_play_lock);

        //视频接收缓冲链表
        delete_all_videonode(TempVideoNode_h);
        if(_wrt_video_buf.buff != NULL)
        {
            printf("free video BUF\n");
            free(_wrt_video_buf.buff);
        }
        WRT_LocalVideoDecodeExit();
        if(g_dec_mode == 0)
        {
            g_dec_mode =1; //在默认情况下本机支持MP4解码
        }
    }
}

void Stopplayvideo()
{
    //return;
    int cnt = 3;
    while(cnt--)
    {
        if (E_VIDEO_DISPLAY_INIT == videoisplay)
        {
            if(get_vdec_type() == V_NONE)
            {
                g_dec_mode = 1;
                videoisplay = E_VIDEO_DISPLAY_DEINIT;
                return;
            }
            printf("g_ipcamera_flag :%d\n",g_ipcamera_flag);

            if(g_ipcamera_flag == 1)
            {
                //system("killall -9 openRTSP");
                //pthread_join(onvif_thread,NULL);
                g_ipcamera_flag = 0;
            }
            if (Talk_m_VideoSocket > 0)
            {
                close(Talk_m_VideoSocket);
                Talk_m_VideoSocket = -1;
            }
            video_rcv_flag = 0; //stop recv video packet
            if (talk_udpvideorcvid >0)
            {
                pthread_join(talk_udpvideorcvid, NULL);
                printf("talk_udpvideorcvid cancel success\n");
                talk_udpvideorcvid = 0;
            }
            usleep(300);
            videoexit_1();
            videoexit_2();

            g_phone_mont = 0;
            g_exit_status = 0;
            videoisplay = E_VIDEO_DISPLAY_DEINIT;
            init_phone_video();
            break;
        }
        usleep(200*1000); //wait video display status to E_VIDEO_DISPLAY_INIT
    }
}

void *tcp_process_thread_func(void *arg)
{
	int sendlength;
	fd_set fds ;
	int ret;
	int run=1;
	int sock =Talk_tcpSocket;
	struct timeval tv ;
	//int Waitflag;
	char LocalDesc[16]="010100100100101";
	char RemoteDesc[16]="010101101101010";
	char rcvbuf[1024];
	char tmp_head[10];
	
	struct TALK_TMSG_HEADER tMsg_header;
	struct TALK_TMSG_HEADER* pMsgHeader;
	
	if(sock>0)
	{
		memcpy(tMsg_header.Head, "WRTI", 4);
		tMsg_header.Length = 40;               // 长度
		tMsg_header.Order = _SUB_HOST_WATCH;     //0x0020   //开始监视              分机->主机
		unsigned short tmp_order;
		tmp_order = tMsg_header.Order << 8;
		tmp_order |= ((tMsg_header.Order & 0xff00) >> 8);
		tMsg_header.Order = tmp_order;
		memcpy(tMsg_header.Src_Desc, LocalDesc, 15);		//本机描述
		memcpy(tMsg_header.Dest_Desc, RemoteDesc, 15);		 //目标描述
		sendlength = tMsg_header.Length;
		send(sock, (char*)(&tMsg_header), sendlength, 0);//发送信息
	}
	
	Startplayvideo(NULL);
#if 1
	FD_ZERO(&fds) ;
	FD_SET(sock, &fds) ;
	tv.tv_sec = 5;//RECV_OUT_TIME;
	tv.tv_usec = 0 ;
	
	while(videoisplay)
	{
		ret = select ( sock+1, &fds, NULL, NULL, &tv ) ;
		if(ret>0)
		{	if(FD_ISSET(sock, &fds))
			{
				ret = recv ( sock, rcvbuf, 1024, 0 ) ;
			}
		}
		else
		{
			usleep(100*1000);
		}
		if(ret == 40)
		{
			
			pMsgHeader = (struct TALK_TMSG_HEADER*)rcvbuf;
			memcpy(tmp_head, pMsgHeader->Head, 4);
			tmp_head[4] = '\0';
			unsigned short tmp_order;
			tmp_order = pMsgHeader->Order << 8;
			tmp_order |= ((pMsgHeader->Order & 0xff00) >> 8);

			pMsgHeader->Order = tmp_order;
			//printf("11111111111 %x\n",tmp_order);
			if(memcmp(tmp_head,"WRTI",4)==0)
			{
				switch(pMsgHeader->Order)
				{
					case _HOST_BUSY:
					case _HOST_SUB_DOOR_WATCHEND:
						//Stopplayvideo();
						//run =0;
						break;
					case _HOST_MONTACK:
						printf("Monitor Ack\n");
						break;
					default :
						break;
				}
			}
			
		}
	}	
	
	if(sock>0)
	{
		memcpy(tMsg_header.Head, "WRTI", 4);
		tMsg_header.Length = 40;               // 长度
		tMsg_header.Order = _SUB_HOST_WATCHEND;     //0x0021   //结束监视   分机->主机
		unsigned short tmp_order;
		tmp_order = tMsg_header.Order << 8;
		tmp_order |= ((tMsg_header.Order & 0xff00) >> 8);
		tMsg_header.Order = tmp_order;
		
		memcpy(tMsg_header.Src_Desc, LocalDesc, 15);		//本机描述
		memcpy(tMsg_header.Dest_Desc, RemoteDesc, 15);		 //目标描述
		sendlength = tMsg_header.Length;
		send(sock, (char*)(&tMsg_header), sendlength, 0);//发送信息
	}

#endif
}

void _Wrt_Monitor(int sock)
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tcp_thread, &attr, tcp_process_thread_func,NULL);
	if ( tcp_thread == 0 ) {
		printf("无法创建视频播放数据处理线程\n");
		
	//VideoPlayStarting = 0; //正在打开
		return;
	}
}

int Tcpsendcmd(char *remoteip,int remoteport,int cmd)
{
	int sockClient;
	struct linger Linger;

	struct sockaddr_in servaddr;
	int len = sizeof(struct sockaddr);

	sockClient = socket(AF_INET, SOCK_STREAM, 0);	//创建套接字
	if(sockClient <= 0)
		printf("create socket error\n");
	// bzero(&servaddr,sizeof(servaddr));
	Talk_tcpSocket = sockClient;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(remoteport);
	servaddr.sin_addr.s_addr = inet_addr(remoteip);
	
	Linger.l_onoff = 1;   // 开启 linger 设定
	Linger.l_linger = 1;  // 设定 linger 时间为 n 秒
	setsockopt(sockClient, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(struct linger));

	if(connect(sockClient, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{
		printf("connet error!\n");
		return -1;
	}
	
	switch(cmd)
	{
		case _SUB_HOST_WATCH:
			printf("wrt_monitor\n");
			_Wrt_Monitor(sockClient);
			break;
		default :
			break;
	}
	return sockClient;
}

void init_phone_video() //by ljw added
{
	int index =0;
	for(index= 0; index<MAX_PHONE;index++)
		g_video_phone_addr[index] = 0;
}

int start_phone_video2(unsigned long ip,int index)
{
	g_video_phone_addr[index] = ip;
}

int stop_phone_video2(int index) //(index 0~3),如果不再次范围，则表示全部停止。
{
	if(index > -1 && index < 4){
		g_video_phone_addr[index] = 0;
	}else{
		for(index= 0; index<MAX_PHONE;index++)
			g_video_phone_addr[index] = 0;
	}
}

#if 1
int videomain(int argc,char **argv)
{
	char ch;
	fd = fopen("udp.m4v","wb");
	sfd = fopen("mmm.m4v","wb");

	printf("monitor\n");
	while(1)
	{
		ch=getchar();
		
		switch(ch)
		{
			case 'c':
				printf("call\n");
				break;
			case 'm':
				printf("m2 %s\n",argv[1]);
				Tcpsendcmd(argv[1],20200,_SUB_HOST_WATCH);//
				break;
			case 's':
				printf("stop video\n");
				Stopplayvideo();
				break;
			case 'a':
				exit(0);
				break;
			case '1':
				Startplayvideo(NULL);
				Startsendvideo((void *)0x0100007F);
				break;
			case '2':
				Stopplayvideo();
				Stopsendvideo();
				break;
				
			default:
				break;
		}
	}
	printf("xxxxxx\n");
	//app_exit();

	return 0;
}
#endif
