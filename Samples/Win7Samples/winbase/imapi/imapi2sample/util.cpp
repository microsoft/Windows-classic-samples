
//
// util.cpp
//
// Helper functions for Imapi2 test
//

#include "common.h"

void PrintHR(HRESULT hr)
{
    LPVOID lpMsgBuf;
    DWORD ret;

    ret = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_HMODULE,
                GetModuleHandle(TEXT("imapi2.dll")),
                hr,
                0, //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

    if (ret != 0)
    {
        printf("  Returned %08x: %s\n", hr, lpMsgBuf);
    }
    
    if (ret == 0)
    {
        ret = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_HMODULE,
                GetModuleHandle(TEXT("imapi2fs.dll")),
                hr,
                0, //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL );

        if (ret != 0)
        {
            printf("  Returned %08x: %s\n", hr, lpMsgBuf);
        }
    }

    if (ret == 0)
    {
        ret = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    hr,
                    0, //MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0, NULL );

        if (ret != 0)
        {
            printf("  Returned %08x: %s\n", hr, lpMsgBuf);
        }
        else
        {
            printf("  Returned %08x (no description)\n\n", hr);
        }
    }

    //if (ret == 0)
    //{
    //    DWORD dw = GetLastError(); 

    //    ret = FormatMessage(
    //        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    //        FORMAT_MESSAGE_FROM_SYSTEM,
    //        NULL,
    //        dw,
    //        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    //        (LPTSTR) &lpMsgBuf,
    //        0, NULL );
    //    
    //    if (ret != 0)
    //    {
    //        printf("  GetLastError returned %08x: %s\n", hr, lpMsgBuf);
    //    }
    //    else
    //    {
    //        printf("  Returned %08x (no description)\n", hr);
    //    }
    //    ExitProcess(dw); 
    //}


    LocalFree(lpMsgBuf);
}