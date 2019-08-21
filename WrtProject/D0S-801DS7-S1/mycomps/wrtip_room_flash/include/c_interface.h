#ifndef _C_INTERFACE_H_
#define _C_INTERFACE_H_

#include "ctrlcmd.h"

//2015-2-5 wyx
//���Ӷ�����������˭���зֻ�
enum{
	TYPE_DOOR_CALLME = 0,
	TYPE_MANAGER_CALLME,
	TYPE_WALL_CALLME,
	TYPE_ROOM_CALLME,
	TYPE_IPDOOR_CALLME,
};
//���Ӷ�����������˭ժ���Խ� 1-�Է�ժ�� 2-�ֻ� 3-���� 4-PX 5-�ֻ� 6-��ֻ� 
enum{
	NONE_HOLDON = 0,
	REMOTE_HOLDON,
	LOCAL_HOLDON,
	PHYKEY_HOLDON,
	WRTPX_HOLDON,
	IOSDEV_HOLDON,
	MOREROOM_HOLDON,
};
#if defined(__cplusplus)
extern "C" {
#endif

        /***********************************************************
        �ӿ�˵����
        1��display Ϊ״̬��ʾ����ָ�룬��ǰ����ʲô����ͨ��״̬����
        ��ͨ������cmd�����������ߣ��ú�������ò�Ҫ������
        2������ѭ��
        wrthost_init();
        wrthost_send_cmd();
        //�����������ģ����øú�������IP��
        // wrthost_set_centerip();//�Ժ�����Щ�ӿ���
        //�������������øú�������IP��
        //   wrthost_set_doorip(); //�Ժ�����Щ�ӿ���
        wrthost_set_displaycallback();
        //�����˳���ʱ�����
        wrthost_uninit������
        3�����ӣ�
        �뿴test_wrtip_host.c��          
        ***********************************************************/


        /*******************************************************************
        �������ƣ�wrthost_init
        �������ܣ���ʼ���ֻ���������պ���������ӿ�
        ���������
        ���������
        ����ֵ��
        0�� �ɹ�
        -1�� ʧ��
        -2�� ֮ǰ�Ѿ���ʼ����ϡ�
        ********************************************************************/
        int  wrthost_init();

        /*******************************************************************
        �������ƣ�wrthost_init
        �������ܣ���ʼ���ֻ���������պ���������ӿ�
        ���������
        ���������
        ����ֵ��
        0�� �ɹ�
        -1�� ʧ��
        -2�� ֮ǰ�Ѿ���ʼ����ϡ�
        ********************************************************************/
        void wrthost_uninit();

        /*******************************************************************
        �������ƣ�wrthost_send_cmd
        �������ܣ����͸���������Զ˾����������ctrlqueue.h�в鿴
        ���������cmd  ��������
        buf   �������������
        buflen ����
        ���������
        ����ֵ��
        ********************************************************************/
        //void wrthost_send_cmd(short cmd,unsigned char* buf, int buflen);
        void wrthost_send_cmd(unsigned long event[4]);

        /*******************************************************************
        �������ƣ�wrthost_set_centerip
        �������ܣ��������ĵ�IP��ַ���������������ĵ�ʱ����Ҫ���øú���
        ���������ip:Ϊ����IP��ַ
        ���������
        ����ֵ��
        ˵����    ��ʱû��
        ********************************************************************/
        void wrthost_set_centerip(unsigned long ip);

        void wrthost_get_centerip(unsigned long* ip);

        /*******************************************************************
        �������ƣ�wrthost_set_doorip
        �������ܣ�����������IP��ַ�����������Ӹ�������ʱ����Ҫ���øú���
        ���������ip:Ϊ����IP��ַ
        ���������
        ����ֵ��
        ˵����    ��Ϊ���ü��ӵ�IP�Ľӿ���ʹ��
        ********************************************************************/
        void wrthost_set_doorip(unsigned long ip);

        void wrthost_get_doorip(unsigned long* ip);

        void wrthost_set_montip(unsigned long ip);

        void wrthost_get_managerip(unsigned long *ip);
        void wrthost_set_managerip(unsigned long ip);

        void wrthost_get_garageip(unsigned long *ip);
        void wrthost_set_garageip(unsigned long ip);

        int wrthost_sysidle();

        int wrthost_get_sys_status();

        void wrthost_initWatchDog();

        /*******************************************************************
        �������ƣ�wrthost_set_peer_room_ip
        �������ܣ����öԶ˷ֻ���IP��ַ�������б�ķֻ���ʱ����Ҫ���øú���
        ���������ip:Ϊ����IP��ַ
        ���������
        ����ֵ��
        ˵����    ��ʱû��
        ********************************************************************/
        void wrthost_set_peer_room_ip(unsigned long ip[4]);
        void wrthost_get_peer_room_ip(unsigned long* ip);

        /*******************************************************************
        �������ƣ�wrthost_set_displaycallback
        �������ܣ�������ʾ״̬�Ļص����������û��յ�������ߴ�����������ʱ��
        �䴦��״̬����ͨ���ûص�������ӳ�������ߡ�
        �ص�������ʵ����ò�Ҫ�������ġ�
        ���������p_display���ص�����ָ��
        ���������
        ����ֵ��  
        ********************************************************************/
        void wrthost_set_displaycallback(display  p_display);

        void wrthost_pause_play_video(int status);


        void wrthost_Set_RoomCallRoom_LocalID(char* id);


        void wrthost_req_sync(unsigned long ip);
		void wrthost_syssetbusy();
		void wrthost_syssetidle();
		void wrthost_send_tscal();
        /*******************************************************************
        DEBUG
        type 1: socket res.
        2: memory  res.
        ********************************************************************/
        void wrthost_debug(int type);

        //////////////////////////////////////////////////////////
        void StartGpio();
        int  GetGpioSignal();
        void StopGpio();
        /////////////////////////////////////////////////////////

        int get_host_ip(unsigned long* ip);
        int get_host_mask(unsigned long*  mask);
        int get_host_gateway(unsigned long* gateway);

        int set_host_ip(unsigned long ip);
        int set_host_mask(unsigned long mask);
        int set_host_gateway(unsigned long gateway);

        int save_modify();


        void SetRingtimes(unsigned long times);
        unsigned long GetRingtimes();
        void SetAvoidtimes(unsigned long hours,unsigned long min);
        void GetAvoidtimes(unsigned long * hours,unsigned long* min);
        void SetAgenttimes(unsigned long hours,unsigned long min);
        void GetAgenttimes(unsigned long * hours,unsigned long* min);
        void wrtip_play_voice(int index);

		void wrtip_default_set_ring();
        void wrtip_set_ring(int type,char* addr,int size);
        int wrtip_is_config_ring(char* addr,int size);
		void wrthost_set_holdon_device(int device);
		int wrthost_get_holdon_device();
		void wrthost_config_networking();
#if defined (__cplusplus)
}
#endif
#endif 