// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//
//  Copyright (C) Microsoft Corporation, 2009
//
//  File:       pch.h
//
//--------------------------------------------------------------------------

#define CRYPT_OID_INFO_HAS_EXTRA_FIELDS

#include <windows.h>
#include <winerror.h>
#include <wincrypt.h>
#include <strsafe.h>
#include <CryptXml.h>

/*****************************************************************************
  HrLoadFile

    Load file into allocated (*ppbData). 
    The caller must free the memory by LocalFree().
*****************************************************************************/
HRESULT
HrLoadFile(
    LPCWSTR     wszFileName,
    BYTE        **ppbData,
    DWORD       *pcbData
    );

/****************************************************************************
 HrSampleResolveExternalXmlReference

****************************************************************************/
HRESULT 
WINAPI 
HrSampleResolveExternalXmlReference(
    LPCWSTR                 wszUri,
    CRYPT_XML_DATA_PROVIDER *pProviderOut
    );

/*****************************************************************************
 HrVerify

*****************************************************************************/
HRESULT
HrVerify(
    LPCWSTR         wszFileIn
    );


/*****************************************************************************
 SIGN_PARA

*****************************************************************************/
typedef struct SIGN_PARA
{
    LPCWSTR         wszCanonicalizationMethod;
    LPCWSTR         wszFileIn;
    LPCWSTR         wszSubject;
    LPCWSTR         wszKeyInfoId;
    LPCWSTR         wszSignatureId;
    LPCWSTR         wszSignatureLocation;
    LPCWSTR         wszHashAlgName;
    BOOL            fKV;                        // Create <KeyValue>
} SIGN_PARA;

/*****************************************************************************
 HrSign

*****************************************************************************/
HRESULT
HrSign(
    LPCWSTR         wszFileOut,
    const SIGN_PARA *pPara,
    ULONG           argc,
    LPWSTR          argv[]
    );