/* HTAB = 4 */
/****************************************************************************
 * madlld.c -- A simple program decoding an MPEG audio stream to 16-bit		*
 * PCM from stdin to stdout. This program is just a simple sample			*
 * demonstrating how the low-level libmad API can be used.					*
 *--------------------------------------------------------------------------*
 * (c) 2001--2004 Bertrand Petit											*
 *																			*
 * Redistribution and use in source and binary forms, with or without		*
 * modification, are permitted provided that the following conditions		*
 * are met:																	*
 *																			*
 * 1. Redistributions of source code must retain the above copyright		*
 *    notice, this list of conditions and the following disclaimer.			*
 *																			*
 * 2. Redistributions in binary form must reproduce the above				*
 *    copyright notice, this list of conditions and the following			*
 *    disclaimer in the documentation and/or other materials provided		*
 *    with the distribution.												*
 * 																			*
 * 3. Neither the name of the author nor the names of its contributors		*
 *    may be used to endorse or promote products derived from this			*
 *    software without specific prior written permission.					*
 * 																			*
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS''		*
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED		*
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A			*
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR		*
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,				*
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT			*
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF			*
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND		*
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,		*
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT		*
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF		*
 * SUCH DAMAGE.																*
 *																			*
 ****************************************************************************/

/*
 * $Name: v1_1p1 $
 * $Date: 2004/03/19 07:13:13 $
 * $Revision: 1.20 $
 */

/****************************************************************************
 * Includes																	*
 ****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <math.h> /* for pow() and log10() */
#include <mad.h>

/* Should we use getopt() for command-line arguments parsing? */
#if (defined(unix) || defined (__unix__) || defined(__unix) || \
	 defined(HAVE_GETOPT)) \
	&& !defined(WITHOUT_GETOPT)
#include <unistd.h>
#define HAVE_GETOPT
#else
#include <ctype.h>
#undef HAVE_GETOPT
#endif


static
unsigned long const bitrate_table[5][15] = {
  /* MPEG-1 */
  { 0,  32000,  64000,  96000, 128000, 160000, 192000, 224000,  /* Layer I   */
       256000, 288000, 320000, 352000, 384000, 416000, 448000 },
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer II  */
       128000, 160000, 192000, 224000, 256000, 320000, 384000 },
  { 0,  32000,  40000,  48000,  56000,  64000,  80000,  96000,  /* Layer III */
       112000, 128000, 160000, 192000, 224000, 256000, 320000 },

  /* MPEG-2 LSF */
  { 0,  32000,  48000,  56000,  64000,  80000,  96000, 112000,  /* Layer I   */
       128000, 144000, 160000, 176000, 192000, 224000, 256000 },
  { 0,   8000,  16000,  24000,  32000,  40000,  48000,  56000,  /* Layers    */
        64000,  80000,  96000, 112000, 128000, 144000, 160000 } /* II & III  */
};

static
unsigned int const samplerate_table[3] = { 44100, 48000, 32000 };


/****************************************************************************
 * Global variables.														*
 ****************************************************************************/
#define MAXINBUFSIZE 8192
struct mad_header   gHeader;
struct mad_frame	gFrame;
struct mad_synth	gSynth;
struct mad_stream	gStream;
mad_timer_t			gTimer;
unsigned long		gFrameCount=0;
int                 gInputFlag;
unsigned char       *InBuf=NULL;
int                 InBufSize;
int                 InputBufferRemainLen;

struct MpegAudioHead 
{
	int version;
	int stereo;
	int layer;
	int sampling_freq;
	int bitrate;
} gMpegAudioHead;

FILE *file;

/* Keeps a pointer to the program invocation name for the error
 * messages.
 */
const char	*ProgName;

/* This table represents the subband-domain filter characteristics. It
 * is initialized by the ParseArgs() function and is used as
 * coefficients against each subband samples when DoFilter is non-nul.
 */
mad_fixed_t	Filter[32];

/* DoFilter is non-nul when the Filter table defines a filter bank to
 * be applied to the decoded audio subbands.
 */
int			DoFilter=0;

/****************************************************************************
 * Return an error string associated with a mad error code.					*
 ****************************************************************************/
/* Mad version 0.14.2b introduced the mad_stream_errorstr() function.
 * For previous library versions a replacement is provided below.
 */
#if (MAD_VERSION_MAJOR>=1) || \
    ((MAD_VERSION_MAJOR==0) && \
     (((MAD_VERSION_MINOR==14) && \
       (MAD_VERSION_PATCH>=2)) || \
      (MAD_VERSION_MINOR>14)))
#define MadErrorString(x) mad_stream_errorstr(x)
#else
static const char *MadErrorString(const struct mad_stream *Stream)
{
	switch(Stream->error)
	{
		/* Generic unrecoverable errors. */
		case MAD_ERROR_BUFLEN:
			return("input buffer too small (or EOF)");
		case MAD_ERROR_BUFPTR:
			return("invalid (null) buffer pointer");
		case MAD_ERROR_NOMEM:
			return("not enough memory");

		/* Frame header related unrecoverable errors. */
		case MAD_ERROR_LOSTSYNC:
			return("lost synchronization");
		case MAD_ERROR_BADLAYER:
			return("reserved header layer value");
		case MAD_ERROR_BADBITRATE:
			return("forbidden bitrate value");
		case MAD_ERROR_BADSAMPLERATE:
			return("reserved sample frequency value");
		case MAD_ERROR_BADEMPHASIS:
			return("reserved emphasis value");

		/* Recoverable errors */
		case MAD_ERROR_BADCRC:
			return("CRC check failed");
		case MAD_ERROR_BADBITALLOC:
			return("forbidden bit allocation value");
		case MAD_ERROR_BADSCALEFACTOR:
			return("bad scalefactor index");
		case MAD_ERROR_BADFRAMELEN:
			return("bad frame length");
		case MAD_ERROR_BADBIGVALUES:
			return("bad big_values count");
		case MAD_ERROR_BADBLOCKTYPE:
			return("reserved block_type");
		case MAD_ERROR_BADSCFSI:
			return("bad scalefactor selection info");
		case MAD_ERROR_BADDATAPTR:
			return("bad main_data_begin pointer");
		case MAD_ERROR_BADPART3LEN:
			return("bad audio data length");
		case MAD_ERROR_BADHUFFTABLE:
			return("bad Huffman table select");
		case MAD_ERROR_BADHUFFDATA:
			return("Huffman data overrun");
		case MAD_ERROR_BADSTEREO:
			return("incompatible block_type for JS");

		/* Unknown error. This switch may be out of sync with libmad's
		 * defined error codes.
		 */
		default:
			return("Unknown error code");
	}
}
#endif

/****************************************************************************
 * Converts a sample from libmad's fixed point number format to a signed	*
 * short (16 bits).															*
 ****************************************************************************/
static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}

/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
static int PrintFrameInfo(FILE *fp, struct mad_header *Header)
{
	const char	*Layer,
				*Mode,
				*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
			break;
		case MAD_LAYER_II:
			Layer="II";
			break;
		case MAD_LAYER_III:
			Layer="III";
			break;
		default:
			Layer="(unexpected layer value)";
			break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
			break;
		default:
			Mode="(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
			break;
#if (MAD_VERSION_MAJOR>=1) || \
	((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
		case MAD_EMPHASIS_RESERVED:
			Emphasis="reserved(!)";
			break;
#endif
		default:
			Emphasis="(unexpected emphasis value)";
			break;
	}

	fprintf(fp,"%lu kb/s audio MPEG layer %s stream %s CRC, "
			"%s with %s emphasis at %d Hz sample rate\n",
			Header->bitrate,Layer,
			Header->flags&MAD_FLAG_PROTECTION?"with":"without",
			Mode,Emphasis,Header->samplerate);
	return(ferror(fp));
}

/****************************************************************************
 * Applies a frequency-domain filter to audio data in the subband-domain.	*
 ****************************************************************************/
static void ApplyFilter(struct mad_frame *Frame)
{
	
#if 0	
	int	Channel,
		Sample,
		Samples,
		SubBand;

	/* There is two application loops, each optimized for the number
	 * of audio channels to process. The first alternative is for
	 * two-channel frames, the second is for mono-audio.
	 */
	Samples=MAD_NSBSAMPLES(&Frame->header);
	if(Frame->header.mode!=MAD_MODE_SINGLE_CHANNEL)
		for(Channel=0;Channel<2;Channel++)
			for(Sample=0;Sample<Samples;Sample++)
				for(SubBand=0;SubBand<32;SubBand++)
					(*Frame->sbsample)[Channel][Sample][SubBand]=
						mad_f_mul((*Frame->sbsample)[Channel][Sample][SubBand],
								  Filter[SubBand]);
	else
		for(Sample=0;Sample<Samples;Sample++)
			for(SubBand=0;SubBand<32;SubBand++)
				(*Frame->sbsample)[0][Sample][SubBand]=
					mad_f_mul((*Frame->sbsample)[0][Sample][SubBand],
							  Filter[SubBand]);
#endif							  
}

#if 0
int MpegAudioGetVersion()
{
	int version;

	if ((gHeader.flags&MAD_FLAG_MPEG_2_5_EXT) != 0)
		version = 1;
	else if ((gHeader.flags&MAD_FLAG_LSF_EXT) != 0)
		version = 1;
	else
		version = 0;

	return version;
}

int MpegAudioIsstereo()
{
	if (gHeader.mode == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int MpegAudioGetLayer()
{
	int layer;

	switch(gHeader.layer)
	{
		case MAD_LAYER_I:
			layer=1;
			break;
		case MAD_LAYER_II:
			layer=2;
			break;
		case MAD_LAYER_III:
			layer=3;
			break;
		default:
			layer=0;
			break;
	}
	return layer;
}

int MpegAudioGetFrequency()
{
	return gHeader.samplerate;
}

int MpegAudioFindHead(unsigned char *InputBuffer, int ReadSize, unsigned int *FramePtr)
{
	int					Status=0;
	unsigned int        skipped;
	int                 version;

	for (skipped=0; skipped <= ReadSize - 4; skipped++) 
	{
		if (InputBuffer[skipped] == 0xff &&
			((InputBuffer[skipped + 1] & 0xe0) == 0xe0)) 
		{
			mad_stream_buffer(&gStream,InputBuffer+skipped,ReadSize-skipped);
			gStream.error=0;

			Status = mad_header_decode(&gHeader,&gStream);

			if ((Status==0) || (gStream.error==MAD_ERROR_BUFLEN))
			{
				if (gHeader.layer == MAD_LAYER_I)
				{
					*FramePtr = 12000*(gHeader.bitrate/1000)/gHeader.samplerate;
					if((gHeader.samplerate==44100) && ((gHeader.flags&MAD_FLAG_PADDING)!=0))
					{
						*FramePtr++;
					}
					*FramePtr<<=2;
				}
				else
				{
					if ((gHeader.flags&MAD_FLAG_MPEG_2_5_EXT) != 0)
						version = 1;
					else if ((gHeader.flags&MAD_FLAG_LSF_EXT) != 0)
						version = 1;
					else
						version = 0;

					*FramePtr = (144000*(gHeader.bitrate/1000))/(gHeader.samplerate<<version);
				}

				return skipped;
			}
		}
	      
	}

	return -1;
}
#else
int MpegAudioGetVersion()
{
	return gMpegAudioHead.version;
}

int MpegAudioIsstereo()
{
	return gMpegAudioHead.stereo;
}

int MpegAudioGetLayer()
{
	return gMpegAudioHead.layer;
}

int MpegAudioGetFrequency()
{
	return gMpegAudioHead. sampling_freq;
}

int MpegAudioFindHead(unsigned char *InputBuffer, int ReadSize, unsigned int *FramePtr)
{
	int					Status=0;
	unsigned int        skipped;
	unsigned char       c;
	int                 mpeg25;
	int                 mode;
	int                 padding;
	int                 lsf_ext;
	int                 bitrateindex;
	int                 frequency;

	for (skipped=0; skipped <= ReadSize - 4; skipped++) 
	{
		if (InputBuffer[skipped] == 0xff &&
			((InputBuffer[skipped + 1] & 0xe0) == 0xe0)) 
		{
			//解析MP3数据头
			c = InputBuffer[skipped+1];

			if ((c & 0x10) == 0)
				mpeg25 = 1;
			else 
				mpeg25 = 0;
			c &= 0xf;
			lsf_ext = 1-((c >> 3) & 1);
			gMpegAudioHead.layer = 4 - ((c >> 1) & 3);
			if (mpeg25 == 0)
				gMpegAudioHead.version = lsf_ext;
			else 
				gMpegAudioHead.version = 1;


			c = InputBuffer[skipped+2] >> 1;

			padding = (c & 1);
			c >>= 1;
			frequency = (c&3);
			if (frequency == 3)
				continue;
			gMpegAudioHead.sampling_freq = samplerate_table[frequency];
			if (lsf_ext) 
			{
				gMpegAudioHead.sampling_freq /= 2;
				if (mpeg25)
				{
					gMpegAudioHead.sampling_freq /= 2;
				}
			}

			c >>= 2;
			bitrateindex = (int) c;
			if( bitrateindex == 15 )
				continue;
			if (lsf_ext)
				gMpegAudioHead.bitrate = bitrate_table[3 + (gMpegAudioHead.layer >> 1)][bitrateindex];
			else
				gMpegAudioHead.bitrate = bitrate_table[gMpegAudioHead.layer - 1][bitrateindex];


			c = InputBuffer[skipped+3] >> 4;

			mode = (c >> 2);
			gMpegAudioHead.stereo = (mode == 3) ? 0 : 1;


			
			// framesize & slots
			if (gMpegAudioHead.layer == 1)
			{
				*FramePtr = 12000*(gMpegAudioHead.bitrate/1000)/gMpegAudioHead.sampling_freq;
				if((gMpegAudioHead.sampling_freq==44100) && (padding))
				{
					*FramePtr++;
				}
				*FramePtr<<=2;
			}
			else
			{
				*FramePtr = (144000*(gMpegAudioHead.bitrate/1000))/(gMpegAudioHead.sampling_freq<<gMpegAudioHead.version);
			}

			return skipped;
		}
	      
	}

	return -1;
}
#endif

void MpegAudioInit()
{
	if (InBuf == NULL)
	{
		InBuf = (unsigned char *)malloc(sizeof(unsigned char)*MAXINBUFSIZE);
		assert(InBuf != NULL);

		/* First the structures used by libmad must be initialized. */
		mad_header_init(&gHeader);
		mad_stream_init(&gStream);
		mad_frame_init(&gFrame);
		mad_synth_init(&gSynth);
		mad_timer_reset(&gTimer);

		gStream.error = 0;
		InBufSize = 0;
		InputBufferRemainLen = 0;
		gInputFlag = 0;
		gFrameCount = 0;
	}
}


void MpegAudioClose()
{
	if (InBuf != NULL)
	{
		free(InBuf);
		InBuf = NULL;

		/* Mad is no longer used, the structures that were initialized must
		 * now be cleared.
		 */
		mad_header_finish(&gHeader);
		mad_synth_finish(&gSynth);
		mad_frame_finish(&gFrame);
		mad_stream_finish(&gStream);
	}
}

/****************************************************************************
 * Main decoding loop. This is where mad is used.							*
 ****************************************************************************/
int MpegAudioDecoder(unsigned char *OutBuffer,unsigned char *InputBuffer, int ReadSize,int *outputlen)
{
	int					Status=0;
	int                 i;
	int                 templen = 0;
	/* Decoding options can here be set in the options field of the
	 * Stream structure.
	 */

	/* Pipe the new buffer content to libmad's stream decoder
     * facility.
	 */
	if (gInputFlag == 0)
	{
		memcpy(InBuf,InputBuffer,ReadSize);
		mad_stream_buffer(&gStream,InBuf,ReadSize);
		gStream.error = 0;
		gInputFlag = 1;
	}
	else if (gStream.error != 0)
	{
		if(!MAD_RECOVERABLE(gStream.error))
		{
			if (gStream.error==MAD_ERROR_BUFLEN)
			{
				memcpy(InBuf,gStream.next_frame,(gStream.bufend-gStream.next_frame));
				templen = (gStream.bufend-gStream.next_frame);
			}
			else
			{
				templen = 0;
			}

			memcpy(InBuf+templen,InputBuffer,ReadSize);
			mad_stream_buffer(&gStream,InBuf,ReadSize+templen);
			gStream.error = 0;
		}
	}

	if(mad_frame_decode(&gFrame,&gStream))
	{
		if(MAD_RECOVERABLE(gStream.error))
		{
			//if(gStream.error!=MAD_ERROR_LOSTSYNC ||
			//	   gStream.this_frame!=GuardPtr)
			if(gStream.error!=MAD_ERROR_LOSTSYNC)
			{
				//fprintf(stderr,"recoverable frame level error (%s)\n",
				//			MadErrorString(&gStream));
				//fflush(stderr);
				//printf("recoverable frame level error (%s)\n",MadErrorString(&gStream));
			}
		}
		else
		{
			if(gStream.error==MAD_ERROR_BUFLEN)
			{
				return ReadSize;
			}
			else
			{
				fprintf(stderr,"unrecoverable frame level error (%s).\n",
						MadErrorString(&gStream));
				Status=1;
				return -1;
			}
		}
	}

	gFrameCount++;
	mad_timer_add(&gTimer,gFrame.header.duration);

	if(DoFilter)
		ApplyFilter(&gFrame);

	mad_synth_frame(&gSynth,&gFrame);

		
	for(i=0;i<gSynth.pcm.length;i++)
	{
		signed short	Sample;

		/* Left channel */
		Sample=MadFixedToSshort((gSynth.pcm.samples)[0][i]); // by ljw (*gSynth.pcm.samples)[0][i] ???
		
		*(OutBuffer++)=Sample&0xff;
		*(OutBuffer++)=Sample>>8;
		*outputlen = *outputlen+2;
		/* Right channel. If the decoded stream is monophonic then
		 * the right output channel is the same as the left one.
		 */
		if(MAD_NCHANNELS(&gFrame.header)==2)
		{
			Sample=MadFixedToSshort((gSynth.pcm.samples)[1][i]);// by ljw (*gSynth.pcm.samples)[1][i] ???
			*(OutBuffer++)=Sample&0xff;
			*(OutBuffer++)=Sample>>8;
			*outputlen = *outputlen+2;
		}
	}

	/* That's the end of the world (in the H. G. Wells way). */
	return 0;
}

/****************************************************************************
 * Prints a message on stderr explaining the usage of the program. Two		*
 * versions of this function are provided, depending on the system type.	*
 ****************************************************************************/
static void PrintUsage(void)
{
#ifdef HAVE_GETOPT /* This version is for Unix systems. */
	fprintf(stderr,"usage: [-p] [-a <amp/atten>]\n"
			"\t-a\tSets an amplification or attenuation factor expressed\n"
			"\t\tin dB. The factor bounds are [-Inf,%f].\n"
			"\t-p\tRequests that the output samples be filtered as if\n"
			"\t\ttransmitted through a telephone switch.\n",
			20.*log10(mad_f_todouble(MAD_F_MAX)));
#else /* HAVE_GETOPT */ /* This other version is for non-Unix systems. */
	fprintf(stderr,"usage: [<number>] [phone]\n"
			"\t<number> is a floating point number expressing an "
			"amplification\n"
			"\t\tor attenuation factor expressed in dB. The factor bounds\n"
			"\t\tare [-Inf,%f].\n"
			"\tThe \"phone\" argument requests that the output samples be "
			"filtered\n"
			"\t\tas if transmitted through a telephone switch.\n",
			20.*log10(mad_f_todouble(MAD_F_MAX)));
#endif /* HAVE_GETOPT */
}

/****************************************************************************
 * Command-line arguments parsing. We use two methods and two command-line	*
 * formats depending on the system type. On unix system we apply the good	*
 * old getopt() method, other system are offered a really primitive options	*
 * interface.																*
 ****************************************************************************/
static int ParseArgs(int argc, char * const argv[])
{
	int				DoPhoneFilter=0,
					i;
	double			AmpFactor;
	mad_fixed_t		Amp=MAD_F_ONE;

#ifdef HAVE_GETOPT /* This version is for Unix systems. */
	int				Option;

	/* Parse the command line. */
	while((Option=getopt(argc,argv,"a:p"))!=-1)
		switch(Option)
		{
			/* {5} Set the amplification/attenuation factor, expressed
			 * in dB.
			 */
			case 'a':
				/* If the current linear amplification factor is not
				 * one (MAD_F_ONE) then is was already set. Setting it
				 * again is not permitted.
				 */
				if(Amp!=MAD_F_ONE)
				{
					fprintf(stderr,"the amplification/attenuation factor "
							"was set several times.\n");
					return(1);
				}

				/* The decibel value is converted to a linear factor.
				 * That factor is checked against the maximum value
				 * that can be stored in a mad_fixed_t. The upper
				 * bound is MAD_F_MAX, it is converted to a double
				 * value with mad_f_todouble() for comparison.
				 */
				AmpFactor=pow(10.,atof(optarg)/20);
				if(AmpFactor>mad_f_todouble(MAD_F_MAX))
				{
					fprintf(stderr,"amplification out of range.\n");
					return(1);
				}

				/* Eventually the amplification factor is converted
				 * from double to fixed point with mad_f_tofixed().
				 */
				Amp=mad_f_tofixed(AmpFactor);
				break;

			/* {6} The output is filtered through a telephone wire. */
			case 'p':
				/* Only one occurrence of the option is permitted. */
				if(DoPhoneFilter)
				{
					fprintf(stderr,"the phone-line simulation option "
							"was already set.\n");
					return(1);
				}

				/* The output will be filtered through a band-pass
				 * filter simulating a phone line transmission.
				 */
				DoPhoneFilter=1;
				break;

			/* Print usage guide for invalid options. */
			case '?':
			default:
				PrintUsage();
				return(1);
		}
#else /* HAVE_GETOPT */ /* This other version is for non-Unix systems. */
	/* Scan all command-line arguments. */
	for(i=1;i<argc;i++)
	{
		/* Set the amplification factor if the current argument looks
		 * like a number. Look at the comment of the case marked {5}
		 * in the Unix section for details.
		 */
		if(*argv[i]=='+' || *argv[i]=='-' || isdigit(*argv[i]))
		{
			if(Amp!=MAD_F_ONE)
			{
				fprintf(stderr,"the amplification/attenuation factor "
						"was set several times.\n");
				return(1);
			}

			AmpFactor=pow(10.,atof(argv[i])/20);
			if(AmpFactor>mad_f_todouble(MAD_F_MAX))
			{
				fprintf(stderr,"amplification out of range.\n");
				return(1);
			}

			Amp=mad_f_tofixed(AmpFactor);
		}
		else
			/* Use the phone-like filter if the argument is the *
			 * 'phone' string. Look at the comment of the case marked
			 * {6} in the Unix section for details.
			 */
			if(strcmp(argv[i],"phone")==0)
			{
				if(DoPhoneFilter)
				{
					fprintf(stderr,"the phone-line simulation option "
							"was already set.\n");
					return(1);
				}
				DoPhoneFilter=1;
			}
			else
			{
				/* The argument is not a recognized one. Print the
				 * usage guidelines and stop there.
				 */
				PrintUsage();
				return(1);
			}
	}
#endif /* HAVE_GETOPT */

	/* Initialize the subband-domain filter coefficients to one if
	 * filtering is requested.
	 */
	if(Amp!=MAD_F_ONE || DoPhoneFilter)
		for(i=0;i<32;i++)
			Filter[i]=MAD_F_ONE;

	/* The amplification/attenuation is applied to the subband-domain
     * filter definition.
	 */
	if(Amp!=MAD_F_ONE)
	{
		DoFilter=1;
		for(i=0;i<32;i++)
			Filter[i]=Amp;
	}

	/* The telephone-like filter is applied to the subband-domain
	 * filter definition. All subbands are set to zero except bands 2
	 * to 6. This programs author has no access to the MPEG audio
	 * specification, he does not know the frequencies bands covered
	 * by the MPEG subbands.
	 */
	if(DoPhoneFilter)
	{
		DoFilter=1;
		Filter[0]=MAD_F(0);
		for(i=5;i<32;i++)
			Filter[i]=MAD_F(0);
	}

	/* Command-line arguments are okay. */
	return(0);
}

/****************************************************************************
 * Program entry point.														*
 ****************************************************************************/
#if 0
int main(int argc, char *argv[])
{
	char	*cptr;
	int		Status;
	int     size;
	FILE *filein,*fileout;
	char    inbuf[4096];
	char    outbuf[40960];
	unsigned int framesize;

	/* Keep this for error messages. */
	cptr=strrchr(argv[0],'/');
	if(cptr==NULL)
		ProgName=argv[0];
	else
		ProgName=cptr+1;

	/* The command-line arguments are analyzed. */
	if(ParseArgs(argc,argv))
		return(1);

	/* Decode stdin to stdout. */
	filein = fopen("e:\\mp3_mad\\1.mp3","rb");
	fileout = fopen("e:\\mp3_mad\\1_.raw","wb");

	MpegAudioInit();
	size = fread(inbuf,1,4096,filein);
	Status = MpegAudioFindHead(inbuf,size,&framesize);
	do
	{
		memcpy(inbuf,inbuf+Status,4096-Status);
		size = fread(inbuf+4096-Status,1,Status,filein);
		if (size <= 0)
		{
			break;
		}
		Status=MpegAudioDecoder(outbuf,inbuf,4096);
		if (Status == -1)
		{
			Status = 4096;
		}
		
		fwrite(outbuf,1,2*1152*sizeof(unsigned short),fileout);
	}while(1);
	MpegAudioClose();
		
	/* Accounting report if no error occurred. */
	if(!Status)
	{
		char	Buffer[80];

		fprintf(stderr,"an error occurred during decoding.\n");

		mad_timer_string(gTimer,Buffer,"%lu:%02lu.%03u",
						 MAD_UNITS_MINUTES,MAD_UNITS_MILLISECONDS,0);
		fprintf(stderr,"%lu frames decoded (%s).\n",gFrameCount,Buffer);
	}

	/* All done. */
	return(Status);
}
#endif
/*  LocalWords:  BUFLEN HTAB madlld libmad bstdfile getopt subband ParseArgs JS
 */
/*  LocalWords:  DoFilter subbands errorstr bitrate scalefactor libmad's lu kb
 */
/*  LocalWords:  SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF FRACBITS madplay fread synth
 */
/*  LocalWords:  ApplyFilter strftime fracunits atten tSets tRequest tThe tas
 */
/*  LocalWords:  ttransmitted unix todouble tofixed
 */
/*
 * Local Variables:
 * tab-width: 4
 * End:
 */

/****************************************************************************
 * End of file madlld.c														*
 ****************************************************************************/
