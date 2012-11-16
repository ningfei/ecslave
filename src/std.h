
typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef unsigned int        uint32_t;
typedef signed long long int int64_t;
typedef unsigned long long int uint64_t;

#ifdef __KERNEL__

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

#define EC_MAX_PORTS 2

#endif
