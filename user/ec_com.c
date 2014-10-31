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

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>


ecat_slave slaves[MAX_SLAVES];
int slaves_nr = 0;
static mqd_t ecq;
static pcap_t *rx_handle = 0;

struct ecat_queue_msg {
	char buf[2000];
	int size;
	int slave_index;
};

int dbg_index(ecat_slave *ecs)
{
	uint8_t *p =   __ecat_frameheader(ecs->pkt_head) + sizeof(ec_frame_header);
	return __ec_dgram_pkt_index(p);
}

uint16_t ec_dbg_wkc(ecat_slave *ecs)
{
	uint8_t *p =   __ecat_frameheader(ecs->pkt_head) + sizeof(ec_frame_header);
	return __ec_wkc(p);
}

static int ec_queue_init(void)
{

        struct mq_attr attr={0};

        attr.mq_msgsize = sizeof(struct ecat_queue_msg);
        attr.mq_maxmsg = MAX_SLAVES;

        ecq = mq_open("/ecq", O_RDWR | O_CREAT,0700, &attr);
        if (ecq < 0) {
		perror("mq_open:");
		return -1;
        }
	return 0;
}

static int ec_queue_pkt(int slave_index, void *data,int sz)
{
	struct 	ecat_queue_msg  m;
	
	memcpy(m.buf, data, sz);
	m.slave_index = slave_index;
	m.size = sz;
	if ( mq_send(ecq, (char *)&m, sizeof(m), 0) < 0){
		perror("Failed to send message\n");
		return -1;
	}
	return 0;
}

//
//  we have the following flows:
//
// forward: slave 0 sends to first virtual slave : QUEUE
// backward: slave 0 sends to master	:ETHERNET
// forward: virt slave to virt slave   	:QUEUE
// backward: virt slave to virt slave 	:QUEUE
// backward: virt slave to slave 0	:QUEUE
void ec_tx_pkt(uint8_t* buf, int size, struct ec_device *intr)
{
	int i;
	int bytes;
	struct ether_header *eh = (struct ether_header *)buf;
	struct sockaddr_ll socket_address = { 0 };
	ecat_slave* ecs = intr->ecslave;

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
	if (ecs->index == (slaves_nr - 1)) {
		// 	transmit back the packet without 
		//	pushing it back 
		bytes = sendto(intr->sock,
			       buf, size, 0,
		       		(struct sockaddr *)&socket_address,
		       		(socklen_t) sizeof(socket_address));
		if (bytes < 0) {
			perror("tx packet: ");
		}
		return;
	}
	// any other slave would queue the packet to the next slave
	if ( ec_queue_pkt(ecs->index, buf ,size ) <  0){
		exit(0);
	}
}

/* we may catch out own transmitted packet */
static int is_outgoing_pkt(ecat_slave *ecs, uint8_t *d)
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
	ecat_slave *ecs = (ecat_slave *)user;
	uint8_t *d = (uint8_t *)bytes;
	struct ec_device *intr = ecs->intr[TX_INT_INDEX];
	struct ecat_event *ev;

	if (!__ec_is_ethercat(d)){
		return;
	}

	if (is_outgoing_pkt(ecs, d)){
		return;
	}

	pthread_mutex_lock(&intr->events_sync);
	while (LIST_FIRST(&intr->events) != NULL) {
	      	ev = LIST_FIRST(&intr->events);
		ev->action(ev->__private);
		LIST_REMOVE(ev, list);
		ev->action = 0x00;
	}
	pthread_mutex_unlock(&intr->events_sync);
	ec_process_datagrams(ecs, h->len, d);
}

int get_pkt(struct ecat_queue_msg* m)
{
	if (mq_receive(ecq, (char *)m, sizeof(*m) ,0)  < 0){
		perror("mq_read error:");
		return -1;
	}	
	return 0;
}

void *ec_local_slave(void *ecslaves)
{
	ecat_slave* ecs;
	struct ecat_queue_msg m;
	struct pcap_pkthdr h;

	while(1) {
		get_pkt(&m);
		ecs = &slaves[m.slave_index];
		h.len = m.size;
		ec_pkt_filter((u_char *)ecs, &h, (u_char *)&m.buf[0]);
	}
}

int ec_capture(void)
{
	ecat_slave *ecs;
	pthread_t t;
 	char errbuf[PCAP_ERRBUF_SIZE];          /* error buffer */
	int snap_len = 1492;	
	int promisc = 1;
	int timeout_ms = 1000;

	ecs = &slaves[0];
	rx_handle = pcap_open_live(ecs->intr[RX_INT_INDEX]->name,
			snap_len, promisc, timeout_ms, errbuf);

	if(!rx_handle){
		puts(errbuf);
		return -1;
	}


	if (ec_queue_init() < 0){
		printf("failed to initialize queue\n");
		return -1;
	}

	// works only with virtslaves. 
	pthread_create(&t, NULL, ec_local_slave, ecs);

	while (1) {
		// works only with the master, recv and transmit
		int num_pkt = 0;
		pcap_loop(rx_handle, num_pkt, ec_pkt_filter ,(u_char *)ecs);
	}
}
