/*
 * Copyright (c) 2005, Dan Ponte
 *
 * sockstuff.c - Implement SOCK_STREAM AF_LOCAL sockets for Tcl
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
/* $Amigan: phoned/lib/tcl/sockstuff.c,v 1.4 2005/06/26 16:06:49 dcp1990 Exp $ */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/select.h>
/* network stuff */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <tcl.h>
#define mseterr(m) Udom_Error(interp, m)

#define CMD_ARGS (ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj *CONST objv[])
typedef struct cdta {
	int fd;
	Tcl_Channel channel;
} Udom_Cdata_t;

Udom_Error(interp, msg)
	Tcl_Interp *interp;
	char *msg;
{
	Tcl_SetResult(interp, msg, TCL_STATIC);
	return TCL_ERROR;
}
int Udom_Close(cdata, interp)
	ClientData cdata;
	Tcl_Interp *interp;
{
	int rc;
	Udom_Cdata_t *idata = (Udom_Cdata_t*)cdata;
	rc = close(idata->fd);
	ckfree((char*)idata);
	return rc == 0 ? rc : errno;
}
int Udom_Input(cdata, buf, sizebuf, errorptr)
	ClientData cdata;
	char *buf;
	int sizebuf;
	int *errorptr;
{
	int ec;
	Udom_Cdata_t *idata = (Udom_Cdata_t*)cdata;
	ec = read(idata->fd, buf, sizebuf);
	if(ec == -1) {
		*errorptr = errno;
		return -1;
	} else {
		return ec;
	}
}

int Udom_Output(cdata, buf, towrite, ecptr)
	ClientData cdata;
	CONST char *buf;
	int towrite;
	int *ecptr;
{
	int rc;
	Udom_Cdata_t *idata = (Udom_Cdata_t*)cdata;
	rc = write(idata->fd, buf, towrite);
	if(rc == -1) {
		*ecptr = errno;
		return -1;
	} else {
		return rc;
	}
}

void Udom_Watch(cdata, mask)
	ClientData cdata;
	int mask;
{
	Udom_Cdata_t *idata = (Udom_Cdata_t*)cdata;
	if(mask) {
		Tcl_CreateFileHandler(idata->fd, mask,
				(Tcl_FileProc*)Tcl_NotifyChannel,
				(ClientData)idata->channel);
	} else {
		Tcl_DeleteFileHandler(idata->fd);
	}
}

int Udom_GetHandle(cdata, dir, handptr)
	ClientData cdata;
	int dir;
	ClientData *handptr;
{
	Udom_Cdata_t *idata = (Udom_Cdata_t*)cdata;
	*handptr = (ClientData)idata->fd;
	return TCL_OK;
}

/* write watcher */
Tcl_ChannelType Udom_ChanType = {
	"udomain",
	TCL_CHANNEL_VERSION_3,
	&Udom_Close,
	&Udom_Input,
	&Udom_Output,
	NULL, /* seek */
	NULL, /* set and getoption, might do later */
	NULL,
	&Udom_Watch,
	&Udom_GetHandle,
	NULL, /* close2proc */
	NULL, /* block mode */
	NULL, /* flush */
	NULL, /* handler */
	NULL /* wideseek */
};



Tcl_Channel Udom_CreateChannel(sockfile, mask)
	CONST char *sockfile;
	int mask;
{
	Udom_Cdata_t *cdt;
	int s;
	char chname[25];
	struct sockaddr_un them;
	cdt = (Udom_Cdata_t*)ckalloc(sizeof(Udom_Cdata_t));
	s = socket(AF_LOCAL, SOCK_STREAM, 0);
	strcpy(them.sun_path, sockfile);
	them.sun_family = AF_LOCAL;
	if(connect(s, (struct sockaddr *)&them, 1 + strlen(them.sun_path) + sizeof(them.sun_family)) == -1) {
		ckfree((char*)cdt);
		return NULL;
	}
	cdt->fd = s;
	snprintf(chname, 24, "udom%d", cdt->fd);
	cdt->channel = Tcl_CreateChannel(&Udom_ChanType, chname, cdt, mask);
	return cdt->channel;
}
int Udom_Cmd (cdata, interp, argc, argv)
	ClientData cdata;
	Tcl_Interp *interp;
	int argc;
	Tcl_Obj *const argv[];
{
	char *arg;
	char *sfl = NULL;
	int a, optind;
	Tcl_Channel res;
	const char *udomopt[] = {
		"-file", (char*)NULL
	};
	enum udomopt {
		UDOM_FILE
	};
	for(a = 1; a < argc; a++) {
		arg = Tcl_GetString(argv[a]);
		if(*arg != '-') break;
		if(Tcl_GetIndexFromObj(interp, argv[a], udomopt, "option", TCL_EXACT, &optind)
				!= TCL_OK) return TCL_ERROR;
		switch((enum udomopt)optind) {
			case UDOM_FILE:
				if(a >= argc) return mseterr("needs file!");
				sfl = Tcl_GetString(argv[a]);
				break;
			default:
				Tcl_Panic("udom: bad optind to opts");
		}
	}
	if(sfl == NULL) return mseterr("file argument REQUIRED.");
	res = Udom_CreateChannel(sfl, TCL_READABLE | TCL_WRITABLE);
	if(res == NULL) return TCL_ERROR;
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, Tcl_GetChannelName(res), (char*)NULL);
	return TCL_OK;
}

int Udom_Init(interp)
	Tcl_Interp *interp;
{
	Tcl_CreateObjCommand(interp, "udom", Udom_Cmd, (ClientData)NULL, (Tcl_CmdDeleteProc*)NULL);
	return TCL_OK;
}
