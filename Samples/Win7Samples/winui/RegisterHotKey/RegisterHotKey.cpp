//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
//  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (C) 2008  Microsoft Corporation.  All rights reserved.
//
//          The following example shows how to use the RegisterHotKey function with the MOD_NOREPEAT flag. 
//          In this example, the hotkey 'ALT+b' is registered for the main thread. When the hotkey is pressed, 
//          the thread will receive a WM_HOTKEY message, which will get picked up in the GetMessage call. 
//          Because this example uses MOD_ALT with the MOD_NOREPEAT value for fsModifiers, the thread will 
//          only receive another WM_HOTKEY message when the 'b' key is released and then pressed again while 
//          the 'ALT' key is being pressed down. 
//

#include <windows.h>
#include <stdio.h>

int wmain(int argc,  wchar_t *argv[])
{           
    if (RegisterHotKey(NULL, 1, MOD_ALT | MOD_NOREPEAT, 0x42))    //0x42 is 'b'
    {
        wprintf(L"Hotkey 'alt+b' registered, using MOD_NOREPEAT flag\n");
    }
 
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            wprintf(L"WM_HOTKEY received\n");
        }
    } 
 
    return 0;
}