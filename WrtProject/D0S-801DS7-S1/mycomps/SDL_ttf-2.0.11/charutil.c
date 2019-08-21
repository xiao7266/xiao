/* charutil.c */

#include "charutil.h"
#include "chardata.c"



/****************************************************************************

	LOCAL FUNCTIONS

****************************************************************************/


/****************************************************************************
* NAME:		binarySearch
* PURPOSR:	perform binary search in uint16 array
* ENTRY:	table	- point to uint16 array
*		tablen	- length of array
*		code	- word to search
* EXIT:		int16 - index of object, -1 for no match
* AUTHOR: 	lvjie November 18, 2003
****************************************************************************/
static short binarySearch(const unsigned short *table, 
						  unsigned short tablen, 
						  unsigned short code)
{
	unsigned short head,tail,middle;

	head = 0;
	tail = tablen-1;
	if ((code < table[head])||(code > table[tail]))
		return(-1);

	while (head <= tail)
	{
		middle = (head+tail)/2;
		if (code == table[middle])
			return (middle);
		else if (code > table[middle])
			head = middle+1;
		else
			tail = middle-1;
	}

	return (NOT_SUPPORTED);
}

static int getUniLenOfGbStr( const unsigned char *p )
{
	int len = 0;

	while ( *p ) {
		if ( *p & 0x80 ) {
			p += 2;
		}
		else {
			p += 1;
		}
		len++;
	}
	return len;
}


static int getGbLenOfUniStr( const unsigned short *p )
{
	int len = 0;

	while ( *p ) {
		if ( *p < 0x80 ) {
			len += 1;
		}
		else {
			len += 2;	/* convert unsupport char to ?? */
		}
		p++;
	}
	return len;
}


/* convert gb2312 word to unicode word. 
 return ? if unsupported */
unsigned short gbc2uc( unsigned short gbc )
{
	short index = binarySearch(gbkAGbkcode, CODE_NUM, gbc);

	if ( index == NOT_SUPPORTED ) {
		return '?';
	}
	else {
		return gbkAUnicode[index];
	}
}


/* convert unicode word to gb2312 word. 
 return ?? if unsupported */
unsigned short uc2gbc( unsigned short uc )
{
	short index = binarySearch(uniAUnicode, CODE_NUM, uc);

	if ( index == NOT_SUPPORTED ) {
		return 0x3f3f;	/* ?? */
	}
	else {
		return uniAGbkcode[index];
	}
}


// ��word��ȡ��ָ���Ĳ���,sָ����λ,eָ����λ,����:
// xxxxs--------exx
// e��2,s��11
// ����:0000s--------e
static unsigned short cut_word(unsigned short w, unsigned char s, unsigned char e)
{
	unsigned short m;
	unsigned char l;

	l = s - e + 1;
	if ( l == 16 ) {
		return w;
	}

	w = w >> e;		// 00xxxxs--------e
	m = 1 << l;		// 0000010000000000
	w &= ( m - 1 );	// 00xxxxs--------e & 0000001111111111
	return w;		// 000000s--------e
}


/****************************************************************************

	GLOBAL FUNCTIONS

****************************************************************************/


/****************************************************************************
* NAME:		gb2uni
* PURPOSR:	Convert gbk string to unicode string
*			the byte order of unicode is dependent on CPU
*			the two-byte code in GBK is always big-endian
* ENTRY:	gbstr	gbk string
*			unibuf	output buffer
*			buflen	length of buffer
* EXIT:		length of unicode string
****************************************************************************/
int gb2uni( const unsigned char *gbstr, unsigned short *unibuf, int buflen )
{
	int unilen, i;
	unilen = getUniLenOfGbStr(gbstr);
	if ( !unibuf || ( buflen <= 0 ) ) {
		return unilen;
	}

	if ( unilen > buflen ) {
		unilen = buflen;
	}

	for ( i = 0; i < unilen; i++ ) {
		if ( *gbstr & 0x80 ) {
			/* gbk-code is big-endian */
			unsigned short gbc = ( gbstr[0] << 8 ) + gbstr[1];
			unibuf[i] = gbc2uc(gbc);
			gbstr += 2;
		}
		else {
			unibuf[i] = *gbstr;
			gbstr += 1;
		}
	}

	return unilen;
}


/****************************************************************************
* NAME:		uni2gb
* PURPOSR:	Convert unicode string to gbk string
*			the byte order of unicode MUST be consistent with underlying CPU
*			the two-byte code in GBK is always big-endian
* ENTRY:	unistr	unicode string
*			gbbuf	output buffer
*			buflen	length of buffer
* EXIT:		length of gbk string
****************************************************************************/
int uni2gb( const unsigned short *unistr, unsigned char *gbbuf, int buflen )
{
	int gblen, i;
	gblen = getGbLenOfUniStr(unistr);
	if ( !gbbuf || ( buflen <= 0 ) ) {
		return gblen;
	}

	if ( gblen > buflen ) {
		gblen = buflen;
	}

	i = 0;
	while ( i < gblen ) {
		if ( *unistr < 0x80 ) {
			gbbuf[i] = (char)(*unistr);
			i++;
		}
		else {
			/* gbk-code is big-endian */
			unsigned short t = uc2gbc(*unistr);
			gbbuf[i++] = (unsigned char)( t >> 8 );
			gbbuf[i++] = (unsigned char)( t & 0xff );
		}
		unistr++;
	}

	return gblen;
}


/*--------------------------------------------------------------------------------------------------------------------
UTF-8������8λΪ��Ԫ��UCS���б��롣��UCS-2��UTF-8�ı��뷽ʽ���£�

UCS-2����(16����) UTF-8 �ֽ���(������) 
0000 - 007F 0xxxxxxx 
0080 - 07FF 110xxxxx 10xxxxxx 
0800 - FFFF 1110xxxx 10xxxxxx 10xxxxxx 

���硰�����ֵ�Unicode������6C49��6C49��0800-FFFF֮�䣬���Կ϶�Ҫ��3�ֽ�ģ���ˣ�
1110xxxx 10xxxxxx 10xxxxxx��
��6C49д�ɶ������ǣ�0110 110001 001001�� ��������������δ���ģ���е�x���õ���11100110 10110001 10001001����E6 B1 89��
--------------------------------------------------------------------------------------------------------------------*/


// �ȸ���ת���������utf8������,ͬʱ���������������С�жϿ���ת����ucs2������
// ת��
int ucs2ToUtf8(const unsigned short *ucs, unsigned char *cbuf, int cbuf_len)
{
	int i, j, l, max_i;
	unsigned short w, w1;

	i = 0;
	max_i = 0;
	j = 0;
	w = ucs[i];
	while (w) {
		if ( w <= 0x7f ) {
			l = 1;
		}
		else if ( w <= 0x7ff ) {
			l = 2;
		}
		else {
			l = 3;
		}

		j += l;		// �ۼ��������
		if ( j <= cbuf_len ) {
			max_i = i;
		}
		i++;		// ��һ��ucs2
		w = ucs[i];
	}

	if ( !cbuf || ( cbuf_len == 0 ) ) {
		return j;
	}

	j = 0;
	for  ( i = 0; i <= max_i; i++ ) {
		w = ucs[i];
		if ( w <= 0x7f ) {
			l = 1;
			cbuf[j++] = (unsigned char)w;
		}
		else if ( w <= 0x7ff ) {
			l = 2;
			w1 = cut_word(w, 10, 6);
			cbuf[j++] = 0xc0|w1;
			w1 = cut_word(w, 5, 0);
			cbuf[j++] = 0x80|w1;
		}
		else {
			l = 3;
			w1 = cut_word(w, 15, 12);
			cbuf[j++] = 0xe0|w1;
			w1 = cut_word(w, 11, 6);
			cbuf[j++] = 0x80|w1;
			w1 = cut_word(w, 5, 0);
			cbuf[j++] = 0x80|w1;
		}
	}

	return j;
}


// �ȸ���ת���������ucs2������, �����������С�ж���Ҫת����ucs2������
// ת��
int utf8ToUcs2(const unsigned char *s, unsigned short *wbuf, int wbuf_len)
{
	int i, j, k;
	unsigned char c;
	unsigned char c3, c2, c4;

	i = 0;
	j = 0;
	c = s[i++];
	while (c) {
		c3 = c & 0xe0;
		c4 = c & 0xf0;
		if ( (c & 0x80) == 0 ) {
			// ���ֽ�
		}
		else if ( c3 == 0xc0 ) {
			c2 = s[i++] & 0xc0;
			if ( c2 != 0x80 ) {
				break;
			}
		}
		else if ( c4 == 0xe0 ) {
			c2 = s[i++] & 0xc0;
			if ( c2 != 0x80 ) {
				break;
			}
			c2 = s[i++] & 0xc0;
			if ( c2 != 0x80 ) {
				break;
			}
		}
		else {
			break;	// ����,��Ϊ����
		}

		j++;
		c = s[i++];
	}

	if ( !wbuf || ( wbuf_len == 0 ) ) {
		return j;
	}

	if ( wbuf_len < j ) {
		j = wbuf_len;
	}

	i = 0;
	for ( k = 0; k < j; k++ ) {
		c = s[i++];
		c3 = c & 0xe0;
		c4 = c & 0xf0;
		if ( (c & 0x80) == 0 ) {
			// ���ֽ�
			wbuf[k] = c;
		}
		else if ( c3 == 0xc0 ) {
			// ȡ�������ֽ�
			wbuf[k] = (c & 0x1f) << 6;
			c = s[i++];
			wbuf[k] |= ( c & 0x3f );
		}
		else if ( c4 == 0xe0 ) {
			// ȡ��3���ֽ�
			wbuf[k] = c << 12;
			c = s[i++];
			wbuf[k] |= ( (c & 0x3f) << 6 );
			c = s[i++];
			wbuf[k] |= (c & 0x3f);
		}
	}

	return j;
}

/* end of charutil.c */
