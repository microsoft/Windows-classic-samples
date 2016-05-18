//---------------------------------------------------------------------
// This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//---------------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include "windows.h"
#include "xolehlp.h"
#include "txdtc.h"
#include "transact.h"
#include "winbase.h"

// Forward declarations
void ParseCommandLine(int argc, WCHAR** argv);
HRESULT CreateTransaction(ITransaction** ppITransaction);
HRESULT GetKernelTransactionHandle(ITransaction* pITransaction, HANDLE* pTransactionHandle);
HRESULT GetLocalTime(char* localTime, size_t sizeInBytes);
HRESULT TransactedFileOperation(HANDLE hTransactionHandle);
HRESULT CommitOrAbortTransaction(ITransaction* pITransaction);

// The sample uses this global boolean variable to determine if 
// the transaction will be aborted or committed at the end.
// The value is set according to the '-abort' command line switch.
BOOL g_fAbort = FALSE;

int __cdecl wmain(int argc, WCHAR* argv[])
{
    HRESULT hr = E_FAIL;
    
    ITransaction* pITransaction = NULL;
    HANDLE hTransactionHandle = INVALID_HANDLE_VALUE;

    // Parse the command line to see if -abort switch is used or not.
    // If the abort switch is used the transaction will be aborted at the end,
    // otherwise it will be committed.
    ParseCommandLine(argc, argv);

    // Get a pointer to a new transaction
    hr = CreateTransaction(&pITransaction);
    if (FAILED(hr)) 
        goto cleanup;

    // Get a transaction handle to use with the transacted file operation
    hr = GetKernelTransactionHandle(pITransaction, &hTransactionHandle);
    if (FAILED(hr)) 
        goto cleanup;

    // Do a transacted file operation
    hr = TransactedFileOperation(hTransactionHandle);
    if (FAILED(hr)) 
        goto cleanup;


    //-------------------------------------------------------------------------
    // Here you can do other operations to various Resource Managers as part 
    // of this transaction using the same ITransaction.
    //-------------------------------------------------------------------------


    // Commit or abort the transaction depending on the g_fAbort boolean variable
    // which was set by using the -abort command line parameter
    hr = CommitOrAbortTransaction(pITransaction);
    if (FAILED(hr)) 
        goto cleanup;

cleanup:
    
    if(INVALID_HANDLE_VALUE != hTransactionHandle)
    {
        CloseHandle(hTransactionHandle);
        hTransactionHandle = INVALID_HANDLE_VALUE;
    }
    
    if(NULL != pITransaction)
    {
        pITransaction->Release();
        pITransaction = NULL;
    }

    return 0;
}

void ParseCommandLine(int argc, WCHAR** argv)
{
    // Parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if ( 0 == _wcsnicmp( argv[i], L"-Abort",6) )
        {
            g_fAbort = TRUE;            
        }
        else
        {
            wprintf(L"\n\nUsage: %s [-Abort]\n\n",argv[0]);
            wprintf(L"     -Abort\n");
            wprintf(L"     If this parameter is given, the sample aborts the transaction.\n");
            wprintf(L"     Otherwise by default the transaction is committed.\n\n");
            exit (1);
        }
    }

    if (!g_fAbort)
    {
        wprintf(L"Transaction will be committed.\n\n");
    }
    else
    {
        wprintf(L"Transaction will be aborted.\n\n");
    }
}

HRESULT CreateTransaction(ITransaction** ppITransaction)
{
    HRESULT hr = E_FAIL;
    ITransactionDispenser* pITransactionDispenser = NULL;
    ITransaction* pITransaction = NULL;
        
    wprintf(L"Creating a transaction...\n");

    hr = DtcGetTransactionManagerEx(
        NULL, 
        NULL, 
        IID_ITransactionDispenser, 
        OLE_TM_FLAG_NONE, 
        NULL, 
        (void**) &pITransactionDispenser);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: Getting a transaction dispenser object failed. HR=0x%x\n", hr);
        goto cleanup;
    }

    // Set the transaction isolation level to ISOLATIONLEVEL_READCOMMITTED, 
    // since this is the isolation level supported by the transactional file system.
    hr = pITransactionDispenser->BeginTransaction(
        NULL, 
        ISOLATIONLEVEL_READCOMMITTED, 
        ISOFLAG_RETAIN_NONE, 
        NULL, 
        &pITransaction);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: Begin transaction call failed. HR=0x%x\n", hr);
        goto cleanup;
    }

    *ppITransaction = pITransaction;
    
cleanup:
    
    if(NULL != pITransactionDispenser)
    {
        pITransactionDispenser->Release();
        pITransactionDispenser = NULL;
    }
    
    return(hr);    
}

HRESULT GetKernelTransactionHandle(ITransaction* pITransaction, HANDLE* pTransactionHandle)
{
    HRESULT hr = E_FAIL;
    IKernelTransaction* pKernelTransaction = NULL;
    HANDLE hTransactionHandle = INVALID_HANDLE_VALUE;
    
    wprintf(L"Getting a transaction handle to use with transacted file operation...\n");
    
    // Query for IKernelTransaction interface for a handle to use with 
    // transacted file operation
    hr = pITransaction->QueryInterface(IID_IKernelTransaction, (void**) &pKernelTransaction);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: QueryInterface for IKernelTransaction failed with hr=0x%x\n", hr);
        goto cleanup;
    }
    hr = pKernelTransaction->GetHandle(&hTransactionHandle);
    if (FAILED(hr))
    {
        wprintf(L"ERROR: GetHandle call on IKernelTransaction failed with hr=0x%x\n", hr);
        goto cleanup;
    }

    *pTransactionHandle = hTransactionHandle;

cleanup:

    if(NULL != pKernelTransaction)
    {
        pKernelTransaction->Release();
        pKernelTransaction = NULL;
    }

    return(hr);
}

HRESULT GetLocalTime(char* localTime, size_t sizeInBytes)
{
    HRESULT hr = E_FAIL;
    
    struct tm newtime;
     __int64 ltime;
    
    errno_t err;
    
    // Get time
     _time64( &ltime );
    
    // Obtain the coordinated universal time 
    err = _localtime64_s( &newtime, &ltime );
    if (err)
    {
       wprintf(L"ERROR: Invalid argument to _localtime64_s.");
    }
       
    // Convert the universal time to an ASCII representation 
    err = asctime_s(localTime, sizeInBytes, &newtime);
    if (err)
    {
        wprintf(L"ERROR: Converting time to a string failed.\n");
    }
    else
    {
        wprintf(L"Local time is: %S", localTime);
        hr = S_OK;
    }

    return(hr);
}


HRESULT TransactedFileOperation(HANDLE hTransactionHandle)
{
    HRESULT hr = E_FAIL;
    HANDLE hTransactedFile = INVALID_HANDLE_VALUE;
    DWORD  dwBytesWritten, dwPos;

    const DWORD MAX_BUF = 128;
    char buf[MAX_BUF];

    wprintf(L"Open file...\n");

    // Open test.txt. If the file does not exist, create it.
    // Any operation done using the handle (hTransactedFile in this case)
    // will be transactional.
    hTransactedFile = CreateFileTransacted(L"test.txt",
        FILE_APPEND_DATA,       // open for writing
        FILE_SHARE_READ,        // allow multiple readers
        NULL,                   // no security
        OPEN_ALWAYS,            // open or create
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL,                   // do not set any attributes
        hTransactionHandle,     // pass the transaction handle
        NULL,                   // no mini version
        NULL);                  // reserved

    if (hTransactedFile == INVALID_HANDLE_VALUE)
    {
       wprintf(L"ERROR: Could not open test.txt.");
       goto cleanup;
    }

    // Any operation done using hTransactedFile handle will now be transactional

    // Append the current time to the file
    hr = GetLocalTime(buf, MAX_BUF);
    if (FAILED(hr))
        goto cleanup;
    wprintf(L"Appending the time to the file...\n");
    dwPos = SetFilePointer(hTransactedFile, 0, NULL, FILE_END);
    if (WriteFile(hTransactedFile, buf, (DWORD)strnlen(buf, MAX_BUF), &dwBytesWritten, NULL) == 0)
    {
        wprintf(L"ERROR: Failed to write to file...");
        goto cleanup;
    }
    char endOfLine[3] = "\r\n";
    if (WriteFile(hTransactedFile, endOfLine, 2, &dwBytesWritten, NULL) == 0)
    {
        wprintf(L"ERROR: Failed to write to file...");
        goto cleanup;
    }

cleanup:

    if(INVALID_HANDLE_VALUE != hTransactedFile)
    {
        CloseHandle(hTransactedFile);
        hTransactedFile = INVALID_HANDLE_VALUE;
    }
    return(hr);    
}

HRESULT CommitOrAbortTransaction(ITransaction* pITransaction)
{
    HRESULT hr = E_FAIL;

    if (g_fAbort == FALSE)
    {
        wprintf(L"Comitting transaction...\n");
        hr = pITransaction->Commit(FALSE, XACTTC_SYNC_PHASEONE, 0);
        if (FAILED(hr))
        {
            wprintf(L"ERROR: Commit operation failed. HR=0x%x\n", hr);
        }
        else
        {
            wprintf(L"Commit operation succeeded.\n\n");
        }
    }
    else
    {
        wprintf(L"Aborting transaction...\n");
        hr = pITransaction->Abort(
            NULL,                       // do not provide a reason for abort
            FALSE,                      // must be FALSE
            FALSE);                     // abort synchronous

        if (FAILED(hr))
        {
            wprintf(L"ERROR: Abort operation failed. HR=0x%x\n", hr);
        }
        else
        {
            wprintf(L"Abort operation succeeded.\n\n");
        }
    }

    return(hr);    
}
