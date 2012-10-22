#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"

void ec_cmd_brd(e_slave * slave)
{
	uint16_t wkc1;
	uint32_t val = 0;
	uint8_t *datagram =
	    (uint8_t *) & slave->pkt[sizeof(struct ether_header)];
	uint16_t size = ec_dgram_size(slave->pkt);
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *data = (uint8_t *) (((uint8_t *) datagram) + sizeof(ec_comt));
	uint16_t *wkc = (uint16_t *) & datagram[size];

	wkc1 = *wkc;
	wkc1++;
	*wkc = wkc1;

	printf("%s index=%d wkc=%d wkc1=%d data len=%d ado=%d\n",
	       __FUNCTION__,
	       slave->pkt_index, *wkc, wkc1, datalen, ec_dgram_ado(slave->pkt));

	if (ec_dgram_data_length(slave->pkt)) {
		if (ec_dgram_ado(slave->pkt) <= ECT_REG_DCCYCLE1
		    && ec_dgram_ado(slave->pkt) >= ECT_REG_TYPE) {

			ec_get_ado(ec_dgram_ado(slave->pkt), &val);
			memcpy(data, &val, datalen);
		}
	}
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}

void ec_cmd_nop(e_slave * slave)
{
	__set_fsm_state(slave, ecs_rx_packet);
}
