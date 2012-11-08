

#include <stdint.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"

#define  WORKING_CNT_SIZE 2

void raw_ecs_tx_packet(e_slave * ecs)
{
	int i;
	int bytes;
	struct ether_header *eh = (struct ether_header *)&ecs->pkt_head[0];
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

	bytes = sendto(intr->sock,
		       ecs->pkt_head,
		       ecs->pkt_size, 0,
		       (struct sockaddr *)&socket_address,
		       (socklen_t) sizeof(socket_address));

	if (bytes < 0) {
		perror("tx packet: ");
	}
}

void ecs_tx_packet(e_slave * ecs,uint8_t *d)
{
	raw_ecs_tx_packet(ecs);
	 __set_fsm_state(ecs, ecs_rx_packet);
}

void ecs_process_next_dgram(e_slave * ecs,uint8_t *d)
{
	if (--ecs->dgrams_cnt) {
		/* move to next packet */
		ecs->dgram_processed += WORKING_CNT_SIZE + sizeof(ec_dgram) + __ec_dgram_dlength(d);
		return __set_fsm_state(ecs, ecs_process_packet);
	}
	 __set_fsm_state(ecs, ecs_tx_packet);
}

int  ec_nr_dgrams(uint8_t *raw_pkt)
{
	int i = 0;
	int f;
	int frame_size = __ec_frame_size(raw_pkt);
	uint8_t* dgram  = __ecat_frameheader(raw_pkt) +  sizeof(ec_frame_header);
		
	f = frame_size;
	for (;frame_size > 0;i++){
		frame_size -= (sizeof(ec_dgram) + __ec_dgram_dlength(dgram) + WORKING_CNT_SIZE) ;
		dgram += sizeof(ec_dgram) + WORKING_CNT_SIZE + __ec_dgram_dlength(dgram); 		
	}
	if (frame_size < 0){
		printf("aieeee %d %d\n",frame_size,f);
	}
	return i;
}

void raw_ecs_rx_packet(e_slave* slave)
{
	int flags = 0;
	ec_interface *intr = ec_rx_interface(slave);
	unsigned int addr_size = sizeof(intr->m_addr);

	while (1) {
		memset(slave->pkt_head, 0, sizeof(slave->pkt_head));
		slave->pkt_size = recvfrom(intr->sock,
					   (void *)&slave->pkt_head,
					   sizeof(slave->pkt_head), flags,
					   (struct sockaddr *)&intr->m_addr,
					   &addr_size);

		if (slave->pkt_size < 0) {
			perror("");
			continue;
		}
		if (!__ec_is_ethercat(slave->pkt_head)) {
			continue;
		}
		/* we might get the packet we just sent */
		if (!memcmp(__ec_get_shost(slave->pkt_head),
					intr->mac.ether_shost,
					sizeof(intr->mac.ether_shost)) ){
			continue;
		}
		break;
	}
	// grab first ecat dgram
	slave->dgram_processed =  __ecat_frameheader(slave->pkt_head) + sizeof(ec_frame_header);
	slave->dgrams_cnt = ec_nr_dgrams(slave->pkt_head);
	__set_fsm_state(slave, ecs_process_packet);
}

void ecs_rx_packet(e_slave *ecs,uint8_t *d)
{
	raw_ecs_rx_packet(ecs);
}

void ecs_process_packet(e_slave * ecs, uint8_t *dgram_ec)
{
	__set_fsm_state(ecs, ec_cmd_nop);
	
	switch (__ec_dgram_command(dgram_ec)) {
	case EC_CMD_NOP:
		puts("Command NOP");
		break;

	case EC_CMD_APRD:
		puts("Auto Increment Read");
		break;

	case EC_CMD_APWR:
		__set_fsm_state(ecs, ec_cmd_apwr);
		break;

	case EC_CMD_APRW:
		puts("Auto Increment Read Write");
		break;

	case EC_CMD_FPRD:
		__set_fsm_state(ecs, ec_cmd_fprd);
		break;

	case EC_CMD_FPWR:
		__set_fsm_state(ecs, ec_cmd_fpwr);
		break;

	case EC_CMD_FPRW:
		puts("Configured Address Read Write");
		break;

	case EC_CMD_BRD:
		__set_fsm_state(ecs, ec_cmd_brd);
		break;

	case EC_CMD_BWR:
		__set_fsm_state(ecs, ec_cmd_brw);
		break;

	case EC_CMD_BRW:
		puts("Broadcast Read Write");
		break;

	case EC_CMD_LRD:
		__set_fsm_state(ecs, ec_cmd_lrd);
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
	ecs->fsm->state(ecs, dgram_ec);
}

void ecs_run(e_slave *ecs)
{
	while (1) {
		ecs->fsm->state(ecs, ecs->dgram_processed);
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
		printf("%s < rx interface> < tx interface>\n", argv[0]);
		return 0;
	}
  	if (ecs_net_init(argc, argv, &ecs) < 0){
		return -1;
	}
	ec_init_regs(&ecs);
	init_sii();

	ecs.fsm = &fsm_slave;
	ecs.dgram_processed = &ecs.pkt_head[0];
	ecs.dgrams_cnt = 0;
	__set_fsm_state(&ecs, ecs_rx_packet);

	ecs_run(&ecs);
	return 0;
}
