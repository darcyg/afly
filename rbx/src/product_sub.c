#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "product_sub.h"
#include "log.h"


static stSubDevCfg_t sdcs[] = {
	[0] = {
		.productKey = "a1eOQheUHpW",
		.deviceName = "00158d0000b4716f",
		.deviceSecret = "Lmh7xC0kdnHGsk4KHxq96voRLRJBUfg3",
		.model = "1100",
	},
	[1] = {
		.productKey = "a1wcKZILMWO",
		.deviceName = "L92bxAd5sgKQg20K2LLF",
		.deviceSecret = "rT8fGtnhTTy3gyH5mL8BjKVvRGUz4GbE",
		.model = "1102",
	},

};

static stSubDev_t subdevs[MAX_SUB_DEV] = {{0}};



//////////////////////////////////////////////////////////////
int product_sub_get_subdev_cfg_num() {
	return sizeof(sdcs)/sizeof(sdcs[0]);
}

stSubDevCfg_t *product_sub_get_subdev_cfg_by_idx(int i) {
	int cnt = product_sub_get_subdev_cfg_num();
	if (i >= cnt) {
		return NULL;
	}
	return &sdcs[i];
}

stSubDevCfg_t *product_sub_get_subdev_cfg_by_model(char *model) {
	int cnt = product_sub_get_subdev_cfg_num();
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubDevCfg_t *sdc = &sdcs[i];
		//log_info("sdc->model:%s, model:%s", sdc->model, model);
		if (strcmp(sdc->model, model) == 0) {
			return sdc;
		}
	}
	return NULL;
}

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

int product_sub_get_subdev_id_by_ieee(char *ieee) {
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubDev_t *sd = &subdevs[i];
		if (memcmp(sd->mac, ieee, 8) == 0) {
			return sd->devid;
		}
	}
	return -1;
}

stSubDev_t *product_sub_get_subdev_by_ieee(char *ieee) {
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubDev_t *sd = &subdevs[i];
		if (memcmp(sd->mac, ieee, 8) == 0) {
			return sd;
		}
	}
	return NULL;
}
static stSubDev_t *product_sub_get_subdev_by_id(int id) {
	int cnt = sizeof(subdevs)/sizeof(subdevs[0]);
	int i = 0;
	for (i = 0; i < cnt; i++) {
		stSubDev_t *sd = &subdevs[i];
		if (sd->devid == id) {
			return sd;
		}
	}
	return NULL;

}

int product_sub_new_subdev(char *ieee, char *model, stSubDevCfg_t *sdc, int id) {
	stSubDev_t *sd = product_sub_malloc();
	if (sd == NULL) {
		return -1; 
	}

	sd->devid = id;

	strcpy(sd->productKey, sdc->productKey);
	strcpy(sd->deviceName, sdc->deviceName);
	strcpy(sd->deviceSecret, sdc->deviceSecret);

	sd->battery = 0;
	sd->online = 0;
	strcpy(sd->type, "unknow");
	strcpy(sd->version, "1.0");
	strcpy(sd->model, model);

	
	memcpy(sd->mac, ieee, 8);
	
	return 0;
}

int product_sub_del_subdev_by_ieee(char *ieee) {
	stSubDev_t *dev = product_sub_get_subdev_by_ieee(ieee);	
	if (dev == NULL) {
		return -1;
	}

	product_sub_free(dev);
	return 0;
}

int product_sub_del_subdev_by_id(int id) {
	stSubDev_t *dev = product_sub_get_subdev_by_id(id);	
	if (dev == NULL) {
		return -1;
	}

	product_sub_free(dev);

	return 0;
}

