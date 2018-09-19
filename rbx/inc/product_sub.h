#ifndef __PRODUCT_SUB_H_
#define __PRODUCT_SUB_H_

#ifdef __cplusplus
extern "C" {
#endif



#define SUBDEV_RAND_BYTES	 (16)
#define IEEE_ADDR_BYTES		 (8)
#define MAX_SUB_DEV       (40)

typedef struct stLock {
    int  devid;
    char productKey[32];
    char deviceName[64];
    char deviceSecret[64];

    int battery;
    int online;
    char type[8];
    char version[8];
    char model[8];

		char mac[32];

		int use;
} stSubDev_t;

typedef struct stDevCfg {
    char *productKey;
    char *deviceName;
    char *deviceSecret;
		char *model;
} stSubDevCfg_t;

int product_sub_get_subdev_cfg_num();
stSubDevCfg_t *product_sub_get_subdev_cfg_by_idx(int i);
stSubDevCfg_t *product_sub_get_subdev_cfg_by_model(char *model);


int product_sub_del_subdev(char *ieee);

int product_sub_get_subdev_id_by_ieee(char *ieee);
stSubDev_t *product_sub_get_subdev_by_ieee(char *ieee);
int product_sub_new_subdev(char *ieee, char *model, stSubDevCfg_t *sdc, int id);
int product_sub_del_subdev_by_id(int id);
int product_sub_del_subdev_by_ieee(char *ieee);

#ifdef __cplusplus
}
#endif



#endif
