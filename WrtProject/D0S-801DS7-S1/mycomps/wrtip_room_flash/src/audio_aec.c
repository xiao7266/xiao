/*******************************************
ÒôÆµ»ØÉùµÄÏû³ý
********************************************/
#include <pna.h> //for struct timeval
#include <psos.h> //for mu_create,mu_lock,mu_unlock



#ifdef __cplusplus
extern "C"{
#endif
        void create_AEC();
        short do_AEC(short ref, short mic);
        void kill_AEC();
#ifdef __cplusplus
}
#endif

#define DO_ECHO_CAN 1
#define AEC_BIT     1

#define MAX_FRAME_SIZE  768
#define PCM_TRACE_LEN   50*MAX_FRAME_SIZE   /* data are stored for 50 frames of 12ms */


#pragma pack(1)

typedef struct __audioaec{
        int					           min_size;
        unsigned long				   timestamp;
        struct	timeval                last_audio_read;
        unsigned long                   aec_mutex;  
#define AEC_MUTEX_LOCK(s) mu_lock(s->aec_mutex,MU_WAIT,0)
#define AEC_MUTEX_UNLOCK(s) mu_unlock(s->aec_mutex)  
        int								echocancel;
        int 							echostate;	/* State of echo canceller */
        unsigned long			        synclock;
        char							*pcm_sent;
        int								pcm_need_resync; 
        int								pcm_rd;
        int								pcm_wr;
        int								bytes_to_throw;
        int								sent_cnt;
        int								recv_cnt;
}audioaec;
#pragma pack()

static audioaec * pAudioAEC = NULL;

#ifdef __cplusplus
extern "C"{
#endif
        void audioAEC_start();
        void audioAEC_store(char* buf,  int len);
        void audioAEC_time();
        void audioAEC_update(char* data,int len);

#ifdef __cplusplus
}
#endif


static void tvsub(register struct timeval *out, register struct timeval *in)
{
        out->tv_usec -= in->tv_usec;

        while(out->tv_usec < 0) {
                --out->tv_sec;
                out->tv_usec += 1000000;
        }

        out->tv_sec -= in->tv_sec;
}



static void do_echo_update(audioaec *s, char *data, int len);
static void store_pcm(audioaec *s, char *buf, int len);
static void echo_resync(audioaec *s);

/* should be called in case of transmit underrun */
static void echo_resync(audioaec *s){
        mu_lock(s->synclock,MU_WAIT,0);
        s->pcm_rd = s->pcm_wr = 0;
        s->sent_cnt = s->recv_cnt = 0;
        s->pcm_need_resync = 1;
        s->bytes_to_throw = 0;
        mu_unlock(s->synclock);
}

static void store_pcm(audioaec *s, char *buf, int len)
{
        int lg;

        mu_lock(s->synclock,MU_WAIT,0);
        if (len <= (PCM_TRACE_LEN - s->pcm_wr))
        {
                memcpy(s->pcm_sent + s->pcm_wr, buf, len);
                s->pcm_wr += len;
                if (s->pcm_wr == PCM_TRACE_LEN)
                        s->pcm_wr = 0;
        }
        else
        {
                lg = PCM_TRACE_LEN - s->pcm_wr;
                memcpy(s->pcm_sent + s->pcm_wr, buf, lg);
                memcpy(s->pcm_sent, buf+lg, len-lg);
                s->pcm_wr = len-lg;
        }
        s->sent_cnt += len;
        if(s->pcm_need_resync && s->sent_cnt > 0)
        {
                int skip=0;
                struct timeval now;

                // compute the time elapsed since the moment application got something
                // from audio diver
                gettimeofday(&now, 0);
                AEC_MUTEX_LOCK(s); 

                tvsub(&now, &s->last_audio_read);


                skip = 8*2 * now.tv_usec/1000;   // number of bytes waiting in the system buffers

                s->bytes_to_throw = skip;

                s->pcm_need_resync = 0;
                AEC_MUTEX_UNLOCK(s);
                printf("EC:Resync OK %d bytes to throw\n", skip);

        }
        mu_unlock(s->synclock);
}

static void do_echo_update2(audioaec *s, char *data, int length)
{
        int i, lg, len;
        short *p1, *p2;

        len = length;

        if (s->pcm_need_resync)
                return ;

        if(s->bytes_to_throw){
                if(s->bytes_to_throw >= len){
                        s->bytes_to_throw -= len;
                        printf("EC1:%d bytes thrown\n", len);
                        return;
                }else{
                        printf("EC2:%d bytes thrown\n", s->bytes_to_throw);
                        len = length - s->bytes_to_throw;
                        s->bytes_to_throw = 0;
                }
        }
        s->recv_cnt += len;
        if(s->recv_cnt > s->sent_cnt)
        {
                printf( "\nUNDERRUN: %d %d\n", s->sent_cnt, s->recv_cnt);
                echo_resync(s);
                return;
        }

        p1 = (short *)(s->pcm_sent + s->pcm_rd);
        p2 = (short *)data;

        if((PCM_TRACE_LEN - s->pcm_rd) >= len)
        {
                for (i=0; i<len/2; i++) 
                {
                        //#if !AEC_BIS
                        //            *p2 = echo_can_update(s->ec, *p1++, *p2);
                        //#else /* AEC_BIS */
                        *p2 = do_AEC(*p1++, *p2);
                        //#endif /* AEC_BIS */
                        p2++;
                }

                s->pcm_rd += len;
                if(s->pcm_rd == PCM_TRACE_LEN)
                        s->pcm_rd = 0; 
        }
        else
        {
                lg = PCM_TRACE_LEN - s->pcm_rd;
                for (i=0; i<lg/2; i++)
                {
                        //#if !AEC_BIS
                        //            *p2 = echo_can_update(s->ec, *p1++, *p2);
                        //#else /* AEC_BIS */
                        *p2 = do_AEC(*p1++, *p2);
                        //#endif /* AEC_BIS */
                        p2++;
                }
                p1 = (short *)(s->pcm_sent);
                for (i=0; i<(len - lg)/2; i++)
                {
                        //#if !AEC_BIS
                        //            *p2 = echo_can_update(s->ec, *p1++, *p2);
                        //#else /* AEC_BIS */
                        *p2 = do_AEC(*p1++, *p2);
                        //#endif /* AEC_BIS */
                        p2++;
                }
                s->pcm_rd = len-lg;
        }

}

static void do_echo_update(audioaec *s, char *data, int length)
{
        while(length)
        {
                int len = (length > 8) ? 8 : length;

                do_echo_update2(s, data, len);
                data += len;
                length -= len;
        }
}




void audioAEC_start(){

        pAudioAEC = (audioaec*)malloc(sizeof(audioaec));
        if(pAudioAEC == NULL){
                printf("audio aec failed \n");
                return ;
        }

        memset(pAudioAEC,0,sizeof(audioaec));

        create_AEC();

        pAudioAEC->echocancel = 256;
        pAudioAEC->pcm_rd = pAudioAEC->pcm_wr = 0;
        pAudioAEC->pcm_sent = (char*)malloc(PCM_TRACE_LEN);
        pAudioAEC->sent_cnt = pAudioAEC->recv_cnt = 0;
        if(pAudioAEC->pcm_sent == 0)
                printf( "No memory for EC  %d\n", pAudioAEC->pcm_sent);
        mu_create("aec1",MU_FIFO,0,&(pAudioAEC->aec_mutex));
        mu_create("aec2",MU_FIFO,0,&(pAudioAEC->synclock));
        pAudioAEC->pcm_need_resync = 1;
        pAudioAEC->bytes_to_throw = 0;
}

void audioAEC_reset(){
        if(pAudioAEC){
                kill_AEC();
                pAudioAEC->pcm_rd = pAudioAEC->pcm_wr = 0;
                pAudioAEC->sent_cnt = pAudioAEC->recv_cnt = 0;
                pAudioAEC->pcm_need_resync = 1;
                pAudioAEC->bytes_to_throw = 0;	 
                create_AEC();
        }
}



void audioAEC_time(){
        if(pAudioAEC){
                gettimeofday(&pAudioAEC->last_audio_read, 0);
        }
}

void audioAEC_store(char* buf,int len){
        if(pAudioAEC){
                store_pcm(pAudioAEC,buf,len);
        }
}

void audioAEC_update(char* data,int len){
        if(pAudioAEC){
                do_echo_update(pAudioAEC,data,len);
        }
}

void audioAEC_lock(){
        if(pAudioAEC)
                AEC_MUTEX_LOCK(pAudioAEC);

}

void audioAEC_unlock(){
        if(pAudioAEC)
                AEC_MUTEX_UNLOCK(pAudioAEC);
}