#ifndef _CMD_HANDLER_H_
#define _CMD_HANDLER_H_

#include "node.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define USE_PACKELIST 0

#if USE_PACKELIST
typedef struct _tagSockPacketList{
        int sockindex;
        int PacketLen;
        int buflen;
        unsigned char* buf;
        struct _tagSockPacketList* next;
}SOCKPACKETLIST;

typedef struct _tagROOMCONNECT{
        unsigned long roomipaddr;
        int connectindex;
        int isackidle;
}ROOMCONNECT;

#endif

class CSocketManager;

class CCmdHandler: public CNode {
public:
        CCmdHandler():CNode() {
                m_sink = NULL;
                memset(m_callipaddr,0,sizeof(m_callipaddr));
                m_bstart = false;
                m_CurrentSocketIndex = -1;
                m_display = NULL;
                m_centeripaddr = 0x2a01a8c0;//0xc0a8012a;
                m_Montipaddr   = 0x3a01a8c0;//0xc0a8013a;

                m_waringipaddr = 0x2a01a8c0;//0xc0a802a;
                m_Groupwaringipaddr = 0x2a01a8c0;//0xc0a8012a;
                m_CurrentItemID  = -1;
#if USE_PACKELIST
                InitPackList();
#endif
                memset(m_downname,0,41);
                memset(m_UserPhoneNumber,0,16);

                m_helpfilelen = 0;
                m_isdivert = 0;
                m_dirvertcount  = 0;
                m_ishostbusy = 0; //2009-7-16
                m_isdoor = 0;//2009-7-29

                m_sipmediaaddr = 0;
                m_sipmediaport = 0;
                
                m_istalkandsmalldoor =0;
                m_is_divert_subroom = 0;
				m_holdon_device = 0;
#ifdef HAVE_SUB_PHONE
                m_phone_ip = 0;
#endif
        }
        ~CCmdHandler() {
                m_sink = NULL;

                m_bstart = false;
                m_CurrentSocketIndex = -1;
                m_display = NULL;


                m_centeripaddr = -1;
                m_Montipaddr  = -1;
                m_Groupwaringipaddr = -1;
                m_waringipaddr = -1;
                m_CurrentItemID = -1;
                m_garageipaddr = 0;
                m_manageripaddr = 0;
                
                memset(m_callipaddr,0,sizeof(m_callipaddr));
#if USE_PACKELIST
                UnInitPackList();
#endif
                memset(m_downname,0,41);
                memset(m_UserPhoneNumber,0,16);
                m_isdivert = 0;
                m_ishostbusy = 0; //2009-7-16
                m_isdoor = 0;//2009-7-29

                m_sipmediaaddr = 0; //2010-4-1 10:51:43
                m_sipmediaport = 0;   //2010-4-1 10:51:48   


#ifdef HAVE_SUB_PHONE
                m_phone_ip = 0;
#endif
        }
#ifdef HAVE_SUB_PHONE
        unsigned long get_phone_ip()
        {
                return m_phone_ip;
        }
#endif
        void SetSocketSink(CSocketManager* psink){
                if(m_sink == NULL && NULL != psink)
                        m_sink = psink;
        }

        void msg_from_phone(unsigned long msg_buf[4])
        {
                m_myMsgQueue.send_message(MSG_NODE_PHONE,
                        msg_buf,sizeof(unsigned long)*4,
                        m_myMsgQueueSemaphore);
        }

        void Send_cmd(unsigned long event[4]){
#if HAVE_USED_SDL

                int ret =  m_myMsgQueue.send_message(MSG_NODE_CMD,
                        event,sizeof(unsigned long)*4,
                        m_myMsgQueueSemaphore);
#else
                unsigned long tmpevent[5];
                tmpevent[0] = MSG_NODE_CMD;
                tmpevent[1] = event[0];
                tmpevent[2] = event[1];
                tmpevent[3] = event[2];
                tmpevent[4] = event[3];

                int ret = q_vsend(m_myMsgID,tmpevent,sizeof(unsigned long)*5);
#endif

        }
        void Send_cmd2(unsigned long event[4]){
#if HAVE_USED_SDL
                m_myMsgQueue.send_message(MSG_NODE_SMALLDOOR_CMD,
                        event,sizeof(unsigned long)*4,
                        m_myMsgQueueSemaphore);
#else
                unsigned long tmpevent[5];
                tmpevent[0] = MSG_NODE_SMALLDOOR_CMD;
                tmpevent[1] = event[0];
                tmpevent[2] = event[1];
                tmpevent[3] = event[2];
                tmpevent[4] = event[3];
                q_vsend(m_myMsgID,tmpevent,sizeof(unsigned long)*5);
#endif
        }

        void SetDisplayCallback(display pdisplay){
                if(m_display == NULL && NULL != pdisplay){
                        m_display = pdisplay;
                }
        }
        void CallDisplayCallBack(unsigned long event[4]){
                if(m_display)
                        m_display(event);
        }

        int IsInit(){
                return m_bstart;
        }

        int GetIsDoorCall(){
                return m_isdoor;
        }

        void SetRoomCallRoomName(char* id){
                //     strncpy(m_UserRoomNumber,id,15);
        }



        void SetCenterIP(unsigned long ip);

        void GetCenterIP(unsigned long * ip){

                *ip=m_centeripaddr;
        }
        void SetWaringIP(unsigned long ip){
                m_waringipaddr  = ip;
        }
        void SetGroupWaringIP(unsigned long ip){
                m_Groupwaringipaddr  = ip;
        }
        void SetDoorIP(unsigned long ip);
        void GetDoorIP(unsigned long* ip);

        void SetMontIP(unsigned long ip){
                m_Montipaddr = ip;
        }
        int  GetHostRoomBusy(){ //2009-7-16 ,只为多分机
                return m_ishostbusy;
        }

        void SetPeerRoomIP(unsigned long ip[4]){
                m_callipaddr[0] = ip[0];
                m_callipaddr[1] = ip[1];
                m_callipaddr[2] = ip[2];
                m_callipaddr[3] = ip[3];
        }
        //此接口已经无用
        void GetPeerRoomIP(unsigned long* ip){
                *ip=0;
        }
        //此接口已经无用
        void SetgarageIP(unsigned long ip);

        //此接口已经无用
        void GetgarageIP(unsigned long *ip){
                *ip= m_garageipaddr;
        }

        void SetManagerIP(unsigned long ip);
        void GetManagerIP(unsigned long *ip){
                *ip = m_manageripaddr;
        }

        int GetCurrentSysStatus(){
                return m_SysStatus;
        }

        char* GetDownLoadName(){
                return m_downname;
        }
        
		void SetCurrentSysBusy()
		{
			m_SysStatus = SYS_BUSY;
		}
		
		void SetCurrentSysIdle()
		{
			m_SysStatus = SYS_IDLE;
		}
		
        bool isupdate(){
                if(m_SysStatus == SYS_UPGRADE)
                        return  true;
                return false;
        }

        void callhelp(){
                unsigned  long tmp_event[4];
                tmp_event[0] = STATUS_START_EMERG;
                tmp_event[1] =tmp_event[2] =tmp_event[3] =0;
                m_display(tmp_event);
        }
        void SetDefaultRing();
        void SetCenterCallRing( char* addr,long size);
        void SetWallCallRing(char* addr,long size);
        void SetDoorCallRing(char* addr,long size);
        void SetRoomCallRing(char* addr,long size);
        void SetSmallDoorCallRing(char* addr,long size);
        void SetWarningRing(char* addr,long size);
        void SetMsgRing(char* addr,long size);
        void SetAlarmClockRing(char* addr,long size);
        char* GetRingConfig(int type,int* len);
        int GetHostIP(unsigned long* ip);
        int GetHostMask(unsigned long*  mask);
        int GetHostGateWay(unsigned long*  gateway);
        int SetHostIP(unsigned long ip);
        int SetHostMask(unsigned long mask);
        int SetHostGateWay(unsigned long gateway);
        int SaveModify();
        int ReInitSubRoomIP();
        unsigned char*  PackCmd(short cmd,unsigned char* buf,int buflen,int* packlen);
        unsigned char*  PacketMontCmd(int index,int * packetlen);

        void playvoice(int index);
        void avoidtimeout(int type);
        
        void SendColorMsg(unsigned long event[4]);//[2011-5-17 13:50:27]

        void SetRingtimes(unsigned long times);
        unsigned long GetRingtimes();
        void SetAvoidtimes(unsigned long hours,unsigned long min);
        void GetAvoidtimes(unsigned long * hours,unsigned long* min);
        void GetAgenttimes(unsigned long* hours,unsigned long * min);
        void SetAgenttimes(unsigned long hours,unsigned long min);

        int udp_send_data(unsigned long event[4]);
        int udp_send_data_hangup(unsigned long event[4]);
        int udp_send_data2(unsigned long ip,unsigned long event[4],char* destid = NULL);
        void room_active_sync(unsigned long ip);
        void room_req_sync(unsigned long ip);
        void ProcessUDP(unsigned long ip,unsigned char* data,int datalen);
        int GetIsSimulateTalk();

#if 1
        unsigned char* GetTestRingAddr(int index,unsigned long* len);
#endif
        //2011-9-29 15:22:48 add
        void playvoice2(int index,int loop);
        
        //2011-10-10 9:37:02
        void lowtension_timer(int start);
		void set_holdon_device(int type);
		int  get_holdon_device();
protected:
        int GetType(){
                return CMD_HANDLER;
        }
        int ThreadMain(void);
        void ProcessCmd(int sock_index,short cmd,unsigned long cmd_event[4]);
        void ProcessPacket(int socket_index,short cmd,unsigned char* buf,int buflen);
        void SendCmd(int sock_index,short cmd,int id=-1);
        void SendCmd2(int sock_index,short cmd,char *buf,int buflen,int id=-1);
        void RequestIP(int sock_index,short cmd,unsigned char buf[12]);
        void RoomAckSysinfo(int sock_index);
        void Init();

        void SendRoomCmd(int sock_index);
        void SendDownRingCmd(int sock_index);


protected:
        void door_call_room(int sock_index,unsigned long event[4]);
        void room_unlock(int sock_index);
        void room_holdon(int sock_index,unsigned long event[4]);
        void room_hangup(int sock_index);
        void center_call_room(int sock_index);
        void door_hangup(int sock_index);
        void door_and_center_hangup(int sock_index);
        void room_call_center(int sock_index);
        void center_ack_idle(int sock_index);
        void room_door_center_ack_busy(int sock_index);
        void center_holdon(int sock_index);
        void room_start_mont(int sock_index,unsigned long event[4]);
        void door_ack_montack(int sock_index);
        void room_stop_mont(int sock_index);
        void room_start_mont_ipc(int sock_index,unsigned long event[4]);
        void room_stop_mont_ipc(int sock_index);
        void room_stop_mont_byKey(int sock_index);
        void door_stop_mont(int sock_index);
        void room_call_room(int sock_index,unsigned char* buf,unsigned long buflen);
        void room_call_room2(int sock_index,unsigned char* buf,unsigned long buflen);
        void room_ack_idle(int sock_index);
        void room_ack_quiet(int sock_index);
        void room_ack_agent(int sock_index,unsigned char* buf,unsigned long buflen);
        void room_stop_leave(int sock_index);
        void door_ack_unlock(int sock_index);
        void room_ack_media_negotiate(int sock_index,unsigned char* buf,unsigned long buflen);
        
        void room_get_weather(int sock_index);

        void door_start_leaveword(int sock_index);
        void door_stop_leaveword(int sock_index);

        void room_waring(int sock_index,unsigned char id);
        void room_emerg(int sock_index);
        void room_queryfee(int sock_index,int id);
        void room_repair(int sock_index,int id);
        void room_browsering(int sock_index);
        void room_ringdownload(int sock_index,int id);
        void room_gettime(int sock_index);
        void room_reqdocinfo(int sock_index);
        void room_reqhelpinfo(int sock_index);
        void room_manual_agent(int sock_index);

        void room_ask_for_sysinfo(unsigned long event[4]);

        void room_sync_agent(int sock_index);
        void room_sync_avoid(int sock_index);
        void room_cancel_sync_agent(int sock_index);
        void room_cancel_sync_avoid(int sock_index);
		void divert_room_to_room_deal();
		void divert_room_to_center_deal();
        void center_reset_room_pwd(int sock_index);

        void center_ack_agent(int sock_index,unsigned char* buf,int buflen);
        void center_ack_minor(int sock_index,unsigned char* buf,int buflen);
        void center_ack_divert_center(int sock_index,unsigned char* buf,int buflen);
        void center_ack_divert_room(int sock_index,unsigned char* buf,int buflen);

        void center_ack_reqdocinfo(int sock_index,unsigned char* buf,int buflen);
        void center_ack_gettime(int sock_index,unsigned char* buf,int buflen);
        void center_ack_repair(int sock_index);
        void center_ack_queryfee(int sock_index,unsigned char* buf,int buflen);
        void center_ack_browserring(int sock_index,unsigned char* buf,int buflen);
        void center_ack_downloadring(int sock_index,unsigned char* buf,int buflen);
        void center_ack_ring_error(int sock_index);
        void center_ack_weather(int sock_index,unsigned char* buf,int buflen);
        //void center_ack_emerg(int sock_index);
        void center_upgrade_room(int sock_index,unsigned char* buf,int buflen,int is2);

        void room_door_center_ack_busy(int sock_index,unsigned char* buf,int buflen);
        void local_upgrade_room(unsigned long event[4]);

        void center_get_idtoiptable(int sock_index);
        void center_req_jiaju_status(int sock_index,short scmd);
        void center_set_jiaju(int sock_index,unsigned char* buf,int buflen,short scmd);
        void center_get_jiaju_status(int sock_index,unsigned char* buf,int buflen,short scmd);
        void center_set_jiaju_scene(int sock_index,unsigned char* buf,int buflen,short scmd);
        void center_set_idtoip_table(int sock_index,unsigned char* buf,int buflen);
        void center_set_iolevel(int sock_index,unsigned char* buf,int buflen);

        void center_set_factory_info(int sock_index,unsigned char* buf,int buflen);
        void center_ask_for_sysinfo_ack(int sock_index,unsigned char* buf,int buflen);

        //2009-7-29 14:17:11
        void center_remote_single_guard(int sock_index,unsigned char* buf,int buflen,short scmd);
        void center_set_elec_file_config(int sock_index,unsigned char* buf,int buflen);
        void center_get_elec_file_config(int sock_index);
        
        //2011-5-16 8:51:39
        void center_set_room_scr(int sock_index,unsigned char* buf,int buflen);



        void door_public_info(int sock_index,unsigned char* buf,int buflen);
        void center_set_sys_config(int sock_index,unsigned char* buf,int buflen);
        void  door_broadcast_pic(int sock_index,unsigned char* buf,int buflen);
        void center_get_sysinfo(int sock_index);


        void center_set_pic(int sock_index,unsigned char* buf,int buflen);

        void net_exception(int sock_index);
        void net_shutdown(int sock_index);

        void ProcessConnectOK(int index);
#if USE_PACKELIST
        void InitPackList();
        void UnInitPackList();
        void AddPackList(int sock_index,unsigned char* buf,int buflen,int PacketLen,bool isfirst);
        void DelPackList(int sock_index);
#endif
        int UpdateFile(const char* url,const char* ver,int is2);
        int Update(unsigned char* buf,unsigned long  len,const char* ver,int is2);




#ifdef DOWNLOAD_FROM_HTTP
       // void download_mp3_over(unsigned char* buf,int buflen);
#endif

        // 小门口机函数
        void smalldoor_call_room(unsigned long event[4]);
        void smalldoor_ack_holdon(unsigned long event[4]);
        void smalldoor_ack_hangup(unsigned long event[4]);
        void smalldoor_ack_openlock(unsigned long event[4]);
        void smalldoor_leave(unsigned long event[4]);
        void smalldoor_ack_mont(unsigned long event[4]);
        void smalldoor_ack_stopmont(unsigned long event[4]);
        void smalldoor_ack_leave(unsigned long event[4]);


		//
        void nvo_holdon(unsigned long event[4]);
        void nvo_hangup(unsigned long event[4]);
        void nvo_unlock(unsigned long event[4]);

        // 家居遥控器远程控制函数 2009-7-30 9:00:59
        void riu_room_defend(unsigned long event[4]);
        void riu_room_remove_defend(unsigned long event[4]);
        void riu_room_open_lock(unsigned long event[4]);
        void riu_room_remove(unsigned long event[4]); //无条件撤防。
        void riu_key(unsigned long event[4]);
        void riu_room_warning(unsigned long event[4]);

        //caller

        void custom_caller_ringoff(unsigned long event[4]);
        void custom_caller_ringon(unsigned long event[4]);
        void custom_caller_holdon(unsigned long event[4]);
        void custom_caller_callend(unsigned long event[4]);
        void custom_caller_prep(unsigned long event[4]);

        //布防、撤防
        void center_arrange_guard(int sock_index,unsigned char* buf,int buflen,short scmd);
        void center_check_guard(int sock_index,short scmd);
        void center_reset_room_pass(int sock_index);

        //召唤电梯
        void room_call_lift(unsigned long event[4]);

        //sip
        void room_call_sip(unsigned long event[4]);
        void call_sip_processing(unsigned long event[4]);
        void call_sip_ring(unsigned long event[4]);
        void call_sip_close(unsigned long event[4]);
        void call_sip_busy(unsigned long event[4]);
        void call_sip_start_audio(unsigned long event[4]);
        void proxy_call_room(unsigned long event[4]);

        //取消各种命令
        void custom_cancel_cmd(unsigned long event[4]);

        //2010.5.31
        void room_services_request(unsigned long event[4]);
        void center_services_ack(int sock_index,unsigned char* buf,int buflen);

        //2010.7.19
        void phone_invite_room(int sock_index,unsigned char* buf,int buflen);
        void phone_inq_all_home_status(int sock_index,unsigned char* buf,int buflen);
        void phone_ctrl_more_safe_status(int sock_index,unsigned char* buf,int buflen);
        void phone_ctrl_one_devices(int sock_index,unsigned char* buf,int buflen);
        void phone_ctrl_safe_status(int sock_index,unsigned char* buf,int buflen);
        void phone_ctrl_scene(int sock_index,unsigned char* buf,int buflen);
        void phone_inq_all_safe_status(int sock_index,unsigned char* buf,int buflen);
        void phone_inq_one_devices(int sock_index,unsigned char* buf,int buflen);

        unsigned char phone_confirm(unsigned char* buf,int buflen);

        //2010.8.4
        void center_set_bomb(int sock_index,unsigned char* buffer,int buflen);
        //
        void ProcessPhone(unsigned long msg_buf[4]);
        


       

protected:
        CSocketManager*     m_sink;
		

        bool                m_bstart;
        int                 m_SysStatus;
        int                 m_CurrentSocketIndex;
        int                 m_CurrentItemID;
        display             m_display;

        unsigned long       m_centeripaddr;                  //普通的中心
        unsigned long       m_Groupwaringipaddr;             //报警分中心1
        unsigned long       m_waringipaddr;                  //报警总中心2
        unsigned long       m_Montipaddr;                    //门口机IP
        unsigned long       m_callipaddr[4];                 //户户通时对端房间的IP

        unsigned long       m_garageipaddr;                  //车库IP
        unsigned long       m_manageripaddr;                 //托管IP
#if USE_PACKELIST
        SOCKPACKETLIST      m_PacketList;                    //收到不完整的包，保存起来。
#endif
        char                m_UserPhoneNumber[16];
        char                m_downname[41];
        unsigned long       m_displayevent[4];
        RINGSET             g_ringset[8];                    //包含所有种类的铃声

        int                 m_helpfilelen;
        int                 m_isdivert;
        int                 m_dirvertcount;
        int                 m_ishostbusy; //2009-7-16

        int                m_isdoor; //2009-7-29 0,主机，2，围墙机。,4 数字小门口机
        unsigned long  m_sipmediaaddr ;
        int                     m_sipmediaport;
        int                m_istalkandsmalldoor;
        int                m_is_divert_subroom;//如果为1则表示有转发信令至5-8
		int 			   m_holdon_device;//0-没摘机 1-对方摘机 2-分机 3-按键 4-PX 5-手机 6-多分机 

#ifdef HAVE_SUB_PHONE
        unsigned long     m_phone_ip;
        char              m_DestNumber[16];

#endif
};

#endif /* _CMD_HANDLER_H_ */
