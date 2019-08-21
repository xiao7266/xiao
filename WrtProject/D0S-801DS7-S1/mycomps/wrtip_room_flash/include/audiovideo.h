#ifndef __AUDIOVIDEO_H__
#define __AUDIOVIDEO_H__

#if defined(__cplusplus)
extern "C"{
#endif

        void JStartVideoProc(void);
        void InitAudioDevice(void);
        void PausePlayVideo(int status); //发送消息
        void PauseAndPlayVideo(int status); //实际动作
        int mp3play(char *mp3buf, int mp3buflen,int loop);
        int mp3stop(void);
        int talkstart(void);
        int talkstop(void);
        int talktostart(unsigned long ip,short port);
        int talktostop(void);
        int talkLeaveStart(void);
        int talkLeaveStop(void);
        int talkLeaveCancel(void);
        int GetLeaveStatus(void);
        void SetSendIPAndPort(unsigned long ip,short port);
        void CapturePhoto();
        void CancelCapturePhoto1();
        void CapturePhoto1();

        void SetVolume(int volume);
        void SetBass(int bass);
        void SetTreble(int treble);

        void StartAlaramPromptVocie();
        void StopAlaramPromptVoice();

        void StartButtonVocie();
        void StartButtonVocie2();

        void OpenAndCloseButtonVoice(bool isopen);

        void OpenAndCloseRing(bool isopen);

        bool GetButtonVoiceStatus();
		int reopenaudio();

        int GetPlayMp3Status(); //add by ljw

        //---------------------------------

        int leaveplay(char* buf,int buflen,int loop);
        int leavestop();

        void starsmalldoorleavervoice();
        void stopsmalldoorleavevoice();

        int start_smalldoor_audio(int issimulate);
        int stop_smalldoor_audio(int issimulate);

        int startsmalldoorleave();
        int stopsmalldoorleave();
        int GetsmalldoorLeaveStatus();
        int cancelsmalldoorleave();
        int  GetPlayVideoStatus();

        void start_record();
        void stop_record();

        void SetPhotoQuality(int quality);



        //设置SIP呼叫时是否采用标准G711，40MS打包
        int SetSipCallParam();
        
        uint32_t tm_get(uint32_t* date,uint32_t *time1,uint32_t* ticks);
        uint32_t tm_set(uint32_t date,uint32_t time1,uint32_t ticks);
        uint32_t tm_getticks(uint32_t* hi,uint32_t* li);

#ifdef HAVE_SUB_PHONE
        int start_talkto_phone(unsigned long ip,unsigned long port);
        int start_phone_audio(unsigned long ip ,unsigned long port);
        int stop_phone_audio();
        int start_phone_video(unsigned long ip,unsigned long port);
        int stop_phone_video();
#endif

#if defined(__cplusplus)
}
#endif

#endif
