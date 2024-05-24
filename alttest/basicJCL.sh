#
# Test basic JCL 
#
#set -x
. setcc basicJCL
jcl2sh -d  -i=../testsrc/crthfs.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
