#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "log.h"
#include "hex.h"
#include "platform.h"
#include "jansson.h"
#include "json_parser.h"


#include "uproto.h"
#include "zigbee.h"
#include "afly.h"


static stUdrvEnv_t	ue;
static struct blob_buf b;

//static int usync_flag = 0;
static const char *usync_mac  = NULL;
static const char *usync_attr = NULL;
static json_t *usync_ret  = NULL;

static int uproto_general_handler(int eid, void *param);
static int  uproto_send_ubus_msg_handler(int eid, void *param);
static UDRV_HANDLER uhs[] = {
	[0x00] = uproto_send_ubus_msg_handler,
};


/* Funcstion for cmd */
static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static int uproto_cmd_handler_attr_rpt(const char *uuid, const char *cmdmac, const char *attr, json_t *value);
static stUprotoCmd_t ucmds[] = {
	{"getAttribute", uproto_cmd_handler_attr_get},
	{"setAttribute", uproto_cmd_handler_attr_set},
	{"reportAttribute", uproto_cmd_handler_attr_rpt},
};



/* Functions for attr */
/**> ZB3 */
static int rpt_zclcmd(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_atr(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_register(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_unregister(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_online(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
//static int rpt_netinfo(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);


/**> Nxp */
static int get_nxp_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_nxp_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_nxp_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int rpt_nxp_new_device_added(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_device_deleted(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_lock_reboot(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_factory_reset(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_nxp_lock_add_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_nxp_lock_del_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_nxp_lock_add_card(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_nxp_lock_del_card(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int rpt_nxp_lock_local_add_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_lock_local_del_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_nxp_lock_del_all_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_lock_check_record(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_lock_pass_adddel_result(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_nxp_lock_mod_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int rpt_nxp_lock_pass_mod_result(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int rpt_nxp_lock_damage_alarm(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static int set_nxp_del_fing(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);
static int set_nxp_add_fing(const char *uuid, const char *cmdmac,  const char *attr, json_t *value);

static stUprotoAttrCmd_t uattrcmds[] = {
	// Z3 :
	//{"ember.zb3.permit",						NULL,									set_permit,					NULL},
	//{"ember.zb3.remove",						NULL,									set_remove,					NULL},
	{"ember.zb3.zclcmd",							NULL,										NULL,								rpt_zclcmd},
	{"ember.zb3.atr",									NULL,										NULL,								rpt_atr},
	//{"ember.zb3.netinfo",							NULL,										NULL,							rpt_netinfo},

	/* wlist status get, enable, disable */
	//{"ember.zb3.wlist.sts",					get_wlist_state,			set_wlist_state,		NULL},
	/* wlist add, del, get */
	//{"ember.zb3.wlist",							get_wlist,						set_wlist,					NULL},

	/* wlist contain some mac */
	//{"ember.zb3.wlist.contain",			get_wlist_contain,		NULL,								NULL},

	{"ember.zb3.register",						NULL,									NULL,								rpt_register},
	{"ember.zb3.unregister",					NULL,									NULL,								rpt_unregister},
	{"ember.zb3.online",								NULL,									NULL,								rpt_online},

	//{"ember.zb3.list",							get_list,							NULL,								NULL},
	//{"ember.zb3.inspect",						get_inspect,					NULL,								NULL},
	

	// Nxp :
	{"mod.device_list",								get_nxp_list,					NULL,								rpt_nxp_list},
	{"mod.add_device",								NULL,									set_nxp_add_device,	NULL},
	{"mod.del_device",								NULL,									set_nxp_del_device,	NULL},
	{"mod.new_device_added",					NULL,									NULL,								rpt_nxp_new_device_added},
	{"mod.device_deleted",						NULL,									NULL,								rpt_nxp_device_deleted},
	{"device.lock.reboot",						NULL,									NULL,								rpt_nxp_lock_reboot},
	{"device.factory_reset",					NULL,									NULL,								rpt_nxp_factory_reset},
	{"device.status",									NULL,									NULL,								rpt_nxp_status},
	{"device.lock_add_password",			NULL,									set_nxp_lock_add_pass,	NULL},
	{"device.lock_del_password",			NULL,									set_nxp_lock_del_pass,	NULL},
	{"device.lock_add_ic_card",				NULL,									set_nxp_lock_add_card,	NULL},
	{"device.lock_del_ic_card",				NULL,									set_nxp_lock_del_card,	NULL},
	{"device.lock_local_add_password",NULL,									NULL,								rpt_nxp_lock_local_add_pass},
	{"device.lock_local_del_password",NULL,									NULL,								rpt_nxp_lock_local_del_pass},
	{"device.lock.del_all_password",  NULL,									set_nxp_lock_del_all_pass, NULL},
	{"device.lock.check_record",			NULL,									NULL,								rpt_nxp_lock_check_record},
	{"device.lock.password_adddel_result", NULL,						NULL,								rpt_nxp_lock_pass_adddel_result},
	{"device.lock.modify_password",		NULL,									set_nxp_lock_mod_pass,	NULL},
	{"device.lock.password_modify_result", NULL,						NULL,								rpt_nxp_lock_pass_mod_result},
	{"device.lock.damage_alarm",			NULL,									NULL,								rpt_nxp_lock_damage_alarm},
	{"device.lock.del_finger_print",	NULL,									set_nxp_del_fing,				NULL},
	{"device.lock.add_finger_print",	NULL,									set_nxp_add_fing,				NULL},
};


static int		uproto_ubus_send(void *_jmsg) {
	json_t *jmsg = (json_t *)_jmsg;

	json_object_del(jmsg, "to");
	json_object_set_new(jmsg, "to", json_string("NXP"));
	char *smsg= json_dumps(jmsg, 0);
	if (smsg != NULL) {
		blob_buf_init(&b, 0);
		blobmsg_add_string(&b, "PKT", smsg);
		/**> TODO determine the dest through the dest segment */
		ubus_send_event(ue.ubus_ctx, UPROTO_EVENT_ID_REPORT_NXP, b.head);
		free(smsg);
	}

	json_object_del(jmsg, "to");
	json_object_set_new(jmsg, "to", json_string("ZB3"));
	smsg= json_dumps(jmsg, 0);
	if (smsg != NULL) {
		blob_buf_init(&b, 0);
		blobmsg_add_string(&b, "PKT", smsg);
		/**> TODO determine the dest through the dest segment */
		ubus_send_event(ue.ubus_ctx, UPROTO_EVENT_ID_REPORT, b.head);
		free(smsg);
	}

	return 0;
}


static int		uproto_step() {
	timer_cancel(ue.th, &ue.step_timer);
	timer_set(ue.th, &ue.step_timer, 10);
	return 0;
}

static int		uproto_push_msg(int eid, void *param, int len) {
	stEvent_t *e = MALLOC(sizeof(stEvent_t));
	if (e == NULL) {
		return -1;
	}
	e->eid = eid;
	e->param = param;
	lockqueue_push(&ue.msgq, e);
	uproto_step();
	return 0;
}

static int		uproto_handler_event(stEvent_t *e) {
	if (e == NULL) {
		return 0;
	}

	UDRV_HANDLER func = NULL;
	if (!(e->eid >= 0 && e->eid <= sizeof(uhs)/sizeof(uhs[0]))) {
		func = uproto_general_handler;
	} else {
		func = uhs[e->eid];
	}

	func(e->eid, e->param);
	
	return 0;
}

static void		uproto_run(struct timer *timer) {
	stEvent_t *e = NULL;
	if (lockqueue_pop(&ue.msgq, (void **)&e) && e != NULL) {
	pthread_mutex_lock(&ue.mutex);
		uproto_handler_event(e);
	pthread_mutex_unlock(&ue.mutex);
		FREE(e);
		uproto_step();
	}
	return ;
}



static void uproto_ubus_in(void *arg, int fd) {
	pthread_mutex_lock(&ue.mutex);
	ubus_handle_event(ue.ubus_ctx);
	pthread_mutex_unlock(&ue.mutex);
	return;
}

static stUprotoCmd_t *uproto_search_ucmd(const char *command) {
	int i;
	for (i = 0; i < sizeof(ucmds)/sizeof(ucmds[0]); i++) {
		if (strcmp(command, ucmds[i].name) == 0) {
			return &ucmds[i];
		}
	}
	return NULL;
}
static stUprotoAttrCmd_t *uproto_search_uattrcmd(const char *attr) {
	int i;
	for (i = 0; i < sizeof(uattrcmds)/sizeof(uattrcmds[0]); i++) {
		if (strcmp(attr, uattrcmds[i].name) == 0) {
			return &uattrcmds[i];
		}
	}
	return NULL;
}

static int _uproto_handler_cmd(const char *from, 
		const char *to,
		const char *ctype,
		const char *mac, 
		int dtime, 
		const char *uuid,
		const char *command,	
		const char *cmdmac,
		const char *attr,
		json_t *value) {
	stUprotoCmd_t *ucmd = uproto_search_ucmd(command);	
	if (ucmd == NULL) {
		log_warn("not support command:%s", command);
		return -1;
	}


	log_info("handler name : %s", ucmd->name);
	return ucmd->handler(uuid, cmdmac, attr, value);
}

int uproto_handler_ubus_cmd_alink(const char *cmd) {
	json_error_t error;
	json_t *jpkt = json_loads(cmd, 0, &error);
	if (jpkt == NULL) {
		log_warn("error: on line %d: %s", error.line, error.text);
		goto parse_jpkt_error;
	}


	const char *from = json_get_string(jpkt, "from");
	const char *to = json_get_string(jpkt, "to");
	/* CLOUD, GATEWAY, NXP, GREENPOWER BLUETOOTH <ZB3> */
	if (strcmp(from, "ZB3") != 0 && strcmp(from, "NXP") != 0 ) {
		log_warn("now not support ubus source : %s", from);
		goto parse_jpkt_error;
	}
	if (strcmp(to, UPROTO_ME) != 0) {
		log_warn("now not support ubus dest : %s", to);
		goto parse_jpkt_error;
	}


	/* registerReq, registerRsp, reportAttribute, reportAttributeResp cmd cmdResult */
	const char *ctype = json_get_string(jpkt, "type");
	//if (strcmp(ctype, "cmd") != 0) {
	if (strcmp(ctype, "cmdResult") != 0 && strcmp(ctype, "reportAttribute") != 0) {
		log_warn("now not support ubus type : %s", ctype);
		goto parse_jpkt_error;
	}

	const char *mac = json_get_string(jpkt, "mac");
	int   dtime = 0; json_get_int(jpkt, "time", &dtime);

	/* verify jdata */
	json_t	*jdata = json_object_get(jpkt, "data");
	if (jdata == NULL) {
		log_warn("not find data item!");
		goto parse_jpkt_error;
	}

#if 0
	const char *uuid = json_get_string(jdata, "id");
	const char *command = json_get_string(jdata, "command");
	json_t *jarg = json_object_get(jdata, "arguments");
	if (jarg == NULL) {
		log_warn("not find arguments!");
		goto parse_jpkt_error;
	}
#else
	const char *uuid = "null";
	const char *command = ctype;
#endif

	const char *cmdmac = json_get_string(jdata, "mac");
	const char *attr   = json_get_string(jdata, "attribute");
	json_t *    value  = json_object_get(jdata, "value");


	log_info("from:%s,to:%s,type:%s,time:%d,,uuid:%s,cmdmac:%s, attr:%s",
			from, to, ctype, dtime, uuid, cmdmac, attr);

	if (strcmp(ctype, "reportAttribute") == 0 && usync_attr != NULL && 
			strcmp(attr, usync_attr) == 0 && usync_ret == NULL) {
		/* fill result */
		if (usync_mac != NULL) {
			if (strcmp(cmdmac, usync_mac) == 0) {
				usync_ret = json_deep_copy(value);
			} else {
				_uproto_handler_cmd(from, to, ctype, mac, dtime, uuid, command, cmdmac, attr, value);
			}
		} else {
			usync_ret = json_deep_copy(value);
		}
	} else {
		_uproto_handler_cmd(from, to, ctype, mac, dtime, uuid, command, cmdmac, attr, value);
	}

parse_jpkt_error:
	if (jpkt != NULL) json_decref(jpkt);
	return -1;
}
static void uproto_handler_ubus_event(struct ubus_context *ctx,struct ubus_event_handler *ev, 
		const char *type,struct blob_attr *msg) {
	char *str;

	log_info("-----------------[ubus msg]: handler ....-----------------");
	str = blobmsg_format_json(msg, true);
	if (str != NULL) {
		log_info("[ubus msg]: [%s]", str);

		json_error_t error;
		json_t *jmsg = json_loads(str, 0, &error);
		if (jmsg != NULL) {
			const char *spkt = json_get_string(jmsg, "PKT");
			if (spkt != NULL) {
				log_info("pks : %s", spkt);
				uproto_handler_ubus_cmd_alink(spkt);
			} else {
				log_warn("not find 'PKT' item!");
			}
			json_decref(jmsg);
		} else {
			log_warn("error: on line %d: %s", error.line, error.text);
		}
		free(str);
	} else {
		log_warn("[ubus msg]: []");
	}
	log_info("-----------------[ubus msg]: handler over-----------------");
}

static int uproto_response_ucmd(const char *uuid, int retval) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
	const char *to					= "ZB3";
	const char *deviceCode	= "00000";
	const char *type				= "cmdResult";
	int ctime								= platform_get_time(); 
	char mac[32];             platform_get_hw_id(mac, sizeof(mac));
	char macstr[32];
	hex_string(macstr, sizeof(macstr), (u8*)mac, 8, 0, 0);

	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(macstr));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));

	json_t *jdata = json_object();
	json_object_set_new(jdata, "id",	 json_string(uuid));
	json_object_set_new(jdata, "code", json_integer(retval));
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);


	return 0;
}


static int uproto_report_umsg(const char *submac, const char *attr, json_t *jret) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
	const char *to					= "ZB3";
	const char *deviceCode	= "00000";
	const char *type				= "reportAttribute";
	int ctime								= platform_get_time();
	char mac[32];             platform_get_hw_id(mac, sizeof(mac));
	char macstr[32];
	hex_string(macstr, sizeof(macstr), (u8*)mac, 8, 0, 0);


	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(macstr));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));

	json_t *jdata = json_object();
	json_object_set_new(jdata, "attribute", json_string(attr));
	//char submac[32];				
	json_object_set_new(jdata, "mac", json_string(submac));
	json_object_set_new(jdata, "value", jret);
	json_object_set_new(jumsg, "data", jdata);

	uproto_push_msg(UE_SEND_MSG, jumsg, 0);

	return 0;
}

static json_t *uproto_make_msg(const char *submac, const char *attr, const char *operation, void *jvalue) {
	json_t *jumsg = json_object();

	const char *from				= UPROTO_ME;
	const char *to					= "ZB3";
	const char *deviceCode	= "00000";
	const char *type				= "cmd";
	int ctime								= platform_get_time(); 
	char mac[32];             platform_get_hw_id(mac, sizeof(mac));
	char macstr[32];
	hex_string(macstr, sizeof(macstr), (u8*)mac, 8, 0, 0);

	json_object_set_new(jumsg, "from", json_string(from));
	json_object_set_new(jumsg, "to", json_string(to));
	json_object_set_new(jumsg, "deviceCode", json_string(deviceCode));
	json_object_set_new(jumsg, "mac", json_string(macstr));
	json_object_set_new(jumsg, "type", json_string(type));
	json_object_set_new(jumsg, "time", json_integer(ctime));
	json_t *jdata = json_object(); json_object_set_new(jumsg, "data", jdata); {
		json_object_set_new(jdata, "id",				json_string(attr));
		json_object_set_new(jdata, "command",		json_string(operation));
		json_t *jarg = json_object(); json_object_set_new(jdata, "arguments", jarg);{
			if (submac == NULL) {
				json_object_set_new(jarg, "mac",				json_string(macstr));
			} else {
				json_object_set_new(jarg, "mac",				json_string(submac));
			}
			json_object_set_new(jarg, "attribute",	json_string(attr));
			json_object_set_new(jarg, "value",			jvalue);
		}
	}
	return jumsg;
}

static void *uproto_wait_response(const char *mac, const char *attr, int timeout) {

	usync_mac = mac;
	usync_attr = attr;
	if (usync_ret != NULL) {
		json_decref(usync_ret);
		usync_ret = NULL;
	}

	struct timeval tv = {timeout/1000, (timeout%1000) * 1000};
	do {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(ue.ubus_ctx->sock.fd, &fds);

		int maxfd = ue.ubus_ctx->sock.fd;

		int ret = select(maxfd + 1, &fds, NULL, NULL, &tv);
		if (ret < 0) {
			break;
		}

		if (ret == 0) {
			break;
		}

		if (!FD_ISSET(ue.ubus_ctx->sock.fd, &fds)) {
			continue;
		}

		ubus_handle_event(ue.ubus_ctx);

		if (usync_ret != NULL) {
			break;
		}

		//timeout = tv.tv_sec * 1000 + tv.tv_usec /1000;

	} while (1);

	usync_mac = NULL;
	usync_attr = NULL;

	json_t *jret = usync_ret;
	usync_ret = NULL;
	

	return jret;
}

// uproto init /////////////////////////////////////////////////////////////////////
int		uproto_init(void *_th, void *_fet) {
	ue.fet = _fet;
	ue.th = _th;

	timer_init(&ue.step_timer, uproto_run);

	lockqueue_init(&ue.msgq);

	ue.ubus_ctx = ubus_connect(NULL);
	memset(&ue.listener, 0, sizeof(ue.listener));
	ue.listener.cb = uproto_handler_ubus_event;

	ubus_register_event_handler(ue.ubus_ctx, &ue.listener, UPROTO_EVENT_ID_LISTEN);
	ubus_register_event_handler(ue.ubus_ctx, &ue.listener, UPROTO_EVENT_ID_LISTEN_NXP);
	file_event_reg(ue.fet, ue.ubus_ctx->sock.fd, uproto_ubus_in, NULL, NULL);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&ue.mutex, &attr);
	pthread_mutexattr_destroy(&attr);

	return 0;
}


// uproto handler /////////////////////////////////////////////////////////////////
static int uproto_general_handler(int eid, void *param) {
	log_warn("[%d] not support event:%d", __LINE__, eid);
	return 0;
}
static int  uproto_send_ubus_msg_handler(int eid, void *param) {
	if (param == NULL) {
		return -1;
	}

	json_t *jmsg = (json_t*)param;
	uproto_ubus_send(jmsg);
	json_decref(jmsg);
	
	return 0;
}

// uproto ubus cmd handler /////////////////////////////////////////////////////// 
static int uproto_cmd_handler_attr_get(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	log_info("handler name : %s", uattrcmd->name);
	if (uattrcmd->get == NULL) {
		log_warn("get function is null!!!");
		return -2;
	}
	return uattrcmd->get(uuid, cmdmac, attr, value);
}
static int uproto_cmd_handler_attr_set(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	if (uattrcmd->set == NULL) {
		log_warn("set function is null!!!");
		return -2;
	}
	return uattrcmd->set(uuid, cmdmac, attr, value);
}

static int uproto_cmd_handler_attr_rpt(const char *uuid, const char *cmdmac, const char *attr, json_t *value) {
	stUprotoAttrCmd_t *uattrcmd = uproto_search_uattrcmd(attr);	
	if (uattrcmd == NULL) {
		log_warn("not support attribute:%s", attr);
		return -1;
	}
	if (uattrcmd->rpt == NULL) {
		log_warn("rpt function is null!!!");
		return -2;
	}
	return uattrcmd->rpt(uuid, cmdmac, attr, value);
}

// uproto ubus attribute handlder ////////////////////////////////////////////////
static int uproto_hexstr_to_int(const char *hexstr) {
	char buf[4];
	hex_parse((u8*)buf, sizeof(buf), hexstr, 0);

	int x = 0;
	x = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (buf[0]);

	return x;
}
static int rpt_register(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	char *sval = json_dumps(value, 0);
	log_info("[%d] uuid:%s, cmdmac:%s, attr:%s, value:%s", __LINE__, uuid, cmdmac, attr, sval);
	if (sval != NULL) {
		free(sval);
	}

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), cmdmac, 0);

	const char *type = json_get_string(value, "type");
	if (type == NULL) {
		type = "0000";
	}

	const char *modelstr = json_get_string(value, "model");
	int model = 0;
	if (modelstr != NULL) {
		model = uproto_hexstr_to_int(modelstr);
	}

	char rand[SUBDEV_RAND_BYTES];
	memset(rand, 0, sizeof(rand));
	const char *randstr	= json_get_string(value, "rand");
	if (randstr != NULL) {
		hex_parse((u8*)rand, sizeof(rand), randstr, 0);
	}

	const char *sign = NULL;
	const char *signstr	= json_get_string(value, "sign");
	if (signstr != NULL) {
		sign = signstr;
	}

	int supe = 0;
	const char *supestr = json_get_string(value, "supe");
	if (supestr != NULL) {
		supe = uproto_hexstr_to_int(supestr);
	}

	//zigbee_rpt_reg(mac, type, model, rand, sign, supe);

	return 0;
}
static int rpt_unregister(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	char *sval = json_dumps(value, 0);
	log_info("[%d] uuid:%s, cmdmac:%s, attr:%s, value:%s", __LINE__, uuid, cmdmac, attr, sval);
	if (sval != NULL) {
		free(sval);
	}

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), cmdmac, 0);
	//zigbee_rpt_unreg(mac);

	return 0;
}

static long schedue_current() {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}


static int rpt_online(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	char *sval = json_dumps(value, 0);
	log_info("[%d] uuid:%s, cmdmac:%s, attr:%s, value:%s", __LINE__, uuid, cmdmac, attr, sval);
	if (sval != NULL) {
		free(sval);
	}

	char mac[32];
	hex_parse((u8*)mac, sizeof(mac), cmdmac, 0);

	int last = 0; json_get_int(value, "last", &last);

	if (last == 0) {
		return 0;
	}

	int now = schedue_current();

	char online = !((now - last) > 1000*60*60*2);

	//zigbee_rpt_online(mac, online);

	return 0;
}

static int rpt_zclcmd(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	char *sval = json_dumps(value, 0);
	log_info("[%d] uuid:%s, cmdmac:%s, attr:%s, value:%s", __LINE__, uuid, cmdmac, attr, sval);
	if (sval != NULL) {
		free(sval);
	}

	const char *epstr		= json_get_string(value, "endpoint");
	const char *clsstr	= json_get_string(value, "cluster");
	const char *valstr	= json_get_string(value, "value");
	if (epstr == NULL || clsstr == NULL || valstr == NULL) {
		log_warn("null ep/cluster/value!!!");
		return -1;
	}
	char ep = 0x00;
	short cluster = 0x0000;
	char buf[64];
	char ieee[16];
	hex_parse((u8*)&ep,			sizeof(ep),				epstr,		0);
	hex_parse((u8*)&cluster, sizeof(cluster),	clsstr,	0);
	int len = hex_parse((u8*)buf,			sizeof(buf),			(const char *)valstr,		0);
	hex_parse((u8*)ieee,			sizeof(ieee),			cmdmac, 0);

	log_info("ep:%02X, cluster:%04X, ieee:%02X%02X...\n", ep&0xff, cluster&0xffff, ieee[0]&0xff, ieee[1]&0xff);
	if (len < 3) {
		log_warn("value len < 3 bytes!!!");
		return -2;
	}

	char cmdid = buf[2]&0xff;

	//zigbee_rpt_event(ieee, ep, cluster, cmdid, buf + 3, len - 3);

	return 0;
}
static int rpt_atr(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	char *sval = json_dumps(value, 0);
	log_info("[%d] uuid:%s, cmdmac:%s, attr:%s, value:%s", __LINE__, uuid, cmdmac, attr, sval);
	if (sval != NULL) {
		free(sval);
	}

	const char *epstr		= json_get_string(value, "endpoint");
	const char *clsstr	= json_get_string(value, "cluster");
	const char *valstr	= json_get_string(value, "data");
	if (epstr == NULL || clsstr == NULL || valstr == NULL ) {
		log_warn("null ep/cluster/attrid/value!!!");
		return -1;
	}
	log_info("epstr:%s, clsstr:%s, valstr:%s", epstr, clsstr, valstr);
	char ep = 0x00;
	short cluster = 0x0000;
	char buf[64];
	char ieee[16];

	hex_parse((u8*)&ep,			sizeof(ep),				epstr,		0);
	hex_parse((u8*)&cluster, sizeof(cluster),	clsstr,	0);
	int len = hex_parse((u8*)buf,			sizeof(buf),			(const char *)valstr,		0);
	hex_parse((u8*)ieee,			sizeof(ieee),			cmdmac, 0);

	log_info("ep:%02X, cluster:%04X, ieee:%02X%02X...\n", ep&0xff, cluster&0xffff, ieee[0]&0xff, ieee[1]&0xff);
	if (len < 4) {
		log_warn("value len < 4 bytes!!!");
		return -2;
	}

	short attrid = buf[0] | (buf[1] << 8);

	//zigbee_rpt_attrs(ieee, ep, cluster, attrid, buf + 3, len - 3);


	return 0;
}

/**> Nxp */
static int rpt_nxp_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}

static int get_nxp_list(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}

static int set_nxp_add_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int set_nxp_del_device(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}

static int rpt_nxp_new_device_added(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int rpt_nxp_device_deleted(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int rpt_nxp_lock_reboot(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int rpt_nxp_factory_reset(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int rpt_nxp_status(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}

static int set_nxp_lock_add_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int set_nxp_lock_del_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int set_nxp_lock_add_card(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int set_nxp_lock_del_card(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}

static int rpt_nxp_lock_local_add_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int rpt_nxp_lock_local_del_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int set_nxp_lock_del_all_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int rpt_nxp_lock_check_record(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}
static int rpt_nxp_lock_pass_adddel_result(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}

static int set_nxp_lock_mod_pass(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int rpt_nxp_lock_pass_mod_result(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}

static int rpt_nxp_lock_damage_alarm(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	return 0;
}

static int set_nxp_del_fing(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}
static int set_nxp_add_fing(const char *uuid, const char *cmdmac,  const char *attr, json_t *value) {
	/**> TODO */
	return 0;
}



// uproto sync interface /////////////////////////////////////////////////////////
void	*uproto_call(const char *mac, const char *attr, const char *operation, void *jvalue, int timeout) {
	log_info("[%d] mac:%s, attr:%s, operation:%s", __LINE__, mac, attr, operation);

	pthread_mutex_lock(&ue.mutex);
	
	json_t *jcmd = uproto_make_msg(mac, attr, operation, jvalue);
	uproto_ubus_send(jcmd);
	json_decref(jcmd);

	json_t *jret = NULL;
	if (timeout > 0) {
		jret = uproto_wait_response(mac, attr, timeout);
	}

	pthread_mutex_unlock(&ue.mutex);

	log_info("over!");
	return jret;
}


