// CAPIWrappers.h - Security items for CAPIWrappers

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include <windows.h>
#include <stdio.h>
#include <wincrypt.h>
#include "rpc.h"

#pragma once

#define DEFAULT_ALGORITHM                   szOID_RSA_SHA1RSA
#define DEFAULT_KEY_SPEC                    AT_KEYEXCHANGE
#define DEFAULT_ENCODING                    (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)
#define MAX_CONTAINER_NAME_LEN              256
#define DEFAULT_PROV_TYPE                   PROV_RSA_AES
#define DEFAULT_PUBLIC_KEY_INFO_SIZE        400
#define DEFAULT_CRYPT_PROV_INFO_SIZE        1024
#define MAX_SIGNATURE_LENGTH                16
#define DEFAULT_KEYPAIR_EXPORT_SIZE         1024
#define MAX_KEYCONTAINER_NAME_LENGTH        256
#define SHA1_LENGTH                         20
#define SHA1_LENGTH_IN_CHARS                SHA1_LENGTH * 2     // two chars per byte
#define MD5_LENGTH                          16

HRESULT ReadCertFromFile(LPCWSTR pwzFileName, 
                         CERT_CONTEXT** ppCert, 
                         HCRYPTPROV* phCryptProv
                         );

HRESULT MakeCert(LPCWSTR pwzLocalCertFileName, 
                 LPCWSTR pwzLocalCertName, 
                 LPCWSTR pwzIssuerCertFileName, 
                 LPCWSTR pwzIssuerCertName
                 );

//******************************************************************************
//
// Class: XCERT_CONTEXT
//
// Description: smart pointer for CertContext objects
//
//******************************************************************************

class XCERT_CONTEXT
{
private:
    CERT_CONTEXT* p;

public:
    XCERT_CONTEXT(CERT_CONTEXT* ptr = NULL) : p(ptr) {}
    ~XCERT_CONTEXT()
    {
        if(p && p != NULL)
        {
            CertFreeCertificateContext(p);
        }
    }
    CERT_CONTEXT** operator&(){return &p;}
    CERT_CONTEXT* operator->(){ return p; }
    operator CERT_CONTEXT*(){return p;}

    void operator=(CERT_CONTEXT* ptr)
    {
        if(p && p != NULL)
        {
            CertFreeCertificateContext(p);
        }
        p = ptr;
    }
};

//******************************************************************************
//
// Class: XHCERTSTORE
//
// Description: Smart pointer for cert store objects
//
//******************************************************************************

class XHCERTSTORE
{
private:
    HCERTSTORE h;

public:
    XHCERTSTORE(HCERTSTORE handle = NULL) : h(handle) {}
    ~XHCERTSTORE()
    {
        if(h && h != NULL)
        {
            CertCloseStore(h, 0);
        }
    }
    HCERTSTORE* operator&(){return &h;}
    operator HCERTSTORE(){return h;}

    void operator=(HCERTSTORE handle)
    {
        if(h && h != NULL)
        {
            CertCloseStore(h, 0);
        }
        h = handle;
    }
};


//******************************************************************************
//
// Class:
//
// Description:
//
//******************************************************************************

class XHCRYPTPROV
{
private:
    HCRYPTPROV _h;

public:
    XHCRYPTPROV(HCRYPTPROV h = NULL) : _h(h) {}
    ~XHCRYPTPROV()
    {
        if(_h && _h != NULL)
        {
            CryptReleaseContext(_h, 0);
        }
    }
    HCRYPTPROV* operator&(){return &_h;}
    operator HCRYPTPROV(){return _h;}

    void operator=(HCRYPTPROV h)
    {
        if(_h && _h != NULL)
        {
            CryptReleaseContext(_h, 0);
        }
        _h = h;
    }
};

