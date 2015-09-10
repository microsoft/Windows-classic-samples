// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "Resource.h"
#include "util.h"
#include <upnp.h> // For the error codes

//+---------------------------------------------------------------------------
//
//  Function:	ReleaseObj
//
//  Purpose:    Releases the COM interface pointer
//
//  Arguments:
//				pInterface	[in]	Interface pointer to be released
//
//  Returns:    None
//
//  Notes:
//				
//


void ReleaseObj(_In_ IUnknown* pInterface){
	if(pInterface){
		pInterface->Release();
	}
}


//+---------------------------------------------------------------------------
//
//  Function:	PrintErrorText
//
//  Purpose:    Prints the appropriate error text
//
//  Arguments:
//				hr	[in]	HRESULT to check
//
//  Returns:    None
//
//  Notes:
//				The upnp error codes are from upnp.h
//				

void PrintErrorText(_In_ HRESULT hr){

    // First check whether it is a UPnP error
    WCHAR wszErrorString[1024] = {0};
    switch(hr){
        // All the upnp error codes are from upnp.h 
        case UPNP_E_ROOT_ELEMENT_EXPECTED:
            SafeStrCopy(
                            wszErrorString, 
                            L"Root Element Expected",
                            1024 
                            );
            break;
        case UPNP_E_DEVICE_ELEMENT_EXPECTED:
            SafeStrCopy(
                            wszErrorString, 
                            L"Device Element Expected",
                            1024 
                            );
            break;
        case UPNP_E_SERVICE_ELEMENT_EXPECTED:
            SafeStrCopy(
                            wszErrorString, 
                            L"Service Element Expected",
                            1024 
                            );
            break;
        case UPNP_E_SERVICE_NODE_INCOMPLETE:
            SafeStrCopy(
                            wszErrorString, 
                            L"Service Node Incomplete",
                            1024 
                            );
            break;
        case UPNP_E_DEVICE_NODE_INCOMPLETE:
            SafeStrCopy(
                            wszErrorString, 
                            L"Device Node Incomplete",
                            1024 
                            );
            break;
        case UPNP_E_ICON_ELEMENT_EXPECTED:
            SafeStrCopy(
                            wszErrorString, 
                            L"Icon Element Expected",
                            1024 
                            );
            break;
        case UPNP_E_ICON_NODE_INCOMPLETE:
            SafeStrCopy(
                            wszErrorString, 
                            L"Icon Node Incomplete",
                            1024 
                            );
            break;
        case UPNP_E_INVALID_ACTION:
            SafeStrCopy(
                           wszErrorString, 
                           L"Invalid Action",
                           1024 
                           );
            break;
        case UPNP_E_INVALID_ARGUMENTS:
            SafeStrCopy(
                           wszErrorString, 
                           L"Invalid Arguments",
                           1024 
                           );
            break;
        case UPNP_E_OUT_OF_SYNC:
            SafeStrCopy(
                            wszErrorString, 
                            L"Out of Sync",
                            1024 
                            );
            break;
        case UPNP_E_ACTION_REQUEST_FAILED:
            SafeStrCopy(
                            wszErrorString, 
                            L"Action Request Failed",
                            1024 
                            );
            break;
        case UPNP_E_TRANSPORT_ERROR:
            SafeStrCopy(
                            wszErrorString, 
                            L"Transport Error",
                            1024 
                            );
            break;
        case UPNP_E_VARIABLE_VALUE_UNKNOWN:
            SafeStrCopy(
                            wszErrorString, 
                            L"Variable Value Unknown",
                            1024 
                            );
            break;
        case UPNP_E_INVALID_VARIABLE:
            SafeStrCopy(
                            wszErrorString, 
                            L"Invalid Variable",
                            1024
                            );
            break;
        case UPNP_E_DEVICE_ERROR:
            SafeStrCopy(
                            wszErrorString, 
                            L"Device Error",
                           1024 
                            );
            break;
        case UPNP_E_PROTOCOL_ERROR:
            SafeStrCopy(
                            wszErrorString, 
                            L"Protocol Error",
                            1024 
                            );
            break;
        case UPNP_E_ERROR_PROCESSING_RESPONSE:
            SafeStrCopy(
                            wszErrorString, 
                            L"Error Processing Response",
                            1024 
                            );
            break;
        case UPNP_E_DEVICE_TIMEOUT:
            SafeStrCopy(
                            wszErrorString, 
                            L"Device Timeout",
                            1024 
                            );
            break;
        case UPNP_E_INVALID_DOCUMENT:
            SafeStrCopy(
                            wszErrorString, 
                            L"Invalid Document",
                            1024 
                            );
            break;
        case UPNP_E_EVENT_SUBSCRIPTION_FAILED:
            SafeStrCopy(
                            wszErrorString, 
                            L"Event Subscription Failed",
                            1024 
                            );
            break;
        default:
                   if ((hr>=UPNP_E_ACTION_SPECIFIC_BASE) && 
                       (hr <= UPNP_E_ACTION_SPECIFIC_MAX))
                   {
            SafeStrCopy(
                            wszErrorString, 
                            L"Action Specific Error",
                            1024 
                            );
            break;
        }
        _snwprintf_s(
                   wszErrorString,
                   1024, 
                   _TRUNCATE,
                   L"An error has occured.\n\nError Code: 0x%x", 
                   hr
                   );                        
    }
    MessageBox( NULL, (LPCWSTR)wszErrorString, L"Error", MB_OK );
}



//+---------------------------------------------------------------------------
//
//  Function:	HrCreateSafeArray
//
//  Purpose:    Creates a SafeArray
//
//  Arguments:
//				vt		[in]	VariantType
//				nArgs	[in]	Number of Arguments
//				ppsa	[out]	Created safearray
//
//  Returns:    S_OK or E_OUTOFMEMORY
//
//  Notes:
//				
//				
HRESULT HrCreateSafeArray(_In_ VARTYPE vt, _In_ int nArgs, _Outptr_ SAFEARRAY **ppsa)
{
    SAFEARRAYBOUND    aDim[1]; 

	if(0 == nArgs){
        aDim[0].lLbound = 0; 
        aDim[0].cElements = 0; 
    }
    else{
        aDim[0].lLbound = 1; 
        aDim[0].cElements = nArgs; 
    }
        
    *ppsa = SafeArrayCreate(vt, 1, aDim);
    if(NULL == *ppsa){
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
//  Function:	HrCreateArgVariants
//
//  Purpose:    Creates argument variants
//
//  Arguments:
//				dwArgs	[in]	Number of arguments
//				pppVars	[out]	Variant array
//
//  Returns:    S_OK or E_OUTOFMEMORY
//
//  Notes:
//				
//				

_When_(dwArgs == 0, _At_(*pppVars, _Post_null_))
_When_(dwArgs > 0, _At_(*pppVars, _Post_notnull_))
HRESULT HrCreateArgVariants(_In_ DWORD dwArgs, _Outptr_result_buffer_maybenull_(dwArgs) VARIANT*** pppVars)
{
    HRESULT             hr = S_OK;
    DWORD               i = 0;
    
	if(0 == dwArgs){
        *pppVars = NULL;
        return hr;
    }
    *pppVars = new VARIANT* [dwArgs];
    if(NULL == *pppVars){
		return E_OUTOFMEMORY;
    }
	ZeroMemory(*pppVars, dwArgs*sizeof(VARIANT *));
    
	for(i = 0; i < dwArgs; i++){
        (*pppVars)[i] = new VARIANT;
        if(NULL == (*pppVars)[i])
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
        VariantInit((*pppVars)[i]);
    }

    goto exit;

error:
    for(i = 0; i < dwArgs; i++)
    {
        if( NULL != (*pppVars)[i] )
        {
            delete ((*pppVars)[i]);
            (*pppVars)[i] = NULL;
        }
    }
    delete [] *pppVars;
    *pppVars = NULL;

exit:
    return hr;

}



//+---------------------------------------------------------------------------
//
//  Function:	HrDestroyArgVariants
//
//  Purpose:    Destroys argument variants
//
//  Arguments:
//				dwArgs	[in]	Number of arguments
//				pppVars	[in,out]	Variant array
//
//  Returns:    S_OK
//
//  Notes:
//				
//				

HRESULT HrDestroyArgVariants(_In_ DWORD dwArgs, _Inout_ _At_(*pppVars, _Pre_writable_size_(dwArgs) _Post_maybenull_) VARIANT*** pppVars)
{
    HRESULT             hr = S_OK;
    VARIANT             *pVar = NULL;

	ASSERT(pppVars && *pppVars);
    if(0 == dwArgs)
    {
        return hr;
    }

    for(DWORD i = 0; i < dwArgs; i++)
    {
        pVar = (*pppVars)[i];
        if(NULL != pVar)
        {
            VariantClear(pVar);
            delete pVar;
            pVar = NULL;;
        }
    }

    delete [] *pppVars;
    *pppVars = NULL;

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Function:	VariantSetVar
//
//  Purpose:    Sets the variant value to given variant
//
//  Arguments:
//				pvarToSet [in]		Variant which is used to set
//				va		[in,out]	Variant to set
//
//  Returns:    None
//
//  Notes:
//				
//				


void VariantSetVar(_In_ VARIANT* pvarToSet, _Inout_ VARIANT& va)
{
    VariantInit(&va);
    va.vt = VT_VARIANT | VT_BYREF;
    va.pvarVal = pvarToSet;
}


//+---------------------------------------------------------------------------
//
//  Function:	VariantSetArray
//
//  Purpose:    Sets the variant value to given array
//
//  Arguments:
//				psa		[in]		SafeArray 
//				va		[in,out]	Variant which is being set
//
//  Returns:    None
//
//  Notes:
//				
//				
void VariantSetArray(_In_ SAFEARRAY* psa, _Inout_ VARIANT& va)
{
    VariantInit(&va);
    va.vt = VT_VARIANT | VT_ARRAY | VT_BYREF;
    va.pparray = &psa;
}


//+---------------------------------------------------------------------------
//
//  Function:	HrGetSafeArrayBounds
//
//  Purpose:    Get SafeArray bounds
//
//  Arguments:
//				psa		[in]	SafeArray
//				plLBound	[out]	Lower bound
//				plUBound	[out]	Upper bound
//
//  Returns:    HRESULT
//
//  Notes:
//				
//				

HRESULT HrGetSafeArrayBounds(_In_ SAFEARRAY *psa, _Out_ long* plLBound, _Out_ long* plUBound)
{
    HRESULT hr = S_OK;

    if(NULL == psa)
    {
        return E_POINTER;
    }

    hr = SafeArrayGetLBound(psa, 1, plLBound);
    if(FAILED(hr))
    {
        return hr;
    }
    hr = SafeArrayGetUBound(psa, 1, plUBound);
    return hr;
}


//+---------------------------------------------------------------------------
//
//  Function:	HrGetVariantElement
//
//  Purpose:    Get Variant Element
//
//  Arguments:
//				psa		[in]	SafeArray
//				lPosition	[in] Position in the array
//				pvar	[out]	Variant Element being set
//
//  Returns:    HRESULT
//
//  Notes:
//				
//				

HRESULT HrGetVariantElement(_In_ SAFEARRAY *psa, _In_ int lPosition, _Out_ VARIANT* pvar)
{
    HRESULT hr = S_OK;
    long    alPos[1];

    if(NULL == psa) 
    {
        return E_POINTER;
    }

    alPos[0] = lPosition;

    hr = SafeArrayGetElement(psa,
                             alPos,
                             pvar);
    return hr;
}


//+--------------------------------------------------------------------------
//
//  Function: SafeStrCopy
//
//  Purpose : Does a Safe string copy using wcsncpy. We will always get a NULL
//            terminated string with this.
//
//  Argument: 
//           src  [in/out] Buffer into which string is to be copied.
//           dst  [in] Buffer to copy from.
//           srcBufLen [in] Size of the source buffer.
//
//  Returns: Nothing
//
//  Notes:
//
//

void SafeStrCopy(_Out_writes_(srcBufLen) LPWSTR src, _In_ LPCWSTR dst, _In_ size_t srcBufLen)
{
   ASSERT(src != NULL);
   ASSERT(dst != NULL);
   ASSERT(srcBufLen != 0);

   wcsncpy_s(src, srcBufLen, dst, (srcBufLen - 1));

}
