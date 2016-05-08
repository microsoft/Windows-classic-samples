// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright 1993 - 2000 Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   dispatch.c
//
//  PURPOSE:  Implement the generic message and command dispatchers.
//    
//
//  FUNCTIONS:
//    DispMessage - Call the function associated with a message.
//    DispCommand - Call the function associated with a command.
//    DispDefault - Call the appropriate default window procedure.
//
//  COMMENTS:
//

#include "globals.h"            // prototypes specific to this application
#include <windows.h>            // required for all Windows applications
#include <windowsx.h>



LRESULT DispDefault(EDWP, HWND, UINT, WPARAM, LPARAM);

//
//  FUNCTION: DispMessage(LPMSDI, HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Call the function associated with a message.
//
//  PARAMETERS:
//    lpmsdi - Structure containing the message dispatch information.
//    hwnd - The window handle
//    uMessage - The message number
//    wparam - Message specific data
//    lparam - Message specific data
//
//  RETURN VALUE:
//    The value returned by the message function that was called.
//
//  COMMENTS:
//    Runs the table of messages stored in lpmsdi->rgmsd searching
//    for a message number that matches uMessage.  If a match is found,
//    call the associated function.  Otherwise, call DispDefault to
//    call the default function, if any, associated with the message
//    structure.  In either case, return the value recieved from the
//    message or default function.
//

LRESULT DispMessage(LPMSDI lpmsdi, 
                    HWND   hwnd, 
                    UINT   uMessage, 
                    WPARAM wparam, 
                    LPARAM lparam)
{
    int  imsd = 0;

    MSD *rgmsd = lpmsdi->rgmsd;
    int  cmsd  = lpmsdi->cmsd;

    for (imsd = 0; imsd < cmsd; imsd++)
    {
        if (rgmsd[imsd].uMessage == uMessage)
            return rgmsd[imsd].pfnmsg(hwnd, uMessage, wparam, lparam);
    }

    return DispDefault(lpmsdi->edwp, hwnd, uMessage, wparam, lparam);
}

//
//  FUNCTION: DispCommand(LPCMDI, HWND, WPARAM, LPARAM)
//
//  PURPOSE: Call the function associated with a command.
//
//  PARAMETERS:
//    lpcmdi - Structure containing the command dispatch information.
//    hwnd - The window handle
//    GET_WM_COMMAND_ID(wparam, lparam) - Identifier of the menu item,
//      control, or accelerator.
//    GET_WM_COMMAND_CMD(wparam, lparam) - Notification code.
//    GET_WM_COMMAND_HWND(wparam, lparam) - The control handle or NULL.
//
//  RETURN VALUE:
//    The value returned by the command function that was called.
//
//  COMMENTS:
//    Runs the table of commands stored in lpcmdi->rgcmd searching
//    for a command number that matches wCommand.  If a match is found,
//    call the associated function.  Otherwise, call DispDefault to
//    call the default function, if any, associated with the command
//    structure.  In either case, return the value recieved from the
//    command or default function.
//


LRESULT DispCommand(LPCMDI lpcmdi, 
                    HWND   hwnd, 
                    WPARAM wparam, 
                    LPARAM lparam)
{
    WORD    wCommand = GET_WM_COMMAND_ID(wparam, lparam);
    int     icmd = 0;

    CMD    *rgcmd = lpcmdi->rgcmd;
    int     ccmd  = lpcmdi->ccmd;

    // Message packing of wparam and lparam have changed for Win32,
    // so use the GET_WM_COMMAND macro to unpack the commnad

    for (icmd = 0; icmd < ccmd; icmd++)
    {
        if (rgcmd[icmd].wCommand == wCommand)
        {
            return rgcmd[icmd].pfncmd(hwnd,
                                      wCommand,
                                      GET_WM_COMMAND_CMD(wparam, lparam),
                                      GET_WM_COMMAND_HWND(wparam, lparam));
        }
    }

    return DispDefault(lpcmdi->edwp, hwnd, WM_COMMAND, wparam, lparam);
}


//
//  FUNCTION: DispDefault(EDWP, HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Call the appropriate default window procedure.
//
//  PARAMETERS:
//    edwp - Enumerate specifying the appropriate default winow procedure.
//    hwnd - The window handle
//    uMessage - The message number
//    wparam - Message specific data
//    lparam - Message specific data
//
//  RETURN VALUE:
//    If there is a default proc, return the value returned by the
//    default proc.  Otherwise, return 0.
//
//  COMMENTS:
//    Calls the default procedure associated with edwp using the specified
//    parameters.
//

LRESULT DispDefault(EDWP   edwp, 
                    HWND   hwnd, 
                    UINT   uMessage, 
                    WPARAM wparam, 
                    LPARAM lparam)
{
    switch (edwp)
    {
        case edwpNone:
            return 0;
        case edwpWindow:
            return DefWindowProc(hwnd, uMessage, wparam, lparam);
        case edwpDialog:
            return DefDlgProc(hwnd, uMessage, wparam, lparam);
        case edwpMDIFrame:
            return DefFrameProc(hwnd, hwndMDIIPXChat, uMessage, wparam, lparam);
        case edwpMDIChild:
            return DefMDIChildProc(hwnd, uMessage, wparam, lparam);
    }
    return 0;
}
