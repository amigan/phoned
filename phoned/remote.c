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
/* $Amigan: phoned/phoned/remote.c,v 1.20 2008/08/07 19:17:23 dcp1990 Exp $ */
/* system includes */
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
/* us */
#include <phoned.h>
#define MAXBUFSIZE 512
#define MAXARGS 15
#define CHK(m)	strcasecmp(m, argvect[cpos]) == 0
#define RNF(m)	s->freeit = 0; free(is); return(m)
#define LOGGEDIN	s->l != NULL

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

int dialogue_cb(fd, sck)
	int fd;
	void *sck;
{
	FILE *so = (FILE*)sck;
	int s;
	fd_set fds;
	fd_set ex;
	char buffer[3];
	char lastc = 0x0;
	int rc;
	s = fileno(so);
	memset(buffer, 0, sizeof(buffer));
	lprintf(debug, "dialogue_cb: Started");
	for(;;) {
		FD_ZERO(&fds);
		FD_ZERO(&ex);
		FD_SET(s, &ex);
		FD_SET(fd, &fds);
		FD_SET(s, &fds);
		switch(select(s + 1, &fds, NULL, &ex, NULL)) {
			case -1:
				lprintf(error, "select on modem: %s", strerror(errno));
				return 0;
			case 0:
				/* WON'T HAPPEN */
				break;
			default:
				{
					if(FD_ISSET(s, &ex) != 0) {
						lprintf(debug, "Exceptional.");
						return 0;
					}
					if(FD_ISSET(fd, &fds) != 0) {
						rc = read(fd, buffer, 1);
						if(rc == -1) {
							lprintf(error, "read(): %s", strerror(errno));
							return 0;
						}
						rc = send(s, buffer, 1, 0x0);
						if(rc == -1) {
							lprintf(error, "send(): %s", strerror(errno));
							return 0;
						}
					}
					if(FD_ISSET(s, &fds) != 0) {
						rc = recv(s, buffer, 1, 0x0);
						if(rc == 0) {
							lprintf(debug, "cb: Socket closed! Got zero!\n");
							return 0;
						} else if(rc == -1) {
							lprintf(debug, "recv(): %s", strerror(errno));
							return 0;
						}
						if(lastc == '%' && *buffer == 'q') return 1;
						lastc = *buffer;
						rc = write(fd, buffer, 1);
						if(rc == -1) {
							lprintf(error, "write(): %s", strerror(errno));
							return 0;
						}
					}
				}
		}
	}
	/* NOTREACHED */
	return 1;
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
						s->st = loggedin;
						RNF("501 LOGGEDIN: Already logged in.\n");
					} else if(rc) {
						s->st = loggedin;
						RNF("501 LOGGEDIN: Logged in! Welcome!\n");
					} else if(!rc) {
						RNF("514 LOGINFAIL: Login failed.\n");
					}
				} else {
					RNF("513 ERROR: Syntax error: Needs username and pass as arguments.\n");
				}
			} else {
				RNF("513 ERROR: No such command (perhaps not logged in?)\n");
			}
			break;
		case loggedin:
			if(CHK("logout")) {
				pthread_mutex_lock(&usermx);
				log_out_user(NULL, &s->l);
				if(s->l == 0x0) usertop = 0;
				pthread_mutex_unlock(&usermx);
				s->l = NULL;
				s->st = init;
				RNF("502 LOGGEDOUT: User logged out.\n");
			} else if(CHK("stmodem")) {
				if(argvect[++cpos] != NULL) {
					char *tempbuf;
					tempbuf = malloc(512 * sizeof(char));
					memset(tempbuf, 0, 512 * sizeof(char));
					sendwr(argvect[cpos], tempbuf, 512);
					s->freeit = 1;
					return tempbuf;
				} else {
					RNF("513 ERROR: Wrong number of arguments.\n");
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
			} else if(CHK("mdlg")) {
				int rc;
				write(s->fd, "500 OK: Entering dialogue...warning: this may crash!\n", sizeof("500 OK: Entering dialogue...warning: this may crash!\n"));
				rc = dialogue_with_modem(&dialogue_cb, (void*)s->fpo);
				if(!rc) {
					return NULL;
				} else {
					RNF("500 OK: Dialogue finished");
				}
			} else if(CHK("tparse")) {
				cid_t* rc;
				rc = parse_cid("802701083132323130383234070F5354414E444953482048454154494E020A343031333937333337325C\n");
				cid_log(rc);
				cid_notify(rc);
				/* XXX: memory leak!! free the members of the cid_t! make sure other code
				 * which uses dynamically-allocated cid_t's does so as well.
				 */
				free(rc);
				*cont = 0;
				RNF("500 OK: Parser tested.\n");
			} else if(CHK("bye")) {
				return NULL;
			} else if(CHK("dumpcalls")) {
				db_dump_calls(s->fd, "700 CALLREC: %s\n");
				RNF("516 DONE: Calls Done.\n");
			} else if(CHK("tmop")) {
				if(argvect[1] != NULL) {
					cid_t *rc;
					rc = parse_cid(argvect[1]);
					if(rc != NULL) {
						cid_log(rc);
						cid_notify(rc);
						free(rc);
					}
					RNF("500 OK: Tested.\n");
				} else {
					RNF("513 ERROR: Needs argument\n");
				}
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
	memset(buffer, 0, sizeof(buffer));
	while(keep_going == 0x1 && !feof(fp)) {
		memset(buffer, 0, sizeof(buffer));
		rc = recv(fd, buffer, MAXBUFSIZE - 1, 0x0);
		if(rc == 0) {
			lprintf(debug, "Socket closed! Got zero!\n");
			break;
		} else if(rc == -1) {
			lprintf(error, "Error with recv: %s\n", strerror(errno));
			break;
		} else if(rc < 3) continue;
		if((c = strrchr(buffer, '\n')) != NULL)
			*c = '\0';
		rcode = parse_command(buffer, &keep_going, &si);
		if(rcode != NULL) {
			if(send(fd, rcode, strlen(rcode) + 1, 0) == 0x0) {
				lprintf(error, "Error: (send() inside begin_dialogue()): %s\n", strerror(errno));
				break;
			}
			if(si.freeit) free(rcode);
		} else break;
	}
	if(si.l != NULL) {
		pthread_mutex_lock(&usermx);
		log_out_user(NULL, &si.l);
		if(si.l == 0x0) usertop = 0;
		pthread_mutex_unlock(&usermx);
	}
}
