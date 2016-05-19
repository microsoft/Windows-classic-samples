//////////////////////////////////////////////////////////////////////
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2003  Microsoft Corporation.  All rights reserved.
//
//  DllMain.cpp
//
//          DllMain module entry point.
//
//////////////////////////////////////////////////////////////////////

#include "Globals.h"
#include "PopupWindow.h"
#include "ExtentVisual.h"
#include "RangeExtent.h"
#include "RangeFromPoint.h"

//+---------------------------------------------------------------------------
//
// DllMain
//
//----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            g_hInst = hInstance;

            if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
                return FALSE;

            CExtentPopupWindow::StaticInit();
            CExtentVisualWindow::StaticInit();
            CRangeExtentViewer::StaticInit();
            CRangeFromPointViewer::StaticInit();

            break;

        case DLL_PROCESS_DETACH:

            DeleteCriticalSection(&g_cs);

            break;
    }

    return TRUE;
}
