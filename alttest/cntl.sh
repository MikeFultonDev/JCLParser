#
# Test CNTL/PRINTDEV/ENDCNTL JCL 
#
#set -x
. setcc cntlJCL
jcl2sh -d <../testsrc/cntl.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
