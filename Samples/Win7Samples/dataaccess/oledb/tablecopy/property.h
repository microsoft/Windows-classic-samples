//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
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
#include "common.h"


////////////////////////////////////////////////////////////////////////////
// Properties
//
////////////////////////////////////////////////////////////////////////////

//Property Info
HRESULT GetPropInfo(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, DBPROPINFO** ppPropInfo);
DBPROPFLAGS GetPropInfoFlags(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet);

//Property Flags
BOOL IsSupportedProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet);
BOOL IsSettableProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet);

//Get Property
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet,	WCHAR** ppwszValue);
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, DBPROP** ppProperty);
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, WCHAR* pwszValue);
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcValue);
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet,	BOOL* pbValue);

//Set Property
HRESULT SetProperty(DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, ULONG_PTR ulValue, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);
HRESULT SetProperty(DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions = DBPROPOPTIONS_REQUIRED, DBID colid = DB_NULLID);

//Free Property
HRESULT FreeProperties(ULONG cPropSets, DBPROPSET* rgPropSets);
HRESULT FreeProperties(ULONG cProperties, DBPROP* rgProperties);
	
//Variants
HRESULT InitVariants(ULONG cVariants, VARIANT* rgVariants);
HRESULT FreeVariants(ULONG cVariants, VARIANT* rgVariants);

//Restrictions
HRESULT SetRestriction(VARIANT* pRestriction, WCHAR* pwszValue);


#endif	//_PROPERTY_H_
