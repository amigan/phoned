/*
 * Copyright (c) 2005, Dan Ponte
 *
 * modem.c - modem code for supra (Rockwell), might work on others...
 * this will be pluggable in the future
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* system includes */
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <phoned.h>
#define INITSTRING "ATZ\r\nAT E0 #CID=2 V0\r\n"
/* globals */
FILE* modem;
int modemfd;
unsigned int cou = 0;
char buffer[512];
short doing_cid = 0;
void stmod(const char* str)
{
	fputs(str, modem);
	fflush(modem);
}
int close_modem(char* dev)
{
	if(strlen(dev) < 7) {
		lprintf(error , "dev %s too short\n", dev);
		return -4;
	}
	if(strncmp(dev, "/dev/", strlen("/dev/")) != 0) {
		lprintf(error, "dev %s must begin with /dev/\n", dev);
		return -5;
	}
	uu_unlock((dev+(sizeof("/dev/")-1))); 
	fclose(modem);
	return 1;
}
int init_modem(char* dev)
{
	int lres = 0;
	if(strlen(dev) < 7) {
		lprintf(error , "dev %s too short\n", dev);
		return -4;
	}
	if(strncmp(dev, "/dev/", strlen("/dev/")) != 0) {
		lprintf(error, "dev %s must begin with /dev/\n", dev);
		return -5;
	}
	lres = uu_lock((dev+(sizeof("/dev/")-1))); 
	if(lres != UU_LOCK_OK) {
		lprintf(error, "%s\n", uu_lockerr(lres));
		return -1;
	}
	modemfd = open(dev, O_RDWR);
	if(modemfd == -1) {
		lprintf(error, "Error opening modem %s: %s\n", dev, strerror(errno));
		return -2;
	}
	modem = fdopen(modemfd, "w+");
	if(!modem) {
		lprintf(error, "Error fdopening modemfd %d: %s\n", modemfd, strerror(errno));
		return -3;
	}
	stmod(INITSTRING);
	return 1;
}
int modem_evalrc(char* result)
{
	int rescode;
	unsigned int i;
	for(i = 0; i <= strlen(result); i++) {
		if(result[i] == '\r' || result[i] == '\n') result[i] = '\0';
	}
	rescode = atoi(result);
	switch(rescode) {
		case 0:
			/* OK */
			return 0;
			break;
		case 2:
			break;
		case 4:
			return -1;
			break;
		default:
			return 0;
			break;
	}
	return 0;
}
void modem_hread(char* cbuf)
{
	if(cou < sizeof(buffer) - 2)
		buffer[cou] = cbuf[0];
	if(buffer[0] == '8' && buffer[1] == '0')
		doing_cid = 1;
	if(cbuf[0] == '\n') {
		if(doing_cid) {
			cid_t *rc;
			rc = parse_cid(buffer);
			cid_log(rc);
			cid_notify(rc);
			memset(buffer, 0, sizeof(buffer));
			doing_cid = 0;
			cou = 0;
		} else {
			modem_evalrc(buffer);
			memset(buffer, 0, sizeof(buffer));
			cbuf[0] = 0;
			cou = 0;
		}
	}	else {
		cou++;
	}
}
