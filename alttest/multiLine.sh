#
# Test multi-line JCL 
#
#set -x
. setcc multiLineJCL
jcl2sh -d  -i=../testsrc/multiLine.jcl | /bin/sed -e 's/ Ptr:.*/ Ptr:********/'
