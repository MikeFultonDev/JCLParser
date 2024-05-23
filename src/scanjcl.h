/*******************************************************************************
 *
 * Contributors:
 *    Mike Fulton - initial implementation and documentation
 *******************************************************************************/
#ifndef __JCLSCAN__
	#define __JCLSCAN__ 1
	
	#include "jclargs.h"
	#include "jclmsgs.h"
	#include "gen.h"
	
	#define JCL_RECLEN (80)
	#define JCL_TXTLEN (71)
	
	#define PREFIX "//"
	#define PREFIX_LEN (sizeof(PREFIX)-1)
	
	#define STRING_CONTINUE_COLUMN (15)
	
	#define NAME_LEN   (8)
	#define NEWLINE    '\n'
	#define SLASH      '/'
	#define ASTERISK   '*'
	#define BLANK      ' '
	#define EQUALS     '='

#if defined(OEMVS)
  #if (__CHARSET_LIB == 1)
	#define ATSIGN     '@'
	#define DOLLARSIGN '$'
	#define POUNDSIGN  '#'
  #else
    /* 
	 * If we are on z/OS AND we are not compiling source code in ASCII,
	 * THEN use hard-coded hex values for the National characters 
	 * in case a code page other than 1047 is in use.
	 */
	#define ATSIGN     0x7C
	#define DOLLARSIGN 0x5B
	#define POUNDSIGN  0x7B
  #endif
#else
	#define ATSIGN     '@'
	#define DOLLARSIGN '$'
	#define POUNDSIGN  '#'
#endif

	#define QUOTE      '\''
	#define COMMA      ','
	#define LPAREN     '('
	#define RPAREN     ')'
	#define DOT        '.'
	
	#define DEFAULT_DELIM "/*"
	#define EMPTY_DELIM   "  "
	#define DELIM_LEN 2
	#define QUOTED_DELIM_LEN (DELIM_LEN + 2)

	#define JCLCMD_KEYWORD   "JCLCMD"	
	#define NULL_KEYWORD     ""	
	
	#define COMMAND_KEYWORD  "COMMAND"
	#define CNTL_KEYWORD     "CNTL"
	#define DD_KEYWORD       "DD"
	#define ENDCNTL_KEYWORD  "ENDCNTL"
	#define EXEC_KEYWORD     "EXEC"
	#define IF_KEYWORD       "IF"
	#define THEN_KEYWORD     "THEN"	
	#define ELSE_KEYWORD     "ELSE"
	#define ENDIF_KEYWORD    "ENDIF"
	#define INCLUDE_KEYWORD  "INCLUDE"
	#define JCLLIB_KEYWORD   "JCLLIB"
	#define JOB_KEYWORD      "JOB"
	#define OUTPUT_KEYWORD   "OUTPUT"
	#define PEND_KEYWORD     "PEND"
	#define PROC_KEYWORD     "PROC"
	#define SET_KEYWORD      "SET"
	#define XMIT_KEYWORD     "XMIT"
	#define PRINTDEV_KEYWORD "PRINTDEV"
	
	#define COMMENT_KEYWORD "//*"
	#define JES3_KEYWORD    "//*"
	#define JES2_KEYWORD    "/*"
	#define GENSYSIN_KEYWORD "DD *"	
	#define GENSYSIN_NAME    "SYSIN "
	
	#define DATA_KEYWORD    "DATA"
	#define DELIM_KEYWORD   "DLM"
	
	#define JCLCMD_KEYLEN   (sizeof(JCLCMD_KEYWORD)-1)	
	#define NULL_KEYLEN     (sizeof(NULL_KEYWORD)-1)	
	
	#define COMMAND_KEYLEN  (sizeof(COMMAND_KEYWORD)-1)
	#define CNTL_KEYLEN     (sizeof(CNTL_KEYWORD)-1)
	#define DD_KEYLEN       (sizeof(DD_KEYWORD)-1)
	#define ENDCNTL_KEYLEN  (sizeof(ENDCNTL_KEYWORD)-1)
	#define EXEC_KEYLEN     (sizeof(EXEC_KEYWORD)-1)
	#define IF_KEYLEN       (sizeof(IF_KEYWORD)-1)
	#define THEN_KEYLEN     (sizeof(THEN_KEYWORD)-1)	
	#define ELSE_KEYLEN     (sizeof(ELSE_KEYWORD)-1)
	#define ENDIF_KEYLEN    (sizeof(ENDIF_KEYWORD)-1)
	#define INCLUDE_KEYLEN  (sizeof(INCLUDE_KEYWORD)-1)
	#define JCLLIB_KEYLEN   (sizeof(JCLLIB_KEYWORD)-1)
	#define JOB_KEYLEN      (sizeof(JOB_KEYWORD)-1)
	#define OUTPUT_KEYLEN   (sizeof(OUTPUT_KEYWORD)-1)
	#define PEND_KEYLEN     (sizeof(PEND_KEYWORD)-1)
	#define PROC_KEYLEN     (sizeof(PROC_KEYWORD)-1)
	#define SET_KEYLEN      (sizeof(SET_KEYWORD)-1)
	#define XMIT_KEYLEN     (sizeof(XMIT_KEYWORD)-1)	
	#define PRINTDEV_KEYLEN (sizeof(PRINTDEV_KEYWORD)-1)		
	
	#define COMMENT_KEYLEN  (sizeof(COMMENT_KEYWORD)-1)		
	#define JES3_KEYLEN     (sizeof(JES3_KEYWORD)-1)		
	#define JES2_KEYLEN     (sizeof(JES2_KEYWORD)-1)	
	#define GENSYSIN_KEYLEN (sizeof(GENSYSIN_KEYWORD)-1)	
	#define GENSYSIN_NAMELEN (sizeof(GENSYSIN_NAME)-1)			
	
	#define DATA_KEYLEN     (sizeof(DATA_KEYWORD)-1)
	
	#define GENERATED_SYSIN_DD "//SYSIN DD *     (Generated)                                                   "
	#define GENERATED_SYSIN_PREFIX_LEN (2+5+1+2)
	typedef struct {
		size_t len;
		char* txt;
	} VarStr_T;
	
	#define JES3CMD_PREFIX        "*"
	#define JES3DATASET_PREFIX    "DATASET"
	#define JES3ENDDATASET_PREFIX "ENDDATASET"
	#define JES3ENDPROCESS_PREFIX "ENDPROCESS"
	#define JES3FORMAT_PREFIX     "FORMAT"
	#define JES3MAIN_PREFIX       "MAIN"
	#define JES3NET_PREFIX        "NET"
	#define JES3NETACCT_PREFIX    "NETACCT"
	#define JES3OPERATOR_PREFIX   "OPERATOR"
	#define JES3PAUSE_PREFIX      "PAUSE"
	#define JES3PROCESS_PREFIX    "PROCESS"
	#define JES3ROUTE_PREFIX      "ROUTE"
	#define JES3SIGNOFF_PREFIX    "SIGNOFF"
	#define JES3SIGNON_PREFIX     "SIGNON"	
	
	#define JES3CMD_LEN           (sizeof(JES3CMD_PREFIX)-1)
	#define JES3DATASET_LEN       (sizeof(JES3DATASET_PREFIX)-1)
	#define JES3ENDDATASET_LEN    (sizeof(JES3ENDDATASET_PREFIX)-1)	
	
	#define JES2CMD_PREFIX		  "$"
	#define JES2JOBPARM_PREFIX    "JOBPARM"
	#define JES2MESSAGE_PREFIX    "MESSAGE"
	#define JES2NETACCT_PREFIX    "NETACCT"
	#define JES2NOTIFY_PREFIX     "NOTIFY"
	#define JES2OUTPUT_PREFIX     "OUTPUT"
	#define JES2PRIORITY_PREFIX   "PRIORITY"
	#define JES2ROUTE_PREFIX      "ROUTE"
	#define JES2SETUP_PREFIX      "SETUP"
	#define JES2SIGNOFF_PREFIX    "SIGNOFF"
	#define JES2SIGNON_PREFIX     "SIGNON"
	#define JES2XEQ_PREFIX        "XEQ"
	#define JES2XMIT_PREFIX       "XMIT"
	
	/* JCL Reference: Chapter 3. Format of Statements */
	typedef enum {
		JCLNotContinued=0,
		JCLContinueParameter=1,
		JCLContinueString=2,
		JCLContinueComment=3,
		JCLContinueJES2ControlStatement=4,
		JCLContinueJES3DatasetControlStatement=5,
		JCLInlineText=6,
		JCLContinueConditional=7,
	} JCLScanState_T;
	
	typedef enum {
		NoDataset=0,
		OutstreamDataset=1,
		InstreamDatasetStar=2,
		InstreamDatasetData=3
	} DatasetType_T;
	
	typedef enum {
		JCLParmInKeyword=1,
		JCLParmInValue=2,
		JCLParmInQuote=3,
		JCLParmInParen=4
	} JCLParmContext_T;
		
	struct JCLLine;
	typedef struct JCLLine {
		struct JCLLine* next;
		char text[JCL_RECLEN+1];
	} JCLLine_T;
	typedef struct {
		JCLLine_T* head;
		JCLLine_T* tail;
		size_t curLine;
	} JCLLines_T;
	
	struct KeyValuePair;
	typedef struct KeyValuePair {
		struct KeyValuePair* next;
		VarStr_T key;
		VarStr_T val;
		char* comment; 
		int hasNewline: 1;
	} KeyValuePair_T;
	
	struct ScannedLine;
	typedef struct ScannedLine {
		struct ScannedLine* next;
		char*  parmText;
		char*  commentText;
	} ScannedLine_T;
	
	typedef struct  {
		char* text;
	} ConditionalExpression_T;
	
	typedef struct  {
		size_t len;			
		char retainDelim[DELIM_LEN+1];			
		char* bytes;
	} InlineData_T;
	
	struct JCLStmt;
	typedef struct JCLStmt {
		struct JCLStmt* next;
		char* name;		
		const char* type;	
		size_t lines;					

		ScannedLine_T* scanhead;
		ScannedLine_T* scantail;
		
		KeyValuePair_T* kvphead;
		KeyValuePair_T* kvptail;
		
		ConditionalExpression_T* conditional;
		InlineData_T* data;
		
		JCLLine_T* firstJCLLine;
	} JCLStmt_T;
	
	typedef struct {
		JCLStmt_T* head;
		JCLStmt_T* tail;
		size_t curStmt;
		JCLScanState_T state;
		DatasetType_T datasetType;
		char delimiter[DELIM_LEN+1];
	} JCLStmts_T;
	
	typedef struct JCL {
		FILE* infp;
		JCLLines_T* lines;
		JCLStmts_T* stmts;
	} JCL_T;	
	
	typedef JCLScanMsg_T (ScanFn_T)(OptInfo_T* optInfo, ProgInfo_T* progInfo, size_t name, size_t column);	
	typedef struct {
		ScanFn_T* scan;
	} Scanner_T;

	
	JCLScanMsg_T scanJCL(OptInfo_T* optInfo, ProgInfo_T* progInfo); 
	JCLScanMsg_T establishInput(OptInfo_T* optInfo, ProgInfo_T* progInfo);
#endif