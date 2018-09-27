#include "nxpx.h"
#include "uproto.h"

int nxp_get_list() {
	json_t *jarg = json_object();
	json_t *jret = uproto_call(NULL, "mod.device_list", "getAttribute", jarg, 0);

	if (jret != NULL) {
		json_decref(jret);
	}

	return 0;
}

int nxp_add_device(char *type, char *mac) {

	json_t *jarg = json_object();
	json_object_set_new(jarg, "mac", json_string(mac));
	json_object_set_new(jarg, "type", json_string(type));
	json_t *jret = uproto_call(NULL, "mod.add_device", "setAttribute", jarg, 0);
	if (jret == NULL) {
		json_decref(jret);
		return -1;
	}

	return 0;
}

int nxp_del_device(char *type, char *mac) {
	json_t *jarg = json_object();
	json_object_set_new(jarg, "mac", json_string(mac));
	json_object_set_new(jarg, "type", json_string(type));
	json_t *jret = uproto_call(NULL, "mod.del_device", "setAttribute", jarg, 0);
	if (jret == NULL) {
		json_decref(jret);
		return -1;
	}

	return 0;

}

int nxp_lock_add_pass(char *macstr, int id, int type, int suspend, int startTime, int endTime,  char *buf, int len) {
	/**> 0 - normal
   * 1 - dynamic 
   * 2 - bcd        no support
   * 5 - time pass  
   * 9 - admin			no support 
   * 10- one key 
   * */

	if (type != 0 && type != 1 && type != 5) {
		return -1;
	}


	json_t *jarg = json_object();
	json_object_set_new(jarg, "passId", json_integer(id));
	json_object_set_new(jarg, "passType", json_integer(type));
	json_object_set_new(jarg, "suspend",	json_integer(suspend));
	json_object_set_new(jarg, "startTime",json_integer(startTime));
	json_object_set_new(jarg, "endTime",	json_integer(endTime));

	/**> normal pass content or dynamic pass seed */
	if (type == 0) {
		int val  = ((buf[3]&0xff) << 24)  | ((buf[2]&0xff) << 16) | ((buf[1]&0xff) << 8) | ((buf[0]&0xff) << 0);
		json_object_set_new(jarg, "passVal1", json_integer(val));
		json_object_set_new(jarg, "passVal2", json_integer(0));
	} else if (type == 1) {
		int seed = ((buf[3]&0xff) << 24)  | ((buf[2]&0xff) << 16) | ((buf[1]&0xff) << 8) | ((buf[0]&0xff) << 0);
		int time = ((buf[7]&0xff) << 24)  | ((buf[6]&0xff) << 16) | ((buf[5]&0xff) << 8) | ((buf[4]&0xff) << 0);
		json_object_set_new(jarg, "passVal1", json_integer(seed));
		json_object_set_new(jarg, "passVal2", json_integer(time));
	} else if (type == 5) {
		int val   = ((buf[3]&0xff) << 24)  | ((buf[2]&0xff) << 16) | ((buf[1]&0xff) << 8) | ((buf[0]&0xff) << 0);
		int count = ((buf[7]&0xff) << 24)  | ((buf[6]&0xff) << 16) | ((buf[5]&0xff) << 8) | ((buf[4]&0xff) << 0);
		json_object_set_new(jarg, "passVal1", json_integer(val));
		json_object_set_new(jarg, "passVal2", json_integer(count));
	} 

	json_t *jret = uproto_call(macstr, "device.lock.add_password", "setAttribute", jarg, 0);
	if (jret == NULL) {
		json_decref(jret);
		return -1;
	}

	return 0;


}

int nxp_lock_del_pass(char *macstr, int id, int type) {
	if (type != 0 && type != 1 && type != 5) {
		return -1;
	}


	json_t *jarg = json_object();
	json_object_set_new(jarg, "passId", json_integer(id));
	json_object_set_new(jarg, "passType", json_integer(type));
	json_object_set_new(jarg, "clearAll", json_integer(0));
	json_t *jret = uproto_call(macstr, "device.lock.del_password", "setAttribute", jarg, 0);
	if (jret == NULL) {
		json_decref(jret);
		return -1;
	}

	return 0;
}

int nxp_lock_add_card(char *macstr, int id, int type, int suspend,  int limit, char *buf, int len) {
	return -1;
}

int nxp_lock_del_card(char *macstr, int id, int type) {
	return -1;
}


int nxp_lock_clr_pass(char *macstr, int type) {
	if (type != 0) {
		return -1;
	}


	json_t *jarg = json_object();
	json_object_set_new(jarg, "passId", json_integer(0));
	json_object_set_new(jarg, "passType", json_integer(0));
	json_object_set_new(jarg, "clearAll", json_integer(0x30ae7b00));
	json_t *jret = uproto_call(macstr, "device.lock.del_password", "setAttribute", jarg, 0);
	if (jret == NULL) {
		json_decref(jret);
		return -1;
	}

	return 0;
}

int nxp_lock_mod_pass(char *macstr, int id, int type, int startTime, int endTime,  char *buf, int len) {
	/**> 0 - normal
   * 1 - dynamic 
   * 2 - bcd        no support
   * 5 - time pass  
   * 9 - admin			no support 
   * 10- one key 
   * */

	if (type != 0) {
		return -1;
	}


	json_t *jarg = json_object();
	json_object_set_new(jarg, "passId", json_integer(id));
	json_object_set_new(jarg, "passType", json_integer(type));
	json_object_set_new(jarg, "startTime",json_integer(startTime));
	json_object_set_new(jarg, "endTime",	json_integer(endTime));

	/**> normal pass content or dynamic pass seed */
	if (type == 0) {
		int val  = ((buf[3]&0xff) << 24)  | ((buf[2]&0xff) << 16) | ((buf[1]&0xff) << 8) | ((buf[0]&0xff) << 0);
		json_object_set_new(jarg, "passVal1", json_integer(val));
		json_object_set_new(jarg, "passVal2", json_integer(0));
	} else if (type == 1) {
		int seed = ((buf[3]&0xff) << 24)  | ((buf[2]&0xff) << 16) | ((buf[1]&0xff) << 8) | ((buf[0]&0xff) << 0);
		int time = ((buf[7]&0xff) << 24)  | ((buf[6]&0xff) << 16) | ((buf[5]&0xff) << 8) | ((buf[4]&0xff) << 0);
		json_object_set_new(jarg, "passVal1", json_integer(seed));
		json_object_set_new(jarg, "passVal2", json_integer(time));
	} else if (type == 5) {
		int val   = ((buf[3]&0xff) << 24)  | ((buf[2]&0xff) << 16) | ((buf[1]&0xff) << 8) | ((buf[0]&0xff) << 0);
		int count = ((buf[7]&0xff) << 24)  | ((buf[6]&0xff) << 16) | ((buf[5]&0xff) << 8) | ((buf[4]&0xff) << 0);
		json_object_set_new(jarg, "passVal1", json_integer(val));
		json_object_set_new(jarg, "passVal2", json_integer(count));
	} 

	json_t *jret = uproto_call(macstr, "device.lock.modify_password", "setAttribute", jarg, 0);
	if (jret == NULL) {
		json_decref(jret);
		return -1;
	}

	return 0;
}


int nxp_lock_add_fing(char *macstr, int id, char *buf, int len) {
	return -1;
}

int nxp_lock_del_fing(char *macstr, int id) {
	return -1;
}


int nxp_lock_add_dynamic(char *macstr, int seed, int interval, int startTime, int endTime) {
	int buf[2];
	buf[0] = seed;
	buf[1] = interval;
	return nxp_lock_add_pass(macstr, 999999, 1, 0, startTime, endTime, (char *)&buf[0], sizeof(buf));
}
