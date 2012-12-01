#ifndef __EC_SII__
#define __EC_SII__

void init_sii(ecat_slave *);
void ec_sii_rw(uint8_t * data, int datalen);
int ec_sii_start_read(uint8_t * data, int datalen);
int ec_sii_pdoes_sizes(ecat_slave *);
void ec_sii_syncm(int reg, uint8_t* data, int datalen);
#endif
