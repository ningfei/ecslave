/* * ec_cmd_apwr.c
 *
 *  Created on: Oct 18, 2012
 *      Author: root
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"

 /** Auto Increment Write. by ring position */
void ec_cmd_apwr(e_slave *ecs, uint8_t *dgram_ec)
{
	uint16_t adp;
	uint16_t ado;
	uint8_t *data = __ec_dgram_data(dgram_ec);

	adp = __ec_dgram_adp(dgram_ec);
	printf("%s ADP = %d\n", __FUNCTION__, adp);
	ado = __ec_dgram_ado(dgram_ec);
	( (ec_dgram *) dgram_ec)->adp++;	/* each slave ++ in APWR */
	__ec_inc_wkc__(dgram_ec);
	// should check if i am addressed
	{
		uint16_t datalen = __ec_dgram_dlength(dgram_ec);

		uint8_t val[datalen];

		ec_raw_get_ado(ecs, ado, &val[0], datalen);
		ec_raw_set_ado(ecs, ado, data, datalen);
		memcpy(data, &val, datalen);
		ec_printf("%s ADO 0x%x WRITE 0x %x %x\n",
		       __FUNCTION__,
		       ado,
		       val[0], val[1]);
	}
	__set_fsm_state(ecs, ecs_process_next_dgram);
}
