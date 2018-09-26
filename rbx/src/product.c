#include "product.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

static stGateway_t gw = {
	.ZB_Band = 2,
	.ZB_Channel = 11,
	.ZB_CO_MAC = "00158d0000******",
	.ZB_PAN_ID = "30ae",
	.EXT_PAN_ID = "00158d0000******",
	.NETWORK_KEY = "********************************",

	.connected =  0,

	.lk_dev = 0,

	.hal_config_dir = "/etc/config/dusun/afly/linkkit.hal",
	.product_key = "a16jEJYhBrU",
	.device_name = "dyxTestGateway",
	.device_secret = "2BQQ2LHBbV8TbVRJrhwtVMpc8JsXD2Dv",
	.product_secret = "a1q89CnOyZM",
	.id = "238709",	
};

int product_init() {
	// /etc/config/dusun/nxp/netinfo
	//[Wed Sep 26 01:27:20 2018]: E_MSG_START_IND: pan_id: 40d2 coor_addr:30ae channel:15 status: 0
	
	log_info("---------------------------------------------");
	
	FILE *fp = fopen("/etc/config/dusun/nxp/netinfo", "r");
	if (fp == 0) {
		log_warn("can't open /etc/config/dusun/nxp/netinfo");
		return -1;
	}

	char buf[1024];
	char *p = fgets(buf, sizeof(buf), fp);
	if (p == NULL) {
		log_warn("fgets failed");
		fclose(fp);
		return -2;
	}

	log_info("Nxp Net Info: [%s]", buf);

	

	p = strstr(buf, "pan_id: ");
	int flag = 0;
	if (p != NULL) {
		char buf[16];
		strncpy(buf, p + strlen("pan_id: "), 5);
		if (strcmp(gw.ZB_PAN_ID, buf) != 0) {
			strncpy(gw.ZB_PAN_ID, p + strlen("pan_id: "), 5);
			flag++;
		}
	}
	p = strstr(buf, "channel:");
	if (p != NULL) {
		char buf[16];
		strncpy(buf, p + strlen("channel:"), 3);
		int ch = atoi(buf);

		if (ch != gw.ZB_Channel) {
			gw.ZB_Channel = ch;
			flag++;
		}
	}

	log_info("pan_id: %s, channel:%d", gw.ZB_PAN_ID, gw.ZB_Channel);
	
	
	return flag;
}

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


