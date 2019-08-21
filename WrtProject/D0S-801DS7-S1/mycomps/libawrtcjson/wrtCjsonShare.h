#include "cJSON.h"
#include <dirent.h>
#include <errno.h>

char *acquireStringFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName);
int acquireNumberFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName);
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
#define CJSON_LOCAL_ID		                   		"localId"
#define CJSON_TARGET_WANT_TO_CALL              		"target"
#define CJSON_TARGET_ID			              		"targetId"
#define CJSON_TARGET_IP			              		"targetIp"

#define CJSON_CMD_GET_IP_TABLED_FILE				100
#define CJSON_CMD_HANDLE_IP_ADDRESS_TABLE			101
#define CJSON_CMD_GET_LOCAL_ID						102
#define CJSON_CMD_MAKE_CALL							103
#define CJSON_CMD_CALL_CENTER						104
