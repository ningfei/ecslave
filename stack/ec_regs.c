#include "xgeneral.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_net.h"
#include "ec_sii.h"
	#include "ecat_timer.h"

#define DC_32	0b0100
#define DC_SIZE	4
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

static uint8_t ec_registers[ECT_REG_DCCYCLE1 + SDOS_ADDR_SPACE] = { 0 };

void ec_init_regs(ecat_slave* esv)
{
	int i = 0;
	uint16_t *dl;

	ec_registers[ECT_REG_ALSTAT] = EC_STATE_PRE_OP;

	ec_registers[ECT_REG_TYPE] = 0;	/* base type 1byte fsm_slave_scan.c line 296 */
	ec_registers[ECT_BASE_REVISION] = 0x11;	/* base revision. 1byte */
	ec_registers[ECT_BASE_BUILD1] = 0x01;	/* base build .2bytes   */
	ec_registers[ECT_BASE_BUILD2] = 0x22;
	ec_registers[ECT_BASE_FMMUS] = 0x2;	/* base fmmu count 1byte */
	ec_registers[ECT_BASE_SYNCM] = 0x2;	/* base sync count 1byte */
	ec_registers[ECT_REG_PORTDES] = 0x000F;	/* octet 1      0x000F one port. mii */
	ec_registers[ECT_REG_STADR] = 0x00;

	/* octet 2 0b00000001   fmmu bit operation
	 *              0b00000100      dc
	 *              0b00001000  dc 64 bit
	 **/
	ec_registers[ECT_REG_ESCSUP] = 0b0001 | DC_32;
	/*
	 * link state
	*/
	dl = (uint16_t *)&ec_registers[ECT_REG_DLSTAT];
	for (i = 0 ; i < esv->interfaces_nr ; i++){
		if (ec_is_nic_link_up(esv, esv->intr[i]))
			*dl |= (1 << (4 + i));
		if (ec_is_nic_loop_closed(esv))
			*dl |=	(1 << (8 + i * 2));
		if (ec_is_nic_signal_detected(esv, esv->intr[i]))
			*dl |= (1 << (9 + i * 2));
	}
}

/*
  0x0981.0 Activate Cyclic Operation
  0x0981.1 Activate SYNC0
  0x0981.2 Activate SYNC1
*/
uint32_t ecat_cyclic_activation(void)
{
	return ec_registers[ECT_REG_DCSYNCACT] & 0b001;
}

uint32_t ecat_cylic_activation_sync0(void)
{
	return ec_registers[ECT_REG_DCSYNCACT] & 0b010;
}

uint64_t ecat_system_time_offset(void)
{
	uint64_t *p = (uint64_t *)&ec_registers[ECT_REG_DCSYSOFFSET];
	return *p;
}

uint32_t ecat_cyclic_interval_ns(void)
{
	uint32_t *p = (uint32_t *)&ec_registers[ECT_REG_DCCYCLE0];
	return *p;
}

uint64_t ecat_get_sync(int port)
{
	uint64_t *p = (uint64_t *)&ec_registers[ECT_REG_DCSTART0 + port];
	return *p;
}

void ecat_set_sync(int port, struct timespec *tm)
{
	memcpy(&ec_registers[ECT_REG_DCSTART0 + port],tm,sizeof(*tm));
}

uint32_t ecat_recieve_time(int port)
{
	uint32_t *p = (uint32_t *)&ec_registers[ECT_REG_DCTIME0 + port * 4];
	return *p;
}

int16_t ec_station_address(void)
{
	return ec_registers[ECT_REG_STADR];
}

/*
 * update sync0 time
*/
void sync0_event(void *priv __attribute__ ((unused)))
{
	struct timespec tm;
	clock_gettime(CLOCK_REALTIME, &tm);

	ecat_set_sync(0, &tm);
}

void ec_set_ado(ecat_slave *ecs, int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1) {
		return ec_mbox(ecs, reg, data, datalen);
	}
	if (reg < ECT_REG_TYPE) {
		ec_printf("%s insane ado\n",__FUNCTION__);
		return;
	}
	memcpy(&ec_registers[reg], data, datalen);

	switch(reg)
	{
	case ECT_REG_DCSYSTIME:
		break;

	case ECT_REG_ALCTL:
		memcpy(&ec_registers[ECT_REG_ALSTAT], data, datalen);
		break;

	case  ECT_REG_DCCUC:
		if (ecat_cyclic_activation()) {
			ecat_create_timer();
		}
		break;

	case ECT_REG_DCCYCLE0:
		ecat_wake_timer();
		break;

	case ECT_REG_DCTIME0: /* user steped on */
		ecat_schedule_event(ecs, &ecs->sync0, sync0_event);
		break;
	}
}

void ec_get_ado(ecat_slave *ecs, int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1) {
		return ec_mbox(ecs, reg, data, datalen);
	}
	if (reg < ECT_REG_TYPE) {
		ec_printf("%s insane ado 0x%x\n",__FUNCTION__,reg);
		return;
	}
	if (reg >= ECT_REG_SM0 && reg <= ECT_REG_SM3) {
		/* ethelan expects here a mail box*/
		return	ec_sii_syncm(reg,  data, datalen);
	}
	memcpy(data, &ec_registers[reg], datalen);
}
