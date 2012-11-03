

#include <stdint.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"

void ecs_tx_packet(e_slave * ecs)
{
	int i;
	int bytes;
	struct ether_header *eh = (struct ether_header *)ecs->pkt;
	struct sockaddr_ll socket_address = { 0 };
	ec_interface *intr = ec_tx_interface(ecs);

	for (i = 0; i < ETH_ALEN; i++) {
		intr->mac.ether_dhost[i] = 0xFF;
	}
	intr->mac.ether_type = htons(ETHERCAT_TYPE);
	socket_address.sll_family = PF_PACKET;
	socket_address.sll_protocol = 0;
	socket_address.sll_ifindex = intr->index;
	socket_address.sll_hatype = htons(ETHERCAT_TYPE);
	socket_address.sll_pkttype = PACKET_BROADCAST;
	socket_address.sll_halen = ETH_ALEN;

	memcpy(socket_address.sll_addr, intr->mac.ether_dhost, ETH_ALEN);
	memcpy(eh->ether_shost,
	       &intr->mac.ether_shost, 
		sizeof(intr->mac.ether_shost));
	eh->ether_type = htons(ETHERCAT_TYPE);
	ec_printf("%s Index=0x%x\n", __FUNCTION__, ec_dgram_pkt_index(ecs->pkt));

	bytes = sendto(intr->sock,
		       ecs->pkt,
		       ecs->pkt_size, 0,
		       (struct sockaddr *)&socket_address,
		       (socklen_t) sizeof(socket_address));

	if (bytes < 0) {
		perror("tx packet: ");
	}
}

void ecs_rx_packet(e_slave * slave)
{
	int flags = 0;
	ec_interface *intr = ec_rx_interface(slave);
	unsigned int addr_size = sizeof(intr->m_addr);

	while (1) {

		memset(slave->pkt, 0, sizeof(slave->pkt));
		slave->pkt_size = recvfrom(intr->sock,
					   (void *)&slave->pkt,
					   sizeof(slave->pkt), flags,
					   (struct sockaddr *)&intr->m_addr,
					   &addr_size);

		if (slave->pkt_size < 0) {
			perror("");
			continue;
		}
		if (!ec_is_ethercat(slave->pkt)) {
			continue;
		}
		if (!memcmp(ec_get_shost(slave->pkt),
					intr->mac.ether_shost,
					sizeof(intr->mac.ether_shost)) ){
			continue;
		}
		slave->pkt_index = ec_dgram_pkt_index(slave->pkt);
		break;
	}
	ec_printf("%s: bytes read=%d element"
	       "index=0x%x size=%d data cmd=%d length=%d\n",
	        __FUNCTION__,
		slave->pkt_size,
	       	slave->pkt_index,
	       	ec_dgram_size(slave->pkt), 
		ec_dgram_command(slave->pkt),
		ec_dgram_data_length(slave->pkt));

	__set_fsm_state(slave, ecs_process_packet);
}

void ecs_process_packet(e_slave * slave)
{
	__set_fsm_state(slave, ec_cmd_nop);

	switch (ec_dgram_command(slave->pkt)) {
	case EC_CMD_NOP:
		puts("Command NOP");
		break;

	case EC_CMD_APRD:
		puts("Auto Increment Read");
		break;

	case EC_CMD_APWR:
		__set_fsm_state(slave, ec_cmd_apwr);
		break;

	case EC_CMD_APRW:
		puts("Auto Increment Read Write");
		break;

	case EC_CMD_FPRD:
		__set_fsm_state(slave, ec_cmd_fprd);
		break;

	case EC_CMD_FPWR:
		__set_fsm_state(slave, ec_cmd_fpwr);
		break;

	case EC_CMD_FPRW:
		puts("Configured Address Read Write");
		break;

	case EC_CMD_BRD:
		__set_fsm_state(slave, ec_cmd_brd);
		break;

	case EC_CMD_BWR:
		__set_fsm_state(slave, ec_cmd_brw);
		break;

	case EC_CMD_BRW:
		puts("Broadcast Read Write");
		break;

	case EC_CMD_LRD:
		puts("Logical Memory Read");
		break;

	case EC_CMD_LWR:
		puts("Logical Memory Write");
		break;

	case EC_CMD_LRW:
		puts("Logical Memory Read Write");
		break;

	case EC_CMD_ARMW:
		puts("Auto Increment Read Multiple Write");
		break;

	case EC_CMD_FRMW:
		puts("Configured Read Multiple Write");
		break;
	default:
		puts("unknown command");
	}
	slave->fsm->state(slave);
}

void ecs_run(e_slave * slave)
{
	while (1) {
		slave->fsm->state(slave);
	}
}

int main(int argc, char *argv[])
{
	struct fsm_slave fsm_slave;
	e_slave ecs;

	if (argc < 2) {
		/*
		 * if you provide two different interfaces 
		 *  then it is considered an open loop,
 		  * else it is a closed loop ,ie, last slave.
		*/
		printf("%s RX <interface> TX <interface>\n", argv[0]);
		return 0;
	}
  	if (ecs_net_init(argc, argv, &ecs) < 0){
		return -1;
	}
	ec_init_regs(&ecs);
	init_sii();

	ecs.fsm = &fsm_slave;

	__set_fsm_state(&ecs, ecs_rx_packet);

	ecs_run(&ecs);
	return 0;
}
