// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Module Name:
//    WcnFdHlpr.cpp
//
// Abstract:
//    Wcn Function discovery helper implements the necessary interfaces to interact with Function discovery
//	  such that the caller can start a Function Discovery search to retrieve a WCN device instance 
//	  based on a supplied UUID.   

#include "WcnFdHelper.h"

/*--
    This function converts any byte to its corresponding HEX representation. This is used 
    for converting the SSID to HEX to be used in FD query.
--*/

HRESULT ConvertStringToHex(
    __in      DWORD                      cbBlob,
    __in_bcount(cbBlob) BYTE      const *pbBlob,
    __in      DWORD                      cchHex,
    __out_ecount(cchHex) PWSTR           wszHex)
{
    HRESULT hr = S_OK;

    static WCHAR HEX[] = L"0123456789ABCDEF";
    DWORD i;

    if (!pbBlob || !wszHex ) 
    {
        return E_POINTER;
    }

    if (cchHex < 2*cbBlob + 1)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    for (i = 0; i < cbBlob; ++i)
    {
        wszHex[2*i]   = HEX[(pbBlob[i] & 0xf0) >> 4];
        wszHex[2*i+1] = HEX[(pbBlob[i] & 0x0f)];
    }

    wszHex[2*cbBlob] = L'\0';

Cleanup:

    return hr;
}
CWcnFdDiscoveryNotify::CWcnFdDiscoveryNotify() :
 m_pFunctionDiscovery(0),
 m_pFunctionInstanceCollectionQuery(0),
 m_pFiCollection(0)
	 {				
	 }

CWcnFdDiscoveryNotify::~CWcnFdDiscoveryNotify()
	{
		
		if(anySearchEvent)
		{
			CloseHandle(anySearchEvent);
		}
	}

//when function discovery finds device during its search look at the device to see if its the one you want
HRESULT CWcnFdDiscoveryNotify::OnUpdate(
        __in      QueryUpdateAction          enumQueryUpdateAction,
        __in      FDQUERYCONTEXT             fdqcQueryContext,
        __in      IFunctionInstance         *pIFunctionInstance)
	{

		HRESULT hr = S_OK;
		CComPtr<IPropertyStore> pPropStore;
		PROPVARIANT propVar;
		PropVariantInit(&propVar);

		WCHAR wszSrchUUID[64] = {0};

		UNREFERENCED_PARAMETER(pIFunctionInstance);
		UNREFERENCED_PARAMETER(fdqcQueryContext);


		switch(enumQueryUpdateAction)
			{
				case QUA_ADD:
					//QUA_ADD is called whenever a new device appears on the network.
					//("New" is defined as "new to you", so at the start of a query, you may
					// see a bunch of QUA_ADDs for all the existing devices).
					//
					//You can inspect the Function Instance's properties by calling
					//IFunctionInstance::OpenPropertyStore.  You can also get an IWCNDevice
					//interface from the IFunctionInstance interface pointer by calling
					//IFunctionInstance::QueryService.
					//
					//The IFunctionInstance interface pointer that is passed into this OnUpdate
					//callback doesn't belong to you, so if you want to save it for use beyond
					//the lifetime of this callback, you need to add a reference to it (and
					//possibly marshal it over to your thread).  Since this sample uses
					//CComPtr smart pointers, the additional reference count is automatic.
					
					//open the property store of this function instance to extract the UUID of the device
					hr = pIFunctionInstance->OpenPropertyStore(STGM_READ,
															   &pPropStore);

					if(FAILED(hr))
						{
							wprintf(L"\nERROR: Failed to open PropertyStore for FI.");
							goto cleanup;
						}
					

					//We need to decide if this IFunctionInstance belongs to the device
					//that we are interested in.  This sample demonstrates two different
					//ways of matching an IFunctionInstance:
					//    1.  Match an AP's SSID; or
					//    2.  Match any device's WPS UUID.
					if (bUseSSID) // 1. Match by SSID
						{
							hr = pPropStore->GetValue(PKEY_WCN_SSID, &propVar);
							if(FAILED(hr))
								{
									wprintf(L"\nERROR: Failed to Get SSID Value");
									goto cleanup;
								}

							if(propVar.vt != VT_LPWSTR)
								{
									wprintf(L"\nERROR: SSID Id is not VT_LPWSTR");
									goto cleanup;
								}	
							
							if (_wcsicmp(propVar.pwszVal, wszSSIDInHex) == 0)
								{
									// The SSID matches the one we computed earlier, so save the
									// function instance and tell the main thread it can continue.
									//
									// Note that if you are using an apartment threaded main thread
									// (e.g., it is a GUI thread), you'll have to marshal this pointer
									// so that it isn't smuggled across an apartment boundary.
									// This sample has all threads in the MTA, so we don't need 
									// to marshal anything.
									//
									// If you aren't using smart pointers, make sure to do AddRef here.
									m_pFunctionInstance = pIFunctionInstance;
									SetEvent(anySearchEvent);
								}
						}
					else // 2. Match by WPS UUID
						{
							hr = pPropStore->GetValue(PKEY_PNPX_GlobalIdentity,&propVar);
							if(FAILED(hr))
								{
									wprintf(L"\nERROR: Failed to Get UUID Value");
									goto cleanup;
								}

							if(propVar.vt != VT_LPWSTR)
								{
									wprintf(L"\nERROR: UUID Id is not VT_LPWSTR");
									goto cleanup;
								}
							
							// put curly braces around UUID so that you can compare it.
							StringCchCopy(wszSrchUUID+1, RTL_NUMBER_OF(wszSrchUUID)-1, propVar.pwszVal);
							wszSrchUUID[0] = L'{';
							wszSrchUUID[37] = L'}';
							wszSrchUUID[38] = L'\0';

							//Compare extracted UUID with the desired UUID
							if(_wcsicmp(wszSrchUUID,wszUUID) == 0)
								{
									// The UUID matches, so save the interface.  See the comments
									// above about threading and reference counting.
									m_pFunctionInstance = pIFunctionInstance;	
									SetEvent(anySearchEvent);
								}
						}

					break;

				case QUA_CHANGE:
					// The function instance may have changed one or more of its properties.
					// If you are displaying the properties to the user, you should mark the
					// display as dirty and refresh them soon.
					//
					// This sample code does not display device icons in a GUI, so it doesn't
					// handle QUA_CHANGE.
					break;

				case QUA_REMOVE:
					// The function instance is about to be removed.  You shouldn't take a
					// reference to the IFunctionInstance here, because it points to a device
					// that no longer exists anyway.  Instead, check its UUID and see if you
					// were displaying any device with that UUID to the user.  If so, remove
					// the device from your display.  Make sure to release any COM interfaces
					// that point to the device being removed.
					//
					// This sample code does not display device icons in a GUI, so it doesn't
					// handle QUA_REMOVE.
					break;


				default:
					break;
			}

cleanup:

		PropVariantClear(&propVar); //Free PKEY_PNPX_GlobalIdentity if not the one we want otherwise will be freed on cleanup
		return hr;
	}

//this callback is invoked if there is a fatal error and the search could not be completed
HRESULT CWcnFdDiscoveryNotify::OnError(
        __in      HRESULT                    hrFD,
        __in      FDQUERYCONTEXT             fdqcQueryContext,
        __in      const WCHAR               *pszProvider)
	{

		HRESULT hr = S_OK;

		UNREFERENCED_PARAMETER(fdqcQueryContext);

		SetEvent(anySearchEvent);

		wprintf(L"\nERROR: Provider [%s] returned error code [0x%x]",pszProvider,hrFD);


		return hr;
	}

//used to signal the end of the Function Discovery Search
HRESULT CWcnFdDiscoveryNotify::OnEvent(
        __in      DWORD                      dwEventID,
        __in      FDQUERYCONTEXT             fdqcQueryContext,
        __in      const WCHAR               *pszProvider)
	{

		HRESULT hr = S_OK;

		UNREFERENCED_PARAMETER(fdqcQueryContext);
		UNREFERENCED_PARAMETER(pszProvider);

		if(dwEventID == FD_EVENTID_SEARCHCOMPLETE)
		{
			wprintf(L"\nINFO: FD_EVENTID_SEARCHCOMPLETE");
			SetEvent(anySearchEvent);				
		}

		return hr;
	}

// create the Function discovery instance and setup an event to be used to signal the end of the Search 
HRESULT CWcnFdDiscoveryNotify::Init(__in BOOL bTurnOnSoftAP)
{

	HRESULT hr = ERROR_SUCCESS;
	anySearchEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	bUseSSID = FALSE;


	if (anySearchEvent == 0)
	{
		wprintf(L"ERROR: Failed to create the search event");
		hr = E_FAIL;
		goto cleanup;
	}

	//This interface is used by client programs to discover function instances, get the default function
	//instance for a category, and create advanced Function Discovery query objects that enable registering 
	//Function Discovery defaults, among other things.
	hr =  CoCreateInstance(
							CLSID_FunctionDiscovery,
							NULL,
							CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
							IID_PPV_ARGS(&m_pFunctionDiscovery));

	if(hr != S_OK)
	{
		wprintf(L"\nERROR: Failed to create instance of IFunctionDiscovery");
		goto cleanup;
	}

	//create an WCN instance collection Query for Function Discovery to use
	//This interface implements the asynchronous query for a collection of function instances based on category 
	//and subcategory. A pointer to this interface is returned when the collection query is created by the 
	//client program.
	hr = m_pFunctionDiscovery->CreateInstanceCollectionQuery(
															FCTN_CATEGORY_WCN,
															NULL,
															FALSE,
															(IFunctionDiscoveryNotification *) this,
															NULL,
															&m_pFunctionInstanceCollectionQuery);

	if(hr != S_OK)
	{
		wprintf(L"\nFailed to create Function Discovery query.");
		goto cleanup;
	}

	if (bTurnOnSoftAP)
	{
		// WCN can optionally turn on the SoftAP (aka WLAN Hosted Network) if the PC has a wireless
		// adapter that supports it.  You should only turn on SoftAP if you expect a wireless device
		// (like a wireless printer or a wireless picture frame) to connect to the hosted network.
		//
		// This API will return S_OK regardless if the computer supports SoftAP or not.  If you really
		// need to know if SoftAP is not supported, call WlanHostedNetworkQueryStatus and check
		// if the returned HostedNetworkState is wlan_hosted_network_unavailable.
		hr = m_pFunctionInstanceCollectionQuery->AddQueryConstraint( 
																	WCN_QUERY_CONSTRAINT_USE_SOFTAP,
																	FD_CONSTRAINTVALUE_TRUE);

		if (hr != S_OK)
		{
			wprintf(L"\nFailed to add the softap query constraint hr=[0x%x]", hr);
			goto cleanup;
		}
	}


cleanup:

	return hr;
}

//start the Function Discovery search based on the supplied UUID
HRESULT CWcnFdDiscoveryNotify::WcnFDSearchStart(__in UUID* pUUID, __in PWSTR pSearchSSID)
{
	HRESULT hr = S_OK;

	if(m_pFunctionInstanceCollectionQuery == NULL)
	{
		wprintf(L"\nERROR: FD Instance query is NULL.");
		hr = E_FAIL;
		goto cleanup;
	}

	//prefer the uuid over the SSID
	if (*pUUID != GUID_NULL)
	{
		//convert the UUID to a string 
		if(StringFromGUID2(*pUUID, wszUUID, ARRAYSIZE(wszUUID)) == 0)
		{
			wprintf(L"\nERROR: Convert from UUID to STRING Failed");
			goto cleanup;
		}
		bUseSSID = FALSE;
	}
	//USE the SSID to find the Router, NOTE: this will return the first SSID it finds which may not be 
	//the rotuer you want.
	else if (pSearchSSID)
	{
		//A SSID is an array of octets.  Since Function Discovery cannot filter searches based on
		//a raw byte array, the SSID is actually encoded in hexadecimal before being saved in the
		//FD property bag.  If you plan to use the PKEY_WCN_SSID, you should first convert the
		//raw octets to Unicode, then convert the Unicode to hexadecimal.
		BYTE Buf[DOT11_SSID_MAX_LENGTH+1];

		int r = WideCharToMultiByte(
									CP_ACP,					
									WC_NO_BEST_FIT_CHARS,	
									pSearchSSID,			
									-1, 
									(LPSTR)Buf, 
									ARRAYSIZE(Buf), 
									NULL, 
									NULL);

		if (r == 0)
		{   
			hr = HRESULT_FROM_WIN32(GetLastError()) ;
			wprintf(L"WideCharToMultiByte failed to covert the input ssid.");
			goto cleanup;
		}

		hr = ConvertStringToHex(
								r-1,
								Buf,
								ARRAYSIZE(wszSSIDInHex),
								wszSSIDInHex);

		if (hr != S_OK)
		{
			wprintf(L"ERROR: Converting the SSID [%s] to HEX failed with error code [%x]", pSearchSSID, hr);
			goto cleanup;
		}

		bUseSSID = TRUE;

	}
	else
	{
		wprintf(L"\nERROR: Search UUID and Search SSID are blank.");
		goto cleanup;
	}


	//Start the search
	wprintf(L"\nINFO: Stating the Function Discovery Search...");

	//Performs the query defined by IFunctionDiscovery::CreateInstanceCollectionQuery.
	hr = m_pFunctionInstanceCollectionQuery->Execute(&m_pFiCollection);

	// We expect asynchronous results.
	if (hr == E_PENDING)
	{
		hr = S_OK;
	}

	if (FAILED(hr))
	{
		wprintf(L"\nERROR: Function Discovery query failed to run with the following error hr =  0x%x.", hr);
		goto cleanup;
	}

cleanup:

	return hr;
}

//monitor when the Function Discovery events
BOOL CWcnFdDiscoveryNotify::WaitForAnyDiscoveryEvent(DWORD Timeout_ms)
{
	DWORD Index = 0;

	HRESULT hr = CoWaitForMultipleHandles(
								COWAIT_WAITALL,
								Timeout_ms,
								1,
								&anySearchEvent,
								&Index);

	if (hr == S_OK && Index == 0)
	{
		ResetEvent(anySearchEvent);
		return TRUE;
	}
	else
	{
		wprintf(L"\nERROR: Discovery timeout (after waiting %ums).", Timeout_ms);
		return FALSE;
	}
}

//once Function Discovery finds the instance we are looking for set the WCN Device instance
BOOL CWcnFdDiscoveryNotify::GetWCNDeviceInstance( __deref_out_opt IWCNDevice** ppWcnDevice)
{
	HRESULT hr = E_FAIL;
	BOOL returnValue = FALSE;

	*ppWcnDevice = NULL;

	//The instance we were looking for was not found.
	if( m_pFunctionInstance == NULL)
	{
		goto cleanup;
	}

	//Attempt to get IWCNDevice
	//Acts as the factory method for any services exposed through an implementation of 
	//IFunctionInstance. QueryService creates and initializes instances of a requested interface 
	//if the service from which the interface was requested supports the interface.
	hr = m_pFunctionInstance->QueryService(
											SID_WcnProvider,
											IID_PPV_ARGS(ppWcnDevice));

	if(hr != S_OK)
	{
		wprintf(L"\nERROR: Failed to get IWCNDevice from the Function Instance hr=[0x%x].",hr);
		returnValue = FALSE;
		goto cleanup;
	}


	returnValue = TRUE;

cleanup:

	return returnValue;
}
