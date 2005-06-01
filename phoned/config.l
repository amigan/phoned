/* configuration lexer */
%{
#include <stdio.h>
#include "y.tab.h"
#include <phoned.h>
extern int	chrcnt, lincnt;
%}
%%
.			++chrcnt; REJECT;
notify			return NOTIFY;
\{			return OBRACE;
\}			return CBRACE;
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
