#
# Test COMMAND JCL 
#
#set -x
. setcc commandJCL
jcl2sh -d <../testsrc/command.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
