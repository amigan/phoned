# cnd Makefile
# (C)2005, Dan Ponte
# $Amigan: phoned/lib/libpvf/Makefile,v 1.1 2005/06/14 02:40:07 dcp1990 Exp $
include ../../global.mk
# basic stuff. we append for a reason.
CPPFLAGS=-I../include
CFLAGS+=-g -Wall -W -ansi ${CPPFLAGS}
LDFLAGS=
# keep these up to date.
MAINBIN=libpvf.a
SRCS=au.c fft.c lib.c linear.c multitech.c rockwell.c usr.c voc.c wav.c zyxel-o56k.c zyxel.c
OBJS=au.o fft.o lib.o linear.o multitech.o rockwell.o usr.o voc.o wav.o zyxel-o56k.o zyxel.o
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
