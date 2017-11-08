//* Create an HFS file system                                           
//CRTHFS PROC NAME=,     No Default name                                
//       VOL=USER01,     Default Volume 'USER01'                        
//       SPACE=20        Default SPACE 20 cylinders                     
//*                                                                     
//HFS  EXEC PGM=IEFBR14                                                 
//HFS01 DD DSN=&NAME.,                                                  
//      SPACE=(CYL,(&SPACE.,&SPACE.)),                                  
//      DSNTYPE=HFS,DCB=(DSORG=PO),                                     
//      VOL=SER=&VOL.,                                                  
//      DISP=(NEW,CATLG,DELETE)                                         
//*                                                                     
//CRTHFS PEND  
