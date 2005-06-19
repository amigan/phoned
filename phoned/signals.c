/*
 * Copyright (c) 2005, Dan Ponte
 *
 * signals.c - signal handlers
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
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <phoned.h>
extern pthread_t networkth;
void handsig(sig)
	int sig;
{
	if(pthread_equal(pthread_self(), networkth)) {
		lprintf(info, "siamo spesa");
		return;
	}
	signal(sig, handsig);
	switch(sig) {
		case SIGINT:
		case SIGTERM:
			lprintf(fatal, "Received signal %d, cleaning up...\n", sig);
			awaken_sel();
			lprintf(info, "woke up sel");
			modem_wake();
			lprintf(info, "woke up mod");
			shutd(0x1 | 0x2 | 0x4 | 0x10 | 0x20);
			exit(0);
			break;
		case SIGHUP:
			lprintf(info, "Received HUP, rereading configuration files...\n");
			break;
		default:
			lprintf(warn, "Received signal %d!\n", sig);
	}
}

void install_handlers(void)
{
	signal(SIGINT, handsig);
	signal(SIGQUIT, handsig);
	signal(SIGHUP, handsig);
	signal(SIGTERM, handsig);
}
