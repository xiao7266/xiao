#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <SDL.h>
#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "idtoiptable.h"

using namespace std; 

//////////////////////////////////////////////////////////////////////////
//房号表的处理

typedef struct _tagMyKey{
        char key[16];
        bool operator < ( const struct _tagMyKey rhs) const
        {
                int ret = strncmp(key,rhs.key,13);
                if(ret < 0)
                        return 1;
                return 0;
        }
}MYKEY;

typedef multimap<MYKEY,unsigned long> IDTOIPMAP;
typedef pair<MYKEY, unsigned long>   IDTOIP_Pair;

static IDTOIPMAP my_IDToIPMap;
static char m_id[8][16];//default 4
static SDL_mutex* m_idtoiplock = NULL;

char idtoip[] = "/customer/wrt/idtoip.txt";

static bool save_file1(char* name,unsigned char* addr,int len){

        FILE* file = NULL;
        
        file = fopen(name,"wb");
        if(file != NULL){
                long write_size = fwrite(addr,1,len,file);
                if(write_size == len){
                        fclose(file);
                        system("sync");
                        return true;
                }
                printf("errno = %d  write_size=%d \n",errno,write_size);
                fclose(file);
                unlink(name);
        }
        printf("errno = %d \n",errno);
        return false;
}

static void reset_idtoip(unsigned char* buf,long buflen){
        int count = 0;
        int  i =0;
        int j=0;
        unsigned char  tmpidtoip[20];

        unsigned char* tmpbuf;
        MYKEY  mykey;
        unsigned long ip = 0;
        if(buflen == 0)
                return;
        count = *(int*)buf;
        if(count <= 0)
                return;
        if((count * 20) > buflen)
                return;        
        tmpbuf = buf+4;
        my_IDToIPMap.clear();
        printf("reset_idtoip,count = %d,%d\n",count,buflen);
        for(i=1; i <=count; i++){
                ip = *(unsigned long*)(buf+4+j);
                memset(&mykey,0,sizeof(mykey));
                memcpy(mykey.key,(buf+8+j),16);
                j = i*20;
                my_IDToIPMap.insert(IDTOIP_Pair(mykey,ip));
        }
}

static void setup_idtoip(){
        FILE* file = NULL;
        int size1 = 0;
        struct stat sbuf;
        if(stat(idtoip,&sbuf) < 0)
                return;
        size1 = sbuf.st_size;
        if(size1 > ((2*1024*1024)+4))
                return;
        unsigned char* tmpbuf = (unsigned char*)ZENMALLOC(size1);
        if(tmpbuf == NULL)
                return ;
        file = fopen(idtoip,"rb");
        if(file != NULL){
                int size = fread(tmpbuf,1,size1,file);
                if(size != size1){
                        ZENFREE(tmpbuf);
                        fclose(file);
                        return ;
                }
                fclose(file);
                file = NULL;
                reset_idtoip(tmpbuf,size1);
        }
        ZENFREE(tmpbuf);
}

static void EnterIDtoIPLock(){
        if(m_idtoiplock != NULL)
                SDL_LockMutex(m_idtoiplock);
}
static void ExitIDtoIPLock(){
        if(m_idtoiplock != NULL)
                SDL_UnlockMutex(m_idtoiplock);
}

void init_idtoip(){
	/*
        if(mu_create("idip",MU_FIFO|MU_LOCAL,0,&m_idtoiplock) != 0){
                m_idtoiplock = -1;
                printf("mu_create idip lock error %x\n",errno);
        }
        */
        m_idtoiplock = SDL_CreateMutex();
        if(m_idtoiplock == NULL)
        	 printf("SDL_CreateMutex idip lock error %x\n",errno);
        setup_idtoip();
}

void save_idtoip(unsigned char* buf,long buflen){
        if(save_file1(idtoip,buf,buflen)){
                reset_idtoip((unsigned char*)buf,buflen);
        }
}

unsigned char* get_idtoipdata(int* len){
        FILE* file = NULL;
        int size1 = 0;
        struct stat sbuf;
        if(stat(idtoip,&sbuf) < 0)
                return NULL;
        size1 = sbuf.st_size;
        unsigned char* tmpbuf = (unsigned char*)ZENMALLOC(size1);
        if(tmpbuf == NULL)
                return NULL ;
        EnterMutex();
        file = fopen(idtoip,"rb");
        if(file != NULL){
                int size = fread(tmpbuf,1,size1,file);
                if(size != size1){
                        ZENFREE(tmpbuf);
                        fclose(file);
                        file = NULL;
                        *len = 0;
                        ExitMutex();
                        return  NULL;
                }
                fclose(file);
                file = NULL;
        }
        *len = size1;
        ExitMutex();
        return  tmpbuf ;
}

void get_ipbyid(char* id,unsigned long* ip,int* num){
        MYKEY mykey;
        int  i =0;
        IDTOIPMAP::iterator myiterator;

        EnterIDtoIPLock();
        memset(&mykey,0,sizeof(mykey));
        strcpy(mykey.key,id);
        memset(m_id,0,sizeof(m_id));


        std::pair<IDTOIPMAP::iterator, IDTOIPMAP::iterator> itp = my_IDToIPMap.equal_range(mykey);
        for(myiterator = itp.first; myiterator != itp.second; myiterator++){
                if(i < *num){
                        ip[i] = myiterator->second;
                        memcpy(m_id[i],(void*)myiterator->first.key,16);
                }else
                        break;
                i++;
        }

        *num = i;
        ExitIDtoIPLock();
}

void get_ipbyid2(char* id,unsigned long *ip){
        MYKEY mykey;
        int  i =0;
        IDTOIPMAP::iterator myiterator;

        EnterIDtoIPLock();
        memset(&mykey,0,sizeof(mykey));
        strcpy(mykey.key,id);
        memset(m_id,0,sizeof(m_id));

        std::pair<IDTOIPMAP::iterator, IDTOIPMAP::iterator> itp = my_IDToIPMap.equal_range(mykey);
        for(myiterator = itp.first; myiterator != itp.second; myiterator++){
                if(strncmp(id,(char*)myiterator->first.key,15) == 0){
                        *ip = myiterator->second;
                        break;
                }
        }
        ExitIDtoIPLock();
}

void get_id2(char* p,int len){
        if(p == 0)
                return;
        if(len < sizeof(m_id))
                return;
        memset(p,0,len);
        memcpy(p,(char*)m_id,sizeof(m_id));
}

