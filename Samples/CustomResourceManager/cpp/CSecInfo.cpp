//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#include "CSecInfo.h"
#include "authz.h"
#include <stdio.h>
#include "sddl.h"
#include <string>
#include "utility.h"
#include <assert.h>

CSecInfo::CSecInfo() :
    m_cRef(0),
    m_AccessTable(g_siForumsAccess),
    m_AccessTableCount(ARRAYSIZE(g_siForumsAccess)),
    m_DefaultAccess(0), // full control by default
    m_defaultSecurityDescriptorSddl(L"O:WDG:BAD:AI(A;CIIO;FA;;;WD)(A;;FA;;;BA)"
        L"S:AI(AU;SAFACIIO;FA;;;WD)"),
    m_editingResource(0),
    m_resources(0),
    m_dwSIFlags(0),
    m_bFailedToConstruct(false)
{
    HRESULT hr = S_OK;
    PWSTR* allSDs;
    PCWSTR noDaclOrSacl = L"O:WDG:BA";
    int parentIndex;
    PSECURITY_DESCRIPTOR pSelfRelativeSD;
    ULONG securityDescriptorSize = 0;
    BOOL bResult;

    m_resources = (PRESOURCE*)LocalAlloc(
        LPTR, 
        NUMBER_OF_RESOURCES * sizeof(PRESOURCE*));
    if ( !m_resources )
    {
        m_bFailedToConstruct = true;
        return;
    }

    allSDs = (PWSTR *)LocalAlloc(LPTR, NUMBER_OF_RESOURCES * sizeof(PWSTR*));
    if ( !allSDs )
    {
        m_bFailedToConstruct = true;
        return;
    }

    for ( int i = 0; i < NUMBER_OF_RESOURCES; i++ )
    {
        hr = AllocAndCopyString(noDaclOrSacl, &allSDs[i]);
        if ( !SUCCEEDED(hr) )
        {
            m_bFailedToConstruct = true;
            return;
        }
    }
    
    m_resources[CONTOSO_FORUMS]     = new Resource(L"Contoso forums",   FORUM,      allSDs[CONTOSO_FORUMS],     NONEXISTENT_OBJECT);
    m_resources[SPORTS]             = new Resource(L"Sports",           SECTION,    allSDs[SPORTS],             CONTOSO_FORUMS);
    m_resources[FAVORITE_TEAM]      = new Resource(L"Favorite team",    TOPIC,      allSDs[FAVORITE_TEAM],      SPORTS);
    m_resources[UPCOMING_EVENTS]    = new Resource(L"Upcoming events",  TOPIC,      allSDs[UPCOMING_EVENTS],    SPORTS);
    m_resources[MOVIES]             = new Resource(L"Movies",           SECTION,    allSDs[MOVIES],             CONTOSO_FORUMS);
    m_resources[NEW_RELEASES]       = new Resource(L"2012 releases",    TOPIC,      allSDs[NEW_RELEASES],       MOVIES);
    m_resources[CLASSICS]           = new Resource(L"Classics",         TOPIC,      allSDs[CLASSICS],           MOVIES);
    m_resources[HOBBIES]            = new Resource(L"Hobbies",          SECTION,    allSDs[HOBBIES],            CONTOSO_FORUMS);
    m_resources[LEARNING_TO_COOK]   = new Resource(L"Learning to cook", TOPIC,      allSDs[LEARNING_TO_COOK],   HOBBIES);
    m_resources[SNOWBOARDING]       = new Resource(L"Snowboarding",     TOPIC,      allSDs[SNOWBOARDING],       HOBBIES);

    // Associate parents with their children
    for ( int i = 0; i < NUMBER_OF_RESOURCES; i++ )
    {
        parentIndex = m_resources[i]->GetParentIndex();
        if ( parentIndex != NONEXISTENT_OBJECT )
        {
            m_resources[parentIndex]->AddChild(i);
        }
    }

    m_objectTypeList.Level = ACCESS_OBJECT_GUID;
    m_objectTypeList.Sbz = 0;
    m_objectTypeList.ObjectType = nullptr;

    // Initialize to a sane value even if we won't be editing the grandparent
    SetCurrentObject(0);

    bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(
        m_defaultSecurityDescriptorSddl,
        SDDL_REVISION_1,
        &pSelfRelativeSD,
        &securityDescriptorSize
        );
    if ( bResult == FALSE )
    { 
        wprintf(L"Error calling ConvertStringSecurityDescriptorToSecurityDescriptor"
            L": %d\n", GetLastError()); 
        m_bFailedToConstruct = true;
        return;
    } 

    // Call SetSecurity on the forums root so that 
    // everything gets an inherited DACL.
    hr = SetSecurity(DACL_SECURITY_INFORMATION, pSelfRelativeSD);
    LocalFree(pSelfRelativeSD);
    if ( !SUCCEEDED(hr) )
    {
        wprintf(L"Error calling SetSecurity: %d\n", GetLastError()); 
        m_bFailedToConstruct = true;
        return;
    }
}

CSecInfo::~CSecInfo()
{
    if ( m_resources != nullptr )
    {
        for ( int i = 0; i < NUMBER_OF_RESOURCES; i++ )
        {
            delete m_resources[i];
        }
        LocalFree(m_resources);
    }

    assert(m_cRef == 0);
}

/* IUnknown */
IFACEMETHODIMP_(ULONG)
CSecInfo::AddRef(void)
{
    return ::InterlockedIncrement(&m_cRef);
}

/* IUnknown */
IFACEMETHODIMP_(ULONG)
CSecInfo::Release(void)
{
    return ::InterlockedDecrement(&m_cRef);
}

/* IUnknown */
IFACEMETHODIMP
CSecInfo::QueryInterface(_In_ REFIID riid, _Outptr_ void ** ppv)
{
    if ( !ARGUMENT_PRESENT(ppv) )
    {
        return E_INVALIDARG;
    }

    if (::IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = this;
    }
    else if(::IsEqualIID(riid, IID_ISecurityInformation))
    {
        *ppv = static_cast<ISecurityInformation *>(this);
    }
    else if(::IsEqualIID(riid, IID_ISecurityInformation3))
    {
        *ppv = static_cast<ISecurityInformation3 *>(this);
    }
    else if( ::IsEqualIID(riid, IID_IEffectivePermission2))
    {
        *ppv = static_cast<IEffectivePermission2 *>(this);
    }
    else if( ::IsEqualIID(riid, IID_ISecurityObjectTypeInfo))
    {
        *ppv = static_cast<ISecurityObjectTypeInfo *>(this);
    }
    else
    {
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

HRESULT CSecInfo::ConvertStringToAbsSD(
    _In_ PWSTR stringSD, 
    _Outptr_ PSECURITY_DESCRIPTOR *ppSD)
{
    BOOL bResult = 0;
    DWORD errorCode = 0;
    HRESULT hr = S_OK;
    PSECURITY_DESCRIPTOR pSelfRelativeSD;
    ULONG securityDescriptorSize = 0;
    
    // Start by getting an SD from our string
    bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(
        stringSD,
        SDDL_REVISION_1,
        &pSelfRelativeSD,
        &securityDescriptorSize
        );
    FailGracefullyGLE(
        bResult, 
        L"ConvertStringSecurityDescriptorToSecurityDescriptor");
    
    hr = ConvertSecurityDescriptor(pSelfRelativeSD, ppSD);
exit_gracefully:
    return hr;
}

HRESULT CSecInfo::OrderDacl(int childIndex, PACL *ppAcl)
{
    BOOL bResult;
    HRESULT hr = S_OK;
    DWORD errorCode = S_OK;
    Resource *resourceToOrder;
    PINHERITED_FROM pInheritArray;
    PACL pOrderedAcl = nullptr;
    ACL_SIZE_INFORMATION aclInformation;
    ACL_SIZE_INFORMATION aclInformation2;
    DWORD totalCount;
    DWORD totalCount2;
    DWORD dwSizeNeeded;
    int parentIndex;
    int grandparentIndex;
    int numPasses;
    LPVOID ace;

    if ( childIndex == 0 )
    {
        // The base object (forums) can only have explicit ACEs since there's
        // nothing above it (GetInheritSource would fail for the forums 
        // anyway because we return not impl)
        return S_OK;
    }

    if ( !ARGUMENT_PRESENT(*ppAcl) )
    {
        return E_INVALIDARG;
    }

    resourceToOrder = m_resources[childIndex];

    // This call has to be made on the child, not the thing we're currently editing
    hr = GetInheritSourceHelper(
        childIndex, 
        DACL_SECURITY_INFORMATION, 
        *ppAcl, 
        &pInheritArray);
    FailGracefully(hr, L"GetInheritSourceHelper");

    bResult = GetAclInformation(
        *ppAcl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");

    totalCount = aclInformation.AceCount;
    dwSizeNeeded = aclInformation.AclBytesFree + aclInformation.AclBytesInUse;
    pOrderedAcl = (PACL)LocalAlloc(LPTR, dwSizeNeeded);
    if ( !pOrderedAcl )
    {
        wprintf(L"LocalAlloc failed.\n");
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }

    bResult = InitializeAcl(
        pOrderedAcl,
        dwSizeNeeded,
        ACL_REVISION
        );
    FailGracefullyGLE(bResult, L"InitializeAcl");

    parentIndex = resourceToOrder->GetParentIndex();
    grandparentIndex = 
        parentIndex == NONEXISTENT_OBJECT 
            ? NONEXISTENT_OBJECT 
            : m_resources[parentIndex]->GetParentIndex();

    // Do two passes for each set of ACEs: explicit, parent, grandparent.
    // One pass is for deny ACEs, one is for allow.
    // This gives us a total of 6 passes for TOPICs (because they have a 
    // grandparent), 4 passes for SECTIONs (because they only have a
    // parent), and 2 passes for FORUMs.
    numPasses = 2;
    if ( parentIndex > -1 ) numPasses += 2;
    if ( grandparentIndex > -1 ) numPasses += 2;

    for (int pass = 0; pass < numPasses; pass ++ )
    {
        for ( DWORD aceIndex = 0; aceIndex < totalCount; aceIndex ++ )
        {
            bResult = GetAce(
                *ppAcl,
                aceIndex,
                &ace
                );
            FailGracefullyGLE(bResult, L"GetAce");
                        
            BYTE aceType = ((PACE_HEADER)ace)->AceType;
            if (
                // Pass 0: explicit deny ACEs
                ( pass == 0 && 
                pInheritArray[aceIndex].GenerationGap == 0 && 
                aceType == ACCESS_DENIED_ACE_TYPE ) ||

                // Pass 1: explicit allow ACEs
                ( pass == 1 && 
                pInheritArray[aceIndex].GenerationGap == 0 && 
                IsAccessAllowedAce(aceType) ) ||
                
                // Pass 2: inherited-from-parent deny ACEs
                ( pass == 2 && 
                pInheritArray[aceIndex].GenerationGap == 1 && 
                aceType == ACCESS_DENIED_ACE_TYPE ) ||

                // Pass 3: inherited-from-parent allow ACEs
                ( pass == 3 && 
                pInheritArray[aceIndex].GenerationGap == 1 && 
                IsAccessAllowedAce(aceType) ) ||
                
                // Pass 3: inherited-from-grandparent deny ACEs
                ( pass == 4 && 
                pInheritArray[aceIndex].GenerationGap == 2 && 
                aceType == ACCESS_DENIED_ACE_TYPE ) ||
                
                // Pass 3: inherited-from-grandparent allow ACEs
                ( pass == 5 && 
                pInheritArray[aceIndex].GenerationGap == 2 && 
                IsAccessAllowedAce(aceType) )
                )
            {
                // We COULD just use AddAce, because we're guaranteed not to 
                // overflow the ACL's size (since we allocated it to exactly 
                // the same size and we're going to add all of the ACEs)
                hr = AddAceToAcl(ace, &pOrderedAcl, true);
                FailGracefully(hr, L"AddAceToAcl");
            }
        }
    }

    bResult = GetAclInformation(
        *ppAcl,
        &aclInformation2,
        sizeof(aclInformation2),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");

    totalCount2 = aclInformation2.AceCount;

    // Sanity check: ensure that we didn't leave out any ACEs
    if ( totalCount != totalCount2 )
    {
        wprintf(L"A different amount of ACEs exists in the ACL now. "
            L"Before: %d   after: %d\n", totalCount, totalCount2);
        hr = E_FAIL;
        goto exit_gracefully;
    }

    // Sanity check: ensure that the ACLs are the same size
    if ( aclInformation.AclBytesFree != aclInformation2.AclBytesFree || 
        aclInformation.AclBytesInUse != aclInformation2.AclBytesInUse )
    {
        wprintf(L"Either AclBytesFree or AclBytesInUse doesn't match up\n");
        hr = E_FAIL;
        goto exit_gracefully;
    }

    // Now point the unordered ACL to the ordered one
    LocalFree(*ppAcl);
    *ppAcl = pOrderedAcl;
    pOrderedAcl = nullptr;

exit_gracefully:
    LocalFree(pOrderedAcl);
    return hr;
}

// pSD represents the security descriptor of the parent
HRESULT CSecInfo::SetSecurityOfChildren(
    int parentIndex, 
    SECURITY_INFORMATION si, 
    PSECURITY_DESCRIPTOR pSD)
{
    BOOL bResult = 0;
    DWORD errorCode = 0;
    HRESULT hr = S_OK;
    BOOL bAclPresent = 0;
    PACL acl = nullptr;
    PACL pChildAcl = nullptr;
    BOOL bAclDefaulted = 0;
    Resource *parentResource = m_resources[parentIndex];
    std::list<int> childIndices = parentResource->GetChildIndices();
    Resource *childResource;
    PSECURITY_DESCRIPTOR childAbsoluteSD;
    DWORD dwRevision;
    SECURITY_DESCRIPTOR_CONTROL sdControl;
    DWORD dwSizeNeeded = 0;
    int childIndex;
    PWSTR stringSD;
    ULONG stringSDLen = 0;
    std::list<int>::iterator it;

    for(it = childIndices.begin(); it != childIndices.end(); ++it)
    {
        childIndex = *it;
        childResource = m_resources[childIndex];

        hr = ConvertStringToAbsSD(childResource->GetSD(), &childAbsoluteSD);
        FailGracefully(hr, L"ConvertStringToAbsSD");

        if ( IS_FLAG_SET(si, DACL_SECURITY_INFORMATION) )
        {
    
            // First of all, if the child has the protected bit set,
            // then they aren't interested in inheriting anything.
            bResult = GetSecurityDescriptorControl(
                childAbsoluteSD,
                &sdControl,
                &dwRevision
            );
            FailGracefullyGLE(bResult, L"GetSecurityDescriptorControl");

            // No need to set the security on the children if it's a protected DACL
            if ( IS_FLAG_SET(sdControl, SE_DACL_PROTECTED) )
            {
                return S_OK;
            }

            // Get the DACL of the parent
            bResult = GetSecurityDescriptorDacl(
                pSD,
                &bAclPresent,
                &acl,
                &bAclDefaulted
                );
            FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");

            // If there was no supplied ACL, then there's nothing 
            // for the child to inherit
            if ( !bAclPresent )
            {
                return S_OK;
            }

            // Now we know there's a DACL, but we don't know if there are
            // any inheritable ACEs. 
            hr = GetSizeOfAllInheritableAces(acl, dwSizeNeeded);
            FailGracefully(hr, L"GetSizeOfAllInheritableAces");
    
            // At this point, we know that the parent has a DACL.
            // We need to get the child's DACL too because 
            // we may need to delete or add entries
            bResult = GetSecurityDescriptorDacl(
                childAbsoluteSD,
                &bAclPresent,
                &pChildAcl,
                &bAclDefaulted
                );
            FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");

            if ( pChildAcl != nullptr )
            {
                // Keep only the explicit ACEs. This way we don't need to do a 
                // differential thing and find out which inherited ACEs went 
                // away or which ones were added. We just wipe them all out 
                // and then add all the inheritable ACEs from the parent.
                hr = RemoveAllInheritedAces(&pChildAcl);
                FailGracefully(hr, L"RemoveAllInheritedAces");
            }

            // There are no inheritable ACEs, so we're done here.
            if ( dwSizeNeeded == 0 )
            {
                return S_OK;
            }
    
            // At this point, we know that there is a parent DACL
            // and that it contains inheritable entries. If the child's ACL
            // is null, then we need to initialize it.
            if ( pChildAcl == nullptr ) 
            {
                // We know how much space we need, so we can allocate it.
                // First though, align it to a DWORD (this is necessary)
                dwSizeNeeded = (dwSizeNeeded + (sizeof(DWORD) - 1)) & 0xfffffffc;

                pChildAcl = (PACL)LocalAlloc(LPTR, dwSizeNeeded);
                if ( pChildAcl == nullptr ) 
                {
                    wprintf(L"LocalAlloc failed.\n");
                    hr = E_OUTOFMEMORY;
                    goto exit_gracefully;
                }

                bResult = InitializeAcl(pChildAcl, dwSizeNeeded, ACL_REVISION);
                FailGracefullyGLE(bResult, L"InitializeAcl");
            }

            hr = AddInheritableAcesFromAcl(acl, &pChildAcl);
            FailGracefully(hr, L"AddInheritableAcesFromAcl");

            // Now, the ACL is fully formed, so set it on the child
            hr = OrderDacl(
                childIndex,
                &pChildAcl
                );
            FailGracefully(hr, L"OrderDacl");

            bResult = SetSecurityDescriptorDacl(
                childAbsoluteSD,
                true,
                pChildAcl,
                false
                );
            FailGracefullyGLE(bResult, L"SetSecurityDescriptorDacl");

            bResult = SetSecurityDescriptorControl(
                childAbsoluteSD,
                SE_DACL_AUTO_INHERITED,
                SE_DACL_AUTO_INHERITED
                );
            FailGracefullyGLE(bResult, L"SetSecurityDescriptorControl");
        }

        // Finally, convert the SD back to a string
        bResult = ConvertSecurityDescriptorToStringSecurityDescriptor(
            childAbsoluteSD,
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
        FailGracefullyGLE(
            bResult, 
            L"ConvertSecurityDescriptorToStringSecurityDescriptor");
    
        childResource->FreeSD();
        childResource->SetSD(stringSD);
        stringSD = nullptr;

        // Now, call SetSecurityOfChildren on the child
        hr = SetSecurityOfChildren(childIndex, si, childAbsoluteSD);
        FailGracefully(hr, L"SetSecurityOfChildren");
    }
exit_gracefully:
    return hr;
}

// ISecurityInformation3
IFACEMETHODIMP 
CSecInfo::GetFullResourceName(_Outptr_ LPWSTR *ppszResourceName)
{
    *ppszResourceName = m_resources[m_editingResource]->GetName();

    return S_OK;
}
IFACEMETHODIMP 
CSecInfo::OpenElevatedEditor(_In_ HWND hWnd, _In_ SI_PAGE_TYPE uPage)
{
    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(uPage);
    return E_NOTIMPL;
}

// IEffectivePermission2
IFACEMETHODIMP CSecInfo::ComputeEffectivePermissionWithSecondarySecurity (THIS_
    _In_ PSID pSid,
    _In_opt_ PSID pDeviceSid,
    _In_ PCWSTR pszServerName,
    _Inout_updates_(dwSecurityObjectCount) PSECURITY_OBJECT pSecurityObjects,
    _In_ DWORD dwSecurityObjectCount,
    _In_opt_ PTOKEN_GROUPS pUserGroups,
    _When_(pUserGroups != nullptr && *pAuthzUserGroupsOperations != AUTHZ_SID_OPERATION_REPLACE_ALL, _In_reads_(pUserGroups->GroupCount))
    _In_opt_ PAUTHZ_SID_OPERATION pAuthzUserGroupsOperations,
    _In_opt_ PTOKEN_GROUPS pDeviceGroups,
    _When_(pDeviceGroups != nullptr && *pAuthzDeviceGroupsOperations != AUTHZ_SID_OPERATION_REPLACE_ALL, _In_reads_(pDeviceGroups->GroupCount))
    _In_opt_ PAUTHZ_SID_OPERATION pAuthzDeviceGroupsOperations,
    _In_opt_ PAUTHZ_SECURITY_ATTRIBUTES_INFORMATION pAuthzUserClaims,
    _When_(pAuthzUserClaims != nullptr && *pAuthzUserClaimsOperations != AUTHZ_SECURITY_ATTRIBUTE_OPERATION_REPLACE_ALL, _In_reads_(pAuthzUserClaims->AttributeCount))
    _In_opt_ PAUTHZ_SECURITY_ATTRIBUTE_OPERATION pAuthzUserClaimsOperations,
    _In_opt_ PAUTHZ_SECURITY_ATTRIBUTES_INFORMATION pAuthzDeviceClaims,
    _When_(pAuthzDeviceClaims != nullptr && *pAuthzDeviceClaimsOperations != AUTHZ_SECURITY_ATTRIBUTE_OPERATION_REPLACE_ALL, _In_reads_(pAuthzDeviceClaims->AttributeCount))
    _In_opt_ PAUTHZ_SECURITY_ATTRIBUTE_OPERATION pAuthzDeviceClaimsOperations,
    _Inout_updates_(dwSecurityObjectCount) PEFFPERM_RESULT_LIST pEffpermResultLists)
{
    // The server name passed in should always be null
    UNREFERENCED_PARAMETER(pszServerName); 

    BOOL bResult;
    DWORD errorCode = S_OK;
    HRESULT hr = S_OK;
    DWORD dwFlags = 0;

    // AuthZ context representing the client.
    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzUserContext = nullptr;
    
    // AuthZ context representing the device.
    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzDeviceContext = nullptr;
    
    // AuthZ context representing the combination of client and device. If no
    // device SID is passed in to this function, then it only represents
    // the user context.
    AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzCompoundContext = nullptr;

    // Access request specifies the desired access mask, principal self sid, 
    // the object type list strucutre (if any).
    AUTHZ_ACCESS_REQUEST request;
    AUTHZ_AUDIT_EVENT_HANDLE hAuditEvent = nullptr;
    PSECURITY_DESCRIPTOR pSD;
    PSECURITY_DESCRIPTOR *pOptionalSecurityDescriptorArray = nullptr;
    DWORD dwOptionalSecurityDescriptorCount = 0;
    AUTHZ_ACCESS_REPLY reply;
    reply.Error = nullptr;
    reply.GrantedAccessMask = nullptr;
    bool needToFreeAccMask = true;
    PAUTHZ_ACCESS_CHECK_RESULTS_HANDLE phAccessCheckResults = nullptr;
    AUTHZ_RESOURCE_MANAGER_HANDLE hAuthzResourceManager = nullptr;

    bool madeResourceManager = false;
    LUID identifier;

    // There is no concept of shares or CAPs in this resource manager,
    // so the only security object in pSecurityObjects will be
    // the SD in question. The following checks aren't necessary when
    // considering the sample code.
    if ( dwSecurityObjectCount != 1 )
    {
        wprintf(L"Unexpected effective permissions argument data: "
            L"dwSecurityObjectCount==%d\n", dwSecurityObjectCount);
        hr = E_FAIL;
        goto exit_gracefully;
    }

    if ( pSecurityObjects[0].Id != SECURITY_OBJECT_ID_OBJECT_SD )
    {
        wprintf(L"Unexpected pSecurityObjects[0].Id: %d\n", pSecurityObjects[0].Id);
        hr = E_FAIL;
        goto exit_gracefully;
    }

    if ( !ARGUMENT_PRESENT(pSid) )
    {
        return E_INVALIDARG;
    }

    bResult = AuthzInitializeResourceManager(
            AUTHZ_RM_FLAG_NO_AUDIT,
            nullptr,
            nullptr,
            nullptr,
            L"SDK Sample Resource Manager",
            &hAuthzResourceManager
            );
    FailGracefullyGLE(bResult, L"AuthzInitializeResourceManager");
    madeResourceManager = true;

    // This is never interpreted by AuthZ
    identifier.HighPart = 0;
    identifier.LowPart = 0;

    bResult = AuthzInitializeContextFromSid(
        0,
        pSid,       // use the SID passed in to this function
        hAuthzResourceManager,
        nullptr,    // token will never expire (this isn't enforced anyway)
        identifier, // never interpreted by authz
        nullptr,
        &hAuthzUserContext
        );
    FailGracefullyGLE(bResult, L"AuthzInitializeContextFromSid");

    // Set up the different contexts
    if ( pDeviceSid != nullptr )
    {
        bResult = AuthzInitializeContextFromSid(
            0,
            pDeviceSid, // use the device SID passed in to this function
            hAuthzResourceManager,
            nullptr, // token will never expire (this isn't enforced anyway)
            identifier, // never interpreted by authz
            nullptr,
            &hAuthzDeviceContext
            );
        
        FailGracefullyGLE(bResult, L"AuthzInitializeContextFromSid (device)");

        bResult = AuthzInitializeCompoundContext(
            hAuthzUserContext,
            hAuthzDeviceContext,
            &hAuthzCompoundContext
            );
        FailGracefullyGLE(bResult, L"AuthzInitializeCompoundContext");

        // Add device claims
        if ( pAuthzDeviceClaims != nullptr )
        {
            bResult = AuthzModifyClaims(
                hAuthzCompoundContext,
                AuthzContextInfoDeviceClaims,
                pAuthzDeviceClaimsOperations,
                pAuthzDeviceClaims
                );
            FailGracefullyGLE(bResult, L"AuthzModifyClaims (device claims)");
        }
    }
    else
    {
        hAuthzCompoundContext = hAuthzUserContext;
    }
    
    // Add user claims
    if ( pAuthzUserClaims != nullptr )
    {
        bResult = AuthzModifyClaims(
            hAuthzCompoundContext,
            AuthzContextInfoUserClaims,
            pAuthzUserClaimsOperations,
            pAuthzUserClaims
            );
        FailGracefullyGLE(bResult, L"AuthzModifyClaims (user claims)");
    }

    // Add "what-if" device groups
    if ( pDeviceGroups != nullptr )
    {
        bResult = AuthzModifySids(
            hAuthzCompoundContext,
            AuthzContextInfoDeviceSids,
            pAuthzDeviceGroupsOperations,
            pDeviceGroups
            );
        FailGracefullyGLE(bResult, L"AuthzModifySids (device groups)");
    }

    // Add "what-if" user groups
    if ( pUserGroups != nullptr && pAuthzUserGroupsOperations != nullptr )
    {
        bResult = AuthzModifySids(
            hAuthzCompoundContext,
            AuthzContextInfoGroupsSids,
            pAuthzUserGroupsOperations,
            pUserGroups
            );
        FailGracefullyGLE(bResult, L"AuthzModifySids (user groups)");
    }

    pSD = (PSECURITY_DESCRIPTOR)pSecurityObjects[0].pData;

    request.DesiredAccess = MAXIMUM_ALLOWED;
    request.PrincipalSelfSid = nullptr;
    request.ObjectTypeList = nullptr;
    request.ObjectTypeListLength = 0;
    request.OptionalArguments = nullptr;

    reply.ResultListLength = 1;
    reply.SaclEvaluationResults = nullptr;
    reply.GrantedAccessMask = (PACCESS_MASK)LocalAlloc(
        LPTR, 
        sizeof(ACCESS_MASK) * reply.ResultListLength);
    if ( !reply.GrantedAccessMask ) 
    {
        wprintf(L"LocalAlloc failed.\n");
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }
    reply.Error = (PDWORD)LocalAlloc(
        LPTR, 
        sizeof(DWORD) * reply.ResultListLength);
    if ( !reply.Error ) 
    {
        wprintf(L"LocalAlloc failed.\n");
        hr = E_OUTOFMEMORY;
        goto exit_gracefully;
    }

    // Finally, call access check. This is the heart of this function.
    bResult = AuthzAccessCheck(
        dwFlags, // deep copy the SD (default)
        hAuthzCompoundContext,
        &request,
        hAuditEvent,
        pSD,
        pOptionalSecurityDescriptorArray,
        dwOptionalSecurityDescriptorCount,
        &reply,
        phAccessCheckResults
        );

    FailGracefullyGLE(bResult, L"AuthzAccessCheck");
    needToFreeAccMask = false;

    // Only one security object is passed into this function
    // (the object's SD), and we ensured that at the beginning.
    pEffpermResultLists[0].fEvaluated = true;
    pEffpermResultLists[0].pGrantedAccessList = reply.GrantedAccessMask;
    
    // We don't support object ACEs, so cObjectTypeListLength has to be 1
    // pObjectTypeList has to be the null GUID, and pGrantedAccessList 
    // should be a single DWORD
    pEffpermResultLists[0].cObjectTypeListLength = 1;

    pEffpermResultLists[0].pObjectTypeList = &m_objectTypeList;

exit_gracefully:

    if ( madeResourceManager ) 
    {
        AuthzFreeResourceManager(hAuthzResourceManager);
    }

    if ( phAccessCheckResults != nullptr )
    {
        AuthzFreeHandle(*phAccessCheckResults);
        phAccessCheckResults = nullptr;
    }
    
    if (hAuthzUserContext != nullptr)
    {
        AuthzFreeContext(hAuthzUserContext);
        hAuthzUserContext = nullptr;
    }

    if (hAuthzDeviceContext != nullptr)
    {
        AuthzFreeContext(hAuthzDeviceContext);
        hAuthzDeviceContext = nullptr;

        if (hAuthzCompoundContext != nullptr)
        {
            AuthzFreeContext(hAuthzCompoundContext);
            hAuthzCompoundContext = nullptr;
        }
    }

    LocalFree(reply.Error);

    if ( needToFreeAccMask )
    {
        LocalFree(reply.GrantedAccessMask);
    }

    return hr;
}

// ISecurityObjectTypeInfo
STDMETHODIMP
CSecInfo::GetInheritSource(
    SECURITY_INFORMATION si, 
    PACL acl, 
    PINHERITED_FROM *ppInheritArray)
{
    return GetInheritSourceHelper(m_editingResource, si, acl, ppInheritArray);
}

HRESULT CSecInfo::GetInheritSourceHelper(
    int childIndex, 
    SECURITY_INFORMATION si, 
    PACL acl, 
    PINHERITED_FROM *ppInheritArray)
{
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;
    DWORD errorCode = S_OK;
    ACL_SIZE_INFORMATION aclInformation;
    Resource *childResource = m_resources[childIndex];
    Resource *parentResource = nullptr;
    int indexOfParent = childResource->GetParentIndex();
    DWORD totalCount;
    LPVOID ace;
    BYTE aceFlags;
    BOOL alreadyExists = FALSE;
    PSECURITY_DESCRIPTOR pParentSD;
    int grandparentIndex;
    PSECURITY_DESCRIPTOR pGrandparentSD;
    Resource *grandparentResource;
    ULONG defaultSecurityDescriptorSize = 0;
    BOOL bDaclPresent = 0;
    BOOL bDaclDefaulted = 0;
    PACL pGrandparentDacl = nullptr;
    PACL pParentDacl = nullptr;

    if ( !ARGUMENT_PRESENT(acl) )
    {
        return E_INVALIDARG;
    }

    // If there's no parent, just return E_NOTIMPL
    if ( indexOfParent == NONEXISTENT_OBJECT )
    {
        return E_NOTIMPL;
    }

    *ppInheritArray = (PINHERITED_FROM)LocalAlloc(
        LPTR, 
        acl->AceCount * sizeof(INHERITED_FROM) );
    if ( !(*ppInheritArray) )
    {
        return E_OUTOFMEMORY;
    }
    
    // Iterate over all of the ACEs.
    bResult = GetAclInformation(
        acl,
        &aclInformation,
        sizeof(aclInformation),
        AclSizeInformation
        );
    FailGracefullyGLE(bResult, L"GetAclInformation");

    totalCount = aclInformation.AceCount;

    for ( DWORD aceIndex = 0; aceIndex < totalCount; aceIndex ++ )
    {
        bResult = GetAce(
            acl,
            aceIndex,
            &ace
            );
        FailGracefullyGLE(bResult, L"GetAce");
                        
        aceFlags = ((PACE_HEADER)ace)->AceFlags;

        if ( indexOfParent != NONEXISTENT_OBJECT )
        {
            parentResource = m_resources[indexOfParent];
        }

        // If we're a SECTION, then it could only have come from the FORUM 
        // (assuming it wasn't orphaned). If we're a TOPIC, then it could 
        // either be the parent SECTION or the grandparent FORUM.
        if (IS_FLAG_SET(aceFlags,INHERITED_ACE)) 
        {
            if ( childResource->GetType() == SECTION )
            {
                // can't just assume that this ACE will be on the parent...
                // need to check it; it may be an orphan
                bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(
                    parentResource->GetSD(),
                    SDDL_REVISION_1,
                    &pParentSD,
                    &defaultSecurityDescriptorSize);
                FailGracefullyGLE(
                    bResult, 
                    L"ConvertStringSecurityDescriptorToSecurityDescriptor");

                alreadyExists = FALSE;
                
                if ( IS_FLAG_SET(si, DACL_SECURITY_INFORMATION) )
                {
                    bResult = GetSecurityDescriptorDacl(
                        pParentSD,
                        &bDaclPresent,
                        &pParentDacl,
                        &bDaclDefaulted
                        );
                    FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");

                    hr = ACEAlreadyInACL(
                        pParentDacl,
                        ace,
                        &alreadyExists,
                        true
                        );
                    FailGracefully(hr, L"ACEAlreadyInACL");
                }
                
                if ( alreadyExists != FALSE )
                {
                    (*ppInheritArray)[aceIndex].GenerationGap = 1;
                    (*ppInheritArray)[aceIndex].AncestorName = parentResource->GetName();
                }
                else
                {
                    (*ppInheritArray)[aceIndex].GenerationGap = 0;
                    (*ppInheritArray)[aceIndex].AncestorName = nullptr;
                }
            }
            else
            {
                // We're a TOPIC. Check to see if this ACE is in the parent.
                // If it isn't, then we check the grandparent.
                // We can't skip that check because it may be possible that
                // this ACE was orphaned, so it may not exist on any ancestor
                grandparentIndex = parentResource->GetParentIndex();
                PSECURITY_DESCRIPTOR pParentSD;
                ULONG defaultSecurityDescriptorSize = 0;
                bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(
                    parentResource->GetSD(),
                    SDDL_REVISION_1,
                    &pParentSD,
                    &defaultSecurityDescriptorSize);
                FailGracefullyGLE(
                    bResult, 
                    L"ConvertStringSecurityDescriptorToSecurityDescriptor");

                if ( IS_FLAG_SET(si, DACL_SECURITY_INFORMATION) )
                {
                    bResult = GetSecurityDescriptorDacl(
                        pParentSD,
                        &bDaclPresent,
                        &pParentDacl,
                        &bDaclDefaulted
                        );
                    FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");

                    hr = ACEAlreadyInACL(
                        pParentDacl,
                        ace,
                        &alreadyExists,
                        true
                        );
                    FailGracefully(hr, L"ACEAlreadyInACL");
                }
                

                if ( alreadyExists != FALSE )
                {
                    (*ppInheritArray)[aceIndex].GenerationGap = 1;
                    (*ppInheritArray)[aceIndex].AncestorName = parentResource->GetName();
                }
                else
                {
                    // The parent didn't have it, so check the grandparent.
                    grandparentResource = m_resources[grandparentIndex];
                    defaultSecurityDescriptorSize = 0;
                    bResult = ConvertStringSecurityDescriptorToSecurityDescriptor(
                        grandparentResource->GetSD(),
                        SDDL_REVISION_1,
                        &pGrandparentSD,
                        &defaultSecurityDescriptorSize);
                    FailGracefullyGLE(
                        bResult, 
                        L"ConvertStringSecurityDescriptorToSecurityDescriptor");

                    alreadyExists = FALSE;
                    if ( IS_FLAG_SET(si, DACL_SECURITY_INFORMATION) )
                    {
                        bResult = GetSecurityDescriptorDacl(
                            pGrandparentSD,
                            &bDaclPresent,
                            &pGrandparentDacl,
                            &bDaclDefaulted
                            );
                        FailGracefullyGLE(bResult, L"GetSecurityDescriptorDacl");

                        hr = ACEAlreadyInACL(
                            pGrandparentDacl,
                            ace,
                            &alreadyExists,
                            true
                            );
                    }

                    if ( alreadyExists != FALSE )
                    {
                        // It came from the grandparent
                        (*ppInheritArray)[aceIndex].GenerationGap = 2;
                        (*ppInheritArray)[aceIndex].AncestorName = grandparentResource->GetName();
                    }
                    else
                    {
                        // This ACE did not come from a [grand]parent
                        (*ppInheritArray)[aceIndex].GenerationGap = 0;
                        (*ppInheritArray)[aceIndex].AncestorName = nullptr;
                    }
                }

            }
        }
        else
        {
            // This ACE did not come from a [grand]parent
            (*ppInheritArray)[aceIndex].GenerationGap = 0;
            (*ppInheritArray)[aceIndex].AncestorName = nullptr;
        }
    }
exit_gracefully:

    return hr;
}
