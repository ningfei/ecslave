#include <stdio.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"

void ec_cmd_brw(e_slave * ecs,uint8_t *dgram_ec)
{
	int val = 0;
	uint16_t ado;
	uint8_t *data = __ec_dgram_data(dgram_ec);
	uint16_t datalen = __ec_dgram_dlength(dgram_ec);

	ado = __ec_dgram_ado(dgram_ec);
	__ec_inc_wkc__(dgram_ec);
	if (__ec_dgram_adp(dgram_ec) == 0) {
		/*
		 * if ecs is addressed it is the only one who reads
		 * */
		ec_raw_get_ado(ecs, ado, data, datalen);
		ec_printf("ADO R 0x%x Val=0x%x dlen=%d\n",
				ado, data[0], datalen);
		goto BRW_EXIT;
	}
	__ec_inc_wkc__(dgram_ec);
	ec_printf("ADO 0x%x Val=0x%x\n", ado, val);
	ec_raw_set_ado(ecs, ado, data, datalen);
BRW_EXIT:
	__set_fsm_state(ecs, ecs_process_next_dgram);
}
