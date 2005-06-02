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

#include <phoned.h>
extern struct conf cf;
short	difflog = 0;

void usage(argv)
	const char* argv;
{
	fprintf(stderr, "%s: usage: %s [-hd] [-c config] [-l log]\n", argv, argv);
}

int main(argc, argv)
	int argc;
	char* argv[];
{
	int c;
	cf.cfile = CONFIGFILE;
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
				difflog = 1;
				break;
			case 'd':
				daemon(0, 0);
				break;
			default:
				usage(strdup(*argv));
				return -2;
		}
	cf.loglevels = LL_ALL;
	initialize();
	network();
	shutd();
	return 0;
}
