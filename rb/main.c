/******************************************************************************
*  Copyright 2016-2018 Hangzhou ZiLai Sensing Technology Co., Ltd.
*  FileName: main.c
*  Description:
*  Author: ding.yuxiang
*  Create Date: 2018-06-22
*  Modification History:
*   <version> <time>  <author> <desc>
*  a)v1.0.0v2018-06-22 ding.yuxiang create this file
******************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>

#include <dusun/dusun.h>

#include "gateway.h"


int linkkitParseJson(json *packet);
void linkkit_cmd_loop_start(void);
int run_ali_linkkit(void);


/*****************************************************************************
 * Function:             write_pid
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static int write_pid()
{
    int fd = -1;

    if (access(RUN_PID_DIR, F_OK) != 0)
    {
        fd = open(RUN_PID_DIR, O_WRONLY | O_CREAT, 0644);
    }
    else
    {
        fd = open(RUN_PID_DIR, O_WRONLY);
    }

    if (fd < 0)
    {
        return -1;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) < 0)
    {
        return -2;
    }

    char buf[64];

    sprintf(buf, "%d\n", (int)getpid());
    if (write(fd, buf, strlen(buf)) != strlen(buf))
    {
        return -3;
    }

    return 0;
}

static attribute_handler attribute_handlers[] =
{
    {NULL, NULL, NULL}
};


/*
 * function: send_pkt_to_mod
 *
 * proto模块的回调函数。负责将报文通过MQTT或UBUS发送给各个模块
 * packet的释放由此函数负责
 */
static bool send_pkt_to_mod(const char *mod, json *packet)
{
    LOG("send_pkt_to_mod");
    if (!strcmp(mod, MOD_CLOUD))
    {
        /* SEND to linkkit */
        LOG("report json to netserver.");
        linkkitParseJson(packet);
        json_put(packet);
    }
    else
    {
        /* SEND ON UBUS */
        char dest[40];
        sprintf(dest, "DS.%s", mod);
        json *obj = json_new();
        json_add_str(obj, "PKT", json_to_str(packet));
        ubus_send(dest, obj);
        json_put(packet);
    }

    return true;
}




/*****************************************************************************
 * Function:             main
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
int main(int argc, char *argv[])
{
    INIT_LOG(PROCESS_NAME);
    
    if (write_pid() != 0)
    {
        LOG("Warning: %s Program Has Been Running!", PROCESS_NAME);
        return -1;
    }

    if (argc >= 2 && !strcmp(argv[1], "cmd")) 
    {
        linkkit_cmd_loop_start();
    }

    proto_init(MOD_GATEWAY, attribute_handlers, send_pkt_to_mod, NULL);
    proto_gw_set_factory_license("dusun", "dusun");
    //timer_init();

    run_ali_linkkit();
    
    proto_default_ubus_loop();

    return 0;

}

