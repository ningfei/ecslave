#include <stdio.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

void ec_cmd_brw(e_slave * slave)
{
	int val = 0;
	uint16_t ado;
	uint16_t wkc1;
	uint8_t *datagram =
	    (uint8_t *) & slave->pkt[sizeof(struct ether_header)];
	uint16_t size = ec_dgram_size(slave->pkt);
	uint8_t *data = (uint8_t *) (((uint8_t *) datagram) + sizeof(ec_comt));
	uint16_t *wkc = (uint16_t *) & datagram[size];
	uint16_t datalen = ec_dgram_data_length(slave->pkt);

	wkc1 = *wkc;
	wkc1++;
	printf("%s index=%d wkc=%d wkc1=%d\n",
	       __FUNCTION__, slave->pkt_index, *wkc, wkc1);

	ado = ec_dgram_ado(slave->pkt);
	if (ec_dgram_adp(slave->pkt) == 0) {
		/*
		 * if slave is adressed it is the only one who reads
		 * */
		ec_raw_get_ado(ado, data, datalen);
		printf("ADO R 0x%x Val=0x%x dlen=%d\n",
				ado, data[0], datalen);
		goto BRW_EXIT;
	}
	wkc1++;
	printf("ADO 0x%x Val=0x%x\n", ado, val);
	ec_raw_set_ado(ado, data, datalen);
BRW_EXIT:
	*wkc = wkc1;
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
