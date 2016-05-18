/**************************************************************************
   THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
   PARTICULAR PURPOSE.

   Copyright 2001 Microsoft Corporation.  All Rights Reserved.
**************************************************************************/

/**************************************************************************

   File:          TSFApp.cpp

   Description:   Main Windows App Implementation

**************************************************************************/

/**************************************************************************
   Include Files
**************************************************************************/

#include "TSFWnd.h"

/**************************************************************************
   Local Function Prototypes
**************************************************************************/

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

/**************************************************************************
   Global Variables
**************************************************************************/

ITfThreadMgr    *g_pThreadMgr;

/**************************************************************************

   WinMain()

**************************************************************************/

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nCmdShow)
{
    CTSFMainWnd *pMainWnd;
    int         nReturn = 0;

    CoInitialize(NULL);

    pMainWnd = new CTSFMainWnd(hInstance);

    if(NULL == pMainWnd)
    {
        return 0;
    }

    if(pMainWnd->Initialize(nCmdShow))
    {
        ITfKeystrokeMgr *pKeyMgr;
        ITfMessagePump  *pMsgPump;
        
        // get the keystroke manager interfce
        if(SUCCEEDED(g_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeyMgr)))
        {
            // get the message pump wrapper interface
            if(SUCCEEDED(g_pThreadMgr->QueryInterface(IID_ITfMessagePump, (void **)&pMsgPump)))
            {
                BOOL    fResult = TRUE;
                
                while(fResult)
                {
                    MSG     msg;
                    BOOL    fEaten;

                    /*
                    Get the next message in the queue. fResult receives FALSE if WM_QUIT is encountered
                    */
                    if(FAILED(pMsgPump->GetMessage(&msg, NULL, 0, 0, &fResult)))
                    {
                        fResult = FALSE;
                    }
                    else if(WM_KEYDOWN == msg.message)
                    {
                        // does an ime want it?
                        if (pKeyMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                            pKeyMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
                        {
                            continue;
                        }
                    }
                    else if(WM_KEYUP == msg.message)
                    {
                        // does an ime want it?
                        if (pKeyMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
                            pKeyMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
                        {
                            continue;
                        }
                    }

                    if(fResult)
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }

                    if(WM_QUIT == msg.message)
                    {
                        nReturn = (int)msg.wParam;
                    }
                }
                
                pMsgPump->Release();
            }
            
            pKeyMgr->Release();
        }
    }

    delete pMainWnd;

    CoUninitialize();

    return nReturn;
}

