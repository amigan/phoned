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
%token NOTIFY OBRACE CBRACE SCOLON QUOTE MODDEV MAIN
%token <string> IPADDR PATH
%%
commands:
	|
	command SCOLON commands
	;
command:
	notify
	|
	main
	;
main:
	MAIN params
	{
		lprintf(debug, "parser: end main\n");
	}
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
	;
notify:
	NOTIFY iplist
	{
		lprintf(debug, "parser: end notify\n");
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
modemdev:
	MODDEV devpath
	{
		lprintf(debug, "parser: end modemdev\n");
	}
	;
devpath:
	QUOTE PATH QUOTE
	{
		lprintf(debug, "Modem dev == %s\n", $2);
		cf.modemdev = $2;
	}
	;
%%
