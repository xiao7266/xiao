// zenmalloc.c: implementation of the zenmalloc class.
//
//////////////////////////////////////////////////////////////////////
#include "zenmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#define USE_MEMPOOL 1
//#undef USE_MEMPOOL

static int g_initpool = 0;
#ifdef USE_MEMPOOL
static SDL_mutex *g_memlock = NULL;
#endif

#ifdef MEM_DEBUG
typedef struct mem_dbg_t
{
        char  filename[256];
        int   line;
        int   size;
        void  *ptr;
        struct mem_dbg_t* next;
}MEM_DBG;

static MEM_DBG* mem_list = NULL;

void addmemdbg(int line,char* filename,int size, void* pvoid)
{
        MEM_DBG* lastmemdbg= NULL;
        MEM_DBG* newmem = NULL;
        if(mem_list == NULL)
        {

                mem_list = (MEM_DBG*)malloc(sizeof(MEM_DBG));
                memset(mem_list,'0',sizeof(MEM_DBG));
                strcpy(mem_list->filename,filename);
                mem_list->line = line;
                mem_list->size = size;
                mem_list->ptr = pvoid;
                mem_list->next = NULL;

                return;
        }
        newmem = (MEM_DBG*)malloc(sizeof(MEM_DBG));
        memset(newmem,'0',sizeof(MEM_DBG));
        strcpy(newmem->filename,filename);
        newmem->line = line;
        newmem->size = size;
        newmem->ptr = pvoid;
        newmem->next = NULL;

        if(mem_list->next == NULL)
        {

                mem_list->next = newmem;
        }
        else{
                lastmemdbg = mem_list;
                while(lastmemdbg->next)
                        lastmemdbg = lastmemdbg->next;

                lastmemdbg->next = newmem;
        }

}
void deletememdbg(void* pvoid)
{
        MEM_DBG* tmpmemdbg= NULL;
        MEM_DBG* prev = NULL;
        if(mem_list == NULL)
                return;
        tmpmemdbg=mem_list;
        while(tmpmemdbg)
        {


                if(tmpmemdbg->ptr == pvoid)
                {
                        if(prev != NULL)
                        {
                                prev->next = tmpmemdbg->next;
                                free(tmpmemdbg);
                                break;
                        }
                        else
                        {
                                mem_list = tmpmemdbg->next;
                                free(tmpmemdbg);
                                break;

                        }
                }
                prev = tmpmemdbg;
                tmpmemdbg  = tmpmemdbg->next;
        }
}

void printfmemdbg()
{
        int size = 0;
        MEM_DBG* tmp = NULL;
        if(mem_list == NULL)
                return;
        tmp = mem_list;

        while(tmp)
        {
                printf("file(%s),line(%d),size(%d),ptr(%x) \n",
                        tmp->filename,tmp->line,tmp->size,tmp->ptr);
                size +=tmp->size;
                tmp = tmp->next;

        }
        printf("size = %d \n",size);

}


#endif


#ifdef USE_MEMPOOL

#ifndef MEM_POOL_SIZE
/* memory pool size is in ints, not bytes! */
#define MEM_POOL_SIZE (1024 * 1024 * 12 )
#endif

extern void* mempool_malloc(int  size);
extern void mempool_free(void* p);
extern void init_mempool(int size);
extern void uninit_mempool();
extern int mempool_membytes();
extern void* mempool_realloc(void* memblock,size_t size);
extern void* mempool_calloc(size_t num,size_t size);


#endif

int zenMemInit(int size)
{

        //  WRT_HAVE_DBG();
#ifdef USE_MEMPOOL
	int isize = 0;
       
    
    if(g_memlock == NULL)
    	g_memlock = SDL_CreateMutex();
    printf("begin malloc mem pool g_memlock = %x\n",g_memlock);
    
    if(size > 0)
    	isize = size;
    else
    	isize = MEM_POOL_SIZE;
    	
    init_mempool(isize);
    g_initpool = 1;
#endif

        return 0;
}

void EnterMemLock(){

#ifdef USE_MEMPOOL
        if(g_memlock != NULL){
                SDL_LockMutex(g_memlock);
        }
#endif

}

void ExitMemLock(){
	
#ifdef USE_MEMPOOL
        if(g_memlock != NULL){
                SDL_UnlockMutex(g_memlock);
        }
#endif

}

int zenMemUninit()
{

#if 1
	int err =0;
	int used =0;
#ifdef MEM_DEBUG
	printfmemdbg();
#endif
#ifdef USE_MEMPOOL
	//  EnterMemLock();
	used = mempool_membytes();
	//  ExitMemLock();
	printf("used  memory = %d \n",used);

	uninit_mempool();
	g_initpool = 0;
	SDL_DestroyMutex(g_memlock);
	g_memlock = NULL;
#endif

	return err;
#endif
}

void*zenMalloc(size_t size)
{

#ifdef USE_MEMPOOL
	
    void*ptr=0;
	if(g_initpool == 0)
 	{
 		zenMemInit(0);
 	}        
	EnterMemLock();
	ptr = mempool_malloc(size);
	ExitMemLock();
	if(ptr == NULL)
		printf("zenmalloc failed %d ,used mem = %d,all mem = %d\n",size,mempool_membytes(),MEM_POOL_SIZE);
	return ptr;
#endif
	return malloc(size);
}

#ifdef MEM_DEBUG
extern void* zenDebugMalloc(int size,char* file,int line)
{
	void* ptr = NULL;

	ptr = zenMalloc(size);
	if(ptr)
		addmemdbg(line,file,size,ptr);
	else{
		EnterMemLock();
		mempool_membytes();
		ExitMemLock();
	}

	return  ptr;
}
#endif

void*zenCalloc(int num,size_t size)
{

#ifdef USE_MEMPOOL
	void * ptr = 0;
	EnterMemLock();
	ptr = mempool_calloc(num,size);
	ExitMemLock();
	return ptr; 
#endif
	return calloc(num,size);
}

void* zenRealloc(void* mem,size_t size){

#ifdef USE_MEMPOOL
	void* ptr = 0;
	EnterMemLock();
	ptr  = mempool_realloc(mem,size);
	ExitMemLock();
	return ptr;
#endif
	return realloc(mem,size);
}

void zenFree(void*ptr)
{

#ifdef USE_MEMPOOL
	EnterMemLock();
#ifdef MEM_DEBUG
	deletememdbg(ptr);
#endif        

	mempool_free(ptr);
	ExitMemLock();
	return;
#else
	free(ptr);
#endif
}
