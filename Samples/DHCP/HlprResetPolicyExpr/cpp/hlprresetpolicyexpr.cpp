// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope 10.0.0.0
*/

#include<stdio.h>
#include<windows.h>
#include<dhcpsapi.h>


#ifndef OPTION_USER_CLASS
#define OPTION_USER_CLASS 77
#endif


int __cdecl main(void)
{
    LPDHCP_POLICY       pPolicy                     = NULL;                 // Policy structure
    DWORD               dwError                     = ERROR_SUCCESS;        // It stores the error code
    DWORD               dwExprIdx                   = 0;                    // Expression Index
    DWORD               dwOptionId                  = OPTION_USER_CLASS;    // Option ID for UserClass
    DWORD               dwSubOptionId               = 0;                    // Sub Option ID for UserClass
    DWORD               dwConditionIdx              = 0;                    // Condition index which will be returned from DhcpHlprAddV4PolicyCondition
    DWORD               dwBytesLength1              = 0;                    // Number of bytes of user data in pUserClassCondValueInBytes1
    DWORD               dwBytesLength2              = 0;                    // Number of bytes of user data in pUserClassCondValueInBytes2
    DWORD               dwParentExpr                = 0;                    // Parent expression index
    LPBYTE              pUserClassCondValueInBytes1 = NULL;                 // Bytes containing condition value (user class based in the current example)
    LPBYTE              pUserClassCondValueInBytes2 = NULL;                 // Bytes containing condition value (user class based in the current example)
    DHCP_IP_ADDRESS     dwSubnet                    = 0;                    // Subnet Address
    LPWSTR              pwszName                    = L"testPolicy";        // Name of the policy
    LPWSTR              pwszDescription             = L"PolicyDescription"; // Policy description
    char*               szUserClassConditionValue1  = {"BOOTP.Microsoft"};  // Default BOOT class
    char*               szUserClassConditionValue2  = {"MSFT Quarantine"};  // Default Network Access Protection cclass
    DHCP_POL_LOGIC_OPER policyOperator              = DhcpLogicalOr;        // Root operator for the conditions and expressions
    DHCP_POL_LOGIC_OPER exprLogicalOperator         = DhcpLogicalAnd;       // Root operator for the conditions and expressions
    DHCP_POL_ATTR_TYPE  policyAttrType              = DhcpAttrOption;       // Policy attribute type
    DHCP_POL_COMPARATOR conditionOper               = DhcpCompEqual;        // Condition operator
    
    //Helper routine is invoked to create/fill the policy structure.
    dwError=DhcpHlprCreateV4Policy(
        pwszName,        // Policy Name
        (dwSubnet == 0), // fGloabalPolicy, if scope is zero, this means it is a global policy else it is for a specific scope
        dwSubnet,        // Scope
        0,               // Processing order
        policyOperator,  // Logical operator, possible values are: DhcpLogicalOr, DhcpLogicalAnd
        pwszDescription, // Policy description
        TRUE,            // Policy active or not
        &pPolicy         // This is the actual structure that holds the policy
    );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpHlprCreateV4Policy failed with Error = %d\n", dwError);
        goto cleanup;
    }
    // Fill in bytes and dwBytesLength
    dwBytesLength1 = (DWORD)strlen(szUserClassConditionValue1);
    pUserClassCondValueInBytes1= (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesLength1);
    if(NULL == pUserClassCondValueInBytes1)
    {
        wprintf(L"Not Enough memory, HeapAllocFailed !!\n");
        goto cleanup;
    }
    memcpy(pUserClassCondValueInBytes1, szUserClassConditionValue1, dwBytesLength1);
    dwError=DhcpHlprAddV4PolicyExpr(
        pPolicy,             // DHCP Policy
        dwParentExpr,        // Parent expression Index
        exprLogicalOperator, // Root operator for the expression
        &dwExprIdx           // expression index
    );
    if(ERROR != dwError)
    {
        wprintf(L"DhcpHlprAddV4PolicyExpr failed with Error = %d\n", dwError);
        goto cleanup;
    }

    // DhcpHlprAddV4PolicyCondition is invoked to add/fill the conditions for the policy 
    // This adds condition "UserClass equals "Default BOOTP Class"" to the policy.
    dwError = DhcpHlprAddV4PolicyCondition(
        pPolicy,                     // Policy where conditions need to be added
        dwExprIdx,                   // Parent expression index
        policyAttrType,              // Policy attribute type, possible values can be: DhcpAttrHWAddr, DhcpAttrOption and DhcpAttrSubOption
        dwOptionId,                  // Option ID
        dwSubOptionId,               // Sub Option ID
        NULL,                        // Vendor Name
        conditionOper,               // Policy comparator operator
        pUserClassCondValueInBytes1, // Condition values in bytes
        dwBytesLength1,              // Number of bytes in the condition value
        &dwConditionIdx              // Condition index
        );
    if(ERROR != dwError)
    {
        wprintf(L"DhcpHlprAddV4PolicyCondition failed with Error = %d\n", dwError);
        goto cleanup;
    }
    dwBytesLength2 = (DWORD)strlen(szUserClassConditionValue2);
    pUserClassCondValueInBytes2= (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesLength2);
    if(NULL == pUserClassCondValueInBytes2)
    {
        wprintf(L"Not Enough memory, HeapAllocFailed !!\n");
        goto cleanup;
    }
    memcpy(pUserClassCondValueInBytes2, szUserClassConditionValue2, dwBytesLength2);

    // DhcpHlprAddV4PolicyCondition is invoked to add/fill the conditions for the policy 
    // This will add another condition {UserClass equals "Default Network Access Protection Class"} to the expression.
    // The final expression will be  {"UserClass equals "Default BOOTP Class", "Default Network Access Protection Class"}
    dwError = DhcpHlprAddV4PolicyCondition(
        pPolicy,                     // Policy where conditions need to be added
        dwExprIdx,                   // Parent expression index
        policyAttrType,              // Policy attribute type, possible values can be: DhcpAttrHWAddr, DhcpAttrOption and DhcpAttrSubOption
        dwOptionId,                  // Option ID
        dwSubOptionId,               // Sub Option ID
        NULL,                        // Vendor Name
        conditionOper,               // Policy comparator operator
        pUserClassCondValueInBytes2, // Condition values in bytes
        dwBytesLength2,              // Number of bytes in the condition value
        &dwConditionIdx              // Condition index
        );
    if(ERROR != dwError)
    {
        wprintf(L"DhcpHlprAddV4PolicyCondition failed with Error = %d\n", dwError);
    }

    wprintf(L"Number of elements in policy expression (before reset) = %d\n",pPolicy->Expressions->NumElements);

    // Resets the policy expression
    dwError = DhcpHlprResetV4PolicyExpr(pPolicy);
    if(ERROR_SUCCESS != dwError){
        wprintf(L"DhcpHlprResetV4PolicyExpr failed with Error = %d\n",dwError);
    }else{
        wprintf(L"Number of elements in policy expression (after reset) = %d\n",pPolicy->Expressions->NumElements);
    }
cleanup:

    // Frees the variable holding the condition values in bytes
    if(NULL != pUserClassCondValueInBytes1)
    {
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pUserClassCondValueInBytes1);
        pUserClassCondValueInBytes1 = NULL;
    }
    
    // Frees the variable holding the condition values in bytes
    if(NULL != pUserClassCondValueInBytes2)
    {
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pUserClassCondValueInBytes2);
        pUserClassCondValueInBytes2 = NULL;
    }
    
    // Frees the policy structure
    if(NULL != pPolicy)
    {
        DhcpHlprFreeV4Policy(pPolicy);
    }
    return 0;
}

