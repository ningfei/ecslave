#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/****************************************************************************/

#include "ecrt.h"

/****************************************************************************/

// Application parameters
#define FREQUENCY 100
#define PRIORITY 1

// Optional features
#define CONFIGURE_PDOS  1
#define SDO_ACCESS      0

/****************************************************************************/

// EtherCAT
static ec_master_t *master = NULL;
static ec_master_state_t master_state = {};

static ec_domain_t *domain1 = NULL;
static ec_domain_state_t domain1_state = {};

static ec_slave_config_t *sc_ana_in = NULL;
static ec_slave_config_state_t sc_ana_in_state = {};

// Timer
static unsigned int sig_alarms = 0;
static unsigned int user_alarms = 0;

/****************************************************************************/

// process data
static uint8_t *domain1_pd = NULL;

#define BusCouplerPos  0, 0
#define DigOutSlavePos 0, 2
#define AnaInSlavePos  0, 3
#define AnaOutSlavePos 0, 4

#define LIBIX_VP 0x000001ee, 0x0000000e /* LIBIX_VP = VENDOR PRODUCT */

// offsets for PDO entries
static unsigned int off_ana_in_status;
static unsigned int off_ana_in_value;
static unsigned int off_ana_out;
static unsigned int off_dig_out;

const static ec_pdo_entry_reg_t domain1_regs[] = {
    {AnaInSlavePos,  LIBIX_VP, 0x1614,0x02, &off_ana_in_status},
    {AnaInSlavePos,  LIBIX_VP, 0x1748,0x01, &off_ana_in_value},
    {}
};

static unsigned int counter = 0;
static unsigned int blink = 0;

/*****************************************************************************/


/* Master 0, Slave 0, "LIBIX_VP ORDER"
 * Vendor ID:       0x000001ee
 * Product code:    0x0000000e
 * Revision number: 0x00000012
 */

ec_pdo_entry_info_t slave_0_pdo_entries[] = {
    {0x1614, 0x02, 8}, /* RXPDO1 LIBIX_VP */
    {0x1748, 0x01, 32}, /* RXPDO1 LIBIX_VP */
    {0x1a01, 0x02, 32}, /* LIBIX_VP ORDER */
    {0x1a03, 0x01, 16}, /* LIBIX_VP ORDER */
};

ec_pdo_info_t slave_0_pdos[] = {
    {0x1600, 2, slave_0_pdo_entries + 0}, /* LIBIX_VP ORDER */
    {0x1a00, 2, slave_0_pdo_entries + 2}, /* LIBIX_VP ORDER */
};

ec_sync_info_t slave_0_syncs[] = {
    {0, EC_DIR_INPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
    {1, EC_DIR_INPUT, 1, slave_0_pdos + 1, EC_WD_DISABLE},
    {0xff}
};

void cyclic_task()
{
    int i;

    // receive process data
    ecrt_master_receive(master);
    ecrt_domain_process(domain1);

    if (counter) {
        counter--;
    } else { // do this at 1 Hz
        counter = FREQUENCY;
        // calculate new process data
        blink = !blink;
    }
#if 0
    // read process data
    printf("AnaIn: state %u value %u\n",
            EC_READ_U8(domain1_pd + off_ana_in_status),
            EC_READ_U16(domain1_pd + off_ana_in_value));
#endif

#if 1
    // write process data
    EC_WRITE_U8(domain1_pd + off_dig_out, blink ? 0x06 : 0x09);
#endif

    // send process data
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);
}

void signal_handler(int signum) 
{
    switch (signum) {
        case SIGALRM:
            sig_alarms++;
            break;
    }
}

int main(int argc, char **argv)
{
    ec_slave_config_t *sc;
    struct sigaction sa;
    struct itimerval tv;
    
    master = ecrt_request_master(0);
    if (!master)
        return -1;

    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
        return -1;

    if (!(sc_ana_in = ecrt_master_slave_config(
                    master, AnaInSlavePos, LIBIX_VP))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }

    printf("Configuring PDOs...\n");
    if (ecrt_slave_config_pdos(sc_ana_in, EC_END, slave_0_syncs)) {
        fprintf(stderr, "Failed to configure PDOs.\n");
        return -1;
    }

    if (!(sc = ecrt_master_slave_config(
                    master, AnaOutSlavePos, LIBIX_VP))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs)) {
        fprintf(stderr, "PDO entry registration failed!\n");
        return -1;
    }

    printf("Activating master...\n");
    if (ecrt_master_activate(master))
        return -1;

    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        return -1;
    }

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, 0)) {
        fprintf(stderr, "Failed to install signal handler!\n");
        return -1;
    }

    printf("Starting timer...\n");
    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = 1000000 / FREQUENCY;
    tv.it_value.tv_sec = 0;
    tv.it_value.tv_usec = 1000;
    if (setitimer(ITIMER_REAL, &tv, NULL)) {
        fprintf(stderr, "Failed to start timer: %s\n", strerror(errno));
        return 1;
    }

    printf("Started.\n");
    while (1) {
        pause();
        while (sig_alarms != user_alarms) {
            cyclic_task();
            user_alarms++;
        }
    }

    return 0;
}
