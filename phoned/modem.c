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
#include <pthread.h>
#define MODEM_C
#include <phoned.h>
#define ROCKWELL_INITSTRING	"ATZ\r\nAT E0 #CID=2 V0\r\n"
#define ROCKWELL_PICKUP		"ATH1\r\n"
#define ROCKWELL_HANGUP		"ATH\r\n"
/* globals */
FILE* modem;
int modemfd;
unsigned int cou = 0;
char buffer[512];
short doing_cid = 0;
extern struct conf cf;
pthread_t modemth;
pthread_mutex_t modemmx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t miomx = PTHREAD_MUTEX_INITIALIZER; /* is modem_io() running? */
pthread_mutex_t mpipemx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffermx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mpcond = PTHREAD_COND_INITIALIZER;
int modempipes[2];
extern pthread_mutex_t cfmx;
modem_t* mo;
extern modem_t rockwell;
void stmod(str)
	const char* str;
{
	if(pthread_mutex_trylock(&modemmx) != 0 && pthread_mutex_trylock(&miomx) != 0) {
		pthread_mutex_lock(&mpipemx);
		write(modempipes[1], "G", 1);
		pthread_mutex_unlock(&mpipemx);
		pthread_mutex_lock(&modemmx);
		write(modemfd, str, strlen(str) + 1);
		pthread_cond_signal(&mpcond);
		pthread_mutex_unlock(&modemmx);
	} else {
		pthread_mutex_lock(&modemmx);
		write(modemfd, str, strlen(str) + 1);
		pthread_mutex_unlock(&modemmx);
	}
}
void modem_wake(void)
{
	if(pthread_mutex_trylock(&miomx) != 0) {
		pthread_mutex_lock(&mpipemx);
		write(modempipes[1], "D", 1);
		pthread_mutex_unlock(&mpipemx);
	} else {
		pthread_mutex_lock(&cfmx);
		close_modem(cf.modemdev);
		pthread_mutex_unlock(&cfmx);
	}
}
void give_me_modem(str) /* warning: deprecated! */
	char *str;
{
	stmod(str);
}
int close_modem(char* dev)
{
	pthread_mutex_lock(&modemmx);
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
	pthread_mutex_unlock(&modemmx);
	return 1;
}
int init_modem(char* dev)
{
	int lres = 0;
	pthread_mutex_lock(&modemmx);
	if(strlen(dev) < 7) {
		lprintf(error , "dev %s too short\n", dev);
		pthread_mutex_unlock(&modemmx);
		return -4;
	}
	if(strncmp(dev, "/dev/", strlen("/dev/")) != 0) {
		lprintf(error, "dev %s must begin with /dev/\n", dev);
		pthread_mutex_unlock(&modemmx);
		return -5;
	}
	lres = uu_lock((dev+(sizeof("/dev/")-1))); 
	if(lres != UU_LOCK_OK) {
		lprintf(error, "%s\n", uu_lockerr(lres));
		pthread_mutex_unlock(&modemmx);
		return -1;
	}
	modemfd = open(dev, O_RDWR);
	if(modemfd == -1) {
		lprintf(error, "Error opening modem %s: %s\n", dev, strerror(errno));
		pthread_mutex_unlock(&modemmx);
		return -2;
	}
	modem = fdopen(modemfd, "w+");
	if(!modem) {
		lprintf(error, "Error fdopening modemfd %d: %s\n", modemfd, strerror(errno));
		pthread_mutex_unlock(&modemmx);
		return -3;
	}
	pthread_mutex_unlock(&modemmx);
	mo = &rockwell;
	mo->init();
	pthread_mutex_lock(&mpipemx);
	pipe(modempipes);
	pthread_mutex_unlock(&mpipemx);
	return 1;
}

void modem_hread(char* cbuf)
{
	pthread_mutex_lock(&buffermx);
	if(cou < sizeof(buffer) - 2)
		buffer[cou] = cbuf[0];
	if(buffer[0] == '8' && buffer[1] == '0')
		doing_cid = 1;
	if(cbuf[0] == '\n') {
		if(doing_cid) {
			cid_t *rc;
			rc = parse_cid(buffer);
			cid_handle(rc);
			free_cid(rc);
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
	pthread_mutex_unlock(&buffermx);
}
/*
 * XXX: this is inefficient
 * Thanks jilles for the tip on the pipe method used below :)
 * Do all this after a trylock().
 */
void *modem_io(k)
	void*	k;
{
	fd_set	fds;
	/* struct timeval tv; 
	short dotm = 0; */
	char	cbuf[2];
	if(k == 0) k = 0;
	*cbuf = '\0'; cbuf[1] = '\0';
	fillset();
	pthread_mutex_lock(&modemmx);
	pthread_mutex_lock(&miomx);
	for(;;) {
		pthread_mutex_lock(&cfmx);
	/*	dotm = cf.modem_tm; */
		pthread_mutex_unlock(&cfmx);
		FD_ZERO(&fds);
		FD_SET(modemfd, &fds);
		FD_SET(modempipes[0], &fds);
	/*	tv.tv_sec = 2;  tunable */
	/*	tv.tv_usec = 0; */
		switch(select(modempipes[0] + 1, &fds, NULL, NULL, /* dotm ? &tv :*/ NULL)) {
			case -1:
				lprintf(error, "select on modem: %s", strerror(errno));
				pthread_mutex_unlock(&modemmx);
				pthread_exit(NULL);
				break;
			case 0:
				pthread_mutex_unlock(&modemmx);
				sleep(1);
				pthread_mutex_lock(&modemmx);
				break;
			default:
				{
					if(FD_ISSET(modemfd, &fds) != 0) {
						read(modemfd, cbuf, 1);
						pthread_mutex_unlock(&modemmx); /* so we can use the same functions for sep. threads */
						modem_hread(cbuf);
						pthread_mutex_lock(&modemmx);
						*cbuf = '\0';
					}
					if(FD_ISSET(modempipes[0], &fds) != 0) {
						read(modempipes[0], cbuf, 1);
						if(*cbuf == 'G') pthread_cond_wait(&mpcond, &modemmx); else {
							pthread_mutex_unlock(&miomx);
							pthread_mutex_unlock(&modemmx);
							pthread_mutex_lock(&cfmx);
							close_modem(cf.modemdev);
							pthread_mutex_unlock(&cfmx);
							pthread_exit(NULL);
							break;
						}
					}
				}
		}
		if(*cbuf == 'D') break;
	}
	/* NOTREACHED */
	pthread_mutex_unlock(&miomx);
	pthread_mutex_unlock(&modemmx);
	pthread_exit(NULL);
	return 0;
}
#if 0 /* all old... */
/* Modem control stuff: be forewarned, this might become pluggable!
 * Rockwell for now.
 */
void modem_pickup(void) 
{
	/* no locking because stmod() does it for us */
	stmod(ROCKWELL_PICKUP);
}
void modem_hangup(void)
{
	stmod(ROCKWELL_HANGUP);
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
#endif
