#ifndef DOORCMD_H
#define DOORCMD_H
#pragma pack(1)
/*
*   �����ǹ��ڽṹ�Ķ���
*/
#ifdef PROTOCOL_VERSION2
#define ADD_LEN 30
#else
#define ADD_LEN 0
#endif

#define HEAD_LENGTH	(4/*Э��ͷWRTI*/+2/*ָ��*/+4/*ָ���*/+ADD_LEN)   //ָ��ͷ����
#define RECVBUF 512                        //���ܻ����� ����Ϊ514��ԭ�� cpu���룬����2���ֽ�
#define DATA_SIZE 			(RECVBUF-HEAD_LENGTH)     //���ݴ�С
#define FILE_HEAD_LENGTH    (HEAD_LENGTH+4/*��ǰ�����*/+4/*�ܰ���*/)   //�ļ�����ʱ�ṹ ͷ����
#define ACK_LENGTH      FILE_HEAD_LENGTH        //ACK��Ӧ���� 10�ֽ�
#define PACKAGE_SIZE  (RECVBUF-FILE_HEAD_LENGTH)        //�ļ�����ʱ,�������ֵ
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
        int  nPackNo;    //��ǰ�����
        int  nPackTotal; //�ܰ���
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
            int  nPackNo;    //��ǰ�����
            int  nPackTotal; //�ܰ���
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
    unsigned long ip;       //ip ��ַ
    NetPack np;
};
typedef struct _wrti_cmd_data_ WRTI_CmdData;
struct _wrti_mon_cmd_data_{
    unsigned long sd;   //socket id
    NetPack np;         //cmd data
};
typedef enum _av_runstatus_{
    DR_IDLE     =0,     /* ���б�ʶ, =0: ����     */
    RM_MONT     =1,     /*           =1: ������� */
    CT_LSN      =2,     /*           =2: ���ļ��� */
    CT_MONT     =3,     /*           =3: ���ļ��� */
    DR_CALLCENTER = 4,  /*           =4: ����ͨ�� */
    DR_CALLROOM  =5     
}AV_RunStatus;
struct _wrti_AV_arg_
{
    unsigned int sd;            //socket                 
    unsigned long ip;           //���Ӷ�ip               
    unsigned long time;         //��ʼͨ��������Ƶʱ��   
    AV_RunStatus f_running;     /*���б�ʶ, =0: ����     
                                *           =1: ������� 
                                *           =2: ���ļ��� 
                                *           =3: ���ļ��� 
                                *           >4: ����ͨ�� 
                                */                       
    int writing_flag;           //���ݿ�ͬ����ʶ      
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
/*������������*/

/*���벻���κδ���*/
    INPUT_TYPE_NONE=0,
/*������Ƿ������*/
    INPUT_TYPE_ROOMID,
/*������Ǳ���ip��ַ*/
    INPUT_TYPE_IPADDR,
/*���������������*/
    INPUT_TYPE_NETMASK,
/*�����������*/
    INPUT_TYPE_GATEWAY,
/*�����������IP*/
    INPUT_TYPE_CENTERIP,
    INPUT_TYPE_CENTERMONITORIP,
    INPUT_TYPE_MANAGERIP,
    INPUT_TYPE_MANAGERMONITORIP,
    INPUT_TYPE_CENTERWARNING,	//9
/*�������λ��*/
    INPUT_TYPE_INPUTNUM,
/*����������ź���λ��*/
    INPUT_TYPE_GROUPNUM,
/*������Ƕ���λ��*/
    INPUT_TYPE_BUILDNUM,
/*������ǵ�Ԫ��λ��*/
    INPUT_TYPE_UNITNUM,
/*�������¥��λ��*/
    INPUT_TYPE_ROOMLAYERNUM,

    INPUT_TYPE_ROOMNUM,	//15

/*���뿪������*/
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
