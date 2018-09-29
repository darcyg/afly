#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "product_sub.h"
#include "log.h"


static char *subdev_file = "/etc/config/dusun/afly/subdev.db";
static stSubDev_t subdevs[MAX_SUB_DEV] = {
	{{0}}
#if 0
	[0] =  {
		.use = 1,
		.devid = 0,
		.productKey = "a1wcKZILMWO",
		.deviceName = "00158d00026c5415",
		.deviceSecret = "goXATTZ7X9QoOxzFdAMbtjurKnqugvOs",

		.battery = 100,
		.online = 0,
		.type = "1203",
		.version = "1.0",
		.model = "1203",
		.app = "NXP",
		.aset = {{0}},
	},
	[1] =  {
		.use = 1,
		.devid = 0,
		.productKey = "a1wcKZILMWO",
		.deviceName = "00158d00026c540a",
		.deviceSecret = "3X1jZmnSKx1Dej9RQvLVtywP1SPe6Xk1",

		.battery = 100,
		.online = 0,
		.type = "1203",
		.version = "1.0",
		.model = "1203",
		.app = "NXP",
		.aset = {{0}},
	},
#endif

};

static stSubProductKeys_t spks[] = {
	{"a1wcKZILMWO", "0009", "1203"},
	{"a1wcKZILMWO", "0066", "1203"},
	{"a1wcKZILMWO", "0008", "1203"},
};

//////////////////////////////////////////////////////////////
static stSubDev_t *product_sub_malloc() {
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubDev_t *sd = &subdevs[i];
		if (sd->use == 0) {
			sd->use = 1;
			return sd;
		}
	}
	return NULL;
}
static void product_sub_free(stSubDev_t *dev) {
	dev->use = 0;
	memset(dev, 0, sizeof(*dev));
}

//////////////////////////////////////////////////////////////////////////////////////
static void product_sub_clr_id_after_load() {
	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);

	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[j];
		if (!sd->use) {
			continue;
		}

		sd->login = 0;
		sd->devid = 0;
	}
}


static int product_sub_file_exsit(const char *file) {
	if (access(file, F_OK) == 0) {
		return 1;
	}
	return 0;
}
static int _product_sub_load_all(const char *db, void *fet) {
	FILE *fp = fopen(db, "r");
	if (fp == NULL) {
		return -1;
	}
	
	fseek(fp, 0, SEEK_SET);
	int ret = fread(&subdevs[0], sizeof(subdevs), 1, fp);
	
	if (ret != 1) {
		product_sub_clr_id_after_load();
		fclose(fp);
		return -2;
	}

	product_sub_clr_id_after_load();

	fclose(fp);

	return 0;
}


int product_sub_save_all() {
	FILE *fp = fopen(subdev_file, "w");
	if (fp == NULL) {
		return -1;
	}
	
	fseek(fp, 0, SEEK_SET);
	int ret = fwrite(&subdevs[0], sizeof(subdevs), 1, fp);
	
	if (ret != 1) {
		fclose(fp);
		return -2;
	}

	fclose(fp);

	return 0;
}
int product_sub_load_all(const char *db, void *fet) {
	subdev_file = (char *)db;

	if (product_sub_file_exsit(subdev_file)) {
		return _product_sub_load_all(subdev_file, fet);
	}
	
	//memset(&subdevs[1], 0, sizeof(subdevs) - sizeof(subdevs[0]) * 1);
	memset(subdevs, 0, sizeof(subdevs));

	return product_sub_save_all(subdev_file);
}

int product_sub_load(stSubDev_t *sd, int off, int size) {
	if (sd == NULL) {
		return -1;
	}

	int i = sd - &subdevs[0];
	
	FILE *fp = fopen(subdev_file, "r");
	if (fp == NULL) {
		return -2;
	}

	fseek(fp, i*sizeof(*sd), SEEK_SET);

	char *p = (char *)sd;
	int ret = fread(p + off, size, 1, fp);
	if (ret != 1) {
		fclose(fp);
		return -3;
	}

	return 0;
}
int product_sub_save(stSubDev_t *sd, int off, int size) {
	if (sd == NULL) {
		return -1;
	}

	int i = sd - &subdevs[0];
	
	FILE *fp = fopen(subdev_file, "r+");
	if (fp == NULL) {
		return -2;
	}

	fseek(fp, 0, SEEK_SET);
	fseek(fp, i*sizeof(*sd), SEEK_SET);

	char *p = (char *)sd;
	int ret = fwrite(p + off, size, 1, fp);
	if (ret != 1) {
		fclose(fp);
		return -3;
	}

	return 0;
}
int product_sub_set(stSubDev_t *sd, int off, int size, char *buf) {
	if (sd == NULL) {
		return -1;
	}

	char *p = (char *)sd;
	memcpy(p + off, buf, size);

	return 0;
}
int product_sub_get(stSubDev_t *sd, int off, int size, char *buf) {
	if (sd == NULL) {
		return -1;
	}

	char *p = (char *)sd;
	memcpy(buf, p + off, size);

	return 0;
}
int product_sub_sset(stSubDev_t *sd, int off, int size, char *buf) {
	int ret = product_sub_set(sd, off, size, buf);
	if (ret != 0) {
		return -1;
	}

	return  product_sub_save(sd, off, size);
}

int product_sub_sget(stSubDev_t *sd, int off, int size, char *buf) {
	int ret = product_sub_load(sd, off, size);
	if (ret != 0) {
		return -1;
	}

	return product_sub_get(sd, off, size, buf);
}


int product_sub_empty(stSubDev_t *sd) {
	sd->devid = 0;
	sd->login = 0;
	memset(&sd->battery, 0, (unsigned int)(sd + 1) - (unsigned int)&sd->battery);
	return 0;
}


stSubDev_t *product_sub_search_by_devid(int devid) {
	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);

	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[j];
		if (!sd->use) {
			continue;
		}

		if (sd->devid != devid) {
			continue;
		}
		
		return sd;
	}

	return NULL;
}
stSubDev_t *product_sub_search_by_name(const char *name) {
	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);

	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[j];
		if (!sd->use) {
			continue;
		}

		if (strcmp(name, sd->deviceName)!= 0) {
			continue;
		}
		
		return sd;
	}

	return NULL;
}

stSubDev_t *product_sub_search_by_product_key(const char *key) {
	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);

	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[j];
		if (!sd->use) {
			continue;
		}

		if (strcmp(key, sd->productKey)!= 0) {
			continue;
		}
		
		return sd;
	}

	return NULL;
}


int product_sub_add(const char *name, const char *key, const char *secret) {
	stSubDev_t *dev = product_sub_search_by_name(name);
	if (dev != NULL) {
		strcpy(dev->productKey, key);
		if (secret != NULL) {
			strcpy(dev->deviceSecret, secret);
		} else {
			dev->deviceSecret[0] = 0;
		}
		return 0;
	}

	dev = product_sub_malloc();
	if (dev == NULL) {
		return -1;
	}

	strcpy(dev->productKey, key);
	if (secret != NULL)  {
		strcpy(dev->deviceSecret, secret);
	} else {
		dev->deviceSecret[0] = 0;
	}
	strcpy(dev->deviceName, name);
	
	dev->devid = 0;
	dev->login = 0;
	dev->battery = 0;
	dev->online = 0;
	dev->type[0] = 0;
	dev->version[0] = 0;
	dev->model[0] = 0;
	dev->app[0] = 0;
	
	memset(&dev->aset, 0, sizeof(dev->aset));
	
	return product_sub_save(dev, 0, sizeof(*dev));
}

int product_sub_del(const char *name) {
	stSubDev_t *dev = product_sub_search_by_name(name);
	if (dev == NULL) {
		return 0;
	}

	product_sub_free(dev);

	product_sub_save(dev, 0, sizeof(*dev));

	return 0;
}


int product_sub_clr() {
	memset(&subdevs[0], 0, sizeof(subdevs));
	return product_sub_save_all(subdev_file);
}

int product_sub_get_num() {
	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);

	int num = 0;
	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[j];
		if (!sd->use) {
			continue;
		}
		
		num++;
	}

	return num;
}
stSubDev_t *product_sub_get_i(int i) {
	int num = product_sub_get_num();
	if (!(i >= 0 && i < num)) {
		return NULL;
	}

	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);
	int idx = 0;
	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[i];
		if (!sd->use) {
			continue;
		}
		if (idx != i) {
			idx++;
			continue;
		}

		return sd;
	}

	return NULL;

}

void product_sub_view() {
	int j = 0;
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);

	for (j = 0; j < cnt; j++) {
		stSubDev_t *sd = &subdevs[j];
		if (!sd->use) {
			continue;
		}
		
		log_info("deviceName:%s, online:%d",sd->deviceName, sd->online );
		log_info("\t   devid: %d", sd->devid);
		log_info("\t   login: %d", sd->login);
		log_info("\t   poductKey: %s", sd->productKey);
		log_info("\t   deviceSecret: %s", sd->deviceSecret);
		log_info("\t   battery: %d", sd->battery);
		log_info("\t   type: %s", sd->type);
		log_info("\t   version: %s", sd->version);
		log_info("\t   model: %s", sd->model);
		log_info("\t   app: %s", sd->app);
		log_info("\t   has_dynamic: %d", sd->dynamic);
		log_info("\t   seed: %d", sd->seed);
		log_info("\t   interval: %d", sd->interval);

	}

}

int product_sub_lock_get_lock_status(stSubDev_t *sd) {
	return sd->aset.lock.lock_status;
}
int product_sub_lock_set_lock_status(stSubDev_t *sd, int status) {
	sd->aset.lock.lock_status = status;
	return 0;
}

static int product_sub_convet_type2idx(int type) {
	switch (type) {
		case 1:
			return 0;
		case 2:
			return 1;
		case 3:
			return 2;
		case 5:
			return 3;
			
		default:
			break;
	}
	return 0;
}
int product_sub_lock_get_key_num(stSubDev_t *sd, int type) {

	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);
	
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
	int num = 0;
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stLockKey_t *key = &sd->aset.lock.keys[adx][i];
		if (!(key->key_state == KEY_STATE_ADDED)) {
			continue;
		}
		num++;
	}

	return num;
}

stLockKey_t *product_sub_lock_get_key_i(stSubDev_t *sd, int i, int type) {
	int num = product_sub_lock_get_key_num(sd, type);

	if (!(i >= 0 && i < num)) {
		return NULL;
	}

	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);

	int j = 0;
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
	int idx = 0;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[adx][j];
		if (!(key->key_state == KEY_STATE_ADDED)) {
			continue;
		}
		if (idx != i) {
			idx++;
			continue;
		}

		return key;
	}

	return NULL;
}

stLockKey_t *product_sub_lock_add_key_wait_ack(stSubDev_t *sd, int type, int limit, char *buf, int len) {
	int num = product_sub_lock_get_key_num(sd, type);
	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
		
	if (!(num >= 0 && num < cnt)) {
		return NULL;
	}

	int j = 0;
	stLockKey_t *key = NULL;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *k = &sd->aset.lock.keys[adx][j];
		if (k->key_state != KEY_STATE_NONE) {
			continue;
		}

		key = k;
		break;
	}

	if (key == NULL) {
		return NULL;
	}

	key->key_state = KEY_STATE_ADDING;

	buf[len] = 0;
	key->last = time(NULL);
	key->id   = 1000000 * type + atoi(buf);
	//key->id   = 1000000 * type + j;
	key->type	= type;
	key->limit = limit;
	key->len = len;
	memcpy(key->buf, buf, len > sizeof(key->buf) ? sizeof(key->buf) : len);

	return key;
}

int product_sub_lock_add_key_complete(stSubDev_t *sd, int type, int id) {
	//int num = product_sub_lock_get_key_num(sd, type);

	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);

	int j = 0;
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
	stLockKey_t *key = NULL;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *k = &sd->aset.lock.keys[adx][j];
		if (k->key_state != KEY_STATE_ADDING) {
			continue;
		}
		if (k->id != id) {
			continue;
		}

		key = k;
		break;
	}

	if (key == NULL) {
		return -1;
	}

	key->key_state = KEY_STATE_ADDED;

	return product_sub_save(sd, 0, sizeof(*sd));
}

int product_sub_lock_del_key(stSubDev_t *sd, int type, int id) {
	int j = 0;

	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
	stLockKey_t *key_del = NULL;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[adx][j];
		if (!key->key_state == KEY_STATE_ADDED) {
			continue;
		}

		if (key->id != id) {
			continue;
		}

		key_del = key;
		break;
	}

	if (key_del == NULL) {
		return -1;
	}

	key_del->key_state = KEY_STATE_NONE;
	memset(key_del, 0, sizeof(*key_del));

	return product_sub_save(sd, 0, sizeof(*sd));
}

int product_sub_lock_clr_key(stSubDev_t *sd, int type) {
	int j = 0;
	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[adx][j];
		if (key->key_state == KEY_STATE_NONE) {
			continue;
		}

		key->key_state = KEY_STATE_NONE;
		memset(key, 0, sizeof(*key));
		product_sub_save(sd, 0, sizeof(*sd));
	}

	return 0;
}

int product_sub_lock_add_dynamic_wait_ack(stSubDev_t *sd, int seed, int interval) {
	sd->seed = seed;
	sd->interval = interval;
	
	return 0;
}
int product_sub_lock_add_dynamic_complete(stSubDev_t *sd) {
	sd->dynamic = 1;
	return product_sub_save(sd, 0, sizeof (*sd));
}



stLockKey_t *product_sub_lock_get_key_by_id(stSubDev_t *sd, int type, int id) {
	int j = 0;
	//int adx = (type + (3-1))%3 + 1;
	int adx = product_sub_convet_type2idx(type);
	int cnt = sizeof(sd->aset.lock.keys[adx])/sizeof(sd->aset.lock.keys[adx][0]);
	stLockKey_t *key_op = NULL;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[adx][j];
		if (key->key_state == KEY_STATE_NONE) {
			continue;
		}

		log_debug("key->id:%d, key->str:%s, id:%d, key->key_state:%d", key->id, key->buf, id, key->key_state);
		if (key->id != id) {
			continue;
		}
		key_op = key;
	}

	return key_op;
}

int product_sub_z3light_get_onoff(stSubDev_t *sd) {
	return sd->aset.z3light.onoff;
}
int product_sub_z3light_set_onoff(stSubDev_t *sd, int onoff) {
	sd->aset.z3light.onoff = onoff;
	return 0;
}



int product_valid_password_string(const char *s) {
	int len = strlen(s);

	if (!(len >= 6 && len <= 15)) {
		return 0;
	}

	int i = 0;
	for (i = 0; i < len; i++) {
		if (! ((s[i] >= '0' && s[i] <= '9') || (s[i] == ',')) ) {
			return 0;
		}
	}

	return 1;
}

stSubProductKeys_t *product_sub_get_product_key_by_type_or_model(char *type, char *model) {
	int cnt = sizeof(spks)/sizeof(spks[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubProductKeys_t *spk = &spks[i];
		if (strcmp(spk->model, model) == 0 || strcmp(spk->type, type) == 0) {
			return spk;
		}
	}
	return NULL;
}

