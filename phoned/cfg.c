/*
 * FUCK....lex overwrote this :-(
 * (C)2005, Dan Ponte...again.
 */
/* $Amigan: phoned/phoned/cfg.c,v 1.5 2005/06/12 23:05:12 dcp1990 Exp $ */
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
extern addrsll_t *top;
struct conf	cf;
extern pthread_mutex_t	addrmx;
pthread_mutex_t	cfmx;
void read_config(void)
{
	FILE* con;
	pthread_mutex_lock(&cfmx);
	con = fopen(cf.cfile, "r");
	pthread_mutex_unlock(&cfmx);
	if(!con) {
		perror("error opening config file");
		exit(-1);
	}
	parse(&con);
	fclose(con);
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
	
