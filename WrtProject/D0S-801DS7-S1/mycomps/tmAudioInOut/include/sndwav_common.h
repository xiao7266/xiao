#ifndef __SNDWAV_COMMON_H
#define __SNDWAV_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "wav_parser.h"

#define DEFAULT_CHANNELS         (2)
#define DEFAULT_SAMPLE_RATE      (16000)
#define DEFAULT_SAMPLE_LENGTH    (16)
#define DEFAULT_DURATION_TIME    (10)


#ifdef __cplusplus
extern "C" {
#endif


        typedef long long off64_t;
        

        typedef struct SNDPCMContainer {
                snd_pcm_t *handle;
                snd_output_t *log;
                snd_pcm_uframes_t chunk_size;
                snd_pcm_uframes_t buffer_size;
                snd_pcm_format_t format;
                uint16_t channels;
                uint32_t rate;
                size_t chunk_bytes;
                size_t bits_per_sample;
                size_t bits_per_frame;

                uint8_t *data_buf;
        } SNDPCMContainer_t;

        ssize_t SNDWAV_ReadPcm(SNDPCMContainer_t *sndpcm, size_t rcount);

        ssize_t SNDWAV_WritePcm(SNDPCMContainer_t *sndpcm, size_t wcount);
        ssize_t SNDWAV_WritePcm2(SNDPCMContainer_t *sndpcm,void* pBuffer, size_t wcount);

        int SNDWAV_SetRecordParams(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav,int framesize);
        
	int SNDWAV_PrepareWAVParams(WAVContainer_t *wav, int seconds);   
	int SNDWAV_SetPlayParams(SNDPCMContainer_t *sndpcm, WAVContainer_t *wav,int framesize); 
	
	 
	

#ifdef __cplusplus
}
#endif


#endif /* #ifndef __SNDWAV_COMMON_H */
