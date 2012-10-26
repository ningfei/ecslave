
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
#define CAT_TYPE_syncm	 0x41
#define CAT_TYPE_TXPDO	 0x50
#define CAT_TYPE_RXPDO	 0x51
#define CAT_TYPE_DC	0x60
#define CAT_TYPE_END	0xFFFF

typedef struct {
	uint16_t pdi_control;
	uint16_t pdi_configuration;
	uint16_t sync_impulse_len;
	uint16_t pdi_configuration2;
	uint16_t alias;
		    /**< The slaves alias if not equal to 0. */
	uint8_t reverved[4];
	uint16_t checksum;
	uint32_t vendor_id;
			/**< Vendor-ID stored on the slave. */
	uint32_t product_code;
			   /**< Product-Code stored on the slave. */
	uint32_t revision_number;
			      /**< Revision-Number stored on the slave. */
	uint32_t serial_number;
			    /**< Serial-Number stored on the slave. */
	uint8_t reserved[8];
	uint16_t boot_rx_mailbox_offset;
	uint16_t boot_rx_mailbox_size;
	uint16_t boot_tx_mailbox_offset;
	uint16_t boot_tx_mailbox_size;
	uint16_t std_rx_mailbox_offset;
	uint16_t std_rx_mailbox_size;
	uint16_t std_tx_mailbox_offset;
	uint16_t std_tx_mailbox_size;
	uint16_t mailbox_protocols;
	uint8_t reserved2[66];
	uint16_t eprom_size_kbits;
	uint16_t version;
} ec_sii_t;

#define GROUP_IDX			0
#define IMAGE_IDX			1
#define ORDER_IDX			2
#define NAME_IDX			3
#define RXPDO_CAT_NAME_IDX	4
#define TXPDO_CAT_NAME_IDX	5
#define TX_PDO1_NAME_IDX	6
#define TX_PDO2_NAME_IDX	7
#define RX_PDO1_NAME_IDX	8
#define RX_PDO2_NAME_IDX	9

#define GROUP_STRING		"LIBIX GROUP"
#define IMAGE_STRING		"LIBIX IMAGE"
#define ORDER_STRING		"LIBIX ORDER"
#define NAME_STRING			"LIBIX NAME"
#define RXPDO_CAT_STRING	"LIBIX RX_PDO"
#define TXPDO_CAT_STRING	"LIBIX TX_PDO"
#define TX_PDO1_NAME		"TXPDO1	LIBIX"
#define TX_PDO2_NAME		"TXPD2 LIBIX"
#define RX_PDO1_NAME		"RXPDO1 LIBIX"
#define RX_PDO2_NAME		"RXPDO2 LIBIX"

#define SDO_ENABLED 		0b000001
#define	SDO_INFO			0b000010
#define	PDO_ASSIGN			0b000100
#define	PDO_CONF			0b001000
#define	STARTUP_UPLOAD		0b010000
#define	SDO_COMPLETE_ACCESS	0b100000

#define STRING0 GROUP_STRING
#define STRING1 IMAGE_STRING
#define STRING2 ORDER_STRING
#define STRING3 NAME_STRING
#define STRING4	RXPDO_CAT_STRING
#define STRING5 TXPDO_CAT_STRING
#define STRING6	TX_PDO1_NAME
#define STRING7 TX_PDO2_NAME
#define STRING8 RX_PDO1_NAME
#define STRING9 RX_PDO1_NAME

#define PORT_MII	0b01
#define PORT0_SHIFT	0


#define NR_PDOS	2

#define NR_STRINGS  10

// table 25
typedef struct {
	uint16_t index;
	uint8_t subindex;
	uint8_t name_idx;	/* idx to strings */
	uint8_t data_type;
	uint8_t bit_len;
	uint16_t flags;
} pdo_entry;

// table 24
typedef struct {
	uint16_t pdo_index;
	uint8_t entries;
	uint8_t syncm;
	uint8_t synchronization;
	uint8_t name_idx;
	uint16_t flags;
	pdo_entry pdo[NR_PDOS];
} category_pdo;

// table 23
typedef struct {
	uint16_t phys_start_address;
	uint16_t length;
	uint8_t ctrl_reg;
	uint8_t status_reg;
	uint8_t enable_syncm;
	uint8_t syncm_type;
} category_syncm;

typedef struct {
	uint8_t fmmu0;
	uint8_t fmmu1;
} category_fmmu;

typedef struct {
	uint8_t groupd_idx;	/*index to strings  */
	uint8_t img_idx;	/* index to strings */
	uint8_t order_idx;	/* index to strings */
	uint8_t name_idx;	/* index to strings */
	uint8_t reserved;
	uint8_t coe_details;
	uint8_t foe_detials;
	uint8_t eoe_detials;
	uint8_t soe_detials;
	uint8_t ds402channels;
	uint8_t sysman_class;
	uint8_t flags;
	uint16_t current_on_bus;
	uint16_t pad_byte1;
	uint16_t physical_port __attribute((packed));
	uint8_t pad_byte2[14];
} category_general;

typedef struct {
	uint8_t nr_strings;
	uint8_t str1_len;
	char str1[sizeof(STRING1)];
	uint8_t str2_len;
	char str2[sizeof(STRING2)];
	uint8_t str3_len;
	char str3[sizeof(STRING3)];
}
category_strings;

typedef struct {
	uint16_t type:15;
	uint16_t vendor_specific:1;
	uint16_t size;
} category_header;

typedef struct {

	category_header strings_hdr __attribute__ ((packed));
	category_strings strings;

	category_header general_hdr __attribute__ ((packed));
	category_general general;

	category_header txpdo_hdr __attribute__ ((packed));
	category_pdo txpdo;

	category_header rxpdo_hdr __attribute__ ((packed));
	category_pdo rxpdo;

	category_header fmmu_hdr __attribute__ ((packed));
	category_fmmu fmmu;

	category_header syncm_hdr __attribute__ ((packed));
	category_syncm syncm __attribute__ ((packed));

} sii_categories;

sii_categories categories;

void init_general(category_general * general,category_header * hdr)
{
	hdr->size = sizeof(*general);
	hdr->type = CAT_TYPE_GENERAL;

	memset(general, 0x00, sizeof(*general));
	general->groupd_idx = GROUP_IDX;
	general->img_idx = IMAGE_IDX;
	general->order_idx = ORDER_IDX;
	general->name_idx = NAME_IDX;

	general->coe_details = SDO_ENABLED | SDO_INFO | PDO_ASSIGN
	    | PDO_CONF | STARTUP_UPLOAD | SDO_COMPLETE_ACCESS;

	general->foe_detials = 0;
	general->eoe_detials = 0;
	general->soe_detials = 0;
	general->ds402channels = 0;
	general->sysman_class = 0;
	general->flags = 0;
	general->current_on_bus = 100;
	general->pad_byte1 = 0;
	general->physical_port = (PORT_MII << PORT0_SHIFT);
}

void init_syncm(category_syncm * syncm,category_header * hdr)
{
	hdr->size = sizeof(*syncm) / 2;
	hdr->type = CAT_TYPE_syncm;

	syncm->ctrl_reg = 0;
	syncm->enable_syncm = 0;
	syncm->length = 0;
	syncm->phys_start_address = 0;
	syncm->status_reg = 0;
	syncm->syncm_type = 0;	// not used
}

void init_fmmu(category_fmmu * fmmu,category_header * hdr)
{
	hdr->size = sizeof(*fmmu) / 2;
	hdr->type = CAT_TYPE_FMMU;
	fmmu->fmmu0 = 0;	// fmmmu not used
	fmmu->fmmu1 = 0;	// fmmmu not used
}

void init_strings(category_strings * str, category_header * hdr)
{
	hdr->size = sizeof(*str) / 2;
	hdr->type = CAT_TYPE_STRINGS;

	str->str1_len = sizeof(STRING1);
	strncpy(str->str1, STRING1, str->str1_len);

	str->str2_len = sizeof(STRING2);
	strncpy(str->str2, STRING2, str->str2_len);

	str->str3_len = sizeof(STRING3);
	strncpy(str->str3, STRING2, str->str3_len);
}

void init_pdo(pdo_entry * pdo,
	      uint16_t index,
	      uint8_t subindex,
	      uint8_t name_idx,
	      uint8_t data_type, uint8_t bit_len, uint16_t flags)
{
	pdo->bit_len = index;
	pdo->data_type = data_type;
	pdo->flags = flags;
	pdo->index = index;
	pdo->name_idx = name_idx;
	pdo->subindex = subindex;
}

void init_sii(void)
{
	init_strings(&categories.strings, &categories.strings_hdr);
	init_fmmu(&categories.fmmu, &categories.fmmu_hdr);
	init_syncm(&categories.syncm, &categories.syncm_hdr);
	init_general(&categories.general, &categories.general_hdr);

	// pdos.
	categories.rxpdo.entries = 2;
	categories.rxpdo.flags = 0;
	categories.rxpdo.name_idx = RXPDO_CAT_NAME_IDX;
	categories.rxpdo.synchronization = 0;
	categories.rxpdo.syncm = 0;
	categories.rxpdo.pdo_index = 0x1600;

	init_pdo(&categories.rxpdo.pdo[0], 0x1614, 0X02, RX_PDO1_NAME_IDX, 0,	// index in the object dictionary
		 32, 0);

	init_pdo(&categories.rxpdo.pdo[1], 0x1748, 0X01, RX_PDO2_NAME_IDX, 0,	// index in the object dictionary
		 32, 0);

	categories.txpdo.entries = 2;
	categories.txpdo.flags = 0;
	categories.txpdo.name_idx = TXPDO_CAT_NAME_IDX;
	categories.txpdo.synchronization = 0;
	categories.txpdo.syncm = 0;
	categories.txpdo.pdo_index = 0x1a00;

	init_pdo(&categories.txpdo.pdo[0], 0x1a01, 0X02, TX_PDO1_NAME_IDX, 0,	// index in the object dictionary
		 32, 0);

	init_pdo(&categories.txpdo.pdo[1], 0x1a03, 0X01, TX_PDO2_NAME_IDX, 0,	// index in the object dictionary
		 16, 0);
}

int ec_sii_fetch(uint8_t * data, int datalen)
{
	int word_offset;

	printf("%s received datalen %d\n", __FUNCTION__, datalen);

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
	printf("%s received word offset %d\n", __FUNCTION__, word_offset);
	memset(data, 0, datalen);
	return 0;
}
