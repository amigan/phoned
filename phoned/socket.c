/*
 * Copyright (c) 2005, Dan Ponte
 *
 * socket.c - sockets.
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
/* network stuff */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <pthread.h>

#include <phoned.h>

extern pthread_mutex_t modemmx;
extern pthread_mutex_t buffermx;
pthread_t networkth;
int selpipes[2];
pthread_mutex_t spipsmx;
void *handclient(k)
	void*	k;
{
	int sk = (int)k;
	FILE* tf;
	tf = fdopen(sk, "r+");
	begin_dialogue(tf, sk);
	fclose(tf);
	pthread_exit(NULL);
	return 0;
}
#if 0
void clsck(sck)
	void *sck;
{
	int s = (int)sck;
	close(s);
}
#endif
void awaken_sel(void)
{
	pthread_mutex_lock(&spipsmx);
	write(selpipes[1], "W", 1);
	pthread_mutex_unlock(&spipsmx);
}
void *network(b)
	void* b;
{
	int 		s, /* us,*/ sn;
	fd_set		fds;
	int		ilen;
	pthread_t	thr;
	char		cbuf[2];
	struct sockaddr_un it;
	fillset();
	cbuf[0] = '\0'; cbuf[1] = '\0';
	if(b == 0) b = 0;
	if((s = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
		lprintf(error, "socket: %s\n", strerror(errno));
		shutd(0x1|0x2|0x4|0x10|0x20);
		exit(-1);
	}
	pthread_mutex_lock(&spipsmx);
	pipe(selpipes);
	pthread_mutex_unlock(&spipsmx);
	strcpy(it.sun_path, SOCKETFILE);
	it.sun_family = AF_LOCAL;
	if(bind(s, (struct sockaddr *)&it, 1 + strlen(it.sun_path) +
					sizeof(it.sun_family)) == -1) {
		lprintf(error, "bind: %s\n", strerror(errno));
		shutd(0x1|0x2|0x4|0x10|0x20);
		exit(-1);
	}
	if(listen(s, 5) == -1) {
		lprintf(error, "listen: %s\n", strerror(errno));
		shutd(0x1|0x2|0x4|0x10|0x20);
		exit(-1);
	}
	for(;;) {
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		FD_SET(selpipes[0], &fds);
		switch(select(selpipes[0] + 1, &fds, NULL, NULL, NULL)) { /* this had better be a cancellation point... */
		case -1:
			lprintf(error, "network select: %s\n", strerror(errno));
			pthread_exit(NULL);
			break;
		case 0:
			/* NOTREACHED */
			break;

		default:
			{
				if(FD_ISSET(s, &fds) != 0) {
					ilen = sizeof(it);
					if((sn = accept(s, (struct sockaddr *)&it, &ilen))
							== -1) {
						lprintf(error, "accept: %s\n",
								strerror(errno));
						shutd(0x1|0x2|0x4|0x10|0x20);
						exit(-3);
					}
					pthread_create(&thr, NULL, handclient, (void*)sn);
					lprintf(info, "Incoming connection\n");
				}
				if(FD_ISSET(selpipes[0], &fds) != 0) {
					read(selpipes[0], cbuf, 1);
					pthread_exit(NULL);
				}
			}
		}
		if(*cbuf != 0) break;
	}
	close(s);
	unlink(SOCKETFILE);
	pthread_exit(NULL);
}
