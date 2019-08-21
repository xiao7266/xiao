#ifndef _MAIN_PROGRAM_H_
#define _MAIN_PROGRAM_H_
#include "SDL_thread.h"
#include "SDL_mutex.h"
#include "wrt_msg_queue.h"
#include "menuswitch.h"
#include "ctrlcmd.h"
#include <string.h>

class CMainProcess{
public:
        CMainProcess() {

                m_myThread = NULL;
                m_myMsgQueueSemaphore = NULL;
                m_stop_thread = false;
                m_bstart = false;
                m_isscreenadjust = 1;
/*
                m_myFlashThread = NULL;
                m_myFlashMsgQueueSemaphore = NULL;
*/
                m_isdisplayrec = false;

        }

        static int  StartThreadCallback(void* data) {//data ->this
                ((CMainProcess*)data)->ThreadMain();
                return 0;
        }

        static int StartFlashThreadCallback(void* data){
                ((CMainProcess*)data)->FlashThreadMain();
                return 0;
        }


        int IsStart(){
                return m_bstart;
        }

        void StartThread() {

            if (m_myThread == NULL) {

                    m_myMsgQueueSemaphore = SDL_CreateSemaphore(0);
                    m_myThread = SDL_CreateThread(StartThreadCallback, this); // modify by ljw 2007.10.10
            }

        }

        void StopThread() {
                m_stop_thread = true;

                if (m_myThread) {
                        m_myMsgQueue.send_message(MSG_NODE_STOP_THREAD,
                                m_myMsgQueueSemaphore);
                        SDL_WaitThread(m_myThread, NULL);
                        m_myThread = NULL;
                        SDL_DestroySemaphore(m_myMsgQueueSemaphore);
                        m_myMsgQueueSemaphore = NULL;
                }

        }

        void Send_msg(void* buf,int len){
                m_myMsgQueue.send_message(MSG_NODE_STATUS,
                        buf,len,
                        m_myMsgQueueSemaphore);
        }


        void Send_key(unsigned long key[4])
        {
                m_myMsgQueue.send_message(MSG_KEY,
                        key,sizeof(unsigned long)*4,
                        m_myMsgQueueSemaphore);

        }

        void Send_Pos(long pos[3]){
                m_myMsgQueue.send_message(MSG_NODE_GUI_POS,
                        pos,sizeof(long)*3,
                        m_myMsgQueueSemaphore);

        }
        void ViewSceenKeep(int index){

                m_myMsgQueue.send_message(MSG_NODE_VIEW_SCEENKEEP,
                        &index,sizeof(int),
                        m_myMsgQueueSemaphore);

        }

        void NoitfyReDraw(long windowtype){
                m_myMsgQueue.send_message(MSG_NODE_NOTIFY_REDRAW,
                        &windowtype,sizeof(long),
                        m_myMsgQueueSemaphore);

        }

        void Start(void) {
                m_myMsgQueue.send_message(MSG_NODE_START,
                        m_myMsgQueueSemaphore);

        }

        void Stop(void) {
                m_myMsgQueue.send_message(MSG_NODE_STOP,
                        m_myMsgQueueSemaphore);

        }
        inline void SetIsAdjust(int isadjust){
                m_isscreenadjust = isadjust;
        }
        virtual ~CMainProcess() {
                StopThread();
                UnInit();
        }
        static void disp_callback(unsigned long event[4]);
protected:

        void StartFlashThread() {
#if 0        	
                if (m_myFlashThread == NULL) {
                        int threapri = MAINTHREADPRI;
                        m_myFlashMsgQueueSemaphore = SDL_CreateSemaphore(0);
                        m_myFlashThread = SDL_CreateThread(StartFlashThreadCallback, this,"FSAT"); // modify by ljw 2007.10.10
                }
#endif
        }

        void StopFlashThread() {
#if 0        	
                m_stop_thread = true;
                if (m_myFlashThread) {
                        m_myFlashMsgQueue.send_message(MSG_NODE_STOP_THREAD,
                                m_myFlashMsgQueueSemaphore);
                        SDL_WaitThread(m_myFlashThread, NULL);
                        m_myFlashThread = NULL;
                        SDL_DestroySemaphore(m_myFlashMsgQueueSemaphore);
                        m_myFlashMsgQueueSemaphore = NULL;
                }
#endif                
        }

protected:
        static const uint32_t MSG_NODE			   			= 3024;
        static const uint32_t MSG_NODE_START		   	 	= MSG_NODE + 1;
        static const uint32_t MSG_NODE_STOP		    		= MSG_NODE + 2;
        static const uint32_t MSG_NODE_STOP_THREAD	    	= MSG_NODE + 3;
        static const uint32_t MSG_NODE_STATUS               = MSG_NODE + 4;
        static const uint32_t MSG_NODE_GUI_POS              = MSG_NODE + 5;
        static const uint32_t MSG_NODE_NOTIFY_REDRAW        = MSG_NODE + 6;
        static const uint32_t MSG_NODE_VIEW_SCEENKEEP       = MSG_NODE + 7;
        static const uint32_t MSG_KEY                       = MSG_NODE + 8;
        int ThreadMain(void);
        int FlashThreadMain(void);
protected:
        void Init();
        void ProcessEvent(unsigned long event[4]);
protected:
	
	void UnInit();

protected:

        SDL_Thread*			m_myThread;
        CWRTMsgQueue		m_myMsgQueue;//g_MainProcess µÄ¶ÓÁÐ
        SDL_sem*			m_myMsgQueueSemaphore;

        volatile bool       m_stop_thread;
        bool                m_bstart;
/*
        SDL_Thread*         m_myFlashThread;
        CWRTMsgQueue        m_myFlashMsgQueue;
        SDL_sem*            m_myFlashMsgQueueSemaphore;
*/

        int                  m_isscreenadjust;
        bool                  m_isdisplayrec;


};
#endif //_MAIN_PROGRAM_H_
