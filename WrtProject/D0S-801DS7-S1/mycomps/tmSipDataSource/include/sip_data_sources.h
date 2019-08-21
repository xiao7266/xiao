#ifndef _SIP_DATA_SOURCES_H_
#define _SIP_DATA_SOURCES_H_

#if defined(__cplusplus)
extern "C" {
#endif


#pragma pack(1)

        typedef struct _sip_data_sources{
                unsigned char type;
                int len;
                char code[16];//15个字符，最后一个为结束符
                unsigned char* data;
        }SIP_DATA_SOURCES;

#pragma pack()

        //增加SIP数据源
        int addSipDataSource(unsigned char type,int len,char* code,unsigned char* data);

        //根据索引删除SIP数据源
        void delSipDataSource(int index);

        //获得SIP数据源的个数
        int getSipDataSourceSize();

        //根据索引获得SIP数据源的具体数据
        SIP_DATA_SOURCES* getSipDataSource(int index);


        //删除所有的数据
        void delAllSipDataSource();
        
        //设置为默认数据
        void set_default_data();



#if defined(__cplusplus)
}
#endif

#endif