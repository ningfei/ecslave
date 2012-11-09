#include <string.h>
#include <stdio.h>
#include "ethercattype.h"
#include "ec_process_data.h"

void init_process_data(void)
{
	memset(process_data,'x',sizeof(process_data));
}
