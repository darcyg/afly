#ifndef __PRODUCT_H__
#define __PRODUCT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stGateway {
    int  ZB_Band;
    int  ZB_Channel;

    char ZB_CO_MAC[32];
    char ZB_PAN_ID[32];
    char EXT_PAN_ID[32];
    char NETWORK_KEY[32];

    int connected;

    int lk_dev;

		char *hal_config_dir;
		char *product_key;
		char *device_name;
		char *device_secret;
		char *product_secret;
		char *id;
} stGateway_t;


stGateway_t *product_get_gw();
 
char *	product_get_hal_config_dir();
char *	product_get_product_key_default();
char *	product_get_device_name_default();
char *	product_get_device_secret_default();
char *	product_get_product_secret_default();
char *	product_get_id_default();

#ifdef __cplusplus
}
#endif
#endif
