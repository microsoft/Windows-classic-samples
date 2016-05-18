// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "StdAfx.h"
#include "PortableDeviceManagerImp.h"

CClientInformation::CClientInformation()
{
    HRESULT hr = S_OK;

    EXEC_CHECKHR(::CoCreateInstance(CLSID_PortableDeviceValues,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceValues,
                          (VOID**)&m_pValues));
    EXEC_CHECKHR(m_pValues->SetStringValue(WPD_CLIENT_NAME, L"EhStorEnumerator"));
    EXEC_CHECKHR(m_pValues->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1));
    EXEC_CHECKHR(m_pValues->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 1));
    EXEC_CHECKHR(m_pValues->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 1));
    EXEC_CHECKHR(m_pValues->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION));
}

CClientInformation::~CClientInformation()
{
    if (m_pValues)
    {
        m_pValues->Release();
        m_pValues = NULL;
    }
}

BOOL CCertProperties::get_CertType(LPTSTR szType, LONG nBufferSize)
{
    if (szType == NULL || nBufferSize <= 1) {
        return FALSE;
    }

    switch(nCertType)
    {
    case CERTTYPE_REMOVE:
        StringCchCopy (szType, nBufferSize, _T("Empty"));
        break;
    case CERTTYPE_ASCM:
        StringCchCopy (szType, nBufferSize, _T("ASCm"));
        break;
    case CERTTYPE_PCP:
        StringCchCopy (szType, nBufferSize, _T("PCp"));
        break;
    case CERTTYPE_ASCH:
        StringCchCopy (szType, nBufferSize, _T("ASCh"));
        break;
    case CERTTYPE_HCH:
        StringCchCopy (szType, nBufferSize, _T("HCh"));
        break;
    case CERTTYPE_SCH:
        StringCchCopy (szType, nBufferSize, _T("SCh"));
        break;
    default:
        StringCchCopy (szType, nBufferSize, _T("Invalid"));
        break;
    }

    return TRUE;
}

BOOL CCertProperties::get_ValidationPolicy(LPTSTR szPolicy, LONG nBufferSize)
{
    if (szPolicy == NULL || nBufferSize <= 1) {
        return FALSE;
    }

    switch (nValidationPolicy)
    {
    case CERTVP_NONE:
        StringCchCopy (szPolicy, nBufferSize, _T("None"));
        break;
    case CERTVP_BASIC:
        StringCchCopy (szPolicy, nBufferSize, _T("Basic"));
        break;
    case CERTVP_EXTENDED:
        StringCchCopy (szPolicy, nBufferSize, _T("Extended"));
        break;
    default:
        StringCchCopy (szPolicy, nBufferSize, _T("Invalid"));
        break;
    }

    return TRUE;
}


CPortableDeviceCmdParams::CPortableDeviceCmdParams()
{
    ::CoCreateInstance(CLSID_PortableDeviceValues,
                      NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_IPortableDeviceValues,
                      (VOID**)&m_pParams);
}

CPortableDeviceCmdParams::~CPortableDeviceCmdParams()
{
    if (m_pParams)
    {
        m_pParams->Release();
        m_pParams = NULL;
    }
}

HRESULT CPortableDeviceCmdParams::SetCommandId(PROPERTYKEY key)
{
    HRESULT hr = S_OK;

    EXEC_CHECKHR(m_pParams->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY, key.fmtid));
    EXEC_CHECKHR(m_pParams->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID, key.pid));

    return hr;
}

HRESULT CPortableDeviceCmdParams::SetValue(PROPERTYKEY key, LPCTSTR str)
{
    HRESULT hr = S_OK;
    WCHAR szUTF8String[256] = {0};
    DWORD cbMultiChar = 0;

    // WideCharToMultiByte fails on empty strings
    if (str && _tcslen(str) > 0)
    {
        cbMultiChar = WideCharToMultiByte(CP_UTF8,
                                          0,
                                          str,
                                          (int)_tcslen(str),
                                          (LPSTR)szUTF8String,
                                          _countof(szUTF8String),
                                          NULL,
                                          NULL);
        if (cbMultiChar == 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    EXEC_CHECKHR(m_pParams->SetBufferValue(key, (PBYTE)szUTF8String, cbMultiChar));

    return hr;
}

HRESULT CPortableDeviceCmdParams::SetValue(PROPERTYKEY key, ULONG nValue)
{
    return m_pParams->SetUnsignedIntegerValue(key, nValue);
}

HRESULT CPortableDeviceCmdParams::SetBuffer(PROPERTYKEY key, const PBYTE pData, DWORD nDataLength)
{
    return m_pParams->SetBufferValue(key, pData, nDataLength);
}


HRESULT CPortableDeviceCmdResults::GetCommandResult()
{
    HRESULT hr;
    HRESULT status;

    hr = m_pResults->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &status);

    return (FAILED(hr)) ? hr : status;
}

HRESULT CPortableDeviceCmdResults::GetValue(PROPERTYKEY key, LPWSTR szValue, LONG nBufferSize)
{
    HRESULT hr = S_OK;
    LPWSTR pwszValue = NULL;

    if (szValue == NULL || nBufferSize == 0)
    {
        return E_INVALIDARG;
    }

    szValue[0] = 0;
    hr = m_pResults->GetStringValue(key, &pwszValue);
    if (SUCCEEDED(hr) && pwszValue)
    {
        StringCchCopy(szValue, nBufferSize, pwszValue);
        CoTaskMemFree(pwszValue);
    }

    return hr;
}

HRESULT CPortableDeviceCmdResults::GetValue(PROPERTYKEY key, ULONG &nValue)
{
    nValue = 0;
    return m_pResults->GetUnsignedIntegerValue(key, &nValue);
}

HRESULT CPortableDeviceCmdResults::GetBuffer(PROPERTYKEY key, PBYTE &pData, DWORD &nDataLen)
{
    HRESULT hr = S_OK;
    PBYTE pbBuffer = NULL;

    nDataLen = 0;
    pData = NULL;

    hr = m_pResults->GetBufferValue(key, &pbBuffer, &nDataLen);
    if (FAILED(hr) || (nDataLen == 0) || (pbBuffer == NULL))
    {
        return hr;
    }

    pData = new BYTE[nDataLen];
    RtlCopyMemory(pData, pbBuffer, nDataLen);

    CoTaskMemFree(pbBuffer);
    pbBuffer = NULL;

    return hr;
}

HRESULT CPortableDeviceCmdResults::GetPasswordSiloInfo(CPasswordSiloInformation &siloInfo)
{
    HRESULT hr = S_OK;
    PBYTE pbBuffer = NULL;
    LPTSTR szStringBuffer = NULL;
    LONG nStringBufferSize = 0;
    DWORD cbBuffer = 0;

    EXEC_CHECKHR(m_pResults->GetBufferValue(ENHANCED_STORAGE_PROPERTY_PASSWORD_SILO_INFO, &pbBuffer, &cbBuffer));
    if (SUCCEEDED(hr) && pbBuffer)
    {
        if(cbBuffer == sizeof(ENHANCED_STORAGE_PASSWORD_SILO_INFORMATION))
        {
            RtlCopyMemory(&siloInfo.SiloInfo, pbBuffer, sizeof(ENHANCED_STORAGE_PASSWORD_SILO_INFORMATION));
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        CoTaskMemFree(pbBuffer);
        pbBuffer = NULL;
    }
    
    EXEC_CHECKHR(m_pResults->GetUnsignedIntegerValue(
                                ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_STATE,
                                &siloInfo.dwAuthnState));

    // retrieve admin hint
    if (SUCCEEDED(hr))
    {
        nStringBufferSize = siloInfo.SiloInfo.MaxAdminHintSize + 1;
        szStringBuffer = (LPTSTR) malloc(nStringBufferSize * sizeof(TCHAR));
        if (szStringBuffer == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }
    EXEC_CHECKHR(GetValue(ENHANCED_STORAGE_PROPERTY_ADMIN_HINT, szStringBuffer, nStringBufferSize));
    if (SUCCEEDED(hr))
    {
        siloInfo.set_AdminHint(szStringBuffer);
        free(szStringBuffer);
    }

    // retrieve user hint
    if (SUCCEEDED(hr))
    {
        nStringBufferSize = siloInfo.SiloInfo.MaxUserHintSize + 1;
        szStringBuffer = (LPTSTR) malloc(nStringBufferSize * sizeof(TCHAR));
        if (szStringBuffer == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }
    EXEC_CHECKHR(GetValue(ENHANCED_STORAGE_PROPERTY_USER_HINT, szStringBuffer, nStringBufferSize));
    if (SUCCEEDED(hr))
    {
        siloInfo.set_UserHint(szStringBuffer);
        free(szStringBuffer);
    }

    // retrieve user name
    if (SUCCEEDED(hr))
    {
        nStringBufferSize = siloInfo.SiloInfo.MaxUserNameSize + 1;
        szStringBuffer = (LPTSTR) malloc(nStringBufferSize * sizeof(TCHAR));
        if (szStringBuffer == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }
    EXEC_CHECKHR(GetValue(ENHANCED_STORAGE_PROPERTY_USER_NAME, szStringBuffer, nStringBufferSize));
    if (SUCCEEDED(hr))
    {
        siloInfo.set_UserName(szStringBuffer);
        free(szStringBuffer);
    }

    // retrieve silo name
    if (SUCCEEDED(hr))
    {
        nStringBufferSize = siloInfo.SiloInfo.MaxSiloNameSize + 1;
        szStringBuffer = (LPTSTR) malloc(nStringBufferSize * sizeof(TCHAR));
        if (szStringBuffer == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }
    EXEC_CHECKHR(GetValue(ENHANCED_STORAGE_PROPERTY_SILO_NAME, szStringBuffer, nStringBufferSize));
    if (SUCCEEDED(hr))
    {
        siloInfo.set_SiloName(szStringBuffer);
        free(szStringBuffer);
    }

    return hr;
}


CPortableDeviceManagerImp::CPortableDeviceManagerImp()
{
    m_pPortableDeviceManager = NULL;
}

CPortableDeviceManagerImp::~CPortableDeviceManagerImp()
{
    if (m_pPortableDeviceManager)
    {
        m_pPortableDeviceManager->Release();
        m_pPortableDeviceManager = NULL;
    }
}

// Init portable device manager
HRESULT CPortableDeviceManagerImp::InitManager()
{
    HRESULT hr = S_OK;

    hr = CoInitialize(NULL);

    EXEC_CHECKHR(CoCreateInstance(CLSID_PortableDeviceManager,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IPortableDeviceManager,
                          (VOID**)&m_pPortableDeviceManager));

    return hr;
}

// Enumerate devices in system
HRESULT CPortableDeviceManagerImp::EnumerateDevices(CPortableDeviceDesc *&arDevices, DWORD &nDeviceCount)
{
    HRESULT hr            = S_OK;
    LPWSTR* pPnpDeviceIDs = NULL;
    DWORD nIndex;

    if (!m_pPortableDeviceManager)
    {
        return E_FAIL;
    }

    // refresh devices list
    m_pPortableDeviceManager->RefreshDeviceList();

    // determine number of devices in system
    EXEC_CHECKHR(m_pPortableDeviceManager->GetDevices(NULL, &nDeviceCount));
    if (nDeviceCount == 0)
    {
        // no devices found
        return hr;
    }

    if (SUCCEEDED(hr))
    {
        pPnpDeviceIDs = new LPWSTR[nDeviceCount];
        if (pPnpDeviceIDs)
        {
            RtlZeroMemory(pPnpDeviceIDs, nDeviceCount * sizeof(LPWSTR*));
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    EXEC_CHECKHR(m_pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &nDeviceCount));

    // walk trough devices and collect information
    if (SUCCEEDED(hr))
    {
        arDevices = new CPortableDeviceDesc[nDeviceCount];

        for (nIndex = 0; nIndex < nDeviceCount; nIndex++)
        {
            CPortableDeviceDesc &device = arDevices[nIndex];
            DWORD cchResult;
            LPWSTR wszResult;

            if (pPnpDeviceIDs[nIndex] == NULL)
            {
                continue;
            }

            device.set_PnpID(pPnpDeviceIDs[nIndex]);

            // retreive device's manufacturer

            // Pass NULL as the out buffer to get the total number of characters to allocate room for the string value.
            cchResult = 0;
            hr = m_pPortableDeviceManager->GetDeviceManufacturer(pPnpDeviceIDs[nIndex], NULL, &cchResult);
            if (SUCCEEDED(hr))
            {
                wszResult = new WCHAR[cchResult];
                if (wszResult)
                {
                    hr = m_pPortableDeviceManager->GetDeviceManufacturer(pPnpDeviceIDs[nIndex], wszResult, &cchResult);
                    if (SUCCEEDED(hr))
                    {
                        device.set_Manufacturer(wszResult);
                    }
                    delete [] wszResult;
                    wszResult = NULL;
                }
            }

            // retreive device's description

            cchResult = 0;
            hr = m_pPortableDeviceManager->GetDeviceDescription(pPnpDeviceIDs[nIndex], NULL, &cchResult);
            if (SUCCEEDED(hr))
            {
                wszResult = new WCHAR[cchResult];
                if (wszResult)
                {
                    hr = m_pPortableDeviceManager->GetDeviceDescription(pPnpDeviceIDs[nIndex], wszResult, &cchResult);
                    if (SUCCEEDED(hr))
                    {
                        device.set_Description(wszResult);
                    }
                    delete [] wszResult;
                    wszResult = NULL;
                }
            }

            // we do not need device ID received, clean it up
            CoTaskMemFree(pPnpDeviceIDs[nIndex]);
            pPnpDeviceIDs[nIndex] = NULL;
        }

        // we are success if at least one device retreived
        hr = nDeviceCount ? S_OK : E_FAIL;
    }

    // delete the array
    if (pPnpDeviceIDs)
    {
        delete[] pPnpDeviceIDs;
        pPnpDeviceIDs = NULL;
    }

    return hr;
}

HRESULT CPortableDeviceImp::OpenDevice(LPCTSTR szPNPid)
{
    HRESULT hr = S_OK;
    CClientInformation clientInfo;

    EXEC_CHECKHR(::CoCreateInstance(CLSID_PortableDevice,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IPortableDevice,
                          (VOID**)&m_pPortableDevice));

    EXEC_CHECKHR(m_pPortableDevice->Open(szPNPid, clientInfo));

    return hr;
}

HRESULT CPortableDeviceImp::PasswordQueryInformation(CPasswordSiloInformation &siloInformation)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_QUERY_INFORMATION));
    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));
    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());
    // if we succeeded we may retreive data block
    EXEC_CHECKHR(cmdResults.GetPasswordSiloInfo(siloInformation));

    return hr;
}

HRESULT CPortableDeviceImp::PasswordChangePassword(
                                PASSWD_INDICATOR nPasswordIndicator,
                                LPCTSTR szOldPassword,
                                LPCTSTR szNewPassword,
                                LPCTSTR szPasswordHint,
                                LPCTSTR szSID)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_CHANGE_PASSWORD));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD_INDICATOR, nPasswordIndicator));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD, szOldPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD_INDICATOR, nPasswordIndicator));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD, szNewPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_SECURITY_IDENTIFIER, szSID));
    switch (nPasswordIndicator)
    {
    case PASSWD_INDICATOR_ADMIN:
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_ADMIN_HINT, szPasswordHint));
        break;
    case PASSWD_INDICATOR_USER:
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_USER_HINT, szPasswordHint));
        break;
    default:
        hr = E_INVALIDARG;
        break;
    }

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordAuthorizeActAccess(PASSWD_INDICATOR nPasswordIndicator, LPCTSTR szPassword)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_AUTHORIZE_ACT_ACCESS));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD_INDICATOR, nPasswordIndicator));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD, szPassword));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordUnauthorizeActAccess()
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_UNAUTHORIZE_ACT_ACCESS));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordCreateUser(LPCTSTR szAdminPassword,
                                               LPCTSTR szUserName,
                                               LPCTSTR szUserPassword,
                                               LPCTSTR szUserPasswordHint)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_CREATE_USER));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD, szAdminPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_USER_NAME, szUserName));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD, szUserPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_USER_HINT, szUserPasswordHint));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordDeleteUser(LPCTSTR szAdminPassword)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_DELETE_USER));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD, szAdminPassword));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordConfigAdministrator(
                                    LPCTSTR szAdminPassword,
                                    ULONG nMaxAuthFailures,
                                    LPCTSTR szSiloName)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_CONFIG_ADMINISTRATOR));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD, szAdminPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_MAX_AUTH_FAILURES, nMaxAuthFailures));
    if (_tcslen(szSiloName))
    {
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_SILO_FRIENDLYNAME_SPECIFIED, 1));
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_SILO_NAME, szSiloName));
    }
    else
    {
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_SILO_FRIENDLYNAME_SPECIFIED, 0UL));
    }

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordInitializeUserPassword(
                                    LPCTSTR szAdminPassword,
                                    LPCTSTR szUserPassword,
                                    LPCTSTR szUserPasswordHint)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_INITIALIZE_USER_PASSWORD));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_PASSWORD, szAdminPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_NEW_PASSWORD, szUserPassword));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_USER_HINT, szUserPasswordHint));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::PasswordInitializeToManufacturerState(LPCTSTR szSID)
{
    HRESULT hr = S_OK;

    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_PASSWORD_START_INITIALIZE_TO_MANUFACTURER_STATE));

    // set data
    if (_tcslen(szSID))
    {
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_SECURITY_IDENTIFIER, szSID));
    }

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertGetSiloFriendlyName(LPTSTR szFriendlyName, LONG nBufferSize)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_GET_ACT_FRIENDLY_NAME));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_ACT_FRIENDLY_NAME, szFriendlyName, nBufferSize));

    return hr;
}

HRESULT CPortableDeviceImp::CertGetSiloGUID(LPTSTR szGUID, LONG nBufferSize)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_GUID));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_GUID, szGUID, nBufferSize));

    return hr;
}

HRESULT CPortableDeviceImp::CertGetCertificatesCount(ULONG &nStoredCertCount, ULONG &nMaxCertCount)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE_COUNT));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_STORED_CERTIFICATE_COUNT, nStoredCertCount));
    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_MAX_CERTIFICATE_COUNT, nMaxCertCount));

    return hr;
}

HRESULT CPortableDeviceImp::CertGetState(ULONG &nState, LPTSTR szState, LONG nBufferSize)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_SILO_GET_AUTHENTICATION_STATE));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_AUTHENTICATION_STATE, nState));

    if (SUCCEEDED(hr))
    {
        switch (nState)
        {
            case 0x0:
                StringCchCopy(szState, nBufferSize, _T("Unknown"));
                break;
            case 0x1:
                StringCchCopy(szState, nBufferSize, _T("No Authentication Required"));
                break;
            case 0x2:
                StringCchCopy(szState, nBufferSize, _T("Not Authenticated"));
                break;
            case 0x3:
                StringCchCopy(szState, nBufferSize, _T("Authenticated"));
                break;
            case 0x80000001:
                StringCchCopy(szState, nBufferSize, _T("Authentication Denied"));
                break;
            case 0x80000002:
                StringCchCopy(szState, nBufferSize, _T("Device Error"));
                break;
            default:
                StringCbPrintf(szState, nBufferSize, _T("Invalid(%d)"), nState);
                break;
        }
    }

    return hr;
}

HRESULT CPortableDeviceImp::CertHostAuthentication()
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_HOST_CERTIFICATE_AUTHENTICATION));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertDeviceAuthentication(LONG nCertificateIndex)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_DEVICE_CERTIFICATE_AUTHENTICATION));

    // set data
    if (nCertificateIndex >= 0)
    {
        EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX, nCertificateIndex));
    }

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertUnAuthentication()
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_UNAUTHENTICATION));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertGetSiloCapablity(SILO_CAPABLITY_TYPE nCapablityType, LPTSTR *&ppCapablities, ULONG &nCapablitiesCnt)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;
    DWORD nIndex;
    PBYTE pCapablityData = NULL;
    DWORD nDataLength = 0;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITY));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_CAPABILITY_TYPE, nCapablityType));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    EXEC_CHECKHR(cmdResults.GetBuffer(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITY, pCapablityData, nDataLength));

    // parse list of zero-separated strings to string array
    if (SUCCEEDED(hr) && pCapablityData && nDataLength)
    {
        ULONG nMaxCapablityLen = 0; // maximum capablity length
        ULONG nCapablityLen = 0;
        nCapablitiesCnt = 0;        // total capablities found
        LPTSTR szCapablity = NULL;

        // pass one, calculate sizes
        for (nIndex = 1; nIndex < nDataLength; nIndex ++)
        {
            if (pCapablityData[nIndex])
            {
                nCapablityLen ++;
            }
            else
            {
                if (nCapablityLen)
                {
                    nMaxCapablityLen = (nCapablityLen > nMaxCapablityLen) ? nCapablityLen : nMaxCapablityLen;
                    nCapablitiesCnt ++;
                    nCapablityLen = 0;
                }
            }
        }

        if (SUCCEEDED(hr))
        {
            ppCapablities = (LPTSTR*)malloc(nCapablitiesCnt * sizeof(LPTSTR));
            if (!ppCapablities) {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            szCapablity = (LPTSTR)(malloc)((nMaxCapablityLen + 1) * sizeof(TCHAR));
            if (!szCapablity) {
                hr = E_OUTOFMEMORY;
            }
        }

        if (SUCCEEDED(hr))
        {
            LONG nCapablityPtr = 0;
            LONG nCapablities = 0;

            // pass two, retreive data
            for (nIndex = 1; nIndex < nDataLength; nIndex ++)
            {
                if (pCapablityData[nIndex])
                {
                    szCapablity[nCapablityPtr ++] = (TCHAR)pCapablityData[nIndex];
                    szCapablity[nCapablityPtr] = 0;
                }
                else
                {
                    if (nCapablityPtr)
                    {
                        ppCapablities[nCapablities ++] = _tcsdup(szCapablity);
                        nCapablityPtr = 0;
                    }
                }
            }

            free(szCapablity);
        }

        delete[] pCapablityData;
    }

    return hr;
}

HRESULT CPortableDeviceImp::CertRemoveCertificate(ULONG nCertIndex)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_SET_CERTIFICATE));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX, nCertIndex));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_TYPE, CERTTYPE_REMOVE));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertGetCertificate(ULONG nCertIndex, CCertProperties &certProperties)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;
    PBYTE pCertData = NULL;
    DWORD nCertDataLen = 0;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_GET_CERTIFICATE));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX, nCertIndex));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    if (SUCCEEDED(hr))
    {
        certProperties.nIndex = nCertIndex;
    }

    EXEC_CHECKHR(cmdResults.GetBuffer(ENHANCED_STORAGE_PROPERTY_CERTIFICATE, pCertData, nCertDataLen));
    if (SUCCEEDED(hr))
    {
        if (pCertData && nCertDataLen)
        {
            certProperties.set_CertificateData(pCertData, nCertDataLen);
            delete[] pCertData;
        }
    }
    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_TYPE, (ULONG&)certProperties.nCertType));
    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_VALIDATION_POLICY, (ULONG&)certProperties.nValidationPolicy));
    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_SIGNER_CERTIFICATE_INDEX, certProperties.nSignerCertIndex));
    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_INDEX, certProperties.nNextCertIndex));
    EXEC_CHECKHR(cmdResults.GetValue(ENHANCED_STORAGE_PROPERTY_NEXT_CERTIFICATE_OF_TYPE_INDEX, (ULONG&)certProperties.nNextCertType));

    return hr;
}

HRESULT CPortableDeviceImp::CertSetCertificate(ULONG nCertIndex, CCertProperties &certProperties)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;
    DWORD nDataCnt = 0;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_SET_CERTIFICATE));

    // set data
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_INDEX, nCertIndex));
    certProperties.get_CertificateData(nDataCnt);
    EXEC_CHECKHR(cmdParams.SetBuffer(ENHANCED_STORAGE_PROPERTY_CERTIFICATE, certProperties.get_CertificateData(nDataCnt), nDataCnt));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_TYPE, certProperties.nCertType));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_VALIDATION_POLICY, certProperties.nValidationPolicy));
    EXEC_CHECKHR(cmdParams.SetValue(ENHANCED_STORAGE_PROPERTY_SIGNER_CERTIFICATE_INDEX, certProperties.nSignerCertIndex));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertCreateCertificateRequest(PBYTE &pRequestData, DWORD &nRequestLength)
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_CREATE_CERTIFICATE_REQUEST));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    EXEC_CHECKHR(cmdResults.GetBuffer(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_REQUEST, pRequestData, nRequestLength));

    return hr;
}

HRESULT CPortableDeviceImp::CertInitializeToManufacturedState()
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_INITIALIZE_TO_MANUFACTURER_STATE));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertAdminAuthentication()
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_ADMIN_CERTIFICATE_AUTHENTICATION));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    return hr;
}

HRESULT CPortableDeviceImp::CertGetSiloCapablities()
{
    HRESULT hr = S_OK;
    CPortableDeviceCmdParams  cmdParams;
    CPortableDeviceCmdResults cmdResults;
    IPortableDeviceValues *pSiloCapabilities = NULL;
    DWORD nCapablity, nNumCapablities = 0;

    // set command
    EXEC_CHECKHR(cmdParams.SetCommandId(ENHANCED_STORAGE_COMMAND_CERT_GET_SILO_CAPABILITIES));

    // send command to device
    EXEC_CHECKHR(m_pPortableDevice->SendCommand(0, cmdParams, &cmdResults.m_pResults));

    // retreive result
    EXEC_CHECKHR(cmdResults.GetCommandResult());

    // get data
    EXEC_CHECKHR(cmdResults.m_pResults->GetIPortableDeviceValuesValue(ENHANCED_STORAGE_PROPERTY_CERTIFICATE_SILO_CAPABILITIES,  &pSiloCapabilities));

    EXEC_CHECKHR(pSiloCapabilities->GetCount(&nNumCapablities));

    for (nCapablity = 0; nCapablity < nNumCapablities; nCapablity ++)
    {
        PROPERTYKEY propKey;
        PROPVARIANT propVariant;
        HRESULT locHR = S_OK;

        if (SUCCEEDED(locHR = pSiloCapabilities->GetAt(nCapablity, &propKey, &propVariant)))
        {
            // TODO: save result from propVariant before disposal
            PropVariantClear(&propVariant);
        }
    }

    return hr;
}
