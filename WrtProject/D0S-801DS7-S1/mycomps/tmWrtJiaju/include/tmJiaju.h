

#ifndef _TM_JIAJU_H
#define _TM_JIAJU_H

//-----------------------------------------------------------------------------
// Standard include files:
//


#ifdef __cplusplus
extern "C"
{
#endif


#define  MAX_IR_MODE  		10

#define  HOMERS485ADDR    0X01
#define THREE_HOMEADDR    0x61

#define    DATA_MAST2CONTROL               0x01
#define    GET_STATUSCHANGE                0x02         /*from master send ,for get control status change */
#define    SEND_DATA                       0x03         /*for long data stream ,nosame control have max length*/
#define    RECIVE_DATA                     0x04         /*for long data stream */
#define    SCENE_SET                       0x05         /* set scene status : RFpackageHead +sceneindex + rometstatus*/
#define    SCENESTATUS_ENTRY               0x06         /* scene status entry RF DestAddress is 0xff+0xff*/
#define    IRSTUDY_SET                     0x07 

#define    RESET_SCENES                    0xD0    //清除情景



#define    SET_ADDR                        0xE0    //设备必须在设置状态
#define    SET_SYSTEMID                    0xE1    //设备必须在设置状态
#define    GET_VER                         0xE2
#define    RESET_MACHINE                   0xE3
#define    SET_MODE                        0xE4



#define	   C_OPEN	           0x01       
#define	   C_CLOSE           0x02          
#define	   C_PLAY  		       0x03
#define	   C_STOP            0x04
#define	   C_PAUSE           0x05
#define	   C_RECORD          0x06
#define    C_NEXT            0x07
#define    C_PREVIEW         0x08 
#define    C_MUTE            0x09
#define    C_KEY0            0x0A  
#define    C_KEY1            0x0B
#define    C_KEY2            0x0C
#define    C_KEY3            0x0D
#define    C_KEY4            0x0E
#define    C_KEY5            0x0F  
#define    C_KEY6            0x10
#define    C_KEY7            0x11
#define    C_KEY8            0x12
#define    C_KEY9            0x13
#define    C_ENTRY           0x14  
#define    C_UP              0x15
#define    C_DOWN            0x16
#define    C_LEFT            0x17
#define    C_RIGHT           0x18
#define    C_CHADD           0x19  
#define    C_CHSUB           0x1A
#define    C_VOLADD          0x1B
#define    C_VOLSUB          0x1C
#define    C_LOCK            0x1D
#define    C_UNLOCK          0x1E
#define    C_REVERSE         0x1F
#define    C_EMPTY           0x20


#define CBUS_OPEN     0x01
#define CBUS_OPEN1    0X02
#define CBUS_CLOSE    0x03
#define CBUS_DELAY_CLOSE 0x05
#define CBUS_D_ADD   0x13
#define CBUS_D_SUB    0x14

#define CBUS_C_OPEN   0x09
#define CBUS_C_CLOSE  0x10
#define CBUS_C_STOP   0x11


#define RESULT_WENDU   0x1
#define RESULT_SHIDU   0X2
#define RESULT_KONGQI  0X3



    typedef  unsigned char   uchar;
    typedef void (*JIAJU_POOL_RESULT_CALLBACK)(unsigned long msg[4]);

    typedef struct{
        uchar roomsort;    	//房间类别
        uchar roomname;   	//房间名
        uchar elecsort;     //电器类别
        uchar elecname;    	//电器名
    }T_ELEC_ID;                  //电器ID


    typedef struct{
        uchar RIUsort;    	//RIU类别，如果是CBus,该位表示是否有红外设备
        uchar RIUNo;     		//RIU	编号
        uchar RIU485Addr;
    }T_RIU_ID;                  //RIUID

    typedef struct{
        uchar RFsort;    		//RF类别
        uchar RFNo;     		//RF	编号
        uchar SubSw;      //
    }T_RF_ID;                  	//RFID


    typedef struct{
        uchar IrSort;				//红外类型
        uchar IrIndex;			//红外命令索引号
    }T_IR_MODE;									//红外模式结构



    typedef struct{
        int valid;
        T_ELEC_ID JiajuID;
        T_RIU_ID  RiuID;
        T_RF_ID   RfID;
        uchar     SysID[4];
        T_IR_MODE IrMode[MAX_IR_MODE];
    }T_JIAJU_TABLE;

    typedef struct{
        uchar majorVerNo;
        uchar minorVerNo;
    }T_SW_VERSION;





    int checksum();
    int Jiaju_SetStatus(T_JIAJU_TABLE  *pJiaju, uchar CmdMode);
    int Jiaju_GetStatus(T_JIAJU_TABLE *pJiaju);
    int Jiaju_SetSysID(T_JIAJU_TABLE *pJiaju, uchar *SysID) ;

    int Jiaju_SetScene(uchar SysID[4],int mode,int opt) ;   

    int Jiaju_SetupScene(T_JIAJU_TABLE *pJiaju,int index,int status);
    int Jiaju_ResetRIU(T_RIU_ID  RiuID);  
    int Jiaju_SetRIUID(T_RIU_ID  RiuID,unsigned char *SysID);
    int Jiaju_GetVersion(T_RIU_ID RiuID, T_SW_VERSION* pSw_ver);
    int Jiaju_BackupTable();                      // 备份配置表到RIU
    int Jiaju_RestoreTable();                      // 恢复备份表到分机
    int Jiaju_SetRIUAddr(T_RIU_ID  RiuID);   
    int Jiaju_SetRfAddr(T_JIAJU_TABLE *pJiaju, uchar RFNo);   
    int Jiaju_GetRfVersion(T_JIAJU_TABLE *pJiaju, T_SW_VERSION  *pRF_ver )    ;
    int Jiaju_ResetRF(T_JIAJU_TABLE *pJiaju)    ;

    int Jiaju_SetupIrStudy(T_JIAJU_TABLE *pJiaju,int IrMode);
    int Jiaju_ResetScene(T_JIAJU_TABLE *pJiaju,int index);
    
    int Jiaju_QueryStatus(unsigned char addr1,unsigned char addr2,unsigned char elecname);
    int Jiaju_SetCbusStatus2(T_JIAJU_TABLE *pJiaju,uchar mode,int param);
    
    void init_pool_jiaju(JIAJU_POOL_RESULT_CALLBACK  _callback);
    
    void send_pool_msg(unsigned long msg[4]);

//////////////////////////////////////////////////////////
//通过网络控制家居接口实现
   void init_net_jiaju(JIAJU_POOL_RESULT_CALLBACK _callback);
   void connect_net_jiaju_gateway();
   int control_scene(int scene);
   void uninit_net_jiaju();
   void set_net_jiaju_callback(JIAJU_POOL_RESULT_CALLBACK _callback);
   


#ifdef __cplusplus
}
#endif

#endif /* _TM_GPIO485_H */
