%union
{
	int number;
	char* string;
}

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <phoned.h>
int chrcnt = 0;
int lincnt = 1;
int yylex(void);
char *numrx = 0x0, *namrx = 0x0;
int factn = 0x0;
int ctflags = 0x0;
extern char* yytext;
extern struct conf cf;
void yyerror(str)
	char* str;
{
	lprintf(fatal, "parser: error: %s at line %d chr %d (near %s)\n", str,
	 lincnt, chrcnt, yytext);
	exit(-1);
}
int yywrap(void)
{
	return 1;
}
%}
%token NOTIFY OBRACE CBRACE SCOLON QUOTE MODDEV MAIN LLEVEL OR
%token FILTERS ACTION NAME PHNUM FILTER FLAGS
 /* HANGUP IGNOREIT PLAY RECORD */
%token <number> LNUML ACTN FLAG
%token <string> IPADDR PATH REGEX FNAME
%%
commands:
	|
	commands command SCOLON
	;
command:
	filters
	|
	notify
	|
	main
	;
main:
	MAIN params
	;
params:
	OBRACE directives CBRACE
	;
directives:
	|
	directives directive SCOLON
	;
directive:
	modemdev
	|
	loglevel
	;
notify:
	NOTIFY iplist
	;
iplist:
	OBRACE ipaddrs CBRACE
	;
ipaddrs:
	|
	ipaddrs ipadr SCOLON
	;
ipadr:
	IPADDR
	{
		lprintf(debug, "Encountered ipaddress %s\n", $1);
		addtoaddrs($1);
	}
	;
modemdev:
	MODDEV devpath
	;
devpath:
	QUOTE PATH QUOTE
	{
		lprintf(debug, "Modem dev == %s\n", $2);
		cf.modemdev = $2;
	}
	;
/* filters */
filters:
	FILTERS filterlist
	;
filterlist:
	OBRACE filtbds CBRACE
	;
filtbds:
	|
	filtbds filtbd SCOLON
	;
filtbd:
	FILTER FNAME OBRACE filtersts CBRACE
	{
		add_condition($2, namrx, numrx, factn, ctflags);
		free($2);
		if(namrx != 0x0) free(namrx);
		if(numrx != 0x0) free(numrx);
		factn = 0x0;
		ctflags = 0x0;
	}
	;
filtersts:
	|
	filtersts filterst SCOLON
	;
filterst:
	fname
	|
	fnumb
	|
	faction
	|
	fflags
	;
fname:
	NAME REGEX
	{
		namrx = $2;
	}
	;
fnumb:
	PHNUM REGEX
	{
		numrx = $2;
	}
	;
fflags:
	FLAGS flgs
	;
flgs:
	|
	flg flgs
	;
flg:
	FLAG OR
	{
		ctflags |= $1;
	}
	|
	ACTN
	{
		ctflags |= $1;
	}
	;
faction:
	ACTION facts
	;
facts:
	|
	fact facts
	;
fact:
	ACTN OR
	{
		factn |= $1;
	}
	|
	ACTN
	{
		factn |= $1;
	}
	;
/* loglevels */
loglevel:
	LLEVEL llvls
	;
llvls:
	|
	llvl llvls
	;
llvl:
	LNUML OR
	{
		cf.loglevels |= $1;
	}
	|
	LNUML
	{
		cf.loglevels |= $1;
	}
	;
%%
