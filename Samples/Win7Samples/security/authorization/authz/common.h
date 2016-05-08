/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2006 Microsoft Corporation.  All rights reserved.

Module Name:

    Common.h


--*/
///////////////////////////////////////////////////////////////////////////////
// Access flags for funds RM
//
///////////////////////////////////////////////////////////////////////////////

//
// Personal expenditures
//
#define ACCESS_FUND_PERSONAL   0x00000001


//
// Company spending
//
#define ACCESS_FUND_CORPORATE  0x00000002


//
// Transfer to other funds
//
#define ACCESS_FUND_TRANSFER   0x00000004



///////////////////////////////////////////////////////////////////////////////
// Codes for expense access attempts
//
///////////////////////////////////////////////////////////////////////////////

//
// Expense approved and subtracted from fund.
//
#define EXPENSE_APPROVED 0

//
// We'll use existing ERROR_ACCESS_DENIED for access denied.
//
// ERROR_ACCESS_DENIED = 0x00000005

//
// Error with bit 29 set are private errors (see SetLastError doc)
//
#define PRIVATE_ERROR_BIT 0x20000000


//
// Expense failed insufficient funds
//
#define ERROR_INSUFFICIENT_FUNDS 0x20000002


//
// Expense failed due to an unknown error
//
#define EXPENSE_UNKNOWN_ERROR 0x20000003



///////////////////////////////////////////////////////////////////////////////
// Expense request packing struct
//
///////////////////////////////////////////////////////////////////////////////
typedef struct _ExpenseBuf
{
   DWORD dwAmmount;
   DWORD dwType;
} EX_BUF,*PEX_BUF;

LPTSTR ExNames[5] = { "\0", TEXT("PERSONAL"),TEXT("CORPORATE"),"\0",TEXT("TRANSFER") };



///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
//
///////////////////////////////////////////////////////////////////////////////

void HandleError(DWORD dwErr, TCHAR *pszAPI, BOOL fAPI, BOOL fExit);

DWORD DisplayAPIError(TCHAR *pszAPI,BOOL bConsole, 
                      BOOL bMsgBox, BOOL bExit);

BOOL SetupNamedPipe (PHANDLE phPipe, LPTSTR szPipeName);

BOOL WriteToPipe(HANDLE hPipe, BYTE* pData, DWORD cbData);

DWORD ReadFromPipe(HANDLE hPipe, BYTE* pBuffer, DWORD cbBuffer);

