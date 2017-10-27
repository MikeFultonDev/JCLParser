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

#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "jclmsgs.h"
#include "jclargs.h"

static JCLScanMsg_T processDebug(const char* value, Option_T* opt, OptInfo_T* optInfo) {
	optInfo->debug = 1;
	return NoError;
}

static JCLScanMsg_T processHelp(const char* value, Option_T* opt, OptInfo_T* optInfo) {
	optInfo->help = 1;
	return NoError;
}

static JCLScanMsg_T processVerbose(const char* value, Option_T* opt, OptInfo_T* optInfo) {
	optInfo->verbose = 1;
	return NoError;
}

static JCLScanMsg_T processInput(const char* value, Option_T* opt, OptInfo_T* optInfo) {
	if (value[0] != ASSIGN_CHAR) {
		printError(NoArgSpecified, opt->longName);
		return NoArgSpecified;
	}
	++value;
	if (value[0] == '\0') {
		printError(NoArgSpecified, opt->longName);
		return NoArgSpecified;
	}	
	
	optInfo->inputFile = value;
	return NoError;
}

static JCLScanMsg_T processOutput(const char* value, Option_T* opt, OptInfo_T* optInfo) {
	if (value[0] != ASSIGN_CHAR) {
		printError(NoArgSpecified, opt->longName);
		return NoArgSpecified;
	}
	++value;
	if (value[0] == '\0') {
		printError(NoArgSpecified, opt->longName);
		return NoArgSpecified;
	}	
	
	optInfo->outputFile = value;
	return NoError;
}

static const char* generalOptCmp(const char* prefix, int prefixLen, const char* optName, const char* argument) {
	int argLen;
	int optLen;
	const char* nextChar;
	if (memcmp(prefix, argument, prefixLen)) {
		return NULL;
	}
	argLen = strlen(argument);
	optLen = strlen(optName);
	if (argLen < prefixLen + optLen) {
		return NULL;
	}
	if (memcmp(&argument[prefixLen], optName, optLen)) {
		return NULL;
	}
	nextChar = &argument[prefixLen+optLen];
	if (nextChar[0] == ASSIGN_CHAR || nextChar[0] == '\0') {
		return nextChar;
	}
	
	return NULL; 
}

static const char* shortOptCmp(const char* optName, const char* argument) {
	return generalOptCmp(SHORT_OPT_PREFIX, SHORT_OPT_PREFIX_LEN, optName, argument);
}

static const char* longOptCmp(const char* optName, const char* argument) {
	return generalOptCmp(LONG_OPT_PREFIX, LONG_OPT_PREFIX_LEN, optName, argument);
}

static JCLScanMsg_T processArg(const char* argument, Option_T* opts, OptInfo_T* optInfo) {
	int argLen = strlen(argument);
	const char* val;
	JCLScanMsg_T rc = NoError;
	
	while (opts->shortName != NULL) {
		if (argLen > SHORT_OPT_PREFIX_LEN && (val = shortOptCmp(opts->shortName, argument)) != NULL) {
			rc = opts->fn(val, opts, optInfo);
			return rc;
		} else if (argLen > LONG_OPT_PREFIX_LEN && (val = longOptCmp(opts->longName, argument)) != NULL) {
			rc = opts->fn(val, opts, optInfo);
			return rc;
		}
		++opts;
	}
	printError(UnrecognizedOption, argument);
	return UnrecognizedOption;	
}

JCLScanMsg_T processArgs(int argc, char* argv[], OptInfo_T* optInfo) {
	Option_T options[] = {
		{ &processHelp, "?", "help" }, 
		{ &processHelp, "h", "info" }, 		
		{ &processDebug, "d", "debug" }, 
		{ &processVerbose, "v", "verbose"},
		{ &processInput, "f", "file"},	
		{ &processInput, "i", "input"},			
		{ &processOutput, "o", "output"},				
		{ NULL, NULL, NULL }
	};
	JCLScanMsg_T rc;
	int i;
		
	if (argc < MIN_ARGS+1) {
		if (MIN_ARGS == 1) {
			printError(TooFewArgsSingular, MIN_ARGS);
		} else {
			printError(TooFewArgsPlural, MIN_ARGS);
		}

		return TooFewArgsPlural;
	}
		
	if (argc > MAX_ARGS) {
		printError(TooManyArgs);		
		return TooManyArgs;
	}	

	for (i=1; i<argc; ++i) {
		rc = processArg(argv[i], options, optInfo);
		if (rc != NoError) {
			return rc;
		}
		if (optInfo->help) {
			return IssueHelp;
		}
	}
	
	
	return NoError;
}
