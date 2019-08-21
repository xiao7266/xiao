/*
*
*----------------------------------------------------------*/
/*
*      file :          RoomGpioUtil.c
*     
*                      分机接口驱动
*
*                      2008-01-04 
*/
/***********************************************************
*
*    208-04-29   增加了小门口机音频控制函数
*
*
************************************************************/
//-----------------------------------------------------------------------------
// Standard include files:
//-----------------------------------------------------------------------------
//
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>  
#include <errno.h>
#include <sys/ioctl.h>
#include "RoomGpioUtil.h"
#include "wrt_log.h"



#define IOC_MAGIC_G	'g'

#define IOCTL_SETLED_RED   _IOW( IOC_MAGIC_G, CTL_IO_SETLED_RED, unsigned int )
#define IOCTL_SETLED_GREEN _IOW( IOC_MAGIC_G, CTL_IO_SETLED_GREEN, unsigned int )
#define IOCTL_SETLED_YELLO _IOW( IOC_MAGIC_G, CTL_IO_SETLED_YELLO, unsigned int )
#define IOCTL_SETLED_OFF   _IOW( IOC_MAGIC_G, CTL_IO_SETLED_OFF, unsigned int )

#define IOCTL_SETCMOS_ON   _IOW( IOC_MAGIC_G, CTL_IO_SETCMOS_POWER_ON, unsigned int )
#define IOCTL_SETCMOS_OFF  _IOW( IOC_MAGIC_G, CTL_IO_SETCMOS_POWER_OFF, unsigned int )
#define IOCTL_SETCMOS_RST  _IOW( IOC_MAGIC_G, CTL_IO_SETCMOS_RESET, unsigned int )

#define IOCTL_SETBK_ON     _IOW( IOC_MAGIC_G, CTL_IO_SETBACKLIGHT_POWER_ON, unsigned int )
#define IOCTL_SETBK_OFF    _IOW( IOC_MAGIC_G, CTL_IO_SETBACKLIGHT_POWER_OFF, unsigned int )
#define IOCTL_SETLCD_RST   _IOW( IOC_MAGIC_G, CTL_IO_SETLCD_RESET, unsigned int )
#define IOCTL_SET_MIC_ON   _IOW( IOC_MAGIC_G, CTL_IO_SET_MIC_ON, unsigned int )
#define IOCTL_SET_MIC_OFF   _IOW( IOC_MAGIC_G, CTL_IO_SET_MIC_OFF, unsigned int )
#define IOCTL_SET_AUDIO_PA_ON _IOW( IOC_MAGIC_G, CTL_IO_SET_AUDIO_PA_ON, unsigned int )
#define IOCTL_SET_AUDIO_PA_OFF _IOW( IOC_MAGIC_G, CTL_IO_SET_AUDIO_PA_OFF, unsigned int )
#define IOCTL_SET_USB1_ON  _IOW( IOC_MAGIC_G, CTL_IO_SET_USB1_ON, unsigned int )
#define IOCTL_SET_USB1_OFF _IOW( IOC_MAGIC_G, CTL_IO_SET_USB1_OFF, unsigned int )
#define IOCTL_SET_PCAP_TS_RST _IOW( IOC_MAGIC_G, CTL_IO_SET_PCAP_TS_RST, unsigned int )
#define IOCTL_SET_UART_485_SEND _IOW( IOC_MAGIC_G, CTL_IO_SET_UART_485_SEND, unsigned int )
#define IOCTL_SET_UART_485_RECV _IOW( IOC_MAGIC_G, CTL_IO_SET_UART_485_RECV, unsigned int )
#define IOCTL_SET_SUB_SPK_ON _IOW( IOC_MAGIC_G, CTL_IO_SET_SUB_SPK_ON, unsigned int )
#define IOCTL_SET_SUB_SPK_OFF _IOW( IOC_MAGIC_G, CTL_IO_SET_SUB_SPK_OFF, unsigned int )
#define IOCTL_SET_SUB_MIC_ON _IOW( IOC_MAGIC_G, CTL_IO_SET_SUB_MIC_ON, unsigned int )
#define IOCTL_SET_SUB_MIC_OFF _IOW( IOC_MAGIC_G, CTL_IO_SET_SUB_MIC_OFF, unsigned int )
#define IOCTL_GET_DOORBELL _IOW( IOC_MAGIC_G, CTL_IO_GET_DOORBELL, unsigned int )
#define IOCTL_GET_EMERGCY  _IOW( IOC_MAGIC_G, CTL_IO_GET_EMERGCY, unsigned int )
#define IOCTL_GET_EMERGCY_OUTSIDE _IOW( IOC_MAGIC_G, CTL_IO_GET_EMERGCY_OUTSIDE, unsigned int )
#define IOCTL_GET_CALLLIFT _IOW( IOC_MAGIC_G, CTL_IO_GET_CALLLIFT, unsigned int )
#define IOCTL_GET_SUB_HOLD _IOW( IOC_MAGIC_G, CTL_IO_GET_SUB_HOLD, unsigned int )
#define IOCTL_GET_SUB_LOCK _IOW( IOC_MAGIC_G, CTL_IO_GET_SUB_LOCK, unsigned int )
#define IOCTL_GET_SUB_ALARM _IOW( IOC_MAGIC_G, CTL_IO_GET_SUB_ALARM, unsigned int )

#define IOCTL_GET_FANGCHAI _IOW( IOC_MAGIC_G, CTL_IO_GET_FANGCHAI, unsigned int )
#define IOCTL_SET_ALARMOUT_ON  _IOW( IOC_MAGIC_G, CTL_IO_SET_ALARMOUT_ON, unsigned int )
#define IOCTL_SET_ALARMOUT_OFF _IOW( IOC_MAGIC_G, CTL_IO_SET_ALARMOUT_OFF, unsigned int )

static int g_gpio_fd = -1;

int InitGpio()
{
	if(g_gpio_fd == -1){
		g_gpio_fd = open("/dev/fgpio",O_RDWR);
		if(g_gpio_fd == -1)
			WRT_DEBUG("打开/dev/fgpio错误 Error:%d",errno);
	}
}


//-----------------------------------------------------------------------------
// fuctions
//-----------------------------------------------------------------------------
int tmGpioSetPin(int pin, int val)
{
	int ret = 0;
	if(g_gpio_fd == -1)
		return -1;
        
 	switch(pin)
 	{	
 		case CTL_IO_SETLED_RED:
			if(val == 0)
				ret = ioctl(g_gpio_fd,IOCTL_SETLED_OFF,(unsigned int *)&val);
			else
				ret = ioctl(g_gpio_fd,IOCTL_SETLED_RED,(unsigned int *)&val);
			break;
		case CTL_IO_SETLED_GREEN:
			if(val == 0)
				ret = ioctl(g_gpio_fd,IOCTL_SETLED_OFF,(unsigned int *)&val);
			else 			
				ret = ioctl(g_gpio_fd,IOCTL_SETLED_GREEN,(unsigned int *)&val);
			break;
		case CTL_IO_SETBACKLIGHT_POWER_ON: 			
			ret = ioctl(g_gpio_fd,IOCTL_SETBK_ON,(unsigned int *)&val);	
			break;
		case CTL_IO_SETBACKLIGHT_POWER_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SETBK_OFF,(unsigned int *)&val);		
			break;
		case CTL_IO_SETCMOS_POWER_ON:
			ret =ioctl(g_gpio_fd,IOCTL_SETCMOS_ON,(unsigned int *)&val);		
			break;
		case CTL_IO_SETCMOS_POWER_OFF:
			ret =ioctl(g_gpio_fd,IOCTL_SETCMOS_OFF,(unsigned int *)&val);	
			break;
		case CTL_IO_SET_AUDIO_PA_ON:
			ret = ioctl(g_gpio_fd,IOCTL_SET_AUDIO_PA_ON,(unsigned int *)&val);
			break;
		case CTL_IO_SET_AUDIO_PA_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SET_AUDIO_PA_OFF,(unsigned int *)&val);
			break;
		case CTL_IO_SET_MIC_ON://主分机MIC
			ret = ioctl(g_gpio_fd,IOCTL_SET_MIC_ON,(unsigned int *)&val);
			break;
		case CTL_IO_SET_MIC_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SET_MIC_OFF,(unsigned int *)&val);
			break;
		case CTL_IO_SET_SUB_SPK_ON:
			ret = ioctl(g_gpio_fd,IOCTL_SET_SUB_SPK_ON,(unsigned int *)&val);
			break;
		case CTL_IO_SET_SUB_SPK_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SET_SUB_SPK_OFF,(unsigned int *)&val);
			break;
		case CTL_IO_SET_SUB_MIC_ON:
			ret = ioctl(g_gpio_fd,IOCTL_SET_SUB_MIC_ON,(unsigned int *)&val);
			break;
		case CTL_IO_SET_SUB_MIC_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SET_SUB_MIC_OFF,(unsigned int *)&val);
			break;
		case CTL_IO_SET_USB1_ON:
			ret = ioctl(g_gpio_fd,IOCTL_SET_USB1_ON,(unsigned int *)&val);
			break;
		case CTL_IO_SET_USB1_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SET_USB1_OFF,(unsigned int *)&val);
			break;
		case CTL_IO_SET_ALARMOUT_ON:
			ret = ioctl(g_gpio_fd,IOCTL_SET_ALARMOUT_ON,(unsigned int *)&val);
			break;
		case CTL_IO_SET_ALARMOUT_OFF:
			ret = ioctl(g_gpio_fd,IOCTL_SET_ALARMOUT_OFF,(unsigned int *)&val);
			break;
		default:
			break;
 	}
 	//WRT_DEBUG("GPIO: g_gpio_fd = %d pin = %d  value=%d,ret = %d,errno = %x\n",g_gpio_fd,pin,val,ret,errno);
    return 0;

}

int GetGpioValue(int pin)
{
	int val = -1;
	if(g_gpio_fd == -1)
		return val;
	switch(pin)
	{
		case CTL_IO_GET_EMERGCY://紧急报警IO状态
			ioctl(g_gpio_fd,IOCTL_GET_EMERGCY,(unsigned int *)&val);
        	//printf("emergch pin = %d\n", val);
        	break;
        case CTL_IO_GET_CALLLIFT://梯控IO状态
        	ioctl(g_gpio_fd,IOCTL_GET_CALLLIFT,(unsigned int *)&val);
        	//printf("calllift pin = %d\n", val);
        	break;
        case CTL_IO_GET_DOORBELL://门铃IO状态
        	ioctl(g_gpio_fd,IOCTL_GET_DOORBELL,(unsigned int *)&val);
        	//printf("doorbell pin = %d\n", val);
        	break;
        case CTL_IO_GET_EMERGCY_OUTSIDE:
        	ioctl(g_gpio_fd,IOCTL_GET_EMERGCY_OUTSIDE,(unsigned int *)&val);
        	break;

		case CTL_IO_GET_SUB_HOLD:
			ioctl(g_gpio_fd,IOCTL_GET_SUB_HOLD,(unsigned int *)&val);
			break;
		case CTL_IO_GET_SUB_LOCK:
			ioctl(g_gpio_fd,IOCTL_GET_SUB_LOCK,(unsigned int *)&val);
			break;
		case CTL_IO_GET_SUB_ALARM:
			ioctl(g_gpio_fd,IOCTL_GET_SUB_ALARM,(unsigned int *)&val);
			break;
		case CTL_IO_GET_FANGCHAI:
			ioctl(g_gpio_fd,IOCTL_GET_FANGCHAI,(unsigned int *)&val);
			break;
        default:
        	break;
	}
	return val;
}


void SetAllMicSpkOFF()
{
	tmGpioSetPin(CTL_IO_SET_AUDIO_PA_OFF,1);
	tmGpioSetPin(CTL_IO_SET_MIC_OFF,1);
	tmGpioSetPin(CTL_IO_SET_SUB_MIC_OFF,1);
	tmGpioSetPin(CTL_IO_SET_SUB_SPK_OFF,1);
}
void SetAllMicSpkON()
{
	tmGpioSetPin(CTL_IO_SET_AUDIO_PA_ON,1);
	tmGpioSetPin(CTL_IO_SET_MIC_ON,1);
	tmGpioSetPin(CTL_IO_SET_SUB_MIC_ON,1);
	tmGpioSetPin(CTL_IO_SET_SUB_SPK_ON,1);
}
void SetMainMicSpkON()
{
	tmGpioSetPin(CTL_IO_SET_AUDIO_PA_ON,1);
	tmGpioSetPin(CTL_IO_SET_MIC_ON,1);
}
void SetMainMicSpkOFF()
{
	tmGpioSetPin(CTL_IO_SET_AUDIO_PA_OFF,1);
	tmGpioSetPin(CTL_IO_SET_MIC_OFF,1);
}
void SetSubMicSpkON()
{
	tmGpioSetPin(CTL_IO_SET_SUB_MIC_ON,1);
	tmGpioSetPin(CTL_IO_SET_SUB_SPK_ON,1);
}
void SetSubMicSpkOFF()
{
	tmGpioSetPin(CTL_IO_SET_SUB_MIC_OFF,1);
	tmGpioSetPin(CTL_IO_SET_SUB_SPK_OFF,1);
}
void SetMainMicStatus(int level)
{
	if(level == 0)
		tmGpioSetPin(CTL_IO_SET_MIC_OFF,1);
	else if(level == 1)
		tmGpioSetPin(CTL_IO_SET_MIC_ON,1);
}
void SetMainSpkStatus(int level)
{
	if(level == 0)
		tmGpioSetPin(CTL_IO_SET_AUDIO_PA_OFF,1);
	else if(level == 1)
		tmGpioSetPin(CTL_IO_SET_AUDIO_PA_ON,1);
}
void SetSubMicStatus(int level)
{
	if(level == 0)
		tmGpioSetPin(CTL_IO_SET_SUB_MIC_OFF,1);
	else if(level == 1)
		tmGpioSetPin(CTL_IO_SET_SUB_MIC_ON,1);
}
void SetSubSpkStatus(int level)
{
	if(level == 0)
		tmGpioSetPin(CTL_IO_SET_SUB_SPK_OFF,0);
	else if(level == 1)
	{
		tmGpioSetPin(CTL_IO_SET_SUB_SPK_ON,0);
	}
}
int GetUrgentPinStatus(){
  
        return 0;
}

//-------------------------------------------
// GPIO51 = 0
//-------------------------------------------
int RoomGpioUtilSetMicOff()
{

        return 0;

}


//-------------------------------------------
// GPIO51 = 1
//-------------------------------------------
int RoomGpioUtilSetMicOn()
{


        return 0;

}



//-------------------------------------------
// GPIO52 = 1
//-------------------------------------------
int RoomGpioUtilSetSpkOff()
{

        return 0;

}


//-------------------------------------------
//
//-------------------------------------------
int RoomGpioUtilSetSpkOn()
{


        return 0;

}


//-------------------------------------------
// GPIO55 = 0
//-------------------------------------------
int RoomGpioUtilSetVolHi()
{



        return 0;

}


//-------------------------------------------
// GPIO55 = 1
//-------------------------------------------
int RoomGpioUtilSetVolLo()
{



        return 0;

}


//-------------------------------------------
//    小门口机音频模拟通道打开   GPIO26=1 GPIO25=1
//    小门口机<---->分机
//-------------------------------------------
int RoomGpioUtilSetDoorAudioOnA()
{


        return 0;


}


//-------------------------------------------
//   小门口机音频通道关闭  GPIO26=0 GPIO25=1
//-------------------------------------------
int RoomGpioUtilSetDoorAudioOff()
{

        return 0;

}


//-------------------------------------------
//   小门口机音频数字通道打开 GPIO26=1 GPIO25=0
//   小门口机<---->中心
//-------------------------------------------
int RoomGpioUtilSetDoorAudioOnD()
{
  

        return 0;

}
//-------------------------------------------
//
//-------------------------------------------

int RoomGpioUtilOpen()
{
	return 0;
}

int RoomGpioUtilClose()
{
	return 0;
}

int RoomGpioUtilStart()
{
	return 0;
}
