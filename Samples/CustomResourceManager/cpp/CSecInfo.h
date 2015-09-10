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
// This file contains the declarations for the CSecInfo class along
// with some helpful #defines and objects.
//
#ifndef _CSECINFO_H_
#define _CSECINFO_H_
#include <aclui.h>
#include <aclapi.h>
#include <windows.h>
#include "resource.h"

#define IS_FLAG_SET(dw,fl)  (((dw) & (fl)) == fl)

// Each individual permission for our resource manager
#define CREATE_PERM         0x0001
#define READ_PERM           0x0002
#define VOTE_PERM           0x0004
#define UPDATE_OWN_PERM     0x0008
#define UPDATE_OTHERS_PERM  0x0010
#define HIDE_PERM           0x0020
#define SHOW_PERM           0x0040
#define LOCK_PERM           0x0080
#define UNLOCK_PERM         0x0100
#define DESTROY_PERM        0x0200
#define VIEW_PERMS_PERM     0x0400
#define CHANGE_PERMS_PERM   0x0800

// Each tier of permissions builds upon the last, but they don't have to.
#define GENERIC_POST_PERM   (\
    CREATE_PERM             |\
    READ_PERM               |\
    VOTE_PERM               |\
    UPDATE_OWN_PERM)
#define GENERIC_MOD_PERM    (\
    GENERIC_POST_PERM       |\
    UPDATE_OTHERS_PERM      |\
    HIDE_PERM               |\
    SHOW_PERM               |\
    LOCK_PERM               |\
    UNLOCK_PERM)
#define GENERIC_ADMIN_PERM  (\
    GENERIC_MOD_PERM        |\
    DESTROY_PERM            |\
    VIEW_PERMS_PERM         |\
    CHANGE_PERMS_PERM)

#define INHERIT_FULL        (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)

//  Define the generic mapping array.  This is used to denote the
//  mapping of each generic access right to a specific access mask.
//  This is used on the basic ACL Editor page.
static GENERIC_MAPPING ObjectMap =
{
    GENERIC_POST_PERM,
    GENERIC_MOD_PERM,
    GENERIC_ADMIN_PERM,
};

// ObjectInherit - applies to parent and only child OBJECTS (e.g. files)
// ContainerInherit - applies to parent and only child CONTAINERS (e.g. folders)
// InheritOnly - doesn't apply to parent, must be combined with something else
// NoPropagateInherit - only applies to child, not grandchildren
//
// Note: I only use container inheritance
static SI_INHERIT_TYPE siSDKInheritTypes[] =  
{  
    &GUID_NULL, 0,                                        (L"This object only"),  
    &GUID_NULL, CONTAINER_INHERIT_ACE,                    (L"This object and children (sections/topics)"),  
    &GUID_NULL, INHERIT_ONLY_ACE | CONTAINER_INHERIT_ACE, (L"Children (sections/topics) only"),  
};  

class CSecInfo : public ISecurityInformation, public ISecurityInformation3, public IEffectivePermission2, public ISecurityObjectTypeInfo
{
private:
    unsigned long m_cRef;
    const SI_ACCESS*  m_AccessTable;
    ULONG             m_AccessTableCount;
    ULONG             m_DefaultAccess;
    PCWSTR            m_defaultSecurityDescriptorSddl;
    OBJECT_TYPE_LIST  m_objectTypeList;

    // This represents the index (see resource.h's ResourceIndices)
    // of the resource that we're currently editing.
    int m_editingResource;

    // This points to all of the resources that the sample keeps track of.
    // The constructor sets these up.
    PRESOURCE*        m_resources;

    // Tell ACL UI what to show
    DWORD m_dwSIFlags;
    
    // This function iterates over a container's children and sets their security.
    //  parentIndex represents the index of the parent (see ResourceIndices)
    //  si can include either DACL_SECURITY_INFORMATION, SACL_SECURITY_INFORMATION, or both
    //  pSD is the security descriptor of the parent
    HRESULT SetSecurityOfChildren(
        int parentIndex, 
        THIS_ SECURITY_INFORMATION si, 
        PSECURITY_DESCRIPTOR pSD);

    // Helper function for GetInheritSource so that we can call it on specific children
    // and not just the object that we're currently editing.
    //  childIndex represents the index of the child (see ResourceIndices)
    HRESULT GetInheritSourceHelper(
        int childIndex, 
        SECURITY_INFORMATION psi, 
        PACL acl, 
PINHERITED_FROM *inheritArray);

    // This orders a DACL canonically. For more information, see:
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa379298(v=vs.85).aspx
    HRESULT OrderDacl(int childIndex, PACL *acl);

    // This function takes a string representing a security descriptor,
    // converts it to a self-relative SD, then finally makes it absolute.
    HRESULT ConvertStringToAbsSD(_In_ PWSTR stringSD, _Outptr_ PSECURITY_DESCRIPTOR *sd);
public:
    // This will be set to true if our ctor produces an error.
    bool m_bFailedToConstruct;
    CSecInfo();
    virtual ~CSecInfo();
    
    void SetCurrentObject(int index)
    {
        m_editingResource = index;
    }

    PRESOURCE GetResource(int index)
    {
        return m_resources[index];
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void **ppv);  
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();  
    
    // ISecurityInformation
    IFACEMETHODIMP GetObjectInformation (THIS_ PSI_OBJECT_INFO pObjectInfo);
    IFACEMETHODIMP GetSecurity (
        THIS_ SECURITY_INFORMATION si, 
        PSECURITY_DESCRIPTOR *sd, 
        BOOL fDefault);
    IFACEMETHODIMP SetSecurity (
        THIS_ SECURITY_INFORMATION si, 
        PSECURITY_DESCRIPTOR sd);
    IFACEMETHODIMP GetAccessRights (
        THIS_ const GUID* guidObjectType, 
        DWORD dwFlags, 
        PSI_ACCESS *access,
        ULONG *accesses, ULONG *defaultAccess);
    IFACEMETHODIMP MapGeneric (
        THIS_ const GUID *guidObjectType, 
        UCHAR *aceFlags, 
        ACCESS_MASK *mask);
    IFACEMETHODIMP GetInheritTypes (
        THIS_ PSI_INHERIT_TYPE *inheritTypes, 
        ULONG *numInheritTypes);
    IFACEMETHODIMP PropertySheetPageCallback (
        THIS_ HWND hwnd, 
        UINT uMsg, 
        SI_PAGE_TYPE uPage);

    // ISecurityInformation3
    IFACEMETHODIMP GetFullResourceName (THIS_ _Outptr_ LPWSTR *resourceName);
    IFACEMETHODIMP OpenElevatedEditor (
        THIS_ _In_ HWND hWnd, 
        _In_ SI_PAGE_TYPE uPage);

    // IEffectivePermission2
    STDMETHOD(ComputeEffectivePermissionWithSecondarySecurity) (THIS_
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
        _Inout_updates_(dwSecurityObjectCount) PEFFPERM_RESULT_LIST pEffpermResultLists);
    
    // ISecurityObjectTypeInfo
    STDMETHOD(GetInheritSource)(
        SECURITY_INFORMATION psi, 
        PACL acl, 
        PINHERITED_FROM *ppInheritArray);
};

const SI_ACCESS g_siForumsAccess[] =
{
    // This structure describes each flag in the file access mask.
    // It is constant. ACLUI displays these strings in its UI.
    { &GUID_NULL, GENERIC_ADMIN_PERM,L"Administer", SI_ACCESS_GENERAL | SI_ACCESS_SPECIFIC | INHERIT_FULL, },
    { &GUID_NULL, GENERIC_MOD_PERM,  L"Moderate",   SI_ACCESS_GENERAL | INHERIT_FULL },  
    { &GUID_NULL, GENERIC_POST_PERM, L"Post",       SI_ACCESS_GENERAL | INHERIT_FULL },  

    // Show advanced rights
    { &GUID_NULL, CREATE_PERM,          L"Create",                          SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, READ_PERM,            L"Read",                            SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, VOTE_PERM,            L"Vote",                            SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, UPDATE_OWN_PERM,      L"Update / edit own content",       SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, UPDATE_OTHERS_PERM,   L"Update / edit others' content",   SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, HIDE_PERM,            L"Hide",                            SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, SHOW_PERM,            L"Show",                            SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, LOCK_PERM,            L"Lock",                            SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, UNLOCK_PERM,          L"Unlock",                          SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, DESTROY_PERM,         L"Destroy / delete",                SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, VIEW_PERMS_PERM,      L"View permissions",                SI_ACCESS_SPECIFIC | INHERIT_FULL },  
    { &GUID_NULL, CHANGE_PERMS_PERM,    L"Change permissions",              SI_ACCESS_SPECIFIC | INHERIT_FULL },  
};

#endif // _CSECINFO_H_
