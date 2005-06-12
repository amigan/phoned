/*
 * phoned initialization
 * (C)2005, Dan Ponte
 * BSD license
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <phoned.h>

extern FILE*	logf;
extern short	difflog;
extern struct conf	cf;
extern cond_t* topcond;

void shutd(void)
{
	lprintf(fatal, "phoned shutting down...\n");
	close_modem(cf.modemdev);
	flush_lists();
	free_condition(topcond, 0x1);
	fclose(logf);
	unlink(SOCKETFILE);
}

void open_logs(void)
{
	if(strcmp(difflog ? cf.logfile : LOGFILE, "-") == 0) logf = stderr;
	else {
		logf = fopen(difflog ? cf.logfile : LOGFILE, "a");
		if(!logf) {
			perror("logf open");
			exit(-1);
		}
	}
	lprintf(info, "phoned v" VERSION " starting..\n");
}

void initialize(void)
{
	open_logs();
	install_handlers();
	read_config();
	if(init_modem(cf.modemdev) != 1) {
		lprintf(warn, "warning: modem didn't initialise properly; see previous messages\n");
	}
}

