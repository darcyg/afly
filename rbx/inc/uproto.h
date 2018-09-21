#ifndef __UPROTO_H_
#define __UPROTO_H_

#include "timer.h"
#include "file_event.h"
#include "lockqueue.h"

#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubus.h>
#include <jansson.h>



#define UPROTO_EVENT_ID_LISTEN "DS.EMBER.RES&RPT"
#define UPROTO_EVENT_ID_REPORT "DS.EMBER.COMMAND"


#define UPROTO_EVENT_ID_LISTEN_NXP "DS.GATEWAY"
#define UPROTO_EVENT_ID_REPORT_NXP "DS.NXP"
#define UPROTO_ME							 "CLOUD"

enum {
	UE_SEND_MSG = 0x00,
};

typedef struct stUdrvEnv {
	struct file_event_table *fet;
	struct timer_head *th;

	struct timer step_timer;

	stLockQueue_t msgq;

	pthread_mutex_t mutex;
	struct ubus_context *ubus_ctx;
	struct ubus_event_handler listener;
	struct ubus_event_handler listenerNxp;
}stUdrvEnv_t;


int		uproto_init(void *_th, void *_fet);

typedef int (*UDRV_HANDLER)(int eid, void *param);



typedef int (*UPROTO_HANDLER)(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
typedef struct stUprotoCmd {
	const char *name;
	UPROTO_HANDLER handler;
}stUprotoCmd_t;

typedef int (*UPROTO_CMD_GET)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef int (*UPROTO_CMD_SET)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef int (*UPROTO_CMD_RPT)(const char *uuid, const char * cmdmac, const char *attr, json_t *value);
typedef struct stUprotoAttrCmd {
	const char *name;
	UPROTO_CMD_GET get;
	UPROTO_CMD_SET set;
	UPROTO_CMD_RPT rpt;
}stUprotoAttrCmd_t;

void	*uproto_call(const char *mac, const char *attr, const char *operation, void *jvalue, int timeout);
#endif
