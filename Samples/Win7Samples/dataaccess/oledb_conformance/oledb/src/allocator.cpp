//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Allocator Implementation Module | 	This module contains definition information
//			for the CAllocator class
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

// allocator.cpp
//#include "MODStandard.hpp"	// Standard headers	
//#include "privstd.h"	// Private library common precompiled header
#include "allocator.hpp"
#include "CPropSet.hpp"
#include "ProviderInfo.h"


void CAllocator::Free(ULONG *pcV, LPVOID **prgV)
{
	CoTaskMemFree(*prgV);
	*pcV	= 0;
	*prgV	= NULL;
} //CAllocator::Free



void CAllocator::Free(ULONG *pcProps, DBPROP **prgProps)
{
	if (!pcProps || !prgProps)
		return;

	for (ULONG index=0; index<*pcProps; index++)
	{
		Free(&(*prgProps)[index]);
	}
	CoTaskMemFree(*prgProps);
	*pcProps	= 0;
	*prgProps	= NULL;
} //CAllocator::Free




void CAllocator::Free(ULONG *pcPropSets, DBPROPSET **prgPropSets)
{
	if (!pcPropSets || !prgPropSets)
		return;

	for (ULONG index=0; index<*pcPropSets; index++)
	{
		CAllocator::Free(&(*prgPropSets)[index].cProperties, &(*prgPropSets)[index].rgProperties);
	}
	CoTaskMemFree(*prgPropSets);
	*pcPropSets		= 0;
	*prgPropSets	= NULL;
} //CAllocator::Free




void CAllocator::Free(ULONG *pcPropSets, CPropSets **prgPropSets)
{
	if (!pcPropSets || !prgPropSets)
		return;

	delete [] *prgPropSets;

	*prgPropSets	= NULL;
	*pcPropSets		= 0;
} //CAllocator::Free




void CAllocator::Free(ULONG *pcPropInfoSets, CPropInfoSets **prgPropInfoSets)
{
	if (!pcPropInfoSets || !prgPropInfoSets)
		return;

	delete [] *prgPropInfoSets;

	*prgPropInfoSets	= NULL;
	*pcPropInfoSets		= 0;
} //CAllocator::Free




void CAllocator::Free(ULONG *pcGUIDs, GUID **prgGUIDs)
{
	if (!pcGUIDs || !prgGUIDs)
		return;

	SAFE_FREE(*prgGUIDs);
	*prgGUIDs = NULL;
	*pcGUIDs = 0;
} //CAllocator::Free




void CAllocator::Free(ULONG *pcArray, ULONG_PTR ***prgArray)
{
	ULONG	index;

	if (!pcArray || !prgArray)
		return;

	for (index = 0; index < *pcArray; index++)
	{
		SAFE_FREE((*prgArray)[index]);
	}
	SAFE_FREE(*prgArray);
	*prgArray = NULL;
	*pcArray = 0;
} //CAllocator::Free




void CAllocator::Free(ULONG *pcSourceInfo, CSourceInfo **prgSourceInfo)
{
	if (!pcSourceInfo || !prgSourceInfo)
		return;

	delete [] *prgSourceInfo;

	*prgSourceInfo	= NULL;
	*pcSourceInfo		= 0;
} //CAllocator::Free




HRESULT CAllocator::Alloc(ULONG cV, LPVOID **prgV)
{
	if (!prgV)
		return E_INVALIDARG;

	if (0 == cV)
	{
		*prgV = NULL;
		return S_OK;
	}

	*prgV = (LPVOID*)CoTaskMemAlloc(cV*sizeof(LPVOID));				
	memset(*prgV, 0, cV*sizeof(LPVOID));
	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(ULONG cProps, DBPROP **prgProps)
{
	if (!prgProps)
		return E_INVALIDARG;

	if (0 == cProps)
	{
		*prgProps = NULL;
		return S_OK;
	}

	*prgProps = (DBPROP*)CoTaskMemAlloc(cProps*sizeof(DBPROP));				
	memset(*prgProps, 0, cProps*sizeof(DBPROP));
	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(ULONG cPropSets, DBPROPSET **prgPropSets)
{
	if (!prgPropSets)
		return E_INVALIDARG;

	if (0 == cPropSets)
	{
		*prgPropSets = NULL;
		return S_OK;
	}

	*prgPropSets = (DBPROPSET*)CoTaskMemAlloc(cPropSets*sizeof(DBPROPSET));				
	memset(*prgPropSets, 0, cPropSets*sizeof(DBPROPSET));
	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(
	ULONG		cPropSets,
	CPropSets	**prgPropSets
)
{
	if (!prgPropSets)
		return E_INVALIDARG;

	*prgPropSets = new CPropSets[cPropSets];

	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(
	ULONG			cPropInfoSets,
	CPropInfoSets	**prgPropInfoSets
)
{
	if (!prgPropInfoSets)
		return E_INVALIDARG;

	*prgPropInfoSets = new CPropInfoSets[cPropInfoSets];

	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(ULONG cGUIDs, GUID **prgGUIDs)
{
	if (!prgGUIDs)
		return E_INVALIDARG;

	if (0 == cGUIDs)
	{
		*prgGUIDs = NULL;
		return S_OK;
	}

	*prgGUIDs = (GUID*)CoTaskMemAlloc(cGUIDs*sizeof(GUID));				
	memset(*prgGUIDs, 0, cGUIDs*sizeof(GUID));
	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(ULONG cArray, ULONG_PTR ***prgArray)
{
	if (!prgArray)
		return E_INVALIDARG;

	if (0 == cArray)
	{
		*prgArray = NULL;
		return S_OK;
	}

	*prgArray = (ULONG_PTR**)CoTaskMemAlloc(cArray*sizeof(ULONG_PTR*));
	memset(*prgArray, 0, cArray*sizeof(ULONG_PTR));
	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Alloc(
	ULONG		cSourceInfo,
	CSourceInfo	**prgSourceInfo
)
{
	if (!prgSourceInfo)
		return E_INVALIDARG;

	*prgSourceInfo = new CSourceInfo[cSourceInfo];

	return S_OK;
} //CAllocator::Alloc




HRESULT CAllocator::Realloc(
	ULONG	cOldV, 
	LPVOID	**prgV, 
	ULONG	cV
)
{
	if (!prgV || (cOldV >= cV))
		return E_INVALIDARG;

	*prgV = (LPVOID*)CoTaskMemRealloc(*prgV, cV*sizeof(LPVOID));				
	memset((*prgV)+cOldV, 0, (cV-cOldV)*sizeof(LPVOID));
	return S_OK;
} //CAllocator::Realloc




HRESULT CAllocator::Realloc(
	ULONG	cOldProps, 
	DBPROP	**prgProps, 
	ULONG	cProps
)
{
	if (!prgProps || (cOldProps >= cProps))
		return E_INVALIDARG;

	*prgProps = (DBPROP*)CoTaskMemRealloc(*prgProps, cProps*sizeof(DBPROP));				
	memset((*prgProps)+cOldProps, 0, (cProps-cOldProps)*sizeof(DBPROP));
	return S_OK;
} //CAllocator::Realloc




HRESULT CAllocator::Realloc(
	ULONG		cOldPropSets, 
	DBPROPSET	**prgPropSets, 
	ULONG		cPropSets
)
{
	if (!prgPropSets || (cOldPropSets >= cPropSets))
		return E_INVALIDARG;

	*prgPropSets = (DBPROPSET*)CoTaskMemRealloc(*prgPropSets, cPropSets*sizeof(DBPROPSET));				
	memset((*prgPropSets)+cOldPropSets, 0, (cPropSets-cOldPropSets)*sizeof(DBPROPSET));
	return S_OK;
} //CAllocator::Realloc




HRESULT CAllocator::Realloc(
	ULONG		cOldPropSets, 
	CPropSets	**prgPropSets, 
	ULONG		cPropSets
)
{
	size_t		index;
	CPropSets	*rgPropSets = NULL;
	DBPROPSET	*rgDBPropSets;
	ULONG		cDBPropSets;

	if (!prgPropSets || (cOldPropSets >= cPropSets))
		return E_INVALIDARG;

	rgPropSets = new CPropSets[cPropSets];

	for (index = 0; index < cOldPropSets; index++)
	{
//		rgPropSets[index] = (*prgPropSets)[index];
		cDBPropSets = (*prgPropSets)[index].cPropertySets();
		rgDBPropSets = (*prgPropSets)[index].pPropertySets();
		(*prgPropSets)[index].Detach();
		rgPropSets[index].Attach(cDBPropSets, rgDBPropSets);
	}

	delete [] (*prgPropSets);
	*prgPropSets = rgPropSets;
	return S_OK;
} //CAllocator::Realloc




HRESULT CAllocator::Realloc(
	ULONG			cOldPropInfoSets, 
	CPropInfoSets	**prgPropInfoSets, 
	ULONG			cPropInfoSets
)
{
	ULONG			index;
	CPropInfoSets	*rgPropInfoSets = NULL;

	if (!prgPropInfoSets || (cOldPropInfoSets >= cPropInfoSets))
		return E_INVALIDARG;

	rgPropInfoSets = new CPropInfoSets[cPropInfoSets];

	for (index = 0; index < cOldPropInfoSets; index++)
	{
		rgPropInfoSets[index] = (*prgPropInfoSets)[index];
	}

	delete [] (*prgPropInfoSets);
	*prgPropInfoSets = rgPropInfoSets;
	return S_OK;
} //CAllocator::Realloc




HRESULT CAllocator::Realloc(
	ULONG		cOldGUIDs, 
	GUID		**prgGUIDs, 
	ULONG		cGUIDs
)
{
	if (!prgGUIDs || (cOldGUIDs >= cGUIDs))
		return E_INVALIDARG;

	*prgGUIDs = (GUID*)CoTaskMemRealloc(*prgGUIDs, cGUIDs*sizeof(GUID));				
	memset((*prgGUIDs)+cOldGUIDs, 0, (cGUIDs-cOldGUIDs)*sizeof(GUID));
	return S_OK;
} //CAllocator::Realloc




HRESULT CAllocator::Realloc(
	ULONG		cOldArray, 
	ULONG_PTR	***prgArray, 
	ULONG		cArray
)
{
	if (!prgArray || (cOldArray >= cArray))
		return E_INVALIDARG;

	*prgArray = (ULONG_PTR**)CoTaskMemRealloc(*prgArray, cArray*sizeof(ULONG_PTR*));
	memset((*prgArray)+cOldArray, 0, (cArray-cOldArray)*sizeof(ULONG_PTR*));
	return S_OK;
} //CAllocator::Realloc



HRESULT CAllocator::Realloc(
	ULONG		cOldSourceInfo, 
	CSourceInfo	**prgSourceInfo, 
	ULONG		cSourceInfo
)
{
	ULONG		index;
	CSourceInfo	*rgSourceInfo = NULL;

	if (!prgSourceInfo || (cOldSourceInfo >= cSourceInfo))
		return E_INVALIDARG;

	rgSourceInfo = new CSourceInfo[cSourceInfo];

	for (index = 0; index < cOldSourceInfo; index++)
	{
		rgSourceInfo[index] = (*prgSourceInfo)[index];
	}

	delete [] (*prgSourceInfo);
	*prgSourceInfo = rgSourceInfo;
	return S_OK;
} //CAllocator::Realloc





/*
template <typename T>
HRESULT	CAllocator<T>::Alloc(ULONG cT, T **prgT)
{
	if (!prgT)
		return E_INVALIDARG;

	*prgT = new T[cT];

	return S_OK;
}


template <typename T>
HRESULT	CAllocator<T>::Realloc(ULONG cOldT, T **prgT, ULONG cT)
{
	ULONG		index;
	T			*rgT = NULL;

	if (!prgT || (cOldT >= cT))
		return E_INVALIDARG;

	rgT = new T[cT];

	for (index = 0; index < cOldT; index++)
	{
		rgT[index] = (*prgT)[index];
	}

	delete [] (*prgT);
	*prgT = rgT;
	return S_OK;
}



template <typename T>
void CAllocator<T>::Free(ULONG *pcT, T **prgT)
{
	if (!pcT || !prgT)
		return;

	delete [] *prgT;

	*prgT	= NULL;
	*pcT	= 0;
} //CAllocator::Free
*/
