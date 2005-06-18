# cnd Makefile
# (C)2005, Dan Ponte
# $Amigan: phoned/phoned/modems/Makefile,v 1.1 2005/06/18 03:10:56 dcp1990 Exp $
include ../../global.mk
# basic stuff. we append for a reason.
CPPFLAGS+=-I../../include
CFLAGS+=-g -Wall -W -ansi ${CPPFLAGS}
LDFLAGS=
# keep these up to date.
MAINBIN=libmodems.a
SRCS=rockwell.c
OBJS=rockwell.o
all: .depend ${MAINBIN}
# I know, I know, but it's good.
.depend: ${SRCS} ${OHDRS}
	mkdep ${CPPFLAGS} -MM -p ${SRCS}
${MAINBIN}: ${OBJS}
	ar rc ${MAINBIN} ${OBJS}
	ranlib ${MAINBIN}
# for this app
%.o: %.c
	${CC} ${CFLAGS} -c ${.SOURCE}
# end ours
clean:
	rm -f *.o ${MAINBIN} .depend *~ *.core ${CLEANFILES}
