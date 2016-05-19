//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CPropSets Implementation Module | 	This module contains definition information
//			for the CPropInfoSets and CPropSets classes
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

#ifndef __PROPSETS__
#define __PROPSETS__

//#include "MODStandard.hpp"	// Standard headers	
//#include "privstd.h"	// Standard headers	
#include "CVectorEx.hpp"
#include "CPropSet.hpp"


ULONG g_nTabs;
const ULONG	cMaxName	= 200;
inline void Ident()
{
	for (ULONG i = 0; i < g_nTabs; i++)
	{
		odtLog << "\t";
	}
} //Ident





CPropSets::CPropSets(ULONG ulASetSizeInc, ULONG ulSizeInc):m_PropSets(ulASetSizeInc, ulSizeInc)
{
	return;
} //CPropSets::CPropSets



void CPropSets::Attach(ULONG cPropSets, DBPROPSET *rgPropSets)
{
	ULONG				indexSet;
	DBPROPSET			*pPropSet;
	CVectorEx<DBPROP>	*pASetWrapper;

	Free();
	
	m_PropSets.Attach(cPropSets, (DBPROPSET*)rgPropSets);
	pASetWrapper = m_PropSets;
	for (indexSet = 0; indexSet < m_PropSets.GetCount(); indexSet++, pASetWrapper++)
	{
		pPropSet = &rgPropSets[indexSet];
		pASetWrapper->Attach(pPropSet->cProperties, (DBPROP*)pPropSet->rgProperties);
	}
} //CPropSets::Attach



// gives up memory used for DBPROPSET array
void CPropSets::Detach() 
{
	m_PropSets.Detach();	
} //CPropSets::Detach



void CPropSets::Free()
{
	m_PropSets.Free();
} //CPropSets::Free



ULONG CPropSets::GetIndex(GUID guidPropSet)
{
	ULONG		i;

	for (i=0; i<cPropertySets(); i++)
	{
		if (m_PropSets[i].guidPropertySet == guidPropSet)
			break;
	}
	return i;
} //CPropSets::GetIndex




DBPROPSET *CPropSets::FindPropertySet(GUID guidPropertySet)
{
	ULONG	index = GetIndex(guidPropertySet);

	return (cPropertySets() > index)? &m_PropSets[index]: NULL;
} //CPropSets::FindPropertySet




DBPROP *CPropSets::FindProperty(DBPROPID PropertyID, GUID guidPropertySet)
{
	DBPROPSET	*pPropSet = FindPropertySet(guidPropertySet);
	ULONG		index;
	
	if (!pPropSet)
		return NULL;

	for (index=0; index<pPropSet->cProperties; index++)
	{
		if (PropertyID == pPropSet->rgProperties[index].dwPropertyID)
			return &(pPropSet->rgProperties[index]);
	}

	return NULL;
} //CPropSets::FindProperty




HRESULT CPropSets::SetProperty(
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	DBTYPE			wType, 
	ULONG_PTR		ulValue, 
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{

	if(wType == DBTYPE_BOOL)
		ulValue = ulValue ? VARIANT_TRUE : VARIANT_FALSE;
	return SetProperty(PropertyID, guidPropertySet, wType, &ulValue, dwOptions, colid);
} //CPropSets::SetProperty




HRESULT CPropSets::SetProperty(
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	VARIANT			*pv, 
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{
	HRESULT		hr		= E_FAIL;
	VARIANT		*pVariant;
	DBPROP		*pProp	= NULL;

	pProp = FindProperty(PropertyID, guidPropertySet);

	if (!pv)
		goto CLEANUP;

	if (NULL == pProp)
		return AddProperty(PropertyID, guidPropertySet, pv, dwOptions, colid);
		
	// update the info
	pProp->dwOptions    = dwOptions;
	ReleaseDBID(&pProp->colid, FALSE);
	DuplicateDBID(colid, &pProp->colid);
	pVariant			= &pProp->vValue;
	VariantClear(pVariant);
	VariantCopy(pVariant, pv);
	
	//Status is supposed to be ignored on input
	pProp->dwStatus     = 0x123456; //INVALID(DBPROPSTATUS);


	hr = S_OK;

CLEANUP:
	return hr;
} //CPropSets::SetProperty





HRESULT CPropSets::SetProperty(
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	DBTYPE			wType, 
	void			*pv, 
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{
	HRESULT		hr		= E_FAIL;
	VARIANT		*pVariant;
	DBPROP		*pProp	= NULL;

	pProp = FindProperty(PropertyID, guidPropertySet);

	if (NULL == pProp)
		return AddProperty(PropertyID, guidPropertySet, wType, pv, dwOptions, colid);
		
	// update the info
	pProp->dwOptions    = dwOptions;
	ReleaseDBID(&pProp->colid, FALSE);
	DuplicateDBID(colid, &pProp->colid);
	pVariant			= &pProp->vValue;
	VariantClear(pVariant);
	
	//Status is supposed to be ignored on input
	pProp->dwStatus     = 0x123456; //INVALID(DBPROPSTATUS);

	//Create the Variant
	switch(wType)
	{
		//Unfortunately BOOL, I2, and I4 expect to have the "Address" passed
		//This makes use to have to have special handling to take the Address.
		//This should be changed to just be a void*, but would need changing
		//all calling sites which are quite a few...
		case DBTYPE_BOOL:
			CreateVariant(pVariant, wType, (void*)*(VARIANT_BOOL*)pv);
			break;
		
		case DBTYPE_I2:
			CreateVariant(pVariant, wType, (void*)*(SHORT*)pv);
			break;

		case DBTYPE_I4:
			CreateVariant(pVariant, wType, (void*)(LONG_PTR)*(LONG*)pv);
			break;
	
		default:
			CreateVariant(pVariant, wType, (void*)pv);
			break;
	}

	hr = S_OK;

	return hr;
} //CPropSets::SetProperty





// this adds a property to the set; if the property already exists it will be duplicated
HRESULT CPropSets::AddProperty(
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	VARIANT			*pv, 
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{
	HRESULT		hr		= E_FAIL;
	DBPROP		Prop;
	ULONG		ulSetIndex;
	VARIANT		*pVariant;

	// identify the proper DBPROPSET
	ulSetIndex = GetIndex(guidPropertySet);

	if (ulSetIndex >= cPropertySets())
	{
		// must add a new property set
		DBPROPSET	PropSet;

		PropSet.cProperties		= 0;
		PropSet.guidPropertySet	= guidPropertySet;
		PropSet.rgProperties	= NULL;
		m_PropSets.AddSet((DBPROPSET*)&PropSet);

	}

	//Add the new property to the list
	Prop.dwPropertyID	= PropertyID;
	Prop.dwOptions		= dwOptions;
	Prop.colid			= colid;
	pVariant			= &Prop.vValue;
	VariantInit(pVariant);
	VariantCopy(pVariant, pv);
	
	//Status is supposed to be ignored on input
	Prop.dwStatus     = 0x123456; //INVALID(DBPROPSTATUS);

	GetSetWrapper(ulSetIndex)->AddElement((DBPROP*)&Prop);
	m_PropSets[ulSetIndex].cProperties++;
	// set the rgProperties field (musai)
	m_PropSets[ulSetIndex].rgProperties = (DBPROP*)(GetSetWrapper(ulSetIndex)->GetElements());

	// set index information

	hr = S_OK;
	return hr;
} //CPropSets::AddProperty




HRESULT CPropSets::AddProperty(
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	DBTYPE			wType, 
	ULONG_PTR		ulValue, 
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{

	if(wType == DBTYPE_BOOL)
		ulValue = ulValue ? VARIANT_TRUE : VARIANT_FALSE;
	return AddProperty(PropertyID, guidPropertySet, wType, &ulValue, dwOptions, colid);
} //CPropSets::AddProperty




// this adds a property to the set; if the property already exists it will be duplicated
HRESULT CPropSets::AddProperty(
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	DBTYPE			wType, 
	void			*pv, 
	DBPROPOPTIONS	dwOptions, 
	DBID			colid
)
{
	HRESULT		hr		= E_FAIL;
	DBPROP		Prop;
	ULONG		ulSetIndex;
	VARIANT		*pVariant;

	// identify the proper DBPROPSET
	ulSetIndex = GetIndex(guidPropertySet);

	if (ulSetIndex >= cPropertySets())
	{
		// must add a new property set
		DBPROPSET	PropSet;

		PropSet.cProperties		= 0;
		PropSet.guidPropertySet	= guidPropertySet;
		PropSet.rgProperties	= NULL;
		m_PropSets.AddSet((DBPROPSET*)&PropSet);

	}

	//Add the new property to the list
	Prop.dwPropertyID = PropertyID;
	Prop.dwOptions    = dwOptions;
	Prop.colid        = colid;
	pVariant			= &Prop.vValue;
	VariantInit(pVariant);
	
	//Status is supposed to be ignored on input
	Prop.dwStatus     = 0x123456; //INVALID(DBPROPSTATUS);

	//Create the Variant
	switch(wType)
	{
		//Unfortunately BOOL, I2, and I4 expect to have the "Address" passed
		//This makes use to have to have special handling to take the Address.
		//This should be changed to just be a void*, but would need changing
		//all calling sites which are quite a few...
		case DBTYPE_BOOL:
			CreateVariant(pVariant, wType, (void*)*(VARIANT_BOOL*)pv);
			break;
		
		case DBTYPE_I2:
			CreateVariant(pVariant, wType, (void*)*(SHORT*)pv);
			break;

		case DBTYPE_I4:
			CreateVariant(pVariant, wType, (void*)(LONG_PTR)*(LONG*)pv);
			break;
	
		default:
			CreateVariant(pVariant, wType, (void*)pv);
			break;
	}

	GetSetWrapper(ulSetIndex)->AddElement((DBPROP*)&Prop);
	m_PropSets[ulSetIndex].cProperties++;
	// set the rgProperties field (musai)
	m_PropSets[ulSetIndex].rgProperties = (DBPROP*)(GetSetWrapper(ulSetIndex)->GetElements());

	// set index information

	hr = S_OK;
	return hr;
} //CPropSets::AddProperty


CPropSets &CPropSets::operator = (CPropSets &X)
{
	ULONG		iSet;
	ULONG		iProp;
	DBPROP		*pProp;
	DBPROPSET	*pPropSet;

	Free();

	for (iSet=0; iSet<X.cPropertySets(); iSet++)
	{
		pPropSet = X.pPropertySets()+iSet;
		for (iProp=0; iProp < pPropSet->cProperties; iProp++)
		{
			pProp = pPropSet->rgProperties + iProp;
			SetProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, 
				&pProp->vValue, pProp->dwOptions, pProp->colid);
		}
	}

	m_pPropInfoSets = X.m_pPropInfoSets;
	return *this;
} //CPropSets::operator = 




BOOL CPropSets::Clone(DBORDINAL *pcPropSets, DBPROPSET **prgPropSets)
{
	TBEGIN
	ULONG		iSet;
	ULONG		iProp;

	DBPROP		*pPropDest;
	DBPROP		*pProp;
	DBPROPSET	*pPropSet;

	DBPROPSET	*rgPropSets;
	DBORDINAL	cPropSets;


	TESTC(NULL != pcPropSets && NULL != prgPropSets);

	*pcPropSets		= m_PropSets.GetCount();
	// alloc space for the property array
	SAFE_ALLOC(*prgPropSets, DBPROPSET, cPropertySets());

	rgPropSets	= *prgPropSets;
	cPropSets	= *pcPropSets;

	pPropSet	= (DBPROPSET*)m_PropSets.GetElements();
	for (iSet=0; iSet<cPropSets; iSet++, pPropSet++)
	{
		rgPropSets[iSet].guidPropertySet	= pPropSet->guidPropertySet;
		rgPropSets[iSet].cProperties		= pPropSet->cProperties;
		SAFE_ALLOC(rgPropSets[iSet].rgProperties, DBPROP, pPropSet->cProperties);

		pProp		= pPropSet->rgProperties;
		pPropDest	= rgPropSets[iSet].rgProperties;

		for (iProp=0; iProp < pPropSet->cProperties; iProp++, pProp++, pPropDest++)
		{
			DuplicateDBID(pProp->colid, &pPropDest->colid);
			pPropDest->dwOptions	= pProp->dwOptions;
			pPropDest->dwPropertyID	= pProp->dwPropertyID;
			pPropDest->dwStatus		= pProp->dwStatus;
			VariantCopy(&pPropDest->vValue, &pProp->vValue);
		}
	}

CLEANUP:
	TRETURN
} //CPropSets::Clone = 




HRESULT CPropSets::PrintPropSets()
{
	ULONG		index, indexSet;
	DBPROPSET	*pPropSet;
	DBPROP		*pProp;
	WCHAR		wszPropValue[cMaxName];

	g_nTabs++;
	Ident();
	odtLog << "<DBPROPSET_ARRAY cPropertySets = " << cPropertySets() << " rgPropertySets = " << (ULONG_PTR)pPropertySets() << ">\n";

	g_nTabs++;

	for (	indexSet = 0, pPropSet = pPropertySets(); 
			pPropSet && indexSet < cPropertySets(); 
			indexSet++, pPropSet++)
	{
		// print property set info
		Ident();
		odtLog << "<DBPROPSET id = \"DBPROPSET:" << indexSet 
			<< "\" guidPropertySet = " << GetPropSetName(pPropSet->guidPropertySet)
			<< "cProperties = " << pPropSet->cProperties << " rgProperties = " << (ULONG_PTR)pPropSet->rgProperties
			<< ">\n";
		g_nTabs++;

		for (	index = 0, pProp = pPropSet->rgProperties; 
				pProp && index < pPropSet->cProperties;
				index++, pProp++)
		{
			Ident();
			if (CHECK(VariantToString(&pProp->vValue, wszPropValue, cMaxName), S_OK))
				odtLog << "<DBPROP id = \"DBPROP:" << index
					<< "\" dwPropertyID = " << GetPropertyName(pProp->dwPropertyID, pPropSet->guidPropertySet)
					<< " vValue = " << wszPropValue
					<< "> </DBPROP>\n";
		}

		g_nTabs--;
		Ident();
		odtLog << "</DBPROPSET>\n";
	}


	g_nTabs--;
	Ident();
	odtLog << "</DBPROPSET_ARRAY>\n";
	g_nTabs--;
	return S_OK;
} //CPropSets::PrintPropSets




BOOL CPropSets::ArePropsIncluded(DBORDINAL cPropSets, DBPROPSET *rgPropSets)
{
	DBORDINAL	cPropSetIndex;
	DBORDINAL	cPropIndex;
	DBPROPSET	*pPropSet;
	DBPROP		*pProp;

	if (	(0 < cPropSets) 
		&&	(NULL == rgPropSets))
		return FALSE;

	pPropSet = rgPropSets;
	for (cPropSetIndex = 0; cPropSetIndex < cPropSets; cPropSetIndex++, pPropSet++)
	{
		pProp = pPropSet->rgProperties;
		if (	(0 < pPropSet->cProperties)
			&&	(NULL == pPropSet->rgProperties))
			return FALSE;

		for (cPropIndex = 0; cPropIndex < pPropSet->cProperties; cPropIndex++, pProp++)
		{
			DBPROP		*pThisProp = NULL;

			if (VT_EMPTY == pProp->vValue.vt)
				continue;

			// look for property in these prop sets
			pThisProp = FindProperty(pProp->dwPropertyID, pPropSet->guidPropertySet);

			if (VT_EMPTY == pThisProp->vValue.vt)
				return FALSE;

			if (!CompareVariant(&pThisProp->vValue, &pProp->vValue))
				return FALSE;
		}
	}

	return TRUE;
} //CPropSets::ArePropsIncluded



CWString CPropSets::ConvertToCWString()
{
	CWString	PropSets;

	DBORDINAL	cPropSetIndex;
	DBORDINAL	cPropIndex;
	DBPROPSET	*pPropSet;
	DBPROP		*pProp;

	if (!m_pPropInfoSets)
		goto CLEANUP;

	pPropSet = pPropertySets();
	TESTC(pPropSet || 0 == cPropertySets());

	for (cPropSetIndex = 0; cPropSetIndex < cPropertySets(); cPropSetIndex++, pPropSet++)
	{
		pProp = pPropSet->rgProperties;
		if (	(0 < pPropSet->cProperties)
			&&	(NULL == pPropSet->rgProperties))
			continue;

		for (cPropIndex = 0; cPropIndex < pPropSet->cProperties; cPropIndex++, pProp++)
		{
			DBPROPINFO			*pPropInfo;
			VARIANT				vValue;

			VariantInit(&vValue);
			// find property name
			pPropInfo = m_pPropInfoSets->FindProperty(pProp->dwPropertyID, pPropSet->guidPropertySet);
			if (pPropInfo && VT_EMPTY != pProp->vValue.vt)
			{
				PropSets + pPropInfo->pwszDescription;
				PropSets + L"=";	
				VariantChangeType(&vValue, &pProp->vValue, VARIANT_NOVALUEPROP, VT_BSTR);
				PropSets + V_BSTR(&vValue);
				PropSets + L"; ";
			}
		}
	}

CLEANUP:
	return PropSets;
} //CPropSets::ConvertToCWString




HRESULT CPropSets::RemoveProperty(DBPROPID dwPropertyID, GUID guidPropertySet)
{
	ULONG				indexSet = GetIndex(guidPropertySet);
	ULONG				index;
	CVectorEx<DBPROP>	*pWrapper;

	if (cPropertySets() <= indexSet)
		return E_FAIL;
	
	pWrapper = GetSetWrapper(indexSet);

	for (	index = 0; 
			index < m_PropSets[indexSet].cProperties 
				&& dwPropertyID != m_PropSets[indexSet].rgProperties[index].dwPropertyID; 
			index++);

	if (index >= m_PropSets[indexSet].cProperties)
		return E_FAIL;

	pWrapper->RemoveAt(index);
	m_PropSets[indexSet].cProperties--;
	memset(m_PropSets[indexSet].rgProperties + pWrapper->GetCount(), 0, sizeof(DBPROP));
	return S_OK;
} //CPropSets::RemoveProperty










CPropInfoSets &CPropInfoSets::operator = (CPropInfoSets &X)
{
	ULONG			iSet;
	ULONG			iPropInfo;
	DBPROPINFO		*pPropInfo;
	DBPROPINFOSET	*pPropInfoSet;
	DBPROPINFO		*pDestPropInfo;
	DBPROPINFOSET	*pDestPropInfoSet;
	size_t			ulBufSize = 0;
	WCHAR			*pwszCrtDesc = NULL;
	
	Free();

	SAFE_ALLOC(m_rgPropInfoSets, DBPROPINFOSET, X.cPropInfoSets());
	m_cPropInfoSets = X.cPropInfoSets();

	for (	ulBufSize = 0, iSet = 0, pPropInfoSet = X.pPropInfoSets();  
			iSet < m_cPropInfoSets; 
			iSet++, pPropInfoSet++)
	{
		for (	iPropInfo = 0, pPropInfo = pPropInfoSet->rgPropertyInfos; 
				iPropInfo < pPropInfoSet->cPropertyInfos; 
				iPropInfo++, pPropInfo++)
		{
			ulBufSize += wcslen(pPropInfo->pwszDescription) + 1;
		}
	}

	SAFE_ALLOC(m_pwszBuffer, WCHAR, ulBufSize);

	for (	iSet = 0, pPropInfoSet = X.pPropInfoSets(), pDestPropInfoSet = m_rgPropInfoSets, pwszCrtDesc = m_pwszBuffer;  
			iSet < m_cPropInfoSets; 
			iSet++, pPropInfoSet++, pDestPropInfoSet++)
	{
		pDestPropInfoSet->guidPropertySet	= pPropInfoSet->guidPropertySet;
		pDestPropInfoSet->cPropertyInfos	= pPropInfoSet->cPropertyInfos;
		SAFE_ALLOC(pDestPropInfoSet->rgPropertyInfos, DBPROPINFO, pDestPropInfoSet->cPropertyInfos);

		for (	iPropInfo = 0, pPropInfo = pPropInfoSet->rgPropertyInfos, pDestPropInfo = pDestPropInfoSet->rgPropertyInfos; 
				iPropInfo < pPropInfoSet->cPropertyInfos; 
				iPropInfo++, pPropInfo++, pDestPropInfo++)
		{
			pDestPropInfo->dwFlags		= pPropInfo->dwFlags;
			pDestPropInfo->dwPropertyID	= pPropInfo->dwPropertyID;
			pDestPropInfo->vtType		= pPropInfo->vtType;
			VariantInit(&pDestPropInfo->vValues);
			VariantCopy(&pDestPropInfo->vValues, &pPropInfo->vValues);

			pDestPropInfo->pwszDescription = pwszCrtDesc;
			wcscpy(pwszCrtDesc, pPropInfo->pwszDescription);
			pwszCrtDesc += wcslen(pPropInfo->pwszDescription) + 1;
		}
	}


CLEANUP:
	return *this;
} //CPropInfoSets::operator = 




void CPropInfoSets::Free()
{
	ULONG	indexSet;
	ULONG	index;

	if (m_rgPropInfoSets != NULL)
	{
		for (indexSet = 0; indexSet < m_cPropInfoSets; indexSet++)
		{
			// release the current prop info set
			for (index = 0; index < m_rgPropInfoSets[indexSet].cPropertyInfos; index++)
			{
				DBPROPINFO	*pPropInfo = &(m_rgPropInfoSets[indexSet].rgPropertyInfos[index]);
				VariantClear(&pPropInfo->vValues);
			}
			SAFE_FREE(m_rgPropInfoSets[indexSet].rgPropertyInfos);
		}
	}

	SAFE_FREE(m_rgPropInfoSets);
	SAFE_FREE(m_pwszBuffer);
	m_cPropInfoSets = 0;
} //CPropInfoSets::Free




LONG CPropInfoSets::GetSetIndex(GUID guidPropSet)
{
	ULONG	indexSet;

	for (indexSet = 0; indexSet < m_cPropInfoSets; indexSet++)
	{
		if (guidPropSet == m_rgPropInfoSets[indexSet].guidPropertySet)
			return indexSet;
	}

	return -1;
} //CPropInfoSets::GetSetIndex




DBPROPINFOSET &CPropInfoSets::operator [] (GUID guidPropSet)
{
	LONG	indexSet = GetSetIndex(guidPropSet);

	if (-1 == indexSet)
		throw;

	return m_rgPropInfoSets[indexSet];
} //PropInfoSets::operator [] 




HRESULT CPropInfoSets::CreatePropInfoSet(IUnknown *pIUnknown)
{
	HRESULT			hr = S_OK;
	IDBProperties	*pIDBProperties = NULL;

	Free();

	if (VerifyInterface(pIUnknown, IID_IDBProperties, DATASOURCE_INTERFACE, (IUnknown**)&pIDBProperties))
	{
		TESTC_(hr = pIDBProperties->GetPropertyInfo(0, NULL, &m_cPropInfoSets, &m_rgPropInfoSets, &m_pwszBuffer), S_OK);
	}
	else
	{
		ASSERT(0 == m_cPropInfoSets);
		ASSERT(NULL == m_rgPropInfoSets);
		ASSERT(NULL == m_pwszBuffer);
	}
	
CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	return hr;
} //PropInfoSets::CreatePropInfoSet
	


DBPROPINFO *CPropInfoSets::FindProperty(DBPROPID PropertyID, GUID guidPropertySet)
{
	ULONG		index;
	LONG		indexSet	= GetSetIndex(guidPropertySet);
	DBPROPINFO	*pPropInfo	= NULL;

	if (-1 == indexSet)
		return NULL;

	pPropInfo = m_rgPropInfoSets[indexSet].rgPropertyInfos;

	for (index = 0; index < m_rgPropInfoSets[indexSet].cPropertyInfos; index++, pPropInfo++)
	{
		if (PropertyID == pPropInfo->dwPropertyID)
			return pPropInfo;
	}
	return NULL;
} //CPropInfoSets::FindProperty



DBPROPINFO *CPropInfoSets::FindProperty(WCHAR *pwszDesc, GUID guidPropertySet)
{
	ULONG		index;
	LONG		indexSet	= GetSetIndex(guidPropertySet);
	DBPROPINFO	*pPropInfo	= NULL;

	if (!pwszDesc || -1 == indexSet)
		return NULL;

	pPropInfo = m_rgPropInfoSets[indexSet].rgPropertyInfos;

	for (index = 0; index < m_rgPropInfoSets[indexSet].cPropertyInfos; index++, pPropInfo++)
	{
		if (pPropInfo->pwszDescription && 0 == wcscmp(pPropInfo->pwszDescription, pwszDesc))
			return pPropInfo;
	}
	return NULL;
} //CPropInfoSets::FindProperty




// this method is used to look for a property when only the property description is known
BOOL CPropInfoSets::FindProperty(WCHAR *pwszDesc, DBPROPINFO **ppPropInfo, GUID *pguidPropertySet)
{
	TBEGIN
	ULONG		index;
	ULONG		indexSet;

	TESTC(NULL != ppPropInfo);
	TESTC(NULL != pguidPropertySet);
	TESTC(NULL != pwszDesc);

	for (indexSet = 0; indexSet < m_cPropInfoSets; indexSet++)
	{
		*ppPropInfo			= m_rgPropInfoSets[indexSet].rgPropertyInfos;
		*pguidPropertySet	= m_rgPropInfoSets[indexSet].guidPropertySet;

		for (index = 0; index < m_rgPropInfoSets[indexSet].cPropertyInfos; index++, (*ppPropInfo)++)
		{
			if ((*ppPropInfo)->pwszDescription && 0 == wcscmp((*ppPropInfo)->pwszDescription, pwszDesc))
				return TRUE;
		}
	}
	
CLEANUP:
	*ppPropInfo			= NULL;
	*pguidPropertySet	= GUID_NULL;
	TRETURN
} //CPropInfoSets::FindProperty




// Finds a property of a given type
BOOL CPropInfoSets::FindProperty(
	VARTYPE			vtType,						// [in]  the type of the sought property
	DBPROPINFO		**ppPropInfo,				// [out] the property pointer
	GUID			*pguidPropSet,				// [out] the propset guid
	ULONG			cExclProp/* = 0*/,			// [in]  the number of props to be excluded 
	DBPROPID		*rgExclPropID/* = NULL*/,	// [in]  the props to be excluded
	GUID			*rgExclPropSet/* = NULL*/	// [in]  the propset of the corresponding props
)
{
	BOOL			fFound = FALSE;
	DBPROPINFO		*pPropInfo;
	DBPROPINFOSET	*pPropInfoSet;
	ULONG			ulPropSet;
	ULONG			ulProp;
	ULONG			cExclIndx;

	TESTC(NULL != ppPropInfo);
	TESTC(NULL != pguidPropSet);
	TESTC(0 == cExclProp || NULL != rgExclPropID && NULL != rgExclPropSet);

	*ppPropInfo		= NULL;
	memset(pguidPropSet, 0, sizeof(GUID));

	for (ulPropSet = 0; ulPropSet < m_cPropInfoSets; ulPropSet++)
	{
		pPropInfoSet = &m_rgPropInfoSets[ulPropSet];
		for (ulProp = 0; ulProp < pPropInfoSet->cPropertyInfos; ulProp++)
		{
			pPropInfo = &pPropInfoSet->rgPropertyInfos[ulProp];
			if (vtType == pPropInfo->vtType)
			{
				for (cExclIndx = 0; cExclIndx < cExclProp; cExclIndx++)
				{
					if (rgExclPropSet[cExclIndx] == pPropInfoSet->guidPropertySet
						&& rgExclPropID[cExclIndx] == pPropInfo->dwPropertyID)
						goto LOOP;
				}

				*ppPropInfo		= pPropInfo;
				*pguidPropSet	= pPropInfoSet->guidPropertySet;
				return TRUE;
			}
LOOP:
	;
		}
	}

CLEANUP:
	return fFound;
} //CPropInfoSets::FindProperty




BOOL CPropInfoSets::SettableProperty(DBPROPID PropertyID, GUID guidPropertySet)
{
	DBPROPINFO	*pPropInfo = FindProperty(PropertyID, guidPropertySet);

	return pPropInfo && (pPropInfo->dwFlags & DBPROPFLAGS_WRITE) 
		&& !(pPropInfo->dwFlags & DBPROPFLAGS_NOTSUPPORTED);
} //CPropInfoSets::SettableProperty




BOOL CPropInfoSets::SupportedProperty(DBPROPID PropertyID, GUID guidPropertySet)
{
	DBPROPINFO	*pPropInfo = FindProperty(PropertyID, guidPropertySet);

	return pPropInfo && !(pPropInfo->dwFlags & DBPROPFLAGS_NOTSUPPORTED);
} //CPropInfoSets::SupportedProperty




HRESULT CProvPropSets::SetProperty(
	GUID			guidProvider, 
	DBPROPID		PropertyID, 
	GUID			guidPropertySet, 
	VARIANT			*pv, 
	DBPROPOPTIONS	dwOptions/* = DBPROPOPTIONS_REQUIRED*/, 
	DBID			colid/* = DB_NULLID*/
)
{
	LONG		index = GetProviderIndex(guidProvider);

	if (-1 >= index)
	{
		return E_FAIL;
	}
	
	return m_rgPropSets[index].SetProperty(PropertyID, guidPropertySet, pv, dwOptions, colid);
} //CPropInfoSets::SetProperty






#endif //__PROPSETS__
