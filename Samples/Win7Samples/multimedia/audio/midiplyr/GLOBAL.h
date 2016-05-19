// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* Global.H
*
* #define's everybody needs
*
*****************************************************************************/

#ifndef _GLOBAL_
#define _GLOBAL_

#ifndef _WIN32
#define  BCODE                  __based(__segname("_CODE"))
#define  BSTACK                 __based(__segname("_STACK"))
#define  BSEG(x)                __based(__segname(x))
#define  HUGE                   __huge
#else
#define  BCODE
#define  BSTACK
#define  BSEG(x)
#define  HUGE
#endif

/* Allow visibility of static functions for debug
*/ 
#ifdef DEBUG
#define  PUBLIC
#define  PRIVATE
#else
#define  PUBLIC                 
#define  PRIVATE                static
#endif

#define  FNLOCAL                NEAR PASCAL
#define  FNGLOBAL               FAR PASCAL
#define  FNEXPORT               FAR PASCAL __export __loadds

#endif
