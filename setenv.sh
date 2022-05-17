#
# To be called from crtTests and runTests to set up the environment
# Modify this file to match your z/OS system
#
export JCL2SH_ROOT=/u/ibmuser/JCL2SH           # Location where jcl2sh installed
export CHLQ=CBC                                # High Level qualifier for C/C++ compiler datasets (SCCNCMP)
export DBGHLQ=EQAE00                           # High Level qualifier for Code Coverage (optional)
export TESTHLQ=IBMUSER                         # High Level qualifier for debug TEST entry/exit service

export PATH=${JCL2SH_ROOT}/bin:${PATH}
export DBGLIB=${DBGHLQ}.SEQAMOD:${TESTHLQ}.CEEV2R2.CEEBINIT
