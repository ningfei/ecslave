#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs_slave.h"
#include "ec_coe.h"
#include "ec_mbox.h"


fsm_mbox mbox={0};

void mbox_set_state(void (*state)(uint8_t* data,int datalen))
{
	mbox.state = state;
}

void ec_mbox(int reg, uint8_t * data, int datalen)
{
	mbox_header *mbxhdr = __mbox_hdr(data);
	
	if (mbxhdr->type ==  MBOX_COE_TYPE){
		coe_parser(reg, data, datalen);
		mbxhdr->cnt++;
		return;
	}
	if (mbox.state) {
		mbox.state(data, datalen);
	}

}
