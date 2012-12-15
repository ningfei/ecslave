#ifndef  __ECAT_TIMER_H__
#define  __ECAT_TIMER_H__

struct ecat_event {
        long* private;	
	void (*action)(void *);
        LIST_ENTRY(ecat_event) list;
};

struct ec_device;

void ecat_add_event_to_device(struct ec_device *, 
		struct ecat_event *, 
		void (*action)(void *), 
		void *private);
void ecat_schedule_event(void *private,struct ecat_event *, void (*action)(void *));
void ecat_create_timer(void);
void ecat_wake_timer(void);
void ecat_calibrate_localtime(uint32_t *systime32);
void ecat_set_rx_time(void *p);
uint32_t ecat_local_time(void);

#endif
