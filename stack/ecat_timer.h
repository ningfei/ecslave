#ifndef  __ECAT_TIMER_H__
#define __ECAT_TIMER_H__

struct ecat_event {
        long* private;	
	void (*action)(void *);
        LIST_ENTRY(ecat_event) list;
};

void ecat_schedule_event(void *private,struct ecat_event *, void (*action)(void *));
void ecat_create_timer(void);
void ecat_wake_timer(void);


#endif
