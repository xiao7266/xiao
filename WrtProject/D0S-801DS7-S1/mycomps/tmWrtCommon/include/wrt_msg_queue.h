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
* msg_queue.h - provides a basic, SDL based message passing routine
*/
#ifndef __WRTMSG_QUEUE_H__
#define __WRTMSG_QUEUE_H__ 

#include "SDL.h"
#include "SDL_thread.h"

#ifdef  __cplusplus
extern "C"
{
#endif

class CWRTMsg {
public:
        CWRTMsg(uint32_t value, const void *msg = NULL, uint32_t msg_len = 0,
                uint32_t param = 0);
        CWRTMsg(uint32_t value, uint32_t param);
        ~CWRTMsg(void);
        int init(uint32_t value, const void *msg = NULL, uint32_t msg_len = 0,
                uint32_t param = 0);
        void init2(uint32_t value, uint32_t param);
        const void *get_message(uint32_t &len);
        void        free_message();
        CWRTMsg *get_next(void) { return m_next; };
        void set_next (CWRTMsg *next) { m_next = next; };
        uint32_t get_value(void) { return m_value;};
        int has_param(void) { return m_has_param; };
        uint32_t get_param (void) { return m_param; };
private:
        CWRTMsg *m_next;
        uint32_t m_value;
        int m_has_param;
        uint32_t m_param;
        uint32_t m_msg_len;
        const void *m_msg;
};

class CWRTMsgQueue {
public:
        CWRTMsgQueue(void);
        ~CWRTMsgQueue(void);
        int send_message(uint32_t msgval,
                const void *msg = NULL,
                uint32_t msg_len = 0,
                SDL_sem *sem = NULL,
                uint32_t param = 0);
        int send_message(uint32_t msgval,
                uint32_t param, 
                SDL_sem *sem = NULL);
        int send_message(uint32_t msgval, SDL_sem *sem) {
			
			printf("send message 4\r\n");
                return send_message(msgval, NULL, 0, sem);
        };
        CWRTMsg *get_message(void);
        void *show_message(void) { return m_msg_queue; };
        
        void release();
private:
        int send_message(CWRTMsg *msg, SDL_sem *sem);
        CWRTMsg *m_msg_queue;
        SDL_mutex *m_msg_queue_mutex;
};

#ifdef  __cplusplus
}
#endif


#endif
