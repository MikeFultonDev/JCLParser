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
	while (cur != NULL) {
		if (!strcmp(cur->type, COMMENT_KEYWORD)) {
			printInfo(InfoScannedComment, cur->scanhead->commentText);
		} else {
			if (!strcmp(cur->type, JES2_KEYWORD)) {
				printInfo(InfoScannedJES2ControlStatement);
			} else if (!strcmp(cur->type, JES3_KEYWORD)) {
				printInfo(InfoScannedJES3ControlStatement);
			} else if (!strcmp(cur->type, JCLCMD_KEYWORD)) {
				/* MSF - the next line needs to be improved */
				fwrite(cur->firstJCLLine->text, strlen(cur->firstJCLLine->text), 1, stdout);
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
	return NoError;
}