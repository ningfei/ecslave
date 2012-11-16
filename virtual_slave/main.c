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
#include "slaves.h"
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

// Timer
static unsigned int sig_alarms = 0;
static unsigned int user_alarms = 0;

/****************************************************************************/

// process data
static uint8_t *domain1_pd = NULL;

#define AnaInSlavePos1 0, 0
#define AnaInSlavePos2 0, 1

#define LIBIX_VP 0x000001ee, 0x0000000e /* LIBIX_VP = VENDOR PRODUCT */

// offsets for PDO entries

static unsigned int off_ana_in[2]={-1};
static unsigned int off_ana_out[2]={-1};

const static ec_pdo_entry_reg_t domain1_regs[] = {
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, 0x02, &off_ana_out[0]},
    {AnaInSlavePos2,  LIBIX_VP, 0x1600, 0x02, &off_ana_in[0]},
    {AnaInSlavePos1,  LIBIX_VP, 0x1a00, 0x02, &off_ana_out[1]},
    {AnaInSlavePos2,  LIBIX_VP, 0x1600, 0x02, &off_ana_in[1]},
    {}
};

static unsigned int counter = 0;
static unsigned int blink = 0;

/*****************************************************************************/

void check_domain1_state(void)
{
    ec_domain_state_t ds;

    ecrt_domain_state(domain1, &ds);

    if (ds.working_counter != domain1_state.working_counter)
        printf("Domain1: WC %u.\n", ds.working_counter);
    if (ds.wc_state != domain1_state.wc_state)
        printf("Domain1: State %u.\n", ds.wc_state);

    domain1_state = ds;
}

void cyclic_task()
{
    // receive process data
    ecrt_master_receive(master);
    ecrt_domain_process(domain1);
    
    check_domain1_state();
    if (counter) {
        counter--;
    } else{ // do this at 1 Hz
        counter = FREQUENCY;
        // calculate new process data
        blink = !blink;
    }
    EC_WRITE_U8(domain1_pd + off_ana_out[0] , counter);
    EC_WRITE_U8(domain1_pd + off_ana_out[1] , counter);
    printf("READ FROM SLAVES %d %d\n",
		EC_READ_U8(domain1_pd + off_ana_in[0] ),
    		EC_READ_U8(domain1_pd + off_ana_in[1] ) );
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
    ec_slave_config_t *sc1;
    ec_slave_config_t *sc2;
    struct sigaction sa;
    struct itimerval tv;
    
    master = ecrt_request_master(0);
    if (!master)
        return -1;

    domain1 = ecrt_master_create_domain(master);
    if (!domain1)
        return -1;

    if (!(sc1 = ecrt_master_slave_config(
                    master, AnaInSlavePos1, LIBIX_VP))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }
    if (!(sc2 = ecrt_master_slave_config(
                    master, AnaInSlavePos2, LIBIX_VP))) {
        fprintf(stderr, "Failed to get slave configuration.\n");
        return -1;
    }

    printf("Configuring PDOs...\n");
    if (ecrt_slave_config_pdos(sc1, EC_END, slave_0_syncs)) {
        fprintf(stderr, "Failed to configure PDOs.\n");
        return -1;
    }

    if (ecrt_slave_config_pdos(sc2, EC_END, slave_1_syncs)) {
        fprintf(stderr, "Failed to configure PDOs.\n");
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

    printf("Offsets in=%d,%d out=%d,%d\n",
	off_ana_in[0], off_ana_in[1],
	off_ana_out[0], off_ana_out[1]);

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
