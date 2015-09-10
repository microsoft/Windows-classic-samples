//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
// ResourceAttributesSample.h
//
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sddl.h>
#include <AccCtrl.h>
#include <AclApi.h>
#include <NTSecAPI.h>
#include <authz.h>
#include <strsafe.h>
#include <assert.h>

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define SDDL_LEN_TAG(Tag)  (sizeof(Tag) / sizeof( WCHAR ) - 1 )
#define ALIGN(Size)        (((ULONG)(Size) + 3) & -4)


typedef struct _RESOURCE_ATTRIBUTE
{
    LPWSTR Name;
    LPWSTR Flags;
    LPWSTR Type;
    LPWSTR Values;
} RESOURCE_ATTRIBUTE, *PRESOURCE_ATTRIBUTE;

__inline BOOL 
FileParameter(_In_ LPCWSTR Flag)
{
    return ((0 == _wcsicmp(Flag, L"-file")) || (0 ==_wcsicmp(Flag, L"/file")));
}

__inline BOOL 
CapParameter(_In_ LPCWSTR Flag)
{
    return ((0 == _wcsicmp(Flag, L"-cap")) || (0 == _wcsicmp(Flag, L"/cap")));
}

__inline BOOL 
RaParameter(_In_ LPCWSTR Flag)
{
    return ((0 == _wcsicmp(Flag, L"-ra")) || (0 == _wcsicmp(Flag, L"/ra")));
}

__inline WORD
GetValueType(_In_ LPWSTR ClaimType)
{
    if(0 == _wcsnicmp(ClaimType, SDDL_INT, SDDL_LEN_TAG(SDDL_INT))) 
    {
        return CLAIM_SECURITY_ATTRIBUTE_TYPE_INT64;
    }
    else if(0 == _wcsnicmp(ClaimType, SDDL_UINT, SDDL_LEN_TAG(SDDL_UINT))) 
    {
        return CLAIM_SECURITY_ATTRIBUTE_TYPE_UINT64;
    }
    else if(0 == _wcsnicmp(ClaimType,SDDL_WSTRING,SDDL_LEN_TAG(SDDL_WSTRING))) 
    {
        return CLAIM_SECURITY_ATTRIBUTE_TYPE_STRING;
    }
    else if(0 == _wcsnicmp(ClaimType,SDDL_BOOLEAN,SDDL_LEN_TAG(SDDL_BOOLEAN))) 
    {
        return CLAIM_SECURITY_ATTRIBUTE_TYPE_BOOLEAN;
    }
    else 
    {
        return CLAIM_SECURITY_ATTRIBUTE_TYPE_INVALID;
    }
}

__inline DWORD
GetValueSize(_In_ WORD ClaimType)
{
    SIZE_T Size;

    switch(ClaimType)
    {
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_INT64:
        Size = sizeof(LONG64);
        break;
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_UINT64:
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_BOOLEAN:
        Size = sizeof(DWORD64);
        break;
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_STRING:
        Size = sizeof(PWSTR);
        break;
    default:
        Size = 0;
        break;
    }
    return static_cast<DWORD>(Size);
}


_Success_(return == TRUE) 
BOOL
CreateSecurityDescriptor(
    _In_        PSID                          CapIDSid, 
    _In_reads_(ResourceAttributeCt) PCLAIM_SECURITY_ATTRIBUTE_V1* ResourceAttributes,
    _In_        DWORD                         ResourceAttributeCt,
    _Outptr_    PSECURITY_DESCRIPTOR*         SecurityDescriptorResult
    );

BOOL 
PerformAccessCheck(
    _In_ PSECURITY_DESCRIPTOR SecurityDescriptor
    );

_Success_(return == TRUE) 
BOOL
IsValidCapID(
    _In_      LPCWSTR CapID,
    _Outptr_  PSID*   CapIDSidResult
    );

_Success_(return == TRUE) 
BOOL
GetFileResourceAttributes(
    _In_                           LPCWSTR FileName,
    _Outptr_result_buffer_(*Count) PCLAIM_SECURITY_ATTRIBUTE_V1* FileResourceAttributes[],
    _Out_                          DWORD*  Count
    );

BOOL
ParseResourceAttributeString(
    _In_    LPWSTR              AttributeString,
    _Inout_ RESOURCE_ATTRIBUTE& Attribute
    );

_Success_(return == TRUE) 
BOOL
InterpretResourceAttribute(
    _In_     RESOURCE_ATTRIBUTE            AttributeTokens,
    _Outptr_ PCLAIM_SECURITY_ATTRIBUTE_V1* ResourceClaimReslut
    );

_Success_(return == TRUE) 
BOOL
ParseResourceAttributesArguments(
    _In_                           LPCWSTR ResourceAttributesArgs,
    _Outptr_result_buffer_(*Count) PCLAIM_SECURITY_ATTRIBUTE_V1* ResourceAttributesResult[],
    _Out_                          DWORD*  Count
    );

VOID
PrintResourceAttribute(
    _In_ RESOURCE_ATTRIBUTE Attribute
    );

VOID
PrintErrorMessage(
    _In_ LPWSTR ErrorMessage,
    _In_ DWORD  ErrorCode
    );

VOID 
PrintUsage();
