#
# This will create the tests and copy contents in from testsrc
# used by runTests.sh
#
. ./setenv.sh

cd testsrc 

# Update the templates and cmd files to have the correct HLQ's

extension="expected"
tgtdir="../tests"
for f in *.template; do
  xx=$(basename ${f}  .template); sed -e "s/@@HLQ@@/${TESTHLQ}/g" ${f} >${tgtdir}/${xx}.${extension}
done
