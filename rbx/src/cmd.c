#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "log.h"
#include "parse.h"
#include "file_event.h"
#include "jansson.h"
#include "json_parser.h"
#include "hex.h"

#include "cmd.h"
//#include "zigbee.h"
#include "product_sub.h"


void do_cmd_exit(char *argv[], int argc);
void do_cmd_help(char *argv[], int argc);

void do_cmd_info(char *argv[], int argc);
void do_cmd_list(char *argv[], int argc);
void do_cmd_permit(char *argv[], int argc);
void do_cmd_remove(char *argv[], int argc);
void do_cmd_getattr(char *argv[], int argc);
void do_cmd_setattr(char *argv[], int argc);
void do_cmd_zclcmd(char *argv[], int argc);

static stCmd_t cmds[] = {
	{"exit", do_cmd_exit, "exit the programe!"},
	{"help", do_cmd_help, "help info"},

	{"info",		do_cmd_info,		"query zigbee network info : info"},
	{"list",		do_cmd_list,		"list all zigbee devices : list"},
	{"permit",	do_cmd_permit,	"open zigbee network : permit <duration>"},
	{"remove",	do_cmd_remove,	"remove a zigbee device : remove <extaddr>"},
	{"getattr",	do_cmd_getattr,	"get a cluster attr : getattr <extaddr> <ep> <attrname>"},
	{"setattr",	do_cmd_setattr,	"set a cluster attr : setattr <extaddr> <ep> <attrname> <attrvalue>"},
	{"zclcmd",	do_cmd_zclcmd,	"zcl command : zclcmd <extaddr> <ep> <cmdname> <cmdargs>"},
};

static stCmdEnv_t ce;

int cmd_init(void *_th, void *_fet) {
	ce.th = _th;
	ce.fet = _fet;

	timer_init(&ce.step_timer, cmd_run);
	lockqueue_init(&ce.eq);

	ce.fd = 0;
	file_event_reg(ce.fet, ce.fd, cmd_in, NULL, NULL);

	return 0;
}
int cmd_step() {
	timer_cancel(ce.th, &ce.step_timer);
	timer_set(ce.th, &ce.step_timer, 10);
	return 0;
}
int cmd_push(stEvent_t *e) {
	lockqueue_push(&ce.eq, e);
	cmd_step();
	return 0;
}
void cmd_run(struct timer *timer) {
	stEvent_t *e;

	if (!lockqueue_pop(&ce.eq, (void**)&e)) {
		return;
	}

	if (e == NULL) {
		FREE(e);
	}

	cmd_step();
}
void cmd_in(void *arg, int fd) {
	char buf[1024];	
	int  size = 0;

	int ret = read(ce.fd, buf, sizeof(buf));
	if (ret < 0) {
		log_warn("what happend?");
		return;
	}

	if (ret == 0) {
		log_warn("error!");
		return;
	}

	size = ret;
	buf[size] = 0;
	if (size >= 1 && buf[size-1] == '\n') {
		buf[size-1] = 0;
		size--;
	}

	if (strcmp(buf, "") != 0) {
		log_info("console input:[%s]", buf);
		char* argv[20];
		int argc;
		argc = parse_argv(argv, sizeof(argv), buf);

		stCmd_t *cmd = cmd_search(argv[0]);
		if (cmd == NULL) {
			log_warn("invalid cmd!");
		} else {
			cmd->func(argv, argc);
		}
	}
	log_info("$");
}

stCmd_t *cmd_search(const char *cmd) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		if (strcmp(cmds[i].name, cmd) == 0) {
			return &cmds[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////
void do_cmd_exit(char *argv[], int argc) {
	exit(0);
}
void do_cmd_help(char *argv[], int argc) {
	int i = 0;
	for (i = 0; i < sizeof(cmds)/sizeof(cmds[0]); i++) {
		log_info("%-12s\t-\t%s", cmds[i].name, cmds[i].desc);
	}
}

void do_cmd_info(char *argv[], int argc) {
	char infostr[256];
	//zigbee_network_information_get(infostr, sizeof(infostr));

	log_info("info : [%s]", infostr);
}
void do_cmd_list(char *argv[], int argc) {
	//char liststr[512];
	//zigbee_list(liststr, sizeof(liststr));
	
	log_info("SubDev List:");
	product_sub_view();
	
	//log_info("list: [%s]", liststr);
}
void do_cmd_permit(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("argments error: <permit> <duraion>");
		return;
	}

	//int duration = atoi(argv[1]);
	//zigbee_permit_join(duration);
}
void do_cmd_remove(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("argments error: <remove> <extaddr>");
		return;
	}

	char extaddr[32];
	hex_parse((u8*)extaddr, sizeof(extaddr), argv[1], 0);

	//zigbee_remove_device(extaddr);
}
void do_cmd_getattr(char *argv[], int argc) {
	if (argc != 4) {
		log_warn("argments error: <getattr> <extaddr> <ep> <attrname>");
		return;
	}

	char extaddr[32];
	hex_parse((u8*)extaddr, sizeof(extaddr), argv[1], 0);
	char ep;
	hex_parse((u8*)&ep, sizeof(ep), argv[2],0);
	//const char *attr_name = argv[3];

	//zigbee_get_attr(extaddr, ep, attr_name);
}
void do_cmd_setattr(char *argv[], int argc) {
	if (argc != 5) {
		log_warn("argments error: <setattr> <extaddr> <ep> <attrname> <attrvalue>");
		return;
	}

	char extaddr[32];
	hex_parse((u8*)extaddr, sizeof(extaddr), argv[1], 0);
	char ep;
	hex_parse((u8*)&ep, sizeof(ep), argv[2],0);
	//const char *attr_name = argv[3];
	//const char *attr_value = argv[4];

	//zigbee_set_attr(extaddr, ep, attr_name, attr_value);
}
void do_cmd_zclcmd(char *argv[], int argc) {
	if (argc != 5) {
		log_warn("argments error: <zclcmd> <extaddr> <ep> <cmdname> <cmdargs>");
		return;
	}

	char extaddr[32];
	hex_parse((u8*)extaddr, sizeof(extaddr), argv[1], 0);
	char ep;
	hex_parse((u8*)&ep, sizeof(ep), argv[2],0);
	//const char *cmd_name = argv[3];
	//const char *cmd_arg = argv[4];

	//zigbee_zclcmd(extaddr, ep, cmd_name, cmd_arg);

}


