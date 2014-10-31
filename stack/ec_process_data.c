#include "../include/xgeneral.h"
#include "ethercattype.h"
#include "ec_process_data.h"
#include "ecs_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"

typedef struct {
	uint8_t* data;
	int size;
}process_data;

static process_data pd;

int init_process_data(ecat_slave *ecs)
{
	pd.size = ec_sii_pdoes_sizes(ecs);
	if (pd.size <= 0 ){
		return -1;
	}	
	pd.data = xmalloc(pd.size);
	if (pd.data == 0)
		return -1;
	memset(pd.data, 'x', pd.size);
	return 0;
}

/*
 * process data 
*/
int set_process_data(uint8_t *data, uint16_t offset, uint16_t datalen)
{
	memcpy(&pd.data[offset % pd.size], data, datalen);
	return 0;
}

int get_process_data(uint8_t *data, uint16_t offset, uint16_t datalen)
{
	memcpy(data, &pd.data[offset % pd.size], datalen);
	return 0;
}

void normalize_sizes(ecat_slave *ecs, uint32_t *offset,uint16_t *datalen)
{
	int off  = (ec_station_address(ecs) -1) * pd.size + *offset;
	if (off < 0) {
		ec_printf("%s off=%d  offset=%d  stafr=%hu\n",
			__func__, off, *offset, ec_station_address(ecs));
		return;
	}
	ec_printf("%s off=%d offset=%d statr=%d datalen=%hu\n",
			__func__, 
			off, 
			*offset, 
			ec_station_address(ecs),
			*datalen);
	*offset = (uint32_t)off;
	*datalen = *datalen % pd.size;
	if (*datalen == 0){
		*datalen = pd.size;
	}
}
