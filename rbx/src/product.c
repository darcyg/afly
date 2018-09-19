#include "product.h"

static stGateway_t gw = {
	.ZB_Band = 2,
	.ZB_Channel = 11,
	.ZB_CO_MAC = "00158d0002010203",
	.ZB_PAN_ID = "03AE",
	.EXT_PAN_ID = "00158d0002010203",
	.NETWORK_KEY = "0102030405060708090a0b0c0d0e0f",

	.connected =  0,

	.lk_dev = 0,

	.hal_config_dir = "/etc/config/dusun/afly/linkkit.hal",
	.product_key = "a16jEJYhBrU",
	.device_name = "dyxTestGateway",
	.device_secret = "2BQQ2LHBbV8TbVRJrhwtVMpc8JsXD2Dv",
	.product_secret = "a1q89CnOyZM",
	.id = "238709",	
};

stGateway_t *product_get_gw() {
	return &gw;
}

char *	product_get_hal_config_dir() {
	return gw.hal_config_dir;
}
char *	product_get_product_key_default() {
	return gw.product_key;
}
char *	product_get_device_name_default() {
	return gw.device_name;
}
char *	product_get_device_secret_default() {
	return gw.device_secret;
}
char *	product_get_product_secret_default() {
	return gw.product_secret;
}
char *	product_get_id_default() {
	return gw.id;
}


