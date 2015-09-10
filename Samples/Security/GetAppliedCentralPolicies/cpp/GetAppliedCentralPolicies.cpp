//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
// GetAppliedCentralPolicies.cpp
//
// A simple command line tool for retrieving the CAP IDs applied to a machine,
// then look up the details of those policies from Active Directory
//
//////////////////////////////////////////////////////////////////////////////

#pragma warning(disable: 28251)
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <sddl.h>
#include <AccCtrl.h>
#include <AclApi.h>
#include <NTSecAPI.h>

#include <objbase.h>
#include <atlbase.h>
#include <atlstr.h>

#include <iads.h>
#include <Adshlp.h>

LPCWSTR pszPolicyContainer =
    L"LDAP://CN=Central Access Policies,CN=Claims Configuration,CN=Services,CN=Configuration,%1";
LPCWSTR pszLdap = L"LDAP://%1";

_Success_(return == TRUE) 
BOOL 
GetCentralAccessPolicyIDs(
    _Outptr_result_buffer_(*Count) PSID*  CapIDs[],
    _Out_                          ULONG* Count
    );
BOOL 
GetNameForPolicyDirectory(
    _Out_ CAtlString* PolicyDir
    );
BOOL 
DisplayCentralAccessPolicyInformation(
    _In_reads_(Count) PSID* CapIDs,
    _In_                ULONG Count
    );
BOOL 
DisplayCentralAccessRuleInformation(
    _In_ LPWSTR Path
    );

int 
__cdecl 
wmain()
{
    HRESULT hr                  = S_OK;
    BOOL    Succeeded           = FALSE;
    BOOL    IsComInitialized    = FALSE;
    PSID*   AppliedCapIds       = NULL;
    ULONG   CapIdCount          = 0;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        printf("CoInitializeEx() Error 0x%08X\n", hr);
        goto Cleanup;
    }
    IsComInitialized = TRUE;

    if (!GetCentralAccessPolicyIDs(&AppliedCapIds, &CapIdCount))
    {
        goto Cleanup;
    }
    else if (CapIdCount < 1 || NULL == AppliedCapIds)
    {
        // No Central Access Policies were found
        printf("No Central Access Policies IDs were returned\n");
        goto Cleanup;
    }

    if(!DisplayCentralAccessPolicyInformation(AppliedCapIds, CapIdCount))
    {
        goto Cleanup;
    }

    Succeeded = TRUE;

Cleanup:
    if (NULL != AppliedCapIds)
    {
        for (ULONG i = 0; i < CapIdCount; ++i)
        {
            LsaFreeMemory(AppliedCapIds[i]);
        }
        LsaFreeMemory(AppliedCapIds);
    }
    if (IsComInitialized)
    {
        CoUninitialize();
    }

    return Succeeded ? EXIT_SUCCESS : EXIT_FAILURE;
}

_Success_(return == TRUE) 
BOOL
GetCentralAccessPolicyIDs(
    _Outptr_result_buffer_(*Count) PSID*  CapIDs[], // valid on success
    _Out_                          ULONG* Count)
{
    BOOL            Succeeded          = FALSE;
    NTSTATUS        Status;
    PSID*           AppliedCapIds   = NULL;
    ULONG           CapIdCount      = 0;

    if (NULL == CapIDs || NULL == Count)
    {
        printf("Invalid Argument\n");
        goto Cleanup;
    }
    *CapIDs = NULL;
    *Count = 0;

    // Get the applied CAP IDs on the machine
    Status = LsaGetAppliedCAPIDs(
                NULL, 
                &AppliedCapIds, 
                &CapIdCount);
    if (Status != 0)
    {
        printf("LsaGetAppliedCAPIDs Error 0x%08X\n", Status);
        goto Cleanup;
    }

    *CapIDs = AppliedCapIds;
    *Count = CapIdCount;
    AppliedCapIds = NULL;

    Succeeded = TRUE;

Cleanup:
    if (NULL != AppliedCapIds)
    {
        for (ULONG i = 0; i < CapIdCount; ++i)
        {
            LsaFreeMemory(AppliedCapIds[i]);
        }
        LsaFreeMemory(AppliedCapIds);
    }

    return Succeeded;
}

BOOL
GetNameForPolicyDirectory(
    _Out_ CAtlString* PolicyDir)
{
    HRESULT         hr                   = S_OK;
    BOOL            Succeeded            = FALSE;
    CAtlString      DistinguishedName;
    CComPtr<IADs>   PtrRootDse;
    CComVariant     NamingContext;
    CComBSTR        Context;

    if (NULL == PolicyDir)
    {
        printf("Invalid Argument\n");
        goto Cleanup;
    }
    *PolicyDir = L"";

    // Get the rootDSE for Active Directory
    hr = ADsGetObject(L"LDAP://rootDSE", IID_IADs, (VOID**)&PtrRootDse);
    if (FAILED(hr))
    {
        printf("ADsGetObject Error 0x%08X\n", hr);
        goto Cleanup;
    }

    try
    {
        Context = CComBSTR(L"defaultNamingContext");
    }
    catch (const CAtlException &e)
    {
        printf ("Memory allocation error 0x%08X\n", e.m_hr);
        goto Cleanup;
    }

    // Get the defaultNamingContext (where the policies live)
    hr = PtrRootDse->Get(Context, &NamingContext);
    if (FAILED(hr))
    {
        printf("IADs->Get(\"defaultNamingContext\") Error 0x%08X\n", hr);
        goto Cleanup;
    }

    if (VT_BSTR != NamingContext.vt)
    {
        printf("IADs->Get(\"defaultNamingContext\") returned non-string type\n");
        goto Cleanup;
    }

    try
    {
        // Construct the distinguished name for the CAP container
        DistinguishedName.FormatMessage(
            pszPolicyContainer, 
            NamingContext.bstrVal);
    }
    catch (const CAtlException &e)
    {
        printf ("Memory allocation error 0x%08X\n", e.m_hr);
        goto Cleanup;
    }

    *PolicyDir = DistinguishedName.GetBuffer();
    Succeeded = TRUE;

Cleanup:

    return Succeeded;
}

BOOL
DisplayCentralAccessPolicyInformation(
    _In_reads_(Count) PSID* CapIDs,
    _In_              ULONG Count)
{
    HRESULT                 hr              = S_OK;
    BOOL                    Succeeded       = FALSE;
    CAtlString              PolicyDir;
    CComPtr<IADsContainer>  PtrContainer;
    CComPtr<IEnumVARIANT>   PtrPolicyEnum;
    ULONG                   Result;
    LONG                    Start           = 0;
    LONG                    End             = 0;
    SIZE_T                  NumBytes        = 0;
    BOOL                    IsAppliedPolicy = FALSE;
    SAFEARRAY*              SafeArray       = NULL;
    VOID*                   ByteArray;
    CComBSTR                ID;
    CComBSTR                CN;
    CComBSTR                Description;
    CComBSTR                Rules;

    // Get the full distinguished name for the Central Access Policy container
    if (!GetNameForPolicyDirectory(&PolicyDir))
    {
        goto Cleanup;
    }

    // Bind to the Active Directory container for Central Access Policies
    hr = ADsGetObject(PolicyDir, IID_IADsContainer, (void**) &PtrContainer);
    if (FAILED(hr))
    {
        printf("ADsGetObject Error 0x%08X\n", hr);
        goto Cleanup;
    }

    // Create an enumerator to iterate over the policies in the container
    hr = ADsBuildEnumerator(PtrContainer, &PtrPolicyEnum);
    if (FAILED(hr))
    {
        printf("ADsBuildEnumerator Error 0x%08X\n", hr);
        goto Cleanup;
    }

    try
    {
        ID = CComBSTR(L"msAuthz-CentralAccessPolicyID");
        CN = CComBSTR(L"cn");
        Description = CComBSTR(L"description");
        Rules = CComBSTR(L"msAuthz-MemberRulesInCentralAccessPolicy");
    }
    catch (const CAtlException &e)
    {
        printf ("Memory allocation error 0x%08X\n", e.m_hr);
        goto Cleanup;
    }

    while (TRUE)
    {
        CComPtr<IDispatch> PtrDisp;
        CComPtr<IADs> PtrPolicy;
        CComVariant CapVariant;
        CComVariant IDVariant;
        CComVariant NameVariant;
        CComVariant DescriptionVariant;
        CComVariant RulesVariant;
        CComVariant RuleVariant;

        SafeArray       = NULL;
        IsAppliedPolicy = FALSE;
        Start           = 0;
        End             = 0;
        NumBytes        = 0;

        // Get the next policy
        hr = ADsEnumerateNext(PtrPolicyEnum, 1, &CapVariant, &Result);
        if (FAILED(hr))
        {
            printf("IEnumVARIANT->ADsEnumerateNext Error 0x%08X\n", hr);
            goto Cleanup;
        }
        else if (S_FALSE == hr)
        {
            // Finished iterating over policies
            break;
        }

        // Get the IDispatch pointer to query for the Active Directory object
        PtrDisp = CapVariant.pdispVal;
        if (PtrDisp == NULL)
        {
            printf("ADsEnumerateNext returned a null IDispatch Pointer\n");
            goto Cleanup;
        }

        // Get the actual Central Access Policy object from Active Directory
        hr = PtrDisp->QueryInterface(IID_IADs, (LPVOID *)&PtrPolicy);
        if (FAILED(hr))
        {
            printf("IDispatch->QueryInterface Error 0x%08X\n", hr);
            goto Cleanup;
        }

        //
        // Get the ID of the policy we're looking at and determine if the SID
        // matches one of the SIDs returned from LsaGetAppliedCAPIDs
        //

        // Get the CAP ID
        hr = PtrPolicy->Get(ID, &IDVariant);
        if (FAILED(hr))
        {
            // If we can't get the ID, then continue on to the next policy.
            continue;
        }

        // The CAP ID SID is stored as a byte array
        SafeArray = V_ARRAY(&IDVariant);

        // Get the lower bound for the byte array
        hr = SafeArrayGetLBound(SafeArray, 1, &Start);
        if (FAILED(hr))
        {
            printf("SafeArrayGetLBound Error 0x%08X\n", hr);
            goto Cleanup;
        }

        // Get the upper bound for the byte array
        hr = SafeArrayGetUBound(SafeArray, 1, &End);
        if (FAILED(hr))
        {
            printf("SafeArrayGetUBound Error 0x%08X\n", hr);
            goto Cleanup;
        }

        if (Start > End)
        {
            printf("Invalid SID array encountered\n");
            goto Cleanup;
        }
        NumBytes = static_cast<SIZE_T>(End - Start);

        // Iterate over the array of applied CAP IDs and check for a match
        for (ULONG i = 0; i < Count; ++i)
        {
            ByteArray = NULL;
            hr = SafeArrayPtrOfIndex(SafeArray, &Start, &ByteArray);
            if (FAILED(hr))
            {
                printf("SafeArrayPtrOfIndex Error 0x%08X\n", hr);
                goto Cleanup;
            }

            if (RtlEqualMemory(ByteArray,CapIDs[i],NumBytes))
            {
                // Found a matching SID
                IsAppliedPolicy = TRUE;
                break;
            }
        }
        if (!IsAppliedPolicy)
        {
            // The CAP ID didn't match any of the applied CAP IDs
            continue;
        }

        printf("\n -------------------------------------------------------\n");
        printf( "|                 Central Access Policy                 |");
        printf("\n -------------------------------------------------------\n");

        // Get the policy name
        hr = PtrPolicy->Get(CN, &NameVariant);
        if (FAILED(hr))
        {
            printf("IADs->Get failed to retrieve cn with error 0x%08X\n", hr);
            goto Cleanup;
        }
        else if (NameVariant.vt == VT_BSTR)
        {
            printf("\n Name       : %S:\n", V_BSTR(&NameVariant));
        }

        // Get the policy description
        hr = PtrPolicy->Get(Description, &DescriptionVariant);
        if (SUCCEEDED(hr))
        {
            if (DescriptionVariant.vt == VT_BSTR)
            {
                // The description is optional
                printf(" Description: %S\n", V_BSTR(&DescriptionVariant));
            }
        }

        // Get the rules associated with the policy
        hr = PtrPolicy->Get(Rules, &RulesVariant);
        if (FAILED(hr))
        {
            printf("IADs->Get Error 0x%08X\n", hr);
            goto Cleanup;
        }

        printf("\n Rules:\n");
        printf("    ------------------------------------------------------\n");

        //
        // Retrieve the Central Access Rules from Active Directory
        //
        if (RulesVariant.vt & VT_ARRAY)
        {
            // There's an array of Central Access Rules
            SafeArray = V_ARRAY(&RulesVariant);

            // Get the lower bound for the byte array
            hr = SafeArrayGetLBound(SafeArray, 1, &Start);
            if (FAILED(hr))
            {
                printf("SafeArrayGetLBound Error 0x%08X\n", hr);
                goto Cleanup;
            }

            // Get the upper bound for the byte array
            hr = SafeArrayGetUBound(SafeArray, 1, &End);
            if (FAILED(hr))
            {
                printf("SafeArrayGetUBound Error 0x%08X\n", hr);
                goto Cleanup;
            }

            // Print out each Central Access Rule in the array.
            for (LONG idx = Start; idx <= End; ++idx)
            {
                hr = SafeArrayGetElement(SafeArray, &idx, &RuleVariant);
                if (FAILED(hr))
                {
                    printf("SafeArrayGetElement Error 0x%08X\n", hr);
                    goto Cleanup;
                }
                else if (RuleVariant.vt == VT_BSTR)
                {
                    DisplayCentralAccessRuleInformation(V_BSTR(&RuleVariant));
                }
                else
                {
                    printf("Cannot query linked Central Access Rule with non-string type/n");
                }
            }
        }
        else if (RulesVariant.vt == VT_BSTR)
        {
            // Single Central Access Rule
            DisplayCentralAccessRuleInformation(V_BSTR(&RulesVariant));
        }
        else
        {
            printf("Couldn't find any linked Central Access Rules\n");
        }
    }

    Succeeded = TRUE;

Cleanup:

    return Succeeded;
}

BOOL 
DisplayCentralAccessRuleInformation(
    _In_ LPWSTR Path)
{
    BOOL            Succeeded       = FALSE;
    HRESULT         hr              = S_OK;
    CAtlString      Rule;
    CComPtr<IADs>   PtrRule;
    CComVariant     NameVariant;
    CComVariant     DescriptionVariant;
    CComVariant     ConditionVariant;
    CComVariant     EffectivePolicyVariant;
    CComVariant     ProposedPolicyVariant;
    CComBSTR        CN;
    CComBSTR        Description;
    CComBSTR        Condition;
    CComBSTR        EffectivePolicy;
    CComBSTR        ProposedPolicy;

    if (NULL == Path)
    {
        printf("Null path for Central Access Rule encountered\n");
        goto Cleanup;
    }

    try
    {
        CN              = CComBSTR(L"cn");
        Description     = CComBSTR(L"description");
        Condition       = CComBSTR(L"msAuthz-ResourceCondition");
        EffectivePolicy = CComBSTR(L"msAuthz-EffectiveSecurityPolicy");
        ProposedPolicy  = CComBSTR(L"msAuthz-ProposedSecurityPolicy");
        Rule.FormatMessage(pszLdap, Path);
    }
    catch (const CAtlException &e)
    {
        printf ("Memory allocation error 0x%08X\n", e.m_hr);
        goto Cleanup;
    }

    hr = ADsGetObject(Rule, IID_IADs, (void**) &PtrRule);
    if (FAILED(hr))
    {
        printf("ADsGetObject Error 0x%08X\n", hr);
        goto Cleanup;
    }

    // Get the Central Access Rule name
    hr = PtrRule->Get(CN, &NameVariant);
    if (FAILED(hr))
    {
        printf("IADs->Get Error 0x%08X\n", hr);
        goto Cleanup;
    }
    else if (NameVariant.vt == VT_BSTR)
    {
        printf("     Name            : %S\n", V_BSTR(&NameVariant));
    }

    // Get the description
    hr = PtrRule->Get(Description, &DescriptionVariant);
    if (SUCCEEDED(hr))
    {
        if (DescriptionVariant.vt == VT_BSTR)
        {
            // Description is optional
            printf("     Description     : %S\n", V_BSTR(&DescriptionVariant));
        }
    }

    // Get the resource condition
    PtrRule->Get(Condition, &ConditionVariant);
    if (SUCCEEDED(hr))
    {
        if (ConditionVariant.vt == VT_BSTR)
        {
            // Resource condition is optional
            printf("     Condition       : %S\n", V_BSTR(&ConditionVariant));
        }
    }

    // Get the security descriptor
    hr = PtrRule->Get(EffectivePolicy, &EffectivePolicyVariant);
    if (FAILED(hr))
    {
        printf("IADs->Get Error 0x%08X\n", hr);
        goto Cleanup;
    }
    else if (EffectivePolicyVariant.vt == VT_BSTR)
    {
        printf("     Effective Policy: %S\n", V_BSTR(&EffectivePolicyVariant));
    }

    // Get the proposed policy
    hr = PtrRule->Get(ProposedPolicy, &ProposedPolicyVariant);
    if (SUCCEEDED(hr))
    {
        if (ProposedPolicyVariant.vt == VT_BSTR)
        {
            // Proposed policy is optional
            printf("     Proposed Policy : %S\n", V_BSTR(&ProposedPolicyVariant));
        }
    }

    printf("    ------------------------------------------------------\n");

    Succeeded = TRUE;

Cleanup:

    return Succeeded;
}
