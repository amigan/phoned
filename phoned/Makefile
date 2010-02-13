# cnd Makefile
# (C)2005, Dan Ponte
# $Amigan: phoned/phoned/Makefile,v 1.16 2010/02/13 19:03:04 dcp1990 Exp $
include ../global.mk
# basic stuff. we append for a reason.
CPPFLAGS+=-I../include -DDEBUG -DYY_NO_UNPUT -DMODDEBUG -DMODEMSAID
CFLAGS+=-g -Wall -W -ansi ${CPPFLAGS} -pthread
LDFLAGS+=-lutil -lpcre -lsqlite3
# keep these up to date.
MAINBIN=phoned
SRCS=main.c init.c log.c cfg.c socket.c y.tab.c lex.yy.c signals.c cid.c modem.c notify.c filters.c remote.c db.c md5.c voice.c
OBJS=main.o init.o log.o cfg.o socket.o y.tab.o lex.yy.o signals.o cid.o modem.o notify.o filters.o remote.o db.o md5.o voice.o \
	modems/libmodems.a
OHDRS=y.tab.h
CLEANFILES=y.tab.c y.tab.h lex.yy.c
LEX=lex
YACC=yacc

include ../main.mk

modems/libmodems.a:
	cd modems && $(MAKE)
y.tab.h: y.tab.c
y.tab.c: config.y
	$(YACC) -d config.y
lex.yy.c: config.l y.tab.c y.tab.h
	$(LEX) config.l
ourclean:
	cd modems && $(MAKE) clean
