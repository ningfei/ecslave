
#include <stdio.h>
#include <string.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

static uint8_t sii[60 << 10];

struct ec_cat_group {
	uint16_t type;
	uint16_t size;
};

struct ec_cat_group cat_groups[8];


int ec_sii_fetch(uint8_t *data, int datalen)
{
	int word_offset;
	struct ec_cat_group *group;

	printf("%s received datalen %d\n",
				__FUNCTION__, datalen);

	if (data[0] != 0x80) {
		printf("%s no two addressed octets %x %x\n",
				__FUNCTION__, data[0], data[1]);
		return 1;
	}
	if (data[1] != 0x01) {
		printf("request read operation\n", __FUNCTION__);
		return 1;
	}
	word_offset = *(uint16_t *) & data[2];
	printf("%s received word offset %d\n",
				__FUNCTION__, word_offset);
	memset(data ,0 , datalen);
	group = (struct ec_cat_group *)&data[6];
	group->type = 0x000A;
	group->size = 0xFFFF;
	return 0;
}
