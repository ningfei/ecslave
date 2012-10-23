#ifndef __ECS_SLAVE_H__
#define __ECS_SLAVE_H__

#include "ethercattype.h"
#include <net/ethernet.h>
#include <arpa/inet.h>

struct fsm_slave;

typedef struct __e_slave__ {

	int subnet_mask;
	struct sockaddr_in m_addr;
	long m_bcast_addr;
	int m_sendsock;
	int m_recvsock;
	char macaddr[32];
	char hostip[32];
	struct ether_header mac;
	uint8_t pkt[1492];
	uint8_t pkt_index;
	int pkt_size;

	struct fsm_slave *fsm;	/* finite state machine */
} e_slave;

int ecs_get_local_conf(e_slave *);
void ecs_setup_jeaders(e_slave *);
int ecs_net_init(e_slave *);
int ecs_init(e_slave *);
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
	return (datagram->elength & 0b011111111111);
}

static inline uint16_t ec_dgram_data_length(uint8_t * d)
{
	ec_comt *datagram = (ec_comt *) __ecat_frameheader(d);
	uint16_t datalen = (datagram->dlength & 0b011111111111);
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

static inline int ec_is_ethercat(uint8_t * h)
{
	struct ether_header *eh = (struct ether_header *)h;
	return htons(eh->ether_type) == ETHERCAT_TYPE;
}

static char eth_interface[10];
static inline char *__eth_interface(void)
{
	return eth_interface;
}

#endif
