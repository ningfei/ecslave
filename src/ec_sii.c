
#include <stdio.h>
#include <string.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

static uint8_t sii[60 << 10];

void ec_sii_fetch(e_slave * slave)
{
	uint16_t word_offset;
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *datagram = (uint8_t *) __ecat_frameheader(slave->pkt);
	uint16_t size = ec_dgram_size(slave->pkt);
	uint8_t *data = ec_dgram_data(slave->pkt);

	if (data[0] != 0x80) {
		printf("%s no two addressed octets\n", __FUNCTION__);
		return;
	}
	if (data[1] != 0x01) {
		printf("request read operation\n", __FUNCTION__);
		return;
	}
	word_offset = *(uint16_t *) & data[2];
	printf("recieved word offset %d\n", word_offset);
/*		struct ec_cat_group *gr;
		gr = (struct ec_category_group *)&data[6];
		gr->type = 0x000A;
		gr->size = 0xFFFF;
		return;
		*/
}
