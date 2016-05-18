//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module PROPERTY.CPP
//
//-----------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "Headers.h"


///////////////////////////////////////////////////////////////
// TPropBase
//
///////////////////////////////////////////////////////////////
template <class TYPE> TPropBase<TYPE>::TPropBase(ULONG cElements, TYPE* rgElements)
{
	if(cElements)
		Attach(cElements, rgElements);
}

///////////////////////////////////////////////////////////////
// ~TPropBase
//
///////////////////////////////////////////////////////////////
template <class TYPE> TPropBase<TYPE>::~TPropBase()
{
	RemoveAll();
}


///////////////////////////////////////////////////////////////
// TPropBase::AddProperty
//
///////////////////////////////////////////////////////////////
template <class TYPE> TYPE* TPropBase<TYPE>::AddProperty(DBPROPID dwPropertyID, const DBID& colid)
{
	//See if the property exists...
	TYPE* pProp = FindProperty(dwPropertyID, colid);
	if(!pProp)
	{
		TYPE dbProp;
		memset(&dbProp, 0, sizeof(TYPE));

		//Add this property...
		pProp = AddElement(dbProp);
	}

	return pProp;
}


///////////////////////////////////////////////////////////////
// CProperties
//
///////////////////////////////////////////////////////////////
CProperties::CProperties(DBPROPSET* pPropSet)
{
	Attach(pPropSet);
}

///////////////////////////////////////////////////////////////
// ~CProperties
//
///////////////////////////////////////////////////////////////
CProperties::~CProperties()
{
}


///////////////////////////////////////////////////////////////
// CProperties::Attach
//
///////////////////////////////////////////////////////////////
void CProperties::Attach(DBPROPSET* pPropSet)
{
	//Delegate
	if(pPropSet)
		CVector<DBPROP>::Attach(pPropSet->cProperties, pPropSet->rgProperties);
}

///////////////////////////////////////////////////////////////
// CProperties::Detach
//
///////////////////////////////////////////////////////////////
void CProperties::Detach(DBPROPSET* pPropSet)
{
	//Delegate
	CVector<DBPROP>::Detach(&pPropSet->cProperties, &pPropSet->rgProperties);
}


///////////////////////////////////////////////////////////////
// CProperties::RemoveAll
//
///////////////////////////////////////////////////////////////
void CProperties::RemoveAll()
{
	//We need to walk and free out-of-line data...
	for(ULONG iProp=0; iProp<m_cElements; iProp++)
		VariantClearFast(&m_rgElements[iProp].vValue);

	//Delegate
	CVector<DBPROP>::RemoveAll();
}


///////////////////////////////////////////////////////////////
// CProperties::FindProperty
//
///////////////////////////////////////////////////////////////
DBPROP*	CProperties::FindProperty(DBPROPID dwPropertyID, const DBID& colid)
{
	//Loop over the array of properties in this set
	for(ULONG iProp=0; iProp<m_cElements; iProp++)
	{
		DBPROP* pProp = &m_rgElements[iProp];
		if(pProp->dwPropertyID == dwPropertyID && DBIDEqual(&pProp->colid, &colid))
			return pProp;
	}

	return NULL;
}


///////////////////////////////////////////////////////////////
// CProperties::SetProperty
//
///////////////////////////////////////////////////////////////
HRESULT	CProperties::SetProperty(DBPROPID dwPropertyID, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions, const DBID& colid)
{
	HRESULT hr = S_OK;

	//Add this property (if it doesn't already exist)
	DBPROP* pProp = AddProperty(dwPropertyID, colid);
	if(!pProp)
		TESTC(hr = E_OUTOFMEMORY);

	//Free any previous value
	VariantClearFast(&pProp->vValue);
	DBIDFree(&pProp->colid);

	//Setup the new property
	pProp->dwPropertyID = dwPropertyID;
	pProp->dwOptions    = dwOptions;
	pProp->dwStatus     = DBPROPSTATUS_OK;
	
	//Copy the colid
	DBIDCopy(&pProp->colid, &colid);

	//Copy the Value
	pProp->vValue.vt    = wType;
	switch(wType)
	{
		case DBTYPE_BOOL:
			V_BOOL(&pProp->vValue)		= (VARIANT_BOOL)pv;
			break;
		
		case DBTYPE_I2:
			V_I2(&pProp->vValue)		= (SHORT)(ULONG_PTR)(pv);
			break;

		case DBTYPE_I4:
			V_I4(&pProp->vValue)		= (LONG)(ULONG_PTR)(pv);
			break;

#ifdef _WIN64
		case DBTYPE_I8:
//TODO64:	V_I8(&pProp->vValue)		= (LONG_PTR)pv;
			pProp->vValue.ullVal		= (LONG_PTR)pv;
			break;
#endif //_WIN64

		case DBTYPE_WSTR:
		case DBTYPE_BSTR:
			V_BSTR(&pProp->vValue)		= SysAllocString((BSTR)pv);
			break;

		case DBTYPE_VARIANT:
			XTEST(VariantCopyFast(&pProp->vValue, (VARIANT*)pv));
			break;

		default:
			ASSERT(FALSE); //Unhandled property type
			TESTC(hr = E_INVALIDARG); 
			break;
	}

CLEANUP:
	return hr;
}



///////////////////////////////////////////////////////////////
// CPropertyInfos
//
///////////////////////////////////////////////////////////////
CPropertyInfos::CPropertyInfos(DBPROPINFOSET* pPropSet)
{
	Attach(pPropSet);
}

///////////////////////////////////////////////////////////////
// ~CPropertyInfos
//
///////////////////////////////////////////////////////////////
CPropertyInfos::~CPropertyInfos()
{
}


///////////////////////////////////////////////////////////////
// CPropertyInfos::Attach
//
///////////////////////////////////////////////////////////////
void CPropertyInfos::Attach(DBPROPINFOSET* pPropSet)
{
	//Delegate
	if(pPropSet)
		CVector<DBPROPINFO>::Attach(pPropSet->cPropertyInfos, pPropSet->rgPropertyInfos);
}


///////////////////////////////////////////////////////////////
// CPropertyInfos::Detach
//
///////////////////////////////////////////////////////////////
void CPropertyInfos::Detach(DBPROPINFOSET* pPropSet)
{
	//Delegate
	CVector<DBPROPINFO>::Detach(&pPropSet->cPropertyInfos, &pPropSet->rgPropertyInfos);
}


///////////////////////////////////////////////////////////////
// CPropertyInfos::RemoveAll
//
///////////////////////////////////////////////////////////////
void CPropertyInfos::RemoveAll()
{
	//We need to walk and free out-of-line data...
	for(ULONG iProp=0; iProp<m_cElements; iProp++)
		VariantClearFast(&m_rgElements[iProp].vValues);

	//Delegate
	CVector<DBPROPINFO>::RemoveAll();
}


///////////////////////////////////////////////////////////////
// CPropertyInfos::FindProperty
//
///////////////////////////////////////////////////////////////
DBPROPINFO*	CPropertyInfos::FindProperty(DBPROPID dwPropertyID, const DBID& colid)
{
	//Loop over the array of properties in this set
	for(ULONG iPropInfo=0; iPropInfo<m_cElements; iPropInfo++)
	{
		DBPROPINFO* pPropInfo = &m_rgElements[iPropInfo];
		if(pPropInfo->dwPropertyID == dwPropertyID)
			return pPropInfo;
	}

	return NULL;
}




///////////////////////////////////////////////////////////////
// CPropertyIDs
//
///////////////////////////////////////////////////////////////
CPropertyIDs::CPropertyIDs(DBPROPIDSET* pPropSet)
{
	Attach(pPropSet);
}

///////////////////////////////////////////////////////////////
// ~CPropertyIDs
//
///////////////////////////////////////////////////////////////
CPropertyIDs::~CPropertyIDs()
{
}


///////////////////////////////////////////////////////////////
// CPropertyIDs::Attach
//
///////////////////////////////////////////////////////////////
void CPropertyIDs::Attach(DBPROPIDSET* pPropSet)
{
	//Delegate
	if(pPropSet)
		CVector<DBPROPID>::Attach(pPropSet->cPropertyIDs, pPropSet->rgPropertyIDs);
}


///////////////////////////////////////////////////////////////
// CPropertyIDs::Detach
//
///////////////////////////////////////////////////////////////
void CPropertyIDs::Detach(DBPROPIDSET* pPropSet)
{
	//Delegate
	CVector<DBPROPID>::Detach(&pPropSet->cPropertyIDs, &pPropSet->rgPropertyIDs);
}


///////////////////////////////////////////////////////////////
// CPropertyIDs::FindProperty
//
///////////////////////////////////////////////////////////////
DBPROPID*	CPropertyIDs::FindProperty(DBPROPID dwPropertyID, const DBID& colid)
{
	//Loop over the array of properties in this set
	for(ULONG iProp=0; iProp<m_cElements; iProp++)
	{
		DBPROPID* pPropID = &m_rgElements[iProp];
		if(*pPropID == dwPropertyID)
			return pPropID;
	}

	return NULL;
}


///////////////////////////////////////////////////////////////
// CPropertyIDs::SetProperty
//
///////////////////////////////////////////////////////////////
HRESULT	CPropertyIDs::SetProperty(DBPROPID dwPropertyID)
{
	HRESULT hr = S_OK;

	//Add this property (if it doesn't already exist)
	DBPROPID* pPropID = AddProperty(dwPropertyID, DB_NULLID);
	if(!pPropID)
		TESTC(hr = E_OUTOFMEMORY);

	//Setup the value
	*pPropID = dwPropertyID;

CLEANUP:
	return hr;
}




///////////////////////////////////////////////////////////////
// TPropSetBase
//
///////////////////////////////////////////////////////////////
template <class TYPE, class OBJTYPE> TPropSetBase<TYPE, OBJTYPE>::TPropSetBase(ULONG cElements, TYPE* rgElements)
{
	if(cElements)
		Attach(cElements, rgElements);
}

///////////////////////////////////////////////////////////////
// ~TPropSetBase
//
///////////////////////////////////////////////////////////////
template <class TYPE, class OBJTYPE> TPropSetBase<TYPE, OBJTYPE>::~TPropSetBase()
{
	RemoveAll();
}


///////////////////////////////////////////////////////////////
// TPropSetBase::RemoveAll
//
///////////////////////////////////////////////////////////////
template <class TYPE, class OBJTYPE> void TPropSetBase<TYPE, OBJTYPE>::RemoveAll()
{
	 //We need to walk and free out-of-line data...
	::FreeProperties(&m_cElements, &m_rgElements);

	//Delegate
	CVector<TYPE>::RemoveAll();
}


///////////////////////////////////////////////////////////////
// TPropSetBase::AddPropSet
//
///////////////////////////////////////////////////////////////
template <class TYPE, class OBJTYPE> TYPE* TPropSetBase<TYPE, OBJTYPE>::AddPropSet(REFGUID guidPropertySet)
{
	//See if the property set already exists...
	TYPE* pPropSet = FindPropSet(guidPropertySet);
	if(!pPropSet)
	{
		TYPE dbPropSet;
		memset(&dbPropSet, 0, sizeof(TYPE));
		dbPropSet.guidPropertySet = guidPropertySet;

		//Add this property set
		pPropSet = AddElement(dbPropSet);
	}

	return pPropSet;
}


///////////////////////////////////////////////////////////////
// TPropSetBase::FindPropSet
//
///////////////////////////////////////////////////////////////
template <class TYPE, class OBJTYPE> TYPE* TPropSetBase<TYPE, OBJTYPE>::FindPropSet(REFGUID guidPropertySet)
{
	//Loop over the number of property sets
	for(ULONG iPropSet=0; iPropSet<m_cElements; iPropSet++)
	{
		TYPE* pPropSet = &m_rgElements[iPropSet];

		//Make sure where looking in the right property set
		if(guidPropertySet == pPropSet->guidPropertySet)
			return pPropSet;
	}

	return NULL;
}


///////////////////////////////////////////////////////////////
// CPropSets
//
///////////////////////////////////////////////////////////////
CPropSets::CPropSets(ULONG cPropSets, DBPROPSET* rgPropSets)
	: TPropSetBase<DBPROPSET, CProperties>(cPropSets, rgPropSets)
{
}

///////////////////////////////////////////////////////////////
// ~CPropSets
//
///////////////////////////////////////////////////////////////
CPropSets::~CPropSets()
{
}


///////////////////////////////////////////////////////////////
// CPropSets::SetProperty
//
///////////////////////////////////////////////////////////////
HRESULT	CPropSets::SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions, const DBID& colid)
{
	//Add this property set (if doesn't already exist)
	DBPROPSET* pPropSet = AddPropSet(guidPropertySet);
	if(!pPropSet)
		return E_OUTOFMEMORY;

	//Delegate
	CProperties sCProperties(pPropSet);	
	HRESULT hr = sCProperties.SetProperty(dwPropertyID, wType, pv, dwOptions, colid);
	sCProperties.Detach(pPropSet);

	return hr;
}


///////////////////////////////////////////////////////////////
// CPropSets::FindProperty
//
///////////////////////////////////////////////////////////////
DBPROP* CPropSets::FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, const DBID& colid)
{
	//Find the property set...
	DBPROPSET* pPropSet = FindPropSet(guidPropertySet);
	if(pPropSet)
	{
		CProperties sCProperties(pPropSet);	
		DBPROP* pProp = sCProperties.FindProperty(dwPropertyID, colid);
		sCProperties.Detach(pPropSet);

		return pProp;
	}

	return NULL;
}




///////////////////////////////////////////////////////////////
// CPropInfoSets
//
///////////////////////////////////////////////////////////////
CPropInfoSets::CPropInfoSets(ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets)
: TPropSetBase<DBPROPINFOSET, CPropertyInfos>(cPropInfoSets, rgPropInfoSets)
{
}

///////////////////////////////////////////////////////////////
// ~CPropInfoSets
//
///////////////////////////////////////////////////////////////
CPropInfoSets::~CPropInfoSets()
{
}


///////////////////////////////////////////////////////////////
// CPropInfoSets::FindProperty
//
///////////////////////////////////////////////////////////////
DBPROPINFO* CPropInfoSets::FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet)
{
	//Find the property set...
	DBPROPINFOSET* pPropSet = FindPropSet(guidPropertySet);
	if(pPropSet)
	{
		CPropertyInfos sCProperties(pPropSet);	
		DBPROPINFO* pProp = sCProperties.FindProperty(dwPropertyID, DB_NULLID);
		sCProperties.Detach(pPropSet);

		return pProp;
	}

	return NULL;
}



///////////////////////////////////////////////////////////////
// CPropIDSets
//
///////////////////////////////////////////////////////////////
CPropIDSets::CPropIDSets(ULONG cPropIDSets, DBPROPIDSET* rgPropIDSets)
	: TPropSetBase<DBPROPIDSET, CPropertyIDs>(cPropIDSets, rgPropIDSets)
{
}

///////////////////////////////////////////////////////////////
// ~CPropIDSets
//
///////////////////////////////////////////////////////////////
CPropIDSets::~CPropIDSets()
{
}


///////////////////////////////////////////////////////////////
// CPropIDSets::SetProperty
//
///////////////////////////////////////////////////////////////
HRESULT	CPropIDSets::SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet)
{
	//Add this property set (if doesn't already exist)
	DBPROPIDSET* pPropSet = AddPropSet(guidPropertySet);
	if(!pPropSet)
		return E_OUTOFMEMORY;

	//Delegate
	CPropertyIDs sCProperties(pPropSet);	
	HRESULT hr = sCProperties.SetProperty(dwPropertyID);
	sCProperties.Detach(pPropSet);

	return hr;
}


///////////////////////////////////////////////////////////////
// CPropIDSets::FindProperty
//
///////////////////////////////////////////////////////////////
DBPROPID* CPropIDSets::FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet)
{
	//Find the property set...
	DBPROPIDSET* pPropSet = FindPropSet(guidPropertySet);
	if(pPropSet)
	{
		CPropertyIDs sCProperties(pPropSet);	
		DBPROPID* pProp = sCProperties.FindProperty(dwPropertyID, DB_NULLID);
		sCProperties.Detach(pPropSet);

		return pProp;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////////
//  Find the Property Set within the PropInfoSets and return a pointer to it
//
////////////////////////////////////////////////////////////////////////////
DBPROPINFOSET* FindPropSet(REFGUID guidPropertySet, ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets)
{
	//Loop over the property sets
	for(ULONG iPropSet=0; iPropSet<cPropInfoSets; iPropSet++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[iPropSet];

		//Make sure where looking in the right property set
		if(guidPropertySet == pPropInfoSet->guidPropertySet)
			return pPropInfoSet;
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////////
//  Find the Property within the PropInfoSets and return a pointer to it
//
////////////////////////////////////////////////////////////////////////////
DBPROPINFO* FindProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets)
{
	//Loop over the number of property sets
	for(ULONG iPropSet=0; iPropSet<cPropInfoSets; iPropSet++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[iPropSet];

		//Make sure where looking in the right property set
		if(guidPropertySet != pPropInfoSet->guidPropertySet)
			continue;

		//Loop over the array of properties in this set
		for(ULONG iProp=0; iProp<pPropInfoSet->cPropertyInfos; iProp++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[iProp];
			if(pPropInfo->dwPropertyID == dwPropertyID)
				return pPropInfo;
		}
	}

	return NULL;
}


////////////////////////////////////////////////////////////////////////////
//  CombineProperties
//
////////////////////////////////////////////////////////////////////////////
HRESULT CombineProperties(ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, ULONG cPropInfoSets2, DBPROPINFOSET* rgPropInfoSets2, BOOL bFreeAddedPropSet)
{
	ASSERT(pcPropInfoSets);
	ASSERT(prgPropInfoSets);
	HRESULT hr = S_OK;

	//Make our lives a little easier...
	ULONG cPropInfoSets = *pcPropInfoSets;
	DBPROPINFOSET* rgPropInfoSets = *prgPropInfoSets;
	
	//Now combine both sets...
	if(cPropInfoSets2 && rgPropInfoSets2)
	{
		if(cPropInfoSets && rgPropInfoSets)
		{
			//Loop over all the provider returned properties, and see if they 
			//already exist in the OLE DB Defined properties.
			for(ULONG iPropSet=0; iPropSet<cPropInfoSets2; iPropSet++)
			{
				DBPROPINFOSET* pPropInfoSet2 = &rgPropInfoSets2[iPropSet];
				DBPROPINFOSET* pFoundSet = FindPropSet(pPropInfoSet2->guidPropertySet, cPropInfoSets, rgPropInfoSets);
				
				if(pFoundSet)
				{
					//Add all properties of this set...
					for(ULONG iProp=0; iProp<pPropInfoSet2->cPropertyInfos; iProp++)
					{
						DBPROPINFO* pPropInfo2 = &pPropInfoSet2->rgPropertyInfos[iProp];
						DBPROPINFO* pFoundProp = FindProperty(pPropInfo2->dwPropertyID, pPropInfoSet2->guidPropertySet, 1, pFoundSet);
						
						if(pFoundProp)
						{
							//Found the propertyset as well as the property
							//just need to update it in the list...
							*pFoundProp = *pPropInfo2;
						}
						else
						{
							//Found the property set, but not the property
							//just need to add this property to the list...
							SAFE_REALLOC(pFoundSet->rgPropertyInfos, DBPROPINFO, pFoundSet->cPropertyInfos + 1);
							pFoundSet->rgPropertyInfos[pFoundSet->cPropertyInfos] = *pPropInfo2;
							pFoundSet->cPropertyInfos++;
						}

						//Do we need to make a copy of the variant...
						if(!bFreeAddedPropSet)
						{
							//TODO
							//Also what about the stringbuffer with property descriptions?
						}
					}
				}
				else
				{
					//Need to add the entire property set, as well as all the properties.
					SAFE_REALLOC(rgPropInfoSets, DBPROPINFOSET, cPropInfoSets + 1);
					rgPropInfoSets[cPropInfoSets].guidPropertySet	= pPropInfoSet2->guidPropertySet;
					rgPropInfoSets[cPropInfoSets].cPropertyInfos	= pPropInfoSet2->cPropertyInfos;

					//Add all the properties of this set...
					SAFE_ALLOC(rgPropInfoSets[cPropInfoSets].rgPropertyInfos, DBPROPINFO, pPropInfoSet2->cPropertyInfos);
					for(ULONG iProp=0; iProp<pPropInfoSet2->cPropertyInfos; iProp++)
					{
						rgPropInfoSets[cPropInfoSets].rgPropertyInfos[iProp] = pPropInfoSet2->rgPropertyInfos[iProp];

						//Do we need to make a copy of the variant...
						if(!bFreeAddedPropSet)
						{
							//TODO
							//Also what about the stringbuffer with property descriptions?
						}
					}
					cPropInfoSets++;
				}

				//We are done with this array of properties...
				if(bFreeAddedPropSet)
					SAFE_FREE(pPropInfoSet2->rgPropertyInfos);
			}

			//NOTE: In the process of combining both of these sets, we are doing
			//direct memory copy of the internal DBPROPINFO items, this way
			//we don't need to variant copy all the values, and duplicate all the 
			//strings.  But this also requires that we don't free the source of the 
			//copies as well.  So we will finally end up freeing the combined 
			//property sets (cPropInfoSets) but not (cPropInfoSets2)
			if(bFreeAddedPropSet)
				SAFE_FREE(rgPropInfoSets2);
		}
		else
		{
			cPropInfoSets = cPropInfoSets2;
			rgPropInfoSets = rgPropInfoSets2;
		}
	}

CLEANUP:
	*pcPropInfoSets = cPropInfoSets;
	*prgPropInfoSets = rgPropInfoSets;
	return hr;
}
	

////////////////////////////////////////////////////////////////////////////
//  CombineProperties
//
////////////////////////////////////////////////////////////////////////////
HRESULT CombineProperties(ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, ULONG cPropSets2, DBPROPSET* rgPropSets2, BOOL bFreeAddedPropSet)
{
	ASSERT(pcPropInfoSets);
	ASSERT(prgPropInfoSets);
	HRESULT hr = S_OK;

	//Make our lives a little easier...
	ULONG cPropInfoSets = *pcPropInfoSets;
	DBPROPINFOSET* rgPropInfoSets = *prgPropInfoSets;
	
	//Now combine both sets...
	if(cPropSets2 && rgPropSets2)
	{
		//Loop over all the provider returned properties, and see if they 
		//already exist in the OLE DB Defined properties.
		for(ULONG iPropSet=0; iPropSet<cPropSets2; iPropSet++)
		{
			DBPROPSET* pPropSet2 = &rgPropSets2[iPropSet];
			DBPROPINFOSET* pFoundSet = FindPropSet(pPropSet2->guidPropertySet, cPropInfoSets, rgPropInfoSets);
			
			if(pFoundSet)
			{
				//Add all properties of this set...
				for(ULONG iProp=0; iProp<pPropSet2->cProperties; iProp++)
				{
					DBPROP* pProp2 = &pPropSet2->rgProperties[iProp];
					DBPROPINFO* pFoundProp = FindProperty(pProp2->dwPropertyID, pPropSet2->guidPropertySet, 1, pFoundSet);
					
					if(pFoundProp)
					{
						//Found the propertyset as well as the property
						//just need to update it in the list...
						pFoundProp->dwPropertyID	= pProp2->dwPropertyID;
						pFoundProp->vValues			= pProp2->vValue;

						//Update the Type (if its meaningful...)
						if(pProp2->vValue.vt != VT_EMPTY)
							pFoundProp->vtType		= pProp2->vValue.vt;
						
						//NOTE: WE store the dwOptions flag within the DBPROPFLAGS_REQUIRED
						ENABLE_BIT(pFoundProp->dwFlags, DBPROPFLAGS_REQUIRED, pProp2->dwOptions == DBPROPOPTIONS_REQUIRED);
					}
					else
					{
						//Found the property set, but not the property
						//just need to add this property to the list...
						SAFE_REALLOC(pFoundSet->rgPropertyInfos, DBPROPINFO, pFoundSet->cPropertyInfos + 1);
						memset(&pFoundSet->rgPropertyInfos[pFoundSet->cPropertyInfos], 0, sizeof(DBPROPINFO));
						pFoundSet->rgPropertyInfos[pFoundSet->cPropertyInfos].dwPropertyID	= pProp2->dwPropertyID;
						pFoundSet->rgPropertyInfos[pFoundSet->cPropertyInfos].vtType		= pProp2->vValue.vt;
						pFoundSet->rgPropertyInfos[pFoundSet->cPropertyInfos].vValues		= pProp2->vValue;

						//NOTE: WE store the dwOptions flag within the DBPROPFLAGS_REQUIRED
						ENABLE_BIT(pFoundSet->rgPropertyInfos[pFoundSet->cPropertyInfos].dwFlags, DBPROPFLAGS_REQUIRED, pProp2->dwOptions == DBPROPOPTIONS_REQUIRED);
						pFoundSet->cPropertyInfos++;
					}

					//Do we need to make a copy of the variant...
					if(!bFreeAddedPropSet)
					{
						//TODO
						//Also what about the stringbuffer with property descriptions?
					}
				}
			}
			else
			{
				//Need to add the entire property set, as well as all the properties.
				SAFE_REALLOC(rgPropInfoSets, DBPROPINFOSET, cPropInfoSets + 1);
				rgPropInfoSets[cPropInfoSets].guidPropertySet	= pPropSet2->guidPropertySet;
				rgPropInfoSets[cPropInfoSets].cPropertyInfos	= pPropSet2->cProperties;

				//Add all the properties of this set...
				SAFE_ALLOC(rgPropInfoSets[cPropInfoSets].rgPropertyInfos, DBPROPINFO, pPropSet2->cProperties);
				for(ULONG iProp=0; iProp<pPropSet2->cProperties; iProp++)
				{
					memset(&rgPropInfoSets[cPropInfoSets].rgPropertyInfos[iProp], 0, sizeof(DBPROPINFO));
					rgPropInfoSets[cPropInfoSets].rgPropertyInfos[iProp].dwPropertyID	= pPropSet2->rgProperties[iProp].dwPropertyID;
					rgPropInfoSets[cPropInfoSets].rgPropertyInfos[iProp].vtType			= pPropSet2->rgProperties[iProp].vValue.vt;
					rgPropInfoSets[cPropInfoSets].rgPropertyInfos[iProp].vValues		= pPropSet2->rgProperties[iProp].vValue;

					//NOTE: WE store the dwOptions flag within the DBPROPFLAGS_REQUIRED
					ENABLE_BIT(rgPropInfoSets[cPropInfoSets].rgPropertyInfos[iProp].dwFlags, DBPROPFLAGS_REQUIRED, pPropSet2->rgProperties[iProp].dwOptions == DBPROPOPTIONS_REQUIRED);

					//Do we need to make a copy of the variant...
					if(!bFreeAddedPropSet)
					{
						//TODO
						//Also what about the stringbuffer with property descriptions?
					}
				}
				cPropInfoSets++;
			}

			//We are done with this array of properties...
			if(bFreeAddedPropSet)
				SAFE_FREE(pPropSet2->rgProperties);
		}

		//NOTE: In the process of combining both of these sets, we are doing
		//direct memory copy of the internal DBPROPINFO items, this way
		//we don't need to variant copy all the values, and duplicate all the 
		//strings.  But this also requires that we don't free the source of the 
		//copies as well.  So we will finally end up freeing the combined 
		//property sets (cPropInfoSets) but not (cPropInfoSets2)
		if(bFreeAddedPropSet)
			SAFE_FREE(rgPropSets2);
	}

CLEANUP:
	*pcPropInfoSets = cPropInfoSets;
	*prgPropInfoSets = rgPropInfoSets;
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
// Get the property information from the data source or object.
// propid specifies the property and propset specifies the property set to which
// propid belongs. The datatype of the property is required for the correct 
// coercion of the data
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(REFIID riid, IUnknown* pIUnknown, DBPROPID dwPropertyID, REFGUID guidPropertySet, DBPROP** ppProperty)
{
	ASSERT(ppProperty);
	*ppProperty = NULL;

	HRESULT hr = S_OK;
	
	//Invalid Arguments
	if(!pIUnknown)
		return E_INVALIDARG;

	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	const ULONG		cPropertyIDSets = 1;
	DBPROPIDSET		rgPropertyIDSets[cPropertyIDSets];
	
	//SetUp input DBPROPIDSET struct (all static)
	rgPropertyIDSets[0].cPropertyIDs = cPropertyIDSets;
	rgPropertyIDSets[0].rgPropertyIDs = &dwPropertyID;
	rgPropertyIDSets[0].guidPropertySet = guidPropertySet;
	
	//Need to figure out which Interface was passed in
	//IDBInitialize, or ICommand, or IRowset, all three allow GetProperties
	if(riid == IID_IDBProperties)
	{
		//GetProperties, property might not be supported
		hr = ((IDBProperties*)pIUnknown)->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
	}
	else if(riid == IID_ICommandProperties)
	{
		//GetProperties, property might not be supported
		hr = ((ICommandProperties*)pIUnknown)->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
	}
	else if(riid == IID_IRowsetInfo)
	{
		//GetProperties, property might not be supported
		hr = ((IRowsetInfo*)pIUnknown)->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
	}
		
	//Check return result
	//Can be DB_S_ / DB_E_ERRORSOCCURRED if notsupported property...
	if(FAILED(hr) && hr != DB_E_ERRORSOCCURRED)
		TESTC(hr);

	//Verify results
	ASSERTC(cPropSets==1 && rgPropSets!=NULL);
	ASSERTC(rgPropSets[0].cProperties==1 && rgPropSets[0].rgProperties!=NULL);
	ASSERTC(rgPropSets[0].rgProperties[0].dwPropertyID == dwPropertyID);

	//Return the property to the user
	*ppProperty = rgPropSets[0].rgProperties;

CLEANUP:	
	//Just Free the outer Struct, 
	//since we return the inner Property for the user to free
	SAFE_FREE(rgPropSets);
	return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(REFIID riid, IUnknown* pIUnknown, DBPROPID dwPropertyID, REFGUID guidPropertySet, DBTYPE wType, void* pv)
{
	ASSERT(pv);
	HRESULT hr = S_OK;
	DBPROP* pProp = NULL;

	//Delegate
	hr = GetProperty(riid, pIUnknown, dwPropertyID, guidPropertySet, &pProp);

	//Put the value in the users buffer...
	if(SUCCEEDED(hr) && pProp)
	{
		//If the types are not what the user expected to receive then
		//we have to fail since pValue is of type wType.
		if(wType != pProp->vValue.vt)
		{
			//Free the property (since were not returning it to the user...)
			XTEST(hr = VariantClearFast(&pProp->vValue));
			TESTC(hr = E_INVALIDARG);
		}
		
		switch(wType)
		{
			case DBTYPE_BOOL:
				*(VARIANT_BOOL*)pv			= V_BOOL(&pProp->vValue);
				break;
			
			case DBTYPE_I2:
				*(SHORT*)pv					= V_I2(&pProp->vValue);
				break;

			case DBTYPE_I4:
				*(LONG*)pv					= V_I4(&pProp->vValue);
				break;

	#ifdef _WIN64
			case DBTYPE_I8:
	//TODO64:	*(LONG_PTR*)pv				= V_I8(&pProp->vValue);
				*(LONG_PTR*)pv				= pProp->vValue.ullVal;
				break;
	#endif //_WIN64

			case DBTYPE_WSTR:
			case DBTYPE_BSTR:
				*(BSTR*)pv					= V_BSTR(&pProp->vValue);
				break;

			case DBTYPE_VARIANT:
				*(VARIANT*)pv				= pProp->vValue;	//Assignment
				break;

			default:
				ASSERT(FALSE); //Unhandled property type
				TESTC(hr = E_INVALIDARG);
				break;
		};
	}

CLEANUP:
	//NOTE: GetProperty returns a pointer to the DBPROP (rgProperties) which is allocated
	//and needs to be free'd.  But the actual value we passed directly to the user which
	//is required to free the value, but we need to the free the outer array
	SAFE_FREE(pProp);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT GetPropInfo
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetPropInfo(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet, DBPROPINFO** ppPropInfo)
{
	if(ppPropInfo)
		*ppPropInfo = NULL;

	if(!pIDBProperties)
		return E_FAIL;	
	
	HRESULT hr = S_OK;
	const ULONG cPropIDSets = 1;
	DBPROPIDSET rgPropIDSets[cPropIDSets];

	ULONG cPropInfoSets = 0;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	
	//Construct the DBPROPIDSET structure
	rgPropIDSets[0].cPropertyIDs = 1;
	rgPropIDSets[0].rgPropertyIDs = &dwPropertyID;
	rgPropIDSets[0].guidPropertySet = guidPropertySet;
		
	//Don't display dialog if errors occurred.
	//Errors can occur if properties are not supported by the provider
	TESTC(hr = pIDBProperties->GetPropertyInfo(cPropIDSets,rgPropIDSets,&cPropInfoSets,&rgPropInfoSets,NULL));
	
CLEANUP:
	*ppPropInfo = rgPropInfoSets ? rgPropInfoSets[0].rgPropertyInfos  : NULL;
 	SAFE_FREE(rgPropInfoSets);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// DBPROPFLAGS GetPropInfoFlags
//
/////////////////////////////////////////////////////////////////////////////
DBPROPFLAGS GetPropInfoFlags(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet)
{
	if(!pIDBProperties)
		return 0;

	DBPROPINFO* pPropInfo = NULL;
	DBPROPFLAGS dwFlags = DBPROPFLAGS_NOTSUPPORTED;

	//GetPropInfo
	TESTC(GetPropInfo(pIDBProperties, dwPropertyID, guidPropertySet, &pPropInfo));
	if(pPropInfo)
	{
		dwFlags = pPropInfo->dwFlags;
	
		//Free the variant, could contain a safearray or similar...
		XTEST(VariantClearFast(&pPropInfo->vValues));
	}
	
CLEANUP:
	SAFE_FREE(pPropInfo);
	return dwFlags; 
}


/////////////////////////////////////////////////////////////////////////////
// BOOL IsSupportedProperty
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsSupportedProperty(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet)
{
	return GetPropInfoFlags(pIDBProperties, dwPropertyID, guidPropertySet) != DBPROPFLAGS_NOTSUPPORTED;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL IsSettableProperty
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsSettableProperty(IDBProperties* pIDBProperties, DBPROPID dwPropertyID, REFGUID guidPropertySet)
{
	return GetPropInfoFlags(pIDBProperties, dwPropertyID, guidPropertySet) & DBPROPFLAGS_WRITE;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SetProperty(DBPROPID dwPropertyID, REFGUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions, const DBID& colid)
{
	ASSERT(prgPropSets && pcPropSets);

	//The simplest Approach is to just use our class...
	CPropSets sCPropSets(*pcPropSets, *prgPropSets);
	
	//Delegate
	HRESULT hr = sCPropSets.SetProperty(dwPropertyID, guidPropertySet, wType, pv, dwOptions, colid);

	//Return the properties to the user and remove from static class...
	sCPropSets.Detach(pcPropSets, prgPropSets);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG* pcProperties, DBPROP** prgProperties)
{
	ASSERT(pcProperties);
	ASSERT(prgProperties);
	HRESULT hr = S_OK;
	
	//no-op case
	if(*pcProperties==0 || *prgProperties==NULL)
		return S_OK;
	
	//Free the inner variants first
	for(ULONG iProp=0; iProp<*pcProperties; iProp++)
	{
		DBPROP* pProp = &((*prgProperties)[iProp]);
		XTEST(hr = VariantClearFast(&pProp->vValue));
	}
	
	//Now NULL the set
	*pcProperties = 0;
	SAFE_FREE(*prgProperties);
	return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG* pcPropSets, DBPROPSET** prgPropSets)
{
	ASSERT(pcPropSets);
	ASSERT(prgPropSets);
	HRESULT hr = S_OK;
	
	//Loop over all the property sets
	for(ULONG iPropSet=0; iPropSet<*pcPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &((*prgPropSets)[iPropSet]);
		FreeProperties(&pPropSet->cProperties, &pPropSet->rgProperties);
	}
		
	//Now NULL the outer set
	*pcPropSets = 0;
	SAFE_FREE(*prgPropSets);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG* pcPropIDSets, DBPROPIDSET** prgPropIDSets)
{
	ASSERT(pcPropIDSets);
	ASSERT(prgPropIDSets);
	HRESULT hr = S_OK;
	
	//Loop over all the property sets
	for(ULONG iPropIDSet=0; iPropIDSet<*pcPropIDSets; iPropIDSet++)
	{
		DBPROPIDSET* pPropIDSet = &((*prgPropIDSets)[iPropIDSet]);
		SAFE_FREE(pPropIDSet->rgPropertyIDs);
	}
		
	//Now NULL the outer set
	*pcPropIDSets = 0;
	SAFE_FREE(*prgPropIDSets);
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets)
{
	ASSERT(pcPropInfoSets);
	ASSERT(prgPropInfoSets);
	HRESULT hr = S_OK;
	
	//Loop over all the property info sets
	for(ULONG iPropSet=0; iPropSet<*pcPropInfoSets; iPropSet++)
	{
		DBPROPINFOSET* pPropInfoSet = &((*prgPropInfoSets)[iPropSet]);
		
		//Free the inner variants first
		for(ULONG iProp=0; iProp<pPropInfoSet->cPropertyInfos; iProp++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[iProp];
			XTEST(hr = VariantClearFast(&pPropInfo->vValues));
		}
		
		//Now free the set
		SAFE_FREE(pPropInfoSet->rgPropertyInfos);
	}
		
	//Now free the outer set
	*pcPropInfoSets = 0;
	SAFE_FREE((*prgPropInfoSets));
	return hr;
}



