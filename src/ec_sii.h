#ifndef __EC_SII__
#define __EC_SII__

void init_sii(void);
void ec_sii_rw(uint8_t * data, int datalen);
int ec_sii_start_read(uint8_t * data, int datalen);

#endif
