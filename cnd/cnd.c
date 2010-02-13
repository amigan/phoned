/*
 * Caller ID Server mgetty support
 * (C)2004, 2010, Dan Ponte
 * Licensed under the BSD license, with advertising clause.
 * Excerpted from the original cidserv distribution. Modified to work
 * with phoned.
 */
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libutil.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <phoned.h>

#define VERSION "0.1"
#define ADDRS "/usr/local/etc/cidserv.conf"
int nhosts = 0;
char hosts[10][18];
static const char rcsid[] = "$Amigan: phoned/cnd/cnd.c,v 1.4 2010/02/13 18:18:32 dcp1990 Exp $";
void send_dgram(char* address, char* datar);
void load_addrs(const char* fl);
void calllog(char *msg);

int parse_netcid(char* tty, char* phone, char* name, int dist_r, char* called, char* call_time);
int main(int argc, char* argv[])
{
	char fdate[25];
	time_t rnow;
	struct tm *ctm;
	char *envtime;
	if(argc < 6) {
		fprintf(stderr, "Needs 5 args:\n"
		"<tty> <CallerID> <Name> <dist-ring-nr.> <Called Nr.>\n");
		exit(-2);
	}
	load_addrs(ADDRS);
	envtime = getenv("CALL_TIME");
	rnow = time(NULL);
	ctm = localtime(&rnow);
	memset(fdate, 0, sizeof fdate);
	strftime(fdate, sizeof(fdate) * sizeof(char),
		(envtime ? "%m%d:" : "%m%d:%H%M"), ctm);
	if(envtime)
		strlcat(fdate, envtime, sizeof(fdate));

	parse_netcid(argv[1], argv[2], argv[3], atoi(argv[4]), argv[5], fdate);
	return 1;
}

int parse_netcid(tty, phone, name, dist_r, called, call_time)
	char* tty __unused;
	char* phone;
	char* name;
	int dist_r __unused;
	char* called __unused;
	char* call_time;
{
	int i = 0;
	char msg[512];
	memset(msg, 0, sizeof msg);
	sprintf(msg, "%s:0:%s:%s", call_time, name, phone);
	calllog(msg);
	for(i = 0; i <= nhosts; i++) {
		send_dgram(hosts[i], msg);
	}
	return 0;
}

void calllog(msg)
	char *msg;
{
	FILE *fh;
	const time_t now = time(NULL);
	char tmt[128];
	if(!(fh = fopen("/var/log/cid.log", "a"))) {
		perror("fopen");
		return; /* do not exit, so the dgrams at least get out */
	}
        strftime(tmt, sizeof(tmt), "%b %d %H:%M:%S", localtime(&now));
	fprintf(fh, "%s: %s\n", tmt, msg);
	fclose(fh);
}
	
void send_dgram(char* address, char* datar)
{
	char msg[212];
	int s;
	int on = 1;
	struct sockaddr_in sin;
	if(strlen(address) < 3) return;
#ifdef DEBUG
	printf("send_dgram(%s) %p\n", address, address);
#endif
	strcpy(msg, datar);
	s = socket(PF_INET, SOCK_DGRAM, 0);
	bzero(&sin, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(address);
	sin.sin_port = htons(3890);
	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*) &on, sizeof(on)) < 0) {
		perror("setsockopt");
		exit(-1);
	}
	if (connect(s, (struct sockaddr *)&sin, sizeof sin) < 0) {
		perror("connect");
		close(s);
		return;
	}
	write(s, msg, strlen(msg) + 1);
	close(s);
	return;
}
void load_addrs(const char* fl)
{
	FILE* tfl;
	char fbuf[128];
	if(!(tfl = fopen(fl, "r"))) {
		perror("fopen");
		exit(-1);
	}
	while(!feof(tfl)) {
		fgets(fbuf, 126, tfl);
		if(fbuf[strlen(fbuf)] == '\n') fbuf[strlen(fbuf)] = 0;
		if(strlen(fbuf) > 4 && fbuf[0] != '#') strcpy(hosts[nhosts++], fbuf);
		memset(fbuf, 0, sizeof fbuf);
	}
	fclose(tfl);
}	
