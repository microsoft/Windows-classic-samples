//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module ACCESSOR.H | CImpIAccessor object implementing the IAccessor interface.
//
//
#ifndef _ACCESSOR_H_
#define _ACCESSOR_H_


//----------------------------------------------------------------------------
// @class CImpIAccessor | Contained IAccessor class
//
class CImpIAccessor : public IAccessor 		//@base public | IAccessor
{
	//	Immediate user objects are friends
	friend class CImpIAccessor;
	friend class CRowset;
	friend class CCommand;

	private: //@access private	
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CBaseObj)
		
		//@cmember array of accessor ptrs
		LPEXTBUFFER m_pextbufferAccessor;

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CBaseObj, CImpIAccessor);

		//@cmember Initialization Routine
		STDMETHODIMP 	FInit(BOOL fUnderRowset);

		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//@cmember Increment Reference count on accessor
		STDMETHODIMP	AddRefAccessor(HACCESSOR hAccessor, DBREFCOUNT* pcRefCounts);
		//@cmember CreateAccessor Method
	    STDMETHODIMP	CreateAccessor(DBACCESSORFLAGS, DBCOUNTITEM, const DBBINDING rgBindings[], DBLENGTH, HACCESSOR*, DBBINDSTATUS rgStatus[]);
		//@cmember GetBindings Method
		STDMETHODIMP	GetBindings(HACCESSOR, DBACCESSORFLAGS*, DBCOUNTITEM*, DBBINDING**);
		//@cmember ReleaseAccessor Method
		STDMETHODIMP	ReleaseAccessor(HACCESSOR, DBREFCOUNT*);
		 //@cmember CopyAccessors Method
		STDMETHODIMP	CopyAccessors(LPEXTBUFFER pextbufferAccessor);
};


#endif _ACCESSOR_H_