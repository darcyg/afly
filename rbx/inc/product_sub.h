#ifndef __PRODUCT_SUB_H_
#define __PRODUCT_SUB_H_

#ifdef __cplusplus
extern "C" {
#endif



#define SUBDEV_RAND_BYTES	 (16)
#define IEEE_ADDR_BYTES		 (8)
#define MAX_SUB_DEV       (40)


typedef struct stLockKey {
	/**> use flash addr to id */
	int		use;
	int		type;
	int		limit;
	int		len;
	char	buf[16];
} stLockKey_t;

typedef struct stSubDev {
		int use;
		char mac[8];

    int  devid;
    char productKey[32];
    char deviceName[64];  // mac
    char deviceSecret[64];

    int battery;
    int online;
    char type[32];
    char version[32];
    char model[32];
		char app[32];
		int rssi;


		union {
			struct {
				int lock_status;
				int passNum;
				int passAll;
				int cardNum;
				int cardAll;
				int fingNum;
				int fingAll;
				stLockKey_t keys[100];
			} lock;
			struct {
				int onoff;
			} z3light;
		} aset;
} stSubDev_t;


#define SD_OFF(m) ((int) (((stSubDev_t*)0)->m))

int product_sub_load_all(const char *db);
//int product_sub_save_all();
//int product_sub_load(stSubDev_t *sd, int off, int size);
int product_sub_save(stSubDev_t *sd, int off, int size);
int product_sub_set(stSubDev_t *sd, int off, int size, char *buf);
int product_sub_get(stSubDev_t *sd, int off, int size, char *buf);
int product_sub_sset(stSubDev_t *sd, int off, int size, char *buf);
//int product_sub_sget(stSubDev_t *sd, int off, int size, char *buf);

int product_sub_empty(stSubDev_t *sd);

int product_sub_add(const char *name, const char *key, const char *secret);
int product_sub_del(const char *name);
int product_sub_get_num();
stSubDev_t *product_sub_get_i(int i);

int product_sub_lock_get_lock_status();
int product_sub_lock_set_lock_status();
int product_sub_lock_get_key_num();
stLockKey_t *product_sub_lock_get_key_i(stSubDev_t *sd, int i);
int product_sub_lock_add_key(stSubDev_t *sd, int type, int limit, char *buf, int len);
int product_sub_lock_del_key(stSubDev_t *sd, int type, int limit, char *buf, int len);
int product_sub_lock_clr_key();

int product_sub_z3light_get_onoff(stSubDev_t *sd);
int product_sub_z3light_set_onoff(stSubDev_t *sd, int onoff);


stSubDev_t *product_sub_search_by_devid(int devid);
stSubDev_t *product_sub_search_by_name(const char *name);

void product_sub_view();

#ifdef __cplusplus
}
#endif



#endif
