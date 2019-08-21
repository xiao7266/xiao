
#ifdef __cplusplus
extern "C"
{
#endif

        void init_idtoip();
        void save_idtoip(unsigned char* buf,long buflen);
        ////////////////////////////////////////////////////
        //   [in] id  输入参数
        //   [out] ip  查找到数据存入ip
        //   [in,out]  num 传入分配ip的个数，输出num得到的ip个数
        void get_ipbyid(char* id,unsigned long* ip,int* num);
        unsigned char* get_idtoipdata(int* len);
        void get_id2(char* p,int len);
        void get_ipbyid2(char* id,unsigned long *ip); 

#ifdef __cplusplus
}
#endif