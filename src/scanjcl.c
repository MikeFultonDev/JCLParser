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
	
const static VarStr_T JES3ControlStatement[] = {
	{sizeof(JES3CMD_PREFIX)-1, JES3CMD_PREFIX },
	{JES3DATASET_LEN, JES3DATASET_PREFIX },
	{JES3ENDDATASET_LEN, JES3ENDDATASET_PREFIX },		
	{sizeof(JES3ENDPROCESS_PREFIX)-1, JES3ENDPROCESS_PREFIX },		
	{sizeof(JES3FORMAT_PREFIX)-1, JES3FORMAT_PREFIX },
	{sizeof(JES3MAIN_PREFIX)-1, JES3MAIN_PREFIX },
	{sizeof(JES3NET_PREFIX)-1, JES3NET_PREFIX },		
	{sizeof(JES3NETACCT_PREFIX)-1, JES3NETACCT_PREFIX },	
	{sizeof(JES3OPERATOR_PREFIX)-1, JES3OPERATOR_PREFIX },
	{sizeof(JES3PAUSE_PREFIX)-1, JES3PAUSE_PREFIX },
	{sizeof(JES3PROCESS_PREFIX)-1, JES3PROCESS_PREFIX },		
	{sizeof(JES3ROUTE_PREFIX)-1, JES3ROUTE_PREFIX },		
	{sizeof(JES3SIGNOFF_PREFIX)-1, JES3SIGNOFF_PREFIX },
	{sizeof(JES3SIGNON_PREFIX)-1, JES3SIGNON_PREFIX }, 
	{0, NULL}
};

static size_t isNational(char c) {
	return (c == ATSIGN || c == DOLLARSIGN || c == POUNDSIGN);
}

static size_t isValidName(const const char* buffer, size_t* nameLen) {
	size_t i;
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
	size_t i=0;
	while (buffer[i] == word[i]) {
		++i;
	}
	if (buffer[i] == BLANK) {
		return 0;
	} else {
		return buffer[i] - word[i];
	}
}
static int wordlcmp(const char* buffer, const char* word, size_t wordlen) {
	size_t i;
	for (i=0; i<wordlen; ++i) {
		if (buffer[i] != word[i]) {
			return 1;
		}
	}
	return 0;
}

static int blankRecord(ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;
	size_t i;
	for (i=PREFIX_LEN; i<JCL_TXTLEN; ++i) {
		if (text[i] != BLANK) { return 0; }
	}
	return 1;
}

static char* copyNormalizedText(char* dst, const char* src, size_t srcLen) {
	size_t i;
	size_t d=0;
	size_t start, end;
	if (src[0] == QUOTE) {
		start = 1;
		if (src[srcLen-1] == QUOTE) {
			end = srcLen-1;
		} else { /* error condition - invalid string */
			end = srcLen;
		}
	} else {
		start = 0;
		end = srcLen;
	}
	for (i=start; i<end; ++i) {
		if (src[i] == QUOTE && src[i+1] == QUOTE) {
			i+=2;
			dst[d++] = QUOTE;
		} else {
			dst[d++] = src[i];
		}
	}
	return dst;
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

static JCLScanMsg_T scanConditional(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i = column;
	int conditionalComplete = 0;
	
	const char* text = progInfo->jcl->lines->tail->text;
	while (i < JCL_TXTLEN) {
		if (text[i-1] == BLANK && i < (JCL_TXTLEN - THEN_KEYLEN)) {
			if (!memcmp(&text[i], THEN_KEYWORD, THEN_KEYLEN) && (text[i+THEN_KEYLEN] == BLANK)) {
				conditionalComplete = 1;
				i += THEN_KEYLEN;
				break;
			}
		}
		++i;
	}
	
	if (conditionalComplete) {
		progInfo->jcl->stmts->state = JCLNotContinued;			
	} else {
		if (optInfo->verbose) { printInfo(InfoInConditional); }	
		progInfo->jcl->stmts->state = JCLContinueConditional;
	}		
	return NoError;
}

static JCLScanMsg_T addSubParm(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* parm, size_t parmLen, const char* value, size_t valLen) {		
	KeyValuePair_T* kvp = malloc(sizeof(KeyValuePair_T));
	if (!kvp) {
		return InternalOutOfMemory;
	}
	kvp->next = NULL;
	kvp->key.len = parmLen;
	kvp->key.txt = malloc(parmLen+1);
	if (!kvp->key.txt) {
		return InternalOutOfMemory;
	}	
	copyNormalizedText(kvp->key.txt, parm, parmLen);
	
	if (value == NULL) {
		kvp->val.len = 0;
		kvp->val.txt = NULL;
	} else {
		kvp->key.txt[parmLen] = '\0';
		kvp->val.len = valLen;
		kvp->val.txt = malloc(valLen+1);
		copyNormalizedText(kvp->val.txt, value, valLen);
		kvp->val.txt[valLen] = '\0';
		if (!kvp->val.txt) {
			return InternalOutOfMemory;
		}		
	}
	
	if (!progInfo->jcl->stmts->tail->kvphead) {
		progInfo->jcl->stmts->tail->kvphead = progInfo->jcl->stmts->tail->kvptail = kvp;
	} else {
		progInfo->jcl->stmts->tail->kvptail->next = kvp;
	}

	progInfo->jcl->stmts->tail->kvptail = kvp;		
	return NoError;
}

static JCLScanMsg_T scanSubParameters(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	size_t i;
	size_t start = progInfo->jcl->stmts->tail->parmListStart;
	const char* text = progInfo->jcl->stmts->tail->scannedStatement;	
	size_t end = strlen(text);

	JCLParmContext_T context = JCLParmInKeyword;
	int curParm = start;
	int curValue;
	int parenNest = 0;
	JCLParmContext_T prevContext = context;
	
	for (i=start; i<end; ++i) {
		switch (context) {
			case JCLParmInKeyword:
				if (text[i] == EQUALS) {
					context = JCLParmInValue;
					curValue=i+1;
				} else if (text[i] == COMMA) {
					addSubParm(optInfo, progInfo, &text[curParm], i-curParm, NULL, 0);
					curParm = i+1;					
				}
				break;
			case JCLParmInValue:
				if (text[i] == COMMA) {
					const char* value;
					context = JCLParmInKeyword;
					if (curValue == i) {
						value = "";
					} else {
						value = &text[curValue];
					}
					addSubParm(optInfo, progInfo, &text[curParm], curValue-curParm-1, value, i-curValue);
					curParm = i+1;
				} else if (text[i] == QUOTE) {
					context = JCLParmInQuote;
					prevContext = JCLParmInValue;
				} else if (text[i] == LPAREN) {
					context = JCLParmInParen;
				}
				break;
			case JCLParmInQuote:
				if (text[i] == QUOTE) {
					context = prevContext;
				}
				break;
			case JCLParmInParen:
				if (text[i] == QUOTE) {
					context = JCLParmInQuote;
					prevContext = JCLParmInParen;
				} else if (text[i] == RPAREN) {
					--parenNest;
					if (parenNest == 0) {
						context = JCLParmInKeyword;
					}
				}
				break;
		}
	}		
	
	if (context == JCLParmInKeyword) {
		addSubParm(optInfo, progInfo, &text[curParm], i-curParm, NULL, 0);
	} else {
		addSubParm(optInfo, progInfo, &text[curParm], curValue-curParm-1, &text[curValue], end-curValue);
	}

	return NoError;
}

static const char* getSubParameter(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* subParmName) {
	KeyValuePair_T* kvp = progInfo->jcl->stmts->tail->kvptail;
	int parmNameLen = strlen(subParmName);
	while (kvp != NULL) {
		if (parmNameLen == kvp->key.len && !memcmp(subParmName, kvp->key.txt, parmNameLen)) {
			return kvp->val.txt;
		}
		kvp = kvp->next;
	}
	return NULL;
}

JCLScanMsg_T createScannedStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text, size_t start, size_t end) {
	progInfo->jcl->stmts->tail->parmListStart = start;
	
	progInfo->jcl->stmts->tail->scannedStatement = malloc(end+1);
	if (!progInfo->jcl->stmts->tail->scannedStatement) {
		return InternalOutOfMemory;
	}
	memcpy(progInfo->jcl->stmts->tail->scannedStatement, text, end);
	progInfo->jcl->stmts->tail->scannedStatement[end] = '\0';
	return NoError;
}

JCLScanMsg_T addToScannedStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text, size_t start, size_t end) {
	char* curTxt = progInfo->jcl->stmts->tail->scannedStatement;
	size_t curLen = strlen(curTxt);
	size_t newLen = curLen + (end-start+1);
	
	progInfo->jcl->stmts->tail->scannedStatement = malloc(newLen+1);
	if (!progInfo->jcl->stmts->tail->scannedStatement) {
		return InternalOutOfMemory;
	}
	
	memcpy(progInfo->jcl->stmts->tail->scannedStatement, curTxt, curLen);
	memcpy(&progInfo->jcl->stmts->tail->scannedStatement[curLen], &text[start], end-start);
	progInfo->jcl->stmts->tail->scannedStatement[curLen+end-start] = '\0';
	
	free(curTxt);
	
	return NoError;
}

JCLScanMsg_T completeScannedStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->jcl->stmts->curStmt++;
	if (optInfo->verboseStatements) {
		printInfo(InfoScannedStatement, progInfo->jcl->stmts->curStmt, progInfo->jcl->stmts->tail->scannedStatement); 
	}
	return NoError;
}

static JCLScanMsg_T scanParameters(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i = column;
	int inString = (progInfo->jcl->stmts->state == JCLContinueString);
	int inComment = (progInfo->jcl->stmts->state == JCLContinueComment);
	int inParameter = (progInfo->jcl->stmts->state == JCLContinueParameter);
	JCLScanMsg_T rc;
	
	const char* text = progInfo->jcl->lines->tail->text;
	int firstLine = (!inString && !inComment && !inParameter);
	size_t start = i;
	size_t end = JCL_TXTLEN;
	
	inParameter = 0;
	
	if (!inComment) {
		while (text[i] == BLANK) {
			++i;
		}	
		start = i;
		while (i < JCL_TXTLEN) {
			if (text[i] == QUOTE) {
				inString ^= 1;	
			} else if (!inString && (text[i] == BLANK)) {
				if (text[i-1] == COMMA) {		
					inParameter = 1;
				}
				end = i;
				break;
			}
			++i;
		}
		if (firstLine) {
			rc = createScannedStatement(optInfo, progInfo, text, start, end);
		} else {
			rc = addToScannedStatement(optInfo, progInfo, text, start, end);
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
		
		rc = completeScannedStatement(optInfo, progInfo);
		if (rc != NoError) {
			return rc;
		}		
		rc = scanSubParameters(optInfo, progInfo);
		if (rc != NoError) {
			return rc;
		}
		/*
		 * Check if an instream dataset is queued up. If so, we need to determine the delimiter, update it
		 * and set the state to inline record processing
		 */
		if (progInfo->jcl->stmts->datasetType == InstreamDatasetStar || progInfo->jcl->stmts->datasetType == InstreamDatasetData) {
			size_t dlmlen;
			const char* dlm = getSubParameter(optInfo, progInfo, DELIM_KEYWORD);
			if (!dlm) {
				dlm = DEFAULT_DELIM;
			}
			dlmlen = strlen(dlm);
			if (dlmlen > 2) {
				dlmlen = 2;
			}
			if (dlmlen == 0) {
				dlm = DEFAULT_DELIM;
				dlmlen = strlen(DEFAULT_DELIM);
			}
			
			progInfo->jcl->stmts->state = JCLInlineText;
			memcpy(progInfo->jcl->stmts->delimiter, dlm, dlmlen);
			progInfo->jcl->stmts->delimiter[dlmlen] = '\0';
		}
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
	stmt->kvphead = stmt->kvptail = NULL;
	stmt->scannedStatement = NULL;
	stmt->parmListStart = 0;
	
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

static JCLScanMsg_T processInvalidRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo, InvalidRecordType_T invalidType) {
	printError(InvalidRecordEncountered, progInfo->jcl->lines->curLine, invalidType);	
	return InputEOF;
}

static JCLScanMsg_T processDelimeterOrControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	return NoError;
}


static JCLScanMsg_T scanJES3ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	const char* text = progInfo->jcl->lines->tail->text;	
	if (optInfo->verbose) { printInfo(InfoProcessJES3ControlStatement); }
	rc = addStatement(optInfo, progInfo, JES3_KEYWORD);
	if (!wordlcmp(&text[column], JES3DATASET_PREFIX, JES3DATASET_LEN)) {
		if (optInfo->verbose) { printInfo(InfoInJES3DatasetControlStatement); }		
		progInfo->jcl->stmts->state = JCLContinueJES3DatasetControlStatement;
	}
	return rc;
}

static JCLScanMsg_T scanSetStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessSetStatement); }
	rc = addStatement(optInfo, progInfo, SET_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanIfStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessIfStatement); }
	rc = addStatement(optInfo, progInfo, IF_KEYWORD);
	if (rc == NoError) {
		rc = scanConditional(optInfo, progInfo, column);
	}	
	return rc;
}

static JCLScanMsg_T scanElseStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessElseStatement); }
	rc = addStatement(optInfo, progInfo, ELSE_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanEndifStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessEndifStatement); }
	rc = addStatement(optInfo, progInfo, ENDIF_KEYWORD);
	return rc;
}


static JCLScanMsg_T scanProcStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessProcStatement); }
	rc = addStatement(optInfo, progInfo, PROC_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanPendStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessPendStatement); }
	rc = addStatement(optInfo, progInfo, PEND_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanIncludeStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessIncludeStatement); }
	rc = addStatement(optInfo, progInfo, INCLUDE_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanJCLLibStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessJCLLibStatement); }
	rc = addStatement(optInfo, progInfo, JCLLIB_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanCommandStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessCommandStatement); }
	rc = addStatement(optInfo, progInfo, COMMAND_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanCntlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessCntlStatement); }
	rc = addStatement(optInfo, progInfo, CNTL_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanEndCntlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessEndCntlStatement); }
	rc = addStatement(optInfo, progInfo, ENDCNTL_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanPrintDevStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessPrintDevStatement); }
	rc = addStatement(optInfo, progInfo, PRINTDEV_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanOutputStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessOutputStatement); }
	rc = addStatement(optInfo, progInfo, OUTPUT_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanXMitStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessXmitStatement); }
	rc = addStatement(optInfo, progInfo, XMIT_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

/*
 * We can determine the dataset type because '*' and 'DATA' are positional
 * but we can not determine the delimiter easily because it (theoretically) could
 * be on a later record, so we will delay the delimiter scan until after the full
 * statement is read in
 */
static DatasetType_T datasetType(const char* text) {
	size_t i=0;
	while (text[i] == BLANK) {
		++i;
	}
	if (text[i] == '*' && (text[i+1] == BLANK || text[i+1] == COMMA)) {
		return InstreamDatasetStar;
	} else if (!wordlcmp(&text[i], DATA_KEYWORD, DATA_KEYLEN)) {
		if (text[i+DATA_KEYLEN] == BLANK || text[i+DATA_KEYLEN] == COMMA) {
			return InstreamDatasetData;
		}
	}
	return OutstreamDataset;
}

static JCLScanMsg_T scanDDStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessDDStatement); }
	rc = addStatement(optInfo, progInfo, DD_KEYWORD);

	if (rc == NoError) {
		const char* text = progInfo->jcl->lines->tail->text;
		DatasetType_T dst = datasetType(&text[column]);
		if (dst != OutstreamDataset) {
			if (optInfo->verbose) { printInfo(InfoProcessInstreamDataset); }	
		}
		progInfo->jcl->stmts->datasetType = dst;
	}
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


static JCLScanMsg_T scanGenerateSYSINStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	JCLScanMsg_T rc;	
	if (optInfo->verbose) { printInfo(InfoProcessGenerateSYSINStatement); }
	rc = addStatement(optInfo, progInfo, GENSYSIN_KEYWORD);	
	if (rc != NoError) {
		return rc;
	}
	rc = createScannedStatement(optInfo, progInfo, GENERATED_SYSIN_DD, 0, sizeof(GENERATED_SYSIN_DD)-1);
	if (rc != NoError) {
		return rc;
	}
	rc = completeScannedStatement(optInfo, progInfo);
	if (rc != NoError) {
		return rc;
	}	
	progInfo->jcl->stmts->datasetType = InstreamDatasetStar;
	progInfo->jcl->stmts->state = JCLInlineText;
	strcpy(progInfo->jcl->stmts->delimiter, DEFAULT_DELIM);
	
	return rc;
}

static JCLScanMsg_T scanJCLCommand(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	/*
	 * JCL Command must be one record. scanCommandStatement() gets driven for the 'COMMAND' statement
	 * which can be multiple lines
	 */
	JCLScanMsg_T rc;	
	if (blankRecord(progInfo)) {
		if (optInfo->verbose) { printInfo(InfoProcessNullStatement); }
		rc = addStatement(optInfo, progInfo, NULL_KEYWORD);	
	} else {
		if (optInfo->verbose) { printInfo(InfoProcessJCLCommand); }
		rc = addStatement(optInfo, progInfo, JCLCMD_KEYWORD);	
	}		
	return rc;
}

static JCLScanMsg_T processStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column, ScanFn_T* scanner) {
	return scanner(optInfo, progInfo, column);
}

static int isJES3ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i=0;
	const char* keyword;
	const char* text = &progInfo->jcl->lines->tail->text[column];
	while ((keyword = JES3ControlStatement[i].txt) != NULL) {		
		if (!wordlcmp(text, keyword, JES3ControlStatement[i].len)) {
			return 1;
		}
		++i;
	} 
	return 0;
}

static JCLScanMsg_T processCommentOrControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	if (isJES3ControlStatement(optInfo, progInfo, column)) {
		return processStatement(optInfo, progInfo, column, scanJES3ControlStatement);
	} else {
		return processStatement(optInfo, progInfo, column, scanCommentStatement); 
	}
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
	} else if (!wordcmp(keyword, PRINTDEV_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+PRINTDEV_KEYLEN, scanPrintDevStatement); 
	} 
	return processInvalidRecord(optInfo, progInfo, InvalidRecordUnknownType);
}

static JCLScanMsg_T processRestrictedJCLStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
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
	} else if (!wordcmp(keyword, ENDCNTL_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+ENDCNTL_KEYLEN, scanEndCntlStatement); 
	} else if (!wordcmp(keyword, XMIT_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+XMIT_KEYLEN, scanXMitStatement); 
	} else if (!wordcmp(keyword, OUTPUT_KEYWORD)) {
		return processStatement(optInfo, progInfo, i+OUTPUT_KEYLEN, scanOutputStatement); 
	} else {
		return processStatement(optInfo, progInfo, i, scanJCLCommand);
	}
}

static JCLScanMsg_T processJCLRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	size_t nameLen;
	const char* text = progInfo->jcl->lines->tail->text;
	if (text[0] == SLASH) {
		if (text[1] == SLASH) {
			if (text[2] == ASTERISK) {
				return processCommentOrControlStatement(optInfo, progInfo, PREFIX_LEN+1);
			} else if (isValidName(&text[2], &nameLen)) {
				return processJCLStatementFirstLine(optInfo, progInfo, PREFIX_LEN+nameLen);
			} else if (text[2] == BLANK) {
				return processRestrictedJCLStatement(optInfo, progInfo, PREFIX_LEN+1);
			} else {
				/*
				 * This really is an error... 
				 */
				return processInvalidRecord(optInfo, progInfo, InvalidRecordSlashSlashUnk);
			}
		} else if (text[1] == ASTERISK) {
			return processDelimeterOrControlStatement(optInfo, progInfo, PREFIX_LEN);
		} else {
			return processStatement(optInfo, progInfo, 0, scanGenerateSYSINStatement); 		
		}
	} else {
		return processStatement(optInfo, progInfo, 0, scanGenerateSYSINStatement); 		
	}
}

static JCLScanMsg_T processInlineRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* dlm = progInfo->jcl->stmts->delimiter;
	size_t dlmlen = strlen(dlm);
	DatasetType_T dst = progInfo->jcl->stmts->datasetType;
	const char* record = progInfo->jcl->lines->tail->text;
	int terminateInstream = 0;
	
	if (dst == InstreamDatasetData || dst == InstreamDatasetStar) {
		if (!wordlcmp(record, dlm, dlmlen)) {
			progInfo->jcl->stmts->datasetType = NoDataset;
			progInfo->jcl->stmts->state = JCLNotContinued;
			return NoError;
		}
	}
	if (dst == InstreamDatasetData) {
		return NoError;
	}
	
	if (!wordlcmp(record, DEFAULT_DELIM, strlen(DEFAULT_DELIM))) {
		progInfo->jcl->stmts->state = JCLNotContinued;
		progInfo->jcl->stmts->datasetType = NoDataset;		
		return NoError;
	}
	
	if (!wordlcmp(record, PREFIX, PREFIX_LEN)) {
		progInfo->jcl->stmts->state = JCLNotContinued;	
		progInfo->jcl->stmts->datasetType = NoDataset;
		return processJCLRecord(optInfo, progInfo);
	}

	return NoError;
}

static JCLScanMsg_T processContinuedComment(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	size_t i;
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return processInvalidRecord(optInfo, progInfo, InvalidRecordContinuedComment);
	}

	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, PREFIX_LEN+1); 
	}	
	return rc;
}

static JCLScanMsg_T processJCLContinueString(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	size_t i;
	
	if (text[0] != SLASH || text[1] != SLASH) {
		return processInvalidRecord(optInfo, progInfo, InvalidRecordContinuedStringNoSlash);
	}
	for (i=PREFIX_LEN; i<STRING_CONTINUE_COLUMN; ++i) {
		if (text[i] != BLANK) {
			return processInvalidRecord(optInfo, progInfo, InvalidRecordContinuedCommentTooFewBlanks);
		}
	}
	
	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, PREFIX_LEN); 
	}	
	return rc;
}

static JCLScanMsg_T processJCLContinueConditional(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return processInvalidRecord(optInfo, progInfo, InvalidRecordContinuedConditional);
	}
	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanConditional(optInfo, progInfo, PREFIX_LEN+1);
	}
	return rc;
}

static JCLScanMsg_T processJCLContinueParameter(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return processInvalidRecord(optInfo, progInfo, InvalidRecordContinuedParameter);
	}
	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, PREFIX_LEN+1);
	}
	return rc;
}

static JCLScanMsg_T processJES3ContinueDataset(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	
	if (text[0] == SLASH && text[1] == SLASH && text[2] == ASTERISK) {
		if (!wordlcmp(&text[3], JES3ENDDATASET_PREFIX, JES3ENDDATASET_LEN)) {
			rc = addStatement(optInfo, progInfo, JES3_KEYWORD);
			progInfo->jcl->stmts->state = JCLNotContinued;
		}
	} else {
		rc = addToStatement(optInfo, progInfo);
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
		case JCLContinueString:
			return processJCLContinueString(optInfo, progInfo);
		case JCLContinueParameter:
			return processJCLContinueParameter(optInfo, progInfo);
		case JCLContinueConditional:
			return processJCLContinueConditional(optInfo, progInfo);	
		case JCLContinueJES3DatasetControlStatement:
			return processJES3ContinueDataset(optInfo, progInfo);
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