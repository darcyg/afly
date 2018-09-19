#ifndef __AFLY_H_
#define __AFLY_H_

#include "timer.h"
#include "file_event.h"
#include "lockqueue.h"

#include "linkkit.h"
#include "product.h"
#include "product_sub.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum emAFlyMsg{
	afly_NONE 					= 0x00,
	afly_TIMER_CHECK 	= 0x01,
}emAFlyMsg_t;


typedef struct stAFlyEnv {
	struct file_event_table *fet;
	struct timer_head *th;
	struct timer step_timer;

	stLockQueue_t msgq;

	stGateway_t gw;
}stAFlyEnv_t;


int		afly_init(void *_th, void *_fet, int loglvl);
int		afly_push_msg(int eid, void *param, int len);

int		afly_step();
void	afly_handler_run(struct timer *timer);

void	afly_pipe_in(void *arg, int fd);

int		afly_handler_event(stEvent_t *event);

void  afly_reg(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned int model_id, const char rand[SUBDEV_RAND_BYTES], const char *sign, int supe);
void	afly_unreg(unsigned char ieee_addr[IEEE_ADDR_BYTES]);
void	afly_upt_online(unsigned char ieee_addr[IEEE_ADDR_BYTES], char online_or_not);
void	afly_rpt_attrs(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *attr_name[], const char *attr_value[]);
void	afly_rpt_event(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *event_name, const char *event_args);

#ifdef __cplusplus
}
#endif

#endif