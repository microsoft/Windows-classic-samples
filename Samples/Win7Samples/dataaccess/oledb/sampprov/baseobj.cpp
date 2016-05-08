//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module BASEOBJ.CPP | Base Object for CCommand and CRowset implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

// CBaseObj::CBaseObj----------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//        
CBaseObj::CBaseObj
	(
	EBaseObjectType		botVal			// @parm IN | Base Object Type
	)
{
	m_BaseObjectType = botVal;
}


//-----------------------------------------------------------------------------
// CBaseObj::~CBaseObj
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//        
CBaseObj::~CBaseObj()
{
}
