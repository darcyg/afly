/******************************************************************************
*  Copyright 2016-2018 Hangzhou ZiLai Sensing Technology Co., Ltd.
*  FileName: translation.c
*  Description:
*  Author: ding.yuxiang
*  Create Date: 2018-06-21
*  Modification History:
*   <version> <time>  <author> <desc>
*  a)
******************************************************************************/

#include <dusun/dusun.h>
#include <osi/libosi.h>

/*****************************************************************************
 * Function:             linkkitParseJson
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
int linkkitParseJson(json *packet)
{
    const char *to = NULL;
    const char *from = NULL;
    const char *type = NULL;
    json *data = NULL;

    to = json_get_str(packet, TAG_TO);
    if (to == NULL)
    {
        LOG("[%s]to N/A!\n", __FUNCTION__);
        return ERROR;
    }

    if (strcmp(to, MOD_CLOUD) != 0)
    {
        LOG("[%s]to[%s] Not Qualified!\n", __FUNCTION__, to);
        return ERROR;
    }

    from = json_get_str(packet, TAG_FROM);
    type = json_get_str(packet, TAG_TYPE);
    data = json_get_obj(packet, TAG_DATA);
    if (from == NULL || type == NULL || data == NULL)
    {
        LOG("[%s]from[%p]/type[%p]/data[%p] N/A!\n",
            __FUNCTION__, from, type, data);
        return ERROR;
    }

    LOG("[%s]type: %s", __FUNCTION__, type);
    if (strcmp(type, TYPE_REPORT_ATTRIBUTE) == 0)
    {
        /* linkkitHandleReportAttr(data); */
        return OK;
    }
    else if (strcmp(type, TYPE_CMD_RESULT) == 0)
    {
        /* linkkitHandleCmdResult(data); */
        return OK;
    }
    else
    {
        LOG("[%s]unknow type: %s", __FUNCTION__, type);
    }

    return OK;
}

