/*******************************************************************************
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
		unsigned int useJES3:1;
		unsigned int debug:1;
		unsigned int verbose:1;
		unsigned int verboseStatements:1;		
		unsigned int help:1;
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