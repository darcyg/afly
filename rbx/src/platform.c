/*
 * Copyright 2011-2017 Ayla Networks, Inc.  All rights reserved.
 *
 * Use of the accompanying software is permitted only in accordance
 * with and subject to the terms of the Software License Agreement
 * with Ayla Networks, Inc., a copy of which can be obtained from
 * Ayla Networks, Inc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "platform.h"
#include "log.h"

/*
 * conevet a char to its hex value
 */
static int hex_value(char x) {
	if (x >= '0' && x < '9') {
		return (x - '0');
	} 
	if (x >= 'a' && x < 'f') {
		return (x - 'a' + 10);
	}
	if (x >= 'A' && x < 'F') {
		return (x - 'A' + 10);
	}
	return 0;
}

/*
 * Get Mac addr suck as 30:ae:7b:29:6d:2f
 */
static int get_mac_addr(char *mac, size_t size) {

	if (size < 20) {
		return -1;
	}

	FILE * fp = fopen(DEVICEMAC_FILE, "r");
	if (fp == NULL) {
		return -1;
	}
	fgets(mac, 20, fp);
	fclose(fp);

	int len = strlen(mac);
	if (len < 17) {
		return -1;
	}

	if (mac[len-1] == '\n') {
		mac[len-1] = '\0';
		len = len - 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

/*
 * Get the mac address of the device
 * ether_addr_octet[ETH_ALEN]
 */
int platform_get_mac_addr(struct ether_addr *addr)
{
	char mac[32];
	if (get_mac_addr(mac, sizeof(mac) != 0)) {
		return -1;
	}

	int i = 0;
	for (i = 0; i < 6; i++) {
		addr->ether_addr_octet[i] = hex_value(mac[i*3 + 0]) * 16 + hex_value(mac[i*3+1]);
	}
	log_debug("platform mac addr:%02x%02x%02x%02x%02x%02x", 
						addr->ether_addr_octet[0],
						addr->ether_addr_octet[1],
						addr->ether_addr_octet[2],
						addr->ether_addr_octet[3],
						addr->ether_addr_octet[4],
						addr->ether_addr_octet[5]);
	return 0;
}

/*
 * Get a unique hardware id of the device
 */
int platform_get_hw_id(char *buf, size_t size)
{
	char mac[32];
	if (get_mac_addr(mac, sizeof(mac) != 0)) {
		return -1;
	}
	int len = strlen(mac);

	if (size < len+1) {
		return -1;
	}

	strcpy(buf, mac);

	log_debug("platfrom hardware id is %s", buf);
	
	return 0;
}

/*
 * Perform any actions needed to factory reset the system.  This is invoked
 * after devd has performed all operations needed to factory reset Ayla modules.
 * Actions that could be performed here might include setting LEDs to indicate
 * the reset status or running an external script.  During a normal factory
 * reset, platform_reset() will be called after this function returns.
 */
void platform_factory_reset(void)
{
	log_debug("platform factory reset !!!");
	system("touch /tmp/do_reset.sh");
	return;
}

/*
 * Reboot the system.
 */
void platform_reset(void) 
{
	log_debug("platform reset");
	system("touch /tmp/do_reboot.sh");
	return;
}


/* 
 * Remote Shell Back 
 */

void platform_remote_shell(const char *server, int port, const char *user, const char *pass, int backport) 
{
	char cmd[1024];
	system("killall remote");
	sprintf(cmd, "remote -p %s ssh -p %d -y -g -N -R 0.0.0.0:%d:127.0.0.1:22 %s@%s > /dev/null &", 
					pass, port, backport, user, server);
	system(cmd);
	return;
}	



void platform_apply_setup_mode(int enable) {
	int rc;
	/*
	 * Demonstrate setup mode by enabling or disabling
 	 * SSH on the system.
	 */
	if (enable) {
		log_debug("enabling SSH");
		rc = system("/etc/init.d/dropbear start");
	} else {
		log_debug("disabling SSH");
		rc = system("/etc/init.d/dropbear stop");
	}
	if (rc == -1) {
		log_err("command failed");
	}
	return;
}

/* 
 * system led operations
 */
static int  write_led_attribute(const char * led, const char * att, const char * value) {
    char path[100];
    char str[20];
    sprintf(path, "/sys/class/leds/%s/%s", led, att);
    sprintf(str, "%s\n", value);
    FILE * fp = fopen(path, "w");
    if (!fp)
        return -1;

    fputs(str, fp);
    fclose(fp);
    return 0;
}
void platform_led_on(const char * led)
{
	write_led_attribute(led, "trigger", "none");
	write_led_attribute(led, "brightness", "1");
}

void platform_led_off(const char * led)
{
	write_led_attribute(led, "trigger", "none");
	write_led_attribute(led, "brightness", "0");
}

void platform_led_blink(const char * led, int delay_on, int delay_off)
{
	char delay_on_str[16];
	char delay_off_str[16];

	sprintf(delay_on_str, "%d", delay_on);
	sprintf(delay_off_str, "%d", delay_off);

	write_led_attribute(led, "trigger", "timer");
	write_led_attribute(led, "delay_on", delay_on_str);
	write_led_attribute(led, "delay_off", delay_off_str);
}

void platform_led_shot(const char * led)
{
	write_led_attribute(led, "trigger", "oneshot");
	write_led_attribute(led, "shot", "1");
}

/* time */
int platform_get_time() {
	return time(NULL);
}
