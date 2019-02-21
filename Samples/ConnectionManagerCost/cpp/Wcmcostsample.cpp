// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "WcmCostSample.h"
#include <new>

//********************************************************************************************
// Function: WCMGetCost
//
// Description: Get cost based on interface GUID and profile name
//
//********************************************************************************************
void WCMGetCost()
{
    GUID interfaceGUID = {0};    
    WCHAR profileName[WLAN_MAX_NAME_LENGTH+1];
    DWORD dwRetCode = NO_ERROR;
    DWORD dwSize = 0;
    WCM_CONNECTION_COST_DATA *data = 0;
    PBYTE pData = NULL;
    HRESULT hr = S_OK;

    ZeroMemory(profileName, WLAN_MAX_NAME_LENGTH*sizeof(WCHAR));
    //Get interface GUID and profile name
    hr = GetInterfaceAndProfileName(&interfaceGUID, profileName, WLAN_MAX_NAME_LENGTH);
    if (hr == S_OK)
    {
        //Get Cost using WcmQueryProperty
        dwRetCode = WcmQueryProperty(&interfaceGUID,
                                    profileName, 
                                    wcm_intf_property_connection_cost, 
                                    NULL, 
                                    &dwSize, 
                                    &pData);
            
        if (dwRetCode == ERROR_SUCCESS)
        {
            if (pData == NULL)
            {
                wprintf(L"Cost data not available\n");
            }
            else
            {
                data = reinterpret_cast<WCM_CONNECTION_COST_DATA*> (pData);
                wprintf(L"Cost is: %d\n", (data->ConnectionCost));

                //Display meaningful cost description
                DisplayCostDescription (data->ConnectionCost);
                DisplayCostSource (data->CostSource);
                WcmFreeMemory(pData);
            }
        }
        else
        {
            wprintf(L"WcmQueryProperty failed\n");
            //Error handling
            DisplayError(dwRetCode);
        }
    }
}

    
//********************************************************************************************
// Function: WCMGetProfileData
//
// Description: Get Profile data based on interface GUID and profile name
//
//********************************************************************************************
void WCMGetProfileData()
{
    GUID interfaceGUID = {0};    
    WCHAR profileName[WLAN_MAX_NAME_LENGTH+1];
    DWORD dwRetCode = NO_ERROR;
    DWORD dwSize = 0;
    PBYTE pData = NULL;
    HRESULT hr = S_OK;
    
    ZeroMemory(profileName, WLAN_MAX_NAME_LENGTH*sizeof(WCHAR));
    //Get interface GUID and profile name
    hr = GetInterfaceAndProfileName(&interfaceGUID, profileName, WLAN_MAX_NAME_LENGTH);
    if (hr == S_OK)
    {
        //Get Profile Data using WcmQueryProperty             
        dwRetCode = WcmQueryProperty(&interfaceGUID, 
                                    profileName, 
                                    wcm_intf_property_dataplan_status, 
                                    NULL, 
                                    &dwSize, 
                                    &pData);
                
        if ((dwRetCode == ERROR_SUCCESS) && (pData != NULL))
        {
            WCM_DATAPLAN_STATUS * data = reinterpret_cast<WCM_DATAPLAN_STATUS*> (pData);
            DisplayProfileData(data);
            WcmFreeMemory(pData);
        }
        else
        {
            wprintf(L"WcmQueryProperty failed.\n");
            //Error handling
            DisplayError(dwRetCode);
        }
    }
}


//********************************************************************************************
// Function: WCMSetCost
//
// Description: Set cost based on interface GUID and profile name
//
//********************************************************************************************
void WCMSetCost()
{
    DWORD dwNewCost = 0;
    GUID interfaceGUID = {0};    
    WCHAR profileName[WLAN_MAX_NAME_LENGTH+1];
    DWORD dwRetCode = NO_ERROR;
    DWORD dwSize = 0;
    PBYTE pData = NULL;
    HRESULT hr = S_OK;
    WCM_CONNECTION_COST_DATA wcmCostData = {0};
    
    ZeroMemory(profileName, WLAN_MAX_NAME_LENGTH*sizeof(WCHAR));
    //Get interface GUID and profile name
    hr = GetInterfaceAndProfileName(&interfaceGUID, profileName, WLAN_MAX_NAME_LENGTH);

    if (hr == S_OK)
    {
        wprintf(L"Enter the new cost:\n");
        wscanf_s(L"%u",&dwNewCost);
        FlushCurrentLine();
        
        wcmCostData.ConnectionCost = dwNewCost;
        pData = (PBYTE)&wcmCostData;
        dwSize = sizeof(wcmCostData);
        
        //Set cost using WcmSetProperty             
        dwRetCode = WcmSetProperty(&interfaceGUID,
                                   profileName,
                                   wcm_intf_property_connection_cost, 
                                   NULL,
                                   dwSize, 
                                   pData
                                   );
        if (dwRetCode == ERROR_SUCCESS)
        {        
            wprintf(L"Cost set successfully\n");
        }
        else
        {
            wprintf(L"WcmSetProperty failed \n");
            //Error handling
            DisplayError(dwRetCode);
        }
    }
}

//********************************************************************************************
// Function: WCMSetProfileData
//
// Description: Set Profile data based on interface GUID and profile name
//
//********************************************************************************************
void WCMSetProfileData()
{
    GUID interfaceGUID = {0};    
    WCHAR profileName[WLAN_MAX_NAME_LENGTH+1];
    DWORD dwRetCode = NO_ERROR;
    DWORD dwSize = 0;
    DWORD profUsageInMegabytes = 0;
    DWORD profDataLimitInMegabytes = 0;
    DWORD profInboundBandwidthInKbps = 0;
    DWORD profOutboundBandwidthInKbps = 0;
    DWORD profMaxTransferSizeInMegabytes = 0;
    PBYTE pData = NULL;
    SYSTEMTIME currentTime = {0};
    HRESULT hr = S_OK;
    
    ZeroMemory(profileName, WLAN_MAX_NAME_LENGTH*sizeof(WCHAR));
    //Get interface GUID and profile name
    hr = GetInterfaceAndProfileName(&interfaceGUID, profileName, WLAN_MAX_NAME_LENGTH);

    if (hr == S_OK)
    {
        //initialize Profile data values
        WCM_DATAPLAN_STATUS wcmProfData = {0};
        wcmProfData.UsageData.UsageInMegabytes = WCM_UNKNOWN_DATAPLAN_STATUS;
        wcmProfData.DataLimitInMegabytes = WCM_UNKNOWN_DATAPLAN_STATUS;
        wcmProfData.InboundBandwidthInKbps = WCM_UNKNOWN_DATAPLAN_STATUS;
        wcmProfData.OutboundBandwidthInKbps = WCM_UNKNOWN_DATAPLAN_STATUS;
        wcmProfData.MaxTransferSizeInMegabytes = WCM_UNKNOWN_DATAPLAN_STATUS;

        //Set Profile data usage
        wprintf(L"Enter Profile Usage value in Megabytes:\n");
        wscanf_s(L"%u", &profUsageInMegabytes);
        FlushCurrentLine();
        wcmProfData.UsageData.UsageInMegabytes = profUsageInMegabytes;
        GetSystemTime(&currentTime);
        SystemTimeToFileTime(&currentTime, &wcmProfData.UsageData.LastSyncTime);

        //Set Profile cap value
        wprintf(L"Enter Profile Data Limit value n Megabytes:\n");
        wscanf_s(L"%u", &profDataLimitInMegabytes);
        FlushCurrentLine();
        wcmProfData.DataLimitInMegabytes = profDataLimitInMegabytes;

        
        //Set Profile speed value
        wprintf(L"Enter Profile Inbound Bandwidth value in Kbps:\n");
        wscanf_s(L"%u", &profInboundBandwidthInKbps);
        FlushCurrentLine();
        wcmProfData.InboundBandwidthInKbps = profInboundBandwidthInKbps;

        wprintf(L"Enter Profile Outbound Bandwidth value in Kbps:\n");
        wscanf_s(L"%u", &profOutboundBandwidthInKbps);
        FlushCurrentLine();
        wcmProfData.OutboundBandwidthInKbps = profOutboundBandwidthInKbps;

        //Set Profile speed value
        wprintf(L"Enter Profile Max Transfer Size in Megabytes:\n");
        wscanf_s(L"%u", &profMaxTransferSizeInMegabytes);
        FlushCurrentLine();
        wcmProfData.MaxTransferSizeInMegabytes = profMaxTransferSizeInMegabytes;
  
        dwSize = sizeof(wcmProfData);
        pData = (PBYTE)&wcmProfData;
        
        //Set Profile Data using WcmSetProperty             
        dwRetCode = WcmSetProperty(&interfaceGUID,
                                   profileName,
                                   wcm_intf_property_dataplan_status, 
                                   NULL,
                                   dwSize, 
                                   pData
                                   );
        
        if (dwRetCode == ERROR_SUCCESS)
        {        
            wprintf(L"Profile Data set successfully\n");
        }
        else
        {
            wprintf(L"WcmSetProperty failed\n");
            //Error handling
            DisplayError(dwRetCode);
        }
    }
}

//********************************************************************************************
// Function: EvaluateUserChoice
//
// Description: Evaulates the user choice and calls the appropriate function.
//
//********************************************************************************************

void EvaluateUserChoice(_In_ int userchoice)
{
    switch (userchoice)
    {
        case 1:
            WCMGetCost();
            break;
        case 2:
            WCMGetProfileData();
            break;
        case 3:
            WCMSetCost();
            break;
        case 4:
            WCMSetProfileData();
            break;
        case 5:
            break;
        default:
            wprintf(L"Invalid choice. Please enter a valid choice number \n");
     }       
}     

//********************************************************************************************
// Function: GetUserChoice
//
// Description: Presents an interactive menu to the user and calls  the function EvaluateUserChoice to implement user's choice
//
// ********************************************************************************************

int GetUserChoice()
{
   int chr = -1;
   int numchoices = 5;
   PCWSTR choices[] = {
      L"Get Cost",
      L"Get Profile Data ",
      L"Set Cost",
      L"Set Profile Data",
      L"Exit" };
   wprintf(L"---------------------------------------------------------\n");
   
   for(int i=0; i<numchoices; i++)
   {
      wprintf(L"   %i. %ls\n",i+1,choices[i]);
   }
   
   wprintf(L"---------------------------------------------------------\n");
   wprintf(L"Enter a choice (1-%i): ",numchoices);
   wscanf_s(L"%i",&chr);
   FlushCurrentLine();
   if (chr > 0 && chr <= numchoices)
   {
      EvaluateUserChoice(chr);      
   }
   else
   {
      wprintf(L"Invalid Choice. Please enter a valid choice number\n");
   }
   return chr;
}


//********************************************************************************************
// Function: Main
//
// Description: The main function, calls GetUserChoice, to enable the user to play with the Sample SDK
//
//********************************************************************************************

int __cdecl main()
{
    int userChoice = -1;
    
    //Get user choice option to play with the WCM sample SDK
    while (userChoice != CHOICE_EXIT)
    {
        userChoice = GetUserChoice();
            
    }
    wprintf(L"WCM Sample SDK exited\n");
    return 0;
}
