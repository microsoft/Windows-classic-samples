// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*
Assumptions:
    1) There exists a scope address 10.0.0.0
*/

#include<windows.h>
#include<stdio.h>
#include<dhcpsapi.h>


#ifndef OPTION_USER_CLASS
#define OPTION_USER_CLASS 77
#endif

int __cdecl main(void)
{
    LPDHCP_POLICY       pPolicy                    = NULL;                 // Policy structure
    DWORD               dwError                    = ERROR_SUCCESS;        // It stores the error code
    DWORD               dwExprIdx                  = 0;                    // Expression Index
    DWORD               dwOptionId                 = OPTION_USER_CLASS;    // Option ID for UserClass
    DWORD               dwSubOptionId              = 0;                    // Sub Option ID for UserClass
    DWORD               dwConditionIdx             = 0;                    // Condition index which will be returned from DhcpHlprAddV4PolicyCondition
    DWORD               dwBytesLength              = 0;                    // Number of bytes of user data in pUserClassCondValueInBytes
    LPBYTE              pUserClassCondValueInBytes = NULL;                 // Bytes containing condition value (user class based in the current example)
    DHCP_IP_ADDRESS     dwSubnet                   = 0xa000000;            // Subnet Address (10.0.0.0)
    LPWSTR              pwszServer                 = NULL;                 // Server IP Address, NULL signifies the current server (on which the program is running).
    LPWSTR              pwszName                   = L"testPolicy";        //Name of the policy to be created
    LPWSTR              pwszDescription            = L"PolicyDescription"; // Description of the policy to be created
    char*               szUserClassConditionValue  = {"BOOTP.Microsoft"};  // Default BOOTP Class
    DHCP_POL_LOGIC_OPER policyOperator             = DhcpLogicalOr;        // Root operator for the conditions and expressions
    DHCP_POL_ATTR_TYPE  policyAttrType             = DhcpAttrOption;       // Policy attribute type
    DHCP_POL_COMPARATOR conditionOper              = DhcpCompEqual;        // Condition operator

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
    dwBytesLength = (DWORD)strlen(szUserClassConditionValue);
    pUserClassCondValueInBytes = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesLength);
    if(NULL == pUserClassCondValueInBytes)
    {
        wprintf(L"Not Enough memory, HeapAllocFailed !!\n");
        goto cleanup;
    }
    // Fill in bytes for szUserClassConditionValue
    memcpy(pUserClassCondValueInBytes, szUserClassConditionValue, dwBytesLength);

    // DhcpHlprAddV4PolicyCondition is invoked to add conditions for the policy.
    // The condition in this case is "user class" equals "Default BOOTP Class"
    dwError = DhcpHlprAddV4PolicyCondition(
        pPolicy,                    // Policy where conditions need to be added
        dwExprIdx,                  // Parent expression index
        policyAttrType,             // Policy attribute type, possible values can be: DhcpAttrHWAddr, DhcpAttrOption and DhcpAttrSubOption
        dwOptionId,                 // Option ID
        dwSubOptionId,              // Sub Option ID
        NULL,                       // Vendor Name
        conditionOper,              // Policy comparator operator
        pUserClassCondValueInBytes, // Condition values in bytes
        dwBytesLength,              // Number of bytes in the condition value
        &dwConditionIdx             // Condition index
        );
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpHlprAddV4PolicyCondition failed with Error = %d\n", dwError);
        goto cleanup;
    }

    // This is the actual RPC call to create policy on the server.
    // pPolicy is formed and filled by helper APIs and is fed to the RPC api for the policy to be created.
    dwError=DhcpV4CreatePolicy(pwszServer, pPolicy);
    if(ERROR_SUCCESS != dwError)
    {
        wprintf(L"DhcpV4CreatePolicy failed with Error = %d\n", dwError);
    }
cleanup:

    // Frees the variable that holds the bytes for condition values
    if(NULL != pUserClassCondValueInBytes)
    {
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, pUserClassCondValueInBytes);
        pUserClassCondValueInBytes = NULL;
    }

    //Frees the variable that holds the policy structure
    if(NULL != pPolicy)
    {
        DhcpHlprFreeV4Policy(pPolicy);
    }
    return 0;
}
