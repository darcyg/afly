#ifndef __NXPX_H_
#define __NXPX_H_

int nxp_get_list();
int nxp_add_device(char *type, char *mac);

int nxp_del_device(char *type, char *mac);

int nxp_lock_add_pass(char *macstr, int id, int type, int suspend, int startTime, int endTime,  char *buf, int len);

int nxp_lock_del_pass(char *macstr, int id, int type);

int nxp_lock_add_card(char *macstr, int id, int type, int suspend,  int limit, char *buf, int len);

int nxp_lock_del_card(char *macstr, int id, int type);

int nxp_lock_clr_pass(char *macstr, int type);

int nxp_lock_mod_pass(char *macstr, int id, int type, int startTime, int endTime,  char *buf, int len);


int nxp_lock_add_fing(char *macstr, int id, char *buf, int len);

int nxp_lock_del_fing(char *macstr, int id);

int nxp_lock_add_dynamic(char *macstr, int seed, int interval, int startTime, int endTime);

#endif
