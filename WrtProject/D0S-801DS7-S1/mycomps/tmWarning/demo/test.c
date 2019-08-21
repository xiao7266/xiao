#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zenmalloc.h"
#include "warningtask.h"


int  main(int argc,char* argv[])
{
	char c;
        zenMemInit(0);
	printf("enter 'q' exit \n");
        WarningInit(0,0x2001a8c0,0,0,0,0);
        while(1)
        {
        	c = getchar();
        	if(c == 'q')
        		break;
        	printf("send waring ....\n");
        	SendWarning("010010100100101",0);
        }
	return 0;
	
}
