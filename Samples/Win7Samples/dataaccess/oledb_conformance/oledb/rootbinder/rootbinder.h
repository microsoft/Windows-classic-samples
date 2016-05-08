//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1999 Microsoft Corporation
//
// @doc
//
// @module ROOTBINDER.H | Header file for ROOTBINDER test module.
//
// @rev 01 | 03-24-99 | Microsoft | Created
//

#ifndef _ROOTBINDER_H_
#define _ROOTBINDER_H_


//////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library

////////////////////////////////////////////////////////////////
// CONSTANTS
////////////////////////////////////////////////////////////////
const GUID	CLSID_PROV1	=	{ 0x83ac8901, 0x6849, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_PROV2	=	{ 0x2799690, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };
const GUID	CLSID_PROV3	=	{ 0x2799691, 0x684b, 0x11d2, { 0x88, 0xdf, 0x0, 0x60, 0x8, 0x9f, 0xc4, 0x66 } };

const WCHAR*	URL1 = L"X-RootBinder://TestURL1";
const WCHAR*	URL2 = L"X-RootBinder://TestURL2";
const WCHAR*	URL3 = L"X-RootBinder://TestURL3";


/////////////////////////////////////////////////////////////////
// DEFINES
/////////////////////////////////////////////////////////////////
#define		NEWURLS		20	//Number of New URLs for testing CreateRow

#define		RUNVAR(func)	ULONG	ulIndex=0;											\
							if(m_eTestCase == TC_ONETHREAD)								\
							{															\
								INIT_THREADS(1);										\
								THREADARG T1Arg = { this };								\
								CREATE_THREADS(func, &T1Arg);							\
								START_THREADS();										\
								END_THREADS();											\
							}															\
							else														\
							{															\
								INIT_THREADS(MAX_THREADS);								\
								THREADARG	T1Arg[MAX_THREADS];							\
								for(ulIndex=0; ulIndex<MAX_THREADS; ulIndex++)			\
								{														\
									T1Arg[ulIndex].pFunc = this;						\
									T1Arg[ulIndex].pArg1 = (void*)(ULONG_PTR)ulIndex; 	\
									CREATE_THREAD(ulIndex, func, &T1Arg[ulIndex]);		\
								}														\
								START_THREADS();										\
								END_THREADS();											\
							}

#define		INITFUNC		ULONG					cPropSets=0;						\
							DBPROPSET*				rgPropSets=NULL;					\
							IBindResource*			pIBR = NULL;						\
							ICreateRow*				pICR = NULL;						\
							IDBBinderProperties*	pIDBBProp = NULL;					\
							IRegisterProvider*		pIRP = NULL;						\
							TCRootBinder*			pThis = (TCRootBinder*)THREAD_FUNC;	\
							ULONG					cThread = (ULONG) THREAD_ARG1;		\
																						\
							TESTC(pThis != NULL)										\
							TESTC(cThread < MAX_THREADS)								\
																						\
							ThreadSwitch();												\
																						\
							if(pThis->m_eTestCase == TC_MULTIPLERB)						\
							{															\
								TESTC(pThis->CreateRBForThread(cThread))				\
								pIBR		= pThis->m_rgIBR[cThread];					\
								pICR		= pThis->m_rgICR[cThread];					\
								pIDBBProp	= pThis->m_rgIDBBProp[cThread];				\
								pIRP		= pThis->m_rgIRP[cThread];					\
							}															\
							else														\
							{															\
								pIBR		= pThis->m_rgIBR[0];						\
								pICR		= pThis->m_rgICR[0];						\
								pIDBBProp	= pThis->m_rgIDBBProp[0];					\
								pIRP		= pThis->m_rgIRP[0];					\
							}

#define		RELEASERB		if(pThis)								\
								pThis->ReleaseMultipleRBs(cThread);

/////////////////////////////////////////////////////////////////
// ENUMS
/////////////////////////////////////////////////////////////////
enum EOBJECTTYPE
{
	DSO				= 0,
	SESSION			= 1,
	ROWSET			= 2,
	ROW				= 3,
	STREAM			= 4,
	INVALID_OBJECT	= 5
};

enum ETESTCASE
{
	TC_ONETHREAD = 1,
	TC_SINGLERB,
	TC_MULTIPLERB
};


#endif 	//_ROOTBINDER_H_
