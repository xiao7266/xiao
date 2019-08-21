#ifndef __CTRLCMD_H__
#define __CTRLCMD_H__

#if defined(__cplusplus)
extern "C" {
#endif

#define FILE_LEN  60
#define MAX_FILE_LEN 256

#define SEND_TO_CENTER 1   //是否发送留言和图片到中心
        //#undef SEND_TO_CENTER

#define HAVE_PLAY_JPEG 1

#define HAVE_USED_CLIENT 1
#define SMALLCALL_SOCK_INDEX 65536
#define HAVE_USED_SDL 1     //是否使用SDL中队列，任务，信号量等函数
        //#undef HAVE_USED_SDL
#define DOWNLOAD_FROM_HTTP 1    //是否使用从WEB server中下载数据
        //#undef  DOWNLOAD_FROM_HTTP
        //#define USED_FFS  1
#define USED_NEW_CMD  1       /*采用新版的命令格式*/
        //#undef  USED_NEW_CMD
#define  UDP_PORT         20008

#define CAP_FRAME_TIME  6


#define MONITOR_TYPE_DOOR 0
#define MONITOR_TYPE_SMALLDOOR 1
#define MONITOR_TYPE_GATE     2
#define MONITOR_TYPE_OTHER   3
#define MONITOR_TYPE_IPCAMERA 4

        /*软炸弹
        */
#define USE_BOMB 0x01

//
#define ENGLISH 0x01
#define CHINESE 0x02
        /*
        #ifndef MEM_DEBUG
        #define MEM_DEBUG 1
        #endif

        #ifdef MEM_DEBUG
        #include "zenmalloc.h"
        #endif
        */

        //自定义命令
#define  CUSTOM_CMD_CANCEL        0x900            /*取消之前的主动产生的动作*/
#define  CUSTOM_CMD_PHOTO         0X901            /*手动抓拍*/
#define  CUSTOM_CMD_LEAVE         0X902            /*小门口机留言*/
#define  CUSTOM_CMD_DOWNLOAD_MP3  0X903            /*下载MP3*/
#define  CUSTOM_CMD_DOWNLOAD_OVER 0X904            /*MP3下载完成*/
#define  CUSTOM_CMD_ROOM_STOP_RECORD 0x905          /*停止录音*/

#define  CUSTOM_CMD_CALLER_SHOW_MSG  0x906          /*为主动呼叫库提供的消息*/
#define  CUSTOM_CMD_CALLER_RINGON    0X907
#define  CUSTOM_CMD_CALLER_RINGOFF   0X908
#define  CUSTOM_CMD_CALLER_HOLDON    0x909
#define  CUSTOM_CMD_CALLER_CALLEND   0X910
#define  CUSTOM_CMD_CALLER_PREP      0X911

#define  CUSTOM_CMD_CALL_ROOM        0X912            /*自定义主动呼叫其他房间*/
#define  CUSTOM_CMD_WARNING_TIMER    0X913            /*防区启用定时器命令*/
#define  CUSTOM_CMD_IS_SCREEN_ADJUST  0x914           /*屏幕校准命令*/
#define  CUSTOM_CMD_ROOM_START_RECORD 0x915            /*住家留言(录音)*/

#define CUSTOM_CMD_LOCAL_UPGRADE      0x916

//


#ifdef HAVE_PLAY_JPEG
#define CUSTOM_CMD_RECORD_JPEG_FRAME  0x919
#define CUSTOM_CMD_PLAY_JPEG_FRAME    0X920
#define CUSTOM_CMD_STOP_JPEG_FRAME    0X921
#endif

#define  CUSTOM_SYSTEM_FORMAT         0x922          /*系统正在格式化*/


        //系统状态
#define SYS_BUSY                    100              /*系统繁忙状态*/
#define SYS_IDLE                    0                /*系统空闲状态*/
#define SYS_CENTERCALLROOMTALK      1                /*中心呼叫用户状态*/
#define SYS_TALKTOMAIN              2                /*门口呼叫用户状态*/
#define SYS_MONT                    3                /*监视状态*/
#define SYS_TRANSMITFILE            4
#define SYS_RECVFILE                5
#define SYS_HELP                    6
#define SYS_MONEY                   7
#define SYS_ROOMCALLCENTERTALK      8                /*呼叫中心状态*/
#define SYS_ROOMTALKROOM            9                /*被动户户通状态*/
#define SYS_GETSYSINFO              10               /*获得系统信息状态*/
#define SYS_WARNING                 11               /*报警状态*/
#define SYS_REQUESTFEE              12               /*请求查询费用状态*/
#define SYS_REQUESTREPAIR           13               /*请求维修状态*/
#define SYS_REQEUSTRINGLIST         14               /*请求铃声列表状态*/
#define SYS_DOWNLOADRING            15               /*下载铃声状态*/
#define SYS_EMERG                   16               /*紧急求助状态*/
#define SYS_GETROOMIP               17               /*获得分机的IP*/
#define SYS_LEAVEWORD               18               /*系统留言*/
#define SYS_UPGRADE                 19               /*系统升级*/
#define SYS_GETTIME                 20               /*系统获取时间*/
#define SYS_SMALLDOORCALL           21               /*小门口机呼叫状态*/
#define SYS_MONTSMALLDOOR           22               /*监视小门口机*/
#define SYS_MYROOMTALKROOM          23               /*主动户户通状态*/
#define SYS_REQDOCINFO              24               /*分机请求便民信息*/
#define SYS_ACTIVE_CALL             25               /*系统进入主动呼叫状态*/
#define SYS_REQHELPINFO             26               /*分机请求帮助信息*/
#define SYS_ISSCREENADJUST          27               /*进入校准屏幕*/
#define SYS_ISRECORD                28               /*系统进入录音状态*/
#define SYS_CALLSIP                 29               /*系统进入呼叫SIP电话*/
#define SYS_SIPCALL                 30               /*系统进入SIP 呼叫终端*/
        //#define SYS_CALL_LIFT               29               /*系统进入呼叫电梯状态*/
#define SYS_FORMAT                  31                     /*系统进入格式化状态*/

        //分机指令
#define ROOM_CALLCENTER		      0x0001               /*呼叫中心*/
#define ROOM_EMERG		          0x0002               /*紧急求助*/
#define ROOM_INQFEE		          0x0003               /*费用查询*/
#define ROOM_REPAIR		          0x0004               /*请求维修*/
#define ROOM_BROWSERING		      0x0005               /*请求铃声列表*/
#define ROOM_RINGDOWN		      0x0006               /*铃声下载*/
#define ROOM_GETTIME		      0x0007               /*获得时间*/
#define ROOM_WARNING              0x0008               /*报警*/
#define ROOM_CALLROOM		      0x0009               /*户户通*/
#define ROOM_INQADDR              0x000a               /*请求对应房间号的IP地址*/
#define ROOM_GETCONFIGACK         0x000b               /*回应请求系统信息*/

#define ROOM_ASKFOR_SYSINFO       0X000C                /*分机主动请求分机配置信息*/
#define ROOM_GET_SYSINFO_ACK      0x000d                /*分机呼应基本信息*/
#define ROOM_INQSERVICEINFO       0x000e                /*房间请求便民信息，暂时自定义*/
#define ROOM_INQHELPINFO          0X000F                /*分机请求帮助信息 */

#define ROOM_GET_WEATHER          0x0019                /*分机向中心请求天气预报*/
        //
#define ROOM_STARTMONT		      0x0020               /*开始监视*/
#define ROOM_STOPMONT		      0x0021               /*停止监视*/
        //家居与中心家居协议
#define ROOM_STATUSACK            0x0010               /*房间回应查询所有的家居状态*/
#define ROOM_SETELECACK           0x0011               /*回应中心控制家居*/
#define ROOM_GETELECACK           0x0012               /*回应中心查询某个家居的状态*/
#define ROOM_SCENEACK             0x0013               /*回应中心情景控制*/

        /*2009-7-29  new add cmd*/
#define ROOM_REMOTE_SWITCH  0X0015             /*通知中心，分机启用、关闭远程控制*/
#define ROOM_SIGNLE_GUARD_ACK 0X0016       /*回应中心，单独控制防区，主要为电话控制所用*/
#define ROOM_GET_ELECFIILE_ACK   0x0017      /*回应中心，获得家居配置表*/
#define ROOM_SET_ELECFILE_ACK   0X0018      /*回应中心，设置家居配置表*/

#define ROOM_SEND_PHOTO_VOICE   0X1107              /*分机发送抓拍图片到中心*/


        //
#define ROOM_BUSY		              0x0090              /*繁忙*/
#define ROOM_IDLE	                  0x0091              /*空闲*/
#define ROOM_HOLDON		              0x0092              /*摘机*/
#define ROOM_HANGUP		              0x0093              /*挂机*/
#define ROOM_UNLOCK		              0x0094              /*开锁*/
#define ROOM_AGENT                    0x0095              /*分机托管*/
#define ROOM_QUIET                    0x0096              /*分机免扰*/
#define ROOM_BROADCASTACK             0x0022              /*回应收到公告成功/失败*/
#define ROOM_UPGRADEACK               0x0023              /*升级回应*/
#define ROOM_LEAVEWORDACK             0x0024              /*留言回应*/
#define ROOM_STOPLEAVEWORDACK         0x0025              /*回应留言结束*/

#define ROOM_CALL_LIFT                0x0030              /*召唤电梯*/
#define ROOM_SEND_COLOR_MSG           0x0031              /*分机请求中心发送彩色短信*/


        //分机与分机之间的命令

#define ROOM_SYNC_AGENT               0x0080              /*分机同步托管*/
#define ROOM_CANCEL_SYNC_AGENT        0x0081              /*分机取消同步托管*/
#define ROOM_SYNC_AVOID               0x0082              /*分机同步免打扰*/
#define ROOM_CANCEL_SYNC_AVOID        0x0083              /*分机取消同步免打扰*/
#define ROOM_SYNC_RING_TIME           0x0084              /*分机铃声时间同步*/
#define ROOM_SMALLDOOR_HOLDON         0X0085              /*小门口机，摘机同步*/
#define ROOM_SMALLDOOR_HANGUP         0X0086              /*小门口机呼叫，挂机同步*/
#define ROOM_REQ_SYNC                 0X0087              /*从分机启动时向主分机发送请求同步的消息*/
#define ROOM_HOST_BUSY                0X0088              /*如果是多分机，小门口机呼叫，主分机繁忙，发送该同步消息*/
#define ROOM_SEND_KEY                 0x0089              /*分机发送键*/
#define ROOM_USED_SIMULATE            0x008a              /*是否有模拟通道在使用。*/
#define ROOM_USED_AGENT               0x008b              /*有分机手动托管*/


#define ROOM_SCR_ACK                  0x0097

#define ROOM_SEND_ALARM                  0X1000        /*房间紧急报警*/



        //主机指令

#define DOOR_CALLCENTER		        0x0101
#define DOOR_MONTACK		        0x0102
#define DOOR_BUSY			        0x0103
#define DOOR_UNLOCKACK		        0x0104
#define DOOR_GETTIME		        0x0105
#define DOOR_CALLROOM		        0x0106
#define DOOR_HANGUP			        0x0107
#define DOOR_ENDMONT                0x0108


#define DOOR_NOPERSON		        0x01a5
#define DOOR_OVERTIMETALK	        0x01a6
#define DOOR_ROOMRINGOFFTIME        0x01a7
#define DOOR_MONITORTIME	        0x01a8

#define DOOR_BROADCAST              0x010a             /*转发公告*/
#define DOOR_BROADCASTPIC           0x010b             /*转发的图片公告*/
#define DOOR_STARTLEAVEWORD         0x010e             /*留言*/
#define DOOR_STOPTLEAVEWORD         0x010f             /*结束留言*/


        //中心指令
#define CENTER_EMERGACK		        0x0201             /*中心回应紧急求助*/
#define CENTER_INQFEEACK	        0x0202             /*中心回应费用查询*/
#define CENTER_REPAIREACK	        0x0203             /*中心回应请求维修*/
#define CENTER_TIMEACK		        0x0204             /*中心回应获得时间*/
#define CENTER_CALLROOM		        0x0205             /*中心呼叫用户*/
#define CENTER_RINGERRACK           0x0206             /*中心回应获取铃声失败*/
#define CENTER_RINGLSTACK           0x0207             /*中心回应获取铃声列表*/
#define CENTER_RINGDOWNACK          0x0208             /*中心回应铃声下载*/
#define CENTER_WEATHER_ACK          0x0209             /*中心应答查询IP地址*/

#define CENTER_SERVERINFOACK        0x020a             /*中心回应获取便民信息，暂时自定义了*/

#define CENTER_STARTMONT	        0x0227             /*中心启动监视*/
#define CENTER_STARTLSN		        0x0228             /*中心启动监听*/
#define CENTER_STOPMONT		        0x0229             /*中心停止监视*/
#define CENTER_STIPLSN		        0x0230             /*中心停止监听*/
#define CENTER_SETIDTOIPTABLE       0X0238             /*中心设置ID TO IP 表*/
#define CENTER_GETIDTOIPTABLE       0x237              /*中心获得地址表*/
#define ROOM_ACK_GETIDTOIP          0x0111             /*房间回应获得分机地址表*/

#define CENTER_SEND_RESET           0x0270             /*中心重置分机*/
#define CENTER_BUSY			        0x0290             /*中心繁忙*/
#define CENTER_IDLE			        0x0291             /*中心空闲*/
#define CENTER_HOLDON		        0x0292             /*中心摘机*/
#define CENTER_HANGUP		        0x0293             /*中心挂机*/
#define CENTER_UNLOAK		        0x0294
#define CENTER_WARNACK              0x0295             /*中心应答报警*/
        //#define CENTER_RELOGIN              0x0296

#define CENTER_AGENT                0x0296             /*中心托管*/
#define CENTER_MINOR                0x0297             /*中心启动备用机*/
#define CENTER_DIVERT_CENTER        0x0298             /*中心转接到管理机*/
#define CENTER_DIVERT_ROOM          0x0299             /*中心转接到分机*/

#define CENTER_ASKFOR_SYSINFO_ACK   0x0242              /*中心回应请求配置信息*/
#define CENTER_GET_SYSINFO          0x0244              /*中心查询获得系统信息*/
#define CENTER_SEND_SYSINFO         0x0243              /*中心更新系统信息*/
#define CENTER_FACTORY_SETTING      0x0245              /*中心设置IP出厂设备*/

//#define CENTER_NOPERSON		              0x02a5
//#define CENTER_OVERTIMETALK	            0x20a6
#define CENTER_SET_ROOM_SCR         0x02a6              /*中心设置分机图片（屏保，主界面等）*/

#define CENTER_BROADCASTDATA            0x231            /*中心广播文字公告*/
#define CENTER_BROADCASTPIC             0x232            /*中心广播图片公告*/

#define CENTER_UPGRADEROOM              0x236            /*中心升级分机软件*/
#define DEV_UPGRADE_ACK                 0x1180           /*中心升级应答指令*/

#define BLTN_SETUP                      0x2A0            /*中心下发帮助图片*/

#define CENTER_IS_DEVONLINE             0x02F0          /*中心在线检测设备*/
#define DEVONLINE_ACK                   0X1181           /*设备回应*/

        //家居与中心家居协议
#define CENTER_GETALLSTATUS             0x0210           /*中心查询所有的家居状态*/
#define CENTER_SETELEC                  0x0211           /*中心控制家居*/
#define CENTER_GETELEC                  0x0212           /*中心查询某个家居的状态*/
#define CENTER_SETSCENE                 0x0213           /*中心设置情景模式*/


        /*2009-7-29  new  add cmd*/
#define CENTER_REMOTE_SINGLE_GUARD          0x0216       /*中心远程单独控制防区*/
#define CENTER_GET_ELEC_CONFIG              0x0217       /*中心获得家居配置表*/
#define CENTER_SET_ELEC_CONFIG              0X0218       /*中心设置家居配置表*/

        /*----------------------中心远程控制安防指令---------------------------------*/
#define CENTER_ARRANGE_GUARD                0X256           /*中心远程控制防区*/
#define CENTER_CHECK_GUARD                  0X257           /*中心获得防区配置*/
#define CENTER_RESET_ROOM_PASS              0X250           /*中心重置分机密码*/
#define ROOM_RESET_PASS_ACK                 0XA01           /*分机应答中心密码*/
#define ROOM_CENTER_ARRANGE_GUARD_ACK       0XA02           /*分机应答远程控制防区*/
#define ROOM_CENTER_CHECK_GUARD_ACK         0XA04           /*分机应答中心获得防区状态*/

        /*-----------------------------------2010/8/2---------------------------------------*/
#define CENTER_BOMB                         0x02b1
#define CENTER_UP_20                        0x02b0
#define DEV_UP_20_ACK                       0x11b0

        /*----------------------------------2010-5-31---------------------------------*/
#define ROOM_SERVICES_REQUEST               0x0014
#define CENTER_SERVICES_ACK                 0X020b


        /*********************************************************************************
        小门口机与分机命令
        *********************************************************************************/
#define SMALLDOOR_IDLE                      0x601           //小门口空闲
#define SMALLDOOR_ACK_STOPMONT              0x602           //小门口机应答停止监视
#define SMALLDOOR_CALLROOM                  0x603           //小门口机呼叫分机
#define SMALLDOOR_ACK_HOLDON                0x604           //小门口机应答摘机
#define SMALLDOOR_ACK_OPENLOCK              0x605           //小门口机应答开锁
#define SMALLDOOR_ACK_HANGUP                0x606           //小门口应答挂机
#define SMALLDOOR_ACK_LEAVE                 0x607           //小门口应答留言
#define SMALLDOOR_ACK_VERSION               0x608           //小门口应答版本

#define ROOM_START_MONT_SMALLDOOR           0x601           //分机监视小门口
#define ROOM_STOP_MONT_SMALLDOOR            0x602           //分机停止监视小门口
#define ROOM_ACK_SMALLDOOR                  0x603           //分机应答小门口机空闲
#define ROOM_HOLDON_SMALLDOOR               0x604           //分机摘机小门口机
#define ROOM_OPENLOCK_SMALLDOOR             0x605           //分机开锁小门口机
#define ROOM_HANGUP_SMALLDOOR               0x606           //分机挂机小门口机
#define ROOM_SMALLDOOR_LEAVE                0x607           //分机留言
#define ROOM_GET_SMALLDOOR_VER              0x608           //分机获取版本

        /*2009-7-29 13:45:07 new add cmd 分机于RIU*/
#define RIU_KEY                             0X0635          /*RIU key*/        
#define ROOM_RIU_DEFEND_ACK                0X0636          /*分机应答RIU全部布防*/
#define ROOM_RIU_REMOVE_DEFEND_ACK          0x0637          /*分机应答RIU全部撤防*/
#define ROOM_RIU_OPEN_LOCK_ACK              0x0638          /*分机应答RIU开锁*/
#define ROOM_RIU_REMOVE_ACK                 0X0639          /*分机应答RIU无条件撤防*/
#define ROOM_RIU_ROOM_WARNING_ACK           0x0640          /*分机应答RIU紧急报警*/       

#define RIU_ROOM_DEFEND                     0X0636          /*RIU 通知分机布防*/
#define RIU_ROOM_REMOVE_DEFEND              0X0637          /*RIU通知分机撤防*/
#define RIU_ROOM_OPEN_LOCK                  0X0638          /*RIU通知分机开锁*/
#define RIU_ROOM_REMOVE                     0X0639          /*RIU通知分机无条件撤防*/
#define RIU_ROOM_WARNING                    0x0640          /*RIU通知分机紧急报警*/

//modify [2012-12-21 13:54  add cmd by wyx]
#define MEDIA_NEGOTIATE                0x1190
#define MEDIA_NEGOTIATE_ACK            0x1191

#define ROOM_STOP_MONT_BYKEY           0x1192 //仅用于本地程序内部区别触摸摘机与按键摘机

//add by wyx [2014-6-9]
#define CENTER_SET_IOLEVEL			   0x1193
#define CENTER_SET_IOLEVEL_ACK		   0x1194

//add by wyx [2014 6 26] 转发媒体协商至从机
#define MEDIA_NEGOTIATE_SESSION		   0x1195
//add by wyx [2014 6 26] 转发房间地址列表至从机 
#define TRANSMIT_IP_TO_OTHERROOM	   0x1196
//add by wyx [2014 7 2] 分机监视监控设备
#define IPCAMERA_MONITOR_START		   0x1197
#define IPCAMERA_MONITOR_END	   0x1198

//2011-12-7 16:28:56
#define ROOM_ALARM_1                        0X0F01 //欠压
#define ROOM_ALARM_2                        0X0F02 //电压恢复
#define ROOM_ALARM_3                        0X0F03//电网断电
#define ROOM_ALARM_4                        0X0F04 //电网恢复

//2012-2-14 //兼容新版的。
#define ROOM_ALARM_11                        0X0F11 //欠压
#define ROOM_ALARM_12                        0X0F12 //电压恢复
#define ROOM_ALARM_13                        0X0F13//电网断电
#define ROOM_ALARM_14                        0X0F14 //电网恢复

        /*2009-10-26,new add cmd 分级与非可视分机*/ 
// modify [2011年3月21日 8:46:24 by ljw]
#define  CALL_SIMULATE                0x609               /*呼叫转移到非可视分机*/
#define  SIMULATE_HOLDON              0x60a
#define  SIMULATE_HANGUP              0x60b
#define  SIMULATE_UNLOCK              0x60c
#define  SIMULATE_ALARM               0x60d
#define  INFORM_SIMULATE_HANGUP       0x60e

#define ROOM_ACK_SIMULATE_ALARM       0x60d
#define ROOM_ACK_SIMULATE_UNLOCK      0x60c
#define ROOM_ACK_SIMULATE_HANGUP      0x60b
#define ROOM_ACK_SIMLUATE_HOLDON      0x60a

        /**************************************SIP caller**********************/
#define  SIP_CALL                   0x0301
#define  SIP_CALL_PROCESSING        0X0302
#define  SIP_BUSY                   0x0303
#define  SIP_CALL_RING              0x0304
#define  SIP_CLOSE                  0x0305
#define  SIP_START_AUDIO            0x0306

        /*--------------------------------------------------------------*/

        /*
        * 手机控制家居、防区协议
        */
#define PHONE_INV_ROOM              0x0611
#define PHONE_INQ_ALL_HOME_STATUS   0x0612
#define PHONE_CTRL_MORE_SAFE_STATUS 0X0613
#define PHONE_CTRL_SAFE_STATU       0x0614
#define PHONE_INQ_ALL_SAFE_STATUS   0x0615
#define PHONE_CTRL_SCENE            0x0616
#define PHONE_CTRL_ONE_DEVICES      0x0617
#define PHONE_INQ_ONE_DEVICES       0x0618
#define PHONE_ACK_KEEP_LIVE         0x0619


#define ROOM_ACK_INV_PHONE                       0x0811
#define ROOM_ACK_ALL_HOME_STATUS                 0x0812
#define ROOM_ACK_CTRL_MORE_SAFE_STATUS           0x0813
#define ROOM_ACK_CTRL_SAFE_STATUS                0x0814
#define ROOM_ACK_INQ_ALL_SAFE_STATUS             0x0815
#define ROOM_ACK_CTRL_SCENE                      0x0816
#define ROOM_ACK_CTRL_ONE_DEVICES                0x0817
#define ROOM_ACK_INQ_ONE_DEVICES                 0x0818
#define ROOM_KEEP_LIVE                           0x0819


        /*************************************************************
        add by ljw
        *************************************************************/
        typedef void (*display)(unsigned long event[4]);
        static const int STATUS_BASE=0x1000;

#define  STATUS_CENTER_CALLROOM              STATUS_BASE+0            /*通知GUI，中心呼叫分机*/
#define  STATUS_DOOR_CALLROOM                STATUS_BASE+1            /*通知GUI，门口机呼叫分机*/
#define  STATUS_CENTER_HANGUP                STATUS_BASE+2            /*通知GUI，中心挂机*/
#define  STATUS_DOOR_HANGUP                  STATUS_BASE+3            /*通知GUI，门口机呼叫分机*/
#define  STATUS_DOOR_BUSY                    STATUS_BASE+4            /*通知GUI，门口机繁忙*/
#define  STATUS_CENTER_BUSY                  STATUS_BASE+5            /*通知GUI，中心繁忙*/
#define  STATUS_CENTER_HOLDON                STATUS_BASE+6            /*通知GUI，中心摘机*/
#define  STATUS_ROOM_STOPMONT                STATUS_BASE+7            /*通知GUI 停止监视*/
#define  STATUS_ROOMTALKROOM                 STATUS_BASE+8            /*通知GUI，分机收到别的分机的呼叫*/
#define  STATUS_ROOM_BUSY                    STATUS_BASE+9            /*通知GUI，分机繁忙*/
#define  STATUS_NET_EXCEPTION                STATUS_BASE+10           /*通知GUI，网络异常，连接中断*/
#define  STATUS_DOOR_UNLOCK                  STATUS_BASE+11           /*通知GUI，门口机开锁成功*/
#define  STATUS_ROOM_WARING_OK               STATUS_BASE+12           /*通知GUI，报警成功*/
#define  STATUS_GET_RING_ERROR               STATUS_BASE+13           /*通知GUI，获取铃声错误*/
#define  STATUS_RING_LIST                    STATUS_BASE+14           /*通知GUI，铃声列表*/
#define  STATUS_RING_DOWNLOAD                STATUS_BASE+15           /*通知GUI，铃声下载*/
#define  STATUS_REQUEST_REPAIR               STATUS_BASE+16           /*通知GUI，维修*/
#define  STATUS_PUBLIC_TEXT_INFO             STATUS_BASE+17           /*通知GUI，发布文本公告*/
#define  STATUS_PUBLIC_PIC_INFO              STATUS_BASE+18           /*通知GUI，发布图片公告*/
#define  STATUS_ROOM_HANGUP                  STATUS_BASE+19           /*通知GUI，分机挂机*/
#define  STATUS_QUERY_FEELIST                STATUS_BASE+20           /*通知GUI，中心回应费用查询*/
        //#define  STATUS_SET_SYS_INFO                 STATUS_BASE+21           /*通知GUI，中心设置系统信息*/
#define  STATUS_DOOR_LEAVEWORD               STATUS_BASE+22           /*通知GUI，门口机留言*/
#define  STATUS_ROOM_EMERG                   STATUS_BASE+23           /*通知GUI，紧急求救成功*/
#define  STATUS_ROOM_SYS_UPDATE              STATUS_BASE+24           /*通知GUI，系统升级中*/
        //#define  STATUS_ROOM_GET_IP                  STATUS_BASE+25           /*通知GUI，获取ROOM地址*/
#define  STATUS_ROOM_CALLING_ROOM            STATUS_BASE+26           /*通知GUI，正在呼叫ROOM*/
#define  STATUS_ROOM_TALK_ROOM               STATUS_BASE+27           /*通知GUI，开始通话*/
#define  STATUS_CONNECTING                   STATUS_BASE+28           /*通知GUI，正在连接*/
#define  STATUS_RING_LIST_OVER               STATUS_BASE+30           /*通知GUI,铃声列表完毕*/
#define  STATUS_FEE_LIST_OVER                STATUS_BASE+31           /*通知GUI，费用列表完毕*/

#define STATUS_START_EMERG                   STATUS_BASE+32           /*通知开始报警*/
        //
        //#define STATUS_ROOM_GET_IP_ERROR            STATUS_BASE+29             /*GET IP error */

#define STATUS_ROOM_VIEW_TIME               STATUS_BASE+33             /*通知GUI显示事件*/

#define STATUS_SMALLDOOR_CALL_ROOM          STATUS_BASE+34             /*通知GUI，小门口机呼叫分机*/
#define STATUS_SMALLDOOR_BUSY               STATUS_BASE+35             /*小门口机繁忙*/

#define STATUS_AVOID_TIMES_TIMEOUT          STATUS_BASE +36           /*通知GUI，免打扰时间到了*/

#define STATUS_SAFE_WARNING                 STATUS_BASE+37            /*防区报警*/

#define STATUS_PAUSE_VIDEO                  STATUS_BASE+38            /*视频暂停，播放*/

#define STATUS_REQDOCINFO_OVER              STATUS_BASE+39            /*获得便民信息结束*/


#define STATUS_START_WARNING                STATUS_BASE+40

#define STATUS_REMOTE_ALARM                 STATUS_BASE+41           /*远程控制布防撤防*/

#define STATUS_RECORD_START                 STATUS_BASE+42           /*请开始录音*/
#define STATUS_RECORD_STOP                  STATUS_BASE+43           /*停止录音*/
#define STATUS_REMOTE_SCENE                STATUS_BASE+44          /*远程控制情景*/

#define STATUS_RMCORD_TIME                  STATUS_BASE+45         /*录音时间*/
#define STATUS_LEAVE_PLAY_OVER              STATUS_BASE+46           /*留言播放完毕*/

#ifdef HAVE_PLAY_JPEG
#define STATUS_PLAY_JPEG                    STATUS_BASE+47
#define STATUS_DISPLAY_REC                  STATUS_BASE+48
#endif

        //sip status
#define STATUS_SIP_CONNECTING               STATUS_BASE+49
#define STATUS_SIP_CLOSE                    STATUS_BASE+50
#define STATUS_SIP_BUSY                     STATUS_BASE+51
#define STATUS_SIP_START_TALK               STATUS_BASE+52
#define STATUS_SIP_CALL_ROOM              STATUS_BASE+53

        //2010/10/8
#define STATUS_CAP_NEW_PHONE               STATUS_BASE+54
#define STATUS_SCREEN_CLOSE                STATUS_BASE+55
//2011-4-28 15:37:34
#define STATUS_CHECK_JIAJU                 STATUS_BASE+56

//2011-5-16 15:13:02
#define STATUS_ALARM_CLOCK                 STATUS_BASE+57

//2011-8-11 10:47:38
#define STATUS_WEATHER_RESULT              STATUS_BASE+58

#define STATUS_REMOTE_UPATE_SYSTEM_INFO    STATUS_BASE+59             /*远程更新了系统信息*/
#define STATUS_DIVERT_ROOM_TO_ROOM    	   STATUS_BASE+60             /*远程更新了系统信息*/
#define STATUS_DIVERT_ROOM_TO_CENTER       STATUS_BASE+61             /*远程更新了系统信息*/
#define  STATUS_DOOR_LEAVEWORD_STOP        STATUS_BASE+62           /*通知GUI，门口机留言*/
#define  STATUS_TS_CAL        				STATUS_BASE+90  
#define  STATUS_PHONE_OR_EXTERNDEVICE_HOLDON   STATUS_BASE+91 
#define STATUS_IOLEVEL_TEST					STATUS_BASE+92
#define STATUS_KEYLOCK_EVENT				STATUS_BASE+93
#define STATUS_KEYCALL_EVENT				STATUS_BASE+94
#define STATUS_KEYHOLD_EVENT				STATUS_BASE+95
#define STATUS_SET_IOLEVEL_EVENT			STATUS_BASE+96

#define STATUS_DEBUG_MSG                    STATUS_BASE+100           /*debug msg*/



        //////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////
        //定义计时事件
#define TIME_MONT_EVENT_START           0x1      //监视计时开始
#define TIME_MONT_EVENT_TIMEOUT      0x2      //计时器时间到
#define TIME_MONT_EVENT_STOP            0x4      //监视计时停止

#define TIME_MONT2_EVENT_START           0x11      //监视计时开始
#define TIME_MONT2_EVENT_TIMEOUT      0x21      //计时器时间到
#define TIME_MONT2_EVENT_STOP            0x41      //监视计时停止
#define TIME_RING_EVENT_START            0x8      //铃声计时开始
#define TIME_RING_EVENT_TIMEOUT       0x10      //计时器时间到
#define TIME_RING_EVENT_STOP             0x20      //铃声计时停止

#define TIME_TALK_EVENT_START           0x40      //通话计时开始
#define TIME_TALK_EVENT_TIMEOUT      0x80
#define TIME_TALK_EVENT_STOP             0x100      //通话计时停止

#define TIME_WARNING_EVENT_START        0x200     //报警或者紧急求助计时开始
#define TIME_WARNING_EVENT_TIMEOUT     0x400     //
#define TIME_WARNING_EVENT_STOP         0x800    //报警或者紧急求助计时停止



#define TIME_MP3_EVENT_START            0x1000     //主要针对的是短消息铃声
#define TIME_MP3_EVENT_TIMEOUT       0x2000    //MSG时间超时
#define TIME_MP3_EVENT_STOP             0x4000     //MSG计时器停止



#define TIME_SMALLDOOR_LEAVE_EVENT_START 0x8000     //小门口留言开始
#define TIME_SMALLDOOR_LEAVE_EVENT_TIMEOUT 0x10000    //超时
#define TIME_SMALLDOOR_LEAVE_EVENT_STOP    0x20000   //停止

#define TIME_AVOID_SETTIMES_EVENT_START        0x40000   //免打扰时间设置事件启动
#define TIME_AVOID_SETTIMES_EVENT_TIMEOUT      0x80000   //免打扰时间设置事件超时
#define TIME_AVOID_SETTIMES_EVENT_STOP         0x100000   //免打扰时间设置事件停止

#define TIME_UNLOCK_EVENT_START                0x200000   //开锁后，5秒挂机的事件启动
#define TIME_UNLOCK_EVENT_TIMEOUT              0x400000  //超时
#define TIME_UNLOCK_EVENT_STOP                 0x800000  //停止

#define TIME_DOOR_LEAVE_EVENT_START            0x1000000  //门口机留言开始
#define TIME_DOOR_LEAVE_EVENT_TIMEOUT       0x2000000  //门口机留言超时
#define TIME_DOOR_LEAVE_EVENT_STOP             0x4000000  //门口机留言计时停止

#define TIME_AGENT_SETTIMES_EVENT_START        0x8000000   //托管时间设置事件启动
#define TIME_AGENT_SETTIMES_EVENT_TIMEOUT      0x10000000   //托管时间设置事件超时
#define TIME_AGENT_SETTIMES_EVENT_STOP         0x20000000   //托管时间设置事件停止

        ///////////////////////////////////////////////////////////
        //timer task 2

#define TIME_DISPLAY_TIME_EVENT_START   0x1     //显示时间事件启动，1分钟显示一次当前时钟 //timer2
#define TIME_DISP_TIME_EVENT_TIMEOUT    0x2     //事件超时。 //timer2

#define TIME_SMALLDOOR_EVENT_START      0x4      //小门口机事件 //timer2
#define TIME_SMALLDOOR_EVENT_TIMEOUT    0x8 //time2
#define TIME_SMALLDOOR_EVENT_STOP       0x10   //timer2

#define TIME_RECORD_EVENT_START                    0x20 //开始录音计时
#define TIME_RECORD_EVENT_TIMEOUT               0x40 //开始录音计时
#define TIME_RECORD_EVENT_STOP                    0x80//开始录音计时
#if 1
#define TIME_JIAJU_CHECK_START            0x100     //启动已经连接的网络检查 5s检查一次
#define TIME_JIAJU_CHECK_STOP             0x200
#define TIME_JIAJU_CHECK                       0x400     //检查网络

#endif

#ifdef HAVE_PLAY_JPEG

#define TIME_PLAY_JPEG_START                    0x800
#define TIME_PLAY_JPEG                          0x1000
#define TIME_PLAY_JPEG_STOP                     0x2000

#endif
/*欠压报警计时器*/
#define TIME_LOWTENSION_WARNING_EVENT_START               0x4000
#define TIME_LOWTENSION_WARNING_EVENT_TIMEOUT             0x8000
#define TIME_LOWTENSION_WARNING_EVENT_STOP                0x10000
        


        /////////////////////////////////////////////////////////////////////////
#define CENTER_CALL_RING         0
#define WALL_CALL_RING           1
#define DOOR_CALL_RING           2
#define SMALL_DOOR_CALL_RING     3
#define ROOM_CALL_RING           4
#define WARNING_RING             5
#define MSG_RING                 6
#define ALARM_CLOCK_RING         7


        typedef struct socket_packet_t{
                int sock_index;
                int  validlen;
                unsigned char buf[256];
        }SOCKET_PACKET,*PSOCKET_PACKET;

        typedef struct socket_packet_t2{
                int sock_index;
                int  validlen;
                unsigned char* buf;
        }SOCKET_PACKET2,*PSOCKET_PACKET2;

        typedef struct _tagcmd{
                char cmdhead[4];
                int len;
                short cmd;
        }CmdHeader,*PCmdHeader;

        typedef struct _tagRoomWaring{
                unsigned long ipaddr;
                unsigned char type;
                short    cmd;
        }ROOMWARING,*PROOMWARING;
        /*
        typedef struct _tagGUICMD{
        short cmd;
        int   len;
        unsigned char type[1];
        }GUICMD,*PGUICMD;
        */
        typedef struct _tagPHOTOINFO{
                int            lock;
                int            idle;
                unsigned char  name[FILE_LEN];
                unsigned char  *addr;
                unsigned long   len;
                int             read;  //高2字节表示地点，0，门口机，1，小门口机，2，围墙机。 低2字节，表示是否阅读，0，未读，1已读。
                unsigned long   dates;
                unsigned long   times;
        }PHOTOINFO;

        typedef struct _tagVOICEINFO{
                int            lock;
                int            idle;
                unsigned char  name[FILE_LEN];
                unsigned char  *addr;
                unsigned long   len;
                int             read;
                unsigned long   dates;
                unsigned long   times;
        }VOICEINFO;

        typedef struct _tagDOWNRINGINFO{
                int           idle;
                int 	      lock;
                char          name[MAX_FILE_LEN];
        }DOWNRINGINFO;

        typedef struct _tagringlist{
                bool isdown;
                char name[41];
                unsigned char* addr;
                unsigned long len;
                struct _tagringlist* next;
        }RINGLIST;

        typedef struct _tagfeelist{
                unsigned char month;
                unsigned char buf[33];
                struct _tagfeelist* next;
        }FEELIST;

        typedef struct _tagLOGINFO{
                int len;
                //#ifdef USED_FFS
                unsigned char   addr[400];
                //#else
                //   unsigned char*  addr;
                //#endif
                int             read;
                int             idle;
                int             lock;
                unsigned long   dates;
                unsigned long   times;
        }LOGINFO;


        typedef struct _tagMSGINFO{
                int read;
                int len;
                //#ifdef USED_FFS
                unsigned char addr[400];
                //#else
                //   unsigned char* addr;
                //#endif
                int  lock;
                int             idle;
                unsigned long   dates;
                unsigned long   times;
        }MSGINFO;

        typedef struct _tagRINGSET{
                //unsigned char* addr;
                //long size;
                char ring_path[MAX_FILE_LEN];
        }RINGSET;

        typedef struct _tagtelephone{
                int idle;
                int lock;
                char name[21];
                char room_number[16];
                unsigned long ipaddr[4];
                unsigned long   dates;
                unsigned long   times;
        }TELEPHONE;

        typedef struct _tagROOMINFO{
                int valid;
                int id;
                unsigned long ipaddr;
        }ROOMINFO;

#if defined(__cplusplus)
}
#endif
#endif

