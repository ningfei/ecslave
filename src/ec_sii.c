
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ethercattype.h"
#include "fsm_slave.h"
#include "ecs_slave.h"
#include "ec_regs.h"
#include "ec_sii.h"

#define EC_FIRST_SII_CATEGORY_OFFSET 0x040

#define CAT_TYPE_NOP	0x00

#define CAT_TYPE_STRINGS 0x000A

#define CAT_TYPE_DATA_TYPE 0x014

#define CAT_TYPE_GENERAL  0x001E
#define CAT_TYPE_FMMU	  0x0028
#define CAT_TYPE_SYNCM	  0x0029
#define CAT_TYPE_TXPDO	  0x0032
#define CAT_TYPE_RXPDO	  0x0033
#define CAT_TYPE_DC	  0x003C
#define CAT_TYPE_END	  0xFFFF

#define MBOX_SIZE	30
#define NR_SYNCM	2
#define	SYNMC_SIZE	1024


typedef struct {
	uint16_t pdi_control; // 0x000
	uint16_t pdi_configuration; // 0x001
	uint16_t sync_impulse_len; // 0x002
	uint16_t pdi_configuration2;  // 0x003
	uint16_t alias; // 0x004  /**< The slaves alias if not equal to 0. */
	uint8_t  reverved1[4]; // 0x005
	uint16_t checksum; //0x007
	uint32_t vendor_id; //0x008 	/**< Vendor-ID stored on the slave. */
	uint32_t product_code; // 0x00a	   /**< Product-Code stored on the slave. */
	uint32_t revision_number; // 0x00c /**< Revision-Number stored on the slave. */
	uint32_t serial_number; // 0x00e  /**< Serial-Number stored on the slave. */
	uint8_t  reserved2[8]; ///0x013
	uint16_t boot_rx_mailbox_offset; // 0x0014
	uint16_t boot_rx_mailbox_size; // 0x0015
	uint16_t boot_tx_mailbox_offset; // 0x0016
	uint16_t boot_tx_mailbox_size; // 0x0017
	uint16_t std_rx_mailbox_offset; //0x0018
	uint16_t std_rx_mailbox_size; // 0x0019
	uint16_t std_tx_mailbox_offset; // 0x01a
	uint16_t std_tx_mailbox_size; // 0x01b
	uint16_t mailbox_protocols; // 0x01c
	uint8_t  reserved3[66]; // 0x2b
	uint16_t eprom_size_kbits; // 0x03e
	uint16_t version; //0x03f
} ec_sii_t;

#define GROUP_IDX		0
#define IMAGE_IDX		1
#define ORDER_IDX		2
#define NAME_IDX		3
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
#define TX_PDO2_NAME		"TXPDO2 LIBIX"
#define RX_PDO1_NAME		"RXPDO1 LIBIX"
#define RX_PDO2_NAME		"RXPDO2 LIBIX"

#define SDO_ENABLED 		0x001
#define	SDO_INFO		0x002
#define	PDO_ASSIGN		0x004
#define	PDO_CONF		0x008
#define	STARTUP_UPLOAD		0x010
#define	SDO_COMPLETE_ACCESS	0x020

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

#define NR_STRINGS  10
#define PORT_MII	0x01
#define PORT0_SHIFT	0
#define NR_PDOS		2

#define STRINGS_SIZE ( sizeof(STRING9) + \
		sizeof(STRING8) + sizeof(STRING7) + sizeof(STRING6) \
		+ sizeof(STRING5) + sizeof(STRING4) + sizeof(STRING3) \
		+ sizeof(STRING2) + sizeof(STRING1) + sizeof(STRING0) \
		+ (NR_STRINGS * sizeof(uint8_t)))


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
	int8_t syncm;
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

typedef uint16_t category_fmmu;
category_fmmu fmmu;

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

typedef struct __category_strings__{
	uint8_t nr_strings;
	uint8_t str1_len;
	char str1[sizeof(STRING1)];
	uint8_t str2_len;
	char str2[sizeof(STRING2)];
	uint8_t str3_len;
	char str3[sizeof(STRING3)];
	uint8_t str4_len;
	char str4[sizeof(STRING4)];
	uint8_t str5_len;
	char str5[sizeof(STRING5)];
	uint8_t str6_len;
	char str6[sizeof(STRING6)];
	uint8_t str7_len;
	char str7[sizeof(STRING7)];
	uint8_t str8_len;
	char str8[sizeof(STRING8)];
	uint8_t str9_len;
	char str9[sizeof(STRING9)];
	uint8_t pad[(((STRINGS_SIZE)/2)*2 + 2) - STRINGS_SIZE];
} category_strings;

typedef struct {
	uint16_t type:15;
	uint16_t vendor_specific:1;
	uint16_t size;
} category_header;

typedef struct {
	ec_sii_t sii __attribute__ ((packed));

	category_header strings_hdr __attribute__ ((packed));
	category_strings strings;

	category_header general_hdr __attribute__ ((packed));
	category_general general __attribute__ ((packed));

	category_header fmmu_hdr __attribute__ ((packed));
	category_fmmu fmmu;

	category_header syncm_hdr0 __attribute__ ((packed));
	category_syncm syncm0  __attribute__ ((packed));

	category_header syncm_hdr1 __attribute__ ((packed));
	category_syncm syncm1  __attribute__ ((packed));

	category_header txpdo_hdr __attribute__ ((packed));
	category_pdo txpdo  __attribute__ ((packed));

	category_header rxpdo_hdr __attribute__ ((packed));
	category_pdo rxpdo  __attribute__ ((packed));

	category_header endhdr __attribute__ ((packed));

} sii_categories;

sii_categories categories;
int last_word_offset = -1;

void write_category_hdr(int off,int datalen, uint8_t *data)
{
	ec_printf("%s off%d \n",__FUNCTION__,off);
}

void read_category_hdr(int off,int datalen, uint8_t *data)
{
	int offset = off*2;
	uint8_t* cat_off = (uint8_t *)&categories;

	ec_printf("%s off %d offset %d data len=%d\n",
			__FUNCTION__,off, offset ,datalen);

	if (offset + datalen > sizeof(categories)){
		ec_printf("%s insane offset offset=%d "
					"datalen=%d sizeof cat %d\n",
				__FUNCTION__,
				offset ,datalen,
				sizeof(categories));
		return;
	}
	memcpy(data, &(cat_off[offset]), datalen);
}

void init_general(category_general * general,category_header * hdr)
{
	hdr->size = sizeof(*general)/2;

	if (sizeof(*general) %2){
		ec_printf("%s illegal size\n",__FUNCTION__);
		exit(0);
	}

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
// table 23
void init_syncm(category_syncm *syncm,int index,category_header * hdr)
{
	hdr->size = ( sizeof(category_syncm)) / 2;
	if (sizeof(*syncm) %2){
		ec_printf("ilegal size\n");
		exit(0);
	}
	hdr->type = CAT_TYPE_SYNCM;

	syncm->length = SYNMC_SIZE;
	syncm->ctrl_reg = 0b00110010;  
	syncm->status_reg = 0b00001000; /*b1000 - 1-buf written,b0000 1-buf read */
	syncm->enable_syncm = 0b01;
	syncm->syncm_type = index % 2 ? 0x03:0x04; /* 0x03 = out*/
	syncm->phys_start_address = (long)categories.sii.std_tx_mailbox_offset; 
	if(syncm->syncm_type == 0x04)
		syncm->phys_start_address = (long)categories.sii.std_rx_mailbox_offset; 
	
}

void toggle_rw_bit(category_syncm *syncm)
{
	if (syncm->status_reg & 0b1000) {
		syncm->status_reg = 0b0000;
	} else{
		syncm->status_reg = 0b1000;
	}
}

void ec_sii_syncm(int reg, uint8_t* data, int datalen)
{
	category_syncm *syncm;

	switch(reg)
	{
		case ECT_REG_SM0:
			syncm = &categories.syncm0;
			break;
		default:
		case ECT_REG_SM1:
			syncm = &categories.syncm1;
	}
	printf("%s reg=%d\n",__FUNCTION__,reg);
	toggle_rw_bit(syncm);
	memcpy(data, syncm, datalen);
}

void init_fmmu(category_fmmu *fmmu,category_header *hdr)
{
	hdr->size = sizeof(category_fmmu) / 2;
	if (sizeof(*fmmu) %2){
		ec_printf("ilegal size\n");
		exit(0);
	}

	hdr->type = CAT_TYPE_FMMU;
	fmmu = 0;	// fmmmu not used
}

void init_end_hdr(category_header * hdr)
{
	hdr->size = 0;
	hdr->type = 0x7FFF;
	hdr->vendor_specific = 0x1; // etherlab does not care for vendor
}

void init_strings(category_strings * str, category_header * hdr)
{
	hdr->size = sizeof(*str) / 2;

	if (sizeof(*str) %2){
		ec_printf("ilegal size\n");
		exit(0);
	}
	
	str->nr_strings = NR_STRINGS;
	hdr->type = CAT_TYPE_STRINGS;

	str->str1_len = sizeof(STRING1);
	strncpy(str->str1, STRING1, str->str1_len);

	str->str2_len = sizeof(STRING2);
	strncpy(str->str2, STRING2, str->str2_len);

	str->str3_len = sizeof(STRING3);
	strncpy(str->str3, STRING2, str->str3_len);

	str->str4_len = sizeof(STRING4);
	strncpy(str->str4, STRING2, str->str4_len);

	str->str5_len = sizeof(STRING5);
	strncpy(str->str5, STRING2, str->str5_len);

	str->str6_len = sizeof(STRING6);
	strncpy(str->str6, STRING2, str->str6_len);

	str->str7_len = sizeof(STRING7);
	strncpy(str->str7, STRING2, str->str7_len);

	str->str8_len = sizeof(STRING8);
	strncpy(str->str8, STRING8, str->str8_len);

	str->str9_len = sizeof(STRING9);
	strncpy(str->str9, STRING9, str->str9_len);
}

void init_pdo(pdo_entry * pdo,
	      uint16_t index,
	      uint8_t subindex,
	      uint8_t name_idx,
	      uint8_t data_type, uint8_t bit_len, uint16_t flags)
{
	pdo->bit_len = bit_len;
	pdo->data_type = data_type;
	pdo->flags = flags;
	pdo->index = index;
	pdo->name_idx = name_idx;
	pdo->subindex = subindex;
}

void init_hdr_dbg()
{
	int cat_off =0;
	ec_printf("sz = category pdo = %u\n",sizeof(category_pdo));

	ec_printf("%s sizes sii %u str=%d gen=%d "
			"tx=%d rx=%d fm=%d sync=%d end=%u\n",	__FUNCTION__,
			sizeof(categories.sii),
			categories.strings_hdr.size,
			categories.general_hdr.size,
			categories.txpdo_hdr.size,
			categories.rxpdo_hdr.size,
			categories.fmmu_hdr.size,
			categories.syncm_hdr0.size,
			categories.endhdr.size);

	cat_off = (uint8_t *)&categories.general_hdr - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);

	cat_off = (uint8_t *)&categories.txpdo_hdr - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);

	cat_off = (uint8_t *)&categories.rxpdo_hdr - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);

	cat_off = (uint8_t *)&categories.fmmu_hdr - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);

	cat_off = (uint8_t *)&categories.syncm_hdr0 - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);

	cat_off = (uint8_t *)&categories.syncm_hdr1 - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);

	cat_off = (uint8_t *)&categories.endhdr - (uint8_t *) &categories.sii;
	ec_printf("%s offset =%d\n",__FUNCTION__,cat_off/2);
}

void init_si_info(ec_sii_t *sii)
{
	memset(sii,0x00,sizeof(*sii));

	sii->alias = 0x04;
	sii->vendor_id = 0x1ee;
	sii->product_code = 0x0e;
	sii->revision_number = 0x12;
	sii->serial_number = 0x45;

	sii->boot_rx_mailbox_offset = __sdo_start() ; 
	sii->boot_rx_mailbox_size = MBOX_SIZE ; 
	sii->boot_tx_mailbox_offset = sii->boot_rx_mailbox_offset + MBOX_SIZE;
	sii->boot_tx_mailbox_size  = MBOX_SIZE; 
	sii->std_rx_mailbox_offset = sii->boot_tx_mailbox_offset + MBOX_SIZE;
	sii->std_rx_mailbox_size = MBOX_SIZE;
	sii->std_tx_mailbox_offset = sii->std_rx_mailbox_offset + MBOX_SIZE;
	sii->std_tx_mailbox_size =  MBOX_SIZE;
	
	printf("sii boot_rx_mailbox_offset %x\n", 
		sii->boot_rx_mailbox_offset);

	printf("sii->boot_tx_mailbox_offset %x\n",
	sii->boot_tx_mailbox_offset);

	printf("sii->std_rx_mailbox_offset %x\n",
		sii->std_rx_mailbox_offset);

	printf("sii->std_tx_mailbox_offset %x\n",
		sii->std_tx_mailbox_offset);

	sii->mailbox_protocols = EC_MBOX_COE;
}

void init_sii(void)
{
	init_si_info(&categories.sii);
	init_strings(&categories.strings, &categories.strings_hdr);
	init_fmmu(&categories.fmmu, &categories.fmmu_hdr);
	init_syncm(&categories.syncm0, 0 ,&categories.syncm_hdr0);
	init_syncm(&categories.syncm1, 1 ,&categories.syncm_hdr1);
	init_general(&categories.general, &categories.general_hdr);
	init_end_hdr(&categories.endhdr);

	// pdos
	categories.rxpdo_hdr.type = CAT_TYPE_RXPDO;
	categories.rxpdo_hdr.size = sizeof(categories.rxpdo)/2;
	categories.rxpdo.entries = 2;
	categories.rxpdo.flags = 0;
	categories.rxpdo.name_idx = RXPDO_CAT_NAME_IDX;
	categories.rxpdo.synchronization = 0;
	categories.rxpdo.syncm = 0;
	categories.rxpdo.pdo_index = 0x1600;
	
	init_pdo(&categories.rxpdo.pdo[0], 0x1614, 0X02, RX_PDO1_NAME_IDX, 0,
		 8, 0);

	init_pdo(&categories.rxpdo.pdo[1], 0x1748, 0X01, RX_PDO2_NAME_IDX, 0,
		 32, 0);

	categories.txpdo_hdr.type = CAT_TYPE_TXPDO;
	categories.txpdo.entries = 2;
	categories.txpdo.flags = 0;
	categories.txpdo.name_idx = TXPDO_CAT_NAME_IDX;
	categories.txpdo.synchronization = 0;
	categories.txpdo.syncm = 1;
	categories.txpdo.pdo_index = 0x1a00;
	categories.txpdo_hdr.size = sizeof(categories.txpdo)/2;

	init_pdo(&categories.txpdo.pdo[0], 0x1a01, 0X02, TX_PDO1_NAME_IDX, 0,	// index in the object dictionary
		 32, 0);

	init_pdo(&categories.txpdo.pdo[1], 0x1a03, 0X01, TX_PDO2_NAME_IDX, 0,	// index in the object dictionary
		 16, 0);
	init_hdr_dbg();
}

void (*sii_command)(int offset, int datalen, uint8_t * data) = 0;

void ec_sii_rw(uint8_t * data, int datalen)
{
	if (sii_command){
			ec_printf("%s datalen =%d\n",__FUNCTION__,datalen);
			sii_command(last_word_offset, datalen - 6, (uint8_t *)&data[6]);
	} else{
		ec_printf("%s no command\n",__FUNCTION__);
	}
	sii_command = 0;
	last_word_offset = -1;
}

int ec_sii_start_read(uint8_t * data, int datalen)
{
	int word_offset;

	ec_printf("%s received data len %d\n",
			__FUNCTION__, datalen);

	if (data[0] != 0x80 && data[0] != 0x81) {
		ec_printf("%s no two addressed octets %x %x\n",
		       __FUNCTION__	, data[0], data[1]);
		return 1;
	}
	word_offset = *(uint16_t *) & data[2];
	ec_printf("%s request %d operation on offset %d\n",
			__FUNCTION__, data[1], word_offset);

	switch(data[1])
	{
	case 0x01: // read
		last_word_offset = word_offset;
		sii_command = read_category_hdr;
		break;

	case 0x02: // write
		last_word_offset = word_offset;
		sii_command = write_category_hdr;
		break;

	default: // unknown
		ec_printf("%s default\n",__FUNCTION__);
		break;
	}

	return 0;
}
