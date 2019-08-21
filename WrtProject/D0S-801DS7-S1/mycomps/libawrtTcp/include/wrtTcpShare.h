/*
wrtTcpShare.h:  build by xiao
this file is shared for any one who want to use the library. 
attention:never put any private parameter in this file.
*/
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <errno.h>
#include "wrtCjsonShare.h"
#ifdef __cplusplus
extern "C"{
#endif
#pragma pack(1)
int initTcp(int (*tcpServerToMainCallBack)(cJSON *,cJSON **,char *,char**));
void returnMsgProcessor(char **returnCjsonMsg,char *basalString,char *jsonString,int returnCode);
//int acquireNumberFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName);
#ifdef __cplusplus
}
#endif
#pragma pack()

