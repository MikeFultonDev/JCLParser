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
#include "scanjcl.h"
#include "gensh.h"

JCLScanMsg_T establishOutput(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->sh = malloc(sizeof(GenSH_T));
	if (!progInfo->jcl) {
		return InternalOutOfMemory;
	}	
	if (optInfo->outputFile) {
		progInfo->sh->outfp = fopen(optInfo->outputFile, "wb");
		if (!progInfo->sh->outfp) {
			printError(UnableToOpenOutput, optInfo->outputFile);
			return UnableToOpenOutput;
		}
	} else {
		progInfo->sh->outfp = stdout;
		if (!progInfo->sh->outfp) {
			printError(UnableToReopenOutput);
			return UnableToReopenOutput;
		}	
	}
	return NoError;
}

JCLScanMsg_T genSH(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	JCLStmts_T* stmts = progInfo->jcl->stmts;
	FILE* fp = progInfo->sh->outfp;
	JCLStmt_T* stmt;
	char* indent;

	if (!stmts) {
		fprintf(fp, "No statements\n");
		return NoError;
	}
	stmt = stmts->head;
	while (stmt) {
		if (!strcmp(stmt->type, EXEC_KEYWORD)) {
			indent="";
		} else {
			indent=" ";
		}
		fprintf(fp, "%s%s %s\n", indent, stmt->type, stmt->name ? stmt->name : "");
		stmt=stmt->next;
	}
	return NoError;
}
