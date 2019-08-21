#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <sys/wait.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

#include "charutil.h"
#include "Handler.h"
#include "public_menu_res.h"
#include "mutilmedia_res.h"

#include "window.h"
#include "audiovideo.h"

static WRT_Rect rc_infobrowsermenuico[18]=
{   
        {32,50,504,40},
        {32,90,504,40},
        {32,130,504,40},
        {32,170,504,40},
        {32,210,504,40},
        {32,250,504,40},
        {32,290,504,40},
        {32,330,504,40},
        {32,370,504,40},   

        {900,44,104,76},           /*清空*/ 
        {900,127,104,76},            /*锁定*/  
        {900,217,104,76},            /*删除*/
        {900,309,104,76},            /*打开*/         
        {40,430,85,53},           /*上翻*/
        {400,430,85,53},            /* 下翻*/

        {900,531,111,69},              //返回
        {6,531,111,69},           //关屏幕	
        {900,399,104,76},            /*随机播放*/
        //{900,4,104,76},            /*打开*/         
};

static const WRT_Rect rc_mutilmediaico[5] =
{
	{233,175,132,160},              //视频
	{446,175,132,160},            // 音乐
	{659,175,132,160},            // 相片
	{900,531,111,69},              //返回
	{6,531,111,69}                 //关屏幕
};

static const WRT_Rect rc_mutilpicico[7] =
{
	{0,0,889,530},
	{900,44,104,76},           /*清空*/ 
    {900,127,104,76},            /*锁定*/  
    {900,217,104,76},            /*删除*/
    {900,309,104,76},            /*打开*/
    {900,531,111,69},              //返回
    {6,531,111,69},           //关屏幕	
};

WRT_Rect rc_infoplayvideoico[5]=
{   
        {900,44,104,76},           /*下一曲*/ 
        {900,127,104,76},            /*上一曲*/  
        {900,217,104,76},            /*播放暂停*/
        {900,309,104,76},            /*停止返回*/ 
        {0,0,1024,600},            /*全屏*/   
};

int g_rec_flag = 0;
#define USB1_PATH "/mnt/sda"//"/mnt/sda/wrt/test"  //路径命名根据实际路径修改
#define USB2_PATH "/mnt/sdb"
#define SD_PATH   "/mnt/mmc"
#define READ_BUF_SIZE 1024
#define MAX_FILE_NAME_LEN 1024
#define MEDIA_FIFO "/dev/my_media_mplayer_fifo"

char TempPath[1024];

#define MAX_FIlENAME_LEN 256
//文件内容结构体
struct DirContent1{
               int isValid;  //有效，未删除标志   1
               unsigned char attribute[4];  //文件属性   2
               char Time[32];    //创建时间    32
               int Type;     //类型        1  目录  2 普通文件  3 未知文件
               int MediaTpye;//1 video  2 audio 3 picture
               int Length;       //长度        4
               char Name[MAX_FIlENAME_LEN];//名称  100
               int Checked;
               int Locked;
              };                               //内存分配为444
//Dir链表
typedef struct node1{
               struct DirContent1 Content;
               struct node1 *llink, *rlink;
}DirNode1;

DirNode1 *DirNode_h;

enum
{
   SINGLE_LOOP =0,//单曲循环
   RANDOM_LOOP,//随机循环
   CIR_LOOP,//顺序循环
};

enum
{
   VIDEO =1,//单曲循环
   AUDIO,//随机循环
   PICTURE,//顺序循环
};
extern unsigned char menu_lingsheng1[7549];
extern unsigned char menu_lingsheng2[7747];
extern unsigned char menu_tupian1[8702];
extern unsigned char menu_tupian2[8659];


static int g_fd_mplayer_fifo = -1;
static int m_usb1_exist;
static int m_usb2_exist;
static int m_sd_exist;
static int m_mediatype = -1;
static int m_curr_select = -1;
static int m_mutilmedia_page = -1;
static int m_mutilmedia_curr_page = -1;
static int m_video_pid;
static int m_isAudioPlay = 0;
static int m_isMplayerRun = 0;
static int m_PlayMode = 0;
static int m_isHitKeyCancel = 0;
static int m_curr_play_audio_listno = -1;
pthread_t video_mplayer_thread = -1;
pthread_t audio_mplayer_thread = -1;

int g_is_doorcallroom = 0;
static int m_mplayer_video_flag;
static int m_mplayer_audio_flag;

sem_t music_mplay_sem;

extern void mDirIsExist(char *dirname);

char *m_DevName;
int ShowMp3Info(char *filename);

int MplayerAudioFunc(int play_seq);
int MplayerVideoFunc(int play_seq);

void ShowMediaFileList(int type);
void *Mplayer_audio_thread_fun(void *args);
void *Mplayer_video_thread_fun(void *args);


void PlayNextMusic();
void PlayPrevMusic();
int IsAudioPlayInfo(char *buf,int play_seq);
void DrawMutilmediaPic();
void DrawMutilmediaVideo();
void drawmutilmedia_func();

static pid_t get_pid_by_name(char *process);
int CheckExtName(char *filename, char *ExtName);
int GetRealName(char *filename, char *RealName);
int GetFileName(char *buf,int play_seq,int type);

static int convertutf8togb(const unsigned char *utf8,unsigned char *outbuf,int outbuflen);
static int convertgb2toutf8(const unsigned char * gbstr,unsigned char *outbuf, int outbuf_len);
int GetMediaCntByType(DirNode1 *h,int type);

DirNode1 * FindDirNode(DirNode1 *h,int seq,int Type);
int free_dirnode(DirNode1 *h);

int CheckExternDevice()
{
	FILE *ckfp = NULL;
	char buf[1024];
	int i=0;
	if(m_DevName == USB1_PATH)
	{
		ckfp = fopen("/dev/sda","rb");
	}
	else if (m_DevName == SD_PATH)
	{
		ckfp = fopen("/dev/mmcblk0","rb");
	}
	else 
		return -1;
		
	if(ckfp == NULL)
	{
		return -1;
	}
	else
	{
		
		fclose(ckfp);
		ckfp = NULL;
		return 0;
	}
}

int CheckExtName(char *filename, char *ExtName)    //检查文件扩展名
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

int GetRealName(char *filename, char *RealName)    //获取文件真实名称
{
	int i;
	int dirlen;
	int pos;
	dirlen = strlen(filename);
	pos = dirlen;
	for(i=dirlen; i>=0; i--)
		if(filename[i] == '.')
		{
			pos = i;
			break;
		}
	memcpy(RealName, filename, pos);//去后缀名
	RealName[pos] = '\0';
	return 1;
}

int GetShortName(char *filename, char *RealName)    //获取文件真实名称
{
	int i;
	int dirlen;
	int pos;
	dirlen = strlen(filename);
	pos = dirlen;
	for(i=dirlen; i>=0; i--)
		if(filename[i] == '/')
		{
			pos = i;
			break;
		}
	memcpy(RealName, filename+pos,dirlen-pos);//去后缀名
	RealName[pos] = '\0';
	return 1;
}

int DelCurrName(char *filename)    //获取文件真实名称
{
	int i;
	int dirlen;
	int pos;
	char tmp[1024];
	memcpy(tmp,filename,strlen(filename));
	dirlen = strlen(filename);
	memset(filename,0,strlen(filename));
	pos = dirlen;
	for(i=dirlen; i>=0; i--)
		if(tmp[i] == '/')
		{
			pos = i;
			break;
		}
	memcpy(filename, tmp,pos);//去后缀名
	filename[pos] = '\0';
	return 1;
}

//---------------------------------------------------------------------------
//函数功能 分配一个DirNode1
DirNode1 * init_dirnode(void) //初始化单链表的函数
{
	DirNode1 *h; // *h保存表头结点的指针，*p指向当前结点的前一个结点，*s指向当前结点
	if((h=(DirNode1 *)malloc(sizeof(DirNode1)))==NULL) //分配空间并检测
	{
		printf("不能分配内存空间!");
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
int creat_dirnode(DirNode1 *h, struct DirContent1  TmpContent)
{
	DirNode1 *t;
	DirNode1 *p;
	t=h;
	//  t=h->next;
	while(t->rlink!=NULL)    //循环，直到t指向空
		t=t->rlink;   //t指向下一结点
	if(t)
	{
		//尾插法建立链表
		if((p=(DirNode1 *)malloc(sizeof(DirNode1)))==NULL) //生成新结点s，并分配内存空间
		{
			printf("不能分配内存空间!\n");
			return 0;
		}
		//p->Content = TmpContent;
		p->Content.Locked = 0;
		memcpy(&(p->Content), &TmpContent, sizeof(struct DirContent1));
		p->rlink=NULL;    //p的指针域为空，此处是在链表尾插入新节点
		p->llink=t;
		t->rlink=p;       //p的next指向这个结点
		t=p;             //t指向这个结点
		return 1;
	}
}
//---------------------------------------------------------------------------
//函数名称：length
//功能描述：求单链表长度
//返回类型：无返回值
//函数参数：h:单链表头指针
int length_dirnode(DirNode1 *h)
{
	DirNode1 *p;
	int i=0;         //记录链表长度  right  left
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
//函数名称：insert
//功能描述：插入函数
//返回类型：整型
//函数参数：h:单链表头指针 x:要插入的元素 i：要插入的位置

//假若s,p,q是连续三个结点的指针，若我们要在p前插入一个新结点r，
//则只需把s的右链域指针指向r，r的左链域指针指向s，r的右链域指针指向p，p的左链域指针指向r即可。
int insert_dirnode(DirNode1 *h, struct DirContent1  TmpContent)
{
	DirNode1 *s;
	DirNode1 *p;
	if(h == NULL)
	return 0;
	if((s = (DirNode1 *)malloc(sizeof(DirNode1)))==NULL) //生成新结点s，并分配内存空间
	{
		printf("不能分配内存空间!\n");
		return 0;
	}
	TmpContent.Locked = 0;
	s->Content = TmpContent;        //把TmpContent赋值给s的数据域

	p = h->rlink;    //此时p为首结点
	if(p == NULL)
	{
		h->rlink = s;
		s->llink = h;
		s->rlink = p;
		//printf("TmpContent.Name = %s, p = NULL\n", TmpContent.Name);
	}
	else
		while(p != NULL)
		{
			//printf("p->Content.Name = %s, p->Content.Type = %d, TmpContent.Name = %s, TmpContent.Type = %d\n",
			//        p->Content.Name, p->Content.Type, TmpContent.Name, TmpContent.Type);
			if((TmpContent.Type < p->Content.Type)||
			((strcmp(p->Content.Name, TmpContent.Name) >= 0)&&(TmpContent.Type <= p->Content.Type))) //进入循环，直到p为空，或找到x
			{
				(p->llink)->rlink = s;
				s->llink = p->llink;
				s->rlink = p;
				p->llink = s;
				//printf("TmpContent.Name = %s, 1\n", TmpContent.Name);
				break;
			}
			if(p->rlink == NULL)
			{
				p->rlink = s;
				s->llink = p;
				s->rlink = NULL;
				//printf("TmpContent.Name = %s, 2\n", TmpContent.Name);
				break;
			}
			p = p->rlink;   //s指向p的下一结点
		}

	return(1);         //如果成功返回1
}

//---------------------------------------------------------------------------
//函数名称：delete_
//功能描述：删除函数
//返回类型：整型
//函数参数：h:单链表头指针 i:要删除的位置
int delete_dirnode(DirNode1 *p)
{
	//未锁定
	// if(p->Content.isLocked == 0)
	{
		//不为最后一个结点
		if(p->rlink != NULL)
		{
			(p->rlink)->llink=p->llink;
			(p->llink)->rlink=p->rlink;
			free(p);
		}
		else
		{
			(p->llink)->rlink=p->rlink;
			free(p);
		}
		return(1);
	}
	return(0);
}

//---------------------------------------------------------------------------
int delete_all_dirnode(DirNode1 *h)
{
	DirNode1 *p,*q;
	p=h->rlink;        //此时p为首结点
	while(p != NULL)   //找到要删除结点的位置
	{
		//不为最后一个结点
		q = p;
		if(p->rlink != NULL)
		{
			(p->rlink)->llink=p->llink;
			(p->llink)->rlink=p->rlink;
		}
		else
			(p->llink)->rlink=p->rlink;
		p = p->rlink;
		free(q);
	}
}

//---------------------------------------------------------------------------
//函数名称：locate_
//功能描述：定位函数
//返回类型：整型
//函数参数：h:单链表头指针 i:要定位的位置
DirNode1 * locate_dirnode(DirNode1 *h,int i)
{
	DirNode1 *p;
	int j;
	p=h->rlink;    //此时p为首结点
	j=1;
	
	while(p&&j<i)  //找到要定位的位置
	{
		++j;
		p=p->rlink;  //p指向下一结点
	}
	if(i>0&&j==i)
		return p;
	else
		return NULL;
}
//---------------------------------------------------------------------------
//函数名称：find_
//功能描述：查找函数
//返回类型：整型
//函数参数：h:单链表头指针 x:要查找的值
DirNode1 * find_dirnode(DirNode1 *h, struct DirContent1  TmpContent)
{
	DirNode1 *p;
	p=h->rlink;    //此时p为首结点
	while(p!=NULL&&(strcmp(p->Content.Name, TmpContent.Name)!=0)) //进入循环，直到p为空，或找到x
		p=p->rlink;   //s指向p的下一结点
	if(p!=NULL)
		return p;
	else
		return NULL;
}
//---------------------------------------------------------------------------
int free_dirnode(DirNode1 *h)
{
	DirNode1 *p,*t;
	int i=0;         //记录链表长度
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

//函数功能-查找对应Type类型在链表中的第seq个节点
DirNode1 * FindDirNode(DirNode1 *h,int seq,int Type)
{
	int i = 0;
	DirNode1 *p;
	p=h;    //此时p为首结点

	while(p->rlink!=NULL)//进入循环，直到p为空，或找到x
	{
		p=p->rlink; //s指向p的下一结点
		if(p->Content.MediaTpye == Type || p->Content.MediaTpye == 8)
		{
			if(i++ ==seq)
			{
				if(p->Content.Name != NULL)
				{
					return p;
				}
			}
		}
		
	}
	
	return NULL;
	
}


//刷新显示列表 --文件列表
void ShowFileList(DirNode1 *h, int Type)//文件显示方框（一级）
{
	int i;
	WRT_Rect curbox;
	int offset = 40;
	int isAlreadyDisp = 0;
	int tmp = 0;
	i = 0;
	int mediatypecnt = 0;
	char lastname[256];//如果播放音乐没有关闭，那么将查找到播放的音乐，并显示信息
	char showname[40];
	DirNode1 *tmpNode;
	tmpNode = init_dirnode();

	curbox.x = 10;
	curbox.y = 10;
	curbox.w = 530;
	curbox.h = 513;
	//DrawRect(curbox,2,rgb_white);//左框
	//DeflateRect(&curbox,4);
	FillRect(0xb2136,curbox);
	
	isAlreadyDisp = (m_mutilmedia_curr_page - 1)*9;
	//printf("aaaaaaaaaaa %d\n",isAlreadyDisp);
	//isAlreadyDisp 已经显示的文件个数
	for(i=0;i<9;i++)
	{
		tmpNode = FindDirNode(h,isAlreadyDisp+i,Type);
		if(tmpNode != NULL)
		{
			char gbbuf[256];
			char unibuf[256];
			char codebuf[20];
			memcpy(codebuf,tmpNode->Content.Name,5);
			//printf("name:%s %x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x_%x\n",tmpNode->Content.Name,codebuf[0],codebuf[1],codebuf[2],codebuf[3],codebuf[4],codebuf[5],codebuf[6],codebuf[7],codebuf[8],codebuf[9],codebuf[10],codebuf[11],codebuf[12],codebuf[13],codebuf[14],codebuf[15],codebuf[16],codebuf[17],codebuf[18],codebuf[19]);
			//memset(gbbuf,0,256);
            //memset(unibuf,0,256);
			//utf8ToUcs2((const unsigned char*)tmpNode->Content.Name,(unsigned short*)unibuf,256);          		
           // uni2gb((const unsigned short*)unibuf,(unsigned char*)gbbuf,256);
            //printf("xxxx %s\n",gbbuf);
			//unsigned short * ptext;
			//int  unilen = gb2uni((const unsigned char *)tmpNode->Content.Name,NULL,0);
		    //ptext =(unsigned short*) malloc((unilen+1)*sizeof(unsigned short));
		    //if(ptext == NULL)
		    //    return;
		    //memset(ptext,0,(unilen+1)*sizeof(unsigned short));
		    //gb2uni((const unsigned char *)tmpNode->Content.Name,ptext,unilen);
#if 1
		    int str_len;
		    if(Type == 1 || Type == 3)
		    {
			    str_len = GetTextWidth((char*)tmpNode->Content.Name,0);
			    if(str_len >800)
			    {
			    	memcpy(showname,tmpNode->Content.Name,30);
			    	showname[30]='\0';
			    	strcat(showname,"...");
			    }else
			    {
			    	strcpy(showname,tmpNode->Content.Name);
			    }
		    }
		    if(Type == 2)
		    {
		    	str_len = GetTextWidth((char*)tmpNode->Content.Name,0);
			    if(str_len >400)
			    {
			    	memcpy(showname,tmpNode->Content.Name,15);
			    	showname[15]='\0';
			    	strcat(showname,"...");
			    }else
			    {
			    	strcpy(showname,tmpNode->Content.Name);
			    }
		    }
#endif
			if(m_curr_select == i)
			{
				DrawText((char *)showname,140,50+i*offset+23,rgb_yellow);
				JpegDisp(70,50+i*offset,ico_select_2,sizeof(ico_select_2));
			}
			else
			{
				DrawText((char *)showname,140,50+i*offset+23,rgb_white);
				JpegDisp(70,50+i*offset,ico_select_1,sizeof(ico_select_1));
			}//
			switch(tmpNode->Content.MediaTpye)
			{
				case 1:
					DrawText(LT("视频列表"),200,40,rgb_yellow,24);
					JpegDisp(100,50+i*offset,shipingtubiao,sizeof(shipingtubiao));
					
	           		break;
	            case 2:
	            	DrawText(LT("歌曲列表"),200,40,rgb_yellow,24);
	           		JpegDisp(100,50+i*offset,ico_lingsheng,sizeof(ico_lingsheng));
					
	           		break;
	            case 3:
	            	DrawText(LT("图片列表"),200,40,rgb_yellow,24);
	            	if(m_curr_select == i)
						JpegDisp(100,50+i*offset,ico_tupian2,sizeof(ico_tupian2));
	           		else
	                    JpegDisp(100,50+i*offset,ico_tupian1,sizeof(ico_tupian1));
	            	break;
	            case 8:
	            	break;
	            	
	            default:
	            	printf("dircent flag\n");
	            	break;
			}
#if 0
            if(tmpNode->Content.Locked){ //锁定
            	JpegDisp(40,50+i*offset,ico_lock,sizeof(ico_lock));
            }
            else{
	            curbox.x = 40;
	            curbox.y = 50+i*offset;
	            curbox.w = 28;
	            curbox.h = 28;
	            FillRect(BACK_COLOR,curbox);
            }
#endif       
		}
		tmpNode =NULL;
	}

	if(Type == 2)
	{
		if(m_curr_play_audio_listno == -1)
			;
		else
			IsAudioPlayInfo(lastname,m_curr_play_audio_listno);
 	}
	if(m_mutilmedia_curr_page !=1 && m_mutilmedia_curr_page >0)
		JpegDisp(rc_infobrowsermenuico[13].x,rc_infobrowsermenuico[13].y,shangyiye_1,sizeof(shangyiye_1));

	if(m_mutilmedia_page>1 && m_mutilmedia_curr_page != m_mutilmedia_page)
		JpegDisp(rc_infobrowsermenuico[14].x,rc_infobrowsermenuico[14].y,xiayiye_1,sizeof(xiayiye_1));

	if(mediatypecnt > 0)
		DrawStatusbar(m_mutilmedia_page,m_mutilmedia_curr_page);
	
	//convertutf8togb((const unsigned char *)h->Content.Name,(unsigned char *)gbbuf,64);
}

//获取媒体类型type所对应的文件个数
int GetMediaCntByType(DirNode1 *h,int type)
{
	int i;
	i = 0;
	DirNode1 *p;
	p=init_dirnode();
	p= h;
	while(p->rlink!=NULL)
	{
		p=p->rlink;
		if(p->Content.MediaTpye == type || p->Content.MediaTpye == 8)
		{
			i++;
		}

	}
	return i;
}

void ShowMediaFileList(int type)//显示文件列表
{
	WRT_Rect rt;
	rt.x =0;
	rt.y =0;
	rt.w =889;
	rt.h =530;
	FillRect(0x00808080,rt);
	DeflateRect(&rt,7);
	FillRect(0xb2136,rt);
	if(type == 2)
	{
		rt.x =550;
		rt.y =0;
		rt.w =7;
		rt.h =530;
		FillRect(0x00808080,rt);
		DrawText(LT("顺序循环"),600,450,rgb_white,24);
	}
	//JpegDisp(0,0,bofang_bak,sizeof(bofang_bak));
	char namebuf[256];
	char gbbuf[256];
	int filecnt =0;
	int offset =40;
	int mediatypecnt = 0;
	mediatypecnt = GetMediaCntByType(DirNode_h,type);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));

	JpegDisp(rc_infobrowsermenuico[15].x,rc_infobrowsermenuico[15].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_infobrowsermenuico[16].x,rc_infobrowsermenuico[16].y,bt_guanping1,sizeof(bt_guanping1));

	if(mediatypecnt <1)
	{
		if(type == 1)
			DrawText(LT("没有视频"),80,77,rgb_white);
		else if(type == 2)
       		DrawText(LT("没有音乐"),80,77,rgb_white);
       	else if(type == 3)
       		DrawText(LT("没有图片..."),80,77,rgb_white);
       	else
       		;
        return;
	}

	m_mutilmedia_page = mediatypecnt/9;
    if(mediatypecnt%9)//如果取模不尽再加1
    	m_mutilmedia_page +=1;
    if(m_mutilmedia_page == 0){
            m_mutilmedia_page = 1;
            m_mutilmedia_curr_page = 1;
    }

	ShowFileList(DirNode_h,type);
}



void ListDirFunc(char *currdir)//显示目录下文件
{
	DIR *dirp;
	struct dirent * ptr;
	struct DirContent1 InitDirCon;

	if(DirNode_h == NULL)
		DirNode_h = init_dirnode();
	else
		delete_all_dirnode(DirNode_h);
		
	dirp =opendir(currdir);
	InitDirCon.Checked = 0;
	

	if(strcmp(currdir, "/") != 0)
	{
		//InitDirCon.Type = 0;
		//strcpy(InitDirCon.Name, "...");
		//creat_dirnode(DirNode_h, InitDirCon);
	}

	int music_num;
	
	if(dirp != NULL)
		while((ptr = readdir(dirp))!=NULL)
		{
			//char gbbuf[256];
			//
			//char unibuf[256];
			//int unilen = gb2uni((const unsigned char *)ptr->d_name,NULL,0);
			
			//unsigned short* unitext = (unsigned short*)malloc((unilen+1)*sizeof(unsigned short));
			//if(unitext == NULL)
			//	return 0;
		   	//memset(unibuf,0,256);
		    //gb2uni((const unsigned char *)ptr->d_name,(unsigned short*)unibuf,256); 
		    //ucs2ToUtf8((const unsigned short *)unibuf,(unsigned char *)gbbuf,256);
			//printf("file name %s\n",gbbuf);
			//WRT_DEBUG("utf8buf = %s ",ptr->d_name);
           // utf8ToUcs2((const unsigned char*)ptr->d_name,(unsigned short*)unibuf,256);
           // WRT_DEBUG("unibuf = %s ",unibuf);             		
           // uni2gb((const unsigned short*)unibuf,(unsigned char*)gbbuf,256);
           // WRT_DEBUG("gbbuf = %s ",gbbuf);
			InitDirCon.MediaTpye = 0;
			switch(ptr->d_type)
			{
				case DT_UNKNOWN:
					InitDirCon.Type = 3;
					strcpy(InitDirCon.Name, ptr->d_name);
					insert_dirnode(DirNode_h, InitDirCon);
					break;
				case DT_REG:
					InitDirCon.Type = 2;
					strcpy(InitDirCon.Name, ptr->d_name);
					
					if((CheckExtName(InitDirCon.Name, "mp3")==1)
						||(CheckExtName(InitDirCon.Name, "MP3")==1)
						||(CheckExtName(InitDirCon.Name, "wma")==1)
						||(CheckExtName(InitDirCon.Name, "WMA")==1)
					)
					{
						InitDirCon.MediaTpye = 2;
						music_num = length_dirnode(DirNode_h);
						if(music_num < 200)
							insert_dirnode(DirNode_h, InitDirCon); //20100305  目录及文件排序
						//creat_dirnode(DirNode_h, InitDirCon);
					}
					
					if((CheckExtName(InitDirCon.Name, "avi")==1)
						||(CheckExtName(InitDirCon.Name, "MP4")==1)
						||(CheckExtName(InitDirCon.Name, "mp4")==1)
						||(CheckExtName(InitDirCon.Name, "m4v")==1)
						||(CheckExtName(InitDirCon.Name, "M4V")==1)
						||(CheckExtName(InitDirCon.Name, "AVI")==1)
						||(CheckExtName(InitDirCon.Name, "vob")==1)
						||(CheckExtName(InitDirCon.Name, "VOB")==1)
						||(CheckExtName(InitDirCon.Name, "H264")==1)
						||(CheckExtName(InitDirCon.Name, "h264")==1)
						||(CheckExtName(InitDirCon.Name, "264")==1)
					)
					{
						InitDirCon.MediaTpye = 1;
						insert_dirnode(DirNode_h, InitDirCon); //20100305  目录及文件排序
						//creat_dirnode(DirNode_h, InitDirCon);
					}

					if((CheckExtName(InitDirCon.Name, "jpg")==1)
						||(CheckExtName(InitDirCon.Name, "JPG")==1)
						||(CheckExtName(InitDirCon.Name, "jpeg")==1)
						||(CheckExtName(InitDirCon.Name, "JPEG")==1)
						||(CheckExtName(InitDirCon.Name, "jpe")==1)
						||(CheckExtName(InitDirCon.Name, "JPE")==1)
					)
					{
						InitDirCon.MediaTpye =3;
						insert_dirnode(DirNode_h, InitDirCon); //20100305  目录及文件排序
						//creat_dirnode(DirNode_h, InitDirCon);
					}
					break;
				case DT_DIR:
					InitDirCon.MediaTpye =8;
					if((strcmp(ptr->d_name, ".")!=0)&&(strcmp(ptr->d_name, "..")!=0))
					{
						//InitDirCon.Type = 1;
						//memcpy(InitDirCon.Name,ptr->d_name,strlen(ptr->d_name));
						//strcpy(InitDirCon.Name, ptr->d_name);
						//insert_dirnode(DirNode_h, InitDirCon); 
						
					}
					break;
			}
		}
		
	if(dirp != NULL)
	{
		closedir(dirp);
		dirp = NULL;
	}
}

static pid_t get_pid_by_name(char *process)
{
    DIR *dir;
    struct dirent *next;
    long* pidList = NULL;
    int i = 0;
    char *pidName;
    pid_t retval;

    pidName = process;

    dir = opendir("/proc");
    if (!dir)
        printf("Cannot open /proc");

    while ((next = readdir(dir)) != NULL)
    {
    	FILE *status;
        char filename[READ_BUF_SIZE];
        char buffer[READ_BUF_SIZE];
        char name[READ_BUF_SIZE];

        if (strcmp(next->d_name, "..") == 0)
            continue;

        if (!isdigit(*next->d_name))
            continue;
        
	    sprintf(filename, "/proc/%s/status", next->d_name);
	        
	    if (! (status = fopen(filename, "r")) )
        {
            continue;
        }
        if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL)
        {
            fclose(status);
            continue;
        }
        fclose(status);

        sscanf(buffer, "%*s %s", name);
        if (strcmp(name, pidName) == 0)
        {
            pidList=(long *)malloc(sizeof(long) * (i+2));//xrealloc( pidList, sizeof(long) * (i+2));
            pidList[i++]=strtol(next->d_name, NULL, 0);
        }
    }
    if (pidList)
        pidList[i]=0;
    else
    {
        pidList=(long *)malloc(sizeof(long) * (i+2));//(long *)xrealloc( pidList, sizeof(long));
        pidList[0]=-1;
    }
	if(dir != NULL)
	{
		closedir(dir);
	}
    retval = (pid_t)pidList[0];
    free(pidList);
    return retval;
}

int g_video_pid = 0;
void exit_mplayer()
{
	pid_t mm_pid;
	char cmd[40];
	if(m_mplayer_audio_flag == 1)
	{
		m_mplayer_audio_flag = 0;
		
	}
	//mm_pid = get_pid_by_name("mplayer");
	//if(mm_pid < 0)
	//	return;
	//mm_pid = g_video_pid; 
	//sprintf(cmd,"kill -9 %d",mm_pid);
	//system(cmd);
	system("killall -9 mplayer");
	g_video_pid = 0;
	if(video_mplayer_thread > 0)
	{
		if(pthread_cancel(video_mplayer_thread) ==0)
			printf("video_mplayer_thread cancel success\n");
		else
			printf("video_mplayer_thread cancel fail\n");
	}
}

static int convertgb2toutf8(const unsigned char * gbstr,unsigned char *outbuf, int outbuf_len)
{

	int unilen = gb2uni(gbstr,NULL,0);
	unsigned short* unitext = (unsigned short*)malloc((unilen+1)*sizeof(unsigned short));
	if(unitext == NULL)
		return 0;
   	memset(unitext,0,(unilen+1)*sizeof(unsigned short));
    gb2uni(gbstr,unitext,unilen); 
    //ucs2ToUtf8(const unsigned short *ucs, unsigned char *cbuf, int cbuf_len);
    	//utf16_to_utf8(unitext,unilen,outbuf,outbuf_len);
 	free(unitext);
	return 1;
}

static int convertutf8togb(const unsigned char *utf8,unsigned char *outbuf,int outbuflen)
{
	int ucs2len;
	ucs2len = utf8ToUcs2(utf8,NULL,0);
	printf("utf8 len %d\n",ucs2len);
	unsigned char uc2buf[256];// = (unsigned short*)malloc((ucs2len+1)*sizeof(unsigned short));
	memset(uc2buf,0,(256));
	
	utf8ToUcs2(utf8,(unsigned short *)uc2buf,256);

	uni2gb((unsigned short *)uc2buf,outbuf,outbuflen);
}

char rfullname[256];

char *GetNextVideoName(DirNode1 *h,int currplay,int playmode)
{
	DirNode1 *tmpNode;
	tmpNode = init_dirnode();

loopVideo:
	
	currplay++;

	if(currplay >= GetMediaCntByType(h,1))
	{
			currplay = 0;
			m_curr_select = 0;
	}
	tmpNode = FindDirNode(h,currplay,1);
	if(tmpNode != NULL)
		;
	else
		return NULL;
	if(tmpNode->Content.MediaTpye != 1)
	{
		//printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		goto loopVideo;//return NULL;
	}
	//sprintf(rfullname,"%s/%s",m_DevName,tmpNode->Content.Name);
	sprintf(rfullname,"%s/%s",TempPath,tmpNode->Content.Name);

	return rfullname;
}

int mplayerstop()
{
    if(g_fd_mplayer_fifo != -1){
    	int ret;
    	while(1)
    	{
    		
    		ret = write(g_fd_mplayer_fifo,"quit\n",strlen("quit\n"));
			
    		if(errno == EINTR)
    			continue;
    		else
    			break;
    	}
    }
    
 	if(g_fd_mplayer_fifo != -1)
 	{
 		close(g_fd_mplayer_fifo);
		g_fd_mplayer_fifo = -1;
 	}
 	if(video_mplayer_thread > 0)
	{
		if(pthread_cancel(video_mplayer_thread) ==0)
			printf("video_mplayer_thread cancel success\n");
		else
			printf("video_mplayer_thread cancel fail\n");
	}
    return 1;
}

int pipedes[2];
pthread_t tid_pr; 

void *pipe_read_pthread(void *)  
{  
    int size;  
    char buf[256]; 
    FILE *fp;
    fp = fopen("text.txt","wb");
    while(1)  
    {  
        memset(buf, 0 , 256) ;  
        if((size = read(pipedes[0],buf,sizeof(buf))) == -1)//读取mplayer发过来的歌曲信息   
        {  
            perror("read pipe");  
            exit(1);  
        }  
        if( size == 0)//如果没有读到信息，则返回继续读取   
            continue;  
        buf[size]='/0';//使信息变成字符串，便于处理  
       // system("clear");
        printf("******************msg_buf=%s\n\n",buf);
        
        fwrite(buf,size+1,1,fp);
         //strcpy(msg_buf,buf);  
        //if(strncmp(buf,"ANS_META",8) ==0)                //获取歌曲信息   
        //{  
        //    buf[strlen(buf)-2]='/0';//多减一个去掉引号   
        //    msg_dispose(buf);  
        //}  
        //sem_post(&cmd_sem) ;  
       // SDL_Delay(5000);
    }  
    return NULL;  
}

void *Mplayer_video_thread_fun(void *args)
{
	int pid;
	int mm_pid;
	
	char *fullname;
	char bakname[256];
	char realname[256];
	char cmd[1024];
	fullname = (char *)malloc(256+1);
	//fullname = (char *)args;
	//memcpy(fullname,(const char *)args,strlen((const char *)args));
	strcpy(fullname,(char *)args); 
	//if(pipe(pipedes)==-1)//创建无名管道用于从mplayer读取歌曲信息   
    //{  
    //    perror("pipe");  
    //    exit(1);  
    //}  
	//pthread_create(&tid_pr,NULL,pipe_read_pthread,NULL);

#if 0
	if(g_fd_mplayer_fifo == -1)
	{
		unlink("/dev/my_mplayer_fifo");
		mkfifo("/dev/my_mplayer_fifo",O_CREAT |0666);                
		g_fd_mplayer_fifo = open("/dev/my_mplayer_fifo",O_RDWR);
		if(g_fd_mplayer_fifo == -1)
			//WRT_DEBUG("open /dev/my_mplayer_fifo failed \n");
		memset(cmd,0,1024);
		sprintf(cmd,"mplayer %s -slave -quiet -afm mp3lib -loop 1 -input file=/dev/my_mplayer_fifo \n",fullname);
		system(cmd);
		//printf("cmd %s\n",cmd);
		WRT_DEBUG("==================system return %d \n",errno);
		//close(g_fd_mplayer_fifo);
		//WRT_DEBUG("execlp return %d \n",errno);
		//g_fd_mplayer_fifo = -1;
	}
	
	
#endif
	
#if 1	
	if(g_fd_mplayer_fifo == -1)
	{
		unlink(MEDIA_FIFO);
		mkfifo(MEDIA_FIFO,O_CREAT |0666);                
		g_fd_mplayer_fifo = open(MEDIA_FIFO,O_RDWR);
		if(g_fd_mplayer_fifo == -1)
			WRT_DEBUG("open /dev/my_media_mplayer_fifo failed \n");
	}
loopvideo:
	//system("killall -9 mplayer");
	m_isHitKeyCancel = 0;
	pid = fork();
	if(g_rec_flag == 0)
	{
		if(CheckExternDevice()== -1)
		{
			CloseAllWindows();
			return NULL;
		}else
			;
	}	
	if(pid<0)
	{
		printf("fork error!\n");
	}
	else if(pid == 0)
	{
        SetVolume(pSystemInfo->mySysInfo.ringvolume-20);
		execl("/usr/bin/mplayer","mplayer","-slave","-quiet","-fs",fullname);
	}
	else
	{
		printf("father process\n");
	}
	g_video_pid = pid;
	wait(NULL);
	if(m_isHitKeyCancel == 1 || g_rec_flag == 1 || g_is_doorcallroom == 1)//按停止键退出mplayer
	{
		printf("主动退出\n");
		if(m_isHitKeyCancel != 1)
			CloseTopWindow();
	}else//播放完进程自动退出mplayer
	{
		printf("播放完毕退出,如果没按停止键，则循环播放\n");
		GetNextVideoName(DirNode_h,m_curr_select+(m_mutilmedia_curr_page -1)*9,m_PlayMode);
		m_curr_select++;
		if(m_curr_select>=9)
		{
			m_curr_select =0;
			m_mutilmedia_curr_page++;
			if(m_mutilmedia_curr_page >m_mutilmedia_page)
			{
				m_mutilmedia_curr_page =1;
			}
		}
		strcpy(fullname,rfullname);
		FILE *afp ;
		afp = fopen(fullname,"r");
		if(afp == NULL)
		{
			printf("Failed open audio file\n");
			return NULL;
		}
		fclose(afp);
		afp =NULL;
		
		goto loopvideo;
	}
#endif
	
	return NULL;
}

int IsVideoPlayInfo(char *buf,int play_seq)
{
    DirNode1 *tmpNode;
	
	char realname[256];
	tmpNode = init_dirnode();
	tmpNode = FindDirNode(DirNode_h,play_seq,1);
	if(tmpNode != NULL)
	{
		sprintf(buf,"%s/%s",TempPath,tmpNode->Content.Name);
	}
	else
		;
	if(tmpNode->Content.MediaTpye != 1)
	{
		printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		return 9;
	}
}

int MplayerVideoEventHandler(int x,int y,int status)
{
	WRT_xy xy;
    int ret = 0;
    int index =0;
    int pos_x,pos_y;
    xy.x = x;
    xy.y = y;
    for(index =0;index<5;index++){
            if(IsInside(rc_infoplayvideoico[index],xy)){
                    pos_x = rc_infoplayvideoico[index].x;
                    pos_y = rc_infoplayvideoico[index].y;
                    ret = 1;
                    if(status == 1)
                            g_rc_status[index] = true;
                    else{
                            if(g_rc_status[index] == false){
                                    ret = 0;
                                    return ret;
                            }
                            g_rc_status[index] = false;
                    }
                    pos_x = rc_infoplayvideoico[index].x;
                    pos_y = rc_infoplayvideoico[index].y;

                    switch(index)
                    {
                    		case 4:
                    		if(status == 1){
								;
							}else{
								printf("close video and exit mplayer\n");
								m_isHitKeyCancel = 1;
								//exit_mplayer();
								//mplayerstop();
								//SDL_Delay(100);
								StartButtonVocie();
								CloseTopWindow();
							}
                    		
                    		break;

                    		default :
                    			break;
                    }
            }
     }
}

void DrawMplayerVideoFunc()
{
	int ret = 0;
	int play_seq =0;
	//char fullname[256];
	WRT_Rect rt;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 600;
	FillRect(0xb2136,rt);
	
	pthread_attr_t attr;
	//memset(rfullname,0,256);
	
	play_seq = m_curr_select + (m_mutilmedia_curr_page-1)*9;
	ret = IsVideoPlayInfo(rfullname,play_seq);
	printf("video file name is %s\n",rfullname);
	
	if(ret == 9)
		return ;
	
	if(m_mplayer_audio_flag == 0)
	{
		//m_isAudioPlay = 1;
		m_mplayer_audio_flag = 1;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&video_mplayer_thread, &attr,Mplayer_video_thread_fun,(void *)rfullname);
		if(video_mplayer_thread < 0)
		{
			printf("create video mplayer thread failed\n");
		}
		pthread_attr_destroy(&attr);
	}
	//sleep(1);
}

static void CloseVideoWin()
{
	m_isHitKeyCancel = 1;
	
	exit_mplayer();

}

void CreateMplayerVideo()
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;   
	pWindow->EventHandler = MplayerVideoEventHandler;
	pWindow->NotifyReDraw = DrawMplayerVideoFunc;
	pWindow->valid = 1;
	pWindow->type = MUTILMEDIA;
	pWindow->CloseWindow = CloseVideoWin;
	m_mplayer_video_flag = 0;
	WindowAddToWindowList(pWindow); 
	pWindow->NotifyReDraw2(pWindow);
}

void DirIsExist(char *dirname)
{
	DIR *dirp;
	
	dirp = opendir(dirname);
	if(dirp == NULL)
	{
		int ret = mkdir(dirname,S_IRWXU);
		if(ret == 0)
			printf("create success\n");
	}
	if(dirp != NULL)
	{
		closedir(dirp);
	}
}

int delete_rec_video(int param)
{
	int del_seq;
	char cmd[1024];
	memset(cmd,0,1024);
	del_seq = m_curr_select +(m_mutilmedia_curr_page-1)*9;
	GetFileName(rfullname,del_seq,1);
	sprintf(cmd,"rm -rf %s",rfullname);
	printf("GetFileName %s\n",rfullname);
	system(cmd);
	system("sync");
	m_curr_select = -1;
	g_isUpdated =0;
	DrawMutilmediaVideo();
	g_isUpdated = 1;
	return 0;
}

int delete_all_rec_video(int param)
{
	char cmd[1024];
	memset(cmd,0,1024);
	DirIsExist("/home/wrt/pic/video");
	sprintf(cmd,"rm -rf /home/wrt/pic/video/*.*");
	system(cmd);
	system("sync");
	g_isUpdated = 0; 
	DrawMutilmediaVideo();
	g_isUpdated = 1;
	return 0;
}

int ConfirmInfo(int param)
{
	return 0;
}

int Copy_Data2SD(int param)
{
	int ret = -1;
	int seq = -1;
	char cmd[1024];
	seq = m_curr_select+(m_mutilmedia_curr_page-1)*9;
	ret = IsVideoPlayInfo(rfullname,seq);
	if(m_sd_exist == 1)
	{
		memset(cmd,0,1024);
		mDirIsExist("/mnt/mmc/video");
		sprintf(cmd,"cp %s /mnt/mmc/video",rfullname);
		system(cmd);
		
	}
	system("sync");
	return 0;
}

int Copy_Data2UDisk(int param)
{
	int ret = -1;
	int seq = -1;
	char cmd[1024];
	seq = m_curr_select+(m_mutilmedia_curr_page-1)*9;
	ret = IsVideoPlayInfo(rfullname,seq);
	if(m_usb1_exist== 1)
	{
		mDirIsExist("/mnt/sda/video");
		mDirIsExist("/mnt/sdb/video");
		memset(cmd,0,1024);
		sprintf(cmd,"cp %s /mnt/sda/video",rfullname);
		system(cmd);
		//memset(cmd,0,1024);
		//sprintf(cmd,"cp %s /mnt/sdb/video",rfullname);
		//system(cmd);
	}
	system("sync");
	return 0;
}

int MutilmediaVideoEventHandler(int x,int y,int status)
{
	WRT_xy xy;
    int ret = 0;
    int index =0;
    int pos_x,pos_y;
    xy.x = x;
    xy.y = y;
    for(index =0;index<18;index++){
            if(IsInside(rc_infobrowsermenuico[index],xy)){
                    pos_x = rc_infobrowsermenuico[index].x;
                    pos_y = rc_infobrowsermenuico[index].y;
                    ret = 1;
                    if(status == 1)
                            g_rc_status[index] = true;
                    else{
                            if(g_rc_status[index] == false){
                                    ret = 0;
                                    return ret;
                            }
                            g_rc_status[index] = false;
                    }
                    pos_x = rc_infobrowsermenuico[index].x;
                    pos_y = rc_infobrowsermenuico[index].y;
                    switch(index)
                    {
						case 9:/*清空*/
							if(g_rec_flag == 0)
								break;
							if(status == 1){
								StartButtonVocie();

								JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
							}else{

								JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));  
								if(g_rec_flag == 1)
									CreateDialogWin2(LT("你确定要清空所有录像"),MB_OK|MB_CANCEL,delete_all_rec_video,NULL);     
							}                                         
							break;
						case 10:/*删除*/
							if(g_rec_flag == 0)
								break;
							if(m_curr_select == -1)
							{
								CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL);//英文未翻译
								break;
							}
							if(status == 1){
								StartButtonVocie();

								JpegDisp(pos_x,pos_y,bt_shanchu2,sizeof(bt_shanchu2));
							}else{
								JpegDisp(pos_x,pos_y,bt_shanchu1,sizeof(bt_shanchu1));
								int del_seq = m_curr_select +(m_mutilmedia_curr_page-1)*9;
								if(del_seq >= GetMediaCntByType(DirNode_h,1))
								{
									CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
									break;
								}else
								{
									if(g_rec_flag == 1)
										CreateDialogWin2(LT("你确定要删除该录像"),MB_OK|MB_CANCEL,delete_rec_video,NULL);
								}

							}                    
							break;
						
							
						case 11:/*播放*/
							if(m_isAudioPlay == 1)
								break;
							if(m_curr_select == -1)
							{
								CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
								break;
							}
							if(status == 1){
								StartButtonVocie();
								JpegDisp(pos_x,pos_y,bt_bofang2,sizeof(bt_bofang2));
							}else{
								JpegDisp(pos_x,pos_y,bt_bofang1,sizeof(bt_bofang1));
								
							
								int ret = -1;
								int seq = -1;
								seq = m_curr_select+(m_mutilmedia_curr_page-1)*9;
								if(seq >= length_dirnode(DirNode_h))
									break;
								ret = IsVideoPlayInfo(rfullname,seq);
								
								if(ret == 9)//已经没有文件夹了
								{
									//char tmp[256];
									
									//GetShortName(rfullname,tmp);
									//strcat(TempPath,tmp);
                        			
                        			//printf("open file %s",TempPath);
                        			//CreateMutilmediaSub();//
                        			//CreateDialogWin2("您选择的是文件夹",MB_OK,NULL,NULL);
								}
								else
								{
									mp3stop();
									CreateMplayerVideo();
								}
							}

							break;
							case 12:/*拷贝到U Disk*/
							//if(m_isAudioPlay == 1||g_rec_flag == 0)
							//	break;
							if(m_usb1_exist ==1 && g_rec_flag ==1)
							{
								if(m_curr_select == -1)
								{
									CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
									break;
								}
								if(status == 1){
									StartButtonVocie();

									JpegDisp(pos_x,pos_y,usb_beifen_2,sizeof(usb_beifen_2));
								}else{
									JpegDisp(pos_x,pos_y,usb_beifen_1,sizeof(usb_beifen_1));
									CreateDialogWin2(LT("拷贝数据到U盘..."),MB_OK|MB_CANCEL,Copy_Data2UDisk,NULL);
								}
							}
							break;
							case 17:/*拷贝到SD Card*/
							if(m_sd_exist ==1 && g_rec_flag ==1)
							{
								if(m_curr_select == -1)
								{
									CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
									break;
								}
								
								if(status == 1){
									StartButtonVocie();

									JpegDisp(pos_x,pos_y,sd_beifen_2,sizeof(sd_beifen_2));
								}else{
									JpegDisp(pos_x,pos_y,sd_beifen_1,sizeof(sd_beifen_1));
									CreateDialogWin2(LT("拷贝数据到SD卡..."),MB_OK|MB_CANCEL,Copy_Data2SD,NULL);
								}
							}
							break;
							case 13:/*上翻*/
							if(m_mutilmedia_curr_page == 1)
								break;
							if(status == 1){
								StartButtonVocie();
								JpegDisp(pos_x,pos_y,shangyiye_2,sizeof(shangyiye_2));
							}else{
								JpegDisp(pos_x,pos_y,shangyiye_1,sizeof(shangyiye_1));
								m_mutilmedia_curr_page--;
								m_curr_select = -1;
								g_isUpdated = 0;
								ShowMediaFileList(m_mediatype);
								update_rect(0,0,600,530);
								g_isUpdated = 1;
							}
							break;
							case 14:/*下翻*/
							if(m_mutilmedia_page == m_mutilmedia_curr_page)
								break;

							if(status == 1){
								StartButtonVocie();
								JpegDisp(pos_x,pos_y,xiayiye_2,sizeof(xiayiye_2));
							}else{
								JpegDisp(pos_x,pos_y,xiayiye_1,sizeof(xiayiye_1)); 
								m_mutilmedia_curr_page++;
								m_curr_select = -1;
								g_isUpdated = 0;
								ShowMediaFileList(m_mediatype);
								update_rect(0,0,600,530);
								g_isUpdated = 1;
							}
							break;
										
                    	case 15://返回
	                        if(status == 1){
	                                StartButtonVocie();
	                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
	                        }else{
	                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
	                                CloseTopWindow();
	                        }                   	
	                        break;
		                case 16://关屏幕
	                        if(status == 1){
	                                StartButtonVocie();
	                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
	                        }else{
	                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
	                                screen_close();
	                                CloseAllWindows();
	                        }
	                        break;
		                default:
		                	if(status == 1)
		                	{
		                		;
		                	}else
		                	{
			                	m_curr_select = index;
			                	
			                	g_isUpdated = 0;
			                	ShowMediaFileList(m_mediatype);
			                	//StartButtonVocie();
			                	if(g_rec_flag == 1)
								{
									int tmp = GetMediaCntByType(DirNode_h,1);
									if(tmp >40)
									{
										;//DrawText(LT2("Rec vid Warning"),200,580,rgb_red,32);
									}
								}
			                	update_rect(0,0,889,600);
	                            g_isUpdated = 1;
	                            
							}
		                    break;
                    }
                    break;
   			}
   	}
}

void dir_file_count()
{
	DIR *dirp;
	struct dirent * ptr;
	int count = 0;
	dirp = opendir(USB1_PATH);
	if(dirp != NULL)
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count >2) // . .. 2个文件夹
		{
			m_usb1_exist =1;
			
		}
		
	}
	if(dirp != NULL)
		closedir(dirp);
	ptr = NULL;
	dirp = NULL;

	count = 0;
	dirp = opendir(USB2_PATH);
	if(dirp != NULL)
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count >2) // . .. 2个文件夹
		{
			m_usb2_exist =1;
		}
		
	}
	if(dirp != NULL)
		closedir(dirp);
	ptr = NULL;
	dirp = NULL;
	
	count = 0;
	dirp = opendir(SD_PATH);
	if(dirp != NULL)
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count >2) // . .. 2个文件夹
		{
			m_sd_exist =1;
			
		}
	}
	if(dirp != NULL)
		closedir(dirp);
	ptr = NULL;
	dirp = NULL;
}

void DrawMutilmediaVideo()
{
	WRT_Rect rt;
	int xoffset = 40;
	m_curr_select = -1;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	m_curr_select = -1;
	ListDirFunc(TempPath);//  ../xxx/video
	
	JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

	if(g_rec_flag == 1)
	{
		JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
		JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));
		dir_file_count();
	}
	
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_bofang1,sizeof(bt_bofang1));
	if(m_usb1_exist ==1 && g_rec_flag ==1)
	{
		JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,usb_beifen_1,sizeof(usb_beifen_1)); 
	}
	if(m_sd_exist == 1 && g_rec_flag ==1)
	{
		JpegDisp(rc_infobrowsermenuico[17].x,rc_infobrowsermenuico[17].y,sd_beifen_1,sizeof(sd_beifen_1)); 
	}
	ShowMediaFileList(VIDEO);
}

void CreateMutilmediaVideo()
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return;
	pWindow->EventHandler = MutilmediaVideoEventHandler;
	pWindow->NotifyReDraw = DrawMutilmediaVideo;
	pWindow->valid = 1;
	pWindow->type = CHILD;//
	m_mutilmedia_page = 1;
	m_mutilmedia_curr_page =1;
	m_curr_select = -1;
	m_mediatype = VIDEO;//视频类型
	m_usb1_exist = 0;
	m_usb2_exist = 0;
	m_sd_exist =0;
	//m_curr_play_audio_listno = -1;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

//===========================Video end==========

//=======================================================
//=====================================
//==================Audio Part
static int g_music_pid =0;
void exit_audio_play()//
{
	pid_t mm_pid;
	char cmd[40];
	#if 1
	WRT_Rect rt;
	rt.x = 558;
	rt.y = 16;
	rt.w = 310;
	rt.h = 500;
	FillRect(0xb2136,rt);

	#endif

	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,shangyishou_1,sizeof(shangyishou_1));//上一曲
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,xiayishou_1,sizeof(xiayishou_1));//下一曲
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_tingzhi1,sizeof(bt_tingzhi1));//停止
	JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));//播放

rekill:
	m_isHitKeyCancel = 1;
	
	//mm_pid = get_pid_by_name("mplayer");
	//if(mm_pid <0 )
	//	return ;
	//
	//mm_pid = g_music_pid;
	//g_music_pid =0;
	//sprintf(cmd,"kill -9 %d",mm_pid);
	//system(cmd);
	//mplayerstop();
	system("killall -9 mplayer");
	if(audio_mplayer_thread>0)
	{
		if(pthread_cancel(audio_mplayer_thread) ==0)
		    printf("music_mplay_thread cancel success\n");
	    else
			printf("music_mplay_thread cancel fail\n");
	}
	//mm_pid = get_pid_by_name("mplayer");
	//if(mm_pid < 0)
	//	;
	//else
	//	goto rekill;
}

void exit_audio_play2()
{
	pid_t mm_pid;
	char cmd[40];
	
	WRT_Rect rt;
	rt.x = 558;
	rt.y = 16;
	rt.w = 310;
	rt.h = 500;
	FillRect(0xb2136,rt);

	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,shangyishou_1,sizeof(shangyishou_1));//上一曲
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,xiayishou_1,sizeof(xiayishou_1));//下一曲
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_tingzhi1,sizeof(bt_tingzhi1));//停止
	JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));//播放
	
	m_curr_play_audio_listno = -1;
	
	m_isHitKeyCancel = 1;
rekill:
	//mm_pid = get_pid_by_name("mplayer");
	//if(mm_pid < 0)
	//	return ;
	//mm_pid = g_music_pid;
	//g_music_pid =0;
	//sprintf(cmd,"kill -9 %d",mm_pid);
	//system(cmd);
	
	//mplayerstop();
	system("killall -9 mplayer");
	
	if(audio_mplayer_thread>0)
	{
		if(pthread_cancel(audio_mplayer_thread) ==0)
		    printf("music_mplay_thread cancel success\n");
	    else
			printf("music_mplay_thread cancel fail\n");
	}
	//mm_pid = get_pid_by_name("mplayer");
	//if(mm_pid < 0)
	//	;
	//else
	//	goto rekill;
}

void PlayNextMusic()
{
	int ret = 0;
	exit_audio_play();
loopnext:
	m_curr_play_audio_listno++;
	if(m_curr_play_audio_listno >=GetMediaCntByType(DirNode_h,2) )
		m_curr_play_audio_listno = 0;
	//SDL_Delay(100);
	//StartButtonVocie();
	//SDL_Delay(100);
	//printf("xxxxxxxx%d\n",m_curr_play_audio_listno);
	m_mutilmedia_curr_page = m_curr_play_audio_listno /9;
	m_curr_select = m_curr_play_audio_listno %9;
	m_mutilmedia_curr_page = m_mutilmedia_curr_page+1;
	//printf("xxxxxxxx%d %d %d\n",m_curr_play_audio_listno,m_mutilmedia_curr_page,m_curr_select);
	g_isUpdated = 0;
	ShowFileList(DirNode_h,2);
	update_rect(0,0,600,530);
	g_isUpdated = 1;
	ret = MplayerAudioFunc(m_curr_play_audio_listno);
	if(ret == 9)
		goto loopnext;
}

void PlayPrevMusic()
{
	int ret = 0;
	exit_audio_play();
loopprev:
	m_curr_play_audio_listno-- ;
	if(m_curr_play_audio_listno <0)
		m_curr_play_audio_listno = GetMediaCntByType(DirNode_h,2)-1;
	//SDL_Delay(100);
	//StartButtonVocie();
	//SDL_Delay(100);
	m_mutilmedia_curr_page = m_curr_play_audio_listno /9;
	m_curr_select = m_curr_play_audio_listno %9;
	m_mutilmedia_curr_page = m_mutilmedia_curr_page+1;
	//printf("xxxxxxxx%d %d %d\n",m_curr_play_audio_listno,m_mutilmedia_curr_page,m_curr_select);
	g_isUpdated = 0;
	ShowFileList(DirNode_h,2);
	update_rect(0,0,600,530);
	g_isUpdated = 1;
	ret = MplayerAudioFunc(m_curr_play_audio_listno);
	
	if(ret == 9)
		goto loopprev;
		
}

void PlayRandomMusic() //没做向上或向下随机规定
{
	int ret = 0;
	exit_audio_play();
looprandom:
	int audio_file_cnt= 0;
	audio_file_cnt = GetMediaCntByType(DirNode_h,2);
	m_curr_play_audio_listno = rand()%audio_file_cnt;
	//SDL_Delay(100);
	//StartButtonVocie();
	//SDL_Delay(100);
	m_mutilmedia_curr_page = m_curr_play_audio_listno /9;
	m_curr_select = m_curr_play_audio_listno %9;
	g_isUpdated = 0;
	//if(m_mutilmedia_curr_page <=0 )
	//	m_mutilmedia_curr_page =1;
	m_mutilmedia_curr_page = m_mutilmedia_curr_page+1;
	//printf("xxxxxxxx%d %d %d\n",m_curr_play_audio_listno,m_mutilmedia_curr_page,m_curr_select);
	ShowFileList(DirNode_h,2);
	update_rect(0,0,600,530);
	g_isUpdated = 1;

	ret = MplayerAudioFunc(m_curr_play_audio_listno);
	
	if(ret == 9)
		goto looprandom;

}

char * GetRandomAudioName(DirNode1 *h)
{
	DirNode1 *tmpNode;
	//char fullname[256];
	char realname[256];
	memset(rfullname,0,256);
#if 1
	WRT_Rect rt;
	rt.x = 558;
	rt.y = 16;
	rt.w = 310;
	rt.h = 500;
	FillRect(0xb2136,rt);
	g_isUpdated = 0;
	update_rect(600,100,150,50);
    g_isUpdated = 1;
#endif

looplabel:

	int audio_file_cnt = 0;
	int random_play_seq= 0;
	audio_file_cnt = GetMediaCntByType(h,2);
	random_play_seq = rand()%audio_file_cnt;
	m_curr_play_audio_listno = random_play_seq;
	tmpNode = FindDirNode(DirNode_h,m_curr_play_audio_listno,2);
	if(tmpNode != NULL)
		;
	else
		return NULL;
	if(tmpNode->Content.MediaTpye != 2)
	{
		//printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		goto looplabel;//return NULL;
	}
	//sprintf(rfullname,"%s/%s",m_DevName,tmpNode->Content.Name);
	sprintf(rfullname,"%s/%s",TempPath,tmpNode->Content.Name);
	GetRealName(tmpNode->Content.Name, realname);

	DrawText((char *)realname,610,150,rgb_white);
	DrawText(LT("正在播放"),640,40,rgb_white,24);
	ShowMp3Info(rfullname);
	
	return rfullname;

}

char * GetNextAudioName(DirNode1 *h)
{
	DirNode1 *tmpNode;
	tmpNode = init_dirnode();
	memset(rfullname,0,256);
	char realname[256];
	
	WRT_Rect rt;
	rt.x = 558;
	rt.y = 16;
	rt.w = 310;
	rt.h = 500;
	FillRect(0xb2136,rt);
	g_isUpdated = 0;
	update_rect(600,100,150,50);
    g_isUpdated = 1;

	
looplabel:
	int audio_file_cnt = 0;
	m_curr_play_audio_listno ++;
	audio_file_cnt = GetMediaCntByType(h,2);
	if(m_curr_play_audio_listno >= audio_file_cnt)
		m_curr_play_audio_listno = 0;
	tmpNode = FindDirNode(DirNode_h,m_curr_play_audio_listno,2);
	if(tmpNode != NULL)
		;
	else
		return NULL;
	if(tmpNode->Content.MediaTpye != 2)
	{
		//printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		goto looplabel;//return NULL;
	}
	//sprintf(rfullname,"%s/%s",m_DevName,tmpNode->Content.Name);
	sprintf(rfullname,"%s/%s",TempPath,tmpNode->Content.Name);
	GetRealName(tmpNode->Content.Name, realname);

	//DrawText((char *)realname,610,150,rgb_white);
	int str_len;
    char showname[30];
    str_len = GetTextWidth((char*)realname,0);
    if(str_len >250)
    {
    	memcpy(showname,realname,19);
    	showname[19]='\0';
    	strcat(showname,"...");
    }else
    {
    	strcpy(showname,realname);
    }
	int disx = (230-str_len)/2;
	if(disx < 0)
		disx = 0;
	DrawText((char *)showname,590+disx,150,rgb_white);
	DrawText(LT("正在播放"),680,70,rgb_white,24);
	ShowMp3Info(rfullname);
	//sleep(2);
	return rfullname;
}

int ShowMp3Info(char *filename)
{
	FILE *mp3fd;
	char pbuf[128];
	char buf[64];
	char showbuf[128];
	int str_len;
    char showname[30];
	mp3fd = fopen(filename,"rb");
	if(mp3fd==NULL)
		printf("Failed to open mp3.\n");

	memset(pbuf,0,sizeof(pbuf));
	fseek(mp3fd,-128,SEEK_END);
	fread(pbuf,1,128,mp3fd);
	fclose(mp3fd);
	if(!((pbuf[0] == 'T'|| pbuf[0] == 't')
		&&(pbuf[1] == 'A'|| pbuf[1] == 'a')
		&&(pbuf[2] == 'G'|| pbuf[0] == 'g'))
	)
	{
		DrawText(LT("歌名:未知"),600,280,rgb_white,16);
		DrawText(LT("作者:未知"),600,320,rgb_white,16);
		DrawText(LT("专辑名:未知"),600,360,rgb_white,16);
		printf("Not a standard MP3 format.\n");
		return 1;
	}

	memset(buf,0,64);
	memcpy(buf,pbuf,3);
	buf[3]='\0';
	//printf("标识:%s\n",buf);
	
	memset(buf,0,64);
	memcpy(buf,pbuf+3,30);
	buf[30]='\0';
	sprintf(showbuf,"%s:%s",LT("歌名"),buf);
    str_len = GetTextWidth((char*)showbuf,1);
    if(str_len >150)
    {
    	memset(showname,0,30);
    	memcpy(showname,showbuf,25);
    	showname[25]='\0';
    	strcat(showname,"...");
    }else
    {
    	strcpy(showname,showbuf);
    	//memcpy(showname,showbuf,30);
    }
	DrawText(showname,600,280,rgb_white,16);

	memset(buf,0,64);
	memcpy(buf,pbuf+33,30);
	buf[63]='\0';
	sprintf(showbuf,"%s:%s",LT("作者"),buf);
	str_len = GetTextWidth((char*)showbuf,1);
    if(str_len >150)
    {
    	memset(showname,0,30);
    	memcpy(showname,showbuf,25);
    	showname[25]='\0';
    	strcat(showname,"...");
    }else
    {
    	strcpy(showname,showbuf);
    	//memcpy(showname,showbuf,30);
    }
	DrawText(showname,600,320,rgb_white,16);
	
	
	memset(buf,0,64);
	memcpy(buf,pbuf+63,30);
	buf[93]='\0';
	sprintf(showbuf,"%s:%s",LT("专辑名"),buf);
	str_len = GetTextWidth((char*)showbuf,1);
    if(str_len >150)
    {
    	memset(showname,0,30);
    	memcpy(showname,showbuf,25);
    	showname[25]='\0';
    	strcat(showname,"...");
    }else
    {
    	strcpy(showname,showbuf);
    	//memcpy(showname,showbuf,30);
    }
	DrawText(showname,600,360,rgb_white,16);
	
	
	memset(buf,0,64);
	memcpy(buf,pbuf+93,4);
	buf[97]='\0';
	sprintf(showbuf,"%s:%s",LT("年份"),buf);
	str_len = GetTextWidth((char*)showbuf,1);
    if(str_len >150)
    {
    	memset(showname,0,30);
    	memcpy(showname,showbuf,25);
    	showname[25]='\0';
    	strcat(showname,"...");
    }else
    {
    	strcpy(showname,showbuf);
    	//memcpy(showname,showbuf,30);
    }
	DrawText(showname,600,390,rgb_white,16);

	
	memset(buf,0,64);
	memcpy(buf,pbuf+97,28);
	buf[125]='\0';
	//printf("本歌注释:%s\n",buf);

	return 0;
	
}

void *Mplayer_audio_thread_fun(void *args)
{
	int pid;
	int mm_pid;
	
	char *fullname;
	char bakname[256];
	char realname[256];
	char cmd[256];
	fullname = (char *)malloc(256+1);
	fullname = (char *)args;
	
	memcpy(bakname,(const char *)args,strlen((const char *)args));

	if(g_fd_mplayer_fifo == -1)
	{
		unlink(MEDIA_FIFO);
		mkfifo(MEDIA_FIFO,O_CREAT |0666);                
		g_fd_mplayer_fifo = open(MEDIA_FIFO,O_RDWR);
		if(g_fd_mplayer_fifo == -1)
			WRT_DEBUG("open /dev/my_media_mplayer_fifo failed \n");
	}
	
looplabel:
	system("killall -9 mplayer");
	

	m_isHitKeyCancel = 0;
	pid = -1;
	pid = fork();
	if(CheckExternDevice()== -1)
	{
		CloseAllWindows();
		return NULL;
	}else
		;
	printf("enter thread fun\n");
	if(pid<0)
	{
		printf("fork error!\n");
	}
	else if(pid == 0)
	{
		SetVolume(pSystemInfo->mySysInfo.ringvolume-13);//wyx -20 ->  10
		execl("/usr/bin/mplayer","mplayer","-slave","-quiet",fullname);//,"-really-quiet"
	}else
	{
		printf("father process\n");
	}
	g_music_pid = pid;

	wait(NULL);

	if(m_isHitKeyCancel == 1 || g_is_doorcallroom == 1)
	{
		printf("主动退出\n");
		return NULL;
	}else
	{
		printf("播放完毕退出,如果没按停止键，则循环播放\n");
		
		SDL_Delay(100);
		
		if(m_PlayMode == RANDOM_LOOP)
		{
			fullname = GetRandomAudioName(DirNode_h);
			if(fullname == NULL)
				return NULL;
			FILE *afp ;
			afp = fopen(fullname,"r");
			if(afp == NULL)
			{
				printf("Failed open audio file\n");
				return NULL;
			}
			fclose(afp);
			afp =NULL;
			memcpy(bakname,fullname,256);
			g_isUpdated = 0;
			m_mutilmedia_curr_page = m_curr_play_audio_listno /9;
			m_curr_select = m_curr_play_audio_listno %9;
			m_mutilmedia_curr_page = m_mutilmedia_curr_page+1;
			ShowFileList(DirNode_h,2);
			update_rect(0,0,600,530);
			g_isUpdated = 1;
		}else if(m_PlayMode == CIR_LOOP)
		{
			fullname = GetNextAudioName(DirNode_h);
			if(fullname == NULL)
				return NULL;
			FILE *afp ;
			afp = fopen(fullname,"r");
			if(afp == NULL)
			{
				printf("Failed open audio file\n");
				return NULL;
			}
			fclose(afp);
			afp =NULL;
			memcpy(bakname,fullname,256);
			g_isUpdated = 0;
			m_mutilmedia_curr_page = m_curr_play_audio_listno /9;
			m_curr_select = m_curr_play_audio_listno %9;
			m_mutilmedia_curr_page = m_mutilmedia_curr_page+1;
			ShowFileList(DirNode_h,2);
			update_rect(0,0,600,530);
			g_isUpdated = 1;
		}else if(m_PlayMode == SINGLE_LOOP)
		{
			fullname = bakname;
			FILE *afp ;
			afp = fopen(fullname,"r");
			if(afp == NULL)
			{
				printf("Failed open audio file\n");
				return NULL;
			}
			fclose(afp);
			afp =NULL;
		}else
		{
			return NULL;//结束
		}

		goto looplabel;
	}
	
	free(fullname);
	m_isAudioPlay = 0;//在接收到外部Kill Mplayer信号之后退出
	
	return NULL;
}

int IsAudioPlayInfo(char *buf,int play_seq)
{
	WRT_Rect rt;
	rt.x = 558;
	rt.y = 16;
	rt.w = 310;
	rt.h = 500;
	FillRect(0xb2136,rt);

	if(m_PlayMode == SINGLE_LOOP)
    {
    	DrawText(LT("单曲循环"),600,450,rgb_white,24);
    }else if(m_PlayMode == RANDOM_LOOP)
    {
    	DrawText(LT("随机循环"),600,450,rgb_white,24);
    }else if(m_PlayMode == CIR_LOOP)
    {
    	DrawText(LT("顺序循环"),600,450,rgb_white,24);
    }else
    {
    	printf("unkown audio play mode!\n");
    }
    
    DirNode1 *tmpNode;
	
	char realname[256];
	tmpNode = init_dirnode();
	m_curr_play_audio_listno = play_seq;
	tmpNode = FindDirNode(DirNode_h,m_curr_play_audio_listno,2);
	
	if(tmpNode != NULL)
	{
		sprintf(buf,"%s/%s",TempPath,tmpNode->Content.Name);
		printf("Node name %s\n",tmpNode->Content.Name);
	}
	else
		;
	if(tmpNode->Content.MediaTpye != 2)
	{
		printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		return 9;
	}

	GetRealName(tmpNode->Content.Name, realname);

	//处理显示字符串过长
	int str_len;
    char showname[30];
    str_len = GetTextWidth((char*)realname,0);
    if(str_len >250)
    {
    	memcpy(showname,realname,19);
    	showname[19]='\0';
    	strcat(showname,"...");
    }else
    {
    	strcpy(showname,realname);
    }
	int disx = (230-str_len)/2;
	if(disx < 0)
		disx = 0;
	DrawText((char *)showname,590+disx,150,rgb_white);
	DrawText(LT("正在播放"),680,70,rgb_white,24);
	ShowMp3Info(buf);
	
	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,shangyishou_1,sizeof(shangyishou_1));//上一曲
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,xiayishou_1,sizeof(xiayishou_1));//下一曲
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_tingzhi1,sizeof(bt_tingzhi1));//停止
	JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));//播放

}

int MplayerAudioFunc(int play_seq)
{
	int ret = 0;
	pthread_attr_t attr;
	
	//char fullname[256];
	//memset(rfullname,0,256);
	printf("5555555555\n");
	if(CheckExternDevice()== -1)
	{
		CloseAllWindows();
		return NULL;
	}else
		;
	ret = IsAudioPlayInfo(rfullname,play_seq);

	printf("real name 1 %s\n",rfullname);
	if(ret == 9)
		return ret;
	//m_isAudioPlay = 1;
	//m_mplayer_audio_flag = 1;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&audio_mplayer_thread, &attr,Mplayer_audio_thread_fun,(void *)rfullname);
	if(audio_mplayer_thread < 0)
	{
		printf("failed to create audio mplayer thread\n");
		return -1;
	}
	pthread_attr_destroy(&attr);
}

int MutilmediaAudioEventHandler(int x,int y,int status)
{
	WRT_xy xy;
    int ret = 0;
    int index =0;
    int pos_x,pos_y;
    xy.x = x;
    xy.y = y;

    for(index =0;index<18;index++){
            if(IsInside(rc_infobrowsermenuico[index],xy)){
                    pos_x = rc_infobrowsermenuico[index].x;
                    pos_y = rc_infobrowsermenuico[index].y;
                    ret = 1;
                    if(status == 1)
                            g_rc_status[index] = true;
                    else{
                            if(g_rc_status[index] == false){
                                    ret = 0;
                                    return ret;
                            }
                            g_rc_status[index] = false;
                    }
                    pos_x = rc_infobrowsermenuico[index].x;
                    pos_y = rc_infobrowsermenuico[index].y;
                    switch(index)
                    {
						case 12:/*播放*/
							
							if(m_isAudioPlay == 1)
							{
								//exit_audio_play2();
								//SDL_Delay(100);
								break;
							}
							if(m_curr_select == -1)
							{
								CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
								break;
							}
								
							if(status == 1){
								StartButtonVocie();
								JpegDisp(pos_x,pos_y,bt_bofang2,sizeof(bt_bofang2));
							}else{
								JpegDisp(pos_x,pos_y,bt_bofang1,sizeof(bt_bofang1));

								int ret = 0;
								m_isAudioPlay = 1;
								int play_seq = 0;
								play_seq = m_curr_select + (m_mutilmedia_curr_page - 1)*9;
								mp3stop();
								if(play_seq >= length_dirnode(DirNode_h))
									break;
								ret = MplayerAudioFunc(play_seq);
								if(ret == 9)
								{
									m_isAudioPlay = 0;
									//char tmp[256];
									
									//GetShortName(rfullname,tmp);
									//strcat(TempPath,tmp);
                        			
                        			//printf("open file %s",TempPath);
                        			//CreateMutilmediaSub();//
                        			//CreateDialogWin2("您选择的是文件夹",MB_OK,NULL,NULL);
								}
							}
							break;
						case 13:/*上翻*/
							if(m_mutilmedia_curr_page == 1)
								break;
							if(status == 1){	
								if(m_isAudioPlay == 0)
									StartButtonVocie();
								JpegDisp(pos_x,pos_y,shangyiye_2,sizeof(shangyiye_2));
							}else{
								JpegDisp(pos_x,pos_y,shangyiye_1,sizeof(shangyiye_1));
								m_mutilmedia_curr_page--;
								m_curr_select = -1;
								g_isUpdated = 0;
								ShowMediaFileList(m_mediatype);
								update_rect(0,0,600,530);
								g_isUpdated = 1;
								SDL_Delay(100);
								
							}
							break;
						case 14:/*下翻*/
							if(m_mutilmedia_page == m_mutilmedia_curr_page)
								break;

							if(status == 1){
								if(m_isAudioPlay == 0)
									StartButtonVocie();
								JpegDisp(pos_x,pos_y,xiayiye_2,sizeof(xiayiye_2));
							}else{
								JpegDisp(pos_x,pos_y,xiayiye_1,sizeof(xiayiye_1)); 
								m_mutilmedia_curr_page++;
								m_curr_select = -1;
								g_isUpdated = 0;
								ShowMediaFileList(m_mediatype);
								update_rect(0,0,600,530);
								g_isUpdated = 1;
								SDL_Delay(100);
							}
							break;
							
						case 9:/*上一首*/
							if(m_isAudioPlay == 0)
								break;
							if(status == 1){
								
								JpegDisp(pos_x,pos_y,shangyishou_2,sizeof(shangyishou_2));
							}else{
								JpegDisp(pos_x,pos_y,shangyishou_1,sizeof(shangyishou_1));
								if(m_PlayMode == CIR_LOOP|| m_PlayMode == SINGLE_LOOP)
									PlayPrevMusic();
								else
									PlayRandomMusic();
								
							}    
							SDL_Delay(300);
							break;
							
						case 10:/*下一首*/
							if(m_isAudioPlay == 0)
								break;

							if(status == 1){
								
								JpegDisp(pos_x,pos_y,xiayishou_2,sizeof(xiayishou_2));
							}else{
								JpegDisp(pos_x,pos_y,xiayishou_1,sizeof(xiayishou_1));
								if(m_PlayMode == CIR_LOOP|| m_PlayMode == SINGLE_LOOP)
									PlayNextMusic();
								else
									PlayRandomMusic();
							}    
							SDL_Delay(300);
							break;
							
						case 11:/*stop*/
							if(m_isAudioPlay == 0)
								break;
							if(status == 1){
								JpegDisp(pos_x,pos_y,bt_tingzhi2,sizeof(bt_tingzhi2));

							}else{
								JpegDisp(pos_x,pos_y,bt_tingzhi1,sizeof(bt_tingzhi1));

	                            m_isAudioPlay = 0;
	                            exit_audio_play2();
	                            SDL_Delay(200);
	                            StartButtonVocie();
								}                 	  
							break;
                    	case 15://返回
	                        if(status == 1){
	                                
	                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
	                        }else{
	                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
	                                exit_audio_play2();
	                                SDL_Delay(200);
	                                StartButtonVocie();
	                                CloseTopWindow();
	                                
	                        }                   	
	                        break;
		                case 16://关屏幕
	                        if(status == 1){
	                                
	                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
	                        }else{
	                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
	                                screen_close();
	                                exit_audio_play2();
	                                SDL_Delay(200);
	                                StartButtonVocie();
	                                CloseAllWindows();
	                        }
	                        break;
	                    case 17://播放模式选择
	                    	
	                        if(status == 1){
	                        		if(m_isAudioPlay == 0)
	                                	StartButtonVocie();
	                                if(m_PlayMode == RANDOM_LOOP)
	                                	JpegDisp(pos_x,pos_y,xunhuanbofang_2,sizeof(xunhuanbofang_2));
	                                else if(m_PlayMode == CIR_LOOP)
	                                	JpegDisp(pos_x,pos_y,danquxunhuan_2,sizeof(danquxunhuan_2));
	                                else if(m_PlayMode == SINGLE_LOOP)
	                                	JpegDisp(pos_x,pos_y,suijibofang_2,sizeof(suijibofang_2));
	                        }else{
	                                if(m_PlayMode == RANDOM_LOOP)
	                                	JpegDisp(pos_x,pos_y,xunhuanbofang_1,sizeof(xunhuanbofang_1));
	                                else if(m_PlayMode == CIR_LOOP)
	                                	JpegDisp(pos_x,pos_y,danquxunhuan_1,sizeof(danquxunhuan_1));
	                                else if(m_PlayMode == SINGLE_LOOP)
	                                	JpegDisp(pos_x,pos_y,suijibofang_1,sizeof(suijibofang_1));
	                                
	                                WRT_Rect rt;
									rt.x =600;
									rt.y =400;
									rt.w =250;
									rt.h =60;
									FillRect(0xb2136,rt);
	                                m_PlayMode ++;
	                                if(m_PlayMode == 3)
	                                {
	                                	m_PlayMode = 0;
	                                }
	                                
	                                if(m_PlayMode == SINGLE_LOOP)
	                                {
	                                	DrawText(LT("单曲循环"),600,450,rgb_white,24);
	                                }else if(m_PlayMode == RANDOM_LOOP)
	                                {
	                                	DrawText(LT("随机循环"),600,450,rgb_white,24);
	                                }else if(m_PlayMode == CIR_LOOP)
	                                {
	                                	DrawText(LT("顺序循环"),600,450,rgb_white,24);
	                                }else
	                                {
	                                	printf("unkown audio play mode!\n");
	                                }
	                                SDL_Delay(200);
	                        }
	                        break;
	                        
		                default:
		                	if(status == 1)
		                	{
		                		;
		                	}
		                	else
		                	{
			                	m_curr_select = index;
			                	g_isUpdated = 0;
			                	ShowMediaFileList(m_mediatype);
			                	//if(m_isAudioPlay == 0)
	                            //    	StartButtonVocie();
			                	update_rect(0,0,889,530);
	                            g_isUpdated = 1;
		                	}
		                    break;
                    }
                    break;
   			}
   	}
}

void DrawMutilmediaAudio()
{
	WRT_Rect rt;
	int xoffset = 40;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	ListDirFunc(TempPath);
	
	JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));

	JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,shangyishou_1,sizeof(shangyishou_1));//上一曲
	JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,xiayishou_1,sizeof(xiayishou_1));//下一曲
	JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_tingzhi1,sizeof(bt_tingzhi1));//停止
	JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,bt_bofang1,sizeof(bt_bofang1));//播放

	JpegDisp(rc_infobrowsermenuico[17].x,rc_infobrowsermenuico[17].y,xunhuanbofang_1,sizeof(xunhuanbofang_1));//播放模式选择

	ShowMediaFileList(AUDIO);//2  音乐
}

static void CloseAudioWin()
{
	m_isAudioPlay = 0;
	exit_audio_play2();
}

//音乐播放器处理
void CreateMutilmediaAudio()
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return;
	pWindow->EventHandler = MutilmediaAudioEventHandler;
	pWindow->NotifyReDraw = DrawMutilmediaAudio;
	pWindow->valid = 1;
	pWindow->type = MUTILMEDIA;
	pWindow->CloseWindow = CloseAudioWin;
	m_mutilmedia_page = 1;
	m_mutilmedia_curr_page =1;
	m_curr_select = -1;
	m_mediatype = AUDIO;
	m_PlayMode = CIR_LOOP;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

//=================Audio Part End
//=============================================
//===========================================================
//函数功能:返回在全局链表中类型为type的第play_seq个元素的名称
int GetFileName(char *buf,int play_seq,int type)
{
	
    DirNode1 *tmpNode;
	
	char realname[256];
	tmpNode = init_dirnode();
	
	tmpNode = FindDirNode(DirNode_h,play_seq,type);

	if(tmpNode != NULL)
		printf("Node name %s\n",tmpNode->Content.Name);
	else
		return 7;
	if(tmpNode->Content.MediaTpye != type)
	{
		printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		return 9;
	}
	
	sprintf(buf,"%s/%s",TempPath,tmpNode->Content.Name);
}
int IsPicPlayInfo(char *buf,int play_seq);

int Copy_PicData2SD(int param)
{
	int ret = -1;
	int seq = -1;
	char cmd[1024];
	seq = m_curr_select+(m_mutilmedia_curr_page-1)*9;
	ret = IsPicPlayInfo(rfullname,seq);
	if(m_sd_exist == 1)
	{
		mDirIsExist("/mnt/mmc/picture");
		sprintf(cmd,"cp %s /mnt/mmc/picture",rfullname);
		system(cmd);
		system("sync");
	}
	return 0;
}

int Copy_PicData2UDisk(int param)
{
	int ret = -1;
	int seq = -1;
	char cmd[1024];
	seq = m_curr_select+(m_mutilmedia_curr_page-1)*9;
	ret = IsPicPlayInfo(rfullname,seq);
	if(m_usb1_exist== 1 )
	{
		mDirIsExist("/mnt/sda/picture");
		//mDirIsExist("/mnt/sdb/picture");
		sprintf(cmd,"cp %s /mnt/sda/picture",rfullname);
		system(cmd);
		//memset(cmd,0,1024);
		//sprintf(cmd,"cp %s /mnt/sdb/picture",rfullname);
		//system(cmd);
		system("sync");
	}
	return 0;

}

int PhotoDisp(int seq)
{
	int ret = 0;
	
	//WRT_Rect rt;
	//rt.x = 0;
	//rt.y = 0;
	//rt.w =889;
	//rt.h = 530;
	//FillRect(0xb2136,rt);
	ret = GetFileName(rfullname,seq,3);
	if(ret == 9)
	{
		printf("文件不是图片\n");
		return ret;
	}
	else
		;//DrawText(rfullname,10,10,rgb_white,24);
	if(ret == 7)
	{
		printf("找不到对应文件\n");
		return ret;
	}
	FILE *jpegfd;
#if 1 //read from buffer
	struct stat temp;
	stat(rfullname,&temp);
	printf("jpeg file name %s\n",rfullname);
	jpegfd = fopen(rfullname,"rb");
	if(jpegfd == NULL)
	{
		printf("Failed to open jpeg file \n");
		return -1;
	}
	char *tmpbuf = (char *)malloc(temp.st_size+1);//5M
	if(tmpbuf == NULL)
	{
		fclose(jpegfd);
		printf("alloc memory failed \n");
		return -1;
	}
	if(temp.st_size > 4*1024*1024)//大于4M直接退出
	{
		fclose(jpegfd);
		free(tmpbuf);
		return -1;
	}
	fread(tmpbuf,temp.st_size,1,jpegfd);
	
	
	printf("size :%d\n",temp.st_size);
	
	//JpegSingleDisp(0,0,(unsigned char *)tmpbuf,temp.st_size);
	JpegDispZoom(0,0,(unsigned char *)tmpbuf,temp.st_size,0.6,0.8);
#else if //read from file
	jpegfd = fopen(rfullname,"rb");
	if(jpegfd == NULL)
	{
		printf("Failed to open jpeg file \n");
		return -1;
	}
	JpegFileDispZoom(0,0,jpegfd);
#endif
	fclose(jpegfd);
	free(tmpbuf);
	jpegfd = NULL;
	return 0;
}

int PhotoWindowEventHandler(int x,int y,int status)
{
	WRT_xy xy;
    int ret = 0;
    int index =0;
    int pos_x,pos_y;
    xy.x = x;
    xy.y = y;
    for(index =0;index<7;index++){
            if(IsInside(rc_mutilpicico[index],xy)){
                    pos_x = rc_mutilpicico[index].x;
                    pos_y = rc_mutilpicico[index].y;
                    ret = 1;
                    if(status == 1)
                            g_rc_status[index] = true;
                    else{
                            if(g_rc_status[index] == false){
                                    ret = 0;
                                    return ret;
                            }
                            g_rc_status[index] = false;
                    }
                    pos_x = rc_mutilpicico[index].x;
                    pos_y = rc_mutilpicico[index].y;
             
             switch(index)
             {
             	case 0://显示框
             		break;

             	case 1://前一张
             		if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,shangyishou_2,sizeof(shangyishou_2));
					}else{
							JpegDisp(pos_x,pos_y,shangyishou_1,sizeof(shangyishou_1));
						loopprev:
						m_curr_select --;
						if(m_curr_select < 0)
						{
							m_curr_select = 8;
							m_mutilmedia_curr_page--;
							if(m_mutilmedia_curr_page <1)
								m_mutilmedia_curr_page =1;
						}
						int currplay = 0;
						int ret= 0;
						currplay = m_curr_select +(m_mutilmedia_curr_page -1)*9;
						if(currplay <0)
						{
							currplay = GetMediaCntByType(DirNode_h,3)-1;
							m_mutilmedia_curr_page = m_mutilmedia_page;
							m_curr_select = currplay%9;
						}
						WRT_Rect rt;
						rt.x = 0;
						rt.y = 0;
						rt.w =889;
						rt.h = 530;
						FillRect(0xb2136,rt);
						//JpegDisp(0,0,bofang_bak,sizeof(bofang_bak));
						DrawText(LT("加载中..."), 400, 250,rgb_white,24);
						ret = PhotoDisp(currplay);
						if(ret == 7 || ret == 9)
							goto loopprev;
						if(ret == -1)
						{
							DrawText(LT("显示错误"),100,100,rgb_white);
						}
					}
					break;

             	case 2://后一张
             		if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,xiayishou_2,sizeof(xiayishou_2));
					}else{
							JpegDisp(pos_x,pos_y,xiayishou_1,sizeof(xiayishou_1));
							loopnext:
						m_curr_select++;
						if(m_curr_select >=9)
						{
							m_curr_select = 0;
							m_mutilmedia_curr_page++;
							if(m_mutilmedia_curr_page >= m_mutilmedia_page)
								m_mutilmedia_curr_page = m_mutilmedia_page;
						}
						
						int currplay= 0;
						int ret = 0;
						currplay = m_curr_select +(m_mutilmedia_curr_page -1)*9;
						if(currplay >= GetMediaCntByType(DirNode_h,3) )
						{
							currplay = 0;
							m_curr_select = 0;
							m_mutilmedia_curr_page = 1;
						}
						WRT_Rect rt;
						rt.x = 0;
						rt.y = 0;
						rt.w =889;
						rt.h = 530;
						FillRect(0xb2136,rt);
						
						DrawText(LT("加载中..."), 400, 250,rgb_white,24);
						ret = PhotoDisp(currplay);
						if(ret == 7|| ret == 9)
							goto loopnext;
						if(ret == -1)
						{
							DrawText(LT("显示错误"),100,100,rgb_white);
						}
					}						
					break;
#if 1
             	case 3:/*拷贝到U Disk*/
						
					if((m_usb1_exist ==1 )&& g_rec_flag ==1)
					{
						if(m_curr_select == -1)
						{
							CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
							break;
						}
						if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,usb_beifen_2,sizeof(usb_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,usb_beifen_1,sizeof(usb_beifen_1));
							CreateDialogWin2(LT("拷贝数据到U盘..."),MB_OK|MB_CANCEL,Copy_PicData2UDisk,NULL);
						}
					}
					break;
				case 4:/*拷贝到SD Card*/
					if(m_sd_exist ==1 && g_rec_flag ==1)
					{
						if(m_curr_select == -1)
						{
							CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
							break;
						}
						
						if(status == 1){
							StartButtonVocie();

							JpegDisp(pos_x,pos_y,sd_beifen_2,sizeof(sd_beifen_2));
						}else{
							JpegDisp(pos_x,pos_y,sd_beifen_1,sizeof(sd_beifen_1));
							CreateDialogWin2(LT("拷贝数据到SD卡..."),MB_OK|MB_CANCEL,Copy_PicData2SD,NULL);
						}
					}
					break;	
#endif
             	case 5://返回
					if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
					}else{
							JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
							CloseTopWindow();
					}				
					break;

             	case 6://关屏
             		if(status == 1){
							StartButtonVocie();
							JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
					}else{
							JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));
							
							screen_close();
							CloseAllWindows();
					}

             	break;

             	default :
             		break;
             }
    	}
    }
}

void DrawPhotoWindow()
{
	int currDisp = 0;
	int ret = 0;
	WRT_Rect rt;
	int xoffset = 40;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	
	JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));
	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
	JpegDisp(rc_mutilmediaico[3].x,rc_mutilmediaico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
	JpegDisp(rc_mutilmediaico[4].x,rc_mutilmediaico[4].y,bt_guanping1,sizeof(bt_guanping1));
	
	JpegDisp(rc_mutilpicico[1].x,rc_mutilpicico[1].y,shangyishou_1,sizeof(shangyishou_1));//上一曲
	JpegDisp(rc_mutilpicico[2].x,rc_mutilpicico[2].y,xiayishou_1,sizeof(xiayishou_1));//下一曲
	//JpegDisp(rc_mutilpicico[3].x,rc_mutilpicico[3].y,bt_suoding1,sizeof(bt_suoding1));//停止
	//JpegDisp(rc_mutilpicico[4].x,rc_mutilpicico[4].y,bt_shanchu1,sizeof(bt_shanchu1));//播放
	if(g_rec_flag == 1)
	{
		if(m_usb1_exist==1 )
			JpegDisp(rc_mutilpicico[3].x,rc_mutilpicico[3].y,usb_beifen_1,sizeof(usb_beifen_1));
		if(m_sd_exist == 1)
			JpegDisp(rc_mutilpicico[4].x,rc_mutilpicico[4].y,sd_beifen_1,sizeof(sd_beifen_1));
	}	
	currDisp = m_curr_select + (m_mutilmedia_curr_page -1)*9;
	ret = PhotoDisp(currDisp);
	if(ret == -1 || ret == 7)
	{
		DrawText(LT("显示错误"),100,100,rgb_white);
	}
}

int g_called_flag = 0;
static int g_pic_pid = 0;

static void CloseMainPhotoWin()
{
	if(g_called_flag == 0)
	{
		;
	}else
	{
		g_called_flag = 0;
		if(g_rec_flag != 1 )
        {
                //char cmd[1024];
                
                //sprintf(cmd,"killall -9 DCPhotoviewer");
                //system(cmd);
                system("killall -9 DCPhotoviewer");
        }
	}
        
}

static void CloseSubPhotoWin()
{
	printf("关闭窗口\n");
}

void CreatePhotoWindow()
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = PhotoWindowEventHandler;
	pWindow->NotifyReDraw = DrawPhotoWindow;
	pWindow->valid = 1;
	pWindow->type  = MUTILMEDIA;
	pWindow->CloseWindow = CloseSubPhotoWin;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

int IsPicPlayInfo(char *buf,int play_seq)
{
	
    DirNode1 *tmpNode;
	
	char realname[256];
	tmpNode = init_dirnode();
	tmpNode = FindDirNode(DirNode_h,play_seq,3);
	if(tmpNode != NULL)
	{
		sprintf(buf,"%s/%s",TempPath,tmpNode->Content.Name);
		printf("Node name %s\n",tmpNode->Content.Name);
	}
	else
		;
	if(tmpNode->Content.MediaTpye != 3)
	{
		printf("the file type is %d\n",tmpNode->Content.MediaTpye);
		return 9;
	}
	
}

int delete_cap_picture(int param)
{
	int del_seq;
	char cmd[1024];
	del_seq = m_curr_select +(m_mutilmedia_curr_page-1)*9;
	GetFileName(rfullname,del_seq,3);
	sprintf(cmd,"rm -rf %s",rfullname);
	system(cmd);
	system("sync");
	m_curr_select = -1;
	g_isUpdated = 0 ;
	DrawMutilmediaPic();
	g_isUpdated = 1;
	return 0;
}

int delete_all_cap_picture(int param)
{
	char cmd[1024];
	DirIsExist("/home/wrt/pic/picture");
	sprintf(cmd,"rm -rf /home/wrt/pic/picture/*.*");
	system(cmd);
	system("sync");
	g_isUpdated = 0;
	DrawMutilmediaPic();
	g_isUpdated = 1;
	return 0;
}

int MutilmediaPicEventHandler(int x,int y,int status)
{
	WRT_xy xy;
    int ret = 0;
    int index =0;
    int pos_x,pos_y;
    xy.x = x;
    xy.y = y;
	if(g_rec_flag == 0)
	{
		return 0;
	}
	
	for(index =0;index<18;index++){
			if(IsInside(rc_infobrowsermenuico[index],xy))
            {
		            pos_x = rc_infobrowsermenuico[index].x;
		            pos_y = rc_infobrowsermenuico[index].y;
		            ret = 1;
		            if(status == 1)
		                    g_rc_status[index] = true;
		            else{
		                    if(g_rc_status[index] == false){
		                            ret = 0;
		                            return ret;
		                    }
		                    g_rc_status[index] = false;
		            }
		            pos_x = rc_infobrowsermenuico[index].x;
		            pos_y = rc_infobrowsermenuico[index].y;
                    switch(index)
                    {
						case 11:/*播放*/
							if(m_curr_select == -1)
							{
								CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
								break;
							}

							if(status == 1){
								StartButtonVocie();

								JpegDisp(pos_x,pos_y,bt_dakai2,sizeof(bt_dakai2));
							}else{
								JpegDisp(pos_x,pos_y,bt_dakai1,sizeof(bt_dakai1));
								int ret = -1;
								int seq = -1;
								seq = m_curr_select+(m_mutilmedia_curr_page-1)*9;
								if(seq >= length_dirnode(DirNode_h))
									break;
								ret = IsPicPlayInfo(rfullname,seq);
								if(ret == 9)
								{
									//CreateDialogWin2("您选择的是文件夹",MB_OK,NULL,NULL);
									//char tmp[256];
									
									//GetShortName(rfullname,tmp);
									//strcat(TempPath,tmp);
                        			
                        			//printf("open file %s",TempPath);
                        			//CreateMutilmediaSub();//
								}
								else
								{
									WRT_Rect rt;
									rt.x = 0;
									rt.y = 0;
									rt.w =889;
									rt.h = 530;
									FillRect(0xb2136,rt);
									DrawText(LT("加载中..."), 400, 250,rgb_white,24);
									CreatePhotoWindow();
								}
							}

							break;
						case 13:/*上翻*/
							if(m_mutilmedia_curr_page == 1)
									break;

							if(status == 1){
								StartButtonVocie();
								JpegDisp(pos_x,pos_y,shangyiye_2,sizeof(shangyiye_2));
							}else{
								JpegDisp(pos_x,pos_y,shangyiye_1,sizeof(shangyiye_1));
								m_mutilmedia_curr_page--;
								m_curr_select = -1;
								g_isUpdated = 0;
								ShowMediaFileList(m_mediatype);
								update_rect(0,0,600,530);
								g_isUpdated = 1;
							}
							break;
						case 14:/*下翻*/  
							if(m_mutilmedia_page == m_mutilmedia_curr_page)
								break;    	
							if(status == 1){
								StartButtonVocie();
								JpegDisp(pos_x,pos_y,xiayiye_2,sizeof(xiayiye_2));
							}else{
								JpegDisp(pos_x,pos_y,xiayiye_1,sizeof(xiayiye_1)); 
								m_mutilmedia_curr_page++;
								m_curr_select = -1;
								g_isUpdated = 0;
								ShowMediaFileList(m_mediatype);
								update_rect(0,0,600,530);
								g_isUpdated = 1;		
							}
							break;
						case 9:/*清空*/
							if(g_rec_flag == 0)
								break;
							if(status == 1){
								StartButtonVocie();

								JpegDisp(pos_x,pos_y,bt_qinkong2,sizeof(bt_qinkong2));
							}else{

								JpegDisp(pos_x,pos_y,bt_qinkong1,sizeof(bt_qinkong1));  
								CreateDialogWin2(LT("你确定要清空所有图片"),MB_OK|MB_CANCEL,delete_all_cap_picture,NULL);     
							}                                         
							break;
						case 10:/*删除*/
							if(g_rec_flag == 0)
								break;
							if(status == 1){
								StartButtonVocie();

								JpegDisp(pos_x,pos_y,bt_shanchu2,sizeof(bt_shanchu2));
							}else{

								JpegDisp(pos_x,pos_y,bt_shanchu1,sizeof(bt_shanchu1)); 
								int del_seq = m_curr_select +(m_mutilmedia_curr_page-1)*9;
								//memset(rfullname,0,256);
								if(del_seq >= GetMediaCntByType(DirNode_h,3))
								{
									CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
									break;
								}else
									CreateDialogWin2(LT("你确定要删除该图片"),MB_OK|MB_CANCEL,delete_cap_picture,NULL);

							}                    
							break;
						case 12:/*拷贝到U Disk*/
							
							if((m_usb1_exist ==1 || m_usb2_exist == 1)&& g_rec_flag ==1)
							{
								if(m_curr_select == -1)
								{
									CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
									break;
								}
								if(status == 1){
									StartButtonVocie();

									JpegDisp(pos_x,pos_y,usb_beifen_2,sizeof(usb_beifen_2));
								}else{
									JpegDisp(pos_x,pos_y,usb_beifen_1,sizeof(usb_beifen_1));
									CreateDialogWin2(LT("拷贝数据到U盘..."),MB_OK|MB_CANCEL,Copy_PicData2UDisk,NULL);
								}
							}
							break;
						case 17:/*拷贝到SD Card*/
							if(m_sd_exist ==1 && g_rec_flag ==1)
							{
								if(m_curr_select == -1)
								{
									CreateDialogWin2(LT("请选择文件"),MB_OK,NULL,NULL); 
									break;
								}
								
								if(status == 1){
									StartButtonVocie();

									JpegDisp(pos_x,pos_y,sd_beifen_2,sizeof(sd_beifen_2));
								}else{
									JpegDisp(pos_x,pos_y,sd_beifen_1,sizeof(sd_beifen_1));
									CreateDialogWin2(LT("拷贝数据到SD卡..."),MB_OK|MB_CANCEL,Copy_PicData2SD,NULL);
								}
							}
							break;
                    	case 15://返回
	                        if(status == 1){
	                                StartButtonVocie();
	                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
	                        }else{
	                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
	                                CloseTopWindow();
	                        }                   	
	                        break;
		                case 16://关屏幕
	                        if(status == 1){
	                                StartButtonVocie();
	                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
	                        }else{
	                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
	                                screen_close();
	                                CloseAllWindows();
	                        }
	                        break;
						default:
							if(status == 1)
								;
							else
							{
			                	m_curr_select = index;
			                	g_isUpdated = 0;
			                	ShowMediaFileList(m_mediatype);
			                	//StartButtonVocie();
			                	if(g_rec_flag == 1)
								{
									int tmp = GetMediaCntByType(DirNode_h,3);
									if(tmp >180)
									{
										;//DrawText(LT2("Cap pic Warning"),200,580,rgb_red,32);
									}
								}
			                	update_rect(0,0,889,600);
	                            g_isUpdated = 1;
                            }
		                    break;
                    }
                    break;
   			}
   	}
}
void *QtPicthread(void *)
{
	int p_pid;

	{
		p_pid = fork();
		if(p_pid <0)
		{
			printf("fork err!\n");
		}else if(p_pid == 0)
		{
			
			execl("/home/wrt/qt/DCPhotoviewer","DCPhotoviewer","-qws",TempPath);
		}
		else
			;
	}

	wait(NULL);
	if(g_called_flag == 1)
	{
		g_called_flag =0;
		CloseTopWindow();
	}
}

void DrawMutilmediaPic()
{
	WRT_Rect rt;
	int xoffset = 40;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);
	ListDirFunc(TempPath);
	
	
	if(g_rec_flag == 0)//test for QTapp 
	{
		int count = length_dirnode(DirNode_h);
		if(count == 0)
		{
			DrawText(LT("没有图片..."),80,77,rgb_white);
			SDL_Delay(2000);//没有图片延迟3秒钟，关闭窗口
			CloseTopWindow();
			//return;
		}
		
		g_called_flag  = 1;
		pthread_attr_t attr;

		pthread_t Picthd;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&Picthd, &attr, QtPicthread, NULL);
		if ( Picthd == 0 ) {
			return;
		}
		pthread_attr_destroy(&attr);
	}
	else
	{	
		
		JpegDisp(889,0,bt_tools_back,sizeof(bt_tools_back));
		JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
		JpegDisp(rc_mutilmediaico[3].x,rc_mutilmediaico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
		JpegDisp(rc_mutilmediaico[4].x,rc_mutilmediaico[4].y,bt_guanping1,sizeof(bt_guanping1));
		if(g_rec_flag == 1)
		{
			dir_file_count();
			JpegDisp(rc_infobrowsermenuico[9].x,rc_infobrowsermenuico[9].y,bt_qinkong1,sizeof(bt_qinkong1));
			JpegDisp(rc_infobrowsermenuico[10].x,rc_infobrowsermenuico[10].y,bt_shanchu1,sizeof(bt_shanchu1));
			if(m_usb1_exist==1 || m_usb2_exist ==1)
				JpegDisp(rc_infobrowsermenuico[12].x,rc_infobrowsermenuico[12].y,usb_beifen_1,sizeof(usb_beifen_1));
			if(m_sd_exist == 1)
				JpegDisp(rc_infobrowsermenuico[17].x,rc_infobrowsermenuico[17].y,sd_beifen_1,sizeof(sd_beifen_1));
		}
		JpegDisp(rc_infobrowsermenuico[11].x,rc_infobrowsermenuico[11].y,bt_dakai1,sizeof(bt_dakai1));

		ShowMediaFileList(3);
	}
}

void CreateMutilmediaPic()
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;   
	pWindow->EventHandler = MutilmediaPicEventHandler;
	pWindow->NotifyReDraw = DrawMutilmediaPic;
	pWindow->valid = 1;
	pWindow->type = MUTILMEDIA;
	pWindow->CloseWindow = CloseMainPhotoWin;
	m_mutilmedia_page = 1;
	m_mutilmedia_curr_page =1;
	m_curr_select = -1;
	m_mediatype = PICTURE;
	m_usb1_exist = 0;
	m_usb2_exist = 0;
	m_sd_exist =0;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

int MutilmediaSubEventHandler(int x,int y,int status)
{
		WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<5;index++){
                if(IsInside(rc_mutilmediaico[index],xy)){
                        pos_x = rc_mutilmediaico[index].x;
                        pos_y = rc_mutilmediaico[index].y;
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_mutilmediaico[index].x;
                        pos_y = rc_mutilmediaico[index].y;
                        switch(index)
                        {
                        	
                        	case 0://视频
                        		if(status==1)
                        		{
                        			StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,shipingwenjian_2,sizeof(shipingwenjian_2));
                        		}else
                        		{
                        			JpegDisp(pos_x,pos_y,shipingwenjian_1,sizeof(shipingwenjian_1));
                        			
                        			memset(TempPath,0,1024);
		                			memcpy(TempPath,m_DevName,strlen(m_DevName));
                        			strcat(TempPath,"/video");
                        			
                        			if(g_rec_flag == 0)
                        				CreateMutilmediaVideo();
                        			else
                        				CreateVideoWin();
                        		}
                        		break;
                        	case 1://音乐
                        		if(g_rec_flag == 1)
                        			break;
                        		if(status==1)
                        		{
                        			StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,menu_lingsheng2,sizeof(menu_lingsheng2));
                        		}else
                        		{
                        			JpegDisp(pos_x,pos_y,menu_lingsheng1,sizeof(menu_lingsheng1));
                        			memset(TempPath,0,1024);
		                			memcpy(TempPath,m_DevName,strlen(m_DevName));
                        			strcat(TempPath,"/music");
                        			CreateMutilmediaAudio();
                        		}
                        		break;
                        	case 2://相册
                        		if(status==1)
                        		{
                        			StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,xiangpianwenjian_2,sizeof(xiangpianwenjian_2));
                        		}else
                        		{
                        			JpegDisp(pos_x,pos_y,xiangpianwenjian_1,sizeof(xiangpianwenjian_1));
                        			memset(TempPath,0,1024);
		                			memcpy(TempPath,m_DevName,strlen(m_DevName));
                        			strcat(TempPath,"/picture");
                        			if(g_rec_flag == 0)
                        				CreateMutilmediaPic();
                        			else
                        				CreatePhotoWin();
                        		}
                        		break;
                        	case 3://返回
		                        if(status == 1){
		                                StartButtonVocie();
		                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
		                        }else{
		                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
		                                
		                                CloseTopWindow();
		                        }                   	
		                        break;
			                case 4: //关屏幕
		                        if(status == 1){
		                                StartButtonVocie();
		                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
		                        }else{
		                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
		                                screen_close(); 
		                                CloseAllWindows();
		                        }
		                        break;
			                default:
			                        break; 
                        }
                        break;
       			}
       	}
}

void DrawMutilmediaSub()//画音乐 视频选择界面
{
	WRT_Rect rt;
	int xoffset = 21;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back)); 
	JpegDisp(rc_mutilmediaico[0].x,rc_mutilmediaico[0].y,shipingwenjian_1,sizeof(shipingwenjian_1));//视频
	if(g_rec_flag == 0)
		JpegDisp(rc_mutilmediaico[1].x,rc_mutilmediaico[1].y,menu_lingsheng1,sizeof(menu_lingsheng1));//音乐
	JpegDisp(rc_mutilmediaico[2].x,rc_mutilmediaico[2].y,xiangpianwenjian_1,sizeof(xiangpianwenjian_1));//相册
	
    JpegDisp(rc_mutilmediaico[3].x,rc_mutilmediaico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
    JpegDisp(rc_mutilmediaico[4].x,rc_mutilmediaico[4].y,bt_guanping1,sizeof(bt_guanping1));

	
	DrawEnText(LT("视频"),rc_mutilmediaico[0].w,rc_mutilmediaico[0].h,rc_mutilmediaico[0].x,rc_mutilmediaico[0].y);
	if(g_rec_flag == 0)
		DrawEnText(LT("音乐"),rc_mutilmediaico[1].w,rc_mutilmediaico[1].h,rc_mutilmediaico[1].x,rc_mutilmediaico[1].y);
	
	DrawEnText(LT("图片"),rc_mutilmediaico[2].w,rc_mutilmediaico[2].h,rc_mutilmediaico[2].x,rc_mutilmediaico[2].y);
}

void CreateMutilmediaSub()
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = MutilmediaSubEventHandler;
	pWindow->NotifyReDraw = DrawMutilmediaSub;
	pWindow->valid = 1;
	pWindow->type = CHILD;
	
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);   
}

int MutilmediaMainEventHandler(int x,int y,int status)
{
		WRT_xy xy;
        int ret = 0;
        int index =0;
        int pos_x,pos_y;
        xy.x = x;
        xy.y = y;
        for(index =0;index<5;index++){
                if(IsInside(rc_mutilmediaico[index],xy)){
                        pos_x = rc_mutilmediaico[index].x;
                        pos_y = rc_mutilmediaico[index].y;
                        ret = 1;
                        if(status == 1)
                                g_rc_status[index] = true;
                        else{
                                if(g_rc_status[index] == false){
                                        ret = 0;
                                        return ret;
                                }
                                g_rc_status[index] = false;
                        }
                        pos_x = rc_mutilmediaico[index].x;
                        pos_y = rc_mutilmediaico[index].y;
                        switch(index)
                        {
                        	case 0://USB1
                        		m_DevName = USB1_PATH;
                        		if(m_usb1_exist == 0)
                        		{
                        			if(m_usb2_exist == 0)
                        				break;
                        			else
                        				m_DevName = USB2_PATH;
                        		}
                        			
                        		if(status==1)
                        		{
                        			StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,usb_kedu_2,sizeof(usb_kedu_2));
                        		}else
                        		{
                        			JpegDisp(pos_x,pos_y,usb_kedu_1,sizeof(usb_kedu_1));
                        			
                        			CreateMutilmediaSub();
                        		}
                        		break;
                        	case 1://USB2
                        		break;
                        		if(m_usb2_exist == 0)
                        			break;
                        		if(status==1)
                        		{
                        			StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,usb_kedu_2,sizeof(usb_kedu_2));
                        		}else
                        		{
                        			JpegDisp(pos_x,pos_y,usb_kedu_1,sizeof(usb_kedu_1));
                        			m_DevName = USB2_PATH;
                        			CreateMutilmediaSub();
                        		}
                        		break;
                        	case 2://SD Card 
                        		if(m_sd_exist == 0)
                        		{
                        			break;
                        		}
                        		if(status==1)
                        		{
                        			StartButtonVocie();
                        			JpegDisp(pos_x,pos_y,sd_kedu_2,sizeof(sd_kedu_2));
                        		}else
                        		{
                        			JpegDisp(pos_x,pos_y,sd_kedu_1,sizeof(sd_kedu_1));
                        			m_DevName = SD_PATH;
                        			CreateMutilmediaSub();
                        		}
                        		break;
                        	case 3://返回
		                        if(status == 1){
		                                StartButtonVocie();
		                                JpegDisp(pos_x,pos_y,bt_fanhui2,sizeof(bt_fanhui2));
		                        }else{
		                                JpegDisp(pos_x,pos_y,bt_fanhui1,sizeof(bt_fanhui1));
		                                CloseTopWindow();
		                        }                   	
		                        break;
			                case 4://关屏幕
		                        if(status == 1){
		                                StartButtonVocie();
		                                JpegDisp(pos_x,pos_y,bt_guanping2,sizeof(bt_guanping2));
		                        }else{
		                                JpegDisp(pos_x,pos_y,bt_guanping1,sizeof(bt_guanping1));  
		                                screen_close(); 
		                                CloseAllWindows();
		                        }
		                        break;
			                default:
			                        break;
                        }
                        break;
       			}
       	}
}

void drawmutilmedia_func()
{
	DIR *dirp;
	struct dirent * ptr;
	int count = 0;
	WRT_Rect rt;
	int xoffset = 21;
	rt.x = 0;
	rt.y = 0;
	rt.w =1024;
	rt.h = 530;
	FillRect(0xb2136,rt);

	JpegDisp(0,530,menu_sub_back,sizeof(menu_sub_back));
	dirp = opendir(USB1_PATH);
	if(dirp == NULL)
	{
		JpegDisp(rc_mutilmediaico[0].x,rc_mutilmediaico[0].y,usb_bukedu_1,sizeof(usb_bukedu_1));
	}else
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count <=2) // . .. 2个文件夹
			JpegDisp(rc_mutilmediaico[0].x,rc_mutilmediaico[0].y,usb_bukedu_1,sizeof(usb_bukedu_1));
		else{
			m_usb1_exist =1;
			JpegDisp(rc_mutilmediaico[0].x,rc_mutilmediaico[0].y,usb_kedu_1,sizeof(usb_kedu_1));
		}
	}
	if(dirp != NULL)
		closedir(dirp);
	dirp = NULL;
	ptr  = NULL;
#if 0	
	count =0;
	dirp = opendir(USB2_PATH);
	if(dirp == NULL)
	{
		JpegDisp(rc_mutilmediaico[1].x,rc_mutilmediaico[1].y,usb_bukedu_1,sizeof(usb_bukedu_1));
	}else
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count <=2)
			JpegDisp(rc_mutilmediaico[1].x,rc_mutilmediaico[1].y,usb_bukedu_1,sizeof(usb_bukedu_1));
		else{
			m_usb2_exist =1;
			JpegDisp(rc_mutilmediaico[1].x,rc_mutilmediaico[1].y,usb_kedu_1,sizeof(usb_kedu_1));
		}
	}
	if(dirp != NULL)
		closedir(dirp);
	dirp = NULL;
	ptr = NULL;
#endif
	count = 0;
	dirp  = opendir(SD_PATH);
	if(dirp == NULL)
	{
		JpegDisp(rc_mutilmediaico[2].x,rc_mutilmediaico[2].y,sd_bukedu_1,sizeof(sd_bukedu_1));
	}else
	{
		while((ptr = readdir(dirp))!=NULL)
		{
			count++;
		}
		
		if(count <=2)
			JpegDisp(rc_mutilmediaico[2].x,rc_mutilmediaico[2].y,sd_bukedu_1,sizeof(sd_bukedu_1));
		else{
			m_sd_exist =1;
			JpegDisp(rc_mutilmediaico[2].x,rc_mutilmediaico[2].y,sd_kedu_1,sizeof(sd_kedu_1));
		}
	}
	ptr = NULL;
	if(dirp != NULL)//
		closedir(dirp);
	dirp = NULL;
}

void DrawMutilmediaMain()//画多媒体主界面
{
	drawmutilmedia_func();

    JpegDisp(rc_mutilmediaico[3].x,rc_mutilmediaico[3].y,bt_fanhui1,sizeof(bt_fanhui1));
    JpegDisp(rc_mutilmediaico[4].x,rc_mutilmediaico[4].y,bt_guanping1,sizeof(bt_guanping1));

	DrawEnText(LT("U盘"),rc_mutilmediaico[1].w,rc_mutilmediaico[0].h,rc_mutilmediaico[0].x,rc_mutilmediaico[0].y);
	DrawEnText(LT("SD卡"),rc_mutilmediaico[2].w,rc_mutilmediaico[2].h,rc_mutilmediaico[2].x,rc_mutilmediaico[2].y);
}

void CreateMutilmediaMain()//多媒体主界面
{
	Window* pWindow = (Window*)New_Window();
	if(pWindow == NULL)
		return ;
	pWindow->EventHandler = MutilmediaMainEventHandler;
	pWindow->NotifyReDraw = DrawMutilmediaMain;
	pWindow->valid = 1;
	pWindow->type = CHILD;
	m_usb1_exist = 0;
	m_usb2_exist = 0;
	m_sd_exist 	 = 0;
	
	g_rec_flag   = 0;
	WindowAddToWindowList(pWindow);
	pWindow->NotifyReDraw2(pWindow);
}

