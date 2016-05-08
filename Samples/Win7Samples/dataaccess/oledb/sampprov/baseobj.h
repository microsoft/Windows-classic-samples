//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module BASEOBJ.H | Base object for CCommand,CRowset, and CSession
// definitions
//
//


#ifndef __BASEOBJ_H__
#define __BASEOBJ_H__

// Used to distinguish behavior based on object type.
// (i.e. cast a void* to either CCommand,CRowset,or CSession)
enum EBaseObjectType
{
	BOT_UNDEFINED,
	BOT_DATASOURCE,
	BOT_COMMAND,
	BOT_ROWSET,
	BOT_SESSION,
	BOT_ROW,
	BOT_STREAM,
	BOT_BINDER,
};


//-----------------------------------------------------------------------------
// Classes 
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CBaseObj | Base Object for CCommand  and CRowset 
//
//

class CBaseObj : public IUnknown		//@base public | IUnknown
{
private: //@access Private Data Members

	//@cmember Base Object Type
	EBaseObjectType		m_BaseObjectType;

protected: //@access Protected Member Functions

	//@cmember Controlling IUnknown
	LPUNKNOWN			m_pUnkOuter;

	//@ Constructor
	CBaseObj(EBaseObjectType botVal);

public: //@access Public Member Functions
	// @cmember DTOR
	virtual ~CBaseObj();

	// @cmember Get the base object type
	EBaseObjectType GetBaseObjectType()
	{
		return m_BaseObjectType;
	}

	//@cmember Get the outer unknown.
	inline IUnknown * GetOuterUnknown()
	{
		return m_pUnkOuter;
	}
};


#endif  // __BASEOBJ_H__

