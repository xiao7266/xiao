#ifndef _DOOR_STRUCT_H_
#define _DOOR_STRUCT_H_
#pragma pack(1)
//#pragma TCS_align=1 
///#if _ROOM_SET_ 
///#define T_LOCALCONFIG T_LOCALCONFIG_ROOM 
///#define T_MYSYSINFO T_MYSYSINFO_ROOM   
///#endif
///
///#if (_DOOR_SET_||_GATE_SET_||_IP_BRIDGE_)
///#define T_LOCALCONFIG T_LOCALCONFIG_DOOR 
///#define T_MYSYSINFO T_MYSYSINFO_DOOR   
///#endif
typedef struct _liftdata_{
    unsigned char doorid[2];
    unsigned char roomid[2];
    int cardid;
    char ld_data[240];
}LiftData_t;
typedef struct _cmd485pack_{
    unsigned char  head;
    unsigned char  ln;
    unsigned char  dst;
    unsigned char  src;
    unsigned short ucmd;
#if 0 //cplusplus
    union _u485_{
        LiftData_t ld;
        char data[249];
    };
#else
    char data[249];
#endif
}Cmd485Pack_t;	
typedef struct roompassword{
	char szroomid[7];
	char pwd[9];
}RoomPassword_t;
typedef struct roompasswordinfo{
	int num;
	RoomPassword_t *password;
}RoomPasswordInfo_t;
///////////////////////////////////////////////////////////////////////////////////////////////
/*单条房间信息*/
typedef struct IpAddrTable_t
{
    unsigned long ip;
    char name[16];
}IpAddrTable;
/*存储房间信息块*/
typedef struct _IpAddrTableInfo_{
    unsigned int nRoomNum;
#ifdef __cplusplus
    union {
    	struct IpAddrTable_t *gIpAddrTable;
    	char *data;
    };
#else
	union {
    	struct IpAddrTable_t *gIpAddrTable;
    	char *data;
    };
#endif
}IpAddrTableInfo;

typedef struct _t_date{
    unsigned short year;
    unsigned char mon;
    unsigned char day;
}t_Date;
/*单条卡片信息*/
/*
卡号（6个字节10进制表示）
房号
时间有效标志
起用时间
终止时间
卡类型 
标志 （是否已经和中心同步）
*/
typedef struct{
	unsigned short m_id;
	unsigned long no;
}t_cardno;

struct _DoorCard_{
    //char CardId[10];
    char roomno[6];
#ifdef __cplusplus
    union {
    	unsigned char cardno[6];
    	t_cardno cardid;
    };
#else
	union {
    	unsigned char cardno[6];
    	t_cardno cardid;
    };
#endif
    t_Date startdate;
    t_Date enddate;
    unsigned char dateflag;    /*0：无限制，1：每天时间段 2，星期时间段*/
    unsigned char cardtype;   /*0:无效卡，1：业主卡，2：限时卡,最高位为1时表示该卡已删除卡；<卡类型做变更修改1~30为无限制卡，2为巡更卡，30以上为特殊卡需验证时间modify by hu 2010.07.28>*/
    unsigned char is2center;  //是否刷新标志 0:未同步， 1: 同步中心已经有这张卡
    unsigned char reuse;     //预留备用
};

typedef struct _DoorCard_ DoorCard;
/*存储主机卡片信息块*/
struct _doorcarinfo_{
    unsigned int nCardNum;
#ifdef __cplusplus
    union{
    	DoorCard *doorcard;
    	char *data;
    };
#else
	union{
    	DoorCard *doorcard;
    	char *data;
    };
#endif
};
typedef struct _doorcarinfo_ DoorCardInfo;


struct _wrti_device_time_
{
#ifdef __cplusplus
    _wrti_device_time_():year(2008),
		month(1),
		day(1),
		hour(0),
		sec(0),
		week(2){
		min = 0;
	}
#endif
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
    unsigned char week;
};
typedef struct _wrti_device_time_ WRTI_DeviceTime;
/*单条日志信息*/
struct _DoorLog_{/*Size : 128Byte*/
    char nNo;   /*1Byte*/
    WRTI_DeviceTime dt;/*7Byte*/
    char info[120];/*120Byte*/
};
typedef struct _DoorLog_ DoorLog;

/*存储主机日志块*/
struct _DoorSysInfo_{
    unsigned short nLogNum : 7;
    DoorLog SysLog[1023];
};
typedef struct _DoorSysInfo_ DoorSysInfo;

typedef struct _t_cardlog_{
    char cardno[6];
    WRTI_DeviceTime dt;   
}t_CardLog;

typedef struct _t_cardlog_info_{
    unsigned char nNum :7;
    t_CardLog cardlog[128];
}t_CardLogInfo;

typedef struct _t_entime_{
    unsigned char start_hour;
    unsigned char start_min;
    unsigned char end_hour;
    unsigned char end_min;
}t_EnTime;
typedef struct _t_timetable_{
    char name[10];
    unsigned char cardtype;/*0:无效卡，1：业主卡，2：限时卡*/
    unsigned char timeflag;	//时段有效标志 （0：不限时段； 1：按日限制； 2： 按星期限制）    
    t_EnTime entbl[7];
}t_TimeTable;
typedef struct {
    unsigned char nNum;
#ifdef __cplusplus
    union{
    	t_TimeTable *timetables;
    	char *data;
    };
#else
	union{
    	t_TimeTable *timetables;
    	char *data;
    };
#endif
}t_CardEnTable;


//*<add by hu 2010.08.06 特殊房号表
typedef struct ALIAS_TABLE_t{
	char id[16];
	char alias[16];
}ALIAS_TABLE;

typedef struct _Alias_Table_Info_{
    unsigned int nAliasNum;
#ifdef __cplusplus
    union {
    	struct ALIAS_TABLE_t *gAliasTable;
    	char *data;
    };
#else
	union {
    	struct ALIAS_TABLE_t *gAliasTable;
    	char *data;
    };
#endif
}Alias_Table_Info;
//add by hu 2010.08.06>*/

/*
*   以上是关于结构的定义
*/
#pragma pack()
#endif
