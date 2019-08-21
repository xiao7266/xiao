/*
* Copyright (C) 2003 Koninklijke Philips Electronics N.V.,
* All Rights Reserved.
*
* This source code and any compilation or derivative thereof is the
* proprietary information of Koninklijke Philips Electronics N.V.
* and is confidential in nature.
* Under no circumstances is this software to be exposed to or placed
* under an Open Source License of any type without the expressed
* written permission of Koninklijke Philips Electronics N.V.
*
*----------------------------------------------------------*/
/*
*      \file           tmTouchScreenTest.c
*
*      This file is designed to demonstrate an example to use up to 2
*		software gpio uart units
*
*/
/*-----------------------------------------------------------
*
*      %version:       ds08#qwang2 %
*      instance:       DS_4
*      %date_created:  Fri Feb 17 18:10:33 2006 %
*
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "wrt_log.h"
#include "zenmalloc.h"

//-----------------------------------------------------------------------------
// Project include files:
//-----------------------------------------------------------------------------
//
#include "tmJiaju.h"


#define JIAJU_DEBUG 1
#undef JIAJU_DEBUG


#ifdef __cplusplus
extern "C"{
#endif

 unsigned char get_select_control_index(T_JIAJU_TABLE* pJiaju,unsigned char cmdmode);

#ifdef __cplusplus
}
#endif



typedef struct _wrtcmdmap{
        unsigned char old_cmd;
        unsigned char cbus_cmd;
}WRT_CMD_MAP;

#if 0

static unsigned long g_pooljiaju_tid = 0xffffffff;
static unsigned long g_pooljiaju_qid = 0xffffffff;
static unsigned long g_jiaju_lock  = 0xffffffff;
#endif

static JIAJU_POOL_RESULT_CALLBACK g_jiaju_result_callback = NULL;




/*
typedef struct{
uchar mode; //模式
uchar len;     //长度
uchar cmd;   //命令
uchar cmd1;   //命令
}MODE;
*/


WRT_CMD_MAP cmd_map[33] ={
        {0,0},
        {C_OPEN,CBUS_OPEN},
        {C_CLOSE  ,CBUS_CLOSE},
        {C_PLAY  ,0},
        {C_STOP   ,CBUS_C_STOP},
        {C_PAUSE  ,0},
        {C_RECORD ,0},
        {C_NEXT   ,0},
        {C_PREVIEW,0},
        {C_MUTE   ,0},
        {C_KEY0   ,0},
        {C_KEY1   ,0},
        {C_KEY2   ,0},
        {C_KEY3   ,0},
        {C_KEY4   ,0},
        {C_KEY5   ,0},
        {C_KEY6   ,0},
        {C_KEY7   ,0},
        {C_KEY8   ,0},
        {C_KEY9   ,0},
        {C_ENTRY  ,0},
        {C_UP     ,CBUS_D_ADD},
        {C_DOWN   ,CBUS_D_SUB},
        {C_LEFT   ,0},
        {C_RIGHT  ,0},
        {C_CHADD  ,0},
        {C_CHSUB  ,0},
        {C_VOLADD ,0},
        {C_VOLSUB ,0},
        {C_LOCK   ,0},
        {C_UNLOCK ,0},
        {C_REVERSE,0},
        {C_EMPTY,0}
};

/*
MODE switch_mode[13] =	{
{0, 0   , 0   ,0},
{1, 0x01, C_OPEN,0},   //关
{2, 0x01, C_CLOSE,0},   //开
{3, 0x01, C_UNLOCK,0},   //开锁
{4, 0x01, C_LOCK,0},   //锁定
{5, 0x01, 0,0},      //
{6, 0x01, 0,0},      //
{7, 0x01, 0,0},      //
{8, 0x01, 0,0},      //
{9, 0x01, 0,0},
{10, 0x01,0,0}
};

MODE adjustable_mode1[17] =	{
{0, 0   , 0   ,0},
{1, 0x01, C_CLOSE,0},   //关
{2, 0x01, C_OPEN,0},   //开
{3, 0x01, C_UNLOCK,0},   //解锁
{4, 0x01, C_LOCK,0},   //锁定
{5, 0x01, C_UP,0},   //强
{6, 0x01, C_DOWN,0},   //弱
{7, 0x01, 0x20,0},   //亮度值 0-9
{8, 0x01, 0x21,0},
{9, 0x01,0x22,0},
{10, 0x01,0x23,0},
{11, 0x01,0x24,0},
{12, 0x01,0x25,0},
{13, 0x01,0x26,0},
{14, 0x01,0x27,0},
{15, 0x01,0x28,0},
{16, 0x01,0x29,0}

};
*/

/*

MODE switch_mode[13] =	{
{0, 0   , 0   ,0},
{1, 0x01, 0x01,0},   //关
{2, 0x01, 0x02,0},   //开
{3, 0x01, 0x04,0},   //开锁
{4, 0x01, 0x08,0},   //锁定
{5, 0x01, 0,0},      //
{6, 0x01, 0,0},      //
{7, 0x01, 0,0},      //
{8, 0x01, 0,0},      //
{9, 0x01, 0,0},
{10, 0x01,0,0}
};

MODE adjustable_mode1[17] =	{
{0, 0   , 0   ,0},
{1, 0x01, 0x10,0},   //关
{2, 0x01, 0x20,0},   //开
{3, 0x01, 0x40,0},   //解锁
{4, 0x01, 0x80,0},   //锁定
{5, 0x01, 0x30,0},   //强
{6, 0x01, 0x00,0},   //弱
{7, 0x01, 0x20,0},   //亮度值 0-9
{8, 0x01, 0x21,0},
{9, 0x01,0x22,0},
{10, 0x01,0x23,0},
{11, 0x01,0x24,0},
{12, 0x01,0x25,0},
{13, 0x01,0x26,0},
{14, 0x01,0x27,0},
{15, 0x01,0x28,0},
{16, 0x01,0x29,0}

};

*/













typedef struct{
        uchar returnmode;
        uchar status;
}RETURN_STATUS;


RETURN_STATUS rt_status[4]={
        {0, 0x05 },   //关 -解锁
        {1, 0x09 },   //关 -锁定
        {2, 0x06 },   //开 -解锁
        {3, 0x0a }   //开 -锁定
};


static void  Lock()
{
#if 0	
	if(g_jiaju_lock != 0xffffffff)
		mu_lock(g_jiaju_lock,MU_WAIT,0);
#endif		
}
static void UnLock()
{
#if 0	
	if(g_jiaju_lock != 0xffffffff)
		mu_unlock(g_jiaju_lock);
#endif		
}

static void clear_recv_buffer()
{
#if 0	
	int ret = 0;
	uchar RxBuffer[30];
	int Bufflen = 30;
	while(1)
	{
		
		ret = tmGpio485Recv_ElecCrl(RxBuffer, &Bufflen,20);
		if(ret != 0)
			break;
	}
	tm_wkafter(500);//每一次send都要延时800毫秒。
#endif	
}


//-------------------------------------------
//
//    int checksum(uchar *data,int len)
//
//    校验和计算
//
//    uchar *data :  校验的数组
//    int len     :  数组的长度
//
//    return      :  计算的校验和
//
//-------------------------------------------
int checksum(uchar *data,int len)
{
        int i;
        int chksum=0;

        for(i=0;i<len;i++)
        {
                chksum = chksum + data[i];
        }
        return (chksum&0xff);

}

/*
int Jiaju_SetCbusStatus2(T_JIAJU_TABLE *pJiaju, uchar CmdMode)
{
        int len = 11;
        int i = 0;
        int retval;
        int Bufflen = 30;
        uchar RxBuffer[30];
        uchar sendbuffer[30];
        uchar cbusaddr[30];
        memset(cbusaddr,0,30);
        memset(sendbuffer,0,30);
      //  sprintf(cbusaddr,"%02x",pJiaju->RfID.RFNo);
        cbusaddr[0] = pJiaju->RfID.RFNo; //设备地址
        cbusaddr[1] = pJiaju->RfID.SubSw; //设备子地址
        
//停
//0A 7F 80 00 05 02 01 11 7F 80 B2

//关
//0A 7F 80 00 02 02 01 10 7F 80 AE

//开
//0A 7F 80 00 01 02 01 09 00 06 AE

        sendbuffer[0] = 0x0a;
        sendbuffer[1] = 0x7F;
        sendbuffer[2] = 0x80;

        sendbuffer[3]  = cbusaddr[0];
        sendbuffer[4]  = cbusaddr[1]; 
        sendbuffer[5] = 0x02;
        sendbuffer[6] = 0x03;
        sendbuffer[7] = pJiaju->RiuID.RIUNo;//通道编号

        sendbuffer[8] = cmd_map[CmdMode].cbus_cmd;
        sendbuffer[9] = 0x00; //控制参数
        sendbuffer[10] = 0x00;


        sendbuffer[11] = 0x0;
       
        for(i = 0; i < sendbuffer[0] - 1 ;i++)
        {
       	   sendbuffer[11] += sendbuffer[i];
        }
        sendbuffer[10] = 0 - sendbuffer[10];

        tmGpio485Send((char*)sendbuffer, len);

        for(i =0; i<len;i++)
                printf("0x%02x",sendbuffer[i]);
                
        retval = tmGpio485Recv_ElecCrl(RxBuffer, &Bufflen,1500);
  	for(i =0; i<Bufflen;i++)
                printf("0x%02x",RxBuffer[i]);       
    
        if(retval == 0)
        {
        	if(RxBuffer[0] == 0xf5 && RxBuffer[6] == 0x07)
                {
                	if( RxBuffer[11]== 0x02 )
                		return 0;
                	else
                		return -1;
                }else
                	return -1;
        }else
        	return -1;


        return 0;
}

*/
static int glight = 0x0;

int Jiaju_GetCbusStatus(T_JIAJU_TABLE  *pJiaju)
{
#if 0
        int len = 16;
        int i = 0;
        int retval;
        uchar sendbuffer[30];
        uchar RxBuffer[30];
        uchar cbusaddr[2];
        int Bufflen = 30;
        memset(cbusaddr,0,2);
        memset(sendbuffer,0,30);
        memset(RxBuffer,0,30);

        cbusaddr[0] = pJiaju->RfID.RFNo;
        cbusaddr[1] = pJiaju->RfID.SubSw;
        sendbuffer[0] = 0xFA;//帧头
        sendbuffer[1] = 0x10;// 16
        sendbuffer[2] = THREE_HOMEADDR; //目标地址
        sendbuffer[3] = HOMERS485ADDR;//源地址
        sendbuffer[4] = 0x07; //命令字
        sendbuffer[5] = 0x01;
        //参数
        sendbuffer[6] = 0x08;//参数长度
        sendbuffer[7] = 0x07; //源器件地址
        sendbuffer[8] = 0x07;

        sendbuffer[9]  = cbusaddr[0]; //目标器件地址
        sendbuffer[10]  = cbusaddr[1]; 

        sendbuffer[11] = 0x03;
        sendbuffer[12] = 0x11;
        sendbuffer[13] = pJiaju->RiuID.RIUsort; //通路编号

        sendbuffer[14] = checksum(sendbuffer+6,8);//参数校验位
        sendbuffer[15] = checksum(sendbuffer,16);//整个数据包的校验位
        
        Lock();
        clear_recv_buffer();

        tmGpio485Send((char*)sendbuffer, len);


        WRT_DEBUG("Send:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",sendbuffer[0],
                sendbuffer[1],sendbuffer[2],sendbuffer[3],sendbuffer[4],sendbuffer[5],sendbuffer[6],
                sendbuffer[7],sendbuffer[8],sendbuffer[9],sendbuffer[10],sendbuffer[11],sendbuffer[12],
                sendbuffer[13],sendbuffer[14],sendbuffer[15],sendbuffer[16],sendbuffer[17],sendbuffer[18]);

        retval = tmGpio485Recv_ElecCrl(RxBuffer, &Bufflen,1500);
        
         WRT_DEBUG("recv:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",RxBuffer[0],
                  RxBuffer[1],RxBuffer[2],RxBuffer[3],RxBuffer[4],RxBuffer[5],RxBuffer[6],
                  RxBuffer[7],RxBuffer[8],RxBuffer[9],RxBuffer[10],RxBuffer[11],RxBuffer[12],
                  RxBuffer[13],RxBuffer[14],RxBuffer[15],RxBuffer[16],RxBuffer[17],RxBuffer[18]);        

        if(retval == 0)
        {
                if(RxBuffer[0] == 0xf5 && RxBuffer[4] == 0x07 && RxBuffer[11] == 0x03 && RxBuffer[12] == 0x12 && sendbuffer[9] == RxBuffer[7] && sendbuffer[10] == RxBuffer[8])
                {
                         
                        switch(RxBuffer[14])
                        {
                        case 0x09:
                        case 0x01:
                        	UnLock();
                                return 1;
                        case 0x10:
                        case 0x03:
                        	UnLock();
                                return 0;
                        case 0x12: //表示关停
                        case 0x11: //表示开停
                        	UnLock();
                        	return 1;
                        default:
                        	UnLock();
                                return 0xfd; //-1,直接返回状态不明
                        }
                       
                                 
                        
                }
        }
        UnLock();
        return 0xfd ;//-1,直接返回状态不明
#endif        
return 0;           
}


int Jiaju_SetCbusStatus(T_JIAJU_TABLE *pJiaju, uchar CmdMode)
{
#if 0	
        int len = 19;
        int i = 0;
        int retval;
        uchar sendbuffer[30];
        uchar RxBuffer[30];
        uchar cbusaddr[2];
        int Bufflen = 30;
        memset(cbusaddr,0,2);
        memset(sendbuffer,0,30);
        memset(RxBuffer,0,30);

        cbusaddr[0] = pJiaju->RfID.RFNo;
        cbusaddr[1] = pJiaju->RfID.SubSw;
        sendbuffer[0] = 0xFA;//帧头
        sendbuffer[1] = 0x13;//长度,19字节
        sendbuffer[2] = THREE_HOMEADDR; //目标地址
        sendbuffer[3] = HOMERS485ADDR;//源地址
        sendbuffer[4] = 0x07; //命令字
        sendbuffer[5] = 0x01;
        //参数
        sendbuffer[6] = 0x0b;//参数长度
        sendbuffer[7] = 0x07; //源器件地址
        sendbuffer[8] = 0x07;
   
        sendbuffer[9]  = cbusaddr[0]; //目标器件地址
        sendbuffer[10]  = cbusaddr[1]; 

        sendbuffer[11] = 0x02;
        sendbuffer[12] = 0x03;

        sendbuffer[13] = pJiaju->RiuID.RIUsort; //通路编号
        if(pJiaju->RiuID.RIUNo == 0x01 ){
                int irindex = get_select_control_index(pJiaju,CmdMode);
                sendbuffer[14] = 0x20;
                sendbuffer[15] = 0;
                sendbuffer[16] = pJiaju->IrMode[irindex].IrIndex;
        }else{
                sendbuffer[14] = cmd_map[CmdMode].cbus_cmd; //子命令字
                sendbuffer[15] = 0x0; //子命令参数
                sendbuffer[16] = 0x0;
        }
        sendbuffer[17] = checksum(sendbuffer+6,11);//参数校验位
        sendbuffer[18] = checksum(sendbuffer,19);//整个数据包的校验位
        
        Lock();
        clear_recv_buffer();
        
        tmGpio485Send((char*)sendbuffer, len);

      
        WRT_DEBUG("Send:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",sendbuffer[0],
                  sendbuffer[1],sendbuffer[2],sendbuffer[3],sendbuffer[4],sendbuffer[5],sendbuffer[6],
                  sendbuffer[7],sendbuffer[8],sendbuffer[9],sendbuffer[10],sendbuffer[11],sendbuffer[12],
                  sendbuffer[13],sendbuffer[14],sendbuffer[15],sendbuffer[16],sendbuffer[17],sendbuffer[18]);
                
        retval = tmGpio485Recv_ElecCrl(RxBuffer, &Bufflen,1500);
         WRT_DEBUG("recv:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",RxBuffer[0],
                  RxBuffer[1],RxBuffer[2],RxBuffer[3],RxBuffer[4],RxBuffer[5],RxBuffer[6],
                  RxBuffer[7],RxBuffer[8],RxBuffer[9],RxBuffer[10],RxBuffer[11],RxBuffer[12],
                  RxBuffer[13],RxBuffer[14],RxBuffer[15],RxBuffer[16],RxBuffer[17],RxBuffer[18]);
  
        if(retval == 0)
        {
        	if(RxBuffer[0] == 0xf5 && RxBuffer[4] == 0x07 && RxBuffer[11] == 0x02 && RxBuffer[12] == 0x04 && sendbuffer[9] == RxBuffer[7] && sendbuffer[10] == RxBuffer[8])
                {
                	UnLock();
                	return 0;
                }else{
                	UnLock();
                	return -1;
                }
        }
        UnLock();        
        return -1;
#endif        
return 0;           
}

//------------------------------------------------------------
//
//    int Jiaju_SetStatus(T_JIAJU_TABLE *pJiaju, uchar CmdMode)
//
//    设置家居状态---开/关等
//
//    T_JIAJU_TABLE  jiaju :  选择的家居的结构，见结构体
//    uchar CmdMode        :  命令模式
//
//    return          :   0 -- 成功   -1 --- 失败
//
//    Send: 0xfa,len ,RIU485add,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0x01,0x00,len1 ,cmd,chksum,]chksum
//    Recv:	0xf5,len ,src485addr,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x01 ,0/1,chksum,]chksum
//------------------------------------------------------------

int Jiaju_SetStatus(T_JIAJU_TABLE *pJiaju, uchar CmdMode)
{
#if 0	
        uchar i;
        uchar sendbuffer[30];

        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int len=0,len1=0;
        int retval;
        int optcnt;
        int loop = 1;

        if(pJiaju->RfID.RFsort == 0xB) //CBUS
        {
                ZENFREE(pRxBuffer);
                pRxBuffer = NULL;
                //if(pJiaju->JiajuID.elecsort ==  0x00) //灯光
                       return  Jiaju_SetCbusStatus(pJiaju,CmdMode);
               // else if(pJiaju->JiajuID.elecsort ==  0x01) //窗帘
               // {
               //       return  Jiaju_SetCbusStatus2(pJiaju,CmdMode);
              //  }
                //return 0;
        }

        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;
        sendbuffer[9] = pJiaju->RiuID.RIUsort;
        sendbuffer[10] =	pJiaju->RiuID.RIUNo;
        sendbuffer[11] =  pJiaju->SysID[0] ;
        sendbuffer[12] =  pJiaju->SysID[1] ;
        sendbuffer[13] =  pJiaju->SysID[2] ;
        sendbuffer[14] =  pJiaju->SysID[3] ;

        sendbuffer[15] = DATA_MAST2CONTROL;
        sendbuffer[16] = 0x00;

        if(pJiaju->RfID.RFsort == 0x7)
                loop = 1;


        switch(pJiaju->RfID.RFsort)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:

                sendbuffer[17]=0x02;
                sendbuffer[18] = pJiaju->RfID.SubSw;
                sendbuffer[19] = cmd_map[CmdMode].old_cmd;
                len1 = sendbuffer[6] = 13 + 2;
                break;
        case 7:

                sendbuffer[17] =0x02;
                // memcpy(sendbuffer+18,&pJiaju->IrMode[CmdMode],2);
                sendbuffer[18] = pJiaju->IrMode[0].IrSort;
                sendbuffer[19] = cmd_map[CmdMode].old_cmd;
                len1 = sendbuffer[6] = 13 + 2;
                break;

        case  8:
                sendbuffer[17] =0x02;
                sendbuffer[18] = pJiaju->RfID.SubSw;
                sendbuffer[19] = cmd_map[CmdMode].old_cmd;
                len1 = sendbuffer[6] = 13 + 2;
                break;

        case  9:
                sendbuffer[17] = 0x02;
                sendbuffer[18] = pJiaju->RfID.SubSw;
                sendbuffer[19] = cmd_map[CmdMode].old_cmd;
                len1 = sendbuffer[6] = 13 + sendbuffer[17];
                break;

        }
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);


       
        WRT_DEBUG("loop = %d  %x",loop,pJiaju->RfID.RFsort);
        Lock();
        clear_recv_buffer();
        for(i=0;i<loop;i++)
        {
#ifdef JIAJU_DEBUG
                printf("send: ");
                for(i =0 ;i<sendbuffer[1];i++)
                        printf("%x ",sendbuffer[i]);
                printf("\n");
#endif
                WRT_DEBUG("tmGpio485Send = %d ",i);
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;

        }


        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {

                        if((uchar)pRxBuffer[18] == 0x00 )

                                retval = 0;

                        else
                                retval =-1;

                }
                else
                        retval=-1;
        }
        if(retval == -3)
                retval = 0;


        ZENFREE(pRxBuffer);

	UnLock();
        return retval;
#endif        
return 0;           

}








//------------------------------------------------------------
//
//    Jiaju_GetStatus(T_JIAJU_TABLE  *pJiaju)
//
//    得到家居状态---开/关等
//
//    T_JIAJU_TABLE jiaju :  选择的家居的结构，见结构体
//    uchar *pStatus      ： 返回状态数据   status [len  ok/err s1  s2 s3 ... ]
//    return          :   0 -- 关/解锁  1 --关/锁定  2 -- 开/解锁  3 --开/锁定   -1 --失败
//
//
//    send: 0xfa,len,RIU485add ,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0x02,0x00,0x00  ,chksum,]chksum
//    Recv: 0xf5,len,src485addr ,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,status,chksum,]chksum
//------------------------------------------------------------
int Jiaju_GetStatus(T_JIAJU_TABLE  *pJiaju)
{
#if 0	
        uchar pStatus[4];
        uchar i;
        uchar sendbuffer[30];
        int len,len1;
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval=0xfd;

        memset(pRxBuffer, 0, 300);

        if(pJiaju->RfID.RFsort == 0x0b)
        {
                ZENFREE(pRxBuffer);
                pRxBuffer = NULL;
                return Jiaju_GetCbusStatus(pJiaju);
        }

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0x00;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0x00;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;
        sendbuffer[9] = pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;

        sendbuffer[11] = pJiaju->SysID[0]; ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = GET_STATUSCHANGE;
        sendbuffer[16] = 0x00;

#if 0
        sendbuffer[17] = 0x0;


        len1 = sendbuffer[6] = 13 + 0;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);
#else
        sendbuffer[17] = 0x1;
        sendbuffer[18] = pJiaju->RfID.SubSw;

        len1 = sendbuffer[6] = 14 + 0;
        len = sendbuffer[1] = sendbuffer[6] + 7;
        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);
#endif
#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
        clear_recv_buffer();

        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {

                        if((uchar)pRxBuffer[18] == 0x00 )
                        {

                                uchar tmpcmd =2;
                                if(pJiaju->RfID.SubSw == 0x00)
                                        tmpcmd = pRxBuffer[19];
                                else if(pJiaju->RfID.SubSw  == 0x01){
                                        tmpcmd = pRxBuffer[20];
                                }else if(pJiaju->RfID.SubSw ==0x02){
                                        tmpcmd =  pRxBuffer[21];
                                }
                                for(i =1; i<sizeof(cmd_map);i++){
                                        if(cmd_map[i].old_cmd == tmpcmd){
                                                if(i == 2)
                                                        retval = 0;
                                                else
                                                        retval = i;
                                                break;
                                        }
                                }
                        }

                }
                else
                        retval= 0xfd;
        }else
        	retval = 0xfd;


        ZENFREE(pRxBuffer);
        UnLock();
        return retval;
#endif        
return 0;           
}


//------------------------------------------------------------
//
//    Jiaju_SetRIUID(T_RIU_ID  RiuID,unsigned char *SysID)
//
//    设置RIU的系统ID
//
//    T_RIU_ID  RiuID:  选择的T_RIU_ID的结构数据，见结构体
//    unsigned char *SysID:  系统ID
//    return             :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add,src485addr,0x07,0x05,id3,id2,id1,id0,chksum
//    Recv: 0xf5,len,src485addr,RIU485add,0x07,0x05,0/1,chksum
//------------------------------------------------------------
int Jiaju_SetRIUID(T_RIU_ID  RiuID,unsigned char *SysID)
{
#if 0	
        uchar i;
        uchar sendbuffer[30];
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;


        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0x0b;
        sendbuffer[2] = RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x05;
        sendbuffer[6] = SysID[0];
        sendbuffer[7] = SysID[1];
        sendbuffer[8] = SysID[2];
        sendbuffer[9] = SysID[3];
        sendbuffer[10] = checksum(sendbuffer,10);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	 clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {

                        if((uchar)pRxBuffer[18] == 0x00 )
                                retval = 0;
                        else
                                retval =-1;

                }
                else
                        retval=-1;
        }
        /*
        if((Bufflen>pRxBuffer[1]-1)&&(!pRxBuffer[6]))
        retval  =  0;
        else
        retval  =  -1;
        */
        ZENFREE(pRxBuffer);
        UnLock();
        return retval;
#endif        
return 0;           

}




//------------------------------------------------------------
//
//    int Jiaju_SetSysID(T_JIAJU_TABLE *pJiaju, unsigned char *SysID)
//
//    设置RF的系统ID
//
//    T_RIU_ID RiuID :  选择的RIU的结构数据，见结构体
//		uchar *SysID   :  系统ID号
//    return         :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add, src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,sysid[4],0x06,0x00,sysid[4],chksum],chksum
//    Recv: 0xf5,len,src485addr ,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,sysid[4],0xa1,0x00,0x01,0/1,chksum],chksum
//------------------------------------------------------------
int Jiaju_SetSysID(T_JIAJU_TABLE *pJiaju, unsigned char *SysID)
{
#if 0	
        uchar sendbuffer[30];
        int len,len1;
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;
        int loop = 2;
        uchar i;
        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] = pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;


        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = SET_SYSTEMID;
        sendbuffer[16] = 0x00;
        sendbuffer[17] = 0x04;

        sendbuffer[18] = SysID[0] ;
        sendbuffer[19] = SysID[1] ;
        sendbuffer[20] = SysID[2] ;
        sendbuffer[21] = SysID[3] ;

        len1 = sendbuffer[6] = 13 + sendbuffer[17];
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

        if(pJiaju->RfID.RFsort == 0x7) //红外设备，只发送1次
                loop = 1;

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif

	Lock();
 	clear_recv_buffer();
        for(i=0;i<loop;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {
                        if((uchar)pRxBuffer[18] == 0x00 )
                                retval = 0;
                        else
                                retval = 1;

                }
                else
                        retval= -1;
        }
        ZENFREE(pRxBuffer);
	UnLock();
        return retval;
#endif        
return 0;        

}



//------------------------------------------------------------
//
//    int Jiaju_ResetRIU(T_RIU_ID  RiuID)
//
//    重启RIU
//
//    T_RIU_ID  RiuID:  T_RIU_ID结构数据，见结构体
//
//    return        :   0--设置成功   -1--设置失败
//
//    send: 0xfa,0x07,RIU485add,src485addr,0x07,0x06,chksum
//    Recv: 0xf5,0x07,src485addr,RIU485add,0x07,0x06,chksum
//------------------------------------------------------------

int Jiaju_ResetRIU(T_RIU_ID  RiuID)
{
#if 0	
        uchar sendbuffer[30];
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;
        uchar i;
        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0x07;
        sendbuffer[2] = RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x06;
        sendbuffer[6] = checksum(sendbuffer,6);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	 clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }

        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5)){
                        if((uchar)pRxBuffer[18] == 0x00 )
                                retval = 0;
                        else
                                retval = 1;
                }else
                        retval=-1;
        }

        ZENFREE(pRxBuffer);
	UnLock();
        return retval;
#endif
	return 0;        
}





//------------------------------------------------------------
//
//    Jiaju_SetScene(uchar SysID[4],int mode,int opt)
//
//    进入情景模式
//    uchar SysID[4]:  系统ID号
//    int mode      :  情景模式号
//    int opt       :   操作模式：  0  进入情景  1清除情景
//    return  :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add,src485addr,0x07,0x01,[len,rf0,rf1,riu0,riu1,id[4],0x06,0x00,0x01,mode,chksum],chksum
//    Recv:	0xf5,len,src485addr,RIU485add,0x07,0x02,[len,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x01 ,0/1,chksum],chksum
//------------------------------------------------------------

int Jiaju_SetScene(uchar SysID[4],int mode,int opt)
{
#if 0	
        uchar i;
        char *pRxBuffer = (char *)ZENMALLOC(300);

        UInt32 Bufflen;
        uchar sendbuffer[30];
        int len,len1;
        int retval;
        if(pRxBuffer == NULL){
                WRT_DEBUG("zen malloc failed 1");
                return -1;
        }

        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = 0x80;   //broadcast address
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0x00;
        sendbuffer[7] = 0x00;    //JiaJuTable[index].RfID.RFsort  情景模式的riu及rf 的ID暂取0
        sendbuffer[8] = 0x00;   //JiaJuTable[index].RfID.RFNo;

        sendbuffer[9] = 0x03;   //JiaJuTable[index].RiuID.RIUsort//2010-6-3 17:23:16
        sendbuffer[10] =0x01;   //JiaJuTable[index].RiuID.RIUNo;//2010-6-3 17:23:11

        sendbuffer[11] = SysID[0] ;
        sendbuffer[12] = SysID[1] ;
        sendbuffer[13] = SysID[2] ;
        sendbuffer[14] = SysID[3] ;


        if(opt==0)
                sendbuffer[15] = SCENESTATUS_ENTRY;
        else
                sendbuffer[15] = RESET_SCENES;

        sendbuffer[16] = 0x00;

        sendbuffer[17] = 0x01;
        sendbuffer[18] = mode;

        len1 = sendbuffer[6] = 13 + 1;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif

	Lock();
         clear_recv_buffer();
        //设置情景采用广播模式，只发送一次,直接返回成功。2008.10.15,by ljw


        WRT_DEBUG("Send:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",sendbuffer[0],
                  sendbuffer[1],sendbuffer[2],sendbuffer[3],sendbuffer[4],sendbuffer[5],sendbuffer[6],
                  sendbuffer[7],sendbuffer[8],sendbuffer[9],sendbuffer[10],sendbuffer[11],sendbuffer[12],
                  sendbuffer[13],sendbuffer[14],sendbuffer[15],sendbuffer[16],sendbuffer[17],sendbuffer[18]);
                  
        tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
        retval = 0;
        /*
        retval =  tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500);   
        
        WRT_DEBUG("recv:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",pRxBuffer[0],
                  pRxBuffer[1],pRxBuffer[2],pRxBuffer[3],pRxBuffer[4],pRxBuffer[5],pRxBuffer[6],
                  pRxBuffer[7],pRxBuffer[8],pRxBuffer[9],pRxBuffer[10],pRxBuffer[11],pRxBuffer[12],
                  pRxBuffer[13],pRxBuffer[14],pRxBuffer[15],pRxBuffer[16],pRxBuffer[17],pRxBuffer[18]);   
                       
       if(retval == 0){
       		retval = -1;
        	if((Bufflen > pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
        	{
        		if((uchar)pRxBuffer[18] == 0x00)
        			retval= 0;
        		else
       		 		retval= -1;
        	}
        	if(pRxBuffer[0] == 0xf5 && pRxBuffer[4] == 0x07 && pRxBuffer[11] == 0x02 && pRxBuffer[12] == 0x04){
        		retval = 0;
        	}
	}else{
		retval = 0;
	}
	*/
        ZENFREE(pRxBuffer);
        UnLock();
        return retval;
#endif
	return 0;
}





//------------------------------------------------------------
//
//    Jiaju_SetupScene(T_JIAJU_TABLE *pJiaju,int index,int devno,int status)
//
//    设置家居的情景模式
//
//    T_JIAJU_TABLE *pJiaju:  选择的T_JIAJU_TABLE的结构，见结构体
//    int mode        :   情景模式号
//    uchar status    :   设置情景的状态: 开关状态  、可调开关状态 、空调模式
//    return          :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0x05,0x00,0x02,mode,status,chksum],chksum
//    Recv:	0xf5,len,src485addr,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x01, 0/1,chksum],chksum
//------------------------------------------------------------
int Jiaju_SetupScene(T_JIAJU_TABLE *pJiaju,int index,int status)
{

#if 0
        uchar i;
        uchar sendbuffer[30];
        int len,len1;
        int loop = 2;
        char *pRxBuffer = (char *)ZENMALLOC(300);

        UInt32 Bufflen;
        int retval;

        memset(pRxBuffer, 0, 300);
        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] =  pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;

        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = SCENE_SET;
        sendbuffer[16] = 0x00;

        sendbuffer[17] =0x04;
        sendbuffer[18] = index;
        sendbuffer[19] = pJiaju->RfID.SubSw;
        sendbuffer[20] = cmd_map[status].old_cmd;
        sendbuffer[21] = 0x0;



#if 0
        /*
        switch(pJiaju->RfID.RFsort)
        {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:

        sendbuffer[17] =0x02;
        sendbuffer[18] =index;

        if(pJiaju->RfID.SubSw == 1)
        switch_mode[status].cmd=((switch_mode[status].cmd)<<4);

        memcpy(sendbuffer+19,&switch_mode[status].cmd,1);

        if(pJiaju->RfID.SubSw == 1)
        switch_mode[status].cmd=((switch_mode[status].cmd)>>4);


        if(pJiaju->RfID.SubSw == 0)
        sendbuffer[19] =0xf0|sendbuffer[19];
        if(pJiaju->RfID.SubSw == 1)
        sendbuffer[19] =0x0f|sendbuffer[19];



        break;

        case  7:

        sendbuffer[17] =0x03;
        sendbuffer[18] =index;

        sendbuffer[19] = pJiaju->RfID.SubSw;
        sendbuffer[20] = status;
        //  memcpy(sendbuffer+19,&pJiaju->IrMode[status],2);

        break;


        case  8:

        sendbuffer[17] =0x02;
        sendbuffer[18] =index;
        sendbuffer[19] =status-1;

        break;

        case  9:



        sendbuffer[17] =0x02+1;
        sendbuffer[18] =index;



        memcpy(sendbuffer+19+pJiaju->RfID.SubSw,&adjustable_mode1[status].cmd,1);

        if(pJiaju->RfID.SubSw==0)
        sendbuffer[20]=0xff;

        if(pJiaju->RfID.SubSw==1)
        sendbuffer[19]=0xff;



        break;






        }
        */
#endif

        len1 = sendbuffer[6] = 13 + sendbuffer[17] ;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	 clear_recv_buffer();

        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {
                        if((uchar)pRxBuffer[18] == 0x00)

                                retval= 0;
                        else
                                retval= -1;


                }
                else
                        retval= -1;
        }
        if(retval == -3)
                retval = 0;


        ZENFREE(pRxBuffer);

	UnLock();
        return retval;

#endif 
	return 0;

}



//------------------------------------------------------------
//
//    Jiaju_SetupIrStudy(T_JIAJU_TABLE *pJiaju,int IrMode)
//
//    进入学习模式
//
//    T_JIAJU_TABLE *pJiaju:  选择的T_JIAJU_TABLE的结构，见结构体
//    int IrMode        :   设备号
//    return          :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0x05,0x00,0x02,index,devno,chksum],chksum
//    Recv:	0xf5,len,src485addr,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x01, 0/1,chksum],chksum
//------------------------------------------------------------
int Jiaju_SetupIrStudy(T_JIAJU_TABLE *pJiaju,int IrMode)
{

#if 0
        uchar i;
        uchar sendbuffer[30];
        int len,len1;
        int loop = 2;
        char *pRxBuffer = (char *)ZENMALLOC(300);

        UInt32 Bufflen;
        int retval;

        memset(pRxBuffer, 0, 300);
        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] =  pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;

        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = IRSTUDY_SET;
        sendbuffer[16] = 0x00;



        sendbuffer[17] = 0x02;
        sendbuffer[18] = pJiaju->RfID.SubSw;
        sendbuffer[19] = 0;

        len1 = sendbuffer[6] = 13 + 2;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

        if(pJiaju->RfID.RFsort == 0x7)
                loop = 1;

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	 clear_recv_buffer();
        for(i=0;i<loop;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {
                        if((uchar)pRxBuffer[18] == 0x00)

                                retval= 0;
                        else
                                retval= -1;


                }
                else
                        retval= -1;
        }


        ZENFREE(pRxBuffer);
	UnLock();

        return retval;
#endif
	return 0;

}






//------------------------------------------------------------
//
//    Jiaju_RestScene(T_JIAJU_TABLE *pJiaju,int index,int devno,int status)
//
//    清除情景模式
//
//    T_JIAJU_TABLE *pJiaju:  选择的T_JIAJU_TABLE的结构，见结构体
//    int index        :   情景模式号
//    return          :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0x05,0x00,0x01,mode,,chksum],chksum
//    Recv:	0xf5,len,src485addr,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x01, 0/1,chksum],chksum
//------------------------------------------------------------
int Jiaju_ResetScene(T_JIAJU_TABLE *pJiaju,int index)
{
#if 0
        uchar i;
        uchar sendbuffer[30];
        int len,len1;
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;
        int loop = 2;

        memset(pRxBuffer, 0, 300);
        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] =  pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;

        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = RESET_SCENES;
        sendbuffer[16] = 0x00;




        sendbuffer[17] = 0x01;
        sendbuffer[18] = index;



        len1 = sendbuffer[6] = 13 + sendbuffer[17] ;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

        if(pJiaju->RfID.RFsort == 0x7)
                loop = 1;

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	 clear_recv_buffer();
        for(i=0;i<loop;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {
                        if((uchar)pRxBuffer[18] == 0x00)

                                retval= 0;
                        else
                                retval= -1;


                }
                else
                        retval= -1;
        }


        ZENFREE(pRxBuffer);

	UnLock();
        return retval;
#endif
	return 0;

}








//------------------------------------------------------------
//
//    int Jiaju_GetVersion(T_RIU_ID RiuID, T_SW_VERSION* pSw_ver)
//
//    得到RIU的版本号
//
//    T_RIU_ID RiuID        :  选择的T_RIU_ID的结构数据，见结构体
//    T_SW_VERSION* pSw_ver :  RIU版本号结构
//
//    return          :   0 -- 获取版本成功   1 ---获取版本失败
//    send : 0xfa 0x07 riu485id home485id 0x07 0x03 chksum
//    recv : 0xf5 0x09 home485id riu485id 0x07 0x03 majorVerNo minorVerNo chksum
//-------------------------------------------------------------
int Jiaju_GetVersion(T_RIU_ID RiuID, T_SW_VERSION* pSw_ver)
{
#if 0	
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        uchar sendbuffer[30];
        int retval;
        uchar i;

        memset(pRxBuffer, 0, 300);
        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0x07;
        sendbuffer[2] = RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x03;
        sendbuffer[6] = checksum(sendbuffer,6);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
        clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {
                        pSw_ver->majorVerNo = pRxBuffer[6];
                        pSw_ver->minorVerNo = pRxBuffer[7];


                        retval = 0 ;
                }
                else
                        retval = -1 ;
        }


        ZENFREE(pRxBuffer);
	UnLock();
        return retval;
#endif
	return 0;        
}


//------------------------------------------------------------
//
//    Jiaju_SetRIUAddr(T_RIU_ID  RiuID)
//
//    设置RIU的地址
//
//    T_RIU_ID  RiuID:  选择的T_RIU_ID的结构数据，见结构体
//    return             :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add,src485addr,0x07,0x04,RIUsort,RIUNo,chksum
//    Recv: 0xf5,len,src485addr,RIU485add,0x07,0x04,0/1,chksum
//------------------------------------------------------------
int Jiaju_SetRIUAddr(T_RIU_ID  RiuID)
{
#if 0	
        uchar i;
        uchar sendbuffer[30];
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;


        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0x09;
        sendbuffer[2] = RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x05;
        sendbuffer[6] = RiuID.RIUsort;
        sendbuffer[7] = RiuID.RIUNo;
        sendbuffer[8] = checksum(sendbuffer,8);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
        clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {
                        if((uchar)pRxBuffer[18] == 0x00)
                                retval= 0;
                        else
                                retval= -1;
                }
                else
                        retval= -1;
        }
        ZENFREE(pRxBuffer);
        UnLock();
        return retval;
#endif
return 0;        

}



//------------------------------------------------------------
//
//    int Jiaju_SetRfAddr(T_JIAJU_TABLE *pJiaju，uchar RFNo)
//
//    设置RF的地址
//
//    T_JIAJU_TABLE *pJiaju :  选择的T_JIAJU_TABLE的结构数据，见结构体
//    return         :   0 -- 设置成功   -1 ---设置失败
//
//    send: 0xfa,len,RIU485add, src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,sysid[4],0xe0,0x00,0x01,RFNO,chksum],chksum		cmd [4,sysid[3],sysid[2],sysid[1],sysid[0] ]
//    Recv: 0xf5,len,src485addr ,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,sysid[4],0xa0,0x00,0x01,0/1,chksum],chksum
//------------------------------------------------------------
int Jiaju_SetRfAddr(T_JIAJU_TABLE *pJiaju, uchar RFNo)
{
#if 0	
        uchar sendbuffer[30];
        int len,len1;
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;
        uchar i;
        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] = pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;


        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = SET_ADDR;
        sendbuffer[16] = 0x00;
        sendbuffer[17] = 0x01;

        sendbuffer[18] = RFNo ;


        len1 = sendbuffer[6] = 13 + 1;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {

                        if((uchar)pRxBuffer[18] == 0x00)
                        {

                                retval= 0;
                        }
                        else
                                retval= -1;

                }
                else
                        retval= -1;
        }

        ZENFREE(pRxBuffer);
	UnLock();
        return retval;
#endif

return 0;        

}





//------------------------------------------------------------
//
//    int Jiaju_GetRfVersion(T_JIAJU_TABLE *pJiaju, T_SW_VERSION  *pRF_ver )
//
//    得到RF版本
//
//    T_JIAJU_TABLE jiaju :  选择的家居的结构，见结构体
//    T_SW_VERSION  *pRF_ver： 获取版本结构
//    return          :   0 --成功 ，-1 --- 失败
//
//
//    send: 0xfa,len,RIU485add ,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0xe2,0x00,0x00  ,chksum,]chksum
//    Recv: 0xf5,len,src485addr ,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x03，0，ver1 , ver0 ，chksum,]chksum
//------------------------------------------------------------

int Jiaju_GetRfVersion(T_JIAJU_TABLE *pJiaju, T_SW_VERSION  *pRF_ver )
{
#if 0
        uchar sendbuffer[30];
        int len,len1;
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;
        uchar i;
        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] = pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;


        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = GET_VER;
        sendbuffer[16] = 0x00;
        sendbuffer[17] = 0x00;


        len1 = sendbuffer[6] = 13 + 0;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
        clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {

                        if((uchar)pRxBuffer[18] == 0x00)
                        {
                                pRF_ver->majorVerNo = pRxBuffer[19];
                                pRF_ver->minorVerNo = pRxBuffer[20];

                                retval= 0;
                        }
                        else
                                retval= -1;
                }
                else
                        retval= -1;
        }
        ZENFREE(pRxBuffer);
	UnLock();	
        return retval;
#endif
	return 0;        

}




//------------------------------------------------------------
//
//    int Jiaju_GetRfVersion(T_JIAJU_TABLE *pJiaju, T_SW_VERSION  *pRF_ver )
//
//    重启RF设备
//
//    T_JIAJU_TABLE jiaju :  选择的家居的结构，见结构体
//    return          :   0 --成功 ，-1 --- 失败
//
//
//    send: 0xfa,len,RIU485add ,src485addr,0x07,0x01,[len1,rf0,rf1,riu0,riu1,id[4],0xe3,0x00,0x00,chksum,]chksum
//    Recv: 0xf5,len,src485addr ,RIU485add,0x07,0x02,[len1,riu0,riu1,rf0,rf1,id[4],0xa0,0x00,0x01,0x00，chksum,]chksum
//------------------------------------------------------------

int Jiaju_ResetRF(T_JIAJU_TABLE *pJiaju)
{
#if 0	
        uchar sendbuffer[30];
        int len,len1;
        char *pRxBuffer = (char *)ZENMALLOC(300);
        UInt32 Bufflen;
        int retval;
        uchar i;
        memset(pRxBuffer, 0, 300);

        sendbuffer[0] = 0xfa;
        sendbuffer[1] = 0;
        sendbuffer[2] = pJiaju->RiuID.RIU485Addr;
        sendbuffer[3] = HOMERS485ADDR;
        sendbuffer[4] = 0x07;
        sendbuffer[5] = 0x01;
        sendbuffer[6] = 0;
        sendbuffer[7] = pJiaju->RfID.RFsort;
        sendbuffer[8] = pJiaju->RfID.RFNo;

        sendbuffer[9] = pJiaju->RiuID.RIUsort;
        sendbuffer[10] = pJiaju->RiuID.RIUNo;


        sendbuffer[11] = pJiaju->SysID[0] ;
        sendbuffer[12] = pJiaju->SysID[1] ;
        sendbuffer[13] = pJiaju->SysID[2] ;
        sendbuffer[14] = pJiaju->SysID[3] ;

        sendbuffer[15] = RESET_MACHINE;
        sendbuffer[16] = 0x00;
        sendbuffer[17] = 0x00;


        len1 = sendbuffer[6] = 13 + 0;
        len = sendbuffer[1] = sendbuffer[6] + 7;

        sendbuffer[len-2] = checksum(&sendbuffer[6],len1-1);
        sendbuffer[len-1] = checksum(sendbuffer,len-1);

#ifdef JIAJU_DEBUG
        printf("send: ");
        for(i =0 ;i<sendbuffer[1];i++)
                printf("%x ",sendbuffer[i]);
        printf("\n");
#endif
	Lock();
	 clear_recv_buffer();
        for(i=0;i<2;i++)
        {
                tmGpio485Send((char*)sendbuffer, sendbuffer[1]);
                Bufflen = 300;
                if( 0 == (retval = tmGpio485Recv_ElecCrl(pRxBuffer, &Bufflen,1500)))
                        break;
        }
        if(retval == 0){
                if((Bufflen>pRxBuffer[1]-1)&&((uchar)pRxBuffer[0] == 0xf5))
                {

                        if((uchar)pRxBuffer[18] == 0x00)
                                retval= 0;
                        else
                                retval= -1;
                }
                else
                        retval= -1;
        }
        ZENFREE(pRxBuffer);
	UnLock();
	  return retval;
#endif	
	return 0;
      

}

int Jiaju_SetCbusStatus2(T_JIAJU_TABLE *pJiaju,uchar mode,int param)
{
#if 0	
        int len = 19;
        int i = 0;
        int retval;
        uchar sendbuffer[30];
        uchar RxBuffer[30];
        uchar cbusaddr[2];
        int Bufflen = 30;
        memset(cbusaddr,0,2);
        memset(sendbuffer,0,30);
        memset(RxBuffer,0,30);

        cbusaddr[0] = pJiaju->RfID.RFNo;
        cbusaddr[1] = pJiaju->RfID.SubSw;
        sendbuffer[0] = 0xFA;//帧头
        sendbuffer[1] = 0x13;//长度,19字节
        sendbuffer[2] = THREE_HOMEADDR; //目标地址
        sendbuffer[3] = HOMERS485ADDR;//源地址
        sendbuffer[4] = 0x07; //命令字
        sendbuffer[5] = 0x01;
        //参数
        sendbuffer[6] = 0x0b;//参数长度
        sendbuffer[7] = 0x07; //源器件地址
        sendbuffer[8] = 0x07;
   
        sendbuffer[9]  = cbusaddr[0]; //目标器件地址
        sendbuffer[10]  = cbusaddr[1]; 

        sendbuffer[11] = 0x02;
        sendbuffer[12] = 0x03;

        sendbuffer[13] = pJiaju->RiuID.RIUsort; //通路编号

        sendbuffer[14] = mode; //子命令字
        if(mode == 0x05){
        	param = param * 60;
        	if(param == 0)
        		param = 1;
        }
        sendbuffer[15] = (param >> 8) & 0x000000ff;	
        sendbuffer[16] = (param & 0x000000ff); //子命令参数
       
        
        sendbuffer[17] = checksum(sendbuffer+6,11);//参数校验位
        sendbuffer[18] = checksum(sendbuffer,19);//整个数据包的校验位
        
        
        Lock();
        clear_recv_buffer();
        
        tmGpio485Send((char*)sendbuffer, len);

      
        WRT_DEBUG("Send:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",sendbuffer[0],
                  sendbuffer[1],sendbuffer[2],sendbuffer[3],sendbuffer[4],sendbuffer[5],sendbuffer[6],
                  sendbuffer[7],sendbuffer[8],sendbuffer[9],sendbuffer[10],sendbuffer[11],sendbuffer[12],
                  sendbuffer[13],sendbuffer[14],sendbuffer[15],sendbuffer[16],sendbuffer[17],sendbuffer[18]);
                
        retval = tmGpio485Recv_ElecCrl(RxBuffer, &Bufflen,1500);
         WRT_DEBUG("recv:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",RxBuffer[0],
                  RxBuffer[1],RxBuffer[2],RxBuffer[3],RxBuffer[4],RxBuffer[5],RxBuffer[6],
                  RxBuffer[7],RxBuffer[8],RxBuffer[9],RxBuffer[10],RxBuffer[11],RxBuffer[12],
                  RxBuffer[13],RxBuffer[14],RxBuffer[15],RxBuffer[16],RxBuffer[17],RxBuffer[18]);
  
        if(retval == 0)
        {
        	if(RxBuffer[0] == 0xf5 && RxBuffer[4] == 0x07 && RxBuffer[11] == 0x02 && RxBuffer[12] == 0x04 && sendbuffer[9] == RxBuffer[7] && sendbuffer[10] == RxBuffer[8])
                {
                	UnLock();
                	return 0;
                }else{
                	UnLock();
                	return -1;
                }
        }
        UnLock();  
#endif              
        return -1;
}


int Jiaju_QueryStatus(unsigned char addr1,unsigned char addr2,unsigned char elecname)
{
#if 0	
        int len = 16;
        int i = 0;
        int retval;
        unsigned long msg[4];
        uchar sendbuffer[30];
        uchar RxBuffer[30];
        uchar cbusaddr[2];
        int Bufflen = 30;
        memset(cbusaddr,0,2);
        memset(sendbuffer,0,30);
        memset(RxBuffer,0,30);
        
        cbusaddr[0] = addr1;
        cbusaddr[1] = addr2;
        sendbuffer[0] = 0xFA;//帧头
        sendbuffer[1] = 0xF;//长度,19字节
        sendbuffer[2] = THREE_HOMEADDR; //目标地址
        sendbuffer[3] = HOMERS485ADDR;//源地址
        sendbuffer[4] = 0x07; //命令字
        sendbuffer[5] = 0x01;
        //参数
        sendbuffer[6] = 0x07;//参数长度
        sendbuffer[7] = 0x07; //源器件地址
        sendbuffer[8] = 0x07;

        sendbuffer[9]  = cbusaddr[0]; //目标器件地址
        sendbuffer[10]  = cbusaddr[1]; 

	//如果不是感应器类型返回
        if(elecname == 0x0){ //温度
        	sendbuffer[11] = 0x03;
        	sendbuffer[12] = 0x01;
        }else if(elecname == 0x01) //湿度
        {
        	sendbuffer[11] = 0x03;
        	sendbuffer[12] = 0x03;
        }
	else if(elecname == 0x07) //风力
        {
        	sendbuffer[11] = 0x03;
        	sendbuffer[12] = 0x05;
        }  
	else if(elecname == 0x08) //雨量
        {
        	sendbuffer[11] = 0x03;
        	sendbuffer[12] = 0x07;
        } 
	else if(elecname == 0x05) //空气
        {
        	sendbuffer[11] = 0x03;
        	sendbuffer[12] = 0x09;
        }  
	else if(elecname == 0x06) //空气
        {
        	sendbuffer[11] = 0x03;
        	sendbuffer[12] = 0x09;
        }else{
        	WRT_DEBUG("暂不存在该电器的查询指令(%d)",elecname);
        	return -1; 
        }                                 
        		
        //sendbuffer[13] = pJiaju->RiuID.RIUNo; //通路编号

        sendbuffer[13] = checksum(sendbuffer+6,7);//参数校验位
        sendbuffer[14] = checksum(sendbuffer,15);//整个数据包的校验位
        
        Lock();
        clear_recv_buffer();

        tmGpio485Send((char*)sendbuffer, len);


        WRT_DEBUG("Send:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",sendbuffer[0],
                sendbuffer[1],sendbuffer[2],sendbuffer[3],sendbuffer[4],sendbuffer[5],sendbuffer[6],
                sendbuffer[7],sendbuffer[8],sendbuffer[9],sendbuffer[10],sendbuffer[11],sendbuffer[12],
                sendbuffer[13],sendbuffer[14],sendbuffer[15],sendbuffer[16],sendbuffer[17],sendbuffer[18]);

        retval = tmGpio485Recv_ElecCrl(RxBuffer, &Bufflen,1500);
        
         WRT_DEBUG("recv:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",RxBuffer[0],
                  RxBuffer[1],RxBuffer[2],RxBuffer[3],RxBuffer[4],RxBuffer[5],RxBuffer[6],
                  RxBuffer[7],RxBuffer[8],RxBuffer[9],RxBuffer[10],RxBuffer[11],RxBuffer[12],
                  RxBuffer[13],RxBuffer[14],RxBuffer[15],RxBuffer[16],RxBuffer[17],RxBuffer[18]);   
        if(retval == 0) 
        {
                if(RxBuffer[0] == 0xf5 && RxBuffer[4] == 0x07  && sendbuffer[9] == RxBuffer[7] && sendbuffer[10] == RxBuffer[8])
                {
     			if(RxBuffer[11] == 0x03 && RxBuffer[12] == 0x02)//温度
     			{
     				if(RxBuffer[13] > 0x80)
     					RxBuffer[13] -= 0x80;
     				else{
     					RxBuffer[13] = ~RxBuffer[13];
     				}
     				UnLock();
     				if(g_jiaju_result_callback){
     					msg[0] = RESULT_WENDU;
     					msg[1] = RxBuffer[13];
     					g_jiaju_result_callback(msg);
     				}
     				
     				return RxBuffer[13];
     			}
			if(RxBuffer[11] == 0x03 && RxBuffer[12] == 0x04)//湿度
     			{
     				UnLock();
     				if(g_jiaju_result_callback){
     					msg[0] = RESULT_SHIDU;
     					msg[1] = RxBuffer[13];
     					g_jiaju_result_callback(msg);
     				}
     				
     				return RxBuffer[13];
     			}
     			if(RxBuffer[11] == 0x03 && RxBuffer[12] == 0x06)//风力
     			{
     				UnLock();
     				return RxBuffer[13];
     			}
     			if(RxBuffer[11] == 0x03 && RxBuffer[12] == 0x08)//雨量
     			{
     				UnLock();
     				return RxBuffer[13];
     			}
     			if(RxBuffer[11] == 0x03 && RxBuffer[12] == 0x10)//空气
     			{

     				char res[4]={0};
     				int result = -1;;
     				res[1]= RxBuffer[13];
     				res[0] = RxBuffer[14];
     				result = *(int*)res;
     				UnLock();
     				if(g_jiaju_result_callback){
     					msg[0] = RESULT_KONGQI;
     					msg[1] = result;
     					g_jiaju_result_callback(msg);  				
     				}
     				
     				return  result;
     			}     			
     			     			
     			
                }
        }
        UnLock();
#endif        
        return -1;	
}
static void  pool_jiaju()
{
	/*
	unsigned long msg[4];
	unsigned char addr1,addr2,elecname;
	
	while(1){
		msg[0] = msg[1] = msg[2] = msg[3] = 0;
		if(0 == q_receive(g_pooljiaju_qid,Q_WAIT,0,msg)){
			addr1 = (msg[0] & 0xff);
			addr2 = (msg[1]& 0xff);
			elecname = (msg[2] & 0xff);
			Jiaju_QueryStatus(addr1,addr2,elecname);
			
		}
			
	}
	t_delete(0);
	*/
}
    
void init_pool_jiaju(JIAJU_POOL_RESULT_CALLBACK  _callback)
{
	/*
	if(g_jiaju_result_callback == NULL)
		g_jiaju_result_callback = _callback;
		
	if(0 != mu_create("jjmc",MU_LOCAL,0,&g_jiaju_lock)){
		WRT_MESSAGE("创建家居锁失败");
		return;
	}
		
	if(0 != q_create("jjqc"	,0,Q_LOCAL | Q_FIFO |Q_NOLIMIT,&g_pooljiaju_qid)){
		WRT_MESSAGE("创建轮询家居感应器队列失败");
		mu_delete(g_jiaju_lock);
		g_jiaju_lock = 0xffffffff;
		return;
	}
	if(0 != t_create("jjtc", 124, 0x1000, 0x1000, 0, &g_pooljiaju_tid)){
		WRT_MESSAGE("创建轮询家居感应器任务失败");
		q_delete(g_pooljiaju_qid);
		g_pooljiaju_qid = 0xffffffff;
		mu_delete(g_jiaju_lock);
		g_jiaju_lock = 0xffffffff;
		return;
	}
        if(0 != t_start(g_pooljiaju_tid, 0, (void (*)())pool_jiaju, NULL)){
        	WRT_MESSAGE("启动轮询家居感应器任务失败");
        	q_delete(g_pooljiaju_qid);
		g_pooljiaju_qid = 0xffffffff;
		t_delete(g_pooljiaju_tid);
		g_pooljiaju_tid = 0xffffffff;
		mu_delete(g_jiaju_lock);
		g_jiaju_lock = 0xffffffff;
		return;
        } 
        */       
}
void send_pool_msg(unsigned long msg[4])
{
	/*
	if(g_pooljiaju_qid != 0xffffffff){
    		q_send(g_pooljiaju_qid,msg);
    	}
    	*/
}

