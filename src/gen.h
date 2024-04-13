/*******************************************************************************
 *
 * Contributors:
 *    Mike Fulton - initial implementation and documentation
 *******************************************************************************/

#ifndef __GEN__
	#define __GEN__ 1
	
	#include "jclargs.h"
	#include "jclmsgs.h"
	
	#include <stdio.h>
	
	typedef struct JCL JCL_T;

	typedef struct Gen {
		FILE* outfp;
	} Gen_T;	

	typedef struct ProgInfo {
		JCL_T* jcl;
		Gen_T* gen;
	} ProgInfo_T;

	JCLScanMsg_T establishOutput(OptInfo_T* optInfo, ProgInfo_T* progInfo);	
	JCLScanMsg_T genJCL(OptInfo_T* optInfo, ProgInfo_T* progInfo);
#endif
