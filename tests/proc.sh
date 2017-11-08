#
# Test proc JCL 
#
#set -x
. setcc procJCL
jcl2sh -d <../testsrc/proc.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
