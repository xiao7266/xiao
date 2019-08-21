#ifndef WRT_SUBROOM_H
#define WRT_SUBROOM_H



#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*SUBROOM_CALLBACK)(unsigned long t_msg[4]);

#define CENTER_CALL_SUBROOM              	0X11
#define SUBROOM_HOLDON                   	0X12
#define SUBROOM_HANGUP                   	0X13
#define CENTER_AND_DOOR_ROOM_HANGUP_SUBROOM     0X14
#define DOOR_CALL_SUBROOM                  	0x15
#define SUBROOM_SESSION_END                	0X16
#define SUBROOM_UNLOCK                     	0X17
#define CONNECT_SUBROOM                    	0x18
#define OTHERROOM_CALL_SUBROOM 		   	0x19
#define SUBROOM_AGENT				0x1A

#define MEDIA_NEGOTIATE_SUBROOM             0x20
#define MEDIA_NEGOTIATE_SUBROOM2            0x21

#define MAX_SUBROOM 4



void init_subroom();
void reinit_subroom_ip();
int send_msg_to_subroom(unsigned long t_msg[4]);

void set_subroom_callback(SUBROOM_CALLBACK  _callback);

int have_idle_subroom();
int have_connect_subroom();
unsigned long get_subroom_connect_ip(int index);
unsigned long get_subroom_ip_by_index(int index);

#ifdef __cplusplus
}
#endif




#endif