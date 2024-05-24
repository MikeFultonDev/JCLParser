#
# Test include JCL 
#
#set -x
. setcc includeJCL
jcl2sh -d <../testsrc/include.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
