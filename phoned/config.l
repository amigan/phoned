/* configuration lexer */
%{
#include <stdio.h>
#include <string.h>
#ifdef BISON
#include "config.tab.h"
#else
#include "y.tab.h"
#endif
#include <phoned.h>
extern int	chrcnt, lincnt;
%}
%%
.			++chrcnt; REJECT;
main			return MAIN;
notify			return NOTIFY;
modemdev		return MODDEV;
\"			return QUOTE;
\{			return OBRACE;
\}			return CBRACE;
\/[a-zA-Z0-9/._-]+	yylval.string = strdup(yytext); return PATH;
[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}	yylval.string = strdup(yytext); return IPADDR;
;			return SCOLON;
\n			chrcnt = 0; ++lincnt;/* DONOTHING */
[\t ]			/* DONOTHING */
.			/* DO nothing */
%%
int parse(fp)
	FILE** fp;
{
	extern int yyparse __P((void));
	yyin = *fp;
	if(yyparse())
		return -1;
	return 0;
}
