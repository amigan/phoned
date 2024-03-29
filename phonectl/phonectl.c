/*
 * Copyright (c) 2005, Dan Ponte
 *
 * phonectl.c - command line client
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
/* $Amigan: phoned/phonectl/phonectl.c,v 1.9 2005/06/29 22:02:23 dcp1990 Exp $ */
/* system includes */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
/* us */
#define DEFSOCK "/tmp/phoned.sock"
int main(argc, argv)
	int argc;
	char* argv[];
{
	int s;
	char *nl;
	char buff[1024];
	struct sockaddr_un it;
	fd_set fds;
	s = socket(AF_LOCAL, SOCK_STREAM, 0);
	strcpy(it.sun_path, DEFSOCK);
	it.sun_family = AF_LOCAL;
	if(connect(s, (struct sockaddr *)&it, 1 + strlen(it.sun_path) + sizeof(it.sun_family))
			== -1) {
		perror("conn");
		exit(-1);
	}
#if 0
	if(setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on)) < 0) {
		perror("setsockopt");
		exit(-1);
	}
#endif
	fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
	if(argc == 1) {
		for(;;) {
			FD_ZERO(&fds);
			FD_SET(fileno(stdin), &fds);
			FD_SET(s, &fds);
			switch(select(s + 1, &fds, NULL, NULL, NULL)) {
				case -1:
					perror("select");
					exit(-1);
					break;
				default:
					{
						if(FD_ISSET(fileno(stdin), &fds)) {
							read(fileno(stdin), buff, 1024);
							nl = strchr(buff, '\n');
							if(nl != NULL) *nl = '\0';
							if(strcmp(buff, "#quit#") == 0) {
								close(s);
								return 0;
							}
							send(s, buff, strlen(buff) + 1, 0x0);
						}
						if(FD_ISSET(s, &fds)) {
							if(recv(s, buff, sizeof(buff), 0x0) == 0) {
								close(s);
								return 0;
							}
							fputs(buff, stdout);
						}
					}
			}
		}
	}
	write(s, argv[1], strlen(argv[1]) + 1);
	read(s, buff, sizeof buff);
	fputs(buff, stdout);
	close(s);
	return 0;
}
