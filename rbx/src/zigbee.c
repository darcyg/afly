#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#include "common.h"
#include "log.h"
#include "jansson.h"
#include "hex.h"
#include "json_parser.h"

#include "zigbee.h"
#include "afly.h"
#include "afly_profile.h"
#include "product.h"
#include "product_sub.h"
#include "uproto.h"




static stZigbeeCache_t zc = {
	.panid 		= 0x1122,

	.extaddr 	= {
		0x01, 0x00, 0x1f, 0x23, 0x1b, 0x23, 0x42, 0x23,
	},

	.channel 	= 0x11,

	.netkey		= {
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 
		0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
	},

	.scene_mode = 0,

	.sound_alarm_enable = 0,
	.sound_alarm = 0,

	.light_alarm_enable = 0,
	.light_alarm = 0,

	.white_list_enable = 0,
	.white_list = NULL,
};

static char *zigbee_fmt_short2str(char *buf, char *fmt, short v) {
	sprintf(buf, fmt, v&0xffff);
	return buf;
}
static char *zigbee_fmt_int2str(char *buf, char *fmt, int v) {
	sprintf(buf, fmt, v);
	return buf;
}
static char *zigbee_fmt_ba2str(char *buf, char *fmt, char *data, int size) {
	int i = 0;
	int len = 0;
	for (i = 0; i < size; i++) {
		len += sprintf(buf + len, fmt, data[i]&0xff);
	}
	return buf;
}

static short zigbee_fmt_str2short(const char *buf, char *fmt, short *v) {
	sscanf(buf, fmt, v);
	return *v;
}
static int zigbee_fmt_str2int(const char *buf, char *fmt, int *i) {
	sscanf(buf, fmt, i);
	return *i;
}
static int zigbee_fmt_str2ba(const char *buf, char *fmt, char *data, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		sscanf(buf + i * 2, fmt, &data[i]);
	}
	return 0;
}
/* list */
int zigbee_list(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);
	buf_out[0] = 0;

	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.list", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	if (!json_is_array(jret)) {
		return -2;
	}

	/*
	int i = 0;
	json_t *jval;
	json_array_foreach(jret, i, jval) {
		const char *extaddr = json_get_string(jval, "extaddr");
	}
	*/
	const char *jdevs = json_dumps(jret, 0);
	if (jdevs != NULL) {
		strncpy(buf_out, jdevs, strlen(jdevs)+1);
	}

	json_decref(jret);

	return 0;
}

/* attr */
int zigbee_network_information_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);

	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.netinfo", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}

	int Channel = 0;						json_get_int(jret, "Channel", &Channel);
	int StackProfile = 0;				json_get_int(jret, "StackProfile", &StackProfile);
	const char *PanId					= json_get_string(jret, "PanId");
	const char *ChannelMask		= json_get_string(jret, "ChannelMask");
	const char *ExtAddr				= json_get_string(jret, "ExtAddr");
	const char *NetKey				= json_get_string(jret, "NetKey");

	log_info("Channel:%d,StackProfile:%d,PanId:%s,ChannlMask:%s,ExtAddr:%s,NetKey:%s",
						Channel, StackProfile, PanId, ChannelMask, ExtAddr, NetKey);

	char tmp[32];
	hex_parse((u8*)tmp, sizeof(tmp), PanId, 0);
	zc.panid = ((tmp[0]&0xff) << 8) | (tmp[1]&0xff);

	hex_parse((u8*)zc.extaddr, sizeof(zc.extaddr), ExtAddr, 0);

	zc.channel = Channel;

	hex_parse((u8*)zc.netkey, sizeof(zc.netkey), NetKey, 0);

	json_decref(jret);

	char buf[64];
	json_t *jout 	= json_object();
	//json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
	//json_t *val = json_object();
	json_object_set_new(jout, "PanId", 		json_string(zigbee_fmt_short2str(buf, "%04x", zc.panid&0xffff)));
	json_object_set_new(jout, "ExtPanId",	json_string(zigbee_fmt_ba2str(buf, "%02x", zc.extaddr, sizeof(zc.extaddr))));
	json_object_set_new(jout, "Channel",		json_string(zigbee_fmt_int2str(buf, "%d", zc.channel)));
	json_object_set_new(jout, "NetworkKey",json_string(zigbee_fmt_ba2str(buf, "%02x", zc.netkey, sizeof(zc.netkey))));
	//json_object_set_new(jout, "value", 			val);

	char *sout = json_dumps(jout, JSON_COMPACT);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}
	json_decref(jout);

	return 0;
}
int zigbee_network_information_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}

	json_t *jval = json_object_get(jin, "value");

	const char *sPanId 			= json_get_string(jval, "PanId");
	const char *sExtPanId		= json_get_string(jval, "ExtPanId");
	const char *sChannel		= json_get_string(jval, "Channel");
	const char *sNetworkKey = json_get_string(jval, "NetworkKey");

	zigbee_fmt_str2short(sPanId, "%04x", &zc.panid);
	zigbee_fmt_str2ba(sExtPanId, "%02x", zc.extaddr, 8);
	zigbee_fmt_str2int(sChannel, "%d", &zc.channel);
	zigbee_fmt_str2ba(sNetworkKey, "%02x", zc.netkey, 16);
	json_decref(jin);

	/* set the zigbee net information here */
	/* now not support */


	return -1;
}


int zigbee_scene_mode_get(char *buf_out, unsigned int buf_ze) {
	log_info("[%d]", __LINE__);

	buf_out[0] = 0;
	sprintf(buf_out, "%d", zc.scene_mode);
	return 0;

	/*
	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.scenemode", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	int scene_mode = 0; json_get_int(jret, "SceneMode", &scene_mode);
	json_decref(jret);
	*/


	char buf[32];
	json_t *jout = json_object();
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
	//json_t *jval = json_object();
	//json_object_set_new(jout, "value", 			jval);
	//json_object_set_new(jval, "SceneMode",	json_string(zigbee_fmt_int2str(buf, "%d", zc.scene_mode)));
	json_object_set_new(jout, "SceneMode",	json_string(zigbee_fmt_int2str(buf, "%d", zc.scene_mode)));
	
	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);

	return 0;
}
int zigbee_scene_mode_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}
	json_t *jval = json_object_get(jin, "value");
	const char *sSceneMode = json_get_string(jval, "SceneMode");
	zigbee_fmt_str2int(sSceneMode, "%d", &zc.scene_mode);
	json_decref(jin);

	/*
	json_t *jarg = json_object();
	json_object_set_new(jarg, "SceneMode", json_string(sSceneMode));
	uproto_call(NULL, "ember.zb3.scenemode", "setAttribute", jarg, 0);
	*/


	return 0;
}

int zigbee_sound_alarm_enable_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);
	buf_out[0] = 0;
	sprintf(buf_out, "%d", zc.sound_alarm_enable);
	return 0;

	/*
	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.soundalarmenable", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	int sound_alarm_enable = 0; json_get_int(jret, "SoundAlarmEnalbe", &sound_alarm_enable);
	json_decref(jret);
	*/

	char buf[32];
	json_t *jout = json_object();
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );

	//json_t *jval = json_object();
	//json_object_set_new(jout, "value", 			jval);
	//json_object_set_new(jval, "SoundAlarmEnable",	json_string(zigbee_fmt_int2str(buf, "%d", zc.sound_alarm_enable)));
	json_object_set_new(jout, "SoundAlarmEnable",	json_string(zigbee_fmt_int2str(buf, "%d", zc.sound_alarm_enable)));
	
	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);
	return 0;
}
int zigbee_sound_alarm_enable_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}
	json_t *jval = json_object_get(jin, "value");
	const char *sSoundAlarmEnable = json_get_string(jval, "SoundAlarmEnable");
	zigbee_fmt_str2int(sSoundAlarmEnable, "%d", &zc.sound_alarm_enable);
	json_decref(jin);

	/*
	json_t *jarg = json_object();
	json_object_set_new(jarg, "SoundAlarmEnable", json_string(sSoundAlarmEnable));
	uproto_call(NULL, "ember.zb3.soundalarmenable", "setAttribute", jarg, 0);
	*/

	return 0;
}


int zigbee_sound_alarm_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);
	buf_out[0] = 0;
	sprintf(buf_out, "%d", zc.sound_alarm);
	return 0;

	/*
	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.soundalarm", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	int sound_alarm = 0; json_get_int(jret, "SoundAlarmEnalbe", &sound_alarm);
	json_decref(jret);
	*/


	char buf[32];
	json_t *jout = json_object();
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
	
	//json_t *jval = json_object();
	//json_object_set_new(jout, "value", 			jval);
	//json_object_set_new(jval, "SoundAlarm",	json_string(zigbee_fmt_int2str(buf, "%d", zc.sound_alarm)));
	json_object_set_new(jout, "SoundAlarm",	json_string(zigbee_fmt_int2str(buf, "%d", zc.sound_alarm)));
	
	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);

	return 0;
}
int zigbee_sound_alarm_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}
	json_t *jval = json_object_get(jin, "value");
	const char *sSoundAlarm = json_get_string(jval, "SoundAlarm");
	zigbee_fmt_str2int(sSoundAlarm, "%d", &zc.sound_alarm);
	json_decref(jin);

	/*
	json_t *jarg = json_object();
	json_object_set_new(jarg, "SoundAlarmEnable", json_string(sSoundAlarm));
	uproto_call(NULL, "ember.zb3.soundalarm", "setAttribute", jarg, 0);
	*/

	return 0;
}



int zigbee_light_alarm_enable_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);
	buf_out[0] = 0;
	sprintf(buf_out, "%d", zc.light_alarm_enable);
	return 0;

	/*
	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.lightalarmenable", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	int light_alarm_enable = 0; json_get_int(jret, "LightAlarmEnalbe", &light_alarm_enable);
	json_decref(jret);
	*/

	char buf[32];
	json_t *jout = json_object();
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
	
	//json_t *jval = json_object();
	//json_object_set_new(jout, "value", 			jval);
	//json_object_set_new(jval, "LightAlarmEnable",	json_string(zigbee_fmt_int2str(buf, "%d", zc.light_alarm_enable)));
	json_object_set_new(jout, "LightAlarmEnable",	json_string(zigbee_fmt_int2str(buf, "%d", zc.light_alarm_enable)));

	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);
	return 0;
}
int zigbee_light_alarm_enable_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}
	json_t *jval = json_object_get(jin, "value");
	const char *sLightAlarmEnable = json_get_string(jval, "LightAlarmEnable");
	zigbee_fmt_str2int(sLightAlarmEnable, "%d", &zc.light_alarm_enable);
	json_decref(jin);

	/*
	json_t *jarg = json_object();
	json_object_set_new(jarg, "LightAlarmEnable", json_string(sLightAlarmEnable));
	uproto_call(NULL, "ember.zb3.lightalarmenable", "setAttribute", jarg, 0);
	*/



	return 0;
}



int zigbee_light_alarm_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);
	char buf[32];
	buf[0] = 0;
	sprintf(buf_out, "%d", zc.light_alarm);
	return 0;

	/*
	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.lightalarm", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	int light_alarm = 0; json_get_int(jret, "LightAlarm", &light_alarm);
	json_decref(jret);
	*/



	json_t *jout = json_object();
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
	
	//json_t *jval = json_object();
	//json_object_set_new(jout, "value", 			jval);
	//json_object_set_new(jval, "LightAlarm",	json_string(zigbee_fmt_int2str(buf, "%d", zc.light_alarm)));
	json_object_set_new(jout, "LightAlarm",	json_string(zigbee_fmt_int2str(buf, "%d", zc.light_alarm)));
	
	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);
	return 0;
}
int zigbee_light_alarm_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}
	json_t *jval = json_object_get(jin, "value");
	const char *sLightAlarm = json_get_string(jval, "LightAlarm");
	zigbee_fmt_str2int(sLightAlarm, "%d", &zc.light_alarm);
	json_decref(jin);


	/*
	json_t *jarg = json_object();
	json_object_set_new(jarg, "LightAlarm", json_string(sLightAlarm));
	uproto_call(NULL, "ember.zb3.lightalarm", "setAttribute", jarg, 0);
	*/


	return 0;
}


int zigbee_device_white_list_enable_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);
	buf_out[0] = 0;

	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.wlist.sts", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	int white_list_enable = 0; json_get_int(jret, "wlist_state", &white_list_enable);
	zc.white_list_enable = white_list_enable;
	json_decref(jret);


#if 0
	char buf[32];
	json_t *jout = json_object();
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
	
	json_t *jval = json_object();
	json_object_set_new(jout, "value", 			jval);
	json_object_set_new(jval, "WhiteListEnable",	json_string(zigbee_fmt_int2str(buf, "%d", zc.white_list_enable)));
	//json_object_set_new(jout, "WhiteListEnable",	json_string(zigbee_fmt_int2str(buf, "%d", zc.white_list_enable)));
	
	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);
#else
	sprintf(buf_out, "%d", zc.white_list_enable);	
#endif

	return 0;
}
int zigbee_device_white_list_enable_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	json_error_t error;
	json_t *jin = json_loads(json_in, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, json_in);
		return -1;
	}
	json_t *jval = json_object_get(jin, "value");
	const char *sWhiteListEnable = json_get_string(jval, "WhiteListEnable");
	zigbee_fmt_str2int(sWhiteListEnable, "%d", &zc.white_list_enable);
	json_decref(jin);

	json_t *jarg = json_object();
	json_object_set_new(jarg, "wlist_state", json_integer(zc.white_list_enable));
	uproto_call(NULL, "ember.zb3.wlist.sts", "setAttribute", jarg, 0);

	return 0;
}


int zigbee_device_white_list_get(char *buf_out, unsigned int buf_sz) {
	log_info("[%d]", __LINE__);

	buf_out[0] = 0;

	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "ember.zb3.wlist", "getAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}
	if (zc.white_list != NULL) {
		json_decref(zc.white_list);
		zc.white_list = NULL;
	}
	zc.white_list = json_deep_copy(jret);
	json_decref(jret);

#if 0
	char buf[32];
	json_t *jout = json_object();
	json_t *jay  = json_array();
	json_object_set_new(jout, "value", 			jay);
	json_object_set_new(jout, "when", 			json_string(zigbee_fmt_int2str(buf, "%d", time(NULL))) );
#else
	json_t *jout = zc.white_list;
	zc.white_list = NULL;
#endif
	
	char *sout = json_dumps(jout, 0);
	if (sout != NULL) {
		strcpy(buf_out, sout);
		free(sout);
		sout = NULL;
	}

	json_decref(jout);
	return 0;
}
int zigbee_device_white_list_set(char *json_in) {
	log_info("[%d] %s", __LINE__, json_in);

	return -1;
}


/* service */
int zigbee_add_device_wite_list(char *s_dev_list) {
	log_info("[%d] %s", __LINE__, s_dev_list);

	json_error_t error;
	json_t *jin = json_loads(s_dev_list, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, s_dev_list);
		return -1;
	}

	json_t *jsnSet = json_object_get(jin, "snSet");
	if (!json_is_array(jsnSet)) {
		log_warn("[%d] error json format: [%s]", __LINE__, s_dev_list);
		json_decref(jin);
		return -1;
	}
	json_t *jlist = json_deep_copy(jsnSet);
	json_decref(jin);

	json_t *jarg = json_object();
	json_object_set_new(jarg, "operation", json_string("add"));
	json_object_set_new(jarg, "wlist", jlist);
	json_t *jret = uproto_call(NULL, "ember.zb3.wlist", "setAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}

	json_decref(jret);

	return 0;
}
int zigbee_delete_device_wite_list(char *s_dev_list) {
	log_info("[%d] %s", __LINE__, s_dev_list);

	json_error_t error;
	json_t *jin = json_loads(s_dev_list, 0, &error);
	if (jin == NULL) {
		log_warn("[%d] error json format: [%s]", __LINE__, s_dev_list);
		json_decref(jin);
		return -1;
	}

	json_t *jsnSet = json_object_get(jin, "snSet");

	if (!json_is_array(jsnSet)) {
		log_warn("[%d] error json format: [%s]", __LINE__, s_dev_list);
		json_decref(jin);
		return -1;
	}
	json_t *jlist = json_deep_copy(jsnSet);
	json_decref(jin);


	json_t *jarg = json_object();
	json_object_set_new(jarg, "operation", json_string("del"));
	json_object_set_new(jarg, "wlist", jlist);
	json_t *jret = uproto_call(NULL, "ember.zb3.wlist", "setAttribute", jarg, 1500);
	if (jret == NULL) {
		log_warn("ubus no response %d", __LINE__);
		return -1;
	}

	json_decref(jret);


	return 0;
}

/* get/set/cmd */
int zigbee_is_support_cluster_attr(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, short clusterid, short attrid) {
	/* TODO */
	return 1;
}
int zigbee_is_support_cluster_cmd(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, short clusterid, char cmdid) {
	/* TODO */
	return 1;
}
int zigbee_get_attr(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, const char *attr_name) {
	char ieeestr[32];
	hex_string(ieeestr, sizeof(ieeestr), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	attr_profile_t *aps[10];
	int cnt = alink_attr_profile_search_by_attrname(attr_name, aps, sizeof(aps)/sizeof(aps[0]));
	if (cnt <= 0) {
		log_warn("not support attr: %s", attr_name);
		return -1;
	}
	
	int i = 0;
	for (i = 0; i < cnt; i++) {
		attr_profile_t *ap = aps[i];
		short cluster = ap->cluster_id;
		short attrid  = ap->attribute_id;
		if (!zigbee_is_support_cluster_attr(ieee_addr, endpoint_id, cluster, attrid)) {
			log_warn("device [%s] not support cluster:%04x, attr:%04x", ieeestr, cluster&0xffff, attrid&0xffff);
			continue;
		}

		char buf[32];
		json_t *jarg = json_object();
		json_object_set_new(jarg, "ep",				json_string(zigbee_fmt_ba2str(buf, "%02X", &endpoint_id, 1)) );
		json_object_set_new(jarg, "cluster",	json_string(zigbee_fmt_short2str(buf, "%04X", cluster)));
		json_object_set_new(jarg, "attrid",		json_string(zigbee_fmt_short2str(buf, "%04X", attrid)));
		uproto_call(ieeestr, "ember.zb3.atr", "getAttribute", jarg, 0);
	}

	return 0;
}
int zigbee_set_attr(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, const char *attr_name, const char *attr_value) {
	char ieeestr[32];
	hex_string(ieeestr, sizeof(ieeestr), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	attr_profile_t *aps[10];
	int cnt = alink_attr_profile_search_by_attrname(attr_name, aps, sizeof(aps)/sizeof(aps[0]));
	if (cnt <= 0) {
		log_warn("not support attr: %s", attr_name);
		return -1;
	}
	
	int i = 0;
	for (i = 0; i < cnt; i++) {
		attr_profile_t *ap = aps[i];
		short cluster = ap->cluster_id;
		short attrid  = ap->attribute_id;
		if (!zigbee_is_support_cluster_attr(ieee_addr, endpoint_id, cluster, attrid)) {
			log_warn("device [%s] not support cluster:%04x, attr:%04x", ieeestr, cluster&0xffff, attrid&0xffff);
			continue;
		}

		char buf[32];
		json_t *jarg = json_object();
		json_object_set_new(jarg, "ep",				json_string(zigbee_fmt_ba2str(buf, "%02X", &endpoint_id, 1)) );
		json_object_set_new(jarg, "cluster",	json_string(zigbee_fmt_short2str(buf, "%04X", cluster)));
		json_object_set_new(jarg, "attrid",		json_string(zigbee_fmt_short2str(buf, "%04X", attrid)));

		char data[128];
		int datalen = alink_attr_profile_json2buf(ap, attr_value, data);
		if (datalen < 0) {
			log_warn("can't parsed attr_value:%s", attr_value);
			json_decref(jarg);
			continue;
		}
		char datastr[256];
		hex_string(datastr, sizeof(datastr), (u8*)data, datalen, 1, 0);
		json_object_set_new(jarg, "data",			json_string(datastr));
		log_info("data is %s", datastr);

		uproto_call(ieeestr, "ember.zb3.atr", "setAttribute", jarg, 0);
	}
	return 0;
}
int zigbee_zclcmd(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, const char *cmd_name, const char *cmd_args) {
	char ieeestr[32];
	hex_string(ieeestr, sizeof(ieeestr), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	cmd_profile_t *cps[10];
	int cnt = alink_cmd_profile_search_by_cmdname(cmd_name, cps, sizeof(cps)/sizeof(cps[0]));
	if (cnt <= 0) {
		log_warn("not support cmd: %s", cmd_name);
		return -1;
	}
	
	int i = 0;
	for (i = 0; i < cnt; i++) {
		cmd_profile_t *cp = cps[i];
		short cluster = cp->cluster_id;
		char cmdid  = cp->cmd_id;

		/* TODO : Check if this device support this cmd */
		if (!zigbee_is_support_cluster_cmd(ieee_addr, endpoint_id, cluster, cmdid)) {
			log_warn("device : [%s] not support cluster: %04x, cmd:%02x", ieeestr, cluster&0xffff, cmdid&0xff);
			continue;
		}

		char buf[32];
		json_t *jarg = json_object();
		json_object_set_new(jarg, "ep",				json_string(zigbee_fmt_ba2str(buf, "%02X", &endpoint_id, 1)) );
		json_object_set_new(jarg, "cluster",	json_string(zigbee_fmt_short2str(buf, "%04X", cluster)));
		json_object_set_new(jarg, "cmdid",		json_string(zigbee_fmt_short2str(buf, "%04X", cmdid)));

		char data[128];
		if (alink_cmd_profile_json2buf(cp,  cmd_args, data) != 0) {
			log_warn("can't parsed args:%s", cmd_args);
			json_decref(jarg);
			continue;
		}
		json_object_set_new(jarg, "data",			json_string(data));

		uproto_call(ieeestr, "ember.zb3.zclcmd", "setAttribute", jarg, 0);
	}

	return 0;
}
int zigbee_remove_device(char ieee_addr[IEEE_ADDR_BYTES]) {
	char ieeestr[32];
	hex_string(ieeestr, sizeof(ieeestr), (u8*)ieee_addr, IEEE_ADDR_BYTES, 0, 0);

	json_t *jarg = json_object();

	uproto_call(ieeestr, "ember.zb3.remove", "setAttribute", jarg, 0);

	return 0;
}
int zigbee_permit_join(int duration) {
	char mac[32];
	char macstr[32];
	platform_get_hw_id(mac, sizeof(macstr));

	hex_string(macstr, sizeof(macstr), (u8*)mac, 8, 0, 0);

	json_t *jarg = json_object();
	json_object_set_new(jarg, "duration", json_integer(duration));

	json_t *jret = uproto_call(macstr, "ember.zb3.permit", "setAttribute", jarg, 0);
	if (jret != NULL) {
		json_decref(jret);
		jret = NULL;
	}

	return 0;
}


/* report */
static void zigbee_md5(char *sign, const char *rand, const char *secert) {
	char buf[256];
	strcpy(buf, rand);
	strcat(buf, secert);

	char tmp[64];
	md5((const uint8_t*)buf, strlen(buf), (uint8_t*)tmp);

	int i = 0;
	for (i = 0; i < 16/2; i++) {
		sprintf(sign + i*2, "%02x", tmp[i]&0xff);
	}

	sign[16] = 0;
}

int zigbee_rpt_reg(char ieee_addr[IEEE_ADDR_BYTES], const char *type, unsigned int _model_id, const char _rand_bytes[SUBDEV_RAND_BYTES], const char *_sign, int supe) {
	static char rand_char[] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
	};

	unsigned int model_id = -1;
	const char *rand_bytes = NULL;
	const char *sign = NULL;

	if (_model_id > 0 && _sign != NULL) {
		model_id = _model_id;
		sign  = _sign;
		rand_bytes = _rand_bytes;
	} else {
		//const char *typename = "door";
		const char *typename = product_sub_get_typename(type);

		model_id = product_sub_get_model(typename);
		const char *secret = product_sub_get_secret(typename);

		if (model_id < 0 || secret == NULL) {
			log_warn("now alink not support the device type:%s(%s)", type, typename);
			return -1;
		}

		char tmp_sign[128];
		char tmp_rand_bytes[SUBDEV_RAND_BYTES+1];
		int i = 0;
		for (i = 0; i < sizeof(tmp_rand_bytes)/sizeof(tmp_rand_bytes[0]); i++) {
			tmp_rand_bytes[i] = rand_char[rand()%16];

			while (1) {
				if (i != 0 && tmp_rand_bytes[i] == tmp_rand_bytes[i-1]) {
					tmp_rand_bytes[i] = rand_char[rand()%16];
					continue;
				}
				break;
			}
		}
		tmp_rand_bytes[SUBDEV_RAND_BYTES] = 0;
		rand_bytes = tmp_rand_bytes;

		zigbee_md5(tmp_sign, tmp_rand_bytes, secret);
		sign = tmp_sign;

		supe = 0;
	}

	alink_reg((unsigned char *)ieee_addr, model_id, rand_bytes, sign, supe);

	return 0;
}
int zigbee_rpt_unreg(char ieee_addr[IEEE_ADDR_BYTES]) {
	alink_unreg((unsigned char *)ieee_addr);
	return 0;
}
int zigbee_rpt_online(char ieee_addr[IEEE_ADDR_BYTES], char online_or_not) {
	alink_upt_online((unsigned char *)ieee_addr, online_or_not);
	return 0;
}
int zigbee_rpt_attrs(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, short clusterid, short attrid, char *buf, int len) {
	attr_profile_t *ap = alink_attr_profile_search_attr(clusterid, attrid);
	if (ap == NULL) {
		log_warn("not support alink attr: %04X(cluster), %04X(attrid)", clusterid&0xffff, attrid&0xffff);
		return 0;
	}
	log_info("Attr Name:%s", ap->attr_name);
	const char *attr_value = alink_attr_profile_buf2json(ap, buf, len);
	if (attr_value == NULL) {
		log_warn("not support alink attr with null args :%04X(cluster), %04X(attrid)", clusterid&0xffff, attrid&0xffff);
		return 0;
	}
	log_info("Attr Value:%s", attr_value);
		
	const char *attr_names[2] = {ap->attr_name, NULL};
	const char *attr_values[2] = {attr_value, NULL};

	alink_rpt_attrs((unsigned char *)ieee_addr, endpoint_id, attr_names, attr_values);

	free((void*)attr_value);
	attr_value = NULL;

	return 0;
}
int zigbee_rpt_event(char ieee_addr[IEEE_ADDR_BYTES], char endpoint_id, short clusterid, char cmdid, char *buf, int len) {
	cmd_profile_t *cp = alink_cmd_profile_search_cmd(clusterid, cmdid);
	if (cp == NULL) {
		log_warn("not support alink cmd: %04X(cluster), %02X(cmd)", clusterid&0xffff, cmdid&0xff);
		return 0;
	}
	log_info("Cmd Name:%s", cp->cmd_name);
	const char *event_args = alink_cmd_profile_buf2json(cp, buf, len);
	if (event_args == NULL) {
		log_warn("not support alink cmd with null args :%04X(cluster), %02X(cmd)", clusterid&0xffff, cmdid&0xff);
		return 0;
	} 
	log_info("Cmd Args: %s", event_args);

	alink_zigbee_report_event((unsigned char *)ieee_addr, endpoint_id, cp->cmd_name, event_args);

	free((void*)event_args);
	event_args = NULL;

	return 0;
}

