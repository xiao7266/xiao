#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <linux/if_ether.h>
#include <unistd.h>
#include "tmSysConfig.h"
//#include "Handler.h"
#define ETH_HW_ADDR_LEN 6
#define IP_ADDR_LEN 4
#define ARP_FRAME_TYPE 0x0806
#define ETHER_HW_TYPE 1
#define IP_PROTO_TYPE 0x0800
#define OP_ARP_REQUEST 2
#define OP_ARP_QUEST 1
#define DEFAULT_DEVICE "eth0"

extern T_SYSTEMINFO    *pSystemInfo;
static int ARP_Socket = 0;
#define CommonH

struct arp_packet
{
    u_char targ_hw_addr[ETH_HW_ADDR_LEN];
    u_char src_hw_addr[ETH_HW_ADDR_LEN];
    u_short frame_type;
    u_short hw_type;
    u_short prot_type;
    u_char hw_addr_size;
    u_char prot_addr_size;
    u_short op;
    u_char sndr_hw_addr[ETH_HW_ADDR_LEN];
    u_char sndr_ip_addr[IP_ADDR_LEN];
    u_char rcpt_hw_addr[ETH_HW_ADDR_LEN];
    u_char rcpt_ip_addr[IP_ADDR_LEN];
    u_char padding[18];
};

int InitArpSocket(void);
void CloseArpSocket(void);
int send_free_arp(void);

int InitArpSocket(void)
{
    ARP_Socket = socket(AF_INET, SOCK_PACKET, htons(ETH_P_RARP));
    if (ARP_Socket < 0)
    {
        perror("Create arpsocket error\r\n");
        return 0;
    }
    return 1;
}

int  send_free_arp(void)
{
    struct arp_packet pkt;
    struct sockaddr sa;

    pkt.frame_type = htons(ARP_FRAME_TYPE);
    pkt.hw_type = htons(ETHER_HW_TYPE);
    pkt.prot_type = htons(IP_PROTO_TYPE);
    pkt.hw_addr_size = ETH_HW_ADDR_LEN;
    pkt.prot_addr_size = IP_ADDR_LEN;
    pkt.op = htons(OP_ARP_QUEST);
    pkt.targ_hw_addr[0] = 0xff;
    pkt.targ_hw_addr[1] = 0xff;
    pkt.targ_hw_addr[2] = 0xff;
    pkt.targ_hw_addr[3] = 0xff;
    pkt.targ_hw_addr[4] = 0xff;
    pkt.targ_hw_addr[5] = 0xff;

    pkt.rcpt_hw_addr[0] = 0x00;
    pkt.rcpt_hw_addr[1] = 0x00;
    pkt.rcpt_hw_addr[2] = 0x00;
    pkt.rcpt_hw_addr[3] = 0x00;
    pkt.rcpt_hw_addr[4] = 0x00;
    pkt.rcpt_hw_addr[5] = 0x00;


    memcpy(pkt.src_hw_addr, pSystemInfo->BootInfo.MAC, 6);
    memcpy(pkt.sndr_hw_addr, pSystemInfo->BootInfo.MAC, 6);
    memcpy(pkt.sndr_ip_addr,&pSystemInfo->LocalSetting.privateinfo.LocalIP, 4);
    memcpy(pkt.rcpt_ip_addr,&pSystemInfo->LocalSetting.privateinfo.LocalIP, 4);
    memset(pkt.padding,0,18);
    strcpy(sa.sa_data, DEFAULT_DEVICE);

    if (sendto(ARP_Socket, &pkt, sizeof(pkt), 0, &sa, sizeof(sa)) < 0)
    {
        perror("arp send error");
        return 0;
    }
    return 1;
}

void CloseArpSocket(void)
{
    close(ARP_Socket);
}

