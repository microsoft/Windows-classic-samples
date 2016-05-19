/*++

Copyright (C)  Microsoft Corporation

Module Name:

	Globals.h

Abstract:


History:

--*/

#ifndef _Globals_H
#define _Globals_H

/******************************************************************************
 *
 *	Name:	Provider_Globals 
 *
 *	
 *  Description:
 *
 *		class encapsulates global helper functions.
 *		Add extra user code here.
 *
 *****************************************************************************/

class Provider_Globals
{
public:

// Public Static Variables.

	static LONG s_LocksInProgress ;
	static LONG s_ObjectsInProgress ;

public:

// Public Functions

	static HRESULT Global_Startup () ;
	static HRESULT Global_Shutdown () ;

	static HRESULT CreateInstance ( 

		const CLSID &a_ReferenceClsid ,
		LPUNKNOWN a_OuterUnknown ,
		const DWORD &a_ClassContext ,
		const UUID &a_ReferenceInterfaceId ,
		void **a_ObjectInterface
	);
} ;

#endif // _Globals_H
