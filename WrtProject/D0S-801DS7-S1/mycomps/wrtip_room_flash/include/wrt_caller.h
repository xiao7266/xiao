/*
文件名：   tmWRTICaller.h
功能：   数字ip设备呼叫功能模块
版本：   Version 0.1
创建日期：   2008-10-22
版权：   WRT 
作者：   胡循锋
*/
#ifndef _TM_WRTI_CALLER_H_
#define _TM_WRTI_CALLER_H_
#define PROTOCOL_VERSION2

#define MSGID_START_CALL_ROOM                            10000                         //开始呼叫分机
#define MSGID_START_CALL_CENTER                          (MSGID_START_CALL_ROOM+1)     //开始呼叫中心
#define MSGID_START_CALL_ANOTHRE_CENTER                  (MSGID_START_CALL_ROOM+2)     //开始呼叫备用中心
#define MSGID_DEVICE_CONNECTING                          (MSGID_START_CALL_ROOM+3)     //正在连接目标...
#define MSGID_CONNECTED_ERROR                            (MSGID_START_CALL_ROOM+4)     //连接目标失败
#define MSGID_SENDING_CALL_REQUEST                       (MSGID_START_CALL_ROOM+5)     //正在发送呼叫请求
#define MSGID_SEND_CALL_REQUEST_ERROR                    (MSGID_START_CALL_ROOM+6)     //发送呼叫请求失败
#define MSGID_WAIT_CALL_REQUEST_ACK                      (MSGID_START_CALL_ROOM+7)     //等待目标响应...
#define MSGID_CENTER_BUSY                                (MSGID_START_CALL_ROOM+8)     //中心繁忙
#define MSGID_CENTER_AGENT_CONNECTING                    (MSGID_START_CALL_ROOM+9)     //中心已托管,转接中...
#define MSGID_CENTER_MINOR_CONNECTING                    (MSGID_START_CALL_ROOM+10)     //中心备用,转接中...
#define MSGID_CENTER_DIVERT_TIMEOUT                      (MSGID_START_CALL_ROOM+11)     //中心转接次数太多
#define MSGID_ROOM_QUIET_MODE                            (MSGID_START_CALL_ROOM+12)     //房间已设免打扰
#define MSGID_ROOM_DIVERT_TIMEOUT                        (MSGID_START_CALL_ROOM+13)     //分机转接次数太多...
#define MSGID_ROOM_AGENT_CONNECTING                      (MSGID_START_CALL_ROOM+14)     //房间已托管,转接中...
#define MSGID_ROOM_BUSY                                  (MSGID_START_CALL_ROOM+15)     //房间繁忙...
#define MSGID_NO_CALL_REQUEST_ACK                        (MSGID_START_CALL_ROOM+16)     //目标无应答...
#define MSGID_CENTER_RINGON                              (MSGID_START_CALL_ROOM+17)     //"呼叫中心响铃中,挂机按*键
#define MSGID_CENTER_DIVERT_CENTER                       (MSGID_START_CALL_ROOM+18)     //中心正转接至中心!
#define MSGID_CENTER_DIVERT_ROOM                         (MSGID_START_CALL_ROOM+19)     //中心正转接至房间...
#define MSGID_TALKING_TO_CENTER                          (MSGID_START_CALL_ROOM+20)     //与中心通话中,挂机按*键
#define MSGID_CENTER_OPEN_LOCK                           (MSGID_START_CALL_ROOM+21)     //中心已开锁,挂机按*键
#define MSGID_ROOM_RINGON                                (MSGID_START_CALL_ROOM+22)     //分机响铃中,挂机按*键
#define MSGID_ROOM_LEAVEWORD_NO_ACK                      (MSGID_START_CALL_ROOM+23)     //留言失败，无应答
#define MSGID_ROOM_UNINIT_LEAVEWORD_INTERFACE            (MSGID_START_CALL_ROOM+24)     //未初始化留言接口
#define MSGID_ROOM_DO_LEAVEWORD_NO_ACK                   (MSGID_START_CALL_ROOM+25)     //呼叫分机留言中,停止按*键
#define MSGID_ROOM_TALKING                               (MSGID_START_CALL_ROOM+26)     //与分机通话中,挂机按*键
#define MSGID_ROOM_OPEN_LOCK                             (MSGID_START_CALL_ROOM+27)     //分机已开锁,挂机按*键
#define MSGID_CMD_DATA_ERROR                             (MSGID_START_CALL_ROOM+28)     //命令数据错误

//2008-11-28新增  
#define MSGID_CMD_ROOM_HUNGUP                            (MSGID_START_CALL_ROOM+29)     //房间分机挂机
#define MSGID_CMD_ROOM_STOPTLEAVEWORDACK                 (MSGID_START_CALL_ROOM+30)     //房间分机停止留言
#define MSGID_CMD_CENTER_HUNGUP                          (MSGID_START_CALL_ROOM+31)     //中心挂机
#define MSGID_DEVICE_BUSY                                (MSGID_START_CALL_ROOM+32)     //本机繁忙，请稍候再试

//2008-12-09新增
#define MSGID_CMD_ROOM_CALL_TIMEOVER                     (MSGID_START_CALL_ROOM+33)    //呼叫分机通话时间到，可自由设置，一般为2分钟
#define MSGID_CMD_ROOM_NO_HOLDON                         (MSGID_START_CALL_ROOM+34)    //分机无人接听
#define MSGID_CMD_CENTER_CALL_TIMEOVER                   (MSGID_START_CALL_ROOM+35)    //呼叫中心通话时间到，同分机
#define MSGID_CMD_CENTER_NO_HOLDON                       (MSGID_START_CALL_ROOM+36)    //中心无人接听

#define MSGID_CMD_REST_ROOM_PASSWORD                   (MSGID_START_CALL_ROOM+37)    //修改房间密码,传出的是密码数据
#define MSGID_CMD_REST_DOOR_PASSWORD                   (MSGID_START_CALL_ROOM+38)    //修改门口机开门密码

typedef void (*pSHOW_FUN)      (int,const char* );
typedef void (*pCALLEND_FUN)   (unsigned long);
typedef void (*pHOLDON_FUN)    (unsigned long);
typedef void (*pRINGON_FUN)    (void);
typedef void (*pRINGOFF_FUN)   (void);
typedef int  (*pDOOROPEN_FUN)  (void);
typedef int  (*pCCDON_FUN)     (void);
typedef int  (*pCCDOFF_FUN)    (void);
typedef int  (*pLEAVEWORD_FUN) (void);
typedef void (*pCALLPREP_FUN)  (void);

typedef struct _callparam_{
        int sd;
        unsigned long ip;
        int ret;
        unsigned char name[16];
}CallParam_t;
typedef struct{
        int                     camera_flag;             /*设备是否有摄像头，0无，其它数值为有*/
        int                     callerstatus;            /*0:空闲 1：已经接通，可以自由挂断 2： 正在建立连接不可挂断*/
        int                     timecallstop;            /*通话时长（单位200ms）， 一般600x200ms  = 2min*/
        int                     timering;                /*响铃时长（单位200ms）， 一般 200x200ms = 40s*/ 
        int                     timemonit;               /*监视时间（单位200ms）,  一般 150x200ms = 30s*/
        unsigned int            MaxVideo:3;              /*最大视频路数，软件支持7路，当前硬件：4路*/
        unsigned int            callipnum:3;             /*当前通话路数，最大7路*/
        unsigned int            maxconnecttime:4;        /*最大连接超时时间 (单位 1s)*/
        unsigned int            device_leavewordflag:1;  /*设备是否支持留言，0不支持， 其它支持*/
        unsigned int            MaxDivertTimes:3;        /*最大转接次数*/
        unsigned int            DeviceType:3;            /*设备类型: 0主机， 1：分机， 2：中心或者管理机 3~7备用  2008-12-04新增*/	  	  
        CallParam_t    cp_calldata[4];          /*当前通话数据，可不用初始化  */
        char           localid[16];             /*本机id标示*/

        pCALLPREP_FUN  fun_prep;                /* 呼叫前准备，（一次通话结束时，也调用，以便准备下一次通话）,
                                                可在此函数中，初始化设备输入状态等信息*/
        pSHOW_FUN      fun_show;                /*呼叫提示信息显示回调函数*/
        pRINGON_FUN    fun_ringon;              /*呼叫响铃处理回调函数*/
        pRINGOFF_FUN   fun_ringoff;             /*呼叫关闭铃声回调函数*/
        pHOLDON_FUN    fun_holdon;              /*摘机处理*/
        pCALLEND_FUN   fun_callend;             /*呼叫结束处理*/
        pDOOROPEN_FUN  fun_opendoor;            /*开门处理*/
        pCCDON_FUN     fun_ccdon;               /*开启摄像头*/
        pCCDOFF_FUN    fun_ccdoff;              /*关闭摄像头*/
        pLEAVEWORD_FUN  fun_leaveword;			/*留言处理,返回0不留言，其他留言*/
        unsigned int usecenternegotiate;
}WRTI_CallerSetup_t;
#ifdef __cplusplus
extern "C" {
#endif

        /*所有IP设备主动呼叫（主叫）外部接口*/
        /*==================start==============================*/
        int wrti_CallerSetup(WRTI_CallerSetup_t callersetup);                              //初始化呼叫
        void wrti_CallerCleanup();                                                         //释放呼叫资源

        /*==========================呼叫多分机流程==========================


        开始呼叫多分机
        /      |      \  
        /       |       \
        /        |        \
        呼叫分机1   ...     呼叫分机n
        |        |         |
        |        |         |
        分机1响铃   ...     分机n响铃
        \                  /
        \  等待分机摘机  /
        |
        |
        有一分机摘机，挂断其余分机连接及响铃
        |
        |
        普通单机通话流程
        |
        结束通话

        */
        int wrti_devicecaller(CallParam_t cpCaller[], int num, unsigned short uCmdId);     //呼叫（非阻塞式）,只初始化CallParam_t 的 ip和name成员
        int wrti_devicehungup( unsigned short uCmdId);                                     //挂机，uCmdId对应设备的挂机指令
        const WRTI_CallerSetup_t * wrti_GetCallerParam();                                  //获取当前WRTI_CallerSetup_t结构参数
        int wrti_GetCallerStatus();                                                        //获取呼叫状态

        /*==========================呼叫多中心流程==========================

        开始呼叫多中心
        |             
        呼叫中心-----------------------|
        |                          |
        连接失败                       |
        |                          |
        呼叫备用中心1---------------------|
        .          ...             |
        呼叫备用中心n     ...       连接应答成功
        .          ...             |
        |                  普通单机通话流程
        |                          |
        结束通话---------------------|

        */
        int wrti_devicecallcenter(CallParam_t cpCaller[], int num, unsigned short uCmdId); //呼叫中心(阻塞式)，带多个ip
        /*==================end==============================*/

        int AV_Server_GetDestStatus();
        void AV_Server_SetDest1(const char *addr,int port);
        void AV_Server_SetDest2(const char *addr,int port);
        void AV_Server_SetDest3(const char *addr,int port);
        void AV_Server_SetDest4(const char *addr,int port);
        void AV_Server_DelDest1(const char *addr,int port);
        void AV_Server_DelDest2(const char *addr,int port);
        void AV_Server_DelDest3(const char *addr,int port);

        void AV_Server_DelDest4(const char *addr,int port);


#ifdef __cplusplus
}
#endif

#endif
