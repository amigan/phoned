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
/* $Amigan: phoned/phoned/cid.c,v 1.8 2005/06/22 04:01:03 dcp1990 Exp $ */
/* system includes */
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <phoned.h>
/* ex: BOGUS(p, cpl, i, s, "numb"); */
#define BOGUS(pnt, cl, st, orig, part)	if(pnt + cl - 1 > orig + st) { \
	lprintf(error, "%s: Packet length of %d is bogus (len is %d, would be 0x%x when should 0x%x!)", part, cl, st, pnt + cl, orig + st); break; }
int free_cid(cid_t* ctf)
{
	free(ctf->name);
	free(ctf->number);
	free(ctf);
	return 1;
}
cid_t *decode_sdmf(s)
	unsigned char *s;
{
	unsigned char *p;
	unsigned char dtbuf[2];
	int i = 0, ld, rsz, j, cstotal, checksum;
	cid_t *c;
	if(*s != 0x04) return NULL; /* 0x04 is SDMF, anything else is garbage */
	c = malloc(sizeof(cid_t));
	memset(c, 0x0, sizeof(cid_t));
	c->name = strdup("NO NAME");
	p = s;
	rsz = strlen(s);
	p++;
	ld = *p++;
	i = strlen(p);
	dtbuf[2] = 0x0;
	*dtbuf = *p++;
	dtbuf[1] = *p++;
	c->month = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
	*dtbuf = *p++;
	dtbuf[1] = *p++;
	c->day = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
	*dtbuf = *p++;
	dtbuf[1] = *p++;
	c->hour = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
	*dtbuf = *p++;
	dtbuf[1] = *p++;
	c->minute = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
	c->number = malloc(strlen(p) * sizeof(unsigned char));
	snprintf(c->number, strlen(p), "%s", p);
	checksum = s[rsz - 1];
	cstotal = 0;
	for(j = 0; j < rsz - 1; j++) {
		cstotal += s[j];
	}
	if((cstotal & 0xff) + checksum != 256)
		lprintf(warn, "decode_sdmf: Warning: Checksum failed! (0x%x real, cs was 0x%x, total %d)",
				(cstotal & 0xff) + checksum, checksum, cstotal);
	return c;
}

cid_t *decode_mdmf(s)
	unsigned char *s;
{
	unsigned char *p;
	unsigned char dtbuf[2];
	int i = 0, j = 0, ld, cpl, checksum, rsz;
	int cstotal;
	cid_t *c;
	if(*s != 0x80) return NULL; /* 0x04 is SDMF, anything else is garbage */
	c = malloc(sizeof(cid_t));
	memset(c, 0x0, sizeof(cid_t));
	rsz = strlen(s);
	p = s;
	p++;
	ld = *p++;
	i = strlen(p);
	lprintf(info, "p == 0x%x", *p);
	for(; p <= s + i; /* look below */) {
		switch(*p++) { /* data type */
			case 0x01: /* date and time */
				cpl = *p++;
				BOGUS(p, cpl, i, s, "dtime");
				if(cpl != 8) lprintf(warn, "decode_mdmf: Warning: date length != 8 (%d really)", cpl);
				*dtbuf = *p++;
				dtbuf[1] = *p++;
				dtbuf[2] = '\0';
				c->month = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
				*dtbuf = *p++;
				dtbuf[1] = *p++;
				c->day = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
				*dtbuf = *p++;
				dtbuf[1] = *p++;
				c->hour = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
				*dtbuf = *p++;
				dtbuf[1] = *p++;
				c->minute = ((*dtbuf - '0') * 10) + (dtbuf[1] - '0');
				break;
			case 0x02: /* number */
				cpl = *p++;
				BOGUS(p, cpl, i, s, "numb");
				if(cpl != 10) lprintf(info, "decode_mdmf: Info: number length != 10 (%d really)", cpl);
				c->number = malloc(cpl * sizeof(unsigned char));
				memset(c->number, 0, cpl * sizeof(unsigned char));
				for(j = 0; j < cpl && *p != 0; j++) {
					c->number[j] = *p++;
				}
				c->number[j] = '\0';
				break;
			case 0x04: /* number not available */
				cpl = *p++;
				BOGUS(p, cpl, i, s, "nna");
				if(cpl < 0x01) {
					lprintf(warn, "decode_mdmf: Warning: type 0x04 (no number) len < 1 (%d really)", cpl);
					break;
				}
				*dtbuf = *p++;
				if(c->number != NULL)
					c->number = strdup(*dtbuf == 'O' ? "UNAVAILABLE" : (*dtbuf == 'P' ? "PRIVATE" : "UNKNOWN")); /* ternary city, baby */
				if(cpl > 0x01) p += cpl - 1;
				break;
			case 0x07: /* name */
				cpl = *p++;
				BOGUS(p, cpl, i, s, "name");
				c->name = malloc(cpl * sizeof(unsigned char));
				memset(c->name, 0, cpl * sizeof(unsigned char));
				for(j = 0; j < cpl && p != 0; j++) {
					c->name[j] = *p++;
				}
				c->name[j] = '\0';
				if(strcmp(c->name, "O") == 0) {
					free(c->name);
					c->name = strdup("UNAVAILABLE");
				} else if(strcmp(c->name, "P") == 0) {
					free(c->name);
					c->name = strdup("PRIVATE");
				}
				break;
			case 0x08: /* name not available */
				cpl = *p++;
				BOGUS(p, cpl, i, s, "nana");
				if(cpl < 0x01) {
					lprintf(warn, "decode_mdmf: Warning: type 0x08 (no name) len < 1 (%d really)", cpl);
					break;
				}
				*dtbuf = *p++;
				if(c->name != NULL)
					c->name = strdup(*dtbuf == 'O' ? "UNAVAILABLE" : (*dtbuf == 'P' ? "PRIVATE" : "UNKNOWN")); /* ternary city, baby */
				if(cpl > 0x01) p += cpl - 1;
				break;
			default:
				p++;
				if(s + i - 1 == p) { /* checksum */
					checksum = *p++;
				}
				break;
		}
	}
	checksum = s[rsz - 1];
	cstotal = 0;
	for(j = 0; j < rsz - 1; j++) {
		cstotal += s[j];
	}
	if((cstotal & 0xff) + checksum != 256)
		lprintf(warn, "decode_mdmf: Warning: Checksum failed! (0x%x real, cs was 0x%x, total %d)",
				(cstotal & 0xff) + checksum, checksum, cstotal);
	return c;
}

cid_t* parse_cid(char* cidstring)
{
	unsigned char *p;
	unsigned int len = 0;
	unsigned char finalbuf[1024];
	unsigned char cch;
	unsigned char cbyte, cbyte2;
	unsigned char bytebuf[10];
	int cur = 0, sz, fbcou = 0;
	short int finl = 0;
	cid_t* c;
	memset(bytebuf, 0, sizeof bytebuf);
	memset(finalbuf, 0, sizeof(finalbuf));
	if(cidstring[strlen(cidstring)] == '\n')
		cidstring[strlen(cidstring)] = 0;
	sz = strlen(cidstring);
	finl = sz / 2;
	strcpy(bytebuf, "0x");
	for(cur = 0; cur <= sz; cur++) {
		cbyte = cidstring[cur++];
		cbyte2 = cidstring[cur];
		bytebuf[2] = cbyte; bytebuf[3] = cbyte2;
		cch = strtol(bytebuf, NULL, 16);
		if(cch == 0) continue;
		if(fbcou <= finl) {
			finalbuf[fbcou] = cch;
			fbcou++;
		} else break;
		/* perche? memset(bytebuf, 0, sizeof(bytebuf)); */
		cbyte = 0;
		cbyte2 = 0;
	}
	sz = fbcou;
	p = finalbuf;
	len = sz;
	if(*finalbuf == 0x80) {
		/* MDMF */
		c = decode_mdmf(finalbuf);
	} else if(*finalbuf == 0x04) {
		c = decode_sdmf(finalbuf);
	} else c = NULL;
#if 0
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
	ct = strchr(cidtime, ':');
	if(ct != NULL) *ct = '\0';
	c->hour = strtol(cidtime, NULL, 10);
	c->minute = strtol(ct + 1, NULL, 10);
	*ct = ':';
	date[2] = 0x0;
	c->month = strtol(date, NULL, 10);
	c->day = strtol(date + 3, NULL, 10);
	date[2] = '/';
#endif
	return c;
}
void cid_handle(c)
	cid_t *c;
{
	time_t ti;
	ti = time(NULL);
	cid_log(c);
	cid_notify(c);
	check_condition(c);
	db_add_call(c, ti);
}
