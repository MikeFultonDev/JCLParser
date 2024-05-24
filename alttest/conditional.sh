#
# Test conditional JCL 
#
#set -x
. setcc conditionalJCL
jcl2sh -d <../testsrc/conditional.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
