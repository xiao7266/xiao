// adsasd.cpp: implementation of the adsasd class.
//
//////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



#define MEM_ROUND_SIZE(x) (((x) + 7) & ~7)

#define TAG_FREE 0
#define TAG_USED 1


/* this struct should have a size multiple of 8 */
typedef struct memBlock_s {
        size_t size;
        int    tag;
        struct memBlock_s* prev;
        struct memBlock_s* next;
} memBlock_t;


/*
make memPool an array of ints to be sure it aligns on at least
4-byte boundaries
*/

static int* memPool=NULL;

static memBlock_t *memHead = NULL;
int mempool_membytes(void);

void init_mempool(int size)
{
        if(memPool == NULL)
                memPool = (int*)malloc(size);
        //	memset(mempPool,0,size);
        printf("set memHead ...\n");
        memHead = (memBlock_t *)MEM_ROUND_SIZE((int)memPool);
        memHead->size = /*sizeof(memPool)*/size;
        memHead->size -= sizeof(memBlock_t);
        memHead->size -= (char *)memHead - (char *)memPool;
        memHead->tag = TAG_FREE;
        memHead->prev = NULL;
        memHead->next = NULL;
}

void uninit_mempool()
{
        printf("mempool_membytes() = %d \n",mempool_membytes());
        if(memPool)
                free(memPool);
        memPool = NULL;
        memHead = NULL;
}

static void malloc_Block(memBlock_t* block, size_t size)
{
        memBlock_t* newBlock;
        int extraBytes;
        extraBytes = block->size - size;
        if (extraBytes > (int)sizeof(memBlock_t)) {
                /* there is enough room to create a new memBlock */
                newBlock = (memBlock_t*)((char*)block + sizeof(memBlock_t) + size);
                newBlock->size = extraBytes - sizeof(memBlock_t);
                newBlock->tag = TAG_FREE;
                newBlock->next = block->next;
                newBlock->prev = block;
                block->next = newBlock;
                if (newBlock->next)
                        newBlock->next->prev = newBlock;
                block->size = size;
        }
}


void mempool_free(void* p)
{
        memBlock_t *base, *other;
        if(p == NULL)
                return ;
        base = (memBlock_t*)((char*)p - sizeof(memBlock_t));

        base->tag = TAG_FREE;

        /* can we merge with adjacent free blocks? *///将前后是空闲的内存连成大块
        other = base->prev;
        if (other && other->tag == TAG_FREE) {
                other->size += base->size + sizeof(memBlock_t);
                other->next = base->next;
                if (other->next)
                        other->next->prev = other;
                base = other;
        }

        other = base->next;
        if (other && other->tag == TAG_FREE) {
                base->size += other->size + sizeof(memBlock_t);
                base->next = other->next;
                if (base->next)
                        base->next->prev = base;
        }

}


void* mempool_malloc(int  size)
{

        memBlock_t* base;

        if(memHead == NULL)
                return NULL;
        //	if (memHead == NULL)
        //		init_mempool();

        size = MEM_ROUND_SIZE(size);
        /* find a free block of sufficient size */
        for (base = memHead; base != NULL; base = base->next) {
                if (base->tag == TAG_FREE && base->size >= size)
                        break;
        }

        if (base == NULL){
                for (base = memHead; base != NULL; base = base->next) {
                        if(base->tag == TAG_USED)
                                printf("malloc failed size = %d base = %x,base->size = %d ",size,base,base->size);
                }

                return NULL;
        }

        /* split the free block into two smaller blocks (one used, one free) if possible */
        malloc_Block(base, size);
        base->tag = TAG_USED;

        return (void*)((char*)base + sizeof(memBlock_t));
}

void* mempool_calloc(size_t num,size_t size){
        size_t reqsize = num*size;
        void* ptr = NULL;
        ptr =  mempool_malloc(reqsize);
        if(ptr != NULL)
                memset(ptr,0,reqsize);
        return ptr;
}

void* mempool_realloc(void* memblock,size_t size){
        memBlock_t *base;
        void*     p;
        int    oldsize =0;
        if(memblock == NULL){
                return mempool_malloc(size);
        }
        base = (memBlock_t*)((char*)memblock - sizeof(memBlock_t));
        oldsize = base->size;
        p = mempool_malloc(size);
        if(p == NULL)
                return NULL;
        memcpy(p,memblock,oldsize);
        mempool_free(memblock);
        return p;
}

/* returns the number of memory blocks allocated */
int mempool_memblocks(void)
{
        int numBlocks;
        memBlock_t* m;
        if(memHead == NULL)
                return 0;

        numBlocks = 0;
        for (m = memHead; m != NULL; m = m->next) {
                if (m->tag == TAG_USED)
                        numBlocks++;
        }

        return numBlocks;
}


/*
returns the number of bytes allocated (excluding header overhead)
NOTE: due to MEM_ROUND_SIZE(), it will usually not equal the sum
of sizes passed to odmalloc()
*/

int mempool_membytes(void)
{
        int numBytes;
        memBlock_t* m;

        if(memHead == NULL)
                return 0;


        numBytes = 0;
        for (m = memHead; m != NULL; m = m->next) {
                if(m->tag == TAG_USED){
                        numBytes += m->size;
                        //  printf("freemem %x,%d \n",m,m->size);
                }
        }

        return numBytes;
}
