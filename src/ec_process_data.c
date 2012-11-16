#include <string.h>
#include <stdio.h>
#include "ethercattype.h"
#include "ec_process_data.h"
#include "ecs_slave.h"
#include "ec_sii.h"

typedef struct {
	uint8_t* data;
	int size;
}process_data;

static process_data pd;

int init_process_data(e_slave *ecs)
{
	pd.size = ec_sii_pdoes_sizes(ecs);
	if (pd.size <= 0 ){
		return -1;
	}	
	pd.data = malloc(pd.size);
	memset(pd.data, 'x', pd.size);
	printf("Process data size = %d\n",pd.size);
	return 0;
}

int set_process_data(uint8_t *data, uint16_t offset, uint16_t datalen)
{
	if (offset + datalen > pd.size) {
		printf("%s ilegal pd access\n",__FUNCTION__);
		return -1;
	}
	memcpy(&pd.data[offset], data, datalen);
	return 0;
}

int get_process_data(uint8_t * data, uint16_t offset, uint16_t datalen)
{
	if (offset + datalen > pd.size) {
		printf("%s ilegal pd access\n",__FUNCTION__);
		return -1;
	}
	memcpy(data, &pd.data[offset], datalen);
	return 0;
}
