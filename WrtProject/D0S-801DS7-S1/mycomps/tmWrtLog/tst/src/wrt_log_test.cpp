/*!
 *      \file           helloWorld.c
 *
 *      This file is designed to demonstrate the basic structure
 *      of a simple program in DVP 4.
 *
 */
/*-----------------------------------------------------------
 *
 *      %version:       9 %
 *      instance:       DS_4
 *      %date_created:  Mon Sep 09 18:11:08 2007 %
 *
 */
 //-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "wrt_log.h"  
#include "zenmalloc.h"


void test_log()
{
      //  WRT_MESSAGE(wrt_log");

        WRT_DEBUG("WRT_DEBUG");
        WRT_DEBUG("%s","WRT_DEBUG 1");

        WRT_WARNING("WRT_WARNING");
        WRT_WARNING("%s","WRT_WARNING 1");

        WRT_ERROR("WRT_ERROR");
        WRT_ERROR("%s","WRT_ERROR 1");

}

int  main(int argc,char * argv[])  // expands to right framework for all OS with init.  See tmMain.h
{
    int level = LOG_MESSAGE|LOG_DEBUG|LOG_WARNING|LOG_ERROR;
    int i = 0;
    WrtLogCallbacks mycallback;

    printf("------------------------------------------\n");
    mycallback.malloc = zenMalloc;
    mycallback.free = zenFree;
    mycallback.realloc = zenRealloc;
    wrt_set_log_memory_adapter(&mycallback);
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    wrt_log_init();
   // WRT_MESSAGE("设置日志服务器的地址和端口以及编号");
    wrt_set_log_server_addr(0x2001a8c0,60000);
    wrt_set_log_id("010010100100101");
 	printf("+++++++++++++++++++++++\n");
    wrt_set_log_remote(1); //开启远程服务，该函数可以动态调 
	for(i =0; i<100;i++){

	    WRT_MESSAGE("test WRT_MESSAGE");


	    WRT_MESSAGE("%s","test WRT_MESSAGE Two");

		WRT_DEBUG("test WRT_DEBUG");
	    WRT_DEBUG("%s","test WRT_DEBUG Two");

	    WRT_WARNING("test WRT_WARNING");
	    WRT_WARNING("%s","test WRT_WARNING Two");

	    WRT_ERROR("test WRT_ERROR");
	    WRT_ERROR("%s", "test WRT_ERROR Two");
	    SDL_Delay(1000);
	}
    getchar();
    wrt_log_uninit();
    
    return 0;
    /*
   
    level |= LOG_DEBUG;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("add debug level LOG_WARNING");
    level |= LOG_WARNING;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("add debug level LOG_ERROR");
    level |= LOG_ERROR;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();
   

    WRT_MESSAGE("cancel debug levelLOG_DEBUG");
    level &= ~LOG_DEBUG;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("cancel debug level LOG_WARNING");
    level &= ~LOG_WARNING;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("cancel debug level  LOG_ERROR");
    level &= ~LOG_ERROR;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("set debug level LOG_END");
    level |= LOG_END;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

   
    level &= ~LOG_END;
    level |=LOG_MESSAGE;
    wrt_set_log_level_mask(level);//该函数可以动态调用
    WRT_MESSAGE("enter set remote");
    getchar();
    wrt_set_log_remote(1); //开启远程服务，该函数可以动态调 
    

   
    WRT_MESSAGE("add debug level LOG_DEBUG");
    level |= LOG_DEBUG;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("add debug level LOG_WARNING");
    level |= LOG_WARNING;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("add debug level LOG_ERROR");
    level |= LOG_ERROR;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();


    WRT_MESSAGE("cancel debug level  LOG_DEBUG");
    level &= ~LOG_DEBUG;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("cancel debug level  LOG_WARNING");
    level &= ~LOG_WARNING;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("cancel debug level  LOG_ERROR");
    level &= ~LOG_ERROR;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();

    WRT_MESSAGE("set debug level  LOG_END");
    level |= LOG_END;
    wrt_set_log_level_mask(level);//该函数可以动态调用。
    test_log();


    level &= ~LOG_END;
    level |=LOG_MESSAGE;
    wrt_set_log_level_mask(level);//该函数可以动态调用
    WRT_MESSAGE("test over");
*/
    
    getchar();


  
}           
            
