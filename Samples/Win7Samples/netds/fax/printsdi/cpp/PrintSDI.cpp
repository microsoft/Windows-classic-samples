//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#include "PrintSDI.h"
using namespace Gdiplus;
//+---------------------------------------------------------------------------
//
//  function:   GiveUsage
//
//  Synopsis:   prints the usage of the application
//
//  Arguments:  [AppName] - Name of the application whose usage has to be printed
//
//  Returns:    void
//
//----------------------------------------------------------------------------

void GiveUsage(LPTSTR AppName)
{
        _tprintf( TEXT("Usage : %s \n \
            /s (optional) Fax Server Name \n \
            /p (optional) Fax Printer Name \n \
            /f Fax Number \n \
            /n Sender Name \n \
            /c Server side coverpage to be sent E.g. confident.cov \n"),AppName);
        _tprintf( TEXT("Usage : %s /? -- help message\n"),AppName);
}

int  __cdecl _tmain(int argc, _TCHAR* argv[])
{        
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrPrinterName=NULL;
        LPTSTR lptstrNumber = NULL;
        LPTSTR lptstrName = NULL;
        LPTSTR lptstrCoverPageName = NULL;

        // FaxPrintInfo is the fax print info
        FAX_PRINT_INFO      FaxPrintInfo;
        // dwFaxId is the fax job id
        DWORD               dwFaxId;
        // FaxContextInfo is the fax context
        FAX_CONTEXT_INFO    FaxContextInfo;
        // CoverPageInfo is the cover page info
        FAX_COVERPAGE_INFO  CoverPageInfo;
        // szFaxPrinterName is the name of the fax printer
        LPTSTR              lptstrFaxPrinterName =NULL;

        // hFaxSvcHandle is the handle to the fax server    
        HANDLE              hFaxSvcHandle = NULL;
        // pFaxConfig is a pointer to the fax configuration
        PFAX_CONFIGURATION  pFaxConfig = NULL;
        // buffer to receive array of job data
        PFAX_JOB_ENTRY      pJobs = NULL;
        GdiplusStartupInput gdiplusStartupInput;
        ULONG_PTR gdiplusToken;
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

        DOCINFO docInfo;
        ZeroMemory(&docInfo, sizeof(docInfo));

        size_t argSize = 0;
        int argcount = 0;

#ifdef UNICODE
        argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
        argv = argvA;
#endif              

        if (argc == 1)
        {
                _tprintf( TEXT("Missing args.\n") );
                GiveUsage(argv[0]);
                bRetVal = false;
                goto Exit1;
        }


        // check for commandline switches
        for (argcount=1; argcount<argc; argcount++)
        {                  
                if(argcount + 1 < argc)
                {
                        hr = StringCbLength(argv[argcount + 1],1024 * sizeof(TCHAR),&argSize);
                        if(!FAILED(hr))
                        {
                                if ((argv[argcount][0] == L'/') || (argv[argcount][0] == L'-'))
                                {
                                        switch (towlower(argv[argcount][1]))
                                        {
                                                case 's':
                                                        if(lptstrServerName == NULL)
                                                        {
                                                                lptstrServerName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                if(lptstrServerName == NULL)
                                                                {
                                                                        _tprintf(_T("lptstrServerName: malloc failed. Error %d \n"), GetLastError());
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                                memset(lptstrServerName, 0, (argSize+1)* sizeof(TCHAR));
                                                                hr = StringCchCopyN(lptstrServerName,argSize+1, argv[argcount+1],argSize);
                                                                if(FAILED(hr))
                                                                {
                                                                        _tprintf(_T("lptstrServerName: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit1;
                                                        }
                                                        argcount++;
                                                        break;
                                                case 'p':
                                                        if(lptstrPrinterName == NULL)
                                                        {
                                                                lptstrPrinterName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                if(lptstrPrinterName == NULL)
                                                                {
                                                                        _tprintf(_T("lptstrPrinterName: malloc failed. Error %d \n"), GetLastError());
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                                memset(lptstrPrinterName, 0, (argSize+1)* sizeof(TCHAR));
                                                                hr = StringCchCopyN(lptstrPrinterName,argSize+1, argv[argcount+1],argSize);
                                                                if(FAILED(hr))
                                                                {
                                                                        _tprintf(_T("lptstrPrinterName: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit1;
                                                        }
                                                        argcount++;
                                                        break;
                                                case 'f':
                                                        if(lptstrNumber == NULL)
                                                        {
                                                                lptstrNumber = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                if(lptstrNumber == NULL)
                                                                {
                                                                        _tprintf(_T("lptstrNumber: malloc failed. Error %d \n"), GetLastError());
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                                memset(lptstrNumber, 0, (argSize+1)* sizeof(TCHAR));
                                                                hr = StringCchCopyN(lptstrNumber,argSize+1, argv[argcount+1],argSize);
                                                                if(FAILED(hr))
                                                                {
                                                                        _tprintf(_T("lptstrNumber: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit1;
                                                        }
                                                        argcount++;
                                                        break;
                                                case 'n':
                                                        if(lptstrName == NULL)
                                                        {
                                                                lptstrName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                if(lptstrName == NULL)
                                                                {
                                                                        _tprintf(_T("lptstrName: malloc failed. Error %d \n"), GetLastError());
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                                memset(lptstrName, 0, (argSize+1)* sizeof(TCHAR));
                                                                hr = StringCchCopyN(lptstrName,argSize+1, argv[argcount+1],argSize);
                                                                if(FAILED(hr))
                                                                {
                                                                        _tprintf(_T("lptstrName: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit1;
                                                        }
                                                        argcount++;
                                                        break;
                                                case 'c':
                                                        if(lptstrCoverPageName == NULL)
                                                        {
                                                                lptstrCoverPageName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                if(lptstrCoverPageName == NULL)
                                                                {
                                                                        _tprintf(_T("lptstrCoverPageName: malloc failed. Error %d \n"), GetLastError());
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                                memset(lptstrCoverPageName, 0, (argSize+1)* sizeof(TCHAR));
                                                                hr = StringCchCopyN(lptstrCoverPageName,argSize+1, argv[argcount+1],argSize);
                                                                if(FAILED(hr))
                                                                {
                                                                        _tprintf(_T("lptstrCoverPageName: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                        bRetVal = false;
                                                                        goto Exit1;
                                                                }
                                                        }
                                                        else
                                                        {
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit1;
                                                        }
                                                        argcount++;
                                                        break;

                                                case '?':
                                                        GiveUsage(argv[0]);
                                                        bRetVal = false;
                                                        goto Exit1;                
                                                default:
                                                        break;
                                        }//switch
                                }//if
                        }
                }
        }//for

        if ((lptstrNumber == NULL) || (lptstrName == NULL ) || (lptstrCoverPageName == NULL))
        {
                _tprintf( TEXT("Missing/Invalid Value.\n") );
                GiveUsage(argv[0]);
                bRetVal = false;
                goto Exit1;
        }

        ZeroMemory(&FaxPrintInfo, sizeof(FAX_PRINT_INFO));
        FaxPrintInfo.SizeOfStruct = sizeof(FAX_PRINT_INFO);

        FaxPrintInfo.RecipientNumber = lptstrNumber;
        FaxPrintInfo.RecipientName   = lptstrName;

        _tprintf(L"Number %s \n" ,FaxPrintInfo.RecipientNumber);

        ZeroMemory(&FaxContextInfo, sizeof(FAX_CONTEXT_INFO));
        FaxContextInfo.SizeOfStruct = sizeof(FAX_CONTEXT_INFO);

        ZeroMemory(&CoverPageInfo, sizeof(FAX_COVERPAGE_INFO));
        CoverPageInfo.SizeOfStruct = sizeof(FAX_COVERPAGE_INFO);

        if (lptstrServerName) 
        {
                if(lptstrPrinterName)
                {
                        size_t szPrinter =  (_tcslen(lptstrServerName) + _tcslen(lptstrPrinterName) + 7);
                        lptstrFaxPrinterName = (LPTSTR)malloc( szPrinter * sizeof(TCHAR));
                        if(lptstrFaxPrinterName == NULL)
                        {
                                _tprintf(L"Malloc for lptstrFaxPrinterName failed");
                                bRetVal = false;
                                goto Exit1;
                        }
                        memset(lptstrFaxPrinterName,0 , szPrinter *sizeof(TCHAR));
                        StringCchCopy(lptstrFaxPrinterName,szPrinter, TEXT("\\\\"));
                        StringCchCat(lptstrFaxPrinterName,szPrinter, lptstrServerName);
                        StringCchCat(lptstrFaxPrinterName, szPrinter,TEXT("\\"));
                        StringCchCat(lptstrFaxPrinterName,szPrinter, lptstrPrinterName);
                }
                else
                {
                        size_t szPrinter =  (_tcslen(lptstrServerName) + 10 );
                        lptstrFaxPrinterName = (LPTSTR)malloc( szPrinter * sizeof(TCHAR));
                        if(lptstrFaxPrinterName == NULL)
                        {
                                _tprintf(L"Malloc for lptstrFaxPrinterName failed");
                                bRetVal = false;
                                goto Exit1;
                        }
                        memset(lptstrFaxPrinterName, 0, szPrinter * sizeof(TCHAR));
                        StringCchCopy(lptstrFaxPrinterName,szPrinter, TEXT("\\\\"));
                        StringCchCat(lptstrFaxPrinterName,szPrinter, lptstrServerName);
                        StringCchCat(lptstrFaxPrinterName, szPrinter,TEXT("\\"));
                        StringCchCat(lptstrFaxPrinterName,szPrinter, TEXT("Fax"));
                }
        }
        else
        {
                if(lptstrPrinterName)
                {
                        size_t szPrinter =  (_tcslen(lptstrPrinterName) + 7);
                        lptstrFaxPrinterName = (LPTSTR)malloc( szPrinter * sizeof(TCHAR));
                        if(lptstrFaxPrinterName == NULL)
                        {
                                _tprintf(L"Malloc for lptstrFaxPrinterName failed");
                                bRetVal = false;
                                goto Exit1;
                        }
                        memset(lptstrFaxPrinterName, 0, szPrinter*sizeof(TCHAR));
                        StringCchCopy(lptstrFaxPrinterName,szPrinter, lptstrPrinterName);
                }
                else
                {    
                        lptstrFaxPrinterName = (LPTSTR)malloc( 4 * sizeof(TCHAR));
                        if(lptstrFaxPrinterName == NULL)
                        {
                                _tprintf(L"Malloc for lptstrFaxPrinterName failed");
                                bRetVal = false;
                                goto Exit1;
                        }
                        memset(lptstrFaxPrinterName, 0, sizeof(TCHAR));
                        StringCchCopy(lptstrFaxPrinterName,4, TEXT("Fax"));
                }
        }

        _tprintf(L"Printer Name: %s \n", lptstrFaxPrinterName);

        // Start a fax
        if (!FaxStartPrintJob(lptstrFaxPrinterName,&FaxPrintInfo, &dwFaxId, &FaxContextInfo)) 
        {
                _tprintf(TEXT("Error, could not start print job on fax printer: %s\r\n"), lptstrFaxPrinterName);
                _tprintf(TEXT("  FaxStartPrintJob failed with error code 0x%08x.\r\n"), GetLastError());
                goto Exit;
        }

        CoverPageInfo.UseServerCoverPage = TRUE;

        CoverPageInfo.CoverPageName = lptstrCoverPageName;
        CoverPageInfo.Subject = lptstrCoverPageName;
        // Print a cover page
        _tprintf(TEXT("(Server .cov). File: %s \n"), lptstrCoverPageName);
        if (!FaxPrintCoverPage(&FaxContextInfo, &CoverPageInfo)) 
        {
                _tprintf(TEXT("Error, could not start print job for coverpage : %s\r\n"), lptstrCoverPageName);
                _tprintf(TEXT("  FaxPrintCoverPage failed with error code 0x%08x.\r\n"), GetLastError());
                goto Exit;
        }

        // Initialize GDI+.
        docInfo.cbSize = sizeof(docInfo);
        docInfo.lpszDocName = L"GdiplusPrint";


        if (!StartPage(FaxContextInfo.hDC))
        {
                _tprintf(TEXT(" Test Error: StartPage failed with error code 0x%08x"),GetLastError());
                goto Exit;
        }

        //draw Line, Rectangle and Ellipse
        Graphics* graphics = new Graphics(FaxContextInfo.hDC);
        if(graphics == NULL)
        {
                _tprintf(L"New for Graphics failed");
                bRetVal = false;
                goto Exit;
        }

        Pen* pen = new Pen(Color(255, 0, 0, 0));
        if(pen == NULL)
        {
                _tprintf(L"New for pen failed");
                bRetVal = false;
                goto Exit;
        }

        graphics->DrawLine(pen, 50, 50, 350, 550);
        graphics->DrawRectangle(pen, 50, 50, 300, 500);
        graphics->DrawEllipse(pen, 50, 50, 300, 500);

        delete pen;
        delete graphics;

        if (!EndPage(FaxContextInfo.hDC))
        {
                _tprintf(TEXT(" Test Error: EndPage failed with error code 0x%08x"),GetLastError());
        }

        if (!EndDoc(FaxContextInfo.hDC))
        {
                _tprintf(TEXT(" Test Error: EndDoc failed with error code 0x%08x"),GetLastError());
        }


Exit:
        if (!DeleteDC(FaxContextInfo.hDC))
        {
                _tprintf(TEXT(" Test Error: DeleteDC failed with error code 0x%08x"),GetLastError());
        }
Exit1:         
        GdiplusShutdown(gdiplusToken);        
        if(lptstrServerName)
                free(lptstrServerName);
        if(lptstrCoverPageName)
                free(lptstrCoverPageName);
        if(lptstrFaxPrinterName)
                free(lptstrFaxPrinterName);
        if(lptstrName)
                free(lptstrName);
        if(lptstrNumber)
                free(lptstrNumber);
        if(lptstrPrinterName)
                free(lptstrPrinterName);

        return bRetVal;

}
