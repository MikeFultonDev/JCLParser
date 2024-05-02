#
# Set up just for 'mac' build. For z/OS, use build.sh as a template
#
DEFAULT_MODE="DEBUG"

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
	export CC_OPTS='-O2'
	export LINK_OPTS='-O2'
fi

mkdir -p bin
cd bin
rm -f jcl2sh jcl2jcl *.o *.lst *.dbg
cc -c ${CC_OPTS}  ../src/jcl2jcl.c
cc -c ${CC_OPTS}  ../src/scanjcl.c
cc -c ${CC_OPTS}  ../src/genjcl.c
cc -c ${CC_OPTS}  ../src/jclargs.c
cc -c ${CC_OPTS}  ../src/jclmsgs.c
cc -o jcl2jcl ${LINK_OPTS}  jcl2jcl.o scanjcl.o genjcl.o jclargs.o jclmsgs.o
