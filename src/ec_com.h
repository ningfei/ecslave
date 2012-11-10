#ifndef __EC_COM_H__
#define __EC_COM_H__

#include "ethercattype.h"
#include "ecs_slave.h"

void raw_ecs_tx_packet(e_slave *ecs);
void raw_ecs_rx_packet(e_slave *ecs);

#endif
