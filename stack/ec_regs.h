/*
 * ec_regs.h
 *
 *  Created on: Oct 17, 2012
 *      Author: raz ben yehuda
 */

#ifndef EC_REGS_H_
#define EC_REGS_H_

#ifdef __cplusplus
extern "C" {
#endif

int  	 ec_init_regs(ecat_slave *);
void 	 ec_set_ado(ecat_slave *,int reg, uint8_t * data, int datalen);
void 	 ec_get_ado(ecat_slave*,int reg, uint8_t * data, int datalen);
uint32_t ecat_cyclic_interval_ns(void);
int16_t  ec_station_address(void);
uint32_t ecat_cyclic_activation(void);
uint32_t ecat_cylic_activation_sync0(void);
uint32_t ecat_systime_offset(void);
uint32_t ecat_system_time(void);
uint32_t ecat_cyclic_interval_ns(void);
uint32_t ecat_get_dcstart(int port);
void 	 ecat_set_dcstart(int port, uint8_t* data, int datalen);
uint32_t ecat_recieve_time(int port);
int16_t  ec_station_address(void);
uint32_t ecat_propagation_delay(void);

#ifdef __cplusplus
}
#endif

#endif /* EC_REGS_H_ */
