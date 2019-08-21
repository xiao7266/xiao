//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
// FILE NAME:    RoomGpioUtil.h
//
// DESCRIPTION:  分机接口驱动
//
// 
//
// NOTES:
//

#ifndef _ROOM_GPIO_UTIL_H
#define _ROOM_GPIO_UTIL_H


enum
{
	CTL_IO_SETLED_RED = 0,
    CTL_IO_SETLED_GREEN,
    CTL_IO_SETLED_YELLO,
    CTL_IO_SETLED_OFF,
    CTL_IO_SETBACKLIGHT_POWER_ON,
    CTL_IO_SETBACKLIGHT_POWER_OFF,
    CTL_IO_SETLCD_RESET,
    CTL_IO_SET_MIC_ON,
    CTL_IO_SET_MIC_OFF,
    CTL_IO_SET_AUDIO_PA_ON,
    CTL_IO_SET_AUDIO_PA_OFF,
    CTL_IO_SET_USB1_ON,
    CTL_IO_SET_USB1_OFF,
    CTL_IO_SET_PCAP_TS_RST,
    CTL_IO_SET_UART_485_SEND,
    CTL_IO_SET_UART_485_RECV,
    CTL_IO_SET_SUB_SPK_ON,
    CTL_IO_SET_SUB_SPK_OFF,
    CTL_IO_SET_SUB_MIC_ON,
    CTL_IO_SET_SUB_MIC_OFF,
    CTL_IO_GET_DOORBELL,
    CTL_IO_GET_EMERGCY,
    CTL_IO_GET_EMERGCY_OUTSIDE,
    CTL_IO_GET_CALLLIFT,
    CTL_IO_GET_SUB_HOLD,
    CTL_IO_GET_SUB_LOCK,
    CTL_IO_GET_SUB_ALARM,
    CTL_IO_SETCMOS_RESET,
    CTL_IO_SETCMOS_POWER_ON,
	CTL_IO_SETCMOS_POWER_OFF,
	CTL_IO_GET_FANGCHAI,
	CTL_IO_SET_ALARMOUT_ON,
	CTL_IO_SET_ALARMOUT_OFF,
};

//-----------------------------------------------------------------------------
// Standard include files:
//

//-----------------------------------------------------------------------------
// Project include files:
//
#ifdef _cplusplus
extern "C" {
#endif

        int RoomGpioUtilSetMicOff();

        int RoomGpioUtilSetMicOn();

        int RoomGpioUtilSetSpkOff();

        int RoomGpioUtilSetSpkOn();

        int RoomGpioUtilSetVolHi();

        int RoomGpioUtilSetVolLo();

        //小门口机音频打开模拟通道
        int RoomGpioUtilSetDoorAudioOnA();

        //小门口机音频打开数字通道
        int RoomGpioUtilSetDoorAudioOnD();

        //小门口机音频通道关闭
        int RoomGpioUtilSetDoorAudioOff();

        int RoomGpioUtilOpen();

        int RoomGpioUtilStart();

        int RoomGpioUtilClose();

        int GetUrgentPinStatus();

        int tmGpioSetPin(int pin, int val);
        
        int InitGpio();

		int GetGpioValue(int pin);

		void SetAllMicSpkOFF();
		void SetAllMicSpkON();
		void SetMainMicSpkON();
		void SetMainMicSpkOFF();
		void SetSubMicSpkON();
		void SetSubMicSpkOFF();
		void SetMainMicStatus(int level);
		void SetMainSpkStatus(int level);
		void SetSubMicStatus(int level);
		void SetSubSpkStatus(int level);
#ifdef _cplusplus
}
#endif

#endif /* _ROOM_GPIO_UTIL_H */
