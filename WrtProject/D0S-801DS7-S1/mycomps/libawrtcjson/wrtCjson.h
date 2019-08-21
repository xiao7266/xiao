#include "cJSON.h"
#include <dirent.h>
#include <errno.h>

//#define android //define it while the library is used in android system
#ifdef  android
#include <android/log.h>
#include <semaphore.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

// log标签
#define  TAG    "cjson"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#include <limits.h> //which define ULONG_MAX
#define SQL_STR_REALLOC		0x1				/* Reallocate the buffer as required */
#define SQL_STR_INC			64				/* Growth increment */
#define sqlMax(a,b)  (((a) > (b)) ? (a) : (b))
#define sqlMin(a,b)  (((a) < (b)) ? (a) : (b))
typedef struct {
	char	*s;							/* Pointer to buffer */
	int		size;						/* Current buffer size */
	int		max;						/* Maximum buffer size */
	int		count;						/* Buffer count */
	int		flags;						/* Allocation flags */
} strbuf_t1;
/*
 *	These values are not prefixed so as to aid code readability
 */

typedef enum {
	undefined	= 0,
	byteint		= 1,
	shortint	= 2,
	integer		= 3,
	hex			= 4,
	percent 	= 5,
	octal		= 6,
	big			= 7,
	flag		= 8,
	floating	= 9,
	string 		= 10,
	bytes 		= 11,
	symbol 		= 12,
	errmsg 		= 13
} vtype_t1;
/*
 *	Sprintf formatting flags
 */
enum flag1 {
	flag_none = 0,
	flag_minus = 1,
	flag_plus = 2,
	flag_space = 4,
	flag_hash = 8,
	flag_zero = 16,
	flag_short = 32,
	flag_long = 64
};
#define PRINTF_BUF_SIZE 5000
#define SQL_PRINTF_ANDROID "ecso"
void sql_put_char(strbuf_t1 *buf, char c);
int myprintf(char * fmt,...);
int networkPrintfPipe[2];
//sem_t  networkPrintfPipeSem;
pthread_t networkPrintfTid;
int printFd = 0;
#define PICTURE_BASE64_URL "/sdcard/web2"
#else
//for linux
#define myprintf  printf//function "myprintf" for android and "pirntf" for linux
#define PICTURE_BASE64_URL "."
#endif

//unsigned long MENU_ERROR = 0;
#define URL_LEN 128
//char URL[URL_LEN]={'\0'};
cJSON *menuJson;
//char *menuJsonText = NULL;
#if 0 
char *acquireStringFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName);

#define DEFAULT_FAILED_INT 		-1
#define TYPE_STANDARD_STATE 	0
#define ture                      							0
#define false												-1
#define MENU_ERROR_urlExceedLimit 							1
#define MENU_ERROR_canNotOpenSqlitedb 						2
#define MENU_ERROR_canNotCreateJsonText 					3
#define CJSON_ERROR_cjsonParseError 						1001
#define CJSON_ERROR_canNotAcquireCmd 						1002
#define CJSON_ERROR_cmdIllegal 								1003
#define CJSON_ERROR_missingDate								1004
#define CJSON_ERROR_illegalDate								1005


#define CjsonError_returnMsg "{\
							   \"return\": \"false\"}"
#define CjsonSucceeded_returnMsg "{\
								   \"return\": \"ture\"}"
#define VERSION "1.00.00"
#define SQL_STATEMENT_EXEC_END		""
#define CJSON_DATE_MISSING 							0

#define RETURN_MSG_LEN								256
#define SQL_STATEMENT_LEN							256
#define CJSON_DATE_DATE 							"date"
#define CJSON_DATE_CMD                      		"cmd"
#define CJSON_DATE_ErrMsg                      		"ErrMsg"
#define CJSON_DATE_Detail                      		"detail"
#define CJSON_DATE_returnCode                   	"code"
#define CJSON_DATE_COUNT		                   	"count"
#define CJSON_TCP_FD		                   		"tcpFd"
#define CJSON_TCP_USED_BYTES                   		"usedbytes"
#define CJSON_IP_TABLE_FILE                   		"ipTableFile"
#define CJSON_IP_SOCKET_FD                   		"tcpSocketFd"

#define CJSON_CMD_GET_IP_TABLED_FILE				100
#define CJSON_CMD_HANDLE_IP_ADDRESS_TABLE			101
#endif
