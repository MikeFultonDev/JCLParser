#
#do a simple scan of all the members of SYS1.PROCLIB
#
members=`tsocmd listds "'sys1.proclib' members" | tail +7 | awk '!/ALIAS\(/'`
for m in ${members}; do
	file="//'SYS1.PROCLIB(${m})'"
	echo ${file}
	cat ${file} | jcl2sh 
	echo $*
done
