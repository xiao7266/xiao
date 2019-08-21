#ifndef COMMON_H
#define COMMON_H

#define HEAD				"WRTI"
#define MSG_BUFF_SIZE		256
/*
**app gateway cmd
**app�㲥�����ػ�Ӧָ��
*/

#define APP_BROADCAST_0x06F0		0x06F0
#define GATEWAY_ACK_0x08F0			0x08F0

/***************/



/*
**process to process
**network���̺����ؽ���ͨѶָ��
*/

#define BROADCAST_GET_CONFIG_0xA001				0xA001   /*��ȡ����ip������id*/
#define BROADCAST_GET_CONFIG_ACK_0xAC02			0xAC02


#define BROADCAST_SEND_0xA002					0xA002   /*֪ͨ���ط���*/
#define BROADCAST_SEND_ERROR_0xA003				0xA003   /*�㲥�����ʹ���*/


/***************/
typedef struct _gateWayConfig{
		unsigned long		gateWayIp;
		char				gateWayID[11];
}GateWayConfig;


typedef struct _p2pPacket{
		char				head[4];
		int					length;
		unsigned long		cmd;
}p2pPacket;

typedef struct _myMsg{
		long				mtype;
		char				mdata[MSG_BUFF_SIZE];
}myMsg;



#endif
