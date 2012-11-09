#ifndef  __EC_PROCESS_DATA_H__
#define  __EC_PROCESS_DATA_H__

extern uint8_t process_data[];
void set_process_data(uint8_t * data, uint16_t offset, uint16_t datalen);
void get_process_data(uint8_t * data, uint16_t offset, uint16_t datalen);
void init_process_data(void);

#endif
