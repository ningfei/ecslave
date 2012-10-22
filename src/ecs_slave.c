
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

#include "ec_regs.h"
#include "fsm_slave.h"
#include "ecs_slave.h"

int ecs_net_init(e_slave * slave)
{
	slave->m_recvsock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	slave->m_sendsock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (slave->m_recvsock < 0 || slave->m_sendsock < 0) {
		perror("socket failed:");
		return -1;
	}
	slave->m_bcast_addr = inet_addr("255.255.255.255");
	return 0;
}

int ecs_get_local_conf(e_slave * slave)
{
	int ret;

	char temp_str[20];
	struct ifreq ifr;
	struct sockaddr_in sin_ip;
	struct sockaddr_in sin_mask;
	struct sockaddr *sa;

	strcpy(ifr.ifr_name, __eth_interface());

	/* Get host's ip */
	ret = ioctl(slave->m_sendsock, SIOCGIFADDR, &ifr);
	if (ret < 0) {
		/* no IP. put all zeros */
		memset(&sin_ip, 0, sizeof(struct sockaddr));
	} else {
		memcpy(&sin_ip, &ifr.ifr_addr, sizeof(struct sockaddr));
	}
	inet_ntop(AF_INET, &sin_ip.sin_addr,
		  slave->hostip, sizeof(slave->hostip));
	printf("%s:\nLOCAL IP %s\n", ifr.ifr_name, slave->hostip);
	/*
	 * get host's subnet mask
	 */
	ret = ioctl(slave->m_sendsock, SIOCGIFNETMASK, &ifr);
	if (ret < 0) {
		/* no mask. put all zeros */
		memset(&sin_mask, 0, sizeof(struct sockaddr));
		printf("LOCAL SUBNET MASK 0.0.0.0\n");
	} else {
		memcpy(&sin_mask, &ifr.ifr_netmask, sizeof(struct sockaddr));
		printf("LOCAL SUBNET MASK %s\n",
		       inet_ntop(AF_INET, &sin_mask.sin_addr, temp_str,
				 sizeof(temp_str)));
	}
	slave->subnet_mask = sin_mask.sin_addr.s_addr;

	/* get mac address */
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, __eth_interface());
	ret = ioctl(slave->m_sendsock, SIOCGIFHWADDR, &ifr);
	if (ret < 0) {
		perror("failed to get interface afdress\n");
		return -1;
	}
	sa = &ifr.ifr_hwaddr;
	if (sa->sa_family != ARPHRD_ETHER) {
		perror("interface without ARPHRD_ETHER");
		return -1;
	}
	/* first byte is type */
	slave->mac.ether_type = HTYPE_ETHER;
	/* other six bytes the actual addresses */
	memcpy(&slave->mac.ether_shost, sa->sa_data, 6);

	sprintf(slave->macaddr,
		"%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		slave->mac.ether_shost[0],
		slave->mac.ether_shost[1],
		slave->mac.ether_shost[2],
		slave->mac.ether_shost[3],
		slave->mac.ether_shost[4], slave->mac.ether_shost[5]);
	printf("LOCAL MAC %s\n", slave->macaddr);
	return 0;
}

int ecs_init(e_slave * slave)
{
	if (ecs_net_init(slave)) {
		puts("failed to initialize network\n");
		return -1;
	}
	if (ecs_get_local_conf(slave)) {
		puts("failed to get local ip\n");
		return -1;
	}
	return 0;
}

void ecs_tx_packet(e_slave * slave)
{
	int i;
	int bytes;
	struct ether_header *eh = (struct ether_header *)slave->pkt;
	struct sockaddr_ll socket_address = { 0 };

	for (i = 0; i < ETH_ALEN; i++) {
		slave->mac.ether_dhost[i] = 0xFF;
	}
	slave->mac.ether_type = htons(ETHERCAT_TYPE);
	socket_address.sll_family = PF_PACKET;
	socket_address.sll_protocol = 0;
	socket_address.sll_ifindex = if_nametoindex(__eth_interface());
	socket_address.sll_hatype = htons(ETHERCAT_TYPE);
	socket_address.sll_pkttype = PACKET_BROADCAST;
	socket_address.sll_halen = ETH_ALEN;

	memcpy(socket_address.sll_addr, slave->mac.ether_dhost, ETH_ALEN);
	memcpy(eh->ether_shost,
	       &slave->mac.ether_shost, sizeof(slave->mac.ether_shost));
	eh->ether_type = htons(ETHERCAT_TYPE);
	printf("%s Index=%d\n", __FUNCTION__, ec_dgram_pkt_index(slave->pkt));

	bytes = sendto(slave->m_sendsock,
		       slave->pkt,
		       slave->pkt_size, 0,
		       (struct sockaddr *)&socket_address,
		       (socklen_t) sizeof(socket_address));

	if (bytes < 0) {
		perror("tx packet: ");
	}
}

void ecs_rx_packet(e_slave * slave)
{
	unsigned int addr_size = sizeof(slave->m_addr);
	int flags = 0;

	while (1) {

		memset(slave->pkt, 0, sizeof(slave->pkt));
		slave->pkt_size = recvfrom(slave->m_recvsock,
					   (void *)&slave->pkt,
					   sizeof(slave->pkt), flags,
					   (struct sockaddr *)&slave->m_addr,
					   &addr_size);

		if (slave->pkt_size < 0) {
			perror("");
			continue;
		}
		if (!ec_is_ethercat(slave->pkt)) {
			continue;
		}
		if (slave->pkt_index == ec_dgram_pkt_index(slave->pkt)) {
			continue;
		}
		slave->pkt_index = ec_dgram_pkt_index(slave->pkt);
		break;
	}
	printf("%s: bytes read=%d element"
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
	e_slave slave = { 0 };

	if (argc < 2) {
		printf("%s <interface>\n", argv[0]);
		return 0;
	}
	strncpy(eth_interface, argv[1], sizeof(eth_interface));
	ec_init_regs();

	slave.fsm = &fsm_slave;

	__set_fsm_state(&slave, ecs_rx_packet);

	if (ecs_init(&slave) < 0)
		return -1;
	ecs_run(&slave);
	return 0;
}
