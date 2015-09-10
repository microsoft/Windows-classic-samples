// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Utils.h"

//********************************************************************************************
// Function: FlushCurrentLine
//
// Description: Clears any input lingering in the STDIN buffer
//
//********************************************************************************************
void FlushCurrentLine()
{
    int i;
    while ((i = getc(stdin)) != EOF && i != '\n')
    {
        continue;
    }
}

//********************************************************************************************
// Function: GetInterfaceAndProfileName
//
// Description: Display the list of interfaces, and associated profile names, and get the user interested interface
// GUID and the profile name
//
//********************************************************************************************
HRESULT GetInterfaceAndProfileName(_Out_ GUID *intfGuid, _Out_writes_(profNameLen) LPWSTR profName, _In_ DWORD profNameLen)
{
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwCurVersion = 0;
    HANDLE hWlan = NULL;
    RPC_WSTR GuidString = NULL;
    
    int iIntf = 0;
    int iProfile = 0;
    
    //variables used for WlanEnumInterfaces
    WLAN_INTERFACE_INFO_LIST* pIfList = NULL;
    WLAN_INTERFACE_INFO* pIfInfo = NULL;

    //variables used for WlanGetProfileList
    PWLAN_PROFILE_INFO_LIST pProfileList = NULL;
    PWLAN_PROFILE_INFO pProfile = NULL;

    //open handle to WLAN
    dwRet = WlanOpenHandle(2, NULL, &dwCurVersion, &hWlan);
    if (dwRet == ERROR_SUCCESS)
    {
        //Get the list of interfaces for WLAN
        dwRet = WlanEnumInterfaces(hWlan, 0, &pIfList);
    }
    else
    {
        printf("Failed to open handle to WLAN. \n");
    }
    if (dwRet == ERROR_SUCCESS)
    {
        for (int i = 0; i < (int) pIfList->dwNumberOfItems; i++)
        {
            pIfInfo = reinterpret_cast<WLAN_INTERFACE_INFO *> (&(pIfList->InterfaceInfo[i]));
            dwRet = UuidToStringW(&(pIfInfo->InterfaceGuid), &GuidString);
            if (dwRet == RPC_S_OK)
            {
                wprintf(L"  Interface GUID [%lu]. %ws\n",i+1, GuidString);

                //Get list of profiles associated with this interface
                dwRet = WlanGetProfileList(hWlan, &(pIfInfo->InterfaceGuid), NULL, &pProfileList);
                
                if (dwRet != ERROR_SUCCESS)
                {
                    wprintf(L"WlanGetProfileList failed. \n");
                    break;
                }
                for (int j = 0; j < (int) pProfileList->dwNumberOfItems; j++)
                {
                    pProfile = reinterpret_cast<WLAN_PROFILE_INFO *> (&(pProfileList->ProfileInfo[j]));
                    wprintf(L"      Profile Name [%u]:  %ws\n", j+1, pProfile->strProfileName);
                }
            }
            else
            {
                wprintf(L"Cannot convert the GUID string.\n");
                break;
            }
        } 
        
        if (dwRet == ERROR_SUCCESS)
        {
            if ((pIfList->dwNumberOfItems == 0) || (pProfileList->dwNumberOfItems == 0))
            {
                dwRet = ERROR_NOT_FOUND; 
                wprintf(L"WLAN interface/profile not found! \n");
            }
            else
            {
                //User input for interested interface Guid and Profile name
                GetInterfaceIdAndProfileIndex((int) (pIfList->dwNumberOfItems), (int) (pProfileList->dwNumberOfItems), &iIntf, &iProfile); 
                //Get the interested Interface GUID and profile Name from the indices
                pIfInfo = reinterpret_cast<WLAN_INTERFACE_INFO *> (&(pIfList->InterfaceInfo[iIntf-1]));
                *intfGuid = pIfInfo -> InterfaceGuid;
                pProfile = reinterpret_cast<WLAN_PROFILE_INFO *> (&(pProfileList->ProfileInfo[iProfile-1]));
                wmemcpy_s(profName, profNameLen, pProfile -> strProfileName, wcslen(pProfile -> strProfileName));
                profName[profNameLen-1] = '\0';
            }
        }
    }
    else
    {
        wprintf(L"WlanEnumInterfaces failed. \n");
    }
    
    if (pProfileList != NULL)
    {
        //Free PWLAN_PROFILE_INFO_LIST
        WlanFreeMemory(pProfileList);
        pProfileList = NULL;
    }
    
    if (pIfList != NULL)
    {
        //Free WLAN_INTERFACE_INFO_LIST
        WlanFreeMemory(pIfList);
    }

    if (hWlan != NULL)
    {
        WlanCloseHandle(hWlan, NULL);
    }

    if (dwRet != ERROR_SUCCESS)
    {
        DisplayError(dwRet);
    }
    return HRESULT_FROM_WIN32(dwRet);
}

//********************************************************************************************
// Function: GetInterfaceIdAndProfileIndex
//
// Description: Choose indices for Interface Guids and Profile names  displayed
//
//********************************************************************************************

void GetInterfaceIdAndProfileIndex(_In_ int numIntfItems, _In_ int numProfNames, _Out_ int *pIntf, _Out_ int *pProfile)
{
    //Get the interested interface GUID and profile name Indices
    wprintf(L"The list of interfaces and profiles for each interface are listed as above.\n");
    do
    {
        wprintf(L"Choose an index for Interface GUID in the range [ 1 -  %u ]: ", numIntfItems);
        wscanf_s(L"%i", pIntf);
        FlushCurrentLine();
    }while (((*pIntf) < 1)|| ((*pIntf) > numIntfItems));
    
    do
    {
        wprintf(L"Choose an index for Profile Name in the range [ 1 -  %u ] :  ", numProfNames);
        wscanf_s(L"%i", pProfile);    
        FlushCurrentLine();
    }while (((*pProfile) < 1)|| ((*pProfile) > numProfNames));   
}  

//********************************************************************************************
// Function: DisplayCost
//
// Description: Displays meaningful cost values to the user
//
//********************************************************************************************

void DisplayCostDescription (_In_ DWORD cost)
{
    if (cost == WCM_CONNECTION_COST_UNKNOWN)
    {
        wprintf(L"Cost                  : Unknown\n");
    }    
    else if (cost & WCM_CONNECTION_COST_UNRESTRICTED)
    {
        wprintf(L"Cost                  : Unrestricted\n");
    }    
    else if (cost & WCM_CONNECTION_COST_FIXED)
    {
        wprintf(L"Cost                  : Fixed\n");
    }    
    else if (cost & WCM_CONNECTION_COST_VARIABLE)
    {
        wprintf(L"Cost                  : Variable\n");
    }    
    if (cost & WCM_CONNECTION_COST_OVERDATALIMIT)
    {
        wprintf(L"OverDataLimit         : Yes\n");
    }
    else
    {
        wprintf(L"OverDataLimit         : No\n");
    }

    if (cost & WCM_CONNECTION_COST_APPROACHINGDATALIMIT)
    {
        wprintf(L"Approaching DataLimit : Yes\n");
    }
    else
    {
        wprintf(L"Approaching DataLimit : No\n");
    }
    
    if (cost & WCM_CONNECTION_COST_CONGESTED)
    {
        wprintf(L"Congested             : Yes\n");
    }
    else
    {
        wprintf(L"Congested             : No\n");
    }
    
    if (cost & WCM_CONNECTION_COST_ROAMING)
    {
        wprintf(L"Roaming               : Yes\n");
    }
    else
    {
        wprintf(L"Roaming               : No\n");
    }
}

//********************************************************************************************
// Function: DisplayCostSource
//
// Description: Displays cost source to the user
//
//********************************************************************************************

void DisplayCostSource (_In_ WCM_CONNECTION_COST_SOURCE costSource)
{
    if (costSource == WCM_CONNECTION_COST_SOURCE_GP)
    {
        wprintf(L"Cost Source is Group Policy\n");
    }
    
    else if (costSource == WCM_CONNECTION_COST_SOURCE_USER)
    {
        wprintf(L"Cost Source is User\n");
    }

    else if (costSource == WCM_CONNECTION_COST_SOURCE_OPERATOR)
    {
        wprintf(L"Cost Source is Operator\n");
    }

    else if (costSource == WCM_CONNECTION_COST_SOURCE_DEFAULT)
    {
        wprintf(L"Cost Source is Default\n");
    }
}


//********************************************************************************************
// Function: DisplayProfileData
//
// Description: Displays profile data values to the user
//
//********************************************************************************************

void DisplayProfileData (_In_ WCM_DATAPLAN_STATUS* pProfileData)
{
    if (FALSE == IsProfileDataAvailable(pProfileData))
    {
        wprintf(L"Profile Data Unknown\n");
    }
    else
    {  
        //check for default or unknown value
        if (pProfileData->UsageData.UsageInMegabytes != WCM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Data Usage in MB                  :   %d\n", pProfileData->UsageData.UsageInMegabytes);
        }

        if ((pProfileData->UsageData.LastSyncTime.dwHighDateTime != 0) || (pProfileData->UsageData.LastSyncTime.dwLowDateTime != 0))
        {
            wprintf(L"Last Sync Time                    :   ");
            PrintFileTime (pProfileData->UsageData.LastSyncTime);
        }
        if (pProfileData->DataLimitInMegabytes != WCM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Data Limit value in MB            :   %d\n", pProfileData->DataLimitInMegabytes);                             
        }
        if (pProfileData->InboundBandwidthInKbps != WCM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Inbound Bandwidth value in Kbps           :   %d\n", pProfileData->InboundBandwidthInKbps );
        }
        if (pProfileData->OutboundBandwidthInKbps != WCM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Outbound Bandwidth value in Kbps           :   %d\n", pProfileData->OutboundBandwidthInKbps );
        }
        if ((pProfileData->BillingCycle.StartDate.dwHighDateTime!= 0) || (pProfileData->BillingCycle.StartDate.dwLowDateTime != 0))
        {
            wprintf(L"Billing Cycle Start Date          :   ");
            PrintFileTime (pProfileData->BillingCycle.StartDate);
            if (IsProfilePlanDurationAvailable (pProfileData->BillingCycle.Duration))
            {
                wprintf(L"Billing Cycle Duration            :   \n");
                DisplayProfilePlanDuration (pProfileData->BillingCycle.Duration);
            }
            
        }
            
        if (pProfileData->MaxTransferSizeInMegabytes != WCM_UNKNOWN_DATAPLAN_STATUS)
        {
            wprintf(L"Maximum Transfer Size in MB  :   %d\n", pProfileData->MaxTransferSizeInMegabytes);
        }   
    }

}

//********************************************************************************************
// Function: IsProfilePlanDurationAvailable
//
// Description: Return TRUE if Profile Plan Duration is available
//
//********************************************************************************************
BOOL IsProfilePlanDurationAvailable (_In_ WCM_TIME_INTERVAL Duration)
{
    BOOL isAvailable = FALSE;
    if ((Duration.wYear > 0 )||
        (Duration.wMonth > 0) ||
        (Duration.wDay > 0) ||
        (Duration.wHour > 0) ||
        (Duration.wMinute > 0) ||
        (Duration.wSecond > 0) ||
        (Duration.wMilliseconds > 0))
    {
        isAvailable = TRUE;
    }
    return isAvailable;
}
        

//********************************************************************************************
// Function: DisplayProfilePlanDuration
//
// Description: Display the Profile Plan Duration 
//
//********************************************************************************************

void DisplayProfilePlanDuration (_In_ WCM_TIME_INTERVAL Duration)
{
        if (Duration.wYear > 0)
        {
            wprintf(L"Years                             :   %d\n", Duration.wYear);
        }
        
        if (Duration.wMonth > 0)
        {
            wprintf(L"Months                            :   %d\n", Duration.wMonth);
        }

        if (Duration.wDay > 0)
        {
            wprintf(L"Days                              :   %d\n", Duration.wDay);
        }

        if (Duration.wHour > 0)
        {
            wprintf(L"Hours                             :   %d\n", Duration.wHour);
        }

        if (Duration.wMinute > 0)
        {
            wprintf(L"Minutes                           :   %d\n", Duration.wMinute);
        }

        if (Duration.wSecond > 0)
        {
            wprintf(L"Seconds                           :   %d\n", Duration.wSecond);
        }

        if (Duration.wMilliseconds > 0)
        {
            wprintf(L"Milliseconds                      :   %d\n", Duration.wMilliseconds);
        }
        
}
        

//********************************************************************************************
// Function: PrintFileTime
//
// Description: Converts file time to local time, to display to the user
//
//********************************************************************************************

void PrintFileTime(_In_ FILETIME time)
{
    SYSTEMTIME stUTC, stLocal;

    // Convert filetime to local time.
    FileTimeToSystemTime(&time, &stUTC);
    SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);      
    wprintf(L"%02d/%02d/%d  %02d:%02d:%02d\n", 
           stLocal.wMonth, stLocal.wDay, stLocal.wYear,stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
}     


//********************************************************************************************
// Function: IsProfileDataAvailable
//
// Description: Checks if the profile data values are default values, or provided by the MNO
//
//********************************************************************************************
BOOL IsProfileDataAvailable(_In_ WCM_DATAPLAN_STATUS *pProfileData)
{
    BOOL isDefined = FALSE;
    //
    // usage data is valid only if both planUsage and lastUpdatedTime are valid
    //
    if (pProfileData->UsageData.UsageInMegabytes != WCM_UNKNOWN_DATAPLAN_STATUS
        && (pProfileData->UsageData.LastSyncTime.dwHighDateTime != 0 
            || pProfileData->UsageData.LastSyncTime.dwLowDateTime!= 0))
    {
        isDefined = TRUE;
    }
    else if (pProfileData->DataLimitInMegabytes != WCM_UNKNOWN_DATAPLAN_STATUS)
    {
        isDefined = TRUE;
    }
    else if (pProfileData->InboundBandwidthInKbps != WCM_UNKNOWN_DATAPLAN_STATUS)
    {
        isDefined = TRUE;
    }
    else if (pProfileData->OutboundBandwidthInKbps != WCM_UNKNOWN_DATAPLAN_STATUS)
    {
        isDefined = TRUE;
    }
    else if (pProfileData->BillingCycle.StartDate.dwHighDateTime != 0 
            || pProfileData->BillingCycle.StartDate.dwLowDateTime!= 0)
    {
        isDefined = TRUE;

    }
    else if (pProfileData->MaxTransferSizeInMegabytes!= WCM_UNKNOWN_DATAPLAN_STATUS)
    {
        isDefined = TRUE;
    }
    return isDefined;
}


//********************************************************************************************
// Function: DisplayError
//
// Description: Displays error description
//
//********************************************************************************************
void DisplayError(_In_ DWORD dwError)
{
    if (dwError == REGKEY_NOT_FOUND)
    {
        wprintf(L"Registry key not found \n");
    }
    else if (dwError == INVALID_PARAMETER)
    {
        wprintf(L"Invalid Parameter\n");
    }
    else
    {
        wprintf(L"Error code: %u\n", dwError);
    }
}
