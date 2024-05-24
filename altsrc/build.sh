#
# GONUM and OFFSET make it easier to debug with no real downside
# LANGLVL(EXTENDED) is required because of the z-specific extensions that are utilized
# Listings aren't required
#
DEFAULT_MODE="OPT"

if [ -z "${1}" ]; then
    	mode=${DEFAULT_MODE};
else
	mode=${1};
fi
if [ ${mode} = 'DEBUG' ] ; then
	echo 'Build Debug Version'
	export ASM_OPTS='' 
	export CC_OPTS='-g -DDEBUG' 
	export LINK_OPTS='-g'
else
	echo 'Build Default Version'
	export ASM_OPTS='' 
	export CC_OPTS='-O2 -Wc,FLAG(I)' #',CHECKOUT(ALL)' 
	export LINK_OPTS='-O2'
fi

. ./setenv.sh
export STEPLIB=${CHLQ}.SCCNCMP
mkdir -p bin
cd bin
rm -f jcl2sh *.o *.lst *.dbg
c89 -c ${CC_OPTS} -Wc,xplink,gonum,offset,langlvl\(extended\),list\(./\) jcl2sh.c
c89 -c ${CC_OPTS} -Wc,xplink,gonum,offset,langlvl\(extended\),list\(./\) ../src/scanjcl.c
c89 -c ${CC_OPTS} -Wc,xplink,gonum,offset,langlvl\(extended\),list\(./\) gensh.c
c89 -c ${CC_OPTS} -Wc,xplink,gonum,offset,langlvl\(extended\),list\(./\) ../src/jclargs.c
c89 -c ${CC_OPTS} -Wc,xplink,gonum,offset,langlvl\(extended\),list\(./\) ../src/jclmsgs.c
c89 -o jcl2sh ${LINK_OPTS} -Wl,xplink jcl2sh.o scanjcl.o gensh.o jclargs.o jclmsgs.o
