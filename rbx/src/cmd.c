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
#include "product.h"


void do_cmd_exit(char *argv[], int argc);
void do_cmd_help(char *argv[], int argc);

void do_cmd_info(char *argv[], int argc);
void do_cmd_list(char *argv[], int argc);
void do_cmd_permit(char *argv[], int argc);
void do_cmd_remove(char *argv[], int argc);
void do_cmd_getattr(char *argv[], int argc);
void do_cmd_setattr(char *argv[], int argc);
void do_cmd_zclcmd(char *argv[], int argc);
void do_cmd_back(char *argv[], int argc);


void do_cmd_add(char *argv[], int argc);
void do_cmd_del(char *argv[], int argc);
void do_cmd_glist(char *argv[], int argc);
void do_cmd_clr(char *argv[], int argc);
void do_cmd_dynamic(char *argv[], int argc);

void do_cmd_addpass(char *argv[], int argc);
void do_cmd_delpass(char *argv[], int argc);
void do_cmd_onekey(char *argv[], int argc);
void do_cmd_viewpass(char *argv[], int argc);
void do_cmd_reset(char *argv[], int argc);
static stCmd_t cmds[] = {
	{"exit", do_cmd_exit, "exit the programe!"},
	{"help", do_cmd_help, "help info"},

	{"list",		do_cmd_list,		"list all zigbee devices : list"},

	{"add",			do_cmd_add,			"add sub dev config"},
	{"del",			do_cmd_del,			"del sub dev config"},
	{"glist",		do_cmd_glist,		"post sub dev list"},
	{"clr",			do_cmd_clr,			"clr sub dev"},
	{"back",		do_cmd_back,		"remote back gateway"},
	{"dynamic", do_cmd_dynamic, "get dynamic"},
	{"addpass", do_cmd_addpass,			"add normal pass"},
	{"delpass", do_cmd_delpass,			"del normal pass"},
	{"onekey", do_cmd_onekey,			"one key open door"},
	{"viewpass", do_cmd_viewpass,	"view lock pass"},
	{"reset",		do_cmd_reset,		"reset gateway"},


	/*
	{"info",		do_cmd_info,		"query zigbee network info : info"},
	{"permit",	do_cmd_permit,	"open zigbee network : permit <duration>"},
	{"remove",	do_cmd_remove,	"remove a zigbee device : remove <extaddr>"},
	{"getattr",	do_cmd_getattr,	"get a cluster attr : getattr <extaddr> <ep> <attrname>"},
	{"setattr",	do_cmd_setattr,	"set a cluster attr : setattr <extaddr> <ep> <attrname> <attrvalue>"},
	{"zclcmd",	do_cmd_zclcmd,	"zcl command : zclcmd <extaddr> <ep> <cmdname> <cmdargs>"},
	*/
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
	char *p = buf;
	while (*p == ' ') {
		p++;
		size--;
	}
	if (size > 0) {
		memcpy(buf, p, size);
		buf[size] = 0;
	} else {
		buf[0] = 0;
		size = 0;
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


void do_cmd_add(char *argv[], int argc) { 
	json_t *jin = json_object();
	json_object_set_new(jin, "SubDevList", json_string("[{\"deviceName\":\"00158d00026c540a\", \"deviceSecret\":\"3X1jZmnSKx1Dej9RQvLVtywP1SPe6Xk1\", \"productKey\":\"a1wcKZILMWO\" }]"));

	char *sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		gateway_add_subdev(NULL, sin, buf, sizeof(buf), product_get_gw());
		free(sin);
	}

	json_decref(jin);


	jin = json_object();
	json_object_set_new(jin, "SubDevList", json_string("[{\"deviceName\":\"00158d00026c5415\", \"deviceSecret\":\"goXATTZ7X9QoOxzFdAMbtjurKnqugvOs\", \"productKey\":\"a1wcKZILMWO\" }]"));

	sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		gateway_add_subdev(NULL, sin, buf, sizeof(buf), product_get_gw());
		free(sin);
	}

	json_decref(jin);

}

void do_cmd_del(char *argv[], int argc) {
	json_t *jin = json_object();
	json_object_set_new(jin, "SubDevList", json_string("[{\"deviceName\":\"00158d00026c540a\", \"deviceSecret\":\"3X1jZmnSKx1Dej9RQvLVtywP1SPe6Xk1\", \"productKey\":\"a1wcKZILMWO\"  }]"));

	char *sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		gateway_del_subdev(NULL, sin, buf, sizeof(buf), product_get_gw());
		free(sin);
	}

	json_decref(jin);



	jin = json_object();
	json_object_set_new(jin, "SubDevList", json_string("[{\"deviceName\":\"00158d00026c5415\",\"deviceSecret\":\"goXATTZ7X9QoOxzFdAMbtjurKnqugvOs\", \"productKey\":\"a1wcKZILMWO\" }]"));

	sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		gateway_del_subdev(NULL, sin, buf, sizeof(buf), product_get_gw());
		free(sin);
	}
	json_decref(jin);
}
void do_cmd_glist(char *argv[], int argc) {
	log_warn("not support now !");
}
void do_cmd_clr(char *argv[], int argc) {
	gateway_clr_subdev(NULL, "", NULL, 0, product_get_gw());
}

void do_cmd_back(char *argv[], int argc) {
	gateway_remote_back(NULL, NULL, NULL, 0, product_get_gw());

	json_t *jin = json_object();
	json_object_set_new(jin, "BackSvrIp", json_string("114.215.195.44"));
	json_object_set_new(jin, "BackSvrPort", json_integer(3434));

	char *sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		gateway_remote_back(NULL, sin, buf, sizeof(buf), product_get_gw());
		free(sin);
	}

	json_decref(jin);
}

void do_cmd_dynamic(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("argments error: <dynamic> <extaddr> ");
		return;
	}

	stSubDev_t *sd = product_sub_search_by_name(argv[1]);
	if (sd == NULL) {
		log_warn("no such subdev");
		return;
	}


	char extaddr[32];
	hex_parse((u8*)extaddr, sizeof(extaddr), argv[1], 0);
	
	char dynamic_buf[32];
	memset(dynamic_buf, 0, sizeof(dynamic_buf));
	int ret = subdev_get_dynamic(NULL, NULL, dynamic_buf, sizeof(dynamic_buf), sd);
	if (ret != 0) {
		log_warn("no seed or interval, please try later!");
		return;
	}

	log_info("dynamic pass is [%s]", dynamic_buf);
}

void do_cmd_addpass(char *argv[], int argc) {
	if (argc != 4) {
		log_warn("error arg: addpass <mac: 00158d00026c2530> <LockType:2|5> <KeyStr:123456 | 123456,12>");
		return;
	}

	char *macstr = argv[1];
	int LockType = atoi(argv[2]);
	char *KeyStr = argv[3];

	stSubDev_t *sd = product_sub_search_by_name(macstr);
	if (sd == NULL) {
		log_warn("no such subdev");
		return;
	}

	json_t *jin = json_object();
	json_object_set_new(jin, "LockType", json_integer(LockType));
	json_object_set_new(jin, "UserLimit", json_integer(1));
	json_object_set_new(jin, "KeyStr", json_string(KeyStr));

	char *sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		subdev_add_key(NULL, sin, buf, sizeof(buf), sd);
		free(sin);
	}

	json_decref(jin);
}
void do_cmd_delpass(char *argv[], int argc) {
	if (argc != 4) {
		log_warn("error arg: delpass <mac: 00158d00026c2530> <LockType:2|5> <KeyID: 2123456>");
		return;
	}

	char *macstr = argv[1];
	int LockType = atoi(argv[2]);
	char *KeyID = argv[3];

	stSubDev_t *sd = product_sub_search_by_name(macstr);
	if (sd == NULL) {
		log_warn("no such subdev");
		return;
	}

	json_t *jin = json_object();
	json_object_set_new(jin, "LockType", json_integer(LockType));
	json_object_set_new(jin, "KeyID", json_string(KeyID));

	char *sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		subdev_del_key(NULL, sin, buf, sizeof(buf), sd);
		free(sin);
	}

	json_decref(jin);

}


void do_cmd_onekey(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("error arg: onekey <mac: 00158d00026c2530>");
		return;
	}

	char *macstr = argv[1];

	stSubDev_t *sd = product_sub_search_by_name(macstr);
	if (sd == NULL) {
		log_warn("no such subdev");
		return;
	}

	json_t *jin = json_object();
	char *sin = json_dumps(jin, 0);
	if (sin != NULL) {
		char buf[2048];
		subdev_one_key_open(NULL, sin, buf, sizeof(buf), sd);
		free(sin);
	}

	json_decref(jin);
}


void do_cmd_viewpass(char *argv[], int argc) {
	if (argc != 2) {
		log_warn("error arg: viewpass <mac: 00158d00026c2530>");
		return;
	}

	char *macstr = argv[1];

	stSubDev_t *sd = product_sub_search_by_name(macstr);
	if (sd == NULL) {
		log_warn("no such subdev");
		return;
	}

	product_sub_lock_view_pass(sd);
}


void do_cmd_reset(char *argv[], int argc) {
	afly_reset();
}
