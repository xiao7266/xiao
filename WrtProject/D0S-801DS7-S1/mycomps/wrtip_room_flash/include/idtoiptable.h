
#ifdef __cplusplus
extern "C"
{
#endif

        void init_idtoip();
        void save_idtoip(unsigned char* buf,long buflen);
        ////////////////////////////////////////////////////
        //   [in] id  �������
        //   [out] ip  ���ҵ����ݴ���ip
        //   [in,out]  num �������ip�ĸ��������num�õ���ip����
        void get_ipbyid(char* id,unsigned long* ip,int* num);
        unsigned char* get_idtoipdata(int* len);
        void get_id2(char* p,int len);
        void get_ipbyid2(char* id,unsigned long *ip); 

#ifdef __cplusplus
}
#endif