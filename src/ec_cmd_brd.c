#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ec_regs.h"

void ec_cmd_brd(e_slave *ecs, uint8_t *dgram_ec)
{
	uint16_t ado;
	uint16_t datalen = __ec_dgram_dlength(dgram_ec);
	uint8_t *data    = __ec_dgram_data(dgram_ec);

	__ec_inc_wkc__(dgram_ec);
	ado = __ec_dgram_ado(dgram_ec);
	ec_printf("%s data len=%d ado=0x%x adp=0x%x\n",
	       __FUNCTION__,
	       datalen, ado, __ec_dgram_adp(dgram_ec));
	ec_raw_get_ado(ecs, ado, data, datalen);
	__set_fsm_state(ecs, ecs_process_next_dgram);

}

void ec_cmd_nop(e_slave * ecs,uint8_t* dgram_ec)
{
	__set_fsm_state(ecs, ecs_process_next_dgram);
}
