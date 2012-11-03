#include "ec_net.h"
#include "ecs_slave.h"
#include "ethercattype.h"

#define EC_MAX_PORTS 4

ec_interface * ec_tx_interface(e_slave *ecv)
{
	return NULL;
}

ec_interface* ec_rx_interface(e_slave* ecv)
{
	return NULL;
}


int is_nic_link_up(e_slave* esv, int eth)
{
	if (eth > EC_MAX_PORTS)
		return 0;
	return esv->intr[eth].link_up;
}

int is_nic_loop_closed(e_slave* esv, int eth)
{
	return 0;
}

int is_nic_signal_detected(e_slave* esv, int eth)
{
	return 0;
}

int ecs_get_intr_conf(ec_interface *intr)
{
	int ret;
	char temp_str[20];
	struct sockaddr_in sin_ip;
	struct sockaddr_in sin_mask;
	struct sockaddr *sa;

	/* Get host's ip */
	ret = ioctl(intr->sock, SIOCGIFADDR, &intr->ifr);
	if (ret < 0) {
		/* no IP. put all zeros */
		memset(&sin_ip, 0, sizeof(struct sockaddr));
	} else {
		memcpy(&sin_ip, &intr->ifr.ifr_addr, sizeof(struct sockaddr));
	}

	inet_ntop(AF_INET, &sin_ip.sin_addr, intr->ip, sizeof(intr->ip));

	ec_printf("%s:\nLOCAL IP %s\n", intr->ifr.ifr_name, intr->ip);
	/*
	 * get host's subnet mask
	 */
	ret = ioctl(intr->sock, SIOCGIFNETMASK, &intr->ifr);
	if (ret < 0) {
		/* no mask. put all zeros */
		memset(&sin_mask, 0, sizeof(struct sockaddr));
		ec_printf("LOCAL SUBNET MASK 0.0.0.0\n");
	} else {
		memcpy(&sin_mask, &intr->ifr.ifr_netmask, sizeof(struct sockaddr));
		ec_printf("LOCAL SUBNET MASK %s\n",
			  inet_ntop(AF_INET, &sin_mask.sin_addr, temp_str,
				    sizeof(temp_str)));
	}
	intr->subnet_mask = sin_mask.sin_addr.s_addr;
	/* get mac address */
	ret = ioctl(intr->sock, SIOCGIFHWADDR, &intr->ifr);
	if (ret < 0) {
		perror("failed to get interface afdress\n");
		return -1;
	}
	sa = &intr->ifr.ifr_hwaddr;
	if (sa->sa_family != ARPHRD_ETHER) {
		perror("interface without ARPHRD_ETHER");
		return -1;
	}
	/* first byte is type */
	intr->mac.ether_type = HTYPE_ETHER;
	/* other six bytes the actual addresses */
	memcpy(&intr->mac.ether_shost, sa->sa_data, 6);

	ec_printf(intr->macaddr,
		  "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		  intr->mac.ether_shost[0],
		  intr->mac.ether_shost[1],
		  intr->mac.ether_shost[2],
		  intr->mac.ether_shost[3],
		  intr->mac.ether_shost[4], intr->mac.ether_shost[5]);

	if (ioctl(intr->sock, SIOCGIFINDEX, &intr->ifr) == -1) {
		return (-1);
	}
  	intr->index = intr->ifr.ifr_ifindex;
	ec_printf("LOCAL MAC %s\n", intr->macaddr);
	return 0;
}

int ecs_sock(ec_interface* intr)
{
	intr->sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (intr->sock < 0) {
		perror("socket failed:");
		return -1;
	}
	return 0;
}

int ecs_net_init(int argc, char *argv[], e_slave * esv)
{
	int i;

	for (i = 0; i < argc; i++) {
		strncpy(esv->intr[i].ifr.ifr_name,
		       argv[i + 1], sizeof(esv->intr[i].ifr.ifr_name));
		if (ecs_sock(&esv->intr[i]))
			return -1;
		if (ecs_get_intr_conf(&esv->intr[i]))
			return -1;
	}
	return 0;
}
