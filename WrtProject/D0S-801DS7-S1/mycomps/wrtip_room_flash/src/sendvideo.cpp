#include <stdio.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include <semaphore.h>       //sem_t
#include <unistd.h>
//#include "common.h"
//#include "virtualDevice.h"
//#include "cxon2venc.h"
//#include "cmem.h"
//#include "ppapi.h"
#include <signal.h>
//#include "video.h"

//#include "videoDecoder.h"
//#include "circlequeue.h"
int image_encode_count;
//io_method g_io;
pthread_t threadID;
pthread_mutex_t smi_mutex;
pthread_cond_t  smi_cond;
int g_cap_width,g_cap_height;
int g_enc_width,g_enc_height;
static int gkeyframe = 0;
int g_enc_mode = 1;

int g_fps;
int g_bitrate;
int g_write;
int g_port;
char *dev_name;
int cap_fd;
struct buffer *         buffers;
unsigned int     	n_buffers;
struct timeval tv;
unsigned image_count;
int g_jpg_q;
char savebuf[512*1024];
char destip[16];
//FILE *h_fd;
//void *g_venc;
void *venc_handle;
static int encflag;
static int enccnt;
int sock_fd;

static int server_len;
char sendbuf[1440];
static unsigned int ts;

void OpenVideoDevice()
{
}

void InitVideoDevice()
{}

void UnInitVideoDevice()
{}
void StopVideoDevice()
{}

void * startH264Encode()
{}

void stopH264Encode()
{}

void *setupOn2(int outfmt)
{}

void open_device()
{}

void close_device()
{}

int xioctl(int  fd,  int  request, void * arg)
{}

void errno_exit  (const char *s)
{}

void initDevice()
{}

void uninitDevice()
{}

void initRead(unsigned int buffer_size)
{}

void initMmap()
{}

void initUserp(unsigned int  buffer_size)
{}

void startCapturing()
{}

void stopCapturing()
{}

//static int gg__mm = 0;

int Enc_One_Frame(void *h_venc,const void *p,char *outbuf)
{}



int Proc_One_Frame(void *h_venc,char *outbuf)
{
}
int FrameNo = 0;
int nowtime = 200;
char nalu_buf[1600];

int My_send(int m_Socket, unsigned long ip,int port,char *buf,int nlength)
{}
int g_send_run = 0;

int Wrt_send_video(int sock,char *sendbuf,int sendlen,unsigned long ip,int port)
{}


int g_send_my_video = 0;
void *sendvideo_task(void *args)
{}
