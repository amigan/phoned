/*
 * Copyright (c) 2005, Dan Ponte
 *
 * modem.c - modem code
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
void stmod(const char* str)
{
	fputs(str, modem);
	fflush(modem);
}
int init_modem(char* dev)
{
	int lres = 0;
	modemfd = open(dev, O_RDWR);
	if(!modemfd) {
		lprintf(error, "Error opening modem %s: %s\n", dev, strerror(errno));
		return -2;
	}
	lres = uu_lock((dev+(sizeof("/dev/")-1))); 
	if(lres != 0) {
		lprintf(error, "%s\n", uu_lockerr(lres));
		return -1;
	}
	modem = fdopen(modemfd, "w+");
	if(!modem) {
		lprintf(error, "Error fdopening modemfd %d: %s\n", modemfd, strerror(errno));
		return -3;
	}
	stmod(INITSTRING);
	return 1;
}
