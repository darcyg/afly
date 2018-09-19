/*
 * Copyright 2011-2017 Ayla Networks, Inc.  All rights reserved.
 *
 * Use of the accompanying software is permitted only in accordance
 * with and subject to the terms of the Software License Agreement
 * with Ayla Networks, Inc., a copy of which can be obtained from
 * Ayla Networks, Inc.
 */
#ifndef __AYLA_PLATFORM_LEGACY_H__
#define __AYLA_PLATFORM_LEGACY_H__

#include <netinet/ether.h>
/*
 * LEGACY platform header.
 *
 * This header has been included for backwards compatibility.  In the future,
 * please include the below headers directly.
 */

#define PLATFORM_HW_ID_MAX_SIZE	(64) /* Enough space any possible HW IDs */
#define DEVICEMAC_FILE  "/sys/class/net/eth0/address"

int platform_get_mac_addr(struct ether_addr *addr);
int platform_get_hw_id(char *buf, size_t size);

void platform_factory_reset(void);
void platform_reset(void) ;
void platform_remote_shell(const char *server, int port, const char *user, const char *pass, int backport) ;
void platform_apply_setup_mode(int enable) ;
void platform_led_on(const char * led);
void platform_led_off(const char * led);
void platform_led_blink(const char * led, int delay_on, int delay_off);
void platform_led_shot(const char * led);
int platform_get_time();

#endif /* __AYLA_PLATFORM_LEGACY_H__ */
