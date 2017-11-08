/*******************************************************************************
 * Copyright (c) 2017 IBM.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    Mike Fulton - initial implementation and documentation
 *******************************************************************************/
#ifndef __JCLSCAN__
	#define __JCLSCAN__ 1
	
	#include "jclargs.h"
	#include "jclmsgs.h"
	#include "jcl2sh.h"
	
	#define JCL_RECLEN (80)
	#define JCL_TXTLEN (71)
	
	#define PREFIX "//"
	#define PREFIX_LEN (sizeof(PREFIX)-1)
	
	#define STRING_CONTINUE_COLUMN (15)
	
	#define NAME_LEN   (8)
	#define NEWLINE    '\n'
	#define SLASH      '/'
	#define ASTERISK   '*'
	#define BLANK      ' '
	#define EQUALS     '='
	#define ATSIGN     0x7C
	#define DOLLARSIGN 0x5B
	#define POUNDSIGN  0x7B
	#define QUOTE      '\''
	#define COMMA      ','
	#define LPAREN     '('
	#define RPAREN     ')'
	
	#define DEFAULT_DELIM "/*"

	#define JCLCMD_KEYWORD   "JCLCMD"	
	#define NULL_KEYWORD     "NULL"	
	
	#define COMMAND_KEYWORD  "COMMAND"
	#define CNTL_KEYWORD     "CNTL"
	#define DD_KEYWORD       "DD"
	#define ENDCNTL_KEYWORD  "ENDCNTL"
	#define EXEC_KEYWORD     "EXEC"
	#define IF_KEYWORD       "IF"
	#define THEN_KEYWORD     "THEN"	
	#define ELSE_KEYWORD     "ELSE"
	#define ENDIF_KEYWORD    "ENDIF"
	#define INCLUDE_KEYWORD  "INCLUDE"
	#define JCLLIB_KEYWORD   "JCLLIB"
	#define JOB_KEYWORD      "JOB"
	#define OUTPUT_KEYWORD   "OUTPUT"
	#define PEND_KEYWORD     "PEND"
	#define PROC_KEYWORD     "PROC"
	#define SET_KEYWORD      "SET"
	#define XMIT_KEYWORD     "XMIT"
	#define PRINTDEV_KEYWORD "PRINTDEV"
	
	#define COMMENT_KEYWORD "//*"
	#define JES3_KEYWORD    "JES3"
	#define JES2_KEYWORD    "JES2"
	#define GENSYSIN_KEYWORD "GEN-SYSIN"	
	
	#define DATA_KEYWORD    "DATA"
	#define DELIM_KEYWORD   "DLM"
	
	#define JCLCMD_KEYLEN   (sizeof(JCLCMD_KEYWORD)-1)	
	#define NULL_KEYLEN     (sizeof(NULL_KEYWORD)-1)	
	
	#define COMMAND_KEYLEN  (sizeof(COMMAND_KEYWORD)-1)
	#define CNTL_KEYLEN     (sizeof(CNTL_KEYWORD)-1)
	#define DD_KEYLEN       (sizeof(DD_KEYWORD)-1)
	#define ENDCNTL_KEYLEN  (sizeof(ENDCNTL_KEYWORD)-1)
	#define EXEC_KEYLEN     (sizeof(EXEC_KEYWORD)-1)
	#define IF_KEYLEN       (sizeof(IF_KEYWORD)-1)
	#define THEN_KEYLEN     (sizeof(THEN_KEYWORD)-1)	
	#define ELSE_KEYLEN     (sizeof(ELSE_KEYWORD)-1)
	#define ENDIF_KEYLEN    (sizeof(ENDIF_KEYWORD)-1)
	#define INCLUDE_KEYLEN  (sizeof(INCLUDE_KEYWORD)-1)
	#define JCLLIB_KEYLEN   (sizeof(JCLLIB_KEYWORD)-1)
	#define JOB_KEYLEN      (sizeof(JOB_KEYWORD)-1)
	#define OUTPUT_KEYLEN   (sizeof(OUTPUT_KEYWORD)-1)
	#define PEND_KEYLEN     (sizeof(PEND_KEYWORD)-1)
	#define PROC_KEYLEN     (sizeof(PROC_KEYWORD)-1)
	#define SET_KEYLEN      (sizeof(SET_KEYWORD)-1)
	#define XMIT_KEYLEN     (sizeof(XMIT_KEYWORD)-1)	
	#define PRINTDEV_KEYLEN (sizeof(PRINTDEV_KEYWORD)-1)		
	
	#define COMMENT_KEYLEN  (sizeof(COMMENT_KEYWORD)-1)		
	#define JES3_KEYLEN     (sizeof(JES3_KEYWORD)-1)		
	#define JES2_KEYLEN     (sizeof(JES2_KEYWORD)-1)	
	#define GENSYSIN_KEYLEN (sizeof(GENSYSIN_KEYWORD)-1)		
	
	#define DATA_KEYLEN     (sizeof(DATA_KEYWORD)-1)
	
	#define GENERATED_SYSIN_DD "//SYSIN DD *"
	
	typedef struct {
		size_t len;
		char* txt;
	} VarStr_T;
	
	#define JES3CMD_PREFIX        "*"
	#define JES3DATASET_PREFIX    "DATASET"
	#define JES3ENDDATASET_PREFIX "ENDDATASET"
	#define JES3ENDPROCESS_PREFIX "ENDPROCESS"
	#define JES3FORMAT_PREFIX     "FORMAT"
	#define JES3MAIN_PREFIX       "MAIN"
	#define JES3NET_PREFIX        "NET"
	#define JES3NETACCT_PREFIX    "NETACCT"
	#define JES3OPERATOR_PREFIX   "OPERATOR"
	#define JES3PAUSE_PREFIX      "PAUSE"
	#define JES3PROCESS_PREFIX    "PROCESS"
	#define JES3ROUTE_PREFIX      "ROUTE"
	#define JES3SIGNOFF_PREFIX    "SIGNOFF"
	#define JES3SIGNON_PREFIX     "SIGNON"	
	
	#define JES3DATASET_LEN       (sizeof(JES3DATASET_PREFIX)-1)
	#define JES3ENDDATASET_LEN    (sizeof(JES3ENDDATASET_PREFIX)-1)	
	
	typedef enum {
		InvalidRecordUnknownType=1,
		InvalidRecordSlashSlashUnk=2,
		InvalidRecordSlashUnk=3,
		InvalidRecordUnk=4,
		InvalidRecordContinuedComment=5,
		InvalidRecordContinuedStringNoSlash=6,
		InvalidRecordContinuedCommentTooFewBlanks=7,
		InvalidRecordContinuedConditional=8,
		InvalidRecordContinuedParameter=9
		
	} InvalidRecordType_T;
	
	/* JCL Reference: Chapter 3. Format of Statements */
	typedef enum {
		JCLNotContinued=0,
		JCLContinueParameter=1,
		JCLContinueString=2,
		JCLContinueComment=3,
		JCLContinueJES2ControlStatement=4,
		JCLContinueJES3DatasetControlStatement=5,
		JCLInlineText=6,
		JCLContinueConditional=7,
	} JCLScanState_T;
	
	typedef enum {
		NoDataset=0,
		OutstreamDataset=1,
		InstreamDatasetStar=2,
		InstreamDatasetData=3
	} DatasetType_T;
	
	typedef enum {
		JCLParmInKeyword=1,
		JCLParmInValue=2,
		JCLParmInQuote=3,
		JCLParmInParen=4
	} JCLParmContext_T;
		
	struct JCLLine;
	typedef struct JCLLine {
		struct JCLLine* next;
		char text[JCL_RECLEN+1];
	} JCLLine_T;
	typedef struct {
		JCLLine_T* head;
		JCLLine_T* tail;
		size_t curLine;
	} JCLLines_T;
	
	struct KeyValuePair;
	typedef struct KeyValuePair {
		struct KeyValuePair* next;
		VarStr_T key;
		VarStr_T val;
	} KeyValuePair_T;
	
	struct JCLStmt;
	typedef struct JCLStmt {
		struct JCLStmt* next;
		const char* type;
		char* scannedStatement;	
		size_t parmListStart;		
		JCLLine_T* firstJCL;
		size_t count;	
		KeyValuePair_T* kvphead;
		KeyValuePair_T* kvptail;		
	} JCLStmt_T;
	
	typedef struct {
		JCLStmt_T* head;
		JCLStmt_T* tail;
		size_t curStmt;
		JCLScanState_T state;
		DatasetType_T datasetType;
		char delimiter[3];
	} JCLStmts_T;
	
	typedef struct JCL {
		FILE* infp;
		JCLLines_T* lines;
		JCLStmts_T* stmts;
	} JCL_T;	
	
	typedef JCLScanMsg_T (ScanFn_T)(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column);	
	typedef struct {
		ScanFn_T* scan;
	} Scanner_T;

	
	JCLScanMsg_T scanJCL(OptInfo_T* optInfo, ProgInfo_T* progInfo); 
	JCLScanMsg_T establishInput(OptInfo_T* optInfo, ProgInfo_T* progInfo);
#endif