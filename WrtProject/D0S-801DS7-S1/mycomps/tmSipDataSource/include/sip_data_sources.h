#ifndef _SIP_DATA_SOURCES_H_
#define _SIP_DATA_SOURCES_H_

#if defined(__cplusplus)
extern "C" {
#endif


#pragma pack(1)

        typedef struct _sip_data_sources{
                unsigned char type;
                int len;
                char code[16];//15���ַ������һ��Ϊ������
                unsigned char* data;
        }SIP_DATA_SOURCES;

#pragma pack()

        //����SIP����Դ
        int addSipDataSource(unsigned char type,int len,char* code,unsigned char* data);

        //��������ɾ��SIP����Դ
        void delSipDataSource(int index);

        //���SIP����Դ�ĸ���
        int getSipDataSourceSize();

        //�����������SIP����Դ�ľ�������
        SIP_DATA_SOURCES* getSipDataSource(int index);


        //ɾ�����е�����
        void delAllSipDataSource();
        
        //����ΪĬ������
        void set_default_data();



#if defined(__cplusplus)
}
#endif

#endif