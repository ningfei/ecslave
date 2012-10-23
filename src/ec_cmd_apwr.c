/*
 * ec_cmd_apwr.c
 *
 *  Created on: Oct 18, 2012
 *      Author: root
 */

#include <stdio.h>
#include <string.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

 /** Auto Increment Write */
void ec_cmd_apwr(e_slave * slave)
{
	uint16_t wkc1;
	uint16_t adp;
	uint16_t ado;
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *datagram = (uint8_t *) __ecat_frameheader(slave->pkt);
	uint16_t size = ec_dgram_size(slave->pkt);
	uint8_t *data = (uint8_t *) (((uint8_t *) datagram) + sizeof(ec_comt));
	uint16_t *wkc = (uint16_t *) & datagram[size];

	adp = ec_dgram_adp(slave->pkt);
	printf("%s ADP = %d\n", __FUNCTION__, adp);

	wkc1 = *wkc;
	wkc1++;

	printf("%s index=0x%x wkc=%d wkc1=%d\n",
	       __FUNCTION__, slave->pkt_index, *wkc, wkc1);

	ado = ec_dgram_ado(slave->pkt);
	if (adp != ec_station_address()) {
		goto APWR_OUT;
	}

	((ec_comt *) datagram)->ADP++;	/* each slave ++ in APWR */
	{
		uint8_t val[datalen];

		ec_raw_get_ado(ado, &val[0], datalen);
		ec_raw_set_ado(ado, data, datalen);
		memcpy(data, &val, datalen);
		printf("%s index=0x%x ADO 0x%x WRITE 0x %x %x\n",
		       __FUNCTION__,
		       slave->pkt_index,
		       ado,
		       val[0], val[1]);
	}
APWR_OUT:

	*wkc = wkc1;
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
