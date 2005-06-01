/*
 * FUCK....lex overwrote this :-(
 * (C)2005, Dan Ponte...again.
 */
/* $Amigan: phoned/phoned/cfg.c,v 1.2 2005/06/01 00:43:07 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <phoned.h>
#include <unistd.h>
#include <stdlib.h>
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
