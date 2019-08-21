#ifndef INFO_LIST_HANDLER_H_
#define INFO_LIST_HANDLER_H_
#include "ctrlcmd.h"

#ifdef __cplusplus
extern "C"{
#endif

#define MAX_PHOTO      100          //最大的图片数量
#define MAX_VIDEO      20           //最大的视频数量
#define MAX_BUFFER     20*1024      //最大的图片缓冲
#define MAX_LOG        100          //最大的日志数量
#define  MAX_MSG       200          //最大的消息数量
#define MAX_VOICE      20            //最大的留言数量
#define MAX_RING       20           //最大可下载的铃声数量

#define MAX_TELEPHONE  18           //最大可保存的用户数量


#define MAX_DEFAULTRING  16          //最大的默认铃声

#define USED_OPTIMIZE_FFS 1      //是否使用优化后的save file 处理。



#pragma pack(1)

        typedef struct _tagFlashRing{
                char name[32];
                unsigned long positon;
                unsigned long len;
        }FLASHRING;

        typedef struct _tagDefaultRing{
                int count; //默认铃声的数量 MAX_DEFAULTRING >count >0
                FLASHRING   flashring[MAX_DEFAULTRING];
        }DEFAULTRING;

#pragma pack()


        //////////////////////////////////////////////////////////////////////
        //
        //抓拍图片的处理，目前处理100张图片
        //初始化
        void init_photo();

        void save_photo();

        int  photo_sort(); //根据时间对图片倒叙排列

        //获得空闲的图片空间
        PHOTOINFO* get_idle_photo();

        unsigned char* get_photo_addr(char*name,long* len);

        //将图片增加到，图片列表中
        int add_photo(PHOTOINFO* pPhotoinfo,unsigned char* addr,long len);

        //删除一幅图片
        int del_photo(PHOTOINFO* pPhotoinfo);

        //获得第几副图片
        PHOTOINFO* get_photo(int count);

        //获得第count幅图片。
        PHOTOINFO* get_select_photo(int count);

        PHOTOINFO* get_time_pioneer_photo();


        //获得抓拍图片的数量
        int get_photo_count();

        int del_all_photo();

        int lock_photo(PHOTOINFO* pPhotoinfo,int islock);

        int set_photo_read(PHOTOINFO* pPhotoinfo,int isread);

        void Set_Max_photo(int maxnum);

        int IsNewPhoto(); //2009-9-2 9:59:12
		int isnoreadpic();

        //
        ////////////////////////////////////////////////////////////////////
		void init_video();
		void save_video();
		int  video_sort(); 
		PHOTOINFO* get_idle_video();
		unsigned char* get_video_addr(char*name,long* len);
		int add_video(PHOTOINFO* pPhotoinfo,unsigned char* addr,long len);
		int del_video(PHOTOINFO* pPhotoinfo);
		PHOTOINFO* get_video(int count);
		PHOTOINFO* get_select_video(int count);
		PHOTOINFO* get_select_video(int count);
		PHOTOINFO* get_time_pioneer_video();
		int get_video_count();
		int del_all_video();
		int lock_video(PHOTOINFO* pPhotoinfo,int islock);
		int set_video_read(PHOTOINFO* pPhotoinfo,int isread);
		void Set_Max_video(int maxnum);
		int IsNewVideo(); //2009-9-2 9:59:12
		int isnoreadvideo();

        ////////////////////////////////////////////////////////////////////
        //以下是对LOG的处理
        void init_log();
        void save_log();
        LOGINFO* get_idle_log();
        int add_log(char* text);
        int add_log2(char* text);
        int del_log(LOGINFO* pLog);
        LOGINFO* get_select_log(int count);
        LOGINFO* get_time_pioneer_log();
        int get_log_count();
        int del_all_log();
        void log_sort();
        int lock_log(LOGINFO* pLog,int islock);
        int set_log_read(LOGINFO* pLog,int isread);


        //////////////////////////////////////////////////////////////////////////////
        //以下是对短信的处理
        void init_msg();
        void save_msg();
        MSGINFO* get_idle_msg();
        int add_msg(char* text);
        int add_msg2(char* name,unsigned char* photobuf,int buflen);
        unsigned char* get_photo_msg(char* name,int* len);
        int del_msg(MSGINFO* pMsg);
        MSGINFO* get_select_msg(int count);
        MSGINFO* get_time_pioneer_msg();
        int get_msg_count();
        int del_all_msg();
        int isnoreadmsg();
        int lock_msg(MSGINFO* pMsg,int islock);
        int set_msg_read(MSGINFO* pMsg,int isread);



        /////////////////////////////////////////////////////////////////////
        //电话本的处理
        void init_telephone();
        void save_telephone();
        TELEPHONE* get_idle_telephone();
        int add_telephone(TELEPHONE* ptelephone);
        int del_telephone(TELEPHONE* ptelephone);
        TELEPHONE* get_select_telephone(int count);
        int get_telephone_count();
        int del_all_telephone();
        TELEPHONE* modify_telephone(char* number);
        int lock_telephone(TELEPHONE* pTelephone,int islock);


        /*void init_Telephone();
        void uninit_Telephone();
        int GetDumpTelephoneStart(bool bForward);
        int get_telephone(long& count,char* text);
        int add_telephone(const char* text);
        int del_telephone(int count);
        int get_telephone_count();
        int del_all_telephone();
        */
        /*
        void init_message();

        MSGINFO* get_idle_message();

        int add_message(MSGINFO* pMsg);

        int del_message(MSGINFO* pMsg);

        MSGINFO* get_next_message(MSGINFO* pMsg);

        MSGINFO* get_prev_message(MSGINFO* pMsg);

        MSGINFO* get_select_message(int count);

        MSGINFO* get_time_pioneer_message();

        int get_message_count();

        int del_all_message();
        */
        ////////////////////////////////////////////////////////////
        //

        ////////////////////////////////////////////////////////////////////
        //以下是对voice的处理

        void init_voice();
        void save_voice();

        VOICEINFO* get_idle_voice();

        unsigned char* get_voice_addr(char*name,long* len);

        int add_voice(VOICEINFO* pVoice,unsigned char* addr,long len);

        int del_voice(VOICEINFO* pVoice);

        VOICEINFO* get_select_voice(int count);

        VOICEINFO* get_time_pioneer_voice();

        int get_voice_count();

        int del_all_voice();

        int lock_voice(VOICEINFO* pVoice,int islock);
        int set_voice_read(VOICEINFO* pVoice,int isread);
        int  isnoreadvoice();//2009-8-5 15:46:32

        void free_mem_all_voice();


        //---------------------------------------------------------
        //对已下载铃声的处理
        void init_downring();

        void save_downring();


        DOWNRINGINFO* get_idle_downring();

        //unsigned char* get_idle_downring_addr();

        unsigned char* get_downring_addr(char*name,long* len);


        int add_downring(DOWNRINGINFO* pPhotoinfo,unsigned char* addr,long len);


        int del_downring(DOWNRINGINFO* pPhotoinfo);


        DOWNRINGINFO* get_select_downring(int count);


        DOWNRINGINFO*  get_time_pioneer_downring();


        int get_downring_count();

        int del_all_downring();

        int lock_downring(DOWNRINGINFO* pdownringinfo,int islock);

        void free_mem_all_downring();

        void set_english_ring();
        void set_chinese_ring();

        void free_all_ring_cache();

        //unsigned char*  get_ring_addr_by_name(char* name,int* len);

        //////////////////////////////////////////////////////////
        //对铃声列表浏览的处理
        RINGLIST* GetRingInfo(char* name);

        bool AddRingToList(char* name);

        void DelAllRingFromList();

        int AddRingToDownList(char* name,int content_len,unsigned char* buf);

        bool RingIsDown(char* name);

        RINGLIST* GetSelectRingElement(int index);

        RINGLIST* GetFirstRingElement();

        int GetRingListCount();

        ////////////////////////////////////////////////////////
        //对费用列表的处理
        void AddFeeToList(unsigned char month,unsigned char* buf);
        void DelAllFeeFromList();
        int  GetFeeListCount();
        FEELIST* GetFirstFeeElement();

        /////////////////////////////////////////////////////////////
        //便民信息的处理
        char* GetDocInfo(int* len);
        void AddDocInfo(unsigned char* buf,int len);
        void FreeDocInfo();

        ///////////////////////
        //
        void init_tmp();
        void save_tmppic(unsigned char* buf,int len,char* name);
        char* read_tmppic(int* len ,char* name);
        int read_alltempimage(unsigned char* pointer[16],int size_len[16]);
        void del_all_tmppic();

	//[2011-5-19 9:43:44]
	void init_screen_save();
	int save_screen_save_image(unsigned char* buf ,int len, char* name);
	int read_all_screensave_image(unsigned char** imagepointer,unsigned long* imagesize);
	void del_all_dir();
	int has_flash_size();
	void free_flash();
	 void delete_all_screen_image_for_factory();
	 void delete_all_screen_image();
	
	unsigned char* get_main_ui_replace_jpg(char* name,int* size);


        //for factory
        void test_info_for_factory();

#if USED_OPTIMIZE_FFS
        void init_optimize_save();
#endif


        typedef struct _JPEGFRAME{
                unsigned char* data;
                int width;
                int height;
                unsigned long size;
                int type;
                struct _JPEGFRAME* next;
        }JPEGFRAME;

        unsigned long get_vod_frame(unsigned char** pbuf,int selected,int framenum);
        void add_vod(JPEGFRAME* frame,int size,int count,int isdoor);
        void release_frame_header();


#ifdef __cplusplus
}
#endif

#endif
