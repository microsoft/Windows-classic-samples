// CustomSecurityProvider.cpp - Implementation of CustomSecurityProvider

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "CustomSecurityProvider.h"

HRESULT DrtCreateCustomSecurityProvider(DRT_SECURITY_PROVIDER** ppDrtSecurityProvider)
{
    if (!ppDrtSecurityProvider)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = CCustomNullSecurityProvider::Init(ppDrtSecurityProvider);

    return hr;
}

void DrtDeleteCustomSecurityProvider(DRT_SECURITY_PROVIDER* pDrtSecurityProvider)
{
    if (!pDrtSecurityProvider )
    {
        return;
    }

    CCustomNullSecurityProvider* pSecProvider = (CCustomNullSecurityProvider*)pDrtSecurityProvider->pvContext;
    delete pSecProvider;
    delete pDrtSecurityProvider;
}


HRESULT CCustomNullSecurityProvider::Init(__out DRT_SECURITY_PROVIDER** ppDrtSecurityProvider)
{
    DRT_SECURITY_PROVIDER* pDrtSecProvider = NULL;
    HRESULT hr = S_OK;

    // create and initialize public interface
    pDrtSecProvider = new DRT_SECURITY_PROVIDER;
    if (!pDrtSecProvider)
    {
        return E_OUTOFMEMORY;
    }

    pDrtSecProvider->SecureAndPackPayload = CCustomNullSecurityProvider::SecureAndPackPayload;
    pDrtSecProvider->ValidateAndUnpackPayload = CCustomNullSecurityProvider::ValidateAndUnpackPayload;
    pDrtSecProvider->RegisterKey = CCustomNullSecurityProvider::RegisterKey;
    pDrtSecProvider->UnregisterKey = CCustomNullSecurityProvider::UnregisterKey;
    pDrtSecProvider->FreeData = CCustomNullSecurityProvider::FreeData;
    pDrtSecProvider->Attach = CCustomNullSecurityProvider::Attach;
    pDrtSecProvider->Detach = CCustomNullSecurityProvider::Detach;
    pDrtSecProvider->EncryptData = CCustomNullSecurityProvider::EncryptData;
    pDrtSecProvider->DecryptData = CCustomNullSecurityProvider::DecryptData;
    pDrtSecProvider->ValidateRemoteCredential = CCustomNullSecurityProvider::ValidateRemoteCredential;
    pDrtSecProvider->GetSerializedCredential = CCustomNullSecurityProvider::GetSerializedCredential;
    pDrtSecProvider->SignData = CCustomNullSecurityProvider::SignData;
    pDrtSecProvider->VerifyData = CCustomNullSecurityProvider::VerifyData;
    pDrtSecProvider->pvContext = NULL;

    *ppDrtSecurityProvider = pDrtSecProvider;
    return hr;
}



HRESULT CCustomNullSecurityProvider::Attach(
    __in        const PVOID pContext
    )
{
    UNREFERENCED_PARAMETER(pContext);
    return S_OK;
}

VOID CCustomNullSecurityProvider::Detach(
    __in        const PVOID pContext
    )
{
    UNREFERENCED_PARAMETER(pContext);
    return;
}


// Purpose:  Validate the incoming data, and unpack the data that the DRT needs direct access to.
//
// Args:     [in]  pSecuredAddressPayload: signed/serialized addresses, flags, and nonce
//           [in]  pCertChain: serialized cert chain of publisher
//           [in]  pClassifier: unused
//           [in]  pNonce: nonce that should match the one in the SecuredAddressPayload
//           [in]  pSecuredPayload: signed application data
//
//           [out] pbProtocolMajor: protocol major version
//           [out] pbProtocolMinor: protocol minor version
//           [out] pKey: key
//           [out] pPayload: original application data
//           [out] ppAddressList: allocated service address list
//           [out] pdwFlags: flags
//
// Returns:  DRT_E_INVALID_MESSAGE for invalid message/security
//           S_OK for a good message
//
// Notes:    ppAddressList returns an array of address pointers. Each address must be freed before
//           freeing the array as a whole (i.e. the pointers are not self-referential)
//
HRESULT CCustomNullSecurityProvider::ValidateAndUnpackPayload(
    __in_opt    VOID* pvContext,
    __in        DRT_DATA* pSecuredAddressPayload,
    __in_opt    DRT_DATA* pCertChain,
    __in_opt    DRT_DATA* pClassifier,
    __in_opt    DRT_DATA* pNonce,
    __in_opt    DRT_DATA* pSecuredPayload,
    __out       BYTE* pbProtocolMajor,
    __out       BYTE* pbProtocolMinor,
    __out       DRT_DATA* pKey,
    __out_opt   DRT_DATA* pPayload,
    __out       CERT_PUBLIC_KEY_INFO** ppPublicKey,
    __out_opt   SOCKET_ADDRESS_LIST** ppAddressList,
    __out       DWORD* pdwFlags)
{
    UNREFERENCED_PARAMETER(pvContext);
    UNREFERENCED_PARAMETER(pCertChain);
    UNREFERENCED_PARAMETER(pClassifier);
    
    CCustomNullSecuredAddressPayload sap;
    HRESULT hr = S_OK;

    // NULL out the out params
    *pbProtocolMajor = 0;
    *pbProtocolMinor = 0;
    ZeroMemory(pKey, sizeof(DRT_DATA));
    if(pPayload != NULL)
    {
        ZeroMemory(pPayload, sizeof(DRT_DATA));
    }
    *ppPublicKey = NULL;
    *pdwFlags = 0;

    // deserialize Secured Address Payload
    hr = sap.DeserializeAndValidate(pSecuredAddressPayload, pNonce);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // When we asked for the payload validate signature of payload
    if (pPayload && pSecuredPayload)
    {
        pPayload->cb = pSecuredPayload->cb;
        pPayload->pb = (BYTE*)malloc(pPayload->cb);
        if (!pPayload->pb)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }
        CopyMemory(pPayload->pb, pSecuredPayload->pb, pPayload->cb);
    }

    *pdwFlags = sap.GetFlags();

    // everything is valid, time to extract the data
    if(ppAddressList != NULL)
    {
        *ppAddressList = sap.GetAddresses();
    }
    *ppPublicKey = sap.GetPublicKey();
    sap.GetKey(pKey);
    sap.GetProtocolVersion(pbProtocolMajor, pbProtocolMinor);

cleanup:
    // if something failed, free all the out params and NULL them out
    if (FAILED(hr))
    {
        *pbProtocolMajor = 0;
        *pbProtocolMinor = 0;
        *pdwFlags = 0;
        free(pKey->pb);
        ZeroMemory(pKey, sizeof(DRT_DATA));
        if(pPayload)
        {
            free(pPayload->pb);
            ZeroMemory(pPayload, sizeof(DRT_DATA));
        }
        free(*ppPublicKey);
        *ppPublicKey = NULL;

        // free all the addresses
        if(ppAddressList != NULL)
        {
            SOCKET_ADDRESS_LIST* pAddressList = *ppAddressList;
            if (pAddressList)
            {
                for (INT i = 0; i < pAddressList->iAddressCount; i++)
                    free(pAddressList->Address[i].lpSockaddr);
                free(pAddressList);
            }
            *ppAddressList = NULL;
        }
    }

    return hr;
}

// Purpose:  Serialize the data to be put on the wire and sign the payloads.
//
// Args:     [in]  pvKeyContext: unused
//           [in]  bProtocolMajor: protocol major version
//           [in]  bProtocolMinor: protocol minor version
//           [in]  dwFlags: flags
//           [in]  pKey: unused
//           [in]  pPayload: original application payload
//           [in]  pAddressList: DRT service addresses
//           [in]  pNonce: nonce to embed in the SecuredAddressPayload
//
//           [out] pSecuredAddressPayload: signed, serialized address payload
//           [out] pClassifier: empty
//           [out] pSecuredPayload: signed application data
//           [out] pCertChain: serialized local cert chain
//
HRESULT CCustomNullSecurityProvider::SecureAndPackPayload(
    __in_opt    VOID* pvContext,
    __in_opt    VOID* pvKeyContext,
                BYTE bProtocolMajor,
                BYTE bProtocolMinor,
                DWORD dwFlags,
    __in        const DRT_DATA* pKey,
    __in_opt    const DRT_DATA* pPayload,
    __in_opt    const SOCKET_ADDRESS_LIST* pAddressList,
    __in        const DRT_DATA* pNonce,
    __out       DRT_DATA* pSecuredAddressPayload,
    __out_opt   DRT_DATA* pClassifier,
    __out_opt   DRT_DATA* pSecuredPayload,
    __out_opt   DRT_DATA* pCertChain)
{
    UNREFERENCED_PARAMETER(pvContext);
    UNREFERENCED_PARAMETER(pvKeyContext);

    CCustomNullSecuredAddressPayload sap;
    HRESULT hr = S_OK;

    // NULL out the out params
    ZeroMemory(pSecuredAddressPayload, sizeof(DRT_DATA));
    if (pClassifier)
        ZeroMemory(pClassifier, sizeof(DRT_DATA));
    if (pSecuredPayload)
        ZeroMemory(pSecuredPayload, sizeof(DRT_DATA));
    if (pCertChain)
        ZeroMemory(pCertChain, sizeof(DRT_DATA));

    // set the payload contents
    sap.SetProtocolVersion(bProtocolMajor, bProtocolMinor);
    sap.SetKey(pKey);
    sap.SetAddresses(pAddressList);
    sap.SetNonce(pNonce);
    sap.SetFlags(dwFlags);

    hr = sap.SerializeAndSign(pSecuredAddressPayload);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    if (pPayload && pSecuredPayload)
    {
        pSecuredPayload->cb = pPayload->cb;
        pSecuredPayload->pb = (BYTE*)malloc(pSecuredPayload->cb);
        if (!pSecuredPayload->pb)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }
        CopyMemory(pSecuredPayload->pb, pPayload->pb, pSecuredPayload->cb);
    }

    // make a copy of the serialized local cert chain
    if (pCertChain)
    {
        pCertChain->cb = sizeof(DWORD);
        pCertChain->pb = (BYTE*)malloc(pCertChain->cb);
        if (!pCertChain->pb)
        {
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }
        DWORD dwDeadBeef = 0xdeadbeef;
        CopyMemory(pCertChain->pb, &dwDeadBeef, sizeof(DWORD));
    }


cleanup:
    // if something failed, free all the out params and NULL them out
    if (FAILED(hr))
    {
        free(pSecuredAddressPayload->pb);
        ZeroMemory(pSecuredAddressPayload, sizeof(DRT_DATA));
        if (pSecuredPayload)
        {
            free(pSecuredPayload->pb);
            ZeroMemory(pSecuredPayload, sizeof(DRT_DATA));
        }
        if (pCertChain)
        {
            free(pCertChain->pb);
            ZeroMemory(pCertChain, sizeof(DRT_DATA));
        }
    }

    return hr;
}

    HRESULT CCustomNullSecurityProvider::GetSerializedCredential(
        __in        const PVOID pvContext,
        __out DRT_DATA *pSelfCredential)
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(pSelfCredential);

        HRESULT hr = S_OK;
        pSelfCredential->cb = 0;
        pSelfCredential->pb = NULL;
        return hr;

    }

    HRESULT CCustomNullSecurityProvider::ValidateRemoteCredential(
        __in        const PVOID pvContext,
        __in DRT_DATA *pRemoteCredential)
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(pRemoteCredential);
        return S_OK;
    }

    HRESULT CCustomNullSecurityProvider::EncryptData(
        __in        const PVOID pvContext,
        __in        const DRT_DATA* pRemoteCredential,
        __in        DWORD dwBuffers,
        __in_ecount(dwBuffers)        DRT_DATA* pDataBuffers,
        __out_ecount(dwBuffers)        DRT_DATA* pEncryptedBuffers,
        __out       DRT_DATA *pKeyToken
        )
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(pRemoteCredential);
        
        HRESULT hr = S_OK;
        //copy all input buffers into out buffers unmodified
        for(DWORD dwIdx=0;dwIdx < dwBuffers;dwIdx++)
        {
            if(NULL == (pEncryptedBuffers[dwIdx].pb = (PBYTE)malloc(pEncryptedBuffers[dwIdx].cb = pDataBuffers[dwIdx].cb)))
            {
                while(dwIdx-- >= 1)
                {
                    free(pEncryptedBuffers[dwIdx].pb);
                }
                ZeroMemory(pEncryptedBuffers, sizeof(DRT_DATA)*dwBuffers);
                hr = E_OUTOFMEMORY;
                goto cleanup;
            }
            CopyMemory(pEncryptedBuffers[dwIdx].pb, pDataBuffers[dwIdx].pb, pEncryptedBuffers[dwIdx].cb);
        }
        pKeyToken->cb = 0;
        pKeyToken->pb = NULL;
        
cleanup:
    return hr;
    }

    HRESULT CCustomNullSecurityProvider::DecryptData(
        __in        const PVOID pvContext,
        __in        DRT_DATA* pKeyToken,
        __in        const PVOID pvKeyContext,
        __in        DWORD dwBuffers,
        __inout_ecount(dwBuffers)    DRT_DATA* pData
        )
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(pKeyToken);
        UNREFERENCED_PARAMETER(pvKeyContext);
        UNREFERENCED_PARAMETER(dwBuffers);
        UNREFERENCED_PARAMETER(pData);
        return S_OK;
    }

    HRESULT CCustomNullSecurityProvider::SignData(
        __in                                const PVOID pvContext,
        __in                                DWORD dwBuffers,
        __in_ecount(dwBuffers) DRT_DATA* pDataBuffers,
        __out                               DRT_DATA *pKeyIdentifier,
        __out                               DRT_DATA *pSignature)
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(dwBuffers);
        UNREFERENCED_PARAMETER(pDataBuffers);
        pSignature->cb = 0;
        pSignature->pb = NULL;
        pKeyIdentifier->cb = 0;
        pKeyIdentifier->pb = NULL;
        return S_OK;

    }

    HRESULT CCustomNullSecurityProvider::VerifyData(
        __in                                const PVOID pvContext,
        __in                                DWORD dwBuffers,
        __in_ecount(dwBuffers) DRT_DATA* pDataBuffers,
        __in                                DRT_DATA *pRemoteCredentials,
        __in                                DRT_DATA *pKeyIdentifier,
        __in                               DRT_DATA *pSignature)
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(dwBuffers);
        UNREFERENCED_PARAMETER(pDataBuffers);
        UNREFERENCED_PARAMETER(pRemoteCredentials);
        UNREFERENCED_PARAMETER(pKeyIdentifier);
        return (pSignature != NULL && pSignature->cb == 0)?S_OK : DRT_E_INVALID_MESSAGE;
    }


CCustomNullSecuredAddressPayload::CCustomNullSecuredAddressPayload()
{
    m_fAllocated = false;
    ZeroMemory(m_signature, sizeof(m_signature));
    m_bProtocolVersionMajor = 0;
    m_bProtocolVersionMinor = 0;
    ZeroMemory(&m_ddNonce, sizeof(DRT_DATA));
    ZeroMemory(&m_ddKey, sizeof(DRT_DATA));
    m_dwFlags = 0;
    m_pAddressList = NULL;
    m_pPublicKey = NULL;
}


CCustomNullSecuredAddressPayload::~CCustomNullSecuredAddressPayload()
{
    if (m_fAllocated)
    {
        free(m_ddNonce.pb);
        free(m_ddKey.pb);
        free(m_pAddressList);
        free(m_pPublicKey);
    }
}


// Purpose:  Serialize a word (big endian)
//
// Args:     [in]  w:
//           [in]  pbIter: serialization iterator
//
// Returns:  new serialization iterator
//
BYTE* CCustomNullSecuredAddressPayload::SerializeWord(WORD w, __in BYTE* pbIter)
{
    *pbIter++ = (BYTE)((w >> 8) & 0xff);
    *pbIter++ = (BYTE)(w & 0xff);
    return pbIter;
}


// Purpose:  Serialize a dword (big endian)
//
// Args:     [in]  dw:
//           [in]  pbIter: serialization iterator
//
// Returns:  new serialization iterator
//
BYTE* CCustomNullSecuredAddressPayload::SerializeDword(DWORD dw, __in BYTE* pbIter)
{
    *pbIter++ = (BYTE)((dw >> 24) & 0xff);
    *pbIter++ = (BYTE)((dw >> 16) & 0xff);
    *pbIter++ = (BYTE)((dw >> 8) & 0xff);
    *pbIter++ = (BYTE)(dw & 0xff);
    return pbIter;
}

// Serialized SecureAddressPayload format:
// bytes    name
// 1        protocol major version
// 1        protocol minor version
// 1        security major version
// 1        security minor version
// 2        key length (KL)
// KL       key
// 1        signature length (SL)
// SL       signature
// 1        nonce length (NL)
// NL       nonce
// 4        flags
// ----- public key -----------
// 1        algorithm length (AL)
// 2        key parameters length (PL)
// 2        public key length (KL)
// 1        unused bits
// AL       algorithm (char)
// PL       key parameters
// KL       public key
// ----- end public key -------
// 1        address count
// ----- for each address -----
// 2        address length (AL)
// AL       address data
// ----- end each address -----
//

// Purpose:  Serialize the SecuredAddressPayload according to the format specified above, and sign
//           it using the specified credentials.
//
// Args:     [in]  pCertChain:
//           [out] pData: serialized/signed data. pData->pb is allocated.
//
// Notes:    The data to be serialized has already been set using the Set* methods.
//
HRESULT CCustomNullSecuredAddressPayload::SerializeAndSign(__out DRT_DATA* pData)
{
    DRT_DATA ddData = {0};
    DRT_DATA ddSignature = {0};
    ULONG cbPayload = 0;
    WORD cbAsWord = 0;
    BYTE* pbIter = NULL;
    BYTE* pbSignature = NULL;
    ULONG cbAlgorithmId = 0;
    CERT_PUBLIC_KEY_INFO publicKey = {0};
    publicKey.Algorithm.pszObjId = "0.0.0.0";
    publicKey.PublicKey.cbData = sizeof(DWORD);
    DWORD dwBaadFood = 0xbaadf00d;
    publicKey.PublicKey.pbData = (BYTE*)&dwBaadFood;
    CERT_PUBLIC_KEY_INFO* pPublicKey = &publicKey;
    HRESULT hr = S_OK;

    ZeroMemory(pData, sizeof(DRT_DATA));

    cbAlgorithmId = (ULONG)strlen(pPublicKey->Algorithm.pszObjId);

    // validate that the lengths are all reasonable (fit in the space provided for their count)
    if ( m_ddNonce.cb > BYTE_MAX ||
        (m_pAddressList && m_pAddressList->iAddressCount > BYTE_MAX) ||
        m_ddKey.cb > WORD_MAX || cbAlgorithmId > BYTE_MAX ||
        pPublicKey->Algorithm.Parameters.cbData > WORD_MAX ||
        pPublicKey->PublicKey.cbData > WORD_MAX ||
        pPublicKey->PublicKey.cUnusedBits > BYTE_MAX)
    {
        hr = E_INVALIDARG;
        goto cleanup;
    }

    // calculate the payload size, up to and including the address count
    cbPayload = 1 + 1 + 1 + 1 + 2 + m_ddKey.cb + 1 + DRT_SIG_LENGTH + 1 + m_ddNonce.cb + sizeof(DWORD) + 1 + 2 + 2 + 1 + 1;

    // calculate the size of the public key data
    cbPayload += cbAlgorithmId;
    cbPayload += pPublicKey->Algorithm.Parameters.cbData;
    cbPayload += pPublicKey->PublicKey.cbData;

    // calculate the size of the address payload
    if (m_pAddressList)
    {
        for (INT i = 0; i < m_pAddressList->iAddressCount; i++)
        {
            cbPayload += 2; // length
            if (m_pAddressList->Address[i].iSockaddrLength > UINT16_MAX)
            {
                hr = E_INVALIDARG;
                goto cleanup;
            }
            cbPayload += m_pAddressList->Address[i].iSockaddrLength;
        }
    }

    ddData.cb = cbPayload;
    ddData.pb = (BYTE*)malloc(cbPayload);
    if (!ddData.pb)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    // serialize away
    ZeroMemory(ddData.pb, cbPayload);
    pbIter = ddData.pb;

    // protocol version
    *pbIter++ = m_bProtocolVersionMajor;
    *pbIter++ = m_bProtocolVersionMinor;

    // security version
    *pbIter++ = DRT_SECURITY_VERSION_MAJOR;
    *pbIter++ = DRT_SECURITY_VERSION_MINOR;

    // key
    pbIter = SerializeWord((WORD)m_ddKey.cb, pbIter);
    CopyMemory(pbIter, m_ddKey.pb, m_ddKey.cb);
    pbIter += m_ddKey.cb;

    // skip over the signature for now (leave it zero while we calculate the signature)
    *pbIter++ = DRT_SIG_LENGTH;
    pbSignature = pbIter; // save the location of the signature for later
    pbIter += DRT_SIG_LENGTH;

    // nonce
    *pbIter++ = (BYTE)m_ddNonce.cb;
    CopyMemory(pbIter, m_ddNonce.pb, m_ddNonce.cb);
    pbIter += m_ddNonce.cb;

    // flags
    pbIter = SerializeDword(m_dwFlags, pbIter);

    // public key sizes
    *pbIter++ = (BYTE)cbAlgorithmId;
    pbIter = SerializeWord((WORD)pPublicKey->Algorithm.Parameters.cbData, pbIter);
    pbIter = SerializeWord((WORD)pPublicKey->PublicKey.cbData, pbIter);
    *pbIter++ = (BYTE)pPublicKey->PublicKey.cUnusedBits;

    // public key data
    CopyMemory(pbIter, pPublicKey->Algorithm.pszObjId, cbAlgorithmId);
    pbIter += cbAlgorithmId;
    if (pPublicKey->Algorithm.Parameters.cbData)
        CopyMemory(pbIter, pPublicKey->Algorithm.Parameters.pbData, pPublicKey->Algorithm.Parameters.cbData);
    pbIter += pPublicKey->Algorithm.Parameters.cbData;
    CopyMemory(pbIter, pPublicKey->PublicKey.pbData, pPublicKey->PublicKey.cbData);
    pbIter += pPublicKey->PublicKey.cbData;

    // addresses
    if (m_pAddressList)
    {
        *pbIter++ = (BYTE)m_pAddressList->iAddressCount;
        for (INT i = 0; i < m_pAddressList->iAddressCount; i++)
        {
            cbAsWord = (WORD)m_pAddressList->Address[i].iSockaddrLength;
            pbIter = SerializeWord(cbAsWord, pbIter);
            CopyMemory(pbIter, m_pAddressList->Address[i].lpSockaddr, cbAsWord);
            pbIter += cbAsWord;
        }
    }
    else
    {
        *pbIter++ = 0; // 0 addresses
    }

    // pass the data back to the caller
    *pData = ddData;

cleanup:
    if (FAILED(hr))
    {
        free(ddData.pb);
    }
    free(ddSignature.pb);
    return hr;
}


// Function: CCustomNullSecuredAddressPayload::DeserializeAndValidate
//
// Purpose:  Deserialize and validate the payload.
//
// Args:     [in]  pData: data to deserialize
//           [in]  pNonce: expected nonce
//           [in]  pCertChain: opt. remote cert chain (if one was in the message)
//           [in]  hCryptProv: crypt provider to use with remote public key
//
// Notes:    The deserialized data is later retrieved via Get* methods.
//
HRESULT CCustomNullSecuredAddressPayload::DeserializeAndValidate(__in DRT_DATA* pData, __in_opt DRT_DATA* pNonce)
{
    BYTE b;
    WORD w;
    BYTE bVersionMajor;
    BYTE bVersionMinor;
    BYTE* pbSignature = NULL;
    DRT_DATA ddSignature = {0};
    SOCKET_ADDRESS_LIST* pAddressList = NULL;
    HRESULT hr = S_OK;

    CDrtCustomNullDeserializer deserializer;
    deserializer.Init(pData);

    m_fAllocated = true;

    // protocol version
    hr = deserializer.ReadByte(&m_bProtocolVersionMajor);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = deserializer.ReadByte(&m_bProtocolVersionMinor);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // security version
    hr = deserializer.ReadByte(&bVersionMajor);
    if (FAILED(hr))
    {
        goto cleanup;
    }
    hr = deserializer.ReadByte(&bVersionMinor);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // ensure we are receiving a version we understand
    if (bVersionMajor != DRT_SECURITY_VERSION_MAJOR || bVersionMinor != DRT_SECURITY_VERSION_MINOR)
    {
        hr = DRT_E_INVALID_MESSAGE;
        goto cleanup;
    }

    // extract key
    hr = deserializer.ReadWord(&w);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = deserializer.ReadByteArray(w, &m_ddKey);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // extract signature
    hr = deserializer.ReadByte(&b);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    if (b != DRT_SIG_LENGTH)
    {
        hr = DRT_E_INVALID_MESSAGE;
        goto cleanup;
    }

    pbSignature = deserializer.GetIter();
    hr = deserializer.ReadByteArray(DRT_SIG_LENGTH, &ddSignature);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // zero out the signature memory, so that we can validate the signature properly
    ZeroMemory(pbSignature, DRT_SIG_LENGTH);

    // extract and validate nonce
    hr = deserializer.ReadByte(&b);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = deserializer.ReadByteArray(b, &m_ddNonce);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // if a nonce was supplied, ensure it matches the nonce in the message
    if (pNonce)
    {
        hr = CompareNonce(pNonce);
        if (FAILED(hr))
        {
            goto cleanup;
        }
    }

    // extract flags
    hr = deserializer.ReadDword(&m_dwFlags);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // extract public key
    hr = deserializer.ReadPublicKey(&m_pPublicKey);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // extract addresses
    hr = deserializer.ReadByte(&b);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    pAddressList = (SOCKET_ADDRESS_LIST*)malloc(SIZEOF_SOCKET_ADDRESS_LIST(b));
    if (!pAddressList)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    ZeroMemory(pAddressList, SIZEOF_SOCKET_ADDRESS_LIST(b));

    for (BYTE i = 0; i < b; i++)
    {
        hr = deserializer.ReadSocketAddress(&pAddressList->Address[i]);
        if (FAILED(hr))
        {
            goto cleanup;
        }

        // we successfully read an address, so increment the address count
        pAddressList->iAddressCount++;
    }

    hr = PackAddressList(pAddressList);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    if (deserializer.GetRemainder() != 0)
    {
        hr = DRT_E_INVALID_MESSAGE;
        goto cleanup;
    }

cleanup:
    free(ddSignature.pb);
    if (pAddressList)
    {
        for (INT i = 0; i < pAddressList->iAddressCount; i++)
            free(pAddressList->Address[i].lpSockaddr);
        free(pAddressList);
    }

    if (FAILED(hr))
    {
    }
    // the remaining allocated memory is free in the destructor, or ownership is passed via
    // GetAddresses
    return hr;
}


// Purpose:  Pack the given address list into a self-referential structure (m_pAddressList)
//
// Args:     [in]  pAddressList:
//
HRESULT CCustomNullSecuredAddressPayload::PackAddressList(__in SOCKET_ADDRESS_LIST* pAddressList)
{
    ULONG cbAddressList = SIZEOF_SOCKET_ADDRESS_LIST(pAddressList->iAddressCount);
    for (int i = 0; i < pAddressList->iAddressCount; i++)
        cbAddressList += pAddressList->Address[i].iSockaddrLength;

    m_pAddressList = (SOCKET_ADDRESS_LIST*)malloc(cbAddressList);
    if (!m_pAddressList)
        return E_OUTOFMEMORY;

    ULONG cbOffset = SIZEOF_SOCKET_ADDRESS_LIST(pAddressList->iAddressCount);
    CopyMemory(m_pAddressList, pAddressList, cbOffset);
    for (int i = 0; i < pAddressList->iAddressCount; i++)
    {
        m_pAddressList->Address[i].lpSockaddr = (LPSOCKADDR)((BYTE*)m_pAddressList + cbOffset);
        CopyMemory(m_pAddressList->Address[i].lpSockaddr, pAddressList->Address[i].lpSockaddr, pAddressList->Address[i].iSockaddrLength);
        cbOffset += pAddressList->Address[i].iSockaddrLength;
    }

    return S_OK;
}


// Purpose:  Compare the nonce provided by the DRT to the nonce received on the wire, returning
//           DRT_E_INVALID_MESSAGE if they don't match.
//
// Args:     [in]  pNonce:
//
HRESULT CCustomNullSecuredAddressPayload::CompareNonce(__in DRT_DATA* pNonce)
{
    if (pNonce->cb != m_ddNonce.cb)
        return DRT_E_INVALID_MESSAGE;
    if (memcmp(pNonce->pb, m_ddNonce.pb, pNonce->cb) != 0)
        return DRT_E_INVALID_MESSAGE;

    return S_OK;
}


// Purpose:  Set the protocol version
//
// Args:     [in]  bMajor:
//           [in]  bMinor:
//
void CCustomNullSecuredAddressPayload::SetProtocolVersion(BYTE bMajor, BYTE bMinor)
{
    m_bProtocolVersionMajor = bMajor;
    m_bProtocolVersionMinor = bMinor;
}


// Purpose:  Copy the address data. The memory for the addresses is only referenced, the
//           ownership is not passed (shallow copy).
//
// Args:     [in]  pAddressList:
//
void CCustomNullSecuredAddressPayload::SetAddresses(__in_opt const SOCKET_ADDRESS_LIST* pAddressList)
{
    m_pAddressList = (SOCKET_ADDRESS_LIST*)pAddressList;
}


// Purpose:  Copy the nonce DRT_DATA. The memory for the nonce itself is only referenced, the
//           ownership is not passed (shallow copy).
//
// Args:     [in]  pData:
//
void CCustomNullSecuredAddressPayload::SetNonce(__in const DRT_DATA* pData)
{
    m_ddNonce = *pData;
}


// Purpose:  Copy the key DRT_DATA. The memory for the key itself is only referenced, the
//           ownership is not passed (shallow copy).
//
// Args:     [in]  pData:
//
void CCustomNullSecuredAddressPayload::SetKey(__in const DRT_DATA* pData)
{
    m_ddKey = *pData;
}


// Purpose:  Set the flags
//
// Args:     [in]  dwFlags:
//
void CCustomNullSecuredAddressPayload::SetFlags(DWORD dwFlags)
{
    m_dwFlags = dwFlags;
}


// Purpose:  Retrieve the addresses. This returns the memory allocated during de-serialization, so
//           can only be called once. Since it will only be called once, there isn't benefit to
//           making another copy of the data.
//
SOCKET_ADDRESS_LIST* CCustomNullSecuredAddressPayload::GetAddresses()
{
    SOCKET_ADDRESS_LIST* pAddressList = m_pAddressList;

    // this object no longer owns the address list
    m_pAddressList = NULL;
    return pAddressList;
}


//
// Purpose:  Retrieve the flags
//
DWORD CCustomNullSecuredAddressPayload::GetFlags()
{
    return m_dwFlags;
}


//
// Purpose:  Retrieve the flags
//
void CCustomNullSecuredAddressPayload::GetProtocolVersion(__out BYTE* pbMajor, __out BYTE* pbMinor)
{
    *pbMajor = m_bProtocolVersionMajor;
    *pbMinor = m_bProtocolVersionMinor;
}


// Purpose:  Retrieve the key deserialized earlier. This returns memory allocated during
//           deserialization, and passes ownership to the caller. This method may only be called
//           once.
//
void CCustomNullSecuredAddressPayload::GetKey(__out DRT_DATA* pData)
{
    *pData = m_ddKey;
    // this object no longer owns the public key
    ZeroMemory(&m_ddKey, sizeof(DRT_DATA));
}

// Purpose:  Retrieve the public key deserialized earlier. This returns memory allocated during
//           deserialization, and passes ownership to the caller. This method may only be called
//           once.
//
CERT_PUBLIC_KEY_INFO* CCustomNullSecuredAddressPayload::GetPublicKey()
{
    CERT_PUBLIC_KEY_INFO* pPublicKey = m_pPublicKey;

    // this object no longer owns the public key
    m_pPublicKey = NULL;
    return pPublicKey;
}


CDrtCustomNullDeserializer::CDrtCustomNullDeserializer()
{
    m_cbRemainder = 0;
    m_pbIter = NULL;
}


// Purpose:  Init the deserializer
//
// Args:     [in]  pData: data to be deserialized
//
void CDrtCustomNullDeserializer::Init(__in DRT_DATA* pData)
{
    m_pbIter = pData->pb;
    m_cbRemainder = pData->cb;
}


// Purpose:  Skip past bytes in the stream
//
// Args:     [in]  cb: number of bytes to skip
//
HRESULT CDrtCustomNullDeserializer::Skip(DWORD cb)
{
    if (m_cbRemainder < cb)
        return DRT_E_INVALID_MESSAGE;

    m_pbIter += cb;
    m_cbRemainder -= cb;
    return S_OK;
}


// Purpose:  Read a byte from the stream
//
// Args:     [out]  pb:
//
HRESULT CDrtCustomNullDeserializer::ReadByte(__out BYTE* pb)
{
    *pb = 0;

    if (m_cbRemainder < 1)
        return DRT_E_INVALID_MESSAGE;

    *pb = *m_pbIter++;
    m_cbRemainder -= 1;
    return S_OK;
}


// Purpose:  Read a word from the stream
//
// Args:     [out]  pw:
//
HRESULT CDrtCustomNullDeserializer::ReadWord(__out WORD* pw)
{
    *pw = 0;

    if (m_cbRemainder < 2)
        return DRT_E_INVALID_MESSAGE;

    *pw = *m_pbIter++;
    *pw <<=8;
    *pw |= *m_pbIter++;

    m_cbRemainder -= 2;
    return S_OK;
}


// Purpose:  Read a dword from the stream
//
// Args:     [out]  pdw:
//
HRESULT CDrtCustomNullDeserializer::ReadDword(__out DWORD* pdw)
{
    *pdw = 0;

    if (m_cbRemainder < 4)
        return DRT_E_INVALID_MESSAGE;

    *pdw = *m_pbIter++;
    *pdw <<=8;
    *pdw |= *m_pbIter++;
    *pdw <<=8;
    *pdw |= *m_pbIter++;
    *pdw <<=8;
    *pdw |= *m_pbIter++;

    m_cbRemainder -= 4;
    return S_OK;
}


// Purpose:  Read a byte array from the stream
//
// Args:     [in]   cb: number of bytes to read
//           [out]  pData: byte array. pData->pb is allocated
//
HRESULT CDrtCustomNullDeserializer::ReadByteArray(DWORD cb, __out DRT_DATA* pData)
{
    ZeroMemory(pData, sizeof(DRT_DATA));

    if (cb == 0)
        return E_INVALIDARG;

    if (m_cbRemainder < cb)
        return DRT_E_INVALID_MESSAGE;

    pData->pb = (BYTE*)malloc(cb);
    if (!pData->pb)
        return E_OUTOFMEMORY;

    CopyMemory(pData->pb, m_pbIter, cb);
    pData->cb = cb;

    m_pbIter += cb;
    m_cbRemainder -= cb;

    return S_OK;
}


// Purpose:  Read a socket address from the stream
//
// Args:     [out]  pAddress: pAddress->lpSockarr is allocated, the rest of the struct is filled in.
//
HRESULT CDrtCustomNullDeserializer::ReadSocketAddress(__out SOCKET_ADDRESS* pAddress)
{
    ZeroMemory(pAddress, sizeof(SOCKET_ADDRESS));

    WORD cbAddress = 0;
    HRESULT hr = ReadWord(&cbAddress);
    if (FAILED(hr))
        return hr;

    DRT_DATA ddAddress = {0};
    hr = ReadByteArray(cbAddress, &ddAddress);
    if (FAILED(hr))
        return hr;

    pAddress->iSockaddrLength = cbAddress;
    pAddress->lpSockaddr = (SOCKADDR*)ddAddress.pb;
    return S_OK;
}


// Purpose:  Read a public key from the stream
//
// Args:     [out] ppPublicKey: public key allocated as a single block of memory (with self-refertial
//           embedded pointers)
//
HRESULT CDrtCustomNullDeserializer::ReadPublicKey(__out CERT_PUBLIC_KEY_INFO** ppPublicKey)
{
    *ppPublicKey = NULL;

    HRESULT hr = S_OK;
    BYTE cbAlgorithmId = 0;
    WORD cbParameters = 0;
    WORD cbPublicKey = 0;
    BYTE cUnusedBits = 0;
    ULONG cbTotal = 0;
    BYTE* pbStructIter = NULL;
    CERT_PUBLIC_KEY_INFO* pPublicKey = NULL;

    hr = ReadByte(&cbAlgorithmId);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = ReadWord(&cbParameters);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = ReadWord(&cbPublicKey);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = ReadByte(&cUnusedBits);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    if (m_cbRemainder < (ULONG)cbAlgorithmId + cbParameters + cbPublicKey)
    {
        hr = DRT_E_INVALID_MESSAGE;
        goto cleanup;
    }

    cbTotal = sizeof(CERT_PUBLIC_KEY_INFO) + ROUND_UP_COUNT(cbAlgorithmId + 1, ALIGN_LPBYTE) +
        ROUND_UP_COUNT(cbParameters, ALIGN_LPBYTE) + ROUND_UP_COUNT(cbPublicKey, ALIGN_LPBYTE);
    pPublicKey = (CERT_PUBLIC_KEY_INFO*)malloc(cbTotal);
    if (!pPublicKey)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }
    ZeroMemory(pPublicKey, cbTotal);
    pbStructIter = (BYTE*)(pPublicKey + 1); // skip the structure

    // copy the algorithm id
    pPublicKey->Algorithm.pszObjId = (LPSTR)pbStructIter;
    CopyMemory(pbStructIter, m_pbIter, cbAlgorithmId);
    pbStructIter += cbAlgorithmId;
    *pbStructIter++ = 0; // NULL-terminate the string
    m_pbIter += cbAlgorithmId;
    m_cbRemainder -= cbAlgorithmId;
    pbStructIter = (BYTE*)ROUND_UP_POINTER(pbStructIter, ALIGN_LPBYTE);

    // copy the key parameters
    if (cbParameters)
    {
        pPublicKey->Algorithm.Parameters.cbData = cbParameters;
        pPublicKey->Algorithm.Parameters.pbData = pbStructIter;
        CopyMemory(pbStructIter, m_pbIter, cbParameters);
        m_pbIter += cbParameters;
        m_cbRemainder -= cbParameters;
        pbStructIter += cbParameters;
        pbStructIter = (BYTE*)ROUND_UP_POINTER(pbStructIter, ALIGN_LPBYTE);
    }

    // copy the key
    pPublicKey->PublicKey.cbData = cbPublicKey;
    pPublicKey->PublicKey.cUnusedBits = cUnusedBits;
    pPublicKey->PublicKey.pbData = pbStructIter;
    CopyMemory(pbStructIter, m_pbIter, cbPublicKey);
    m_pbIter += cbPublicKey;
    m_cbRemainder -= cbPublicKey;
    pbStructIter += cbPublicKey;
    pbStructIter = (BYTE*)ROUND_UP_POINTER(pbStructIter, ALIGN_LPBYTE);

    *ppPublicKey = pPublicKey;
cleanup:
    if (FAILED(hr))
    {
        free(pPublicKey);
    }
    return hr;
}


// Purpose:  Retrieve the current position of the iterator
//
BYTE* CDrtCustomNullDeserializer::GetIter()
{
    return m_pbIter;
}


// Purpose:  Retrieve the remaining size of the stream
//
ULONG CDrtCustomNullDeserializer::GetRemainder()
{
    return m_cbRemainder;
}
