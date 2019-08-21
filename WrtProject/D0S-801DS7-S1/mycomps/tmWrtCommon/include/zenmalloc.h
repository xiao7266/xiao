// zenmalloc.h: interface for the zenmalloc class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZENMALLOC_H__
#define __ZENMALLOC_H__
#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdio.h>
#include <string.h>

#define MEM_DEBUG 1
//#undef MEM_DEBUG

        /*
        #define ZENMALLOC(x)zenMalloc((x))
        #define OCMALLOC(x) ocMalloc((x))
        #define ZENFREE   zenFree
        #define OCFREE zenFree
        */


#define ZENFREE  zenFree

        extern int zenMemInit(int size);

#ifdef MEM_DEBUG
        extern void* zenDebugMalloc(int size,char* file,int line);
#endif

        extern void* zenMalloc(size_t size);
        extern void* zenCalloc(int num,size_t size);
        extern void* zenRealloc(void* mem,size_t size);
        extern void zenFree(void*ptr);
        extern int zenMemUninit();


#ifdef MEM_DEBUG
#define ZENMALLOC(x) zenDebugMalloc(x,__FILE__,__LINE__)
#else
#define ZENMALLOC(x) zenMalloc((x))
#endif

#ifdef __cplusplus
}
#endif
#endif
