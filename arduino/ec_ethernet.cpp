#include <Arduino.h>
#include "etherShield.h"
#include "xgeneral.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_device.h"
#include "fsm_slave.h"
#include "ec_sii.h"
#include "ec_regs.h"
#include "ec_net.h"
#include "ec_cmd.h"
#include "ec_com.h"


// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
static uint8_t mymac[6] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x24 };
static uint8_t myip[4] = { 192, 168, 1, 15 };

// how did I get the mac addr? Translate the first 3 numbers into ascii is: TUX

#define BUFFER_SIZE 200
unsigned char _buf[BUFFER_SIZE + 1];
unsigned char *buf = (unsigned char*)&_buf[0];
EtherShield es = EtherShield();

void __ethernetSetup()
{

	/*initialize enc28j60 */
	es.ES_enc28j60Init(mymac);
	es.ES_enc28j60clkout(2);	// change clkout from 6.25MHz to 12.5MHz

	delay(10);

	/* Magjack leds configuration, see enc28j60 datasheet, page 11 */
	// LEDA=green LEDB=yellow
	//
	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	es.ES_enc28j60PhyWrite(PHLCON, 0x880);
	delay(500);

	//
	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	es.ES_enc28j60PhyWrite(PHLCON, 0x990);
	delay(500);

	//
	// 0x880 is PHLCON LEDB=on, LEDA=on
	// enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
	es.ES_enc28j60PhyWrite(PHLCON, 0x880);
	delay(500);

	//
	// 0x990 is PHLCON LEDB=off, LEDA=off
	// enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
	es.ES_enc28j60PhyWrite(PHLCON, 0x990);
	delay(500);

	//
	// 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	// enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
	es.ES_enc28j60PhyWrite(PHLCON, 0x476);
	delay(100);

	//init the ethernet/ip layer:
	es.ES_init_ip_arp_udp_tcp(mymac, myip, 80);

        Serial.println("Ethernet setup");       
}

extern "C" void ethernetSetup()
{
	__ethernetSetup();
}

void ecat_rcv(ecat_slave  *ecs)
{
	int len = es.ES_enc28j60PacketReceive(BUFFER_SIZE, buf);
	if (len <= 0) {
		return;
	}
	if (eth_hdr(buf)->ether_type !=  htons(ETH_P_ECAT)){
        	Serial.print("!eCAT");
		return;
	}
	ec_process_datagrams(ecs, len, buf);
}

extern "C" void ec_tx_pkt(uint8_t *buf, int sz, struct ec_device *txdev)
{
        es.ES_WriteRawPacket(buf, sz);
}
