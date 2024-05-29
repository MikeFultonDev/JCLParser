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
		return InternalOutOfMemory_A;
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

JCLScanMsg_T gen(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
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
		if (!strcmp(stmt->type, EXEC_KEYWORD)) {
			fprintf(fp, "\nmvscmd --pgm=%s ", stmt->name);
		} else if (!strcmp(stmt->type, DD_KEYWORD)) {
			KeyValuePair_T* kvp = stmt->kvphead;
			while (kvp) {
				fprintf(fp, " %s=%s", kvp->key.txt, kvp->val.txt);
				kvp = kvp->next;
			}

		}
		stmt=stmt->next;
	}
	return NoError;
}