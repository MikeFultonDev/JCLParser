/*******************************************************************************
 *
 * Contributors:
 *    Mike Fulton - initial implementation and documentation
 *******************************************************************************/
#ifndef __JCL2SH__
	#define __JCL2SH__ 1
	
	#include <stdio.h>
	
	typedef struct JCL JCL_T;
	typedef struct GenSH GenSH_T;
	typedef struct {
		JCL_T* jcl;
		GenSH_T*    sh;
	} ProgInfo_T;
	
#endif