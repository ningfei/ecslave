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
#include "ec_sii.h"

/** Configured Address Read */
void ec_cmd_fprd(e_slave * slave)
{
	uint16_t ado, adp;
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *data = ec_dgram_data(slave->pkt);

	ado = ec_dgram_ado(slave->pkt);
	adp = ec_dgram_adp(slave->pkt);
	if (adp != ec_station_address()) {
		printf("%s not me adp=%x,%x \n",
			__FUNCTION__,
			adp,
			ec_station_address());
		goto FPRD_OUT;
	}
	__ec_inc_wkc(slave);
	ec_printf("%s index=%d ADO=0x%x data len=%d\n",
	       __FUNCTION__, slave->pkt_index,  ado, datalen);

	if (datalen == 0) {
		printf("insane no length\n");
		goto FPRD_OUT;
	}
	ec_raw_get_ado(ado, data, datalen);
	if (adp == ec_station_address()) {
		if (ado == ECT_REG_EEPSTAT){
			ec_sii_rw(data, datalen);
		}
	}
FPRD_OUT:
	ecs_tx_packet(slave);
	__set_fsm_state(slave, ecs_rx_packet);
}
