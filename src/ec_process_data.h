#ifndef  __EC_PROCESS_DATA_H__
#define  __EC_PROCESS_DATA_H__

struct __e_slave__ ;

int set_process_data(uint8_t * data, uint16_t offset, uint16_t datalen);
int get_process_data(uint8_t * data, uint16_t offset, uint16_t datalen);
int init_process_data(struct __e_slave__ *);

#endif
