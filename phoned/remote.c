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
/* $Amigan: phoned/phoned/remote.c,v 1.8 2005/06/20 01:39:50 dcp1990 Exp $ */
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

login_t *usertop = 0x0;
pthread_mutex_t usermx = PTHREAD_MUTEX_INITIALIZER;

/* taken from FreeBSD /usr/src/lib/libc/string/strsep.c */
char * mysep(stringp, delim)
	char **stringp;
	const char *delim;
{
	char *s;
	const char *spanp;
	int c, sc, inquot = 0;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		if(c == '"') {
			if(inquot) {
				s[-1] = 0;
				c = *s++;
			}
			inquot = inquot ? 0 : 1;
		}
		do {
			if ((sc = *spanp++) == c && !inquot) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

login_t *check_logged_in(loginna, top)
	char *loginna;
	login_t *top;
{
	login_t *c;
	if(top == 0x0) return 0;
	for(c = top; c->next != NULL; c = c->next) {
		if(strcmp(loginna, c->name) == 0) {
			return c;
		}
	}
	if(strcmp(loginna, c->name) == 0) {
		return c;
	}
	return NULL;
}

void log_out_user(loginna, toppt) /* needs locking */
	char *loginna;
	login_t **toppt;
{
	login_t *c, *top, *l;
	short found = 0;
	top = *toppt;
	if(loginna != NULL) {
		for(c = top; c != NULL; c = c->next) {
			if(strcmp(loginna, c->name) == 0) {
				found = 1;
				break;
			}
		}
		if(!found) return;
		top = c;
	}
	if(top->last == 0x0) {
		if(top->next != 0x0) c = top->next; else c = 0x0;
		free_login(top, 0);
		if(!c) *toppt = 0x0; else *toppt = c;
	} else {
		c = top->next;
		l = top->last;
		free_login(top, 0);
		l->next = c;
		c->last = l;
	}
}

void free_login(t, traverse)
	login_t *t;
	short traverse;
{
	login_t *c, *nex;
	if(!traverse) {
		free(t->name);
		free(t);
	} else {
		c = t;
		while(c != NULL) {
			nex = c->next;
			free_login(c, 0); /* recursive, babayyy */
			c = nex;
		}
	}
}

void flush_logins(void)
{
	pthread_mutex_lock(&usermx);
	free_login(usertop, 1);
	pthread_mutex_unlock(&usermx);
}

login_t *add_to_login_list(loginna, toppt)
	char *loginna;
	login_t **toppt;
{
	login_t *c, *n, *top;
	time_t now;
	now = time(NULL);
	top = *toppt;
	n = malloc(sizeof(login_t));
	memset(n, 0x0, sizeof(login_t));
	n->logintime = now;
	n->name = strdup(loginna);
	if(top == 0x0) {
		*toppt = n;
	} else {
		for(c = top; c->next != NULL; c = c->next);
		n->last = c;
		c->next = n;
	}
	return n;
}
		

short log_in_user(loginna, pass, lnt)
	char *loginna;
	char *pass;
	login_t **lnt;
{
	pthread_mutex_lock(&usermx);
	if((*lnt = check_logged_in(loginna, usertop))) {
		pthread_mutex_unlock(&usermx);
		return -1;
	}
	if(db_check_crend(loginna, pass)) {
		*lnt = add_to_login_list(loginna, &usertop);
		pthread_mutex_unlock(&usermx);
		return 1;
	} else {
		pthread_mutex_unlock(&usermx);
		return 0;
	}
	/* NOTREACHED */
	pthread_mutex_unlock(&usermx);
	return 0;
}

char *parse_command(cmd, cont, s)
	const char *cmd;
	short *cont;
	state_info_t *s;
{
	char **ap, *is, *argvect[MAXARGS], *ia;
	int cpos = 0;
	int rc;
	memset(argvect, 0, sizeof(argvect));
	is = strdup(cmd);
	ia = is;
	*cont = 0x1;
	for(ap = argvect; (*ap = mysep(&ia, " \t")) != NULL;) {
		if(**ap == '"') *ap = *ap + 1;
		if(**ap != '\0')
			if(++ap >= &argvect[MAXARGS])
				break;
	}
	/* begin checking */
	switch(s->st) {
		case init:
			if(CHK("login")) {
				++cpos;
				if(argvect[cpos] != NULL && argvect[cpos + 1] != NULL) {
					rc = log_in_user(argvect[cpos], argvect[cpos + 1], &s->l);
					cpos++;
					if(rc == -1) {
						RNF("501 LOGGEDIN: Already logged in.\n");
					} else if(rc) {
						RNF("501 LOGGEDIN: Logged in! Welcome!\n");
					} else if(!rc) {
						RNF("514 LOGINFAIL: Login failed.\n");
					}
				} else {
					RNF("513 ERROR: Syntax error: Needs username and pass as arguments.\n");
				}
			} else if(CHK("logout")) {
				if(s->l != NULL) {
					pthread_mutex_lock(&usermx);
					log_out_user(NULL, &s->l);
					if(s->l == 0x0) usertop = 0;
					pthread_mutex_unlock(&usermx);
					s->l = NULL;
					RNF("502 LOGGEDOUT: User logged out.\n");
				} else {
					RNF("513 ERROR: Not logged in!\n");
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
			} else if(CHK("echo")) {
				char *it;
				s->freeit = 1;
				if(argvect[1] != NULL) it = strdup(argvect[1]); else it = strdup("die fucka");
				free(is);
				return(it);
			} else {
				RNF("513 ERROR: Unrecognised Command\n");
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
	if(si.l != NULL) {
		pthread_mutex_lock(&usermx);
		log_out_user(NULL, &si.l);
		if(si.l == 0x0) usertop = 0;
		pthread_mutex_unlock(&usermx);
	}
	lprintf(info, "Connection closed in begin_dialogue\n");
}
