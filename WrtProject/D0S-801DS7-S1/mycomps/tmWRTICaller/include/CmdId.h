#ifndef _CMD_ID_H_
#define _CMD_ID_H_

/*
*   以下是关于指令的定义(具体含义请参考协议)
*/
#ifdef CMD_ID
#undef CMD_ID
#endif
#define  CMD_ID(cmdid) ((cmdid>>8)|((cmdid&0x00FF)<<8))
#define PC_DATA_PACK_SIZE 512
//分机指令
#define ROOM_CALLCENTER		    (CMD_ID(0x0001))
#define ROOM_EMERG		        (CMD_ID(0x0002))
#define ROOM_INQFEE		        (CMD_ID(0x0003))
#define ROOM_REPAIR		        (CMD_ID(0x0004))
#define ROOM_BROWSERING		    (CMD_ID(0x0005))
#define ROOM_RINGDOWN		    (CMD_ID(0x0006))
#define ROOM_GETTIME		    (CMD_ID(0x0007))
#define ROOM_WARNING        	(CMD_ID(0x0008))
#define ROOM_CALLROOM          	(CMD_ID(0x0009))
//
#define ROOM_STARTMONT		    (CMD_ID(0x0020))
#define ROOM_STOPMONT		    (CMD_ID(0x0021))
#define ROOM_BROADCASTACK	    (CMD_ID(0x0022))
#define ROOM_UPGRADEACK  	    (CMD_ID(0x0023))
//2008-10-17
#define ROOM_LEAVEWORDACK       (CMD_ID(0x0024))        //回应留言
#define ROOM_STOPTLEAVEWORDACK  (CMD_ID(0x0025))		//停止留言

//add [2012-12-21 13:54  add cmd by wyx]
#define MEDIA_NEGOTIATE     (CMD_ID(0x1190))           
#define MEDIA_NEGOTIATE_ACK (CMD_ID(0x1191)) 



//
#define ROOM_BUSY		        (CMD_ID(0x0090))
#define ROOM_IDLE		        (CMD_ID(0x0091))
#define ROOM_HOLDON		        (CMD_ID(0x0092))
#define ROOM_HANGUP		        (CMD_ID(0x0093))
#define ROOM_UNLOCK		        (CMD_ID(0x0094))

//2008-10-17
#define ROOM_AGENT              (CMD_ID(0x0095))     //托管
#define ROOM_QUIET              (CMD_ID(0x0096))     //免扰

//2008-12-10
#define ROOM_REST_PASS_ACK     (CMD_ID(0x0a01))

//主机指令

#define DOOR_CALLCENTER		    (CMD_ID(0x0101))
#define DOOR_MONTACK		    (CMD_ID(0x0102))
#define DOOR_BUSY		        (CMD_ID(0x0103))
#define DOOR_UNLOCKACK		    (CMD_ID(0x0104))
#define DOOR_GETTIME		    (CMD_ID(0x0105))
#define DOOR_CALLROOM		    (CMD_ID(0x0106))
#define DOOR_HANGUP		        (CMD_ID(0x0107))
#define DOOR_ENDMONT      	    (CMD_ID(0x0108))
#define DOOR_SENDLOG		    (CMD_ID(0x0109))
#define DOOR_BROADCAST		    (CMD_ID(0x010A))
#define DOOR_BROADCASTPIC	    (CMD_ID(0x010B))
#define DOOR_BROADCASTACK	    (CMD_ID(0x010C))
#define DOOR_UPGRADEACK		    (CMD_ID(0x010D))
#define DOOR_STARTLEAVEWORD     (CMD_ID(0x010E))
#define DOOR_ENDLEAVEWORD       (CMD_ID(0x010F))
#define DOOR_DATA_TEST          (CMD_ID(0xfff0))
#define DOOR_FLASH_SYS_FMT      (CMD_ID(0xfff1))

#define DOOR_AV_SERVER_STATUS    (CMD_ID(0xfff2)) /*READ STATUS*/
#define DOOR_AV_SERVER_PAUSE     (CMD_ID(0xfff3)) /*PAUSE*/
#define DOOR_AV_SERVER_STOP      (CMD_ID(0xfff4)) /*STOP*/
#define DOOR_AV_SERVER_START     (CMD_ID(0xfff5)) /*START*/
#define DOOR_AV_SERVER_RESTART   (CMD_ID(0xfff6)) /*RESTART*/
#define DOOR_AV_SERVER_INIT      (CMD_ID(0xfff7)) /*INITIALIZE*/
#define DOOR_AV_SERVER_TO144     (CMD_ID(0xfff8)) /*SEND TO 192.168.1.144*/
#define DOOR_AUTO_CALL           (CMD_ID(0xfff9)) /*自动呼叫依次地址表*/

#define DOOR_GETCONFIGACK       (CMD_ID(0x0110))
#define DOOR_GETDOORADDRLISTACK (CMD_ID(0x0111))
//#define DOOR_LOGIN            (CMD_ID(0x0111))


//2008-11-14
#define DOOR_ASKFOR_SYSINFO     (CMD_ID(0x0112))
#define DOOR_GET_SYSINFO_ACK    (CMD_ID(0x0113))

//2008-11-20 回应中心恢复操作密码
#define DOOR_RESET_PASS_ACK     (CMD_ID(0x0183))

#define DOOR_MODIF_PASS_ACK     (CMD_ID(0x0184))
/*副主机召唤电梯主机 2009-10-15*/
#define DOOR_CALL_LIFT          (CMD_ID(0x0130))

//------回应中心设置MAC地址-----
#define  DOOR_SET_MAC_ACK      (CMD_ID(0x0003))
#define  DOOR_REQ_MAC              (CMD_ID(0x0001))
//2012-0504
#define CENTER_SET_MAC            (CMD_ID(0x0002))

//中心指令
#define CENTER_EMERGACK		    (CMD_ID(0x0201))
#define CENTER_INQFEEACK	    (CMD_ID(0x0202))
#define CENTER_REPAIREACK	    (CMD_ID(0x0203))
#define CENTER_TIMEACK		    (CMD_ID(0x0204))
#define CENTER_CALLROOM		    (CMD_ID(0x0205))
#define CENTER_RINGERR		    (CMD_ID(0x0206))
#define CENTER_RINGLST		    (CMD_ID(0x0207))
#define CENTER_RINGDOWN		    (CMD_ID(0x0208))
#define CENTER_CALLROOMACK	    (CMD_ID(0x0209))

#define CENTER_STARTMONT	    (CMD_ID(0x0227))
#define CENTER_STARTLSN		    (CMD_ID(0x0228))
#define CENTER_STOPMONT		    (CMD_ID(0x0229))
#define CENTER_STOPLSN		    (CMD_ID(0x0230))
#define CENTER_BROADCASTDATA	(CMD_ID(0x0231))
#define CENTER_BROADCASTPIC	    (CMD_ID(0x0232))
#define CENTER_GETCONFIG	    (CMD_ID(0x0233))
#define CENTER_SETCONFIG	    (CMD_ID(0x0234))
#define CENTER_UPGRADEDOOR	    (CMD_ID(0x0235))
#define CENTER_UPGRADEROOM	    (CMD_ID(0x0236))
#define CENTER_UP_20 	(CMD_ID(0x02B0))//add by hu 2010.08.23
#define CENTER_BONG 	(CMD_ID(0x02B1))//add by hu 2010.08.23
#define CENTER_UPGRADESYSFILE	(CMD_ID(0x023d))//add by hu 2010.08.23
/*下发房间信息*/
#define CENTER_PCGETROOM	    (CMD_ID(0x0237))
#define CENTER_DOWNLOADROOM	    (CMD_ID(0x0238))
#define CENTER_DOWNLOADCARD	    (CMD_ID(0x0239))
//#define CENTER_PCGETCARD        (CMD_ID(0x023a))
#define CENTER_DOWN_ALIAS_TABLE (CMD_ID(0x023a))//add by hu 2010.08.23
#define CENTER_DOWNLOADROOMPWD  (CMD_ID(0x023b))
#define CENTER_GETROOMPWD       (CMD_ID(0x023c))

#define CENTER_BUSY		        (CMD_ID(0x0290))
#define CENTER_IDLE		        (CMD_ID(0x0291))
#define CENTER_HOLDON		    (CMD_ID(0x0292))
#define CENTER_HANGUP		    (CMD_ID(0x0293))
#define CENTER_UNLOCK		    (CMD_ID(0x0294))
#define CENTER_WARNACK     	    (CMD_ID(0x0295))

//2008-10-17
#define CENTER_AGENT            (CMD_ID(0x0296))        //托管
#define CENTER_MINOR            (CMD_ID(0x0297))        //备用
#define CENTER_DIVERT_CENTER    (CMD_ID(0x0298))
#define CENTER_DIVERT_ROOM      (CMD_ID(0x0299))
#define CENTER_CALL_CENTER      (CMD_ID(0x028f))

//2008-11-14
#define CENTER_ASKFOR_SYSINFO_ACK    (CMD_ID(0x0242))
#define CENTER_SEND_SYSINFO          (CMD_ID(0x0243))
#define CENTER_GET_SYSINFO           (CMD_ID(0x0244))
#define CENTER_CENTER_FACTORY_SETTING    (CMD_ID(0x0245))
//2008-11-20 中心恢复密码
#define CENTER_RESET_ROOM_PASS       (CMD_ID(0x0250))          
#define CENTER_RESET_DOOR_PASS       (CMD_ID(0x0251))      //修改主机密码

//2008-11-21 广告牌播放
#define CENTER_AD_SETUP              (CMD_ID(0x02a4))
#define CENTER_AD_STOP               (CMD_ID(0x02a5))

//中心触发设备请求头 2008-11-24
#define CENTER_ACTIVE_SEND_SYSINFO   (CMD_ID(0x0246))

//200903-30
#define CENTER_RESET_DEVICE          (CMD_ID(0x0270))

//=====================门禁指令=============================
//===================中心门禁指令=======================
#define     CENTER_GETCARDRECORD          (CMD_ID(0x0300))
#define  CENTER_CARDREGISTER_ACK          (CMD_ID(0x0301))
#define        CENTER_CARDBACKUP          (CMD_ID(0x0302))
#define       CENTER_CARDRESTORE          (CMD_ID(0x0303))
#define      CENTER_CARDREGISTER          (CMD_ID(0x0304))
#define        CENTER_CARDLOGOUT          (CMD_ID(0x0305))
#define       CENTER_SETCARDTYPE          (CMD_ID(0x0306))
#define       CENTER_SYN_REQUEST          (CMD_ID(0x0307))
#define        CENTER_LOGOUT_ACK          (CMD_ID(0x0308))
#define    CENTER_SYN_RESULT_ACK          (CMD_ID(0x0309))

//=================门口机(围墙机)门禁指令================
#define       DOOR_SENDCARDRECORD         (CMD_ID(0x0150))
#define         DOOR_CARDREGISTER         (CMD_ID(0x0151))
#define       DOOR_CARDBACKUP_ACK         (CMD_ID(0x0152))
#define      DOOR_CARDRESTORE_ACK         (CMD_ID(0x0153))
#define     DOOR_CARDREGISTER_ACK         (CMD_ID(0x0154))
#define       DOOR_CARDLOGOUT_ACK         (CMD_ID(0x0155))
#define     DOOR_LOGOUTCARDBYROOM         (CMD_ID(0x0156))
#define     DOOR_LOGOUTCARDBYCARD         (CMD_ID(0x0157))
#define  DOOR_CENTER_CARDTYPE_ACK         (CMD_ID(0x0158))
#define         DOOR_CARD_SYN_ACK         (CMD_ID(0x0159))
#define   	 DOOOR_SEND_A_CARDLOG         (CMD_ID(0x015A))

//////////////////2009-09-15////////////////////////////////
//===================485 门禁头指令========================
#define D_DOOR_CARD_ENTER                 (CMD_ID(0x0201))
#define D_DOOR_ACK_UNLOCK                 (CMD_ID(0x0202))
#define IP_DOOR_ACK_CARD_ENTER            (CMD_ID(0x0201))
#define IP_DOOR_UNLOCK                    (CMD_ID(0x0202))

//====================电梯联动器指令=======================
#define IP_CALL_OK                        (CMD_ID(0x0F01))
#define IP_CODE_OK                        (CMD_ID(0x0F02))
#define IP_ROOM_CALL_LIFT                 (CMD_ID(0x0F03))
#define IP_CARD_OK                        (CMD_ID(0x0F04))

//====================分机召唤电梯==========================
#define ROOM_CALL_LIFT                    (CMD_ID(0x0030))


/////////////////2008-10-17////////////////////////////////
//中心报警
#define CENTER_SEND_WARNING               (CMD_ID(0x0380))
#define CENTER_HANDLE_WARNING_ACK         (CMD_ID(0x0381))
#define CENTER_CLEAR_WARNING              (CMD_ID(0x0382))

//设备报警
#define DEVICE_SEND_WARNING               (CMD_ID(0x1000))
#define DEVICE_HANDLE_WARNING_ACK         (CMD_ID(0x1001))
#define DEVICE_CLEAR_WARNING              (CMD_ID(0x1002))
#define DEVICE_UPGRADE_ACK                (CMD_ID(0x1180))
#define DEV_UP_20_ACK 	(CMD_ID(0x11B0))//add by hu 2010.08.23

/***2009-11-27 start ***/
#define CENTER_SET_RESET                  (CMD_ID(0x0271))
#define CENTER_GET_RESET                  (CMD_ID(0x0272))
#define CENTER_LOG_ON                     (CMD_ID(0x0273))

#define DEV_CENTER_GET_RESET_ACK          (CMD_ID(0x1184))

#define DEV_SEND_DEBUGLOG                 (CMD_ID(0x1183))
/***2009-11-27 end ***/
//报警id
#define DOOR_UNINSTALLWARNINGID           100
#define DOOR_OPENDOORWARNINGID            101
#define DOOR_FORCEDOPENWARNINGID          102
#define DOOR_UNLOCKEDWARNINGID            103
#endif

