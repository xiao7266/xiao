#ifdef __cplusplus
extern "C" {
#endif	


/**
�������ƣ�WRTRC4Encrypt
�������ܣ�ʹ��RC4�㷨���������ݿ�,
���������
	  pwd, 16λ���롣��Ҫ����������
	  buf, ��Ҫ���ܵ����ݿ�ĵ�ַ��
	  buflen,��Ҫ���ܵ����ݿ�ĳ��ȡ�
	  outbuf:���������ݡ�������buflenһ����
���أ�
	��0:���سɹ���-1����ʧ�ܡ�
	  
*/
int  WRTRC4Encrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf);


/**
�������ƣ�WRTRC4Decrypt
�������ܣ�ʹ��RC4�㷨���������ݿ�
���������
	  pwd: 16λ���롣��Ҫ����������
	  buf: ��Ҫ���ܵ����ݿ�ĵ�ַ��
	  buflen:��Ҫ���ܵ����ݿ�ĳ��ȡ�
	  outbuf:���������ݡ�������buflenһ����
���أ�
	��0:���سɹ���-1����ʧ�ܡ�
*/	
int  WRTRC4Decrypt(const char* pwd, unsigned char* buf, int buflen, unsigned char* outbuf);

/**
�������ƣ�WRTMD5
�������ܣ�ʹ��MD5�㷨������MD5У��ֵ��
���������
	  buf: ��Ҫ��������ݿ�ĵ�ַ��
	  buflen:��Ҫ��������ݿ�ĳ��ȡ�
	  outbuf:������У����,����base64������16���ֽڡ�
	  outbuflen:outbuf �����С
���أ�
	��0:���سɹ���-1����ʧ�ܡ�
*/
	
int  WRTMD5(unsigned char* buf, int buflen, unsigned char* outbuf,int outbuflen);



#ifdef __cplusplus
}
#endif