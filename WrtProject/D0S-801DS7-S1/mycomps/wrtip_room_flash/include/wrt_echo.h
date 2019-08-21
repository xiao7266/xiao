#ifndef WRT_ECHO_H_
#define WRT_ECHO_H_

#ifdef __cplusplus
extern "C"{
#endif	

void wrt_echo_init(int framesize,int rate,int pttime);

void wrt_echo_uninit();

void wrt_echo_playback(char* buffer);

void wrt_echo_capture(char* buffer, char *out);

#ifdef __cplusplus
}
#endif

#endif