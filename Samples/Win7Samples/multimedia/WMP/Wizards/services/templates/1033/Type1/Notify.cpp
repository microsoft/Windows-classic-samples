#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    SetCallback method
//    Notify method
//    StationEvent method
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

extern CONTENT_PARTNER_GLOBALS g;

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::SetCallback(
   IWMPContentPartnerCallback* pCallback)
{  
   HRESULT hr = S_OK;
   VARIANT context;

   VariantInit(&context);

   if(pCallback)
   {
      // Store the callback interface pointer.
      m_spCallback = pCallback;

      m_spCallback->Notify(wmpcnNewCatalogAvailable, &context);
   }
   else
   {
      hr = this->ShutdownThreads();
      m_spCallback.Release();
   }

   return hr;
}

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::Notify(
   WMPPartnerNotification type, 
   VARIANT* /*pContext*/)
{
   HRESULT hr = S_OK;

   ATLTRACE2("%x: Notify: type = %d.\n", GetCurrentThreadId(), type);

   switch(type)
   {
   case wmpsnBackgroundProcessingBegin:

      // ToDo: If this is the first notification to begin
      // background processing, create a background-processing
      // thread.

      // ToDo: Set an event (put it in the signaled state)
      // so that the background-processing thread can run.
      break;

   case wmpsnBackgroundProcessingEnd:

      // ToDo: Reset an event (put it in the non-signaled state)
      // so that the background-processing thread will wait.
      break;

   case wmpsnCatalogDownloadFailure:
   
      // ToDo: Extract the error code from pContext->scode
      // and take appropriate action. 
      break;

   case wmpsnCatalogDownloadComplete:

      // ToDo: Set state indicating that catalog download is complete.
      break;

   default:

      hr = E_UNEXPECTED;
      break;
   }

   return hr;
}    
  
HRESULT STDMETHODCALLTYPE C[!output Safe_root]::StationEvent( 
   BSTR bstrStationEventType,
   ULONG /*StationId*/,
   ULONG /*PlaylistIndex*/,
   ULONG /*TrackID*/,
   BSTR /*TrackData*/,
   DWORD /*dwSecondsPlayed*/)
{
   HRESULT hr = S_OK;

   ATLTRACE2("%x: StationEvent: bstrStationEventType = %d.\n", GetCurrentThreadId(), bstrStationEventType);

   if(NULL == bstrStationEventType)
   {
      hr = E_INVALIDARG;
   }
   else if( 0 == wcscmp(g_szStationEvent_Started, bstrStationEventType) )
   {
      // ToDo: Log Started event.
   }
   else if( 0 == wcscmp(g_szStationEvent_Complete, bstrStationEventType) )
   {
      // ToDo: Log Completed event.
   }
   else if( 0 == wcscmp(g_szStationEvent_Skipped, bstrStationEventType) )
   {
      // ToDo: Log Skipped event.
   }
   else
   {
      hr = E_INVALIDARG;
   }

   return hr;
}