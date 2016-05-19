// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//-----------------------------------------------------------------------------
// File: DXUtil.cpp
//
// Desc: Shortcut macros and functions for using DX objects
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <assert.h>
#include <tchar.h>
#include <commctrl.h>
#include "dxutil.h"


#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif
#include <strsafe.h>

#ifdef UNICODE
    typedef HINSTANCE (WINAPI* LPShellExecute)(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd);
#else
    typedef HINSTANCE (WINAPI* LPShellExecute)(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);
#endif

bool DXUtil_FindMediaSearchParentDirs( TCHAR* strSearchPath, int cchSearch, TCHAR* strStartAt, TCHAR* strLeafName );
bool DXUtil_FindMediaSearchTypicalDirs( TCHAR* strSearchPath, int cchSearch, LPCTSTR strLeaf, TCHAR* strExePath, TCHAR* strExeName, TCHAR* strMediaDir );





#ifndef UNDER_CE

//--------------------------------------------------------------------------------------
// Search a set of typical directories
//--------------------------------------------------------------------------------------
bool DXUtil_FindMediaSearchTypicalDirs( TCHAR* strSearchPath, int cchSearch, LPCTSTR strLeaf,
                                        TCHAR* strExePath, TCHAR* strExeName, TCHAR* strMediaDir )
{
    // Typical directories:
    //      .\
    //      ..\
    //      ..\..\
    //      %EXE_DIR%\
    //      %EXE_DIR%\..\
    //      %EXE_DIR%\..\..\
    //      %EXE_DIR%\..\%EXE_NAME%
    //      %EXE_DIR%\..\..\%EXE_NAME%
    //      DXSDK media path

    /* Search in .\ */
    (void)StringCchCopy( strSearchPath, cchSearch, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in ..\ */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("..\\%s"), strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in ..\..\ */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("..\\..\\%s"), strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in ..\..\ */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("..\\..\\%s"), strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in the %EXE_DIR%\ */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("%s\\%s"), strExePath, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in the %EXE_DIR%\..\ */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("%s\\..\\%s"), strExePath, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in the %EXE_DIR%\..\..\ */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("%s\\..\\..\\%s"), strExePath, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in "%EXE_DIR%\..\%EXE_NAME%\".  This matches the DirectX SDK layout */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("%s\\..\\%s\\%s"), strExePath, strExeName, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in "%EXE_DIR%\..\..\%EXE_NAME%\".  This matches the DirectX SDK layout */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("%s\\..\\..\\%s\\%s"), strExePath, strExeName, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    /* Search in DirectX SDK's media dir */
    (void)StringCchPrintf( strSearchPath, cchSearch, TEXT("%s%s"), strMediaDir, strLeaf ); strSearchPath[cchSearch-1] = 0;
    if( GetFileAttributes( strSearchPath ) != 0xFFFFFFFF )
        return true;

    return false;
}



//--------------------------------------------------------------------------------------
// Search parent directories starting at strStartAt, and appending strLeafName
// at each parent directory.  It stops at the root directory.
//--------------------------------------------------------------------------------------
bool DXUtil_FindMediaSearchParentDirs( TCHAR* strSearchPath, int cchSearch, TCHAR* strStartAt, TCHAR* strLeafName )
{
    TCHAR strFullPath[MAX_PATH] = {0};
    TCHAR strFullFileName[MAX_PATH] = {0};
    TCHAR strSearch[MAX_PATH] = {0};
    TCHAR* strFilePart = NULL;

    GetFullPathName( strStartAt, MAX_PATH, strFullPath, &strFilePart );
    if( strFilePart == NULL )
        return false;

    while( strFilePart != NULL )
    {
        (void)StringCchPrintf( strFullFileName, MAX_PATH, TEXT("%s\\%s"), strFullPath, strLeafName ); strFullFileName[MAX_PATH-1] = 0;
        if( GetFileAttributes( strFullFileName ) != 0xFFFFFFFF )
        {
            (void)StringCchCopy( strSearchPath, cchSearch, strFullFileName ); strSearchPath[cchSearch-1] = 0;
            return true;
        }

        (void)StringCchPrintf( strSearch, MAX_PATH, TEXT("%s\\.."), strFullPath ); strSearch[MAX_PATH-1] = 0;
        GetFullPathName( strSearch, MAX_PATH, strFullPath, &strFilePart );
    }

    return false;
}
#endif // !UNDER_CE




//-----------------------------------------------------------------------------
// Name: DXUtil_ReadStringRegKeyCch()
// Desc: Helper function to read a registry key string
//       cchDest is the size in TCHARs of strDest.  Be careful not to
//       pass in sizeof(strDest) on UNICODE builds.
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadStringRegKeyCch( HKEY hKey, LPCTSTR strRegName, TCHAR* strDest,
                                    DWORD cchDest, LPCTSTR strDefault )
{
    DWORD dwType;
    DWORD cbDest = cchDest * sizeof(TCHAR);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType,
                                          (BYTE*)strDest, &cbDest ) )
    {
        (void)StringCchCopy( strDest, cchDest, strDefault );
        strDest[cchDest-1] = 0;
        return S_FALSE;
    }
    else
    {
        if( dwType != REG_SZ )
        {
            (void)StringCchCopy( strDest, cchDest , strDefault);
            strDest[cchDest-1] = 0;
            return S_FALSE;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_WriteStringRegKey()
// Desc: Helper function to write a registry key string
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteStringRegKey( HKEY hKey, LPCTSTR strRegName,
                                  LPCTSTR strValue )
{
    if( NULL == strValue )
        return E_INVALIDARG;

    DWORD cbValue = ((DWORD)_tcslen(strValue)+1) * sizeof(TCHAR);

    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_SZ,
                                        (BYTE*)strValue, cbValue ) )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ReadFloatRegKey()
// Desc: Helper function to read a registry key string
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadFloatRegKey( HKEY hKey, LPCTSTR strRegName, FLOAT* pfDest, FLOAT fDefault )
{
    if( NULL == pfDest )
        return E_INVALIDARG;

    TCHAR sz[256];
    float fResult;

    TCHAR strDefault[256];
    (void)StringCchPrintf( strDefault, 256, TEXT("%f"), fDefault );
    strDefault[255] = 0;

    if( SUCCEEDED( DXUtil_ReadStringRegKeyCch( hKey, strRegName, sz, 256, strDefault ) ) )
    {
        int nResult = _stscanf_s( sz, TEXT("%f"), &fResult );
        if( nResult == 1 )
        {
            *pfDest = fResult;
            return S_OK;
        }
    }

    *pfDest = fDefault;
    return S_FALSE;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_WriteFloatRegKey()
// Desc: Helper function to write a registry key string
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteFloatRegKey( HKEY hKey, LPCTSTR strRegName, FLOAT fValue )
{
    TCHAR strValue[256];
    (void)StringCchPrintf( strValue, 256, TEXT("%f"), fValue );
    strValue[255] = 0;

    return DXUtil_WriteStringRegKey( hKey, strRegName, strValue );
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ReadIntRegKey()
// Desc: Helper function to read a registry key int
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadIntRegKey( HKEY hKey, LPCTSTR strRegName, DWORD* pdwDest,
                              DWORD dwDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(DWORD);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType,
                                          (BYTE*)pdwDest, &dwLength ) )
    {
        *pdwDest = dwDefault;
        return S_FALSE;
    }
    else
    {
        if( dwType != REG_DWORD )
        {
            *pdwDest = dwDefault;
            return S_FALSE;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_WriteIntRegKey()
// Desc: Helper function to write a registry key int
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteIntRegKey( HKEY hKey, LPCTSTR strRegName, DWORD dwValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD,
                                        (BYTE*)&dwValue, sizeof(DWORD) ) )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ReadBoolRegKey()
// Desc: Helper function to read a registry key BOOL
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadBoolRegKey( HKEY hKey, LPCTSTR strRegName, BOOL* pbDest,
                              BOOL bDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(BOOL);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType,
                                          (BYTE*)pbDest, &dwLength ) )
    {
        *pbDest = bDefault;
        return S_FALSE;
    }
    else
    {
        if( dwType != REG_DWORD )
        {
            *pbDest = bDefault;
            return S_FALSE;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_WriteBoolRegKey()
// Desc: Helper function to write a registry key BOOL
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteBoolRegKey( HKEY hKey, LPCTSTR strRegName, BOOL bValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD,
                                        (BYTE*)&bValue, sizeof(BOOL) ) )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ReadGuidRegKey()
// Desc: Helper function to read a registry key guid
//-----------------------------------------------------------------------------
HRESULT DXUtil_ReadGuidRegKey( HKEY hKey, LPCTSTR strRegName, GUID* pGuidDest,
                               GUID& guidDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(GUID);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType,
                                          (LPBYTE) pGuidDest, &dwLength ) )
    {
        *pGuidDest = guidDefault;
        return S_FALSE;
    }
    else
    {
        if( dwType != REG_BINARY )
        {
            *pGuidDest = guidDefault;
            return S_FALSE;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_WriteGuidRegKey()
// Desc: Helper function to write a registry key guid
//-----------------------------------------------------------------------------
HRESULT DXUtil_WriteGuidRegKey( HKEY hKey, LPCTSTR strRegName, GUID guidValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_BINARY,
                                        (BYTE*)&guidValue, sizeof(GUID) ) )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_Timer()
// Desc: Performs timer opertations. Use the following commands:
//          TIMER_RESET           - to reset the timer
//          TIMER_START           - to start the timer
//          TIMER_STOP            - to stop (or pause) the timer
//          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
//          TIMER_GETABSOLUTETIME - to get the absolute system time
//          TIMER_GETAPPTIME      - to get the current time
//          TIMER_GETELAPSEDTIME  - to get the time that elapsed between
//                                  TIMER_GETELAPSEDTIME calls
//-----------------------------------------------------------------------------
FLOAT __stdcall DXUtil_Timer( TIMER_COMMAND command )
{
    static BOOL     m_bTimerInitialized = FALSE;
    static BOOL     m_bUsingQPF         = FALSE;
    static BOOL     m_bTimerStopped     = TRUE;
    static LONGLONG m_llQPFTicksPerSec  = 0;

    // Initialize the timer
    if( FALSE == m_bTimerInitialized )
    {
        m_bTimerInitialized = TRUE;

        // Use QueryPerformanceFrequency() to get frequency of timer.  If QPF is
        // not supported, we will timeGetTime() which returns milliseconds.
        LARGE_INTEGER qwTicksPerSec;
        m_bUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec );
        if( m_bUsingQPF )
            m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
    }

    if( m_bUsingQPF )
    {
        static LONGLONG m_llStopTime        = 0;
        static LONGLONG m_llLastElapsedTime = 0;
        static LONGLONG m_llBaseTime        = 0;
        double fTime;
        double fElapsedTime;
        LARGE_INTEGER qwTime;

        // Get either the current time or the stop time, depending
        // on whether we're stopped and what command was sent
        if( m_llStopTime != 0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
            qwTime.QuadPart = m_llStopTime;
        else
            QueryPerformanceCounter( &qwTime );

        // Return the elapsed time
        if( command == TIMER_GETELAPSEDTIME )
        {
            fElapsedTime = (double) ( qwTime.QuadPart - m_llLastElapsedTime ) / (double) m_llQPFTicksPerSec;
            m_llLastElapsedTime = qwTime.QuadPart;
            return (FLOAT) fElapsedTime;
        }

        // Return the current time
        if( command == TIMER_GETAPPTIME )
        {
            double fAppTime = (double) ( qwTime.QuadPart - m_llBaseTime ) / (double) m_llQPFTicksPerSec;
            return (FLOAT) fAppTime;
        }

        // Reset the timer
        if( command == TIMER_RESET )
        {
            m_llBaseTime        = qwTime.QuadPart;
            m_llLastElapsedTime = qwTime.QuadPart;
            m_llStopTime        = 0;
            m_bTimerStopped     = FALSE;
            return 0.0f;
        }

        // Start the timer
        if( command == TIMER_START )
        {
            if( m_bTimerStopped )
                m_llBaseTime += qwTime.QuadPart - m_llStopTime;
            m_llStopTime = 0;
            m_llLastElapsedTime = qwTime.QuadPart;
            m_bTimerStopped = FALSE;
            return 0.0f;
        }

        // Stop the timer
        if( command == TIMER_STOP )
        {
            if( !m_bTimerStopped )
            {
                m_llStopTime = qwTime.QuadPart;
                m_llLastElapsedTime = qwTime.QuadPart;
                m_bTimerStopped = TRUE;
            }
            return 0.0f;
        }

        // Advance the timer by 1/10th second
        if( command == TIMER_ADVANCE )
        {
            m_llStopTime += m_llQPFTicksPerSec/10;
            return 0.0f;
        }

        if( command == TIMER_GETABSOLUTETIME )
        {
            fTime = qwTime.QuadPart / (double) m_llQPFTicksPerSec;
            return (FLOAT) fTime;
        }

        return -1.0f; // Invalid command specified
    }
    else
    {
        // Get the time using timeGetTime()
        static double m_fLastElapsedTime  = 0.0;
        static double m_fBaseTime         = 0.0;
        static double m_fStopTime         = 0.0;
        double fTime;
        double fElapsedTime;

        // Get either the current time or the stop time, depending
        // on whether we're stopped and what command was sent
        if( m_fStopTime != 0.0 && command != TIMER_START && command != TIMER_GETABSOLUTETIME)
            fTime = m_fStopTime;
        else
            fTime = GETTIMESTAMP() * 0.001;

        // Return the elapsed time
        if( command == TIMER_GETELAPSEDTIME )
        {
            fElapsedTime = (double) (fTime - m_fLastElapsedTime);
            m_fLastElapsedTime = fTime;
            return (FLOAT) fElapsedTime;
        }

        // Return the current time
        if( command == TIMER_GETAPPTIME )
        {
            return (FLOAT) (fTime - m_fBaseTime);
        }

        // Reset the timer
        if( command == TIMER_RESET )
        {
            m_fBaseTime         = fTime;
            m_fLastElapsedTime  = fTime;
            m_fStopTime         = 0;
            m_bTimerStopped     = FALSE;
            return 0.0f;
        }

        // Start the timer
        if( command == TIMER_START )
        {
            if( m_bTimerStopped )
                m_fBaseTime += fTime - m_fStopTime;
            m_fStopTime = 0.0f;
            m_fLastElapsedTime  = fTime;
            m_bTimerStopped = FALSE;
            return 0.0f;
        }

        // Stop the timer
        if( command == TIMER_STOP )
        {
            if( !m_bTimerStopped )
            {
                m_fStopTime = fTime;
                m_fLastElapsedTime  = fTime;
                m_bTimerStopped = TRUE;
            }
            return 0.0f;
        }

        // Advance the timer by 1/10th second
        if( command == TIMER_ADVANCE )
        {
            m_fStopTime += 0.1f;
            return 0.0f;
        }

        if( command == TIMER_GETABSOLUTETIME )
        {
            return (FLOAT) fTime;
        }

        return -1.0f; // Invalid command specified
    }
}







//-----------------------------------------------------------------------------
// Name: DXUtil_LaunchReadme()
// Desc: Finds and opens the readme for this sample
//-----------------------------------------------------------------------------
VOID DXUtil_LaunchReadme( HWND hWnd, LPCTSTR strLoc )
{
#ifdef UNDER_CE
    // This is not available on PocketPC
    MessageBox( hWnd, TEXT("For operating instructions, please open the ")
                      TEXT("readme.txt file included with the project."),
                TEXT("DirectX SDK Sample"), MB_ICONWARNING | MB_OK );

    return;
#endif

    const int NUM_FILENAMES = 2;
    LPCTSTR strFilenames[] =
    {
        TEXT("readme.htm"),
        TEXT("readme.txt")
    };

    TCHAR strReadmePath[1024];
    TCHAR strExeName[MAX_PATH];
    TCHAR strExePath[MAX_PATH];
    TCHAR strSamplePath[MAX_PATH];
    TCHAR* strLastSlash = NULL;

    for( int i=0; i < NUM_FILENAMES; i++ )
    {
        LPCTSTR strFilename = strFilenames[i];
        bool bSuccess = false;
        bool bFound = false;

        (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), TEXT("") );
        (void)StringCchCopy( strExePath, NUMELMS(strExePath), TEXT("") );
        (void)StringCchCopy( strExeName, NUMELMS(strExeName), TEXT("") );
        (void)StringCchCopy( strSamplePath, NUMELMS(strSamplePath), TEXT("") );

        // If the user provided a location for the readme, check there first.
        if( strLoc )
        {
            HKEY  hKey;
            LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        _T("Software\\Microsoft\\DirectX SDK"),
                                        0, KEY_READ, &hKey );
            if( ERROR_SUCCESS == lResult )
            {
                DWORD dwType;
                DWORD dwSize = MAX_PATH * sizeof(TCHAR);
                lResult = RegQueryValueEx( hKey, _T("DX9D4SDK Samples Path"), NULL,
                                        &dwType, (BYTE*)strSamplePath, &dwSize );
                strSamplePath[MAX_PATH-1] = 0; // RegQueryValueEx doesn't NULL term if buffer too small

                if( ERROR_SUCCESS == lResult )
                {
                    (void)StringCchPrintf( strReadmePath, 1023, TEXT("%s\\C++\\%s\\%s"),
                                strSamplePath, strLoc, strFilename );
                    strReadmePath[1023] = 0;

                    if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                        bFound = TRUE;
                }

                RegCloseKey( hKey );
            }
        }

        // Get the exe name, and exe path
        GetModuleFileName( NULL, strExePath, MAX_PATH );
        strExePath[MAX_PATH-1]=0;

        strLastSlash = _tcsrchr( strExePath, TEXT('\\') );
        if( strLastSlash )
        {
            (void)StringCchCopy( strExeName, MAX_PATH, &strLastSlash[1] );
            strExeName[MAX_PATH-1]=0;

            // Chop the exe name from the exe path
            *strLastSlash = 0;

            // Chop the .exe from the exe name
            strLastSlash = _tcsrchr( strExeName, TEXT('.') );
            if( strLastSlash )
                *strLastSlash = 0;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\..\%EXE_NAME%\".  This matches the DirectX SDK layout
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );

            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),strExeName );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\..\BumpMapping\%EXE_NAME%\".
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath),strExePath );

            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),TEXT("\\BumpMapping\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),strExeName );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\..\EnvMapping\%EXE_NAME%\".
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );

            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),TEXT("\\EnvMapping\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),strExeName );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath),strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\..\Meshes\%EXE_NAME%\".
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );

            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\Meshes\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strExeName );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\..\StencilBuffer\%EXE_NAME%\".
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );

            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\StencilBuffer\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strExeName );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\"
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\.."
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );
            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( !bFound )
        {
            // Search in "%EXE_DIR%\..\.."
            (void)StringCchCopy( strReadmePath, NUMELMS(strReadmePath), strExePath );
            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            strLastSlash = _tcsrchr( strReadmePath, TEXT('\\') );
            if( strLastSlash )
                *strLastSlash = 0;
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), TEXT("\\") );
            (void)StringCchCat( strReadmePath, NUMELMS(strReadmePath), strFilename );
            if( GetFileAttributes( strReadmePath ) != 0xFFFFFFFF )
                bFound = TRUE;
        }

        if( bFound )
        {
            // GetProcAddress for ShellExecute, so we don't have to include shell32.lib
            // in every project that uses dxutil.cpp
            LPShellExecute pShellExecute = NULL;
            HINSTANCE hInstShell32 = LoadLibrary(TEXT("shell32.dlTEXT("));
            if (hInstShell32 != NULL)
            {
    #ifdef UNICODE
                pShellExecute = (LPShellExecute)GetProcAddress(hInstShell32, _TWINCE("ShellExecuteW"));
    #else
                pShellExecute = (LPShellExecute)GetProcAddress(hInstShell32, _TWINCE("ShellExecuteA"));
    #endif
                if( pShellExecute != NULL )
                {
                    if( pShellExecute( hWnd, TEXT("open"), strReadmePath, NULL, NULL, SW_SHOW ) > (HINSTANCE) 32 )
                        bSuccess = true;
                }

                FreeLibrary(hInstShell32);
            }
        }

        if( bSuccess )
            return;
    }

    // Tell the user that the readme couldn't be opened
    MessageBox( hWnd, TEXT("Could not find readme"),
                TEXT("DirectX SDK Sample"), MB_ICONWARNING | MB_OK );
}





//-----------------------------------------------------------------------------
// Name: DXUtil_Trace()
// Desc: Outputs to the debug stream a formatted string with a variable-
//       argument list.
//-----------------------------------------------------------------------------
VOID DXUtil_Trace( LPCTSTR strMsg, ... )
{
#if defined(DEBUG) | defined(_DEBUG)
    TCHAR strBuffer[512];

    va_list args;
    va_start(args, strMsg);
    (void)StringCchVPrintf( strBuffer, NUMELMS(strBuffer), strMsg, args );
    va_end(args);

    OutputDebugString( strBuffer );
#else
    UNREFERENCED_PARAMETER(strMsg);
#endif
}




//-----------------------------------------------------------------------------
// Name: CArrayList constructor
// Desc:
//-----------------------------------------------------------------------------
CArrayList::CArrayList( ArrayListType Type, UINT BytesPerEntry )
{
    if( Type == AL_REFERENCE )
        BytesPerEntry = sizeof(void*);
    m_ArrayListType = Type;
    m_pData = NULL;
    m_BytesPerEntry = BytesPerEntry;
    m_NumEntries = 0;
    m_NumEntriesAllocated = 0;
}



//-----------------------------------------------------------------------------
// Name: CArrayList destructor
// Desc:
//-----------------------------------------------------------------------------
CArrayList::~CArrayList( void )
{
    if( m_pData != NULL )
        delete[] m_pData;
}




//-----------------------------------------------------------------------------
// Name: CArrayList::Add
// Desc: Adds pEntry to the list.
//-----------------------------------------------------------------------------
HRESULT CArrayList::Add( void* pEntry )
{
    if( m_BytesPerEntry == 0 )
        return E_FAIL;
    if( m_pData == NULL || m_NumEntries + 1 > m_NumEntriesAllocated )
    {
        void* pDataNew;
        UINT NumEntriesAllocatedNew;
        if( m_NumEntriesAllocated == 0 )
            NumEntriesAllocatedNew = 16;
        else
            NumEntriesAllocatedNew = m_NumEntriesAllocated * 2;
        pDataNew = new BYTE[NumEntriesAllocatedNew * m_BytesPerEntry];
        if( pDataNew == NULL )
            return E_OUTOFMEMORY;
        if( m_pData != NULL )
        {
            CopyMemory( pDataNew, m_pData, m_NumEntries * m_BytesPerEntry );
            delete[] m_pData;
        }
        m_pData = pDataNew;
        m_NumEntriesAllocated = NumEntriesAllocatedNew;
    }

    if( m_ArrayListType == AL_VALUE )
        CopyMemory( (BYTE*)m_pData + (m_NumEntries * m_BytesPerEntry), pEntry, m_BytesPerEntry );
    else
        *(((void**)m_pData) + m_NumEntries) = pEntry;
    m_NumEntries++;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CArrayList::Remove
// Desc: Remove the item at Entry in the list, and collapse the array.
//-----------------------------------------------------------------------------
void CArrayList::Remove( UINT Entry )
{
    // Decrement count
    m_NumEntries--;

    // Find the entry address
    BYTE* pData = (BYTE*)m_pData + (Entry * m_BytesPerEntry);

    // Collapse the array
    MoveMemory( pData, pData + m_BytesPerEntry, ( m_NumEntries - Entry ) * m_BytesPerEntry );
}




//-----------------------------------------------------------------------------
// Name: CArrayList::GetPtr
// Desc: Returns a pointer to the Entry'th entry in the list.
//-----------------------------------------------------------------------------
void* CArrayList::GetPtr( UINT Entry )
{
    if( m_ArrayListType == AL_VALUE )
        return (BYTE*)m_pData + (Entry * m_BytesPerEntry);
    else
        return *(((void**)m_pData) + Entry);
}




//-----------------------------------------------------------------------------
// Name: CArrayList::Contains
// Desc: Returns whether the list contains an entry identical to the
//       specified entry data.
//-----------------------------------------------------------------------------
bool CArrayList::Contains( void* pEntryData )
{
    for( UINT iEntry = 0; iEntry < m_NumEntries; iEntry++ )
    {
        if( m_ArrayListType == AL_VALUE )
        {
            if( memcmp( GetPtr(iEntry), pEntryData, m_BytesPerEntry ) == 0 )
                return true;
        }
        else
        {
            if( GetPtr(iEntry) == pEntryData )
                return true;
        }
    }
    return false;
}




