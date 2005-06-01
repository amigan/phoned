%union
{
	int number;
	char* string;
}

%{
#include <stdio.h>
#include <string.h>
#include <phoned.h>
int chrcnt = 0;
int lincnt = 0;
int yylex(void);
void yyerror(str)
	char* str;
{
	lprintf(fatal, "parser: error: %s at line %d chr %d\n", str, lincnt,
		chrcnt);
	exit(-1);
}
int yywrap(void)
{
	return 1;
}
%}
%token NOTIFY OBRACE CBRACE SCOLON
%token <string> IPADDR
%%
commands:
	|
	command commands SCOLON
	;
command:
	notify
	;
notify:
	NOTIFY iplist
	{
		lprintf(info, "parser: end notify\n");
	}
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
	}
	;
%%
