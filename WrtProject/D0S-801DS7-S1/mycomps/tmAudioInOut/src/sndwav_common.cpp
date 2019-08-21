#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
//#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include "wrt_log.h"
#include "zenmalloc.h"
#include "sndwav_common.h"

#define ALSA_PERIODS 8
#define ALSA_PERIOD_SIZE 256

int SNDWAV_P_GetFormat(WAVContainer_t *wav, snd_pcm_format_t *snd_format)
{
        if (LE_SHORT(wav->format.format) != WAV_FMT_PCM)
                return -1;

        switch (LE_SHORT(wav->format.sample_length)) 
        {
        case 16:
                *snd_format = SND_PCM_FORMAT_S16_LE;
                break;
        case 8:
                *snd_format = SND_PCM_FORMAT_U8;
                break;
        default:
                *snd_format = SND_PCM_FORMAT_UNKNOWN;
                break;
        }

        return 0;
}

ssize_t SNDWAV_ReadPcm(SNDPCMContainer_t *sndpcm, size_t rcount)
{
        ssize_t r;
        size_t result = 0;
        size_t count = rcount;
        uint8_t *data = sndpcm->data_buf;
        
        if(sndpcm->handle == NULL)
        	return -1;
      //  printf("sndpcm->chunk_size = %d,count = %d \n",sndpcm->chunk_size,rcount);
        if (count != sndpcm->chunk_size) {
                count = sndpcm->chunk_size;
        }

        while (count > 0) {
                r = snd_pcm_readi(sndpcm->handle, data, count);
  
                if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
                        snd_pcm_wait(sndpcm->handle, 1000);
                } else if (r == -EPIPE) {
                        snd_pcm_prepare(sndpcm->handle);
                        printf( "<<<<<<<<<<<<<<< read Buffer Underrun >>>>>>>>>>>>>>>/n");
                } else if (r == -ESTRPIPE) {
                        printf( "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>/n");
                } else if (r < 0) {
                        printf( "Error snd_pcm_readi: [%s]", snd_strerror(r));
                        return 0;
                }
                if (r > 0) {
                        result += r;
                        count -= r;
                        data += r * sndpcm->bits_per_frame / 8;
                }
        }
        return rcount;
}

ssize_t SNDWAV_WritePcm2(SNDPCMContainer_t *sndpcm,void* pBuffer, size_t wcount)
{
        snd_pcm_sframes_t r =0;
        ssize_t result = 0;
        result = (wcount /(sndpcm->bits_per_frame / 8));
	if(result == 0)
		return 0;

	do{
 		r = snd_pcm_writei(sndpcm->handle, pBuffer, result);
 	        if(r == -EINTR)
 	        	r = 0;
		else if (r == -EAGAIN || (r >= 0 && (size_t)r < result)) {
		        snd_pcm_wait(sndpcm->handle, 1000);
		} else if (r == -EPIPE) {
		        snd_pcm_prepare(sndpcm->handle);
		        printf( "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
		} else if (r == -ESTRPIPE) {
		        printf( "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>\n");
			while ((r = snd_pcm_resume(sndpcm->handle)) == -EAGAIN)
		  		sleep(1);	        
		} else if (r < 0) {
		        printf( "Error snd_pcm_writei: [%s]", snd_strerror(r));
			if ((r = snd_pcm_prepare(sndpcm->handle)) < 0) {
		  		return(0);
		  		break;
			}	        
		       return 0;
		}

	}while(r == 0);
        return  r < 0? r: r * sndpcm->bits_per_frame / 8;		
}

static void alsa_fill_w_2 (snd_pcm_t *pcm_handle)
{
	snd_pcm_hw_params_t *hwparams=NULL;
	unsigned int channels;
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
	buffer = malloc (buffer_size_bytes);
	memset (buffer, 0, buffer_size_bytes);

	/* write data */
	snd_pcm_writei(pcm_handle, buffer, buffer_size);
	free(buffer);
}

ssize_t SNDWAV_WritePcm(SNDPCMContainer_t *sndpcm, size_t wcount)
{
        ssize_t r;
        ssize_t result = 0;
        uint8_t *data = sndpcm->data_buf;
        if(sndpcm->handle == NULL)
        	return -1;

       // printf("wcount = %d ,chunk_size = %d \n",wcount,sndpcm->chunk_size);
/*
        if (wcount < sndpcm->chunk_size) {
                snd_pcm_format_set_silence(sndpcm->format,
                        data + wcount * sndpcm->bits_per_frame / 8,
                        (sndpcm->chunk_size - wcount) * sndpcm->channels);
                wcount = sndpcm->chunk_size;
        }
*/       
        while (wcount > 0) {
                r = snd_pcm_writei(sndpcm->handle, data, wcount);
                if (r == -EAGAIN || (r >= 0 && (size_t)r < wcount)) {
                        snd_pcm_wait(sndpcm->handle, 1000);
                } else if (r == -EPIPE) {
                        snd_pcm_prepare(sndpcm->handle);
                        
                        alsa_fill_w_2(sndpcm->handle);                        
                        r = snd_pcm_writei(sndpcm->handle, data, wcount);
                         if (r<0)
                         	 printf("alsa_card_write: Error writing sound buffer (nsamples=%i):%s",wcount,snd_strerror(r));
                        
                        //printf( "<<<<<<<<<<<<<<< write Buffer Underrun >>>>>>>>>>>>>>>\n");
                } else if (r == -ESTRPIPE) {
                        printf( "<<<<<<<<<<<<<<< Need suspend >>>>>>>>>>>>>>>\n");
                } else if (r < 0) {
                        printf( "Error snd_pcm_writei: [%s]", snd_strerror(r));
                        return 0;
                }
                if (r > 0) {
                        result += r;
                        wcount -= r;
                        data += r * sndpcm->bits_per_frame / 8;
                      //  printf("wcount = %d,result= %d \n",wcount,result);
                        
                }
        }
        
        return result;

}

int SNDWAV_SetRecordParams(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav,int framesize)
{
	
	snd_pcm_hw_params_t *hwparams=NULL;
	snd_pcm_sw_params_t *swparams=NULL;
	int dir;
	unsigned int exact_uvalue;
	unsigned long exact_ulvalue;
	int channels;
	int periods=ALSA_PERIODS;
	int periodsize=ALSA_PERIOD_SIZE;
	snd_pcm_uframes_t buffersize;
	int err;
	snd_pcm_format_t format;
	
	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);
	
	/* Init hwparams with full configuration space */
	if (snd_pcm_hw_params_any(pcm_handle, hwparams) < 0) {
		ms_warning("alsa_set_params: Cannot configure this PCM device.");
		return -1;
	}
	
	if (snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		ms_warning("alsa_set_params: Error setting access.");
		return -1;
	}
	/* Set sample format */
	//format=SND_PCM_FORMAT_S16;
	/* Set sample format */
        if (SNDWAV_P_GetFormat(wav, &format) < 0) {
                printf( "Error get_snd_pcm_format/n");
                goto ERR_SET_PARAMS;
        }
	if (snd_pcm_hw_params_set_format(pcm_handle, hwparams, format) < 0) {
		ms_warning("alsa_set_params: Error setting format.");
		return -1;
	}
	
	 sndpcm->format = format;
	/* Set number of channels */
	//if (stereo) channels=2;
	//else channels=1;
	if (snd_pcm_hw_params_set_channels(pcm_handle, hwparams, LE_SHORT(wav->format.channels)) < 0) {
		ms_warning("alsa_set_params: Error setting channels.");
		return -1;
	}
	 sndpcm->channels = LE_SHORT(wav->format.channels);
	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */ 
	exact_uvalue= LE_INT(wav->format.sample_rate);
	dir=0;
	sndpcm->rate = exact_rate;
	if ((err=snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_uvalue, &dir))<0){
		ms_warning("alsa_set_params: Error setting rate to %i:%s",rate,snd_strerror(err));
		return -1;
	}
	if (dir != 0) {
		ms_warning("alsa_set_params: The rate %d Hz is not supported by your hardware.\n "
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
		ms_warning("alsa_set_params: Error setting period size.");
		return -1;
	}
	if (dir != 0) {
		ms_warning("alsa_set_params: The period size %d is not supported by your hardware.\n "
		"==> Using %d instead.", periodsize, (int)exact_ulvalue);
	}
	ms_warning("alsa_set_params: periodsize:%d Using %d", periodsize, (int)exact_ulvalue);
	periodsize=exact_ulvalue;
	/* Set number of periods. Periods used to be called fragments. */ 
	exact_uvalue=periods;
	dir=0;
	if (snd_pcm_hw_params_set_periods_near(pcm_handle, hwparams, &exact_uvalue, &dir) < 0) {
		ms_warning("alsa_set_params: Error setting periods.");
		return -1;
	}
	ms_warning("alsa_set_params: period:%d Using %d", periods, exact_uvalue);
	if (dir != 0) {
		ms_warning("alsa_set_params: The number of periods %d is not supported by your hardware.\n "
		"==> Using %d instead.", periods, exact_uvalue);
	}
	periods=exact_ulvalue;

	if (snd_pcm_hw_params_get_buffer_size(hwparams, &buffersize)<0){
		buffersize=0;
		ms_warning("alsa_set_params: could not obtain hw buffer size.");
	}
	
	/* Apply HW parameter settings to */
	/* PCM device and prepare device  */
	if ((err=snd_pcm_hw_params(pcm_handle, hwparams)) < 0) {
		ms_warning("alsa_set_params: Error setting HW params:%s",snd_strerror(err));
		return -1;
	}
	/*prepare sw params */
	if (rw){
		snd_pcm_sw_params_alloca(&swparams);
		snd_pcm_sw_params_current(pcm_handle, swparams);
		//ms_message("periodsize=%i, buffersize=%i",(int) periodsize, (int)buffersize);
		if ((err=snd_pcm_sw_params_set_start_threshold(pcm_handle, swparams,periodsize*2 ))<0){
			ms_warning("alsa_set_params: Error setting start threshold:%s",snd_strerror(err));
		}
		if ((err=snd_pcm_sw_params_set_stop_threshold(pcm_handle, swparams,periodsize*periods))<0){
			ms_warning("alsa_set_params: Error setting stop threshold:%s",snd_strerror(err));
		}
		if ((err=snd_pcm_sw_params(pcm_handle, swparams))<0){
			ms_warning("alsa_set_params: Error setting SW params:%s",snd_strerror(err));
			return -1;
		}
	}
	return 0;		
	#if 0
        snd_pcm_hw_params_t *hwparams;
        snd_pcm_sw_params_t *swparams;
        snd_pcm_format_t format;
        uint32_t exact_rate;
        uint32_t buffer_time, period_time;
        snd_pcm_uframes_t buffer_frames,period_frame;
        int dir = 0;
        int err = 0;

        /* Allocate the snd_pcm_hw_params_t structure on the stack. */
        snd_pcm_hw_params_alloca(&hwparams);
        snd_pcm_sw_params_alloca(&swparams);

        /* Init hwparams with full configuration space */
        if (snd_pcm_hw_params_any(sndpcm->handle, hwparams) < 0) {
                printf( "Error snd_pcm_hw_params_any/n");
                goto ERR_SET_PARAMS;
        }

        if (snd_pcm_hw_params_set_access(sndpcm->handle, hwparams
                , SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
                        printf( "Error snd_pcm_hw_params_set_access/n");
                        goto ERR_SET_PARAMS;
        }

        /* Set sample format */
        if (SNDWAV_P_GetFormat(wav, &format) < 0) {
                printf( "Error get_snd_pcm_format/n");
                goto ERR_SET_PARAMS;
        }
        if (snd_pcm_hw_params_set_format(sndpcm->handle, hwparams, format) < 0) {
                printf( "Error snd_pcm_hw_params_set_format/n");
                goto ERR_SET_PARAMS;
        }
        sndpcm->format = format;

        /* Set number of channels */
        if (snd_pcm_hw_params_set_channels(sndpcm->handle, hwparams
                , LE_SHORT(wav->format.channels)) < 0) {
                        printf("Error snd_pcm_hw_params_set_channels/n");
                        goto ERR_SET_PARAMS;
        }
        sndpcm->channels = LE_SHORT(wav->format.channels);

        /* Set sample rate. If the exact rate is not supported */
        /* by the hardware, use nearest possible rate.         */
        exact_rate = LE_INT(wav->format.sample_rate);
        sndpcm->rate = exact_rate;
        if (snd_pcm_hw_params_set_rate_near(sndpcm->handle, hwparams, &exact_rate, 0) < 0) {
                printf("Error snd_pcm_hw_params_set_rate_near/n");
                goto ERR_SET_PARAMS;
        }
        if (LE_INT(wav->format.sample_rate) != exact_rate) {
                printf("The rate %d Hz is not supported by your hardware./n ==> Using %d Hz instead./n",
                        LE_INT(wav->format.sample_rate), exact_rate);
        }

	//if(framesize == 0){
        	if (snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time, 0) < 0) {
        	        printf("Error snd_pcm_hw_params_get_buffer_time_max");
        	        goto ERR_SET_PARAMS;
        	}
        	if (buffer_time > 500000) 
        		buffer_time = 500000;
        	printf("buffer_time = %d \n",buffer_time);
        	if (snd_pcm_hw_params_set_buffer_time_near(sndpcm->handle, hwparams
        	        , &buffer_time, 0) < 0) {
        	                printf("Error snd_pcm_hw_params_set_buffer_time_near");
        	                goto ERR_SET_PARAMS;
        	}
        	        		
        	
        	period_time = 24000;
        	
        	if (snd_pcm_hw_params_set_period_time_near(sndpcm->handle, hwparams
        	        , &period_time, 0) < 0) {
        	                printf( "Error snd_pcm_hw_params_set_period_time_near");
        	                goto ERR_SET_PARAMS;
        	}
        	        	
        	

		printf("buffer_time  = %d ,period_time = %d \n",buffer_time,period_time);
/*
	}else{
		period_frame = framesize;
		buffer_frames = period_frame * 4;
		dir = 0;
		if(snd_pcm_hw_params_set_period_size_near(sndpcm->handle,hwparams,&period_frame,&dir) < 0){
			printf( "Error snd_pcm_hw_params_set_period_size_near period_frame = %d",period_frame);
        	        goto ERR_SET_PARAMS;
		}
		if(dir != 0){
		        printf ("old framesize = %d ,new framesize = %d \n",framesize,period_frame);
		}
		
		if(snd_pcm_hw_params_set_buffer_size_near(sndpcm->handle,hwparams,&buffer_frames) < 0){
			printf( "Error snd_pcm_hw_params_set_period_size_near buffer_frames = %d",buffer_frames);
        	        goto ERR_SET_PARAMS;
		}

		
	}
*/
        /* Set hw params */
        if (snd_pcm_hw_params(sndpcm->handle, hwparams) < 0) {
                printf("Error snd_pcm_hw_params(handle, params)");
                goto ERR_SET_PARAMS;
        }

        snd_pcm_hw_params_get_period_size(hwparams, &sndpcm->chunk_size, 0);
        snd_pcm_hw_params_get_buffer_size(hwparams, &sndpcm->buffer_size);
        if (sndpcm->chunk_size == sndpcm->buffer_size) {
                printf("Can't use period equal to buffer size (%lu == %lu)"
                        , sndpcm->chunk_size, sndpcm->buffer_size);
                goto ERR_SET_PARAMS;
        }
        
         snd_pcm_sw_params_current(sndpcm->handle, swparams);
   	err = snd_pcm_sw_params_set_sleep_min(sndpcm->handle, swparams,0);
   	 assert(err >= 0);         
   	 err = snd_pcm_sw_params_set_avail_min(sndpcm->handle, swparams, sndpcm->chunk_size);
    	assert(err >= 0);

    	err = snd_pcm_sw_params_set_start_threshold(sndpcm->handle, swparams, 1);
   	 assert(err >= 0);
    	err = snd_pcm_sw_params_set_stop_threshold(sndpcm->handle, swparams, sndpcm->buffer_size);
    	 assert(err >= 0);
    	 
    	 snd_pcm_sw_params(sndpcm->handle, swparams);
    	  assert(err >= 0);
    	 
        sndpcm->bits_per_sample = snd_pcm_format_physical_width(format);
        sndpcm->bits_per_frame = sndpcm->bits_per_sample * LE_SHORT(wav->format.channels);

        sndpcm->chunk_bytes = sndpcm->chunk_size * sndpcm->bits_per_frame / 8;

	printf("sndpcm->format = %d \n",sndpcm->format);
	printf("sndpcm->channels= %d \n",sndpcm->channels);
	printf("sndpcm->buffer_size = %d \n",sndpcm->buffer_size);
	printf("sndpcm->chunk_bytes = %d \n",sndpcm->chunk_bytes);
	printf("sndpcm->bits_per_sample= %d \n",sndpcm->bits_per_sample);
	printf("sndpcm->bits_per_frame = %d \n",sndpcm->bits_per_frame);
	printf("sndpcm->chunk_size = %d \n",sndpcm->chunk_size);
        /* Allocate audio data buffer */
        sndpcm->data_buf = (uint8_t *)ZENMALLOC(sndpcm->chunk_bytes);
        if (!sndpcm->data_buf) {
                printf( "Error ZENMALLOC: [data_buf]");
                goto ERR_SET_PARAMS;
        }

        return 0;

ERR_SET_PARAMS:
        return -1;
#endif
        
}

int SNDWAV_PrepareWAVParams(WAVContainer_t *wav, int seconds)
{
     assert(wav);
     
     uint32_t duration_time = seconds*1000;//DEFAULT_DURATION_TIME;
     if(wav->format.channels == 0)
     	wav->format.channels = DEFAULT_CHANNELS;
     if( wav->format.sample_rate == 0)
     	 wav->format.sample_rate = DEFAULT_SAMPLE_RATE;
     if(wav->format.sample_length == 0)
     	wav->format.sample_length = DEFAULT_SAMPLE_LENGTH;



     /* Const */
     wav->header.magic = WAV_RIFF;
     wav->header.type = WAV_WAVE;
     wav->format.magic = WAV_FMT;
     wav->format.fmt_size = LE_INT(DEFAULT_SAMPLE_LENGTH);
     wav->format.format = LE_SHORT(WAV_FMT_PCM);
     wav->chunk.type = WAV_DATA;

     /* User definition */
     wav->format.channels = LE_SHORT( wav->format.channels);
     wav->format.sample_rate = LE_INT(wav->format.sample_rate);
     wav->format.sample_length = LE_SHORT(wav->format.sample_length);

     /* See format of wav file */
	wav->format.blocks_align = LE_SHORT(wav->format.channels * wav->format.sample_length / 8);
	wav->format.bytes_p_second = LE_INT((uint16_t)(wav->format.blocks_align) * wav->format.sample_rate);

	wav->chunk.length = LE_INT(duration_time  * (uint32_t)(wav->format.bytes_p_second)/1000);
	wav->header.length = LE_INT((uint32_t)(wav->chunk.length) +
         sizeof(wav->chunk) + sizeof(wav->format) + sizeof(wav->header) - 8);
	//printf("wav->header.length = %x ,wav->chunk.length = %x \n",wav->header.length ,wav->chunk.length);
     return 0;	
}


int SNDWAV_SetPlayParams(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav,int framesize)
{
	unsigned period_time = 0;
	unsigned buffer_time = 0;
	snd_pcm_uframes_t period_frames = 0;
	snd_pcm_uframes_t buffer_frames = 0;
    	snd_pcm_hw_params_t *params;
    	snd_pcm_sw_params_t *swparams;	
    	snd_pcm_uframes_t xfer_align;  
	int avail_min = -1;
	int start_delay = 0;
	int stop_delay = 0;
	int dir = 0;

	
	int err = 0;
	size_t n = 0;
	unsigned int rate = 0;
	snd_pcm_uframes_t start_threshold = 0, stop_threshold = 0;
	snd_pcm_format_t snd_format;

	assert(sndpcm);
	assert(wav);
        if (SNDWAV_P_GetFormat(wav, &snd_format)  < 0) {
                printf("SNDWAV_P_GetFormat error");
                return -1;
        }
	
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	
	err = snd_pcm_hw_params_any(sndpcm->handle, params);
	if(err < 0){
		printf("snd_pcm_hw_params_any error %d",err);
		return -1;
	}

	err = snd_pcm_hw_params_set_access(sndpcm->handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if(err < 0){
		printf("snd_pcm_hw_params_set_access error %d",err);
		return -1;		
	}
						 

	err = snd_pcm_hw_params_set_format(sndpcm->handle, params, snd_format);
	if(err < 0){
		printf("snd_pcm_hw_params_set_format error %d",err);
		return -1;		
	}

	err = snd_pcm_hw_params_set_channels(sndpcm->handle, params, wav->format.channels);
	if(err < 0){
		printf("snd_pcm_hw_params_set_format error %d",err);
		return -1;			
	}
	
	sndpcm->channels = LE_SHORT(wav->format.channels);
	sndpcm->format  = snd_format;
		

	rate =  wav->format.sample_rate;
	err = snd_pcm_hw_params_set_rate_near(sndpcm->handle, params, &rate, 0);
	if(err < 0){
		printf("snd_pcm_hw_params_set_rate_near error %d",err);
		return -1;
	}
	
	wav->format.sample_rate = rate;
	
	//if(framesize == 0){
		 dir = 0;
		if (buffer_time == 0 && buffer_frames == 0) {
			err = snd_pcm_hw_params_get_buffer_time_max(params,&buffer_time, 0);
			if(err < 0){
				printf("snd_pcm_hw_params_get_buffer_time_max error %d",err);
				return -1;
			}
			if (buffer_time > 500000)
				buffer_time = 500000;
		}
		if(framesize == 768)
			period_time = 3840 ;
		else 
			period_time = buffer_time/4;
		printf("period_time = %d \n",period_time);

		err = snd_pcm_hw_params_set_period_time_near(sndpcm->handle, params,
								     &period_time, &dir);

		if(err < 0){
			printf("period_time = %d ,snd_pcm_hw_params_set_period_size_near error %d",period_time,err);
			return -1;
		}
		if(dir != 0){
			printf("period_time instead of = %d \n",period_time);
		}

		
		
		err = snd_pcm_hw_params_set_buffer_time_near(sndpcm->handle, params,
								     &buffer_time, 0);
		if(err < 0){
			printf("buffer_time = %d ,snd_pcm_hw_params_set_period_size_near error %d",buffer_time,err);
			return -1;
		}
		
		printf("snd_pcm_hw_params_set_buffer_time_near %d \n",buffer_time);				



		printf("buffer_time  = %d ,period_time = %d \n",buffer_time,period_time);
#if 0
	}else{  
		 period_frames = framesize;    
		 buffer_frames = period_frames * 4;
		 dir = 0;
		 err = snd_pcm_hw_params_set_period_size_near(sndpcm->handle, params,&period_frames, &dir);	
		 if(err < 0){
			printf("period_frames = %d ,snd_pcm_hw_params_set_period_size_near error %d",period_frames,err);
			return -1;
		 }
		 if(dir != 0){
		 	printf("period_frames size  %d is not support by your hardware used %d instead dir =%d\n err = %d",framesize,period_frames,dir,err);
		 }
		 printf("period_frames = %d \n",period_frames);
 		 err = snd_pcm_hw_params_set_buffer_size_near(sndpcm->handle, params,&buffer_frames);	
		 if(err < 0){
			printf("buffer_frames = %d ,snd_pcm_hw_params_set_period_size_near error %d",buffer_frames,err);
			return -1;
		 }				 
		 printf("buffer_frames = %d \n",buffer_frames);
	}
#endif	
	
	err = snd_pcm_hw_params(sndpcm->handle, params);

	snd_pcm_hw_params_get_period_size(params, &sndpcm->chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &sndpcm->buffer_size);

	snd_pcm_sw_params_current(sndpcm->handle, swparams);
	err = snd_pcm_sw_params_get_xfer_align(swparams, &xfer_align);  
 	if (err < 0) {  
  		printf("Unable to obtain xfer align\n");
  		return -1;
  	
	}
 	printf("xfer_align = %d \n",xfer_align);
	if (avail_min < 0)
		n = sndpcm->chunk_size;
	else
		n = (double) rate * avail_min / 1000000;
	err = snd_pcm_sw_params_set_avail_min(sndpcm->handle, swparams, n);

	/* round up to closest transfer boundary */
	//n = sndpcm->buffer_size;
	n = (sndpcm->buffer_size / xfer_align) * xfer_align;
	if (start_delay <= 0) {
		start_threshold = n + (double) rate * start_delay / 1000000;
	} else
		start_threshold = (double) rate * start_delay / 1000000;
	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > n)
		start_threshold = n;
		
	err = snd_pcm_sw_params_set_start_threshold(sndpcm->handle, swparams,start_threshold);
	assert(err >= 0);
	
	if (stop_delay <= 0)
		stop_threshold = sndpcm->buffer_size + (double) rate * stop_delay / 1000000;
	else
		stop_threshold = (double) rate * stop_delay / 1000000;
 	
 	//stop_threshold = sndpcm->chunk_size * sndpcm->chunk_size;
 	//snd_pcm_sw_params_get_boundary(swparams, &stop_threshold);
	err = snd_pcm_sw_params_set_stop_threshold(sndpcm->handle, swparams, stop_threshold);
	assert(err >= 0);

	if ((err= snd_pcm_sw_params(sndpcm->handle, swparams)) < 0) {
		printf("snd_pcm_sw_params error %s",snd_strerror(err));
		return -1;
	}
	
	sndpcm->bits_per_sample = snd_pcm_format_physical_width(snd_format);
	sndpcm->bits_per_frame = sndpcm->bits_per_sample * wav->format.channels;
	sndpcm->chunk_bytes = sndpcm->chunk_size  * sndpcm->bits_per_frame / 8;
	sndpcm->rate = rate;	

	
	printf("<<SNDWAV_SetPlayParams>>\n");
	printf("sndpcm->format = %d \n",sndpcm->format);
	printf("sndpcm->channels= %d \n",sndpcm->channels);
	printf("sndpcm->buffer_size = %d \n",sndpcm->buffer_size);
	printf("sndpcm->chunk_bytes = %d \n",sndpcm->chunk_bytes);
	printf("sndpcm->bits_per_sample= %d \n",sndpcm->bits_per_sample);
	printf("sndpcm->bits_per_frame = %d \n",sndpcm->bits_per_frame);
	printf("sndpcm->chunk_size = %d \n",sndpcm->chunk_size);
	printf("stop_threshold = %d \n",stop_threshold);
	


        sndpcm->data_buf = (uint8_t *)ZENMALLOC(sndpcm->chunk_bytes);
        if (!sndpcm->data_buf) {
                printf( "Error ZENMALLOC: [data_buf]");
               return -1;
        }

	return 0;	
}


