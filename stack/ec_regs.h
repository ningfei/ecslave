/*
 * ec_regs.h
 *
 *  Created on: Oct 17, 2012
 *      Author: root
 */

#ifndef EC_REGS_H_
#define EC_REGS_H_

void ec_init_regs(ecat_slave *);
void ec_set_ado(ecat_slave *,int reg, uint8_t * data, int datalen);
void ec_get_ado(ecat_slave*,int reg, uint8_t * data, int datalen);
uint32_t ecat_cyclic_interval_ns(void);

//long sdo_high(void);
int16_t ec_station_address(void);
#endif /* EC_REGS_H_ */
