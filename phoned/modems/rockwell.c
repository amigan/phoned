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
/* $Amigan: phoned/phoned/modems/rockwell.c,v 1.8 2005/06/26 21:44:40 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <phoned.h>
#define ROCKWELL_INITSTRING	"ATZ\r\nAT E0 #CID=2 V0\r\n"
#define ROCKWELL_PICKUP		"ATH1"
#define ROCKWELL_HANGUP		"ATH"
#define ROCKWELL_RESET		"ATZ"
/* LINTLIBRARY */ /* PROTOLIB1 */
short plug_init(void)
{
	return 1; /* ok */
}
/* modem stuff */
int rw_init(void)
{
	stmod(ROCKWELL_INITSTRING);
	stmod("AT#CID=2");
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
	rescode = (int)strtol(result, NULL, 10);
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
void rw_sdev(d)
	enum device_t d;
{
	char buf[256];
	int dv;
	switch(d) {
		case dialup:
			dv = 0;
			break;
		case handset:
			dv = 1;
			break;
		case speaker:
			dv = 2;
			break;
		case mic:
			dv = 3;
			break;
		case phonewspk:
			dv = 4;
			break;
		case teleemu:
			dv = 5;
			break;
		case spkrphone:
			dv = 6;
			break;
		case musonhold:
			dv = 7;
			break;
		case handsetconvo:
			dv = 8;
			break;
		case soundchip:
			dv = 9;
			break;
		default:
			dv = 0;
			break;
	}
	sprintf(buf, "AT#VLS=%d", dv);
	stmod(buf);
}
/* voice */
void rw_voice_init(void)
{
	stmod("ATE0");
	stmod("ATV0");
	stmod("AT#VSP=55");
	stmod("AT#VSD=0");
	stmod("AT#VBS=4");
	stmod("AT#BDR=16");
	stmod("A#VTD=3F,3F,3F");
	stmod("ATS30=60");
	stmod("AT#CLS=8");
	rw_sdev(dialup);
	stmod("AT#CID=2");
	stmod("AT#CLS=8");
}
void rw_set_rings(rings)
	int rings;
{
	char buf[20];
	sprintf(buf, "AT S0=%d", rings);
	stmod(buf);
}
modem_t rockwell = {
	"ROCKWELL",
	0x0,
	0x0,
	&rw_init,
	&rw_destroy,
	&rw_evalrc,
	&rw_pickup,
	&rw_hangup,
	&rw_sdev,
	&rw_voice_init,
	&rw_set_rings
};
