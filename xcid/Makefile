# for xcid
# $Amigan: phoned/xcid/Makefile,v 1.1 2005/06/23 02:23:14 dcp1990 Exp $
CPPFLAGS+=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib -lXt -lX11 -lXaw -lpthread
.ifdef DEBUG
CPPFLAGS+=-DDEBUG
.endif
.ifndef NO_XOSD
LDFLAGS+=-lxosd
CPPFLAGS+=-DUSE_XOSD
.endif
CFLAGS+=-g -Wall $(CPPFLAGS)
OBJS=xcid.o network.o wind.o
SRCS=xcid.c network.c wind.c
all: xcid
xcid: ${OBJS}
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o xcid
xcid.o: xcid.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c xcid.c
network.o: network.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c network.c
wind.o: wind.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c wind.c
clean:
	rm -f *.o core.* *.core *~ xcid
