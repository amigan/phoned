# main makefile...
# $Amigan: phoned/Makefile,v 1.2 2005/06/01 00:43:07 dcp1990 Exp $
# Oh My Fucking God, this is such a big, unportable mess. Oh well.
# BSD Make > *
APPS=cnd phoned
APPSUF=${APPS:S/$/_app/}
APPSCLEAN=${APPS:S/$/_cl/}
all: ${APPSUF}
${APPSUF}:
	@cd ${@:S/_app$//} && make
clean: ${APPSCLEAN} ourclean
${APPSCLEAN}:
	@cd ${@:S/_cl$//} && make clean
ourclean:
#	rm -f .conf
#.conf:
#	@perl -e 'require 5.0001; require Modem::fgetty' \
#	|| echo "Needs Modem::Vgetty and perl 5.0001 at least."
