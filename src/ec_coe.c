#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_regs.h"
#include "ec_coe.h"

// table 45
void obj_desc_request(uint8_t *data, int datalen)
{
	typedef struct {
		uint16_t index;	
		uint16_t data_type;
		uint8_t  max_subindex;
		uint8_t  object_code;
		char     name[1];
	}sdo_info_service_data;

	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	sdo_info_service_data *obj_desc = 
		(sdo_info_service_data *)&sdoinfo->sdo_info_service_data[0];
	printf("%s index = 0x%x\n",__FUNCTION__,obj_desc->index);
	obj_desc->data_type = 0x05;
	obj_desc->max_subindex = 1;
	obj_desc->object_code = 7;
	sprintf(obj_desc->name,"LINUX HD SDO %d",obj_desc->index);
}

void entry_desc_request(uint8_t *data, int datalen)
{
	typedef struct {
		uint16_t index;	
		uint8_t  subindex;
		uint8_t  valueinfo;
		uint16_t datatype;
		uint16_t bit_len;
		uint16_t object_access;
	}sdo_entry_info_data;

	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	sdo_entry_info_data *entry_desc = 
		(sdo_entry_info_data *)&sdoinfo->sdo_info_service_data[0];
	
	printf("%s %x:%x\n",
		__FUNCTION__,
		entry_desc->index,entry_desc->subindex);
	entry_desc->valueinfo =  0b1000;
	entry_desc->datatype = 0;
	entry_desc->bit_len = 8;
	entry_desc->object_access = 0x0FFF;
}

void od_list_request(uint8_t * data, int datalen)
{	
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	coe_sdo_service_data *srvdata =
		(coe_sdo_service_data *)&data[sizeof(mbox_header) + sizeof(coe_header) + sizeof(coe_sdo_info_header)];
	
	srvdata->list_type = 0x1;
	// do the reponse
	sdoinfo->opcode = 0x02; // table 43
	printf("%s\n",__FUNCTION__);
}

void coe_sdo_info(uint8_t * data, int datalen)
{
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);

	printf("%s opcode=%d\n",__FUNCTION__,sdoinfo->opcode);
	switch(sdoinfo->opcode)
	{
	case OD_LIST_REQUEST:
		od_list_request(data, datalen);
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

	if (reg > __sdo_high()){
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
