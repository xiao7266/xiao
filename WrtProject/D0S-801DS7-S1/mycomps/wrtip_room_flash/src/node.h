#ifndef _NODE_H_
#define _NODE_H_

#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "wrt_msg_queue.h"


//#include "zenmalloc.h"
#include "ctrlcmd.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define THREADPRI   35



#define  UNKNOWN     0
#define CMD_HANDLER  1
#define NET_HANDLER  2
#define MSG_NODE_NEW_SOCKET_CLIENT    1024 + 16;
#define MSG_NODE_NEW_CLIENT           1024 + 17;
#define MSG_NODE_CHECK_RECV           1024 + 18;
#define MSG_NODE_RECV_UDP_MSG         1024 + 21;
#define  MSG_PROCESS_UDP_MSG           1024 + 22;

class CNode{
public:
        CNode() {
#if HAVE_USED_SDL         
                m_myThread = NULL;
                m_myMsgQueueSemaphore = NULL;
                m_stop_thread = false;    

#endif

        }
#if HAVE_USED_SDL
        static int  StartThreadCallback(void* data) {
                CNode* pData = (CNode*)data;
                   printf("  ((CNode*)pData)->ThreadMain();j\n");
                ((CNode*)pData)->ThreadMain();
                return 0;
        }

#endif  

        void StartThread() {
#if HAVE_USED_SDL         
                if (m_myThread == NULL) {
                        int threapri = THREADPRI;
                        m_myMsgQueueSemaphore = SDL_CreateSemaphore(0); 
                        printf("SDL CreateThread StartThreadCallback \n");
                        m_myThread = SDL_CreateThread(StartThreadCallback, this); // modify by ljw 2007.10.10
                }
             
#endif                  
        }


        void StopThread() {
                m_stop_thread = true;
#if HAVE_USED_SDL   
                if (m_myThread) {
                        m_myMsgQueue.send_message(MSG_NODE_STOP_THREAD,
                                m_myMsgQueueSemaphore);
                        SDL_WaitThread(m_myThread, NULL);
                        m_myThread = NULL;
                        SDL_DestroySemaphore(m_myMsgQueueSemaphore);
                        m_myMsgQueueSemaphore = NULL;
                }
#endif    

        }
        int Send_msg(int type ,void* buf,int len){
#if HAVE_USED_SDL     
                return  m_myMsgQueue.send_message(type,
                        buf,len,
                        m_myMsgQueueSemaphore);

#endif            
                return 0;                        
        }

        void Start(void) {
#if HAVE_USED_SDL   
                m_myMsgQueue.send_message(MSG_NODE_START,
                        m_myMsgQueueSemaphore);
       
#endif
        }

        void Stop(void) {
#if HAVE_USED_SDL         
                m_myMsgQueue.send_message(MSG_NODE_STOP,
                        m_myMsgQueueSemaphore);

#endif              
        }
        virtual ~CNode() {
                StopThread();
        }

protected:
        static const uint32_t MSG_NODE          = 1024;
        static const uint32_t MSG_NODE_START        = MSG_NODE + 1;
        static const uint32_t MSG_NODE_STOP       = MSG_NODE + 2;
        static const uint32_t MSG_NODE_STOP_THREAD      = MSG_NODE + 3;
        static const uint32_t MSG_NODE_SEND_MSG             = MSG_NODE + 4;
        static const uint32_t MSG_NODE_SOCKET_CLOSE         = MSG_NODE + 5;
        static const uint32_t MSG_NODE_CMD                  = MSG_NODE + 7;
        static const uint32_t MSG_NODE_REQ_CONNECT          = MSG_NODE + 8;
        static const uint32_t MSG_NODE_CONNECT_OK           = MSG_NODE + 9;
        static const uint32_t MSG_NODE_NET_EXCEPTION        = MSG_NODE + 10;
        static const uint32_t MSG_NODE_RECV_MSG             = MSG_NODE + 11;
        static const uint32_t MSG_NODE_ROOM_WARING          = MSG_NODE + 12;
        static const uint32_t MSG_NODE_SEND_SYSINFO         = MSG_NODE + 13;
        static const uint32_t MSG_NODE_EMERG_MSG            = MSG_NODE + 14;
        static const uint32_t MSG_NODE_SOCKET_CUT           = MSG_NODE + 15;
        static const uint32_t MSG_NODE_SMALLDOOR_CMD        = MSG_NODE + 19;
        static const uint32_t MSG_NODE_SEND_JIAJU_INFO      = MSG_NODE + 20;
        static const uint32_t MSG_NODE_PHONE                = MSG_NODE + 23;




        virtual int ThreadMain(void)=0;
        virtual int GetType(){
                return UNKNOWN;
        }
        virtual int IsInit(){
                return true;
        }

protected:
#if HAVE_USED_SDL
        SDL_Thread*     m_myThread;
        CWRTMsgQueue     m_myMsgQueue;
        SDL_sem*      m_myMsgQueueSemaphore;

#endif        
        volatile bool m_stop_thread;
};
#endif //_NODE_H_