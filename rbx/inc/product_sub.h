#ifndef __PRODUCT_SUB_H_
#define __PRODUCT_SUB_H_

#include "schedule.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SUBDEV_RAND_BYTES	 (16)
#define IEEE_ADDR_BYTES		 (8)
#define MAX_SUB_DEV       (40)


enum {
	KEY_STATE_NONE		= 0x00,	// no use
	KEY_STATE_ADDING	= 0x01, // 
	KEY_STATE_ADDED	  = 0x02,	//
};

typedef struct stLockKey {
	/**> use flash addr to id */
	int		last;
	int		key_state;
	int		id;
	int		type;						//1 -> fing, 2->pass, 3->card, 4->hardkey
	int		limit;					//1 -> normal, 2->admin, 3->hijacking
	int		len;						//pass->6~16, card->4 , 8, 12, fing->810+
	char	buf[32];
} stLockKey_t;


typedef struct stSubDev {
		int use;

    int  devid;
		int  login;
    char productKey[48];
    char deviceName[64];  // mac
    char deviceSecret[64];

    int battery;
    int online;
    char type[32];
    char version[32];
    char model[32];
		char app[32];
		int rssi;
		int dynamic;
		int seed;
		int inteval;


		union {
			struct {
				int lock_status;
				int passNum;
				int passAll;
				int cardNum;
				int cardAll;
				int fingNum;
				int fingAll;
				stLockKey_t keys[3][128];
			} lock;
			struct {
				int onoff;
			} z3light;
		} aset;

		stSchduleTask_t task;
} stSubDev_t;


#define SD_OFF(m) ((int) (((stSubDev_t*)0)->m))

int product_sub_load_all(const char *db, void *fet);
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
int product_sub_clr();
int product_sub_get_num();
stSubDev_t *product_sub_get_i(int i);

int product_sub_lock_get_lock_status();
int product_sub_lock_set_lock_status();

int product_sub_lock_get_key_num(stSubDev_t *sd, int type);
stLockKey_t *product_sub_lock_get_key_i(stSubDev_t *sd, int i, int type);
stLockKey_t *product_sub_lock_add_key_wait_ack(stSubDev_t *sd, int type, int limit, char *buf, int len);
int product_sub_lock_add_key_complete(stSubDev_t *sd, int type, int id);
int product_sub_lock_del_key(stSubDev_t *sd, int type, int id);
int product_sub_lock_clr_key(stSubDev_t *sd, int type);

stLockKey_t *product_sub_lock_get_key_by_id(stSubDev_t *sd, int type, int id);



int product_sub_z3light_get_onoff(stSubDev_t *sd);
int product_sub_z3light_set_onoff(stSubDev_t *sd, int onoff);


stSubDev_t *product_sub_search_by_devid(int devid);
stSubDev_t *product_sub_search_by_name(const char *name);

void product_sub_view();


int product_valid_password_string(const char *s);

#ifdef __cplusplus
}
#endif



#endif
