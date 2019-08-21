
#include <stdio.h>
#include <stdlib.h>
#include "httpclient.h"

void main(void)
{
        int filesize = 0;
        int ws = 0;
        int total = 0;
        char* filebuf = NULL;
        int fd = HttpClientOpen("http://192.168.1.32/1.mi");
        if(fd < 0)
                printf("HttpclientOpen failed \n");
        filesize = HttpClientGetContentLength(fd);
        if(filesize < 0){
                HttpClientClose(fd);
                printf("HttpClientGetContentLength failed \n");
                return;
        }
        filebuf = (char*)malloc(filesize);
        if(filebuf == NULL)
        {
                HttpClientClose(fd);
                printf("malloc failed \n");
                return;
        }
        do
        {
                if ((ws  = HttpClientRead(fd,filebuf+total,filesize-total))>0){
                        total+=ws;
                }

        }while(ws > 0);
        printf("read httpData ok ....size = %d\n",total);
        free(filebuf);
        HttpClientClose(fd);	
}

