/*+-----------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1992 - 2000.  Microsoft Corporation.  All rights reserved.

File:        stubs.c

Contents:    user-mode stubs for security API


------------------------------------------------------------------------*/
#define _NO_KSECDD_IMPORT_
#include "sampssp.h"


SecurityFunctionTableW SecTableW = {SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
                                    EnumerateSecurityPackagesW,
                                    NULL,
                                    AcquireCredentialsHandleW,
                                    FreeCredentialsHandle,
                                    NULL, // LogonUser
                                    InitializeSecurityContextW,
                                    AcceptSecurityContext,
                                    CompleteAuthToken,
                                    DeleteSecurityContext,
                                    ApplyControlToken,
                                    QueryContextAttributesW,
                                    ImpersonateSecurityContext,
                                    RevertSecurityContext,
                                    MakeSignature,
                                    VerifySignature,
                                    FreeContextBuffer,
                                    QuerySecurityPackageInfoW
                                   };


PSEC_CONTEXT ContextList;
PCREDENTIAL CredentialList;
ULONG NextId;
TimeStamp Forever = {0x7fffffff,0xfffffff};
TimeStamp Never = {0,0};

//+-------------------------------------------------------------------------
//
//  Function:   LocateContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


PSEC_CONTEXT
LocateContext(ULONG ContextId)
{
    PSEC_CONTEXT TestContext;
    EnterCriticalSection(&DllCritSect);

    TestContext = ContextList;
    while (TestContext != NULL)
    {
        if (TestContext->ContextId == ContextId)
        {
            break;
        }
        TestContext = TestContext->Next;
    }
    LeaveCriticalSection(&DllCritSect);
    return(TestContext);

}

//+-------------------------------------------------------------------------
//
//  Function:   DeleteContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


BOOLEAN
DeleteContext(ULONG ContextId)
{
    PSEC_CONTEXT TestContext, LastContext;

    EnterCriticalSection(&DllCritSect);
    TestContext = ContextList;
    LastContext = NULL;

    while (TestContext != NULL)
    {
        if (TestContext->ContextId == ContextId)
        {
            break;
        }
        LastContext = TestContext;
        TestContext = TestContext->Next;
    }
    if (TestContext != NULL)
    {
        if (LastContext != NULL)
        {
            LastContext->Next = TestContext->Next;
        }
        else
        {
            assert(ContextList == TestContext);
            ContextList = TestContext->Next;
        }
        LocalFree(TestContext);
    }
    LeaveCriticalSection(&DllCritSect);
    return( TestContext == NULL ? FALSE : TRUE );
}

//+-------------------------------------------------------------------------
//
//  Function:   AddContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


VOID
AddContext(PSEC_CONTEXT Context)
{
    EnterCriticalSection(&DllCritSect);
    Context->Next = ContextList;
    ContextList = Context;
    LeaveCriticalSection(&DllCritSect);
}


//+-------------------------------------------------------------------------
//
//  Function:   InitSecurityInterfaceW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



PCREDENTIAL
LocateCredential(ULONG CredentialId)
{
    PCREDENTIAL TestCredential;
    EnterCriticalSection(&DllCritSect);

    TestCredential = CredentialList;
    while (TestCredential != NULL)
    {
        if (TestCredential->CredentialId == CredentialId)
        {
            break;
        }
        TestCredential = TestCredential->Next;
    }
    LeaveCriticalSection(&DllCritSect);
    return(TestCredential);

}

BOOLEAN
DeleteCredential(ULONG CredentialId)
{
    PCREDENTIAL TestCredential, LastCredential;

    EnterCriticalSection(&DllCritSect);
    TestCredential = CredentialList;
    LastCredential = NULL;

    while (TestCredential != NULL)
    {
        if (TestCredential->CredentialId == CredentialId)
        {
            break;
        }
        LastCredential = TestCredential;
        TestCredential = TestCredential->Next;
    }
    if (TestCredential != NULL)
    {
        if (LastCredential != NULL)
        {
            LastCredential->Next = TestCredential->Next;
        }
        else
        {
            assert(CredentialList == TestCredential);
            CredentialList = TestCredential->Next;
        }
        LocalFree(TestCredential);
    }
    LeaveCriticalSection(&DllCritSect);
    return( TestCredential == NULL ? FALSE : TRUE );
}

VOID
AddCredential(PCREDENTIAL Credential)
{
    EnterCriticalSection(&DllCritSect);
    Credential->Next = CredentialList;
    CredentialList = Credential;
    LeaveCriticalSection(&DllCritSect);
}

ULONG
GetNewId()
{
    ULONG NewId;

    EnterCriticalSection(&DllCritSect);
    NewId = NextId;
    NextId ++;
    LeaveCriticalSection(&DllCritSect);
    return(NewId);

}

PSecBuffer
LocateBuffer(PSecBufferDesc Buffer, ULONG MinimumSize)
{
    ULONG Index;
    if (Buffer == NULL)
    {
        return(NULL);
    }

    for (Index = 0; Index < Buffer->cBuffers  ; Index++)
    {
        if (((Buffer->pBuffers[Index].BufferType) & ~SECBUFFER_ATTRMASK)
            == SECBUFFER_TOKEN)
        {

            //
            // Do size checking
            //

            if (Buffer->pBuffers[Index].cbBuffer < MinimumSize)
            {
                return(NULL);
            }
            return(&Buffer->pBuffers[Index]);
        }
    }
    return(NULL);
}



PSecBuffer
LocateSecBuffer(PSecBufferDesc Buffer)
{
    return(LocateBuffer(Buffer, sizeof(MESSAGE)));
}


PSecBuffer
LocateSigBuffer(PSecBufferDesc Buffer)
{
    return(LocateBuffer(Buffer, sizeof(SIGNATURE)));
}


//+-------------------------------------------------------------------------
//
//  Function:   InitSecurityInterfaceW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------
PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW(VOID)
{
    InitializeCriticalSection( &DllCritSect );
    return(&SecTableW);
}


//+-------------------------------------------------------------------------
//
//  Function:   AcquireCredentialsHandleW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleW(
    LPWSTR                      pszPrincipal,       // Name of principal
    LPWSTR                      pszPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    )
{
    PCREDENTIAL NewCredential = NULL;

    if ((fCredentialUse & (SECPKG_CRED_BOTH)) == 0)
    {
        return(SEC_E_UNKNOWN_CREDENTIALS);
    }
    NewCredential = (PCREDENTIAL) LocalAlloc(0,sizeof(CREDENTIAL));
    if (NewCredential == NULL)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }
    NewCredential->CredentialId = GetNewId();
    NewCredential->Use = fCredentialUse;
    NewCredential->AuthData = (ULONG *)pAuthData;
    phCredential->dwUpper = NewCredential->CredentialId;
    *ptsExpiry = Forever;
    AddCredential(NewCredential);
    return(SEC_E_OK);

}



//+-------------------------------------------------------------------------
//
//  Function:   FreeCredentialsHandle
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandle(
    PCredHandle                 phCredential        // Handle to free
    )
{
    if (DeleteCredential(phCredential->dwUpper))
    {
        return(SEC_E_OK);
    }
    else
    {
        return(SEC_E_UNKNOWN_CREDENTIALS);
    }
}


//+-------------------------------------------------------------------------
//
//  Function:   InitializeSecurityContextW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextW(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    LPWSTR                      pszTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    PCREDENTIAL Credential = NULL;
    PSEC_CONTEXT Context = NULL;
    PMESSAGE Message = NULL;
    PSecBuffer OutputBuffer;
    PSecBuffer InputBuffer;
    MESSAGE SampleMessage;


    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    OutputBuffer = LocateSecBuffer(pOutput);
    if (OutputBuffer == NULL)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (phContext == NULL)
    {

        Credential = LocateCredential(phCredential->dwUpper);
        if (Credential == NULL)
        {
            return(SEC_E_UNKNOWN_CREDENTIALS);
        }

        if ((Credential->Use & SECPKG_CRED_OUTBOUND) == 0)
        {
            return(SEC_E_UNKNOWN_CREDENTIALS);
        }

    }

    //
    // If the context is NULL, create a new one.
    //

    if (phContext == NULL)
    {
        Context = (PSEC_CONTEXT) LocalAlloc(0,sizeof(SEC_CONTEXT));
        if (Context == NULL)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }
        Context->ContextId = GetNewId();
        phNewContext->dwUpper = Context->ContextId;
        Context->CredentialId = phCredential->dwUpper;
        Context->State = FirstInit;
        Context->Nonce = 0;
        Context->ContextFlags = fContextReq;
        *pfContextAttr = fContextReq;
        *ptsExpiry = Forever;

        //
        // Build an output token.
        //

        Message = (PMESSAGE) OutputBuffer->pvBuffer;
        Message->MessageType = Negotiate;
        memset(Message->Buffer,'x',MESSAGE_SIZE);
        OutputBuffer->cbBuffer = sizeof(MESSAGE);

        AddContext(Context);

        return(SEC_I_CONTINUE_NEEDED);

    } else {
        //
        // This is the second call. Lookup the old context.
        //

        Context = LocateContext(phContext->dwUpper);
        if (Context == NULL)
        {
            return(SEC_E_INVALID_HANDLE);
        }

        if ((Context->State != FirstInit) &&
            (Context->State != SecondInit))
        {
            return(SEC_E_INVALID_HANDLE);
        }


        //
        // Build the new output message
        //

        if (Context->State == FirstInit)
        {

            //
            // Check that the input message is what we expected.
            //

            InputBuffer = LocateSecBuffer(pInput);
            if (InputBuffer == NULL)
            {
                return(SEC_E_INVALID_TOKEN);
            }
            Message = (PMESSAGE) InputBuffer->pvBuffer;

            SampleMessage.MessageType = Challenge;
            memset(SampleMessage.Buffer, 'y', MESSAGE_SIZE);

            if (memcmp(&SampleMessage,Message,MESSAGE_SIZE) != 0)
            {
                return(SEC_E_INVALID_TOKEN);
            }

            Message = (PMESSAGE) OutputBuffer->pvBuffer;
            Message->MessageType = ChallengeResponse;
            memset(Message->Buffer,'z',MESSAGE_SIZE);
        }
        else
        {
            Message = (PMESSAGE) OutputBuffer->pvBuffer;
            Message->MessageType = ReAuthenticate;
            memset(Message->Buffer,'q',MESSAGE_SIZE);
        }

        OutputBuffer->cbBuffer = sizeof(MESSAGE);
        Context->State = SecondInit;
        return(SEC_E_OK);
    }
}



//+------------------------------------------- ------------------------------
//
//  Function:   AcceptSecurityContext
//
//  Synopsis:   Allows a remotely initiated security context between the
//              application and a remote peer to be established.  To complete
//              the establishment of context one or more reply tokens may be
//              required from remote peer.
//              This function is the server counterpart to the
//              InitializeSecurityContext API.  The ContextAttributes is a bit
//              mask representing various context level functions such as
//              delegation, mutual authentication, confidentiality, replay
//              detection and sequence detection.  This API is used by the
//              server side.  When a request comes in, the server uses the
//              ContextReqFlags parameter to specify what it requires of the
//              session.  In this fashion, a server can specify that clients
//              must be capable of using a confidential or integrity checked
//              session, and fail clients that can't meet that demand.
//              Alternatively, a server can require nothing, and whatever the
//              client can provide or requires is returned in the
//              pfContextAttributes parameter.  For a package that supports 3
//              leg mutual authentication, the calling sequence would be:
//              Client provides a token, server calls Accept the first time,
//              generating a reply token.  The client uses this in a second
//              call to InitializeSecurityContext, and generates a final token.
//              This token is then used in the final call to Accept to complete
//              the session.  Another example would be the LAN Manager/NT
//              authentication style.  The client connects to negotiate a
//              protocol.  The server calls Accept to set up a context and
//              generate a challenge to the client.  The client calls
//              InitializeSecurityContext and creates a response.  The server
//              then calls Accept the final time to allow the package to verify
//              the response is appropriate for the challenge.
//
//  Effects:
//
//
//  Arguments:  [phCredential] - Handle to the credentials to be used to
//              create the context.
//
//              [phContext] - Handle to the partially formed context, if
//              this is a second call (see above) or NULL if this is the first call.
//
//              [pInput] - Pointer to the input token.  In the first call
//              this token can either be NULL or may contain
//              security package specific information.
//
//              [fContextReq] - Requirements of the context, package specific. See
//              the ASC_REQ_xxx defines in SSPI.H.
//
//              [TargetDataRep] - Long indicating the data representation (byte
//              ordering, etc) on the target.  The constant
//              SECURITY_NATIVE_DREP may be supplied by the
//              transport indicating that the native format
//              is in use.
//
//              [phNewContext] - New context handle.  If this is a second
//              call, this can be same as OldContextHandle.
//
//              [pOutput] - Buffer to receive the output token.
//
//              [pfContextAttr] - Attributes of the context established. See
//              the ASC_RET_xxx defines in SSPI.H
//
//              [ptsExpiry] - Expiration time of the context.
//
//  Requires:
//
//  Returns:    STATUS_SUCCESS - Message handled
//              SEC_I_CALLBACK_NEEDED - Caller should call again later
//              SEC_E_NO_SPM - Security Support Provider is not running
//              SEC_E_INVALID_TOKEN - Token improperly formatted
//              SEC_E_INVALID_HANDLE - Credential/Context Handle is invalid
//              SEC_E_BUFFER_TOO_SMALL- Buffer for output token isn't big enough
//              SEC_E_LOGON_DENIED - User is no allowed to logon to this server
//              SEC_E_INSUFFICIENT_MEMORY - Not enough memory
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
AcceptSecurityContext(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    PSecBufferDesc              pInput,             // Input buffer
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               TargetDataRep,      // Target Data Rep
    PCtxtHandle                 phNewContext,       // (out) New context handle
    PSecBufferDesc              pOutput,            // (inout) Output buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attributes
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    PCREDENTIAL Credential = NULL;
    PSEC_CONTEXT Context = NULL;
    PMESSAGE Message = NULL;
    PSecBuffer OutputBuffer;
    PSecBuffer InputBuffer;
    MESSAGE SampleMessage;


    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    InputBuffer = LocateSecBuffer(pInput);
    if (InputBuffer == NULL)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (phContext == NULL)
    {

        Credential = LocateCredential(phCredential->dwUpper);
        if (Credential == NULL)
        {
            return(SEC_E_UNKNOWN_CREDENTIALS);
        }

        if ((Credential->Use & SECPKG_CRED_INBOUND) == 0)
        {
            return(SEC_E_UNKNOWN_CREDENTIALS);
        }

    }

    //
    // If the context is NULL, create a new one.
    //

    if (phContext == NULL)
    {
        //
        // Make sure the output buffer exists.
        //

        OutputBuffer = LocateSecBuffer(pOutput);
        if (OutputBuffer == NULL)
        {
            return(SEC_E_INVALID_TOKEN);
        }



        //
        // Check that the input message is what we expected.
        //

        Message = (PMESSAGE) InputBuffer->pvBuffer;
        SampleMessage.MessageType = Negotiate;
        memset(SampleMessage.Buffer, 'x', MESSAGE_SIZE);

        if (memcmp(&SampleMessage,Message,MESSAGE_SIZE) != 0)
        {
            return(SEC_E_INVALID_TOKEN);
        }

        //
        // Build a new context.
        //

        Context = (PSEC_CONTEXT) LocalAlloc(0,sizeof(SEC_CONTEXT));
        if (Context == NULL)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }
        Context->ContextId = GetNewId();
        phNewContext->dwUpper = Context->ContextId;
        Context->CredentialId = phCredential->dwUpper;
        Context->State = FirstAccept;
        Context->Nonce = 0;
        Context->ContextFlags = fContextReq;
        *pfContextAttr = fContextReq;
        *ptsExpiry = Forever;

        //
        // Build an output token.
        //

        Message = (PMESSAGE) OutputBuffer->pvBuffer;
        Message->MessageType = Challenge;
        memset(Message->Buffer,'y',MESSAGE_SIZE);
        OutputBuffer->cbBuffer = sizeof(MESSAGE);

        AddContext(Context);

        return(SEC_I_CONTINUE_NEEDED);

    } else {
        //
        // This is the second call. Lookup the old context.
        //

        Context = LocateContext(phContext->dwUpper);
        if (Context == NULL)
        {
            return(SEC_E_INVALID_HANDLE);
        }
        if ((Context->State != FirstAccept) &&
            (Context->State != SecondAccept))
        {
            return(SEC_E_INVALID_HANDLE);
        }

        Message = (PMESSAGE) InputBuffer->pvBuffer;

        //
        // Check that the input message is what we expected.
        //

        if (Context->State == FirstAccept)
        {
            SampleMessage.MessageType = ChallengeResponse;
            memset(SampleMessage.Buffer, 'z', MESSAGE_SIZE);
        }
        else
        {
            SampleMessage.MessageType = ReAuthenticate;
            memset(SampleMessage.Buffer, 'q', MESSAGE_SIZE);
        }

        if (memcmp(&SampleMessage,Message,MESSAGE_SIZE) != 0)
        {
            return(SEC_E_INVALID_TOKEN);
        }

        Context->State = SecondAccept;
        return(SEC_E_OK);
    }
}






//+-------------------------------------------------------------------------
//
//  Function:   DeleteSecurityContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
DeleteSecurityContext(
    PCtxtHandle                 phContext           // Context to delete
    )
{
    if (DeleteContext(phContext->dwUpper))
    {
        return(SEC_E_OK);
    }
    else
    {
        return(SEC_E_INVALID_HANDLE);
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   ApplyControlToken
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
ApplyControlToken(
    PCtxtHandle                 phContext,          // Context to modify
    PSecBufferDesc              pInput              // Input token to apply
    )
{
    return(SEC_E_UNSUPPORTED_FUNCTION);

}




//+-------------------------------------------------------------------------
//
//  Function:   EnumerateSecurityPackagesW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfoW SEC_FAR *      ppPackageInfo       // Receives array of info
    )
{
    SECURITY_STATUS SecStatus;

    SecStatus = QuerySecurityPackageInfoW(
                    PACKAGE_NAME,
                    ppPackageInfo
                    );
    if (SecStatus == SEC_E_OK)
    {
        *pcPackages = 1;
    }

    return(SecStatus);
}



//+-------------------------------------------------------------------------
//
//  Function:   QuerySecurityPackageInfoW
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
QuerySecurityPackageInfoW(
    LPWSTR                      pszPackageName,     // Name of package
    PSecPkgInfoW SEC_FAR *      ppPackageInfo        // Receives package info
    )
{
    PSecPkgInfoW PackageInfo;
    ULONG PackageInfoSize;
    PUCHAR Where;

    if (_wcsicmp(pszPackageName, PACKAGE_NAME))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    PackageInfoSize = sizeof(SecPkgInfoW) +
                        (wcslen(PACKAGE_NAME) + 1 +
                         wcslen(PACKAGE_COMMENT) + 1) * sizeof(WCHAR);

    PackageInfo = (PSecPkgInfoW) LocalAlloc(0,PackageInfoSize);
    if (PackageInfo == NULL)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }
    PackageInfo->fCapabilities = PACKAGE_CAPABILITIES;
    PackageInfo->wVersion = PACKAGE_VERSION;
    PackageInfo->wRPCID = PACKAGE_RPCID;
    PackageInfo->cbMaxToken = PACKAGE_MAXTOKEN;

    Where = (PUCHAR) (PackageInfo + 1);
    PackageInfo->Name = (LPWSTR) Where;
    Where += (wcslen(PACKAGE_NAME) + 1) * sizeof(WCHAR);
    RtlCopyMemory(PackageInfo->Name, PACKAGE_NAME, sizeof(PACKAGE_NAME));

    PackageInfo->Comment = (LPWSTR) Where;
    Where += (wcslen(PACKAGE_COMMENT) + 1) * sizeof(WCHAR);
    RtlCopyMemory(PackageInfo->Comment, PACKAGE_COMMENT, sizeof(PACKAGE_COMMENT));

    assert(Where - (PBYTE) PackageInfo == (LONG) PackageInfoSize);

    *ppPackageInfo = PackageInfo;
    return(SEC_E_OK);
}






//+-------------------------------------------------------------------------
//
//  Function:   FreeContextBuffer
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------

SECURITY_STATUS SEC_ENTRY
FreeContextBuffer(
    void SEC_FAR *      pvContextBuffer
    )
{
    LocalFree(pvContextBuffer);
    return(SEC_E_OK);
}



//+-------------------------------------------------------------------------
//
//  Function:   CompleteAuthToken
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
CompleteAuthToken(
    PCtxtHandle                 phContext,          // Context to complete
    PSecBufferDesc              pToken              // Token to complete
    )
{

    return(SEC_E_OK);

}



//+-------------------------------------------------------------------------
//
//  Function:   ImpersonateSecurityContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
ImpersonateSecurityContext(
    PCtxtHandle                 phContext           // Context to impersonate
    )
{
    return(SEC_E_OK);
}



//+-------------------------------------------------------------------------
//
//  Function:   RevertSecurityContext
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------


SECURITY_STATUS SEC_ENTRY
RevertSecurityContext(
    PCtxtHandle                 phContext           // Context from which to re
    )
{
    return(SEC_E_OK);
}


//+-------------------------------------------------------------------------
//
//  Function:   QueryContextAttributes
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
QueryContextAttributesW(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    PSEC_CONTEXT Context;
    PSecPkgContext_Sizes ContextSizes;
    PSecPkgContext_NamesW ContextNames;
    PSecPkgContext_Lifespan ContextLifespan;
    PSecPkgContext_DceInfo  ContextDceInfo;

    Context = LocateContext(phContext->dwUpper);

    if (Context == NULL)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    switch(ulAttribute) {
    case SECPKG_ATTR_SIZES:
        ContextSizes = (PSecPkgContext_Sizes) pBuffer;
        ContextSizes->cbMaxSignature = PACKAGE_SIGNATURE_SIZE;
        if ((Context->ContextFlags & ISC_REQ_CONFIDENTIALITY) != 0)
        {
            ContextSizes->cbSecurityTrailer = PACKAGE_SIGNATURE_SIZE;
            ContextSizes->cbBlockSize = 1;
        }
        else
        {
            ContextSizes->cbSecurityTrailer = 0;
            ContextSizes->cbBlockSize = 0;
        }
        ContextSizes->cbMaxToken = PACKAGE_MAXTOKEN;
        break;
    case SECPKG_ATTR_NAMES:
        ContextNames = (PSecPkgContext_Names) pBuffer;
        ContextNames->sUserName = (LPWSTR) LocalAlloc(0, sizeof(L"dummy user"));
        if (ContextNames->sUserName == NULL)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }
        RtlCopyMemory(ContextNames->sUserName, L"dummy user", sizeof(L"dummy user"));
        break;
    case SECPKG_ATTR_LIFESPAN:
        ContextLifespan = (PSecPkgContext_Lifespan) pBuffer;
        ContextLifespan->tsStart = Never;
        ContextLifespan->tsExpiry = Forever;
        break;
    case SECPKG_ATTR_DCE_INFO:
        ContextDceInfo = (PSecPkgContext_DceInfo) pBuffer;
        ContextDceInfo->AuthzSvc = 0;
        ContextDceInfo->pPac = (PVOID) LocalAlloc(0,sizeof(L"dummy user"));
        if (ContextDceInfo->pPac == NULL)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }
        RtlCopyMemory((LPWSTR) ContextDceInfo->pPac, L"dummy user", sizeof(L"dummy user"));

        break;
    default:
        return(SEC_E_INVALID_TOKEN);
    }

    return(SEC_E_OK);
}

#if 0

//+-------------------------------------------------------------------------
//
//  Function:   QueryCredentialsAttributes
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//
//--------------------------------------------------------------------------



SECURITY_STATUS SEC_ENTRY
QueryCredentialsAttributesW(
    PCredHandle                 phCredentials,      // Credentials to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    return( pspPackages[phCredentials->dwLower].pftTableW->QueryCredentialsAttributesW(
                                        phCredentials,
                                        ulAttribute,
                                        pBuffer ) );
}

#endif

//+-------------------------------------------------------------------------
//
//  Function:   MakeSignature
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [phContext]     -- context to use
//              [fQOP]          -- quality of protection to use
//              [pMessage]      -- message
//              [MessageSeqNo]  -- sequence number of message
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS SEC_ENTRY
MakeSignature(  PCtxtHandle         phContext,
                ULONG               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{
    PSEC_CONTEXT Context;
    PSecBuffer SignatureBuffer;
    PSIGNATURE Signature;

    Context = LocateContext(phContext->dwUpper);
    if (Context == NULL)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    SignatureBuffer = LocateSigBuffer(pMessage);
    if (SignatureBuffer == NULL)
    {
        return(SEC_E_INVALID_TOKEN);
    }
    Signature = (PSIGNATURE) SignatureBuffer->pvBuffer;
    Signature->MessageId = Context->Nonce++;
    Signature->Tag = PACKAGE_SIGNATURE_TAG;
    SignatureBuffer->cbBuffer = sizeof(SIGNATURE);
    return(SEC_E_OK);
}



//+-------------------------------------------------------------------------
//
//  Function:   VerifySignature
//
//  Synopsis:
//
//  Effects:
//
//  Arguments:  [phContext]     -- Context performing the unseal
//              [pMessage]      -- Message to verify
//              [MessageSeqNo]  -- Sequence number of this message
//              [pfQOPUsed]     -- quality of protection used
//
//  Requires:
//
//  Returns:
//
//  Notes:
//
//--------------------------------------------------------------------------
SECURITY_STATUS SEC_ENTRY
VerifySignature(PCtxtHandle     phContext,
                PSecBufferDesc  pMessage,
                ULONG           MessageSeqNo,
                ULONG *         pfQOP)
{
    PSEC_CONTEXT Context;
    PSecBuffer SignatureBuffer;
    PSIGNATURE Signature;

    Context = LocateContext(phContext->dwUpper);
    if (Context == NULL)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    SignatureBuffer = LocateSigBuffer(pMessage);
    if (SignatureBuffer == NULL)
    {
        return(SEC_E_INVALID_TOKEN);
    }
    Signature = (PSIGNATURE) SignatureBuffer->pvBuffer;

    if (Signature->Tag != PACKAGE_SIGNATURE_TAG)
    {
        return(SEC_E_MESSAGE_ALTERED);
    }

    if (Signature->MessageId != Context->Nonce++)
    {
        Context->State = Idle;
        return(SEC_E_OUT_OF_SEQUENCE);
    }

    return(SEC_E_OK);
}





