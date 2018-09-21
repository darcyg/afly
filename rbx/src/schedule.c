#include "schedule.h"
#include <stdio.h>
#include <stdlib.h>

static stSchduleTask_t * task_list = NULL;

long schedue_current() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static stSchduleTask_t * schedue_search(stSchduleTask_t * at) {
	stSchduleTask_t *curr = task_list;
	
	while (curr != NULL) {
		if (curr == at) {
			break;
		}
		curr = curr->next;
	}

	return curr;
}

static stSchduleTask_t * _schedue_add(stSchduleTask_t *at) {
	stSchduleTask_t *prev = task_list;
	
	while (prev != NULL && prev->next != NULL) {
		prev = prev->next;
	}

	if (prev == NULL) {
		task_list = at;
	} else {
		prev->next = at;
	}

	at->next = NULL;

	return at;
}

void schedue_add(stSchduleTask_t *at, long ms, void *func, void *arg) {
	
	if (schedue_search(at) == NULL) {
		_schedue_add(at);
	}

	at->func = func;
	at->arg = arg;
	at->start = schedue_current();
	at->delt = ms;

}

stSchduleTask_t *schedue_first_task_to_exec() {
	stSchduleTask_t *curr = task_list;
	stSchduleTask_t *min  = curr;

	
	while (curr != NULL) {
		if (curr->start + curr->delt <= min->start + min->delt) {
			min = curr;
		}
		curr = curr->next;
	}
	
	return min;
}

long schedue_first_task_delay() {
	stSchduleTask_t *min = schedue_first_task_to_exec();
	if (min == NULL) {
		return -1;
	}
	long ret = min->start + min->delt - schedue_current();

	if (ret < 0) {
		ret = 0; //10ms
	}
	
	return ret;
}


void schedue_del(stSchduleTask_t *at) {
	stSchduleTask_t *prev = NULL;
	stSchduleTask_t *curr = task_list;

	while (curr != NULL) {
		if (curr == at) {
			break;
		}
	
		prev = curr;
		curr = curr->next;
	}

	if (curr == at && curr != NULL) {
		if (prev == NULL) {
			task_list = curr->next;
		} else {
			prev->next = curr->next;
		}
	}
	
}


