// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "NetCostEventSink.h"

//
// Presents an interactive menu to the user and returns the user's choice
//
int GetUserChoice();

//
// Evaluates user input and takes the requested action
//Takes the user option choice as the input
//
void EvaluateUserChoice(_In_ int userchoice);

//
//Registers for machine cost change notifications, and waits for cost change events
//
void RegisterForMachineCostChangeNotifications();

//
//Registers for destination cost change notifications, and waits for cost change events
//
void RegisterForDestinationCostChangeNotifications();

//
//Registers for connection cost change notifications, and waits for cost change events
//
void RegisterForConnectionCostChangeNotifications();

//
//Gets machine cost and data plan status, and displays to the user, along with suggested appropriate actions based on the 
//retrieved cost and data plan status values.
//
void GetMachineCostandDataPlanStatus();

//
//Gets destination based cost and data plan status, and displays to the user, along with suggested appropriate actions based on the 
//retrieved cost and data plan status values.
//
void GetDestinationCostandDataPlanStatus();

//
//Gets connection cost and data plan status, and displays to the user, along with suggested appropriate actions based on the 
//retrieved cost and data plan status values.
//
void GetConnectionCostandDataPlanStatus();

//
//Unregiser for machine wide notifications, and releases any handles used to release the memory
//
void UnRegisterForMachineCostChangeNotifications();

//
//Unregiser for destination based notifications, and releases any handles used to release the memory
//
void UnRegisterForDestinationCostChangeNotifications();

//
//Unregiser for connection cost notifications, and releases any handles used to release the memory
//
void UnRegisterForConnectionCostChangeNotifications();

//
// Description: Cancels registration for notifications, quits the thread and closes the thread handle
//
void FreeMemoryAndExit();

//
// Takes cost and data plan status as input, and suggests appropriate actions to the user based on the 
// cost and data plan status values.
//
void CostBasedSuggestions(_In_ DWORD cost, _In_ const NLM_DATAPLAN_STATUS *pDataPlanStatus);
