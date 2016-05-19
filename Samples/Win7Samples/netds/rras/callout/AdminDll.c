/******************************************************************************
*       This is a part of the Microsoft Source Code Samples.
*       Copyright (C) 1992 - 2000 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       MprAdmin API usage and  is meant to help users with
*       MprAdmin API calling conventions. Use this code at your own risk.
*       Microsoft disclaims any implied warranty. This sample code can
*       be copied and distributed.
*
******************************************************************************/

/********************************************************************
*  admindll.c -- Sample program demonstrating the use of MprAdmin
*                     CallOut API's
*
*****************************************************************************/
/***************************************************************************
*  Functions:
*        Init(void);
*        Init2(void);
*        Debug_Print_RasConnection(
*                       IN RAS_CONNECTION_0 *   pRasConnection0,
*                       IN RAS_CONNECTION_1 *   pRasConnection1)
*
*        File_Print_RasConnection(
*                       IN RAS_CONNECTION_0 *   pRasConnection0,
*                       IN RAS_CONNECTION_1 *   pRasConnection1)
*
*        Debug_Print_RasPort(
*                       IN RAS_PORT_0 *   pRasPort0,
*                       IN RAS_PORT_1 *   pRasPort1)
*
*        File_Print_RasPort(
*                       IN RAS_PORT_0 *   pRasPort0,
*                       IN RAS_PORT_1 *   pRasPort1)
*****************************************************************************/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <lm.h>
#include <stdio.h>
#include <time.h>
#include <mprapi.h>
#include <memory.h>
#include "Shlwapi.h" // Platform SDK wasn't including this .h file so it's part of this project
#include <strsafe.h>

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

//Functions
VOID
Debug_Print_RasConnection(
   IN RAS_CONNECTION_0 *   pRasConnection0,
   IN RAS_CONNECTION_1 *   pRasConnection1
   );

VOID
File_Print_RasConnection(IN RAS_CONNECTION_0 *   pRasConnection0,
   IN RAS_CONNECTION_1 *   pRasConnection1
   );

VOID
Debug_Print_RasPort(
   IN RAS_PORT_0 *   pRasPort0,
   IN RAS_PORT_1 *   pRasPort1
   );

VOID
File_Print_RasPort(
   IN RAS_PORT_0 *   pRasPort0,
   IN RAS_PORT_1 *   pRasPort1
);

VOID Init(void);
VOID Init2(void);
VOID Finalize(void);

//Global Variables
#define Number_Of_Flags 6
#define Num_Ip_Address 254

DWORD  TempIpAddress = 0x010A0F0E;
HANDLE IPFile = NULL;
HANDLE ConDisFile = NULL;
HANDLE ErrLogFile = NULL;
HANDLE ErrLogFileIP = NULL;
BOOL IPFileInValid = FALSE;
BOOL ConnectDisconnectInValid = FALSE;
BOOL ErrorLogInValid = FALSE;
BOOL ErrLogIPInValid = FALSE;
BOOL First_Call = TRUE;

WCHAR*
StrDupWFromA(
    CHAR* psz );

static DWORD Multilink_Count = 0;
HANDLE Connection_Handle;
/* the above two variables are to control the number of ports a connection */
/* can use for multilink.  Multilink_Count can be set via regedit.exe to   */
/* make it little more portable */


typedef struct Static_Info_Database
   {
      BOOL bTaken;
      HANDLE hConnection;
      RAS_CONNECTION_0 pRasConnection0;
      RAS_CONNECTION_1 pRasConnection1;
   } Static_Info_Database;

Static_Info_Database  Static_Connection_Info[Num_Ip_Address];

typedef struct Static_Statistic
   {
      WCHAR wszUserName[UNLEN+1];
      WCHAR wszPortName[MAX_PORT_NAME];
      DWORD IpAddress;
      BOOL bTaken;
   } Static_Statistic;

Static_Statistic  Static_Check_Stats[Num_Ip_Address];


WCHAR*
StrDupWFromA(
    CHAR* psz )

    /* Returns heap block containing a copy of 0-terminated string 'psz' or
    ** NULL on error or if 'psz' is NULL.  The output string is converted to
    ** UNICODE.  It is caller's responsibility to Free the returned string.
    */
{
   WCHAR* pszNew = NULL;

   if (psz)
   {
      DWORD cb;

      cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, NULL, 0 );
      pszNew =(WCHAR*)malloc( cb * sizeof(TCHAR) );
      if (!pszNew)
      {
         return NULL;
      }

      cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, pszNew, cb );
      if (cb == 0)
      {
         free( pszNew );
         return NULL;
      }
   }

   return pszNew;

}



//
// Format and write debug information to OutputDebugString
//
ULONG
_cdecl
DbgPrint(
    PCH Format,
    ...
    )
{
    WCHAR buffer[255];
    WCHAR *temp=NULL;
    LPINT lpResult=0;
    va_list marker;
    va_start (marker,Format);

    temp = StrDupWFromA(Format);
    if (temp)
    {
        StringCchVPrintf((LPTSTR)buffer, CELEMS(buffer), (LPTSTR)temp, marker);
        OutputDebugString ((LPTSTR)buffer);
        free(temp);
    }

    return TRUE;

}



//
// Called from MprAdminGetIpAddressForUser, which itself gets called by RasMan
//
VOID Init(void)
{
INT i=0;
        
   IPFile = CreateFile(TEXT("IpAddress.log"),
                       GENERIC_READ|GENERIC_WRITE,
                       FILE_SHARE_READ,
                       (LPSECURITY_ATTRIBUTES) NULL,
                       CREATE_ALWAYS,
                       0,
                       (HANDLE) NULL);
   if (IPFile== INVALID_HANDLE_VALUE)
   {
      IPFileInValid = TRUE;
   }
   ErrLogFileIP = CreateFile(TEXT("IpAddError.log"),
                             GENERIC_READ|GENERIC_WRITE,
                             FILE_SHARE_READ,
                             (LPSECURITY_ATTRIBUTES) NULL,
                             CREATE_ALWAYS,
                             0,
                             (HANDLE) NULL);

   if (ErrLogFileIP== INVALID_HANDLE_VALUE)
   {
      ErrLogIPInValid = TRUE;
   }

}


//
// Called by MprAdminAcceptNewConnection, itself gets called by RAS Server
//
VOID Init2(void)
{
INT i=0;

   ConDisFile = CreateFile(TEXT("ConDis.log"),
                           GENERIC_READ|GENERIC_WRITE,
                           FILE_SHARE_READ,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           OPEN_ALWAYS,
                           0,
                           (HANDLE) NULL);
   if (ConDisFile== INVALID_HANDLE_VALUE)
   {
      ConnectDisconnectInValid = TRUE;
   }

   ErrLogFile = CreateFile(TEXT("AdminErr.log"),
                           GENERIC_READ|GENERIC_WRITE,
                           FILE_SHARE_READ,
                           (LPSECURITY_ATTRIBUTES) NULL,
                           OPEN_ALWAYS,
                           0,
                           (HANDLE) NULL);

   if (ErrLogFile == INVALID_HANDLE_VALUE)
   {
      ErrorLogInValid = TRUE;
   }

   for (i=0;i < Num_Ip_Address; i++)
   {
      Static_Check_Stats[i].bTaken=FALSE;
      TempIpAddress += 0x01000000;

      if ((TempIpAddress | 0x0ff000000) == 0x0ff000000)
         TempIpAddress = 0x010A0F0E;

      Static_Check_Stats[i].IpAddress = TempIpAddress;
      Static_Connection_Info[i].bTaken = FALSE;
   }

}

VOID Finalize() 
{
    CloseHandle(IPFile);
    CloseHandle(ErrLogFileIP);
    CloseHandle(ConDisFile);
    CloseHandle(ErrLogFile);
}


BOOL DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason )
    {
       case DLL_PROCESS_ATTACH:
       // Initialize once for each new process.
       // Return FALSE to fail DLL load.
        break;

       case DLL_THREAD_ATTACH:
       // Do thread-specific initialization.
       break;


       case DLL_THREAD_DETACH:
       // Do thread-specific cleanup.
       break;

       case DLL_PROCESS_DETACH:
       // Perform any necessary cleanup.
       break;

    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

DWORD APIENTRY
MprAdminTerminateDll( ) {
    DbgPrint("\nMprAdminTerminateDll called\n");
    Finalize();
    return TRUE;

}



DWORD APIENTRY
MprAdminGetIpAddressForUser(
    IN WCHAR *      lpszUserName,
    IN WCHAR *      lpszPortName,
    IN OUT DWORD * pipAddress,
    OUT BOOL *      pfNotifyDLL
)
{
DWORD i=0;
DWORD dwBytesWritten=0;
CHAR buf[120];

    if (NULL == lpszUserName || NULL == lpszPortName || NULL == pipAddress || NULL == pfNotifyDLL)
    {
        return ERROR_INVALID_PARAMETER;
    }

   //to check if the call is first rasman call out. If yes, call Initialize routine
   if (First_Call)
   {
      Init();
      Init2();
      First_Call = FALSE;
   }
   DbgPrint("First_Call  %d\n",First_Call);
   DbgPrint("\nGetIpAddressForUser called UserName=%ws,Port=%ws,IpAddress=%d\n",
            lpszUserName,lpszPortName,(DWORD)*pipAddress );

   for (i=0;i < Num_Ip_Address;i++)
   {
      if (Static_Check_Stats[i].bTaken == FALSE)
      {
         StringCchCopy((LPTSTR)Static_Check_Stats[i].wszUserName, CELEMS(Static_Check_Stats[i].wszUserName),(LPTSTR)lpszUserName);
         StringCchCopy((LPTSTR)Static_Check_Stats[i].wszPortName, CELEMS(Static_Check_Stats[i].wszPortName),(LPTSTR)lpszPortName);
         *pipAddress = Static_Check_Stats[i].IpAddress;
         DbgPrint("Trying to get the IP Address   %d",(DWORD) *pipAddress);
         Static_Check_Stats[i].bTaken=TRUE;
         break;
      }
   }
   DbgPrint("GetIpAddressForUser setting IpAddress=%d\n",(DWORD)*pipAddress );
   StringCchPrintf((LPTSTR)buf,CELEMS(buf), (LPTSTR)"GetIpAddressForUser setting IpAddress=%d\n",(DWORD)*pipAddress );
   WriteFile(IPFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   *pfNotifyDLL = TRUE;

   return( NO_ERROR );
}


VOID APIENTRY
MprAdminReleaseIpAddress(
    IN WCHAR *      lpszUserName,
    IN WCHAR *      lpszPortName,
    IN OUT DWORD * pipAddress
)
{
DWORD dwBytesWritten=0;
CHAR buf[120];
int i=0;

    if (NULL == lpszUserName || NULL == lpszPortName)
    {
        DbgPrint( "\nMprAdminReleaseIpAddress - Invalid Parameter");
        return;
    }

    DbgPrint( "\nReleaseIpAddressr called UserName=%ws,Port=%ws,IpAddress=%d\n",
              lpszUserName,lpszPortName,(DWORD)*pipAddress );

    for (i=0;i < Num_Ip_Address;i++)
    {
       if (Static_Check_Stats[i].IpAddress == (DWORD) *pipAddress)
       {
          if (Static_Check_Stats[i].bTaken == TRUE)
          {
             if (wcscmp(Static_Check_Stats[i].wszUserName,lpszUserName) == 0)
             {
                if (wcscmp(Static_Check_Stats[i].wszPortName,lpszPortName) == 0)
                {
                   Static_Check_Stats[i].bTaken = FALSE;
                   break;
                }
                else
                {
                   DbgPrint("ERROR ReleaseIP PortName, Cannot free IP Address    %ws\n",
                            Static_Check_Stats[i].wszPortName);
                   if (!(ErrLogIPInValid))
                   {
                      StringCchPrintf((LPTSTR)buf,CELEMS(buf),(LPTSTR)"ERROR ReleaseIP PortName, Cannot free IP Address   %ws\n",
                              Static_Check_Stats[i].wszPortName);
                      WriteFile(ErrLogFileIP, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
                      break;
                   }
                }
             }
             else
             {
                DbgPrint("ERROR ReleaseIP UserName Incorrect, Cannot free IP Address   %ws\n",
                         Static_Check_Stats[i].wszUserName);
                if (!(ErrLogIPInValid))
                {
                   StringCchPrintf((LPTSTR)buf,CELEMS(buf), (LPTSTR)"ERROR ReleaseIP UserName Incorrect, Cannot free IP Address   %ws\n",
                           Static_Check_Stats[i].wszUserName);
                   WriteFile(ErrLogFileIP, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
                }
                break;
             }
          }
          else
          {
             DbgPrint("ERROR ReleaseIP Wrong IP address, doesn't exist, Cannot free IP Address    %d\n",
                      Static_Check_Stats[i].IpAddress);
             if (!(ErrLogIPInValid))
             {
                StringCchPrintf((LPTSTR)buf,CELEMS(buf), (LPTSTR)"ERROR ReleaseIP Wrong IP Address, doesn't exist Cannot free IP Address   %d\n",
                        Static_Check_Stats[i].IpAddress);
                WriteFile(ErrLogFileIP, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
             }
             break;
          }
       }
    }

}


BOOL APIENTRY
MprAdminAcceptNewConnection(
   IN RAS_CONNECTION_0 *   pRasConnection0,
   IN RAS_CONNECTION_1 *   pRasConnection1
)
{
INT i=0;
CHAR buf[512];
DWORD dwBytesWritten=0;

   DbgPrint("MprAdminAcceptNewConnection Called\n");

   if (NULL == pRasConnection0 || NULL == pRasConnection1)
   {
       DbgPrint("MprAdminAcceptNewConnection Invalid Parameter\n");
        return FALSE;
   }

   Debug_Print_RasConnection(pRasConnection0,pRasConnection1);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf,CELEMS(buf), (LPTSTR)"MprAdminAcceptNewConnection Called::dumping pRasConnection0\n\n");
   File_Print_RasConnection(pRasConnection0,pRasConnection1);

   if (( lstrcmp((LPCSTR) pRasConnection0->wszUserName, TEXT("Administrator") ) ) == 0)
   {
      DbgPrint("Do not Accept Connection\n");
      FlushFileBuffers(ErrLogFile);
      FlushFileBuffers(ConDisFile);
      return( FALSE );
   }
   else
   {
      for (i=0;i < Num_Ip_Address; i++)
         if (!(Static_Connection_Info[i].bTaken))
         {
            Static_Connection_Info[i].bTaken = TRUE;
            Static_Connection_Info[i].hConnection=pRasConnection0->hConnection;
            memcpy(&Static_Connection_Info[i].pRasConnection0,pRasConnection0, sizeof(RAS_CONNECTION_0));
            memcpy(&Static_Connection_Info[i].pRasConnection1,pRasConnection1, sizeof(RAS_CONNECTION_1));
            break;
         }

       DbgPrint("Accept Connection\n");
       FlushFileBuffers(ErrLogFile);
       FlushFileBuffers(ConDisFile);
       return( TRUE );
    }
    return( TRUE );
}


BOOL APIENTRY
MprAdminAcceptNewLink(
   IN RAS_PORT_0 *   pRasPort0,
   IN RAS_PORT_1 *   pRasPort1
)
{
CHAR  buf[512];
DWORD dwBytesWritten = 0;

    if (NULL == pRasPort0 || NULL == pRasPort1)
    {
        return FALSE;
    }

   Multilink_Count++;
   /* Multilink_Count can be set through regedit.exe to make it little more portable */
   /* Currently it is hard coded in this sample, so that no connection can have more than 2 ports */
   if (Multilink_Count == 1)
      Connection_Handle = pRasPort0->hConnection;/* this is the first port */

   //to check if the call is first rassrv call out. If yes, call Initialize routine
   DbgPrint("MprAdminAcceptNewLink Called, times  %d\n",Multilink_Count);
   Debug_Print_RasPort(pRasPort0,pRasPort1);

   //dumping pRasPort0 to a file
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf,CELEMS(buf), (LPTSTR)"MprAdminAcceptNewLink Called time %d::dumping pRasPort0 to file\n",Multilink_Count);
   File_Print_RasPort(pRasPort0,pRasPort1);

   /* if the Connection_Handle is not same then it is a new call, reset the counter to 1 */
   if (Connection_Handle != pRasPort0->hConnection)
   {
      if (Multilink_Count > 1)
      {
         Connection_Handle = pRasPort0->hConnection;
         Multilink_Count = 1;
         return TRUE;
      }
      else
         return FALSE;
   }
   /* Connection_Handle same as previous port then restrict the number of ports. */
   /* Just return FALSE */
   if (Multilink_Count > 2)
   {
      DbgPrint("MprAdminAcceptNewLink(): Multilink_Count=%d > 2  so return FALSE\n", Multilink_Count);
      return FALSE;
   }

   return TRUE;

}


VOID APIENTRY
MprAdminLinkHangupNotification(
   IN RAS_PORT_0 *   pRasPort0,
   IN RAS_PORT_1 *   pRasPort1
)
{
   char buf[512];
   DWORD dwBytesWritten = 0;

   DbgPrint("MprAdminLinkHangupNotification Called\n");

   if (NULL == pRasPort0 || NULL == pRasPort1)
   {
       DbgPrint("MprAdminLinkHangupNotification Invalid Parameter\n");

       return;
   }

   Debug_Print_RasPort(pRasPort0,pRasPort1);
   //dumping pRasPort0 to a file
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf,CELEMS(buf), (LPTSTR)"MprAdminLinkHangupNotification Called::dumping pRasPort0 to file\n");
   File_Print_RasPort(pRasPort0,pRasPort1);

}


VOID APIENTRY
MprAdminConnectionHangupNotification(
   IN RAS_CONNECTION_0 *   pRasConnection0,
   IN RAS_CONNECTION_1 *   pRasConnection1
)
{
INT i = 0;
DWORD dwBytesWritten = 0;
CHAR  buf[120];

   DbgPrint( "\nMprAdminConnectionHangupNotification called \n" );

    if (NULL == pRasConnection0 || NULL == pRasConnection1)
    {
        DbgPrint( "\nMprAdminConnectionHangupNotification Invalid Parameter\n" );
        return;
    }

   Debug_Print_RasConnection(pRasConnection0,pRasConnection1);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"MprAdminConnectionHangupNotification called::dumping pRasConnection0\n\n");
   File_Print_RasConnection(pRasConnection0,pRasConnection1);

   //Sanity checks
   for (i=0; i < Num_Ip_Address; i++)
   {
      if ((Static_Connection_Info[i].bTaken) && (Static_Connection_Info[i].hConnection == pRasConnection0->hConnection))
      {
         Static_Connection_Info[i].bTaken = FALSE;
         if (ErrorLogInValid)
         {
            if (Static_Connection_Info[i].pRasConnection0.hConnection != pRasConnection0->hConnection)
               DbgPrint("ERROR Connection Handle differs   %d-> Stored,    %d->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.hConnection, pRasConnection0->hConnection);

            if (Static_Connection_Info[i].pRasConnection0.hInterface != pRasConnection0->hInterface)
               DbgPrint("ERROR Interface Handle differs   %d-> Stored,    %d->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.hInterface, pRasConnection0->hInterface);

            if (Static_Connection_Info[i].pRasConnection0.dwConnectDuration > pRasConnection0->dwConnectDuration)
               DbgPrint("ERROR Duration time is less %d-> Stored,    %d->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.dwConnectDuration, pRasConnection0->dwConnectDuration);

            if (Static_Connection_Info[i].pRasConnection0.dwInterfaceType != pRasConnection0->dwInterfaceType)
               DbgPrint("ERROR Interface Type differs   %d-> Stored,    %d->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.dwInterfaceType != pRasConnection0->dwInterfaceType);

            if (Static_Connection_Info[i].pRasConnection0.dwConnectionFlags != pRasConnection0->dwConnectionFlags)
               DbgPrint("ERROR Connection Flags differs   %d-> Stored,    %d->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.dwConnectionFlags, pRasConnection0->dwConnectionFlags);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszInterfaceName,pRasConnection0->wszInterfaceName) != 0)
               DbgPrint("ERROR Interface Name Differs    %ws ->Stored,   %ws ->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.wszInterfaceName,pRasConnection0->wszInterfaceName);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszUserName,pRasConnection0->wszUserName) != 0)
               DbgPrint("ERROR User Name Differs    %ws ->Stored,   %ws ->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.wszUserName,pRasConnection0->wszUserName);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszLogonDomain,pRasConnection0->wszLogonDomain) != 0)
               DbgPrint("ERROR Logon Name Differs    %ws ->Stored,   %ws ->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.wszLogonDomain,pRasConnection0->wszLogonDomain);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszRemoteComputer,pRasConnection0->wszRemoteComputer) != 0)
               DbgPrint("ERROR Remote Name Differs    %ws ->Stored,   %ws ->Passed\n",
                        Static_Connection_Info[i].pRasConnection0.wszRemoteComputer,pRasConnection0->wszRemoteComputer);
         }
         else
         {
            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
            if (Static_Connection_Info[i].pRasConnection0.hConnection != pRasConnection0->hConnection)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Connection Handle differs   %l64d-> Stored,    %l64d->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.hConnection, pRasConnection0->hConnection);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
            if (Static_Connection_Info[i].pRasConnection0.hInterface != pRasConnection0->hInterface)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Interface Handle differs   %l64d-> Stored,    %l64d->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.hInterface, pRasConnection0->hInterface);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
            if (Static_Connection_Info[i].pRasConnection0.dwConnectDuration > pRasConnection0->dwConnectDuration)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Duration time is less %d-> Stored,    %d->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.dwConnectDuration, pRasConnection0->dwConnectDuration);
            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

            if (Static_Connection_Info[i].pRasConnection0.dwInterfaceType != pRasConnection0->dwInterfaceType)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Interface Type differs   %l64d-> Stored,    %l64d->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.dwInterfaceType != pRasConnection0->dwInterfaceType);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

            if (Static_Connection_Info[i].pRasConnection0.dwConnectionFlags != pRasConnection0->dwConnectionFlags)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Connection Flags differs   %d-> Stored,    %d->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.dwConnectionFlags, pRasConnection0->dwConnectionFlags);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszInterfaceName,pRasConnection0->wszInterfaceName) != 0)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Interface Name Differs    %ws ->Stored,   %ws ->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.wszInterfaceName,pRasConnection0->wszInterfaceName);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszUserName,pRasConnection0->wszUserName) != 0)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR User Name Differs    %ws ->Stored,   %ws ->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.wszUserName,pRasConnection0->wszUserName);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszLogonDomain,pRasConnection0->wszLogonDomain) != 0)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Logon Name Differs    %ws ->Stored,   %ws ->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.wszLogonDomain,pRasConnection0->wszLogonDomain);

            WriteFile(ErrLogFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

            if (wcscmp(Static_Connection_Info[i].pRasConnection0.wszRemoteComputer,pRasConnection0->wszRemoteComputer) != 0)
               StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"ERROR Remote Name Differs    %ws ->Stored,   %ws ->Passed\n",
                       Static_Connection_Info[i].pRasConnection0.wszRemoteComputer,pRasConnection0->wszRemoteComputer);
         }
      }
      FlushFileBuffers(ErrLogFile);
      FlushFileBuffers(ConDisFile);
   }

}

VOID Debug_Print_RasConnection(
   IN RAS_CONNECTION_0 *   pRasConnection0,
   IN RAS_CONNECTION_1 *   pRasConnection1
)
{
WCHAR *Interface_Type[] =
   {
   L"ROUTER_IF_TYPE_CLIENT",
   L"ROUTER_IF_TYPE_HOME_ROUTER",
   L"ROUTER_IF_TYPE_FULL_ROUTER",
   L"ROUTER_IF_TYPE_DEDICATED",
   L"ROUTER_IF_TYPE_INTERNAL",
   L"ROUTER_IF_TYPE_INTERNAL"
   };

INT i = 0;
DWORD dwBytesWritten = 0;

   if (NULL == pRasConnection0 || NULL == pRasConnection1)
   {
      return;
   }

   //dumping pRasConnection0
   DbgPrint("pRasConnection0->hConnection  %d\n",pRasConnection0->hConnection);
   DbgPrint("pRasConnection0->hInterface   %d\n",pRasConnection0->hInterface);
   DbgPrint("pRasConnection0->dwConnectDuration  %d\n",pRasConnection0->dwConnectDuration);
   DbgPrint("pRasConnection0->dwInterfaceType  %ws\n",Interface_Type[pRasConnection0->dwInterfaceType]);
   DbgPrint("pRasConnection0->dwConnectionFlags  %d\n",pRasConnection0->dwConnectionFlags);
   DbgPrint("pRasConnection0->wszInterfaceName  %ws\n",pRasConnection0->wszInterfaceName);
   DbgPrint("pRasConnection0->wszUserName  %ws\n",pRasConnection0->wszUserName);
   DbgPrint("pRasConnection0->wszLogonDomain  %ws\n",pRasConnection0->wszLogonDomain);
   DbgPrint("pRasConnection0->wszRemoteComputer  %ws\n",pRasConnection0->wszRemoteComputer);

   //dumping pRasConnection1
   DbgPrint( "\n\npRasConnection1->hConnection  %d\n",pRasConnection1->hConnection);
   DbgPrint( "pRasConnection1->hInterface   %d\n",pRasConnection1->hInterface);
   DbgPrint( "pRasConnection1->PppInfo.nbf.dwError %d\n",pRasConnection1->PppInfo.nbf.dwError);
   DbgPrint( "pRasConnection1->PppInfo.nbf.wszWksta %ws\n",pRasConnection1->PppInfo.nbf.wszWksta);
   DbgPrint( "pRasConnection1->PppInfo.ip.dwError %d\n",pRasConnection1->PppInfo.ip.dwError);
   DbgPrint( "pRasConnection1->PppInfo.ip.wszAddress %ws\n",pRasConnection1->PppInfo.ip.wszAddress);
   DbgPrint( "pRasConnection1->PppInfo.ip.wszRemoteAddress %ws\n",pRasConnection1->PppInfo.ip.wszRemoteAddress);
   DbgPrint( "pRasConnection1->PppInfo.ipx.dwError %d\n",pRasConnection1->PppInfo.ipx.dwError);
   DbgPrint( "pRasConnection1->PppInfo.ipx.wszAddress %ws\n",pRasConnection1->PppInfo.ipx.wszAddress);
   DbgPrint( "pRasConnection1->PppInfo.at.dwError  %d\n",pRasConnection1->PppInfo.at.dwError );
   DbgPrint( "pRasConnection1->PppInfo.at.wszAddress %ws\n",pRasConnection1->PppInfo.at.wszAddress);

   //connection stats
   DbgPrint( "pRasConnection1->dwBytesXmited=%d\n", pRasConnection1->dwBytesXmited );
   DbgPrint( "pRasConnection1->dwBytesRcved=%d\n", pRasConnection1->dwBytesRcved );
   DbgPrint( "pRasConnection1->dwFramesXmited=%d\n", pRasConnection1->dwFramesXmited );
   DbgPrint( "pRasConnection1->dwFramesRcved=%d\n", pRasConnection1->dwFramesRcved);
   DbgPrint( "pRasConnection1->dwCrcErr=%d\n", pRasConnection1->dwCrcErr);
   DbgPrint( "pRasConnection1->dwTimeoutErr=%d\n", pRasConnection1->dwTimeoutErr);
   DbgPrint( "pRasConnection1->dwAlignmentErr=%d\n", pRasConnection1->dwAlignmentErr);
   DbgPrint( "pRasConnection1->dwHardwareOverrunErr=%d\n", pRasConnection1->dwHardwareOverrunErr);
   DbgPrint( "pRasConnection1->dwFramingErr=%d\n", pRasConnection1->dwFramingErr);
   DbgPrint( "pRasConnection1->dwBufferOverrunErr=%d\n", pRasConnection1->dwBufferOverrunErr);
}



VOID File_Print_RasConnection(IN RAS_CONNECTION_0 *   pRasConnection0,
        IN RAS_CONNECTION_1 *   pRasConnection1
)
{
   WCHAR *Interface_Type[] =
     {
     L"ROUTER_IF_TYPE_CLIENT",
     L"ROUTER_IF_TYPE_HOME_ROUTER",
     L"ROUTER_IF_TYPE_FULL_ROUTER",
     L"ROUTER_IF_TYPE_DEDICATED",
     L"ROUTER_IF_TYPE_INTERNAL",
     L"ROUTER_IF_TYPE_INTERNAL"
     };


   INT  i = 0;
   CHAR buf[512];
   DWORD dwBytesWritten = 0;

   if (NULL == pRasConnection0 || NULL == pRasConnection1)
   {
       return;
   }

   //write to file
   //dumping pRasConnection0
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->hConnection  %l64d\n",pRasConnection0->hConnection);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->hInterface   %l64d\n",pRasConnection0->hInterface);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->dwConnectDuration  %d\n",pRasConnection0->dwConnectDuration);

   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->dwInterfaceType  %ws\n",Interface_Type[pRasConnection0->dwInterfaceType]);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->dwConnectionFlags  %d\n",pRasConnection0->dwConnectionFlags);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->wszInterfaceName  %ws\n",pRasConnection0->wszInterfaceName);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->wszUserName  %ws\n",pRasConnection0->wszUserName);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->wszLogonDomain  %ws\n",pRasConnection0->wszLogonDomain);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasConnection0->wszRemoteComputer  %ws\n",pRasConnection0->wszRemoteComputer);

   //dumping pRasConnection1
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"dumping pRasConnection1\n");
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"\n\npRasConnection1->hConnection  %l64d\n",pRasConnection1->hConnection);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->hInterface   %l64d\n",pRasConnection1->hInterface);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.nbf.dwError %d\n",pRasConnection1->PppInfo.nbf.dwError);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.nbf.wszWksta %ws\n",pRasConnection1->PppInfo.nbf.wszWksta);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.ip.dwError %d\n",pRasConnection1->PppInfo.ip.dwError);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.ip.wszAddress %ws\n",pRasConnection1->PppInfo.ip.wszAddress);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.ip.wszRemoteAddress %ws\n",pRasConnection1->PppInfo.ip.wszRemoteAddress);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.ipx.dwError %d\n",pRasConnection1->PppInfo.ipx.dwError);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.ipx.wszAddress %ws\n",pRasConnection1->PppInfo.ipx.wszAddress);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.at.dwError  %d\n",pRasConnection1->PppInfo.at.dwError );
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->PppInfo.at.wszAddress %ws\n",pRasConnection1->PppInfo.at.wszAddress);

   //connection stats
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwBytesXmited=%d\n", pRasConnection1->dwBytesXmited );
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwBytesRcved=%d\n", pRasConnection1->dwBytesRcved );
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwFramesXmited=%d\n", pRasConnection1->dwFramesXmited );
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwFramesRcved=%d\n", pRasConnection1->dwFramesRcved);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwCrcErr=%d\n", pRasConnection1->dwCrcErr);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwTimeoutErr=%d\n", pRasConnection1->dwTimeoutErr);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwAlignmentErr=%d\n", pRasConnection1->dwAlignmentErr);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwHardwareOverrunErr=%d\n", pRasConnection1->dwHardwareOverrunErr);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwFramingErr=%d\n", pRasConnection1->dwFramingErr);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
   StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasConnection1->dwBufferOverrunErr=%d\n", pRasConnection1->dwBufferOverrunErr);
   WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
}

VOID Debug_Print_RasPort(
    IN RAS_PORT_0 *   pRasPort0,
    IN RAS_PORT_1 *   pRasPort1
)
{
   WCHAR *Line_Condition[] =
      {
      L"RAS_PORT_NON_OPERATIONAL",
      L"RAS_PORT_DISCONNECTED",
      L"RAS_PORT_CALLING_BACK",
      L"RAS_PORT_LISTENING",
      L"RAS_PORT_AUTHENTICATING",
      L"RAS_PORT_AUTHENTICATED",
      L"RAS_PORT_INITIALIZING"
      };

   WCHAR *Hdw_Error[] =
      {
      L"RAS_HARDWARE_OPERATIONAL",
      L"RAS_HARDWARE_FAILURE"
      };

   DWORD dwBytesWritten = 0;

   if (NULL == pRasPort0 || NULL == pRasPort1)
   {
        return;
   }

  //dumping pRasPort0
   DbgPrint("pRasPort0->hPort  %d\n",pRasPort0->hPort);
   DbgPrint("pRasPort0->hConnection   %d\n",pRasPort0->hConnection);
   DbgPrint("pRasPort0->dwPortCondition  %ws\n",Line_Condition[pRasPort0->dwPortCondition-1]);
   DbgPrint("pRasPort0->dwTotalNumberOfCalls  %d\n",pRasPort0->dwTotalNumberOfCalls);
   DbgPrint("pRasPort0->dwConnectDuration  %d\n",pRasPort0->dwConnectDuration);
   DbgPrint("pRasPort0->wszPortName  %ws\n",pRasPort0->wszPortName);
   DbgPrint("pRasPort0->wszMediaName  %ws\n",pRasPort0->wszMediaName);
   DbgPrint("pRasPort0->wszDeviceName  %ws\n",pRasPort0->wszDeviceName);
   DbgPrint("pRasPort0->wszDeviceType  %ws\n",pRasPort0->wszDeviceType);

   //dumping pRasPort1
   DbgPrint("pRasPort1->hPort  %d\n",pRasPort1->hPort);
   DbgPrint("pRasPort1->hConnection   %d\n",pRasPort1->hConnection);
   DbgPrint("pRasPort1->dwHardwareCondition  %ws\n",Hdw_Error[pRasPort1->dwHardwareCondition]);

   //connection stats
   DbgPrint( "pRasPort1->dwLineSpeed=%d\n", pRasPort1->dwLineSpeed );
   DbgPrint( "pRasPort1->dwBytesXmited=%d\n", pRasPort1->dwBytesXmited );
   DbgPrint( "pRasPort1->dwBytesRcved=%d\n", pRasPort1->dwBytesRcved );
   DbgPrint( "pRasPort1->dwFramesXmited=%d\n", pRasPort1->dwFramesXmited );
   DbgPrint( "pRasPort1->dwFramesRcved=%d\n", pRasPort1->dwFramesRcved);
   DbgPrint( "pRasPort1->dwCrcErr=%d\n", pRasPort1->dwCrcErr);
   DbgPrint( "pRasPort1->dwTimeoutErr=%d\n", pRasPort1->dwTimeoutErr);
   DbgPrint( "pRasPort1->dwAlignmentErr=%d\n", pRasPort1->dwAlignmentErr);
   DbgPrint( "pRasPort1->dwHardwareOverrunErr=%d\n", pRasPort1->dwHardwareOverrunErr);
   DbgPrint( "pRasPort1->dwFramingErr=%d\n", pRasPort1->dwFramingErr);
   DbgPrint( "pRasPort1->dwBufferOverrunErr=%d\n", pRasPort1->dwBufferOverrunErr);
}

VOID File_Print_RasPort(
    IN RAS_PORT_0 *   pRasPort0,
    IN RAS_PORT_1 *   pRasPort1
)
{
    WCHAR *Line_Condition[] =
    {
      L"RAS_PORT_NON_OPERATIONAL",
      L"RAS_PORT_DISCONNECTED",
      L"RAS_PORT_CALLING_BACK",
      L"RAS_PORT_LISTENING",
      L"RAS_PORT_AUTHENTICATING",
      L"RAS_PORT_AUTHENTICATED",
      L"RAS_PORT_INITIALIZING"
     };

    WCHAR *Hdw_Error[] =
    {
      L"RAS_HARDWARE_OPERATIONAL",
      L"RAS_HARDWARE_FAILURE"
     };

    CHAR  buf[512];
    DWORD dwBytesWritten = 0;

    if (NULL == pRasPort0 || NULL == pRasPort1)
    {
        return;
    }

    //dumping pRasPort0 to a file
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->hPort  %l64d\n",pRasPort0->hPort);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->hConnection   %l64d\n",pRasPort0->hConnection);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->dwPortCondition  %ws\n",Line_Condition[pRasPort0->dwPortCondition-1]);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->dwTotalNumberOfCalls  %d\n",pRasPort0->dwTotalNumberOfCalls);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->dwConnectDuration  %d\n",pRasPort0->dwConnectDuration);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->wszPortName  %ws\n",pRasPort0->wszPortName);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->wszMediaName  %ws\n",pRasPort0->wszMediaName);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->wszDeviceName  %ws\n",pRasPort0->wszDeviceName);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort0->wszDeviceType  %ws\n",pRasPort0->wszDeviceType);

    //dumping pRasPort1
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"\n dumping pRasPort1\n");
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort1->hPort  %l64d\n",pRasPort1->hPort);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort1->hConnection   %l64d\n",pRasPort1->hConnection);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort1->dwHardwareCondition  %ws\n",Hdw_Error[pRasPort1->dwHardwareCondition]);

    //connection stats
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwLineSpeed=%d\n", pRasPort1->dwLineSpeed );
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwBytesXmited=%d\n", pRasPort1->dwBytesXmited );
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwBytesRcved=%d\n", pRasPort1->dwBytesRcved );
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf), (LPTSTR)"pRasPort1->dwFramesXmited=%d\n", pRasPort1->dwFramesXmited );
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwFramesRcved=%d\n", pRasPort1->dwFramesRcved);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwCrcErr=%d\n", pRasPort1->dwCrcErr);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwTimeoutErr=%d\n", pRasPort1->dwTimeoutErr);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwAlignmentErr=%d\n", pRasPort1->dwAlignmentErr);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwHardwareOverrunErr=%d\n", pRasPort1->dwHardwareOverrunErr);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwFramingErr=%d\n", pRasPort1->dwFramingErr);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);
    StringCchPrintf((LPTSTR)buf, CELEMS(buf),  (LPTSTR)"pRasPort1->dwBufferOverrunErr=%d\n", pRasPort1->dwBufferOverrunErr);
    WriteFile(ConDisFile, (LPSTR)buf, (int) strlen(buf), &dwBytesWritten, NULL);

}

