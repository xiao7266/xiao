#ifndef WRT_LOG_H
#define WRT_LOG_H

#ifdef __cplusplus
extern "C"{
#endif	

//         WRTIP系统日志模块
//                 1：为了方便调试，以及查看系统运行情况。设计一个通用的系统日志模块，
//                 输出到控制台或者远程服务器。
// 
//                 2：系统日志采用格式：
// 
//                 //通用日志格式
//                 #日志类型(级别)  系统ID号(15字节)  日志内容 日期
// 
//                 //调试(错误/警告)日志格式
//                 #日志类型(级别)  系统ID号(15字节) 日志内容 产生日志文件名:行号 日期
// 
// 
//                 系统ID号：分机，主机，管理等设备的ID号，目前WRTIP采用15字节字符。
//                 日志类型：
//                 1)系统消息(WRT_MESSAGE) 主要记录系统运行状态，流程。
//                 2)系统调试消息(WRT_DEBUG)  详细记录系统每一步流程。
//                 3)系统警告消息(WRT_WARNING) 记录系统运行时出现的一些错误，该错误又不影响系统的
//                 运行。
//                 4)系统错误消息(WRT_ERROR) 主要记录系统运行时的致命错误，比如内存分配失败，溢出，
//                 以及导致系统不正常运行错误。
//                  3：输出到远程服务器时，默认采用端口60000
// 
//                    使用宏定义时需要在后面添加分号。
//                    如果需要输出文件名和行号请使用宏输出.消息内容后面不需要加\n或者\r\n.
//                    WRT_ERROR("内存分配失败");
//                    #wrt-error 010010100100101 内存分配失败 cmd_handler.cpp:1234 2011/3/30 12:07 
//                    
// 使用列子:
//         WrtLogCallbacks mycallback;
//         mycallback.malloc = mymalloc;
//         mycallback.free = myfree;
//         mycallback.realloc = myrealloc;
//         wrt_set_log_memory_adapter(&mycallback);
//         wrt_set_log_id("010010100100101");
//         wrt_set_log_level_mask(LOG_MESSAGE);//该函数可以动态调用。可以多个级别同时：LOG_MESSAGE|LOG_DEBUG
//         wrt_log_init();
//         wrt_set_log_server_addr(0x2001a8c0,60000);
//         wrt_set_log_remote(1); //开启远程服务，该函数可以动态调用。

//使用注意 不能在正在打印输出的时候设置LOGLEVEL
#include <stdarg.h>

typedef enum {
	LOG_MESSAGE = 0x1,
	LOG_WARNING = 0x10,
	LOG_DEBUG = 0x100,
	LOG_ERROR = 0x1000,
	LOG_END = 0x10000
} WrtLogLevel;

typedef struct
{
    void * (*malloc) (size_t size);
    void  ( *free) (void * memory);
    void*  ( *realloc) (void* memory,size_t size);
}WrtLogCallbacks;


typedef void (*WrtLogFunc)(WrtLogLevel lev, const char *fmt, va_list args,const char* _file,const int _line);

extern WrtLogFunc wrt_logv_out;

extern unsigned int wrt_log_mask;


#define wrt_log_level_enabled(level)	(wrt_log_mask & (level))


#define wrt_logv(level,fmt,args,_file,_line) \
        do{\
        if (wrt_logv_out!=NULL && wrt_log_level_enabled(level)) \
        wrt_logv_out(level,fmt,args,_file,_line);\
        }while(0)

//替换日志处理函数，默认函数指针输出日志到stderr或者远程服务器。
void wrt_set_log_handler(WrtLogFunc func);

//设置日志调试级别。调试级别定义请看WrtLogLevel。如果设置为LOG_END,则不输出任何日志
void wrt_set_log_level_mask(int levelmask);

//设置分机/主机/其他机器的ID号。默认全部为字符0.
void wrt_set_log_id(char* id);

//初始化日志模块，主要启动发送日志到远程服务器的任务。
void wrt_log_init();

//退出日志模块，主要退出网络发送任务。
void wrt_log_uninit();

//设置远程服务器的地址和端口，如果port = 0 则采用默认port=60000;
void wrt_set_log_server_addr(unsigned long ip,short port);

//开启发送日志到远程服务器，在调用该函数之前请确保wrt_set_log_server_addr已经调用。
//1:开始，0:关闭。
void wrt_set_log_remote(int _start);
/************************************************************************/
/* 替换日志系统的内存处理模块，一旦调用该函数，则三个内存处理函数都不能为
   空                                                                   */
/************************************************************************/
void wrt_set_log_memory_adapter(WrtLogCallbacks* _callback); 

int wrt_get_log_remote();

void wrt_debug(char* _file,int _line,const char *fmt,...);
void wrt_message(const char *fmt,...);
void wrt_warning(char* _file,int _line,const char *fmt,...);
void wrt_error(char* _file,int _line,const char *fmt,...);


#define WRT_DEBUG(x...)  wrt_debug(__FILE__,__LINE__,##x)
#define WRT_MESSAGE(x...) wrt_message(x)
#define WRT_WARNING(x...) wrt_warning(__FILE__,__LINE__,##x)
#define WRT_ERROR(x...)  wrt_error(__FILE__,__LINE__,##x)


#ifdef __cplusplus
}
#endif	

#endif


