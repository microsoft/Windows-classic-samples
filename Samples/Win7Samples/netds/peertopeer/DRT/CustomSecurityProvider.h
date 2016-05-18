// CustomSecurityProvider.h - Interface for CustomSecurityProvider

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include <p2p.h>
#include <drt.h>
#include <stdlib.h>
#include <intsafe.h>

// Original 0x8000 + space for extended payload (4k plus some overhead)
#define     MAX_MESSAGE_SIZE        (0x8000 + 0x1200)

// default security provider constants
#define DRT_SECURITY_VERSION_MAJOR 1
#define DRT_SECURITY_VERSION_MINOR 0

#define SHA2_SIG_LENGTH 0x80
#define SHA1_SIG_LENGTH 0x80
#define DRT_SIG_LENGTH SHA2_SIG_LENGTH
#define DRT_ALGORITHM_OID szOID_RSA_SHA1RSA
#define DRT_ALGORITHM CALG_SHA_256
#define DRT_SHA2_LENGTH 32
#define DRT_DERIVED_KEY_SIZE 32

// If Count is not already aligned, then
// round Count up to an even multiple of "Pow2".  "Pow2" must be a power of 2.
//
// DWORD
// ROUND_UP_COUNT(
//     IN DWORD Count,
//     IN DWORD Pow2
//     );
#define ROUND_UP_COUNT(Count,Pow2) \
        ( ((Count)+(Pow2)-1) & (~(((LONG)(Pow2))-1)) )

// LPVOID
// ROUND_UP_POINTER(
//     IN LPVOID Ptr,
//     IN DWORD Pow2
//     );

// If Ptr is not already aligned, then round it up until it is.
#define ROUND_UP_POINTER(Ptr,Pow2) \
        ( (LPVOID) ( (((ULONG_PTR)(Ptr))+(Pow2)-1) & (~(((LONG)(Pow2))-1)) ) )


// Usage: myPtr = ROUND_UP_POINTER( unalignedPtr, ALIGN_LPVOID )

#define ALIGN_LPBYTE            sizeof(LPBYTE)

HRESULT DrtCreateCustomSecurityProvider(DRT_SECURITY_PROVIDER** ppDrtSecurityProvider);

void DrtDeleteCustomSecurityProvider(DRT_SECURITY_PROVIDER* pDrtSecurityProvider);

class CDrtCustomNullDeserializer
{
public:
    CDrtCustomNullDeserializer();
    void Init(DRT_DATA* pData);

    HRESULT Skip(DWORD cb);
    HRESULT ReadByte(BYTE* pb);
    HRESULT ReadWord(WORD* pw);
    HRESULT ReadDword(DWORD* pdw);
    HRESULT ReadSocketAddress(SOCKET_ADDRESS* pAddress);
    HRESULT ReadPublicKey(CERT_PUBLIC_KEY_INFO** ppPublicKey);
    HRESULT ReadByteArray(DWORD cb, DRT_DATA* pData);
    BYTE* GetIter();
    ULONG GetRemainder();

private:
    ULONG m_cbRemainder;
    BYTE* m_pbIter;
};


// Class:    CCustomNullSecuredAddressPayload
//
// Purpose:  Class for implementing the (de)serialization of a the SecuredAddressPayload.
//
class CCustomNullSecuredAddressPayload
{
public:
    CCustomNullSecuredAddressPayload();
    ~CCustomNullSecuredAddressPayload();

    HRESULT SerializeAndSign(__out DRT_DATA* pData);
    HRESULT DeserializeAndValidate(__in DRT_DATA* pData, __in_opt DRT_DATA* pNonce);

    void SetProtocolVersion(BYTE bMajor, BYTE bMinor);
    void SetKey(__in const DRT_DATA* pKey);
    void SetAddresses(__in_opt const SOCKET_ADDRESS_LIST* pAddressList);
    void SetNonce(__in const DRT_DATA* pData);
    void SetFlags(DWORD dwFlags);

    void GetProtocolVersion(__out BYTE* pbMajor, __out BYTE* pbMinor);
    CERT_PUBLIC_KEY_INFO* GetPublicKey();
    void GetKey(__out DRT_DATA* pKey);
    SOCKET_ADDRESS_LIST* GetAddresses();
    DWORD GetFlags();

private:
    HRESULT CompareNonce(__in DRT_DATA* pNonce);
    BYTE* SerializeWord(WORD w, __in BYTE* pbIter);
    BYTE* SerializeDword(DWORD dw, __in BYTE* pbIter);
    HRESULT PackAddressList(__in SOCKET_ADDRESS_LIST* pAddressList);

    bool m_fAllocated; // set if the data needs to be freed when destroyed (true when deserializing)

    BYTE m_signature[DRT_SIG_LENGTH];
    BYTE m_bProtocolVersionMajor;
    BYTE m_bProtocolVersionMinor;
    DRT_DATA m_ddNonce;
    DRT_DATA m_ddKey;
    DWORD m_dwFlags;
    SOCKET_ADDRESS_LIST* m_pAddressList;
    CERT_PUBLIC_KEY_INFO* m_pPublicKey;
};


class CCustomNullSecurityProvider
{
public:
    static HRESULT Init(__out DRT_SECURITY_PROVIDER** ppDrtSecurityProvider);

private:
    // static functions which will be passed in DRT_SECURITY_PROVIDER
    static HRESULT  Attach(
        __in        const PVOID pvContext);

    static VOID     Detach(
        __in        const PVOID pvContext);


    // static functions which will be passed in DRT_SECURITY_PROVIDER
    static HRESULT RegisterKey(
        __in_opt    const PVOID pvContext,
        __in        const DRT_REGISTRATION *pRegistration,
        __in_opt    PVOID pvKeyContext)
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(pRegistration);
        UNREFERENCED_PARAMETER(pvKeyContext);
        return S_OK;
    }

    static HRESULT UnregisterKey(
        __in_opt    const PVOID pvContext,
        __in        const DRT_DATA *pKey,
        __in_opt    PVOID pvKeyContext)
    {
        UNREFERENCED_PARAMETER(pvContext);
        UNREFERENCED_PARAMETER(pKey);
        UNREFERENCED_PARAMETER(pvKeyContext);
        return S_OK;
    }

    static HRESULT ValidateAndUnpackPayload(
        __in_opt    const PVOID pvContext,
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
        __out       DWORD* pdwFlags);

    static HRESULT SecureAndPackPayload(
        __in_opt    const PVOID pvContext,
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
        __out_opt   DRT_DATA* pCertChain);

    static void FreeData(
        __in_opt    const PVOID pvContext,
        __in_opt    VOID* pv)
    {
        UNREFERENCED_PARAMETER(pvContext);
        free(pv);
    }
    static HRESULT EncryptData(
        __in        const PVOID pvContext,
        __in        const DRT_DATA* pRemoteCredential,
        __in        DWORD dwBuffers,
        __in_ecount(dwBuffers)        DRT_DATA* pDataBuffers,
        __out_ecount(dwBuffers)        DRT_DATA* pEncryptedBuffers,
        __out       DRT_DATA *pKeyToken
        );
    static HRESULT DecryptData(
        __in        const PVOID pvContext,
        __in        DRT_DATA* pKeyToken,
        __in        const PVOID pvKeyContext,
        __in        DWORD dwBuffers,
        __inout_ecount(dwBuffers) DRT_DATA* pData
        );

    static HRESULT GetSerializedCredential(
        __in        const PVOID pvContext,
        __out DRT_DATA *pSelfCredential);

    static HRESULT ValidateRemoteCredential(
        __in        const PVOID pvContext,
        __in DRT_DATA *pRemoteCredential);

    static HRESULT SignData(
        __in                                const PVOID pvContext,
        __in                                DWORD dwBuffers,
        __in_ecount(dwBuffers) DRT_DATA* pDataBuffers,
        __out                               DRT_DATA *pKeyIdentifier,
        __out                               DRT_DATA *pSignature);

    static HRESULT VerifyData(
        __in                                const PVOID pvContext,
        __in                                DWORD dwBuffers,
        __in_ecount(dwBuffers) DRT_DATA* pDataBuffers,
        __in                                DRT_DATA *pRemoteCredentials,
        __in                                DRT_DATA* pKeyIdentifier,
        __in                               DRT_DATA *pSignature);


};