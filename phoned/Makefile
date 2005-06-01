# cnd Makefile
# (C)2005, Dan Ponte
# $Amigan: phoned/phoned/Makefile,v 1.3 2005/06/01 01:11:23 dcp1990 Exp $
include ../global.mk
# basic stuff. we append for a reason.
CPPFLAGS=-I../include -DDEBUG
CFLAGS+=-g -Wall -W -ansi ${CPPFLAGS}
LDFLAGS=
# keep these up to date.
MAINBIN=phoned
SRCS=main.c init.c log.c cfg.c socket.c y.tab.c lex.yy.c signals.c cid.c
OBJS=main.o init.o log.o cfg.o socket.o y.tab.o lex.yy.o signals.o cid.o
OHDRS=y.tab.h
CLEANFILES=y.tab.c y.tab.h lex.yy.c

include ../main.mk

y.tab.h: y.tab.c
y.tab.c: config.y
	yacc -d config.y
lex.yy.c: config.l y.tab.c y.tab.h
	lex config.l
