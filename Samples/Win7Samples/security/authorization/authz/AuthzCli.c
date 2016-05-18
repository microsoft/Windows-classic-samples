/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2006 Microsoft Corporation.  All rights reserved.

Module Name:

    AuthzCli.c

Abstract:

    Sample client - will connect to sample Resource Manager AuthzSvr.


--*/

#include<windows.h>
#include<stdio.h>
#include<tchar.h>
#include"common.h"


void Usage(LPTSTR AppName)
{
   printf(
      "Usage: %s <\\\\server> <Personal|Corporate|Transfer> <Ammount (cents)>\n"
       ,AppName);
   ExitProcess(0);
}
   
   

void main (int argc, char *argv[])
{
   HANDLE   hPipe;
   TCHAR    szPipeName[256];
   TCHAR    szServerName[128] = "\\\\.";
   EX_BUF   exBuf = {0};
   DWORD    dwResponce = ERROR_ACCESS_DENIED;


   if (argc < 4)
      Usage(argv[0]);
   
   //
   // Verify expnese input
   //
   
   if (!lstrcmpi(argv[2],TEXT("Personal"))) {
      exBuf.dwType = ACCESS_FUND_PERSONAL;
   }
   else if (!lstrcmpi(argv[2],TEXT("Corporate"))) {
      exBuf.dwType = ACCESS_FUND_CORPORATE;
   }
   else if (!lstrcmpi(argv[2],TEXT("Transfer"))) {
      exBuf.dwType = ACCESS_FUND_TRANSFER;
   }
   else
      Usage(argv[0]);
   
   if (!(exBuf.dwAmmount = atoi(argv[3])))
       Usage(argv[0]);
   
   printf("expense: %s Ammount: %u\n",ExNames[exBuf.dwType],exBuf.dwAmmount); 
   
   
   _stprintf_s(szPipeName, 256 * sizeof(TCHAR),"%s\\pipe\\AuthzSamplePipe",szServerName);

   // Wait for an instance of the pipe
   if (!WaitNamedPipe(szPipeName,NMPWAIT_WAIT_FOREVER) )
      HandleError(GetLastError(),TEXT("WaitNamedPipe"),TRUE,TRUE);
   

   // Connect to pipe
   hPipe = CreateFile(  szPipeName, GENERIC_READ|GENERIC_WRITE, 0,
                        NULL, OPEN_EXISTING, 0, NULL);
   if (hPipe == INVALID_HANDLE_VALUE) 
      HandleError(GetLastError(),TEXT("CreateFile"),TRUE,TRUE);

   // Send off request
   WriteToPipe(hPipe, (PBYTE)&exBuf, sizeof(EX_BUF));
   
   // wait till server responds with one DWORD
   if (!ReadFromPipe(hPipe,(PBYTE)&dwResponce,sizeof(dwResponce))) {
      printf("Error reading form Svr\n");
      return;
   }

   switch(dwResponce)
   {
   case EXPENSE_APPROVED:
      printf("Expense Approved.\n");
      break;

   case ERROR_ACCESS_DENIED:
      printf("Expense denied: Access denied.\n");
      break;

   case ERROR_INSUFFICIENT_FUNDS:
      printf("Expense denied: Insufficeint funds.\n");
      break;

   default:
      printf("Expense failed: unexpected error.\n");
   }
    
   CloseHandle(hPipe);
}
