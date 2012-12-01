#include "../include/xgeneral.h"
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include "ec_sii.h"

/** Configured Address Read */
void ec_cmd_fprd(ecat_slave *ecs,uint8_t *dgram_ec)
{
	uint16_t ado, adp;
	uint16_t datalen = __ec_dgram_dlength(dgram_ec);
	uint8_t *data = __ec_dgram_data(dgram_ec);

	ado = __ec_dgram_ado(dgram_ec);
	adp = __ec_dgram_adp(dgram_ec);
	if (adp != ec_station_address()) {
		ec_printf("%s Index=%x not me adp=%x,%x \n",
			__FUNCTION__,
			__ec_dgram_pkt_index(dgram_ec),
			adp,
			ec_station_address());
		goto FPRD_OUT;
	}
	ec_printf("%s ado=0x%x data len=%d\n",
	       __FUNCTION__,ado, datalen);

	if (datalen == 0) {
		ec_printf("insane no length\n");
		goto FPRD_OUT;
	}
	__ec_inc_wkc__(dgram_ec);
	ec_get_ado(ecs, ado, data, datalen);
	if (ado == ECT_REG_EEPSTAT){
		ec_sii_rw(data, datalen);
	}
FPRD_OUT:
        __set_fsm_state(ecs, ecs_process_next_dgram);
}
