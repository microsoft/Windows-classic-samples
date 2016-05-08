// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*****************************************************************************
*
* Debug.H
*
* Public include file for debug output
*
*****************************************************************************/

#ifndef _INC_DEBUG
#define _INC_DEBUG
#ifdef __cplusplus
extern "C"
{
#endif

#define  ISRDEBUG             1
#define  DEBUG_SECTION        "debug"        /* section name for                */
#define  DEBUG_MODULE_NAME    "MIDIPLYR"     /* key name and prefix for output  */
#define  DEBUG_MAX_LINE_LEN   255            /* max line length (bytes)         */
                                             
#define  DRV_ENABLE_DEBUG     (DRV_USER+1)   /* Enable/disable debug message    */
#define  DRV_SET_DEBUG_LEVEL  (DRV_USER+2)   /* Message to set the debug level  */

#ifdef DEBUG
    VOID WINAPI WinAssert(LPSTR lpstrExp, LPSTR lpstrFile, DWORD dwLine);
    BOOL WINAPI DbgEnable(BOOL fEnable);
    UINT WINAPI DbgSetLevel(UINT uLevel);
    UINT WINAPI DbgInitialize(BOOL fEnable);

    void FAR CDECL dprintf(UINT uDbgLevel, LPSTR szFmt, ...);

    #define assert(exp) \
        ( (exp) ? (void) 0 : WinAssert(#exp, __FILE__, __LINE__) )

    #define DPF                  dprintf

    #define D1(sz)               dprintf(1,sz) 
    #define D2(sz)               dprintf(2,sz) 
    #define D3(sz)               dprintf(3,sz) 
    #define D4(sz)               dprintf(4,sz) 
#else
    #define assert(exp)          ((void)0)
    
    #define DbgEnable(x)         FALSE
    #define DbgSetLevel(x)       0
    #define DbgInitialize(x)     0

    #define DPF                  1 ? (void)0 : (void)

    #define D1(sz)
    #define D2(sz)
    #define D3(sz)
    #define D4(sz)
#endif


#ifdef __cplusplus
}
#endif
#endif
