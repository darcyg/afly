/******************************************************************************
*  Copyright 2002-2016 Hangzhou Dusun Digital Technology Co., Ltd.
*  FileName: cmd.c
*  Description:
*  Author:
*  Create Date:
*  Modification History:
*   <version> <time>  <author> <desc>
*  a)
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include <dusun/dusun.h>

static pthread_t cmd_thread;

typedef void (*cmd_handler)(int argc, char *argv[]);
void linkkit_cmd_help(int argc, char *argv[]);
void linkkit_cmd_dump(int argc, char *argv[]);
void linkkit_cmd_setpass(int argc, char *argv[]);
void linkkit_cmd_quit(int argc, char *argv[]);
void linkkit_cmd_nxpreset(int argc, char *argv[]);

struct CMD
{
    char *name;
    cmd_handler handler;
    char *usage;
} cmds[] =
{
    {"help", linkkit_cmd_help, "help"},
    {"quit", linkkit_cmd_quit, "terminate program"},
    {"dump", linkkit_cmd_dump, "dump system state"},
};


/*****************************************************************************
 * Function:             ns_cmd_loop
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
static void linkkit_cmd_loop(void)
{
    char *argv[100];
    char *p;
    char *name;
    int argc;
    char cmd[1024];
    int i = 0;

    while (1)
    {
        printf("\ncmd# ");
        cmd[0] = '\0';
        fgets(cmd, 1024, stdin);
        if (strlen(cmd) <= 2)
            continue;

        cmd[strlen(cmd) - 1] = '\0';
        argc = 0;
        p = strtok(cmd, " ");
        name = p;
        if (!name)
            continue;

        while (p)
        {
            argv[argc] = p;
            argc++;
            p = strtok(NULL, " ");
        }

        for (i = 0; i < sizeof(cmds) / sizeof(struct CMD); i++)
        {
            if (!strcmp(cmds[i].name, name))
            {
                cmds[i].handler(argc, argv);
                continue;
            }
        }
    }
}

/*****************************************************************************
 * Function:             ns_cmd_loop_start
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
void linkkit_cmd_loop_start(void)
{
    pthread_create(&cmd_thread, NULL, (void *) linkkit_cmd_loop, NULL);
}

/*****************************************************************************
 * Function:             ns_cmd_help
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
void linkkit_cmd_help(int argc, char *argv[])
{
    int i = 0;

    printf("\nusage:");
    for (i = 0; i < sizeof(cmds) / sizeof(struct CMD); i++)
    {
        printf("\n\t%s : %s", cmds[i].name, cmds[i].usage);
    }
}

/*****************************************************************************
 * Function:             ns_cmd_dump
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
void linkkit_cmd_dump(int argc, char *argv[])
{
    /* cloud_dump(); */
}

/*****************************************************************************
 * Function:             ns_cmd_quit
 * Description:          TODO
 * Input:                TODO
 * Output:               N/A
 * Return:
 *   OK:                    Successful
 *   ERROR:                 Failed
 ******************************************************************************/
void linkkit_cmd_quit(int argc, char *argv[])
{
    sync();
    exit(-1);
}


