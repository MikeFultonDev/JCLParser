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

#ifndef __JCLMSG__
	#define __JCLMSG__ 1
	
	/*
	 * The JCLScanMsg_T enum and JCLScanMessage must be kept in sync.
	 */
	typedef enum {
		UnreachableCodeError=-2,
		InputEOF=-1,
		NoError=0,
		TooFewArgsSingular=1,
		TooFewArgsPlural=2,
		TooManyArgs=3,
		UnrecognizedOption=4,
		InternalOutOfMemory=5,
		UnableToEstablishEnvironment=6,
		ErrorEstablishingEnvironment=7,
		IssueHelp=8,
		NoArgSpecified=9,
		UnableToOpenInput=10,
		UnableToOpenOutput=11,
		UnableToReopenInput=12,
		UnableToReopenOutput=13,
		InvalidRecordEncountered=14,
		ErrorScanningJCL=15,
	} JCLScanMsg_T;
	
	/*
	 * The JCLScanInfo_T enum and JCLScanInfo must be kept in sync.
	 */
	typedef enum {
		InfoNone=0,

		InfoSyntax01=1,
		InfoSyntax02=2,
		InfoSyntax03=3,
		InfoSyntax04=4,
		InfoSyntax05=5,
		InfoSyntax06=6,
		InfoSyntax07=7,
		InfoSyntax08=8,
		InfoSyntax09=9,
		InfoSyntax10=10,
		InputRecordTruncated=11,
		InfoJCLText=12,
		InfoProcessDDStatement=13,
		InfoProcessExecStatement=14,	
		InfoProcessJobStatement=15,		
		InfoProcessCommentStatement=16,	
		InfoProcessIfStatement=17,	
		InfoProcessElseStatement=18,	
		InfoProcessEndifStatement=19,
		InfoProcessJCLCommand=20,
		InfoProcessCommandStatement=21,
		InfoProcessCntlStatement=22,
		InfoProcessEndCntlStatement=23,		
		InfoProcessPrintDevStatement=24,
		InfoProcessIncludeStatement=25,	
		InfoProcessJCLLibStatement=26,
		InfoProcessNullStatement=27,
		InfoProcessOutputStatement=28,	
		InfoProcessProcStatement=29,	
		InfoProcessPendStatement=30,	
		InfoProcessSetStatement=31,	
		InfoProcessXmitStatement=32,	
		InfoProcessJES3ControlStatement=33,			
		InfoProcessJES2ControlStatement=34,	
		InfoProcessGenerateSYSINStatement=35,
		InfoInParameter=36,	
		InfoInString=37,						
		InfoInComment=38,
		InfoInConditional=39,
		InfoInJES3DatasetControlStatement=40,
		InfoProcessInstreamDataset=41,	
		InfoScannedStatement=42,		
		InfoScannedStatementNoName=43,
		InfoScannedComment=44,
		InfoKeyOnly=45,	
		InfoKeyValuePair=46,
		InfoStmtDump=47,
		InfoSeparator=48,
		InfoNewLine=49,
		InfoComment=50,
		InfoStmtPrefix=51,
		InfoScannedJES2ControlStatement=52,
		InfoScannedJES3ControlStatement=53,
		InfoScannedIFStatement=54,
		InfoScannedDelimiter=55,
	} JCLScanInfo_T;
	
#ifdef DEBUG
	#define FORCE(FAIL_Condition) \
		(getenv(#FAIL_Condition))
#else
	#define FORCE(FAIL_Condition) (0)
#endif
	void printHelp(const char* pgmName);

	void printError(JCLScanMsg_T pfm, ...);
	void printInfo(JCLScanInfo_T pim, ...);
#endif	