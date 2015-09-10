//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
//
// This file contains the definitions for all of the ISecurityInformation
// functions from CSecInfo. This got its own file because the implementation
// of ISecurityInformation is the largest of the interfaces.
//
#include "CSecInfo.h"
#include "utility.h"
#include "authz.h"
#include <stdio.h>
#include "sddl.h"
#include <string>

IFACEMETHODIMP CSecInfo::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo)
{
    Resource *currentResource;

    // SI_OBJECT_INFO: http://msdn.microsoft.com/en-us/library/windows/desktop/aa379605(v=vs.85).aspx
    m_dwSIFlags = 0
        | SI_ADVANCED       // The "advanced" button is displayed on the basic 
                            //     security property page
        | SI_EDIT_PERMS     // The basic security property page always allows 
                            //     basic editing of the object's DACL
        | SI_EDIT_OWNER     // This lets you change the owner on the advanced page
        | SI_PAGE_TITLE     // Use pObjectInfo->pszPageTitle for the basic page's 
                            //     title
        | SI_VIEW_ONLY      // Displays a read-only version of the ACL Editor 
                            //     dialog boxes. This is required if you're 
                            //     implementing ISecurityInformation3.
        | SI_EDIT_EFFECTIVE
        | SI_ENABLE_EDIT_ATTRIBUTE_CONDITION
        ;

    currentResource = m_resources[m_editingResource];

    if ( currentResource->IsContainer() != FALSE)
    {
        // This will make ACL UI show the inheritance controls
        m_dwSIFlags |= SI_CONTAINER;
    }

    pObjectInfo->dwFlags = m_dwSIFlags;
    pObjectInfo->hInstance = 0;                               
    pObjectInfo->pszServerName = nullptr;

    // ACL Editor won't free this, so we don't need to make a copy
    pObjectInfo->pszObjectName = currentResource->GetName();
    pObjectInfo->pszPageTitle = L"Forums Resource Manager";

    return S_OK;
}

IFACEMETHODIMP CSecInfo::GetSecurity(
    SECURITY_INFORMATION si, 
    PSECURITY_DESCRIPTOR *ppSD, 
    BOOL fDefault)
{
    BOOL bResult = 0;
    HRESULT hr = S_OK;
    DWORD errorCode = 0;
    Resource *currentResource = m_resources[m_editingResource];
    ULONG defaultSecurityDescriptorSize = 0;

    // This may be the default SD, or it may 
    // be the SD on the resource we're editing
    PWSTR sdToEdit = NULL;

    if ( fDefault )
    {
        hr = AllocAndCopyString(m_defaultSecurityDescriptorSddl, &sdToEdit);
        FailGracefully(hr, L"AllocAndCopyString");
    }
    else
    {
        hr = AllocAndCopyString(
            const_cast<PCWSTR>(currentResource->GetSD()), 
            &sdToEdit);
        FailGracefully(hr, L"AllocAndCopyString");
    }
    
    if ( 
        IS_FLAG_SET(si, DACL_SECURITY_INFORMATION) || 
        IS_FLAG_SET(si, OWNER_SECURITY_INFORMATION) || 
        IS_FLAG_SET(si, GROUP_SECURITY_INFORMATION)  
        ) 
    {
        // The following function will populate the entire SD.
        bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(
            sdToEdit,
            SDDL_REVISION_1,
            ppSD,
            &defaultSecurityDescriptorSize);
        
        FailGracefullyGLE(
            bResult, 
            L"ConvertStringSecurityDescriptorToSecurityDescriptor");
    }

exit_gracefully:
    LocalFree(sdToEdit);

    return hr;
}


IFACEMETHODIMP CSecInfo::SetSecurity(
    SECURITY_INFORMATION si, 
    PSECURITY_DESCRIPTOR pSD)
{
    BOOL bResult = 0;
    DWORD errorCode = 0;
    HRESULT hr = S_OK;
    PACL pDestDacl = nullptr;
    BOOL bDaclPresent = 0;
    BOOL bDaclDefaulted = 0;
    PACL pSourceDacl = nullptr;
    DWORD dwSizeNeeded;
    SECURITY_DESCRIPTOR_CONTROL sdControl;
    DWORD dwRevision;
    int parentIndex;
    PSECURITY_DESCRIPTOR pSDOfParent;
    PSID group;
    BOOL bGroupDefaulted = 0;
    PSID owner;
    BOOL bOwnerDefaulted = 0;
    PWSTR stringSD;
    ULONG stringSDLen = 0;
    SECURITY_DESCRIPTOR_CONTROL currentObjectSDControl;

    PSECURITY_DESCRIPTOR absoluteCurrentSD;
    Resource *currentResource = m_resources[m_editingResource];

    hr = ConvertStringToAbsSD(currentResource->GetSD(), &absoluteCurrentSD);
    FailGracefully(hr, L"ConvertStringToAbsSD");
    
    if ( IS_FLAG_SET(si, DACL_SECURITY_INFORMATION) ) 
    {
        bResult = GetSecurityDescriptorDacl(
            pSD,
            &bDaclPresent,
            &pSourceDacl,
            &bDaclDefaulted
            );
        FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");
        
        bResult = GetSecurityDescriptorDacl(
            absoluteCurrentSD,
            &bDaclPresent,
            &pDestDacl,
            &bDaclDefaulted
            );
        FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");
        
        if ( pDestDacl == nullptr )
        {
            // Align sizeNeeded to a DWORD
            dwSizeNeeded = (sizeof(ACL) + (sizeof(DWORD) - 1)) & 0xfffffffc;

            pDestDacl = (PACL)LocalAlloc(LPTR, dwSizeNeeded);
            if ( pDestDacl == nullptr ) 
            {
                wprintf(L"LocalAlloc failed.\n");
                hr = E_OUTOFMEMORY;
                goto exit_gracefully;
            }

            bResult = InitializeAcl(pDestDacl, dwSizeNeeded, ACL_REVISION);
            FailGracefullyGLE(bResult, L"InitializeAcl");
        }

        // Before doing anything else, we need to change the protected 
        // bit in case we end up reenabling inheritance.
        
        // If the 'P' flag was set, e.g. D:PAR(A;CIIO;FA;;;WD)
        // then we need to remove all inherited entries
        bResult = GetSecurityDescriptorControl(
          pSD,
          &sdControl,
          &dwRevision
        );
        FailGracefullyGLE(bResult, L"GetSecurityDescriptorControl");
        
        bResult = GetSecurityDescriptorControl(
            absoluteCurrentSD,
            &currentObjectSDControl,
            &dwRevision
        );
        FailGracefullyGLE(bResult, L"GetSecurityDescriptorControl");
        BOOL currentObjectWasProtected = 
            IS_FLAG_SET(currentObjectSDControl, SE_DACL_PROTECTED)
            ? TRUE
            : FALSE;

        // Now that we've gotten the SE_DACL_PROTECTED bit off of the current 
        // object, we can set it to what it needs to be. We needed to capture 
        // it in the case that we're reenabling inheritance.
        bResult = SetSecurityDescriptorControl(
            absoluteCurrentSD,
            SE_DACL_PROTECTED,
            IS_FLAG_SET(sdControl, SE_DACL_PROTECTED)
                ? SE_DACL_PROTECTED
                : 0
            );
        FailGracefullyGLE(bResult, L"SetSecurityDescriptorControl");

        if ( IS_FLAG_SET(sdControl, SE_DACL_PROTECTED) )
        {
            hr = RemoveAllInheritedAces(
                &pDestDacl
                );
            FailGracefully(hr, L"RemoveAllInheritedAces");
        }
        else
        {
            // The user reenabled inheritance (i.e. didn't pass in 
            // SE_DACL_PROTECTED, but the object used to have that flag).
            if ( currentObjectWasProtected != FALSE )
            {
                // This means we need to call SetSecurityOfChildren on the 
                // parent. This is why we disabled the SE_DACL_PROTECTED flag 
                // already - otherwise the function would exit immediately.
                parentIndex = currentResource->GetParentIndex();
                if ( parentIndex != NONEXISTENT_OBJECT )
                {
                    Resource *parentResource = m_resources[parentIndex];
                    // Because we're keeping SDs as strings, we need to do
                    // a bit of hackery here to set up for SetSecurityOfChildren.

                    // First, save the current security descriptor to where 
                    // SetSecurityOfChildren can pick it up
                    {
                        bResult = ConvertSecurityDescriptorToStringSecurityDescriptor(
                            absoluteCurrentSD,
                            SDDL_REVISION_1,
                            OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION |
                                LABEL_SECURITY_INFORMATION |
                                ATTRIBUTE_SECURITY_INFORMATION |
                                SCOPE_SECURITY_INFORMATION,
                            &stringSD,
                            &stringSDLen
                            );
    
                        FailGracefullyGLE(bResult, L"ConvertSecurityDescriptorToStringSecurityDescriptor");

                        currentResource->FreeSD();
                        currentResource->SetSD(stringSD);
                        stringSD = nullptr;
                    }

                    hr = ConvertStringToAbsSD(
                        parentResource->GetSD(), 
                        &pSDOfParent);
                    FailGracefully(hr, L"ConvertStringToAbsSD");

                    hr = SetSecurityOfChildren(
                        parentIndex, 
                        DACL_SECURITY_INFORMATION, 
                        pSDOfParent);
                    FailGracefully(hr, L"SetSecurityOfChildren");
                    // Now, the current resource's SD will be set to what we want...
                    // so we need to make sure we're working on that by making
                    // absoluteCurrentSD into that.
                    hr = ConvertStringToAbsSD(
                        currentResource->GetSD(), 
                        &absoluteCurrentSD);
                    FailGracefully(hr, L"ConvertStringToAbsSD");

                    // The DACL probably just changed
                    bResult = GetSecurityDescriptorDacl(
                        absoluteCurrentSD,
                        &bDaclPresent,
                        &pDestDacl,
                        &bDaclDefaulted
                        );
                    FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");
                }
            }
        }

        hr = AddAllAcesFromAcl(pSourceDacl, &pDestDacl, TRUE);
        FailGracefully(hr, L"AddAllAcesFromAcl");

        // Finally, remove the explicit ACEs that don't appear in pSD.
        // These are ACEs that used to be on the DACL, but the user just
        // removed.
        hr = RemoveExplicitUniqueAces(
            pSourceDacl,
            &pDestDacl
            );
        FailGracefully(hr, L"RemoveExplicitUniqueAces");

        hr = OrderDacl(
            m_editingResource,
            &pDestDacl
            );
        FailGracefully(hr, L"OrderDacl");
        
        bResult = SetSecurityDescriptorDacl(
            absoluteCurrentSD,
            true,
            pDestDacl,
            bDaclDefaulted
            );
        FailGracefullyGLE(bResult, L"SetSecurityDescriptorDacl");

        pDestDacl = nullptr;
    }
    if ( IS_FLAG_SET(si, GROUP_SECURITY_INFORMATION) ) 
    {
        bResult = GetSecurityDescriptorGroup(
            pSD,
            &group,
            &bGroupDefaulted
            );
        FailGracefullyGLE(bResult, L"GetSecurityDescriptorGroup");
        
        bResult = SetSecurityDescriptorGroup(
            absoluteCurrentSD,
            group,
            bGroupDefaulted
            );
        FailGracefullyGLE(bResult, L"SetSecurityDescriptorGroup");
    }
    if ( IS_FLAG_SET(si, OWNER_SECURITY_INFORMATION) ) 
    {
        bResult = GetSecurityDescriptorOwner(
            pSD,
            &owner,
            &bOwnerDefaulted
            );
        FailGracefullyGLE(bResult, L"GetSecurityDescriptorOwner");

        bResult = SetSecurityDescriptorOwner(
            absoluteCurrentSD,
            owner,
            bOwnerDefaulted
            );
        FailGracefullyGLE(bResult, L"SetSecurityDescriptorOwner");
    }
    
    // Finally, convert whatever changes we made to the absolute SD back to a string
    bResult = ConvertSecurityDescriptorToStringSecurityDescriptor(
        absoluteCurrentSD,
        SDDL_REVISION_1,
        OWNER_SECURITY_INFORMATION |
            GROUP_SECURITY_INFORMATION |
            DACL_SECURITY_INFORMATION |
            LABEL_SECURITY_INFORMATION |
            ATTRIBUTE_SECURITY_INFORMATION |
            SCOPE_SECURITY_INFORMATION,
        &stringSD,
        &stringSDLen
        );
    
    FailGracefullyGLE(bResult, L"ConvertSecurityDescriptorToStringSecurityDescriptor");
    
    currentResource->FreeSD();
    currentResource->SetSD(stringSD);
    stringSD = nullptr;

    if ( currentResource->IsContainer() != FALSE)
    {
        hr = SetSecurityOfChildren(m_editingResource, si, absoluteCurrentSD);
        FailGracefully(hr, L"SetSecurityOfChildren");
    }

exit_gracefully:
    
    LocalFree(pDestDacl);

    return hr;
}

IFACEMETHODIMP CSecInfo::GetAccessRights(
    const GUID* pguidObjectType, 
    DWORD dwFlags, 
    PSI_ACCESS *ppAccess,
    ULONG *pcAccesses, 
    ULONG *piDefaultAccess)
{
    UNREFERENCED_PARAMETER(dwFlags);

    if ( pguidObjectType == nullptr || *pguidObjectType == GUID_NULL )
    {
        *ppAccess        = const_cast<SI_ACCESS *>(m_AccessTable);
        *pcAccesses      = m_AccessTableCount;

        // This is the index of the default access you want when you're 
        // adding a permission. It ends up indexing m_AccessTable, which 
        // is really g_siForumsAccess, so 0 is Full Control.
        *piDefaultAccess = m_DefaultAccess;
    }

    return S_OK;
}

// This function requests that the generic access rights in an access mask
// be mapped to their corresponding standard and specific access rights.
IFACEMETHODIMP CSecInfo::MapGeneric(
    const GUID *pguidObjectType, 
    UCHAR *pAceFlags, 
    ACCESS_MASK *pMask)
{
    if ( !pAceFlags || !pMask )
    {
        return E_INVALIDARG;
    }

    // This sample doesn't include object inheritance, so that bit can be 
    // safely removed.
    *pAceFlags &= ~OBJECT_INHERIT_ACE;
    MapGenericMask(pMask, &ObjectMap);

    UNREFERENCED_PARAMETER(pguidObjectType);
    return S_OK;
}
IFACEMETHODIMP CSecInfo::GetInheritTypes(
    PSI_INHERIT_TYPE *ppInheritTypes, 
    ULONG *pcInheritTypes)
{
    if ( m_resources[m_editingResource]->IsContainer() != FALSE) 
    {
        *ppInheritTypes = siSDKInheritTypes;
        *pcInheritTypes = ARRAYSIZE(siSDKInheritTypes);
        return S_OK;
    }
    
    return E_NOTIMPL;
}

IFACEMETHODIMP CSecInfo::PropertySheetPageCallback(
    HWND hwnd, 
    UINT uMsg, 
    SI_PAGE_TYPE uPage)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(uPage);
    return E_NOTIMPL;
}
