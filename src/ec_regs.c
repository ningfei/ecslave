#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/ethernet.h>
#include <time.h>

#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_net.h"
#include "ec_mbox.h"

#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
#define CLOCK_TO_USE CLOCK_REALTIME
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define SDOS_ADDR_SPACE	4096

static uint8_t ec_registers[ECT_REG_DCCYCLE1 + SDOS_ADDR_SPACE] = { 0 };

long sdo_high(void)
{
	return (long ) &ec_registers[ECT_REG_DCCYCLE1 + SDOS_ADDR_SPACE];
}

void ec_init_regs(e_slave* esv)
{
	int i = 0;
	uint16_t *dl;

	ec_registers[ECT_REG_ALSTAT] = EC_STATE_PRE_OP;

	ec_registers[ECT_REG_TYPE] = 0;	/* base type 1byte fsm_slave_scan.c line 296 */
	ec_registers[ECT_BASE_REVISION] = 0x11;	/* base revision. 1byte */
	ec_registers[ECT_BASE_BUILD1] = 0x01;	/* base build .2bytes   */
	ec_registers[ECT_BASE_BUILD2] = 0x22;
	ec_registers[ECT_BASE_FMMUS] = 0x0;	/* base fmmu count 1byte */
	ec_registers[ECT_BASE_SYNCM] = 0x0;	/* base sync count 1byte */
	ec_registers[ECT_REG_PORTDES] = 0x000F;	/* octet 1      0x000F one port. mii */
	ec_registers[ECT_REG_STADR] = 0x07;

	/* octet 2 0b00000001   fmmu bit operation
	 *              0b00000100      dc
	 *              0b00001000  dc 64 bit
	 **/
	ec_registers[ECT_REG_ESCSUP] = 0x0001 | 0x0004;
	/*
	 * link state
	*/
	dl = (uint16_t *)&ec_registers[ECT_REG_DLSTAT];
	for (i = 0 ; i < esv->interfaces_nr ; i++){
		if (ec_is_nic_link_up(esv, i))
			*dl |= (1 << (4 + i));
		if (ec_is_nic_loop_closed(esv, i))
			*dl |=	(1 << (8 + i * 2));
		if (ec_is_nic_signal_detected(esv, i))
			*dl |= (1 << (9 + i * 2));
	}
}

int16_t ec_station_address(void)
{
	return ec_registers[ECT_REG_STADR];
}

void ec_raw_set_ado(int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1) {
		return ec_mbox(reg, data, datalen);
	}
	if (reg < ECT_REG_TYPE) {
		printf("%s insane ado\n",__FUNCTION__);
		return;
	}
	memcpy(&ec_registers[reg], data, datalen);
	if (reg == ECT_REG_ALCTL){
		memcpy(&ec_registers[ECT_REG_ALSTAT], data, datalen);
	}
}

void ec_raw_get_ado(int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1) {
		return ec_mbox(reg, data, datalen);
	}
	if (reg < ECT_REG_TYPE) {
		printf("%s insane ado 0x%x\n",__FUNCTION__,reg);
		return;
	}
	if (reg == ECT_REG_DCSYSTIME){

	    struct timespec tm;
	    uint64_t t;
	    uint8_t* p = (uint8_t *)&t;
	   
	    clock_gettime(CLOCK_TO_USE, &tm);
	    t = TIMESPEC2NS(tm);
	    if (datalen == 4)
		p = (uint8_t*)&tm.tv_nsec;
	    memcpy(data, p, datalen);
	    return;
	  }
	memcpy(data, &ec_registers[reg], datalen);
}
