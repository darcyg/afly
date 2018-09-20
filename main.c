#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>


#include "log.h"
#include "list.h"
#include "common.h"
#include "timer.h"
#include "file_event.h"


#include "afly.h"
#include "cmd.h"


static void	run_main();
static void ds_child_exit_handler(int s);
static void ds_sig_exit_handler(int s);
static void ds_sigpipe_handler(int s);
static void ds_exit_handler(void);
static void sig_set();

static int parse_args(int argc, char *argv[]);
static int usage(char *app);
static int write_pid();
static void write_version(const char *verfile);


static int ds_child_died = 0;
static int port = 8888;
static int loglvl = 0;
static int use_cmd = 0;
struct timer_head th = {.first = NULL};
static char *version_path = "/etc/config/dusun/afly/version";
static char *pid_file = "/var/run/afly.pid";

int main(int argc, char *argv[]) {
	sig_set();

	log_init(argv[0], LOG_OPT_DEBUG | LOG_OPT_CONSOLE_OUT | LOG_OPT_TIMESTAMPS | LOG_OPT_FUNC_NAMES);

	if (parse_args(argc, argv) != 0) {
		usage(argv[0]);
		return -1;
	}

	if (write_pid() != 0) {
		log_err("%s has startted!", argv[0]);
		return -2;
	}

	system("mkdir -p /etc/config/dusun/afly/");
	write_version(version_path);

	run_main();


	return 0;
}

static void ds_child_exit_handler(int s) {
	log_info("[%s] %d", __func__, __LINE__);
	ds_child_died = 1;
}
static void ds_sig_exit_handler(int s) {
	log_info("[%s] %d : %d", __func__, __LINE__, s);
	exit(1);
}
static void ds_sigpipe_handler(int s) {
	log_info("[%s] %d : %d", __func__, __LINE__, s);
}
static void ds_exit_handler(void) {
	log_info("[%s] %d", __func__, __LINE__);
}


static void sig_set() {
	log_info("[%s] %d", __func__, __LINE__);

	struct sigaction sigHandler;

	memset(&sigHandler, 0, sizeof(sigHandler));

	sigHandler.sa_handler = ds_sig_exit_handler;
	sigemptyset(&sigHandler.sa_mask);
	sigaction(SIGINT, &sigHandler, NULL);
	sigaction(SIGTERM, &sigHandler, NULL);

	sigHandler.sa_handler = ds_child_exit_handler;
	sigaction(SIGCHLD, &sigHandler, NULL);

	sigHandler.sa_handler = ds_sigpipe_handler;
	sigaction(SIGPIPE, &sigHandler, NULL);

	atexit(ds_exit_handler);
}

static int parse_args(int argc, char *argv[]) {
	int ch = 0;
	while((ch = getopt(argc,argv,"P:L:C"))!= -1){
		switch(ch){
			case 'P':
				port = atoi(optarg);
				break;
			case 'C':
				use_cmd = 1;
				break;
			case 'L':
				loglvl = atoi(optarg);
				break;
			default:
				return -1;
				break;
		}
	}
	return 0;
}

static int usage(char *app) {
	printf(	"Usage: %s [options] ...\n"
					"Options:\n"
					"  -P                       Udp port to listen.\n"
					"  -L                       Debug Level (0 ~ 5).\n"
					"   C                       cmd.\n"
					"For more infomation, please mail to dlauciende@gmail.com\n", app);
	return 0;
}


static int write_pid() {
	int fd = -1;
	if (access(pid_file, F_OK) != 0) {
		fd = open(pid_file, O_WRONLY | O_CREAT, 0644);
	} else {
		fd = open(pid_file, O_WRONLY);
	}
	
	if (fd < 0) {
		return -1;
	}
	
	if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
		return -2;
	}
	
	char buf[64];
	
	sprintf(buf, "%d\n", (int)getpid());
	if (write(fd, buf, strlen(buf)) != strlen(buf)) {
		return -3;
	}
	
	return 0;
}

static void write_version(const char *verfile) {
	FILE *fp = fopen(verfile, "w");
	if (fp == NULL) {
		return;
	}

	char buf[256];
	sprintf(buf, "Version:%d.%d.%d, DateTime:%s %s\n", MAJOR, MINOR, PATCH, TIME, DATE);
	fwrite(buf, strlen(buf), 1, fp);
	fclose(fp);
}


static void timerout_cb(struct timer *t) {
	log_info("[%s] %d : %p", __func__, __LINE__, t);
}

static void	run_main() {
	log_info("[%s] %d", __func__, __LINE__);


	struct timer tr;
	timer_init(&tr, timerout_cb);

	struct file_event_table fet;
	file_event_init(&fet);


	afly_init(&th, &fet, loglvl);	

	uproto_init(&th, &fet);
	
	if (use_cmd) {
		cmd_init(&th, &fet);
	}

	timer_set(&th, &tr, 10);
	log_info("[%s] %d : goto main loop", __func__, __LINE__);
	while (1) {
		s64 next_timeout_ms;
		next_timeout_ms = timer_advance(&th);
		if (file_event_poll(&fet, next_timeout_ms) < 0) {
			log_warn("poll error: %m");
		}
	}
}
