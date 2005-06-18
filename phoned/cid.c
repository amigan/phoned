/*
 * Copyright (c) 2005, Dan Ponte
 *
 * cid.c - caller ID code (borrowed from cidserv)
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
/* $Amigan: phoned/phoned/cid.c,v 1.4 2005/06/18 20:40:15 dcp1990 Exp $ */
/* system includes */
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include "phoned.h"
int free_cid(cid_t* ctf)
{
	free(ctf->name);
	free(ctf->number);
	free(ctf);
	return 1;
}
cid_t* parse_cid(char* cidstring)
{
	char *p, *datep;
	unsigned int len = 0, i;
	char finalbuf[1024], msg[512];
	char printbuf[2048];
	unsigned char cch;
	char cbyte, cbyte2;
	char bytebuf[10];
	char date[7];
	time_t rnow;
	struct tm *ctm;
	char cidtime[7];
	char name[128];
	char phone[128];
	int cur = 0, sz, fbcou = 0;
	short int finl = 0;
	cid_t* c;
	c = malloc(sizeof(cid_t));
	memset(msg, 0, sizeof msg);
	memset(bytebuf, 0, sizeof bytebuf);
	memset(date, 0, sizeof date);
	memset(finalbuf, 0, sizeof(finalbuf));
	memset(printbuf, 0, sizeof(printbuf) * sizeof(char));
	if(cidstring[strlen(cidstring)] == '\n')
		cidstring[strlen(cidstring)] = 0;
	sz = strlen(cidstring);
	finl = (sz / 2) - 2;
	rnow = time(NULL);
	ctm = localtime(&rnow);
	for(cur = 0; cur <= sz; cur++) {
		cbyte = cidstring[cur++];
		cbyte2 = cidstring[cur];
		sprintf(bytebuf, "0x%c%c", cbyte, cbyte2);
		sscanf(bytebuf, "%X", (int*)&cch);
		if(cch == 0) continue;
		if(fbcou <= finl) {
			finalbuf[fbcou] = cch;
			fbcou++;
		} else break;
		memset(bytebuf, 0, sizeof(bytebuf));
		cbyte = 0;
		cbyte2 = 0;
	}
	sz = fbcou;
	p = finalbuf;
	len = sz;
	while(len && !(*p >= 0x30 && *p <= 0x39)) {
		p++;
		len--;
	}
	datep = p;
	date[0] = datep[0];
	date[1] = datep[1];
	date[2] = '/';
	date[3] = datep[2];
	date[4] = datep[3];
	date[5] = 0;
	datep += 4;
	cidtime[0] = datep[0];
	cidtime[1] = datep[1];
	cidtime[2] = ':';
	cidtime[3] = datep[2];
	cidtime[4] = datep[3];
	cidtime[5] = 0;
	p += 8;
	len -= 8;
	while(len && !isprint(*p)) {
		p++;
		len--;
	}
	i = 0;
	while(len && (i < sizeof(finalbuf)) && isprint(*p)) {
		name[i++] = *p++;
		len--;
	}
	name[i] = 0;
	if(name[0] == 'P' && !(isalpha(name[1]) || (name[1] >= '0' && name[1] <= '9'))) {
		strcpy(name, "PRIVATE");
	}
	if(name[0] == 'O' && !(isalpha(name[1]) || (name[1] >= '1' && name[1] <= '9'))) {
		strcpy(name, "UNAVAILABLE");
	}
	while(len && !isprint(*p)) {
		p++;
		len--;
	}
	i = 0;
	while(len && (i < sizeof(finalbuf)) && isprint(*p) && 
			(p[0] >= 0x20 && p[0] <= 0x7e)) {
		if(!((p[0] >= '0' && p[0] <= '9') || p[0] == 'O' || p[0] == 'P'))
		{
			p++;
			len--;
		       continue;
		} else {
			phone[i++] = *p++;
			len--;
		       }
	}
	phone[i] = 0;
	if(phone[0] == 'P')
	       strcpy(phone, "PRIVATE");
	if(phone[0] == 'O')
		strcpy(phone, "UNAVAILABLE");
#if 0
	c->name = malloc((sizeof(char) * strlen(name)) + 1);
	strncpy(c->name, name, strlen(name) + 1);
	c->number = malloc((sizeof(char) * strlen(phone)) + 1);
	strncpy(c->number, phone, strlen(phone) + 1);
	sscanf(date, "%d/%d", (int*)&c->month, (int*)&c->day);
#endif
	c->name = strdup(name);
	c->number = strdup(phone);
	sscanf(cidtime, "%d:%d", (int*)&c->hour, (int*)&c->minute);
	date[2] = 0x0;
	c->month = strtol(date, NULL, 10);
	c->day = strtol(date + 3, NULL, 10);
	date[2] = '/';
	return c;
}
