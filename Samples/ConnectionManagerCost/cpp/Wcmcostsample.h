// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Utils.h"
//
// Get cost based on interface GUID and profile name
//
void WCMGetCost();

//
// Get Profile data based on interface GUID and profile name
//
void WCMGetProfileData();

//
// Set cost based on interface GUID and profile name
//
void WCMSetCost();

//
// Set Profile data based on interface GUID and profile name
//
void WCMSetProfileData();

//
// Evaulates the user choice and calls the appropriate function.
//
void EvaluateUserChoice(_In_ int userchoice);

//
// Presents an interactive menu to the user and calls  the function EvaluateUserChoice to implement user's choice
//
int GetUserChoice();
