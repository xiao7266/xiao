#ifndef __VIRTUAL_DEVICE_H
#define __VIRTUAL_DEVICE_H



/* Use 'k' as magic number */
#define HX280ENC_IOC_MAGIC  'k'
/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": G and S atomically
 * H means "sHift": T and Q atomically
 */
 /*
  * #define HX280ENC_IOCGBUFBUSADDRESS _IOR(HX280ENC_IOC_MAGIC,  1, unsigned long *)
  * #define HX280ENC_IOCGBUFSIZE       _IOR(HX280ENC_IOC_MAGIC,  2, unsigned int *)
  */
#define HX280ENC_STREAM_HEAD_SIZE 252
typedef struct  _hx280_enc_stream_head {
	unsigned int len;
	unsigned char data[HX280ENC_STREAM_HEAD_SIZE];
}hx280_enc_stream_head;

#define HX280ENC_AUDIO_HEAD_SIZE 252
typedef struct  _hx280_aud_stream_head {
	unsigned int len;
	unsigned char data[HX280ENC_AUDIO_HEAD_SIZE];
}hx280_aud_stream_head;

#define HX280ENC_IOCGHWOFFSET      _IOR(HX280ENC_IOC_MAGIC,  3, unsigned long *)
#define HX280ENC_IOCGHWIOSIZE      _IOR(HX280ENC_IOC_MAGIC,  4, unsigned int *)
#define HX280ENC_VIRT_TO_PHYS      _IOWR(HX280ENC_IOC_MAGIC, 9 , unsigned long)
#define HX280ENC_SET_STREAM_HEAD   _IOW(HX280ENC_IOC_MAGIC, 10 , hx280_enc_stream_head)
#define HX280ENC_SET_KEY_FRAME     _IO(HX280ENC_IOC_MAGIC, 11)
#define HX280ENC_SET_AUDIO_HEAD    _IOW(HX280ENC_IOC_MAGIC, 12 , hx280_aud_stream_head)













#endif

