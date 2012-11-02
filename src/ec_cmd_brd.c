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
	uint16_t ado;
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *data = ec_dgram_data(slave->pkt);

	__ec_inc_wkc(slave);
	ado = ec_dgram_ado(slave->pkt);
	ec_printf("%s index=%d data len=%d ado=0x%x adp=0x%x\n",
	       __FUNCTION__,
	       slave->pkt_index, datalen, ado, ec_dgram_adp(slave->pkt));
	ec_raw_get_ado(ado, data, datalen);
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}

void ec_cmd_nop(e_slave * slave)
{
	__set_fsm_state(slave, ecs_rx_packet);
}
