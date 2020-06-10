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
/*
 * The following runopts is because the current algorithm for allocating inline
 * data is to just keep allocating a slightly larger amount (one more record),
 * copying the data, then freeing.
 * This is not very efficient and it also causes the default allocation to become
 * unhappy for very large tests with inline data streams
 *
 * The better fix is to rewrite the inline data record scanning.
 */
#pragma runopts("HEAP(16M,16M,ANY,KEEP,8K,4K)")
#include <stdio.h>
#include "jclargs.h"
#include "jclmsgs.h"
#include "scanjcl.h"
#include "gensh.h"

static JCLScanMsg_T establishEnvironment(void) {
	return NoError;
}

int main(int argc, char* argv[]) {
	OptInfo_T optInfo = { 0 };
	ProgInfo_T progInfo = { 0 };

	JCLScanMsg_T rc;

	rc = establishEnvironment();

	if (rc != NoError) {
		return 16;
	}
	
	rc = processArgs(argc, argv, &optInfo);

	if (rc == IssueHelp) {
		printHelp(argv[0]);
	}	
	if (rc != NoError) {
		return 8;
	}
	
	rc = establishInput(&optInfo, &progInfo);
	if (rc != NoError) {
		return 8;
	}
	
	rc = establishOutput(&optInfo, &progInfo);
	if (rc != NoError) {
		return 8;
	}	

	rc = scanJCL(&optInfo, &progInfo);	
	if (rc != NoError) {
		return 8;
	}	
	rc = genSH(&optInfo, &progInfo);	
	if (rc != NoError) {
		return 8;
	}	
	
	return 0;
}
