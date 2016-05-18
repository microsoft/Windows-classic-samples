// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

// helper macro for simple operations
#define EXEC_CHECKHR(s) {if (SUCCEEDED(hr)) { hr = s; } }

typedef enum
{
    CAPTYPE_HASH_ALGS = 1,
    CAPTYPE_ASSYM_KEYS,
    CAPTYPE_SIGNING_ALGS,
    CAPTYPE_CERT_SUPPORT,
    CAPTYPE_OPT_FEATURES,
} SILO_CAPABLITY_TYPE;

typedef enum
{
    // used to remove specific certificate
    CERTTYPE_REMOVE = 0,
    // Manufacturer Certificate Chain
    CERTTYPE_ASCM,
    // Provisioning Certificate
    CERTTYPE_PCP,
    // Authentication Silo Certificate
    CERTTYPE_ASCH,
    // Host Certificate
    CERTTYPE_HCH,
    // Signer Certificate
    CERTTYPE_SCH,
} CERTIFICATE_TYPES;

// see chapter 9.2.2 in IEEE 1667 spec
typedef enum
{
    CERTVP_NONE = 1,
    CERTVP_BASIC,
    CERTVP_EXTENDED,
} CERTIFICATE_VALIDATION_POLICIES;

typedef enum
{
    PASSWD_INDICATOR_ADMIN = 0,
    PASSWD_INDICATOR_USER,
} PASSWD_INDICATOR;

class CCertProperties
{
public:
    CCertProperties()
    {
        nIndex              = -1;
        nCertType           = CERTTYPE_REMOVE;
        nValidationPolicy   = CERTVP_NONE;
        nSignerCertIndex    = -1;
        nNextCertIndex      = -1;
        nNextCertType       = CERTTYPE_REMOVE;
        pCertificateData    = NULL;
        nCertificateDataLen = 0;
    }

    CCertProperties &operator=(CCertProperties& src)
    {
        if (src.pCertificateData && src.nCertificateDataLen)
        {
            nCertificateDataLen = src.nCertificateDataLen;
            pCertificateData = new BYTE[nCertificateDataLen];
            RtlCopyMemory(pCertificateData, src.pCertificateData, nCertificateDataLen);
        }

        nIndex              = src.nIndex;
        nCertType           = src.nCertType;
        nValidationPolicy   = src.nValidationPolicy;
        nSignerCertIndex    = src.nSignerCertIndex;
        nNextCertIndex      = src.nNextCertIndex;
        nNextCertType       = src.nNextCertType;

        return *this;

    }

    virtual ~CCertProperties()
    {
        if (pCertificateData)
        {
            delete[] pCertificateData;
            pCertificateData = NULL;
        }
    }

    BOOL get_CertType(LPTSTR szType, LONG nBufferSize);
    BOOL get_ValidationPolicy(LPTSTR szPolicy, LONG nBufferSize);

    void set_CertificateData(PBYTE pData, DWORD nDataCnt)
    {
        nCertificateDataLen = nDataCnt;
        pCertificateData = new BYTE[nCertificateDataLen];
        RtlCopyMemory(pCertificateData, pData, nCertificateDataLen);
    }

    const PBYTE get_CertificateData(DWORD &nDataCnt)
    {
        nDataCnt = nCertificateDataLen;
        return pCertificateData;
    }

    ULONG               nIndex;
    CERTIFICATE_TYPES   nCertType;
    CERTIFICATE_VALIDATION_POLICIES nValidationPolicy;
    ULONG               nSignerCertIndex;
    ULONG               nNextCertIndex;
    CERTIFICATE_TYPES   nNextCertType;
protected:
    PBYTE               pCertificateData;
    DWORD               nCertificateDataLen;
};

class CPortableDeviceDesc
{
public:
    CPortableDeviceDesc()
    {
        m_szPnpID = NULL;
        m_szManufacturer = NULL;
        m_szDescription = NULL;
    }
    
    virtual ~CPortableDeviceDesc()
    {
        if (m_szPnpID)
        {
            free(m_szPnpID);
            m_szPnpID = NULL;
        }

        if (m_szManufacturer)
        {
            free(m_szManufacturer);
            m_szManufacturer = NULL;
        }

        if (m_szDescription)
        {
            free(m_szDescription);
            m_szDescription = NULL;
        }
    }

    LPCTSTR get_PnpID() { return m_szPnpID; }
    void set_PnpID(LPCTSTR s)
    {
        if (s)
        {
            if (m_szPnpID) {
                free(m_szPnpID);
            }
            m_szPnpID = _tcsdup(s);
        }
    }

    LPCTSTR get_Manufacturer() { return m_szManufacturer; }
    void set_Manufacturer(LPCTSTR s)
    {
        if (s)
        {
            if (m_szManufacturer) {
                free(m_szManufacturer);
            }
            m_szManufacturer = _tcsdup(s);
        }
    }

    LPCTSTR get_Description() { return m_szDescription; }
    void set_Description(LPCTSTR s)
    {
        if (s)
        {
            if (m_szDescription) {
                free(m_szDescription);
            }
            m_szDescription = _tcsdup(s);
        }
    }
protected:
    LPTSTR m_szPnpID;             // device's PNP ID
    LPTSTR m_szManufacturer;      // device's manufacturer
    LPTSTR m_szDescription;       // device's description
};

class CPasswordSiloInformation
{
public:
    CPasswordSiloInformation()
    {
        RtlZeroMemory(&SiloInfo, sizeof(ENHANCED_STORAGE_PASSWORD_SILO_INFORMATION));
        dwAuthnState = 0;

        m_szAdminHint = NULL;
        m_szUserHint = NULL;
        m_szUserName = NULL;
        m_szSiloName = NULL;
    }

    virtual ~CPasswordSiloInformation()
    {
        if (m_szAdminHint)
        {
            free(m_szAdminHint);
            m_szAdminHint = NULL;
        }

        if (m_szUserHint)
        {
            free(m_szUserHint);
            m_szUserHint = NULL;
        }

        if (m_szUserName)
        {
            free(m_szUserName);
            m_szUserName = NULL;
        }

        if (m_szSiloName)
        {
            free(m_szSiloName);
            m_szSiloName = NULL;
        }
    }

    void set_AdminHint(LPCTSTR szHint)
    {
        if (szHint)
        {
            if (m_szAdminHint) {
                free(m_szAdminHint);
            }
            m_szAdminHint = _tcsdup(szHint);
        }
    }
    LPCTSTR get_AdminHint() { return m_szAdminHint; }

    void set_UserHint(LPCTSTR szHint)
    {
        if (szHint)
        {
            if (m_szUserHint) {
                free(m_szUserHint);
            }
            m_szUserHint = _tcsdup(szHint);
        }
    }
    LPCTSTR get_UserHint() { return m_szUserHint; }

    void set_UserName(LPCTSTR szUserName)
    {
        if (szUserName)
        {
            if (m_szUserName) {
                free(m_szUserName);
            }
            m_szUserName = _tcsdup(szUserName);
        }
    }
    LPCTSTR get_UserName() { return m_szUserName; }

    void set_SiloName(LPCTSTR szSiloName)
    {
        if (szSiloName)
        {
            if (m_szSiloName) {
                free(m_szSiloName);
            }
            m_szSiloName = _tcsdup(szSiloName);
        }
    }
    LPCTSTR get_SiloName() { return m_szSiloName; }

    ENHANCED_STORAGE_PASSWORD_SILO_INFORMATION SiloInfo;
    DWORD   dwAuthnState;
protected:
    LPTSTR m_szAdminHint;
    LPTSTR m_szUserHint;
    LPTSTR m_szUserName;
    LPTSTR m_szSiloName;
};

// class to manipulate device command and its parameters
class CPortableDeviceCmdParams
{
public:
    CPortableDeviceCmdParams();
    virtual ~CPortableDeviceCmdParams();

    HRESULT SetCommandId(PROPERTYKEY key);
    HRESULT SetValue(PROPERTYKEY key, LPCTSTR str);
    HRESULT SetValue(PROPERTYKEY key, ULONG nValue);
    HRESULT SetBuffer(PROPERTYKEY key, const PBYTE pData, DWORD nDataLength);
    operator IPortableDeviceValues* ( ) const
    {
        return m_pParams;
    }
protected:
    IPortableDeviceValues *m_pParams;
};

// class to manipulate command result and returning values
class CPortableDeviceCmdResults
{
public:
    CPortableDeviceCmdResults()
    {
        m_pResults = NULL;
    }
    virtual ~CPortableDeviceCmdResults()
    {
        if (m_pResults)
        {
            m_pResults->Release();
            m_pResults = NULL;
        }
    }

    HRESULT GetCommandResult();
    HRESULT GetValue(PROPERTYKEY key, LPWSTR szValue, LONG nBufferSize);
    HRESULT GetValue(PROPERTYKEY key, ULONG &nValue);
    HRESULT GetBuffer(PROPERTYKEY key, PBYTE &pData, DWORD &nDataLen);
    HRESULT GetPasswordSiloInfo(CPasswordSiloInformation &siloInfo);
public:
    IPortableDeviceValues *m_pResults;
};


// class to hold client information required to open WPD device
class CClientInformation
{
public:
    CClientInformation();
    virtual ~CClientInformation();
    operator IPortableDeviceValues* ( ) const
    {
        return m_pValues;
    }
protected:
    IPortableDeviceValues *m_pValues;
};


// class that manages device
class CPortableDeviceImp
{
public:
    CPortableDeviceImp()
    {
        m_pPortableDevice = NULL;
    }
    virtual ~CPortableDeviceImp()
    {
        if (m_pPortableDevice)
        {
            m_pPortableDevice->Release();
            m_pPortableDevice = NULL;
        }
    }

    // open device
    HRESULT OpenDevice(LPCTSTR szPNPid);

    // password silo
    HRESULT PasswordQueryInformation(CPasswordSiloInformation &siloInformation);
    HRESULT PasswordChangePassword(PASSWD_INDICATOR nPasswordIndicator,     // type of password to be changed
                                   LPCTSTR szOldPassword,                  // Old password, leave blank if set
                                   LPCTSTR szNewPassword,                  // New password
                                   LPCTSTR szPasswordHint,                 // Password hint
                                   LPCTSTR szSID);                         // Device Security ID (if supported)
    HRESULT PasswordAuthorizeActAccess(PASSWD_INDICATOR nPasswordIndicator, LPCTSTR szPassword);
    HRESULT PasswordUnauthorizeActAccess();
    HRESULT PasswordConfigAdministrator(LPCTSTR szAdminPassword,
                                        ULONG nMaxAuthFailures,
                                        LPCTSTR szSiloName);
    HRESULT PasswordCreateUser(LPCTSTR szAdminPassword,
                               LPCTSTR szUserName,
                               LPCTSTR szUserPassword,
                               LPCTSTR szUserPasswordHint);
    HRESULT PasswordDeleteUser(LPCTSTR szAdminPassword);
    HRESULT PasswordInitializeUserPassword(
                                LPCTSTR szAdminPassword,
                                LPCTSTR szUserPassword,
                                LPCTSTR szUserPasswordHint);
    HRESULT PasswordInitializeToManufacturerState(LPCTSTR szSID);

    // certificate silo
    HRESULT CertGetSiloFriendlyName(LPTSTR szFriendlyName, LONG nBufferSize);
    HRESULT CertGetSiloGUID(LPTSTR szGUID, LONG nBufferSize);
    HRESULT CertGetCertificatesCount(ULONG &nStoredCertCount, ULONG &nMaxCertCount);
    HRESULT CertGetState(ULONG &nState, LPTSTR szState, LONG nBufferSize);
    HRESULT CertHostAuthentication();
    HRESULT CertAdminAuthentication();
    HRESULT CertDeviceAuthentication(LONG nCertificateIndex = -1);
    HRESULT CertUnAuthentication();
    HRESULT CertGetSiloCapablity(SILO_CAPABLITY_TYPE nCapablityType, LPTSTR *&ppCapablities, ULONG &nCapablitiesCnt);
    HRESULT CertGetSiloCapablities();
    HRESULT CertGetCertificate(ULONG nCertIndex, CCertProperties &certProperties);
    HRESULT CertSetCertificate(ULONG nCertIndex, CCertProperties &certProperties);
    HRESULT CertRemoveCertificate(ULONG nCertIndex);
    HRESULT CertCreateCertificateRequest(PBYTE &pRequestData, DWORD &nRequestLength);
    HRESULT CertInitializeToManufacturedState();
protected:
    IPortableDevice *m_pPortableDevice;
};

class CPortableDeviceManagerImp
{
public:
    CPortableDeviceManagerImp();
    virtual ~CPortableDeviceManagerImp();

    HRESULT InitManager();
    HRESULT EnumerateDevices(CPortableDeviceDesc *&arDevices, DWORD &nDeviceCount);
protected:
    // Holds referrence to IPortableDeviceManager we wrap
    IPortableDeviceManager *m_pPortableDeviceManager;
};
