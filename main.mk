# $Amigan: phoned/main.mk,v 1.5 2005/06/18 20:22:07 dcp1990 Exp $
all: .depend ${MAINBIN}
# I know, I know, but it's good.
.depend: ${SRCS} ${OHDRS}
	mkdep ${CPPFLAGS} -MM -p ${SRCS}
${MAINBIN}: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${MAINBIN} ${OBJS}
# for this app
%.o: %.c
	${CC} ${CFLAGS} -c ${.SOURCE}
lint:
	lint ${CPPFLAGS} -pces ${SRCS}
# end ours
clean: ourclean
	rm -f *.o ${MAINBIN} .depend *~ *.core ${CLEANFILES}
