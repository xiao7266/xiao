

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <errno.h>
#include "public.h"
#include "info_list_handler.h"
//#include "tmFlashFs.h" //[2011-5-19 8:59:47]

#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "wrt_log.h"
#include "tmAppport.h"

//handler.h

extern "C"  int GetCurrentLanguage();

#define DEFAULT_RING_COUNT 8
/////////////////////////////////////////////////////////////////////////

int g_no_flash_size = 0;
//#define USED_FFS 1




#ifdef __cplusplus
extern "C"
{
#endif

int wrtip_is_config_ring(char* addr,int size);

	int uni2gb( const unsigned short *unistr, unsigned char *gbbuf, int buflen );
	int utf8ToUcs2(const unsigned char *s, unsigned short *wbuf, int wbuf_len);

	int gb2uni( const unsigned char *gbstr, unsigned short *unibuf, int buflen );
	int ucs2ToUtf8(const unsigned short *ucs, unsigned char *cbuf, int cbuf_len);
#ifdef __cplusplus
}
#endif


/*************************************************************
判断是否有足够的磁盘空间
*************************************************************/

static int is_has_flash_size(int size)
{
	int _size;
	struct statfs vbuf;
	statfs("/customer/wrt",&vbuf);
	_size = (vbuf.f_bsize * vbuf.f_bfree); //获取可用的磁盘空间，Byte.
	WRT_DEBUG("free flash size %d\n",_size);
	if(_size > size){
		g_no_flash_size = 0;
		return 1;
	}
	g_no_flash_size = 1;
	return 0;
}

static void dir_is_exist(char* name)
{
		DIR * dirp = NULL;
        	if((dirp = opendir(name)) == NULL){
        		if(mkdir(name,1) < 0)
        			printf("error to mkdir %s\n",name);
        	}
        	if(dirp != NULL){
        		closedir(dirp);
        		dirp = NULL;
        	}
}

/*************************************************************
optimize process save file
*************************************************************/
#if USED_OPTIMIZE_FFS

static SDL_sem* g_opt_msg_sem = NULL;
static CWRTMsgQueue g_opt_msg_queue;


static int optimize_save_task(void* pvoid){
        unsigned long rc = 0;
        unsigned long msg[4] = {0};
        char* name  = NULL;
        char* addr = NULL;
        CWRTMsg* pMsg = NULL;
        unsigned long len = 0;
        uint32_t ticks1,ticks2;
        g_opt_msg_sem = SDL_CreateSemaphore(0);
        assert(g_opt_msg_sem);
        while(1){
                memset(msg,0,16);
                rc = -1;
                SDL_SemWait(g_opt_msg_sem);
                pMsg = g_opt_msg_queue.get_message();
                if(pMsg){
                	rc = parse_msg(pMsg,msg);
                	if(rc < 0)
                		break;
                	if(rc == 0)
                		continue;
                	rc = 0;
                }
                	
                //rc = q_receive(g_opt_qid,Q_WAIT,0,msg);
                if(rc == 0){
                        FILE* file = NULL;
                        name = (char*)msg[0];
                        addr = (char*)msg[1];
                        len = msg[2];
                        if(name == NULL || addr == NULL ||len == 0){
                                if(NULL != name)
                                        ZENFREE(name);
                                name = 0;
                                if(NULL != addr)
                                        ZENFREE(addr);
                                addr = 0;
                                len = 0;
                                continue;
                        }
                        ticks1 = ticks2 = 0;
                        tm_getticks(NULL,&ticks1);
                        EnterMutex();
                        unlink(name);
                        WRT_MESSAGE("存储文件%s",name);
                        file = fopen(name,"wb");
                        if(file != NULL){
                                long write_size = fwrite(addr,1,len,file);
                                if(write_size == len){
                                        fclose(file);   
                                        file = NULL;                                   
                                        ZENFREE(addr);                                        
                                        addr = 0;
                                        ExitMutex();
                                        tm_getticks(NULL,&ticks2);
                                        system("sync");
                                        WRT_MESSAGE("存储文件%s成功",name);
                                        ZENFREE(name);
                                        name = 0;
                                        len = 0;
                                        continue;
                                }
                                WRT_MESSAGE("存储文件%s失败0x%x行号%d",name,errno,__LINE__);
                                fclose(file);
                                file = NULL;
                                unlink(name);
                        }
                        WRT_MESSAGE("存储文件%s失败0x%x行号%d",name,errno,__LINE__);
                        ZENFREE(name);
                        ZENFREE(addr);
                        name = 0;
                        addr = 0;
                        len = 0;
                        ExitMutex();
                }
        }
        g_opt_msg_queue.release();
        SDL_DestroySemaphore(g_opt_msg_sem);
        g_opt_msg_sem = NULL;
        return 0;
       
}

void init_optimize_save(){

	DIR * dirp = NULL;
       // q_create("optq",10,Q_LIMIT|Q_FIFO|Q_LOCAL,&g_opt_qid);
      //  char currentpath[MAX_FILE_LEN]={0};
       // memset(currentpath,0,MAX_FILE_LEN);
        //getcwd(currentpath,MAX_FILE_LEN-1);
        dir_is_exist("/customer");
        dir_is_exist("/customer/wrt");

	SDL_CreateThread(optimize_save_task,NULL);

}

#endif

/*-------------------------------------------------------------
--该段主要处理有关抓拍图片的存储处理                         --
-------------------------------------------------------------*/
//100幅图片空间，将来可以在init中。改成动态分配

//extern MYSYSINFO*  g_mysysinfo;
extern T_SYSTEMINFO* pSystemInfo;


static PHOTOINFO           m_photoinfo[MAX_PHOTO];
static unsigned char*      photo_addr=0;
static int                 m_photocount=0;
static int                 m_photo_index_save = 0;
static char photodir1[]="/customer/wrt/pic";
static char photodir[]="/customer/wrt/pic/picture";
static char photoindex[] = "/customer/wrt/pic/photo_index.txt";
static int                m_max_photo = MAX_PHOTO;
static int                m_photo_no_read = 0;
//static unsigned  long      m_writeflashmutex;

static void Photo_Is_No_Read(){
        int i = 0;
        for(i=0;i<m_max_photo;i++){
                if(m_photoinfo[i].idle == 1){
                        if((m_photoinfo[i].read & 0xffff) == 0){
                                m_photo_no_read = 1;
                                return;
                        }
                }
        }
        if(i == m_max_photo)
                m_photo_no_read = 0;
}

int IsNewPhoto(){
        return m_photo_no_read;
}
int isnoreadpic(){
        Photo_Is_No_Read();
        return IsNewPhoto();
}

void Set_Max_photo(int maxnum){
        if(maxnum < 0 || maxnum > 100){
                return;
        }
        m_max_photo = maxnum;
        pSystemInfo->mySysInfo.maxphoto = m_max_photo;
}
/**
*获得当先存储的图片的个数
*/
static int  get_photo_count_1(){
        int i =0;
        int j =0;
        for(i=0;i<m_max_photo;i++)
                if(m_photoinfo[i].idle == 1)
                        j++;
        return j;
}

/**
* 判断当前图片是否锁住
*/
static bool  get_photo_lock(PHOTOINFO* pPhotoinfo,int *islock){
        if(pPhotoinfo == 0){
                *islock = 0;
                return false;
        }
        *islock = pPhotoinfo->lock;
        return true;
}

/**
*根据保存的时间对图片进行倒序排列
*/
static int time_compare(unsigned long dates1, unsigned long times1, unsigned long dates2, unsigned long times2){
        if(dates1 == dates2 && times1 == times2)
                return 0;
        if(dates1 == dates2){
                if(times1 >times2){
                        return -1;
                }else
                        return 1;
        }
        if(dates1 > dates2){
                return -1;
        }
        return 1;

}

/**
*根据保存的时间对图片进行倒序排列，回调函数
*/
static int photo_compare(const void* pData1,const void* pData2){
        PHOTOINFO* p1 = (PHOTOINFO*)pData1;
        PHOTOINFO* p2 = (PHOTOINFO*)pData2;
        return time_compare(p1->dates,p1->times,p2->dates,p2->times);
}


PHOTOINFO* get_time_pioneer_photo(){
        int i =0;
        PHOTOINFO* pTemp= NULL;
        i = m_max_photo -1;
        do{
                pTemp = get_select_photo(i);
                if(pTemp && (pTemp->lock == 0))
                        break;
                i--;
        }while(i > (-1));

        return pTemp;
}

/**
*根据保存的时间对图片进行倒序排列
*/
int  photo_sort(){
        qsort(m_photoinfo,MAX_PHOTO,sizeof(PHOTOINFO),photo_compare);
        return 1;
}

//////////////////////////////////////////////////////////////////////////
//                                                                      //
//////////////////////////////////////////////////////////////////////////


/**
*启动时从FLASH中重新读出数据，构建数据列表
*/
static void setup_photo(){
#if 1
        FILE* file = 0;
        int size1 = sizeof(m_photoinfo);
        file = fopen(photoindex,"rb");
        if(file){
                int size = fread(m_photoinfo,1,size1,file);
                if(size == size1){
                        photo_sort();
                }else{
                        memset(m_photoinfo,0,size1);
                }
                fclose(file);
                file = 0;
        }
#else
        int file = 0;
        int size1 = sizeof(m_photoinfo);
        file = open(photoindex,O_RDONLY|O_BINARY,1);
        if(file != -1){
                int size = read(file,m_photoinfo,size1);
                if(size == size1)
                        photo_sort();
                else
                        memset(m_photoinfo,0,size1);
                close(file);
                file = -1;

        }
#endif

}

/**
*保存文件到flash
*/
static int save_file(char* name,unsigned char* addr,long len){
#if 0
        FILE* file = 0;
        printf("save file name = %s\n",name);
        file = fopen(name,"wb");
        if(file){
                fseek(file,0,SEEK_SET);
                long write_size = fwrite(addr,1,len,file);
                if(write_size == len){
                        fclose(file);
                        return true;
                }
                printf("errno = %d  write_size=%d \n",errno,write_size);
                fclose(file);
                unlink(name);
        }
        printf("errno = %d \n",errno);

        return false;
#else

#if USED_OPTIMIZE_FFS
        unsigned long rc = 0;
        unsigned long tmpmsg[4];
        memset(tmpmsg,0,16);
        char* tmpname = (char*)ZENMALLOC(strlen(name)+1);
        if(tmpname == NULL)
                return 0;
        memset(tmpname,0,strlen(name)+1);
        strcpy(tmpname,name);
        char* tmpaddr = (char*)ZENMALLOC(len);
        if(tmpaddr == NULL){
                ZENFREE(tmpname);
                tmpname = NULL;
                return 0;
        }
        memset(tmpaddr,0,len);
        memcpy(tmpaddr,addr,len);
        tmpmsg[0] = (unsigned long)tmpname;
        tmpmsg[1] = (unsigned long)tmpaddr;
        tmpmsg[2] = len;
        rc = g_opt_msg_queue.send_message(MODULE_MSG,(void*)tmpmsg,sizeof(tmpmsg),g_opt_msg_sem);
        if(rc == 0)
                return 1;
        return 0;
#else
        int file = 0;
        unsigned long ticks1,ticks2;
        //  mu_lock(m_writeflashmutex,MU_WAIT,0);
        ticks1 = ticks2 = 0;
        tm_getticks(NULL,&ticks1);
        EnterMutex();
        file = open(name,O_RDWR | O_CREAT | O_TRUNC,1);
        if(file != -1){
                long write_size = write(file,addr,len);
                if(write_size == len){
                        close(file);
                        // mu_unlock(m_writeflashmutex);
                        ExitMutex();
                        tm_getticks(NULL,&ticks2);
                        return true;
                }
                close(file);
                unlink(name);
        }
        // mu_unlock(m_writeflashmutex);
        ExitMutex();
#endif
        return false;
#endif
}


/**
*从磁盘或者内存中读出图片数据
*/
unsigned char* get_photo_addr(char*name,long* len){

        //  if(g_mysysinfo->isusedffs == 1){
        if(pSystemInfo->mySysInfo.isusedffs == 1){

                FILE* file = NULL;
                long tmplen=0;
                char path[60];
                sprintf(path,"%s/%s", photodir,name);
                //mu_lock(m_writeflashmutex,MU_WAIT,0);
                EnterMutex();
                WRT_MESSAGE("阅读文件%s",name);
                file= fopen(path,"rb");
                if(file != NULL){
                        fseek(file,0,SEEK_END);
                        tmplen = ftell(file);
                        if(tmplen > MAX_BUFFER){
                                if(photo_addr){
                                        ZENFREE(photo_addr);
                                        photo_addr = (unsigned char*)ZENMALLOC(tmplen);
                                }
                        }
                        fseek(file,0,SEEK_SET);
                        long read_size = fread(photo_addr,1,tmplen,file);
                        if(read_size == tmplen){
                                fclose(file);
                                *len = tmplen;
                                WRT_MESSAGE("阅读文件%s成功",name);
                                ExitMutex();
                                return photo_addr;
                        }
                        fclose(file);
                        *len = -1;
                        ExitMutex();
                        WRT_MESSAGE("阅读文件%s失败",name);
                        return 0;
                }
                *len = -1;
                WRT_MESSAGE("阅读文件%s失败",name);
                ExitMutex();
                return 0;

        }else{
                PHOTOINFO* pTemp =  m_photoinfo;
                for(int i=0;i<m_max_photo;i++){
                        if(strcmp((char*)pTemp->name,name) == 0){
                                *len = pTemp->len;
                                return pTemp->addr;
                        }
                        pTemp++;
                }
                *len = -1;
                return 0;
        }
}

/**
*初始化图片模块
*/
void init_photo(){
        memset(m_photoinfo,0,sizeof(PHOTOINFO)* MAX_PHOTO);
        m_photocount = 0;

        if(pSystemInfo->mySysInfo.isusedffs == 1){
        	dir_is_exist(photodir1);
        	dir_is_exist(photodir);
                photo_addr = (unsigned char*)ZENMALLOC(MAX_BUFFER);//20k公共的内存。
                setup_photo();
                m_photo_index_save = 0;
                m_photocount = get_photo_count_1();
        }

        Photo_Is_No_Read();
        // mu_create("mwfa",MU_LOCAL | MU_FIFO,0,&m_writeflashmutex);

}

/**
*
*/
void save_photo(){

        if(pSystemInfo->mySysInfo.isusedffs == 1){
                if(m_photo_index_save == 1){
                        if(save_file(photoindex,(unsigned char*)m_photoinfo,sizeof(m_photoinfo))){
                                WRT_MESSAGE("保存文件%s成功",photoindex);
                        }
                        m_photo_index_save = 0;
                }
        }

}

/**
*从列表中获得空闲的一个元素
*/
PHOTOINFO* get_idle_photo(){
        int i =0;
        if(m_photocount >= m_max_photo)
                return 0;
        for(i =0;i<m_max_photo;i++){
                if(m_photoinfo[i].idle == 0){
                        m_photoinfo[i].idle  = 1;
                        m_photoinfo[i].read = 0;
                        return (PHOTOINFO* )&m_photoinfo[i];
                }
        }
        return 0;
}

/**
*
*/
int lock_photo(PHOTOINFO* pPhotoinfo,int islock){
        if(pPhotoinfo == 0)
                return 0;
        pPhotoinfo->lock = islock;
        m_photo_index_save = 1;
        return 1;
}

int set_photo_read(PHOTOINFO* pPhotoinfo,int isread){
        if(pPhotoinfo == 0)
                return 0;
        pPhotoinfo->read |= (isread&0xffff);
        m_photo_index_save = 1;
        Photo_Is_No_Read();
        return 1;
}

/**
*
*/
int add_photo(PHOTOINFO* pPhotoinfo,unsigned char* addr,long len){
        PHOTOINFO* pTemp;
        if(pPhotoinfo == 0)
                return 0;
        if(len >0 && addr != 0){
                uint32_t dates;
                uint32_t times;
                tm_get(&dates,&times,NULL);
                if(pSystemInfo->mySysInfo.isusedffs == 1){
                        char path[60];
        		if(is_has_flash_size(len) == 0)
        		{
        			memset((void*)pPhotoinfo,0,sizeof(PHOTOINFO));
                                ZENFREE(addr);
                                addr = 0;
                                pPhotoinfo->idle = 0;
               			WRT_DEBUG("磁盘空间不够，保存文件失败，请释放磁盘空间");
               			return 0;                	
        		}                        
                        sprintf(path,"%s/%s", photodir,pPhotoinfo->name);
                        if(!save_file(path,addr,len)){
                                memset((void*)pPhotoinfo,0,sizeof(PHOTOINFO));
                                ZENFREE(addr);
                                addr = 0;
                                pPhotoinfo->idle = 0;
                                return 0;
                        }

                        pPhotoinfo->dates = dates;
                        pPhotoinfo->times = times;
                        pPhotoinfo->read |=( 0 & 0xffff);
                        m_photocount++;
                        ZENFREE(addr);
                        photo_sort();//重新排序
                        m_photo_index_save = 1;
                        save_photo();
                        m_photo_no_read  = 1;

                }else{
                        pPhotoinfo->dates = dates;
                        pPhotoinfo->times = times;
                        pPhotoinfo->addr = addr;
                        pPhotoinfo->len = len;
                        pPhotoinfo->read  |=( 0 & 0xffff);
                        m_photocount++;
                        photo_sort();//重新排序
                        m_photo_index_save = 1;
                        save_photo();
                        m_photo_no_read  = 1;
                }
        }
        return 1;
}

/**
*
*/
PHOTOINFO* get_photo(int count){
        PHOTOINFO* temp;
        if(count < 0 || count > (m_max_photo-1))
                return 0;
        temp =  m_photoinfo;
        if(temp[count].idle == 1)
                return (PHOTOINFO*)(temp+count);
        return 0;

}

/**
*
*/
int get_photo_count(){
        return m_photocount;
}

/**
*
*/
PHOTOINFO* get_select_photo(int count){
        return get_photo(count);
}

/**
*
*/
int del_all_photo(){
        PHOTOINFO* pTemp;
        pTemp = m_photoinfo;
        char path[60];
        for(int i =0;i<m_max_photo;i++){
                if(pTemp->idle == 1 && pTemp->lock == 0){
                        m_photocount --;
                        if(m_photocount < 0)
                                m_photocount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = 0;
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->read = 0;

                        if(pSystemInfo->mySysInfo.isusedffs == 1){
                                sprintf(path,"%s/%s", photodir,pTemp->name);
                                unlink(path);
                        }

                        memset(pTemp->name ,0,FILE_LEN);
                        pTemp->dates = 0;
                        pTemp->times = 0;
                }
                pTemp++;
        }
        photo_sort();
        m_photo_index_save = 1;
        save_photo();
        Photo_Is_No_Read();
        return 1;
}

int free_mem_all_photo(){
}

int del_all_photo_for_factory(){
        PHOTOINFO* pTemp;
        pTemp = m_photoinfo;
        char path[60];
        for(int i =0;i<m_max_photo;i++){
                if(pTemp->idle == 1){
                        m_photocount --;
                        if(m_photocount < 0)
                                m_photocount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = 0;
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->read = 0;

                        if(pSystemInfo->mySysInfo.isusedffs == 1){
                                sprintf(path,"%s/%s", photodir,pTemp->name);
                                unlink(path);
                        }

                        memset(pTemp->name ,0,FILE_LEN);
                        pTemp->dates = 0;
                        pTemp->times = 0;
                }
                pTemp++;
        }
        m_photo_index_save  =0;
        m_photocount = 0;
        memset(m_photoinfo,0,sizeof(PHOTOINFO)* MAX_PHOTO);
        unlink(photoindex);
        m_photo_no_read = 0;
        return 1;
}

/**
*
*/
int del_photo(PHOTOINFO* pPhotoinfo){

        PHOTOINFO* pTemp =pPhotoinfo ;
        if(pTemp == 0)
                return 0;
        if(pTemp->lock == 1)
                return 0;
        char path[60];
        if(pTemp){
                if(pTemp->idle == 1 && pTemp->lock == 0){
                        m_photocount --;
                        if(m_photocount < 0)
                                m_photocount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = 0;
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->read = 0;

                        if(pSystemInfo->mySysInfo.isusedffs == 1){
                                sprintf(path,"%s/%s", photodir,pTemp->name);
                                unlink(path);
                        }

                        memset(pTemp->name ,0,FILE_LEN);
                        pTemp->dates = 0;
                        pTemp->times = 0;
                        photo_sort();
                        m_photo_index_save = 1;
                        save_photo();
                        return 1;
                }
        }
        Photo_Is_No_Read();
        return 0;

}
//////////////////////////////////////////////////////////////////////////////////////////
static PHOTOINFO           m_videoinfo[MAX_VIDEO];
static unsigned char*      video_addr=0;
static int                 m_videocount=0;
static int                 m_video_index_save = 0;
static char videodir[]="/customer/wrt/pic/video";
static char videoindex[] = "/customer/wrt/pic/video_index.txt";
static int                m_max_video = MAX_VIDEO;
static int                m_video_no_read = 0;
static void Video_Is_No_Read(){
        int i = 0;
        for(i=0;i<m_max_video;i++){
                if(m_videoinfo[i].idle == 1){
                        if((m_videoinfo[i].read & 0xffff) == 0){
                                m_video_no_read = 1;
                                return;
                        }
                }
        }
        if(i == m_max_video)
                m_video_no_read = 0;
}
int IsNewVideo(){
        return m_video_no_read;
}
int isnoreadvideo(){
        Video_Is_No_Read();
        return IsNewVideo();
}
void Set_Max_video(int maxnum){
        if(maxnum < 0 || maxnum > 100){
                return;
        }
        m_max_video = maxnum;
        pSystemInfo->mySysInfo.maxvideo = m_max_video;
}
//
static int  get_video_count_1(){
        int i =0;
        int j =0;
        for(i=0;i<m_max_video;i++)
                if(m_videoinfo[i].idle == 1)
                        j++;
        return j;
}

static bool  get_video_lock(PHOTOINFO* pVideoinfo,int *islock){
        if(pVideoinfo == 0){
                *islock = 0;
                return false;
        }
        *islock = pVideoinfo->lock;
        return true;
}
static int video_compare(const void* pData1,const void* pData2){
        PHOTOINFO* p1 = (PHOTOINFO*)pData1;
        PHOTOINFO* p2 = (PHOTOINFO*)pData2;
        return time_compare(p1->dates,p1->times,p2->dates,p2->times);
}
PHOTOINFO* get_time_pioneer_video(){
        int i =0;
        PHOTOINFO* pTemp= NULL;
        i = m_max_video -1;
        do{
                pTemp = get_select_video(i);
                if(pTemp && (pTemp->lock == 0))
                        break;
                i--;
        }while(i > (-1));
        return pTemp;
}
int  video_sort(){
        qsort(m_videoinfo,MAX_VIDEO,sizeof(PHOTOINFO),video_compare);
        return 1;
}
static void setup_video(){
#if 1
        FILE* file = 0;
        int size1 = sizeof(m_videoinfo);
        file = fopen(videoindex,"rb");
        if(file){
                int size = fread(m_videoinfo,1,size1,file);
                if(size == size1){
                        video_sort();
                }else{
                        memset(m_videoinfo,0,size1);
                }
                fclose(file);
                file = 0;
        }
#else
        int file = 0;
        int size1 = sizeof(m_photoinfo);
        file = open(photoindex,O_RDONLY|O_BINARY,1);
        if(file != -1){
                int size = read(file,m_photoinfo,size1);
                if(size == size1)
                        photo_sort();
                else
                        memset(m_photoinfo,0,size1);
                close(file);
                file = -1;
        }
#endif
}
unsigned char* get_video_addr(char*name,long* len){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE* file = NULL;
                long tmplen=0;
                char path[60];
                sprintf(path,"%s/%s", videodir,name);
                EnterMutex();
                WRT_MESSAGE("阅读文件%s",name);
                file= fopen(path,"rb");
                if(file != NULL){
                        fseek(file,0,SEEK_END);
                        tmplen = ftell(file);
                        if(tmplen > MAX_BUFFER){
                                if(video_addr){
                                        ZENFREE(video_addr);
                                        video_addr = (unsigned char*)ZENMALLOC(tmplen);
                                }
                        }
                        fseek(file,0,SEEK_SET);
                        long read_size = fread(video_addr,1,tmplen,file);
                        if(read_size == tmplen){
                                fclose(file);
                                *len = tmplen;
                                WRT_MESSAGE("阅读文件%s成功",name);
                                ExitMutex();
                                return video_addr;
                        }
                        fclose(file);
                        *len = -1;
                        ExitMutex();
                        WRT_MESSAGE("阅读文件%s失败",name);
                        return 0;
                }
                *len = -1;
                WRT_MESSAGE("阅读文件%s失败",name);
                ExitMutex();
                return 0;
        }else{
                PHOTOINFO* pTemp =  m_videoinfo;
                for(int i=0;i<m_max_video;i++){
                        if(strcmp((char*)pTemp->name,name) == 0){
                                *len = pTemp->len;
                                return pTemp->addr;
                        }
                        pTemp++;
                }
                *len = -1;
                return 0;
        }
}
void init_video(){
        memset(m_videoinfo,0,sizeof(PHOTOINFO)* MAX_VIDEO);
        m_videocount = 0;
        if(pSystemInfo->mySysInfo.isusedffs == 1){
            dir_is_exist(photodir1);
            dir_is_exist(videodir);
            video_addr = (unsigned char*)ZENMALLOC(MAX_BUFFER);//20k公共的内存。
            setup_video();
            m_video_index_save = 0;
            m_videocount = get_video_count_1();
        }
        Video_Is_No_Read();
}
void save_video(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                if(m_video_index_save == 1){
                        if(save_file(videoindex,(unsigned char*)m_videoinfo,sizeof(m_videoinfo))){
                                WRT_MESSAGE("保存文件%s成功",videoindex);
                        }
                        m_video_index_save = 0;
                }
        }
}
PHOTOINFO* get_idle_video(){
        int i =0;
        if(m_videocount >= m_max_video)
                return 0;
        for(i =0;i<m_max_video;i++){
                if(m_videoinfo[i].idle == 0){
                        m_videoinfo[i].idle  = 1;
                        m_videoinfo[i].read = 0;
                        return (PHOTOINFO* )&m_videoinfo[i];
                }
        }
        return 0;
}
int lock_video(PHOTOINFO* pVideoinfo,int islock){
        if(pVideoinfo == 0)
                return 0;
        pVideoinfo->lock = islock;
        m_video_index_save = 1;
        return 1;
}
int set_video_read(PHOTOINFO* pVideoinfo,int isread){
        if(pVideoinfo == 0)
                return 0;
        pVideoinfo->read |= (isread&0xffff);
        m_video_index_save = 1;
        Video_Is_No_Read();
        return 1;
}
int add_video(PHOTOINFO* pVideoinfo,unsigned char* addr,long len){
        PHOTOINFO* pTemp;
        if(pVideoinfo == 0)
                return 0;
                uint32_t dates;
                uint32_t times;
                tm_get(&dates,&times,NULL);
                if(pSystemInfo->mySysInfo.isusedffs == 1){
                        char path[60];
                        pVideoinfo->dates = dates;
                        pVideoinfo->times = times;
                        pVideoinfo->read |=( 0 & 0xffff);
                        m_videocount++;
                        video_sort();//重新排序
                        m_video_index_save = 1;
                        save_video();
                        m_video_no_read  = 1;
                }else{
                        pVideoinfo->dates = dates;
                        pVideoinfo->times = times;
                        pVideoinfo->addr = NULL;
                        pVideoinfo->len = 0;
                        pVideoinfo->read  |=( 0 & 0xffff);
                        m_videocount++;
                        video_sort();//重新排序
                        m_video_index_save = 1;
                        save_video();
                        m_video_no_read  = 1;
                }
        return 1;
}
PHOTOINFO* get_video(int count){
        PHOTOINFO* temp;
        if(count < 0 || count > (m_max_video-1))
                return 0;
        temp =  m_videoinfo;
        if(temp[count].idle == 1)
                return (PHOTOINFO*)(temp+count);
        return 0;
}
int get_video_count(){
        return m_videocount;
}
PHOTOINFO* get_select_video(int count){
        return get_video(count);
}
int del_all_video(){
        PHOTOINFO* pTemp;
        pTemp = m_videoinfo;
        char path[60];
        for(int i =0;i<m_max_video;i++){
                if(pTemp->idle == 1 && pTemp->lock == 0){
                        m_videocount --;
                        if(m_videocount < 0)
                                m_videocount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = 0;
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->read = 0;
                        if(pSystemInfo->mySysInfo.isusedffs == 1){
                                sprintf(path,"%s/%s", videodir,pTemp->name);
                                unlink(path);
                        }
                        memset(pTemp->name ,0,FILE_LEN);
                        pTemp->dates = 0;
                        pTemp->times = 0;
                }
                pTemp++;
        }
        video_sort();
        m_video_index_save = 1;
        save_video();
        Video_Is_No_Read();
        return 1;
}
int free_mem_all_video(){
}
int del_all_video_for_factory(){
        PHOTOINFO* pTemp;
        pTemp = m_videoinfo;
        char path[60];
        for(int i =0;i<m_max_video;i++){
                if(pTemp->idle == 1){
                        m_videocount --;
                        if(m_videocount < 0)
                                m_videocount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = 0;
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->read = 0;
                        if(pSystemInfo->mySysInfo.isusedffs == 1){
                                sprintf(path,"%s/%s", videodir,pTemp->name);
                                unlink(path);
                        }
                        memset(pTemp->name ,0,FILE_LEN);
                        pTemp->dates = 0;
                        pTemp->times = 0;
                }
                pTemp++;
        }
        m_video_index_save =0;
        m_videocount = 0;
        memset(m_videoinfo,0,sizeof(PHOTOINFO)* MAX_VIDEO);
        unlink(videoindex);
        m_video_no_read = 0;
        return 1;
}
int del_video(PHOTOINFO* pVideoinfo){
        PHOTOINFO* pTemp =pVideoinfo ;
        if(pTemp == 0)
                return 0;
        if(pTemp->lock == 1)
                return 0;
        char path[60];
        if(pTemp){
                if(pTemp->idle == 1 && pTemp->lock == 0){
                        m_videocount --;
                        if(m_videocount < 0)
                                m_videocount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = 0;
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->read = 0;
                        if(pSystemInfo->mySysInfo.isusedffs == 1){
                                sprintf(path,"%s/%s", videodir,pTemp->name);
                                unlink(path);
                        }
                        memset(pTemp->name ,0,FILE_LEN);
                        pTemp->dates = 0;
                        pTemp->times = 0;
                        video_sort();
                        m_video_index_save = 1;
                        save_video();
                        return 1;
                }
        }
        Video_Is_No_Read();
        return 0;
}

/*————————————————————————————————————————————————————————————————————
————————————————————————————————————————————————————————————————————*/
//全局静态变量


static  LOGINFO                   m_loginfo[MAX_LOG];
static  int                       m_logcount;
static  int                       m_logindex_save = 0;
static char logdir[]="/customer/wrt/log";
static char logindex[] = "/customer/wrt/log/log.txt";

/**
*根据保存的时间对日志进行倒序排列，回调函数
*/
static int log_compare(const void* pData1,const void* pData2){
        LOGINFO* p1 = (LOGINFO*)pData1;
        LOGINFO* p2 = (LOGINFO*)pData2;
        return time_compare(p1->dates,p1->times,p2->dates,p2->times);
}

static int get_log_count_1(){
        int i,j;
        i=j=0;
        for(i =0; i<MAX_LOG;i++)
                if(m_loginfo[i].idle == 1)
                        j++;
        return j;

}

LOGINFO* get_time_pioneer_log(){
        int i;
        i = MAX_LOG-1;
        LOGINFO* pTemp = NULL;
        do{
                pTemp = get_select_log(i);
                if(pTemp && (pTemp->lock == 0))
                        break;
                i--;
        }while(i > (-1));
        return pTemp;

}

static void  setup_log(){

        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE* file = NULL;
                int size1 = sizeof(m_loginfo);
                file = fopen(logindex,"rb");
                if(file != NULL){
                        int size = fread(m_loginfo,1,size1,file);
                        if(size == size1)
                                log_sort();
                        else
                                memset(m_loginfo,0,size1);
                        fclose(file);
                        file = NULL;

                }
        }

}


void log_sort(){

        qsort(m_loginfo,MAX_LOG,sizeof(LOGINFO),log_compare);

}

void init_log(){
        memset(m_loginfo,0,sizeof(LOGINFO)*MAX_LOG);
        m_logcount = 0;
        if(pSystemInfo->mySysInfo.isusedffs == 1){
        	dir_is_exist(logdir);

                setup_log();
                m_logcount =get_log_count_1();
        }

}

void save_log(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                if(m_logindex_save == 1){
                        save_file(logindex,(unsigned char*)m_loginfo,sizeof(m_loginfo));
                        m_logindex_save = 0;
                }
        }
}

LOGINFO* get_idle_log(){
        int i= 0;
        LOGINFO* pTemp;
        pTemp = m_loginfo;

        for(i =0; i<MAX_LOG;i++){
                if(pTemp[i].idle == 0){
                        pTemp[i].idle =1 ;
                        return (LOGINFO*)(pTemp+i);
                }
        }
        return 0;
}

int add_log2(char* text){
        uint32_t  times,dates;
        unsigned long day,Month,years ;
        unsigned long min,second,Hour;
        LOGINFO* pLog=get_idle_log();
        if(pLog == 0){
                int i= 1;
                while(1){
                        pLog = get_select_log(MAX_LOG-i);
                        if(pLog && pLog->idle == 1 && pLog->lock == 0)
                                break;
                        i++;
                }
                if(pLog)
                        del_log(pLog);
                pLog = get_idle_log();
                if(pLog == 0)
                        return 0;
        }
        tm_get(&dates,&times,NULL);
        day= dates&0xff;
        Month = (dates >> 8) &0xff;
        years = (dates>>16) & 0xffff;
        Hour = (times >> 16)& 0XFFFF;
        second = (times & 0xff);
        min = (times >> 8) &0xff;
        int tmplen = strlen(text);    
        sprintf((char*)pLog->addr,"%s\n",text);
        pLog->len = tmplen+25;
        pLog->times = times;
        pLog->dates = dates;
        /*
        if(pSystemInfo->mySysInfo.isusedffs== 1){
        sprintf((char*)pLog->addr,"%s %04d-%02d-%02d_%02d-%02d-%02d",text ,years,Month,day,Hour,min,second);
        pLog->len = tmplen+25;
        pLog->times = times;
        pLog->dates = dates;
        printf("pLog->addr = %s \n",pLog->addr);
        //  save_file(logindex,(unsigned char*)m_loginfo ,sizeof(m_loginfo));

        }else{
        #ifndef USED_FFS
        char* tmptext = (char*)ZENMALLOC(tmplen+25);
        sprintf(tmptext,"%s %04d-%02d-%02d_%02d-%02d-%02d",text ,years,Month,day,Hour,min,second);
        pLog->addr = (unsigned char*)tmptext;
        pLog->len = tmplen+25;
        pLog->times = times;
        pLog->dates = dates;
        #endif
        }
        */
        m_logcount++;
        //if(m_logcount > 1)
        log_sort();
        m_logindex_save = 1;
        save_log();

        return 0;
}
static int newlog = 0;

int add_log(char* text){
        uint32_t  times,dates;
        unsigned long day,Month,years ;
        unsigned long min,second,Hour;
        LOGINFO* pLog=get_idle_log();
        if(pLog == 0){
                int i= 1;
                while(1){
                        pLog = get_select_log(MAX_LOG-i);
                        if(pLog && pLog->idle == 1 && pLog->lock == 0)
                                break;
                        i++;
                }
                if(pLog)
                        del_log(pLog);
                pLog = get_idle_log();
                if(pLog == 0)
                        return 0;
        }
        tm_get(&dates,&times,NULL);
        day= dates&0xff;
        Month = (dates >> 8) &0xff;
        years = (dates>>16) & 0xffff;
        Hour = (times >> 16)& 0XFFFF;
        second = (times & 0xff);
        min = (times >> 8) &0xff;
        int tmplen = strlen(text);      

        sprintf((char*)pLog->addr,"%s %04d-%02d-%02d_%02d-%02d-%02d",text ,years,Month,day,Hour,min,second);
        pLog->len = tmplen+25;
        pLog->times = times;
        pLog->dates = dates;
        /*
        if(pSystemInfo->mySysInfo.isusedffs== 1){
        sprintf((char*)pLog->addr,"%s %04d-%02d-%02d_%02d-%02d-%02d",text ,years,Month,day,Hour,min,second);
        pLog->len = tmplen+25;
        pLog->times = times;
        pLog->dates = dates;
        printf("pLog->addr = %s \n",pLog->addr);
        //  save_file(logindex,(unsigned char*)m_loginfo ,sizeof(m_loginfo));

        }else{
        #ifndef USED_FFS
        char* tmptext = (char*)ZENMALLOC(tmplen+25);
        sprintf(tmptext,"%s %04d-%02d-%02d_%02d-%02d-%02d",text ,years,Month,day,Hour,min,second);
        pLog->addr = (unsigned char*)tmptext;
        pLog->len = tmplen+25;
        pLog->times = times;
        pLog->dates = dates;
        #endif
        }
        */
        m_logcount++;
        //if(m_logcount > 1)
        log_sort();
        m_logindex_save = 1;
        newlog++;
        if(newlog > 10){
                newlog = 0;
                save_log();
        }

        return 0;
}

int lock_log(LOGINFO* pLog,int islock){
        if(pLog == 0)
                return 0;
        pLog->lock = islock;
        m_logindex_save = 1;
        return 1;
}

int set_log_read(LOGINFO* pLog,int isread){
        if(pLog == 0)
                return 0;
        pLog->read = isread;
        m_logindex_save = 1;
        return 1;
}

int del_log(LOGINFO* pLog){
        LOGINFO* pTemp;
        if(pLog == 0)
                return 0;
        if(pLog->lock == 1)
                return 0;
        pTemp = m_loginfo;
        int size = sizeof(LOGINFO);
        for(int i=0;i<MAX_LOG;i++){
                if(pTemp == pLog ){
                        memset(pTemp,0,size);
                        log_sort();
                        m_logindex_save = 1;
                        m_logcount--;
                        save_log();
                        return 1;
                }
                pTemp++;
        }
        return 0;
}



LOGINFO* get_select_log(int count){
        if(count < 0 || count > (MAX_LOG-1))
                return 0;
        LOGINFO* pTemp = 0;
        pTemp = m_loginfo;
        return (LOGINFO*)(pTemp+count);
}


int get_log_count(){
        return m_logcount;
}
int del_all_log(){
        LOGINFO* pTemp;
        pTemp = m_loginfo;
        for(int i =0;i<MAX_LOG;i++){
                if(pTemp->idle == 1 && pTemp->lock == 0){
                        m_logcount --;

                        if(pSystemInfo->mySysInfo.isusedffs == 1)
                                memset(pTemp->addr,0,400);
                        else{
                                if(pTemp->addr)
                                        ZENFREE(pTemp->addr);
                        }

                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->dates = 0;
                        pTemp->times = 0;
                }
                pTemp++;
        }
        log_sort();
        m_logindex_save = 1;
        save_log();
        return 1;
}

int del_all_log_for_factory(){
        LOGINFO* pTemp;
        pTemp = m_loginfo;
        for(int i =0;i<MAX_LOG;i++){
                if(pTemp->idle == 1){
                        m_logcount --;

                        if(pSystemInfo->mySysInfo.isusedffs == 1)
                                memset(pTemp->addr,0,400);
                        else{
                                if(pTemp->addr)
                                        ZENFREE(pTemp->addr);
                        }

                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->dates = 0;
                        pTemp->times = 0;
                }
                pTemp++;
        }
        m_logcount = 0;
        m_logindex_save = 0;
        memset(m_loginfo,0,sizeof(LOGINFO)*100);
        unlink(logindex);
        return 1;
}

////////////////////////////////////////////////////////////////////////
///


static  MSGINFO                   m_msginfo[MAX_MSG];
static  int                       m_msgcount;
static  int                       m_msgindex_save = 0;
static char msgdir[]="/customer/wrt/msg";
static char msgindex[] = "/customer/wrt/msg/msg.txt";


static int msg_compare(const void* pData1,const void* pData2){
        MSGINFO* p1 = (MSGINFO*)pData1;
        MSGINFO* p2 = (MSGINFO*)pData2;
        return time_compare(p1->dates,p1->times,p2->dates,p2->times);
}

static int get_msg_count_1(){
        MSGINFO* pTemp;
        int i,j;
        i=j=0;
        pTemp = m_msginfo;
        for(i=0;i<MAX_MSG;i++){
                if(pTemp && pTemp->idle == 1)
                        j++;
                pTemp++;
        }
        return j;
}

void msg_sort(){
        qsort(m_msginfo,MAX_MSG,sizeof(MSGINFO),msg_compare);
}

MSGINFO* get_time_pioneer_msg(){
        int i = 0;
        MSGINFO* pTemp = NULL;
        i = MAX_MSG-1;
        do{
                pTemp  = get_select_msg(i);
                if(pTemp && (pTemp->lock == 0))
                        break;
                i--;
        }while(i > (-1));
        return pTemp;
}


static void setup_msg(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE* file = NULL;
                int size1 = sizeof(m_msginfo);
                file = fopen(msgindex,"rb");
                if(file != NULL){
                        int size = fread(m_msginfo,1,size1,file);
                        if(size == size1)
                                msg_sort();
                        else
                                memset(m_msginfo,0,size1);
                        fclose(file);
                        file = NULL;

                }
        }
}



void init_msg()
{
        memset(m_msginfo,0,sizeof(MSGINFO)*MAX_MSG);
        m_msgcount = 0;
        if(pSystemInfo->mySysInfo.isusedffs == 1){
		dir_is_exist(msgdir);
                setup_msg();
                m_msgcount = get_msg_count_1();
        }


}

void save_msg(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                if(m_msgindex_save == 1){
                        save_file(msgindex,(unsigned char*)m_msginfo,sizeof(m_msginfo));
                        m_msgindex_save = 0;
                }
        }
}

MSGINFO* get_idle_msg()
{
        MSGINFO* ptemp = m_msginfo;
        int i= 0;
        for(i =0; i<MAX_MSG;i++){
                if(ptemp[i].idle == 0){
                        ptemp[i].idle = 1;
                        return (MSGINFO*)(ptemp+i);
                }
        }

        return 0;
}
unsigned char* get_photo_msg(char* name,int* len)
{

        if(pSystemInfo->mySysInfo.isusedffs == 1){

                FILE *file = NULL;
                long tmplen=0;
                char path[60];
                unsigned char* photobuf = NULL;
                sprintf(path,"%s/%s", msgdir,name);
                //mu_lock(m_writeflashmutex,MU_WAIT,0);
                EnterMutex();
                file= fopen(path,"rb");
                if(file != NULL){
                        fseek(file,0,SEEK_END);
                        tmplen = ftell(file);
                        photobuf = (unsigned char*)ZENMALLOC(tmplen);
                        if(photobuf == NULL){
                                fclose(file);
                                *len =0;
                                file = NULL;
                                ExitMutex();
                                return NULL;
                        }
                        fseek(file,0,SEEK_SET);
                        long read_size = fread(photobuf,1,tmplen,file);
                        if(read_size == tmplen){
                                fclose(file);
                                *len = tmplen;
                                file = NULL;
                                ExitMutex();
                                return photobuf;
                        }
                        fclose(file);
                        file = NULL;
                        *len = 0;
                        ZENFREE(photobuf);
                        photobuf = NULL;
                        
                        ExitMutex();
                        return 0;
                }
                *len = 0;
                ExitMutex();
                return 0;
        }
        *len = 0;
        return NULL;
}

int add_msg2(char* name,unsigned char* photobuf,int buflen){
        uint32_t times,dates;

        MSGINFO* pMsg;
        if(photobuf == NULL)
                return 0;
        char path[60];
        if(is_has_flash_size(buflen) == 0)
        {
               WRT_DEBUG("磁盘空间不够，保存文件失败，请释放磁盘空间");
               return 0;                	
        }        
        if((pMsg = get_idle_msg()) == 0){
                int i =1;
                while(1){
                        pMsg = get_select_msg(MAX_MSG-i);
                        if(pMsg && pMsg->idle == 1 && pMsg->lock == 0)
                                break;
                        i++;
                }
                if(pMsg)
                        del_msg(pMsg);
                pMsg = get_idle_msg();
                if(pMsg == 0){
                        return 0;
                }
        }
        int tmplen = strlen(name);
        tm_get(&dates,&times,NULL);
        sprintf((char*)pMsg->addr,"%s",name);

        pMsg->len = tmplen;
        pMsg->read = 2;
        pMsg->dates = dates;
        pMsg->times = times;
        sprintf(path,"%s/%s",msgdir,name);
        if(save_file(path,photobuf,buflen) == 0){
                memset(pMsg,0,sizeof(MSGINFO));
                return 0;
        }

        m_msgcount++;
        msg_sort();
        m_msgindex_save = 1;
        save_msg();
        return 1;
}

int add_msg(char* text)
{
       uint32_t times,dates;

        MSGINFO* pMsg;
        if((pMsg = get_idle_msg()) == 0){
                int i =1;
                while(1){
                        pMsg = get_select_msg(MAX_MSG-i);
                        if(pMsg && pMsg->idle == 1 && pMsg->lock == 0)
                                break;
                        i++;
                }
                if(pMsg)
                        del_msg(pMsg);
                pMsg = get_idle_msg();
                if(pMsg == 0){
                        return 0;
                }
        }
        int tmplen = strlen(text);
        tm_get(&dates,&times,NULL);
        sprintf((char*)pMsg->addr,"%s",text);
        pMsg->len = tmplen;
        /*   if(pSystemInfo->mySysInfo.isusedffs== 1){
        sprintf((char*)pMsg->addr,"%s",text);
        pMsg->len = tmplen;
        }else{
        #ifndef USED_FFS
        char* tmptext = (char*)ZENMALLOC(tmplen+1);
        sprintf(tmptext,"%s",text);
        pMsg->addr = (unsigned char*)tmptext;
        pMsg->len = tmplen;
        #endif
        }*/
        pMsg->dates = dates;
        pMsg->times = times;
        m_msgcount++;
        msg_sort();
        m_msgindex_save = 1;
        save_msg();
        return 1;
}
int isnoreadmsg(){
        int ret = 0;
        MSGINFO* pMsg =m_msginfo;
        for(int i=0;i<MAX_MSG;i++){
                if(pMsg->idle == 1 && (pMsg->read == 0 || pMsg->read == 2)){
                        ret = 1;
                        break;
                }
                pMsg++;
        }
        return ret;
}

int lock_msg(MSGINFO* pMsg,int islock){
        if(pMsg == 0)
                return 0;
        pMsg->lock = islock;
        m_msgindex_save = 1;
        return 1;
}

int set_msg_read(MSGINFO* pMsg,int isread){
        if(pMsg == 0)
                return 0;
        pMsg->read = isread;
        m_msgindex_save = 1;
        return 1;
}

int del_msg(MSGINFO* pMsg)
{
        int ret =0;
        MSGINFO* pTemp;
        if(pMsg == 0)
                return 0;
        if(pMsg->lock == 1)
                return 0;
        pTemp = m_msginfo;
        for(int i=0;i<MAX_MSG;i++){
                if(pTemp == pMsg && pMsg->lock == 0){
                        if(pSystemInfo->mySysInfo.isusedffs== 1){
                                if(pMsg->read == 2 || pMsg->read == 3){
                                        char path[60];
                                        sprintf(path,"%s/%s", msgdir,pMsg->addr);
                                        unlink(path);
                                }
                        }
                        memset(pMsg,0,sizeof(MSGINFO));
                        msg_sort();
                        m_msgcount--;
                        if(m_msgcount < 0)
                                m_msgcount = 0;
                        m_msgindex_save  = 1;
                        save_msg();
                        return 1;
                }
                pTemp++;
        }
        return 0;
}



MSGINFO* get_select_msg(int count)
{
        if(count < 0 || count > (MAX_MSG-1))
                return 0;
        MSGINFO* pTemp = 0;
        int i =0;
        pTemp = m_msginfo;
        return (MSGINFO*)(pTemp+count);
}

int get_msg_count(){
        return m_msgcount;
}

int del_all_msg(){
        MSGINFO* pTemp;
        pTemp = m_msginfo;
        for(int i =0;i<MAX_MSG;i++){
                if(pTemp->idle == 1 && pTemp->lock == 0){
                        m_msgcount --;
                        if(m_msgcount < 0)
                                m_msgcount = 0;
                        if(pSystemInfo->mySysInfo.isusedffs== 1){
                                if(pTemp->read == 2 || pTemp->read == 3){
                                        char path[60];
                                        sprintf(path,"%s/%s", msgdir,pTemp->addr);
                                        unlink(path);
                                }
                                memset(pTemp->addr,0,400);
                        }else{
                                if(pTemp->addr)
                                        ZENFREE(pTemp->addr);
                        }
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->dates = 0;
                        pTemp->times = 0;
                        pTemp->read = 0;

                }
                pTemp++;
        }
        msg_sort();
        m_msgindex_save = 1;
        save_msg();
        return 1;
}

int del_all_msg_for_factory(){
        MSGINFO* pTemp;
        pTemp = m_msginfo;
        for(int i =0;i<MAX_MSG;i++){
                if(pTemp->idle == 1){
                        m_msgcount --;
                        if(m_msgcount < 0)
                                m_msgcount = 0;
                        if(pSystemInfo->mySysInfo.isusedffs== 1){
                                if(pTemp->read == 2 || pTemp->read == 3){
                                        char path[60];
                                        sprintf(path,"%s/%s", photodir,pTemp->addr);
                                        unlink(path);
                                }
                                memset(pTemp->addr,0,400);
                        }else{
                                if(pTemp->addr)
                                        ZENFREE(pTemp->addr);
                        }
                        pTemp->len = 0;
                        pTemp->idle = 0;
                        pTemp->dates = 0;
                        pTemp->times = 0;
                        pTemp->read = 0;

                }
                pTemp++;
        }
        memset(m_msginfo,0,sizeof(MSGINFO)*MAX_MSG);
        m_msgcount = 0;
        m_msgindex_save = 0;
        unlink(msgindex);
        return 1;
}

/*-------------------------------------------------------------
--------------------------------------------------------------*/
/*——————————————————————————————
声音留处理
——————————————————————————————*/
//全局静态变量

static  VOICEINFO                 m_voiceinfo[MAX_VOICE];
static  int                       m_voicecount = 0;
static  int                       m_voiceindex_save = 0;
static  char voicedir[]="/customer/wrt/voice";
static  char voiceindex[] = "/customer/wrt/voice/voice_index.txt";

/**
*/
static int voice_compare(const void* pData1,const void* pData2){
        VOICEINFO* p1 = (VOICEINFO*)pData1;
        VOICEINFO* p2 = (VOICEINFO*)pData2;
        return time_compare(p1->dates,p1->times,p2->dates,p2->times);
}

/**
*/
static int get_voice_count_1(){
        VOICEINFO* pTemp;
        int i,j;
        i=j=0;
        pTemp = m_voiceinfo;
        for(i=0;i<MAX_VOICE;i++){
                if(pTemp && pTemp->idle == 1){
                        j++;
                }
                pTemp++;
        }
        return j;
}



/**
*/
 VOICEINFO* get_time_pioneer_voice(){
        int i =0;
        i = MAX_VOICE-1;
        VOICEINFO* pTemp = NULL;
        do{
                pTemp = get_select_voice(i);
                if(pTemp && (pTemp->lock == 0))
                        break;
                i--;
        }while( i  > (-1));


        return pTemp;
}

/**
*/
static bool  get_voice_lock(VOICEINFO* pVoice,int *islock){
        if(pVoice == 0)
                return false;
        *islock = pVoice->lock;
        return true;
}

/**
*/
void voice_sort(){
        qsort(m_voiceinfo,MAX_VOICE,sizeof(VOICEINFO),voice_compare);
}


/**
*/
static void setup_voice(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE *file = NULL;
                int size1 = sizeof(m_voiceinfo);
                file = fopen(voiceindex,"rb");
                if(file != NULL){
                        int size = fread(m_voiceinfo,1,size1,file);
                        if(size == size1)
                                voice_sort();
                        else
                                memset(m_voiceinfo,0,size1);
                        fclose(file);
                        file = NULL;

                }
        }
}





/**
*/
void init_voice(){
        int i;
        memset(m_voiceinfo,0,sizeof(m_voiceinfo));
        m_voicecount = 0;
        if(pSystemInfo->mySysInfo.isusedffs == 1){
        	dir_is_exist(voicedir);
                setup_voice();
                m_voicecount = get_voice_count_1();
        }
        for(i =0; i< MAX_VOICE;i++){
                m_voiceinfo[i].addr = 0;
                m_voiceinfo[i].len = 0;
        }


}

void save_voice(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                if(m_voiceindex_save == 1){
                       save_file(voiceindex,(unsigned char*)m_voiceinfo,sizeof(m_voiceinfo));
                        m_voiceindex_save = 0;
                }
        }
}

/**
*/
unsigned char* get_voice_addr(char*name,long* len){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                FILE* file;
                long tmplen=0;
                unsigned char* voice_addr =0;

                char path[60];
                sprintf(path,"%s/%s", voicedir,name);
                EnterMutex();
                file= fopen(path,"rb");
                if(file != NULL){
                        fseek(file,0,SEEK_END);
                        tmplen =ftell(file);
                        voice_addr = (unsigned char*)ZENMALLOC(tmplen);
                        fseek(file,0,SEEK_SET);
                        long read_size = fread(voice_addr,1,tmplen,file);
                        if(read_size == tmplen){
                                fclose(file);
                                *len = tmplen;
                                ExitMutex();
                                return voice_addr;
                        }
                        fclose(file);
                        *len = -1;
                        ExitMutex();
                        return 0;
                }
                *len = -1;
                ExitMutex();
                return 0;
        }else{
                VOICEINFO* pTemp =  m_voiceinfo;
                for(int i=0;i<MAX_VOICE;i++){
                        if(strcmp((char*)pTemp->name,(char*)name) == 0){
                                *len = pTemp->len;
                                return pTemp->addr;
                        }
                        pTemp++;
                }
                *len = -1;
                return 0;
        }
}

/**
*/
VOICEINFO* get_idle_voice(){
        int i= 0;
        VOICEINFO* pTemp =  m_voiceinfo;
        for(i =0; i<MAX_VOICE;i++){
                if(pTemp[i].idle == 0){
                        pTemp[i].idle = 1;
                        pTemp[i].read = 0;
                        return (VOICEINFO*)(pTemp+i);
                }
        }
        pTemp = get_time_pioneer_voice();
        if(pTemp)
                del_voice(pTemp);
        pTemp =m_voiceinfo;
        for(i =0; i<MAX_VOICE;i++){
                if(pTemp[i].idle == 0){
                        pTemp[i].idle = 1;
                        return (VOICEINFO*)(pTemp+i);
                }
        }
        return 0;
}

/**
*判断是否存在未读的留言，1，有未读，0，没有未读的留言。
*2009-8-5 15:46:48
*/
int  isnoreadvoice(){
        int i= 0;
        VOICEINFO* pTemp =  m_voiceinfo;
        for(i =0; i<MAX_VOICE;i++){
                if(pTemp[i].idle == 1){
                        if((pTemp[i].read & 0xffff) == 0){
                        		
                                return 1;
                        }
                }
        }
        return 0;
}

/**
*/
int lock_voice(VOICEINFO* pVoice,int islock){
        if(pVoice == 0)
                return 0;
        pVoice->lock = islock;
        m_voiceindex_save = 1;
}

int set_voice_read(VOICEINFO* pVoice,int isread){
        if(pVoice == 0)
                return 0;
        pVoice->read |= (isread & 0xffff);
        m_voiceindex_save = 1;
}

/**
*/
int add_voice(VOICEINFO* pVoice,unsigned char* addr,long len){
        VOICEINFO* pTemp;
        if(pVoice == 0)
                return 0;
        if(len >0 && addr != 0){
                uint32_t dates;
                uint32_t times;
                tm_get(&dates,&times,NULL);
                printf("h\n");
                if(pSystemInfo->mySysInfo.isusedffs == 1){
                        char path[60];
                        if(is_has_flash_size(len) == 0)
		        		{
		        		 	memset((void*)pVoice,0,sizeof(VOICEINFO));
		                                ZENFREE(addr);
		                                addr = 0;
		                    printf("l\n");
		        			WRT_DEBUG("磁盘空间不够，保存文件失败请释放磁盘空间");
		        			return 0;                	
		        		}
                        sprintf(path,"%s/%s", voicedir,pVoice->name);
                        if(!save_file(path,addr,len)){
                                // memset(pPhotoinfo->name,0,FILE_LEN);
                                memset((void*)pVoice,0,sizeof(VOICEINFO));
                                ZENFREE(addr);
                                addr = 0;
                                return 0;
                        }

                        pVoice->dates = dates;
                        pVoice->times = times;
                        pVoice->read  |=  (0 & 0xffff);
                       
                        m_voicecount++;
                        ZENFREE(addr);
                        voice_sort();//重新排序
                        m_voiceindex_save = 1;
                        
                        save_voice();
                        

                }else{
                		printf("j\n");
                        pVoice->dates = dates;
                        pVoice->times = times;
                        pVoice->addr = addr;
                        pVoice->len = len;
                        pVoice->read  |=  (0 & 0xffff);
                        m_voicecount++;
                        voice_sort();//重新排序
                        m_voiceindex_save = 1;
                        printf("k\n");
                        save_voice();
                        ZENFREE(addr);
                }
        }else{
        		printf("u\n");
                memset(pVoice,0,sizeof(VOICEINFO));
        }
        return 1;
}

/**
*/
int del_voice(VOICEINFO* pVoice){
        VOICEINFO* pTemp;
        if(pVoice == 0)
                return 0;
        if(pVoice->lock == 1)
                return 0;
        char path[60];
        pTemp = m_voiceinfo;
        for(int i=0;i<MAX_VOICE;i++){
                if(pTemp->lock== 0 && pTemp == pVoice){
                        sprintf(path,"%s/%s", voicedir,pVoice->name);
                        unlink(path);
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        memset(pTemp,0,sizeof(VOICEINFO));
                        m_voicecount --;
                        if(m_voicecount < 0)
                                m_voicecount = 0;
                        voice_sort();
                        m_voiceindex_save = 1;
                        save_voice();
                        return 1;
                }
                pTemp++;
        }
        return 0;

}

void free_mem_all_voice(){
        VOICEINFO* pTemp = NULL;
        pTemp = m_voiceinfo;
        for(int i=0;i<MAX_VOICE;i++){
                if(pTemp->idle == 1){
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        pTemp->addr = NULL;
                        pTemp->len = 0;
                }
                pTemp++;
        }
        m_voiceindex_save = 1;
        save_voice();
}

/**
*/
VOICEINFO* get_select_voice(int count){
        if(count < 0 || count > (MAX_VOICE-1))
                return 0;
        VOICEINFO* pTemp = m_voiceinfo;
        return (VOICEINFO*)(pTemp+count);
}

/**
*/
int get_voice_count(){
        return m_voicecount;
}

/**
*/
int del_all_voice(){
        VOICEINFO* pTemp;
        char path[60];
        pTemp = m_voiceinfo;
        for(int i =0;i<MAX_VOICE;i++){
                if(pTemp->lock == 0 && pTemp->idle == 1){
                        m_voicecount --;
                        if(m_voicecount < 0)
                                m_voicecount = 0;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        if(pSystemInfo->mySysInfo.isusedffs == 1) {
                                sprintf(path,"%s/%s", voicedir,pTemp->name);
                                unlink(path);
                        }
                        memset(pTemp,0,sizeof(VOICEINFO));

                }
                pTemp++;
        }
        voice_sort();
        m_voiceindex_save = 1;
        save_voice();
        return 1;
}

int del_all_voice_forfactory(){
        VOICEINFO* pTemp;
        char path[60];
        pTemp = m_voiceinfo;
        for(int i =0;i<MAX_VOICE;i++){
                if(pTemp->idle == 1){
                        m_voicecount --;
                        if(pTemp->addr)
                                ZENFREE(pTemp->addr);
                        if(pSystemInfo->mySysInfo.isusedffs == 1) {
                                sprintf(path,"%s/%s", voicedir,pTemp->name);
                                unlink(path);
                        }

                        memset(pTemp,0,sizeof(VOICEINFO));

                }
                pTemp++;
        }
        m_voiceindex_save =0;
        m_voicecount =0;
        memset(m_voiceinfo,0,sizeof(m_voiceinfo));
        unlink(voiceindex);
        return 1;
}


/*————————————————————————————————————————————————————————————————————
已下载铃声的处理
————————————————————————————————————————————————————————————————————*/
//全局静态变量

static  DOWNRINGINFO              m_downringinfo[MAX_RING];
static  char downringdir[]="/customer/wrt/ring";
static  char  default_zh_ringdir[]="/home/wrt/zh/ring";
static  char  default_en_ringdir[]="/home/wrt/en/ring";

////////////////////////////////////////////////////////
//
//默认铃声的获取
static void GetDefaultRing(){

}

static int get_downring_count_1(){


}

static int downring_compare(const void* pData1,const void* pData2){

}

void downring_sort(){

}


static void setup_downring(){
    
}

static void get_dir_ring(char* path)
{
	DIR* mydir;
	char gbbuf[256];
	char unibuf[256];
	mydir = opendir(path);
	if(mydir){
		    struct  dirent  *ptr=NULL;
		    DOWNRINGINFO* ringinfo = NULL;
		    int i =0 ;
		    while((ptr = readdir(mydir)) != NULL){
		    	
		    	        if((ptr->d_type &DT_DIR))
                			continue;
                		memset(gbbuf,0,256);
                		memset(unibuf,0,256);
                		int  i =0;
                		if(strstr(ptr->d_name,".mp3") == NULL)
                			continue;
                		if(strcmp(path,downringdir) == 0){
                			strcpy(gbbuf,ptr->d_name);
                		}else{
                			WRT_DEBUG("utf8buf = %s ",ptr->d_name);
                			utf8ToUcs2((const unsigned char*)ptr->d_name,(unsigned short*)unibuf,256);
                			WRT_DEBUG("unibuf = %s ",unibuf);             		
                			uni2gb((const unsigned short*)unibuf,(unsigned char*)gbbuf,256);
                			WRT_DEBUG("gbbuf = %s ",gbbuf);             		
                		}
		    	  			ringinfo = get_idle_downring();
                       		 if(ringinfo){
                       		 	memset(ringinfo->name,0,MAX_FILE_LEN);
                                	sprintf(ringinfo->name,"%s/%s",path,gbbuf);
                                	WRT_MESSAGE("ringinfo->idle=%d\n",ringinfo->idle);
                        	}
                        	ringinfo = NULL;
                       
		    }
		    closedir(mydir);
		    mydir = NULL;
	} 
}


void init_downring(){
      memset(m_downringinfo,0,sizeof(m_downringinfo));
      DIR* mydir = opendir(downringdir);
      if(mydir == NULL){
      		mkdir(downringdir,1);
      		//return ;//注释的话，第一次启动铃声列表没声音
      } 
      if(mydir != NULL)
      	closedir(mydir);
      mydir = NULL;
      //获取默认的铃声
      get_dir_ring(downringdir);
      if(GetCurrentLanguage() == ENGLISH){
      		get_dir_ring(default_en_ringdir);
      }else
      		get_dir_ring(default_zh_ringdir);
      //获得已下载的铃声。
}

void save_downring(){

}

int lock_downring(DOWNRINGINFO* pdownringinfo,int islock){

}

static DOWNRINGINFO*  get_ring_by_name(char* name){

}

void set_english_ring()
{
	init_downring();//重新初始化即可

}
void set_chinese_ring(){
	init_downring(); //重新初始化即可
}

/**
*请转入映射的名称
*/
unsigned char* get_downring_addr(char*name,long* len){

}

DOWNRINGINFO* get_idle_downring(){ //获取空闲的时候，查找是否存在足够的存储空间。
        int i= 0;
        DOWNRINGINFO* pTemp = m_downringinfo;
        for(i =0; i<MAX_RING;i++){
                if(pTemp[i].idle == 0){
                        pTemp[i].idle = 1;
                        return (DOWNRINGINFO*)(pTemp+i);
                }
        }
        return 0;	
}




int del_downring(DOWNRINGINFO* pdownring){
    char cmd[MAX_FILE_LEN];
    sprintf(cmd,"rm -rf %s",pdownring->name);
    system(cmd);
}


DOWNRINGINFO* get_select_downring(int count){
        if(count < 0 || count > (MAX_RING-1))
                return 0;
        DOWNRINGINFO* pTemp = &m_downringinfo[count];
        return (DOWNRINGINFO*)(pTemp);
}


int get_downring_count(){
 	int i= 0;
 	int j = 0;
        DOWNRINGINFO* pTemp = m_downringinfo;
        for(i =0; i<MAX_RING;i++){
                if(pTemp[i].idle == 1){
                      j++;
                }
        }
        return j;	
}

int del_all_downring(){
	
    char cmd[MAX_FILE_LEN];
    sprintf(cmd,"rm -rf %s/*.mp3",downringdir);
    system(cmd);
    return 1;
}

void free_all_ring_cache()
{

}


void free_mem_all_downring(){

}

int del_all_downring_for_factory(){
	char cmd[MAX_FILE_LEN];
    	sprintf(cmd,"rm -rf %s/*.mp3",downringdir);
    	system(cmd);
        return 1;

}
/////////////////////////////////////////////////////////
//
static RINGLIST*           m_pringlist=0;

RINGLIST* GetRingInfo(char* name)
{
        if(m_pringlist == NULL)
                return NULL;
        RINGLIST* pTemp = m_pringlist;
        while(pTemp){
                if(strcmp(pTemp->name,name) == 0){
                        break;
                }
                pTemp = pTemp->next;
        }
        return pTemp;
}

bool AddRingToList(char* name){
        if(m_pringlist == NULL){
                int len = sizeof(RINGLIST);
                RINGLIST* pTemp = (RINGLIST*)ZENMALLOC(len);
                memset(pTemp,0,len);
                pTemp->isdown = RingIsDown(name);
                strcpy(pTemp->name,name);
                pTemp->next = NULL;
                m_pringlist = pTemp;
                return pTemp->isdown;
        }
        RINGLIST* pTemp1 = NULL;
        RINGLIST* pTemp = m_pringlist;
        do{
                pTemp1=pTemp;
                pTemp = pTemp->next;
        }while(pTemp);
        int len = sizeof(RINGLIST);
        RINGLIST* pTemp2 = (RINGLIST*)ZENMALLOC(len);
        memset(pTemp2,0,len);
        pTemp2->isdown = RingIsDown(name);
        strcpy(pTemp2->name,name);
        pTemp2->next = NULL;
        pTemp1->next = pTemp2;
        return pTemp2->isdown;

}
void DelAllRingFromList(){
        if(m_pringlist == NULL)
                return;
        RINGLIST* pTemp1 = NULL;
        RINGLIST* pTemp = m_pringlist;
        do{
                pTemp1 = pTemp;
                pTemp = pTemp->next;
                if(pTemp1->addr)
                        ZENFREE(pTemp1->addr);
                pTemp1->addr = 0;
                pTemp1->len = 0;
                ZENFREE(pTemp1);
                pTemp1 = NULL;
        }while(pTemp);
        m_pringlist = NULL;
}

int AddRingToDownList(char* name,int content_len,unsigned char* buf)
{
	char path[MAX_FILE_LEN];
	memset(path,0,MAX_FILE_LEN);
   	sprintf(path,"%s/%s", downringdir,name);
    if(!save_file(path,buf,content_len)){
    	return -1;
    }
    return 1;
}

bool RingIsDown(char* name)
{
        DOWNRINGINFO* tmp = 0;
        int i =0;
        do{
                tmp = get_select_downring(i);
                if(tmp){
                        if(strstr(tmp->name,name) != NULL)
                                return true;
                }
                i++;
        }while(tmp != 0);
        return false;
}
RINGLIST* GetFirstRingElement(){
        if(m_pringlist == 0)
                return 0;
        return m_pringlist;
}
RINGLIST* GetSelectRingElement(int index){
        int count = 0;
        if(m_pringlist == NULL)
                return NULL;
        count = index;
        //   count  = (m_cur_ring_page-1)*10+index;
        RINGLIST* pTemp = m_pringlist;
        while(count--){
                if(pTemp)
                        pTemp = pTemp->next;
                else
                        break;
        }
        return pTemp;
}
int GetRingListCount(){
        if(m_pringlist == NULL)
                return 0;
        int count = 0;
        RINGLIST* pTemp = m_pringlist;
        while(pTemp){
                count++;
                pTemp = pTemp->next;
        }
        return count;
}

//////////////////////////////////////////////////////////////////
//
static FEELIST* m_pfeelist=0;

FEELIST* GetFirstFeeElement(){
        return m_pfeelist;
}

void AddFeeToList(unsigned char month,unsigned char* buf)
{

        int templen = sizeof(FEELIST);
        if(m_pfeelist == NULL){
                FEELIST* pTemp = (FEELIST*)ZENMALLOC(templen);
                memset(pTemp,0,templen);
                pTemp->month = month;
                strcpy((char*)pTemp->buf,(char*)buf);
                pTemp->next = NULL;
                m_pfeelist = pTemp;
                return;
        }
        FEELIST* pTemp1 = NULL;
        FEELIST* pTemp = m_pfeelist;
        do{
                pTemp1 = pTemp;
                pTemp = pTemp->next;
        }while(pTemp);
        FEELIST* pTemp2 = (FEELIST*)ZENMALLOC(templen);
        memset(pTemp2,0,templen);
        pTemp2->month = month;
        strcpy((char*)pTemp2->buf,(char*)buf);
        pTemp2->next = NULL;
        pTemp1->next = pTemp2;
}

void DelAllFeeFromList()
{
        if(m_pfeelist == NULL)
                return;
        FEELIST* pTemp1 = NULL;
        FEELIST* pTemp = m_pfeelist;
        do{
                pTemp1 = pTemp;
                pTemp = pTemp->next;
                ZENFREE(pTemp1);
                pTemp1 = NULL;
        }while(pTemp);
        m_pfeelist = NULL;
}

int GetFeeListCount(){
        if(m_pfeelist == NULL)
                return 0;
        int count = 0;
        FEELIST* pTemp = m_pfeelist;
        while(pTemp){
                count++;
                pTemp = pTemp->next;
        }
        return count;
}

/////////////////////////////////////////////////////////////////////////////////
//电话本的处理

static TELEPHONE     g_telephone[MAX_TELEPHONE];
static  int                         m_telephonecount;
static  int                         m_telephoneindex_save = 0;
static char telephonedir[] = "/customer/wrt/telephone";
static char telephoneindex[] = "/customer/wrt/telephone/telephone.txt";

static int get_telephone_count_1(){
        int i,j;
        i = j = 0;
        TELEPHONE* pTemp =(TELEPHONE*) g_telephone;
        for(i=0;i<MAX_TELEPHONE;i++,pTemp++){
                if(pTemp->idle == 1){
                        j++;
                }
        }
        return j;
}

static int telephone_compare(const void* pData1,const void* pData2){
	TELEPHONE* p1 = (TELEPHONE*)pData1;
	TELEPHONE* p2 = (TELEPHONE*)pData2;
	return time_compare(p1->dates,p1->times,p2->dates,p2->times);
}

void telephone_sort(){
	qsort(g_telephone,MAX_TELEPHONE,sizeof(TELEPHONE),telephone_compare);
}

static void setup_telephone(){
	if(pSystemInfo->mySysInfo.isusedffs== 1){
		FILE *file = NULL;
		int size1 = sizeof(g_telephone);
		file = fopen(telephoneindex,"rb");
		if(file != NULL){
			
			int size = fread(g_telephone,1,size1,file);
			if(size == size1)
			    telephone_sort();
			else
			    memset(g_telephone,0,size1);
			
			fclose(file);
			file = NULL;
		}
	}
}

void init_telephone(){
	memset(g_telephone,0,sizeof(TELEPHONE)*MAX_TELEPHONE);
	m_telephonecount = 0;
	if(pSystemInfo->mySysInfo.isusedffs == 1){
		dir_is_exist(telephonedir);

		setup_telephone();
		m_telephonecount = get_telephone_count_1();
	}
}


void save_telephone(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
                if(m_telephoneindex_save == 1){
                        save_file(telephoneindex,(unsigned char*)g_telephone,sizeof(g_telephone));
                                
                        m_telephoneindex_save = 0;
                }
        }
}

TELEPHONE* get_idle_telephone(){
        int i= 0;
        for(i =0; i<MAX_TELEPHONE;i++){
                if(g_telephone[i].idle == 0){
                        g_telephone[i].idle =1 ;
                        return (TELEPHONE*)&g_telephone[i];
                }
        }
        return 0;
}


int add_telephone(TELEPHONE* ptelephone){
        if(ptelephone == NULL)
                return 0;

        uint32_t times,dates;
        tm_get(&dates,&times,NULL);
        ptelephone->dates = dates;
        ptelephone->times = times;
        m_telephonecount++;
        telephone_sort();
        m_telephoneindex_save = 1;
        save_telephone();

        return 1;


}

int del_telephone(TELEPHONE* ptelephone){
        TELEPHONE* pTemp;
        if(ptelephone == 0)
                return 0;
        if(ptelephone->lock == 1)
                return 0;
        pTemp = g_telephone;
        for(int i=0;i<MAX_TELEPHONE;i++){
                if(pTemp == ptelephone && pTemp->lock == 0 ){
                        memset(pTemp,0,sizeof(TELEPHONE));
                        telephone_sort();
                        m_telephonecount--;
                        if(m_telephonecount < 0)
                                m_telephonecount = 0;
                        m_telephoneindex_save = 1;
                        save_telephone();
                        return 1;
                }
                pTemp++;
        }
        return 0;

}

int lock_telephone(TELEPHONE* pTelephone,int islock){
        if(pTelephone == 0)
                return 0;
        pTelephone->lock = islock;
        m_telephoneindex_save = 1;
        return 1;
}


TELEPHONE* get_select_telephone(int count){
        if(count < 0 || count > (MAX_TELEPHONE-1))
                return 0;
        TELEPHONE* pTemp = g_telephone;
        return (TELEPHONE*)(pTemp+count);
}

int get_telephone_count(){
        return m_telephonecount;
}

int del_all_telephone(){
        TELEPHONE* pTemp;
        pTemp = g_telephone;
        for(int i=0;i<MAX_TELEPHONE;i++){
                if(pTemp && pTemp->idle == 1 && pTemp->lock == 0 ){
                        memset(pTemp,0,sizeof(TELEPHONE));
                        m_telephonecount--;
                        if(m_telephonecount < 0)
                                m_telephonecount = 0;
                        m_telephoneindex_save = 1;

                }
                pTemp++;
        }
        telephone_sort();
        save_telephone();
        //  m_telephonecount = 0;
        /*
        memset(g_telephone,0,sizeof(g_telephone));
        if(pSystemInfo->mySysInfo.isusedffs == 1)
        unlink(telephoneindex);

        m_telephonecount =0;
        */
}

TELEPHONE* modify_telephone(char* number){
        TELEPHONE* pTemp;
        pTemp = g_telephone;
        for(int i =0; i<MAX_TELEPHONE;i++){
                if(strcmp(pTemp->room_number,number) == 0){
                        return pTemp;
                }
                pTemp++;
        }
        return NULL;
}
//////////////////////////////////////////////////////////////////////////////
//
static char* g_DocInfo = 0;
static int g_DocnInfolen = 0;


char* GetDocInfo(int* len){
        *len = g_DocnInfolen;
        return g_DocInfo;
}

void FreeDocInfo(){
        if(g_DocInfo){
                ZENFREE(g_DocInfo);
                g_DocInfo = 0;
                g_DocnInfolen =0;
        }
}

void AddDocInfo(unsigned char* buf,int len){
        if(g_DocInfo){
                ZENFREE(g_DocInfo);
                g_DocInfo = 0;
                g_DocnInfolen =0;
        }
        g_DocInfo = (char*)ZENMALLOC(len+1);
        memset(g_DocInfo,0,len+1);
        memcpy(g_DocInfo,buf,len);
        g_DocnInfolen = len;
}

///////////////////////////////
//
typedef struct _HELPPIC{
        char name[10];
        time_t  time;
}HELPPIC;

static char tmpdir[] = "/customer/temp";
static HELPPIC  g_helppic[16];
static int helppic_comp(const void* pData1,const void* pData2){
        HELPPIC* p1 = (HELPPIC*)pData1;
        HELPPIC* p2 = (HELPPIC*)pData2;
        if(p1->time > p2->time)
                return 1;
        if(p1->time < p2->time)
                return -1;
        return 0;
}

int  helppic_sort(){
        qsort(g_helppic,16,sizeof(HELPPIC),helppic_comp);

        return 1;
}

void init_tmp(){
        if(pSystemInfo->mySysInfo.isusedffs == 1){
        	dir_is_exist(tmpdir);
        }
        memset(g_helppic,0,sizeof(g_helppic));
        for(int i=0; i<16;i++)
                g_helppic[i].time = 20;
}

void del_all_tmppic(){
        DIR  *dir=NULL;
        struct  dirent  *ptr=NULL;
        int file;
        int icount = 0;
        char tmpname[100];
        char* tmpbuf = NULL;

        dir = opendir(tmpdir);
        if(dir != NULL){
                while((ptr = readdir(dir)) != NULL){
                        memset(tmpname,0,100);
                        sprintf(tmpname,"%s/%s",tmpdir,ptr->d_name);
                        unlink(tmpname);
                }
                closedir(dir);
                dir = NULL;
        }
        memset(g_helppic,0,sizeof(g_helppic));
        for(int i=0; i<16;i++)
                g_helppic[i].time = 20;
}
int  read_sort_file(){

        DIR  *dir=NULL;
        struct  dirent  *ptr=NULL;
        int file;
        int icount = 0;
        char tmpname[100];
        struct stat buffer;
        dir = opendir(tmpdir);

        if(dir != NULL){
                while((ptr = readdir(dir)) != NULL){
                        memset(tmpname,0,100);
                        tmpname[0] = ptr->d_name[3];
                        sprintf(g_helppic[icount].name,"%s",ptr->d_name);
                        g_helppic[icount].time = atoi(tmpname)+1;

                        icount++;
                        if(icount > 15)
                                break;
                }
                closedir(dir);
                dir = NULL;
        }
        helppic_sort();
        return icount;
}
void save_tmppic(unsigned char* buf,int len,char* name){
        char tmpname[100];
         if(is_has_flash_size(len) == 0)
        {
        	WRT_DEBUG("磁盘空间不够，保存文件失败，请释放磁盘空间");
        	return ;                	
        }    
        memset(tmpname,0,100);
        sprintf(tmpname,"%s/%s.jpg",tmpdir,name);
        save_file(tmpname,buf,len);
}

char* read_tmppic(int* len ,char* name){
        char tmpname[100];
        char* tmpbuf = NULL;
        memset(tmpname,0,100);
        sprintf(tmpname,"%s/%s.jpg",tmpdir,name);
        FILE *file = NULL;
        *len =0;

        file = fopen(tmpname,"rb");
        if(file != NULL){
                int size1=0;
                fseek(file,0,SEEK_END);
                size1 = ftell(file);
                *len = size1;
                tmpbuf = (char*)ZENMALLOC(size1);
                if(tmpbuf == NULL){
                        fclose(file);
                        file = NULL;
                        *len =0;
                        return NULL;
                }
                fseek(file,0,SEEK_SET);
                int size = fread(tmpbuf,1,size1,file);
                if(size != size1){
                        ZENFREE(tmpbuf);
                        tmpbuf = NULL;
                        *len = 0;
                }
                fclose(file);
                file = NULL;

        }
        return tmpbuf;

}

int read_alltempimage(unsigned char* pointer[16],int size_len[16]){
        int icount =0;
        int i;

        char tmpname[100];
        FILE* file = NULL;
        char* tmpbuf = NULL;
        memset(g_helppic,0,sizeof(g_helppic));
        for(i=0; i<16;i++)
                g_helppic[i].time = 20;
        read_sort_file();
        for(i = 0;i<16;i++){
                if(g_helppic[i].time == 0)
                        break;
                memset(tmpname,0,100);
                sprintf(tmpname,"%s/%s",tmpdir,g_helppic[i].name);
                file = fopen(tmpname,"rb");
                if(file != NULL){
                        int size1 = 0;
                        fseek(file,0,SEEK_END);
                        size1 = ftell(file);
                        tmpbuf = (char*)ZENMALLOC(size1);
                        if(tmpbuf == NULL){
                                fclose(file);
                                file = NULL;
                                tmpbuf = NULL;
                                continue;
                        }
                        fseek(file,0,SEEK_SET);
                        int size = fread(tmpbuf,1,size1,file);
                        if(size != size1){
                                ZENFREE(tmpbuf);
                                tmpbuf = NULL;
                        }else{
                                pointer[icount] = (unsigned char*)tmpbuf;
                                size_len[icount] = size;
                                icount++;
                                if(icount > 15){
                                        fclose(file);
                                        file = NULL;
                                        return icount;
                                }
                        }
                        fclose(file);
                        file = NULL;

                }

        }
        return icount;
}
///////////////////////////////////////////////////////////////
//
//
//
static char screensavedir[]="/customer/scsave";

void init_screen_save()
{
    if(pSystemInfo->mySysInfo.isusedffs == 1){
        dir_is_exist(screensavedir);
    }
}

int save_screen_save_image(unsigned char* buf ,int len, char* name)
{
        char tmpname[100];
         if(is_has_flash_size(len) == 0)
        {
            WRT_DEBUG("磁盘空间不够，保存文件失败，请释放磁盘空间");
            return 0;                	
        }    
        memset(tmpname,0,100);
        sprintf(tmpname,"%s/%s.jpg",screensavedir,name);
        if(save_file(tmpname,buf,len))
            return 1;
        return 0;
}

unsigned char* get_main_ui_replace_jpg(char* name,int* size){
        unsigned char* tmpbuf = NULL;
	char tmpname[100];
	FILE *file = NULL;
	sprintf(tmpname,"%s/%s.jpg",screensavedir,name);
	file = fopen(tmpname,"rb");
        if(file != NULL){
        	int size1 = 0;
        	 fseek(file,0,SEEK_END);
        	 size1 = ftell(file);
        	 tmpbuf = (unsigned char*)ZENMALLOC(size1);
        	if(tmpbuf == NULL){
               	 	fclose(file);
               		file = NULL;
                	tmpbuf = NULL;                	
                	WRT_DEBUG("内存分配失败，无法读取图片");
                        return NULL;
        	}
        	fseek(file,0,SEEK_SET);
        	int size2 = fread(tmpbuf,1,size1,file);
        	if(size2 != size1){
              		  ZENFREE(tmpbuf);
               		  tmpbuf = NULL;
               		  WRT_DEBUG("读取文件%s错误 0x%x",tmpname,errno);
        	}else{
               		*size = size2;
                        fclose(file);
                        file = NULL;
                        return tmpbuf;
                }
                fclose(file);
                file = NULL;
        }
        return NULL;                			
}


int read_all_screensave_image(unsigned char** imagepointer,unsigned long* imagesize)
{
 	DIR  *dir=NULL;
        struct  dirent  *ptr=NULL;
        FILE *file = NULL;
        int icount = 0;
        unsigned char* tmpbuf = NULL;
	char tmpname[100];	
	WRT_DEBUG("read_all_screensave_image ----");
        dir = opendir(screensavedir);
        if(dir != NULL){
                while((ptr = readdir(dir)) != NULL){
                	if(strstr(ptr->d_name,"screen") == NULL)
                		continue;
                	sprintf(tmpname,"%s/%s",screensavedir,ptr->d_name);
                	WRT_DEBUG("tmpname=%s",tmpname);
 			file = fopen(tmpname,"rb");
                	if(file != NULL){
                        	int size1 = 0;
                        	fseek(file,0,SEEK_END);
                        	size1 = ftell(file);
                       		 tmpbuf = (unsigned char*)ZENMALLOC(size1);
                        	if(tmpbuf == NULL){
                               	 	fclose(file);
                               		file = NULL;
                                	tmpbuf = NULL;
                                	closedir(dir);
                                	WRT_DEBUG("内存分配失败，无法读取图片");
                                        return 0;
                        	}
                        	fseek(file,0,SEEK_SET);
                       		 int size = fread(tmpbuf,1,size1,file);
                        	if(size != size1){
                              		  ZENFREE(tmpbuf);
                               		  tmpbuf = NULL;
                               		  WRT_DEBUG("读取文件%s错误 0x%x",tmpname,errno);
                        	}else{
                               		 imagepointer[icount] = (unsigned char*)tmpbuf;
                                	 imagesize[icount] = size;
                               		 icount++;
                               		 if(icount >5){
                                        	fclose(file);
                                        	file = NULL;
                                        	closedir(dir);
                                        	WRT_DEBUG("阅读屏保图片的总数%d",icount);
                                        	return icount;
                                        }
                                }
                                fclose(file);
                                file = NULL;
                        }
                }
                closedir(dir);
                dir = NULL;
        }else{
        	WRT_DEBUG("打开目录%s 错误0x%x",screensavedir,errno);
        }
        WRT_DEBUG("阅读屏保图片的总数%d",icount);
        return icount;
}

 void delete_all_screen_image()
{
	  DIR  *dir=NULL;
        struct  dirent  *ptr=NULL;
        int file;
        int icount = 0;
        char tmpname[100];
        char* tmpbuf = NULL;

        dir = opendir(screensavedir);
        if(dir != NULL){
                while((ptr = readdir(dir)) != NULL){
                	if(strstr(ptr->d_name,"screen") == NULL)
                		continue;
                        memset(tmpname,0,100);
                        sprintf(tmpname,"%s/%s",screensavedir,ptr->d_name);
                        unlink(tmpname);
                }
                closedir(dir);
                dir = NULL;
        }
}

 void delete_all_screen_image_for_factory()
{
	  DIR  *dir=NULL;
        struct  dirent  *ptr=NULL;
        int file;
        int icount = 0;
        char tmpname[100];
        char* tmpbuf = NULL;

        dir = opendir(screensavedir);
        if(dir != NULL){
                while((ptr = readdir(dir)) != NULL){
                	if((ptr->d_type &DT_DIR))
                		continue;
                        memset(tmpname,0,100);
                        sprintf(tmpname,"%s/%s",screensavedir,ptr->d_name);
                        unlink(tmpname);
                }
                closedir(dir);
                dir = NULL;
        }
}
///////////////////////////////////////////////////////////////
static unsigned char* g_vod_frame_header = NULL;

void release_frame_header(){
        if(g_vod_frame_header)
                ZENFREE(g_vod_frame_header);
        g_vod_frame_header = NULL;
}

unsigned long get_vod_frame(unsigned char** pbuf,int selected,int framenum){
        int framecount =0;
        int i =0;
        unsigned char* databuf;
        int datalen = 0;
        int copylen = 0;

        if(g_vod_frame_header == NULL){


                PHOTOINFO* pTemp = get_select_photo(selected);
                if(pTemp == NULL){
                        WRT_DEBUG("获取VOD Frame %d失败",selected);
                        return 0;
                }
                FILE *file = NULL;
                long tmplen=0;
                char path[60];
                unsigned char* addr = NULL;
                sprintf(path,"%s/%s", photodir,pTemp->name);
                EnterMutex();
                file= fopen(path,"rb");
                if(file != NULL){
                        fseek(file,0,SEEK_END);
                        tmplen = ftell(file);
                        addr = (unsigned char*)ZENMALLOC(tmplen);
                        if(addr == NULL){
                                fclose(file);
                                ExitMutex();
                                WRT_WARNING("分配VOD帧内存失败");
                                return 0;
                        }
                        fseek(file,0,SEEK_SET);
                        long read_size = fread(addr,1,tmplen,file);
                        if(read_size != tmplen){
                                fclose(file);
                                // mu_unlock(m_writeflashmutex);
                                ExitMutex();
                                return 0;
                        }
                        fclose(file);
                        ExitMutex();
                }
                g_vod_frame_header = (unsigned char*)addr;
                ExitMutex();
        }
        memcpy(&framecount,g_vod_frame_header+copylen,4);
        copylen +=4;
        for(i =0; i<framecount; i++){
                memcpy(&datalen,g_vod_frame_header+copylen,4);
                copylen += 4;
                databuf = g_vod_frame_header+copylen;
                copylen += datalen;
                if(i == framenum)
                        break;
        }
        *pbuf = databuf;
        return datalen;
}

void add_vod(JPEGFRAME* frame,int size,int count,int isdoor)
{
        JPEGFRAME* tmp = NULL;
        tmp = frame;
        unsigned long  datalen = size+(count+1)*4;
        int i ;
        int copylen = 0;
        unsigned char* databuf = NULL;
        
        if(is_has_flash_size(size) == 0)
        {
        	WRT_DEBUG("磁盘空间不够，保存文件失败，请释放磁盘空间");
        	return ;                	
        }         
        databuf = (unsigned char*)ZENMALLOC(datalen);
        if(databuf == NULL)
                return;
        memcpy(databuf+copylen,&count,4);
        copylen += 4;
        while(tmp){
                memcpy(databuf+copylen,&(tmp->size),4);
                copylen +=4;
                memcpy(databuf+copylen,tmp->data,tmp->size);
                copylen += tmp->size;
                tmp = tmp->next;
        }

        for(i =0;i<m_max_photo;i++){
                if(m_photoinfo[i].idle == 1){
                        if(strstr((char*)m_photoinfo[i].name,"vod") != NULL)
                                break;
                }
        }
        if(i != m_max_photo){
                //删除原来的
                PHOTOINFO* tmp = &m_photoinfo[i];
                tmp->lock  = 0;
                del_photo(tmp);
        }
        PHOTOINFO* pTemp =0;
        pTemp =  get_idle_photo();
        if(pTemp == 0){
                i =1;
                while(1){
                        pTemp = get_select_photo(MAX_PHOTO-i);
                        if(pTemp){
                                if(pTemp->idle == 1 && pTemp->lock == 0)
                                        break;
                        }
                        i++;
                }
                if(pTemp)
                        del_photo(pTemp);
                pTemp =  get_idle_photo();
        }
        uint32_t  times,dates;
        unsigned long day,Month,years ;
        unsigned long min,second,Hour;
        tm_get(&dates,&times,NULL);
        day= dates&0xff;
        Month = (dates >> 8) &0xff;
        years = (dates>>16) & 0xffff;
        Hour = (times >> 16)& 0XFFFF;
        second = (times & 0xff);
        min = (times >> 8) &0xff;
        sprintf((char*)pTemp->name,"vod_%04d-%02d-%02d_%02d-%02d-%02d%s",years,Month,day,Hour,min,second,".vod");
        pTemp->read  = 0;
        pTemp->read |= ((isdoor <<16) & 0XFFFF0000);
        add_photo(pTemp,databuf,datalen);
}

void free_flash()
{
	int ret = 0;
	del_all_msg_for_factory();
	del_all_photo_for_factory();
	del_all_voice_forfactory();
	del_all_log_for_factory();
	ret = rmdir(logdir);
	WRT_DEBUG("rmdir %s %d,0x%x",logdir,ret,errno);
	rmdir(msgdir);
	rmdir(photodir);
	rmdir(voicedir);

	init_photo();

	init_msg();

	init_log();

	init_voice();
	if(g_no_flash_size ==1 )
		g_no_flash_size = 0;          
}

void del_all_dir()
{
        rmdir(downringdir);
        rmdir(logdir);
        rmdir(msgdir);
        rmdir(photodir);
        rmdir(videodir);
        rmdir(telephonedir);
        rmdir(tmpdir);
        rmdir(voicedir);
        rmdir(screensavedir);

}

int has_flash_size()
{
	return !g_no_flash_size;
}


void test_info_for_factory(){
        del_all_tmppic();
        del_all_downring_for_factory();
        del_all_msg_for_factory();
        del_all_photo_for_factory();
        del_all_video_for_factory();
        del_all_telephone();
        del_all_voice_forfactory();
        del_all_log_for_factory();
        delete_all_screen_image_for_factory();

        del_all_dir();
}
