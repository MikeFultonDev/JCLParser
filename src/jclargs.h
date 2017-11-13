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

#ifndef __JCLARGS__
	#define __JCLARGS__ 1
	
	#include "jclmsgs.h"
	
	#define MIN_ARGS (0)
	#define MAX_ARGS (INT_MAX)
	#define MAX_ARGS_LENGTH (100)
	#define ASSIGN_CHAR '='	
	
	#define PROGNAME_ARG (1)
	#define FIRST_PROG_ARG (PROGNAME_ARG+1)
	
	#define LONG_OPT_PREFIX "--"
	#define LONG_OPT_PREFIX_LEN (sizeof(LONG_OPT_PREFIX)-1)
	
	#define SHORT_OPT_PREFIX "-"
	#define SHORT_OPT_PREFIX_LEN (sizeof(SHORT_OPT_PREFIX)-1)
		
	typedef struct {
		const char* arguments;
		const char* inputFile;
		const char* outputFile;
		int useJES3:1;
		int debug:1;
		int verbose:1;
		int verboseStatements:1;		
		int help:1;
	} OptInfo_T;
	
	struct Option;
	typedef struct Option {
		JCLScanMsg_T (*fn)(const char* argument, struct Option* opt, OptInfo_T* optInfo);
		const char* shortName;
		const char* longName;
		const char* defaultValue;
		const char* specifiedValue;
	} Option_T;	
	
	JCLScanMsg_T processArgs(int argc, char* argv[], OptInfo_T* optInfo);

#endif