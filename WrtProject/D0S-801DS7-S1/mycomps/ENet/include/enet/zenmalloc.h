// zenmalloc.h: interface for the zenmalloc class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ZENMALLOC_H__
#define __ZENMALLOC_H__
#ifdef __cplusplus
extern "C"{
#endif


#define MEM_DEBUG 1
#undef MEM_DEBUG

        /*
        #define ZENMALLOC(x)zenMalloc((x))
        #define OCMALLOC(x) ocMalloc((x))
        #define ZENFREE   zenFree
        #define OCFREE zenFree
        */
#ifdef MEM_DEBUG
#define ZENMALLOC(x) zenDebugMalloc(x,__FILE__,__LINE__)
#else
#define ZENMALLOC(x) zenMalloc((x))
#endif

#define ZENFREE  zenFree

        extern int zenMemInit(int reserved);

#ifdef MEM_DEBUG
        extern void* zenDebugMalloc(int size,char* file,int line);
#endif

        extern void* zenMalloc(int size);
        extern void* zenCalloc(int num,int size);
        extern void* zenRealloc(void* mem,int size);
        extern void zenFree(void*ptr);
        extern int zenMemUninit();




#ifdef __cplusplus
}
#endif
#endif
