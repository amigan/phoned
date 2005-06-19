/*
 * Copyright (c) 2005, Dan Ponte
 *
 * remote.c - parser for use with clients.
 * MAKE IT THREAD SAFE, FOR PETE'S SAKE!
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
/* $Amigan: phoned/phoned/remote.c,v 1.6 2005/06/19 01:24:16 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
/* us */
#include <phoned.h>
#define MAXBUFSIZE 512
#define MAXARGS 15
#define CHK(m)	strcasecmp(m, argvect[cpos]) == 0
#define RNF(m)	s->freeit = 0; free(is); return(m)

char *parse_command(cmd, cont, s)
	const char *cmd;
	short *cont;
	state_info_t *s;
{
	char **ap, *is, *argvect[MAXARGS];
	int cpos = 0;
	memset(argvect, 0, sizeof(argvect));
	is = strdup(cmd);
	*cont = 0x1;
	for(ap = argvect; (*ap = strsep(&is, " \t")) != NULL;)
		if(**ap != '\0')
			if(++ap >= &argvect[MAXARGS])
				break;
	/* begin checking */
	switch(s->st) {
		case init:
			if(CHK("login")) {
				++cpos;
				if(argvect[cpos] != NULL) {
					/* TODO: put login stuff here */
					RNF("100 PASSWORD: Please give me a password.\n");
				} else {
					RNF("513 ERROR: Syntax error: Needs usernamd as argument.\n");
				}
			} else if(CHK("thandler")) {
				cid_t c;
				c.number = "7823020300";
				c.name = "BUSH,GEORGE W.";
				c.day = 2;
				c.hour = 3;
				c.month = 4;
				c.minute = 20;
				cid_handle(&c);
				*cont = 0;
				RNF("500 OK: Handler tested.\n");
			} else if(CHK("tparse")) {
				cid_t* rc;
				rc = parse_cid("802701083132323130383234070F5354414E444953482048454154494E020A343031333937333337325C\n");
				lprintf(info, "nam=%s;month=%d\n", rc->name, rc->month);
				cid_notify(rc);
				cid_log(rc);
				*cont = 0;
				RNF("500 OK: Parser tested.\n");
			} else if(CHK("gmm")) {
				stmod(argvect[1] != NULL ? argvect[1] : "AT\r\n");
				*cont = 0;
				RNF("500 OK: Give Me Modem tested.\n");
			}
			break;
		case loginstage:
			break;
		case pass:
			break;
	}
	return NULL;
}
void begin_dialogue(fp, fd)
	FILE* fp;
	int fd;
{
	char buffer[MAXBUFSIZE]; /* molto importante */
	char *rcode, *c;
	short keep_going = 0x1;
	int rc = 0;
	state_info_t si;
	memset(&si, 0, sizeof(state_info_t));
	si.fpo = fp; /* this is JUST for data, not real commands */
	si.fd = fd;
	while(keep_going == 0x1 && !feof(fp)) {
		rc = recv(fd, buffer, MAXBUFSIZE - 1, 0x0);
		if(rc == 0) {
			lprintf(debug, "Socket closed! Got zero!\n");
			break;
		} else if(rc == -1) {
			lprintf(error, "Error with recv: %s\n", strerror(errno));
			break;
		}
		if((c = strrchr(buffer, '\n')) != NULL)
			*c = '\0';
		rcode = parse_command(buffer, &keep_going, &si);
		if(rcode != NULL) {
			if(send(fd, rcode, strlen(rcode) + 1, 0) == 0x0) {
				lprintf(error, "Error: (send() inside begin_dialogue()): %s\n", strerror(errno));
				break;
			}
			if(si.freeit) free(rcode);
		}
	}
	lprintf(info, "Connection closed in begin_dialogue\n");
}
