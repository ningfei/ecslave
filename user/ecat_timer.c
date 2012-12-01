#include "xgeneral.h"
#include "ethercattype.h"
#include "ecs_slave.h"
#include "ec_mbox.h"
#include "ec_net.h"
#include "ec_sii.h"
#include "ecat_timer.h"
#include "ec_regs.h"

#include <semaphore.h>
#include <pthread.h>

static sem_t timersem;
static int ecat_timer_run  = 0;
static int interval_ns = 0;
static pthread_mutex_t timer_sync;

LIST_HEAD(events_head, ecat_event) ecat_events;
struct events_head* headp;

void *ecat_timer(void *dummy __attribute__ ((unused)) )
{
	struct ecat_event *ev;
	ecat_timer_run = 1;
	sem_wait(&timersem);

	while(1) {
		pthread_mutex_lock(&timer_sync);
		while (LIST_FIRST(&ecat_events) != NULL) {
		      	ev = LIST_FIRST(&ecat_events);
			ev->action(ev->private);
			LIST_REMOVE(LIST_FIRST(&ecat_events), list);
		}
		pthread_mutex_unlock(&timer_sync);
	}
}

void  ecat_schedule_event(void *private,struct ecat_event *event, void (*action)(void *))
{
	event->action = action;
	event->private = private;
	pthread_mutex_lock(&timer_sync);
        LIST_INSERT_HEAD(&ecat_events, event, list);
	pthread_mutex_unlock(&timer_sync);
}

void ecat_create_timer(void)
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_mutexattr_t mta;

	if (ecat_timer_run){
		return;
	}
	sem_init(&timersem, 0, 0);
	pthread_mutexattr_init(&mta);
	pthread_mutex_init(&timer_sync, &mta);
	LIST_INIT(&ecat_events);
	pthread_create(&thread , &attr, ecat_timer, 0);
}

void ecat_wake_timer(void)
{
	if (ecat_timer_run == 0){
		printf("Insane 980 before 9a0\n");
		return;
	}
	interval_ns = ecat_cyclic_interval_ns();
	printf("interval %dns\n", interval_ns);
	if (interval_ns < 1000000){
		printf("WARNING: user space ecat cannot "
			"handle less than 1ms intervals");
	}
	sem_post(&timersem);
}
