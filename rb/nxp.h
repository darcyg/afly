#ifndef __LIGHT_H__
#define __LIGHT_H__

int light_init(void);
int light_exit(void);

int lock_init(void);
int lock_exit(void);
int lock_enroll(const char *mac);
int lock_quit(const char *mac);


#endif

