# $Amigan: phoned/main.mk,v 1.2 2005/06/01 00:43:07 dcp1990 Exp $
all: .depend ${MAINBIN}
# I know, I know, but it's good.
.depend: ${SRCS} ${OHDRS}
	mkdep ${CPPFLAGS} -MM -p ${SRCS}
${MAINBIN}: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${MAINBIN} ${OBJS}
# for this app
%.o: %.c
	${CC} ${CFLAGS} -c ${.SOURCE}
# end ours
clean:
	rm -f *.o ${MAINBIN} .depend *~ *.core ${CLEANFILES}
