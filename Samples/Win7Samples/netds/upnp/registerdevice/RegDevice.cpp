// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//
// RegDevice.cpp:  Provides a tool to register custom device with DeviceHost
// 

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <objbase.h>
#include <initguid.h>
#include <atlbase.h>
#include <upnphost.h>
#include "dimmerdevice.h"
#include "dimmerdevice_i.c"

#define DEFAULT_DEVICE_LIFETIME 3200

BOOL DescriptionDocToBSTR (WCHAR* szDDocFilename, BSTR *pdXMLDoc)
{
   // This function reads in the description doc and converts it to a BSTR that
   // can be passed to the Device Host during Registration.
   // Memory is allocated for the BSTR here and freed when we are done with registration

   HANDLE fHandle = NULL;
   BOOL retVal = TRUE;
   BOOL status = TRUE;
   DWORD lowerFileSize = 0;
   LPVOID pBuffer = NULL;
   WCHAR *pwBuffer = NULL;
   DWORD numBytesRead = 0;

   if ((szDDocFilename==NULL) || (pdXMLDoc==NULL))
   {
      goto error;
   }

   fHandle = CreateFile(
      szDDocFilename,         // file name
      GENERIC_READ,           // access mode
      FILE_SHARE_READ,        // share mode
      NULL,                   // SD
      OPEN_EXISTING,          // how to create
      FILE_ATTRIBUTE_NORMAL,  // file attributes
      NULL                    // handle to template file
      );

   if (fHandle == INVALID_HANDLE_VALUE)
   {
      fHandle = NULL;
      goto error;
   }

   // Otherwise we read the contents of the file and convert it to a BSTR
   lowerFileSize = GetFileSize (fHandle, NULL);

   if (lowerFileSize == INVALID_FILE_SIZE)
   {
      // Do not support such large file sizes or GetFileSize returned an error
      goto error;
   }

   pBuffer = CoTaskMemAlloc(lowerFileSize);
   if (pBuffer == NULL)
   {
      goto error;
   }

   pwBuffer= (WCHAR *) CoTaskMemAlloc((lowerFileSize+1)*sizeof(WCHAR));

   if (pwBuffer==NULL)
   {
      goto error;
   }

   // Else read from the file into the buffer
   status = ReadFile(
      fHandle,
      pBuffer,
      lowerFileSize,
      &numBytesRead,
      NULL
      );
   if (!status || (numBytesRead == 0)) 
   {
      goto error;
   }

   // Convert to a WCHAR string
   if (MultiByteToWideChar(CP_ACP, // code page
      MB_PRECOMPOSED,              // character-type options
      (LPCSTR) pBuffer,            // string to map
      lowerFileSize,               // number of bytes in string
      pwBuffer,                    // wide-character buffer
      lowerFileSize                // size of wide char buffer
      )==0)
   {
      goto error;
   }

   //adding a "\0" to the end of the buffer
   pwBuffer[lowerFileSize]=_TEXT('\0');
   //now convert the file to a BSTR
   *pdXMLDoc = NULL;
   *pdXMLDoc = SysAllocString(pwBuffer); 

   if (*pdXMLDoc == NULL)
   {
      goto error;
   }

   CloseHandle(fHandle);
   CoTaskMemFree(pBuffer);
   CoTaskMemFree(pwBuffer);

   // Have copied the description doc to a BSTR and so return TRUE
   return retVal;

error:

   retVal = FALSE;
   if (fHandle)
   {
      CloseHandle(fHandle);
   }
   CoTaskMemFree(pBuffer);
   CoTaskMemFree(pwBuffer);
   return retVal;

}



int _cdecl main() 
{
   // This executable makes the call to the Device Host to register a device

   HRESULT hr = S_OK;
   IUnknown *punk = NULL;
   IUPnPRegistrar *pReg = NULL;
   IUPnPReregistrar *pRereg = NULL;
   LPWSTR resourcePath = NULL;
   DWORD dwSize = 0;
   WCHAR fileDescriptionDoc[] = L"DimmerDevice-Desc.xml";
   WCHAR initString[] = L"DimmerDevice SDK Sample";
   LONG lifeTime = DEFAULT_DEVICE_LIFETIME;
   BSTR DeviceID = NULL;
   BSTR desDoc = NULL;
   BSTR initBSTR = NULL;
   BSTR resourcePathBSTR = NULL;
   CHAR option;
   BOOL retVal = TRUE;

   printf("\nSetup:");
   printf("\nBefore running this program please make sure that the two XML files");
   printf("\nare in the same directory as the EXE.\n");
   printf("\nAlso make sure that the command 'regsvr32 UPNPSampleDimmerDevice.dll'");
   printf("\nhas been run to register device-specific COM object.\n");

   hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
   if (FAILED(hr))
   {
      printf("\nCould not initialize COM library");
      return 0;
   }

   hr = CoInitializeSecurity(
      NULL,
      -1,
      NULL,
      NULL,
      RPC_C_AUTHN_LEVEL_DEFAULT,
      RPC_C_IMP_LEVEL_IMPERSONATE,
      NULL,
      EOAC_SECURE_REFS,
      NULL
      );

   if (FAILED(hr))
   {
      printf("\nCould not initialize COM security");
      goto error;
   }

   // Now try to instantiate the Device Control Object
   hr = CoCreateInstance(
      CLSID_UPNPSampleDimmerDevice, 
      NULL,
      CLSCTX_INPROC_SERVER,
      IID_IUnknown,
      (LPVOID *) &punk
      ); 

   if (FAILED(hr))
   {
      printf("\nCould not Instantiate the Device Control Object. - %x",hr);
      printf("\nCheck that the dll UPNPSampleDimmerDevice.dll has been registered");
      goto error;
   }

   // Continue with the registration and register the running device with the Device Host
   // get the resource path or the current directory where the XML files are located

   // Call GetCurrentDirectory() specifying nBufferLength = 0 to get the size of the 
   // required buffer to hold path, then call this function again to fill in the buffer
   dwSize = GetCurrentDirectoryW(0, NULL);
   resourcePath = (WCHAR *) CoTaskMemAlloc((dwSize + 1) * sizeof (WCHAR)); // add for for trailing "\"
   if (resourcePath == NULL)
   {
      printf("\nMemory Allocation Failure.. exiting");
      goto error;
   }

   if (GetCurrentDirectoryW(dwSize, resourcePath) == 0)
   {
      printf("\nCould not get the current directory Path where the XML files are located");
      goto error;
   }

   wcscat_s(resourcePath,dwSize + 1, L"\\");

   //now we try to read in the description doc for the DimmerDevice
   retVal = DescriptionDocToBSTR(fileDescriptionDoc, &desDoc);

   if (retVal==FALSE)
   {
      printf("\nCould not load the description document for the device");
      goto error;
   }

   initBSTR = SysAllocString(initString);
   if (initBSTR==NULL)
   {
      printf("\nMemory Allocation Failure.. exiting");
      goto error;
   }

   resourcePathBSTR = SysAllocString(resourcePath);
   if (resourcePathBSTR == NULL)
   {
      printf("\nMemory Allocation Failure.. exiting");
      goto error;
   }

   //now register the device with the Device Host service
   hr = CoCreateInstance(
      CLSID_UPnPRegistrar,
      NULL,
      CLSCTX_LOCAL_SERVER,
      IID_IUPnPRegistrar,
      (LPVOID *) &pReg
      );
   if (FAILED(hr))
   {
      printf("\nCould not get a pointer to the IUPnPRegistrar Interface failed - %x",hr);
      printf("\nCheck that the UPNP Device Host Component is installed on this machine");
      goto error;
   }

   //now get a pointer to the IID_IUPnPReregistrar
   hr = CoCreateInstance(
      CLSID_UPnPRegistrar,
      NULL,
      CLSCTX_LOCAL_SERVER,
      IID_IUPnPReregistrar,
      (LPVOID *) &pRereg
      );

   if (FAILED(hr))
   {
      printf("\nCould not get the pointer to the IUPnPReregistrar Interface - %x",hr);
      printf("\nCheck that the UPNP Device Host Component is installed on this machine");
      goto error;
   }

   hr = pReg->RegisterRunningDevice(desDoc,punk,initBSTR,resourcePathBSTR,lifeTime,&DeviceID);

   if (FAILED(hr))
   {
      printf("Could not register the device with the Device Host - %x",hr);
      goto error;
   }

   printf("\nDevice Registered Successfully with the UPNP Device Host...");
   printf("\nHit d to unregister the device -  ");
   while (1)
   {
      option =  (CHAR)_getche();
      if((option=='d')||(option=='D'))
      {
         break;
      }
   }

   printf("\nUnregistering the device with the UPNP Device Host and Quitting ..");
   //deregister the device 
   hr = pReg->UnregisterDevice(DeviceID,TRUE); 

   if (SUCCEEDED(hr))
   {
      printf("\nDevice was unregistered successfully");
   }
   else
   {
      printf("\nDevice could not be unregistered - %x",hr);
   }

   CoTaskMemFree(resourcePath);
   SysFreeString(desDoc);
   SysFreeString(initBSTR);
   SysFreeString(resourcePathBSTR);
   //release the references to the interfaces
   pRereg->Release();
   punk->Release();
   pReg->Release();
   CoUninitialize();
   return 1;

error:

   CoTaskMemFree(resourcePath);
   SysFreeString(desDoc);
   SysFreeString(initBSTR);
   SysFreeString(resourcePathBSTR);
   if (punk)
   {
      punk->Release();
   }
   if (pRereg)
   {
      pRereg->Release();
   }
   if (pReg)
   {
      pReg->Release();
   }
   CoUninitialize();
   return 0;
}
