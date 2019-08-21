#ifndef _SOCK_HANDLER_H_
#define _SOCK_HANDLER_H_

//#include <pna.h>

#include "node.h"
//#include "ctrlcmd.h"
#include "zenmalloc.h"

#define SOCK_MAX    16
/////////////////////////////////////////
void initSocketServer();
///////////////////////////////////////
class CCmdHandler;

class CSocketManager: public CNode {
public:
        CSocketManager():CNode() {
                for(int i =0; i< SOCK_MAX;i++){
                        m_socket[i] = -1;
                        m_ipaddr[i] = -1;
                        m_socketstarttime[i] = 0;
                }
                m_socket_max_index = -1;	
                m_sink = NULL;	
                m_recvbuffer = NULL;
                m_recvbuffer = (unsigned char*)ZENMALLOC(1024);
                memset(m_recvbuffer,'\0',1024);
                m_bIsListen = false;
                m_bIslink = false;
#if 1		
                FD_ZERO(&readfd);
                FD_ZERO(&writefd);
                max_fd = -1;
#endif		

        }
        ~CSocketManager() {
                for(int i =0; i< SOCK_MAX;i++){
                        m_socket[i] = -1;
                        m_ipaddr[i] = -1;
                        m_socketstarttime[i] = 0;
                }
                m_sink = NULL;
                m_socket_max_index = -1;
                if(m_recvbuffer)
                        ZENFREE(m_recvbuffer);
                m_recvbuffer = NULL;
                m_bIsListen = false;
        }

        void SetCmdSink(CCmdHandler* psink){
                if(m_sink == NULL && NULL != psink)
                        m_sink = psink;
        }
        void CloseSocket(int index){
#if HAVE_USED_SDL
               // WRT_DEBUG("send closesocket %d\n",index);
                if(index == -1)
                        return;
                m_myMsgQueue.send_message(MSG_NODE_SOCKET_CLOSE,
                        &index,sizeof(int),
                        m_myMsgQueueSemaphore);
#else				    
                unsigned long msg[2];
                msg[0] = MSG_NODE_SOCKET_CLOSE;
                msg[1] = index;
                q_vsend(m_myMsgID,(void*)&msg,8);	
#endif            			    
        }
        int IsInit(){
                return m_bIsListen;
        }

        void socket_printf();

        int get_max_fd(){
                return m_socket_max_index;
        }

        bool ModifyHostIpAndMask(unsigned long ipaddr,unsigned long netmask);
        bool ModifyHostGateWay(unsigned long gatewayip);

        unsigned long GetIPAddrByIndex(int index){
                if(index >=0 && index < 16)
                        return m_ipaddr[index];
                return 0;               
        }
        void ProcessRecv(SOCKET_PACKET* pPacket);   

        void ProcessRecv2(int index);

        void  SendResultToCenter(unsigned long ip,unsigned char* buf,int len);

        void send_msg_shr_socket(unsigned long tmsg[4])
        {
                m_myMsgQueue.send_message(MSG_NODE_PHONE,
                        tmsg,sizeof(unsigned long)*4,
                        m_myMsgQueueSemaphore);
        }

protected:
        int ThreadMain(void);
        void ProcessSend(SOCKET_PACKET* pPacket);
        void ProcessSend2(int index,unsigned char* buf,int buflen);

        int  GetIdleSocket();
        int GetType(){
                return NET_HANDLER;
        } 	
        void ProcessNewSocket1(unsigned long event[4]);
        void ProcessNewSocket2(unsigned long event[4]);
        void ProcessMulNewSocket2(unsigned long event[4]);
protected:
        int                 m_socket[SOCK_MAX];
        unsigned long       m_ipaddr[SOCK_MAX];
        unsigned long      	m_socketstarttime[SOCK_MAX];
        int                 m_socket_max_index;
        CCmdHandler*        m_sink;
        unsigned char*      m_recvbuffer;
        bool                m_bIsListen;
        bool                m_bIslink;

#if 1	
        fd_set              readfd;
        fd_set              writefd;
        int                 max_fd;
#endif    

};

#endif /* _SOCK_HANDLER_H_ */