#ifdef __cplusplus
extern "C"{
#endif



        void init_smalldoor();
        int start_smalldoor_video();
        int stop_smalldoor_video();

        void send_voice_photo_to_center(int type,int cmd,unsigned char* buf,int buflen);

        void send_control_cmd_to_center(unsigned long event[4]);

        void send_smalldoor_cmd(unsigned short event[4]);

        void cap_smalldoor_photo();
        void cap_smalldoor_photo_frame();

        void start_auto_cap_smalldoor_photo();
        void stop_auto_cap_smalldoor_photo();
        void send_dsp_to_mcu(unsigned short param);

        void mcu_reset_sys(unsigned char param); //param 1：重启系统，0，正常

        void init_16safe();

        //2010/10/14
        void start_or_stop_smvideo_phone(unsigned long ip,short port);

#ifdef __cplusplus
}
#endif
