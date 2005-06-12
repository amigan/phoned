/*
 * phoned initialization
 * (C)2005, Dan Ponte
 * BSD license
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include <phoned.h>

extern FILE*	logf;
extern pthread_mutex_t logfmx;
extern short	difflog;
extern struct conf	cf;
extern pthread_mutex_t cfmx;
extern cond_t* topcond;

void shutd(void)
{
	lprintf(fatal, "phoned shutting down...\n");
	pthread_mutex_lock(&cfmx);
	close_modem(cf.modemdev);
	pthread_mutex_unlock(&cfmx);
	flush_lists();
	free_condition(topcond, 0x1);
	pthread_mutex_lock(&logfmx);
	fclose(logf);
	pthread_mutex_unlock(&logfmx);
	unlink(SOCKETFILE);
}

void open_logs(void)
{
	pthread_mutex_lock(&cfmx);
	if(strcmp(difflog ? cf.logfile : LOGFILE, "-") == 0) logf = stderr;
	else {
		logf = fopen(difflog ? cf.logfile : LOGFILE, "a");
		if(!logf) {
			perror("logf open");
			exit(-1);
		}
	}
	pthread_mutex_unlock(&cfmx);
	lprintf(info, "phoned v" VERSION " starting..\n");
}

void initialize(void)
{
	open_logs();
	install_handlers();
	read_config();
	pthread_mutex_lock(&cfmx);
	if(init_modem(cf.modemdev) != 1) {
		lprintf(warn, "warning: modem didn't initialise properly; see previous messages\n");
	}
	pthread_mutex_unlock(&cfmx);
}

