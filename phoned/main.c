/*
 * main.c - phoned main
 * (C)2005, Dan Ponte
 * BSD license with advert. clause.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#include <phoned.h>
extern struct conf cf;
extern pthread_mutex_t cfmx;
extern pthread_t networkth;
extern pthread_t modemth;
pthread_t mainth;
pthread_cond_t maincond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mainmx = PTHREAD_MUTEX_INITIALIZER;
void usage(argv)
	const char* argv;
{
	fprintf(stderr, "%s: usage: %s [-hd] [-c config] [-l log]\n", argv, argv);
}
void *mainthf(v)
	void *v;
{
	if(v == 0) v = 0;
	pthread_mutex_lock(&mainmx);
	pthread_cond_wait(&maincond, &mainmx);
	pthread_mutex_unlock(&mainmx);
	exit(0);
}
int main(argc, argv)
	int argc;
	char* argv[];
{
	int c;
	pthread_mutex_lock(&cfmx);
	cf.cfile = CONFIGFILE;
	cf.dbfile = DBFILE;
	cf.logfile = LOGFILE;
#define OPTSTRING "dhc:l:"
	while((c = getopt(argc, argv, OPTSTRING)) != -1)
		switch(c) {
			case 'c':
				cf.cfile = strdup(optarg);
				break;
			case 'h':
				usage(strdup(*argv));
				return 0;
			case 'l':
				cf.logfile = strdup(optarg);
				break;
			case 'd':
				daemon(1, 0);
				break;
			default:
				usage(strdup(*argv));
				return -2;
		}
	cf.modem_tm = 0;
	cf.loglevels = LL_ALL;
	pthread_mutex_unlock(&cfmx);
	initialize();
	pthread_create(&networkth, NULL, network, NULL);
	pthread_create(&modemth, NULL, modem_io, NULL);
	pthread_create(&mainth, NULL, mainthf, NULL);
	pthread_exit(NULL);
	return 0;
}
