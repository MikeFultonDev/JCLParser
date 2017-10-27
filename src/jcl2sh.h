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