#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_regs.h"
#include "ec_coe.h"

void od_list_response(uint8_t* data,int datalen)
{
	int i;

	mbox_header *mbxhdr = __mbox_hdr(data);
	coe_header *coehdr = __coe_header(data);
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	coe_sdo_service_data *srvdata =__coe_sdo_service_data(data);
	uint32_t *sdo_data = (uint32_t *) (&srvdata->list_type); /* each sdo is 8 bytes */	

	mbxhdr->type =  MBOX_COE_TYPE;
	mbxhdr->len = 8 + NR_SDOS * 4;
	coehdr->coe_service = COE_SDO_INFO;
	sdoinfo->opcode = OD_LIST_RESPONSE;
	srvdata->list_type = 0x1;
	// do the reponse
	sdoinfo->opcode = 0x02; // table 43
	for (i = 0  ; i < NR_SDOS; i++) { 
		/* starting from 0x1888 create NR SDOS */
		sdo_data[i] = 0x1888 + i; 	
	}
}

static int obj_index = 0;
static int obj_subindex = 0;

// table 45
void obj_desc_response(uint8_t *data, int datalen)
{
	typedef struct {
		uint16_t index;	
		uint16_t data_type;
		uint8_t  max_subindex;
		uint8_t  object_code;
		char     name[1];
	}sdo_info_service_data;

	coe_header *coehdr = __coe_header(data);
	mbox_header *mbxhdr = __mbox_hdr(data);
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	sdo_info_service_data *obj_desc = 
		(sdo_info_service_data *)&sdoinfo->sdo_info_service_data[0];

	mbxhdr->type =  MBOX_COE_TYPE;

	coehdr->number = 0;
	coehdr->reserved = 0;
	coehdr->coe_service = COE_SDO_INFO;

	sdoinfo->reserved = 0;
	sdoinfo->frag_list = 0;
	sdoinfo->opcode = OBJ_DESC_RESPONSE;

	obj_desc->index = obj_index;
	obj_desc->data_type = 0x05;
	obj_desc->max_subindex = 4;
	obj_desc->object_code = 7;

	sprintf(obj_desc->name,"LINUX DRIVE SDO 0x%X",obj_desc->index);
	mbxhdr->len = sizeof(*sdoinfo) + sizeof(*coehdr) + sizeof(*obj_desc) + strlen(obj_desc->name);
}

// table 44
void obj_desc_request(uint8_t *data, int datalen)
{
	typedef struct {
		uint16_t  index;
	}sdo_info_service_data;

	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	sdo_info_service_data *obj_desc = 
		(sdo_info_service_data *)&sdoinfo->sdo_info_service_data[0];
	obj_index = obj_desc->index;
	mbox_set_state(obj_desc_response);
}

void entry_desc_response(uint8_t *data, int datalen)
{	// table 47
	typedef struct {
		uint16_t index;	
		uint8_t  subindex;
		uint8_t  valueinfo;
		uint16_t datatype;
		uint16_t bit_len;
		uint16_t object_access;
		char     name[1];
	}sdo_entry_info_data;

	coe_header *coehdr = __coe_header(data);
	mbox_header *mbxhdr = __mbox_hdr(data);
	coe_sdo_info_header *sdoinfo = __sdo_info_hdr(data);
	sdo_entry_info_data *entry_desc = 
		(sdo_entry_info_data *)&sdoinfo->sdo_info_service_data[0];
	
	coehdr->number 	    = 0;
	coehdr->coe_service =  COE_SDO_INFO;

	sdoinfo->opcode = ENTRY_DESC_RESPONSE;
	sdoinfo->frag_list = 0;

	mbxhdr->type =  MBOX_COE_TYPE;

	entry_desc->valueinfo =  0b1000;
	entry_desc->datatype  = 0x5;
	entry_desc->bit_len   = 8;
	entry_desc->object_access = 0x0FFF;
	entry_desc->index	  = obj_index;
	entry_desc->subindex 	  = obj_subindex; 
	sprintf(entry_desc->name,"LINUX SDO ENTRY 0x%4X:0x%X",entry_desc->index,entry_desc->subindex);
	mbxhdr->len = 	sizeof(*sdoinfo) + 
			sizeof(*coehdr) + 
			sizeof(*entry_desc) + strlen(entry_desc->name);
}

// table 46
void entry_desc_request(uint8_t *data, int datalen)
{
	typedef struct {
		uint16_t index;	
		uint8_t  subindex;
		uint8_t  valueinfo;
	}sdo_entry_info_data;

	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	sdo_entry_info_data *entry_desc = 
		(sdo_entry_info_data *)&sdoinfo->sdo_info_service_data[0];
	obj_index = entry_desc->index;
	obj_subindex = entry_desc->subindex;
	mbox_set_state(entry_desc_response);
}

void od_list_request(uint8_t * data, int datalen)
{
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);
	coe_sdo_service_data *srvdata =__coe_sdo_service_data(data);

	srvdata->list_type = 0x1;
	// do the reponse
	sdoinfo->opcode = 0x02; // table 43
	mbox_set_state(od_list_response);
}

void coe_sdo_info(uint8_t * data, int datalen)
{
	coe_sdo_info_header * sdoinfo = __sdo_info_hdr(data);

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
