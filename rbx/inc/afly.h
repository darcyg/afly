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

	struct timer sync_list_timer;

	stLockQueue_t msgq;

	stGateway_t gw;
}stAFlyEnv_t;


typedef struct stAflyService {
	char *app;
	char *model;
	char *name;
	int  (*func)(void *arg, char *in, char *out, int out_len, void *ctx);
} stAflyService_t;


int		afly_init(void *_th, void *_fet, int loglvl, char *dbfile);
int		afly_push_msg(int eid, void *param, int len);

int		afly_step();
void	afly_handler_run(struct timer *timer);
void	afly_handler_sync_list_run(struct timer *timer);

void	afly_pipe_in(void *arg, int fd);

int		afly_handler_event(stEvent_t *event);

void  afly_z3_reg(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned int model_id, const char rand[SUBDEV_RAND_BYTES], const char *sign, int supe);
void	afly_z3_unreg(unsigned char ieee_addr[IEEE_ADDR_BYTES]);
void	afly_z3_upt_online(unsigned char ieee_addr[IEEE_ADDR_BYTES], char online_or_not);
void	afly_z3_rpt_attrs(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *attr_name[], const char *attr_value[]);
void	afly_z3_rpt_event(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *event_name, const char *event_args);


enum {
	EVENT_NXP_NONE					= 0x00,
	EVENT_NXP_REBOOT				= 0x01,
	EVENT_NXP_FACTORY_RESET = 0x02,
	EVENT_NXP_CHECK_RECORD  = 0x03,
	EVENT_NXP_ADD_DEL_PASS_RET = 0x04,
	EVENT_NXP_MOD_PASS_RET	= 0x05,
	EVENT_NXP_DAMAGE_ALARM	= 0x06,
	EVENT_NXP_SYS_STATUS		= 0x07,
};
void afly_nxp_reg(const char *name, const char *model, const char *type,
				 const char *version, int battery, int online, int rssi);
void afly_nxp_unreg(const char *name);
void afly_nxp_upt_online(const char *name, int online, const char *type, int battery,
					int passNum, int passAll, int cardNum, int cardAll, int fingNum, int fingAll, int rssi);
void afly_nxp_rpt_attrs();
void afly_nxp_rpt_event(const char *name, int eid, char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif
