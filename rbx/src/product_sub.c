#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "product_sub.h"
#include "log.h"


static char *subdev_file = "/etc/config/dusun/afly/subdev.db";
static stSubDev_t subdevs[MAX_SUB_DEV] = {{0}};

//////////////////////////////////////////////////////////////
static stSubDev_t *product_sub_malloc() {
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubDev_t *sd = &subdevs[i];
		if (sd->use == 0) {
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
static int product_sub_file_exsit(const char *file) {
	if (access(file, F_OK) == 0) {
		return 1;
	}
	return 0;
}
static int _product_sub_load_all(const char *db) {
	FILE *fp = fopen(db, "r");
	if (fp == NULL) {
		return -1;
	}
	
	int ret = fread(&subdevs[0], sizeof(subdevs), 1, fp);
	
	if (ret != 1) {
		fclose(fp);
		return -2;
	}

	fclose(fp);

	return 0;
}


int product_sub_save_all() {
	FILE *fp = fopen(subdev_file, "w");
	if (fp == NULL) {
		return -1;
	}
	
	int ret = fwrite(&subdevs[0], sizeof(subdevs), 1, fp);
	
	if (ret != 1) {
		fclose(fp);
		return -2;
	}

	fclose(fp);

	return 0;
}
int product_sub_load_all(const char *db) {
	subdev_file = (char *)db;

	if (product_sub_file_exsit(subdev_file)) {
		return _product_sub_load_all(subdev_file);
	}
	
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
	
	FILE *fp = fopen(subdev_file, "w");
	if (fp == NULL) {
		return -2;
	}

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



int product_sub_add(const char *name, const char *key, const char *secret) {
	stSubDev_t *dev = product_sub_search_by_name(name);
	if (dev != NULL) {
		strcpy(dev->productKey, key);
		strcpy(dev->deviceSecret, secret);
		return 0;
	}

	dev = product_sub_malloc();
	if (dev == NULL) {
		return -1;
	}

	strcpy(dev->productKey, key);
	strcpy(dev->deviceSecret, secret);
	strcpy(dev->deviceName, name);
	
	dev->devid = 0;
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
	if (!(i >= 0 && i < num - 1)) {
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



int product_sub_lock_get_lock_status(stSubDev_t *sd) {
	return sd->aset.lock.lock_status;
}
int product_sub_lock_set_lock_status(stSubDev_t *sd, int status) {
	sd->aset.lock.lock_status = status;
	return 0;
}
int product_sub_lock_get_key_num(stSubDev_t *sd) {
	int i = 0;
	int cnt = sizeof(sd->aset.lock.keys)/sizeof(sd->aset.lock.keys[0]);
	int num = 0;
	for (i = 0; i < cnt; i++) {
		stLockKey_t *key = &sd->aset.lock.keys[i];
		if (!key->use) {
			continue;
		}
		num++;
	}

	return num;
}

stLockKey_t *product_sub_lock_get_key_i(stSubDev_t *sd, int i) {
	int num = product_sub_lock_get_key_num(sd);
	if (!(i >= 0 && i < num - 1)) {
		return NULL;
	}

	int j = 0;
	int cnt = sizeof(sd->aset.lock.keys)/sizeof(sd->aset.lock.keys[0]);
	int idx = 0;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[j];
		if (!key->use) {
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

int product_sub_lock_add_key(stSubDev_t *sd, int type, int limit, char *buf, int len) {
	int num = product_sub_lock_get_key_num(sd);
	if (num >= sizeof(sd->aset.lock.keys) ) {
		return -1;
	}

	int j = 0;
	int cnt = sizeof(sd->aset.lock.keys)/sizeof(sd->aset.lock.keys[0]);
	stLockKey_t *key = &sd->aset.lock.keys[j];
	for (j = 0; j < cnt; j++) {
		stLockKey_t *k = &sd->aset.lock.keys[j];
		if (k->use) {
			continue;
		}
		key = k;
		break;
	}

	key->use = 1;
	key->type = type;
	key->limit = limit;
	memcpy(key->buf, buf, len);

	return 0;
}
int product_sub_lock_del_key(stSubDev_t *sd, int type, int limit, char *buf, int len) {
	int j = 0;
	int cnt = sizeof(sd->aset.lock.keys)/sizeof(sd->aset.lock.keys[0]);
	stLockKey_t *key_del = NULL;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[j];
		if (!key->use) {
			continue;
		}

		if (!(key->len == len && memcmp(key->buf, buf, len) == 0)) {
			continue;
		}
		
		key_del = key;
		break;
	}

	if (key_del == NULL) {
		return -1;
	}

	key_del->use = 0;
	memset(key_del, 0, sizeof(*key_del));

	return 0;
}

int product_sub_lock_clr_key(stSubDev_t *sd) {
	int j = 0;
	int cnt = sizeof(sd->aset.lock.keys)/sizeof(sd->aset.lock.keys[0]);
	stLockKey_t *key_del = NULL;
	for (j = 0; j < cnt; j++) {
		stLockKey_t *key = &sd->aset.lock.keys[j];
		if (!key->use) {
			continue;
		}

		key_del->use = 0;
		memset(key_del, 0, sizeof(*key_del));
	}

	return 0;
}

int product_sub_z3light_get_onoff(stSubDev_t *sd) {
	return sd->aset.z3light.onoff;
}
int product_sub_z3light_set_onoff(stSubDev_t *sd, int onoff) {
	sd->aset.z3light.onoff = onoff;
	return 0;
}



