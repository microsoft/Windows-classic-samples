//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#ifndef _UTILITY_H_
#define _UTILITY_H_
#include <aclui.h>
#include <aclapi.h>
#include <windows.h>
#include "resource.h"

// INHERIT_ONLY_ACE and NO_PROPAGATE_INHERIT_ACE are inheritable ACEs, 
// but if they don't have OBJECT or CONTAINER inheritance set, then 
// they aren't considered.
#define IsInheritableAce(aceFlags) \
    (IS_FLAG_SET((aceFlags), OBJECT_INHERIT_ACE) || \
        IS_FLAG_SET((aceFlags), CONTAINER_INHERIT_ACE)) 

#define IsAccessAllowedAce(aceType) \
    ((aceType) == ACCESS_ALLOWED_CALLBACK_ACE_TYPE ||\
        (aceType) == ACCESS_ALLOWED_ACE_TYPE)

#define FailGracefully(hr, text)                    \
    { if ( FAILED(hr) ) { wprintf(text); goto exit_gracefully; } }

#define FailGracefullyGLE(bResult, text)                      \
    { if ( (bResult) == FALSE ) { errorCode = GetLastError(); \
    hr = HRESULT_FROM_WIN32(errorCode);                       \
    if ( FAILED(HRESULT_FROM_WIN32(hr)) ) { wprintf(text);    \
    goto exit_gracefully; } } }
    
#define ARGUMENT_PRESENT(ArgumentPointer) (\
    (CHAR *)((ULONG_PTR)(ArgumentPointer)) != (CHAR *)(NULL) ) 

// Allocates space for 'dest' and copies 'source' into it
HRESULT AllocAndCopyString(_In_ PCWSTR source, _Outptr_ PWSTR *dest);

// Removes all ACEs with the "ID" flag set from ppAcl.
HRESULT RemoveAllInheritedAces(PACL *acl);

HRESULT AddAllAcesFromAcl(PACL sourceAcl, PACL *destAcl, bool onlyAddUnique);

HRESULT RemoveExplicitUniqueAces(PACL acl, PACL *destAcl);

// Determine if an ACE is in an ACL.
//  acl - the ACL
//  ace - the ACE to check for
//  lpbAcePresent - this will be set to true if the ACE exists in the ACL
//  forInheritancePurposes - if true, this will ignore the "ID" flag on the ACE
//                          and other inheritance flags on ACEs in the ACL
HRESULT ACEAlreadyInACL(
    _In_  PACL acl, 
    _In_  LPVOID ace, 
    _Out_ LPBOOL lpbAcePresent, 
    _In_  bool forInheritancePurposes);

HRESULT AddAceToAcl(LPVOID pNewAce, PACL *acl, bool bAddToEndOfList);

LPVOID GetSidStart(BYTE aceType, LPVOID ace);

HRESULT AddInheritableAcesFromAcl(PACL sourceAcl, PACL *destAcl);

HRESULT GetSizeOfAllInheritableAces(PACL acl, DWORD &dwSizeNeeded);

// Converts a self-relative security descriptor to absolute.
// Note: GetSecurityDescriptor* APIs work either with self-relative OR absolute,
// but SetSecurityDescriptorDacl will only work with absolute.
HRESULT ConvertSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSelfRelSD, 
    PSECURITY_DESCRIPTOR *absoluteSD);

#endif // _UTILITY_H_
