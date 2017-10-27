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

#ifndef __GENSH__
	#define __GENSH__ 1
	
	#include "jclargs.h"
	#include "jclmsgs.h"
	#include "jcl2sh.h"
	
	typedef struct GenSH {
		FILE* outfp;
	} GenSH_T;	

	JCLScanMsg_T establishOutput(OptInfo_T* optInfo, ProgInfo_T* progInfo);	
#endif