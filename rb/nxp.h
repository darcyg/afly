#ifndef __NXP_LOCK_H__
#define __NXP_LOCK_H__


int lock_init(void);
int lock_exit(void);
int lock_enroll(const char *mac);
int lock_quit(const char *mac);


#endif

