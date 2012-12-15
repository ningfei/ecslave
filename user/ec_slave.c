#include "xgeneral.h"
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_process_data.h"
#include "ec_com.h"

int main(int argc, char *argv[])
{
	struct fsm_slave fsm_slave;
	ecat_slave ecs;

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
	init_sii(&ecs);
	ecat_create_timer();
	if (init_process_data(&ecs) < 0){
		printf ("%s illegal pdo configuration\n",argv[0]);
		return -1;
	}
	ecs.fsm = &fsm_slave;
	ecs.dgram_processed = &ecs.pkt_head[0];
	ecs.dgrams_cnt = 0;
	ec_capture(&ecs);
	return 0;
}

