#ifndef WRT_AUDIO_H_
#define WRT_AUDIO_H_	

#define TYPE_CAPTURE	0x1
#define TYPE_PLAY	0x2	



#ifdef __cplusplus
extern "C" {
#endif
	//channel and samples and bits is 0,set audio param for default value.
	int OpenAudio(int type,unsigned int channel,unsigned int samples_rate,unsigned int bits,int framesize);
	void PlayAudio(unsigned char* buffer,int len);
	void StopAudio(int type);
	int GetAudio(unsigned char* buf,int *len);
	
	void SetAlsaVolume(int type,int level);
	int  GetAlsaVolume(int type);
	void open_alsa_dev();
	
#ifdef __cplusplus
}
#endif	
	
#endif	
	