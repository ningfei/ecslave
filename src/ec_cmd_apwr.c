/*
 * ec_cmd_apwr.c
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
void ec_cmd_apwr(e_slave *slave)
{
	uint16_t adp;
	uint16_t ado;
	uint8_t *datagram = (uint8_t *) __ecat_frameheader(slave->pkt);
	uint8_t *data = ec_dgram_data(slave->pkt);

	adp = ec_dgram_adp(slave->pkt);
	ec_printf("%s ADP = %d\n", __FUNCTION__, adp);
	ado = ec_dgram_ado(slave->pkt);
	((ec_comt *) datagram)->ADP++;	/* each slave ++ in APWR */
	__ec_inc_wkc(slave);
	{
		uint16_t datalen = ec_dgram_data_length(slave->pkt);

		uint8_t val[datalen];

		ec_raw_get_ado(ado, &val[0], datalen);
		ec_raw_set_ado(ado, data, datalen);
		memcpy(data, &val, datalen);
		ec_printf("%s index=0x%x ADO 0x%x WRITE 0x %x %x\n",
		       __FUNCTION__,
		       slave->pkt_index,
		       ado,
		       val[0], val[1]);
	}
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
