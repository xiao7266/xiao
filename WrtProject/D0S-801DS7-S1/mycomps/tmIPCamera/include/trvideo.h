#ifndef __TRVIDEO__H
#define __TRVIDEO__H

#ifdef __cplusplus
extern"C"
{
#endif
//int Wrt_set_monitor_camera_ip(unsigned long ip);
int Wrt_set_monitor_camera_ip(char* onvif_server,bool onvif_auth,char* user_name,char* user_password,char* save_rtsp_url);
void Wrt_get_resolution(unsigned long *enc_w,unsigned long *enc_h);
void Wrt_start_streaming();
void Wrt_stop_streaming();
#ifdef __cplusplus
}
#endif

#endif
