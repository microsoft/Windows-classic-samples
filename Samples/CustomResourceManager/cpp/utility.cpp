//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "utility.h"
#include "CSecInfo.h"
#include "authz.h"
#include <stdio.h>
#include <strsafe.h> // for StringCchCopy
#include <intsafe.h> // for UShortAdd, UShortMult
#include "sddl.h"
#include <string>

HRESULT AllocAndCopyString(_In_ PCWSTR source, _Outptr_ PWSTR *dest)
{
    HRESULT hr = S_OK;
    size_t len;

    if ( !ARGUMENT_PRESENT(source) )
    {
        return E_INVALIDARG;
    }

    len = wcslen(source);
    // Remember: wcslen excludes the null character when calculating the length.
    // Also, StringCchCopyW takes the total length of the destination buffer
    *dest = (PWSTR)LocalAlloc(LMEM_ZEROINIT, ((len + 1) * sizeof(wchar_t)));
    if ( !(*dest) )
    {
        wprintf(L"LocalAlloc failed.\n");
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }

    hr = StringCchCopyW(*dest, len + 1, source);
    FailGracefully(hr, L"StringCchCopyW");

exit_gracefully:

    if ( !SUCCEEDED(hr) )
    {
        LocalFree(*dest);
    }

    return hr;
}

HRESULT RemoveAllInheritedAces(PACL *ppAcl)
{
    BOOL bResult = FALSE;
    DWORD errorCode = S_OK;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    DWORD totalCount;
    DWORD aceIndex = 0;
    LPVOID ace = nullptr;
    BYTE aceFlags = 0;

    if ( !ARGUMENT_PRESENT(*ppAcl) )
    {
        return E_INVALIDARG;
    }

    bResult = GetAclInformation(
        *ppAcl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");
    
    totalCount = aclInformation.AceCount;
    while ( aceIndex < totalCount ) 
    {
        bResult = GetAce(
            *ppAcl,
            aceIndex,
            &ace
            );
        FailGracefullyGLE(bResult, L"GetAce");
                        
        aceFlags = ((PACE_HEADER)ace)->AceFlags; 

        if (IS_FLAG_SET(aceFlags,INHERITED_ACE)) 
        {
            bResult = DeleteAce(
                *ppAcl,
                aceIndex);
            FailGracefullyGLE(bResult, L"DeleteAce");

            totalCount--;
        }
        else
        {
            aceIndex++;
        }
    }

exit_gracefully:
    return hr;
}

// Adds all ACEs from sourceAcl to ppDestAcl.
// If onlyAddUnique is specified, then this function will ensure that
// ACEs from sourceAcl aren't added if they already appear in ppDestAcl.
HRESULT AddAllAcesFromAcl(PACL sourceAcl, PACL *ppDestAcl, bool onlyAddUnique)
{
    BOOL bResult = 0;
    DWORD errorCode = S_OK;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    LPVOID ace;
    bool addIt = true;
    BOOL alreadyExists = FALSE;

    if ( !ARGUMENT_PRESENT(sourceAcl) || !ARGUMENT_PRESENT(*ppDestAcl) )
    {
        return E_INVALIDARG;
    }

    bResult = GetAclInformation(
        sourceAcl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );

    FailGracefullyGLE(bResult, L"GetAclInformation");

    for ( DWORD i = 0; i < aclInformation.AceCount; i ++ )
    {
        bResult = GetAce(
            sourceAcl,
            i,
            &ace
            );

        FailGracefullyGLE(bResult, L"GetAce");

        // If we only want to add unique ACEs, then we
        // need to make sure that this doesn't already exist
        // in the destination.
        addIt = true;
        if ( onlyAddUnique )
        {

            hr = ACEAlreadyInACL(
                *ppDestAcl,
                ace,
                &alreadyExists,
                false
                );

            FailGracefully(hr, L"ACEAlreadyInACL");

            if ( alreadyExists != FALSE )
            {
                addIt = false;
            }
        }

        if ( addIt )
        {
            hr = AddAceToAcl(ace, ppDestAcl, false);
            FailGracefully(hr, L"AddAceToAcl");
        }
    }
exit_gracefully:

    return hr;
}

// This function will remove the noninherited ACEs from
// ppDestAcl that don't show up in 'acl'. This is needed for
// SetSecurity - inherited ACEs need to stay, and any ACEs that
// are being set need to stay, but all others need to go.
HRESULT RemoveExplicitUniqueAces(PACL acl, PACL *ppDestAcl)
{
    BOOL bResult = 0;
    DWORD errorCode = S_OK;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    DWORD totalCount;
    DWORD aceIndex = 0;
    LPVOID ace;
    BYTE aceFlags;
    BOOL alreadyExists = FALSE;

    if ( !ARGUMENT_PRESENT(acl) || !ARGUMENT_PRESENT(*ppDestAcl) )
    {
        return E_INVALIDARG;
    }

    bResult = GetAclInformation(
        *ppDestAcl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");

    totalCount = aclInformation.AceCount;
    while ( aceIndex < totalCount )
    {
        bResult = GetAce(
            *ppDestAcl,
            aceIndex,
            &ace
            );

        FailGracefullyGLE(bResult, L"GetAce");
        
        aceFlags = ((PACE_HEADER)ace)->AceFlags; 

        // We only care about explcit (i.e. non-inherited) ACEs
        if ( IS_FLAG_SET(aceFlags, INHERITED_ACE) )
        {
            aceIndex++;
            continue;
        }

        hr = ACEAlreadyInACL(
            acl,
            ace,
            &alreadyExists,
            false
            );

        FailGracefully(hr, L"ACEAlreadyInACL");
        
        // If it's not in 'acl', then it's unique
        if ( alreadyExists == FALSE )
        {
            bResult = DeleteAce(*ppDestAcl, aceIndex);
            FailGracefullyGLE(bResult, L"DeleteAce");
            totalCount--;
        }
        else
        {
            aceIndex++;
        }
    }

exit_gracefully:
    return hr;
}

// Go through all of the ACEs in the ACL and see if 'ace' is equal to any of them.
HRESULT ACEAlreadyInACL(
        _In_ PACL acl, 
        _In_ LPVOID ace, 
        _Out_ LPBOOL lpbAcePresent, 
        _In_ bool forInheritancePurposes)
{
    BOOL bResult = 0;
    DWORD errorCode = 0;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    BYTE aceType;
    WORD aceSize;
    BYTE aceFlags;
    BYTE existingAceType;
    WORD existingAceSize;
    BYTE existingAceFlags;
    DWORD totalCount;
    LPVOID pExistingAce;
    LPVOID sidStart1;
    LPVOID sidStart2;
    DWORD sidLength1;
    DWORD sidLength2;
    int result;
    
    if ( !ARGUMENT_PRESENT(acl) || !ARGUMENT_PRESENT(ace) )
    {
        return E_INVALIDARG;
    }

    aceType = ((PACE_HEADER)ace)->AceType;
    aceSize = ((PACE_HEADER)ace)->AceSize;
    aceFlags = ((PACE_HEADER)ace)->AceFlags;

    if ( forInheritancePurposes )
    {
        // If the ACE wasn't inherited, then we don't care about it
        if ( !IS_FLAG_SET(aceFlags, INHERITED_ACE) )
        {
            *lpbAcePresent = FALSE;
            return S_OK;
        }
        else
        {
            // It was inherited. Don't use that flag for comparison though
            // because it won't appear on the source.
            aceFlags &= ~INHERITED_ACE;
        }
    }

    bResult = GetAclInformation(
        acl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");
    
    totalCount = aclInformation.AceCount;
    for ( DWORD i = 0; i < totalCount; i ++ )
    {
        bResult = GetAce(
            acl,
            i,
            &pExistingAce
            );
        FailGracefullyGLE(bResult, L"GetAce");
        
        existingAceType = ((PACE_HEADER)pExistingAce)->AceType;
        existingAceSize = ((PACE_HEADER)pExistingAce)->AceSize;
        existingAceFlags = ((PACE_HEADER)pExistingAce)->AceFlags; 

        if ( forInheritancePurposes )
        {
            if ( !IsInheritableAce(existingAceFlags) )
            {
                // We only care about inheritable ACEs
                continue;
            }
            else
            {
                // Wipe out the inheritance flags that couldn't possibly be on the 
                // child. (e.g. when we have a NO_PROPAGATE_INHERIT_ACE, the ACE 
                // will make it to the child, but the flag will get unset in 
                // AddInheritableAcesFromAcl).
                existingAceFlags &= ~INHERIT_ONLY_ACE;

                if ( IS_FLAG_SET(existingAceFlags, NO_PROPAGATE_INHERIT_ACE) )
                {
                    existingAceFlags &= ~NO_PROPAGATE_INHERIT_ACE;

                    // The child doesn't get the [...]_INHERIT_ACE
                    // flag if NO_PROPAGATE was specified.
                    existingAceFlags &= ~CONTAINER_INHERIT_ACE;
                    existingAceFlags &= ~OBJECT_INHERIT_ACE;
                }
            }
        }

        if ( existingAceFlags == aceFlags &&
               existingAceSize == aceSize &&
               existingAceType == aceType )
        {
            // That was our quick check, now we should compare the 
            // actual contents (mask and SID)
            sidStart1 = GetSidStart(aceType, pExistingAce);
            sidStart2 = GetSidStart(aceType, ace);
            sidLength1 = GetLengthSid((PSID)sidStart1);
            sidLength2 = GetLengthSid((PSID)sidStart2);

            // This is needed even though we just compared the sizes because 
            // of callback aces - the size of the ACLs may coincidentally be 
            // the same because the sums of the sid length and condition 
            // sizes are equal
            if ( sidLength1 != sidLength2 )
            {
                continue;
            }

            result = memcmp(
                (PSID)sidStart1, 
                (PSID)sidStart2, 
                aceSize - sizeof(ACE_HEADER) - sizeof(ACCESS_MASK));
            if ( result == 0  )
            {
                *lpbAcePresent = TRUE;
                return S_OK;
            }
        }
    }

    *lpbAcePresent = FALSE;
exit_gracefully:
    return hr;
}

// Add an ACE to an ACL, expanding the ACL if needed.
HRESULT AddAceToAcl(LPVOID pNewAce, PACL *ppAcl, bool bAddToEndOfList)
{
    DWORD addPosition = bAddToEndOfList ? MAXDWORD : 0;
    BOOL bResult = FALSE;
    DWORD errorCode = S_OK;
    PACL biggerDACL = nullptr;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    WORD aceSize;
    DWORD oldSize;
    USHORT uSize;
    DWORD dwNewSizeNeeded;

    if ( !ARGUMENT_PRESENT(pNewAce) || !ARGUMENT_PRESENT(*ppAcl) )
    {
        return E_INVALIDARG;
    }

    bResult = AddAce(
        *ppAcl,
        ACL_REVISION, 
        addPosition,
        pNewAce,
        ((PACE_HEADER)pNewAce)->AceSize
        );

    // See if we failed due to an insufficient buffer size
    if ( !bResult )
    {
        errorCode = GetLastError();
        if ( errorCode == ERROR_INSUFFICIENT_BUFFER )
        {
            // Not enough space. Allocate a bigger ACL.

            bResult = GetAclInformation(
                *ppAcl,
                &aclInformation,
                sizeof(aclInformation),
                AclSizeInformation
                );
            FailGracefullyGLE(bResult, L"GetAclInformation");
                            
            aceSize = ((PACE_HEADER)pNewAce)->AceSize;
            oldSize = aclInformation.AclBytesInUse;

            // Can't overflow WORD boundary
            hr = UShortAdd(aceSize, (USHORT)oldSize, &uSize);
            FailGracefully(hr, L"UShortAdd");
            
            // Give us some extra space so that we don't have to keep
            // reallocating.
            hr = UShortMult(uSize, 2, &uSize);
            FailGracefully(hr, L"UShortMult");

            dwNewSizeNeeded = (DWORD)uSize;
            // Align to a DWORD
            dwNewSizeNeeded = (dwNewSizeNeeded + (sizeof(DWORD) - 1)) & 0xfffffffc;

            // Make sure the alignment didn't overflow the size of a WORD
            // (the process of aligning can add up to 3)
            if ( dwNewSizeNeeded > USHORT_MAX )
            {
                // It's safe to subtrat now (despite the "size needed" name)
                // because we overestimated earlier by multiplying by 2.
                dwNewSizeNeeded -= 4;
            }

            biggerDACL = (PACL)LocalAlloc(LPTR, dwNewSizeNeeded);
            if ( !biggerDACL )
            {
                wprintf(L"LocalAlloc failed.\n");
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }
                            
            // Note: no need to initialize the new ACL
            memcpy(biggerDACL, *ppAcl, oldSize);

            // This cast is safe because we checked for overflow earlier
            biggerDACL->AclSize = (WORD)dwNewSizeNeeded;

            *ppAcl = biggerDACL;
            
            // Make sure this doesn't get freed later
            biggerDACL = nullptr;

            bResult = AddAce(
                *ppAcl,
                ACL_REVISION, 
                addPosition,
                pNewAce,
                ((PACE_HEADER)pNewAce)->AceSize
                );
            FailGracefullyGLE(bResult, L"AddAce");
        }
        else
        {
            return HRESULT_FROM_WIN32(errorCode);
        }
    }
exit_gracefully:
    LocalFree(biggerDACL);

    return hr;
}

LPVOID GetSidStart(BYTE aceType, LPVOID ace)
{
    LONG offset = 0;
    switch (aceType) 
    {
    case ACCESS_ALLOWED_ACE_TYPE:
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
        offset = FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart);
        break;
    case SYSTEM_AUDIT_CALLBACK_ACE_TYPE:
        offset = FIELD_OFFSET(SYSTEM_AUDIT_CALLBACK_ACE, SidStart);
        break;
    case ACCESS_ALLOWED_CALLBACK_ACE_TYPE:
        offset = FIELD_OFFSET(ACCESS_ALLOWED_CALLBACK_ACE, SidStart);
        break;
    default:
        return 0;
    }

    // We use PUCHAR so that we can add an offset to the pointer
    return (PUCHAR)ace + offset;
}

HRESULT ConvertSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSelfRelSD, 
    PSECURITY_DESCRIPTOR *ppAbsoluteSD)
{
    BOOL bResult;
    PACL pDacl = nullptr;
    PACL pSacl = nullptr;
    PSID pOwner = nullptr;
    PSID pPrimaryGroup = nullptr;
    DWORD dwAbsoluteSDSize = 0;
    DWORD dwDaclSize = 0;
    DWORD dwSaclSize = 0;
    DWORD dwOwnerSize = 0;
    DWORD dwPrimaryGroupSize = 0;
    size_t sizeNeeded = 0;
    *ppAbsoluteSD = nullptr;

    // Confirm that pSelfRelSD is actually self relative
    if( 
        !ARGUMENT_PRESENT(pSelfRelSD) || 
        !IS_FLAG_SET(((SECURITY_DESCRIPTOR*)(pSelfRelSD))->Control, SE_SELF_RELATIVE)
        )
    {
        return E_INVALIDARG;
    }

    // Call it once so that the sizes will be set to the correct values. This 
    // will return a buffer-too-small error, so we don't check the return value
    MakeAbsoluteSD(
        pSelfRelSD,
        nullptr,
        &dwAbsoluteSDSize,
        nullptr,
        &dwDaclSize,
        nullptr,
        &dwSaclSize,
        nullptr,
        &dwOwnerSize,
        nullptr,
        &dwPrimaryGroupSize
        );

    sizeNeeded = 
        dwAbsoluteSDSize + 
        dwDaclSize + 
        dwSaclSize + 
        dwOwnerSize + 
        dwPrimaryGroupSize;
    *ppAbsoluteSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, sizeNeeded);

    if ( !*ppAbsoluteSD )
    {
        return E_OUTOFMEMORY;
    }
    
    BYTE* position = (BYTE *)(*ppAbsoluteSD);  
    pDacl = reinterpret_cast<PACL>(position += dwAbsoluteSDSize);  
    pSacl  = reinterpret_cast<PACL>(position += dwDaclSize);  
    pOwner = reinterpret_cast<PSID>(position += dwSaclSize);  
    pPrimaryGroup = reinterpret_cast<PSID>(position += dwOwnerSize); 

    bResult = MakeAbsoluteSD(
        pSelfRelSD,
        *ppAbsoluteSD,
        &dwAbsoluteSDSize,
        pDacl,
        &dwDaclSize,
        pSacl,
        &dwSaclSize,
        pOwner,
        &dwOwnerSize,
        pPrimaryGroup,
        &dwPrimaryGroupSize
        );

    if ( !bResult )
    {
        LocalFree(*ppAbsoluteSD);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

// This goes through every ACE in 'acl', sums the size of the inheritable
// ACEs, and puts it in dwSizeNeeded.
HRESULT GetSizeOfAllInheritableAces(PACL acl, DWORD &dwSizeNeeded)
{
    BOOL bResult;
    DWORD errorCode = S_OK;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    DWORD totalCount;
    LPVOID ace;
    BYTE aceFlags;

    if ( !ARGUMENT_PRESENT(acl) )
    {
        return E_INVALIDARG;
    }

    bResult = GetAclInformation(
        acl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");

    totalCount = aclInformation.AceCount;

    // Start with zero as the size. We'll only initialize this
    // to sizeof(ACL) if we find an inheritable ACE.
    dwSizeNeeded = 0;
                    
    for ( DWORD aceIndex = 0; aceIndex < totalCount; aceIndex ++ )
    {
        bResult = GetAce(
            acl,
            aceIndex,
            &ace
            );
        FailGracefullyGLE(bResult, L"GetAce");
                        
        aceFlags = ((PACE_HEADER)ace)->AceFlags; 

        // Only count the inheritable ACEs
        if (IsInheritableAce(aceFlags)) 
        {
            // Initialize the size now that we've found an inheritable ACE
            if ( dwSizeNeeded == 0 )
            {
                dwSizeNeeded = sizeof(ACL);
            }
            dwSizeNeeded += ((PACE_HEADER)ace)->AceSize;
        }
    }
exit_gracefully:
    return hr;
}

// Adds any inheritable ACEs from sourceAcl to ppDestAcl.
HRESULT AddInheritableAcesFromAcl(PACL sourceAcl, PACL *ppDestAcl)
{
    BOOL bResult = 0;
    DWORD errorCode = S_OK;
    HRESULT hr = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    LPVOID newAce = nullptr;
    LPVOID ace;
    BYTE aceFlags;
    WORD aceSize;
    
    if ( !ARGUMENT_PRESENT(sourceAcl) || !ARGUMENT_PRESENT(*ppDestAcl) )
    {
        return E_INVALIDARG;
    }

    bResult = GetAclInformation(
        sourceAcl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");

    for ( DWORD i = 0; i < aclInformation.AceCount; i ++ )
    {
        bResult = GetAce(
            sourceAcl,
            i,
            &ace
            );

        FailGracefullyGLE(bResult, L"GetAce");
        
        aceFlags = ((PACE_HEADER)ace)->AceFlags; 

        // We only care about the inheritable ACEs
        if (IsInheritableAce(aceFlags)) 
        {
            // We don't need to see if the ACE is already in the child's DACL
            // because we always remove all inherited ACEs before calling this
            // function, so an inheritable ACE can't exist in the child.
            //
            // The ACE that we add needs to have the "ID" flag set,
            // but we don't want to modify the parent's ACE, so we
            // need to make a new ACE.
            aceSize = ((PACE_HEADER)ace)->AceSize;
            newAce = (LPVOID)LocalAlloc(LPTR, aceSize);
            if ( !newAce )
            {
                wprintf(L"LocalAlloc failed.\n");
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }

            memcpy(newAce, ace, aceSize);
            ((PACE_HEADER)newAce)->AceFlags |= INHERITED_ACE;

            // It's important that we unset the inherit-only flag, or this
            // ACE will be overlooked by AccessCheck (likewise by effective
            // access checks).
            ((PACE_HEADER)newAce)->AceFlags &= ~INHERIT_ONLY;

            // NO_PROPAGATE_INHERIT_ACE means the inheritance only applies to
            // the child, not the grandchildren, so we need to unset the 
            // applicable inheritance flags.
            if ( IS_FLAG_SET(aceFlags, NO_PROPAGATE_INHERIT_ACE) )
            {
                ((PACE_HEADER)newAce)->AceFlags &= ~NO_PROPAGATE_INHERIT_ACE;
                ((PACE_HEADER)newAce)->AceFlags &= ~OBJECT_INHERIT_ACE;
                ((PACE_HEADER)newAce)->AceFlags &= ~CONTAINER_INHERIT_ACE;
            }

            hr = AddAceToAcl(newAce, ppDestAcl, false);
            FailGracefully(hr, L"AddAceToAcl");
            
            LocalFree(newAce);
            newAce = nullptr;
        }
    }

exit_gracefully:

    LocalFree(newAce);
    return hr;
}
