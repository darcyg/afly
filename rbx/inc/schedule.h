#ifndef _AMBER_SCHEDUE_H_
#define _AMBER_SCHEDUE_H_

typedef struct stSchduleTask {
	void			*func;
	void			*arg;
	long  		start;
	long  		delt;
	
	struct stSchduleTask *next;
}stSchduleTask_t;


void schedue_add(stSchduleTask_t *at, long ms, void *func, void *arg);

stSchduleTask_t *schedue_first_task_to_exec();

long schedue_first_task_delay();

void schedue_del(stSchduleTask_t *at);

long schedue_current();

#endif
