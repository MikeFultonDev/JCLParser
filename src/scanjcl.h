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
	#define PREFIX_LEN (2) /* length of '//' */
	
	#define STRING_CONTINUE_COLUMN (16)
	
	#define NAME_LEN   (8)
	#define NEWLINE    '\n'
	#define SLASH      '/'
	#define ASTERISK   '*'
	#define BLANK      ' '
	#define ATSIGN     0x7C
	#define DOLLARSIGN 0x5B
	#define POUNDSIGN  0x7B
	#define QUOTE      '\''
	#define COMMA      ','
	
	#define COMMAND_KEYWORD "COMMAND"
	#define CNTL_KEYWORD    "CNTL"
	#define DD_KEYWORD      "DD"
	#define ENDCNTL_KEYWORD "ENDCNTL"
	#define EXEC_KEYWORD    "EXEC"
	#define IF_KEYWORD      "IF"
	#define ELSE_KEYWORD    "ELSE"
	#define ENDIF_KEYWORD   "ENDIF"
	#define INCLUDE_KEYWORD "INCLUDE"
	#define JCLLIB_KEYWORD  "JCLLIB"
	#define JOB_KEYWORD     "JOB"
	#define OUTPUT_KEYWORD  "OUTPUT"
	#define PEND_KEYWORD    "PEND"
	#define PROC_KEYWORD    "PROC"
	#define SET_KEYWORD     "SET"
	#define XMIT_KEYWORD    "XMIT"
	
	#define COMMENT_KEYWORD "//*"
	
	#define COMMAND_KEYLEN  (sizeof(COMMAND_KEYWORD)-1)
	#define CNTL_KEYLEN     (sizeof(CNTL_KEYWORD)-1)
	#define DD_KEYLEN       (sizeof(DD_KEYWORD)-1)
	#define ENDCNTL_KEYLEN  (sizeof(ENDCNTL_KEYWORD)-1)
	#define EXEC_KEYLEN     (sizeof(EXEC_KEYWORD)-1)
	#define IF_KEYLEN       (sizeof(IF_KEYWORD)-1)
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
	
	#define COMMENT_KEYLEN  (sizeof(COMMENT_KEYWORD)-1)		
	
	/* JCL Reference: Chapter 3. Format of Statements */
	typedef enum {
		JCLNotContinued=0,
		JCLContinueParameter=1,
		JCLContinueString=2,
		JCLContinueComment=3,
		JCLContinueJES2ControlStatement=4,
		JCLContinueJES3ControlStatement=5,
		JCLInlineText=6,
	} JCLScanState_T;
		
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
	
	struct JCLStmt;
	typedef struct JCLStmt {
		struct JCLStmt* next;
		const char* type;
		size_t count;
		JCLLine_T* firstJCL;
	} JCLStmt_T;
	typedef struct {
		JCLStmt_T* head;
		JCLStmt_T* tail;
		JCLScanState_T state;
		char delimiter[2];
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