#ifndef __EC_DEVICE_H__
#define __EC_DEVICE_H__

struct ec_device {
	int index;
	struct ether_header mac;
	char name[16];
	char ip[32];
	char macaddr[32];
	int sock;
	int subnet_mask;
	int link_up;
};

#endif
