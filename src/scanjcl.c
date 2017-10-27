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
#define _ALL_SOURCE_NO_THREADS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "scanjcl.h"


static int isNational(char c) {
	return (c == ATSIGN || c == DOLLARSIGN || c == POUNDSIGN);
}

static int isValidName(const const char* buffer, size_t* nameLen) {
	int i;
	*nameLen = 0; 
	if (!isupper(buffer[0]) && !isNational(buffer[0])) {
		return 0;
	}
	for (i=1; i<NAME_LEN; ++i) {
		*nameLen = i;		
		if (buffer[i] == BLANK) {
			return 1;
		}
		if (!isupper(buffer[i]) && !isdigit(buffer[i]) && !isNational(buffer[i])) {
			return 0;
		}	
	}
	*nameLen = i;		
	return (buffer[NAME_LEN] == BLANK);
}

static int wordcmp(const char* buffer, const char* word) {
	int i=0;
	while (buffer[i] == word[i]) {
		++i;
	}
	if (buffer[i] == BLANK) {
		return 0;
	} else {
		return buffer[i] - word[i];
	}
}
	
static JCLScanMsg_T scanRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo, char* buffer, JCLScanInfo_T* scanInfo) {
	int i, rc;
	JCLScanMsg_T scanRC = NoError;
	
	buffer[JCL_RECLEN] = '\0';
	for (i=0; i<=JCL_RECLEN; ++i) {
		rc = getc(progInfo->jcl->infp);
		if (rc == NEWLINE) {
			break;		
		}
		if (rc < 0) {
			scanRC = InputEOF; break;
		}
		if (i != JCL_RECLEN) {
			buffer[i] = (char) rc;
		}
	}

	if (i < JCL_RECLEN) {
		memset(&buffer[i], ' ', JCL_RECLEN-i);
	} else {
		if (rc != NEWLINE) { 
			if ((*scanInfo == InfoNone) || optInfo->verbose) {
				*scanInfo = InputRecordTruncated;
				printInfo(*scanInfo, progInfo->jcl->lines->curLine);
			}
			while (1) {
				rc = getc(progInfo->jcl->infp);
				if (rc == NEWLINE) {
					break;
				}
				if (rc < 0) { scanRC = InputEOF; break; }
			}
		}
	}
	
	/*
	 * Re-process EOF if it isn't the first character of the record
	 */
	if (scanRC == InputEOF && i != 0) {
		scanRC = NoError;
	}
	return scanRC;
}

static JCLScanMsg_T scanParameters(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i = column;
	int inString = (progInfo->jcl->stmts->state == JCLContinueString);
	int inComment = (progInfo->jcl->stmts->state == JCLContinueComment);
	int inParameter = 0;
	
	const char* text = progInfo->jcl->lines->tail->text;
	
	if (!inComment) {
		while (text[i] == BLANK) {
			++i;
		}		
		while (i < JCL_TXTLEN) {
			if (text[i] == QUOTE) {
				inString ^= 1;	
			} else if (!inString && (text[i] == BLANK)) {
				if (text[i-1] == COMMA) {		
					inParameter = 1;
				}
				break;
			}
			++i;
		}
	}

    /*
     * msf - if multiple conditions hold, this is an error (e.g. in a string -and- in a comment)
     * so this code needs to be enhanced
     */
	if (inString) {
		if (optInfo->verbose) { printInfo(InfoInString); }		
		progInfo->jcl->stmts->state = JCLContinueString;
	} else if (inParameter) {
		if (optInfo->verbose) { printInfo(InfoInParameter); }		
		progInfo->jcl->stmts->state = JCLContinueParameter;	
	} else if (text[JCL_TXTLEN] != BLANK) {
		if (optInfo->verbose) { printInfo(InfoInComment); }		
		progInfo->jcl->stmts->state = JCLContinueComment;	
	} else {
		progInfo->jcl->stmts->state = JCLNotContinued;			
	}
	
	return NoError;
}

JCLScanMsg_T addStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* stmtType) {
	JCLStmt_T* stmt = malloc(sizeof(JCLStmt_T));
	if (!stmt) {
		return InternalOutOfMemory;
	}
	if (!progInfo->jcl->stmts->head) {
		progInfo->jcl->stmts->head = progInfo->jcl->stmts->tail = stmt;
	} else {
		progInfo->jcl->stmts->tail->next = stmt;
	}
	stmt->next = NULL;
	stmt->type = stmtType;
	stmt->count = 1;
	stmt->firstJCL = progInfo->jcl->lines->tail;
	
	progInfo->jcl->stmts->tail = stmt;
		
	return NoError;
}

JCLScanMsg_T addToStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->jcl->stmts->tail->count++;
	return NoError;
}

void printStatements(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	size_t stmtNum = 0;
	JCLStmts_T* stmts = progInfo->jcl->stmts;
	JCLStmt_T* cur = stmts->head;
	
	while (cur != NULL) {
		printInfo(InfoStmtDump, ++stmtNum, cur->type, cur->count, cur->firstJCL);
		cur = cur->next;
	}
}

static JCLScanMsg_T processInvalidRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	printError(InvalidRecordEncountered, progInfo->jcl->lines->curLine);
	return InputEOF;
}

static JCLScanMsg_T processDelimeterOrControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanSetStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanIfStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanElseStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanEndifStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}


static JCLScanMsg_T scanProcStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanPendStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanIncludeStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanJCLLibStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanCommandStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanCntlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanEndCntlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanOutputStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanXMitStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T scanDDStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessDDStatement); }
	rc = addStatement(optInfo, progInfo, DD_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanExecStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessExecStatement); }
	rc = addStatement(optInfo, progInfo, EXEC_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}	
	return rc;
}

static JCLScanMsg_T scanJobStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessJobStatement); }
	rc = addStatement(optInfo, progInfo, JOB_KEYWORD);	
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}	
	return rc;
}

static JCLScanMsg_T scanCommentStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;	
	if (optInfo->verbose) { printInfo(InfoProcessCommentStatement); }
	rc = addStatement(optInfo, progInfo, COMMENT_KEYWORD);	
	return rc;
}

static JCLScanMsg_T processStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column, ScanFn_T* scanner) {
	return scanner(optInfo, progInfo, column);
}

static JCLScanMsg_T processCommentOrControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return processStatement(optInfo, progInfo, column, scanCommentStatement); /* msf - need to add support for control statement */
}

static JCLScanMsg_T processJCLStatementFirstLine(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i=column;
	const char* keyword;
	const char* text = progInfo->jcl->lines->tail->text;
	
	while (text[i] == BLANK) {
		++i;
	}
	keyword = &text[i];
	
	if (!wordcmp(keyword, DD_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+DD_KEYLEN, scanDDStatement); 
	} else if (!wordcmp(keyword, EXEC_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+EXEC_KEYLEN, scanExecStatement); 
	} else if (!wordcmp(keyword, SET_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+SET_KEYLEN, scanSetStatement); 
	} else if (!wordcmp(keyword, IF_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+IF_KEYLEN, scanIfStatement); 
	} else if (!wordcmp(keyword, ELSE_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+ELSE_KEYLEN, scanElseStatement); 
	} else if (!wordcmp(keyword, ENDIF_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+ENDIF_KEYLEN, scanEndifStatement); 
	} else if (!wordcmp(keyword, JOB_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+JOB_KEYLEN, scanJobStatement); 
	} else if (!wordcmp(keyword, PROC_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+PROC_KEYLEN, scanProcStatement); 
	} else if (!wordcmp(keyword, PEND_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+PEND_KEYLEN, scanPendStatement); 
	} else if (!wordcmp(keyword, INCLUDE_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+INCLUDE_KEYLEN, scanIncludeStatement); 
	} else if (!wordcmp(keyword, JCLLIB_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+JCLLIB_KEYLEN, scanJCLLibStatement); 
	} else if (!wordcmp(keyword, COMMAND_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+COMMAND_KEYLEN, scanCommandStatement); 
	} else if (!wordcmp(keyword, CNTL_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+CNTL_KEYLEN, scanCntlStatement); 
	} else if (!wordcmp(keyword, ENDCNTL_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+ENDCNTL_KEYLEN, scanEndCntlStatement); 
	} else if (!wordcmp(keyword, OUTPUT_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+OUTPUT_KEYLEN, scanOutputStatement); 
	} else if (!wordcmp(keyword, XMIT_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+XMIT_KEYLEN, scanXMitStatement); 
	} 
	return processInvalidRecord(optInfo, progInfo);
}

static JCLScanMsg_T processImplicitJCLCommand(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}

static JCLScanMsg_T processJCLRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	size_t i;
	size_t nameLen;
	const char* text = progInfo->jcl->lines->tail->text;	
	if (text[0] == SLASH) {
		if (text[1] == SLASH) {
			if (text[2] == ASTERISK) {
				return processCommentOrControlStatement(optInfo, progInfo, PREFIX_LEN+1);
			} else if (isValidName(&text[2], &nameLen)) {
				return processJCLStatementFirstLine(optInfo, progInfo, PREFIX_LEN+nameLen);
			} else if (text[2] == BLANK) {
				return processImplicitJCLCommand(optInfo, progInfo, PREFIX_LEN+1);
			} else {
				return processInvalidRecord(optInfo, progInfo);
			}
		} else if (text[1] == ASTERISK) {
			return processDelimeterOrControlStatement(optInfo, progInfo, PREFIX_LEN);
		} else {
			return processInvalidRecord(optInfo, progInfo);
		}
	} else {
		return processInvalidRecord(optInfo, progInfo);	
	}
}

static JCLScanMsg_T processInlineRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	return NoError;
}

static JCLScanMsg_T processContinuedComment(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	size_t i;
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return processInvalidRecord(optInfo, progInfo);
	}

	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, PREFIX_LEN+1); 
	}	
	return rc;
}

static JCLScanMsg_T processJES2ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	return NoError;
}

static JCLScanMsg_T processJES3ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	return NoError;
}

static JCLScanMsg_T processJCLContinueString(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	size_t i;
	
	if (text[0] != SLASH || text[1] != SLASH) {
		return processInvalidRecord(optInfo, progInfo);
	}
	for (i=PREFIX_LEN; i<STRING_CONTINUE_COLUMN; ++i) {
		if (text[i] != BLANK) {
			return processInvalidRecord(optInfo, progInfo);
		}
	}
	
	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, PREFIX_LEN); 
	}	
	return rc;
}

static JCLScanMsg_T processJCLContinueParameter(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return processInvalidRecord(optInfo, progInfo);
	}
	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, PREFIX_LEN+1);
	}
	return rc;
}

static JCLScanMsg_T addRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text) {
	JCLLine_T* line = malloc(sizeof(JCLLine_T));
	if (!line) {
		return InternalOutOfMemory;
	}
	if (!progInfo->jcl->lines->head) {
		progInfo->jcl->lines->head = progInfo->jcl->lines->tail = line;
	} else {
		progInfo->jcl->lines->tail->next = line;
	}
	line->next = NULL;
	
	memcpy(line->text, text, JCL_RECLEN+1);
	progInfo->jcl->lines->tail = line;
	
	return NoError;
}
	
static JCLScanMsg_T processRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* buffer) {
	int i;
	JCLScanMsg_T rc;
	JCLScanState_T current = progInfo->jcl->stmts->state;
	
	if (optInfo->verbose) {
		printInfo(InfoJCLText, progInfo->jcl->lines->curLine, buffer);
	}
	rc = addRecord(optInfo, progInfo, buffer);
	if (rc != NoError) {
		return rc;
	}
	switch (current) {
		case JCLNotContinued:
			return processJCLRecord(optInfo, progInfo);		
		case JCLInlineText:
			return processInlineRecord(optInfo, progInfo);
		case JCLContinueComment:
			return processContinuedComment(optInfo, progInfo);
		case JCLContinueJES2ControlStatement:
			return processJES2ControlStatement(optInfo, progInfo);
		case JCLContinueJES3ControlStatement:
			return processJES2ControlStatement(optInfo, progInfo);
		case JCLContinueString:
			return processJCLContinueString(optInfo, progInfo);
		case JCLContinueParameter:
			return processJCLContinueParameter(optInfo, progInfo);
		default:
			return UnreachableCodeError;
	}		
}

JCLScanMsg_T scanJCL(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	char buffer[JCL_RECLEN+1];
	JCLScanMsg_T scanRC;
	JCLScanInfo_T scanInfo = InfoNone;
	size_t lineNum = 1;
	
	buffer[JCL_RECLEN] = '\0';

	do {
		progInfo->jcl->lines->curLine = lineNum;
		scanRC = scanRecord(optInfo, progInfo, buffer, &scanInfo);
		if (scanRC == NoError) {
			scanRC = processRecord(optInfo, progInfo, buffer);
		}
		++lineNum;
	} while (scanRC == NoError);
	
	fclose(progInfo->jcl->infp);
	progInfo->jcl->infp = NULL;
	
	if (optInfo->debug) {
		printStatements(optInfo, progInfo);
	}
	return NoError;
}

JCLScanMsg_T establishInput(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->jcl = calloc(1, sizeof(JCL_T));
	if (!progInfo->jcl) {
		return InternalOutOfMemory;
	}
	progInfo->jcl->stmts = calloc(1, sizeof(JCLStmts_T));
	if (!progInfo->jcl->stmts) {
		return InternalOutOfMemory;
	}	
	progInfo->jcl->lines = calloc(1, sizeof(JCLLines_T));
	if (!progInfo->jcl->lines) {
		return InternalOutOfMemory;
	}		
	
	if (optInfo->inputFile) {
		progInfo->jcl->infp = fopen(optInfo->inputFile, "rb");
		if (!progInfo->jcl->infp) {
			printError(UnableToOpenInput, optInfo->inputFile);
			return UnableToOpenInput;
		}
	} else {
		progInfo->jcl->infp = stdin;
		if (!progInfo->jcl->infp) {
			printError(UnableToReopenInput);
			return UnableToReopenInput;
		}	
	}

	return NoError;
}