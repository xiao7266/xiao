#ifndef SNDTOOLS_H
#define SNDTOOLS_H




#ifdef __cplusplus
extern "C"
{
#endif

#define AUDIORECORDDEVICE  1
#define AUDIOPLAYDEVICE 2

#define FMT8BITS 8 
#define FMT16BITS 16

#define FMT8K 8000
#define FMT16K 16000
#define FMT22K 22000
#define FMT44K 44000

#define MONO 0
#define STERO 1

#define AUIDOBLKSIZE 512


//Open sound device, return 1 if open success
//else return 0
int OpenSnd(int nWhich);     //1 record  2 play

//Close sound device
int CloseSnd(int nWhich);

int SoundUmount(void);

//Set record or playback format, return 1 if success
//else return 0
int SetFormat(int nWhich, int bits, int hz, int chn);

//Record
int RecordAudio(char *buf, int size);

//Playback
int PlayAudio(char *buf, int size);


#ifdef __cplusplus
}
#endif

#endif //ifndef SNDTOOLS_H
