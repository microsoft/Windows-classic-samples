/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) Microsoft Corporation.  All rights reserved.

 MODULE:   client.c

 PURPOSE:  This program is a command line oriented
           demonstration of the Simple service sample.

----------------------------------------------------------------------------*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>

VOID __cdecl main(int argc, char *argv[])
{
   char    outbuf[180];                         // size of the szOut Buffer defined in SERVICE.C -> ServiceStart
   DWORD   bytesRead;
   BOOL    ret;
   LPSTR   lpszPipeName = "\\\\.\\pipe\\simple";
   LPSTR   lpszString = "World";
   int     ndx;
   HRESULT hr;

   // allow user to define pipe name
   for ( ndx = 1; ndx < argc; ndx++ )
   {
      if ( (*argv[ndx] == '-') || (*argv[ndx] == '/') )
      {
         if ( !(_stricmp( "pipe", argv[ndx]+1 )) && ( (ndx + 1) < argc))
         {
            lpszPipeName = argv[++ndx];
         }
         else if ( !(_stricmp( "string", argv[ndx]+1 )) && ( (ndx + 1) < argc))
         {
            lpszString = argv[++ndx];
         }
         else
         {
            printf("usage: client [-pipe <pipename>] [-string <string>]\n");
            exit(1);
         }
      }
      else
      {
         printf("usage: client [-pipe <pipename>] [-string <string>]\n");
         exit(1);
      }

   }

   hr = StringCchLength(lpszString,100,&ndx);
   if (FAILED(hr))
    {
        printf("Buffer is too large!\n");
        return;
    }

   ret = CallNamedPipe(lpszPipeName,
                       lpszString, ndx,
                       outbuf,180 ,
                       &bytesRead, NMPWAIT_WAIT_FOREVER);

   if (!ret)
   {
      printf("client: CallNamedPipe failed, GetLastError = %d\n", GetLastError());
      exit(1);
   }

   printf("client: received: %s\n", outbuf);
}
