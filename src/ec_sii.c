
#include <stdio.h>
#include <string.h>
#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include <arpa/inet.h>

#define CAT_TYPE_NOP	0x00
#define CAT_TYPE_STRINGS 0x10
#define CAT_TYPE_DATA_TYPE 0x20
#define CAT_TYPE_GENERAL  0x30
#define CAT_TYPE_FMMU	 0x40
#define CAT_TYPE_SYNMC	 0x41
#define CAT_TYPE_TXPDO	 0x50
#define CAT_TYPE_RXPDO	 0x51
#define CAT_TYPE_DC	0x60
#define CAT_TYPE_END	0xFFFF

typedef struct {
    uint16_t pdi_control;
    uint16_t pdi_configuration;
    uint16_t sync_impulse_len;
    uint16_t pdi_configuration2;
    uint16_t alias; /**< The slaves alias if not equal to 0. */
    uint8_t reverved[4]; 	   
    uint16_t checksum;
    uint32_t vendor_id; /**< Vendor-ID stored on the slave. */
    uint32_t product_code; /**< Product-Code stored on the slave. */
    uint32_t revision_number; /**< Revision-Number stored on the slave. */
    uint32_t serial_number; /**< Serial-Number stored on the slave. */
    uint8_t  reserved[8];
    uint16_t boot_rx_mailbox_offset;
    uint16_t boot_rx_mailbox_size;
    uint16_t boot_tx_mailbox_offset;
    uint16_t boot_tx_mailbox_size;
    uint16_t std_rx_mailbox_offset;
    uint16_t std_rx_mailbox_size;
    uint16_t std_tx_mailbox_offset;
    uint16_t std_tx_mailbox_size;
    uint16_t mailbox_protocols;
    uint8_t  reserved2[66];
    uint16_t eprom_size_kbits;
    uint16_t version;
} ec_sii_t;

#define STRING1 "string 1"
#define STRING2 "string 2"
#define STRING3 "string 3"
#define NR_PDOS	2

#define NR_STRINGS ( NR_PDOS  )

// table 25
typedef struct {
	uint16_t index;
	uint8_t subindex;
	uint8_t name_idx; /* idx to strings */
	uint8_t data_type;
	uint8_t bit_len;
	uint16_t flags;	
}pdo_entry;

// table 24
typedef struct {
	uint16_t pdo_index;
	uint8_t  entries;
	uint8_t  syncm;
	uint8_t  synchronization;
	uint8_t  name_idx;
	uint16_t flags;
	pdo_entry pdos[NR_PDOS];
}category_pdo;

// table 23
typedef struct {
	uint16_t phys_start_address;
	uint16_t length;
	uint8_t  ctrl_reg;
	uint8_t	 status_reg;
	uint8_t  enable_syncm;
	uint8_t  syncm_type;
}category_syncm;

typedef struct {
	uint8_t fmmu0;
	uint8_t fmmu1;
}category_fmmu;

typedef struct {
	uint8_t groupd_idx; /*index to strings  */
	uint8_t img_idx;    /* index to strings */
	uint8_t order_idx;   /* index to strings */
	uint8_t name_idx;   /* index to strings */
	uint8_t reserved;
	uint8_t coe_details;
	uint8_t foe_detials;
	uint8_t eoe_detials;
	uint8_t soe_detials;
	uint8_t ds402channels;
	uint8_t flags;
	uint16_t current_on_bus;
	uint16_t pad_byte1;
	uint16_t physical_port __attribute ((packed));
	uint8_t pad_byte2[14];
} category_general;

typedef struct {
	uint8_t nr_strings;
	uint8_t str1_len;
	char str1[sizeof(STRING1)];
	uint8_t str2_len;
	char str2[sizeof(STRING2)];
	uint8_t str3_len ;
	char str3[sizeof(STRING3)];
}category_string;

typedef struct{
	uint16_t type :15;
	uint16_t vendor_specific:1;		
	uint16_t size;
}category_header;

typedef struct {

	category_header strings_hdr  __attribute__ ((packed));
	category_string strings;

	category_header general_hdr  __attribute__ ((packed));
	category_general general;

	category_header txpdo_hdr  __attribute__ ((packed));
	category_pdo txpdo;

	category_header rxpdo_hdr  __attribute__ ((packed));
	category_pdo rxpdo;

	category_header fmmu_hdr __attribute__ ((packed));
	category_fmmu fmmu;
	
	category_header syncm_hdr  __attribute__ ((packed));
	category_syncm syncm __attribute__ ((packed));

} sii_categories ;

sii_categories categories;

void init_sii(void)
{
 }


int ec_sii_fetch(uint8_t *data, int datalen)
{
	int word_offset;

	printf("%s received datalen %d\n",
				__FUNCTION__, datalen);

	if (data[0] != 0x80) {
		printf("%s no two addressed octets %x %x\n",
				__FUNCTION__, data[0], data[1]);
		return 1;
	}
	if (data[1] != 0x01) {
		printf("request read operation\n", __FUNCTION__);
		return 1;
	}
	word_offset = *(uint16_t *) & data[2];
	printf("%s received word offset %d\n",
				__FUNCTION__, word_offset);
	memset(data ,0 , datalen);
	return 0;
}
