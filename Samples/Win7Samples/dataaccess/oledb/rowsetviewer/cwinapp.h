//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CWINAPP.H
//
//-----------------------------------------------------------------------------------

#ifndef _CWINAPP_H_
#define _CWINAPP_H_


/////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "CDialogLite.h"	//CAppLite
#include "Spy.h"			//CMallocSpy
						

//////////////////////////////////////////////////////////////////////////
// CWinApp
//
//////////////////////////////////////////////////////////////////////////
class CWinApp : public CAppLite
{
public:
	CWinApp();
	virtual ~CWinApp();

	//Overrides
	virtual BOOL	InitInstance();
	virtual int		ExitInstance();

	//Helpers

//protected:
	//Data
	CMallocSpy*		m_pCMallocSpy;
};


#endif //_CWINAPP_H_
