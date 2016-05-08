// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// DimmerService.cpp


#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <objbase.h>
#include <atlbase.h>
#include "DeviceDll.h"
#include <atlcom.h>
#include <initguid.h>
#include <upnphost.h>
#include "resource.h"
#include "dimmerdevice.h"
#include "DimmerService.h"


UPNPDimmerService::UPNPDimmerService()
{
   // Set the default values for the state table variables power and dimLevel
   power = VARIANT_FALSE;
   dimnessLevel = 0;

   InitializeCriticalSection(&csSync);
   // Make it NULL and this will be later set when the device host calls the Advise()
   esEventingManager = NULL; 
}



UPNPDimmerService::~UPNPDimmerService()
{
   // Cleanup before the service exits
   if (esEventingManager)
   {
      // we have to release the pointer to the Eventing Manager Interface on shutdown
      esEventingManager->Release();
      esEventingManager = NULL;
   }

   DeleteCriticalSection(&csSync);
}



HRESULT UPNPDimmerService::get_Power(VARIANT_BOOL *pval)
{    
   // Return the value of the state table variable Power
   EnterCriticalSection(&csSync);
   *pval = power;
   LeaveCriticalSection(&csSync);
   return S_OK;
}



HRESULT UPNPDimmerService::get_dimLevel(LONG *dLevel)
{    
   // Return the value of the state table variable dimLevel
   EnterCriticalSection(&csSync);
   *dLevel = dimnessLevel;
   LeaveCriticalSection(&csSync);
   return S_OK;
}



HRESULT UPNPDimmerService::PowerOn()
{
   DISPID rgdispidChanges[1];

   EnterCriticalSection(&csSync);

   if (power != VARIANT_TRUE)
   { 
      power = VARIANT_TRUE;
      LeaveCriticalSection(&csSync);	

      // We have to send an event here indicating that the value of Power has changed
      if(esEventingManager)
      {
         // Send the event that power = TRUE through OnStateChanged API exposed by IUPnPEventSource 
         rgdispidChanges[0]=DISPID_POWER;
         esEventingManager->OnStateChanged(1, rgdispidChanges);
      }
   }
   else
   {
      LeaveCriticalSection(&csSync);
   }

   return S_OK;
}




HRESULT UPNPDimmerService::PowerOff()
{
   DISPID rgdispidChanges[1];

   EnterCriticalSection(&csSync);
   if (power != VARIANT_FALSE)
   { 
      power = VARIANT_FALSE;
      // We have to send an event here since the value of a State table variable 
      // has changed
      LeaveCriticalSection(&csSync);

      if(esEventingManager)
      {
         // Send the event using the OnStateChanged API and indicate the the 
         // value of the state table variable Power has changed
         rgdispidChanges[0]=DISPID_POWER;
         esEventingManager->OnStateChanged(1, rgdispidChanges);
      }
   }
   else
   {
      LeaveCriticalSection(&csSync);
   }

   return S_OK;
}



HRESULT UPNPDimmerService::GetPowerValue(VARIANT_BOOL*powerVal)
{   
   EnterCriticalSection(&csSync);
   *powerVal = power;
   LeaveCriticalSection(&csSync);

   // We have no events to send for this action
   return S_OK;
}



HRESULT UPNPDimmerService::SetDimLevel(LONG dLevel)
{
   DISPID rgdispidChanges[1];

   EnterCriticalSection(&csSync);
   dimnessLevel = dLevel;
   // The possible values for dimnessLevel are 0 - 100
   if (dimnessLevel > 100) dimnessLevel = 100;
   if (dimnessLevel < 0) dimnessLevel = 0;
   LeaveCriticalSection(&csSync);

   // We have to send an event here
   if(esEventingManager)
   {
      // Send the event using the OnStateChanged API to indicate that value 
      // of State table varaible dimLevel has changed
      rgdispidChanges[0]=DISPID_DIMLEVEL;
      esEventingManager->OnStateChanged(1, rgdispidChanges);    
   }

   return S_OK;
}


HRESULT UPNPDimmerService::GetDimLevel(LONG *dLevel)
{   
   EnterCriticalSection(&csSync);
   *dLevel = dimnessLevel;
   LeaveCriticalSection(&csSync);

   // We have no events to send here as the state table has not changed     
   return S_OK;
}



HRESULT UPNPDimmerService::GetConfigDetails(VARIANT_BOOL *powerVal,LONG *dLevel)
{
   EnterCriticalSection(&csSync);
   *dLevel = dimnessLevel;
   *powerVal = power;
   LeaveCriticalSection(&csSync);

   // We have no events to send here as the state table has not changed  
   return S_OK;
}


HRESULT UPNPDimmerService::Advise (IUPnPEventSink *punkSubscriber)
{
   // We have to get query for the IUPnPEventSink Interface to send events 
   // later using the OnStateChanged API

   HRESULT hr = S_OK;

   // Query the pointer passed to the Advise function for the IUPnPEventSink Interface
   hr = punkSubscriber->QueryInterface(IID_IUPnPEventSink, (void **)&esEventingManager);
   if (FAILED(hr))
   {
      OutputDebugString(_TEXT("UPNPDimmerService: Query Interface failed.  Could not get pointer to IUPnPEventSink\n"));
      esEventingManager = NULL;
      return hr;
   }       

   return hr;
}



HRESULT UPNPDimmerService::Unadvise(IUPnPEventSink *punkSubscriber)
{	
   UNREFERENCED_PARAMETER(punkSubscriber);

   // Device Host is not interested in receiving more events and so set the 
   // pointer to the IUPnPEventSink interface to NULL
   if (esEventingManager)
   {
      // esEventingManager is already set by the advise
      esEventingManager->Release();
      esEventingManager = NULL; 
   }

   return S_OK;
}
