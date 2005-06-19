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
#include <errno.h>

#include <phoned.h>

extern FILE*	logf;
extern pthread_mutex_t logfmx;
extern struct conf	cf;
extern pthread_mutex_t cfmx;
extern cond_t* topcond;
#define WD_LOGS		0x1
#define WD_HANDLERS	0x2
#define WD_CONFIG	0x4
#define WD_MODEM	0x10
#define	WD_DBINIT	0x20

void shutd(whatdone)
	int whatdone;
{
	lprintf(fatal, "phoned shutting down...\n");
	pthread_mutex_lock(&cfmx);
	lprintf(info, "got cf lock");
	if(whatdone & WD_MODEM) close_modem(cf.modemdev);
	pthread_mutex_unlock(&cfmx);
	flush_lists();
	free_condition(topcond, 0x1);
	pthread_mutex_lock(&logfmx);
	pthread_mutex_unlock(&logfmx);
	if(whatdone & WD_DBINIT) db_destroy();
	lprintf(info, "got log lock");
	if(whatdone & WD_LOGS) fclose(logf);
	unlink(SOCKETFILE);
}

int open_logs(void)
{
	pthread_mutex_lock(&cfmx);
	if(strcmp(cf.logfile, "-") == 0) logf = stderr;
	else {
		logf = fopen(cf.logfile, "a");
		if(!logf) {
			lprintf(error, "logf open: %s\n", strerror(errno));
			return 0;
		}
	}
	pthread_mutex_unlock(&cfmx);
	lprintf(info, "phoned v" VERSION " starting..\n");
	return 1;
}

void initialize(void)
{
	int whatdone = 0;
	if(!open_logs()) {
		fprintf(stderr, "Logs won't open!\n");
		shutd(whatdone);
		exit(-1);
	} else whatdone |= WD_LOGS;
	install_handlers(); whatdone |= WD_HANDLERS;
	if(!read_config()) {
		lprintf(fatal, "fatal: error reading config, see earlier messages\n");
		shutd(whatdone);
		exit(-1);
	} else whatdone |= WD_CONFIG; /* XXX: if you put anything before this, edit config.y's bitmask on shutd() */
	pthread_mutex_lock(&cfmx);
	if(init_modem(cf.modemdev) != 1) {
		lprintf(fatal, "fatal error: modem didn't initialise properly; see previous messages\n");
		shutd(whatdone);
		exit(-1);
	} else whatdone |= WD_MODEM;
	if(!db_init(cf.dbfile)) {
		lprintf(fatal, "fatal error: database did not open, see previous messages\n");
		shutd(whatdone);
		exit(-1);
	} else whatdone |= WD_DBINIT;
	pthread_mutex_unlock(&cfmx);
}

