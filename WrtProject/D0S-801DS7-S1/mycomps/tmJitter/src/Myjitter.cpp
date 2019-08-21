#include <iostream>
#include <deque>
#include <queue>
#include <vector>

#include "myjitter.h"
#include "zenmalloc.h"



using namespace std;

#define MAX_JITTER_DEPTH 4


class rtpcomp{
public:
    bool operator()(wrt_rtp_header_t* rtp1,wrt_rtp_header_t* rtp2){
        if(rtp1->seq_number > rtp2->seq_number)
            return true;
        return false;

    }
};

typedef priority_queue<wrt_rtp_header_t* ,vector<wrt_rtp_header_t*>,rtpcomp>  RTPQUEUE;

RTPQUEUE  grtpqueue;
static int gjitterdepth = 0;

void InitMyJitter(){
    gjitterdepth = 0;
    if(!grtpqueue.empty())
        ExitMyJitter();
}
void ExitMyJitter(){
    if(grtpqueue.empty())
        return;
     wrt_rtp_header_t* p = NULL;
     while(!grtpqueue.empty()){
        p = grtpqueue.top();
        grtpqueue.pop();
        if(p != NULL)
            ZENFREE(p);
        p = NULL;
     }
     gjitterdepth = 0;
}
void PutMyJitter( wrt_rtp_header_t* p){
    if(grtpqueue.size() > 100){
        ZENFREE(p);
        return;
    }
         
    grtpqueue.push(p);
    if(gjitterdepth < MAX_JITTER_DEPTH){
        gjitterdepth ++;
    }
}
wrt_rtp_header_t* GetMyJitter(){
    if(grtpqueue.empty())
        return NULL;
    if(gjitterdepth < MAX_JITTER_DEPTH)
       return NULL;

    wrt_rtp_header_t* p = NULL;
    p = grtpqueue.top();
    grtpqueue.pop();
    return p;
}
//////////////////////////////////////////////////
//

/*
#include "jitterbuf.h"
static jitterbuf* jb = NULL;

static void FreeAll(){
     jb_frame f;
	while (jb_getall(jb, &f) == JB_OK) {
		ZENFREE(f.data);
	}    
}

void InitMyJitter2()
{
    if(jb != NULL)
        jb_destroy(jb);
    jb = jb_new();
    jb_reset(jb);
    
 
}
void ExitMyJitter2()
{
    
    if(jb != NULL){
        FreeAll();
        jb_destroy(jb);
    }
}
int PutMyJitter2(wrt_rtp_header_t* p)
{
    unsigned long ticks;
    tm_getticks(NULL,&ticks);
    int ret = jb_put(jb,(void*)p,JB_TYPE_VOICE,160,p->timestamp,ticks);
    if(ret == JB_OK){
        return 1;
    }
    if(ret == JB_DROP){
        return 0;
    }
    if(ret == JB_SCHED){
         jb_next(jb);
    }
    return 1;
}

int GetMyJitter2(unsigned char** data)
{
    unsigned long ticks;
    jb_frame frame;

    tm_getticks(NULL,&ticks);
    
	long next = jb_next(jb);
	if (ticks < next) {
	     printf("\tJB_GET {now=%ld}: now < next=%ld\n", ticks,next);
		return 0;
	}
	    
    int ret = jb_get(jb, &frame, ticks, 20);
    *data = (unsigned char*)(char*)frame.data;
    if(ret  == JB_OK)
        return 1;
     return 0;
}
*/