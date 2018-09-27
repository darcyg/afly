#ifndef _HMAC_H_
#define _HMAC_H_

#include "platform.h"
#include "schedule.h"

#ifndef _U8
#define _U8 unsigned char
#endif

#ifndef _I32
#define _I32 int
#endif


#define _MEMSET memset

#define _MEMCPY memcpy




void hmac_sha1(const _U8 *key, _I32 keyLength,const _U8 *data, _I32 dataLength,_U8 *result, _I32 resultLength) __attribute__((visibility("hidden")));

#endif /* _HMAC_H_ */
