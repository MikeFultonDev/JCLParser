/*******************************************************************************
 *
 * Contributors:
 *    Mike Fulton - initial implementation and documentation
 *******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "jclmsgs.h"

#define JCL2SH_ERR_PREFIX "JCL2SH"


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
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	JCL2SH_ERR_PREFIX "001 Unknown type '%s' on line %d\n",
	JCL2SH_ERR_PREFIX "002 Unexpected character '%c' after '//' on line %d\n",
	JCL2SH_ERR_PREFIX "003 Unexpected character '%c' after '/' on line %d\n",
	JCL2SH_ERR_PREFIX "004 Unexpected character %d on line %d\n",
	JCL2SH_ERR_PREFIX "005 Unexpected text at start of line '%3.3s' processing continued comment on line %d. Expected '// '\n",
	JCL2SH_ERR_PREFIX "006 Unexpected text at start of line '%2.2s' processing continued string on line %d. Expected '//'\n",
	JCL2SH_ERR_PREFIX "007 Unexpected text at start of line '%.*s' processing continued comment. Expected %d blanks on line %d after '//'\n",
	JCL2SH_ERR_PREFIX "008 Unexpected text at start of line '%3.3s' processing continued conditional on line %d. Expected '// '\n",
	JCL2SH_ERR_PREFIX "009 Unexpected text at start of line '%3.3s' processing continued parameter on line %d. Expected '// '\n"	
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

JCLScanMsg_T printError(JCLScanMsg_T reason, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, reason);
	vfprintf(stderr, JCLScanMessage[reason], arg_ptr);
	va_end(arg_ptr);
	return reason;
}

JCLScanInfo_T printInfo(JCLScanInfo_T reason, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, reason);
	vfprintf(stdout, JCLScanInfo[reason], arg_ptr);
	va_end(arg_ptr);
	return reason;
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
