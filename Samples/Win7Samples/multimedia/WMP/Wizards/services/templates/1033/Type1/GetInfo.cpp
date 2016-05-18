#include "stdafx.h"
#include "[!output root].h"

////////////////////////////////////////////////////////////////////
// C[!output Safe_root]
//    GetItemInfo method
//    GetContentPartnerInfo method
//    GetStreamingURL method
//    GetCatalogURL method
//    GetTemplate method
//
// Copyright (c) Microsoft Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////

extern CONTENT_PARTNER_GLOBALS g;

HRESULT STDMETHODCALLTYPE C[!output Safe_root]::GetItemInfo(
   BSTR bstrInfoName,   // in
   VARIANT *pContext,   // in
   VARIANT *pData)      // out
{
   WCHAR url[INTERNET_MAX_URL_LENGTH] = L"";
   HRESULT hr = S_OK;

   // Set output parameter type to VT_EMPTY in case we fail.
   if(NULL != pData)
   {
      VariantInit(pData);    
   }

   if(NULL == bstrInfoName || NULL == pContext || NULL == pData)
   {
      return E_INVALIDARG;
   }

   ATLTRACE2("%x: GetItemInfo: pstrInfoName = %S.\n", GetCurrentThreadId(), bstrInfoName);

   if( 0 == wcscmp(bstrInfoName, g_szItemInfo_PopupURL) )
   {
      // ToDo: Get the index of the popup by inspecting pContext->lVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/Popup.htm");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // Note: The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_AuthenticationSuccessURL) )
   {   
      if(VT_I4 == pContext->vt)
      {
         // pContext holds the index of an authentication-success Web page.
         // The sample store has only one authentication-success Web page,
         // and its index is 1.
         switch (pContext->lVal)
         {
         case 1:
            wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
            wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/AuthenticationSuccess1.htm");

            pData->bstrVal = SysAllocString(url);
            // The caller must free pData->bstrVal.

            if(NULL != pData->bstrVal)
            {
               pData->vt = VT_BSTR;
            }
            else
            {
               hr = E_OUTOFMEMORY;
            }
           
            break;

         default:
            hr = E_INVALIDARG;
            
         } // switch

      } // if(VT_I4 == pContext->vt)

      else
      {
         hr = E_INVALIDARG;
      } 
   } // g_szItemInfo_AuthenticationSuccessURL       

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_LoginFailureURL) )
   {    
      // ToDo: Get the index by inspecting pContext->ulVal.  

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/LoginFailure.htm");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_HTMLViewURL) )
   {     
      // We are passed a string value from an ASX file.
      // <param name="HTMLFLINK" value="xxx">

      // ToDo: Get the string by inspecting pContext->bstrVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/HtmlView.htm");
     
      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal;
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_PopupCaption) )
   {   
      // ToDo: Get the index by inspecting pContext->lVal.
      
      pData->bstrVal = SysAllocString(L"CP Test Popup Caption");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ForgetPasswordURL) )
   {
      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/ForgotPassword.htm");
      
      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_CreateAccountURL) )
   {    
      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/CreateAccount.htm");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ArtistArtURL) )
   {
      // ToDo: Get the artist ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/Artist.png");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_AlbumArtURL) )
   {
      // ToDo: Get the album ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/Album.png");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ListArtURL) )
   {
      // ToDo: Get the list ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/List.png");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_GenreArtURL) )
   {
      // ToDo: Get the genre ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/Genre.png");
;
      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_SubGenreArtURL) )
   {
      // ToDo: Get the sub-genre ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/Subgenre.png");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_RadioArtURL) )
   {
      // ToDo: Get the radio playlist ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/Radio.png");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_TreeListIconURL) )
   {
      // ToDo: Get the list ID by inspecting pContext->ulVal.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Images/Tree.png");
      
      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ErrorDescription) )
   {
      // ToDo: Get the error code by inspecting pContext->scode.

      pData->bstrVal = SysAllocString(L"Test error description. The operation was aborted.");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ErrorURL) )
   {
      // ToDo: Get the error code by inspecting pContext->scode.

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/Error.htm");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ErrorURLLinkText) )
   {
      // ToDo: Get the error code by inspecting pContext->scode.

      pData->bstrVal = SysAllocString(L"Test error link text: Error Details");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ALTLoginURL) )
   {
      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/AltLogin.htm?DlgX=800&DlgY=400");

      pData->bstrVal = SysAllocString(url);

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if( 0 == wcscmp(bstrInfoName, g_szItemInfo_ALTLoginCaption) )
   {
      pData->bstrVal = SysAllocString(L"Alternative Login");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else 
   {
      hr = E_UNEXPECTED;
   }

   return hr;
}


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::GetContentPartnerInfo(
   BSTR bstrInfoName,
   VARIANT *pData)
{
   HRESULT hr = S_OK;

   // Set the output parameter type to VT_EMPTY in case we fail.
   if(NULL != pData)
   {
      VariantInit(pData);    
   }

   if(NULL == bstrInfoName || NULL == pData)
   {
      return E_INVALIDARG;
   }

   ATLTRACE2("%x: GetContentPartnerInfo: bstrInfoName = %S.\n", GetCurrentThreadId(), bstrInfoName);

   if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_LoginState))
   {
      pData->vt = VT_BOOL;

      if( 1 == InterlockedCompareExchange( &(g.userLoggedIn), 0, 0 ) )
      {
         ATLTRACE2("%x: GetContentPartnerInfo: User is logged in.\n", GetCurrentThreadId());
         pData->boolVal = VARIANT_TRUE;
      }
      else
      {
         ATLTRACE2("%x: GetContentPartnerInfo: User is not logged in.\n", GetCurrentThreadId());
         pData->boolVal = VARIANT_FALSE;
      }
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_MediaPlayerAccountType))
   {
      // ToDo: Determine the account type.

      pData->vt = VT_UI4;
      pData->ulVal = wmpatSubscription; // or another enumeration member  
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_AccountType))
   {   
      pData->bstrVal = SysAllocString(L"Super Subscription Account");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_HasCachedCredentials))
   {
      ATLTRACE2("%x: GetContentPartnerInfo: bstrInfoName = %S.\n", GetCurrentThreadId(), bstrInfoName);

      pData->vt = VT_BOOL;

      LONG cached = 0;
      HaveCachedCredentials(&cached);

      if(1 == cached)
      {
         pData->boolVal = VARIANT_TRUE;
         ATLTRACE2("%x: GetContentPartnerInfo: Plug-in has cached credentials.\n", GetCurrentThreadId());
      }
      else
      {
         pData->boolVal = VARIANT_FALSE;
         ATLTRACE2("%x: GetContentPartnerInfo: Plug-in does not have cached credentials.\n", GetCurrentThreadId());
      }   
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_LicenseRefreshAdvanceWarning))
   {
      pData->vt = VT_UI4;
      pData->ulVal = 5; // or some other number of days
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_PurchasedTrackRequiresReDownload))
   {
      pData->vt= VT_BOOL;
      pData->boolVal = VARIANT_TRUE; // or VARIANT_FALSE
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_MaximumTrackPurchasePerPurchase))
   {
      pData->vt = VT_UI4;
      pData->ulVal = 30; // or some other number of tracks
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_AccountBalance))
   {
      pData->bstrVal = SysAllocString(L"$dd.cc");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }
      
      // The caller must free pData->bstrVal.
   }

   else if(0 == wcscmp(bstrInfoName, g_szContentPartnerInfo_UserName))
   {
      pData->bstrVal = SysAllocString(L"xxx");

      if(NULL != pData->bstrVal)
      {
          pData->vt = VT_BSTR;
      }
      else
      {
          hr = E_OUTOFMEMORY;
      }

      // The caller must free pData->bstrVal.
   }

   else
   {
      hr = E_UNEXPECTED;
   }

   return hr;
}



HRESULT STDMETHODCALLTYPE C[!output Safe_root]::GetStreamingURL( 
   WMPStreamingType st,
   VARIANT *pStreamContext,
   BSTR *pbstrURL)
{
   HRESULT hr = S_OK;
   WCHAR url[INTERNET_MAX_URL_LENGTH] = L"";
   ULONG trackId = 0xffffffff;
   ULONG availableUrlStrings = 0;

   // Set the output parameter to NULL in case we fail.
   if(NULL != pbstrURL)
   {
      *pbstrURL = NULL;
   }

   if(NULL == pStreamContext || NULL == pbstrURL)
   {
      return E_INVALIDARG;
   }

   if(VT_UI4 != pStreamContext->vt)
   {
      return E_INVALIDARG;
   }

   ATLTRACE2("%x: GetStreamingURL: st = %d.\n", GetCurrentThreadId(), st);
 

   switch(st)
   {
   case wmpstMusic:

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_audioRootURL);

      // Get the track ID from pContext.
      trackId = pStreamContext->ulVal;

      availableUrlStrings = sizeof(g_tracks)/sizeof(g_tracks[0]);

      if(trackId < availableUrlStrings)
      {    
         wcscat_s(url, INTERNET_MAX_URL_LENGTH, g_tracks[trackId]);
      }
      else
      {
         wcscat_s(url, INTERNET_MAX_URL_LENGTH, g_placeholderTrack);       
      }

      *pbstrURL = SysAllocString(url);

      if(NULL == *pbstrURL)
      {
         hr = E_OUTOFMEMORY;
      }

      // The caller must free *pbstrURL.
      break;

   case wmpstVideo:
       // ToDo: Get the media item ID by inspecting pStreamContext->ulVal.
       // Return the URL.
   case wmpstRadio:
      // ToDo: Get the media item ID by inspecting pStreamContext->ulVal. 
      // Return the URL.    
   default:
      *pbstrURL = NULL;
      hr = E_UNEXPECTED;
      break;

   }  // switch

   return hr;
}


HRESULT STDMETHODCALLTYPE C[!output Safe_root]::GetCatalogURL( 
   DWORD dwCatalogVersion,
   DWORD dwCatalogSchemaVersion,
   LCID /*catalogLCID*/,
   DWORD *pdwNewCatalogVersion,  // out
   BSTR *pbstrCatalogURL,        // out
   VARIANT *pExpirationDate)     // out
{
   WCHAR url[INTERNET_MAX_URL_LENGTH] = L"";
   HRESULT hr = S_OK;

   // Initialize output parameters in case we fail.

   if(NULL != pdwNewCatalogVersion)
   {
      *pdwNewCatalogVersion = 0;
   }

   if(NULL != pbstrCatalogURL)
   {
      *pbstrCatalogURL = NULL;
   }

   if(NULL != pExpirationDate)
   {
      VariantInit(pExpirationDate);
   }


   if(NULL == pdwNewCatalogVersion || 
      NULL == pbstrCatalogURL || 
      NULL == pExpirationDate ||
      NULL == pdwNewCatalogVersion ||
      NULL == pbstrCatalogURL ||
      NULL == pExpirationDate)
   {
      return E_INVALIDARG;
   }

   ATLTRACE("%x: GetCatalogURL:\n", GetCurrentThreadId());
   ATLTRACE("%x: GetCatalogVersion: dwCatalogVersion = %d.\n", GetCurrentThreadId(), dwCatalogVersion);
   ATLTRACE("%x: GetCatalogVersion: dwCatalogSchemaVersion = %d.\n", GetCurrentThreadId(), dwCatalogSchemaVersion);

   *pdwNewCatalogVersion = 3;
   wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
   wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Catalog/3/catalog.wmdb.lz"); 
 
   *pbstrCatalogURL = SysAllocString(url);

   if(NULL == *pbstrCatalogURL)
   {
      hr = E_OUTOFMEMORY;
   }

   // The caller must free *pbstrCatalogURL.

   pExpirationDate->vt = VT_DATE;
   pExpirationDate->date = 43831.00; // January 1, 2020

   return hr;
}
        
HRESULT STDMETHODCALLTYPE C[!output Safe_root]::GetTemplate( 
   WMPTaskType task,
   BSTR location,
   VARIANT* /*pContext*/,
   BSTR /*clickLocation*/,
   VARIANT* /*pClickContext*/,
   BSTR /*bstrFilter*/,
   BSTR /*bstrViewParams*/,
   BSTR *pbstrTemplateURL,           // out
   WMPTemplateSize *pTemplateSize)   // out
{
   HRESULT hr = S_OK;
   WCHAR url[INTERNET_MAX_URL_LENGTH] = L"";

   // Initialize output parameters in case we fail.

   if(NULL != pbstrTemplateURL)
   {
      *pbstrTemplateURL = NULL;
   }

   if(NULL != pTemplateSize)
   {
      *pTemplateSize = wmptsSmall; // wmpstSmall = 0
   }


   if(NULL == pbstrTemplateURL || NULL == pTemplateSize)
   {
      return E_INVALIDARG;
   }

   ATLTRACE2("%x: GetTemplate: task = %d, location = %S.\n", GetCurrentThreadId(), task, location);

   switch(task)
   {
   case wmpttBrowse:

      if( 0 == wcscmp(g_szRootLocation, location) )
      {
         *pTemplateSize = wmptsLarge;
      }
      else
      {
         *pTemplateSize = wmptsMedium;
      }

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/library.htm");    
      *pbstrTemplateURL = SysAllocString(url);

      if(NULL == *pbstrTemplateURL)
      {
         hr = E_OUTOFMEMORY;
      }

      // The caller must free *pbstrTemplateURL.
      break;

   case wmpttSync:

      *pTemplateSize = wmptsMedium;

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/sync.htm");    
      *pbstrTemplateURL = SysAllocString(url);

      if(NULL == *pbstrTemplateURL)
      {
         hr = E_OUTOFMEMORY;
      }

      // The caller must free *pbstrTemplateURL.      
      break;

   case wmpttBurn:
 
      if( 0 == wcscmp(g_szAllCPAlbumIDs, location) || 
          0 == wcscmp(g_szCPAlbumID, location) )
      {
         *pTemplateSize = wmptsMedium;
         wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
         wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/BurnAlbums.htm");    
      }
      else if( 0 == wcscmp(g_szAllCPTrackIDs, location) || 
               0 == wcscmp(g_szCPTrackID, location) )
      {
         *pTemplateSize = wmptsMedium;
         wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
         wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/BurnSongs.htm");    
      }
      else
      {
         *pTemplateSize = wmptsSmall;
         wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
         wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/Burn.htm");    
      }
    
      *pbstrTemplateURL = SysAllocString(url);

      if(NULL == *pbstrTemplateURL)
      {
         hr = E_OUTOFMEMORY;
      }

      // The caller must free *pbstrTemplateURL.
      break;

   case wmpttCurrent:

      *pTemplateSize = wmptsSmall;

      wcscpy_s(url, INTERNET_MAX_URL_LENGTH, g_rootURL);
      wcscat_s(url, INTERNET_MAX_URL_LENGTH, L"Pages/library.htm");    
      *pbstrTemplateURL = SysAllocString(url);

      if(NULL == *pbstrTemplateURL)
      {
         hr = E_OUTOFMEMORY;
      }

      // The caller must free *pbstrTemplateURL.
      break;

   default:
      *pbstrTemplateURL = NULL;
      hr = E_INVALIDARG;
      break;
   }

   return hr;
}