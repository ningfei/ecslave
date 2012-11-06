#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs_slave.h"
#include "ec_coe.h"

#define MBOX_COE_TYPE 0x03
#define NR_SDOS 8

static int sdo_list_len = NR_SDOS * 8;

void ec_mbox(int reg, uint8_t * data, int datalen)
{
	mbox_header *mbxhdr = __mbox_hdr(data);
	
	if (mbxhdr->type ==  MBOX_COE_TYPE){
		coe_parser(reg, data, datalen);
		mbxhdr->cnt++;
		return;
	}
	printf("%s\n",__FUNCTION__);
	{
	int i;
	coe_header *coehdr = __coe_header(data);
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	coe_sdo_service_data *srvdata =
		(coe_sdo_service_data *)&data[sizeof(mbox_header) + sizeof(coe_header) + sizeof(coe_sdo_info_header)];
	uint64_t *sdo_data = (uint64_t *) (&srvdata->list_type); /* each sdo is 8 bytes */	

	mbxhdr->type =  MBOX_COE_TYPE;
	mbxhdr->len = sdo_list_len;
	coehdr->coe_service = COE_SDO_INFO;
	sdoinfo->opcode = OD_LIST_RESPONSE;
	srvdata->list_type = 0x1;
	// do the reponse
	sdoinfo->opcode = 0x02; // table 43
	for (i = 0  ; i < 8; i++) 
		sdo_data[i] = 0x1888 + i;
	}
}
