//***************************************************************************
//    THIS CODE AND INFORMATION IS PROVIDED 'AS IS' WITHOUT WARRANTY OF
//    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//    PARTICULAR PURPOSE.
//
//    Copyright Microsoft Corporation. All Rights Reserved.
//***************************************************************************

//***************************************************************************
//
//    File:          Main.cpp
//
//    Description:   
//
//***************************************************************************

//***************************************************************************
//    #include statements
//***************************************************************************

#include "stdafx.h"

#define MAX_ADSPATH_CHARS 2048

//***************************************************************************
//    local function prototypes
//***************************************************************************

int LocalToWideChar(LPWSTR pWide, size_t buffSize, LPCTSTR pLocal, DWORD dwChars);

/***************************************************************************

    _tmain()

***************************************************************************/

int _tmain(int argc, _TCHAR* argv[])
{
    TCHAR szTemp[MAX_ADSPATH_CHARS];
    LPWSTR pwszADsPath = NULL;

    CoInitialize(NULL);

    if(argc < 2)
    {
        // Prompt the user for the ADsPath.
        _tprintf(TEXT("Enter the ADsPath of the object to display the property sheet for:\n"));
        _fgetts(szTemp, MAX_ADSPATH_CHARS - 1, stdin);

        // Trim the last character, which is the carriage return.
        szTemp[lstrlen(szTemp) - 1] = 0;
    }
    else
    {
        _tcsncpy_s(szTemp, ARRAYSIZE(szTemp), argv[1], MAX_ADSPATH_CHARS - 1);
    }

    DWORD dwChars = lstrlen(szTemp) + 1;
    pwszADsPath = new WCHAR[dwChars];

    if(pwszADsPath)
    {
        HRESULT hr;

        HINSTANCE hInstance = NULL;
        HWND hwndConsole = GetConsoleWindow();
        if(hwndConsole)
        {
            hInstance = (HINSTANCE)(LONG_PTR)GetWindowLongPtr(hwndConsole, GWLP_HINSTANCE);
        }

        CPropSheetHost *pHost = new CPropSheetHost(hInstance);

        LocalToWideChar(pwszADsPath, dwChars, szTemp, dwChars);

        // Hold a reference count for the CPropSheetHost object.
        pHost->AddRef();

        hr = pHost->SetObject(pwszADsPath);
        if(FAILED(hr))
        {
            goto ExitMain;
        }

        pHost->Run();

        /*
        Release the CPropSheetHost object. Other components may still hold a 
        reference to the object, so this cannot just be deleted here. Let 
        the object delete itself when all references are released.
        */
        pHost->Release();
    }

ExitMain:
    if(pwszADsPath)
    {
        delete pwszADsPath;
    }

    CoUninitialize();

    return 0;
}

/**************************************************************************

   LocalToWideChar()
   
**************************************************************************/

int LocalToWideChar(LPWSTR pWide, size_t buffSize, LPCTSTR pLocal, DWORD dwChars)
{
    *pWide = 0;

    #ifdef UNICODE
    wcsncpy_s(pWide, buffSize, pLocal, dwChars);
    #else
    MultiByteToWideChar( CP_ACP, 
                        0, 
                        pLocal, 
                        -1,
                        pWide, 
                        dwChars); 
    #endif

    return lstrlenW(pWide);
}

