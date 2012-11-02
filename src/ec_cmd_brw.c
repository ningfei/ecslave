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
	uint8_t *data = ec_dgram_data(slave->pkt);
	uint16_t datalen = ec_dgram_data_length(slave->pkt);


	ec_printf("%s index=%d\n",
	       __FUNCTION__, slave->pkt_index);

	ado = ec_dgram_ado(slave->pkt);
	__ec_inc_wkc(slave);
	if (ec_dgram_adp(slave->pkt) == 0) {
		/*
		 * if slave is addressed it is the only one who reads
		 * */
		ec_raw_get_ado(ado, data, datalen);
		printf("ADO R 0x%x Val=0x%x dlen=%d\n",
				ado, data[0], datalen);
		goto BRW_EXIT;
	}
	__ec_inc_wkc(slave);
	printf("ADO 0x%x Val=0x%x\n", ado, val);
	ec_raw_set_ado(ado, data, datalen);
BRW_EXIT:

	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
