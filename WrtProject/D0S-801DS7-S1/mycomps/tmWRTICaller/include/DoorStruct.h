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
/*����������Ϣ*/
typedef struct IpAddrTable_t
{
    unsigned long ip;
    char name[16];
}IpAddrTable;
/*�洢������Ϣ��*/
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
/*������Ƭ��Ϣ*/
/*
���ţ�6���ֽ�10���Ʊ�ʾ��
����
ʱ����Ч��־
����ʱ��
��ֹʱ��
������ 
��־ ���Ƿ��Ѿ�������ͬ����
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
    unsigned char dateflag;    /*0�������ƣ�1��ÿ��ʱ��� 2������ʱ���*/
    unsigned char cardtype;   /*0:��Ч����1��ҵ������2����ʱ��,���λΪ1ʱ��ʾ�ÿ���ɾ������<������������޸�1~30Ϊ�����ƿ���2ΪѲ������30����Ϊ���⿨����֤ʱ��modify by hu 2010.07.28>*/
    unsigned char is2center;  //�Ƿ�ˢ�±�־ 0:δͬ���� 1: ͬ�������Ѿ������ſ�
    unsigned char reuse;     //Ԥ������
};

typedef struct _DoorCard_ DoorCard;
/*�洢������Ƭ��Ϣ��*/
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
/*������־��Ϣ*/
struct _DoorLog_{/*Size : 128Byte*/
    char nNo;   /*1Byte*/
    WRTI_DeviceTime dt;/*7Byte*/
    char info[120];/*120Byte*/
};
typedef struct _DoorLog_ DoorLog;

/*�洢������־��*/
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
    unsigned char cardtype;/*0:��Ч����1��ҵ������2����ʱ��*/
    unsigned char timeflag;	//ʱ����Ч��־ ��0������ʱ�Σ� 1���������ƣ� 2�� ���������ƣ�    
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


//*<add by hu 2010.08.06 ���ⷿ�ű�
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
*   �����ǹ��ڽṹ�Ķ���
*/
#pragma pack()
#endif
