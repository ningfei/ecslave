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
	uint16_t ado;
	uint8_t *datagram =
	    (uint8_t *) & slave->pkt[sizeof(struct ether_header)];
	uint16_t size = ec_dgram_size(slave->pkt);
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *data = (uint8_t *) (((uint8_t *) datagram) + sizeof(ec_comt));
	uint16_t *wkc = (uint16_t *) & datagram[size];

	wkc1 = *wkc;
	wkc1++;
	*wkc = wkc1;
	ado = ec_dgram_ado(slave->pkt);
	dprintf("%s index=%d wkc=%d wkc1=%d data len=%d ado=0x%x adp=0x%x\n",
	       __FUNCTION__,
	       slave->pkt_index, *wkc, wkc1, datalen, ado, ec_dgram_adp(slave->pkt));
	ec_raw_get_ado(ado, data, datalen);
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}

void ec_cmd_nop(e_slave * slave)
{
	__set_fsm_state(slave, ecs_rx_packet);
}
