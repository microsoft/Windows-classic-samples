// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/******************************************************************************
*   Common.h 
*       This module contains the definitions used by all modules in the
*       CoffeeShop0 application.         
******************************************************************************/

#define NORMAL_LOADSTRING  100                  // Normal size of loaded strings
#define MAX_LOADSTRING  256                     // Normal size of loaded strings
#define GRAMMARID1      161                     // Arbitrary grammar id
#define WM_RECOEVENT    WM_USER+190             // Arbitrary user defined message for reco callback
#define WM_GOTOCOUNTER      WM_USER+202         // Notification to go to counter pane
#define WM_INITPANE         WM_USER+203         // Notification for any pane to initialize
#define MY_RULE_ID      458                     // Arbitrary rule id
#define MAX_ID_ARRAY    7                       // Max number of ids in espresso rule
#define MINMAX_WIDTH    640                     // Window width
#define MINMAX_HEIGHT   480                     // Window height
#define TIMEOUT         12000                   // Timer fires on this interval in ms

typedef LRESULT (*PMSGHANDLER) (HWND, UINT, WPARAM, LPARAM );  // typedef msg handler

