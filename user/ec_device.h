#ifndef __EC_DEVICE_H__
#define __EC_DEVICE_H__

#include "ecat_timer.h"

struct ec_device {
	pthread_mutex_t events_sync;
        LIST_HEAD(porteve, ecat_event) events;
 	struct ecat_event rx_time;
	int index;
	struct ether_header mac;
	char name[16];
	char ip[32];
	char macaddr[32];
	int sock;
	int subnet_mask;
	int link_up;
};

void ec_init_device(struct ec_device *ec);
void ecat_add_event_to_device(struct ec_device *ec, 
		struct ecat_event* ev,
		void (*action)(void *), 
		void *private);
#endif
