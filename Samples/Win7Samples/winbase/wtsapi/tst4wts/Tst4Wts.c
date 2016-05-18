/**********************************************************************
*
* tst4wts.c
*
* Test dynamic determination of presence of WTSAPI DLL.
*
* Copyright 1995-1999, Citrix Systems Inc.
* Copyright (c) 1997 - 2000  Microsoft Corporation
*
**********************************************************************/


#include <windows.h>
#include <tchar.h> // for _tprintf()
#include <stdio.h> // for printf() (which _tprintf() calls)
#include <wtsapi32.h>
#include <wctype.h>


typedef BOOL (WINAPI *PWTSENUMERATESERVERSW) (
    IN LPWSTR pDomainName,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SERVER_INFOW * ppServerInfo,
    OUT DWORD * pCount
    );
typedef BOOL (WINAPI *PWTSENUMERATESERVERSA) (
    IN LPSTR pDomainName,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SERVER_INFOA * ppServerInfo,
    OUT DWORD * pCount
    );

#ifdef UNICODE
typedef PWTSENUMERATESERVERSW PWTSENUMERATESERVERS;
char aWTSEnumerateServers[] = "WTSEnumerateServersW";
#else
typedef PWTSENUMERATESERVERSA PWTSENUMERATESERVERS;
char aWTSEnumerateServers[] = "WTSEnumerateServersA";
#endif

PWTSENUMERATESERVERS pWTSEnumerateServers = NULL;


typedef VOID (WINAPI *PWTSFREEMEMORY)(IN PVOID );
char aWTSFreeMemory[] = "WTSFreeMemory";
PWTSFREEMEMORY pWTSFreeMemory = NULL;    


/********************************************************************
 *
 *  EnumerateServers
 *
 *    Display a list of the Terminal Servers within the specified 
 *    Windows NT domain
 *
 *
 * ENTRY:
 *    pDomainName (input)
 *       Pointer to Windows NT domain name (or NULL for current domain)
 *
 * EXIT:
 *    nothing
 *
 *********************************************************************/

void
EnumerateServers( LPTSTR pDomainName )
{
    PWTS_SERVER_INFO pServerInfo;
    DWORD Count;
    DWORD i;

    _tprintf( TEXT("\nWTSEnumerateServers: domain %s\n"), pDomainName );

    if ( !( (*pWTSEnumerateServers)( pDomainName,
                                    0,
                                    1,
                                    &pServerInfo,
                                    &Count ) ) )
    {
        _tprintf( TEXT("WTSEnumerateServers failed, error %u\n"),
                  GetLastError() );
        return;
    }

    _tprintf( TEXT("WTSEnumerateServers: count %u\n"), Count );

    for ( i=0; i < Count; i++ )
    {
        _tprintf( TEXT("%s "), pServerInfo[i].pServerName );
    }
    _tprintf( TEXT("\n") );

    (*pWTSFreeMemory)( pServerInfo );

} // EnumerateServers()


/**********************************************************************
 *
 *  LoadWTSAPI
 *
 *    Attempt to load WTSAPI32.DLL and find the entry point for the
 *    WTSEnumerateServers() function call.
 *
 * ENTRY:
 *    nothing
 *
 * EXIT:
 *    TRUE if WTSAPI.DLL loads successfully, FALSE if not.
 *
 *********************************************************************/

BOOL
LoadWTSAPI(void)
{
    HMODULE hWTSAPI = NULL;

    /*
     *  Get handle to WTSAPI.DLL
     */
    if ((hWTSAPI = LoadLibrary(TEXT("WTSAPI32"))) == NULL )
        return(FALSE);

    /*
     *  Get entry point for WTSEnumerateServers
     */
    pWTSEnumerateServers =
       (PWTSENUMERATESERVERS)GetProcAddress( hWTSAPI,
                                            aWTSEnumerateServers );

    /*
     *  Get entry point for WTSFreeMemory
     */
    pWTSFreeMemory =
       (PWTSFREEMEMORY)GetProcAddress( hWTSAPI, aWTSFreeMemory );

    return(TRUE);
}
                                                     

/**********************************************************************
 *
 *  main
 *
 *********************************************************************/

void main( INT argc, CHAR **argv )
{
    /*
     *  Attempt to load the WTSAPI32.DLL.  If this fails, the program
     *  will exit gracefully. 
     *
     *  If your  application will run on both plain Windows NT and
     *  on Terminal Server, you can choose to use a reduced feature
     *  set on Windows NT.
     */
    if ( !LoadWTSAPI() )
    {
        _tprintf( TEXT("\
Unable to load WTSAPI32.DLL.  This is because the system is running \n\
on a non-Terminal Server system.  This program will now terminate. \n\
(Your application could perform different things based on this result). \n") );
    }
    else
    {
        _tprintf( TEXT("\
WTSAPI32.DLL loaded successfully, now enumerating servers...\n\n") );
        EnumerateServers( NULL );
    }

} // main()



