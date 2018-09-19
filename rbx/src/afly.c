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


void _afly_init(int loglvl);
void _afly_end();

static stAFlyEnv_t env = {0};

//////////////////////////////////////////////////////////////////////////
int		afly_init(void *_th, void *_fet, int loglvl) {
	env.th = _th;
	env.fet = _fet;
	
	timer_init(&env.step_timer, afly_handler_run);

	lockqueue_init(&env.msgq);

	_afly_init(loglvl);

	afly_reg((unsigned char*)"\x01\x02\x03\x04\x05\x06\x07\x08", 1102, (char *)"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", "sign", 0);
	afly_upt_online((unsigned char *)"\x01\x02\x03\x04\x05\x06\x07\x08", 1);

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

int		afly_handler_event(stEvent_t *event) {
	log_info("[%d] afly module now not support event handler, only free the event!!!", __LINE__);
	return 0;
}


///////////////////////////// AFly Lock Callback //////////////////////////////////////////////
static int subdev_get_property(char *in, char *out, int out_len, void *ctx) {
    log_info("in : %s", in);
		stSubDevCfg_t *sdc = (stSubDevCfg_t*)sdc;

    return 0;
}

static int subdev_set_property(char *in, void *ctx) {

    log_info("in : %s", in);
		stSubDevCfg_t *sdc = (stSubDevCfg_t*)sdc;

    return 0;
}

static int subdev_call_service(char *identifier, char *in, char *out, int out_len, void *ctx) {
    log_info("in : %s", in);

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

#if 0
    lock_t *lock = ctx;
    linkkit_gateway_post_property_json_sync(lock->devid, "{\"SetTimer\": \"hello, world!\"}", 5000);
#endif

    return 0;
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
			} else {
				;
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
		log_warn("not support service : %s", identifier);
    return -1;
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
#if 0
    cJSON *pJson = cJSON_CreateObject();

    if (!pJson) {
        return -1;
		}

    cJSON_AddNumberToObject(pJson, "ZB_Band", gw->ZB_Band);
    cJSON_AddNumberToObject(pJson, "ZB_Channel", gw->ZB_Channel);
    cJSON_AddStringToObject(pJson, "ZB_CO_MAC", gw->ZB_CO_MAC);
    cJSON_AddStringToObject(pJson, "ZB_PAN_ID", gw->ZB_PAN_ID);
    cJSON_AddStringToObject(pJson, "EXT_PAN_ID", gw->EXT_PAN_ID);
    cJSON_AddStringToObject(pJson, "NETWORK_KEY", gw->NETWORK_KEY);

    char *p = cJSON_PrintUnformatted(pJson);
    if (!p) {
        cJSON_Delete(pJson);
        return -1;
    }

    log_info("property: %s", p);

    linkkit_gateway_post_property_json_sync(gw->lk_dev, p, 5000);

    cJSON_Delete(pJson);
    free(p);
#endif

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
void  afly_reg(unsigned char ieee_addr[IEEE_ADDR_BYTES],  unsigned int model_id, const char rand[SUBDEV_RAND_BYTES], const char *sign, int supe) {
	char ieee_addr_str[32];
	hex_string(ieee_addr_str, sizeof(ieee_addr_str), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	char rand_str[64];
	hex_string(rand_str, sizeof(rand_str), (u8*)rand, SUBDEV_RAND_BYTES, 0, 0);

	char model_id_str[32];
	sprintf(model_id_str, "%d", model_id);

	log_info("registering [%s] with model_id:%08x, rand:%s, sign:%s", ieee_addr_str, model_id, rand_str, sign);

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
		log_warn("linkkit gateway subdev register failed: ret(%d) key(%s),name(%s), secret(%s)",
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
}

void	afly_unreg(unsigned char ieee_addr[IEEE_ADDR_BYTES]) {
	char ieee_addr_str[32];
	hex_string(ieee_addr_str, sizeof(ieee_addr_str), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	log_info("unregister : %s", ieee_addr_str);

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
}

void	afly_upt_online(unsigned char ieee_addr[IEEE_ADDR_BYTES], char online_or_not) {
	char ieee_addr_str[32];
	hex_string(ieee_addr_str, sizeof(ieee_addr_str), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	log_info("update online : %s->%d", ieee_addr_str, online_or_not);

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
}

void	afly_rpt_attrs(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *attr_name[], const char *attr_value[]) {
	log_info("report attr : Name:%s, Value:%s", attr_name[0], attr_value[0]);

	/*
	int ret = afly_zigbee_report_attrs((unsigned char *)ieee_addr, endpoint_id, attr_name, attr_value);
	log_info("report ret : %d", ret);


		linkkit_gateway_post_property_json_sync
		linkkit_gateway_post_property_json()
	*/
}

void	afly_rpt_event(unsigned char ieee_addr[IEEE_ADDR_BYTES], unsigned char endpoint_id, const char *event_name, const char *event_args) {
	log_info("report cmd : %s", event_name);

	/*
	int ret = afly_zigbee_report_event((unsigned char *)ieee_addr, endpoint_id,  event_name, event_args);
	log_info("report ret : %d", ret);

	linkkit_gateway_trigger_event_json_sync();
	linkkit_gateway_trigger_event_json();
	*/
}


