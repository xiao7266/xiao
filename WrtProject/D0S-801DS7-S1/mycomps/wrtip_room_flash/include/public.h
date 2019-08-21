#include <assert.h>
#include "SDL.h"
#include "wrt_msg_queue.h"


#define MODULE_MSG      0x01
#define MODULE_EXIT     0x02

#ifdef __cplusplus
extern "C"{
#endif	

int parse_msg(CWRTMsg* pMsg,unsigned long* RMsg);
uint32_t tm_getticks(uint32_t* hi,uint32_t* li);
uint32_t tm_get(uint32_t* date,uint32_t *time1,uint32_t* ticks);
uint32_t tm_set(uint32_t date,uint32_t time1,uint32_t ticks);

void  add_and_del_timer(unsigned long evt);
void  add_and_del_timer2(unsigned long evt);

#ifdef __cplusplus
}
#endif	