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

void shutd(void)
{
	lprintf(fatal, "phoned shutting down...\n");
	fclose(logf);
	unlink(SOCKETFILE);
}

void open_logs(void)
{
	logf = fopen(difflog ? cf.logfile : LOGFILE, "a");
	if(!logf) {
		perror("logf open");
		exit(-1);
	}
	lprintf(info, "phoned v" VERSION " starting..\n");
}

void initialize(void)
{
	open_logs();
	install_handlers();
	read_config();
}

