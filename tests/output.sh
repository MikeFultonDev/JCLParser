#
# Test output JCL 
#
#set -x
. setcc outputJCL
jcl2sh -d <../testsrc/output.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
