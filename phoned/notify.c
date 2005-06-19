/*
 * Copyright (c) 2005, Dan Ponte
 *
 * notify.c - notification of clients (CID)
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
/* $Amigan: phoned/phoned/notify.c,v 1.7 2005/06/19 04:46:15 dcp1990 Exp $ */
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
#include <errno.h>
#include <pthread.h>
#define HAVE_INET_INCS
#include <phoned.h>
addrsll_t head;
addrsll_t *top = &head;
pthread_mutex_t addrmx = PTHREAD_MUTEX_INITIALIZER;

addrsll_t* getlast(addrsll_t* hd)
{
	addrsll_t *cur;
	for(cur = hd; cur->next != 0x0; cur = cur->next) ;
	return cur;
}
void freeaddrl(addrsll_t* hd)
{
	addrsll_t *cur, *nx;
	nx = cur = hd;
	for(cur = hd; nx != 0x0; cur = nx) {
		nx = cur->next;
		if(cur != &head) free(cur);
	}
}
addrsll_t* allocaddr(void)
{
	addrsll_t* nw;
	nw = malloc(sizeof(addrsll_t));
	memset(nw, 0x0, sizeof(addrsll_t));
	return nw;
}
void flush_lists(void)
{
	pthread_mutex_lock(&addrmx);
	freeaddrl(top);
	top = 0;
	pthread_mutex_unlock(&addrmx);
}
int cid_notify(cid_t* c)
{
	short len = 0;
	char* msg;
	int s;
	struct sockaddr_in sin;
	int on = 0x1;
	addrsll_t *cur;
	len = strlen(c->number) + strlen(c->name) + 8 + 5 + 4;
	msg = malloc(len * sizeof(char));
	snprintf(msg, len, "%02d%02d:%02d%02d:0:%s:%s", c->month, c->day, c->hour, c->minute,
			c->name, c->number);
	s = socket(PF_INET, SOCK_DGRAM, 0);
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(3890);
	if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof(on)) < 0) {
		lprintf(error, "setsockopt: %s...sure you're root?\n", strerror(errno));
		return -1;
	}
	pthread_mutex_lock(&addrmx);
	for(cur = top; cur->next != 0x0; cur = cur->next) {
		sin.sin_addr.s_addr = cur->addr;
		if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			lprintf(error, "connect: %s\n", strerror(errno));
			return -1;
		}
		write(s, msg, strlen(msg) + 1);
	}
	pthread_mutex_unlock(&addrmx);
	close(s);
	return 1;
}
