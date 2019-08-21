#define TEST_HTTP_DRIVER 0
#define TEST_WEB_SERVER  1
#undef  TEST_WEB_SERVER

#include <stdio.h> 

#include <errno.h> 
#include <dirent.h>  
#include <fcntl.h> 

#include <arpa/inet.h>  //for in_addr     
#include <linux/rtnetlink.h>    //for rtnetlink     
#include <net/if.h> //for IF_NAMESIZ, route_info     
#include <stdlib.h> //for malloc(), free()     
#include <string.h> //for strstr(), memset()     

#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#if 0
#include "wrtfont.h"
#include "c_interface.h"
#include "ctrlcmd.h"    
#include "Handler.h"
#include "audiovideo.h"      
#include "main_program.h"  
#include "jpeg.h"   
#include "image.h"
#include "window.h" 
#include "zenmalloc.h"
#include "RoomGpioUtil.h"
#include "tslib.h"  

#include "fbutils.h"
#include "testutils.h"
#include "wrt_audio.h"
#include "RoomGpioUtil.h"
#include "tmSysConfig.h"
#include "c_interface.h"

#include "wrt_audiostream.h"

#include "sstardisp.h"
#include "mi_disp.h"

#include "wrtTcp.h"
#endif
#define PROTOCOL_VERSION2
#include "wrtTcpShare.h"
#include "DoorStruct.h"
#include "DoorCmd.h"
#include "tmSysConfig.h"
#define iptable_name "wrtdata/iptable"
IpAddrTableInfo gIpRoomTable;
void systemGetLocalID(char *localID);
int readnetdata(unsigned long sd, unsigned char *pdest, int maxsize)
{ 
	fd_set readfd;
	unsigned long max_sd;
	//int mode=65536;
    struct timeval timeout;
	int readbytes=0,ret=0,currecvsize=512;
	char buf[512]="";
	timeout.tv_sec =0;
	timeout.tv_usec = 200000;
	max_sd = sd+1;
	//setsockopt(sd,SOL_SOCKET,SO_RCVBUF,(const char*)&mode,sizeof(int));
	//printf("%08x\n",userdata);
	for(;;){
		if(readbytes>=maxsize)
			break;
		FD_ZERO(&readfd);
		FD_SET(sd,&readfd);
		if(select(max_sd,&readfd,NULL,NULL,&timeout)<0){
			printf("select <0: readbytes: %d maxsize:%d\r\n",readbytes,maxsize);
		}
		if(FD_ISSET(sd,&readfd)){
			currecvsize= (maxsize-readbytes>512)?512:(maxsize-readbytes);
			ret = recv(sd, (char*)buf, currecvsize, 0);	
			if(ret<=0){
				printf("readnetdata error, ret:%d, errno: %04x, read:%d, maxsize: %d\r\n",ret,errno,readbytes,maxsize);
				if(errno==0x5023||errno==0x5024){
					continue;
				}
				goto READ_END;
			}
			//printf("readnetdata recv:%d, error: 0x%04x, read:%d, maxsize: %d\n",ret,errno,readbytes,maxsize);
			memcpy((void*)(pdest+readbytes),buf,ret);
			readbytes+=ret;
		}
	}
READ_END:
	printf("readnetdata recv:%d, error: 0x%04x, read:%d, maxsize: %d\r\n",ret,errno,readbytes,maxsize);
	return readbytes;
}
int ReadRoomIpTable()
{
    struct stat st_buf;
    int count=0;
    FILE* ip_fd = NULL;
    memset(&st_buf,0,sizeof(struct stat));
	//for(i = 0;i < OTHER_INDOORMACHINE_NUM;i++){otherIndoorMachineTableNum[i] = initInt;}
	if((ip_fd = fopen(iptable_name, "rb")) == NULL){
		printf("\tCannot open ipaddress table\n");
		return -2;
	}
	if (fstat(fileno(ip_fd), &st_buf) != -1){printf("stat successfully size = %ld\r\n",st_buf.st_size);}
	else {printf("ReadRoomIpTable error: stat failed,errno = %d\r\n",errno);}



	
    //if(fstat(iptable_name,&st_buf)){
	//	printf("ReadRoomIpTable error: stat failed,errno = %d\r\n",errno);
    //    return -1;
	//	}

    if(st_buf.st_size>0){
        gIpRoomTable.nRoomNum = st_buf.st_size/sizeof(IpAddrTable);
        printf("ReadRoomIpTable gIpRoomTable.nRoomNum:%d count:%d\n",gIpRoomTable.nRoomNum,count);
        if(gIpRoomTable.nRoomNum>50000){
			printf("ReadRoomIpTable error: room number is greater than 50000\r\n");
            return -1;
			}
        
        if(gIpRoomTable.nRoomNum>0){
            if(gIpRoomTable.gIpAddrTable){
                gIpRoomTable.gIpAddrTable = (IpAddrTable*)realloc(gIpRoomTable.gIpAddrTable,gIpRoomTable.nRoomNum*sizeof(IpAddrTable));
            }else{
                gIpRoomTable.gIpAddrTable = (IpAddrTable*)malloc(gIpRoomTable.nRoomNum*sizeof(IpAddrTable));
            }
            
            //if((ip_fd = fopen(iptable_name, "rb")) == NULL)
            //{
            //    printf("\tCannot open ipaddress table\n");
            //    return -2;
            //}
            
            if(ip_fd){
            	fseek(ip_fd, 0, SEEK_SET);
            	count = fread(&gIpRoomTable.nRoomNum, 4, 1, ip_fd);
            	printf("ReadRoomIpTable gIpRoomTable.nRoomNum:%d count:%d\n",gIpRoomTable.nRoomNum,count);
            	if(count<=0){
            	    fclose(ip_fd);
            	    return -3;
            	}
                count = fread(gIpRoomTable.gIpAddrTable, gIpRoomTable.nRoomNum*sizeof(IpAddrTable), 1, ip_fd);
                printf("count=%d\n",count);
                fclose(ip_fd);
            }else
                return -4;
        }else
            return -1;
    }
    if(count<=0)
        return -5;
	//findOutEveryIndoorMachineIP(LOCALID, gIpRoomTable.nRoomNum, gIpRoomTable.gIpAddrTable);
	//SendWarning(LOCALID,0,ROOM_LEAVE_HOME);
    return 0;

}
/*
findOutEveryIndoorMachineIPForCall:
find out every indoor machine ip in order to send call
*/
int findOutEveryIndoorMachineIPForCall(const char *roomid, int nNum, const IpAddrTable *iptable)
{
    int imid, ihigh, ilow, ret,i = 0;
    ilow = 0;
    ihigh = nNum;
	//GetIpByID(roomid);
	//GetIpByID("010010100100102");
	char firstRoomID[16] = {'\0'};
	memcpy(&firstRoomID[0],roomid,13);
	memcpy(&firstRoomID[13],"01",2);//this function find out the first indoor machine and then the next one will be other indoor machine in the same room
	printf("findOutEveryIndoorMachineIP:first roomid:%s\r\n",firstRoomID);
	if (!nNum){printf("findOutEveryIndoorMachineIP:there is not a room in ip address table\r\n"); return -1;}
	//for(i = 0;i < OTHER_INDOORMACHINE_NUM;i++){callIndoorMachineTableNum[i] = initInt;}
	i = 0;
    do
    {
        imid = (ilow + ihigh) / 2;
        printf("ilow=%d,ihigh=%d,iptable[%d].name= %s,iptable[%d].ip= %08x\n", ilow, ihigh, imid, iptable[imid].name, imid, (unsigned int)iptable[imid].ip);
        ret = strncmp(firstRoomID, iptable[imid].name, 15);
        if(ret > 0){
            ilow = imid + 1;
        }
        else if(ret < 0){
            ihigh = imid - 1;
        }
        else{//found out one of them
        	//printf("found out one of them imid = %d\r\n",imid);
        	for(i = 0;i < 4;imid ++)
			//for(i = 0;i < OTHER_INDOORMACHINE_NUM;imid ++)
        		{
        		if(strncmp(firstRoomID, iptable[imid].name, 13) == 0){
					//callIndoorMachineTableNum[i] = imid;//save the number which will be used when we send defence situation to the other indoor machine in the same room.
					printf("find out %d indoor machine,ip is 0x%x\r\n",i,(unsigned int)iptable[imid].ip);
					i ++;
					}
				else break;
				//imid ++;
        		}
			return ture;
			} 
    }
    //while(ilow <= ihigh && i < OTHER_INDOORMACHINE_NUM);
    while(ilow <= ihigh && i < 4);
    return ture;
}

int tcpServerCallbackReciveAndSaveAddressTable(cJSON *json,char *structBody)
{
	int nIpNum =0,size=0,readcount=0;
	char *pdata;
	FILE *ip_fd = NULL;
	int wl = 0;
	int usedbytes,sd;
	struct _net_pack_ *np = (struct _net_pack_ *)structBody;
	if((usedbytes = acquireNumberFromCjsonAbsolutely(json,CJSON_TCP_USED_BYTES))==DEFAULT_FAILED_INT){printf("usedbytes error:can't be found\r\n");return false;}
	if((sd = acquireNumberFromCjsonAbsolutely(json,CJSON_IP_SOCKET_FD))==DEFAULT_FAILED_INT){printf("sd error:can't be found\r\n");return false;}
	memcpy(&nIpNum,np->CMD_DATA,4);
	size = nIpNum*sizeof(IpAddrTable);
	if(size > 1024*1024){
		printf("recv ipaddrtable pak too large\n");
		return false;
	}	
	pdata = (char *)malloc(size+128);
	printf("table size = %d, used = %d src = %s\n", size, usedbytes, np->srcid);
	if(pdata){
		memcpy((void*)pdata, (void*)(np->CMD_DATA+4), usedbytes-HEAD_LENGTH-4);
		if(size>(usedbytes-HEAD_LENGTH-4)){
			readcount = readnetdata(sd,(unsigned char*)(pdata+usedbytes-HEAD_LENGTH-4),size-(usedbytes-HEAD_LENGTH-4));
		}
		if((readcount+usedbytes-HEAD_LENGTH-4)==size){
			printf("download successfully first pack: %d, firstdata: %d,subcount %d, all: %d\n",usedbytes,usedbytes-HEAD_LENGTH-4, readcount,size);
		}
	}	
	if((ip_fd = fopen(iptable_name, "wb")) == NULL){
		printf("open %s error\r\n",iptable_name);
		return -2;
	}
	else{
		//int readcount=0;
		fseek(ip_fd, 0, SEEK_SET);
		fwrite(&nIpNum, 4 , 1, ip_fd);
		wl = fwrite( pdata, 1 , size, ip_fd);
		printf("wl = %d \n",wl);
		if(wl != size){
				printf("fwrite error %x\r\n",errno);
				return -3;
		}
	printf("handler_addrtable: fwrite the address successfully\r\n");
		fflush(ip_fd);
		fclose(ip_fd);
		free(pdata);
		pdata = NULL;
		ip_fd = NULL;
		usleep(30);
	}
	usleep(30);
	ReadRoomIpTable();	//
	//findOutEveryIndoorMachineIPForCall("010010100100101", gIpRoomTable.nRoomNum, gIpRoomTable.gIpAddrTable);
	return ture;
}
int tcpServerCallbackGetLocalID(cJSON **jsonReturn)
{
	printf("tcpServerCallbackGetLocalID\r\n");
	char localID[16]={'\0'};
	systemGetLocalID(localID);
	localID[15] = '\0';
	cJSON_AddStringToObject(*jsonReturn,CJSON_LOCAL_ID,localID);
	return ture;
}
int tcpServerToMainCallback(cJSON *json,cJSON **jsonReturn,char *structBody,char**structBodyReturn)
{
	printf("tcpServerToMainCallback\r\n");
	int out = false;
	if(structBodyReturn == NULL){printf("structBodyReturn:unused parameter\r\n");}
	if(jsonReturn == NULL){printf("jsonReturn:unused parameter,please send cJSON **jsonReturn to me\r\n");return false;}
	else{*jsonReturn = cJSON_CreateObject();}
	if (!json) {
		printf("NULL json pointer\r\n");
		cJSON_AddNumberToObject(*jsonReturn, CJSON_DATE_returnCode, CJSON_ERROR_illegalDate);
		return CJSON_ERROR_illegalDate;
	}
 	else{
 		int cmd = DEFAULT_FAILED_INT;
		if((cmd = acquireNumberFromCjsonAbsolutely(json,CJSON_DATE_CMD)) == DEFAULT_FAILED_INT){//missing data"cmd"
			cJSON_AddNumberToObject(*jsonReturn, CJSON_DATE_returnCode, CJSON_ERROR_canNotAcquireCmd);
			out = CJSON_ERROR_canNotAcquireCmd;
			goto END;
			}
		switch(cmd){
			case CJSON_CMD_GET_IP_TABLED_FILE:
				//cJSON_AddStringToObject(*jsonReturn,CJSON_IP_TABLE_FILE,"/home/iptable");
				break;
			case CJSON_CMD_HANDLE_IP_ADDRESS_TABLE:
				//int socketFd = 0;
				out = tcpServerCallbackReciveAndSaveAddressTable(json,structBody);
				break;
			case CJSON_CMD_GET_LOCAL_ID:
				out = tcpServerCallbackGetLocalID(jsonReturn);
				break;
			default:
				cJSON_AddNumberToObject(*jsonReturn, CJSON_DATE_returnCode, CJSON_ERROR_cmdIllegal);
				out = CJSON_ERROR_cmdIllegal;
				goto END;
				break;
			}
 	}
	END:
	return out;
}


