#ifndef __ECS_SLAVE_H__
#define __ECS_SLAVE_H__

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/time.h>

#include <net/if_arp.h>
#include <arpa/inet.h>
#include <string.h>
#include "ethercattype.h"
#include <net/ethernet.h>
#include <arpa/inet.h>

#define EC_MAX_PORTS 4

struct fsm_slave;

typedef struct __ec_interface__ {
	int index;
	struct sockaddr_in m_addr;
	struct ifreq ifr;
	char ip[32];
	char macaddr[32];
	struct ether_header mac;
	int sock;
	int subnet_mask;
	int link_up;
}ec_interface;

typedef struct __e_slave__ {

	uint8_t pkt[1492];
	uint8_t pkt_index;
	int pkt_size;

	ec_interface intr[EC_MAX_PORTS];
	struct fsm_slave *fsm;	/* finite state machine */
} e_slave;

int  ecs_net_init(int ,char *argv[], e_slave *);
int  ecs_init(e_slave *);
void ecs_run(e_slave *);

#define HTYPE_ETHER     	0x1	/* Ethernet  */
#define ETHERCAT_TYPE 		0x88a4

void ec_cmd_apwr(e_slave * slave);
void ec_cmd_fprd(e_slave * slave);
void ec_cmd_fpwr(e_slave * slave);
void ec_cmd_brw(e_slave * slave);
void ec_cmd_brd(e_slave * slave);
void ec_cmd_nop(e_slave * slave);

static inline uint8_t *__ecat_frameheader(uint8_t * h)
{
	return (uint8_t *) & h[sizeof(struct ether_header)];
}

static inline uint8_t *ec_dgram_data(uint8_t * d)
{
	return (uint8_t *) & (__ecat_frameheader(d)[sizeof(ec_comt)]);
}

static inline int ec_dgram_command(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
	return datagram->command;
}

static inline uint16_t ec_dgram_size(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
//	return (datagram->elength & 0b011111111111);
	return (datagram->elength & 0x07FF);
}

static inline uint16_t ec_dgram_data_length(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
//	uint16_t datalen = (datagram->dlength & 0b011111111111);
	uint16_t datalen = (datagram->dlength & 0x07FF);
	return datalen;
}

static inline uint16_t ec_dgram_adp(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
	return (datagram->ADP);
}

static inline uint32_t ec_dgram_laddr(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
	return ((uint32_t) datagram->ADP | (uint32_t) (datagram->ADO << 16));
}

/* address offset */
static inline uint16_t ec_dgram_ado(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
	return datagram->ADO;
}

static inline uint8_t ec_dgram_pkt_index(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
	return datagram->index;
}

static inline uint8_t *ec_get_shost(uint8_t* h)
{
	struct ether_header *eh = (struct ether_header *)h;

	return (uint8_t *)&(eh->ether_shost[0]);
}

static inline int ec_is_ethercat(uint8_t * h)
{
	struct ether_header *eh = (struct ether_header *)h;
	return htons(eh->ether_type) == ETHERCAT_TYPE;
}

static inline void __ec_inc_wkc(e_slave *slave)
{
	uint16_t wkc1;
	uint8_t *datagram = __ecat_frameheader(slave->pkt);
	uint16_t size = ec_dgram_size(slave->pkt);
	uint16_t *wkc = (uint16_t *) & datagram[size];

	wkc1 = *wkc;
	wkc1++;
	*wkc = wkc1;
}

#endif
