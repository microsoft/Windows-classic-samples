//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
// ResourceAttributesSample.cpp
//
// A simple command line tool for creating a security descriptor with
// resource attribute ACEs and a CAP ID ACE, then calling AuthzAccessCheck
// on that security descriptor to display the resulting access rights granted.
// 
// The caller can specify the resource attributes to use by either:
//     1. Supplying resource attributes through the command line, in the form
//        (Name,ValueType,Flags,Value1,Value2...)
//     2. Pointing to a file that already has resource attributes.
//
// The program will do the following:
//     1. Read in the CAP ID, and check if it is applied to the machine:
//            a. If CAP is applied, then it can be used in the AccessCheck.
//            b. If CAP is not applied, the applied CAPs are printed and the
//               program exits.
//     2. If a file is given, it tries to get its resource attributes.
//            a. If present, the attributes are printed and used in the
//               security descriptor.
//            b. If not present, the program exits.
//     3. If resource attributes are given, they are parsed, and used in
//        the security descriptor.
//     4. A security descriptor with a CAP ID ACE and resource attribute ACEs
//        is created.
//     5. AuthzAccessCheck is called using this security descriptor, and the
//        the resulting access rights granted are printed to the console.
//
//////////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 26018)
#pragma warning(disable: 28251)
#include "ResourceAttributesSample.h"


_Success_(return == TRUE) 
BOOL
CreateSecurityDescriptor(
    _In_        PSID                          CapIDSid, 
    _In_reads_(ResourceAttributeCt) PCLAIM_SECURITY_ATTRIBUTE_V1* ResourceAttributes,
    _In_        DWORD                         ResourceAttributeCt,
    _Outptr_    PSECURITY_DESCRIPTOR*         SecurityDescriptorResult
    )
/*++

Routine Description:

    Creates a self-relative security descriptor (SD) with System as the 
    owner and group. The DACL of the SD grants everyone full control, and
    a SACL is created with a CAPID ACE and resource attribute ACEs and added
    to the SD.

Arguments:

    CapIDSid                 : The SID of the CAP ID Ace to add to the SD

    ResourceAttributes       : An array of resource attributes to add to the SD

    ResourceAttributeCt      : The length of the ResourceAttributes array

    SecurityDescriptorResult : The resulting SD upon success, otherwise NULL

Return Value:

     TRUE/FALSE - If the function succeeds, TRUE is returned, otherwise FALSE.

--*/
{
    BOOL                     Succeeded          = FALSE;
    SECURITY_DESCRIPTOR      SecurityDescriptor = {0};
    PSID                     SystemSID          = NULL;
    PSID                     EveryoneSID        = NULL;
    PACL                     Dacl               = NULL;
    PACL                     Sacl               = NULL;
    UCHAR                    DaclBuffer[1024]   = {0};
    UCHAR                    SaclBuffer[1024]   = {0};
    SID_IDENTIFIER_AUTHORITY SIDAuthNt          = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld       = SECURITY_WORLD_SID_AUTHORITY;
    DWORD                    Bytes              = 0;
    DWORD                    Size               = 0;
    CLAIM_SECURITY_ATTRIBUTES_INFORMATION ClaimInfo   = {0};

    if (SecurityDescriptorResult == NULL ||
        CapIDSid                 == NULL ||
        ResourceAttributes       == NULL)
    {
        wprintf(L"ERROR: Invalid argument\n");
        goto Cleanup;
    }
    *SecurityDescriptorResult = NULL;

    //
    // Create a default security descriptor with system as the owner, system
    // as a the primary group, and a DACL that grants everyone full control
    //

    // Initialize the security descriptor
    if (!InitializeSecurityDescriptor(
                &SecurityDescriptor, 
                SECURITY_DESCRIPTOR_REVISION))
    {
        PrintErrorMessage(L"ERROR: InitializeSecurityDescriptor Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Create the sytem SID for the security descriptor owner/group
    if(!AllocateAndInitializeSid(
            &SIDAuthNt,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &SystemSID))
    {
        PrintErrorMessage(L"ERROR: AllocateAndInitializeSid Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Create the everyone SID
    if(!AllocateAndInitializeSid(
            &SIDAuthWorld,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            &EveryoneSID))
    {
        PrintErrorMessage(L"ERROR: AllocateAndInitializeSid Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Set the owner of the security descriptor to System.
    if (!SetSecurityDescriptorOwner(&SecurityDescriptor, SystemSID, TRUE))
    {
        PrintErrorMessage(L"ERROR: SetSecurityDescriptorOwner Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Set the primary group to System.
    if (!SetSecurityDescriptorGroup(&SecurityDescriptor, SystemSID, TRUE))
    {
        PrintErrorMessage(L"ERROR: SetSecurityDescriptorGroup Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Initialize the DACL
    Dacl = (PACL)DaclBuffer;
    if (!InitializeAcl(Dacl, ARRAYSIZE(DaclBuffer), ACL_REVISION))
    {
        PrintErrorMessage(L"ERROR: InitializeAcl Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Add an ACE to the DACL to grant everyone full control
    if (!AddAccessAllowedAce(Dacl, ACL_REVISION, KEY_ALL_ACCESS, EveryoneSID))
    {
        PrintErrorMessage(L"ERROR: AddAccessAllowedAce Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Now add the dACL to the security descriptor.
    if (!SetSecurityDescriptorDacl(&SecurityDescriptor, TRUE, Dacl, FALSE))
    {
        PrintErrorMessage(L"ERROR: SetSecurityDescriptorDacl Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Initialize the SACL.
    // Note: The ACL buffer is a static 1K buffer, so adding a large number of 
    // resource attribute ACEs will cause AddResourceAttributeAce to fail if 
    // the size exceeds the 1K limit.  If you have a need for a large number 
    // of ACEs, consider increasing the buffer size or using dynamic memory
    // allocation.
    Sacl = (PACL)SaclBuffer;
    if (!InitializeAcl(Sacl, ARRAYSIZE(SaclBuffer), ACL_REVISION))
    {
        PrintErrorMessage(L"ERROR: InitializeAcl Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Add a CAPID Ace to the ACL 
    if (!AddScopedPolicyIDAce(Sacl, ACL_REVISION, 0, 0, CapIDSid))
    {
        PrintErrorMessage(L"ERROR: AddScopedPolicyIDAce Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Build up the claim information structure.
    ClaimInfo.AttributeCount = 1;
    ClaimInfo.Version = CLAIM_SECURITY_ATTRIBUTES_INFORMATION_VERSION_V1;

    // Iterate over the resource claims and add an ACE for each one.
    for (DWORD i = 0; i < ResourceAttributeCt; ++i)
    {
        ClaimInfo.Attribute.pAttributeV1 = ResourceAttributes[i];
        if (!AddResourceAttributeAce(
                Sacl,
                ACL_REVISION,
                0,
                0,
                EveryoneSID,
                &ClaimInfo,
                &Bytes))
        {
            PrintErrorMessage(L"ERROR: AddResourceAttributeAce Error", 
                              GetLastError());
            wprintf(L"    -Attribute = %ws\n", ResourceAttributes[i]->Name);
            goto Cleanup;
        }
    }

    // Now add the SACL to the security descriptor.
    if (!SetSecurityDescriptorSacl(&SecurityDescriptor, TRUE, Sacl, FALSE))
    {
        PrintErrorMessage(L"ERROR: SetSecurityDescriptorSacl Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Convert to a self relative security descriptor
    if (!MakeSelfRelativeSD(&SecurityDescriptor, NULL, &Size))
    {
        // First call to MakeSelfRelativeSD should return 
        // ERROR_INSUFFICIENT_BUFFER and set Size to the necessary buffer
        // length
        assert(GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        *SecurityDescriptorResult = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,Size);
        if (NULL == *SecurityDescriptorResult)
        {
            wprintf(L"ERROR: Out of memory\n");
            goto Cleanup;
        }

        if (!MakeSelfRelativeSD(
                &SecurityDescriptor, 
                *SecurityDescriptorResult, 
                &Size))
        {
            PrintErrorMessage(L"ERROR: MakeSelfRelativeSD Error", 
                              GetLastError());
            LocalFree(*SecurityDescriptorResult); 
            *SecurityDescriptorResult = NULL;
            goto Cleanup;
        }

        // We have successfully created a self-relative security descriptor 
        // with the Central Access Policy and resource attributes
        Succeeded = TRUE;
    }
    else
    {
        *SecurityDescriptorResult = NULL;
    }

Cleanup:
    if (EveryoneSID) 
    {
        FreeSid(EveryoneSID);
    }
    if (SystemSID)
    {
        FreeSid(SystemSID);
    }
    return Succeeded;
}


BOOL 
PerformAccessCheck(
    _In_ PSECURITY_DESCRIPTOR SecurityDescriptor
    )
/*++

Routine Description:

    Initializes an Authz context and calls AuthzAccessCheck on the supplied
    security descriptor.  The results of the access check are printed out to
    the console.

Arguments:

     SecurityDescriptor : The security descriptor used in the access check.

Return Value:

     TRUE/FALSE - If the function succeeds, TRUE is returned, otherwise FALSE.

--*/
{
    BOOL                            Succeeded           = FALSE;
    HANDLE                          Token               = NULL;
    AUTHZ_RESOURCE_MANAGER_HANDLE   AuthzResMgrHandle   = NULL;
    AUTHZ_CLIENT_CONTEXT_HANDLE     AuthzClientHandle   = NULL;
    LUID                            ZeroLuid            = {0, 0};
    AUTHZ_ACCESS_REQUEST            AccessRequest       = {0};
    AUTHZ_ACCESS_REPLY              AccessReply         = {0};
    BYTE                            Buffer[1024]        = {0};

    if (NULL == SecurityDescriptor)
    {
        wprintf(L"ERROR: Invalid argument\n");
        goto Cleanup;
    }

    // Open the process token
    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &Token))
    {
        PrintErrorMessage(L"ERROR: OpenProcessToken Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Initialize the resource manager for AuthzAccessCheck
    if (!AuthzInitializeResourceManager(
            AUTHZ_RM_FLAG_NO_AUDIT,
            NULL,
            NULL,
            NULL,
            NULL,
            &AuthzResMgrHandle))
    {
        PrintErrorMessage(L"ERROR: AuthzInitializeResourceManager Error", 
                          GetLastError());
        goto Cleanup;
    }

    //  Get the authorization context from the token to use for access check
    if (!AuthzInitializeContextFromToken(
            0,
            Token,
            AuthzResMgrHandle,
            NULL,
            ZeroLuid,
            NULL,
            &AuthzClientHandle))
    {
        PrintErrorMessage(L"ERROR: AuthzInitializeContextFromToken Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Initialize the request and reply structures passed to AuthzAccessCheck
    AccessRequest.DesiredAccess         = MAXIMUM_ALLOWED;
    AccessRequest.PrincipalSelfSid      = NULL;
    AccessRequest.ObjectTypeList        = NULL;
    AccessRequest.ObjectTypeListLength  = 0;
    AccessRequest.OptionalArguments     = NULL;

    AccessReply.ResultListLength        = 1;
    AccessReply.GrantedAccessMask       = (PACCESS_MASK)Buffer;
    AccessReply.Error                   = (PDWORD)(Buffer+sizeof(ACCESS_MASK));

    // Perform the actual access check
    if (!AuthzAccessCheck(
            0,
            AuthzClientHandle,
            &AccessRequest,
            NULL,
            SecurityDescriptor,
            NULL,
            0,
            &AccessReply,
            NULL))
    {
        PrintErrorMessage(L"ERROR: AuthzAccessCheck Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Print out the results of the AuthzAccessCheck
    wprintf(L"\nAccess Mask = 0x%08x\n\n", AccessReply.GrantedAccessMask[0]);

    Succeeded = TRUE;

Cleanup:
    if (NULL != AuthzClientHandle)
    {
        AuthzFreeContext(AuthzClientHandle);
    }
    if (NULL != AuthzResMgrHandle)
    {
        AuthzFreeResourceManager(AuthzResMgrHandle);
    }
    if (NULL != Token)
    {
        CloseHandle(Token);
    }

    return Succeeded;
}


_Success_(return == TRUE) 
BOOL
IsValidCapID(
    _In_      LPCWSTR CapID,
    _Outptr_  PSID*   CapIDSidResult
    )
{
/*++

Routine Description:

    Checks if the requested CAP ID is present on the machine.  If it is not,
    then the applied CAPs are printed out to the console.  If it is present,
    then the string SID is converted to a binary SID and returned to the 
    caller.

Arguments:

     CapID             : The CAP ID requested

     CapIDSidResult    : The SID form of the requested CAP ID returned if the
                         CAP is present on the machine, otherwise NULL

Return Value:

     TRUE/FALSE - If the CAP is present, TRUE is returned, otherwise FALSE.

--*/
    NTSTATUS        Status;
    BOOL            IsValidCapID    = FALSE;
    PSID            CapIDSid        = NULL;
    PSID*           AppliedCaps     = NULL;
    ULONG           CapCount        = 0;
    LPWSTR          CapIDString     = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthCap = SECURITY_SCOPED_POLICY_ID_AUTHORITY;

    if (NULL == CapID || NULL == CapIDSidResult)
    {
        wprintf(L"ERROR: Invalid argument\n");
        goto Cleanup;
    }
    *CapIDSidResult = NULL;

    // Convert the CAPID string to a SID
    if (!ConvertStringSidToSid(CapID, &CapIDSid))
    {
        PrintErrorMessage(L"ERROR: ConvertStringSidToSid Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Check that the CAPID is a valid SID
    if (!IsValidSid(CapIDSid))
    {
        wprintf(L"ERROR: The supplied CAPID is not a valid SID\n");
        goto Cleanup;
    }

    // Check that the SID is a valid CAP ID
    if (!RtlEqualMemory(
            &((PISID)CapIDSid)->IdentifierAuthority, 
            &SIDAuthCap, 
            sizeof(SID_IDENTIFIER_AUTHORITY)))
    {
        wprintf(L"ERROR: The CAPID doesn't have the correct authority S-1-17\n");
        goto Cleanup;
    }

    // Get the applied CAPIDs on the machine
    Status = LsaGetAppliedCAPIDs(NULL, &AppliedCaps, &CapCount);
    if (!NT_SUCCESS(Status))
    {
        wprintf(L"ERROR: LsaGetAppliedCAPIDs Error %u\n", Status);
        goto Cleanup;
    }

    // Search the applied CAPs on the machine for a match to the requested CAP
    for (ULONG i = 0; i < CapCount; i++)
    {
        if (EqualSid(AppliedCaps[i], CapIDSid))
        {
            IsValidCapID = TRUE;
            break;
        }
    }

    if (IsValidCapID)
    {
        *CapIDSidResult = CapIDSid;
        CapIDSid = NULL;
    }
    else
    {
        // The requested CAPID is not part of the applied Central Policies
        wprintf(L"The requested CAPID could not be found on the target machine."
                L"\nThe applied CAPIDs are:\n");
        for (ULONG i = 0; i < CapCount; i++)
        {
            if (!ConvertSidToStringSid(AppliedCaps[i], &CapIDString))
            {
                PrintErrorMessage(L"ERROR: ConvertSidToStringSid Error", 
                                  GetLastError());
                goto Cleanup;
            }
            wprintf(L"    %ws\n", CapIDString);

            LocalFree(CapIDString);
            CapIDString = NULL;
        }
    }

Cleanup:
    if (AppliedCaps != NULL) 
    {
        for (ULONG i = 0; i < CapCount; i++)
        {
            LsaFreeMemory(AppliedCaps[i]);
        }
        LsaFreeMemory(AppliedCaps);
    }
    if (CapIDSid)
    {
        FreeSid(CapIDSid);
    }
    LocalFree(CapIDString);

    return IsValidCapID;
}


_Success_(return == TRUE) 
BOOL
GetFileResourceAttributes(
    _In_                           LPCWSTR FileName,
    _Outptr_result_buffer_(*Count) PCLAIM_SECURITY_ATTRIBUTE_V1* FileResourceAttributes[],
    _Out_                          DWORD*  Count
    )
/*++

Routine Description:

    Gets the resource attribute ACEs from a file, parses the SDDL for the
    resource attributes ACEs to print them in a friendly format to the console.
    Then it interprets the resource attributes SDDL and populates a resource
    claim structure that can be used to add a resouce attribute ACE to a 
    security descriptor.

Arguments:

    FileName               : The file to get the resource properties from.

    FileResourceAttributes : The resulting array of resource attribute 
                             structures.

    Count                  : The number of claims in the array.

Return Value:

     TRUE/FALSE - If the function succeeds, TRUE is returned, otherwise FALSE.

--*/
{
    BOOL                            Succeeded           = FALSE;
    DWORD                           Success             = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR            SecurityDescriptor  = NULL;
    PACL                            Sacl                = NULL;
    LPWSTR                          SDDL                = NULL;
    LPWSTR                          NextToken           = NULL;
    LPWSTR                          NextAttribute       = NULL;
    PRESOURCE_ATTRIBUTE             AttributeTokens     = NULL;
    PCLAIM_SECURITY_ATTRIBUTE_V1*   ResourceAttributes  = NULL;
    DWORD                           AttributeCount      = 0;
    DWORD                           CurrentAttribute    = 0;
    DWORD                           ByteCount           = 0;

    if (NULL == FileName               || 
        NULL == FileResourceAttributes || 
        NULL == Count)
    {
        wprintf(L"ERROR: Invalid argument\n");
        goto Cleanup;
    }
    *FileResourceAttributes = NULL;
    *Count                  = 0;

    //
    // Get the security descriptor for the file.
    // Note that we will use the SecurityDescriptor, but Sacl is now
    // also pointing to the attribute ACEs.
    //
    Success = GetNamedSecurityInfo(
                    FileName, 
                    SE_FILE_OBJECT, 
                    ATTRIBUTE_SECURITY_INFORMATION, 
                    NULL, 
                    NULL, 
                    NULL, 
                    &Sacl, 
                    &SecurityDescriptor);
    if (ERROR_SUCCESS != Success)
    {
        PrintErrorMessage(L"ERROR: GetNamedSecurityInfo Error", 
                          Success);
        goto Cleanup;
    }

    // Convert the binary security descriptor to SDDL.
    if (!ConvertSecurityDescriptorToStringSecurityDescriptor(
                    SecurityDescriptor,
                    SDDL_REVISION_1,
                    ATTRIBUTE_SECURITY_INFORMATION, // attribute ACEs only
                    &SDDL,
                    NULL))
    {
        PrintErrorMessage(L"ERROR: "
                          L"ConvertSecurityDescriptorToStringSecurityDescriptor "
                          L"Error", 
                          GetLastError());
        goto Cleanup;
    }

    // Count the number of resource attributes in the SDDL
    NextToken = SDDL;
    while (NextToken)
    {
        NextToken = wcsstr(NextToken, L"(RA;");
        if (NextToken != NULL)
        {
            AttributeCount++;
            NextToken++;
        }
    }

    if (AttributeCount < 1)
    {
        wprintf(L"\nThe file %ws does not have resource attributes\n",
               FileName);
        goto Cleanup;
    }

    AttributeTokens = new RESOURCE_ATTRIBUTE[AttributeCount];
    if (NULL == AttributeTokens)
    {
        wprintf(L"ERROR: Out of memory\n");
        goto Cleanup;
    }
    ZeroMemory(AttributeTokens, AttributeCount * sizeof(RESOURCE_ATTRIBUTE));

    // Parse the SDDL string to pull out the name, flags, type, and values
    // tokens from each resource attribute.
    NextToken = SDDL;
    while (NextToken)
    {
        // Look for (RA; which is the resource ACE signature
        NextToken = wcsstr(NextToken, L"(RA;");

        if (NextToken != NULL)
        {
            // Skip past AceType
            NextToken = wcsstr(NextToken, L";");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }
            NextToken++;

            // Skip past AceFlags
            NextToken = wcsstr(NextToken, L";");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }
            NextToken++;

            // Skip past Rights
            NextToken = wcsstr(NextToken, L";");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }
            NextToken++;

            // Skip past ObjectGuid
            NextToken = wcsstr(NextToken, L";");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }
            NextToken++;

            // Skip past InheritObjectGuid
            NextToken = wcsstr(NextToken, L";");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }
            NextToken++;

            // Skip past AccountSid
            NextToken = wcsstr(NextToken, L";");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }
            NextToken++;

            // Get the start of the resource claim string
            NextToken = wcsstr(NextToken, L"(");
            if (NULL == NextToken)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }

            // Resource claim string ends at closing parenthesis
            NextAttribute = wcstok_s(NULL,L")",&NextToken);
            if (NULL == NextAttribute)
            {
                wprintf(L"ERROR: Invalid SDDL encountered\n");
                goto Cleanup;
            }

            // Parse the resource attribute string into its tokens
            if (!ParseResourceAttributeString(
                    NextAttribute, 
                    AttributeTokens[CurrentAttribute]))
            {
                goto Cleanup;
            }

            NextAttribute = NULL;
            CurrentAttribute++;
        }
    }

    // Allocate an array of resource claim structures to fill and return
    ByteCount = sizeof(PCLAIM_SECURITY_ATTRIBUTE_V1) * AttributeCount;
    ResourceAttributes = static_cast<PCLAIM_SECURITY_ATTRIBUTE_V1*>(
            LocalAlloc(LPTR, ByteCount));
    if (NULL == ResourceAttributes)
    {
        wprintf(L"ERROR: Out of memory\n");
        AttributeCount = 0;
        goto Cleanup;
    }

    // Loop over the parsed resource attributes to print the values to the
    // console, and fill the claim structures to return to the caller.
    wprintf(L"\nResource attributes for file %ws:\n\n", FileName);
    for (CurrentAttribute = 0; 
         CurrentAttribute < AttributeCount; 
         CurrentAttribute++)
    {
        PrintResourceAttribute(AttributeTokens[CurrentAttribute]);
        if (!InterpretResourceAttribute(
                    AttributeTokens[CurrentAttribute], 
                    &ResourceAttributes[CurrentAttribute]))
        {
            goto Cleanup;
        }
    }

    *FileResourceAttributes = ResourceAttributes;
    *Count                  = AttributeCount;
    ResourceAttributes      = NULL;
    AttributeCount          = 0;

    Succeeded = TRUE;

Cleanup:
    if (AttributeTokens)
    {
        delete [] AttributeTokens;
    }

    for (DWORD i = 0; i < AttributeCount && ResourceAttributes; ++i)
    {
        LocalFree(ResourceAttributes[i]);
    }
    LocalFree(ResourceAttributes);

    LocalFree(SDDL);
    LocalFree(SecurityDescriptor);

    return Succeeded;
}


BOOL
ParseResourceAttributeString(
    _In_    LPWSTR              AttributeString,
    _Inout_ RESOURCE_ATTRIBUTE& Attribute
    )
/*++

Routine Description:

    Given a resource attribute in the SDDL format: 
        (Name, ValueType, Flags, Value1, Value2,...)
    The string is validated and parsed to extract the name, type, flags, and
    values strings.

Arguments:

    AttributeString  : The full resource attribute in (name,type,flags,values).

    Attribute        : A structure containing the tokenized parts of the
                       attribute.

Return Value:

     TRUE/FALSE - If the function succeeds, TRUE is returned, otherwise FALSE.

--*/
{
    BOOL    Succeeded   = FALSE;
    LPWSTR  NextToken   = NULL;

    if (NULL == AttributeString)
    {
        wprintf(L"ERROR: Invalid parameter\n");
        goto Cleanup;
    }
    NextToken = AttributeString;

    //
    // Tokenize the string to get pointers to the name, type, flags, and values
    //

    // Extract Name
    if (NextToken[0] != L'(')
    {
        wprintf(L"ERROR: Invalid resource attribute format - attribute doesn't"
                L"have starting '('\n");
        goto Cleanup;
    }
    NextToken++;

    Attribute.Name = wcstok_s(NextToken, L",", &NextToken);
    if (NULL == Attribute.Name)
    {
        wprintf(L"ERROR: Invalid resource attribute format - no name\n");
        goto Cleanup;
    }

    // Extract Type
    Attribute.Type = wcstok_s(NULL, L",", &NextToken);
    if (NULL == Attribute.Type)
    {
        wprintf(L"ERROR: Invalid resource attribute format - no type\n");
        goto Cleanup;
    }

    // Extract Flags
    Attribute.Flags = wcstok_s(NULL,L",",&NextToken);
    if (NULL == Attribute.Flags)
    {
        wprintf(L"ERROR: Invalid resource attribute format - no flags\n");
        goto Cleanup;
    }

    // Extract Values
    Attribute.Values = wcstok_s(NULL,L")",&NextToken);
    if (NULL == Attribute.Values)
    {
        wprintf(L"ERROR: Invalid resource attribute format - no values\n");
        goto Cleanup;
    }

    Succeeded = TRUE;

Cleanup:

    return Succeeded;
}


_Success_(return == TRUE) 
BOOL
InterpretResourceAttribute(
    _In_     RESOURCE_ATTRIBUTE            AttributeTokens,
    _Outptr_ PCLAIM_SECURITY_ATTRIBUTE_V1* ResourceClaimResult
    )
/*++

Routine Description:

    Takes a tokenized resource attribute string structure and produces a
    self-relative claim structure that can be used to add a resource attribute
    ACE to a security descriptor.

Arguments:

    AttributeTokens      : The tokenized resource attribute string structure.

    ResourceClaimResult  : The resulting resource claim structure.

Return Value:

     TRUE/FALSE - If the function succeeds, TRUE is returned, otherwise FALSE.

--*/
{
    BOOL                            Succeeded           = FALSE;
    HRESULT                         hr                  = S_OK;
    LPWSTR                          NextToken           = NULL;
    LPWSTR*                         Values              = NULL;
    LPWSTR                          EndPtr              = NULL;
    PCLAIM_SECURITY_ATTRIBUTE_V1    ResourceAttribute   = NULL;
    PBYTE                           NextMemory          = NULL;
    DWORD                           ByteCount           = 0;
    DWORD                           ValueCount          = 0;
    WORD                            ValueType           = 0;
    SIZE_T                          NameBytes           = 0;
    DWORD                           Flags               = 0;

    if (NULL == ResourceClaimResult)
    {
        wprintf(L"ERROR: Invalid parameter\n");
        goto Cleanup;
    }

    // Interpret the value type string into an actual value type
    ValueType = GetValueType(AttributeTokens.Type);
    if (ValueType == CLAIM_SECURITY_ATTRIBUTE_TYPE_INVALID)
    {
        wprintf(L"Claim %ws has an unknown type (%ws)", 
                AttributeTokens.Name,
                AttributeTokens.Type);
        goto Cleanup;
    }

    // Get the number of claim values in the string (comma separated)
    NextToken = AttributeTokens.Values;
    while (NextToken)
    {
        ValueCount++;
        NextToken = wcsstr(NextToken, L",");
        if (NextToken != NULL)
        {
            NextToken++;
        }
    }

    // Must be at lease one value
    if (ValueCount < 1)
    {
        wprintf(L"ERROR: No values for the resource claim\n");
        goto Cleanup;
    }

    // Break up the claim values string into individual values
    Values = new LPWSTR[ValueCount];
    if (NULL == Values)
    {
        wprintf(L"ERROR: Out of memory\n");
        goto Cleanup;
    }

    // Get a pointer to each individual value
    NextToken = AttributeTokens.Values;
    for (DWORD i = 0; i < ValueCount && NextToken; ++i)
    {
        Values[i] = wcstok_s(NextToken, L",", &NextToken);
    }

    // Get the entire size required for the resource attribute structure, so we
    // can allocate a single block of contiguous memory.
    NameBytes  = (wcsnlen(AttributeTokens.Name, MAX_PATH) + 1) * sizeof(WCHAR);
    ByteCount  = ALIGN(sizeof(CLAIM_SECURITY_ATTRIBUTE_V1));
    ByteCount += ALIGN(NameBytes);
    ByteCount += ALIGN(ValueCount * GetValueSize(ValueType));

    if (ValueType == CLAIM_SECURITY_ATTRIBUTE_TYPE_STRING)
    {
        // Additional allocation for each string in the array
        for (DWORD i = 0; i < ValueCount; ++i)
        {
            ByteCount += ALIGN((wcslen(Values[i]) + 1) * sizeof(WCHAR));
        }
    }

    NextMemory = static_cast<PBYTE>(LocalAlloc(LPTR, ByteCount));
    if (NULL == NextMemory)
    {
        wprintf(L"ERROR: Out of memory\n");
        goto Cleanup;
    }

    ResourceAttribute  = (PCLAIM_SECURITY_ATTRIBUTE_V1)NextMemory;
    NextMemory        += ALIGN(sizeof(CLAIM_SECURITY_ATTRIBUTE_V1));

    // Fill out resource claim structure
    ResourceAttribute->ValueType  = ValueType;
    ResourceAttribute->ValueCount = ValueCount;
    ResourceAttribute->Reserved   = 0;
    ResourceAttribute->Name       = (LPWSTR)NextMemory;

    hr = StringCbCopy(ResourceAttribute->Name, NameBytes, AttributeTokens.Name);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: StringCbCopy hr = 0x%08x\n", hr);
        goto Cleanup;
    }
    NextMemory += ALIGN(NameBytes);

    // Convert the string hex flags to the actual integer value
    EndPtr = AttributeTokens.Flags + wcslen(AttributeTokens.Flags);
    Flags  = wcstoul(AttributeTokens.Flags, &EndPtr, 0);

    if(((Flags & 0x0000FFFF) & (~CLAIM_SECURITY_ATTRIBUTE_VALID_FLAGS)) != 0)
    {
        wprintf(L"ERROR: Invalid flags\n");
        goto Cleanup;
    }
    ResourceAttribute->Flags = Flags;

    // Fill up the values array
    switch(ValueType)
    {
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_INT64:
        ResourceAttribute->Values.pInt64 = (LONG64*)NextMemory;
        for (DWORD i = 0; i < ValueCount; ++i)
        {
            EndPtr = Values[i] + wcslen(Values[i]);
            ResourceAttribute->Values.pInt64[i] = _wcstoi64(Values[i], 
                                                            &EndPtr, 
                                                            0);
        }
        break;
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_UINT64:
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_BOOLEAN:
        ResourceAttribute->Values.pUint64 = (DWORD64*)NextMemory;
        for (DWORD i = 0; i < ValueCount; ++i)
        {
            EndPtr = Values[i] + wcslen(Values[i]);
            ResourceAttribute->Values.pUint64[i] = _wcstoui64(Values[i], 
                                                              &EndPtr, 
                                                              0);
        }
        break;
    case CLAIM_SECURITY_ATTRIBUTE_TYPE_STRING:
        ResourceAttribute->Values.ppString = (LPWSTR*)NextMemory;
        NextMemory += ALIGN(ValueCount * GetValueSize(ValueType));

        for (DWORD i = 0; i < ValueCount; ++i)
        {
            ResourceAttribute->Values.ppString[i] = (LPWSTR)NextMemory;
            hr = StringCbCopy(ResourceAttribute->Values.ppString[i],
                              (wcslen(Values[i]) + 1) * sizeof(WCHAR),
                              Values[i]);
            if (FAILED(hr))
            {
                wprintf(L"ERROR: StringCbCopy hr = 0x%08\n");
                goto Cleanup;
            }
            NextMemory += ALIGN((wcslen(Values[i]) + 1) * sizeof(WCHAR));
        }
        break;
    default:
        break;
    }

    *ResourceClaimResult = ResourceAttribute;
    ResourceAttribute = NULL;
    Succeeded = TRUE;

Cleanup:
    if (Values)
    {
        delete [] Values;
    }
    LocalFree(ResourceAttribute);
    return Succeeded;
}


_Success_(return == TRUE) 
BOOL
ParseResourceAttributesArguments(
    _In_                           LPCWSTR ResourceAttributesArgs,
    _Outptr_result_buffer_(*Count) PCLAIM_SECURITY_ATTRIBUTE_V1* ResourceAttributesResult[],
    _Out_                          DWORD*  Count
    )
/*++

Routine Description:

    Parses the resource attribute arguments passed in on the command line using
    the SDDL format for resource attributes:
        (Name, ValueType, Flags, Value1, Value2,...)

Arguments:

     ResourceAttributesArgs   : The resource attributes arguments string.

     ResourceAttributesResult : An array of parsed resource attributes in claim
                                structures that can be used to add a resource
                                claim to a security descriptor.

     Count                    : The number of claims in the array.

Return Value:

     TRUE/FALSE - If the function succeeds, TRUE is returned, otherwise FALSE.

--*/
{
    BOOL                            Succeeded           = FALSE;
    PCLAIM_SECURITY_ATTRIBUTE_V1*   ResourceAttributes  = NULL;
    DWORD                           AttributeCount      = 0;
    LPWSTR                          NextToken           = NULL;
    LPWSTR                          NextAttribute       = NULL;
    DWORD                           ByteCount           = 0;
    RESOURCE_ATTRIBUTE              Attribute           = {0};
    DWORD                           CurrentAttribute    = 0;

    if (NULL == ResourceAttributesArgs   || 
        NULL == ResourceAttributesResult || 
        NULL == Count)
    {
        wprintf(L"ERROR: Invalid argument\n");
        goto Cleanup;
    }
    *ResourceAttributesResult = NULL;
    *Count = 0;

    if (ResourceAttributesArgs[0] != L'(')
    {
        wprintf(L"ERROR: Invalid resource attributes parameter\n");
        goto Cleanup;
    }

    // Get the number of resource attributes by searching for the "(" token
    // Resource attributes must be of the form: (Name, Type, Flags, Value1,...)
    NextToken = (LPWSTR)ResourceAttributesArgs;
    while(NextToken != NULL)
    {
        NextToken = wcsstr(NextToken, L"(");
        if (NULL != NextToken)
        {
            AttributeCount++;
            NextToken++;
        }
    }

    if (AttributeCount < 1)
    {
        wprintf(L"ERROR: Invalid resource attributes parameter format\n");
        goto Cleanup;
    }

    // Allocate an array of claim structures
    ByteCount = AttributeCount * sizeof(PCLAIM_SECURITY_ATTRIBUTE_V1);
    ResourceAttributes = static_cast<PCLAIM_SECURITY_ATTRIBUTE_V1*>(
                            LocalAlloc(LPTR, ByteCount));
    if (NULL == ResourceAttributes)
    {
        wprintf(L"ERROR: Out of memory\n");
        AttributeCount = 0;
        goto Cleanup;
    }

    // Iterate over the resource attributes argument, parsing each attribute,
    // then convert the string resource attribute into a claim structure 
    // required for adding the resource claims to an ACE
    wprintf(L"\nThe requested resource attributes are:\n\n");
    NextToken = (LPWSTR)ResourceAttributesArgs;
    while(NextToken)
    {
        // Start of the next claim
        NextToken = wcsstr(NextToken, L"(");
        if (NextToken != NULL)
        {
            // End of claim
            NextAttribute = wcstok_s(NULL,L")",&NextToken);
            if (NULL == NextAttribute)
            {
                wprintf(L"ERROR: Invalid resource attribute parameter format:"
                        L"%ws\n", NextToken);
                goto Cleanup;
            }

            // Intermediate parsing of the resource string
            if (!ParseResourceAttributeString(NextAttribute, Attribute))
            {
                goto Cleanup;
            }
            PrintResourceAttribute(Attribute);

            // Convert the string resource structure into an actual claim 
            // structure
            if (!InterpretResourceAttribute(
                        Attribute, 
                        &ResourceAttributes[CurrentAttribute]))
            {
                goto Cleanup;
            }

            NextAttribute = NULL;
            CurrentAttribute++;
        }
    }

    *ResourceAttributesResult = ResourceAttributes;
    *Count                    = AttributeCount;
    ResourceAttributes        = NULL;
    AttributeCount            = 0;

    Succeeded                 = TRUE;

Cleanup:
    for (DWORD i = 0; i < AttributeCount && ResourceAttributes; ++i)
    {
        LocalFree(ResourceAttributes[i]);
    }
    LocalFree (ResourceAttributes);

    return Succeeded;
}


VOID
PrintResourceAttribute(
    _In_ RESOURCE_ATTRIBUTE Attribute
    )
/*++

Routine Description:

    Helper function to prints out a resource attribute in a friendly format.

Arguments:

    Attribute        : A structure containing the resource attribute to print.

Return Value:

--*/
{
    LPWSTR Type              = Attribute.Type;;
    LPWSTR FriendlyValueType = NULL;

    if(0 == _wcsnicmp(Type, SDDL_INT, SDDL_LEN_TAG(SDDL_INT))) 
    {
        FriendlyValueType = L"Integer";
    }
    else if(0 == _wcsnicmp(Type, SDDL_UINT, SDDL_LEN_TAG(SDDL_UINT))) 
    {
        FriendlyValueType = L"Unsigned Integer";
    }
    else if(0 == _wcsnicmp(Type, SDDL_WSTRING, SDDL_LEN_TAG(SDDL_WSTRING))) 
    {
        FriendlyValueType = L"String";
    }
    else if(0 == _wcsnicmp(Type, SDDL_BOOLEAN, SDDL_LEN_TAG(SDDL_BOOLEAN))) 
    {
        FriendlyValueType = L"Boolean";
    }
    else 
    {
        FriendlyValueType = L"Unknown";
    }

    wprintf(L"Resource Attribute:\n");
    wprintf(L"    - Name   :  %ws\n", Attribute.Name);
    wprintf(L"    - Type   :  %ws (%ws)\n", FriendlyValueType, Type);
    wprintf(L"    - Flags  :  %ws\n", Attribute.Flags);
    wprintf(L"    - Values :  %ws\n", Attribute.Values);
    wprintf(L"\n");
}


int 
__cdecl 
wmain(
    _In_              DWORD     argc, 
    _In_reads_(argc)  LPCWSTR   argv[]
    )
{
    BOOL                            Succeeded               = FALSE;
    LPWSTR                          FileName                = NULL;
    LPWSTR                          CapID                   = NULL;
    LPWSTR                          Attributes              = NULL;
    PSID                            CapIDSid                = NULL;
    PSECURITY_DESCRIPTOR            SecurityDescriptor      = NULL;
    PCLAIM_SECURITY_ATTRIBUTE_V1*   ResourceAttributes      = NULL;
    DWORD                           ResourceAttributeCt     = 0;

    if (argc != 5)
    {
        PrintUsage();
        goto Cleanup;
    }

    for (DWORD i = 1, j = 2; j < argc; i += 2, j += 2)
    {
        if (FileParameter(argv[i]))
        {
            FileName = (LPWSTR)argv[j];
        }
        else if (CapParameter(argv[i]))
        {
            CapID = (LPWSTR)argv[j];
        }
        else if (RaParameter(argv[i]))
        {
            Attributes = (LPWSTR)argv[j];
        }
        else
        {
            PrintUsage();
            goto Cleanup;
        }
    }

    if ((CapID == NULL)                          || 
        (Attributes == NULL && FileName == NULL) ||
        (Attributes != NULL && FileName != NULL))
    {
        PrintUsage();
        goto Cleanup;
    }

     if (Attributes && !ParseResourceAttributesArguments(
                            (LPCWSTR)Attributes, 
                            &ResourceAttributes, 
                            &ResourceAttributeCt))
    {
        goto Cleanup;
    }

    if (FileName && !GetFileResourceAttributes(
                            FileName, 
                            &ResourceAttributes, 
                            &ResourceAttributeCt))
    {
        goto Cleanup;
    }

    if (!IsValidCapID((LPCWSTR)CapID, &CapIDSid))
    {
        goto Cleanup;
    }

    if (!CreateSecurityDescriptor(
            CapIDSid, 
            ResourceAttributes, 
            ResourceAttributeCt,
            &SecurityDescriptor))
    {
        goto Cleanup;
    }

    if (!PerformAccessCheck(SecurityDescriptor))
    {
        goto Cleanup;
    }

    Succeeded = TRUE;

Cleanup:

    if (CapIDSid)
    {
        FreeSid(CapIDSid);
    }
    LocalFree(SecurityDescriptor);

    for (DWORD i = 0; i < ResourceAttributeCt; ++i)
    {
        LocalFree(ResourceAttributes[i]);
    }
    LocalFree(ResourceAttributes);

    return Succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}

VOID
PrintErrorMessage(
    _In_ LPWSTR ErrorMessage,
    _In_ DWORD  ErrorCode
    )
{
    LPWSTR SystemMsg = NULL;

    if (FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER    | 
                    FORMAT_MESSAGE_MAX_WIDTH_MASK | 
                    FORMAT_MESSAGE_FROM_SYSTEM, 
                NULL, 
                ErrorCode, 
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
                (LPWSTR)&SystemMsg, 
                0, 
                NULL))
    {
        wprintf(L"%ws '%ws'(%u)\n", ErrorMessage, SystemMsg, ErrorCode);
    }
    else
    {
        wprintf(L"%ws %u\n", ErrorMessage, ErrorCode);
    }

    LocalFree(SystemMsg);
}

VOID 
PrintUsage()
{
    printf(
"                                                                           \n"
"    A command line tool for creating a security descriptor with resource   \n"
"    property claim ACEs and a Central Access Policy ACE, and displaying    \n"
"    the results of AuthzAccessCheck on that security descriptor.           \n"
"                                                                           \n"
"    Usage:                                                                 \n"
"                                                                           \n"
"    ResourceAttributesSample [-cap <ID>] [-file <PATH> | -ra <ATTRIBUTES>] \n"
"           -cap  : A Central Access Policy SID (CAP ID). CAP IDs are of the\n"
"                   form: S-1-17-...                                        \n"
"           -file : A file path used to get resource properties from a file \n"
"                   to use in the security descriptor. The file's resource  \n"
"                   properties are also printed to the console.             \n"
"           -ra   : Resource properties to add to the security descriptor.  \n"
"                   Resource properties should be specified in the following\n"
"                   form:                                                   \n"
"                   (Name, ValueType, Flags, Value1, Value2...)             \n"
"                       -Name      = Resource Property Name                 \n"
"                       -ValueType = TI (Integer)                           \n"
"                                  = TU (Unsigned Integer)                  \n"
"                                  = TB (Boolean)                           \n"
"                                  = TS (String)                            \n"
"                       -Flags     = Hexadecimal Flag Value                 \n"
"                       -Value     = Comma separated values                 \n"
"    Examples:                                                              \n"
"                                                                           \n"
"    ResourceAttributesSample -cap S-1-17-11 -file C:\\test.txt             \n"
"    ResourceAttributesSample -cap S-1-17-22 -ra (\"Int\",TI,0x0,123)       \n"
"                                                                           \n"
           );
}
