/*
 * ec_regs.h
 *
 *  Created on: Oct 17, 2012
 *      Author: root
 */

#ifndef EC_REGS_H_
#define EC_REGS_H_

void ec_init_regs(e_slave *);
void ec_set_ado(int reg, long val);
void ec_get_ado(int reg, uint8_t * data);
void ec_raw_set_ado(int reg, uint8_t * data, int datalen);
void ec_raw_get_ado(int reg, uint8_t * data, int datalen);

int16_t ec_station_address(void);
#endif /* EC_REGS_H_ */
