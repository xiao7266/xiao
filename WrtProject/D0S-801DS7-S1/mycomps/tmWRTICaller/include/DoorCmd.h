#ifndef DOORCMD_H
#define DOORCMD_H
#pragma pack(1)
/*
*   以下是关于结构的定义
*/
#ifdef PROTOCOL_VERSION2
#define ADD_LEN 30
#else
#define ADD_LEN 0
#endif

#define HEAD_LENGTH	(4/*协议头WRTI*/+2/*指令*/+4/*指令长度*/+ADD_LEN)   //指令头长度
#define RECVBUF 512                        //接受缓冲区 定义为514的原因： cpu对齐，增加2个字节
#define DATA_SIZE 			(RECVBUF-HEAD_LENGTH)     //数据大小
#define FILE_HEAD_LENGTH    (HEAD_LENGTH+4/*当前包序号*/+4/*总包数*/)   //文件传送时结构 头长度
#define ACK_LENGTH      FILE_HEAD_LENGTH        //ACK回应长度 10字节
#define PACKAGE_SIZE  (RECVBUF-FILE_HEAD_LENGTH)        //文件传送时,包的最大值
#define FILE_PACKNO           dbMsg.fb.nPackNo
#define FILE_FILESIZE         FILE_PACKNO
#define FILE_PACKTOTAL        dbMsg.fb.nPackTotal
#define FILE_DATA             dbMsg.fb.cFileData
#define CMD_DATA              dbMsg.Data
#define CMD_DATA_IP           dbMsg.ip
#define CMD_DATA_ROOMIPLIST   dbMsg.roomips
#if 0
union DataBuf
{
    struct FileBuf
    {
        int  nPackNo;    //当前包序号
        int  nPackTotal; //总包数
        char cFileData[DATA_SIZE-8];
    }fb;
    char Data[DATA_SIZE];
};//dbMsg;

struct _net_pack_
{
    char head[4];//WRTI    
    int len;
    unsigned short uCmd;
    union DataBuf dbMsg;
};

#else
struct _net_pack_
{
    char head[4];//WRTI    
    int len;
    unsigned short uCmd;
#ifdef PROTOCOL_VERSION2
    char srcid[15];
    char dstid[15];
#endif
    union DataBuf
    {
        struct FileBuf
        {
            int  nPackNo;    //当前包序号
            int  nPackTotal; //总包数
            char cFileData[DATA_SIZE-8];
        }fb;
        unsigned long ip;
        struct IpAddrTable_t roomips[4];
        struct {
            unsigned short year;
            unsigned char month;
            unsigned char day;
            unsigned char hour;
            unsigned char min;
            unsigned char sec;
            unsigned char week;
        }dt;
        char Data[DATA_SIZE];
    }dbMsg;
};
#endif

struct _pack_head_{
    char head[4];//WRTI    
    int len;
    unsigned short uCmd;
#ifdef PROTOCOL_VERSION2
    char srcid[15];
    char dstid[15];
#endif
};

typedef struct _net_pack_ NetPack;
struct _wrti_cmd_data_
{
    unsigned long ip;       //ip 地址
    NetPack np;
};
typedef struct _wrti_cmd_data_ WRTI_CmdData;
struct _wrti_mon_cmd_data_{
    unsigned long sd;   //socket id
    NetPack np;         //cmd data
};
typedef enum _av_runstatus_{
    DR_IDLE     =0,     /* 运行标识, =0: 空闲     */
    RM_MONT     =1,     /*           =1: 房间监视 */
    CT_LSN      =2,     /*           =2: 中心监听 */
    CT_MONT     =3,     /*           =3: 中心监视 */
    DR_CALLCENTER = 4,  /*           =4: 呼叫通话 */
    DR_CALLROOM  =5     
}AV_RunStatus;
struct _wrti_AV_arg_
{
    unsigned int sd;            //socket                 
    unsigned long ip;           //连接端ip               
    unsigned long time;         //开始通话或者视频时间   
    AV_RunStatus f_running;     /*运行标识, =0: 空闲     
                                *           =1: 房间监视 
                                *           =2: 中心监听 
                                *           =3: 中心监视 
                                *           >4: 呼叫通话 
                                */                       
    int writing_flag;           //数据块同步标识      
    char dstid[16];   
};
typedef enum _card_oper_t{
    co_REGISTRYBYROOM=0,
    co_DELETEBYROOM,
    co_REGISTRYBYRDCARD,
    co_DELETEBYINPUTCARD,
    co_DELETEBYRDCARD,
    co_OPENDOOR
}Card_Oper_t;
typedef enum _card2center_status_t{
    ccs_SENDED=0,
    ccs_NOSEND
}Card2CenterStatus_t;
typedef enum {
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
/*按键输入类型*/

/*输入不做任何处理*/
    INPUT_TYPE_NONE=0,
/*输入的是房间号码*/
    INPUT_TYPE_ROOMID,
/*输入的是本机ip地址*/
    INPUT_TYPE_IPADDR,
/*输入的是子网掩码*/
    INPUT_TYPE_NETMASK,
/*输入的是网关*/
    INPUT_TYPE_GATEWAY,
/*输入的是中心IP*/
    INPUT_TYPE_CENTERIP,
    INPUT_TYPE_CENTERMONITORIP,
    INPUT_TYPE_MANAGERIP,
    INPUT_TYPE_MANAGERMONITORIP,
    INPUT_TYPE_CENTERWARNING,	//9
/*输入的是位数*/
    INPUT_TYPE_INPUTNUM,
/*输入的是组团号码位数*/
    INPUT_TYPE_GROUPNUM,
/*输入的是栋号位数*/
    INPUT_TYPE_BUILDNUM,
/*输入的是单元号位数*/
    INPUT_TYPE_UNITNUM,
/*输入的是楼层位数*/
    INPUT_TYPE_ROOMLAYERNUM,

    INPUT_TYPE_ROOMNUM,	//15

/*输入开门密码*/
    INPUT_TYPE_OPENPWD,
    INPUT_TYPE_VIDEOBITRATE,	//add by hu 2010.10.21
    INPUT_TYPE_ACCESSBYTES	,	//18 add by hu 2010.08.30
    INPUT_TYPE_IPTEST	//add by hu 2010.08.31
}EnInputType;
typedef enum {
    SHOW_WELCOME_MSG=0,
    SHOW_INFORMATION=1,
    SHOW_FORMAT_MSG =2,
    SHOW_STATUS_MSG,
    SHOW_DEVICE_IP,
    SHOW_NET_MASK,
    SHOW_GATE_WAY,
    SHOW_CENTER_IP,
    SHOW_CENTERMONITOR_IP,
    SHOW_MANAGER_IP,
    SHOW_MANAGERMONITOR_IP,
    SHOW_CENTERWARNING_IP,
    SHOW_PASS_WORD,
    SHOW_LAYER_CALLFMT,
    SHOW_LAYER_DELAYTIME,
    SHOW_LAYER_TIME,
    SHOW_LAYER_PICTURE,
    SHOW_INPUT_CARD,
    SHOW_INPUT_OLD_PWD,
    SHOW_INPUT_NEW_PWD,
    SHOW_INPUT_CONFIRM_PWD,
    SHOW_DEVICE_TIME,
    SHOW_SET_AUTOREBOOT,	//add by hu 2010.05.06
    SHOW_SYSOP_IP,		//add by hu 2010.08.30
    SHOW_SYSOP_CARDBYTES,	//add by hu 2010.08.30
    SHOW_SYSOP_VIDEOBITRATE	//add by hu 2010.10.21
}EnShowType_t;
typedef enum{
	ddt_WALL_DEVICE=0,
    ddt_MAIN_DOOR_DEVICE=1,
    ddt_DOOR_DEVICE
}DoorDeviceType_t;
typedef enum{
    iot_AREA_ID=0,
    iot_BUILD_ID,
    iot_UNIT_ID,
    iot_DEVICE_ID
}IdOperType_t;
#pragma pack()
#endif
