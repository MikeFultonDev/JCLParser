#!/bin/sh

# Run regression on tricky tests

# Defects open for some problems already - these tests are skipped
# because the filetype is 'skipjcl'. These will be added to the 
# 'bucket' as they are fixed.

export JCL2JCL_TEST_NOHACKS=1

./testtree.sh ../testsrc
