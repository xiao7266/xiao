/** 
@file  unix.c
@brief ENet Unix system specific functions
*/
#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

#ifdef HAS_FCNTL
#include <fcntl.h>
#endif

#ifdef __APPLE__
#undef HAS_POLL
#endif

#ifdef HAS_POLL
#include <sys/poll.h>
#endif

#ifndef HAS_SOCKLEN_T
typedef int socklen_t;
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif



static AppENetCallbacks appcallbacks = {NULL,NULL};

static pthread_t  gudpThread;
static short  listen_port = 20302;
static short  send_port = 20300;
static ENetHost * Sendclient = NULL; 
static int    g_thread_exit = 0;
static int    g_init = 0;
static enet_uint32 timeBase = 0;

int
enet_initialize (void)
{
        return 0;
}

void
enet_deinitialize (void)
{
}

enet_uint32
enet_time_get (void)
{
        struct timeval timeVal;

        gettimeofday (& timeVal, NULL);

        return timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000 - timeBase;
}

void
enet_time_set (enet_uint32 newTimeBase)
{
        struct timeval timeVal;

        gettimeofday (& timeVal, NULL);

        timeBase = timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000 - newTimeBase;
}

int
enet_address_set_host (ENetAddress * address, const char * name)
{
        struct hostent * hostEntry = NULL;
#ifdef HAS_GETHOSTBYNAME_R
        struct hostent hostData;
        char buffer [2048];
        int errnum;

#if defined(linux) || defined(__FreeBSD__)
        gethostbyname_r (name, & hostData, buffer, sizeof (buffer), & hostEntry, & errnum);
#else
        hostEntry = gethostbyname_r (name, & hostData, buffer, sizeof (buffer), & errnum);
#endif
#else
        hostEntry = gethostbyname (name);
#endif

        if (hostEntry == NULL ||
                hostEntry -> h_addrtype != AF_INET)
        {
#ifdef HAS_INET_PTON
                if (! inet_pton (AF_INET, name, & address -> host))
#else
                if (! inet_aton (name, (struct in_addr *) & address -> host))
#endif
                        return -1;
                return 0;
        }

        address -> host = * (enet_uint32 *) hostEntry -> h_addr_list [0];

        return 0;
}

int
enet_address_get_host_ip (const ENetAddress * address, char * name, size_t nameLength)
{
#ifdef HAS_INET_NTOP
        if (inet_ntop (AF_INET, & address -> host, name, nameLength) == NULL)
#else
        char * addr = inet_ntoa (* (struct in_addr *) & address -> host);
        if (addr != NULL)
                strncpy (name, addr, nameLength);
        else
#endif
                return -1;
        return 0;
}

int
enet_address_get_host (const ENetAddress * address, char * name, size_t nameLength)
{
        struct in_addr in;
        struct hostent * hostEntry = NULL;
#ifdef HAS_GETHOSTBYADDR_R
        struct hostent hostData;
        char buffer [2048];
        int errnum;

        in.s_addr = address -> host;

#if defined(linux) || defined(__FreeBSD__)
        gethostbyaddr_r ((char *) & in, sizeof (struct in_addr), AF_INET, & hostData, buffer, sizeof (buffer), & hostEntry, & errnum);
#else
        hostEntry = gethostbyaddr_r ((char *) & in, sizeof (struct in_addr), AF_INET, & hostData, buffer, sizeof (buffer), & errnum);
#endif
#else
        in.s_addr = address -> host;

        hostEntry = gethostbyaddr ((char *) & in, sizeof (struct in_addr), AF_INET);
#endif

        if (hostEntry == NULL)
                return enet_address_get_host_ip (address, name, nameLength);

        strncpy (name, hostEntry -> h_name, nameLength);

        return 0;
}

ENetSocket
enet_socket_create (ENetSocketType type, const ENetAddress * address)
{
        ENetSocket newSocket = socket (PF_INET, type == ENET_SOCKET_TYPE_DATAGRAM ? SOCK_DGRAM : SOCK_STREAM, 0);
        struct sockaddr_in sin;

        if (newSocket == ENET_SOCKET_NULL)
                return ENET_SOCKET_NULL;

        if (address == NULL)
                return newSocket;

        memset (& sin, 0, sizeof (struct sockaddr_in));

        sin.sin_family = AF_INET;
        sin.sin_port = ENET_HOST_TO_NET_16 (address -> port);
        sin.sin_addr.s_addr = address -> host;

        if (bind (newSocket, 
                (struct sockaddr *) & sin,
                sizeof (struct sockaddr_in)) == -1 ||
                (type == ENET_SOCKET_TYPE_STREAM &&
                address -> port != ENET_PORT_ANY &&
                listen (newSocket, SOMAXCONN) == -1))
        {
                close (newSocket);

                return ENET_SOCKET_NULL;
        }

        return newSocket;
}

int
enet_socket_set_option (ENetSocket socket, ENetSocketOption option, int value)
{
        int result = -1;
        switch (option)
        {
        case ENET_SOCKOPT_NONBLOCK:
#ifdef HAS_FCNTL
                result = fcntl (socket, F_SETFL, O_NONBLOCK | fcntl (socket, F_GETFL));
#else
                result = ioctl (socket, FIONBIO, & value);
#endif
                break;

        case ENET_SOCKOPT_BROADCAST:
                result = setsockopt (socket, SOL_SOCKET, SO_BROADCAST, (char *) & value, sizeof (int));
                break;

        case ENET_SOCKOPT_RCVBUF:
                result = setsockopt (socket, SOL_SOCKET, SO_RCVBUF, (char *) & value, sizeof (int));
                break;

        case ENET_SOCKOPT_SNDBUF:
                result = setsockopt (socket, SOL_SOCKET, SO_SNDBUF, (char *) & value, sizeof (int));
                break;

        default:
                break;
        }
        return result == -1 ? -1 : 0;
}

int
enet_socket_connect (ENetSocket socket, const ENetAddress * address)
{
        struct sockaddr_in sin;

        memset (& sin, 0, sizeof (struct sockaddr_in));

        sin.sin_family = AF_INET;
        sin.sin_port = ENET_HOST_TO_NET_16 (address -> port);
        sin.sin_addr.s_addr = address -> host;

        return connect (socket, (struct sockaddr *) & sin, sizeof (struct sockaddr_in));
}

ENetSocket
enet_socket_accept (ENetSocket socket, ENetAddress * address)
{
        int result;
        struct sockaddr_in sin;
        socklen_t sinLength = sizeof (struct sockaddr_in);

        result = accept (socket, 
                address != NULL ? (struct sockaddr *) & sin : NULL, 
                address != NULL ? & sinLength : NULL);

        if (result == -1)
                return ENET_SOCKET_NULL;

        if (address != NULL)
        {
                address -> host = (enet_uint32) sin.sin_addr.s_addr;
                address -> port = ENET_NET_TO_HOST_16 (sin.sin_port);
        }

        return result;
} 

void
enet_socket_destroy (ENetSocket socket)
{
        close (socket);
}

int
enet_socket_send (ENetSocket socket,
                  const ENetAddress * address,
                  const ENetBuffer * buffers,
                  size_t bufferCount)
{
        struct msghdr msgHdr;
        struct sockaddr_in sin;
        int sentLength;

        memset (& msgHdr, 0, sizeof (struct msghdr));

        if (address != NULL)
        {
                sin.sin_family = AF_INET;
                sin.sin_port = ENET_HOST_TO_NET_16 (address -> port);
                sin.sin_addr.s_addr = address -> host;

                msgHdr.msg_name = & sin;
                msgHdr.msg_namelen = sizeof (struct sockaddr_in);
        }

        msgHdr.msg_iov = (struct iovec *) buffers;
        msgHdr.msg_iovlen = bufferCount;

        sentLength = sendmsg (socket, & msgHdr, MSG_NOSIGNAL);

        if (sentLength == -1)
        {
                if (errno == EWOULDBLOCK)
                        return 0;

                return -1;
        }

        return sentLength;
}

int
enet_socket_receive (ENetSocket socket,
                     ENetAddress * address,
                     ENetBuffer * buffers,
                     size_t bufferCount)
{
#if 0
	struct sockaddr_in sin;
	int recvLength;
        int sockaddrlen = sizeof(struct sockaddr_in);
	recvLength = 0;
	memset(&sin,0,sizeof(struct sockaddr_in));
	recvLength = recvfrom(socket,buffers->data,buffers->dataLength,0,(struct sockaddr*)&sin,&sockaddrlen);
	if(recvLength == -1)
	{
		if(errno == EWOULDBLOCK)
			return 0;
		return -1;
	}

	if(address != NULL)
	{
		address->host = (enet_uint32)sin.sin_addr.s_addr;
		address->port = ENET_NET_TO_HOST_16(sin.sin_port);
	}
	printf("recv databuffer %x,len = %d,recvlen = %d \n",
		buffers->data,buffers->dataLength,recvLength);
#endif
        struct msghdr msgHdr;
        struct sockaddr_in sin;
        int recvLength;

        memset (& msgHdr, 0, sizeof (struct msghdr));

        if (address != NULL)
        {
                msgHdr.msg_name = & sin;
                msgHdr.msg_namelen = sizeof (struct sockaddr_in);
        }

        msgHdr.msg_iov = (struct iovec *) buffers;
        msgHdr.msg_iovlen = bufferCount;

        recvLength = recvmsg (socket, & msgHdr, MSG_NOSIGNAL);

        if (recvLength == -1)
        {
                if (errno == EWOULDBLOCK)
                        return 0;

                return -1;
        }

#ifdef HAS_MSGHDR_FLAGS
        if (msgHdr.msg_flags & MSG_TRUNC)
                return -1;
#endif

        if (address != NULL)
        {
                address -> host = (enet_uint32) sin.sin_addr.s_addr;
                address -> port = ENET_NET_TO_HOST_16 (sin.sin_port);
        }

        return recvLength;
}

int
enet_socket_wait (ENetSocket socket, enet_uint32 * condition, enet_uint32 timeout)
{
#ifdef HAS_POLL
        struct pollfd pollSocket;
        int pollCount;

        pollSocket.fd = socket;
        pollSocket.events = 0;

        if (* condition & ENET_SOCKET_WAIT_SEND)
                pollSocket.events |= POLLOUT;

        if (* condition & ENET_SOCKET_WAIT_RECEIVE)
                pollSocket.events |= POLLIN;

        pollCount = poll (& pollSocket, 1, timeout);

        if (pollCount < 0)
                return -1;

        * condition = ENET_SOCKET_WAIT_NONE;

        if (pollCount == 0)
                return 0;

        if (pollSocket.revents & POLLOUT)
                * condition |= ENET_SOCKET_WAIT_SEND;

        if (pollSocket.revents & POLLIN)
                * condition |= ENET_SOCKET_WAIT_RECEIVE;

        return 0;
#else
        fd_set readSet, writeSet;
        struct timeval timeVal;
        int selectCount;

        timeVal.tv_sec = timeout / 1000;
        timeVal.tv_usec = (timeout % 1000) * 1000;

        FD_ZERO (& readSet);
        FD_ZERO (& writeSet);

        if (* condition & ENET_SOCKET_WAIT_SEND)
                FD_SET (socket, & writeSet);

        if (* condition & ENET_SOCKET_WAIT_RECEIVE)
                FD_SET (socket, & readSet);

        selectCount = select (socket + 1, & readSet, & writeSet, NULL, & timeVal);

        if (selectCount < 0)
                return -1;

        * condition = ENET_SOCKET_WAIT_NONE;

        if (selectCount == 0)
                return 0;

        if (FD_ISSET (socket, & writeSet))
                * condition |= ENET_SOCKET_WAIT_SEND;

        if (FD_ISSET (socket, & readSet))
                * condition |= ENET_SOCKET_WAIT_RECEIVE;

        return 0;
#endif
}

#endif




static void service(ENetHost *client) {
        ENetEvent event;    
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service (client, & event, 1000) > 0)
        {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_CONNECT:
                        {
                                /* Store any relevant client information here. */
                                event.peer -> data = "Client information";
                        }
                        break;

                case ENET_EVENT_TYPE_RECEIVE:
                        {
                                printf("ENET_EVENT_TYPE_RECEIVE\r\n");

                                //ENetPacket *packetresp;
                                if((event.packet ->dataLength > 39) && strncmp("WRTI",event.packet->data,4) == 0){
                                        short tmpcmd = *(short*)((char*)event.packet->data+8);
                                        tmpcmd = ntohs(tmpcmd);
                                        printf("<<enet>>tmpcmd = %x \n",tmpcmd);
                                        if(tmpcmd > 0x300 && tmpcmd < 0x309){ //sip系列命令
                                                if(appcallbacks.AppRecvCallback2 != NULL){
                                                        appcallbacks.AppRecvCallback2(event.peer ->address.host,event.peer -> address.port,event.packet -> data,event.packet -> dataLength);
                                                }
                                        }else if(tmpcmd >= 0x80 && tmpcmd < 0x90){ //同步的系列命令
                                                if(appcallbacks.AppRecvCallback2 != NULL){
                                                        appcallbacks.AppRecvCallback2(event.peer ->address.host,event.peer -> address.port,event.packet -> data,event.packet -> dataLength);
                                                }
                                        }else if(tmpcmd == 0x02F0){
                                                if(appcallbacks.AppRecvCallback2 != NULL){
                                                        printf("appcallbacks.AppRecvCallback2  = %x \n",appcallbacks.AppRecvCallback2);
                                                        appcallbacks.AppRecvCallback2(event.peer ->address.host,event.peer -> address.port,event.packet -> data,event.packet -> dataLength);
                                                }                    	
                                        }else{
                                                if(appcallbacks.AppRecvCallback != NULL){   

                                                        appcallbacks.AppRecvCallback(event.peer ->address.host,event.peer -> address.port,event.packet -> data,event.packet -> dataLength);
                                                }			        
                                        } 
                                }                                
                                /*
                                if(appcallbacks.AppRecvCallback2 != NULL)
                                appcallbacks.AppRecvCallback2(event.peer ->address.host,event.peer -> address.port,event.packet -> data,event.packet -> dataLength);
                                */
                                /* Clean up the packet now that we're done using it. */
                                enet_packet_destroy (event.packet);
                        }
                        break;

                case ENET_EVENT_TYPE_DISCONNECT:
                        /* Reset the peer's client information. */

                        event.peer -> data = NULL;
			break;
		default:
			break;
                }
        }
}

static void ENet_Recv_Task(void* param){
        ENetAddress address;
        ENetHost * server;

        if (enet_initialize () != 0)
        {
                printf("An error occurred while initializing ENet.\n");
                return;
        }

        /* Bind the server to the default localhost.     */
        /* A specific host address can be specified by   */
        /* enet_address_set_host (& address, "x.x.x.x"); */

        address.host = ENET_HOST_ANY;
        /* Bind the server to port 1234. */
        address.port = listen_port;

        server = enet_host_create (& address /* the address to bind the server host to */, 
                64      /* allow up to 32 clients and/or outgoing connections */,
                0      /* assume any amount of incoming bandwidth */,
                0      /* assume any amount of outgoing bandwidth */);
        if (server == NULL)
        {
                printf("An error occurred while trying to create an ENet server host.errno =%x\n",errno);
                return;
        }
        g_thread_exit = 0;
        while(1) { 
		if(g_thread_exit == 1)
			break;
                service(server);
        }   
       enet_host_destroy(server);
       server = NULL;
}

ENET_API  int App_SetCallBack2(const AppENetCallbacks * inits)
{
    if(inits == NULL)
        return -1;
    if(inits->AppRecvCallback != 0)
        appcallbacks.AppRecvCallback = inits->AppRecvCallback;

    if(inits->AppRecvCallback2 != 0){
        appcallbacks.AppRecvCallback2 = inits->AppRecvCallback2;

    }
    return 0;	
}

int  App_InitENet(short listenport,short sendport,AppENetCallbacks* callbacks){

        pthread_attr_t attr;
        ENetAddress address; 
        if(g_init == 1){
        	printf("�Ѿ���ʼ��\n");
        	return 0;
        }
       
         
        if(callbacks != NULL){
                appcallbacks.AppRecvCallback2 = 0;
				appcallbacks.AppRecvCallback = 0;
                if(callbacks->AppRecvCallback2 != 0)
	                appcallbacks.AppRecvCallback2 = callbacks->AppRecvCallback2;
                if(callbacks->AppRecvCallback != 0)
                	appcallbacks.AppRecvCallback = callbacks->AppRecvCallback;
        }
        if(listenport != 0)
                listen_port = (listenport);
        if(sendport != 0)
                send_port = (sendport);
        

        /* Bind the server to the default localhost.     */
        /* A specific host address can be specified by   */
        /* enet_address_set_host (& address, "x.x.x.x"); */

        address.host = ENET_HOST_ANY;
        /* Bind the server to port 1234. */
        address.port = send_port;

        Sendclient = enet_host_create (NULL /* the address to bind the server host to */, 
                1      /* allow up to 32 clients and/or outgoing connections */,
                0      /* assume any amount of incoming bandwidth */,
                0      /* assume any amount of outgoing bandwidth */);
        if (Sendclient == NULL)
        {
                printf ("An error occurred while trying to create an ENet client host.\n");
                return -1;
        }
        pthread_attr_init(&attr);

        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
        pthread_create(&gudpThread,&attr,(void*)ENet_Recv_Task,NULL);
        pthread_attr_destroy(&attr);
        if(gudpThread == 0)
                printf("无法创建enet接收线程\n");
        g_init = 1;
        return 0;
}

int   App_UninitENet()
{
	g_thread_exit = 1;
	appcallbacks.AppRecvCallback = NULL;  
	appcallbacks.AppRecvCallback2 = NULL;
	enet_host_destroy(Sendclient);
	Sendclient = NULL;
	usleep(40*1000);
	pthread_cancel(gudpThread);
	gudpThread = 0;
	g_init = 0;

	return 0;
}

static ENetPeer* sconnect(ENetHost * client, char *host, int port) {
        ENetAddress address;
        ENetEvent event;
        ENetPeer *peer;

        /* Connect to some.server.net:1234. */
        //enet_address_set_host (& address, "10.0.0.103");
        enet_address_set_host (& address, host);
        address.port = port;

        /* Initiate the connection, allocating the two channels 0 and 1. */
        peer = enet_host_connect (client, & address, 2);    

        if (peer == NULL)
        {
                return NULL;
        }

        /* Wait up to 5 seconds for the connection attempt to succeed. */
        if (enet_host_service (client, & event, 300) > 0 &&
                event.type == ENET_EVENT_TYPE_CONNECT)
        {
                return peer;
        }

        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (peer);


        return NULL;
}

static ENetPeer* sconnect2(ENetHost * client, unsigned long ip, int port) {
        ENetAddress address;
        ENetEvent event;
        ENetPeer *peer;

        /* Connect to some.server.net:1234. */
        //enet_address_set_host (& address, "10.0.0.103");
       // enet_address_set_host (& address, host);
        address.port = port;
        address.host = ip;

        /* Initiate the connection, allocating the two channels 0 and 1. */
        peer = enet_host_connect (client, & address, 2);    

        if (peer == NULL)
        {
                return NULL;
        }

        /* Wait up to 5 seconds for the connection attempt to succeed. */
        if (enet_host_service (client, & event, 300) > 0 &&
                event.type == ENET_EVENT_TYPE_CONNECT)
        {
                return peer;
        }

        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset (peer);


        return NULL;
}

static void sdisconnect(ENetHost *client, ENetPeer *peer) 
{
        ENetEvent event;
        enet_peer_disconnect (peer, 0);

        /* Allow up to 3 seconds for the disconnect to succeed
        and drop any packets received packets.
        */
        while (enet_host_service (client, & event, 300) > 0)
        {
                switch (event.type)
                {
                case ENET_EVENT_TYPE_RECEIVE:
                        enet_packet_destroy (event.packet);
                        break;

                case ENET_EVENT_TYPE_DISCONNECT:
                        printf("Disconnection succeeded.");
                        return;
                default:
					break;
                }
        }

        /* We've arrived here, so the disconnect attempt didn't */
        /* succeed yet.  Force the connection down.             */
        enet_peer_reset (peer);
}

int App_Enet_Send(char* ip,short port,void* buf,int len)
{
        ENetPeer* server = NULL;
        ENetPacket *packet;
        ENetEvent   event;
       	if(g_init == 0)
            return -2;

        if(len <= 0)
                return -2;
        if(buf == 0)
                return -3;

        if(port == 0)
                port = 20302;

        if(server == NULL)
                server = sconnect(Sendclient, ip,port);	 
        if(server == NULL) {
                printf("Could not connect,errno = %d.\n",errno);
                return -1;
        }

        packet = enet_packet_create (buf, 
                len, 
                ENET_PACKET_FLAG_RELIABLE);

        enet_peer_send (server, 0, packet);

        if(enet_host_service (Sendclient, &event, 1000) < 0){
                sdisconnect(Sendclient, server);

                server = NULL;
                return -1;
        }
        sdisconnect(Sendclient,server);
        return 0;

}

int App_Enet_Send2(unsigned long ip,short port,void* buf,int len)
{
        ENetPeer* server = NULL;
        ENetPacket *packet;
        ENetEvent   event;
       	if(g_init == 0)
            return -2;

        if(len <= 0)
                return -2;
        if(buf == 0)
                return -3;

        if(port == 0)
                port = 20302;

        if(server == NULL)
                server = sconnect2(Sendclient, ip,port);	 
        if(server == NULL) {
                printf("Could not connect,errno = %d.\n",errno);
                return -1;
        }

        packet = enet_packet_create (buf, 
                len, 
                ENET_PACKET_FLAG_RELIABLE);

        enet_peer_send (server, 0, packet);

        if(enet_host_service (Sendclient, &event, 1000) < 0){
                sdisconnect(Sendclient, server);

                server = NULL;
                return -1;
        }
        sdisconnect(Sendclient,server);
        return 0;

}
