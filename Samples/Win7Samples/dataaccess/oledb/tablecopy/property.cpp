//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module PROPERTY.CPP
//
//-----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// Includes
//
////////////////////////////////////////////////////////////////////////
#include "WinMain.h"
#include "Property.h"



/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
// Get the property information from the data source or object.
// propid specifies the property and propset specifies the property set to which
// propid belongs. The datatype of the property is required for the correct 
// coercion of the data
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet,
                   DBPROP** ppProperty)
{
    ASSERT(pIUnknown && ppProperty);
    *ppProperty = NULL;

    HRESULT hr = S_OK;
    
    IDBProperties* pIDBProperties = NULL;
    ICommandProperties* pICommandProperties = NULL;
    IRowsetInfo* pIRowsetInfo = NULL;

    ULONG		cPropSets = 0;
    DBPROPSET*	rgPropSets = NULL;
        
    const ULONG		cPropertyIDSets = 1;
    DBPROPIDSET		rgPropertyIDSets[cPropertyIDSets];
    
    //SetUp input DBPROPIDSET struct (all static)
    rgPropertyIDSets[0].cPropertyIDs = cPropertyIDSets;
    rgPropertyIDSets[0].rgPropertyIDs = &PropertyID;
    rgPropertyIDSets[0].guidPropertySet = guidPropertySet;
    
    //Need to figure out which Interface was passed in
    //IDBInitialize, or ICommand, or IRowset, all three allow GetProperties
    if(SUCCEEDED(hr = pIUnknown->QueryInterface(IID_IDBProperties,(void **)&pIDBProperties)))
    {
        //GetProperties	
        //Property might not be supported
        hr = pIDBProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
    }
    else if(SUCCEEDED(hr = pIUnknown->QueryInterface(IID_ICommandProperties,(void **)&pICommandProperties)))
    {
        //GetProperties	
        //Property might not be supported
        hr = pICommandProperties->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
    }
    else
    {
        XTESTC(hr = pIUnknown->QueryInterface(IID_IRowsetInfo, (void **)&pIRowsetInfo));
                    
        //GetProperties	
        //Property might not be supported
        hr = pIRowsetInfo->GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);
    }
        
    //Check return result
    //Can be DB_S_ / DB_E_ERRORSOCCURRED if notsupported property...
    if(FAILED(hr) && hr != DB_E_ERRORSOCCURRED)
        XTESTC(hr);

    //Verify results
    CHECKC(cPropSets==1 && rgPropSets!=NULL);
    CHECKC(rgPropSets[0].cProperties==1 && rgPropSets[0].rgProperties!=NULL);
    CHECKC(rgPropSets[0].rgProperties[0].dwPropertyID == PropertyID);

    //Return the property to the user
    *ppProperty = rgPropSets[0].rgProperties;
    hr = S_OK;

CLEANUP:	
    //Just Free the outer Struct, 
    //since we return the inner Property for the user to free
    SAFE_FREE(rgPropSets);
    
    SAFE_RELEASE(pIDBProperties);
    SAFE_RELEASE(pICommandProperties);
    SAFE_RELEASE(pIRowsetInfo);
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, WCHAR** ppwszValue)
{
    ASSERT(pIUnknown && ppwszValue);
    HRESULT hr;

    DBPROP* pProperty = NULL;
    *ppwszValue = NULL;

    //Property might not be supported
    hr = GetProperty(pIUnknown, PropertyID, guidPropertySet, &pProperty);

    //Copy the value
    //GetProperty might return VT_EMPTY for no default string
    if(SUCCEEDED(hr) && pProperty && pProperty->vValue.vt == VT_BSTR)
        (*ppwszValue) = wcsDuplicate(V_BSTR(&pProperty->vValue));

    if(*ppwszValue == NULL)
    {
        SAFE_ALLOC(*ppwszValue, WCHAR, 1);
        (*ppwszValue)[0] = EOL;
    }

CLEANUP:
    FreeProperties(1, pProperty);
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, WCHAR* pwszValue, size_t cwchValue)
{
    ASSERT(pIUnknown && pwszValue);
    HRESULT hr;

    DBPROP* pProperty = NULL;
    *pwszValue = EOL;

    //Might not be supported
    hr = GetProperty(pIUnknown, PropertyID, guidPropertySet, &pProperty);
    
    //Copy the value
    //GetProperty might return VT_EMPTY for no default string
    if(SUCCEEDED(hr) && pProperty && pProperty->vValue.vt == VT_BSTR)
        StringCchCopyW(pwszValue, cwchValue, V_BSTR(&pProperty->vValue));

    FreeProperties(1, pProperty);
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcValue)
{
    ASSERT(pIUnknown && pcValue);
    HRESULT hr;

    DBPROP* pProperty = NULL;
    *pcValue = 0;

    //Might not be supported
    hr = GetProperty(pIUnknown, PropertyID, guidPropertySet, &pProperty);

    //Copy the value
    //GetProperty might return VT_EMPTY for no default string
    if(SUCCEEDED(hr) && pProperty && pProperty->vValue.vt == VT_I4)
        *pcValue = V_I4(&pProperty->vValue);

    FreeProperties(1, pProperty);
    return hr;
}
                   
/////////////////////////////////////////////////////////////////////////////
// HRESULT GetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, BOOL* pbValue)
{
    ASSERT(pIUnknown && pbValue);
    HRESULT hr;

    DBPROP* pProperty = NULL;
    *pbValue = FALSE;

    //Might not be supported
    hr = GetProperty(pIUnknown, PropertyID, guidPropertySet, &pProperty);

    //Copy the value
    //GetProperty might return VT_EMPTY for no default string
    if(SUCCEEDED(hr) && pProperty && pProperty->vValue.vt == VT_BOOL)
        *pbValue = V_BOOL(&pProperty->vValue) == VARIANT_TRUE;

    FreeProperties(1, pProperty);
    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT GetPropInfo
//
/////////////////////////////////////////////////////////////////////////////
HRESULT GetPropInfo(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet, DBPROPINFO** ppPropInfo)
{
    ASSERT(pIUnknown && ppPropInfo);
    HRESULT hr;
    IDBProperties* pIDBProperties = NULL;

    const ULONG cPropIDSets = 1;
    DBPROPIDSET rgPropIDSets[cPropIDSets];

    ULONG cPropInfoSets = 0;
    DBPROPINFOSET* rgPropInfoSets = NULL;
    
    //Construct the DBPROPIDSET structure
    rgPropIDSets[0].cPropertyIDs = 1;
    rgPropIDSets[0].rgPropertyIDs = &PropertyID;
    rgPropIDSets[0].guidPropertySet = guidPropertySet;
        
    //Obtain IDBProperties
    XTESTC(hr = pIUnknown->QueryInterface(IID_IDBProperties,(void**)&pIDBProperties));
    
    //Don't display dialog if errors occurred.
    //Errors can occur if properties are not supported by the provider
    QTESTC(hr = pIDBProperties->GetPropertyInfo(cPropIDSets,rgPropIDSets,&cPropInfoSets,&rgPropInfoSets,NULL));
    
CLEANUP:
    *ppPropInfo = rgPropInfoSets ? rgPropInfoSets[0].rgPropertyInfos  : NULL;
    SAFE_FREE(rgPropInfoSets);
    SAFE_RELEASE(pIDBProperties);
    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// DBPROPFLAGS GetPropInfoFlags
//
/////////////////////////////////////////////////////////////////////////////
DBPROPFLAGS GetPropInfoFlags(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet)
{
    ASSERT(pIUnknown);
    DBPROPINFO* pPropInfo = NULL;
    DBPROPFLAGS dwFlags = DBPROPFLAGS_NOTSUPPORTED;

    //GetPropInfo
    QTESTC(GetPropInfo(pIUnknown, PropertyID, guidPropertySet, &pPropInfo))
    if(pPropInfo)
        dwFlags = pPropInfo->dwFlags;
    
CLEANUP:
    SAFE_FREE(pPropInfo);
    return dwFlags; 
}


/////////////////////////////////////////////////////////////////////////////
// BOOL IsSupportedProperty
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsSupportedProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet)
{
    return GetPropInfoFlags(pIUnknown, PropertyID, guidPropertySet) != DBPROPFLAGS_NOTSUPPORTED;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL IsSettableProperty
//
/////////////////////////////////////////////////////////////////////////////
BOOL IsSettableProperty(IUnknown* pIUnknown, DBPROPID PropertyID, GUID guidPropertySet)
{
    return GetPropInfoFlags(pIUnknown, PropertyID, guidPropertySet) & DBPROPFLAGS_WRITE;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SetProperty(DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, ULONG_PTR ulValue, DBPROPOPTIONS dwOptions, DBID colid)
{
    if(wType == DBTYPE_BOOL)
        ulValue = ulValue ? VARIANT_TRUE : VARIANT_FALSE;
    return SetProperty(PropertyID, guidPropertySet, pcPropSets, prgPropSets, wType, &ulValue, dwOptions, colid);
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SetProperty
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SetProperty(DBPROPID PropertyID, GUID guidPropertySet, ULONG* pcPropSets, DBPROPSET** prgPropSets, DBTYPE wType, void* pv, DBPROPOPTIONS dwOptions, DBID colid)
{
    ASSERT(PropertyID && prgPropSets && pcPropSets && pv);
    HRESULT hr = S_OK;

    ULONG cProperties = 0;
    DBPROP* rgProperties = NULL;

    //Make our lives a little easier
    ULONG cPropSets = *pcPropSets;
    DBPROPSET* rgPropSets = *prgPropSets;
    
    ULONG iPropSet = ULONG_MAX;
    
    //Find the correct PropSet structure to add the property to
    for(ULONG i=0; i<cPropSets; i++)
        if(guidPropertySet == rgPropSets[i].guidPropertySet)
            iPropSet = i;

    //Do we need to create another PropSets structure for this property
    if(iPropSet==ULONG_MAX)
    {
        iPropSet = cPropSets;
        SAFE_REALLOC(rgPropSets, DBPROPSET, cPropSets+1);
        rgPropSets[iPropSet].cProperties = 0;
        rgPropSets[iPropSet].rgProperties = NULL;
        rgPropSets[iPropSet].guidPropertySet = guidPropertySet;
        cPropSets++;
    }

    //Now make our lives really easy
    cProperties = rgPropSets[iPropSet].cProperties;
    rgProperties = rgPropSets[iPropSet].rgProperties;

    //do we need to enlarge the list
    SAFE_REALLOC(rgProperties, DBPROP, cProperties+1);
    
    //Add the new property to the list
    rgProperties[cProperties].dwPropertyID = PropertyID;
    rgProperties[cProperties].dwOptions    = dwOptions;
    rgProperties[cProperties].dwStatus     = DBPROPSTATUS_OK;
    rgProperties[cProperties].colid        = colid;
    VariantInit(&rgProperties[cProperties].vValue);

    switch(wType)
    {
        case DBTYPE_BOOL:
            rgProperties[cProperties].vValue.vt          = VT_BOOL;
            V_BOOL(&(rgProperties[cProperties].vValue))  = *(VARIANT_BOOL*)pv;
            break;
        
        case DBTYPE_I2:
            rgProperties[cProperties].vValue.vt			= VT_I2;
            V_I2(&(rgProperties[cProperties].vValue))	= *(SHORT*)pv;
            break;

        case DBTYPE_I4:
            rgProperties[cProperties].vValue.vt			= VT_I4;
            V_I4(&(rgProperties[cProperties].vValue))	= *(LONG*)pv;
            break;

        case DBTYPE_I8:
            rgProperties[cProperties].vValue.vt			= VT_I8;
            V_I8(&(rgProperties[cProperties].vValue))	= *(LONG_PTR*)pv;
            break;
   
        case DBTYPE_WSTR:
        case DBTYPE_BSTR:
            rgProperties[cProperties].vValue.vt          = VT_BSTR;
            V_BSTR(&(rgProperties[cProperties].vValue))  = SysAllocString((BSTR)pv);
            break;

        case DBTYPE_VARIANT:
            VariantCopy(&rgProperties[cProperties].vValue, (VARIANT*)pv);
            break;

        default:
            ASSERT(FALSE); //Unhandled property type
            break;
    }

    //Increment the number of properties
    cProperties++;

CLEANUP:
    //Now go back to the rediculous property structures
    rgPropSets[iPropSet].cProperties  = cProperties;
    rgPropSets[iPropSet].rgProperties = rgProperties;
    *pcPropSets = cPropSets;
    *prgPropSets = rgPropSets;
    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT SetRestriction
//
/////////////////////////////////////////////////////////////////////////////
HRESULT SetRestriction(VARIANT* pRestriction, WCHAR* pwszValue)
{
    ASSERT(pRestriction);
    ASSERT(pwszValue);
    
    //VT_BSTR case
    if(pwszValue && pwszValue[0])
    {
        pRestriction->vt = VT_BSTR;
        SAFE_SYSALLOC(V_BSTR(pRestriction), pwszValue);
    }

CLEANUP:
    return S_OK;
}
    

/////////////////////////////////////////////////////////////////////////////
// HRESULT InitVariants
//
/////////////////////////////////////////////////////////////////////////////
HRESULT InitVariants(ULONG cVariants, VARIANT* rgVariants)
{
    HRESULT hr = S_OK;

    //Free all variants
    for(ULONG i=0; i<cVariants; i++)
        VariantInit(&rgVariants[i]);
    
    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeVariants
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeVariants(ULONG cVariants, VARIANT* rgVariants)
{
    HRESULT hr = S_OK;

    //Free the inner variants first
    for(ULONG i=0; i<cVariants; i++)
        XTEST(hr = VariantClear(&rgVariants[i]));
    
    return hr;
}


/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG cProperties, DBPROP* rgProperties)
{
    HRESULT hr = S_OK;
    
    //no-op case
    if(cProperties==0 || rgProperties==NULL)
        return S_OK;
    
    //Free the inner variants first
    for(ULONG i=0; i<cProperties; i++)
    {
        //if DBPROPSTATUS_NOTSUPPORTED then vValue is undefined
        if(rgProperties[i].dwStatus != DBPROPSTATUS_NOTSUPPORTED)
            XTEST(hr = VariantClear(&rgProperties[i].vValue));
    }
    
    //Now free the set
    SAFE_FREE(rgProperties);
    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// HRESULT FreeProperties
//
/////////////////////////////////////////////////////////////////////////////
HRESULT FreeProperties(ULONG cPropSets, DBPROPSET* rgPropSets)
{
    HRESULT hr = S_OK;
    
    //Loop over all the property sets
    for(ULONG i=0; i<cPropSets; i++)
        FreeProperties(rgPropSets[i].cProperties, rgPropSets[i].rgProperties);
        
    //Now free the outer set
    SAFE_FREE(rgPropSets);
    return hr;
}





