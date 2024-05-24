#set -x
. ./setenv.sh 

cd ../alttest
rm -f *.actual

if [ -z $1 ] ; then
	tests=*.sh
else
	tests=${1}.sh
fi
for test in ${tests}; do
	echo ${test}
	name="${test%.*}"
	${test} >${name}.actual 2>&1
	diff -w ${name}.actual ${name}.expected
done
