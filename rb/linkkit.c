/******************************************************************************
*  Copyright 2016-2018 Hangzhou Roombanker Technology Co., Ltd.
*  FileName: main.c
*  Description:
*  Author: ding.yuxiang
*  Create Date: 2018-06-19
*  Modification History:
*   <version> <time>  <author> <desc>
*  a)1.0.0  2018-06-19 ding.yuxiang create this file
******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <dusun/dusun.h>

#include "nxp.h"
#include "linkkit.h"
#include "iot_import.h"

#include "cJSON.h"
#include "gateway.h"



/*****************************************************************************
 * Function:             gateway_get_property
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static int gateway_get_property(char *in, char *out, int out_len, void *ctx)
{
    DPRINT("in: %s\n", in);

    gateway_t *gw = ctx;
    if (!gw)
    {
        DPRINT("gateway not found\n");
        return -1;
    }

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    int iSize = cJSON_GetArraySize(rJson);
    if (iSize <= 0)
    {
        cJSON_Delete(rJson);
        return -1;
    }

    cJSON *pJson = cJSON_CreateObject();
    if (!pJson)
    {
        cJSON_Delete(rJson);
        return -1;
    }

    int i;
    for (i = 0; i < iSize; i++)
    {
        cJSON *pSub = cJSON_GetArrayItem(rJson, i);

        if (strcmp(pSub->valuestring, "ZB_Band") == 0)
        {
            cJSON_AddNumberToObject(pJson, "ZB_Band", gw->ZB_Band);
        }
        else if (strcmp(pSub->valuestring, "ZB_Channel") == 0)
        {
            cJSON_AddNumberToObject(pJson, "ZB_Channel", gw->ZB_Channel);
        }
        else if (strcmp(pSub->valuestring, "ZB_CO_MAC") == 0)
        {
            cJSON_AddStringToObject(pJson, "ZB_CO_MAC", gw->ZB_CO_MAC);
        }
        else if (strcmp(pSub->valuestring, "ZB_PAN_ID") == 0)
        {
            cJSON_AddStringToObject(pJson, "ZB_PAN_ID", gw->ZB_PAN_ID);
        }
        else if (strcmp(pSub->valuestring, "EXT_PAN_ID") == 0)
        {
            cJSON_AddStringToObject(pJson, "EXT_PAN_ID", gw->EXT_PAN_ID);
        }
        else if (strcmp(pSub->valuestring, "NETWORK_KEY") == 0)
        {
            cJSON_AddStringToObject(pJson, "NETWORK_KEY", gw->NETWORK_KEY);
        }
    }

    char *p = cJSON_PrintUnformatted(pJson);
    if (!p)
    {
        cJSON_Delete(rJson);
        cJSON_Delete(pJson);
        return -1;
    }

    if (strlen(p) >= out_len)
    {
        cJSON_Delete(rJson);
        cJSON_Delete(pJson);
        free(p);
        return -1;
    }

    strcpy(out, p);

    DPRINT("out: %s\n", out);

    cJSON_Delete(rJson);
    cJSON_Delete(pJson);
    free(p);

    return 0;
}

/*****************************************************************************
 * Function:             gateway_set_property
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static int gateway_set_property(char *in, void *ctx)
{
    gateway_t *gw = ctx;

    DPRINT("in: %s\n", in);

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    cJSON *ZB_Band = cJSON_GetObjectItem(rJson, "ZB_Band");
    if (ZB_Band)
        gw->ZB_Band = ZB_Band->valueint;

    cJSON *ZB_Channel = cJSON_GetObjectItem(rJson, "ZB_Channel");
    if (ZB_Channel)
        gw->ZB_Channel = ZB_Channel->valueint;

    cJSON *ZB_PAN_ID = cJSON_GetObjectItem(rJson, "ZB_PAN_ID");
    if (ZB_PAN_ID)
        strncpy(gw->ZB_PAN_ID, ZB_PAN_ID->valuestring, sizeof(gw->ZB_PAN_ID) - 1);

    cJSON *EXT_PAN_ID = cJSON_GetObjectItem(rJson, "EXT_PAN_ID");
    if (EXT_PAN_ID)
        strncpy(gw->EXT_PAN_ID, EXT_PAN_ID->valuestring, sizeof(gw->EXT_PAN_ID) - 1);

    cJSON *ZB_CO_MAC = cJSON_GetObjectItem(rJson, "ZB_CO_MAC");
    if (ZB_CO_MAC)
        strncpy(gw->ZB_CO_MAC, ZB_CO_MAC->valuestring, sizeof(gw->ZB_CO_MAC) - 1);

    cJSON *NETWORK_KEY = cJSON_GetObjectItem(rJson, "NETWORK_KEY");
    if (NETWORK_KEY)
        strncpy(gw->NETWORK_KEY, NETWORK_KEY->valuestring, sizeof(gw->NETWORK_KEY) - 1);

    linkkit_gateway_post_property_json_sync(gw->lk_dev, in, 5000);
    cJSON_Delete(rJson);

    return 0;
}

/*****************************************************************************
 * Function:             gateway_call_service
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static int gateway_call_service(char *identifier, char *in, char *out, int out_len, void *ctx)
{
    if (strcmp(identifier, "SetTimerTask") == 0)
    {
        snprintf(out, out_len, "{\"SetTimer\": \"hello, gateway!\"}");
    }
    else if (strcmp(identifier, "TimeReset") == 0)
    {
        DPRINT("TimeReset params: %s\n", in);
    }

    return 0;
}

/*****************************************************************************
 * Function:             post_all_properties
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static int post_all_properties(gateway_t *gw)
{
    cJSON *pJson = cJSON_CreateObject();

    if (!pJson)
        return -1;

    cJSON_AddNumberToObject(pJson, "ZB_Band", gw->ZB_Band);
    cJSON_AddNumberToObject(pJson, "ZB_Channel", gw->ZB_Channel);
    cJSON_AddStringToObject(pJson, "ZB_CO_MAC", gw->ZB_CO_MAC);
    cJSON_AddStringToObject(pJson, "ZB_PAN_ID", gw->ZB_PAN_ID);
    cJSON_AddStringToObject(pJson, "EXT_PAN_ID", gw->EXT_PAN_ID);
    cJSON_AddStringToObject(pJson, "NETWORK_KEY", gw->NETWORK_KEY);

    char *p = cJSON_PrintUnformatted(pJson);
    if (!p)
    {
        cJSON_Delete(pJson);
        return -1;
    }

    DPRINT("property: %s\n", p);

    linkkit_gateway_post_property_json_sync(gw->lk_dev, p, 5000);

    cJSON_Delete(pJson);
    free(p);

    return 0;
}

/*****************************************************************************
 * Function:             event_handler
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static int event_handler(linkkit_event_t *ev, void *ctx)
{
    gateway_t *gw = ctx;

    switch (ev->event_type)
    {
        case LINKKIT_EVENT_CLOUD_CONNECTED:
            DPRINT("cloud connected\n");

            post_all_properties(gw);    /* sync to cloud */
            gw->connected = 1;

            break;
        case LINKKIT_EVENT_CLOUD_DISCONNECTED:
            gw->connected = 0;
            DPRINT("cloud disconnected\n");
            break;
        case LINKKIT_EVENT_SUBDEV_DELETED:
        {
            char *productKey = ev->event_data.subdev_deleted.productKey;
            char *deviceName = ev->event_data.subdev_deleted.deviceName;
            DPRINT("delete subdev %s<%s>\n", productKey, deviceName);
        }
        break;
        case LINKKIT_EVENT_SUBDEV_PERMITED:
        {
            char *productKey = ev->event_data.subdev_permited.productKey;
            int timeoutSec = ev->event_data.subdev_permited.timeoutSec;
            DPRINT("permit subdev %s in %d seconds\n", productKey, timeoutSec);
        }
        break;
    }

    return 0;
}

static linkkit_cbs_t alink_cbs = {
    .get_property = gateway_get_property,
    .set_property = gateway_set_property,

    .call_service = gateway_call_service,
};

/*****************************************************************************
 * Function:             ota_callback
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static void ota_callback(int event, const char *version, void *ctx)
{
    DPRINT("event: %d\n", event);
    DPRINT("version: %s\n", version);

    linkkit_gateway_ota_update(512);
}

static inline void parseGatewayMac(char *mac)
{
    int i, j;
    char gwMac[32];

    memset(gwMac, 0, sizeof(gwMac));
    get_mac_address(gwMac);
    
    for (i = 0; gwMac[i] != '\0'; i++)
    {
        if (gwMac[i] != ':')
        {
            continue;
        }
        for (j = i; gwMac[j] != '\0'; j++)
        {
            gwMac[j] = gwMac[j + 1];
        }
        
        i--;
    }

    sprintf(mac, "0000%s", gwMac);

    DPRINT("gwMac[%s]mac[%s]\n", gwMac, mac);

}
/*****************************************************************************
 * Function:             run_ali_linkkit
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
int run_ali_linkkit(int channel)
{
    char gwMac[32];
    gateway_t gateway;
    
    memset(&gateway, 0, sizeof(gateway_t));

    /* fill fake zigbee network info */
    gateway.ZB_Band = FAKE_ZIGBEE_BAND;
    gateway.ZB_Channel = channel;

    memset(gwMac, 0, sizeof(gwMac));
    parseGatewayMac(gwMac);

    strcpy(gateway.ZB_PAN_ID, &gwMac[12]);
    strcpy(gateway.EXT_PAN_ID, gwMac);
    strcpy(gateway.ZB_CO_MAC, gwMac);
    strcpy(gateway.NETWORK_KEY, FAKE_NETWORK_KEY);

    linkkit_params_t *initParams = linkkit_gateway_get_default_params();
    if (!initParams)
    {
        DPRINT("linkkit_gateway_get_default_params failed\n");
        return -1;
    }

    int maxMsgSize = LINKKIT_MAX_MSG_SIZE;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_MAX_MSG_SIZE, &maxMsgSize, sizeof(int));

    int maxMsgQueueSize = LINKKIT_MAX_MSG_QUE_SIZE;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_MAX_MSG_QUEUE_SIZE, &maxMsgQueueSize, sizeof(int));

    int loglevel = LINKKIT_LOG_LEVEL;
    linkkit_gateway_set_option(initParams, LINKKIT_OPT_LOG_LEVEL, &loglevel, sizeof(int));

    linkkit_gateway_set_event_callback(initParams, event_handler, &gateway);

    if (linkkit_gateway_init(initParams) < 0)
    {
        DPRINT("linkkit_gateway_init failed\n");
        return -1;
    }

    gateway.lk_dev = linkkit_gateway_start(&alink_cbs, &gateway);
    if (gateway.lk_dev < 0)
    {
        DPRINT("linkkit_gateway_start failed\n");
        return -1;
    }

#if 0
    while (gateway.connected == 0)
        sleep(1);

#endif

    linkkit_gateway_ota_init(ota_callback, NULL);

    //light_init();

    lock_init();

#if 0
    while (1)
    {
        /*        linkkit_gateway_trigger_event_json_sync(gateway.lk_dev, "Error", "{\"ErrorCode\": 0}", 10000); */
        usleep(1000 * 1000 * 10);
    }

    light_exit();

    linkkit_gateway_stop(gateway.lk_dev);
    linkkit_gateway_exit();
#endif

    return 0;
}


