
#include "xgeneral.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_net.h"
#include "ec_sii.h"
#include "ecat_timer.h"
#include "ec_device.h"
#include "ec_regs.h"
#include "ec_regs_pool.h"

#define DC_32	0b0100
#define DC_SIZE	4


int ec_init_regs(ecat_slave* esv)
{
	int i = 0;
	uint16_t dl = 0;

	ecat_set_reg(ECT_REG_ALSTAT, EC_STATE_PRE_OP);

	ecat_set_reg(ECT_REG_TYPE, 0) ;	/* base type 1byte fsm_slave_scan.c line 296 */
	ecat_set_reg(ECT_BASE_REVISION , 0x11);	/* base revision. 1byte */
	ecat_set_reg(ECT_BASE_BUILD1 , 0x01);	/* base build .2bytes   */
	ecat_set_reg(ECT_BASE_BUILD2 , 0x22);
	ecat_set_reg(ECT_BASE_FMMUS , 0x2);	/* base fmmu count 1byte */
	ecat_set_reg(ECT_BASE_SYNCM , 0x2);	/* base sync count 1byte */
	ecat_set_reg(ECT_REG_STADR , 0x00);

	/* octet 2 0b00000001   fmmu bit operation
	 *              0b00000100      dc
	 *              0b00001000  dc 64 bit
	 **/
	ecat_set_reg(ECT_REG_ESCSUP, 0b0001 | DC_32);
	/*
	 * link state and port configuration
	*/
	ecat_set_reg(ECT_REG_PORTDES, 0);

	for (i = 0 ; i < EC_MAX_PORTS ; i++){
		if ( i < esv->interfaces_nr ){
			if (ec_is_nic_link_up(esv, esv->intr[i]))
				dl |= (1 << (4 + i));
			if (ec_is_nic_loop_closed(esv))
				dl |=	(1 << (8 + i * 2));
			if (ec_is_nic_signal_detected(esv, esv->intr[i]))
				dl |= (1 << (9 + i * 2));
			ecat_set_reg(ECT_REG_PORTDES, 0b00000011 << (2 * i));
			continue;
		} 
		dl |= (1 << (8 + i * 2));
	}
	copy_to_reg(ECT_REG_DLSTAT, (uint8_t*)&dl, sizeof(dl));	
	return 0;
}

/*
  0x0981.0 Activate Cyclic Operation
  0x0981.1 Activate SYNC0
  0x0981.2 Activate SYNC1
*/
uint32_t ecat_cyclic_activation(void)
{
	return (*ecat_reg(ECT_REG_DCSYNCACT)) & 0b001;
}

uint32_t ecat_cylic_activation_sync0(void)
{
	return (*ecat_reg(ECT_REG_DCSYNCACT)) & 0b010;
}

uint32_t ecat_systime_offset(void)
{
	int *p = (int *)ecat_reg(ECT_REG_DCSYSOFFSET);
	return *(uint32_t *)p;
}

uint32_t ecat_system_time(void)
{
	int *p = (int *)ecat_reg(ECT_REG_DCSYSTIME);
	return *(uint32_t *)p;
}
 
uint32_t ecat_cyclic_interval_ns(void)
{
	int *p = (int *)ecat_reg(ECT_REG_SYNC0CYCLE);
	return *(uint32_t *)p;
}

uint32_t ecat_get_dcstart(int port)
{
	int *p = (int *)ecat_reg(ECT_REG_SYNC0START + port);
	return *(uint32_t *)p;
}

void ecat_set_dcstart(int port, uint8_t* data, int datalen)
{
	copy_to_reg(ECT_REG_SYNC0START + port, data, datalen);
}

uint32_t ecat_recieve_time(int port)
{
	uint8_t *p = ecat_reg(ECT_REG_RX_TIME_PORT0 + port * 4);
	return *(uint32_t *)p;
}

int16_t ec_station_address(void)
{
	return *(int16_t *)ecat_reg(ECT_REG_STADR);
}

uint32_t ecat_propagation_delay(void)
{
	return *(uint32_t *)ecat_reg(ECT_REG_DCSYSDELAY);
}

void ecat_process_write_ados(ecat_slave* ecs, int reg1,int reg2,uint8_t *data)
{
	struct ec_device *intr;
	void *p;

	while (reg1 < reg2) {
		switch(reg1)
		{
		case ECT_REG_DCSYSTIME:
			ecat_calibrate_localtime((uint32_t *)data);
			break;

		case ECT_REG_ALCTL:
			copy_to_reg(ECT_REG_ALSTAT, data, 4);
			break;

		case  ECT_REG_DCCUC: /* cycle unit control */
			break;

		case ECT_REG_DCSYSDELAY:
			break;

		case ECT_REG_DCSYSOFFSET:
			break;

		case ECT_REG_SYNC0START:
			/* set starting time */
			ecat_set_dcstart(0, data, reg2 - reg1);
			ecat_wake_timer();
			break;

		case ECT_REG_RX_TIME_PORT0:
		/* 
		 * save local recieve time and schedule 
		 * event to save the receive time of the
		 *  returning packet
		*/
			ecat_set_rx_time(ecat_reg(ECT_REG_RX_TIME_PORT0));
		case ECT_REG_RX_TIME_PORT1:
			intr = ecs->intr[TX_INT_INDEX];
			p  = ecat_reg(ECT_REG_RX_TIME_PORT1);
			ecat_add_event_to_device(intr,
				&intr->rx_time,
				ecat_set_rx_time,
				p);
			break;
		}
		reg1 += 4;
	}
}

void ec_set_ado(ecat_slave *ecs, int reg, uint8_t * data, int datalen)
{

	if (reg > ECT_REG_DCCYCLE1) {
		return ec_mbox(ecs, reg, data, datalen);
	}
	if (reg < ECT_REG_TYPE) {
		return;
	}
	ecat_process_write_ados(ecs, reg, reg + datalen, data);
	copy_to_reg(reg, data,  datalen);
}

void ecat_process_read_ados(int reg1,int reg2)
{
	uint32_t t;

	while(reg1 < reg2){
	  switch(reg1)
	  {
		case ECT_REG_DCSYSTIME:
			t = ecat_local_time() + ecat_systime_offset();
			copy_to_reg(0x910, (uint8_t *)&t, sizeof(t));
		break;
	   }
	  reg1 += 4;
	}
}

void ec_get_ado(ecat_slave *ecs, int reg, uint8_t * data, int datalen)
{
	if (reg > ECT_REG_DCCYCLE1) {
		return ec_mbox(ecs, reg, data, datalen);
	}
	if (reg < ECT_REG_TYPE) {
		return;
	}
	if (reg >= ECT_REG_SM0 && reg <= ECT_REG_SM3) {
		/* ethelab expects here a mail box */
		return	ec_sii_syncm(reg,  data, datalen);
	}
	ecat_process_read_ados(reg,reg + datalen);
	copy_from_reg(data, reg, datalen);
}

