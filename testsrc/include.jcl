//IBMUSERC JOB 'inc-1',MSGCLASS=S,NOTIFY=&SYSUID,REGION=0M              
//MYLIB JCLLIB ORDER=IBMUSER.DEV.PROCLIB                                
//GETIT INCLUDE MEMBER=MEM1                                             
//                                                                      
//IBMUSERC JOB 'inc-2',MSGCLASS=S,NOTIFY=&SYSUID,REGION=0M              
//MYLIB JCLLIB ORDER=('IBMUSER.DEV.PROCLIB','IBMUSER.DEV2.PROCLIB','IBM 
//             USER.DEV3.PROCLIB',                                      
//            IBMUSER.DEV4.PROCLIB)                                     
//GETIT INCLUDE MEMBER=MEM2                                             
//
