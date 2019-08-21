/*******************************************************************
文件描述：
该文件实现了，SOCKET处理的封装。以及和CMD处理线程的消息交互
版本：   0.1
日期：   2007.10.25
作者：   李建文
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/if.h>
#include <linux/route.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "public.h"
#include "wrt_log.h"
#include "sock_handler.h"
#include "cmd_handler.h"
#include "audiovideo.h"
//#include "tmosal.h"  
#include "tmSysConfig.h"

#define closesocket close
#define ioctlsocket ioctl
#define SYS_LOG 1

typedef unsigned long UInt32 ;

#if SYS_LOG 
#include "info_list_handler.h"
#endif


#define DEBUG_TEST  1
#undef  DEBUG_TEST 

#define _LJW_TEST  1
#undef _LJW_TEST

#define LJW_DEBUG_LISTEN 1
#undef LJW_DEBUG_LISTEN

#define SOCKET_DEBUG2  1
#undef SOCKET_DEBUG2


#define SO_CONTIMEO       0x100A
#define CONNECT_EXCEPTION 0x600
#define SOCKET_CONNECT    0x01
#define WRT_TEST          0x02
extern CSocketManager* m_pSocketManager;
extern CCmdHandler*      m_pCmdHandler;
extern T_SYSTEMINFO*   pSystemInfo;

static unsigned long   g_listen_tid = -1;
static unsigned long   g_newclient_tid = -1;
//static  unsigned long       g_qid1 = -1;
static CWRTMsgQueue         g_socket_msg_queue;
static SDL_sem*             g_socket_msg_sem = NULL;
static unsigned long g_tmpipaddr[4] = {0,0,0,0};

#ifdef _LJW_TEST
static int g_debug_sock = 0;
#endif

static SOCKET_PACKET gpacket;



extern int parse_msg(CWRTMsg* pMsg,unsigned long* RMsg);

#ifdef LJW_DEBUG_LISTEN
static int g_listen_socket = 0;
static int StartListen();
#endif

static void debug_printf(int i)
{
#ifdef _LJW_TEST
        // if(g_debug_sock == 1){
        WRT_DEBUG("%d",i);
        //  }
#endif
}


int CSocketManager::ThreadMain(){    
        uint32_t len;
        unsigned long evt=-1;
        unsigned long msg[4];
        char   closebuf[10];
        const int TYPE1 = MSG_NODE_NEW_SOCKET_CLIENT;
        const int TYPE2 = MSG_NODE_NEW_CLIENT;
        const int TYPE3 = MSG_NODE_CHECK_RECV;
        //t_ident("tnet",0,&socketmanager_task_id);
        // socketmanager_task_id = SDL_GetThreadID(m_myThread);
        WRT_MESSAGE("网络任务的ID以及消息信号量 =  %x",m_myMsgQueueSemaphore);
        int rc=0;
        while (1) {
                debug_printf(1);
#if HAVE_USED_SDL    
                if (m_bIsListen && (m_bIslink == false)) {
#ifdef LJW_DEBUG_LISTEN
                        rc =SDL_SemTryWait(m_myMsgQueueSemaphore); //SDL_SemWaitTimeout(m_myMsgQueueSemaphore,100);
#else
                        rc =SDL_SemWait(m_myMsgQueueSemaphore); //SDL_SemWaitTimeout(m_myMsgQueueSemaphore,100);
#endif
                } else if(m_bIsListen && (m_bIslink == true)){

                        rc = SDL_SemTryWait(m_myMsgQueueSemaphore);
                }else{
                        rc =SDL_SemWait(m_myMsgQueueSemaphore);
                }    
                if (rc == -1) {
                        WRT_DEBUG("网络模块消息semaphore error %d",errno);
                        break;
                } 
                if (rc == 0) {
                        CWRTMsg* pMsg = m_myMsgQueue.get_message();
                        if (pMsg != NULL) {
                                switch (pMsg->get_value()) {

#endif                      
        case MSG_NODE_STOP_THREAD:
#if HAVE_USED_SDL    
                pMsg->free_message();      
                ZENFREE(pMsg);
#endif          
                return 0;
        case MSG_NODE_START:
                initSocketServer();
#ifdef LJW_DEBUG_LISTEN
                g_listen_socket = StartListen();
#endif
                m_bIsListen = true;
                break;
        case MSG_NODE_STOP :
                break;
        case TYPE1://有连接进来,产生的new socket
                {
                        memset((void*)msg,0,sizeof(unsigned long)*4);
#if HAVE_USED_SDL  
                        unsigned char* buf = (unsigned char*)(pMsg->get_message(len));  
                        if(buf == NULL)
                                break;          
                        memcpy((void*)msg,buf,sizeof(unsigned long)*4);         
#else   
                        memcpy((void*)msg,(unsigned char*)(msgbuf+4),sizeof(unsigned long)*4);  
#endif                               
                        ProcessNewSocket1(msg);
                }         
                break;
        case TYPE2://主动连接出去，产生的new socket
                {
                        memset((void*)msg,0,sizeof(unsigned long)*4);
#if HAVE_USED_SDL 
                        unsigned char* buf = (unsigned char*)(pMsg->get_message(len));  
                        if(buf == NULL)
                                break;              
                        memcpy((void*)msg,buf,sizeof(unsigned long)*4);
#else
                        memcpy((void*)msg,(unsigned char*)(msgbuf+4),sizeof(unsigned long)*4);
#endif                                     
                        if(msg[0] == -1){
                                if(m_pCmdHandler){
                                        //发送连接失败的消息
#if DEBUG_TEST                          
                                        CreateDialogWin("CONNECT_OK failed\n",0);
#endif                           
                                        int ok = -1;
                                   
                                        m_pCmdHandler->Send_msg(MSG_NODE_CONNECT_OK,&ok,sizeof(int));          
                                } 
                                if(msg[2] > 0){
                                        ZENFREE((void*)msg[1]);
                                        break;
                                } 
                                break;
                        }

                        if(msg[3] == 0)
                                ProcessNewSocket2(msg);                    
                        else{
                                ProcessMulNewSocket2(msg);
                        }
                }
                break;
        case MSG_NODE_REQ_CONNECT:   
                {
						printf("MSG_NODE_REQ_CONNECT\r\n");
                        unsigned long tmpmsg[4];
                        memset(tmpmsg,0,sizeof(unsigned long)*4);
#if HAVE_USED_SDL
                        unsigned char* buf = (unsigned char*)(pMsg->get_message(len));  
                        if(buf == NULL)
                                break;           
                        memcpy(tmpmsg,buf,4*sizeof(unsigned long)); 
#else
                        memcpy((void*)tmpmsg,(unsigned char*)(msgbuf+4),sizeof(unsigned long)*4);   
#endif           
                       
                        g_socket_msg_queue.send_message(MODULE_MSG,tmpmsg,sizeof(tmpmsg),g_socket_msg_sem);     
         
                }
                break;
        case MSG_NODE_PHONE:
                {
                	/*
#ifdef HAVE_SUB_PHONE
                        unsigned long tmsg[4];
                        memset(tmsg,0,sizeof(tmsg));
                        unsigned char* buf = (unsigned char*)(pMsg->get_message(len));
                        if(buf == NULL)
                                break;
                        memcpy(tmsg,buf,sizeof(tmsg));
                        int shr_sd = shr_socket(m_socket[tmsg[1]],tmsg[0]);
                        //将sd送到，指定的任务。

                        //关闭已有的链接
                        closesocket(m_socket[tmsg[1]]);
                        FD_CLR(m_socket[tmsg[1]],&readfd);
                        FD_CLR(m_socket[tmsg[1]],&writefd);
                        m_socket[tmsg[1]] = -1;
                        m_ipaddr[tmsg[1]] = -1; 
                        m_socketstarttime[tmsg[1]] = 0;

#endif
*/

                }
                break;
        case MSG_NODE_SOCKET_CLOSE: //收到CMD处理任务发送过来的关闭socket的命令
                {


#if HAVE_USED_SDL     
                        unsigned char* buf = (unsigned char*)(pMsg->get_message(len));  
                        if(buf == NULL)
                                break;            
                        int index = *(int*)buf;
#else         
                        int index = *(int*)(msgbuf+4);                
#endif       
                        WRT_MESSAGE("收到关闭socket消息 index= %d ,socket = %d",index ,  m_socket[index] );
                        if(index >= 0 && m_socket[index] != -1){
                                FD_CLR(m_socket[index],&readfd);
                                FD_CLR(m_socket[index],&writefd);
                                closesocket(m_socket[index]);              
                                m_socket[index] = -1;
                                m_ipaddr[index] = -1; 
                                m_socketstarttime[index] = 0;
                                WRT_MESSAGE("关闭socket index  = %d",index);

                        } 

                }
                break;
        case MSG_NODE_SEND_MSG: //表示CMD处理线程，需要SOCK线程发送消息  
                {   
#if HAVE_USED_SDL
                        SOCKET_PACKET* pPacket   = (SOCKET_PACKET*)pMsg->get_message(len);       
#else
                        SOCKET_PACKET* pPacket   = (SOCKET_PACKET*)(msgbuf+4);  
#endif                    
                        if(pPacket != NULL){               
                                ProcessSend(pPacket);     
                        }            
                }
                break;
        case MSG_NODE_SEND_JIAJU_INFO:
                {
                        unsigned long tmpmsg[4];
                        memset(tmpmsg,0,sizeof(unsigned long)*4);
#if HAVE_USED_SDL   
                        unsigned char* tbuf = (unsigned char*)(pMsg->get_message(len));  
                        if(tbuf == NULL)
                                break;          
                        memcpy(tmpmsg,(void*)tbuf,4*sizeof(unsigned long)); 
#else
                        memcpy((void*)tmpmsg,(unsigned char*)(msgbuf+4),sizeof(unsigned long)*4);   
#endif   
                        int temp_len = 0;
                        unsigned char* buf = (unsigned char*)tmpmsg[1];
                        if(m_socket[tmpmsg[0]] != -1){

                                ProcessSend2(tmpmsg[0],buf,tmpmsg[2]);
                                //temp_len = send(m_socket[tmpmsg[0]],(char*)buf,tmpmsg[2],0); 
                        }
                        ZENFREE(buf);   
                        buf = NULL;                                      
                }
                break;
        case MSG_NODE_SEND_SYSINFO:
                {
#if HAVE_USED_SDL                
                        SOCKET_PACKET* pPacket=(SOCKET_PACKET*)pMsg->get_message(len);
#else
                        SOCKET_PACKET* pPacket   = (SOCKET_PACKET*)(msgbuf+4);           
#endif           
                        if(pPacket != NULL){ 
                                int temp_len = 0;
                                unsigned char* buf = (unsigned char*)(int)pPacket->buf;
                                if(pPacket->sock_index >=0 && m_socket[pPacket->sock_index] != -1)
                                        ProcessSend2(pPacket->sock_index,buf,pPacket->validlen);                
                                ZENFREE(buf);   
                                buf = NULL;
                        } 
                }
                break; 
        case TYPE3://处理接收          
                ProcessRecv(NULL);  
                break;
        default:
                break;
                        }//switch
#if HAVE_USED_SDL    
                        pMsg->free_message();    
                        ZENFREE(pMsg);
                }//if(pMsg!=NULL)
#endif        
        }//if(rc == 0) 

        debug_printf(2);
        ProcessRecv(NULL);
        debug_printf(30);
        SDL_Delay(100); //防止连接满的时候，其他任务强不到时间。


}//while
return -1;
}

void CSocketManager::ProcessNewSocket1(unsigned long event[4]){

        int index = GetIdleSocket();
        //当已经接收到一个SOCKET以后,没有空闲的SOCKET pool返回一个忙的指令给呼叫者
        if(index != -1  && !m_pCmdHandler->isupdate() ){
                int flag = 1;
                int sockbuflen = 16*1024;
                uint32_t thi,tlo;
                m_socket[index] = event[0];
                m_ipaddr[index] = event[1];//htonl(dstaddr.sin_addr.s_addr);
                tm_getticks(&thi,&tlo);
                m_socketstarttime[index] =  tlo;

                ioctlsocket( m_socket[index],FIONBIO,(char*)&flag);
                setsockopt( m_socket[index], SOL_SOCKET, SO_SNDBUF, (char*)&sockbuflen, sizeof(int));
                setsockopt( m_socket[index], SOL_SOCKET, SO_RCVBUF, (char*)&sockbuflen, sizeof(int));    
#ifdef SOCKET_DEBUG2

                setsockopt(m_socket[index],SOL_SOCKET ,SO_REUSEADDR,(const char*)&flag,sizeof(int));
                flag = 0;
                setsockopt(m_socket[index],SOL_SOCKET,SO_LINGER,(const char*)&flag,sizeof(int));
#endif
                WRT_MESSAGE("被动创建了一个新的socket,index = %d 创建时间: %ld",index, m_socketstarttime[index] );       
                m_bIslink = true;                   
        }
        else
        {
                //返回忙的指令给该SOCKET，然后closesocket;
                char* buf = (char*)ZENMALLOC(40);
                int  len = 40;
                short cmd = 0x90;
                cmd = htons(cmd);
                memset(buf,0,40);
                strncpy(buf,"WRTI",4);
                memcpy(buf+4,&len,4);
                memcpy(buf+8,&cmd,2);
                memcpy(buf+10,pSystemInfo->LocalSetting.privateinfo.LocalID,15);
                //memcpy(buf+10,roomtest.LocalRoom.LocalID,15);
                send(event[0],buf,10,0);
                ZENFREE(buf);
                buf = NULL;
                closesocket(event[0]);
        }   
}

/**************************************************************************
**************************************************************************/
void CSocketManager::ProcessMulNewSocket2(unsigned long event[4]){
        int i=0;
        int index[4]={-1,-1,-1,-1};
        int tmpindex = 0;
        int tmpsocketindex= 0;
        int   sockbuflen = 16*1024;
        int flag = 1;
        unsigned long * psock = (unsigned long*)(unsigned char*)event[0];
        for(i =0; i < 4; i++){
                if((int)psock[i] == -1){ //如果其中某一路没有连接成功
                        WRT_DEBUG("第%d路网络不通",i);
                        continue;
                }
                index[i] = GetIdleSocket();
                if(index[i] == -1)
                        break;  
                tmpindex = index[i];     
                m_socket[tmpindex] = psock[i];
                m_ipaddr[tmpindex] = g_tmpipaddr[i];
                ioctlsocket( m_socket[tmpindex],FIONBIO,(char*)&flag);
                setsockopt( m_socket[tmpindex], SOL_SOCKET, SO_SNDBUF, (char*)&sockbuflen, sizeof(int));
                setsockopt( m_socket[tmpindex], SOL_SOCKET, SO_RCVBUF, (char*)&sockbuflen, sizeof(int));        
        }
        m_bIslink = true;
        if(m_pCmdHandler){
                tmpsocketindex  = 0;
                tmpsocketindex  = (index[0] & 0xff);
                tmpsocketindex |= (index[1] << 8) & 0xff00;
                tmpsocketindex |= (index[2] << 16) & 0xff0000;
                tmpsocketindex |= (index[3] << 24) & 0xff000000;        
                m_pCmdHandler->Send_msg(MSG_NODE_CONNECT_OK,&tmpsocketindex,sizeof(int));
        }
        if(event[2] > 0){
                for(i=0; i< 4;i++){
                        if(index[i] != -1){
                                SOCKET_PACKET   socketpack;
                                memset(&socketpack,0,sizeof(SOCKET_PACKET));
                                socketpack.sock_index = index[i];
                                socketpack.validlen = event[2];
                                memcpy(socketpack.buf,(void*)event[1],socketpack.validlen);                 
                                ProcessSend(&socketpack);  
                        }
                }
                ZENFREE((void*)event[1]);          
        }
        if(event[3] > 0)
                ZENFREE((void*)event[0]);


}
/**
*主动连接
*/
void CSocketManager::ProcessNewSocket2(unsigned long event[4]){
        int index = GetIdleSocket();
        if(index != -1){
                int flag = 1;
                int   sockbuflen = 16*1024;
                uint32_t thi,tlo;
                m_socket[index] = event[0];
                m_ipaddr[index] =  g_tmpipaddr[0];
                tm_getticks(&thi,&tlo);
                m_socketstarttime[index] = tlo;
                ioctlsocket( m_socket[index],FIONBIO,(char*)&flag);
                setsockopt( m_socket[index], SOL_SOCKET, SO_SNDBUF, (char*)&sockbuflen, sizeof(int));
                setsockopt( m_socket[index], SOL_SOCKET, SO_RCVBUF, (char*)&sockbuflen, sizeof(int));
				WRT_MESSAGE("主动创建了一个新的socket,index = %d 创建时间: %ld",index, m_socketstarttime[index] );    
#if 0      
                FD_SET(m_socket[index],&readfd);
                FD_SET(m_socket[index],&writefd);
                if(m_socket[index] > max_fd){
                        max_fd = m_socket[index];
                }      
#endif       
                m_bIslink = true;
                if(m_pCmdHandler)
                        m_pCmdHandler->Send_msg(MSG_NODE_CONNECT_OK,&index,sizeof(int));           
                if(event[2] > 0){
                        SOCKET_PACKET   socketpack;
                        memset(&socketpack,0,sizeof(SOCKET_PACKET));
                        socketpack.sock_index = index;
                        socketpack.validlen=event[2];
                        memcpy(socketpack.buf,(void*)event[1],socketpack.validlen);
                        ZENFREE((void*)event[1]);
                        ProcessSend(&socketpack);                     
                }

        }else{
                if(m_pCmdHandler){
                        //发送连接失败的消息
                        int ok = -1;
                        m_pCmdHandler->Send_msg(MSG_NODE_CONNECT_OK,&ok,sizeof(int));          
                }            
                closesocket(event[0]);     
        }  
}



int CSocketManager::GetIdleSocket(){
        for(int i= 0; i<SOCK_MAX;i++){
                if(m_socket[i] == -1){
                        m_socket_max_index = i;
                        return i;
                }
        }
        return -1;
}

void CSocketManager::ProcessSend2(int index,unsigned char* buf,int buflen){
        int ret;
        struct timeval timeout;
#if 1  
        FD_ZERO(&writefd);

        max_fd = -1;
        for(int i =0; i<SOCK_MAX;i++)
        {       
                if(m_socket[i] != -1 ){
                        FD_SET(m_socket[i],&writefd);         
                        if(m_socket[i] > max_fd)
                                max_fd = m_socket[i];
                }            
        }
#endif  

        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        ret = select(max_fd+1,NULL,&writefd,NULL,&timeout);
        if(ret <= 0){
                WRT_DEBUG("发送网络数据select timeout");
                return;
        }
        //处理需要发送的包
        if(index >= 0 &&m_socket[index] != -1){
                if(FD_ISSET(m_socket[index],&writefd) && buflen > 0){
                        int temp_len = send(m_socket[index],(char*)buf,buflen,0);

                        if(temp_len != buflen){
                                FD_CLR(m_socket[index],&writefd);
                                //可能需要发送消息给 cmd处理线程，告之网络故障               
                                if(m_pCmdHandler){
                                        int failed = index;            
                                        WRT_DEBUG("发送网络数据send错误 socket index=%d errno=0x%x",index,errno);
                                        m_pCmdHandler->Send_msg(MSG_NODE_NET_EXCEPTION,&failed ,sizeof(int));
                                }         
                        }
                }     
        } 	
}

void CSocketManager::ProcessSend(SOCKET_PACKET* pPacket){
        int ret;
        struct timeval timeout;
#if 1  
        FD_ZERO(&writefd);

        max_fd = -1;
        for(int i =0; i<SOCK_MAX;i++)
        {       
                if(m_socket[i] != -1 ){
                        FD_SET(m_socket[i],&writefd);         
                        if(m_socket[i] > max_fd)
                                max_fd = m_socket[i];
                }            
        }
#endif  

        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;
        ret = select(max_fd+1,NULL,&writefd,NULL,&timeout);
        if(ret <= 0){

                return;
        }
        //处理需要发送的包
        if(pPacket->sock_index > (-1)  &&( pPacket->sock_index < 16)&&m_socket[pPacket->sock_index] != -1){
                if(FD_ISSET(m_socket[pPacket->sock_index],&writefd) && pPacket->validlen > 0){

                        int temp_len = send(m_socket[pPacket->sock_index],pPacket->buf,pPacket->validlen,0);
                        if(temp_len != pPacket->validlen){
                                FD_CLR(m_socket[pPacket->sock_index],&writefd);
                                //   closesocket(m_socket[pPacket->sock_index]);
                                //  m_socket[pPacket->sock_index] = -1;
                                //   m_ipaddr[pPacket->sock_index] = -1;
                                //可能需要发送消息给 cmd处理线程，告之网络故障

                                if(m_pCmdHandler){
                                        int failed = pPacket->sock_index;
                                        WRT_DEBUG("发送网络数据send错误 socket index=%d errno=0x%x",failed,errno);
                                        m_pCmdHandler->Send_msg(MSG_NODE_NET_EXCEPTION,&failed ,sizeof(int));
                                }

                        }
                }     
        } 
}

void CSocketManager::ProcessRecv2(int index){

        struct timeval timeout; 
        int temp_len = 0;  
        int ret = 0;
        fd_set readfd1;
        FD_ZERO(&readfd1);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;//300000;	
        memset(m_recvbuffer,0,1024);
        debug_printf(11);
        temp_len = recv(m_socket[index],(char*)m_recvbuffer,40,0);
        if(temp_len <= 0){
                if(errno == EWOULDBLOCK){
                        debug_printf(12);
                        return;
                }                     
                //需要通知CMD处理程序，网络异常。取消当前的状态。
                if(m_pCmdHandler && temp_len != 0 ){
                        int failed = index; 
						WRT_DEBUG("接收网络数据recv错误 socket index=%d errno=0x%x",index,errno);
                        m_pCmdHandler->Send_msg(MSG_NODE_NET_EXCEPTION,&failed ,sizeof(int));

                }
                debug_printf(13);
                return;
        }else if(temp_len > 0){
                debug_printf(13);
                if(strncmp((char*)m_recvbuffer,"WRTI",4) == 0){
                        int ipacketlen = *(int*)(m_recvbuffer+4);
                        if(ipacketlen == temp_len){
                                SOCKET_PACKET2 tpakcet;
                                unsigned char* tmp = (unsigned char*)ZENMALLOC(ipacketlen);
                                if(tmp != NULL){
                                        memcpy(tmp,m_recvbuffer,ipacketlen);
                                        memset(&tpakcet,0,sizeof(tpakcet));
                                        tpakcet.sock_index = index;
                                        tpakcet.validlen = ipacketlen;
                                        tpakcet.buf = tmp;
                                        WRT_DEBUG("Recv call cmd %d\n", ipacketlen);
                                        m_pCmdHandler->Send_msg(MSG_NODE_RECV_MSG,(void*)&tpakcet,sizeof(tpakcet));     	     	   	
                                }
                                debug_printf(15);
                        }else if(ipacketlen > temp_len){
                                unsigned char* tmp = (unsigned char*) ZENMALLOC(ipacketlen+1);     
                                if(tmp == NULL){
                                        debug_printf(16);
                                        return;
                                }
                                int icopy = temp_len;
                                memcpy(tmp,m_recvbuffer,temp_len); 
                                debug_printf(27);  
                                FD_SET(m_socket[index],&readfd1);
                                while(1){
                                        ret = select(m_socket[index]+1,&readfd1,NULL,NULL,&timeout); //max_fd +1 ,by ljw 2011-1-7
                                        if(ret < 0){
                                                ZENFREE(tmp);
                                                debug_printf(17);
                                                return;
                                        }
                                        if(ret == 0){
                                                debug_printf(18);
                                                continue;
                                        }
                                        debug_printf(19);
                                        if(FD_ISSET(m_socket[index],&readfd1)){
                                                debug_printf(20);
                                                temp_len = recv(m_socket[index],(char*)(tmp+icopy),ipacketlen-icopy,0);
                                                if(temp_len <= 0){
                                                        if(errno == EWOULDBLOCK){
                                                                debug_printf(21);
                                                                continue;
                                                        }                     
                                                        //需要通知CMD处理程序，网络异常。取消当前的状态。
                                                        if(m_pCmdHandler && temp_len != 0){

                                                                int failed = index;  
                                                                //printf("ProcessRecv2 MSG_NODE_NET_EXCEPTION 2--%d \n",index);          
																WRT_DEBUG("接收网络数据recv错误 socket index=%d errno=0x%x",index,errno);
                                                                m_pCmdHandler->Send_msg(MSG_NODE_NET_EXCEPTION,&failed ,sizeof(int));

                                                        }

                                                        ZENFREE(tmp);
                                                        tmp = NULL;
                                                        debug_printf(22);
                                                        break;
                                                }else if(temp_len > 0){
                                                        debug_printf(23);
                                                        icopy  += temp_len;
                                                        if(icopy == ipacketlen){

                                                                SOCKET_PACKET2 tpakcet;
                                                                memset(&tpakcet,0,sizeof(tpakcet));
                                                                tpakcet.sock_index = index;
                                                                tpakcet.validlen = ipacketlen;
                                                                tpakcet.buf = tmp;

                                                                WRT_MESSAGE("接收到网络数据包长度%d ,socket index %d",ipacketlen,index);

                                                                m_pCmdHandler->Send_msg(MSG_NODE_RECV_MSG,(void*)&tpakcet,sizeof(tpakcet)); 
                                                                debug_printf(24);
                                                                break;
                                                        }else{
                                                                debug_printf(25);
                                                                continue;
                                                        }
                                                }
                                        } 
                                        // SDL_Delay(50);
                                }//while(1)
                                debug_printf(26);
                        }	     	   	   
                }
                printf("eeeeeeeeeeeeeeeeeee\n");
                debug_printf(28);
        }
        debug_printf(29);
}
#if 0
UInt32  TicksToMillisec(UInt32 timeInTicks){
        UInt32   millisecs;
        const UInt32   ticks2sec = 1000;
        if (timeInTicks < (ULONG_MAX / 1000))
        {
                /* for accuracy first multiply, then divide */
                millisecs = (timeInTicks * 1000) / ticks2sec;
        }
        else
        {
                /* timeInTicks is to large to multiply it first, therefore first divide.
                * Note that accuracy is good enough with such a large number */
                millisecs = (timeInTicks / ticks2sec) * 1000 +
                        ((timeInTicks % ticks2sec) * 1000) / ticks2sec;

        }

        return (millisecs);	
} 
#endif

void CSocketManager::ProcessRecv(SOCKET_PACKET* pPacket){

        int ret;
        struct timeval timeout;
        uint32_t nhi,nlo;
        const unsigned long linkout = 180000;
        UInt32  unowtime,subresult;
        FD_ZERO(&readfd);    
        max_fd = -1;
        tm_getticks(&nhi,&nlo);
        unowtime = nlo;
        debug_printf(3);
#ifdef LJW_DEBUG_LISTEN
        FD_SET(g_listen_socket,&readfd);
        timeout.tv_sec = 0;
        timeout.tv_usec = 300000;
        ret = select(g_listen_socket+1,&readfd,NULL,NULL,&timeout);
        if(ret > 0)
        {
                if(FD_ISSET(g_listen_socket,&readfd))
                {
                        int tmp_sock = -1;
                        unsigned long t_msg[4];
                        struct sockaddr_in dstaddr;
                        int addlen = sizeof(struct sockaddr_in);    
                        if((tmp_sock = accept(g_listen_socket, (struct sockaddr*)&dstaddr,&addlen)) != -1)
                        {
                                t_msg[0] = tmp_sock;
                                t_msg[1] = dstaddr.sin_addr.s_addr;
                                t_msg[2] = t_msg[3] = 0;
                                ProcessNewSocket1(t_msg);

                        }else{
                                WRT_DEBUG("网络错误g_listen_socket =%d accept  error %x",g_listen_socket,errno);
                                socket_printf();
                        }

                }
        }
        FD_ZERO(&readfd);
        max_fd = -1;
        tm_getticks(&nhi,&nlo);
        unowtime = nlo;
#endif
        for(int i =0; i<SOCK_MAX;i++)
        {  
                if(m_socket[i] != -1 ){
#if 1  	
                        subresult = unowtime - m_socketstarttime[i];
                        if( subresult > linkout ){ //3分钟,如果该链接如果大于3分钟，还未断开，则表示异常链接。
                                //发送数据到上层，告知异常。
                                WRT_DEBUG("超长网络连接将被关闭index %d,sd = %d ",i,m_socket[i]);
                                closesocket(m_socket[i]);                  
                                m_socket[i] = -1;
                                m_ipaddr[i] = -1;
                                m_socketstarttime[i] = 0;
                                int result = i;

                                m_pCmdHandler->Send_msg(MSG_NODE_SOCKET_CUT,&result ,sizeof(int));
                                debug_printf(4);
                                continue;
                        }   
#endif               
                        FD_SET(m_socket[i],&readfd);       
                        if(m_socket[i] > max_fd){
                                max_fd = m_socket[i];
                        }
                }            
        }
        debug_printf(5);
        if(max_fd == -1){
#ifndef LJW_DEBUG_LISTEN
                m_bIslink = false;
#endif
                debug_printf(6);
                return;
        }
        timeout.tv_sec  = 0;
        timeout.tv_usec = 150000;
        ret = select(max_fd+1,&readfd,NULL,NULL,&timeout);
        if(ret <= 0){
                debug_printf(7);
                return;
        }
        debug_printf(8);
        //当有包可以收的时候，收到包，并发给CMD处理任务。
        for(int j =0; j< SOCK_MAX;j++){
                if(m_socket[j]  != -1){
                        if(FD_ISSET(m_socket[j],&readfd)){
                                debug_printf(9);
                                ProcessRecv2(j);
                        }  
                }
        }
        debug_printf(10);

}

bool CSocketManager::ModifyHostIpAndMask(unsigned long ipaddr,unsigned long netmask)
{
	
        int tmp_sock = -1;
        struct ifreq T_ifreq;
        tmp_sock  = socket( AF_INET ,SOCK_DGRAM, 0);
        WRT_DEBUG("ModifyHostIpAndMask%x,%x \n",ipaddr,netmask);
        if(tmp_sock < 0){
                WRT_WARNING("创建socket failed ");
                return false;
        }
        if(ipaddr != 0){
              
                strcpy(T_ifreq.ifr_name,"eth0");
                ((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_family = AF_INET;
                ((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_addr.s_addr =ipaddr; //htonl(ipaddr);
                if(0 !=ioctlsocket(tmp_sock,SIOCSIFADDR,(char*)&T_ifreq)){
                        WRT_DEBUG("修改IP地址失败new ipaddr = 0x%x",ipaddr);
                        closesocket(tmp_sock);
                        return false;
                }
        }
        if(netmask != 0){
                memset(&T_ifreq,0,sizeof(struct ifreq));
                strcpy(T_ifreq.ifr_name,"eth0");
                ((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_family = AF_INET;
                ((struct sockaddr_in*)(&T_ifreq.ifr_addr))->sin_addr.s_addr =netmask;// htonl(netmask);
                if(0 != ioctlsocket(tmp_sock,SIOCSIFNETMASK,(char*)&T_ifreq)){
                        WRT_DEBUG("修改网络掩码失败new mask = 0x%x %d",netmask,errno);
                        closesocket(tmp_sock);
                        return false;
                }
        }
        closesocket(tmp_sock);
     
        return true;      
}

bool CSocketManager::ModifyHostGateWay(unsigned long gatewayip)
{
	
	struct in_addr in;
	int ret;
	char cmd[256];
	in.s_addr = gatewayip;
	char* ip = inet_ntoa(in);
	memset(cmd,0,256);
	sprintf(cmd,"route add default gateway %s",ip);
	if(system(cmd) < 0){
		WRT_DEBUG("cmd =%s 失败",cmd);
		return false;
	}
	WRT_DEBUG("cmd =%s ok",cmd);
	return true;
	
	
	
	/*
        struct rtentry T_rty;
        int tmp_sock = -1;
        if(gatewayip == 0)
                return false;
        WRT_DEBUG("ModifyHostGateWay%x\n",gatewayip);
        tmp_sock  = socket( AF_INET ,SOCK_DGRAM, 0);
        if(tmp_sock < 0){
                 WRT_WARNING("创建socket failed ");
                return false;
        }
        T_rty.rt_flags = RTF_GATEWAY;
        T_rty.rt_dev = "eth0";
        ((struct sockaddr_in*)(&T_rty.rt_dst))->sin_family = AF_INET;
        ((struct sockaddr_in*)(&T_rty.rt_dst))->sin_addr.s_addr=0x0;
        ((struct sockaddr_in*)(&T_rty.rt_gateway))->sin_family = AF_INET;
        ((struct sockaddr_in*)(&T_rty.rt_gateway))->sin_addr.s_addr = gatewayip;//htonl(gatewayip);
        if(0 != ioctlsocket(tmp_sock,SIOCADDRT,(char*)&T_rty)){ //修改默认的网关
				WRT_DEBUG("修改网关失败new gateway = 0x%x,errno=%x",gatewayip,errno);
                closesocket(tmp_sock);
                return false;
        }
        closesocket(tmp_sock);
        */
        return true;
}

void CSocketManager::socket_printf(){
        for(int i =0; i<SOCK_MAX;i++)
                if(m_socket[i] != -1){
                        WRT_DEBUG("当前存在的socket index = %d,m_socket  = %d,m_ipaddr = 0x%x",i,m_socket[i],m_ipaddr[i]);
                }
}

/////////////////////////////////////////////////////////////////////////////////
//1:监听任务
//2:主动连接任务
/////////////////////////////////////////////////////////////////////////////////
static int  StartListen(){
        struct sockaddr_in serveraddr;
        int connected_len = 0;
        unsigned long flag = 1;
        int g_listensocket=-1;       
        g_listensocket = socket( AF_INET ,SOCK_STREAM, 0);    
        if(g_listensocket == -1){
        	WRT_DEBUG("create g_listensocket error 0x%x",errno);
        	return  -1;
        }
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_addr.s_addr = INADDR_ANY;
        serveraddr.sin_port   = htons(20200);
        if(!bind(g_listensocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr))){
                listen(g_listensocket,5); 
        } else{
        	WRT_DEBUG("bind g_listensocket error 0x%x",errno);
        	closesocket(g_listensocket);
        	g_listensocket = -1;
        	return  -1;
        }
#ifdef LJW_DEBUG_LISTEN
        ioctlsocket( g_listensocket,FIONBIO,(char*)&flag);
#endif
        return  g_listensocket;

}

/**
*socket 监听任务
*/
static int  socket_listen_task(void* param){
        struct sockaddr_in dstaddr;
        int listensocket = -1;
        socklen_t addlen = (socklen_t)sizeof(struct sockaddr_in);
        unsigned long msg[4];
        unsigned long t_count = 0;
        listensocket = StartListen();
        if(listensocket == -1){
        	WRT_DEBUG("socket_listen_task start error,reboot");
        	system("reboot -f ");
        	return 0;
        }
        while(1){
                int tmp_sock = -1;
                memset(msg,0,sizeof(unsigned long)*4);  
                WRT_DEBUG("socket_listen_task  wait accept ");     
                if((tmp_sock = accept(listensocket, (struct sockaddr*)&dstaddr,&addlen)) != -1){
                        int ret = 0;
                        msg[0] =tmp_sock; //shr_socket(tmp_sock,socketmanager_task_id);
                        msg[1] = dstaddr.sin_addr.s_addr ;
                        msg[2] = 0;
                        msg[3] =0; 
                        t_count++;

                        //发送有新的connect 到socketmanager;
                        if(m_pSocketManager){
                                int ret =0;
                                int type= MSG_NODE_NEW_SOCKET_CLIENT;
                                ret = m_pSocketManager->Send_msg(type,(unsigned char*)msg,sizeof(unsigned long)*4);
                                WRT_MESSAGE("接收到网络连接 socket  = %d ",msg[0] );
                        }
                } else{
                	WRT_DEBUG("accept ERROR 0x%x",errno);
                }     
                SDL_Delay(200); //休息200MS，防止连接快速连续。
        }
        return 0;
       // tmosalTaskExit();
}

static int mul_connect_client(unsigned long tmpmsg[4]){
        int tmpsock[4] = {-1,-1,-1,-1};
        int tmpshrsock[4] = {-1,-1,-1,-1};
        struct timeval timeout;
        struct sockaddr_in addr;
        fd_set writefd;
        int i=0;
        int rc = 0;
        int flag = 1;
        int port = 20200;
        int ok[4] = {0,0,0,0};
        int maxfd =  -1;
        FD_ZERO(&writefd);

        memcpy(g_tmpipaddr,(void*)tmpmsg[0],sizeof(unsigned long)*4);
        for(i =0; i< 4;i++){
                if(g_tmpipaddr[i] == 0)
                        continue;
                WRT_MESSAGE("多路主动IP连接 ip = 0x%x ",g_tmpipaddr[i]);
                tmpsock[i] = socket(AF_INET,SOCK_STREAM, 0);
                if(tmpsock[i] < 0){
                        continue;
                }
                memset(&addr,0,sizeof(struct sockaddr_in));
                addr.sin_family = AF_INET;      
                addr.sin_addr.s_addr = g_tmpipaddr[i];
                addr.sin_port   = htons(port);
                ioctlsocket(tmpsock[i],FIONBIO,(char*)&flag);
                if(connect(tmpsock[i],(struct sockaddr*)&addr,sizeof(struct sockaddr_in)) < 0){
                        if(errno  != EINPROGRESS){
                                closesocket(tmpsock[i]);
                                tmpsock[i] = -1;
                                continue;
                        }
                }
                if(maxfd < tmpsock[i])
                        maxfd = tmpsock[i];
                FD_SET(tmpsock[i],&writefd);
                WRT_MESSAGE("多路网络连接maxfd = %d  ,i = %d",maxfd,i);
        }
        if(maxfd == -1)  //一个都没有连接成功。
                return -1;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;     
        for(i =0; i< 5;i++){ //最多等待5s
                rc = select(maxfd+1,NULL,&writefd,NULL,&timeout);
                if(rc <= 0)
                        continue;
                if(tmpsock[0] != -1 && FD_ISSET(tmpsock[0],&writefd)){
                        ok[0] = 1;
                        FD_CLR(tmpsock[0],&writefd);
                        WRT_DEBUG("第一路网络连接OK");
                }
                if(tmpsock[1] != -1 && FD_ISSET(tmpsock[1],&writefd)){
                        ok[1] = 1;
                        FD_CLR(tmpsock[1],&writefd);
                        WRT_DEBUG("第二路网络连接OK");
                }
                if(tmpsock[2] != -1 && FD_ISSET(tmpsock[2],&writefd)){
                        ok[2] = 1;
                        FD_CLR(tmpsock[2],&writefd);
                        WRT_DEBUG("第三路网络连接OK");
                } 
                if(tmpsock[3] != -1  && FD_ISSET(tmpsock[3],&writefd)){
                        ok[3] = 1;
                        FD_CLR(tmpsock[3],&writefd);
                       WRT_DEBUG("第死路网络连接OK");
                }
                if(ok[0] == 1 && ok[1] == 1 && ok[2] == 1 && ok[3] == 1)                    
                        break;
        } 
        if(ok[0] == 0 && ok[1] == 0 && ok[2] == 0 && ok[3] == 0){
                for(i=0;i<4;i++){
                        if(tmpsock[i] != -1)
                                closesocket(tmpsock[i]);
                }
                return -1; //表示一个都没有连接成功

        }

        for(i =0; i< 4;i++){     
                if(ok[i] == 1 && tmpsock[i] != -1 ){
                        tmpshrsock[i] =tmpsock[i];// shr_socket(tmpsock[i],socketmanager_task_id);
                        //closesocket(tmpsock[i]);
                        tmpsock[i] = -1;
                        continue;
                }
                if(tmpsock[i] != -1){
                        closesocket(tmpsock[i]);
                        tmpsock[i] = -1;
                }
        }
        unsigned long* pTemp = (unsigned long*)(unsigned char*)tmpmsg[0];
        for(i=0;i<4;i++){
                pTemp[i] = tmpshrsock[i];
        }
        if(m_pSocketManager){
                int type = MSG_NODE_NEW_CLIENT;
                m_pSocketManager->Send_msg(type,(unsigned char*)tmpmsg,sizeof(unsigned long)*4);
        }     
        return 0;

}

/**
*连接客户
*/
static int connect_client(unsigned long ipaddr){

        struct sockaddr_in addr;
        int tmp_sock = -1;
        int port = 20200;
        fd_set writefd;
        struct timeval timeout;    
        int max_fd = -1; 
        int flag = 1;
        int rc =0;
        tmp_sock  = socket(AF_INET,SOCK_STREAM, 0);
        if(tmp_sock < 0){
                WRT_DEBUG("创建socket failed 0x%x",errno);              
                return -1;
        }          
        memset(&addr,0,sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;      
        addr.sin_addr.s_addr = ipaddr;
        addr.sin_port   = htons(port);

        ioctlsocket(tmp_sock,FIONBIO,(char*)&flag);
        if(connect(tmp_sock,(struct sockaddr*)&addr,sizeof(struct sockaddr_in)) < 0){
                if(errno  != EINPROGRESS){           
                        closesocket(tmp_sock);
                        tmp_sock = -1;  
                        return tmp_sock;      
                }
        }  


        FD_ZERO(&writefd);
        if(max_fd < tmp_sock)
                max_fd = tmp_sock;
        FD_SET(tmp_sock,&writefd);   
        int valopt,optlen;
        optlen = sizeof(valopt);
        int cnt=5;
 		while(cnt--){         
 		 timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        rc = select(max_fd+1,NULL,&writefd,NULL,&timeout);
         if(rc < 0 && errno == EINTR)
         {
          continue;
         }
        if(rc > 0){      
                if(FD_ISSET(tmp_sock,&writefd)){
              if(getsockopt(tmp_sock,SOL_SOCKET,SO_ERROR,&valopt,(socklen_t *)&optlen) < 0){
               break;
              }
             // printf("cnt %d val %d\n",cnt,valopt);
              if(valopt == EINPROGRESS)
              {
               continue;
              }
              if(valopt)
              {
              		if(cnt ==1)
               		break;
              		else
               			continue;
              }
                        WRT_MESSAGE("主动连接connect ok  ipaddr = 0x%x socket =%d ",ipaddr,tmp_sock);
                        return tmp_sock;
                }  
        }else{
                WRT_MESSAGE("主动连接connect ip 0x%x error 0x%x",ipaddr,errno);
                 break;
        }
 		}

        closesocket(tmp_sock);
        tmp_sock = -1;
        return tmp_sock;
}

static int socket_client_task(void *pParam){
        unsigned long tmpmsg[4];
        unsigned long size=0;
        int err = 0;
        CWRTMsg* pMsg = NULL;
        g_socket_msg_sem = SDL_CreateSemaphore(0);
        assert(g_socket_msg_sem);
        while(1){
                int rc = -1;
                memset(tmpmsg,0,sizeof(tmpmsg));
                SDL_SemWait(g_socket_msg_sem);
                pMsg = g_socket_msg_queue.get_message();
                if(pMsg){
                	err = parse_msg(pMsg,tmpmsg);
                	if(err < 0)
                		break;
                	if(err == 0)
                		continue;
                	rc = 0;
                }
                
                // rc = tmosalQueueReceive(g_qid,tmpmsg,&size,NULL,tmosalQueueReceiveFlagNone);
              //  rc = q_receive(g_qid1,Q_WAIT,0,tmpmsg);
                if(rc == 0 /*&& tmpmsg[0] == SOCKET_CONNECT */){
                        if(tmpmsg[3] == 0){	
                                int tmpfd = connect_client(tmpmsg[0]);
                                if(tmpfd != -1){
                                        g_tmpipaddr[0] = tmpmsg[0];
                                        tmpmsg[0] =tmpfd;// shr_socket(tmpfd,socketmanager_task_id);
                                        if(m_pSocketManager){
                                                int type = MSG_NODE_NEW_CLIENT;
                                                m_pSocketManager->Send_msg(type,(unsigned char*)tmpmsg,sizeof(unsigned long)*4);
                                        }
                                     //   closesocket(tmpfd);

                                } else{
                                        //发送消息主动连接失败
                                        if(tmpmsg[2] > 0){
                                                ZENFREE((void*)tmpmsg[1]);
                                        } 
                                        tmpmsg[0] = -1;
                                        tmpmsg[1] = 0;
                                        tmpmsg[2] = 0;
                                        tmpmsg[3] = 0;            
                                        if(m_pSocketManager){
                                                int type = MSG_NODE_NEW_CLIENT;
                                                m_pSocketManager->Send_msg(type,(unsigned char*)tmpmsg,sizeof(unsigned long)*4);
                                        }		    
                                }
                        }else{
                                //如果同时连接多路
                                if(mul_connect_client(tmpmsg) == -1){
                                        //发送消息主动连接失败
                                        if(tmpmsg[2] > 0){
                                                ZENFREE((void*)tmpmsg[1]);
                                        } 
                                        if(tmpmsg[3] > 0)
                                                ZENFREE((void*)tmpmsg[0]);
                                        tmpmsg[0] = -1;
                                        tmpmsg[1] = 0;
                                        tmpmsg[2] = 0;
                                        tmpmsg[3] = 0;            
                                        if(m_pSocketManager){
                                                int type = MSG_NODE_NEW_CLIENT;
                                                m_pSocketManager->Send_msg(type,(unsigned char*)tmpmsg,sizeof(unsigned long)*4);
                                        }		         
                                }
                        }
                }
        }
        g_socket_msg_queue.release();
        SDL_DestroySemaphore(g_socket_msg_sem);
        g_socket_msg_sem = NULL;
        return 0;
        //tmosalTaskExit();
}




void initSocketServer(){
        int rc;
        int ret;



#ifndef LJW_DEBUG_LISTEN
	SDL_CreateThread(socket_listen_task,NULL);

        /*
        rc = tmosalTaskCreate(socket_listen_task,NULL,60,0x4000,&g_listen_tid,"t110",tmosalTaskStarted);
        if(rc != TM_OK){
                WRT_DEBUG("tmosalTaskCreate t110 failed \n");
                return;
        }
        */
#endif

#if 1   
       /* rc = tmosalTaskCreate (socket_client_task,NULL,60,0x4000,&g_newclient_tid,"t111",tmosalTaskStarted);
        if(rc != TM_OK){
                WRT_DEBUG("tmosalTaskCreate t111 failed \n");
                return;
        }
        */
        SDL_CreateThread(socket_client_task,NULL);
       
        
#endif

        WRT_MESSAGE("网络模块初始化完毕");
}

void Send_Test(){

}

void CSocketManager::SendResultToCenter(unsigned long ip ,unsigned char* buf,int len){
        int tmpsocket = -1;
        tmpsocket = connect_client(ip);	
        if(tmpsocket != -1){
                fd_set writefd1;
                fd_set  readfd1;
                fd_set  exceptfd1;
                struct linger Linger;
                int ret =0;
                int sendlen = 0;
                int sendlen1 = 0;
                FD_ZERO(&writefd1);
                FD_SET(tmpsocket,&writefd1);
                FD_ZERO(&readfd1);
                FD_SET(tmpsocket,&readfd1);
                FD_ZERO(&exceptfd1);
                FD_SET(tmpsocket,&exceptfd1);
                struct timeval timeout;
                int flag = 1;
                int sendok = 0;
                int timeoutcount = 0;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
  		Linger.l_onoff = 1;   // 开启 linger 设定
    		Linger.l_linger = 2;  // 设定 linger 时间为 n 秒 内部延时关闭
    		setsockopt(tmpsocket, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(struct linger));

               // setsockopt(tmpsocket,IPPROTO_TCP,TCP_NODELAY,(const char*)&flag,sizeof(int)); 
               
                while(1){
                        ret = select(tmpsocket+1,&readfd1,&writefd1,&exceptfd1,&timeout);
                        if(ret < 0){
                                FD_CLR(tmpsocket,&writefd1);
                                closesocket(tmpsocket);
                                WRT_DEBUG("SendResultToCenter:select error");
                                return;
                        }
                        if(ret == 0){      
                                if(sendok == 1){
                                        timeoutcount++;
                                        if(timeoutcount > 5){
                                        	WRT_DEBUG("SendResultToCenter: select timeout end break"); 	 
                                                break;
                                        }
                                } 
                                WRT_DEBUG("SendResultToCenter:select timeout"); 	 
                                continue;
                        }
                        if(FD_ISSET(tmpsocket,&exceptfd1)){
                        	WRT_DEBUG("发送数据到中心 检测到网络发生错误，关闭socket");
                        	break;
                        }
                        if(FD_ISSET(tmpsocket,&writefd1)){       	
                                sendlen = send(tmpsocket,(char*)buf+sendlen1,len-sendlen1,0);
                                if(sendlen < 0){
                                	WRT_DEBUG("SendResultToCenter:send error %x",errno);
                                        break;
                                }
                                if(errno == EWOULDBLOCK){
                                	 WRT_DEBUG("发送数据到中心 send EWOULDBLOCK %d",sendlen);
                                }
                                	
                                sendlen1 += sendlen;
                                if(sendlen1 == len){
                                        FD_CLR(tmpsocket,&writefd1);
                                        sendok  = 1;
                                        WRT_DEBUG("发送数据到中心 发送完毕");
                                        break;
                                }
                        }
                        //屏蔽这个的原因是，当发送完数据，后，如果对方没有关闭SOCKET，则，容易导致死循环。
                        //因此当发送完毕后，就直接退出然后关闭SOCKET。
#if 0
                        if(FD_ISSET(tmpsocket,&readfd1)){
                                int ret = 0;
                                char buf[2];
                                ret = recv(tmpsocket,(char*)&buf,1,0);
                                if(ret < 0 || ret == 0){
                                        if(errno == EWOULDBLOCK){
                                                continue;
                                        }            	  	  
                                        break;
                                }
                        }
#endif
                }
                if(sendok ==0)
                        FD_CLR(tmpsocket,&writefd1);
                FD_CLR(tmpsocket,&readfd1);
                WRT_DEBUG("关闭soket %d,发送数据长度%d",tmpsocket,len);
                SDL_Delay(300);
                closesocket(tmpsocket);	
        }
        WRT_MESSAGE("发送数据到中心完毕");
}

