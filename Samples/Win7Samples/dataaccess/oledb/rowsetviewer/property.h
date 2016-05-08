//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module PROPERTY.H
//
//-----------------------------------------------------------------------------
#ifndef _PROPERTY_H_
#define _PROPERTY_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// TPropBase
//
///////////////////////////////////////////////////////////////
template <class TYPE> class TPropBase : public CVector<TYPE>
{
public:
	//constructors
	TPropBase(ULONG cElements = 0, TYPE* rgElements = NULL);
	virtual ~TPropBase();

	//Interface
	virtual TYPE*			GetProperties()		{ return (TYPE*)GetElements(); }

	//Interating
	virtual TYPE*			AddProperty(DBPROPID dwPropertyID, const DBID& colid);
	
	//Pure Virtual
	//NOTE: The base class can't implement this for two reasons.  First is that 
	//the structures are not the same, ie: they all don't have a dwPropertyID member 
	//(DBPROPID is just a DWORD array).  Second is that some require comparing colid's as well...
	virtual TYPE*			FindProperty(DBPROPID dwPropertyID, const DBID& colid) = 0;

protected:
	//data
};


///////////////////////////////////////////////////////////////
// CProperties
//
///////////////////////////////////////////////////////////////
class CProperties	: public TPropBase<DBPROP>
{
public:
	//constructors
	CProperties(DBPROPSET* pPropSet = NULL);
	virtual ~CProperties();

	//Interface
	virtual void			Attach(DBPROPSET* pPropSet);
	virtual void			Detach(DBPROPSET* pPropSet);

	//Overloads
	virtual void			RemoveAll();
	virtual DBPROP*			FindProperty(DBPROPID dwPropertyID, const DBID& colid);

	//Helpers
	virtual HRESULT			SetProperty(DBPROPID dwPropertyID, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, const DBID& colid = DB_NULLID);


protected:
	//data
};

	
///////////////////////////////////////////////////////////////
// CPropertyInfos
//
///////////////////////////////////////////////////////////////
class CPropertyInfos	: public TPropBase<DBPROPINFO>
{
public:
	//constructors
	CPropertyInfos(DBPROPINFOSET* pPropInfoSet = NULL);
	virtual ~CPropertyInfos();

	//Interface
	virtual void			Attach(DBPROPINFOSET* pPropSet);
	virtual void			Detach(DBPROPINFOSET* pPropSet);

	//Overloads
	virtual void			RemoveAll();
	virtual DBPROPINFO*		FindProperty(DBPROPID dwPropertyID, const DBID& colid);

	//Helpers
//	virtual HRESULT			SetProperty(DBPROPID dwPropertyID);

protected:
	//data
};


///////////////////////////////////////////////////////////////
// CPropertyIDs
//
///////////////////////////////////////////////////////////////
class CPropertyIDs	: public TPropBase<DBPROPID>
{
public:
	//constructors
	CPropertyIDs(DBPROPIDSET* pPropSet = NULL);
	virtual ~CPropertyIDs();

	//Interface
	virtual void			Attach(DBPROPIDSET* pPropSet);
	virtual void			Detach(DBPROPIDSET* pPropSet);

	//Overloads
	virtual DBPROPID*		FindProperty(DBPROPID dwPropertyID, const DBID& colid);

	//Helpers
	virtual HRESULT			SetProperty(DBPROPID dwPropertyID);

protected:
	//data
};


///////////////////////////////////////////////////////////////
// TPropSetBase
//
///////////////////////////////////////////////////////////////
template <class TYPE, class OBJTYPE> class TPropSetBase : public CVector<TYPE>
{
public:
	//constructors
	TPropSetBase(ULONG cElements = 0, TYPE* rgElements = NULL);
	virtual ~TPropSetBase();

	//Overloads
	virtual void			RemoveAll();

	//Interface
	virtual TYPE*			GetPropSets() { return (TYPE*)GetElements(); }

	//Interating
	virtual TYPE*			AddPropSet(REFGUID guidPropertySet);
	virtual TYPE*			FindPropSet(REFGUID guidPropertySet);

protected:
	//data

private:
};



///////////////////////////////////////////////////////////////
// CPropSets
//
///////////////////////////////////////////////////////////////
class CPropSets	: public TPropSetBase<DBPROPSET, CProperties>
{
public:
	//constructors
	CPropSets(ULONG cPropSets = 0, DBPROPSET* rgPropSets = NULL);
	virtual ~CPropSets();

	//Helpers
	virtual HRESULT			SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, const DBID& colid = DB_NULLID);
	virtual DBPROP*			FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, const DBID& colid);

protected:
	//data
};



///////////////////////////////////////////////////////////////
// CPropInfoSets
//
///////////////////////////////////////////////////////////////
class CPropInfoSets	: public TPropSetBase<DBPROPINFOSET, CPropertyInfos>
{
public:
	//constructors
	CPropInfoSets(ULONG cPropInfoSets = 0, DBPROPINFOSET* rgPropInfoSets = NULL);
	virtual ~CPropInfoSets();

	//Helpers
//	virtual HRESULT			SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, const DBID& colid = DB_NULLID);
	virtual DBPROPINFO*		FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet);

protected:
	//data
};


///////////////////////////////////////////////////////////////
// CPropIDSets
//
///////////////////////////////////////////////////////////////
class CPropIDSets	: public TPropSetBase<DBPROPIDSET, CPropertyIDs>
{
public:
	//constructors
	CPropIDSets(ULONG cPropIDSets = 0, DBPROPIDSET* rgPropIDSets = NULL);
	virtual ~CPropIDSets();

	//Helpers
	virtual HRESULT			SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet);
	virtual DBPROPID*		FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet);

protected:
	//data
};



////////////////////////////////////////////////////////////////////////////
// Properties
//
////////////////////////////////////////////////////////////////////////////

//Find Property
DBPROPINFOSET*	FindPropSet(REFGUID guidPropertySet, ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets);
DBPROPINFO*		FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets);

//Combine Proeprties
HRESULT			CombineProperties(ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, ULONG cPropInfoSets2, DBPROPINFOSET* rgPropInfoSets2, BOOL bFreeAddedPropSet = TRUE);
HRESULT			CombineProperties(ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, ULONG cPropSets2, DBPROPSET* rgPropSets2, BOOL bFreeAddedPropSet = TRUE);

//Property Info
HRESULT			GetPropInfo(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet, DBPROPINFO** ppPropInfo);
DBPROPFLAGS		GetPropInfoFlags(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet);

//Property Flags
BOOL			IsSupportedProperty(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet);
BOOL			IsSettableProperty(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet);

//Get Property
HRESULT			GetProperty(REFIID riid, IUnknown* pIUnknown, DBPROPID dwPropertyID, REFGUID guidPropertySet, DBPROP** ppProperty);
HRESULT			GetProperty(REFIID riid, IUnknown* pIUnknown, DBPROPID dwPropertyID, REFGUID guidPropertySet, DBTYPE wType, void* pv);

//Set Property
HRESULT			SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, const DBID& colid = DB_NULLID);

//Free Property
HRESULT			FreeProperties(ULONG* pcPropSets, DBPROPSET** prgPropSets);
HRESULT			FreeProperties(ULONG* pcPropIDSets, DBPROPIDSET** prgPropIDSets);
HRESULT			FreeProperties(ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets);
HRESULT			FreeProperties(ULONG* pcProperties, DBPROP** prgProperties);
	

#endif	//_PROPERTY_H_
