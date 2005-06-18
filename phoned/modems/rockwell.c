/*
 * Copyright (c) 2005, Dan Ponte
 *
 * rockwell.c - Rockwell modem stuff
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
/* $Amigan: phoned/phoned/modems/rockwell.c,v 1.1 2005/06/18 03:10:56 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <phoned.h>
#define ROCKWELL_INITSTRING	"ATZ\r\nAT E0 #CID=2 V0\r\n"
#define ROCKWELL_PICKUP		"ATH1\r\n"
#define ROCKWELL_HANGUP		"ATH\r\n"
#define ROCKWELL_RESET		"ATZ\r\n"

short plug_init(void)
{
	return 1; /* ok */
}
/* modem stuff */
int rw_init(void)
{
	stmod(ROCKWELL_INITSTRING);
	return 1;
}
int rw_destroy(void)
{
	stmod(ROCKWELL_RESET);
	return 1;
}
mod_res_t rw_evalrc(result)
	char* result;
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
			return ok;
			break;
		case 2:
			return ring;
			break;
		case 4:
			return err;
			break;
		default:
			return ok;
			break;
	}
}
void rw_pickup(void) 
{
	/* no locking because stmod() does it for us */
	stmod(ROCKWELL_PICKUP);
}
void rw_hangup(void)
{
	stmod(ROCKWELL_HANGUP);
}

modem_t rockwell = {
	"ROCKWELL",
	0x0,
	0x0,
	&rw_init,
	&rw_destroy,
	&rw_evalrc,
	&rw_pickup,
	&rw_hangup
};
