#include <stdio.h>
#include <string.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

 /** Logical Read Write */
void ec_cmd_lrd(e_slave *ecs, uint8_t *dgram_ec)
{
/*
	int val = 0;
	uint16_t datalen = ec_dgram_data_length(slave->pkt);
	uint8_t *datagram = (uint8_t *) __ecat_frameheader(slave->pkt);
	uint16_t size = ec_dgram_size(slave->pkt);
	uint8_t *data = ec_dgram_data(slave->pkt);
	uint32_t offset = 0;
*/
	__ec_inc_wkc__(dgram_ec);
//	printf("%s index=%d\n",
//	       __FUNCTION__, ecs->pkt_index);

//LWR_OUT:
  	__set_fsm_state(ecs, ecs_process_next_dgram);
}
