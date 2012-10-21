/*
 * fsm_slave.h
 *
 *  Created on: Oct 14, 2012
 *      Author: root
 */
#ifndef __FSM_SLAVE_H__
#define __FSM_SLAVE_H__

#include "ecs_slave.h"

struct fsm_slave {
	void (*state)(e_slave *);
};


static inline void __set_fsm_state(e_slave *slave,
		void (*state)(e_slave *))
{
	slave->fsm->state = state;
}

void  ecs_rx_packet(e_slave *);
void  ecs_tx_packet(e_slave *);
void  ecs_process_packet(e_slave *);

#endif

