#ifdef __cpluscplus
extern "C"{
#endif
/*
#define ENGLISH 0X01
#define CHINESE 0x0

  	int GetCurrentLanguage();
  	 void ReDrawCurrentWindow2(int type);
  	int iptv_image_write_jpeg_420(unsigned char *y,unsigned char *u,unsigned char *v,int width,int height,unsigned char *outputbuf,int quality,int destwidth);
         int send_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec,unsigned char status);
        unsigned char* get_jiaju_config_buf_for_center(int* len);
        int get_jiaju_status_for_center(unsigned char roomtype,unsigned char room,unsigned char electtype,unsigned char elec);
        int send_scene_mode(int mode);
        void set_security_status(int index,int status);
        int isenablesafe();
        int get_security_status(int index);
        int GetIsHaveJiaju();
        void ReDrawCurrentWindow2(int type);
        void ReDrawCurrentWindow();
        void modifypwd();
        char* Get_jiaju_table_file(int *len);
        int Set_jiaju_table_file(unsigned char* buf,int buflen);

        void sync_pwd(unsigned char id,unsigned char* buf,int len);
*/
void V_Server_SetDest1(const char *addr,int port);
void A_Server_SetDest1(const char *addr,int port);
void A_Server_SetDest2(const char *addr,int port);
void V_Server_SetDest2(const char *addr,int port);
void A_Server_SetDest3(const char *addr,int port);
void V_Server_SetDest3(const char *addr,int port);
void A_Server_SetDest4(const char *addr,int port);
void V_Server_SetDest4(const char *addr,int port);
void A_Server_DelDest2();
void V_Server_DelDest2();
void A_Server_DelDest3();
void V_Server_DelDest3();
void A_Server_DelDest4();
void V_Server_DelDest4();
        
#ifdef __cpluscplus
}
#endif