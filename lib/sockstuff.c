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
/* $Amigan: phoned/lib/sockstuff.c,v 1.1 2005/06/26 04:47:20 dcp1990 Exp $ */
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

#define CMD_ARGS (ClientData clientData, Tcl_Interp *interp, int argc, Tcl_Obj *CONST objv[])

int UdomConnectSocket (cdata, interp, argc, argv)
	ClientData cdata;
	Tcl_Interp *interp;
	int argc;
	char **argv;
{
	int s;
	struct sockaddr_un them;
	if(argc < 2) {
		return TCL_ERROR;
	}
	s = socket(AF_LOCAL, SOCK_STREAM, 0);
	strcpy(them.sun_path, sockfile);
	them.sun_family = AF_LOCAL;
	if(connect(s, (struct sockaddr *)&them, 1 + strlen(them.sun_path) + sizeof(them.sun_family)) == -1) {
		return TCL_ERROR;
	}
}
int UdomClose(idata, interp)
	ClientData idata;
	Tcl_Interp *interp;
{
	close(socket);
}
int UdomInput(idata, buf, sizebuf, errorptr)
	ClientData idata;
	char *buf;
	int sizebuf;
	int *errorptr;
{
	int ec;
	ec = read((int)idata, buf, sizebuf);
	if(ec == -1) {
		*errorptr = errno;
		return -1;
	} else {
		return ec;
	}
}

int UdomOutput(idata, buf, towrite, ecptr)
	ClientData idata;
	CONST char *buf;
	int towrite;
	int *ecptr;
{
	int rc;
	rc = write((int)idata, buf, towrite);
	if(rc == -1) {
		*ecptr = errno;
		return -1;
	} else {
		return rc;
	}
}

/* write watcher */
Tcl_ChannelType ourchan = {
	"udomain",
	TCL_CHANNEL_VERSION_3,
	&UdomClose,
	&UdomInput,
	&UdomOutput,
	NULL, /* seek */
	NULL, /* set and getoption, might do later */
	NULL,
	&UdomWatch,
	&UdomGetHandle,
	NULL, /* close2proc */
	NULL, /* block mode */
	&UdomFlush,
	&UdomHandler,
	&UdomWideSeek /* return EINVAL */
};
