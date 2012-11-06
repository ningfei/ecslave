#ifndef __EC_COE_H__
#define __EC_COE_H__

#include "ec_mbox.h"


enum {
	OD_LIST_REQUEST   = 0x01,	
	OD_LIST_RESPONSE  = 0x02,
	OBJ_DESC_REQUEST  = 0x03,
	OBJ_DESC_RESPONSE = 0x04,
	ENTRY_DESC_REQUEST = 0X05,
	ENTRY_DESC_RESPONSE = 0X06,
	SDO_INFO_ERROR_REQUEST  = 0X07
}sdo_info_hdr_opcode;

void coe_parser(int reg, uint8_t * data, int datalen);

// table 42
typedef struct {
	uint16_t list_type;
}coe_sdo_service_data;
#endif
