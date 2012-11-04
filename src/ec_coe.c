#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_regs.h"

#define MBOX_COE_TYPE 0x03

enum {
	OD_LIST_REQUEST   = 0x01,	
	OD_LIST_RESPONSE  = 0x02,
	OBJ_DESC_REQUEST  = 0x03,
	OBJ_DESC_RESPONSE = 0x04,
	ENTRY_DESC_REQUEST = 0X05,
	ENTRY_DESC_RESPONSE = 0X06,
	SDO_INFO_ERROR_REQUEST  = 0X07
}sdo_info_hdr_opcode;

static void obj_desc_request(uint8_t *data, int datalen)
{
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	uint32_t *sdo_index;

	sdo_index =  (uint32_t *)&sdoinfo->sdo_info_service_data;
	printf("%s sdo index = %d\n",__FUNCTION__, *sdo_index);
	*sdo_index = 0x8888;
}

static void entry_desc_request(uint8_t *data, int datalen)
{
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	uint16_t *sdo_index;
	uint8_t *sdo_subindex;

	sdo_index =  (uint16_t *)&sdoinfo->sdo_info_service_data;
	sdo_subindex = (uint8_t *)&sdoinfo->sdo_info_service_data[2];
	printf("%s sdo index 0x%x:0x%x\n", 
		__FUNCTION__,
		*sdo_index, *sdo_subindex);
}

void coe_sdo_info(uint8_t * data, int datalen)
{
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);

	printf("%s opcode=%d\n",__FUNCTION__,sdoinfo->opcode);
	switch(sdoinfo->opcode)
	{
	case OD_LIST_REQUEST:
		break;
	case OD_LIST_RESPONSE:
		break;
	case OBJ_DESC_REQUEST:
		obj_desc_request(data, datalen);
		break;
	case OBJ_DESC_RESPONSE:
		break;
	case ENTRY_DESC_REQUEST:
		entry_desc_request(data, datalen);
		break;
	case ENTRY_DESC_RESPONSE:
		break;
	case SDO_INFO_ERROR_REQUEST:
		break;
	}
}

void coe_parser(int reg, uint8_t * data, int datalen)
{
	coe_header *hdr = __coe_header(data);

	if (reg > sdo_high()){
		printf("%s no such address %d\n",
			__FUNCTION__,
			reg);
		return;
	}
	printf("coe header ->service  %d \n",
		hdr->coe_service);
	switch (hdr->coe_service) 
	{
	case COE_EMERGENCY:
		break;
	case COE_SDO_REQUEST:
		break;
	case COE_SDO_RESPONSE:
		break;
	case COE_TX_PDO:
		break;
	case COE_RX_PDO:
		break;
	case COE_TX_PDO_REMOTE:
		break;
	case COE_RX_PDO_REMOTE:
		break;
	case COE_SDO_INFO:
		coe_sdo_info(data, datalen);
		break;
	}
}

void ec_mbox(int reg, uint8_t * data, int datalen)
{
	mbox_header *mbxhdr = __mbox_hdr(data);
	
	if (mbxhdr->type ==  MBOX_COE_TYPE){
		coe_parser(reg, data, datalen);
		return;
	}
	puts("MBOX AIIIEEE");
}
