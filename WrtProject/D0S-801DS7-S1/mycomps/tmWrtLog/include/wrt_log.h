#ifndef WRT_LOG_H
#define WRT_LOG_H

#ifdef __cplusplus
extern "C"{
#endif	

//         WRTIPϵͳ��־ģ��
//                 1��Ϊ�˷�����ԣ��Լ��鿴ϵͳ������������һ��ͨ�õ�ϵͳ��־ģ�飬
//                 ���������̨����Զ�̷�������
// 
//                 2��ϵͳ��־���ø�ʽ��
// 
//                 //ͨ����־��ʽ
//                 #��־����(����)  ϵͳID��(15�ֽ�)  ��־���� ����
// 
//                 //����(����/����)��־��ʽ
//                 #��־����(����)  ϵͳID��(15�ֽ�) ��־���� ������־�ļ���:�к� ����
// 
// 
//                 ϵͳID�ţ��ֻ���������������豸��ID�ţ�ĿǰWRTIP����15�ֽ��ַ���
//                 ��־���ͣ�
//                 1)ϵͳ��Ϣ(WRT_MESSAGE) ��Ҫ��¼ϵͳ����״̬�����̡�
//                 2)ϵͳ������Ϣ(WRT_DEBUG)  ��ϸ��¼ϵͳÿһ�����̡�
//                 3)ϵͳ������Ϣ(WRT_WARNING) ��¼ϵͳ����ʱ���ֵ�һЩ���󣬸ô����ֲ�Ӱ��ϵͳ��
//                 ���С�
//                 4)ϵͳ������Ϣ(WRT_ERROR) ��Ҫ��¼ϵͳ����ʱ���������󣬱����ڴ����ʧ�ܣ������
//                 �Լ�����ϵͳ���������д���
//                  3�������Զ�̷�����ʱ��Ĭ�ϲ��ö˿�60000
// 
//                    ʹ�ú궨��ʱ��Ҫ�ں�����ӷֺš�
//                    �����Ҫ����ļ������к���ʹ�ú����.��Ϣ���ݺ��治��Ҫ��\n����\r\n.
//                    WRT_ERROR("�ڴ����ʧ��");
//                    #wrt-error 010010100100101 �ڴ����ʧ�� cmd_handler.cpp:1234 2011/3/30 12:07 
//                    
// ʹ������:
//         WrtLogCallbacks mycallback;
//         mycallback.malloc = mymalloc;
//         mycallback.free = myfree;
//         mycallback.realloc = myrealloc;
//         wrt_set_log_memory_adapter(&mycallback);
//         wrt_set_log_id("010010100100101");
//         wrt_set_log_level_mask(LOG_MESSAGE);//�ú������Զ�̬���á����Զ������ͬʱ��LOG_MESSAGE|LOG_DEBUG
//         wrt_log_init();
//         wrt_set_log_server_addr(0x2001a8c0,60000);
//         wrt_set_log_remote(1); //����Զ�̷��񣬸ú������Զ�̬���á�

//ʹ��ע�� ���������ڴ�ӡ�����ʱ������LOGLEVEL
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

//�滻��־��������Ĭ�Ϻ���ָ�������־��stderr����Զ�̷�������
void wrt_set_log_handler(WrtLogFunc func);

//������־���Լ��𡣵��Լ������뿴WrtLogLevel���������ΪLOG_END,������κ���־
void wrt_set_log_level_mask(int levelmask);

//���÷ֻ�/����/����������ID�š�Ĭ��ȫ��Ϊ�ַ�0.
void wrt_set_log_id(char* id);

//��ʼ����־ģ�飬��Ҫ����������־��Զ�̷�����������
void wrt_log_init();

//�˳���־ģ�飬��Ҫ�˳����緢������
void wrt_log_uninit();

//����Զ�̷������ĵ�ַ�Ͷ˿ڣ����port = 0 �����Ĭ��port=60000;
void wrt_set_log_server_addr(unsigned long ip,short port);

//����������־��Զ�̷��������ڵ��øú���֮ǰ��ȷ��wrt_set_log_server_addr�Ѿ����á�
//1:��ʼ��0:�رա�
void wrt_set_log_remote(int _start);
/************************************************************************/
/* �滻��־ϵͳ���ڴ洦��ģ�飬һ�����øú������������ڴ洦����������Ϊ
   ��                                                                   */
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


