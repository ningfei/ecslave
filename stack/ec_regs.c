#include "xgeneral.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_net.h"
#include "ec_sii.h"
#include "ecat_timer.h"
#include "ec_device.h"
#include "ec_regs.h"

#define DC_32	0b0100
#define DC_SIZE	4


struct ecat_regs {
	uint8_t base;
	uint8_t revision;
	uint8_t portdes;
	uint16_t alstat;
	uint8_t dlstat;
	uint8_t station_address;
	uint8_t alias;
	uint8_t dlctl;
	uint8_t dlport;
	uint8_t dlalias;
	uint8_t alctl;
	uint8_t alstacode;
	uint8_t pdictl;
	uint8_t itqmask;
	uint8_t rxerr;
	uint8_t eepcfg;
	uint8_t eepctl;
	uint8_t eepaddr;
	uint8_t eepdat;
	uint8_t cycle_unit_ctrl;
	uint8_t assign_active;
	uint32_t propagation_delay;
	uint32_t offset_from_systemtime;
	uint32_t drift;
	uint32_t sync0_start;
	uint32_t sync1_start;
	uint32_t cycle_ns;
	uint32_t dcoffset;
	uint32_t rxtime_port[4];
};

struct ecat_regs registers ={0};

int ec_init_regs(ecat_slave* esv)
{
	int i = 0;
	uint16_t dl = 0;

	registers.alstat = EC_STATE_PRE_OP;

	for (i = 0 ; i < EC_MAX_PORTS ; i++){
		if ( i < esv->interfaces_nr ){
			if (ec_is_nic_link_up(esv, esv->intr[i]))
				dl |= (1 << (4 + i));
			if (ec_is_nic_loop_closed(esv))
				dl |=	(1 << (8 + i * 2));
			if (ec_is_nic_signal_detected(esv, esv->intr[i]))
				dl |= (1 << (9 + i * 2));
			registers.portdes |= 0b00000011 << (2 * i);
			continue;
		}
		dl |= (1 << (8 + i * 2));
	}
	registers.dlstat = dl;
	return 0;
}

/*
  0x0981.0 Activate Cyclic Operation
  0x0981.1 Activate SYNC0
  0x0981.2 Activate SYNC1
*/
uint32_t ecat_cyclic_activation(void)
{
        return registers.assign_active;
}

uint32_t ecat_cylic_activation_sync0(void)
{
        return  0;
}

uint32_t ecat_systime_offset(void)
{
        return  0;
}

uint32_t ecat_system_time(void)
{
        return 0;
}

uint32_t ecat_cyclic_interval_ns(void)
{
        return 0;
}

uint16_t ec_station_address(void)
{
	return registers.station_address;
}

uint32_t ecat_get_dcstart(int port)
{
        return  registers.rxtime_port[port];
}
        
uint32_t ecat_propagation_delay(void)
{
	return registers.propagation_delay;
}

void 	 ecat_set_dcstart(int port, uint8_t* data, int datalen)
{
}

void ecat_process_write_ados(int reg,uint8_t *data, int len)
{
	int i = 0;
	int reg1 = reg;
	int reg2 = reg + len;
	int reg_size = 1;

	while(reg1 < reg2){
	  switch(reg1)
	  {
   		case ECT_REG_TYPE : 
				registers.base = data[i];
			break;

 		case ECT_BASE_REVISION:
				registers.revision = data[i];
			break;

    	   	case ECT_BASE_BUILD1:
			break;

	    	case ECT_BASE_BUILD2:
			break;

		case ECT_BASE_FMMUS :
			break;

		case ECT_BASE_SYNCM :
			break;
	
    		case ECT_REG_PORTDES:
				registers.portdes = data[i];
			break;

    		case ECT_REG_ESCSUP: 
			break;

    		default:
			break;

    		case ECT_REG_STADR:
				registers.station_address = data[i];
			break;

	    	case ECT_REG_ALIAS:
				registers.alias = data[i];
			break;

   	 	case ECT_REG_DLCTL://       = 0x0100,
				registers.dlctl = data[i];
			break;

    		case ECT_REG_DLPORT: //      = 0x0101,
				registers.dlport = data[i];
			break;		

    		case ECT_REG_DLALIAS:  //   = 0x0103,
				registers.dlalias = data[i];
			break;		
			
    		case ECT_REG_DLSTAT://      = 0x0110,
				registers.dlstat = data[i];
			break;		

    		case ECT_REG_ALCTL://       = 0x0120,
				registers.alctl = data[i];
			break;		

  	  	case ECT_REG_ALSTAT://      = 0x0130,
				memcpy(&registers.alstat, &data[i],
					sizeof(registers.alstat));
				reg_size = sizeof(registers.alstat);
			break;

    		case ECT_REG_ALSTATCODE://	= 0x0134,
				registers.alstacode = data[i];
			break;		

    		case ECT_REG_PDICTL://      = 0x0140,
				registers.pdictl = data[i];
			break;

	 	case ECT_REG_IRQMASK://     = 0x0200,
				registers.itqmask = data[i] ;
			break;		

    		case ECT_REG_RXERR://       = 0x0300,
				registers.rxerr = data[i];
			break;		

    		case ECT_REG_EEPCFG://	= 0x0500,
				registers.eepcfg = data[i] ;
			break;		

    		case ECT_REG_EEPCTL://      = 0x0502,ECT_REG_EEPSTAT
				registers.eepctl = data[i];
			break;
			
    		case ECT_REG_EEPADR ://     = 0x0504,
				registers.eepaddr = data[i] ;
			break;
				
  		case ECT_REG_EEPDAT ://     = 0x0508,
				registers.eepdat = data[i];
			break;
  
		case ECT_REG_SM0STAT:    // = ECT_REG_SM0 + 0x05,
		case ECT_REG_SM1STAT:    // = ECT_REG_SM1 + 0x05,
		case ECT_REG_SM1ACT:     // = ECT_REG_SM1 + 0x06,
		case ECT_REG_SM1CONTR:   // = ECT_REG_SM1 + 0x07,
			break;
	
    		case ECT_REG_RX_TIME_PORT0:// = 0x0900, /* receive time of packet on port 0 */
    		case ECT_REG_RX_TIME_PORT1:// = 0x0904, /* receive time of packet on port 1*/
  	 	case ECT_REG_RX_TIME_PORT2:// = 0x0908,
    		case ECT_REG_RX_TIME_PORT3:// = 0x090C,
			reg_size = DC_SIZE;
			break;

		case ECT_REG_DCSYSTIME:
			reg_size = DC_SIZE;
			break;
    		
    		case ECT_REG_DCSOF://       = 0x0918, 
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCSYSOFFSET:// = 0x0920, /* offset from 910 */
			memcpy(&registers.offset_from_systemtime, &data[i],
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

   		case ECT_REG_DCSYSDELAY :// = 0x0928,
			memcpy(&registers.propagation_delay, &data[i],
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCSYSDRIFT://   = 0x092C, /* drift register */
			memcpy(&registers.drift, &data[i],
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCSPEEDCNT :// = 0x0930,
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCTIMEFILT://  = 0x0934,
			reg_size = DC_SIZE;
			break;
 
   		case ECT_REG_DCCUC ://      = 0x0980, /* cycle unit control -> */
			registers.cycle_unit_ctrl = data[i];
			break;

    		case ECT_REG_DCSYNCACT://  = 0x0981, /* ActiveActive */
			registers.assign_active = data[i];
			break;

    		case ECT_REG_SYNC0START :// = 0x0990, /* sync0 start time */
			reg_size = DC_SIZE;
			break;

		case ECT_REG_SYNC0CYCLE://  = 0x09A0, /* sync0 cycle time */
			reg_size = DC_SIZE;
			break;
   	
		case ECT_REG_DCCYCLE1://    = 0x09A4
			reg_size = DC_SIZE;
			break;

	   }
	  reg1 += reg_size;
	  i+= reg_size;
	  reg_size = 1;
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
	ecat_process_write_ados(reg, data,  datalen);
	if (reg == ECT_REG_ALCTL){
		memcpy(&registers.alstat, data,
			sizeof(registers.alstat));
        }
}

void ecat_process_read_ados(int reg,uint8_t *data, int len)
{
	int i = 0;
	uint32_t t;
	int reg1 = reg;
	int reg2 = reg + len;
	int reg_size = 1;

	while(reg1 < reg2){
	  switch(reg1)
	  {
   		case ECT_REG_TYPE : 
				data[i] = registers.base;
			break;

 		case ECT_BASE_REVISION:
				data[i] =  registers.revision;
			break;

    	   	case ECT_BASE_BUILD1:
				data[i] =  0x11;
			break;

	    	case ECT_BASE_BUILD2:
				data[i] =  0x22;
			break;

		case ECT_BASE_FMMUS :
				data[i] =  0x2;
			break;

		case ECT_BASE_SYNCM :
				data[i] =  0x2;
			break;
	
    		case ECT_REG_PORTDES:
				data[i] = registers.portdes;
			break;

    		case ECT_REG_ESCSUP: 
				data[i] = 0b0001 | DC_32;
			break;

    		default:
				//data[i] = 0;
			break;

    		case ECT_REG_STADR:
				data[i] = registers.station_address;
			break;

	    	case ECT_REG_ALIAS:
				data[i] = registers.alias;
			break;

   	 	case ECT_REG_DLCTL://       = 0x0100,
				data[i] = registers.dlctl;
			break;

    		case ECT_REG_DLPORT: //      = 0x0101,
				data[i] = registers.dlport;
			break;		

    		case ECT_REG_DLALIAS:  //   = 0x0103,
				data[i] = registers.dlalias;
			break;		
			
    		case ECT_REG_DLSTAT://      = 0x0110,
				data[i] = registers.dlstat;
			break;		

    		case ECT_REG_ALCTL://       = 0x0120,
				data[i] = registers.alctl;
			break;		

  	  	case ECT_REG_ALSTAT://      = 0x0130,
				memcpy(&data[i], &registers.alstat,
					sizeof(registers.alstat));
				reg_size = sizeof(registers.alstat);
			break;		

    		case ECT_REG_ALSTATCODE://	= 0x0134,
				data[i] = registers.alstacode;
			break;		

    		case ECT_REG_PDICTL://      = 0x0140,
				data[i] = registers.pdictl;
			break;

	 	case ECT_REG_IRQMASK://     = 0x0200,
				data[i] = registers.itqmask;
			break;		

    		case ECT_REG_RXERR://       = 0x0300,
				data[i] = registers.rxerr;
			break;		

    		case ECT_REG_EEPCFG://	= 0x0500,
				data[i] = registers.eepcfg;
			break;		

    		case ECT_REG_EEPCTL://      = 0x0502,ECT_REG_EEPSTAT
				data[i]  = registers.eepctl;
			break;
			
    		case ECT_REG_EEPADR ://     = 0x0504,
				data[i]  = registers.eepaddr;
			break;
				
  		case ECT_REG_EEPDAT ://     = 0x0508,
				data[i]  = registers.eepdat;
			break;
  
		case ECT_REG_SM0STAT:    // = ECT_REG_SM0 + 0x05,
		case ECT_REG_SM1STAT:    // = ECT_REG_SM1 + 0x05,
		case ECT_REG_SM1ACT:     // = ECT_REG_SM1 + 0x06,
		case ECT_REG_SM1CONTR:   // = ECT_REG_SM1 + 0x07,
				data[i]  = 0;
			break;
	
    		case ECT_REG_RX_TIME_PORT0:// = 0x0900, /* receive time of packet on port 0 */
    		case ECT_REG_RX_TIME_PORT1:// = 0x0904, /* receive time of packet on port 1*/
  	 	case ECT_REG_RX_TIME_PORT2:// = 0x0908,
    		case ECT_REG_RX_TIME_PORT3:// = 0x090C,
				{
				int port = (reg1 - ECT_REG_RX_TIME_PORT0)/4;
				memcpy(&data[i], &registers.rxtime_port[port], 
					DC_SIZE);
				reg_size = DC_SIZE;
				}
			break;

		case ECT_REG_DCSYSTIME:
			t = ecat_local_time() + registers.offset_from_systemtime;
			memcpy(&data[i], &t, DC_SIZE);
			reg_size = DC_SIZE;
			break;
    		
    		case ECT_REG_DCSOF://       = 0x0918, 
			memcpy(&data[i], &registers.dcoffset, DC_SIZE);
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCSYSOFFSET:// = 0x0920, /* offset from 910 */
			memcpy(&data[i], &registers.offset_from_systemtime,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

   		case ECT_REG_DCSYSDELAY :// = 0x0928,
			memcpy(&data[i], &registers.propagation_delay,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCSYSDRIFT://   = 0x092C, /* drift register */
			memcpy(&data[i], &registers.drift,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCSPEEDCNT :// = 0x0930,
			memcpy(&data[i], &registers.drift,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

    		case ECT_REG_DCTIMEFILT://  = 0x0934,
			memset(&data[i], 0, DC_SIZE);
			reg_size = DC_SIZE;
			break;
 
   		case ECT_REG_DCCUC ://      = 0x0980, /* cycle unit control -> */
			data[i] = registers.cycle_unit_ctrl;
			reg_size = 1;
			break;

    		case ECT_REG_DCSYNCACT://  = 0x0981, /* ActiveActive */
			data[i] = registers.assign_active;
			reg_size = 1;
			break;

    		case ECT_REG_SYNC0START :// = 0x0990, /* sync0 start time */
			memcpy(&data[i], &registers.sync0_start,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

		case ECT_REG_SYNC0CYCLE://  = 0x09A0, /* sync0 cycle time */
			memcpy(&data[i], &registers.sync1_start,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;
   	
		case ECT_REG_DCCYCLE1://    = 0x09A4
			memcpy(&data[i], &registers.cycle_ns,
					DC_SIZE);
			reg_size = DC_SIZE;
			break;

	   }
	  reg1 += reg_size;
	  i+= reg_size;
	  reg_size = 1;
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
	ecat_process_read_ados(reg, data, datalen);
}
