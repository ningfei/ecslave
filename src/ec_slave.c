

#include <stdint.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"

#define  WORKING_CNT_SIZE 2

void ecs_tx_packet(e_slave * ecs,uint8_t *d)
{
	raw_ecs_tx_packet(ecs);
	 __set_fsm_state(ecs, ecs_rx_packet);
}

void ecs_process_next_dgram(e_slave * ecs,uint8_t *d)
{
	if (--ecs->dgrams_cnt) {
		/* move to next packet */
		ecs->dgram_processed += WORKING_CNT_SIZE + sizeof(ec_dgram) + __ec_dgram_dlength(d);
		return __set_fsm_state(ecs, ecs_process_packet);
	}
	 __set_fsm_state(ecs, ecs_tx_packet);
}

int  ec_nr_dgrams(uint8_t *raw_pkt)
{
	int i = 0;
	int f;
	int frame_size = __ec_frame_size(raw_pkt);
	uint8_t* dgram  = __ecat_frameheader(raw_pkt) +  sizeof(ec_frame_header);
		
	f = frame_size;
	for (;frame_size > 0;i++){
		frame_size -= (sizeof(ec_dgram) + __ec_dgram_dlength(dgram) + WORKING_CNT_SIZE) ;
		dgram += sizeof(ec_dgram) + WORKING_CNT_SIZE + __ec_dgram_dlength(dgram); 		
	}
	if (frame_size < 0){
		printf("aieeee %d %d\n",frame_size,f);
	}
	return i;
}

void ecs_process_packet(e_slave * ecs, uint8_t *dgram_ec)
{
	__set_fsm_state(ecs, ec_cmd_nop);
	
	switch (__ec_dgram_command(dgram_ec)) {
	case EC_CMD_NOP:
		puts("Command NOP");
		break;

	case EC_CMD_APRD:
		puts("Auto Increment Read");
		break;

	case EC_CMD_APWR:
		__set_fsm_state(ecs, ec_cmd_apwr);
		break;

	case EC_CMD_APRW:
		puts("Auto Increment Read Write");
		break;

	case EC_CMD_FPRD:
		__set_fsm_state(ecs, ec_cmd_fprd);
		break;

	case EC_CMD_FPWR:
		__set_fsm_state(ecs, ec_cmd_fpwr);
		break;

	case EC_CMD_FPRW:
		puts("Configured Address Read Write");
		break;

	case EC_CMD_BRD:
		__set_fsm_state(ecs, ec_cmd_brd);
		break;

	case EC_CMD_BWR:
		__set_fsm_state(ecs, ec_cmd_brw);
		break;

	case EC_CMD_BRW:
		puts("Broadcast Read Write");
		break;

	case EC_CMD_LRD:
		__set_fsm_state(ecs, ec_cmd_lrd);
		break;

	case EC_CMD_LWR:
		puts("Logical Memory Write Read");
		break;

	case EC_CMD_LRW:
		__set_fsm_state(ecs, ec_cmd_lrw);
		break;

	case EC_CMD_ARMW:
		puts("Auto Increment Read Multiple Write");
		break;

	case EC_CMD_FRMW:
		puts("Configured Read Multiple Write");
		break;
	default:
		puts("unknown command");
	}
	ecs->fsm->state(ecs, dgram_ec);
}

void ecs_run(e_slave *ecs)
{
	while (1) {
		ecs->fsm->state(ecs, ecs->dgram_processed);
	}
}

void ecs_rx_packet(e_slave *ecs,uint8_t *d)
{
	do{
		raw_ecs_rx_packet(ecs);
	} while (!__ec_is_ethercat(ecs->pkt_head));

	// grab first ecat dgram
	ecs->dgram_processed =  __ecat_frameheader(ecs->pkt_head) + sizeof(ec_frame_header);
	ecs->dgrams_cnt = ec_nr_dgrams(ecs->pkt_head);
	__set_fsm_state(ecs, ecs_process_packet);
}

int main(int argc, char *argv[])
{
	struct fsm_slave fsm_slave;
	e_slave ecs;

	if (argc < 2) {
		/*
		 * if you provide two different interfaces 
		 *  then it is considered an open loop,
 		  * else it is a closed loop ,ie, last slave.
		*/
		printf("%s < rx interface> < tx interface>\n", argv[0]);
		return 0;
	}
  	if (ecs_net_init(argc, argv, &ecs) < 0){
		return -1;
	}
	ec_init_regs(&ecs);
	init_sii();
	init_process_data();
	ecs.fsm = &fsm_slave;
	ecs.dgram_processed = &ecs.pkt_head[0];
	ecs.dgrams_cnt = 0;
	__set_fsm_state(&ecs, ecs_rx_packet);

	ecs_run(&ecs);
	return 0;
}

