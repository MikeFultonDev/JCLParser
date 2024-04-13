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
		return InternalOutOfMemory;
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
	JCLStmts_T* stmts = progInfo->jcl->stmts;
	FILE* fp = progInfo->gen->outfp;
	JCLStmt_T* stmt;
	char* indent;
	char* pgmName;

	if (!stmts) {
		fprintf(fp, "No statements\n");
		return NoError;
	}
	stmt = stmts->head;

	while (stmt) {
		if (!strcmp(stmt->type, COMMENT_KEYWORD)) {
			fprintf(fp, "//* Comment removed");
		} else if (!strcmp(stmt->type, JES2_KEYWORD)) {
			fprintf(fp, "/*%s %s ", stmt->name, stmt->type);
		} else if (stmt->name && stmt->type) {
			fprintf(fp, "//%s %s ", stmt->name, stmt->type);
		} else if (stmt->name && !stmt->type) {
			fprintf(fp, "//%s ", stmt->name);
		} else if (!stmt->name && stmt->type) {
			fprintf(fp, "//        %s ", stmt->type);
		} else /* if (!stmt->name && !stmt->type) */ {
			; /* this is just a comment */
		} 
		KeyValuePair_T* kvp = stmt->kvphead;
		while (kvp) {
			if (kvp->key.len > 0 && kvp->val.len > 0) {
				fprintf(fp, "%s=%s", kvp->key.txt, kvp->val.txt);
			} else if (kvp->key.len > 0) {
				fprintf(fp, ",%s", kvp->key.txt);
			} else {
				; /* this is just a comment */
			}
			
			if (kvp->hasNewline && (kvp->next != NULL)) {
				fprintf(fp, ",\n//        ");
			}
			kvp = kvp->next;
		}
		fprintf(fp, "\n");
		stmt=stmt->next;
	}
	return NoError;
}