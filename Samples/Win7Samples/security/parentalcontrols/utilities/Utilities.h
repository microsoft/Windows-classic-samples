
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    FILE: Windows Parental Controls utility function header file

    PURPOSE: Common functions for WMI support

    FUNCTIONS:

        WinMain() - calls initialization function, processes message loop
        DemoInit() - initializes window data and registers window
        DemoWndProc() - processes messages
        About() - processes messages for "About" dialog box

    COMMENTS:
        This code is a modified version of the CURSOR.C program.  Instead of
        using inline code for drawing the shape, the routines from the Select
        library are called.

****************************************************************************/

#pragma once


#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <string.h>
#include <wchar.h>
#include <oleauto.h>
#include <stdio.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <objbase.h>
#include <sddl.h>
#include <strsafe.h>
#include <wpc.h>

# pragma comment(lib, "wbemuuid.lib")



HRESULT WpcuCOMInit();
void WpcuCOMCleanup();

HRESULT WpcuSidStringFromUserName(PCWSTR pcszUserName, PWSTR* ppszSID);

HRESULT WpcuSidStringForCurrentUser(PWSTR* ppszSID);

HRESULT WpcuWmiConnect(PCWSTR pcszWpcNamespace, IWbemServices** ppiWmiServices);

HRESULT WpcuWmiObjectGet(IWbemServices* piWmiServices, PCWSTR pcszObjectPath,
                                      IWbemClassObject** ppiWmiSystemSettings);

HRESULT WpcuWmiInstancePutString(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                 PCWSTR pcszValue);
HRESULT WpcuWmiInstancePutStringArray(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                      DWORD dwNumElements, PCWSTR* pcszValue);
HRESULT WpcuWmiInstancePutDWORD(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                DWORD dwValue);
HRESULT WpcuWmiInstancePutDWORDArray(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                     DWORD dwNumElements, DWORD* dwValue);
HRESULT WpcuWmiInstancePutBOOL(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                               BOOL fValue);
HRESULT WpcuWmiInstancePutNULLVariant(IWbemClassObject* piInstance, PCWSTR pcszProperty);

HRESULT WpcuWmiStringFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                  PWSTR* ppszValue);
HRESULT WpcuWmiStringArrayFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                       DWORD* pdwNumElements, PWSTR** pppszValue);
HRESULT WpcuWmiDWORDFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                 DWORD* pdwValue);
HRESULT WpcuWmiDWORDArrayFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                 DWORD* pdwNumElements, DWORD** ppdwValue);
HRESULT WpcuWmiBOOLFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                BOOL* pfValue);


extern BOOL g_fCoInit;


