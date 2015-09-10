// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "NetCostSample.h"

//defining global instances for CNetCostEventSink
CNetCostEventSink *g_pSinkCostMgr = NULL;
CNetCostEventSink *g_pSinkConnectionCostMgr = NULL;
CNetCostEventSink *g_pSinkDestCostMgr = NULL;


//********************************************************************************************
// Function: RegisterForMachineCostChangeNotifications
//
// Description: Registers for machine cost change notifications, and waits in the message loop.
//
//********************************************************************************************

void RegisterForMachineCostChangeNotifications()
{
    HRESULT hr = S_OK;
    
    //Registration is allowed only once, before unregister.
    if (!g_pSinkCostMgr)
    {
        hr = CNetCostEventSink::StartListeningForEvents(IID_INetworkCostManagerEvents, NULL, &g_pSinkCostMgr);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Listening for Machine cost change events...\n");
        }
        else
        {
            wprintf(L"Registration failed, please try again. \n");
        }
    }
    else
    {
        wprintf(L"You have already registered for Machine cost notifications. Please unregister before registering for events again.\n");
        wprintf(L"The Win32 Cost API feature allows multiple registrations, but the sample SDK does not allow this.  \n");
    }
    DisplayError(hr);
}

//********************************************************************************************
// Function: RegisterForDestinationCostChangeNotifications
//
// Description: Registers for Destination cost change notifications, and waits in the message loop.
//
//********************************************************************************************

void RegisterForDestinationCostChangeNotifications()
{
    HRESULT hr = S_OK;
    DESTINATION_INFO sockAddr;

    hr = GetDestinationAddress(&sockAddr);
    if (hr == S_OK)
    {
        //Registration is allowed only once, before unregister.
        if (!g_pSinkDestCostMgr)
        {
            hr = CNetCostEventSink::StartListeningForEvents(IID_INetworkCostManagerEvents, &sockAddr, &g_pSinkDestCostMgr);
            if (SUCCEEDED(hr))
            {
                wprintf(L"Listening for Destination address based cost change events...\n");
            }
            else
            {
                wprintf(L"Registration failed, please try again. \n");
            }
        }
        else
        {
            wprintf(L"You have already registered for Destination cost notifications. Please unregister before registering for events again.\n");
            wprintf(L"The Win32 Cost API feature allows multiple registrations, but the sample SDK does not allow this.  \n");
            
        }
    }
    
    DisplayError(hr);
}

//********************************************************************************************
// Function: RegisterForConnectionCostChangeNotifications
//
// Description: Registers for connection cost change notifications, and waits in the message loop.
//
//********************************************************************************************
void RegisterForConnectionCostChangeNotifications()
{
    HRESULT hr = S_OK;
    
    //Registration is allowed only once, before unregister.
    if (!g_pSinkConnectionCostMgr)
    {
        hr = CNetCostEventSink::StartListeningForEvents(IID_INetworkConnectionCostEvents, NULL, &g_pSinkConnectionCostMgr);
        if (SUCCEEDED(hr))
        {
            wprintf(L"Listening for Connection cost change events...\n");
        }
    }
    else
    {
        wprintf(L"You have already registered for Connection cost notifications. Please unregister before registering for events again.\n");
        wprintf(L"The Win32 Cost API feature allows multiple registrations, but the sample SDK does not allow this.  \n");
    }
    DisplayError(hr);
}

//********************************************************************************************
// Function: GetMachineCostandDataPlanStatus
//
// Description: Gets machine cost and data plan status, and displays to the user, along with suggested appropriate actions based on the 
// retrieved cost and data plan status values.
//
//********************************************************************************************

void GetMachineCostandDataPlanStatus()
{
    HRESULT hr = S_OK;    
    DWORD cost = 0;
    NLM_DATAPLAN_STATUS dataPlanStatus;

    CComPtr<INetworkCostManager> pCostManager;
    hr = CoCreateInstance(CLSID_NetworkListManager, NULL, 
        CLSCTX_ALL, __uuidof(INetworkCostManager), (LPVOID*)&pCostManager);

    if (hr == S_OK)
    {
        hr = pCostManager->GetCost(&cost, NULL);  
    }
    
    if (hr == S_OK)
    {
        hr = pCostManager->GetDataPlanStatus(&dataPlanStatus, NULL);
    }
    
    if (hr == S_OK)
    {
        DisplayCostDescription(cost);
        DisplayDataPlanStatus(&dataPlanStatus);
        
        //to give suggestions for data usage, depending on cost and data usage.
        CostBasedSuggestions(cost, &dataPlanStatus);
    }
    DisplayError(hr);          
}
   

//********************************************************************************************
// Function: GetDestinationCostandDataPlanStatus
//
// Description: Gets cost and data plan status for the destination specified, and displays to the user, along with suggested appropriate actions based on the 
// retrieved cost and data plan status values.
//
//********************************************************************************************

void GetDestinationCostandDataPlanStatus()
{
    
    HRESULT hr = S_OK;    
    DWORD cost = 0;
    NLM_DATAPLAN_STATUS dataPlanStatus = {0};
    DESTINATION_INFO sockAddr;
    
    CComPtr<INetworkCostManager> pCostManager;
    hr = CoCreateInstance(CLSID_NetworkListManager, NULL, 
        CLSCTX_ALL, __uuidof(INetworkCostManager), (LPVOID*)&pCostManager);
    
    if (hr == S_OK)
    {
        hr = GetDestinationAddress(&sockAddr);
        if (hr == S_OK)
        {
            hr = pCostManager->GetCost(&cost, &(sockAddr.ipAddr));  
        }
        
        if (hr == S_OK)
        {    
            hr = pCostManager->GetDataPlanStatus(&dataPlanStatus, &(sockAddr.ipAddr));  
        }
        
        if (hr == S_OK)
        {
            DisplayCostDescription(cost);
            DisplayDataPlanStatus(&dataPlanStatus);
            //to give suggestions for data usage, depending on cost and data usage.
            CostBasedSuggestions(cost, &dataPlanStatus);
        }
    }
    DisplayError(hr);          
}        
        

    
//********************************************************************************************
// Function: GetConnectionCostandDataPlanStatus
//
// Description: Enumerates the connections and displays the cost and data plan status for each connection
//
//********************************************************************************************

void GetConnectionCostandDataPlanStatus()
{   
    HRESULT hr = S_OK;
    ULONG cFetched = 0;
    BOOL  bDone = FALSE;
    DWORD cost = 0;
    NLM_DATAPLAN_STATUS dataPlanStatus = {0};
    int numberOfConnections = 0;
    GUID interfaceGUID = {0};
    
    CComPtr<INetworkCostManager> pCostManager;
    CComPtr<INetworkListManager> pNLM;
    CComPtr<IEnumNetworkConnections> pNetworkConnections;
    hr = CoCreateInstance(CLSID_NetworkListManager, NULL, 
        CLSCTX_ALL, __uuidof(INetworkCostManager), (LPVOID*)&pCostManager);
    
    if (hr == S_OK)
    {
        hr = pCostManager->QueryInterface(IID_PPV_ARGS(&pNLM));
    }
    
    if (hr == S_OK)
    {
        hr = pNLM->GetNetworkConnections(&pNetworkConnections);  
    }
    
    if (hr == S_OK)
    {
        while (!bDone)
        {
            CComPtr<INetworkConnection> pConnection;
            CComPtr<INetworkConnectionCost> pConnectionCost;
            
            //Get cost and data plan status info for each of the connections on the machine
            hr = pNetworkConnections->Next(1, &pConnection, &cFetched);
            if ((S_OK == hr) && (cFetched > 0))
            {
                numberOfConnections ++;
                hr = pConnection->GetAdapterId(&interfaceGUID);
                wprintf(L"--------------------------------------------------------------\n");
                GetInterfaceType(interfaceGUID, hr);

                // get the connection interface
                hr = pConnection->QueryInterface(IID_PPV_ARGS(&pConnectionCost));  
                if (hr == S_OK)
                {
                    hr = pConnectionCost->GetCost(&cost);
                }
                if (hr == S_OK)
                {
                    hr = pConnectionCost->GetDataPlanStatus(&dataPlanStatus);  
                }
                if (hr == S_OK)
                {
                    DisplayCostDescription(cost);
                    DisplayDataPlanStatus(&dataPlanStatus);
                    
                    //to give suggestions for data usage, depending on cost and data usage.
                    CostBasedSuggestions(cost, &dataPlanStatus);     
                }
                DisplayError(hr);                                           
            }
            else
            {
                bDone = TRUE;
            }  
            
            if (numberOfConnections == 0)
            {
                wprintf(L"Machine has no network connection\n");
            }
        } 
        
    }
     
    if (hr != S_FALSE)
    {
        DisplayError (hr);
    }
}


//********************************************************************************************
// Function: UnRegisterForMachineCostChangeNotifications
//
// Description: Cancels registration for machine cost change notifications, and quits the listening thread
//
//********************************************************************************************


void UnRegisterForMachineCostChangeNotifications()
{
    HRESULT hr = S_OK;
    if (g_pSinkCostMgr)
    {
        hr = g_pSinkCostMgr->StopListeningForEvents();
        if (hr == S_OK)
        {
            wprintf(L"Successfully cancelled Registration for Machine Cost Notifications\n");
            g_pSinkCostMgr = NULL;
        }
        else
        {
            wprintf(L" Unregister failed, please try again.\n");
        }
    }
    else
    {
        wprintf(L"You have not registered for Machine cost Notifications\n");
    }
    DisplayError(hr);
}   
    

//********************************************************************************************
// Function: UnRegisterForDestinationCostChangeNotifications
//
// Description: 
//
//********************************************************************************************

void UnRegisterForDestinationCostChangeNotifications()
{
    HRESULT hr = S_OK;
    if (g_pSinkDestCostMgr)
    {
        hr = g_pSinkDestCostMgr->StopListeningForEvents();
        if (hr == S_OK)
        {
            wprintf(L"Successfully cancelled Registration for Destination Cost Notifications\n");
            g_pSinkDestCostMgr = NULL;
        }
        else
        {
            wprintf(L" Unregister failed, please try again.\n");
        }
    }
    else
    {
        wprintf(L"You have not registered for Destination cost Notifications\n");
    }
    DisplayError(hr);
}

//********************************************************************************************
// Function: UnRegisterForConnectionCostChangeNotifications
//
// Description: 
//
//********************************************************************************************

void UnRegisterForConnectionCostChangeNotifications()
{
    HRESULT hr = S_OK;
    if (g_pSinkConnectionCostMgr)
    {
        hr = g_pSinkConnectionCostMgr->StopListeningForEvents();
        if (hr == S_OK)
        {
            wprintf(L"Successfully cancelled Registration for Connection Cost Notifications\n");
            g_pSinkConnectionCostMgr = NULL;
        }
        else
        {
            wprintf(L" Unregister failed, please try again.\n");
        }
    }
    else
    {
        wprintf(L"You have not registered for Connection cost Notifications\n");
    }
    DisplayError(hr);
}

//********************************************************************************************
// Function: CostBasedSuggestions
//
// Description: Takes cost and data plan status as input, and suggests appropriate actions to the user based on the 
// cost and data plan status values.
//
//********************************************************************************************
      
void CostBasedSuggestions(_In_ DWORD cost, _In_ const NLM_DATAPLAN_STATUS *pDataPlanStatus)
{   
    if (cost == NLM_CONNECTION_COST_UNKNOWN)
    {
        wprintf(L"Cost value unknown\n");
        wprintf(L"Please register for cost change notifications, to receive cost change value. \n");
    }

    else if (cost & NLM_CONNECTION_COST_ROAMING)
    {
        wprintf(L"Connection is out of the MNO's network. Continuing data usage may lead to high charges, \
            so the application can try to to stop or limit its data usage, to avoid high charges \n");
    }
    
    else if (cost & NLM_CONNECTION_COST_OVERDATALIMIT)
    {
        if (cost & NLM_CONNECTION_COST_UNRESTRICTED)
        {
            wprintf(L"Plan data usage has exceeded the cap limit, but Cost type is unrestricted, so application can \
            continue its current data usage.\n");
        }
        else
        {
            wprintf(L"Plan data usage has exceeded the cap limit, so the application can limit or stop its current data usage, \
                and try again later, to prevent additional charges.\n");
        }
    }

    else if (cost & NLM_CONNECTION_COST_CONGESTED)
    {
        wprintf(L"Network is in a state of congestion, so the application can limit or stop its current data usage, \
            and try again later.\n");
    } 
    
    else if (cost & NLM_CONNECTION_COST_VARIABLE)
    {
        wprintf(L"Cost type is variable, and data usage charged on a per byte basis. The application can therefore \
                    try to limit or stop its current data usage\n");
    }
    else if (cost & NLM_CONNECTION_COST_FIXED)
    {
        if ((FALSE == IsDataPlanStatusAvailable(pDataPlanStatus)) || (NLM_UNKNOWN_DATAPLAN_STATUS == pDataPlanStatus->UsageData.UsageInMegabytes)
            || (NLM_UNKNOWN_DATAPLAN_STATUS == pDataPlanStatus->DataLimitInMegabytes))
        {
            // No access to data usage, to compare the data usage and the data limit
            wprintf(L"Cost type is Fixed. No access to data plan status, to compare the data usage as a percent of data limit.\n");
        }
        else if (cost & NLM_CONNECTION_COST_APPROACHINGDATALIMIT)
        {
            wprintf(L"Data usage is approaching the data limit, the application can limit its data usage \
                    to avoid high charges\n");
        }
        else
        {
            wprintf(L"Data usage is within limits, application can continue its current usage\n"); 
        }
    }
    else if (cost & NLM_CONNECTION_COST_UNRESTRICTED)
    {
        wprintf(L"Cost type is unrestricted, application can continue its current data usage\n");    
    }
    wprintf(L"--------------------------------------------------------------\n");
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
            RegisterForMachineCostChangeNotifications();
            break;
        case 2:
            RegisterForDestinationCostChangeNotifications();
            break;
        case 3:
            RegisterForConnectionCostChangeNotifications();
            break;
        case 4:
            GetMachineCostandDataPlanStatus();
            break;
        case 5:
            GetDestinationCostandDataPlanStatus();
            break;
        case 6:
            GetConnectionCostandDataPlanStatus();
            break;
        case 7:
            UnRegisterForMachineCostChangeNotifications();
            break;
        case 8:
            UnRegisterForDestinationCostChangeNotifications();
            break;
        case 9:
            UnRegisterForConnectionCostChangeNotifications();
            break;
        case 10:
            FreeMemoryAndExit();
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
   int numchoices = 10;
   PCWSTR choices[] = {
      L"Register for machine Internet cost notifications",
      L"Register for destination cost notifications",
      L"Register for connection cost notifications",
      L"Get machine wide cost and data plan status",
      L"Get destination address based cost and data plan status",
      L"Get connection cost and data plan status",
      L"Unregister for machine cost notifications",
      L"Unregister for destination cost notifications",
      L"Unregister for connection cost notifications",
      L"Exit" };
   wprintf(L"---------------------------------------------------------\n");
   for(int i=0; i<numchoices; i++)
   {
      wprintf(L"   %i. %s\n",i+1,choices[i]);
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
// Function: FreeMemoryAndExit
//
// Description: Cancels registration for notifications, quits the thread and closes the thread handle
//
//********************************************************************************************
void FreeMemoryAndExit()
{
    HRESULT hr = S_OK;
    if (g_pSinkCostMgr)
    {
        wprintf(L"Unregistering for machine cost change events..\n");
        hr = g_pSinkCostMgr->StopListeningForEvents();
    }
    if (g_pSinkConnectionCostMgr)
    {
        wprintf(L"Unregistering for Connection cost change events..\n");
        hr = g_pSinkConnectionCostMgr->StopListeningForEvents();
    }
    if (g_pSinkDestCostMgr)
    {
        wprintf(L"Unregistering for Destination address based cost change events..\n");
        hr = g_pSinkDestCostMgr->StopListeningForEvents();
    }
    DisplayError(hr);
}

//********************************************************************************************
// Function: Main
//
// Description: The main function, initializes a multithreaded apartment, and if successful, starts user interaction.
//
//********************************************************************************************

void __cdecl main()
{
    //Initialize COM on a multithreaded apartment
    HRESULT hrCoinit = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    int userChoice = -1;
    if (SUCCEEDED(hrCoinit) || (RPC_E_CHANGED_MODE == hrCoinit))
    {
        while (userChoice != CHOICE_EXIT)
        {
            userChoice = GetUserChoice();
        }
        wprintf(L"Net Cost Sample SDK exited\n");
        
        // When CoInitizlizeEx returns RPC_E_CHANGED_MODE, COM is already initialized. So, no need to uninitialize at the end.
        if (RPC_E_CHANGED_MODE != hrCoinit)
        {
            CoUninitialize();
        }
    }
    DisplayError(hrCoinit);
}

