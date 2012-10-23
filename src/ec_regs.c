#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/ethernet.h>

#include "ethercattype.h"

static uint8_t ec_registers[ECT_REG_DCCYCLE1] = { 0 };

void ec_init_regs(void)
{
	ec_registers[ECT_REG_ALSTAT] = EC_STATE_PRE_OP;

	ec_registers[ECT_REG_TYPE] = 0;	/* base type 1byte fsm_slave_scan.c line 296 */
	ec_registers[ECT_BASE_REVISION] = 0x11;	/* base revision. 1byte */
	ec_registers[ECT_BASE_BUILD1] = 0x01;	/* base build .2bytes   */
	ec_registers[ECT_BASE_BUILD2] = 0x22;
	ec_registers[ECT_BASE_FMMUS] = 0x1;	/* base fmmu count 1byte */
	ec_registers[ECT_BASE_SYNCM] = 0x4;	/* base sync count 1byte */
	ec_registers[ECT_REG_PORTDES] = 0x000F;	/* octet 1      0x000F one port. mii */
	ec_registers[ECT_REG_ESCSUP] = 0b00000001 | 0b00000100;
	/* octet 2 0b00000001   fmmu bit operation
	 *              0b00000100      dc
	 *              0b00001000  dc 64 bit
	 **/
	ec_registers[ECT_REG_STADR] = 0x00; /* illegal station address */
	ec_registers[ECT_REG_DLSTAT] = 0b00001000;	/* data link state */
	ec_registers[ECT_REG_DLSTAT + 1] = 0x00;	/* data link state */
}

int16_t ec_station_address(void)
{
	return ec_registers[ECT_REG_STADR];
}

int ec_raw_set_ado(int reg, uint8_t * data, int datalen)
{
	int ret = 1;

	if (reg > ECT_REG_DCCYCLE1 || reg < ECT_REG_TYPE) {
		printf("%s insane ado\n",__FUNCTION__);
		return 0;
	}
	memcpy(&ec_registers[reg], data, datalen);
	return ret;
}

void ec_raw_get_ado(int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1 || reg < ECT_REG_TYPE) {
		printf("%s insane ado\n",__FUNCTION__);
		return;
	}
	memcpy(data, &ec_registers[reg], datalen);
}
