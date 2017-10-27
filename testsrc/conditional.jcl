//IBMUSERM JOB  'JOB',MSGCLASS=S,NOTIFY=&SYSUID,REGION=0M               
//DUMMY EXEC PGM=IEFBR14                                                
//CHECK IF (RC > 0) THEN                                                
//NORUN EXEC PGM=UNK                                                    
//SYSIN    DD DSN=UNK.UNK,DISP=OLD                                      
//SYSPRINT DD SYSOUT=*                                                  
//CHECK ELSE                                                            
//YESRUN EXEC PGM=IEFBR14                                               
//SYSPRINT DD SYSOUT=*                                                  
//CHECK ENDIF 
// IF                                                    
// RC >= 0
// THEN                  I can put some comments in here
//YES2RUN EXEC PGM=IEFBR14
//SYSUT1 DD SYSOUT=*
// ENDIF
