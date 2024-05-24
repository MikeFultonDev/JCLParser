#
# Test error scenarios for jcl2sh
#
#set -x
. setcc errInvalidOptionTooShort
jcl2sh --in=/tmp/file
. setcc errInvalidOptionTooLong
jcl2sh --inputFileName=/tmp/file  
. setcc errNoInputFileSpecified
jcl2sh -i=
. setcc errNoArgSpecified
jcl2sh --output=
. setcc longRecordQuiet
jcl2sh -i=../testsrc/shortAndLong.jcl
. setcc longRecordVerbose
jcl2sh -v -i=../testsrc/shortAndLong.jcl