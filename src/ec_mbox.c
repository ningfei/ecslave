#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs_slave.h"
#include "ec_coe.h"

#define MBOX_COE_TYPE 0x03

void ec_mbox(int reg, uint8_t * data, int datalen)
{
	mbox_header *mbxhdr = __mbox_hdr(data);
	
	if (mbxhdr->type ==  MBOX_COE_TYPE){
		coe_parser(reg, data, datalen);
		mbxhdr->cnt++;
		return;
	}
	puts("MBOX AIIIEEE");
}

//  etherlan gave Sync manager and wishes to get a mail box
void ec_mbox_syncm(int reg, uint8_t* data, int datalen)
{
	
}
