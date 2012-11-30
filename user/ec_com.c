#include "xgeneral.h"
#include "ec_device.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "fsm_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"
#include "ec_cmd.h"
#include <pcap/pcap.h>

static pcap_t *tx_handle = 0;
static pcap_t *rx_handle = 0;

int dbg_index(e_slave *ecs)
{
	uint8_t *p =   __ecat_frameheader(ecs->pkt_head) + sizeof(ec_frame_header);
	return __ec_dgram_pkt_index(p);
}

uint16_t ec_dbg_wkc(e_slave *ecs)
{
	uint8_t *p =   __ecat_frameheader(ecs->pkt_head) + sizeof(ec_frame_header);
	return __ec_wkc(p);
}

void ec_tx_pkt(uint8_t* buf, int size, struct ec_device *intr)
{
	int i;
	int bytes;
	struct ether_header *eh = (struct ether_header *)buf;
	struct sockaddr_ll socket_address = { 0 };

	for (i = 0; i < ETH_ALEN ; i++) {
		intr->mac.ether_dhost[i] = 0xFF;
	}
	intr->mac.ether_type = htons(ETHERCAT_TYPE);
	socket_address.sll_family = PF_PACKET;
	socket_address.sll_protocol = ETHERCAT_TYPE;
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
		       buf,
		       size, 0,
		       (struct sockaddr *)&socket_address,
		       (socklen_t) sizeof(socket_address));

	if (bytes < 0) {
		perror("tx packet: ");
	}
}

/* we may catch out own transmitted packet */
static int is_outgoing_pkt(e_slave *ecs, uint8_t *d)
{
	if (!memcmp(__ec_get_shost(d),
		ecs->intr[RX_INT_INDEX]->mac.ether_shost,
		ETH_ALEN)) {
       		return 1;
	}
	if (!memcmp(__ec_get_shost(d),
		ecs->intr[TX_INT_INDEX]->mac.ether_shost,
		ETH_ALEN)) {
       		return 1;
	}
	return 0;
}

static void ec_pkt_filter(u_char *user, const struct pcap_pkthdr *h,
                                   const u_char *bytes)
{
	e_slave *ecs = (e_slave *)user;
	uint8_t *d = (uint8_t *)bytes;

	if (!__ec_is_ethercat(d)){
		return;
	}
	if (is_outgoing_pkt(ecs, d)){
		return;
	}
	ec_process_datagrams(ecs, h->len, d);
}

/* handler of traffic that is passing by back to master */
void passing_pkt(u_char *user, const struct pcap_pkthdr *h,
                                   const u_char *bytes)
{
	e_slave *ecs = (e_slave *)user;
	uint8_t *d = (uint8_t *)bytes;

	if (!__ec_is_ethercat(d)){
		return;
	}
	if (is_outgoing_pkt(ecs, d)){
		return;
	}
	ec_tx_pkt(d ,h->len , ecs->intr[RX_INT_INDEX]);
}

void *pkt_passing_thread(void *ecs)
{
	int num_pkt = 0;
	while(1) {
		pcap_loop(tx_handle, num_pkt, passing_pkt, (u_char *)ecs);	
	}
}

int ec_capture(e_slave *ecs)
{
	pthread_t t;
 	char errbuf[PCAP_ERRBUF_SIZE];          /* error buffer */
	int snap_len = 1492;	
	int promisc = 1;
	int timeout_ms = 1000;

	rx_handle = pcap_open_live(ecs->intr[RX_INT_INDEX]->name,
			snap_len, promisc, timeout_ms, errbuf);
	if(!rx_handle){
		puts(errbuf);
		return -1;
	}
	if (ecs->interfaces_nr > 1) {
		tx_handle = pcap_open_live(ecs->intr[TX_INT_INDEX]->name,
			snap_len, promisc, timeout_ms, errbuf);
		if(!tx_handle){
			puts(errbuf);
			return -1;
		}
		pthread_create(&t, NULL,
			pkt_passing_thread, ecs);
	}
	while(1) {
		int num_pkt = 0;
		pcap_loop(rx_handle, num_pkt, ec_pkt_filter ,(u_char *)ecs);
	}
}
