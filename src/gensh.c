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
#include "gensh.h"

JCLScanMsg_T establishOutput(OptInfo_T* optInfo, ProgInfo_T* progInfo) {
	progInfo->sh = malloc(sizeof(GenSH_T));
	if (!progInfo->jcl) {
		return InternalOutOfMemory;
	}	
	if (optInfo->outputFile) {
		progInfo->sh->outfp = fopen(optInfo->inputFile, "wb");
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