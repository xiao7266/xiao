/*
* The contents of this file are subject to the Mozilla Public
* License Version 1.1 (the "License"); you may not use this file
* except in compliance with the License. You may obtain a copy of
* the License at http://www.mozilla.org/MPL/
* 
* Software distributed under the License is distributed on an "AS
* IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
* implied. See the License for the specific language governing
* rights and limitations under the License.
* 
* The Original Code is MPEG4IP.
* 
* The Initial Developer of the Original Code is Cisco Systems Inc.
* Portions created by Cisco Systems Inc. are
* Copyright (C) Cisco Systems Inc. 2000, 2001.  All Rights Reserved.
* 
* Contributor(s): 
*              Bill May        wmay@cisco.com
*/
/*
* msg_queue.cpp - generic class to send/receive messages.  Uses SDL mutexs
* to protect queues
*/
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "wrt_msg_queue.h"
#include "zenmalloc.h"


/*****************************************************************************
* CMsg class methods.  Defines information about a single message
*****************************************************************************/
CWRTMsg::CWRTMsg (uint32_t value, const void *msg, uint32_t msg_len, uint32_t param)
{
        m_value = value;
        m_msg_len = 0;
        m_has_param = (param != 0);
        m_param = param;
        m_next = NULL;

        if (msg_len) {
                void *temp = ZENMALLOC(msg_len);
                if (temp) {
                        memcpy(temp, msg, msg_len);
                        m_msg_len = msg_len;
                }else{
                        printf("ZENMALLOC failed  %s %d\n",__FILE__,__LINE__);
                        temp = NULL;
                }
                m_msg = temp;
        } else {
                m_msg = msg;
        }
}

int CWRTMsg::init(uint32_t value, const void *msg, uint32_t msg_len, uint32_t param){
        m_value = value;
        m_msg_len = 0;
        m_has_param = (param != 0);
        m_param = param;
        m_next = NULL;

        if (msg_len) {
                void *temp = ZENMALLOC(msg_len);
                if (temp) {
                        memcpy(temp, msg, msg_len);
                        m_msg_len = msg_len;
                }else{
                        m_msg = NULL;
                        m_msg_len = 0;
                        printf("ZENMALLOC failed  %s %d\n",__FILE__,__LINE__);
                        return 0;
                }
                m_msg = temp;
        } else {
                m_msg = msg;
        }
        return 1;
}

CWRTMsg::CWRTMsg (uint32_t value, uint32_t param)
{
        m_value = value;
        m_msg_len = 0;
        m_has_param = 1;
        m_param = param;
        m_next = NULL;
}

void CWRTMsg::init2(uint32_t value, uint32_t param){
        m_value = value;
        m_msg_len = 0;
        m_has_param = 1;
        m_param = param;
        m_next = NULL;    
}



CWRTMsg::~CWRTMsg (void) 
{
        if (m_msg_len) {
                if(m_msg != NULL)
                        ZENFREE((void *)m_msg);
                m_msg = NULL;
                m_msg_len = 0;
        }
}
void  CWRTMsg::free_message(){
        if (m_msg_len) {
                if(m_msg != NULL){
                        ZENFREE((void *)m_msg);
                }
                m_msg = NULL;
                m_msg_len = 0;
        }    
}

const void *CWRTMsg::get_message (uint32_t &len)
{
        len = m_msg_len;
        return (m_msg);
}

/*****************************************************************************
* CMsgQueue class methods.  Defines information about a message queue
*****************************************************************************/
CWRTMsgQueue::CWRTMsgQueue(void)
{
        m_msg_queue = NULL;
        m_msg_queue_mutex = SDL_CreateMutex();
}

CWRTMsgQueue::~CWRTMsgQueue (void) 
{
        CWRTMsg *p;
        SDL_LockMutex(m_msg_queue_mutex);
        while (m_msg_queue != NULL) {
                p = m_msg_queue->get_next();
                ZENFREE(m_msg_queue);
                m_msg_queue = p;
        }
        SDL_UnlockMutex(m_msg_queue_mutex);
        SDL_DestroyMutex(m_msg_queue_mutex);
        m_msg_queue_mutex = NULL;
}

int CWRTMsgQueue::send_message (uint32_t msgval, 
                                const void *msg, 
                                uint32_t msg_len, 
                                SDL_sem *sem,
                                uint32_t param)
{

        CWRTMsg *newmsg = (CWRTMsg *)ZENMALLOC(sizeof(CWRTMsg));
        if (newmsg == NULL){ 
                printf("CWRTMsgQueue::zen malloc failed  used malloc = %d\n",zenMemUninit());
                return (-1);
        }
        if(newmsg->init(msgval, msg, msg_len, param) == 0){
                ZENFREE(newmsg);
                return (-1);
        }

//printf("send message 1\r\n");
        return (send_message(newmsg, sem));
}

int CWRTMsgQueue::send_message (uint32_t msgval, uint32_t param, SDL_sem *sem)
{
        CWRTMsg *newmsg = (CWRTMsg *)ZENMALLOC(sizeof(CWRTMsg));
        if (newmsg == NULL) return -1;	
        newmsg->init2(msgval, param);
		//printf("send message 2\r\n");

        return (send_message(newmsg, sem));
}

int CWRTMsgQueue::send_message(CWRTMsg *newmsg, SDL_sem *sem)
{
        SDL_LockMutex(m_msg_queue_mutex);
        if (m_msg_queue == NULL) {
                m_msg_queue = newmsg;
        } else {
                CWRTMsg *p = m_msg_queue;
                while (p->get_next() != NULL) p = p->get_next();
                p->set_next(newmsg);
        }
        SDL_UnlockMutex(m_msg_queue_mutex);
		//printf("send message 3\r\n");
        if (sem != NULL) {
                SDL_SemPost(sem);
        }
        return (0);
}

CWRTMsg *CWRTMsgQueue::get_message (void) 
{
        CWRTMsg *ret;

        if (m_msg_queue == NULL) 
                return(NULL);

        SDL_LockMutex(m_msg_queue_mutex);
        if (m_msg_queue == NULL) 
                ret = NULL;
        else {
                ret = m_msg_queue;
                m_msg_queue = ret->get_next();
        }
        SDL_UnlockMutex(m_msg_queue_mutex);
        if (ret) {
                ret->set_next(NULL);
        }
        return (ret);
}

void CWRTMsgQueue::release(){
 	CWRTMsg* pMsg = NULL;
	while(1){
		pMsg = get_message();
		if(pMsg){
			pMsg->free_message();
			ZENFREE(pMsg);
			pMsg = NULL;
		}else
			break;
	}
}

/* end file msg_queue.cpp */
