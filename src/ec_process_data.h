#ifndef  __EC_PROCESS_DATA_H__
#define  __EC_PROCESS_DATA_H__

static uint8_t process_data[65536];

static inline void set_process_data(uint8_t * data, uint16_t offset, uint16_t datalen)
{
	memcpy(&process_data[offset], data, datalen);
}

static inline void get_process_data(uint8_t * data, uint16_t offset, uint16_t datalen)
{
	memcpy(data, &process_data[offset], datalen);
}

#endif
