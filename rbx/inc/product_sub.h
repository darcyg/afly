#ifndef __PRODUCT_SUB_H_
#define __PRODUCT_SUB_H_

#ifdef __cplusplus
extern "C" {
#endif



#define SUBDEV_RAND_BYTES	 (16)
#define IEEE_ADDR_BYTES		 (8)
#define MAX_SUB_DEV       (40)


typedef struct stLockKey {
	int		use;
	int		type;
	int		limit;
	int		len;
	char	buf[16];
} stLockKey_t;

typedef struct stSubDev {
		int use;

    int  devid;
    char productKey[32];
    char deviceName[64];  // mac
    char deviceSecret[64];

    int battery;
    int online;
    char type[32];
    char version[32];
    char model[32];


		union {
			struct {
				int lock_status;
				stLockKey_t keys[100];
			} lock;
			struct {
				int onoff;
			} z3light;
		} aset;
} stSubDev_t;


#define SD_OFF(m) ((int) (((stSubDev_t*)0)->m))

int product_sub_load_all(const char *db);
//int product_sub_save_all(const char *db);
//int product_sub_load(stSubDev_t *sd, int off, int size);
//int product_sub_save(stSubDev_t *sd, int off, int size);
int product_sub_set(stSubDev_t *sd, int off, int size, char *buf);
int product_sub_get(stSubDev_t *sd, int off, int size, char *buf);
int product_sub_sset(stSubDev_t *sd, int off, int size, char *buf);
//int product_sub_sget(stSubDev_t *sd, int off, int size, char *buf);

int product_sub_add(char *name, char *key, char *secret);
int product_sub_del(char *name);

int product_sub_lock_get_lock_status();
int product_sub_lock_set_lock_status();
int product_sub_lock_get_key_num();
stLockKey *product_sub_lock_get_key_i(int i);
int product_sub_lock_add_key(int type, int limit, char *key, int len);
int product_sub_lock_del_key(int type, int limit, char *key, int len);
int product_sub_lock_clr_key();

int product_sub_z3light_get_onoff();
int product_sub_z3light_set_onoff(int onoff);


stSubDev_t *product_sub_search_by_devid(int devid);
stSubDev_t *product_sub_search_by_name(char *name);


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
