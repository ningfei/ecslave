#include <stdio.h>
#include <string.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

uint8_t process_data[65536];

void set_data_lrw(uint8_t * data, uint16_t offset, uint16_t datalen)
{
	memcpy(&process_data[offset], data, datalen);
}

void get_data_lrw(uint8_t * data, uint16_t offset, uint16_t datalen)
{
	memcpy(data, &process_data[offset], datalen);
}

 /** Logical Read Write */
void ec_cmd_lrw(e_slave * slave)
{
	int val = 0;
	uint16_t wkc1;
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *datagram = (uint8_t *) __ecat_frameheader(slave->pkt);
	uint16_t size = ec_dgram_size(slave->pkt);
	uint8_t *data = ec_dgram_data(slave->pkt);
	uint16_t *wkc = (uint16_t *) & datagram[size];
	uint32_t offset = 0;

	wkc1 = *wkc;
	wkc1++;

	printf("%s index=%d wkc=%d wkc1=%d\n",
	       __FUNCTION__, slave->pkt_index, *wkc, wkc1);

LWR_OUT:
	*wkc = wkc1;
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
