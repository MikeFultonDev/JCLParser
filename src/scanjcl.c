/*******************************************************************************
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
#include "gen.h"
	
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

const static VarStr_T JES2ControlStatement[] = {
	{sizeof(JES2CMD_PREFIX)-1, JES2CMD_PREFIX },
	{sizeof(JES2JOBPARM_PREFIX)-1, JES2JOBPARM_PREFIX },
	{sizeof(JES2MESSAGE_PREFIX)-1, JES2MESSAGE_PREFIX },
	{sizeof(JES2NETACCT_PREFIX)-1, JES2NETACCT_PREFIX },
	{sizeof(JES2NOTIFY_PREFIX)-1, JES2NOTIFY_PREFIX },
	{sizeof(JES2OUTPUT_PREFIX)-1, JES2OUTPUT_PREFIX },
	{sizeof(JES2PRIORITY_PREFIX)-1, JES2PRIORITY_PREFIX },
	{sizeof(JES2ROUTE_PREFIX)-1, JES2ROUTE_PREFIX },
	{sizeof(JES2SETUP_PREFIX)-1, JES2SETUP_PREFIX },
	{sizeof(JES2SIGNOFF_PREFIX)-1, JES2SIGNOFF_PREFIX },
	{sizeof(JES2SIGNON_PREFIX)-1, JES2SIGNON_PREFIX },
	{sizeof(JES2XEQ_PREFIX)-1, JES2XEQ_PREFIX },
	{sizeof(JES2XMIT_PREFIX)-1, JES2XMIT_PREFIX },
	{0, NULL}
};

static size_t isNational(char c) {
	return (c == ATSIGN || c == DOLLARSIGN || c == POUNDSIGN);
}

static size_t isValidName(const char* buffer, size_t* nameLen) {
	size_t i;
	size_t dotLocation = 0;
	*nameLen = 0; 
	
	if (!isupper(buffer[0]) && !isNational(buffer[0])) {
		return 0;
	}
	
	for (i=1; i<NAME_LEN*2+1; ++i) {
		*nameLen = i;		
		if (buffer[i] == BLANK) {
			return 1;
		}
		if (!isupper(buffer[i]) && !isdigit(buffer[i]) && !isNational(buffer[i])) {
			if (buffer[i] == DOT && dotLocation == 0) {
				dotLocation = i;
				if (i > NAME_LEN) {
					return 0;
				}
			} else {
				return 0;
			}
		}	
	}
	if (i - dotLocation > NAME_LEN) {
		return 0; 
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


static JCLScanMsg_T addSubParm(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* parm, size_t parmLen, const char* value, size_t valLen, char* comment, int hasNewline) {		
	KeyValuePair_T* kvp = malloc(sizeof(KeyValuePair_T));

	if (!kvp) {
		return InternalOutOfMemory_C;
	}
	kvp->next = NULL;
	kvp->key.len = parmLen;
	kvp->key.txt = malloc(parmLen+1);
	if (!kvp->key.txt) {
		return InternalOutOfMemory_D;
	}	
	/*copyNormalizedText(kvp->key.txt, parm, parmLen);*/
	memcpy(kvp->key.txt, parm, parmLen);
	kvp->key.txt[parmLen] = '\0';
	
	kvp->comment = comment;
	kvp->hasNewline = hasNewline;
	if (value == NULL) {
		kvp->val.len = 0;
		kvp->val.txt = NULL;
	} else {
		kvp->key.txt[parmLen] = '\0';
		kvp->val.len = valLen;
		kvp->val.txt = malloc(valLen+1);
		if (!kvp->val.txt) {
			return InternalOutOfMemory_E;
		}
		/*copyNormalizedText(kvp->val.txt, value, valLen);*/
		memcpy(kvp->val.txt, value, valLen);		
		kvp->val.txt[valLen] = '\0';	
	}
	
	if (!progInfo->jcl->stmts->tail->kvphead) {
		progInfo->jcl->stmts->tail->kvphead = progInfo->jcl->stmts->tail->kvptail = kvp;
	} else {
		progInfo->jcl->stmts->tail->kvptail->next = kvp;
	}

#if 0
	if (optInfo->debug) {
		if (value) {
			printInfo(InfoKeyValuePair, kvp->key.txt, kvp->val.txt);
		} else {
			printInfo(InfoKeyOnly, kvp->key.txt);
		}
		printInfo(InfoNewLine);
	}
#endif	
	
	progInfo->jcl->stmts->tail->kvptail = kvp;		
	return NoError;
}

static char* copyParmText(ScannedLine_T* curLine) {
	size_t end;
	char* text;
	
	if (!curLine->parmText) {
		end = 0;
	} else {
		end = strlen(curLine->parmText);
	}
	text = malloc(end+1);
	if (!text) {
		return NULL;
	}
	memcpy(text, curLine->parmText, end);
	text[end] = '\0';
	
	return text;
}
	
static JCLScanMsg_T scanSubParameters(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	char* comment;
	ScannedLine_T* curLine = progInfo->jcl->stmts->tail->scanhead;
	char* text;	
	char* next;
	size_t i;
	size_t start = 0;
	int hasNewline;
	JCLScanMsg_T rc;

	JCLParmContext_T context = JCLParmInKeyword;
	int curParm = start;
	int curValue = -1;
	int parenNest = 0;
	JCLParmContext_T prevContext = context;
	size_t end;
	
	if (curLine->parmText) {
		end = strlen(curLine->parmText);
		text = copyParmText(curLine);
		if (!text) {
			return InternalOutOfMemory_G;
		}	
	} else {
		end = start;
		text = NULL;
	} 
	
	while (curLine != NULL) {
		for (i=start; i<end; ++i) {
			switch (context) {
				case JCLParmInKeyword:
					if (text[i] == EQUALS) {
						context = JCLParmInValue;
						curValue=i+1;
					} else if (text[i] == COMMA) {
						if (i+1 == end) {
							comment = curLine->commentText;
							hasNewline = 1;
						} else {
							comment = NULL;
							hasNewline = 0;
						}
						rc = addSubParm(optInfo, progInfo, &text[curParm], i-curParm, NULL, 0, comment, hasNewline);
						if (rc != NoError) { return rc; }
						curParm = i+1;					
					}
					break;
				case JCLParmInValue:
					if (text[i] == COMMA && parenNest == 0) {
						const char* value;
						context = JCLParmInKeyword;
						if (curValue == i) {
							value = "";
						} else {
							value = &text[curValue];
						}
						if (i+1 == end) {
							comment = curLine->commentText;
							hasNewline = 1;
						} else {
							comment = NULL;
							hasNewline = 0;
						}					
						rc = addSubParm(optInfo, progInfo, &text[curParm], curValue-curParm-1, value, i-curValue, comment, hasNewline);
						if (rc != NoError) { return rc; }						
						curParm = i+1;
					} else if (text[i] == QUOTE) {
						context = JCLParmInQuote;
						prevContext = JCLParmInValue;
					} else if (text[i] == LPAREN) {
						++parenNest;
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
							context = JCLParmInValue;
						}
					}
					break;
			}
		}
		comment = curLine->commentText;
		curLine = curLine->next;	
		if (curLine != NULL) {
			next = copyParmText(curLine);
			if (!next) {
				return InternalOutOfMemory_H;
			}
		} else {
			next = NULL;
		}
		if (next != NULL && (context == JCLParmInQuote || context == JCLParmInParen || parenNest > 0)) { /* in the middle of processing */
			size_t nextStart = end-curParm;
			size_t nextEnd = strlen(next);			
			char* temp = malloc(nextStart+nextEnd+1);
			if (!temp) {
				return InternalOutOfMemory_I;
			}
			memcpy(temp, &text[curParm], nextStart);
			memcpy(&temp[nextStart], next, nextEnd);
			temp[nextStart+nextEnd] = '\0';
			free(text);
			free(next);
			text = temp;
			start = nextStart;
			curValue -= curParm;
			end = nextStart+nextEnd;
			curParm = 0;						
		} else {  /* clean end of parameter at end of line */
			if (curParm < end) {
				if (context == JCLParmInKeyword) {
					rc = addSubParm(optInfo, progInfo, &text[curParm], i-curParm, NULL, 0, comment, 1);
				} else {
					rc = addSubParm(optInfo, progInfo, &text[curParm], curValue-curParm-1, &text[curValue], end-curValue, comment, 1);
				}
				if (rc != NoError) { return rc; }				
			}
			free(text);
			text = next;
			if (next != NULL) {
				start = 0;
				end = strlen(text);	
				curParm = start;
			} else {
				curParm = end;
			}
		}
	} 	
	if (curParm < end) {
		if (context == JCLParmInKeyword) {
			rc = addSubParm(optInfo, progInfo, &text[curParm], i-curParm, NULL, 0, comment, 1);
		} else {
			rc = addSubParm(optInfo, progInfo, &text[curParm], curValue-curParm-1, &text[curValue], end-curValue, comment, 1);
		}
		if (rc != NoError) { return rc; }		
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

static void skipBlanks(const char* text, size_t* pstart, size_t* pend) {
	size_t start = *pstart;
	size_t end = *pend;
	
	while (start < end) {
		if (text[start] != BLANK) break;
		++start;
	}

	if (start < end) {
		while (end > start) {
			if (text[end] != BLANK) break;
			--end;
		}
		end++;
	}
	*pstart = start;
	*pend = end;
}

static JCLScanMsg_T addScannedLine(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text, 
	size_t parmStart, size_t parmEnd, size_t commentStart, size_t commentEnd) {

	ScannedLine_T* line = malloc(sizeof(ScannedLine_T));
	char* parmText;
	char* commentText;
	
	if (!line) {
		return InternalOutOfMemory_J;
	}
	if (parmEnd > parmStart) {
		parmText = malloc(parmEnd-parmStart+1);	
		if (!parmText) {
			return InternalOutOfMemory_K;
		}		
	} else {
		parmText = NULL;
	}
	
	if (commentEnd > commentStart) {
		commentText = malloc(commentEnd-commentStart+1);
		if (!commentText) {
			return InternalOutOfMemory_L;
		}
	} else {
		commentText = NULL;	
	}

	if (!progInfo->jcl->stmts->tail->scanhead) {
		progInfo->jcl->stmts->tail->scanhead = progInfo->jcl->stmts->tail->scantail = line;
	} else {
		progInfo->jcl->stmts->tail->scantail->next = line;
		progInfo->jcl->stmts->tail->scantail = line;
	}
	line->next = NULL;
	line->parmText = parmText;
	line->commentText = commentText;

	if (line->parmText) {	
		memcpy(line->parmText, &text[parmStart], parmEnd-parmStart);
		line->parmText[parmEnd-parmStart] = '\0';
	}
	
	if (line->commentText) {
		memcpy(line->commentText, &text[commentStart], commentEnd-commentStart);
		line->commentText[commentEnd-commentStart] = '\0';	
	}
		
	return NoError;
}

static JCLScanMsg_T appendCommentText(char** commentText, const char* text, size_t start, size_t end) {
	size_t origLen;
	size_t newLen;
	char* orig = *commentText;
	char* next;
	
	skipBlanks(text, &start, &end); 
	newLen = end-start;

	if (orig) {
		origLen = strlen(orig);
	} else {
		origLen = 0;
	}
	next = malloc(origLen+newLen+1);
	if (!next) {
		return InternalOutOfMemory_W;
	}
	if (origLen != 0) {
		memcpy(next, orig, origLen);
	}
	memcpy(&next[origLen], &text[start], newLen);
	next[origLen+newLen] = '\0';
	
	*commentText = next;
	free(orig);
	
	return NoError;

}

static JCLScanMsg_T appendScannedComment(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text, size_t start, size_t end) {
	ScannedLine_T* line = progInfo->jcl->stmts->tail->scantail;
	char* orig = line->commentText;

	JCLScanMsg_T rc = appendCommentText(&orig, text, start, end);
	line->commentText = orig;
	return rc;
}
	
static int isValidDelimiter(const char* text) {
	if (text == NULL) {
		return 0;
	}
	size_t len = strlen(text);
	if (len != QUOTED_DELIM_LEN) {
		return 0;
	}
	if (text[0] != QUOTE || text[QUOTED_DELIM_LEN-1] != QUOTE) {
		return 0;
	}
	return 1;
}
	
static JCLScanMsg_T scanParametersFromText(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column, char* text) {
	size_t i = column;
	int inString = (progInfo->jcl->stmts->state == JCLContinueString);
	int inComment = (progInfo->jcl->stmts->state == JCLContinueComment);
	int inParameter = (progInfo->jcl->stmts->state == JCLContinueParameter);
	JCLScanMsg_T rc;
	
	int firstLine = (!inString && !inComment && !inParameter);
	size_t start = i;
	size_t end = JCL_TXTLEN;
	size_t commentStart = 0;
	size_t commentEnd = 0;
	
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
				end=i;
				commentStart = i;
				commentEnd = JCL_TXTLEN;
				break;
			}
			++i;
		}
		/*
		 * Catch the special case that column 70 has a comma in it - no blank required
		 * after the comma.
		 */
		if (!inString && !commentStart && (text[JCL_TXTLEN-1] == COMMA)) {
			inParameter = 1;
		}
		skipBlanks(text, &commentStart, &commentEnd);			
		rc = addScannedLine(optInfo, progInfo, text, start, end, commentStart, commentEnd);
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
		
		progInfo->jcl->stmts->curStmt++;		
		rc = scanSubParameters(optInfo, progInfo);
		if (rc != NoError) {
			return rc;
		}
		/*
		 * Check if an instream dataset is queued up. If so, we need to determine the delimiter, update it
		 * and set the state to inline record processing
		 */
		if (progInfo->jcl->stmts->datasetType == InstreamDatasetStar || progInfo->jcl->stmts->datasetType == InstreamDatasetData) {
			const char* dlmOrig = getSubParameter(optInfo, progInfo, DELIM_KEYWORD);
			const char* dlmTxt;
			if (isValidDelimiter(dlmOrig)) {
				dlmTxt = &dlmOrig[1];
			} else {
				dlmTxt = DEFAULT_DELIM;
			}
			
			progInfo->jcl->stmts->state = JCLInlineText;
			memcpy(progInfo->jcl->stmts->delimiter, dlmTxt, DELIM_LEN);
			progInfo->jcl->stmts->delimiter[DELIM_LEN] = '\0';
		}
	}
	
	return NoError;
}

static JCLScanMsg_T scanParameters(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) { 
	char* text = progInfo->jcl->lines->tail->text;
	return scanParametersFromText(optInfo, progInfo, column, text);
}

static JCLScanMsg_T addStatementFromText(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t nameEndOffset, const char* stmtType, const char* text) {
		
	JCLStmt_T* stmt = malloc(sizeof(JCLStmt_T));
	char* name;

	if (!stmt) {
		return InternalOutOfMemory_M;
	}

	if (!progInfo->jcl->stmts->head) {
		progInfo->jcl->stmts->head = progInfo->jcl->stmts->tail = stmt;
	} else {
		progInfo->jcl->stmts->tail->next = stmt;
	}
	stmt->next = NULL;
	stmt->type = stmtType;

	if (nameEndOffset != 0) {
		stmt->name = malloc(nameEndOffset+1);
		if (!stmt->name) {
			return InternalOutOfMemory_N;
		}
		memcpy(stmt->name, text, nameEndOffset);
		stmt->name[nameEndOffset] = '\0';		
	} else {
		stmt->name = NULL;
	}	

	stmt->lines = 1;
	stmt->firstJCLLine = progInfo->jcl->lines->tail;	
	stmt->kvphead = stmt->kvptail = NULL;
	stmt->scanhead = stmt->scantail = NULL;
	stmt->conditional = NULL;
	stmt->data = NULL;
	
	progInfo->jcl->stmts->tail = stmt;
		
	return NoError;
}

static JCLScanMsg_T addStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t nameEndOffset, const char* stmtType) {
	char* text = &progInfo->jcl->lines->tail->text[PREFIX_LEN];
	return addStatementFromText(optInfo, progInfo, nameEndOffset-PREFIX_LEN, stmtType, text);
}

static JCLScanMsg_T addToStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->jcl->stmts->tail->lines++;
	return NoError;
}

void printDebugStatements(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
        size_t stmtNum = 0;
        JCLStmts_T* stmts = progInfo->jcl->stmts;
        JCLStmt_T* cur = stmts->head;

        while (cur != NULL) {
			printInfo(InfoStmtDump, ++stmtNum, cur->type, cur->lines, cur->firstJCLLine);
			cur = cur->next;
		}
}

static void printVerboseStatements(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	size_t stmtNum = 0;
	JCLStmts_T* stmts = progInfo->jcl->stmts;
	JCLStmt_T* cur = stmts->head;
	while (cur != NULL) {
		if (!strcmp(cur->type, COMMENT_KEYWORD)) {
			printInfo(InfoScannedComment, cur->scanhead->commentText);
		} else {
			if (!strcmp(cur->type, JES2_KEYWORD)) {
				printInfo(InfoScannedJES2ControlStatement);
			} else if (!strcmp(cur->type, JES3_KEYWORD)) {
				printInfo(InfoScannedJES3ControlStatement);
			} else {
				if (!cur->name) {
					printInfo(InfoScannedStatementNoName, cur->type); 
				} else {
					printInfo(InfoScannedStatement, cur->name, cur->type); 
				}	
			}
			if (cur->conditional) {
				printInfo(InfoScannedIFStatement, cur->conditional->text);
			}
			KeyValuePair_T* kvp = cur->kvphead;
			if (kvp == NULL) {
				printInfo(InfoNewLine);
			}
			while (kvp != NULL) {
				if (!kvp->val.txt) {
					printInfo(InfoKeyOnly, kvp->key.txt);
				} else {
					printInfo(InfoKeyValuePair, kvp->key.txt, kvp->val.txt);
				}
				if (kvp->next != NULL) {
					printInfo(InfoSeparator);
				}
				if (kvp->comment) {
					printInfo(InfoComment, kvp->comment);
				}
				if (kvp->next == NULL || kvp->hasNewline) {
					printInfo(InfoNewLine);
				}
				if (kvp->hasNewline && kvp->next != NULL) {
					printInfo(InfoStmtPrefix);
				}
				kvp = kvp->next;
			}
			if (cur->data) {
				if (cur->data->bytes && (cur->data->len > 0)) {
					fwrite(cur->data->bytes, cur->data->len, 1, stdout);
				}

				if (memcmp(cur->data->retainDelim, EMPTY_DELIM, DELIM_LEN)) {
					printInfo(InfoScannedDelimiter, cur->data->retainDelim);
				}
			}
		}
		cur = cur->next;
	}
}

static JCLScanMsg_T addToRelationalExpression(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text, size_t start, size_t end) {
	ConditionalExpression_T* cond = progInfo->jcl->stmts->tail->conditional;
	size_t prevLen;
	char* newText;
	skipBlanks(text, &start, &end);
	
	if (!cond) {
		cond = malloc(sizeof(ConditionalExpression_T));
		if (!cond) {
			return InternalOutOfMemory_O;
		}
		cond->text = NULL;
		cond->comment = NULL;
		prevLen = 0;
		progInfo->jcl->stmts->tail->conditional = cond;
	} else {
		prevLen = strlen(cond->text);
	}
	newText = malloc(prevLen+end-start+1);
	if (!newText) {
		return InternalOutOfMemory_P;
	}
	if (cond->text) {
		memcpy(newText, cond->text, prevLen);
		free(cond->text);
	}
	memcpy(&newText[prevLen], &text[start], end-start);
	newText[prevLen+end-start] = '\0';
	
	cond->text = newText;
	return NoError;
}

static JCLScanMsg_T addToInlineData(OptInfo_T* optInfo, ProgInfo_T* progInfo, const char* text, size_t start, size_t end, const char* retainDelim) {
	InlineData_T* data = progInfo->jcl->stmts->tail->data;
	char* newBytes;
	
	if (!data) {
		data = malloc(sizeof(InlineData_T));
		if (!data) {
			return InternalOutOfMemory_Q;
		}
		data->bytes = NULL;
		data->len = 0;
		strcpy(data->retainDelim, EMPTY_DELIM);
		progInfo->jcl->stmts->tail->data = data;
	}
	
	if (retainDelim) {
		strcpy(data->retainDelim, retainDelim);
	} else {			
		newBytes = malloc(data->len+end-start+1);
		if (!newBytes) {
			return InternalOutOfMemory_R;
		}

		if (data->bytes) {
			memcpy(newBytes, data->bytes, data->len);
			free(data->bytes);
		}
		memcpy(&newBytes[data->len], &text[start], end-start);
		newBytes[data->len+end-start] = '\n';
		
		data->bytes = newBytes;
		data->len += (end-start+1);
	}
	return NoError;
}

static JCLScanMsg_T scanConditional(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i = column;
	int conditionalComplete = 0;
	size_t nameLen;
	JCLScanMsg_T rc;
	
	char* text = progInfo->jcl->lines->tail->text;

	while (i < JCL_TXTLEN) {
		if (text[i-1] == BLANK && i < (JCL_TXTLEN - THEN_KEYLEN)) {
			if (!memcmp(&text[i], THEN_KEYWORD, THEN_KEYLEN) && (text[i+THEN_KEYLEN] == BLANK)) {
				conditionalComplete = 1;
				--i;
				break;
			}
		}
		++i;
	}
	
	rc = addToRelationalExpression(optInfo, progInfo, text, column, i); 
	if (rc != NoError) {
		return rc;
	}
	if (conditionalComplete) {
		i += THEN_KEYLEN;
		progInfo->jcl->stmts->state = JCLNotContinued;
		char* conditional_comment = NULL;
		size_t start = i+1;
		char* comment = &text[start];
		size_t end = strlen(comment);
		rc = appendCommentText(&conditional_comment, text, start, end);
		if (strlen(conditional_comment) > 0) {
			progInfo->jcl->stmts->tail->conditional->comment = conditional_comment;
		} else {
			free(conditional_comment);
		}
	} else {
		if (optInfo->verbose) { printInfo(InfoInConditional); }	
		progInfo->jcl->stmts->state = JCLContinueConditional;
		rc = NoError;
	}		
	return rc;
}

static JCLScanMsg_T scanJES2ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	const char* text = progInfo->jcl->lines->tail->text;
	if (optInfo->useJES3) {
		return 0;
	}
	if (optInfo->verbose) { printInfo(InfoProcessJES2ControlStatement); }
	rc = addStatementFromText(optInfo, progInfo, name, JES2_KEYWORD, &text[PREFIX_LEN]);
	if (rc == NoError) {
		rc = addSubParm(optInfo, progInfo, &text[PREFIX_LEN], JCL_TXTLEN-PREFIX_LEN+1, NULL, 0, NULL, 1);
	}		
	return rc;
}

static JCLScanMsg_T scanJES3ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	const char* text = progInfo->jcl->lines->tail->text;	
	if (!optInfo->useJES3) {
		return NoError;
	}
	if (optInfo->verbose) { printInfo(InfoProcessJES3ControlStatement); }
	rc = addStatementFromText(optInfo, progInfo, name, JES3_KEYWORD, &text[PREFIX_LEN]);
	if (!wordlcmp(&text[column], JES3DATASET_PREFIX, JES3DATASET_LEN)) {
		if (optInfo->verbose) { printInfo(InfoInJES3DatasetControlStatement); }		
		progInfo->jcl->stmts->state = JCLContinueJES3DatasetControlStatement;
	} else {
		if (rc == NoError) {
			rc = addSubParm(optInfo, progInfo, &text[PREFIX_LEN], JCL_TXTLEN-PREFIX_LEN+1, NULL, 0, NULL, 1);
		}		
	}
	return rc;
}

static JCLScanMsg_T scanSetStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessSetStatement); }
	rc = addStatement(optInfo, progInfo, name, SET_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanIfStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessIfStatement); }
	rc = addStatement(optInfo, progInfo, name, IF_KEYWORD);
	if (rc == NoError) {
		rc = scanConditional(optInfo, progInfo, column);
	}	
	return rc;
}

static JCLScanMsg_T scanElseStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessElseStatement); }
	rc = addStatement(optInfo, progInfo, name, ELSE_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanEndifStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessEndifStatement); }
	rc = addStatement(optInfo, progInfo, name, ENDIF_KEYWORD);
	return rc;
}


static JCLScanMsg_T scanProcStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessProcStatement); }
	rc = addStatement(optInfo, progInfo, name, PROC_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanPendStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessPendStatement); }
	rc = addStatement(optInfo, progInfo, name, PEND_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanIncludeStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessIncludeStatement); }
	rc = addStatement(optInfo, progInfo, name, INCLUDE_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanJCLLibStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessJCLLibStatement); }
	rc = addStatement(optInfo, progInfo, name, JCLLIB_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanCommandStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessCommandStatement); }
	rc = addStatement(optInfo, progInfo, name, COMMAND_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanCntlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessCntlStatement); }
	rc = addStatement(optInfo, progInfo, name, CNTL_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanEndCntlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessEndCntlStatement); }
	rc = addStatement(optInfo, progInfo, name, ENDCNTL_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanPrintDevStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessPrintDevStatement); }
	rc = addStatement(optInfo, progInfo, name, PRINTDEV_KEYWORD);
	return rc;
}

static JCLScanMsg_T scanOutputStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessOutputStatement); }
	rc = addStatement(optInfo, progInfo, name, OUTPUT_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}
	return rc;
}

static JCLScanMsg_T scanXMitStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessXmitStatement); }
	rc = addStatement(optInfo, progInfo, name, XMIT_KEYWORD);
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

static JCLScanMsg_T scanDDStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessDDStatement); }
	rc = addStatement(optInfo, progInfo, name, DD_KEYWORD);

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

static JCLScanMsg_T scanExecStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessExecStatement); }
	rc = addStatement(optInfo, progInfo, name, EXEC_KEYWORD);
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}	
	return rc;
}

static JCLScanMsg_T scanJobStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;
	if (optInfo->verbose) { printInfo(InfoProcessJobStatement); }
	rc = addStatement(optInfo, progInfo, name, JOB_KEYWORD);	
	if (rc == NoError) {
		rc = scanParameters(optInfo, progInfo, column);
	}	
	return rc;
}

static JCLScanMsg_T scanCommentStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;	
	char* text = &progInfo->jcl->lines->tail->text[PREFIX_LEN+1];
	if (optInfo->verbose) { printInfo(InfoProcessCommentStatement); }
	rc = addStatementFromText(optInfo, progInfo, 0, COMMENT_KEYWORD, text);	
	if (rc != NoError) {
		return rc;
	}
	rc = addScannedLine(optInfo, progInfo, text, 0, 0, 0, JCL_TXTLEN-PREFIX_LEN);	
	return rc;
}

static JCLScanMsg_T scanJCLCommand(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	/*
	 * JCL Command must be one record. scanCommandStatement() gets driven for the 'COMMAND' statement
	 * which can be multiple lines
	 */
	JCLScanMsg_T rc;	
	if (blankRecord(progInfo)) {
		if (optInfo->verbose) { printInfo(InfoProcessNullStatement); }
		rc = addStatement(optInfo, progInfo, name, NULL_KEYWORD);	
	} else {
		if (optInfo->verbose) { printInfo(InfoProcessJCLCommand); }
		rc = addStatement(optInfo, progInfo, name, JCLCMD_KEYWORD);	
	}		
	return rc;
}

static JCLScanMsg_T processStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t nameEndOffset, size_t column, ScanFn_T* scanner) {
	return scanner(optInfo, progInfo, nameEndOffset, column);
}

static int isJES3ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i=0;
	const char* keyword;
	const char* text = &progInfo->jcl->lines->tail->text[column];
	if (!optInfo->useJES3) {
		return 0;
	}
	if (!wordlcmp(text, JES3CMD_PREFIX, JES3CMD_LEN)) {
		if (text[JES3CMD_LEN] == ASTERISK) { return 0; } /* override "/ / * * * ..." as a comment, not JES3 command */
	}
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
		return processStatement(optInfo, progInfo, 0, column, scanJES3ControlStatement);
	} else {
		return processStatement(optInfo, progInfo, 0, column, scanCommentStatement); 
	}
}

static int isJES2ControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	size_t i=0;
	const char* keyword;
	const char* text = &progInfo->jcl->lines->tail->text[column];
	if (optInfo->useJES3) {
		return 0;
	}	
	while ((keyword = JES2ControlStatement[i].txt) != NULL) {		
		if (!wordlcmp(text, keyword, JES2ControlStatement[i].len)) {
			return 1;
		}
		++i;
	} 
	return 0;
}

static JCLScanMsg_T processDelimeterOrControlStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t column) {
	if (isJES2ControlStatement(optInfo, progInfo, column)) {
		return processStatement(optInfo, progInfo, 0, column, scanJES2ControlStatement);
	} else {
		return NoError;
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
		return processStatement(optInfo, progInfo, column, i+DD_KEYLEN, scanDDStatement); 
	} else if (!wordcmp(keyword, EXEC_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+EXEC_KEYLEN, scanExecStatement); 
	} else if (!wordcmp(keyword, SET_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+SET_KEYLEN, scanSetStatement); 
	} else if (!wordcmp(keyword, IF_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+IF_KEYLEN, scanIfStatement); 
	} else if (!wordcmp(keyword, ELSE_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+ELSE_KEYLEN, scanElseStatement); 
	} else if (!wordcmp(keyword, ENDIF_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+ENDIF_KEYLEN, scanEndifStatement); 
	} else if (!wordcmp(keyword, JOB_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+JOB_KEYLEN, scanJobStatement); 
	} else if (!wordcmp(keyword, PROC_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+PROC_KEYLEN, scanProcStatement); 
	} else if (!wordcmp(keyword, PEND_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+PEND_KEYLEN, scanPendStatement); 
	} else if (!wordcmp(keyword, INCLUDE_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+INCLUDE_KEYLEN, scanIncludeStatement); 
	} else if (!wordcmp(keyword, JCLLIB_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+JCLLIB_KEYLEN, scanJCLLibStatement); 
	} else if (!wordcmp(keyword, COMMAND_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+COMMAND_KEYLEN, scanCommandStatement); 
	} else if (!wordcmp(keyword, CNTL_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+CNTL_KEYLEN, scanCntlStatement); 
	} else if (!wordcmp(keyword, ENDCNTL_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+ENDCNTL_KEYLEN, scanEndCntlStatement); 
	} else if (!wordcmp(keyword, OUTPUT_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+OUTPUT_KEYLEN, scanOutputStatement); 
	} else if (!wordcmp(keyword, XMIT_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+XMIT_KEYLEN, scanXMitStatement); 
	} else if (!wordcmp(keyword, PRINTDEV_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+PRINTDEV_KEYLEN, scanPrintDevStatement); 
	} 
	return printError(InvalidRecordUnknownType, keyword, progInfo->jcl->lines->curLine);
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
		return processStatement(optInfo, progInfo, column, i+DD_KEYLEN, scanDDStatement); 
	} else if (!wordcmp(keyword, EXEC_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+EXEC_KEYLEN, scanExecStatement); 
	} else if (!wordcmp(keyword, SET_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+SET_KEYLEN, scanSetStatement); 
	} else if (!wordcmp(keyword, IF_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+IF_KEYLEN, scanIfStatement); 
	} else if (!wordcmp(keyword, ELSE_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+ELSE_KEYLEN, scanElseStatement); 
	} else if (!wordcmp(keyword, ENDIF_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+ENDIF_KEYLEN, scanEndifStatement); 
	} else if (!wordcmp(keyword, PROC_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+PROC_KEYLEN, scanProcStatement); 
	} else if (!wordcmp(keyword, PEND_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+PEND_KEYLEN, scanPendStatement); 
	} else if (!wordcmp(keyword, INCLUDE_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+INCLUDE_KEYLEN, scanIncludeStatement); 
	} else if (!wordcmp(keyword, JCLLIB_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+JCLLIB_KEYLEN, scanJCLLibStatement); 
	} else if (!wordcmp(keyword, COMMAND_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+COMMAND_KEYLEN, scanCommandStatement); 
	} else if (!wordcmp(keyword, ENDCNTL_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+ENDCNTL_KEYLEN, scanEndCntlStatement); 
	} else if (!wordcmp(keyword, XMIT_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+XMIT_KEYLEN, scanXMitStatement); 
	} else if (!wordcmp(keyword, OUTPUT_KEYWORD)) {
		return processStatement(optInfo, progInfo, column, i+OUTPUT_KEYLEN, scanOutputStatement); 
	} else {
		return processStatement(optInfo, progInfo, column, i, scanJCLCommand);
	}
}

static JCLScanMsg_T processInlineRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo);

static JCLScanMsg_T scanGenerateSYSINStatement(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column) {
	JCLScanMsg_T rc;	
	int parmStart = sizeof(GENERATED_SYSIN_DD)-1; 
	int parmEnd = sizeof(GENERATED_SYSIN_DD);
	if (optInfo->verbose) { printInfo(InfoProcessGenerateSYSINStatement); }
	
	rc = addStatementFromText(optInfo, progInfo, GENSYSIN_NAMELEN, DD_KEYWORD, GENSYSIN_NAME);	
	if (rc != NoError) {
		return rc;
	}
	if (rc == NoError) {
		rc = scanParametersFromText(optInfo, progInfo, GENERATED_SYSIN_PREFIX_LEN, GENERATED_SYSIN_DD);
	}		
	if (rc != NoError) {
		return rc;
	}
	progInfo->jcl->stmts->datasetType = InstreamDatasetStar;
	progInfo->jcl->stmts->state = JCLInlineText;
	strcpy(progInfo->jcl->stmts->delimiter, DEFAULT_DELIM);
	rc = processInlineRecord(optInfo, progInfo);
	
	return rc;
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
				return printError(InvalidRecordSlashSlashUnk, text[2], progInfo->jcl->lines->curLine);
			}
		} else if (text[1] == ASTERISK) {
			return processDelimeterOrControlStatement(optInfo, progInfo, PREFIX_LEN);
		} else {
			return processStatement(optInfo, progInfo, 0, 0, scanGenerateSYSINStatement); 		
		}
	} else {
		return processStatement(optInfo, progInfo, 0, 0, scanGenerateSYSINStatement); 		
	}
}

static JCLScanMsg_T processInlineRecord(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	char* dlm = progInfo->jcl->stmts->delimiter;
	DatasetType_T dst = progInfo->jcl->stmts->datasetType;
	const char* record = progInfo->jcl->lines->tail->text;
	int terminateInstream = 0;
	JCLScanMsg_T rc = NoError;
	char* retainDelim = NULL;
	
	if (dst == InstreamDatasetData || dst == InstreamDatasetStar) {
		if (!wordlcmp(record, dlm, DELIM_LEN)) {
			progInfo->jcl->stmts->datasetType = NoDataset;
			progInfo->jcl->stmts->state = JCLNotContinued;
			retainDelim = dlm;
		}			
	} 
	if (dst != InstreamDatasetData) {
		if (!wordlcmp(record, DEFAULT_DELIM, DELIM_LEN)) {
			progInfo->jcl->stmts->state = JCLNotContinued;
			progInfo->jcl->stmts->datasetType = NoDataset;	
			retainDelim = DEFAULT_DELIM;
		} else if (!wordlcmp(record, PREFIX, PREFIX_LEN)) {
			progInfo->jcl->stmts->state = JCLNotContinued;	
			progInfo->jcl->stmts->datasetType = NoDataset;
			retainDelim = EMPTY_DELIM;
		}
	}

	rc = addToInlineData(optInfo, progInfo, record, 0, JCL_TXTLEN+1, retainDelim); 
	
	if (rc == NoError && retainDelim != NULL && !memcmp(retainDelim, EMPTY_DELIM, DELIM_LEN)) {
		rc = processJCLRecord(optInfo, progInfo);
	}
	
	return rc;
}
static JCLScanMsg_T processContinuedComment(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	size_t i;
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return printError(InvalidRecordContinuedComment, text, progInfo->jcl->lines->curLine);
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
		return printError(InvalidRecordContinuedStringNoSlash, text, progInfo->jcl->lines->curLine);		
	}
	for (i=PREFIX_LEN; i<STRING_CONTINUE_COLUMN; ++i) {
		if (text[i] != BLANK) {
			return printError(InvalidRecordContinuedCommentTooFewBlanks, STRING_CONTINUE_COLUMN, text, STRING_CONTINUE_COLUMN, progInfo->jcl->lines->curLine);					
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
		return printError(InvalidRecordContinuedConditional, text, progInfo->jcl->lines->curLine);							
	}
	rc = addToStatement(optInfo, progInfo);
	if (rc == NoError) {
		rc = scanConditional(optInfo, progInfo, PREFIX_LEN);
	}
	return rc;
}

static JCLScanMsg_T processJCLContinueParameter(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	const char* text = progInfo->jcl->lines->tail->text;		
	JCLScanMsg_T rc;
	
	if (text[0] == SLASH && text[1] == SLASH && text[2] == ASTERISK) {
		appendScannedComment(optInfo, progInfo, "\n", 0, 1);
		appendScannedComment(optInfo, progInfo, text, PREFIX_LEN+1, JCL_TXTLEN-PREFIX_LEN);
		return NoError;
	}
	
	if (text[0] != SLASH || text[1] != SLASH || text[2] != BLANK) {
		return printError(InvalidRecordContinuedParameter, text, progInfo->jcl->lines->curLine);									
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
			rc = addStatement(optInfo, progInfo, 0, JES3_KEYWORD);
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
		return InternalOutOfMemory_S;
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
	if (scanRC == InputEOF) {
		scanRC = NoError;
	} else {
		printError(ErrorScanningJCL, scanRC);
	}
	
	fclose(progInfo->jcl->infp);
	progInfo->jcl->infp = NULL;

    if (optInfo->debug) {
            printDebugStatements(optInfo, progInfo);
    }	
	if (optInfo->verboseStatements) {
		printVerboseStatements(optInfo, progInfo);
	}
	
	return scanRC;
}

JCLScanMsg_T establishInput(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->jcl = calloc(1, sizeof(JCL_T));
	if (!progInfo->jcl) {
		return InternalOutOfMemory_T;
	}
	progInfo->jcl->stmts = calloc(1, sizeof(JCLStmts_T));
	if (!progInfo->jcl->stmts) {
		return InternalOutOfMemory_U;
	}	
	progInfo->jcl->lines = calloc(1, sizeof(JCLLines_T));
	if (!progInfo->jcl->lines) {
		return InternalOutOfMemory_V;
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