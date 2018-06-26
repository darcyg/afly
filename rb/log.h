/******************************************************************************
*  Copyright 2016-2018 Hangzhou ZiLai Sensing Technology Co., Ltd.
*  FileName: log.h
*  Description:
*  Author: ding.yuxiang
*  Create Date: 2018-06-22
*  Modification History:
*   <version> <time>  <author> <desc>
*  a)
******************************************************************************/

#ifndef __ROOMBANKER_LOG_H__
#define __ROOMBANKER_LOG_H__

#include <syslog.h>

#define  INIT_LOG(tag) \
    do          \
    {           \
        openlog(tag, LOG_PERROR | LOG_PID, LOG_DAEMON); \
    } while (0);



#define LOG(fmt, args...) syslog(LOG_DEBUG, fmt, ## args)


#endif

