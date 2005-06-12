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
/* $Amigan: phoned/phoned/filters.c,v 1.7 2005/06/12 18:56:20 dcp1990 Exp $ */
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
	free(cond->namerx.prex);
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

cond_t* add_condition(filtname, nameregex, numregex, action)
	char* filtname;
	char* nameregex;
	char* numregex;
	int action;
{
	cond_t *c, *nc;
	nc = malloc(sizeof(cond_t));
	memset(nc, 0, sizeof(cond_t));
	pthread_mutex_lock(&condmx); /* ALWAYS do this if dealing with next, last or topcond!! */
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
	} else {
		nc->name = NULL;
		nc->namerx.prex = 0x0;
	}
	if(numregex != 0x0) {
		nc->number = strdup(numregex);
		nc->numbrx.prex = pcre_compile(nc->number, 0x0, &nc->numbrx.error,
				&nc->numbrx.erroroffset, NULL);
	} else {
		nc->number = NULL;
		nc->numbrx.prex = 0x0;
	}
	nc->action = action;
	lprintf(info, "Added filter %s, namerx = %s, numrx = %s, action = %b\n",
			filtname, nameregex, numregex, action, "\10" SCTACT_IGN "IGNORE" SCTACT_HUP "HANGUP"
			SCTACT_RNOT "MAIL" SCTACT_ANS "ANSWER" SCTACT_PLAY "PLAY" SCTACT_REC "RECORD\n");
	pthread_mutex_unlock(&condmx); /* done */
	return nc;
}
