/******************************************************************************
*  Copyright 2016-2018 Hangzhou ZiLai Sensing Technology Co., Ltd.
*  FileName: gateway.h
*  Description:
*  Author: ding.yuxiang
*  Create Date: 2018-06-22
*  Modification History:
*   <version> <time>  <author> <desc>
*  a)1.0.0  2018-06-22 ding.yixiang create this header from sdk example source
******************************************************************************/

#ifndef __ALI_GATEWAY_H__
#define __ALI_GATEWAY_H__

#define DPRINT(...)                                      \
    do {                                                     \
        printf("\033[1;31;40m%s.%d: ", __func__, __LINE__);  \
        printf(__VA_ARGS__);                                 \
        printf("\033[0m");                                   \
    } while (0)

typedef struct
{
    int ZB_Band;
    int ZB_Channel;

    char ZB_CO_MAC[32];
    char ZB_PAN_ID[32];
    char EXT_PAN_ID[32];
    char NETWORK_KEY[32];

    int connected;

    int lk_dev;
} gateway_t, *PST_GATEWAY_T;



#endif

