
#ifndef __EC_CMD_H__
#define __EC_CMD_H__

void ecs_process_cmd(e_slave *ecs, uint8_t *dgram_ec);
void ec_process_datagrams(e_slave *ecs,int len, uint8_t *dgram);

#endif
