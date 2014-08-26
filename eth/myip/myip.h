
#ifndef __MYIP_H__
#define __MYIP_H__

#include "main.h"

#define LOCAL_IP_ADDR 0xC0A80001
#if 0
#define LOCAL_MAC_ADDR0 0x00
#define LOCAL_MAC_ADDR1 0x01
#define LOCAL_MAC_ADDR2 0x02
#define LOCAL_MAC_ADDR3 0x03
#define LOCAL_MAC_ADDR4 0x04
#define LOCAL_MAC_ADDR5 0x05
#endif

#define ARP_FRAME_TYPE  0x0608

#define IP_FRAME_TYPE   0x0008

#define ICMP_ECHO       0x8

#pragma pack(1)
typedef struct
{
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
} preamble1;

typedef struct
{
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;

    uint8_t ver_ihl;
    uint8_t serv_type;
    uint16_t total_len;
    uint16_t id;
    uint16_t frag_offset;
    uint8_t ttl;
    uint8_t proto;
    uint16_t header_cksum;
    uint32_t src_ip_addr;
    uint32_t dst_ip_addr;
} preamble2;


#define MAC_HEADER_SIZE sizeof(preamble1)
//#define ARP_HEADER_SIZE 28

typedef union
{
    uint8_t packet[ETH_MAX_PACKET_SIZE];
    struct
    {
        preamble1 p;
        uint8_t *payload;
    } e;
} ETH_FRAME;

typedef struct
{
    preamble1 p;
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t src_mac_addr[6];
    uint32_t src_ip_addr;
    uint8_t dst_mac_addr[6];
    uint32_t dst_ip_addr;
} ARP_FRAME;

typedef struct
{
    preamble2 p;

    uint8_t *data;
} IP_FRAME;

typedef struct {
    preamble2 p;

    uint8_t type;
    uint8_t code;
    uint16_t cksum;

    uint8_t *data;
} ICMP_FRAME;

typedef struct {
    preamble2 p;

    uint16_t src_port;
    uint16_t dst_port;
    uint16_t len;
    uint16_t cksum;

    uint8_t *data;
} UDP_FRAME;

typedef uint16_t (*con_handler)(UDP_FRAME* frm, uint16_t sz);

typedef struct {
    uint32_t port;
    con_handler con_handler_ptr;
} UDP_ENTRY;

#define UDP_HEADER_SIZE     0x8

#define UDP_CON_CLOSED      0x0
#define UDP_CON_LISTEN      0x1
#define UDP_CON_ESTABLISHED 0x2

typedef struct
{
    uint32_t ip_addr;
    uint8_t mac_addr[6];
    uint32_t time;
} ARP_ENTRY;

#define ARP_TABLE_SZ        10
#define UDP_TABLE_SZ        10

#define ARP_HTYPE           0x0100
#define ARP_PTYPE           0x0008
#define ARP_HLEN            0x06
#define ARP_PLEN            0x04
#define ARP_OPER_REQ        0x0100
#define ARP_OPER_REPL       0x0200

#define HTONS_16( x ) ((((x) >> 8) + ((x) << 8)) & 0x0000FFFF)
#define HTONS_32( x ) ( ((x >> 24) & 0x000000FF) + ((x >> 8) & 0x0000FF00) + \
                        ((x << 8) & 0x00FF0000) + ((x << 24) & 0xFF000000))

#define IP_VER_IHL          0x45
#define IP_TTL              0x80
#define FRAG_MASK           0xFFBF
#define FRAG_FLAG           0x0020

#define ICMP_PROTO          0x01
#define ICMP_ECHO_REPLY     0x0

#define UDP_PROTO           0x11

void        myip_init(void);
void        myip_update_arp_table(uint32_t ip_addr, uint8_t *mac_addr);
void        myip_add_udp_con(uint16_t port, con_handler con_handler_ptr);
uint16_t    myip_handle_arp_frame(ETH_FRAME *frm, uint16_t sz);
uint16_t    myip_handle_ip_frame(ETH_FRAME *frm, uint16_t sz);
uint16_t    myip_handle_icmp_frame(ETH_FRAME *frm, uint16_t sz);
uint16_t    myip_handle_udp_frame(ETH_FRAME *frm, uint16_t sz);

#endif
