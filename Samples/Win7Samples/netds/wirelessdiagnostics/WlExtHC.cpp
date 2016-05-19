// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 2006  Microsoft Corporation.  All Rights Reserved.
//
// Module:
//    WlExtHC.cpp
//
// Abstract:
//    This sample shows how to create a sample Extensible Helper Class for Wireless Diagnostics
//    This sample is post-Windows 2006 only.
//
// Usage:
//    WlExtHC.exe [/RegServer]
//           /RegServer          Register the HC
//
// Author:
//    Mohammad Shabbir Alam
//


#include "precomp.h"
#pragma hdrstop


GUID gWirelessHelperExtensionRepairId = { /* cb2a064e-b691-4a63-9e7e-7d2970bbe025 */
    0xcb2a064e,
    0xb691,
    0x4a63,
    {0x9e, 0x7e, 0x7d, 0x29, 0x70, 0xbb, 0xe0, 0x25}};


__inline 
HRESULT 
StringCchCopyWithAlloc (
    __deref_out_opt PWSTR* Dest, 
    size_t cchMaxLen,
    __in PCWSTR Src
    )
{
    size_t cchSize = 0;

    if (NULL == Dest || NULL == Src || cchMaxLen > USHRT_MAX)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = StringCchLength (Src, cchMaxLen - 1, &cchSize); 
    if (FAILED(hr))
    {
        return hr;
    }

    size_t cbSize = (cchSize + 1)*sizeof(WCHAR);
    *Dest = (PWSTR) CoTaskMemAlloc(cbSize);
    if (NULL == *Dest)
    {
        return E_OUTOFMEMORY;
    }
    SecureZeroMemory(*Dest, cbSize);

    return StringCchCopy((STRSAFE_LPWSTR)*Dest, cchSize + 1, Src);    
}

HRESULT
CWirelessHelperExtension::GetAttributeInfo(
    __RPC__out ULONG *pcelt, 
    __RPC__deref_out_ecount_full_opt(*pcelt) HelperAttributeInfo **pprgAttributeInfos
    )
{
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {
        if (pcelt==NULL || pprgAttributeInfos==NULL)
        {
            hr = E_INVALIDARG;
            break;
        }

        HelperAttributeInfo *info = (HelperAttributeInfo *)CoTaskMemAlloc(3*sizeof(HelperAttributeInfo));
        if (info==NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        SecureZeroMemory(info,3*sizeof(HelperAttributeInfo));
    
        StringCchCopyWithAlloc(&info[0].pwszName, MAX_PATH, L"IfType");
        info[0].type=AT_UINT32;
        StringCchCopyWithAlloc(&info[1].pwszName, MAX_PATH, L"LowHealthAttributeType");
        info[1].type=AT_UINT32;
        StringCchCopyWithAlloc(&info[2].pwszName, MAX_PATH, L"ReturnRepair");
        info[2].type=AT_BOOLEAN;

        *pcelt=3;
        *pprgAttributeInfos=info;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT 
CWirelessHelperExtension::Initialize( 
    ULONG celt,
    __RPC__in_ecount_full(celt) HELPER_ATTRIBUTE rgAttributes[  ]
    )
{   
    WDIAG_IHV_WLAN_ID   WDiagID = {0};
    HRESULT             hr = S_OK;

    LogEntry (this);

    do
    {
        if (celt == 0)
        {
            break;
        }

        if (rgAttributes == NULL)
        {
            hr = E_INVALIDARG;
            break;
        }

        for (UINT i = 0; i < celt; i++)
        {
            switch (rgAttributes[i].type)
            {
                case (AT_GUID):
                {
                    LogInfo ("\t[%d] GUID Attribute <%ws>, value=<%!guid!>\n",
                        i+1, rgAttributes[i].pwszName, &rgAttributes[i].Guid);

                    break;
                }

                case (AT_UINT32):
                {
                    LogInfo ("\t[%d] UINT Attribute <%ws>, value=<%d>\n",
                        i+1, rgAttributes[i].pwszName, rgAttributes[i].DWord);

                    break;
                }

                case (AT_BOOLEAN):
                {
                    LogInfo ("\t[%d] BOOLEAN Attribute <%ws>, value=<%d>\n",
                        i+1, rgAttributes[i].pwszName, rgAttributes[i].Boolean);

                    break;
                }

                case (AT_STRING):
                {
                    LogInfo ("\t[%d] STRING Attribute <%ws>, value=<%ws>\n",
                        i+1, rgAttributes[i].pwszName, rgAttributes[i].PWStr);

                    break;
                }

                case (AT_OCTET_STRING):
                {
                    LogInfo ("\t[%d] AT_OCTET_STRING Attribute <%ws>, Length=<%d>\n",
                        i+1, rgAttributes[i].pwszName, rgAttributes[i].OctetString.dwLength);

                    if (rgAttributes[i].OctetString.dwLength < sizeof(WDIAG_IHV_WLAN_ID))
                    {
                        LogInfo ("\t\tLength=<%d> < sizeof(WDIAG_IHV_WLAN_ID)=<%d>\n",
                            rgAttributes[i].OctetString.dwLength, sizeof(WDIAG_IHV_WLAN_ID));
                        break;
                    }

                    RtlCopyMemory (&WDiagID, rgAttributes[i].OctetString.lpValue, sizeof (WDIAG_IHV_WLAN_ID));
                    LogInfo ("\t\tProfileName=<%ws>\n", WDiagID.strProfileName);
                    LogInfo ("\t\tSsidLength=<%d>\n", WDiagID.Ssid.uSSIDLength);
                    LogInfo ("\t\tBssType=<%d>\n", WDiagID.BssType);
                    LogInfo ("\t\tReasonCode=<%d>\n", WDiagID.dwReasonCode);
                    LogInfo ("\t\tFlags=<0x%x>\n", WDiagID.dwFlags);

                    break;
                }

                default:
                {
                    LogInfo ("\t[%d] Attribute <%ws>, Unknown type=<%d>\n",
                        i+1, rgAttributes[i].pwszName, rgAttributes[i].type);

                    break;
                }
            }
        }

        if (S_OK != hr)
        {
            break;
        }

        InterlockedCompareExchange(&m_initialized, 1, 0);

    } while (FALSE);
    
    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetDiagnosticsInfo( 
    __RPC__deref_out_opt DiagnosticsInfo **ppInfo
    )
{
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {
        if (NULL == ppInfo)
        {
            break;
        }

        *ppInfo = (DiagnosticsInfo *)CoTaskMemAlloc(sizeof(DiagnosticsInfo));

        if (*ppInfo == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        (*ppInfo)->cost = 0;
        (*ppInfo)->flags = DF_TRACELESS;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}   

HRESULT  
CWirelessHelperExtension::GetKeyAttributes( 
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) HELPER_ATTRIBUTE **pprgAttributes
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pcelt);
    UNREFERENCED_PARAMETER (pprgAttributes);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::LowHealth( 
    __RPC__in_opt LPCWSTR pwszInstanceDescription,        
    __RPC__deref_out_opt_string LPWSTR *ppwszDescription,
    __RPC__out long *pDeferredTime,
    __RPC__out DIAGNOSIS_STATUS *pStatus
    )
{  
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pwszInstanceDescription);

    LogEntry (this);

    do
    {
        if (m_initialized <= 0)
        {
            hr = E_UNEXPECTED;
            break;
        }

        if (pDeferredTime == NULL || pStatus == NULL || ppwszDescription == NULL)
        {
            hr = E_INVALIDARG;
            break;
        }

        //
        // this class will always confirm. this way we know the class
        // was instantiated and was called successfully
        //
        *pStatus = DS_CONFIRMED;
        m_ReturnRepair = TRUE;
        hr = StringCchCopyWithAlloc(ppwszDescription, 256, L"Helper Extension LowHealth call succeeded.");

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::HighUtilization( 
    __RPC__in_opt LPCWSTR pwszInstanceDescription,        
    __RPC__deref_out_opt_string LPWSTR *ppwszDescription,
    __RPC__out long *pDeferredTime,
    __RPC__out DIAGNOSIS_STATUS *pStatus
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pwszInstanceDescription);
    UNREFERENCED_PARAMETER (ppwszDescription);
    UNREFERENCED_PARAMETER (pDeferredTime);
    UNREFERENCED_PARAMETER (pStatus);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetLowerHypotheses( 
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses
    )
{
    HYPOTHESIS *hypothses = NULL;
    HELPER_ATTRIBUTE *attributes = NULL;
    size_t size = sizeof(HYPOTHESIS);
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {
        if (m_initialized <= 0)
        {
            hr = E_UNEXPECTED;
            break;
        }

        if (pcelt == NULL)
        {
            hr = E_INVALIDARG;
            break;
        } 

        //if no attribute type specified we don't perform a lower hypotheses
        if(m_LowHealthAttributeType==0)
        {
            *pcelt = 0;
            *pprgHypotheses=NULL;
            hr = S_OK;
            break;
        }

        // check to see if additional storage is needed for attributes.

        hypothses = (HYPOTHESIS*)CoTaskMemAlloc(size);
        if (hypothses == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        SecureZeroMemory(hypothses, size);      
    
        hr = StringCchCopyWithAlloc(&(hypothses->pwszClassName), MAX_PATH, L"LowHealthHypotheses");
        if (FAILED (hr))
        {
            break;
        }

        hr = StringCchCopyWithAlloc(&(hypothses->pwszDescription), MAX_PATH, L"TestHypotheses");
        if (FAILED (hr))
        {
            break;
        }

        hypothses->celt = 1;
        attributes = hypothses->rgAttributes = (PHELPER_ATTRIBUTE)CoTaskMemAlloc(sizeof HELPER_ATTRIBUTE);  

        if (attributes == NULL)
        {        
            hr = E_OUTOFMEMORY;
            break;
        }

        switch(m_LowHealthAttributeType)
        {
            case AT_BOOLEAN:
                attributes->type = AT_BOOLEAN;
                break;
            case AT_INT8:
                attributes->type = AT_INT8;
                break;
            case AT_UINT8:
                attributes->type = AT_UINT8;
                break;
            case AT_INT16:
                attributes->type = AT_INT16;
                break;
            case AT_UINT16:
                attributes->type = AT_UINT16;
                break;
            case AT_INT32:
                attributes->type = AT_INT32;
                break;
            case AT_UINT32:
                attributes->type = AT_UINT32;
                break;
            case AT_INT64:
                attributes->type = AT_INT64;
                break;
            case AT_UINT64:
                attributes->type = AT_UINT64;
                break;
            case AT_STRING:
                attributes->type = AT_STRING;
                StringCchCopyWithAlloc(&(attributes->PWStr), MAX_PATH, L"String Attribute Value");
                break;
            case AT_GUID:
                attributes->type = AT_GUID;
                break;
            case AT_LIFE_TIME:
                attributes->type = AT_LIFE_TIME;
            break;
            case AT_SOCKADDR:
                attributes->type = AT_SOCKADDR;
                break;
            case AT_OCTET_STRING:
                attributes->type = AT_OCTET_STRING;
                attributes->OctetString.dwLength = 32;
                attributes->OctetString.lpValue = (BYTE *)CoTaskMemAlloc(32*sizeof(BYTE));
                if(attributes->OctetString.lpValue==NULL)
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
                SecureZeroMemory(attributes->OctetString.lpValue,32*sizeof(BYTE));
                break;
        }

        if (FAILED (hr))
        {
            break;
        }

        if (hypothses->celt!=0)
        {
            hr = StringCchCopyWithAlloc(&(attributes->pwszName), MAX_PATH, L"TestAttribute");
            if (FAILED (hr))
            {
                break;
            }
        }

        *pcelt = 1;
        if (pprgHypotheses)
        {    
            *pprgHypotheses = hypothses;
        }

    } while (FALSE);

    if ((FAILED (hr)) &&
        (hypothses))
    {
        CoTaskMemFree(hypothses->pwszClassName);

        attributes = hypothses->rgAttributes;
        if (attributes)
        {
            for (UINT i = 0; i < hypothses->celt; i++, ++attributes)
            {     
                CoTaskMemFree(attributes->pwszName);
                switch(attributes->type)
                {
                    case AT_STRING:
                        if(attributes->PWStr!=NULL) CoTaskMemFree(attributes->PWStr);
                    case AT_OCTET_STRING:
                        if(attributes->OctetString.lpValue!=NULL) CoTaskMemFree(attributes->OctetString.lpValue);
                }
            }
            CoTaskMemFree(attributes);
        }
        CoTaskMemFree(hypothses);
    }

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetDownStreamHypotheses( 
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pcelt);
    UNREFERENCED_PARAMETER (pprgHypotheses);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetHigherHypotheses( 
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pcelt);
    UNREFERENCED_PARAMETER (pprgHypotheses);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetUpStreamHypotheses( 
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) HYPOTHESIS **pprgHypotheses
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pcelt);
    UNREFERENCED_PARAMETER (pprgHypotheses);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::Repair( 
    __RPC__in RepairInfo *pInfo,
    __RPC__out long *pDeferredTime,
    __RPC__out REPAIR_STATUS *pStatus
    )
{
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {
        if (m_initialized <= 0)
        {
            hr = E_UNEXPECTED;
            break;
        }

        if (pInfo == NULL || pDeferredTime == NULL || pStatus == NULL)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (!IsEqualGUID(pInfo->guid, gWirelessHelperExtensionRepairId))
        {
            hr = E_NOTIMPL;
            break;
        }

        *pDeferredTime = 0;
        *pStatus = RS_REPAIRED;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::Validate( 
    PROBLEM_TYPE problem,
    __RPC__out long *pDeferredTime,
    __RPC__out REPAIR_STATUS *pStatus
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (problem);
    UNREFERENCED_PARAMETER (pDeferredTime);
    UNREFERENCED_PARAMETER (pStatus);

    LogEntry (this);

    do
    {
        if (m_initialized <= 0)
        {
            hr = E_UNEXPECTED;
            break;
        }

        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetRepairInfo( 
    PROBLEM_TYPE problem,
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) RepairInfo **ppInfo
    )
{
    RepairInfo *info = NULL;
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (problem);

    LogEntry (this);

    do
    {
        if (m_initialized <= 0)
        {
            hr = E_UNEXPECTED;
            break;
        }

        if (pcelt == NULL)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (ppInfo == NULL)
        {
            hr = S_OK;
            break;
        }

        if(!m_ReturnRepair)
        {
            *pcelt=0;
            *ppInfo=NULL;

            hr = S_OK;
            break;
        }

        info = (RepairInfo *)CoTaskMemAlloc(sizeof(RepairInfo));
        if (info == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        SecureZeroMemory(info, sizeof(RepairInfo));

        info->guid = gWirelessHelperExtensionRepairId;
        hr = StringCchCopyWithAlloc(&(info->pwszClassName), MAX_PATH, L"SampleWirelessHelperExtension");
        if (FAILED (hr))
        {
            break;
        }

        hr = StringCchCopyWithAlloc(&(info->pwszDescription), MAX_PATH, L"Sample repair from SampleWirelessHelperExtension");
        if (FAILED (hr))
        {
            break;
        }

        info->cost = 5;
        info->sidType = WinBuiltinNetworkConfigurationOperatorsSid;
        info->flags = RF_INFORMATION_ONLY;
        info->scope = RS_SYSTEM;
        info->risk = RR_NORISK;
        info->UiInfo.type = UIT_NONE;

        *pcelt = 1;
        *ppInfo = info;

    } while (FALSE);
    
    if ((FAILED (hr)) &&
        (info))
    {            
        CoTaskMemFree(info->pwszClassName);
        CoTaskMemFree(info->pwszDescription);
        CoTaskMemFree(info);
    }
    
    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetLifeTime( 
    __RPC__out LIFE_TIME *pLifeTime
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pLifeTime);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::SetLifeTime( 
    LIFE_TIME lifeTime
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (lifeTime);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetCacheTime( 
    __RPC__out FILETIME *pCacheTime
    )
{
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {
        SecureZeroMemory (pCacheTime, sizeof(FILETIME));

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT  
CWirelessHelperExtension::GetAttributes( 
    __RPC__out ULONG *pcelt,
    __RPC__deref_out_ecount_full_opt(*pcelt) HELPER_ATTRIBUTE **pprgAttributes
    )
{
    HRESULT     hr = S_OK;

    UNREFERENCED_PARAMETER (pcelt);
    UNREFERENCED_PARAMETER (pprgAttributes);

    LogEntry (this);

    do
    {
        hr = E_NOTIMPL;

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT 
CWirelessHelperExtension::Cancel(
    )
{
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {

    } while (FALSE);

    LogExit (hr);
    return (hr);
}

HRESULT 
CWirelessHelperExtension::Cleanup(
    )
{
    HRESULT     hr = S_OK;

    LogEntry (this);

    do
    {

    } while (FALSE);

    LogExit (hr);
    return (hr);
}
