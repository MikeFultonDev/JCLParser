//IBMUSERM JOB  'JOB',MSGCLASS=S,NOTIFY=&SYSUID,REGION=0M,              
// MSGLEVEL=(1,1)                                                       
//STEP1 EXEC    PGM=IEFBR14,REGION=20M,PARM='/is/a/very/long/directory/ 
//              that/extends/across/three/lines/and/therefore/can/showc 
//              ase/long/strings'                                                    
//MYDD  DD      SYSOUT=* This is a multi-line comment                  *
// indicated with an asterisk in column 72 and then adding             *
// another line with another asterisk                                   
//* Standard comment to end the example 
