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
#include <stdio.h>
#include <stdarg.h>
#include "jclmsgs.h"

static const char* JCLScanMessage[] = {
	"",
	"At least %d argument must be specified.\n", 
	"At least %d arguments must be specified.\n", 
	"No more than %s arguments can be specified.\n",
	"Option %s not recognized.\n",
	"Out of Memory. Program ended abnormally.\n",
	"Unable to establish environment.\n",
	"Error establishing environment.\n",
	"Issue-Help",
	"No value specified for option: %s.\n",	
	"Unable to open input file %s for read.\n",
	"Unable to open output file %s for write.\n",
	"Unable to re-open stdin for record-oriented read.\n",
	"Unable to re-open stdout for write.\n",
	"Unexpected record encountered on line %d of JCL. Invalid Type: %d. Scanning terminated.\n",
	"Error %d encountered reading JCL. Scanning terminated.\n",
};

static const char* JCLScanInfo[] = {
	"",
	"Syntax: %s [<args>]\n", 
	" where <args> is one or more of the following:\n",
	" --help | -? (this help)\n",
	" --info | -h (this help)\n",
	" --verbose | -v (verbose messages). Default is off\n",
	" --debug | -d (even more verbose messages). Default is off.\n",
	" --input=<file> | -i=<file> (Input JCL). Default is to read from stdin.\n",
	" --file=<file> | -f=<file> (alias for --input). \n",	
	" --output=<file> | -o=<file> (Output sh script). Default is to write to stdout.\n",
	"Example: %s --input=//SYSADMIN.JCL\\(CRTZFS\\) --output=/tmp/crtzfs.sh\n",
	"Input JCL had unexpected line longer than 80 bytes. Record %d truncated.\n",
	"%.9d %.80s|\n",
	"-> DD Statement\n",
	"-> EXEC Statement\n",	
	"-> JOB Statement\n",	
	"-> Comment Statement\n",
	"-> IF Statement\n",
	"-> ELSE Statement\n",
	"-> ENDIF Statement\n",
	"-> JCL Command\n",	
	"-> COMMAND Statement\n",	
	"-> CNTL Statement\n",	
	"-> ENDCNTL Statement\n",	
	"-> PRINTDEV Statement\n",	
	"-> INCLUDE Statement\n",		
	"-> JCLLIB Statement\n",		
	"-> NULL Statement\n",		
	"-> OUTPUT Statement\n",		
	"-> PROC Statement\n",		
	"-> PEND Statement\n",		
	"-> SET Statement\n",		
	"-> XMIT Statement\n",
	"-> JES3 Control Statement\n",
	"-> JES2 Control Statement\n",	
	"-> SYSIN Generated Statement\n",		
	"--> continued parameter\n",
	"--> continued string\n",
	"--> continued comment\n",
	"--> continued conditional\n",	
	"--> JES3 inline dataset\n",	
	"--> instream dataset\n",		
	"//%s %s ",
	"//         %s %s ",	
	"//*%s\n", 
	"%s",
	"%s=%s",
	"Stmt %.9d %8s Lines:%.2d Ptr:%p\n",
	",",
	"\n",
	"  %s",
	"// ",
	"/*",
	"//*",
	"%s THEN",
	"%s\n",
};

void printError(JCLScanMsg_T reason, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, reason);
	vfprintf(stderr, JCLScanMessage[reason], arg_ptr);
	va_end(arg_ptr);
}

void printInfo(JCLScanInfo_T reason, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, reason);
	vfprintf(stdout, JCLScanInfo[reason], arg_ptr);
	va_end(arg_ptr);
}

void printHelp(const char* progName) {
	printInfo(InfoSyntax01, progName);
	printInfo(InfoSyntax02);
	printInfo(InfoSyntax03);
	printInfo(InfoSyntax04);
	printInfo(InfoSyntax05);
	printInfo(InfoSyntax06);
	printInfo(InfoSyntax07);
	printInfo(InfoSyntax08);
	printInfo(InfoSyntax09);
	printInfo(InfoSyntax10, progName);	
}	
