/*
 * FUCK....lex overwrote this :-(
 * (C)2005, Dan Ponte...again.
 */
/* $Amigan: phoned/phoned/cfg.c,v 1.3 2005/06/02 02:40:52 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define HAVE_INET_INCS
#include <phoned.h>
#include <unistd.h>
#include <stdlib.h>
extern addrsll_t *top;
struct conf	cf;
void read_config(void)
{
	FILE* con;
	con = fopen(cf.cfile, "r");
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
}
	
