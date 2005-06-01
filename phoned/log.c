/*
 * log.c - logfile stuff
 * (C)2005, Dan Ponte.
 * BSDL with advert.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <phoned.h>
#include <time.h>

FILE*	logf;
extern struct conf cf;

int check_loglevel(lt, ll)
	enum ltype lt;
	int ll;
{
	switch(lt) {
		case debug:
			if(!(ll & LL_DEBUG))
				return -1;
		case garbage:
			if(!(ll & LL_GARBAGE))
				return -1;
		case info:
			if(!(ll & LL_INFO))
				return -1;
		case warn:
			if(!(ll & LL_WARN))
				return -1;
		case error:
			if(!(ll & LL_ERROR))
				return -1;
		case critical:
			if(!(ll & LL_CRITICAL))
				return -1;
			/* we cannot ignore FATAL. Because it's fatal. */
		default:
			return 1;
	}
	return 1;
}


int lprintf(enum ltype logtype, const char* fmt, ...)
{
	va_list ap;
	const char* fmtp;
	char* ofmt;
	int cnt = 0;
	int j, i, l;
	unsigned uns;
	double dou;
	char cha;
	char* str;
	void* voi;
	int maxsize;
	time_t now;
	char tmt[128];
	now = time(NULL);
	l = cf.loglevels;
	if(!check_loglevel(logtype, l))
		return -1;
	strftime(tmt, 128, "%d%b %H:%M:%S: ", localtime(&now));
	fmtp = fmt;
	maxsize = strlen(fmt) + 1;
	ofmt = malloc(sizeof(char) * (strlen(fmt) + 2));
	fputs(tmt, logf);
	va_start(ap, fmt);
	while(*fmtp) {
		for(i = 0; fmtp[i] && fmtp[i] != '%' && i < maxsize; i++)
			ofmt[i] = fmtp[i];
		if(i) {
			ofmt[i] = '\0';
			cnt += fprintf(logf, ofmt);
			fmtp += i;
		} else {
			for(i = 0; !isalpha(fmtp[i]) && i < maxsize; i++) {
				ofmt[i] = fmtp[i];
				if(i && fmtp[i] == '%') break;
			}
			ofmt[i] = fmtp[i];
			ofmt[i + 1] = '\0';
			fmtp += i + 1;
			switch(ofmt[i]) {
				case 'd':
					j = va_arg(ap, int);
					cnt += fprintf(logf, ofmt, j);
					break;
				case 'o':
				case 'x':
				case 'X':
				case 'u':
					uns = va_arg(ap, unsigned);
					cnt += fprintf(logf, ofmt, uns);
					break;
				case 'c':
					cha = (char) va_arg(ap, int);
					cnt += fprintf(logf, ofmt, cha);
					break;
				case 's':
					str = va_arg(ap, char*);
					cnt += fprintf(logf, ofmt, str);
					break;
				case 'f':
				case 'e':
				case 'E':
				case 'G':
				case 'g':
					dou = va_arg(ap, double);
					cnt += fprintf(logf, ofmt, dou);
					break;
				case 'p':
					voi = va_arg(ap, void*);
					cnt += fprintf(logf, ofmt, voi);
					break;
				case 'n':
					cnt += fprintf(logf, "%d", cnt);
					break;
				case '%':
					fputc('%', logf);
					cnt++;
					break;
				default:
					fprintf(stderr, "Invalid format in log!\n");
					break;
			}
		}
	}
	fflush(logf);
	va_end(ap);
	free(ofmt); /* MUST do this */
	return cnt;
}
