/*++

Copyright (c) Microsoft Corporation. All rights reserved

Abstract:

    Creates a filters to block all incoming traffic except traffic destined
    to Windows Live Messenger.

Environment:

    User mode

--*/

#include "windows.h"
#include "winioctl.h"
#include "strsafe.h"

#ifndef _CTYPE_DISABLE_MACROS
#define _CTYPE_DISABLE_MACROS
#endif

#include "fwpmu.h"

#include <conio.h>
#include <stdio.h>

#define MSN_MESSENGER_FILE_PATH L"%ProgramFiles%\\Windows Live\\Messenger\\msnmsgr.exe"
#define MSN_MESSENGER_FILE_PATH_WOW64 L"%ProgramFiles(x86)%\\Windows Live\\Messenger\\msnmsgr.exe"

DWORD
MsnFltrAppGetMSNMessengerApplicationId(
    __in const wchar_t* fileName,
    __out FWP_BYTE_BLOB** appId)
{
   DWORD error = NO_ERROR;
   
   error = FwpmGetAppIdFromFileName0(fileName, appId);

   return error;
}

DWORD
MsnFltrAppAddFilters(
   IN    HANDLE         engineHandle,
   IN    FWP_BYTE_BLOB* applicationPath)
/*++

Routine Description:

    Adds filters to the necessary layers.

Arguments:

Return Value:

    None.

Notes:

    This routine adds 3 filters:
      1 to the Application Level Enforcement (ALE) Connect layer and
      2 to the ALE Receive Accept layer.

    The net result is that MSN is permitted to receive incoming traffic, but 
    all other applications cannot receive incoming traffic. Traffic can still
    flow in based on active outbound connections, but no incoming connections
    can be initiated while the filters are present.  Any previously existing
    inbound connections that are not permitted based on the new filters will
    be torndown.

--*/
{
   DWORD                   error = NO_ERROR;
   RPC_STATUS              rpcStatus;
   FWPM_SUBLAYER0          msnFltrSubLayer;
   FWPM_FILTER0            filter;
   FWPM_FILTER_CONDITION0  filterConditions[3]; // We only need three for this call.

   RtlZeroMemory(&msnFltrSubLayer, sizeof(FWPM_SUBLAYER0));
   rpcStatus = UuidCreate(&msnFltrSubLayer.subLayerKey);

   if (RPC_S_OK != rpcStatus)
   {
      goto cleanup;
   }

   msnFltrSubLayer.displayData.name = L"MsnFltr Sub layer";
   msnFltrSubLayer.displayData.description = L"MsnFltr Sub layer";
   msnFltrSubLayer.flags = 0;

   // We don't really mind what the order of invocation is.
   msnFltrSubLayer.weight = 0x100;
   
   error = FwpmSubLayerAdd0(engineHandle, &msnFltrSubLayer, NULL);

   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   RtlZeroMemory(&filter, sizeof(FWPM_FILTER0));

   filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
   filter.action.type = FWP_ACTION_PERMIT;
   filter.subLayerKey = msnFltrSubLayer.subLayerKey;
   filter.weight.type = FWP_EMPTY; // auto-weight.
   filter.numFilterConditions = 0; // this applies to all applications/traffic

   filter.displayData.name = L"Authorize Connect Layer";
   filter.displayData.description = L"Filter for authorizing outbound connections (TCP/UDP etc).";

   printf("Adding Filter\n");

   error = FwpmFilterAdd0(engineHandle, 
                          &filter, 
                          NULL, 
                          NULL);
   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   printf("Successfully added filter\n");


   RtlZeroMemory(&filter, sizeof(FWPM_FILTER0));

   filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
   filter.action.type = FWP_ACTION_BLOCK;
   filter.subLayerKey = msnFltrSubLayer.subLayerKey;
   filter.weight.type = FWP_EMPTY; // auto-weight.
   filter.numFilterConditions = 0; // this applies to all applications/traffic

   filter.displayData.name = L"Receive/Accept Layer Block";
   filter.displayData.description = L"Filter to block all inbound connections (TCP/UDP etc).";

   printf("Adding Filter\n");

   error = FwpmFilterAdd0(engineHandle, 
                          &filter, 
                          NULL, 
                          NULL);
   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   printf("Successfully added filter\n");

   RtlZeroMemory(&filter, sizeof(FWPM_FILTER0));

   filter.layerKey = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
   filter.action.type = FWP_ACTION_PERMIT; // Block the traffic
   filter.subLayerKey = msnFltrSubLayer.subLayerKey;
   filter.weight.type = FWP_EMPTY; // auto-weight.
   filter.filterCondition = filterConditions;
   filter.numFilterConditions = 2; // The conditions will determine the action.

   filter.displayData.name = L"Receive/Accept Layer Permit Filter";
   filter.displayData.description = L"Filter to allow Windows Live Messenger traffic";

   RtlZeroMemory(filterConditions, sizeof(filterConditions));

   //
   // Add the application path to the filter conditions.
   //
   filterConditions[0].fieldKey = FWPM_CONDITION_ALE_APP_ID;
   filterConditions[0].matchType = FWP_MATCH_EQUAL;
   filterConditions[0].conditionValue.type = FWP_BYTE_BLOB_TYPE;
   filterConditions[0].conditionValue.byteBlob = applicationPath;

   //
   // Since MSN Messenger appears to use dynamic ports, we cannot
   // set any other conditions besides protocol.
   //
   filterConditions[1].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
   filterConditions[1].matchType = FWP_MATCH_EQUAL;
   filterConditions[1].conditionValue.type = FWP_UINT8;
   filterConditions[1].conditionValue.uint8 = IPPROTO_TCP;
   
   printf("Adding Filter\n");

   error = FwpmFilterAdd0(engineHandle, 
                          &filter, 
                          NULL, 
                          NULL);
   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   printf("Successfully added filter\n");

cleanup:
   
   return error;
}


void __cdecl main(int argc, char* argv[])
/*++

Routine Description:

    Adds an Entry to serial log.

Arguments:

Return Value:

    None.

--*/
{
   HANDLE            MsnFltrDevice = NULL;
   HANDLE            engineHandle = NULL;
   DWORD             error;
   FWPM_SESSION0     session;
   FWP_BYTE_BLOB*    applicationId = NULL;

   printf("Retrieving Windows Live Messenger Application Id\n");
   error = MsnFltrAppGetMSNMessengerApplicationId(MSN_MESSENGER_FILE_PATH,
                                                  &applicationId);
#if _WIN64
   // If this is 64-bit we may not have a 64-bit Messenger executable.
   if (error != NO_ERROR)
   {
      error = MsnFltrAppGetMSNMessengerApplicationId(MSN_MESSENGER_FILE_PATH_WOW64,
                                                     &applicationId);
   }
#endif

   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   printf("Successfully retrieved Windows Live Messenger Application Id\n");

   RtlZeroMemory(&session, sizeof(FWPM_SESSION0));

   session.displayData.name = L"MSN MsnFltr Session";
   session.displayData.description = L"MsnFltr";
   
   // Let the Base Firewall Engine cleanup after us.
   session.flags = FWPM_SESSION_FLAG_DYNAMIC;
   
   printf("Opening Filtering Engine\n");
   error =  FwpmEngineOpen0(
                            NULL,
                            RPC_C_AUTHN_WINNT,
                            NULL,
                            &session,
                            &engineHandle
                            );

   if (NO_ERROR != error)
   {
      goto cleanup;

   }
   printf("Successfully opened Filtering  Engine\n");

   printf("Adding Filters through the Filtering Engine\n");

   error = MsnFltrAppAddFilters(engineHandle, 
                                applicationId);
      
   if (error != NO_ERROR)
   {
      goto cleanup;
   }

   printf("Successfully added Filters through the Filtering Engine\n");

   printf("Windows Live Messenger Traffic is now permitted (all other traffic is denied).  Please press any key to exit (this will remove the filters).\n");

   _getch();

cleanup:

   if (error != NO_ERROR)
   {
      printf("MsnFltr.\tError 0x%x occurred during execution\n", error);
   }

   //
   // Free the application Id that we retrieved.
   //
   if (applicationId)
   {
      FwpmFreeMemory0((void**)&applicationId);
   }

   error =  FwpmEngineClose0(engineHandle);
   engineHandle = NULL;

   return;
}