#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/ethernet.h>
#include <time.h>

#include "ethercattype.h"

#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
#define CLOCK_TO_USE CLOCK_REALTIME
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

static uint8_t ec_registers[ECT_REG_DCCYCLE1] = { 0 };

void ec_init_regs(void)
{
	ec_registers[ECT_REG_ALSTAT] = EC_STATE_PRE_OP;

	ec_registers[ECT_REG_TYPE] = 0;	/* base type 1byte fsm_slave_scan.c line 296 */
	ec_registers[ECT_BASE_REVISION] = 0x11;	/* base revision. 1byte */
	ec_registers[ECT_BASE_BUILD1] = 0x01;	/* base build .2bytes   */
	ec_registers[ECT_BASE_BUILD2] = 0x22;
	ec_registers[ECT_BASE_FMMUS] = 0x0;	/* base fmmu count 1byte */
	ec_registers[ECT_BASE_SYNCM] = 0x0;	/* base sync count 1byte */
	ec_registers[ECT_REG_PORTDES] = 0x000F;	/* octet 1      0x000F one port. mii */
	ec_registers[ECT_REG_ESCSUP] = 0b00000001 | 0b00000100;
	/* octet 2 0b00000001   fmmu bit operation
	 *              0b00000100      dc
	 *              0b00001000  dc 64 bit
	 **/
	ec_registers[ECT_REG_STADR] = 0x00;
	ec_registers[ECT_REG_DLSTAT] = 0b00001000;	/* data link state */
	ec_registers[ECT_REG_DLSTAT + 1] = 0x00;	/* data link state */
}

int16_t ec_station_address(void)
{
	return ec_registers[ECT_REG_STADR];
}

int ec_raw_set_ado(int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1 || reg < ECT_REG_TYPE) {
		printf("%s insane ado\n",__FUNCTION__);
		return 0;
	}
	memcpy(&ec_registers[reg], data, datalen);
	if (reg == ECT_REG_ALCTL){
		memcpy(&ec_registers[ECT_REG_ALSTAT], data, datalen);
	}
	return 0;
}

void ec_raw_get_ado(int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1 || reg < ECT_REG_TYPE) {
		printf("%s insane ado\n",__FUNCTION__);
		return;
	}
	if (reg == ECT_REG_DCSYSTIME){

		struct timespec tm;
	    uint64_t t;
	    uint8_t *p = (uint8_t *)&t;

	    clock_gettime(CLOCK_TO_USE, &tm);
	    t = TIMESPEC2NS(tm);
	    if (datalen == 4)
	    	p += 4;
	    memcpy(data, p, datalen);
	    return;
	  }
	memcpy(data, &ec_registers[reg], datalen);
}
