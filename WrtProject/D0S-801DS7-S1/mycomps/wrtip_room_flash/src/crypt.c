#include <stdio.h>
#include <stdlib.h>
#include <polarssl/base64.h>  
#include <polarssl/ssl.h>  
#include <polarssl/arc4.h> 
#include <polarssl/md5.h> 
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>
#include "wrt_crypt.h"




/**
函数名称：WRTRC4Encrypt
函数功能：使用RC4算法，加密数据块,
输入参数：
	  pwd, 16位密码。需要带结束符。
	  buf, 需要加密的数据块的地址。
	  buflen,需要加密的数据块的长度。
	  outbuf:编码后的数据。长度与buflen一样。
返回：
	　0:返回成功。-1返回失败。
	  
*/
int  WRTRC4Encrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf)
{
	
	arc4_context rc4key;  
	if(pwd == NULL || buf == NULL || buflen == 0 || outbuf == NULL)
		return -1;
	arc4_setup(&rc4key, (const unsigned char*)pwd, strlen(pwd));
	arc4_crypt(&rc4key,buflen,buf,outbuf);
	return 0;	
}


/**
函数名称：WRTRC4Decrypt
函数功能：使用RC4算法，解密数据块
输入参数：
	  pwd: 16位密码。需要带结束符。
	  buf: 需要解密的数据块的地址。
	  buflen:需要解密的数据块的长度。
	  outbuf:解码后的数据。长度与buflen一样。
返回：
	　0:返回成功。-1返回失败。
	  
*/
int  WRTRC4Decrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf)
{
	arc4_context rc4key;  
	if(pwd == NULL || buf == NULL || buflen == 0 || outbuf == NULL)
		return -1;
	arc4_setup(&rc4key, (const unsigned char*)pwd, strlen(pwd));
	arc4_crypt(&rc4key,buflen,buf,outbuf);

	return 0;
}

/**
函数名称：WRTMD5
函数功能：使用MD5算法，计算MD5校验值。
输入参数：
	  buf: 需要计算的数据块的地址。
	  buflen:需要计算的数据块的长度。
	  outbuf:计算后的校验码,经过base64编码后的16个字节。
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTMD5(unsigned char* buf, int buflen, unsigned char* outbuf, int outbuflen)
{
	
	unsigned char binout[16];
	char out[30];
	int outlen = 30;
	if(outbuf == NULL || outbuflen < 16 || buf == NULL)
		return -1;
	md5_context state;
	md5_starts(&state);
	md5_update(&state, buf, buflen);
	md5_finish(&state, binout);
	memset(out,0,30);
	base64_encode(out,&outlen,(void const*)binout,16);
	memcpy(outbuf,out,outbuflen);
	return 0;
	
	

		
	
}



/**
函数名称：WRTMD5
函数功能：使用MD5算法，计算MD5校验值。
输入参数：
	  buf: 需要计算的数据块的地址。
	  buflen:需要计算的数据块的长度。
	  outbuf:计算后的校验码,32字节。
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTMD5_16(unsigned char* buf, int buflen, char* outbuf, int outbuflen)
{
	
	unsigned char binout[16];
	char out[30];
	int outlen = 30;
	int i = 0;
	if(outbuf == NULL || outbuflen < 16 || buf == NULL)
		return -1;
	md5_context state;
	md5_starts(&state);
	md5_update(&state, buf, buflen);
	md5_finish(&state, binout);
	for(i = 0; i<16; i++){
		sprintf(outbuf+i*2,"%02x",(unsigned char)binout[i]);
	}
	return 0;
	
}




/**
函数名称：WRTGetRandPwd
函数功能：使用MD5算法，获取随机密码
输入参数：
	  outrandpwd: 随机密码缓冲。
	  randpwdlen 随机密码长度16个字节
返回：
	　0:返回成功。-1返回失败。
*/
	
int  WRTGetRandPwd( char* outrandpwd,int randpwdlen)
{
	entropy_context entropy;
	ctr_drbg_context ctr_drbg;
	char _out[30];
	int _outlen = 30;
	unsigned char tmp[30];	

	if(outrandpwd == NULL)
		return -1;
		
	memset(_out,0,30);
	memset(tmp,0,16);
	entropy_init( &entropy );
	ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,NULL,0);
	ctr_drbg_random(&ctr_drbg,tmp,16); 
	base64_encode(_out, &_outlen,(const char*)tmp, (size_t)16);
	memcpy(outrandpwd,_out,randpwdlen);	
	entropy_free(&entropy);
	/*
	ortp_init();
	ortp_crypto_get_random(tmp, 16);
	base64_encode(_out, &_outlen,(const char*)tmp, (size_t)16);
	memcpy(outrandpwd,_out,randpwdlen);
	ortp_exit();
	*/
	
	
	return 0;
}