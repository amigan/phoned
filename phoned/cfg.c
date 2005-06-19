/*
 * FUCK....lex overwrote this :-(
 * (C)2005, Dan Ponte...again.
 */
/* $Amigan: phoned/phoned/cfg.c,v 1.6 2005/06/19 00:04:06 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define HAVE_INET_INCS
#include <phoned.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
extern addrsll_t *top;
struct conf	cf;
extern pthread_mutex_t	addrmx;
pthread_mutex_t	cfmx;
short read_config(void)
{
	FILE* con;
	pthread_mutex_lock(&cfmx);
	con = fopen(cf.cfile, "r");
	pthread_mutex_unlock(&cfmx);
	if(!con) {
		lprintf(error, "error opening config file: %s\n", strerror(errno));
		return 0;
	}
	if(parse(&con) == -1) {
		lprintf(error, "error parsing\n");
		fclose(con);
		return 0;
	}
	fclose(con);
	return 1;
}
void addtoaddrs(const char* par)
{
	addrsll_t *l;
	pthread_mutex_lock(&addrmx);
	l = getlast(top);
#ifdef DEBUG
	if(l->next != 0x0) {
		lprintf(fatal, "your last routine is broken, moron! fix it!\n");
		return;
	}
#endif
	l->next = allocaddr();
	l = l->next;
	l->addr = inet_addr(par);
	pthread_mutex_unlock(&addrmx);
}
	
