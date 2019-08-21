#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <enet/enet.h>

//extern "C" int App_UninitENet();
void recv_call_back(unsigned long ip ,short port,void* buf,int len)
{
	printf("recv ip = 0x%08x, port = %d ,datalen = %d \r\n",ip,port,len);
//	printf("recv data \r\n");
}

unsigned char*  test_pointer(unsigned char** pt)
{
   ( *pt) += 5;
    return *pt;
}

int  main(int argc,char* argv[])
{
	 char  c;
         char buf[48];
         int  len = 48;
         short cmd = 0x1000;
         int walarmtype = 0;
         int num = 1;
  
	AppENetCallbacks callbacks;
	callbacks.AppRecvCallback = NULL;
	callbacks.AppRecvCallback2 = recv_call_back;

	App_InitENet(0,0,&callbacks);
#if 1
        memset(buf,0,48);
        strcpy(buf,"WRTI");
        memcpy(buf+4,&len,4);
        cmd = ENET_HOST_TO_NET_16(cmd);
        memcpy(buf+8,&cmd,2);
        strncpy(buf+10,"010010100100101",15);
        memcpy(buf+40,&walarmtype,4);
        memcpy(buf+44,&num,4);
#endif
        //memset(tbuf,0,11);
       // strcpy(tbuf,"test1test2");
       // tbuf2 = tbuf;
       // printf("%x,%x\n",tbuf2,test_pointer(&tbuf2));
       // printf("%x\n",tbuf2);
        printf("init Enet over start recv data\r\n");
	printf("enter 'q' is exit,other send 'test' to 192.1678.1.32\r\n");
        while(1)
	{
              c = getchar();
	      if(c == 'q')
         	  break;
              printf("send ret = %d\n", App_Enet_Send("192.168.1.32",0,buf,48));
              
	}
        App_UninitENet();
	return 0;
	
}
