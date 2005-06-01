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
