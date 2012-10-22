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
	uint8_t *datagram = (uint8_t *) __ecat_header(slave->pkt);
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
	if (adp != ec_station_address() && ec_station_address() != 0xFF ) {
		goto APWR_OUT;
	}
	((ec_comt *) datagram)->ADP++;	/* each slave ++ in APWR */
	if (ado == ECT_REG_STADR){
	   }else{
		uint8_t val[datalen];
		ec_raw_get_ado(ado, &val[0], datalen);
		ec_raw_set_ado(ado, data, datalen);
		memcpy(data, &val, datalen);
		
	}
	printf("%s index=%d ADO WRITE 0x%x\n",
	       __FUNCTION__, slave->pkt_index, data[0]);
	ec_raw_set_ado(ado, data, datalen);
APWR_OUT:
	*wkc = wkc1;
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
