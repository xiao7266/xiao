#ifndef WRT_SUB_PHONE_H
#define WRT_SUB_PHONE_H



#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*PHONE_CALLBACK)(unsigned long t_msg[4]);

#define CENTER_CALL_PHONE                0X01
#define PHONE_HOLDON                     0X02
#define PHONE_HANGUP                     0X03
#define CENTER_AND_DOOR_ROOM_HANGUP      0X04
#define DOOR_CALL_PHONE                  0x05
#define PHONE_SESSION_END                0X06
#define PHONE_UNLOCK                     0X07
#define CONNECT_PHONE                    0x08

#define PHONE_MEDIA_INFO                 0x1290 //add by wyx 2014-3-25
#define PHONE_1CHNL_8K_16BIT			 0x01
#define PHONE_2CHNL_8K_16BIT			 0x02
#define PHONE_1CHNL_16K_16BIT			 0x03
#define PHONE_2CHNL_16K_16BIT			 0x04
#define PHONE_H264					 	 0x05
#define PHONE_MP4					 	 0x06

#define SET_DIVERT_PHONE_SD              0XFFFFFFF0
#define CLOSE_DIVERT_PHONE_SD            0XFFFFFFF1


void init_sub_phone();

int send_msg_to_sub_phone(unsigned long t_msg[4]);

void set_phone_callback(PHONE_CALLBACK  _callback);

int have_idle_phone();
int have_connect_phone();
unsigned long get_connect_ip(int index);


#ifdef __cplusplus
}
#endif




#endif