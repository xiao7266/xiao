#ifndef _MADPLAY_H_
#define _MADPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

int MpegAudioGetVersion();

int MpegAudioIsstereo();

int MpegAudioGetLayer();

int MpegAudioGetFrequency();

int MpegAudioFindHead(unsigned char *InputBuffer, int ReadSize, unsigned int *FramePtr);

void MpegAudioInit();

void MpegAudioClose();

int MpegAudioDecoder(unsigned char *OutBuffer,unsigned char *InputBuffer, int ReadSize,int *outputlen);

#ifdef __cplusplus
}
#endif

#endif /* _MADPLAY_H_ */
