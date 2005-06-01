# cnd Makefile
# (C)2005, Dan Ponte
# $Amigan: phoned/phoned/Makefile.bison,v 1.1 2005/06/01 20:31:59 dcp1990 Exp $
include ../global.mk
# basic stuff. we append for a reason.
CPPFLAGS=-I../include -DDEBUG -DYYERROR_VERBOSE -DBISON
CFLAGS+=-g -Wall -W -ansi ${CPPFLAGS}
LDFLAGS=-lutil
# keep these up to date.
MAINBIN=phoned
SRCS=main.c init.c log.c cfg.c socket.c config.tab.c lex.yy.c signals.c cid.c modem.c
OBJS=main.o init.o log.o cfg.o socket.o config.tab.o lex.yy.o signals.o cid.o modem.o
OHDRS=config.tab.h
CLEANFILES=config.tab.c config.tab.h lex.yy.c
LEX=lex
YACC=bison

include ../main.mk

config.tab.h: config.tab.c
config.tab.c: config.y
	$(YACC) -d config.y
lex.yy.c: config.l config.tab.c config.tab.h
	$(LEX) config.l
