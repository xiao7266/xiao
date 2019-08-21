#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zenmalloc.h"
#include "tmSysConfig.h"
#include "wrt_msg_queue.h"
#include "idtoiptable.h"

#define MSG_ID 0x03


int main()
{
	unsigned char* pMsg = NULL;
	CWRTMsg* pmsg = NULL;
	char* envpath = NULL;
	unsigned long ip[4] ={0};
	int num = 4;
	int i =0;
	SDL_sem* m_myMsgQueueSemaphore;
	CWRTMsgQueue  m_wrtmsgqueue;
	SDL_Init(0);
	zenMemInit(0);
	InitSystemInfo();
	init_idtoip();
	
	pMsg = (unsigned char*)ZENMALLOC(10);
	memset(pMsg,0,10);
	memcpy(pMsg,"123456789",9);
	m_myMsgQueueSemaphore = SDL_CreateSemaphore(0); 
	m_wrtmsgqueue.send_message(MSG_ID,pMsg,10,m_myMsgQueueSemaphore);
	envpath = getenv("LD_LIBRARY");
	printf("send msg ok envpath = %s\n",envpath);
	SDL_Delay(1000); 
	
	SDL_SemWait(m_myMsgQueueSemaphore);
	pmsg = m_wrtmsgqueue.get_message();
	if(pmsg->get_value() == MSG_ID)
	{
		uint32_t len = 0;
		char* p = (char*)pmsg->get_message(len);
		printf("%s\n",p);
	}
	pmsg->free_message();
	ZENFREE(pmsg);
	
	printf(" ≤‚ ‘µÿ÷∑±Ì\n");
	
	get_ipbyid("010010100100101",ip,&num);
	for(i = 0; i<num;i++)
	   printf("010010100100101 ip[%d] = 0x%08x\n",i,ip[i]);
	
	ip[0] = ip[1] = ip[2] = ip[3] = 0   ;
	get_ipbyid("010010100100201",ip,&num);
	for(i = 0; i<num;i++)
	   printf(" 010010100100201 ip[%d] = 0x%08x\n",i,ip[i]);	
	
	ip[0] = ip[1] = ip[2] = ip[3] = 0   ;
	get_ipbyid("010010100100301",ip,&num);
	for(i = 0; i<num;i++)
	   printf(" 010010100100301 ip[%d] = 0x%08x\n",i,ip[i]);		
	
			
	printf("ok \n");
	uninit_idtoip();
	UninitSystemInfo();
	zenMemUninit();
	SDL_Quit();
	return 0;	
}


