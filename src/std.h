
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;

#define EC_MAX_PORTS 2

#ifdef __KERNEL__

#include <linux/if_ether.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/module.h>

struct ether_header
{
  u_int8_t  ether_dhost[ETH_ALEN];	/* destination eth addr	*/
  u_int8_t  ether_shost[ETH_ALEN];	/* source ether addr	*/
  u_int16_t ether_type;		        /* packet type ID field	*/
} __attribute__ ((__packed__));


#else

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <string.h>
#include "ethercattype.h"
#include <net/ethernet.h>
#include <arpa/inet.h>


#endif

