#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>
#include "SDL.h"
#include "wrt_log.h"
#include "zenmalloc.h"
#include "wrt_audio.h"
#include "sndwav_common.h"
#include "virtualDevice.h"

#define NUM_BUFS 8 

#define ALSA_PERIODS 8
#define ALSA_PERIOD_SIZE 1024 //512.1024.4096

static SNDPCMContainer_t  g_playContainer_t;
static SNDPCMContainer_t  g_captureContainer_t;
static SDL_sem*    g_play_sem = NULL;

static int    g_isplay = 0;
static int    g_isrecord = 0;

static int g_isexit = 0;
static int g_record_isexit  = 0;

static unsigned char *g_buffer[NUM_BUFS];
static unsigned int g_buf_read=0;
static unsigned int g_buf_write=0;
static unsigned int g_buf_read_pos=0;
static unsigned int g_buf_write_pos=0;

static int g_full_buffers=0;
//static int g_bufferead_bytes=0;
static int g_buffer_period_size = 0;

static int totalcount = 0;
static int g_ticks1 = 0;
static int g_ticks2 = 0;


static unsigned char *g_record_buffer[NUM_BUFS];
static unsigned int g_record_buf_read=0;
static unsigned int g_record_buf_write=0;
static unsigned int g_record_buf_read_pos=0;
static unsigned int g_record_buf_write_pos=0;

static int g_record_full_buffers=0;
//static int g_record_bufferead_bytes=0;
static int g_record_buffer_period_size = 0;
static int g_devfd = -1;

static char* g_devname="/dev/hx280_aud";



static int write_buffer(unsigned char* data,int len){
	int len2=0;
	int x;
	while(len>0){
		if(g_full_buffers == NUM_BUFS){ 
			//printf("************************\n");
			break;
		}
		x=g_buffer_period_size-g_buf_write_pos;
		if(x>len) 
			x=len;
		memcpy(g_buffer[g_buf_write]+g_buf_write_pos,data+len2,x);
		len2 += x; 
		len -= x;
		//g_bufferead_bytes += x; 
		g_buf_write_pos += x;
		if(g_buf_write_pos >= g_buffer_period_size){
			// block is full, find next!
			g_buf_write=(g_buf_write+1)%NUM_BUFS;
			++g_full_buffers;
			g_buf_write_pos=0;
		}
	}
	return len2;
}

static int read_buffer(unsigned char* data,int len){
	int len2=0;
	int x;
	while(len>0){
		if(g_full_buffers==0){ 
			break; // no more data buffered!
		}
		x=g_buffer_period_size-g_buf_read_pos;
		if(x>len) 
			x=len;
		memcpy(data+len2,g_buffer[g_buf_read]+g_buf_read_pos,x);
		len2 += x; 
		len -= x;
		//g_bufferead_bytes-=x; 
		g_buf_read_pos+=x;
		if(g_buf_read_pos>=g_buffer_period_size){
			// block is empty, find next!
			g_buf_read=(g_buf_read+1)%NUM_BUFS;
			--g_full_buffers;
			g_buf_read_pos=0;
		}
	}
	return len2;
}

static int record_write_buffer(unsigned char* data,int len){
	int len2=0;
	int x;
	while(len>0){
		if(g_record_full_buffers == NUM_BUFS){ 
			break;
		}
		x = g_record_buffer_period_size - g_record_buf_write_pos;
		if( x > len) 
			x = len;
		memcpy(g_record_buffer[g_record_buf_write]+g_record_buf_write_pos,data+len2,x);
		len2 += x; 
		len -= x;
		//g_record_bufferead_bytes += x; 
		g_record_buf_write_pos += x;
		if(g_record_buf_write_pos >= g_record_buffer_period_size){
			// block is full, find next!
			g_record_buf_write=(g_record_buf_write+1)%NUM_BUFS;
			++g_record_full_buffers;
			g_record_buf_write_pos=0;
		}
	}
	return len2;
}

static int record_read_buffer(unsigned char* data,int len){
	int len2=0;
	int x;
	while(len>0){
		if(g_record_full_buffers==0){ 
		       // printf("g_record_full_buffers is empty\n");
			break; // no more data buffered!
		}
		x=g_record_buffer_period_size-g_record_buf_read_pos;
		if(x>len) 
			x=len;
		memcpy(data+len2,g_record_buffer[g_record_buf_read]+g_record_buf_read_pos,x);
		//SDL_MixAudio(data+len2, data+len2, x, volume);
		len2 += x; 
		len -= x;
		//g_record_bufferead_bytes-=x; 
		g_record_buf_read_pos+=x;
		if(g_record_buf_read_pos>=g_record_buffer_period_size){
			// block is empty, find next!
			g_record_buf_read=(g_record_buf_read+1)%NUM_BUFS;
			--g_record_full_buffers;
			g_record_buf_read_pos=0;
		}
	}
	return len2;
}

static void reset_ringbuffer(){
	int i;
	/* Reset ring-buffer state */
	g_buf_read=0;
	g_buf_write=0;
	g_buf_read_pos=0;
	g_buf_write_pos=0;

	g_full_buffers=0;
	//g_bufferead_bytes=0;
	
	g_buffer_period_size = 0;
	for(i=0;i<NUM_BUFS;i++) {
		g_buffer[i] = NULL;	
	}	

}

static void reset_record_ringbuffer(){
	int i;
	/* Reset ring-buffer state */
	g_record_buf_read=0;
	g_record_buf_write=0;
	g_record_buf_read_pos=0;
	g_record_buf_write_pos=0;

	g_record_full_buffers=0;
	//g_record_bufferead_bytes=0;
	
	g_record_buffer_period_size = 0;
	for(i=0;i<NUM_BUFS;i++) {
		g_record_buffer[i] = NULL;	
	}	

}

#if 0

static int alsa_write(snd_pcm_t *handle,unsigned char *buf,int nsamples)
{
	int err;
	if ((err=snd_pcm_writei(handle,buf,nsamples))<0){
		if (err==-EPIPE){
			snd_pcm_prepare(handle);
#ifdef EPIPE_BUGFIX
			alsa_fill_w (handle);
#endif
			err=snd_pcm_writei(handle,buf,nsamples);
			if (err<0) printf("alsa_card_write: Error writing sound buffer (nsamples=%i):%s",nsamples,snd_strerror(err));
		}else if (err!=-EWOULDBLOCK){
			printf("alsa_card_write: snd_pcm_writei() failed:%s.",snd_strerror(err));
		}
	}else if (err!=nsamples) {
		printf("Only %i samples written instead of %i",err,nsamples);
	}
	return err;
}

#ifdef EPIPE_BUGFIX
static void alsa_fill_w (snd_pcm_t *pcm_handle)
{
	snd_pcm_hw_params_t *hwparams=NULL;
	int channels;
        snd_pcm_uframes_t buffer_size;
	int buffer_size_bytes;
	void *buffer;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_hw_params_current(pcm_handle, hwparams);

	/* get channels */
	snd_pcm_hw_params_get_channels (hwparams, &channels);

	/* get buffer size */
	snd_pcm_hw_params_get_buffer_size (hwparams, &buffer_size);

	/* fill half */
	buffer_size /= 2;

	/* allocate buffer assuming 2 bytes per sample */
	buffer_size_bytes = buffer_size * channels * 2;
	buffer = alloca (buffer_size_bytes);
	memset (buffer, 0, buffer_size_bytes);

	/* write data */
	snd_pcm_writei(pcm_handle, buffer, buffer_size);
}
#endif


static int alsa_set_params(SNDPCMContainer_t* sndpcm, int rw, int bits, int stereo, int rate)
{
	snd_pcm_hw_params_t *hwparams=NULL;
	snd_pcm_sw_params_t *swparams=NULL;
	int dir;
	uint exact_uvalue;
	unsigned long exact_ulvalue;
	int channels;
	int periods=ALSA_PERIODS;
	int periodsize=ALSA_PERIOD_SIZE;
	int err;
	int format;
	snd_pcm_t *pcm_handle =sndpcm->handle;
	
	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);
	printf("periodsize === %d \n",periodsize);
	
	/* Init hwparams with full configuration space */
	if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
		printf("alsa_set_params: Cannot configure this PCM device.");
		return -1;
	}
	
	if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		printf("alsa_set_params: Error setting access.");
		return -1;
	}
	/* Set sample format */
	format=SND_PCM_FORMAT_S16;
	if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, format) < 0) {
		printf("alsa_set_params: Error setting format.");
		return -1;
	}
	/* Set number of channels */
	if (stereo) channels=2;
	else channels=1;
	if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, channels) < 0) {
		printf("alsa_set_params: Error setting channels.");
		return -1;
	}
	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */ 
	exact_uvalue=rate;
	dir=0;
	if ((err=snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_uvalue, &dir))<0){
		printf("alsa_set_params: Error setting rate to %i:%s",rate,snd_strerror(err));
		return -1;
	}
	if (dir != 0) {
		printf("alsa_set_params: The rate %d Hz is not supported by your hardware.\n "
		"==> Using %d Hz instead.", rate, exact_uvalue);
	}
	/* choose greater period size when rate is high */
	periodsize=periodsize*(rate/8000);	
	
	/* Set buffer size (in frames). The resulting latency is given by */
	/* latency = periodsize * periods / (rate * bytes_per_frame)     */
	/* set period size */
	exact_ulvalue=periodsize;
	dir=0;
	if (snd_pcm_hw_params_set_period_size_near(pcm_handle, hwparams, &exact_ulvalue, &dir) < 0) {
		printf("alsa_set_params: Error setting period size.");
		return -1;
	}
	if (dir != 0) {
		printf("alsa_set_params: The period size %d is not supported by your hardware.\n "
		"==> Using %d instead.", periodsize, (int)exact_ulvalue);
	}
	printf("alsa_set_params: periodsize:%d Using %d", periodsize, (int)exact_ulvalue);
	periodsize=exact_ulvalue;
	/* Set number of periods. Periods used to be called fragments. */ 
	exact_uvalue=periods;
	dir=0;
	if (snd_pcm_hw_params_set_periods_near(pcm_handle, hwparams, &exact_uvalue, &dir) < 0) {
		printf("alsa_set_params: Error setting periods.");
		return -1;
	}
	printf("alsa_set_params: period:%d Using %d", periods, exact_uvalue);
	if (dir != 0) {
		printf("alsa_set_params: The number of periods %d is not supported by your hardware.\n "
		"==> Using %d instead.", periods, exact_uvalue);
	}
	/* Apply HW parameter settings to */
	/* PCM device and prepare device  */
	if ((err=snd_pcm_hw_params(pcm_handle, hwparams)) < 0) {
		printf("alsa_set_params: Error setting HW params:%s",snd_strerror(err));
		return -1;
	}
	/*prepare sw params */
	if (rw){
		snd_pcm_sw_params_alloca(&swparams);
		snd_pcm_sw_params_current(pcm_handle, swparams);
		if ((err=snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams,periodsize*2 ))<0){
			printf("alsa_set_params: Error setting start threshold:%s",snd_strerror(err));
		}
		if ((err=snd_pcm_sw_params_set_stop_threshold(pcm_handle, swparams,periodsize*periods ))<0){
			printf("alsa_set_params: Error setting stop threshold:%s",snd_strerror(err));
		}
		if ((err=snd_pcm_sw_params(pcm_handle, swparams))<0){
			printf("alsa_set_params: Error setting SW params:%s",snd_strerror(err));
			return -1;
		}
	}
	sndpcm->chunk_size = periods;
	
	return 0;	
}

static snd_pcm_t * alsa_open_w(const char *pcmdev,int bits,int stereo,int rate)
{
	snd_pcm_t *pcm_handle;
	
	if (snd_pcm_open(&pcm_handle, pcmdev,SND_PCM_STREAM_PLAYBACK,SND_PCM_NONBLOCK) < 0) {
		printf("alsa_open_w: Error opening PCM device %s",pcmdev );
		return NULL;
	}
	alsa_set_params(pcm_handle,1,bits,stereo,rate);
#if 0
	{
	struct timeval tv1;
	struct timeval tv2;
	struct timezone tz;
	int diff = 0;
	int err;
	err = gettimeofday(&tv1, &tz);
	while (1) { 
		if (!(alsa_set_params(pcm_handle,1,bits,stereo,rate)<0)){
			ms_message("alsa_open_w: Audio params set");
			break;
		}
		if (!gettimeofday(&tv2, &tz) && !err) {
			diff = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);
		} else {
			diff = -1;
		}
		if ((diff < 0) || (diff > 3000000)) { /* 3 secondes */
			ms_error("alsa_open_w: Error setting params for more than 3 seconds");
			snd_pcm_close(pcm_handle);
			return NULL;
		}
		printf("alsa_open_w: Error setting params (for %d micros)", diff);
		usleep(200000);
	}
	}
#endif
	return pcm_handle;
}
#endif

static int playaudiotask(void* pvoid)
{
	int readlen = 0;


        int ret1 = 0;
        int ret2 = 0;
	int isplay = 0;
	int delay =0;
	int err =0;


	unsigned long ticks1,ticks2;
	int buffers = g_playContainer_t.chunk_bytes/g_buffer_period_size;
	if(g_playContainer_t.chunk_bytes%g_buffer_period_size > 0)
		buffers += 1;
        printf("playaudiotask begin buffers = %d\n",buffers);
        
	while(!g_isexit){
	        if(isplay == 0){
	               SDL_SemWait(g_play_sem);
	               isplay++;
                       printf("OK being play ....\n");
	        } 
                ret2 = g_full_buffers * g_buffer_period_size ;
                if(ret2 > g_playContainer_t.chunk_bytes)
                	ret2 = g_playContainer_t.chunk_bytes;
                	
                ret1 = 0;
		while(1){
			if(g_isexit > 0)
		        	break;
		        readlen = read_buffer(g_playContainer_t.data_buf+ret1,ret2-ret1 );
		        ret1 += readlen;
		        if(readlen == 0){
		        	//printf("^");
		        	break;
		        }
		        if (ret1 == ret2)
		        	break;
		}
		if(ret1 == 0){
			delay= g_playContainer_t.chunk_bytes * 1000/(g_playContainer_t.rate * g_playContainer_t.bits_per_frame / 8);
			if(delay == 0)
				delay = 10;
			SDL_Delay(delay); 	                		
			continue;
		}
		 ret1 = ret1 * 8 / g_playContainer_t.bits_per_frame; 
		// ticks1 = SDL_GetTicks();
		 err = SNDWAV_WritePcm(&g_playContainer_t,ret1);                        
		 //ticks2 = SDL_GetTicks();
		// printf("play11 time = %d ret1 = %d,err = %d\n",ticks2-ticks1,ret1,err);

        }
        printf("playaudiotask exit \n");
        g_isexit = 2;
}

static int recordaudiotask(void* pvoid)
{
	int readlen = 0;
	int icount =0;
	int readlen1 = 0;
	int ret = 0;
	unsigned long ticks1,ticks2;
	ret = snd_pcm_start(g_captureContainer_t.handle);
	printf("recordaudiotask begin ret = %d --%x\n",ret,g_captureContainer_t.handle);
	
	while(!g_record_isexit){
		readlen = g_captureContainer_t.chunk_bytes;
		readlen =  readlen * 8 /g_captureContainer_t.bits_per_frame;
		//ticks1  = SDL_GetTicks();
                readlen1 = SNDWAV_ReadPcm(&g_captureContainer_t,readlen);
              //  ticks2  = SDL_GetTicks();
             //   printf("cap data frame len %d time  %d \n",readlen1,ticks2-ticks1);
                if(readlen1 == readlen){
			readlen = readlen * g_captureContainer_t.bits_per_frame / 8;
                        record_write_buffer(g_captureContainer_t.data_buf,readlen); //如果缓冲满就丢弃录制的帧
                        icount += readlen;
                        //WRT_MESSAGE("#");
                    
                }

        }
        printf("recordaudiotask exit icount = %d\n",icount);
        g_record_isexit = 2;
        return 0;
}

static void sndwaveWriteHeader(WAVContainer_t *wav)
{
	hx280_aud_stream_head head;
	head.len = sizeof(WAVContainer_t);
	memcpy(head.data , wav , sizeof(WAVContainer_t));
	if(ioctl(g_devfd , HX280ENC_SET_AUDIO_HEAD , &head) < 0)
	{
		printf("Error write wave header error = %s\n",strerror (errno));
	}
	printf("g_devfd = %d ,HX280ENC_SET_AUDIO_HEAD ok \n",printf);
}
int OpenAudio(int type,unsigned int channel,unsigned int samples_rate,unsigned int bits,int framesize)
{
	char devicename[] = "default";
	int i;
	WAVContainer_t wav;

	printf("OpenAudio type %d framesize =%d\n",type,framesize);
	if(type == TYPE_PLAY){
		memset(&g_playContainer_t,0,sizeof(SNDPCMContainer_t));
		memset(&wav,0,sizeof(WAVContainer_t));
		
     		if (snd_output_stdio_attach(&g_playContainer_t.log, stderr, 0) < 0) {
         		printf("Error snd_output_stdio_attach");
         		return NULL;
     		}
     
		if (snd_pcm_open(&g_playContainer_t.handle, devicename, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
			printf("Error snd_pcm_open [ %s]/n", devicename);
         		return NULL;
     		}
     		wav.format.channels = channel;
     		wav.format.sample_rate = samples_rate;
     		wav.format.sample_length = bits;
     		SNDWAV_PrepareWAVParams(&wav,time);

     		if(SNDWAV_SetPlayParams(&g_playContainer_t, &wav,framesize) < 0)
		{
			printf("Error SNDWAV_SetParams %d,%d,%d",wav.format.channels,wav.format.sample_rate,wav.format.sample_length);
			StopAudio(type);
			return NULL;
			
		}
		snd_pcm_dump(g_playContainer_t.handle, g_playContainer_t.log);
		
		
		/* Allocate ring-buffer memory */
		g_isplay = 0;
                g_isexit = 0;
		reset_ringbuffer();
		g_buffer_period_size = framesize;
		for(i=0;i<NUM_BUFS;i++) 
			g_buffer[i]=(unsigned char *) ZENMALLOC(g_buffer_period_size);
		g_play_sem = SDL_CreateSemaphore(0);		
		SDL_CreateThread(playaudiotask,&g_playContainer_t);
		printf("TYPE_PLAY open ok \n");
		return g_playContainer_t.handle;
	}else if(type == TYPE_CAPTURE){
		memset(&g_captureContainer_t,0,sizeof(SNDPCMContainer_t));
		memset(&wav,0,sizeof(WAVContainer_t));
  		if ((g_devfd = open(g_devname, O_WRONLY | O_CREAT, 0644)) == -1) {
         		printf( "Error open: [%s]/n", g_devname);
         		return NULL;
     		}		
     		if (snd_output_stdio_attach(&g_captureContainer_t.log, stderr, 0) < 0) {
         		printf("Error snd_output_stdio_attach");
         		return NULL;
     		}
		if (snd_pcm_open(&g_captureContainer_t.handle, devicename,SND_PCM_STREAM_CAPTURE , 0) < 0) {
			printf("Error snd_pcm_open [ %s]/n", devicename);
         		return NULL;
     		}
     		wav.format.channels = channel;
     		wav.format.sample_rate = samples_rate;
     		wav.format.sample_length = bits;
     		SNDWAV_PrepareWAVParams(&wav,time);
		if(SNDWAV_SetRecordParams(&g_captureContainer_t, &wav,0) < 0)
		{
			printf("Error SNDWAV_SetParams %d,%d,%d",wav.format.channels,wav.format.sample_rate,wav.format.sample_length);
			StopAudio(type);
			return NULL;
			
		}
		snd_pcm_dump(g_captureContainer_t.handle, g_captureContainer_t.log);
		
		sndwaveWriteHeader(&wav);
		g_isrecord = 0;
                g_record_isexit = 0;
		reset_record_ringbuffer();
		g_record_buffer_period_size = g_captureContainer_t.chunk_bytes;
		for(i=0;i<NUM_BUFS;i++) 
			g_record_buffer[i]=(unsigned char *) ZENMALLOC(g_record_buffer_period_size);
                SDL_CreateThread(recordaudiotask,&g_captureContainer_t);
                
                printf("TYPE_CAPTURE open ok %x \n",g_captureContainer_t.handle);			
		return g_captureContainer_t.handle;     		
		
	}
	return NULL;
}


void PlayAudio(unsigned char* buffer,int len)
{
    
        int ret =0;
        int delay =0;
        while(1){
        	if(g_isexit > 0)
        		break;
        	ret = write_buffer(buffer,len);
        	if(ret == 0){
  			delay = (len * 1000)/(g_playContainer_t.rate * (g_playContainer_t.bits_per_frame / 8));
  			//printf("buffer,delay = %d \n",delay);
      			SDL_Delay(delay);        		
	  		continue;
        	}else
        		break;
        }

        

	if(g_full_buffers >6 && g_isplay == 0){
	         SDL_SemPost(g_play_sem);
                 printf("开始播放audio %d \n",g_full_buffers);
	        g_isplay++;
	}	
}


void StopAudio(int type)
{
        int i;
	if(type ==  TYPE_PLAY){
		
		g_isexit  = 1;
                SDL_SemPost(g_play_sem);
		while(g_isexit != 2){
		        SDL_Delay(1000);
		}
		snd_pcm_drain(g_playContainer_t.handle);
     		snd_output_close(g_playContainer_t.log);
     		snd_pcm_close(g_playContainer_t.handle);
     		ZENFREE(g_playContainer_t.data_buf);
     		g_playContainer_t.data_buf = NULL;

		
		for(i=0;i<NUM_BUFS;i++) {
			if(g_buffer[i])
				ZENFREE(g_buffer[i]);
			g_buffer[i] = NULL;	
		}
		SDL_DestroySemaphore(g_play_sem);
		g_play_sem = NULL;
		g_isplay  = 0;
		printf("StopAudio TYPE_PLAY ok\n");
	}else if(type == TYPE_CAPTURE){
	        g_record_isexit = 1;
	        while(g_record_isexit != 2)
	                SDL_Delay(1000);
 		snd_pcm_drain(g_captureContainer_t.handle);
     		snd_output_close(g_captureContainer_t.log);
     		snd_pcm_close(g_captureContainer_t.handle);
     		ZENFREE(g_captureContainer_t.data_buf);
     		g_playContainer_t.data_buf = NULL;
     		printf("StopAudio\n");
		for(i=0;i<NUM_BUFS;i++) {
			if(g_record_buffer[i])
				ZENFREE(g_record_buffer[i]);
			g_record_buffer[i] = NULL;	
		}
		if(g_devfd != -1){
			close(g_devfd);
			g_devfd = -1;
		}
		printf("StopAudio free ok \n");     				
	}
}


int GetAudio(unsigned char* buf,int *len)
{
	
	int ret = 0;

        if(g_record_full_buffers < 6 && g_isrecord == 0){     
	        return 0;
	}else{
	        if(g_isrecord == 0)
	                g_isrecord++;
	}
        ret = record_read_buffer(buf,len);
        return ret;

}

void SetAlsaVolume(int type,int volume)
{
	snd_mixer_t *mixerFd;
	snd_mixer_elem_t *elem;
	int result;
 	if ((result = snd_mixer_open( &mixerFd, 0)) < 0)
 	{
 	       printf("snd_mixer_open error %x",result);
 	       mixerFd = NULL;
 	}
 	// Attach an HCTL to an opened mixer
 	if ((result = snd_mixer_attach( mixerFd, "default")) < 0)
 	{
 	       printf("snd_mixer_attach error %x", result);
 	       snd_mixer_close(mixerFd);
 	       mixerFd = NULL;
 	       return ;
 	}
 	// 注册混音器
 	if ((result = snd_mixer_selem_register( mixerFd, NULL, NULL)) < 0)
 	{
 	       printf("snd_mixer_selem_register error",result);
 	       snd_mixer_close(mixerFd);
 	       mixerFd = NULL;
 	       return;
 	}
 	// 加载混音器
 	if ((result = snd_mixer_load( mixerFd)) < 0)
 	{
 	       printf("snd_mixer_load error",result);
 	       snd_mixer_close(mixerFd);
 	       mixerFd = NULL;
 	       return;
 	}
 	//Playback Capture
 	for(elem=snd_mixer_first_elem(mixerFd); elem; elem=snd_mixer_elem_next(elem))
    	{
       	 	if (snd_mixer_elem_get_type(elem) == SND_MIXER_ELEM_SIMPLE &&
             				snd_mixer_selem_is_active(elem)) // 找到可以用的, 激活的elem
        	{
        		if(strcmp(snd_mixer_selem_get_name(elem),"Capture") == 0){
        			if(snd_mixer_selem_has_capture_volume(elem)){
        				long lvolume = 0;
        				long imin,imax;
        				snd_mixer_selem_get_capture_volume(elem,0,&lvolume);
        				printf("capture  front left value = %d \n",lvolume);
        
        				lvolume = 0;
        				snd_mixer_selem_get_capture_volume(elem,3,&lvolume);
        				printf("capture front right value = %d \n",lvolume); 
        				int err = snd_mixer_selem_set_capture_volume_all(elem,80);
        				printf("snd_mixer_selem_set_capture_volume_all err =%d \n",err);
        				
        				snd_mixer_selem_get_capture_volume(elem,0,&lvolume);
        				printf("capture  front left value = %d \n",lvolume);
        				
        				snd_mixer_selem_get_capture_volume(elem,3,&lvolume);
        				printf("capture front left value = %d \n",lvolume);
        				    
        				imin = imax =0;
        				 snd_mixer_selem_get_capture_volume_range(elem,&imin,&imax);
        				printf("capture volume range %d~%d\n",imin ,imax);
	    				
  
        				   				
        			}
        		}else if(strcmp(snd_mixer_selem_get_name(elem),"Playback") == 0){
        			if(snd_mixer_selem_has_playback_volume(elem)){
        				long lvolume = 0;
        				snd_mixer_selem_get_playback_volume(elem,0,&lvolume);
        				printf("Play front left value = %d \n",lvolume);
        				lvolume = 0;
        				snd_mixer_selem_get_playback_volume(elem,2,&lvolume);
        				printf("Play  rear left value = %d \n",lvolume);
        				snd_mixer_selem_get_playback_volume(elem,1,&lvolume);
        				printf("Play front right value = %d \n",lvolume);
        				lvolume = 0;
        				snd_mixer_selem_get_playback_volume(elem,3,&lvolume);
        				printf("Play rear right value = %d \n",lvolume);  
        				
        			}
        			
        		}
            		printf("snd_mixer_selem_get_name(elem) = %s \n",snd_mixer_selem_get_name(elem) );
            	
        	}
   	 }
 	
 	 snd_mixer_close(mixerFd);
 	 mixerFd = NULL;

}



int  GetAlsaVolume(int type)
{
	
	return 0;	
}


void open_alsa_dev()
{

}