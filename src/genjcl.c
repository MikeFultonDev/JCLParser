/*******************************************************************************
 *
 * Contributors:
 *    Mike Fulton - initial implementation and documentation
 *******************************************************************************/
#define _ALL_SOURCE_NO_THREADS
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanjcl.h"
#include "gen.h"

JCLScanMsg_T establishOutput(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->gen = malloc(sizeof(Gen_T));
	if (!progInfo->jcl) {
		return InternalOutOfMemory_B;
	}	
	if (optInfo->outputFile) {
		progInfo->gen->outfp = fopen(optInfo->outputFile, "wb");
		if (!progInfo->gen->outfp) {
			printError(UnableToOpenOutput, optInfo->outputFile);
			return UnableToOpenOutput;
		}
	} else {
		progInfo->gen->outfp = stdout;
		if (!progInfo->gen->outfp) {
			printError(UnableToReopenOutput);
			return UnableToReopenOutput;
		}	
	}
	return NoError;
}

JCLScanMsg_T genJCL(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	size_t stmtNum = 0;
	JCLStmts_T* stmts = progInfo->jcl->stmts;
	JCLStmt_T* cur = stmts->head;
	int col=0;
	while (cur != NULL) {
		if (!strcmp(cur->type, COMMENT_KEYWORD)) {
			col += printInfo(InfoScannedComment, cur->scanhead->commentText);
		} else {
			if (!strcmp(cur->type, JES2_KEYWORD)) {
				col+= printInfo(InfoScannedJES2ControlStatement);
			} else if (!strcmp(cur->type, JES3_KEYWORD)) {
				col+= printInfo(InfoScannedJES3ControlStatement);
			} else if (!strcmp(cur->type, JCLCMD_KEYWORD)) {
				/* MSF - the next line needs to be improved */
				col+= fwrite(cur->firstJCLLine->text, strlen(cur->firstJCLLine->text), 1, stdout);
			} else {
				if (!cur->name) {
					col+= printInfo(InfoScannedStatementNoName, cur->type); 
				} else {
					col+= printInfo(InfoScannedStatement, cur->name, cur->type); 
				}	
			}
			if (cur->conditional) {
				col+= printInfo(InfoScannedIFStatement, cur->conditional->text);
			}
			KeyValuePair_T* kvp = cur->kvphead;
			if (kvp == NULL) {
				printInfo(InfoNewLine);
				col = 0;
			}
			while (kvp != NULL) {
				if (!kvp->val.txt) {
					col+= printInfo(InfoKeyOnly, kvp->key.txt);
				} else {
					if (col+kvp->key.len+kvp->val.len < JCL_TXTLEN) {
						col+= printInfo(InfoKeyValuePair, kvp->key.txt, kvp->val.txt);
					} else {
						col+= printInfo(InfoKeyOnly, kvp->key.txt);
						/* print out start, 0->N middle, end value chunks */
						char record[JCL_RECLEN+1];
						int next_start = JCL_TXTLEN-col;
						memcpy(record, kvp->val.txt, next_start);
						record[next_start] = '\0';
						col+= printInfo(InfoValueStart, record);
						col = 0;
						while (next_start+JCL_TXTLEN-STRING_CONTINUE_COLUMN < kvp->val.len) {
							memcpy(record, &kvp->val.txt[next_start], JCL_TXTLEN-STRING_CONTINUE_COLUMN-1);
							record[JCL_TXTLEN-STRING_CONTINUE_COLUMN-1] = '\0';
							col+= printInfo(InfoValueMiddle, record);
							col = 0;
							next_start += JCL_TXTLEN-STRING_CONTINUE_COLUMN-1;
						}
						int remainder = kvp->val.len - next_start;
						memcpy(record, &kvp->val.txt[next_start], remainder);
						record[remainder] = '\0';
						col+= printInfo(InfoValueEnd, record);
					}
				}
				if (kvp->next != NULL) {
					col+= printInfo(InfoSeparator);
				}
				if (kvp->comment) {
					col+= printInfo(InfoComment, kvp->comment);
				}
				if (kvp->next == NULL || kvp->hasNewline) {
					printInfo(InfoNewLine);
					col = 0;
				}
				if (kvp->hasNewline && kvp->next != NULL) {
					col+= printInfo(InfoStmtPrefix);
				}
				kvp = kvp->next;
			}
			if (cur->data) {
				if (cur->data->bytes && (cur->data->len > 0)) {
					fwrite(cur->data->bytes, cur->data->len, 1, stdout);
				}

				if (memcmp(cur->data->retainDelim, EMPTY_DELIM, DELIM_LEN)) {
					col+= printInfo(InfoScannedDelimiter, cur->data->retainDelim);
				}
			}
		}
		cur = cur->next;
	}
	return NoError;
}