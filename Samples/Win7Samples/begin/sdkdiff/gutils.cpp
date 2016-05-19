// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "precomp.h"
#include "gutilsrc.h"

/* dll global data */
HANDLE hLibInst;
extern void gtab_init(void);
extern BOOL StatusInit(HANDLE);

BOOL WINAPI InitGutils(HANDLE hInstance, DWORD dwReason, LPVOID reserved)
{
        if (dwReason == DLL_PROCESS_ATTACH) {
                hLibInst = hInstance;
                gtab_init();
                StatusInit(hLibInst);
        }
        return(TRUE);
}
