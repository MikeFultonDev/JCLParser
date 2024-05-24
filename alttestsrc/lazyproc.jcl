//IBMUSERC JOB 'inline',MSGCLASS=S,MSGLEVEL=(1,1)                       
//*
//* The proc is invalid (" instead of ') but
//* the JCL scanner processes this ok since  
//* the proc is not actually referenced
//*
//PLIXCLM PROC LLPREFX='SHARE.PLI111',                                  
//         CEETASK="TSCTEST.BETA.SIBMTASK'                              
//FRED EXEC PGM=IEFBR14                                                 
// PEND                                                                 
//JOE EXEC PGM=IEFBR14 
