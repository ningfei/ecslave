#include <string.h>
#include <stdio.h>
#include "ethercattype.h"
#include "ec_process_data.h"

uint8_t process_data[66500];

void init_process_data(void)
{
	memset(process_data, 'x', sizeof(process_data));
}

void set_process_data(uint8_t *data, uint16_t offset, uint16_t datalen)
{
	memcpy(&process_data[offset], data, datalen);
}

void get_process_data(uint8_t * data, uint16_t offset, uint16_t datalen)
{
	memcpy(data, &process_data[offset], datalen);
}
