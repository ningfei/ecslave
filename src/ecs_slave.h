#ifndef __ECS_SLAVE_H__
#define __ECS_SLAVE_H__


struct __e_slave__;
struct fsm_slave;

typedef struct __ec_interface__ {
	int index;
#ifndef __KERNEL__
	struct sockaddr_in m_addr;
	struct ifreq ifr;
	struct ether_header mac;
#endif
	char name[16];
	char ip[32];
	char macaddr[32];
	int sock;
	int subnet_mask;
	int link_up;
}ec_interface;

typedef struct {
	void (*state)(struct __e_slave__ *,uint8_t *,int);
}fsm_mbox;

typedef struct {
	uint16_t obj_index;
	uint8_t obj_subindex;
}fsm_coe; 

typedef struct __e_slave__ {
	
	uint8_t *pkt_head;
	uint8_t *dgram_processed; /* current ethercat dgram processed */
	uint8_t dgrams_cnt;
	int pkt_size;
	int trigger_latch;
	int interfaces_nr;
	ec_interface* intr[EC_MAX_PORTS];
	struct fsm_slave *fsm;	/* finite state machine */
	fsm_coe  coe;
	fsm_mbox mbox;
} e_slave;

int  ecs_net_init(int ,char *argv[], e_slave *);
int  ecs_init(e_slave *);
void ecs_run(e_slave *);

#define HTYPE_ETHER     	0x1	/* Ethernet  */
#define ETHERCAT_TYPE 		0x88a4

void ec_cmd_apwr(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_armw(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_aprw(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_aprd(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_fprd(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_frmw(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_fpwr(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_fprw(e_slave * slave, uint8_t *ecdgram);
void ec_cmd_brw(e_slave * slave,  uint8_t *ecdgram);
void ec_cmd_bwr(e_slave * slave,  uint8_t *ecdgram);
void ec_cmd_brd(e_slave * slave,  uint8_t *ecdgram);
void ec_cmd_nop(e_slave * slave,  uint8_t *ecdgram);
void ec_cmd_lrd(e_slave * slave,  uint8_t *ecdgram);
void ec_cmd_lrw(e_slave * slave,  uint8_t *ecdgram);
void ec_cmd_lwr(e_slave * slave,  uint8_t *ecdgram);

void ecs_process_next_dgram(e_slave * slave,  uint8_t *ecdgram);
int  ec_nr_dgrams(uint8_t *raw_pkt);

/* d points at start of datagram.  */
static inline uint8_t *__ec_dgram_data(uint8_t *d)
{
	return (uint8_t *)&d[sizeof(ec_dgram)];
}

/* d points at start of datagram.  */
static inline int __ec_dgram_command(uint8_t * d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	return datagram->command;
}

static inline int __ec_is_last_dgram(uint8_t *d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	return datagram->dlength & 0b1000000000000000;
}
/* d points at start of datagram.  */
static inline uint16_t __ec_dgram_dlength(uint8_t *d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	uint16_t datalen = (datagram->dlength & 0x07FF);
	return datalen;
}

/* d points at start of datagram.  */
static inline uint16_t __ec_dgram_adp(uint8_t * d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	return (datagram->adp);
}

/* d points at start of datagram.  */
static inline void __ec_dgram_set_adp(uint8_t *d,uint16_t adp)
{
	ec_dgram *datagram = (ec_dgram *)d;
	datagram->adp = adp;
}

/* d points at start of datagram.  */
static inline uint32_t __ec_dgram_laddr(uint8_t * d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	return ((uint32_t) datagram->adp | (uint32_t) (datagram->ado << 16));
}

/* d points at start of datagram.  */
/* address offset */
static inline uint16_t __ec_dgram_ado(uint8_t * d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	return datagram->ado;
}

/* d points at start of datagram.  */
static inline uint8_t __ec_dgram_pkt_index(uint8_t * d)
{
	ec_dgram *datagram = (ec_dgram *)d;
	return datagram->index;
}

static inline uint8_t *__ec_get_shost(uint8_t* h)
{
	struct ether_header *eh = (struct ether_header *)h;
	return (uint8_t *)&(eh->ether_shost[0]);
}

static inline int __ec_is_ethercat(uint8_t * h)
{
	struct ether_header *eh = (struct ether_header *)h;
	return htons(eh->ether_type) == ETHERCAT_TYPE;
}

/* d points at start of datagram.  */
static inline void __ec_inc_wkc__(uint8_t *d)
{
	uint16_t wkc1;
	uint16_t size =  __ec_dgram_dlength(d);
	uint16_t *wkc = (uint16_t *)&d[size + sizeof(ec_dgram)];

	wkc1 = *wkc;
	wkc1++;
	*wkc = wkc1;
}

static inline uint16_t __ec_wkc(uint8_t *d)
{
	uint16_t size =  __ec_dgram_dlength(d);
	uint16_t *wkc = (uint16_t *)&d[size + sizeof(ec_dgram)];
	return *wkc;
}

/* h points to mac address */
static inline uint8_t *__ecat_frameheader(uint8_t *h)
{
	return (uint8_t *) & h[sizeof(struct ether_header)];
}

/* d is on mac address */
static inline uint16_t __ec_frame_size(uint8_t * d)
{
	ec_frame_header *ec_frame = (ec_frame_header *) __ecat_frameheader(d);
	return (ec_frame->elength & 0x07FF); /* old compilers do not like binary numbers*/
}

#endif
