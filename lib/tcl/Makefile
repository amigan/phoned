# xfone Makefile
# (C)2005, Dan Ponte
# $Amigan: phoned/lib/tcl/Makefile,v 1.1 2005/06/27 00:12:28 dcp1990 Exp $
include ../global.mk
# basic stuff. we append for a reason.
CPPFLAGS+=-I/usr/local/include/tcl8.4
CFLAGS+=-g ${CPPFLAGS}
LDFLAGS+=-shared
# keep these up to date.
MAINBIN=udom.so
SRCS=sockstuff.c
OBJS=sockstuff.o

include ../main.mk
ourclean:

