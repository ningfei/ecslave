#include <stdint.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"

void raw_ecs_tx_packet(e_slave *ecs)
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
		/* we might get the packet we just sent */
		if (!memcmp(__ec_get_shost(slave->pkt_head),
					intr->mac.ether_shost,
					sizeof(intr->mac.ether_shost)) ){
			continue;
		}
		return;
	}
}

