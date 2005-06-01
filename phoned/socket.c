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

#include <phoned.h>

void handclient(sk)
	int	sk;
{
	char	buffer[1024];
	int	rlen;
	rlen = recv(sk, buffer, sizeof(buffer), 0);
	lprintf(debug, "Client said %s.", buffer);
}

void network(void)
{
	int 		s, us, sn;
	fd_set		fds;
	struct timeval	tv;
	int		si = 1;
	int		sin_size, ilen;
	struct sockaddr_un it;
	if((s = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(-1);
	}
	strcpy(it.sun_path, SOCKETFILE);
	it.sun_family = AF_LOCAL;
	if(bind(s, (struct sockaddr *)&it, 1 + strlen(it.sun_path) +
					sizeof(it.sun_family)) == -1) {
		perror("bind");
		exit(-1);
	}
	if(listen(s, 5) == -1) {
		perror("listen");
		exit(-1);
	}
	sin_size = sizeof(struct sockaddr_in);
	for(;;) {
		FD_ZERO(&fds);
		FD_SET(s, &fds);
		switch(select(s + 1, &fds, NULL, NULL, NULL)) {
		case -1:
			perror("select");
			exit(-1);
			break;
		case 0:
			/* NOTREACHED */
			break;

		default:
			{
				if(FD_ISSET(s, &fds) != 0) {
					ilen = sizeof(it);
					if((sn = accept(s,
						(struct sockaddr *)&it, &ilen))
							== -1) {
						perror("s accept");
						exit(-3);
					}
					handclient(sn);
					close(sn);
				}
			}
		}
	}
	close(s);
	unlink(SOCKETFILE);
}
