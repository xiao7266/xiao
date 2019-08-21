#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wrtCjson.h"
#include "wrtCjsonShare.h"
/*
acquireObjectFromCjsonAbsolutely:  穷尽数据获取json数据一个json格式的数据
objectName : 数据名称
返回json格式的数据字符串
*/
char *acquireObjectFromCjsonAbsolutely(cJSON *jsonSrc,char *objectName)
{
	cJSON *tmp;char *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		{
				if(tmp->type == cJSON_Object ){
					//printf("serching objectName :%s\n",tmp->string);
					if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
					else if((strlen(tmp->string)==strlen(objectName)) && (memcmp(objectName,tmp->string,strlen(objectName))==0)){
							printf("acquire cJSON_Object :%s\n",tmp->string);
							out=cJSON_Print(tmp);
							printf("%s:%s\n",tmp->string,out);
							break;
							}
					else{
						out = acquireObjectFromCjsonAbsolutely(tmp->child,objectName);//穷尽json数据
						if(out!=NULL) break;
						}
					tmp = tmp->next;
					}
				else{
					tmp = tmp->next;
					}
				
				if(tmp == NULL) break;
				}
		}
	return out;
	
}
/*
acquireStringFromCjsonAbsolutely:  根据一个名称穷尽数据获取json数据一个字符串数据
targetName : 数据名称
返回获取的数据，未找到数据返回NULL
*/
char *acquireStringFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName)
{
	cJSON *tmp;char *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		if(tmp->type == cJSON_Object){
			//printf("serching cJSON_Object :%s\n",tmp->string);
			if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
			else{
				out = acquireStringFromCjsonAbsolutely(tmp->child,targetName);//穷尽json数据
				if(out!=NULL) break;
				}
			tmp = tmp->next;
			}
		else if(tmp->type == cJSON_String){
			//printf("serching cJSON_String :%s\n",tmp->string);
			if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
				printf("acquire cJSON_String :%s\n",tmp->string);
				//out=print_string(tmp,0);
				out=tmp->valuestring;
				//out=cJSON_Print(tmp);
				printf("%s:%s\n",tmp->string,out);
				break;
				}
			tmp = tmp->next;
			}
		else{
			tmp = tmp->next;
			}
		if(tmp == NULL) break;
		}
	return out;
	
}
/*
acquireNumberFromCjsonAbsolutely:  根据一个名称穷尽数据获取json数据一个int型数据
targetName : 数据名称
返回获取的数据，未找到数据返回NULL
*/
int acquireNumberFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName)
{
	cJSON *tmp;
	int out = DEFAULT_FAILED_INT;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		if(tmp->type == cJSON_Object){
			//printf("serching cJSON_Object :%s\n",tmp->string);
			if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
			else{
				out = acquireNumberFromCjsonAbsolutely(tmp->child,targetName);//穷尽json数据
				if(out!=DEFAULT_FAILED_INT) break;
				}
			tmp = tmp->next;
			}
		else if(tmp->type == cJSON_Number){
			//printf("serching cJSON_String :%s\n",tmp->string);
			if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
				printf("acquire cJSON_Number :%s\n",tmp->string);
				//out=print_string(tmp,0);
				out=tmp->valueint;
				//out=cJSON_Print(tmp);
				printf("%s:%d\n",tmp->string,out);
				break;
				}
			tmp = tmp->next;
			}
		else{
			tmp = tmp->next;
			}
		if(tmp == NULL) break;
		}
	return out;
	
}
/*
setNumberToCjsonAbsolutely:  根据一个名称穷尽数据赋值json数据一个int型数据
targetName : 数据名称
setNumber:传入设置数据
返回获取的数据，未找到数据返回NULL
*/
int setNumberToCjsonAbsolutely(cJSON *jsonSrc,char *targetName,int setNumber)
{
	cJSON *tmp;
	int out = DEFAULT_FAILED_INT;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		if(tmp->type == cJSON_Object){
			//printf("serching cJSON_Object :%s\n",tmp->string);
			if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
			else{
				out = setNumberToCjsonAbsolutely(tmp->child,targetName,setNumber);//穷尽json数据
				if(out!=DEFAULT_FAILED_INT) break;
				}
			tmp = tmp->next;
			}
		else if(tmp->type == cJSON_Number){
			//printf("serching cJSON_String :%s\n",tmp->string);
			if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
				//printf("set cJSON_Number :%s\n",tmp->string);
				//out=print_string(tmp,0);
				out=tmp->valueint=setNumber;//没搞懂为什么要两个变量同时改
				out=tmp->valuedouble=setNumber;
				//out=cJSON_Print(tmp);
				//printf("%s:%lu\n",tmp->string,tmp->valueint);
				break;
				}
			tmp = tmp->next;
			}
		else{
			tmp = tmp->next;
			}
		if(tmp == NULL) break;
		}
	return out;
	
}

/*
acquireArrayPointerFromCjsonAbsolutely:  穷尽数据获取json数据一个数组数据的指针
targetArray : 数据名称
返回array格式的数据指针
*/
cJSON *acquireArrayPointerFromCjsonAbsolutely(cJSON *jsonSrc,char *targetArray)
{
	cJSON *tmp, *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		{
				if(tmp->type == cJSON_Object ){
					//printf("serching objectName :%s\n",tmp->string);
					if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
					else{
						out = acquireArrayPointerFromCjsonAbsolutely(tmp->child,targetArray);//穷尽json数据
						if(out!=NULL) break;
						}
					tmp = tmp->next;
					}
				else if(tmp->type == cJSON_Array){
					//printf("serching cJSON_String :%s\n",tmp->string);
					if((strlen(tmp->string)==strlen(targetArray)) && (memcmp(targetArray,tmp->string,strlen(targetArray))==0)){
						printf("acquire cJSON_Array :%s\n",tmp->string);
						//out=print_string(tmp,0);
						//out=tmp->valueint;
						out = tmp;
						break;
						}
					tmp = tmp->next;
					}
				else{
					tmp = tmp->next;
					}
				
				if(tmp == NULL) break;
				}
		}
	return out;
	
}
/*
acquireArrayFromCjsonAbsolutely:  穷尽数据获取json数据一个数组数据
targetArray : 数据名称
targetObject:函数外部指针接住cjson数据指针
返回array格式的数据字符串
*/
char *acquireArrayFromCjsonAbsolutely(cJSON *jsonSrc,char *targetArray,cJSON **targetObject)
{
	cJSON *tmp;char *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		{
				if(tmp->type == cJSON_Object ){
					//printf("serching objectName :%s\n",tmp->string);
					if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
					else{
						out = acquireArrayFromCjsonAbsolutely(tmp->child,targetArray,targetObject);//穷尽json数据
						if(out!=NULL) break;
						}
					tmp = tmp->next;
					}
				else if(tmp->type == cJSON_Array){
					//printf("serching cJSON_String :%s\n",tmp->string);
					if((strlen(tmp->string)==strlen(targetArray)) && (memcmp(targetArray,tmp->string,strlen(targetArray))==0)){
						printf("acquire cJSON_Array :%s\n",tmp->string);
						//out=print_string(tmp,0);
						//out=tmp->valueint;
						*targetObject = tmp;
						out=cJSON_Print(tmp);
						printf("%s:%s\n",tmp->string,out);
						break;
						}
					tmp = tmp->next;
					}
				else{
					tmp = tmp->next;
					}
				
				if(tmp == NULL) break;
				}
		}
	return out;
	
}

/*
acquireStringOfArrayFromCjsonAbsolutely:  获取数组的第n个数据
nextTarget: 指向下一个数据
返回第n个数据
*/
char *acquireStringOfArrayFromCjson(cJSON *jsonSrc,int n,cJSON **nextTarget)
{
	cJSON *tmp;char *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	if(tmp->type == cJSON_Array)tmp = tmp->child;
	while(1){
		if(tmp == NULL) break;
		if(n > 0){
			tmp = tmp->next;
			n--;
			continue;
			}
		if(tmp->type == cJSON_String){
			out = tmp->valuestring;
			}
		*nextTarget = tmp->next;
		break;
		}
	return out;
	
}

/*
acquireNumberOfArrayFromCjson:  获取数组的第n个数据
nextTarget: 指向下一个数据
返回第n个数据
*/
int acquireNumberOfArrayFromCjson(cJSON *jsonSrc,int n,cJSON **nextTarget)
{
	cJSON *tmp;int out=DEFAULT_FAILED_INT;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	if(tmp->type == cJSON_Array)tmp = tmp->child;
	while(1){
		if(tmp == NULL) break;
		if(n > 0){
			tmp = tmp->next;
			n--;
			continue;
			}
		if(tmp->type == cJSON_Number){
			out = tmp->valueint;
			}
		*nextTarget = tmp->next;
		break;
		}
	return out;
	
}



/*
acquireValueFromCjsonAbsolutely:  根据一个名称穷尽数据获取json数据一个数据
targetName : 数据名称
返回获取的数据，未找到数据返回NULL，返回的指针务必free掉防止内存泄漏
*/
char *acquireValueFromCjsonAbsolutely(cJSON *jsonSrc,char *targetName,int *type)
{
	cJSON *tmp;char *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		if(tmp->type == cJSON_Object){
			//printf("serching cJSON_Object :%s\n",tmp->string);
			if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
			else if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
					printf("acquire JSON_Object :%s\n",tmp->string);
					out=cJSON_Print(tmp);
					*type = tmp->type;
					printf("%s:%s\n",tmp->string,out);
					break;
					}
			else{
				out = acquireValueFromCjsonAbsolutely(tmp->child,targetName,type);//穷尽json数据
				if(out!=NULL) break;
				}
			tmp = tmp->next;
			}
		else if(tmp->type == cJSON_String){
			//printf("serching cJSON_String :%s\n",tmp->string);
			if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
				printf("acquire cJSON_String :%s\n",tmp->string);
				//out=print_string(tmp,0);
				//out=tmp->valuestring;
				out=cJSON_Print(tmp);
				*type = tmp->type;
				printf("%s:%s\n",tmp->string,out);
				break;
				}
			tmp = tmp->next;
			}
		else if(tmp->type == cJSON_Array){
			//printf("serching cJSON_Array :%s\n",tmp->string);
			if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
				printf("acquire cJSON_Array :%s\n",tmp->string);
				//out=print_array(tmp,0,1,0);
				out=cJSON_Print(tmp);
				*type = tmp->type;
				printf("%s:%s\n",tmp->string,out);
				break;
				}
			tmp = tmp->next;
			}
		else if(tmp->type == cJSON_Number){
			//printf("serching cJSON_Number :%s\n",tmp->string);
			if((strlen(tmp->string)==strlen(targetName)) && (memcmp(targetName,tmp->string,strlen(targetName))==0)){
				printf("acquire cJSON_Number :%s\n",tmp->string);
				//out=print_number(tmp,0);
				out=cJSON_Print(tmp);
				*type = tmp->type;
				printf("%s:%s\n",tmp->string,out);
				break;
				}
			tmp = tmp->next;
			}
		else{
			tmp = tmp->next;
			}
		if(tmp == NULL) break;
		}
	return out;
	
}


/*
acquireValueFromCjsonObjectAbsolutely:  穷尽数据，找到objectName当中targetName的数据
objectName :目标cjson数据名称
targetName :目标array数据名称
返回json格式的数据字符串
*/
char *acquireValueFromCjsonObjectAbsolutely(cJSON *jsonSrc,char *objectName,char *targetName,int *type)
{
	cJSON *tmp;char *out=NULL;
	if (!jsonSrc) return out;
	tmp = jsonSrc;
	while(1){
		{
				if(tmp->type == cJSON_Object ){
					//printf("serching objectName :%s\n",tmp->string);
					if(tmp->string == NULL && tmp->child != NULL){tmp = tmp->child; continue;}
					else if((strlen(tmp->string)==strlen(objectName)) && (memcmp(objectName,tmp->string,strlen(objectName))==0)){
							printf("acquire cJSON_Object :%s\n",tmp->string);
							out=acquireValueFromCjsonAbsolutely(tmp->child,targetName,type);
							//printf("%s:%s\n",tmp->string,out);
							break;
							}
					else{
						out = acquireValueFromCjsonObjectAbsolutely(tmp->child,objectName,targetName,type);//穷尽json数据
						if(out!=NULL) break;
						}
					tmp = tmp->next;
					}
				else{
					tmp = tmp->next;
					}
				
				if(tmp == NULL) break;
				}
		}
	return out;
	
}
#ifdef  android
/*
 *	Add a string to a string buffer
 */

static void sql_put_string(strbuf_t1 *buf, char *s, int len, int width,
		int prec, enum flag1 f)
{
	int		i;

	if (len < 0) { 
		len = sqlStrnlen(s, prec >= 0 ? prec : ULONG_MAX); 
	} else if (prec >= 0 && prec < len) { 
		len = prec; 
	}
	if (width > len && !(f & flag_minus)) {
		for (i = len; i < width; ++i) { 
			sql_put_char(buf, ' '); 
		}
	}
	for (i = 0; i < len; ++i) { 
		sql_put_char(buf, s[i]); 
	}
	if (width > len && f & flag_minus) {
		for (i = len; i < width; ++i) { 
			sql_put_char(buf, ' '); 
		}
	}
}

/*
 *	Return the length of a string limited by a given length
 */

int sqlStrnlen(char *s, unsigned int n)
{
	unsigned int 	len;

	len = strlen(s);
	return sqlMin(len, n);
}

/******************************************************************************/
/*
 *	Add a long to a string buffer
 */

static void sql_put_ulong(strbuf_t1 *buf, unsigned long int value, int base,
		int upper, char *prefix, int width, int prec, enum flag1 f)
{
	unsigned long	x, x2;
	int				len, zeros, i;

	for (len = 1, x = 1; x < ULONG_MAX / base; ++len, x = x2) {
		x2 = x * base;
		if (x2 > value) { 
			break; 
		}
	}
	zeros = (prec > len) ? prec - len : 0;
	width -= zeros + len;
	if (prefix != NULL) { 
		width -= sqlStrnlen(prefix, ULONG_MAX); 
	}
	if (!(f & flag_minus)) {
		if (f & flag_zero) {
			for (i = 0; i < width; ++i) { 
				sql_put_char(buf, '0'); 
			}
		} else {
			for (i = 0; i < width; ++i) { 
				sql_put_char(buf, ' '); 
			}
		}
	}
	if (prefix != NULL) { 
		sql_put_string(buf, prefix, -1, 0, -1, flag_none); 
	}
	for (i = 0; i < zeros; ++i) { 
		sql_put_char(buf, '0'); 
	}
	for ( ; x > 0; x /= base) {
		int digit = (value / x) % base;
		sql_put_char(buf, (char) ((digit < 10 ? '0' : (upper ? 'A' : 'a') - 10) +
			digit));
	}
	if (f & flag_minus) {
		for (i = 0; i < width; ++i) { 
			sql_put_char(buf, ' '); 
		}
	}
}

/*
 *	Add a character to a string buffer
 */

void sql_put_char(strbuf_t1 *buf, char c)
{
	if (buf->count >= (buf->size - 1)) {
		if (! (buf->flags & SQL_STR_REALLOC)) {
			return;
		}
		buf->size += SQL_STR_INC;
		if (buf->size > buf->max && buf->size > SQL_STR_INC) {
/*
 *			Caller should increase the size of the calling buffer
 */
			buf->size -= SQL_STR_INC;
			return;
		}
		if (buf->s == NULL) {
			buf->s = malloc( buf->size * sizeof(char));
		} else {
			buf->s = realloc( buf->s, buf->size * sizeof(char));
		}
	}
	buf->s[buf->count] = c;
	if (c != '\0') {
		++buf->count;
	}
}

/******************************************************************************/
/*
 *	Dynamic sprintf implementation. Supports dynamic buffer allocation.
 *	This function can be called multiple times to grow an existing allocated
 *	buffer. In this case, msize is set to the size of the previously allocated
 *	buffer. The buffer will be realloced, as required. If msize is set, we
 *	return the size of the allocated buffer for use with the next call. For
 *	the first call, msize can be set to -1.
 */

static int sqlDsnprintf(char **s, int size, char *fmt, va_list arg, int msize)
{
	strbuf_t1	buf;
	char		c;
	memset(&buf, 0, sizeof(buf));
	buf.s = *s;

	if (*s == NULL || msize != 0) {
		buf.max = size;
		buf.flags |= SQL_STR_REALLOC;
		if (msize != 0) {
			buf.size = sqlMax(msize, 0);
		}
		if (*s != NULL && msize != 0) {
			buf.count = strlen(*s);
		}
	} else {
		buf.size = size;
	}

	while ((c = *fmt++) != '\0') {
		if (c != '%' || (c = *fmt++) == '%') {
			sql_put_char(&buf, c);
		} else {
			enum flag1 f = flag_none;
			int width = 0;
			int prec = -1;
			for ( ; c != '\0'; c = *fmt++) {
				if (c == '-') { 
					f |= flag_minus; 
				} else if (c == '+') { 
					f |= flag_plus; 
				} else if (c == ' ') { 
					f |= flag_space; 
				} else if (c == '#') { 
					f |= flag_hash; 
				} else if (c == '0') { 
					f |= flag_zero; 
				} else {
					break;
				}
			}
			if (c == '*') {
				width = va_arg(arg, int);
				if (width < 0) {
					f |= flag_minus;
					width = -width;
				}
				c = *fmt++;
			} else {
				for ( ; iswdigit((int)c); c = *fmt++) {
					width = width * 10 + (c - '0');
				}
			}
			if (c == '.') {
				f &= ~flag_zero;
				c = *fmt++;
				if (c == '*') {
					prec = va_arg(arg, int);
					c = *fmt++;
				} else {
					for (prec = 0; iswdigit((int)c); c = *fmt++) {
						prec = prec * 10 + (c - '0');
					}
				}
			}
			if (c == 'h' || c == 'l') {
				f |= (c == 'h' ? flag_short : flag_long);
				c = *fmt++;
			}
			if (c == 'd' || c == 'i') {
				long int value;
				if (f & flag_short) {
					value = (short int) va_arg(arg, int);
				} else if (f & flag_long) {
					value = va_arg(arg, long int);
				} else {
					value = va_arg(arg, int);
				}
				if (value >= 0) {
					if (f & flag_plus) {
						sql_put_ulong(&buf, value, 10, 0, "+", width, prec, f);
					} else if (f & flag_space) {
						sql_put_ulong(&buf, value, 10, 0, " ", width, prec, f);
					} else {
						sql_put_ulong(&buf, value, 10, 0, NULL, width, prec, f);
					}
				} else {
					sql_put_ulong(&buf, -value, 10, 0, "-", width, prec, f);
				}
			} else if (c == 'o' || c == 'u' || c == 'x' || c == 'X') {
				unsigned long int value;
				if (f & flag_short) {
					value = (unsigned short int) va_arg(arg, unsigned int);
				} else if (f & flag_long) {
					value = va_arg(arg, unsigned long int);
				} else {
					value = va_arg(arg, unsigned int);
				}
				if (c == 'o') {
					if (f & flag_hash && value != 0) {
						sql_put_ulong(&buf, value, 8, 0, "0", width, prec, f);
					} else {
						sql_put_ulong(&buf, value, 8, 0, NULL, width, prec, f);
					}
				} else if (c == 'u') {
					sql_put_ulong(&buf, value, 10, 0, NULL, width, prec, f);
				} else {
					if (f & flag_hash && value != 0) {
						if (c == 'x') {
							sql_put_ulong(&buf, value, 16, 0, "0x", width, 
								prec, f);
						} else {
							sql_put_ulong(&buf, value, 16, 1, "0X", width, 
								prec, f);
						}
					} else {
						sql_put_ulong(&buf, value, 16, 0, NULL, width, prec, f);
					}
				}

			} else if (c == 'c') {
				char value = va_arg(arg, int);
				sql_put_char(&buf, value);

			} else if (c == 's' || c == 'S') {
				char *value = va_arg(arg, char *);
				if (value == NULL) {
					sql_put_string(&buf, "(null)", -1, width, prec, f);
				} else if (f & flag_hash) {
					sql_put_string(&buf,
						value + 1, (char) *value, width, prec, f);
				} else {
					sql_put_string(&buf, value, -1, width, prec, f);
				}
			} else if (c == 'p') {
				void *value = va_arg(arg, void *);
				sql_put_ulong(&buf,
					(unsigned long int) value, 16, 0, "0x", width, prec, f);
			} else if (c == 'n') {
				if (f & flag_short) {
					short int *value = va_arg(arg, short int *);
					*value = buf.count;
				} else if (f & flag_long) {
					long int *value = va_arg(arg, long int *);
					*value = buf.count;
				} else {
					int *value = va_arg(arg, int *);
					*value = buf.count;
				}
			} else {
				sql_put_char(&buf, c);
			}
		}
	}
	if (buf.s == NULL) {
		sql_put_char(&buf, '\0');
	}

/*
 *	If the user requested a dynamic buffer (*s == NULL), ensure it is returned.
 */
	if (*s == NULL || msize != 0) {
		*s = buf.s;
	}

	if (*s != NULL && size > 0) {
		if (buf.count < size) {
			(*s)[buf.count] = '\0';
		} else {
			(*s)[buf.size - 1] = '\0';
		}
	}

	if (msize != 0) {
		return buf.size;
	}
	return buf.count;
}

/*
redefine the standard function printf();
redefine it to work on android system:buf will be sent  to PC by means of TCP on port 9876,also print log to android studio 
attention:if we use myprintf() to print log on PC,the log may be delayed within several hundred ms.
*/
int myprintf(char * fmt,...)
{
	va_list		 vargs;
	char		*buf=NULL;
	int			 rc;
	va_start(vargs, fmt);
	sqlDsnprintf(&buf, PRINTF_BUF_SIZE, fmt, vargs,0);
	va_end(vargs);
	//printf("%s\n",buf);
	LOGI("%s:%s",SQL_PRINTF_ANDROID,buf);//this line should be shieled while release a final version
	if(printFd){
		write(networkPrintfPipe[1], buf, strlen(buf));
		write(networkPrintfPipe[1], "\r\n", strlen("\r\n"));//a automatic line feed for windows print
		}
	if(buf)free(buf);
}

void * netPrintfPhtread(void * arg)
{
	int result;
    int ret;
    int fd;
	int old_stdout;
	char buf[1024];
    struct sockaddr_in server_addr, client_addr;
    int addrlen;
    addrlen = sizeof(struct sockaddr_in);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9876);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    result = bind(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
	if(result < 0){
		myprintf("bind error");
		return ;
		}
    listen(fd, 16);
    //old_stdout = dup(1);
    old_stdout = dup(networkPrintfPipe[1]);
    while(1)
    {
        printFd = accept(fd,  (struct sockaddr *)&client_addr, &addrlen);
		//dup2(cfd, 1);
		dup2(printFd, networkPrintfPipe[1]);
        for(;;) 
		{
			if (read(printFd, buf,1024) <= 0)
			{
				//dup2(old_stdout, 1);
				dup2(old_stdout, networkPrintfPipe[1]);
				close(printFd);
				printFd = 0;
				break;
			}
        }
    }
}
/*
networkPrintfPipeInitialize:
this function let us print log to the PC with cmd "telnet IP 9876" while we connect
*/
int networkPrintfPipeInitialize()
{
    int mode = 1;
	//sem_init(&networkPrintfPipeSem, 0, 0);
	if(pipe(networkPrintfPipe) == -1 )
	{
		myprintf("pipe failed");
	}
	ioctl(networkPrintfPipe[0], FIONBIO, (char *)&mode);
	ioctl(networkPrintfPipe[1], FIONBIO, (char *)&mode);
	pthread_create(&networkPrintfTid, NULL, netPrintfPhtread, NULL);
}
#else
#endif
/*
returnMsgProcessor:  packing arguments into json data 
*/
void returnMsgProcessor(char **returnCjsonMsg,char *basalString,char *jsonString,int returnCode)
{
 cJSON *json;
 json=cJSON_Parse(basalString);
 if(json){
	 if(jsonString)cJSON_AddStringToObject(json,CJSON_DATE_Detail,jsonString);
	 cJSON_AddNumberToObject(json,CJSON_DATE_returnCode,returnCode);
 	}
 else printf("returnMsgProcessor missing basalString \n");
 if(returnCjsonMsg) *returnCjsonMsg = cJSON_Print(json);
 cJSON_Delete(json); 
 //printf("returnMsgProcessor:%s\n",*returnCjsonMsg);
}
/*
cjsonProcessor:  json data processor
cjsonString :json data text needed processing
returnCjsonMsg:the result of processing ,which must be release
return ture ,or the code of failed reason
*/
int cjsonProcessor(char *cjsonString,char **returnCjsonMsg)
{
 cJSON *json = NULL;int out = DEFAULT_FAILED_INT;
 char returnMsg[RETURN_MSG_LEN]={'\0'};
 if(!cjsonString){
	 sprintf(returnMsg,"who send me a NULL pointer?");
	 returnMsgProcessor(returnCjsonMsg,CjsonError_returnMsg,returnMsg,CJSON_ERROR_cjsonParseError);
	 return CJSON_ERROR_cjsonParseError;
 	}
 json=cJSON_Parse(cjsonString);
 if (!json) {
 	sprintf(returnMsg,"cjsonProcessor:parse failed,check the json date:");
	memcpy(returnMsg+strlen(returnMsg),cJSON_GetErrorPtr(),50);
	returnMsgProcessor(returnCjsonMsg,CjsonError_returnMsg,returnMsg,CJSON_ERROR_cjsonParseError);
	return CJSON_ERROR_cjsonParseError;
	}
 else{
 	int cmd = DEFAULT_FAILED_INT;
	if((cmd = acquireNumberFromCjsonAbsolutely(json,CJSON_DATE_CMD)) == DEFAULT_FAILED_INT){//missing data"cmd"
		returnMsgProcessor(returnCjsonMsg,CjsonError_returnMsg,"cjsonProcessor:json data cmd missing",CJSON_ERROR_canNotAcquireCmd);
		out = CJSON_ERROR_canNotAcquireCmd;
		goto END;
		}
	switch(cmd){
			break;
		default:
			returnMsgProcessor(returnCjsonMsg,CjsonError_returnMsg,"cjsonProcessor:json data cmd illgal",CJSON_ERROR_cmdIllegal);
			out = CJSON_ERROR_cmdIllegal;
			goto END;
			break;
		}
 	}
 END:
	 if (json){printf("cjsonProcessor free json\n");cJSON_Delete(json);json = NULL;}
	 printf("out = %d\n",out);
	 return out;
}
