//IBMUSER JOB ,D10.JOBS,MSGLEVEL=(1,1),MSGCLASS=C,CLASS=A
//BERD     EXEC PGM=BERD,TIME=1440,PERFORM=3
//SYSPRINT DD SYSOUT=(*,,STD),HOLD=YES
//SYSPOOL  DD SYSOUT=(I,INTRDR)
//*
//* This JCL uncovered a subtle bug where a comma at the
//* end of a comment had the parser think it was a continuation
//* line (see the 'ISD,' line)
//*
//SCP      DD DSN=BERD.CB33,DISP=SHR,             SORT
//         DCB=(BLKSIZE=23440)
//         DD DSN=BERD.CB84JOB,DISP=SHR          COMMERCIAL BATCH 1984
//*                                          CBL,CRS,ECR,GNR,IAM,IWS,
//*                                          KN,PY,ROCK,SMM,SYN
//         DD DSN=BERD.D3380.RDMGMT,DISP=SHR      DND,FCD,ISAM,FVD,ISD,
//*                                          OCD,OCE,OCI,PBD
//*                                          SBD,SQD,TAI,TBP,TOC,TPB
//*                                          TPD,TPL,TPQ,TPR,UCB,VTD
//         DD DSN=BERD.D3380.RFUNCT,DISP=SHR      IMB,LED,LOD,TDG,TUEN
//         DD DSN=BERD.D3380.SCHED,DISP=SHR       UBJ : SCHEDULER
