/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1993 - 2000.  Microsoft Corporation.  All rights reserved.

Module Name:

    init.c

Abstract:

    NT LM Security Support Provider client side initialization.

Environment:  User Mode

Revision History:

--*/


#include "sampssp.h"

CRITICAL_SECTION DllCritSect;    // Serializes access to all globals in module


BOOLEAN
DllInit(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This is the Dll initialization routine for ntlmssp.dll

Arguments:

    Standard.

Return Status:

    TRUE: if initialization succeeded

--*/

{

    //
    // On process attach,
    //      initialize the critical section,
    //      defer any additional initialization.
    //

    switch (Reason) {
    case DLL_PROCESS_ATTACH:
#if defined (DEBUG)
            DebugBreak();
#endif
        InitializeCriticalSection( &DllCritSect );
        break;

    //
    // Handle process detach.
    //

    case DLL_PROCESS_DETACH:


        //
        // Shutdown the common routines.
        //

        EnterCriticalSection( &DllCritSect );
        LeaveCriticalSection( &DllCritSect );

        //
        // Finally, Delete the critical section
        //

        DeleteCriticalSection( &DllCritSect );

        break;

    }

    return TRUE;
    UNREFERENCED_PARAMETER( Context );
    UNREFERENCED_PARAMETER( DllHandle );

}



