/*
 * Copyright (c) 2005, Dan Ponte
 *
 * filters.c - conditional filtering of calls/other actions
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
/* $Amigan: phoned/phoned/filters.c,v 1.8 2005/06/12 22:01:23 dcp1990 Exp $ */
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <phoned.h>

cond_t* topcond = 0x0;
pthread_mutex_t condmx = PTHREAD_MUTEX_INITIALIZER;
void free_cond_elms(cond)
	cond_t* cond;
{
	free(cond->name);
	free(cond->number);
	free(cond->filtername);
	if(cond->namerx.prex != NULL) free(cond->namerx.prex);
	if(cond->numbrx.prex != NULL) free(cond->numbrx.prex);
}
void free_condition(h, traverse)
	cond_t* h;
	short traverse;
{
	cond_t *ls, *c, *tp;
	tp = h;
	if(tp == 0x0) return;
	pthread_mutex_lock(&condmx);
	if(traverse) {
		c = tp;
		while(c != NULL) {
			free_cond_elms(c);
			ls = c;
			c = c->next;
			free(ls);
		}
	} else {
		free_cond_elms(tp);
		free(tp);
	}
	pthread_mutex_unlock(&condmx);
}

cond_t* add_condition(filtname, nameregex, numregex, action, flags)
	char* filtname;
	char* nameregex;
	char* numregex;
	int action;
	int flags;
{
	cond_t *c, *nc;
	nc = malloc(sizeof(cond_t));
	memset(nc, 0, sizeof(cond_t));
	pthread_mutex_lock(&condmx); /* ALWAYS do this if dealing with any part, for it may be freed! */
	if(topcond == 0x0)
		topcond = nc;
	else {
		for(c = topcond; c->next != 0x0; c = c->next);
		nc->last = c;
		c->next = nc;
	}
	nc->filtername = strdup(filtname);
	if(nameregex != 0x0) {
		nc->name = strdup(nameregex);
		nc->namerx.prex = pcre_compile(nc->name, 0x0, &nc->namerx.error,
				&nc->namerx.erroroffset, NULL);
		if(nc->namerx.prex == NULL) {
			lprintf(error, "NAME PCRE compilation failed at offset %d: %s\n", nc->namerx.erroroffset, nc->namerx.error);
		} else {
			pcre_fullinfo(nc->namerx.prex, NULL, PCRE_INFO_SIZE, &nc->namerx.pcresize);
		}
	} else {
		nc->name = NULL;
		nc->namerx.prex = 0x0;
	}
	if(numregex != 0x0) {
		nc->number = strdup(numregex);
		nc->numbrx.pcresize = 0x0;
		nc->numbrx.prex = pcre_compile(nc->number, 0x0, &nc->numbrx.error,
				&nc->numbrx.erroroffset, NULL);
		if(nc->numbrx.prex == NULL) {
			lprintf(error, "NUMBER PCRE compilation failed at offset %d: %s\n", nc->numbrx.erroroffset, nc->numbrx.error);
		} else {
			pcre_fullinfo(nc->numbrx.prex, NULL, PCRE_INFO_SIZE, &nc->numbrx.pcresize);
		}
	} else {
		nc->number = NULL;
		nc->numbrx.prex = 0x0;
	}
	nc->action = action;
	nc->flags = flags;
	lprintf(info, "Added filter %s, namerx = %s, numrx = %s, action = %b\n",
			filtname, nameregex, numregex, action, "\10" SCTACT_IGN "IGNORE" SCTACT_HUP "HANGUP"
			SCTACT_RNOT "MAIL" SCTACT_ANS "ANSWER" SCTACT_PLAY "PLAY" SCTACT_REC "RECORD\n");
	pthread_mutex_unlock(&condmx); /* done */
	return nc;
}
cond_t* copy_condition(con)
	cond_t* con;
{
	cond_t* c;
	c = malloc(sizeof(cond_t));
	memset(c, 0, sizeof(cond_t));
	c->filtername = strdup(con->filtername);
	if(con->name != NULL)
		c->name = strdup(con->name);
	/* XXX: is this safe? */
	if(con->namerx.prex != NULL) {
		c->namerx.pcresize = con->namerx.pcresize;
		c->namerx.prex = malloc(c->namerx.pcresize);
		memmove(c->namerx.prex, con->namerx.prex, c->namerx.pcresize);
	}
	if(con->number != NULL) 
		c->number = strdup(con->number);
	if(con->numbrx.prex != NULL) {
		c->numbrx.pcresize = con->numbrx.pcresize;
		c->numbrx.prex = malloc(c->numbrx.pcresize);
		memmove(c->numbrx.prex, con->numbrx.prex, c->numbrx.pcresize);
	}
	c->action = con->action;
	c->flags = con->flags;
	/* next and last won't be copied; this is for a *single* node */
	return c;
}
void check_condition(cid)
	cid_t* cid;
{
	cond_t *ourcond = 0x0, *c, *d, *e;
	int rcna = -1, rcnu = -1;
	int ovec[30];
	int result = 0;
	pthread_mutex_lock(&condmx);
	c = topcond;
	while(c != NULL) {
		if(c->namerx.prex != NULL)
			rcna = pcre_exec(c->namerx.prex, NULL, cid->name, strlen(cid->name), 0, 0, ovec, 30);
		if(c->numbrx.prex != NULL)
			rcnu = pcre_exec(c->numbrx.prex, NULL, cid->number, strlen(cid->number), 0, 0, ovec, 30);
		if(rcna > -1) result |= CTFLG_CHECKNAME;
		if(rcnu > -1) result |= CTFLG_CHECKNUMB;
		result = result >> 1;
		if(result >= (c->flags >> 1)) { /* aren't we clever? */
			if(ourcond == 0x0) {
					ourcond = copy_condition(c); /* we copy this so if something blocks later on, threads can still continue */
			} else {
				e = copy_condition(c);
				for(d = ourcond; d->next != 0x0; d = d->next);
				e->last = d;
				d->next = e;
			}
		}
		if(c->flags & CTFLG_STOPPROC) break;
		result = 0x0;
		rcna = -1;
		rcnu = -1;
		c = c->next;
	}
	pthread_mutex_unlock(&condmx); /* we're done with that stuff now since we have our own copies */
	c = ourcond;
	while(c != NULL) {
		if(c->action & CTACT_IGN) {
			c = c->next;
			continue;
		}
		if(c->action & CTACT_HUP) {
			lprintf(error, "Hangup not yet implemented! Check back later. (cond %s)\n", c->filtername);
		}
		if(c->action & CTACT_RNOT) {
			lprintf(error, "Mail not yet implemented! Check back later. (cond %s)\n", c->filtername);
		}
		if(c->action & CTACT_ANS) {
			lprintf(error, "Answer not yet implemented! Check back later. (cond %s)\n", c->filtername);
		}
		if(c->action & CTACT_PLAY) {
			lprintf(error, "Play not yet implemented! Check back later. (cond %s)\n", c->filtername);
		}
		if(c->action & CTACT_REC) {
			lprintf(error, "Record not yet implemented! Check back later. (cond %s)\n", c->filtername);
		}
		c = c->next;
	}
	free_condition(ourcond, 0x1);
}
