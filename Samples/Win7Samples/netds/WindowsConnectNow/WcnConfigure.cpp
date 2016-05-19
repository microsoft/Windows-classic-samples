// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Module Name:
//    WcnConfigure.cpp
//
// Abstract:
//    This is the program entry point were the following takes place
//			Command line parsing
//			Command line validation
//			call the WCN functions to perform a configure or push button scenario on a WCN enabled device

#include "WcnConfigure.h"

class ATLGlobal:public CAtlExeModuleT<ATLGlobal>{} _Module; 

HRESULT GetWCNDeviceInformation(__in IWCNDevice* pDevice, __out WCN_DEVICE_INFO_PARAMETERS* pWCNDeviceInformation)
{
	HRESULT hr = ERROR_SUCCESS;
	
	//A WCN device can have a variety of attributes.  (These attributes generally correspond
	//to TLVs in the WPS specification, although not all WPS TLVs are available as WCN attributes).
	//You can use the IWCNDevice::Get*Attribute to read  these attributes.  Not all devices send 
	//all attributes -- if the device did not send a particular attribute, the Get*Attribute API 
	//will return HRESULT_FROM_WIN32(ERROR_NOT_FOUND).
	//
	//This sample demonstrates how to get the most common attributes that would be useful for
	//displaying in a user interface.


	//
	// WCN_TYPE_DEVICE_NAME
	//

	//The IWCNDevice::GetStringAttribute method gets a cached attribute from the device as a string.
	hr = pDevice->GetStringAttribute(
									WCN_TYPE_DEVICE_NAME, 
									ARRAYSIZE(pWCNDeviceInformation->wszDeviceName), 
									pWCNDeviceInformation->wszDeviceName);
	if (hr != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR:  Failed to get the Device Name from the IWCNDevice instance.  hr=[0x%x]", hr);
		goto cleanup;
	}

	wprintf(L"\nINFO: Device Name: [%s]",pWCNDeviceInformation->wszDeviceName);


	//
	// WCN_TYPE_MANUFACTURER
	//

	hr = pDevice->GetStringAttribute(
									WCN_TYPE_MANUFACTURER, 
									ARRAYSIZE(pWCNDeviceInformation->wszManufacturerName), 
									pWCNDeviceInformation->wszManufacturerName);
	if (hr != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR: Failed to get the device manufacturer from the ICWNDevice instance, hr=[0x%x]", hr);
		goto cleanup;
	}

	wprintf(L"\nINFO: Manufacturer Name: [%s]", pWCNDeviceInformation->wszManufacturerName);


	//
	// WCN_TYPE_MODEL_NAME
	//

	hr = pDevice->GetStringAttribute(
										WCN_TYPE_MODEL_NAME, 
										ARRAYSIZE(pWCNDeviceInformation->wszModelName), 
										pWCNDeviceInformation->wszModelName);
	if (hr != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR: Failed to get the device model name from the ICWNDevice instance, hr=[0x%x]", hr);
		goto cleanup;				
	}

	wprintf(L"\nINFO: Model Name: [%s]", pWCNDeviceInformation->wszModelName);


	//
	// WCN_TYPE_MODEL_NUMBER
	// Note that the Model Number is actually a string.  Most devices have alpha-numeric
	// model numbers, like "AB1234CD".

	hr = pDevice->GetStringAttribute(
										WCN_TYPE_MODEL_NUMBER, 
										ARRAYSIZE(pWCNDeviceInformation->wszModelNumber), 
										pWCNDeviceInformation->wszModelNumber);
	if (hr != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR: Failed to get the device model name from the ICWNDevice instance, hr=[0x%x]", hr);
		goto cleanup;				
	}
	
	wprintf(L"\nINFO: Model Number: [%s]", pWCNDeviceInformation->wszModelNumber);


	//
	// WCN_TYPE_SERIAL_NUMBER
	// Note that the Serial Number is actually a string.  Some devices send strings that
	// aren't meaningful, like "(none)" or just the empty string.

	hr = pDevice->GetStringAttribute(
										WCN_TYPE_SERIAL_NUMBER, 
										ARRAYSIZE(pWCNDeviceInformation->wszSerialNumber), 
										pWCNDeviceInformation->wszSerialNumber);
	if (hr != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR: Failed to get the device model name from the ICWNDevice instance, hr=[0x%x]", hr);
		goto cleanup;				
	}

	wprintf(L"\nINFO: Serial Number: [%s]", pWCNDeviceInformation->wszSerialNumber);


	//
	// WCN_TYPE_CONFIG_METHODS
	// This is a bit mask of the values from WCN_VALUE_TYPE_CONFIG_METHODS.
	// For example, a devices indicates support for pushbutton if its Config
	// Methods value includes the WCN_VALUE_CM_PUSHBUTTON flag.

	//The GetIntegerAttribute method gets a cached attribute from the device as an integer.
	hr = pDevice->GetIntegerAttribute(
										WCN_TYPE_CONFIG_METHODS, 
										&pWCNDeviceInformation->uConfigMethods);
	if (hr != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR: Failed to get the device model name from the ICWNDevice instance, hr=[0x%x]", hr);
		goto cleanup;				
	}
		
cleanup:
	return hr;
}

HRESULT RunScenario(__in CONFIGURATION_PARAMETERS* configParams)
{
	//common declarations
	UINT status = ERROR_SUCCESS;	
	HRESULT hr = S_OK;
	UINT pinLen = Pin_Length_8;

	//pin needs to be a null terminated ascii char[] for the IWCNDevice::SetPassword function 
	char pin[Pin_Length_8 + 1] = {0};

	
	int result = 0;

	//WCN declarations
	CComPtr<IWCNDevice> pDevice;
	CComObject<WcnConnectNotification>* pWcnConNotif = NULL;	
	CComObject<CWcnFdDiscoveryNotify> * wcnFdDiscoveryNotify = NULL;	

	//Wlan variable declarations
	WCHAR profileBuffer[WCN_API_MAX_BUFFER_SIZE] = {0}; 
	HANDLE wlanHandle = 0;
	DWORD negVersion = 0;
	GUID interfaceGuid = {0};		
	WLAN_INTERFACE_INFO_LIST* pInterfaceList = 0;
	DWORD wlanResult = 0;		
	WLAN_CONNECTION_PARAMETERS connParams;
	ZeroMemory(&connParams,sizeof(connParams));
	WCN_DEVICE_INFO_PARAMETERS WCNDeviceInformation;
	PWSTR pWlanProfileXml = NULL;
	DWORD dwFlags = WLAN_PROFILE_GET_PLAINTEXT_KEY; 


	//The following wlan profile xml is used to configure an unconfigured WCN enabled Router or device.
	//See http://msdn.microsoft.com/en-us/library/bb525370(VS.85).aspx on how to generate a wlan profile.
	//Alternatively, you can read an existing network profile by calling WlanGetProfile.
	WCHAR WCNConnectionProfileTemplate[] =
		L"<?xml version=\"1.0\" ?>"
		L""
		L"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">"
		L"    <name>%s</name>"
		L""
		L"    <SSIDConfig>"
		L"        <SSID>"
		L"            <name>%s</name>"
		L"        </SSID>"
		L"    </SSIDConfig>"
		L"    "
		L"    <connectionType>ESS</connectionType>"
		L"    <connectionMode>auto</connectionMode>"
		L""
		L"    <MSM>"
		L"        <security>"
		L"            <authEncryption>"
		L"                <authentication>WPA2PSK</authentication>"
		L"                <encryption>AES</encryption>"
		L"            </authEncryption>"
		L""
		L""
		L"            <sharedKey>"
		L"                <keyType>passPhrase</keyType>"
		L"                <protected>false</protected>"
		L"                <keyMaterial>%s</keyMaterial>"
		L"            </sharedKey>"
		L""
		L"        </security>"
		L"    </MSM>"
		L"</WLANProfile>";


	std::wstring profileXML;

	//open a wlan handle - this will be used later for saving the profile to the system
	status = WlanOpenHandle(
							WLAN_API_VERSION_2_0,
							NULL,
							&negVersion,
							&wlanHandle);

	if (status != ERROR_SUCCESS)
	{
		wprintf(L"\nERROR: WlanOpenHandle failed with the following error code [%d]", status);
		hr = S_FALSE;
		goto cleanup;
	}

	// Get the first wlan device
	// ideally you would want to be able to choose the wireless device you want to use
	status = WlanEnumInterfaces(
								wlanHandle,
								NULL,
								&pInterfaceList);

	if(status != ERROR_SUCCESS)
	{				
		wprintf(L"\nERROR: WlanEnumInterfaces failed with the following error code [0x%d]",status);
		hr = S_FALSE;
		goto cleanup;		
	}

	//Make sure there is at least one wlan interface on the system
	if (pInterfaceList == 0 || pInterfaceList->dwNumberOfItems == 0)
	{
		wprintf(L"\nERROR: No wireless network adapters on the system");
		hr = S_FALSE;
		goto cleanup;
	}

	//get the wlan interface GUID
	interfaceGuid = pInterfaceList->InterfaceInfo[0].InterfaceGuid;

	//Create an instance of the IWCNConnectNotify Interface
	hr = CComObject<WcnConnectNotification>::CreateInstance(&pWcnConNotif);
	if (hr != S_OK)
	{
		wprintf(L"\nERROR: Creating an instance of WcnConnectNotification failed with the following error hr=[0x%x]", hr);
		goto cleanup;
	}
	pWcnConNotif->AddRef();

	hr = CComObject<CWcnFdDiscoveryNotify>::CreateInstance(&wcnFdDiscoveryNotify);
	if (hr != S_OK)
	{
		wprintf(L"\nERROR: Creating an instance of CWcnFdDiscoveryNotify failed with the following error hr=[0x%x]", hr);
		goto cleanup;
	}
	wcnFdDiscoveryNotify->AddRef();

	//initialize WcnConnectNotification
	hr = pWcnConNotif->Init();
	if(hr !=S_OK)
	{
		wprintf(L"\nERROR: Creating a connection notification event failed with the following error hr=[0x%x]", hr);
		goto cleanup;
	}

	//initialize CWcnFdDiscoveryNotify 
	hr = wcnFdDiscoveryNotify->Init(configParams->bTurnOnSoftAP);
	if(hr != S_OK)
	{
		wprintf(L"\nERROR: Initializing Function Discovery notify failed with the following error hr=[0x%x].",hr);
		goto cleanup;
	}

	//Search for WCN device with function discovery
	hr = wcnFdDiscoveryNotify->WcnFDSearchStart(&configParams->pDeviceUUID, configParams->pSearchSSID);		
	if(hr != S_OK)
	{
		wprintf(L"\nERROR: Function Discovery search failed to start with the following error hr=[0x%x].",hr);
		goto cleanup;
	}

	//Wait for Function Discovery to complete
	wcnFdDiscoveryNotify->WaitForAnyDiscoveryEvent(Discovery_Event_Wait_Time_MS);

	//Attempt to get the IWCNDevice instance
	if(wcnFdDiscoveryNotify->GetWCNDeviceInstance(&pDevice))
	{
		//get information about the device from the IWCNDevice instance
		wprintf(L"\nINFO: The following Device was found by Function Discovery.");
		hr = GetWCNDeviceInformation(pDevice, &WCNDeviceInformation);
		if (hr != S_OK)
		{
			wprintf(L"\nERROR: Failed to get the Device information from the IWCNDevice Instance, hr=[0x%x]", hr);
			goto cleanup;
		}
	}
	else
	{
		wprintf(L"\nERROR: Device was NOT found by Function Discovery.");
		hr = S_FALSE;
		goto cleanup;
	}
	
	

	//The following segment generates a WLAN profile from the template above then saves it to the
	//WLAN store. It the retrieves the profile from the WLAN store for use in configuring a router
	//or device.
	if (configParams->enumConfigScenario != PCConfigPin 
		&& configParams->enumConfigScenario != PCConfigPushButton)
	{
		//add the profiles ssid and passphrase to the wlan profile template
		swprintf_s(
				profileBuffer, 
				WCNConnectionProfileTemplate, 
				configParams->pProfileSSID, 
				configParams->pProfileSSID, 
				configParams->pProfilePassphrase);


		//Add the created profile to the wlan store
		status = WlanSetProfile(
								wlanHandle, 
								&interfaceGuid, 
								0,				//all-user profile
								profileBuffer, 
								NULL,			// Default Security - All user profile
								TRUE,			// Overwrite profile
								NULL,			// reserved
								&wlanResult);

		if (status != ERROR_SUCCESS)
		{
			wprintf(L"\nERROR: Failed to save the profile return code was [0x%x]", wlanResult);
			hr = S_FALSE;
			goto cleanup;
		}
		else
		{
			wprintf(L"\nINFO: Successfully saved the profile to the wlan store");
		}

		//Here is where the profile is retrieved from the wlan store to be used in the configuration
		//of the device.  
		//If so desired a list of available profiles could be presented to the user so that 
		//they could decied which profile will be used to configure the device
		//The wlan profile must be retrieved in plain text inorder for the IWCNDEVICE::SetNetWorkProfile
		// method to succeede.  In order to do this you need to be elevated to get the wlan profile
		// in plain text.
		status = WlanGetProfile(
								wlanHandle,
								&interfaceGuid,
								configParams->pProfileSSID,
								NULL,						//reserved
								&pWlanProfileXml,
								&dwFlags,					// Flags - get profile in plain text 
								NULL);						// GrantedAccess - none

		if (status != ERROR_SUCCESS)
		{
			wprintf(L"\nERROR: WlanGetprofile Failed to get profile [%s] with error code [0x%x]", configParams->pProfileSSID, status);
			hr = S_FALSE;
			goto cleanup;
		}
		else
		{
			wprintf(L"\nINFO: Successfully retrieved profile [%s] from the wlan store.", configParams->pProfileSSID);
		}

		//check to make sure the profile from the wlan store is not a Group Policy profile
		if (WLAN_PROFILE_GROUP_POLICY & dwFlags)
		{
			wprintf(L"\nERROR: Profile [%s] is a group policy WLAN profile which is not supported by WCN", configParams->pProfileSSID);
			hr = S_FALSE;
			goto cleanup;
		}


		//The IWCNDevice::SetNetworkProfile method queues an XML WLAN profile to be 
		//provisioned to the device. This method may only be called prior to IWCNDevice::Connect.
		hr = pDevice->SetNetworkProfile(pWlanProfileXml);
		if(hr != S_OK)
		{
			wprintf(L"\nERROR: IWCNDevice::SetNetworkProfile failed with error code [0x%x]", hr);
			goto cleanup;
		}
		else
		{				
			wprintf(L"\nINFO: IWCNDevice::SetNetworkProfile() succeeded with result [0x%x]", hr);
		}
	}
	
	switch (configParams->enumConfigScenario)
	{
		case DeviceConfigPushButton:
			
			pinLen = 0;
			break;

		case DeviceConfigPin:
		case RouterConfig:
			if (configParams->pDevicePin == 0)
			{
				wprintf(L"\nERROR: Pin must not be 0 when doing a pin configuration");
				hr = S_FALSE;
				goto cleanup;
			}


			 result = WideCharToMultiByte(
							CP_UTF8,
							0,
							configParams->pDevicePin,
							-1,
							(LPSTR)pin,
							sizeof(pin),
							NULL,
							NULL);
			if (result == 0 )
			{
				wprintf(L"\nERROR: Failed to convert the pin to multibyte.");
				goto cleanup;
			}


			pinLen = sizeof(pin) - 1 ;
			break;

		case PCConfigPushButton:
			//check to make sure the device supports push button before doing the push button configuration
			if (WCNDeviceInformation.uConfigMethods & WCN_VALUE_CM_PUSHBUTTON) 
			{
				//set the pin length to 0 this is necessary for a Push button configuration scenario				
				pinLen = 0;
			}
			else
			{
				wprintf(L"ERROR: The [%s] device does not support the Push Button Method", WCNDeviceInformation.wszDeviceName);
				hr = S_FALSE;
				goto cleanup;
			}
			break;
			
		case PCConfigPin:
			//check to make sure the device supports pin before doing the pin configuration
			if ((WCNDeviceInformation.uConfigMethods & WCN_VALUE_CM_LABEL)|| 
				(WCNDeviceInformation.uConfigMethods & WCN_VALUE_CM_DISPLAY))
			{
				if (configParams->pDevicePin == 0)
				{
					wprintf(L"\nERROR: Pin must not be 0 when doing a pin configuration");
					hr = S_FALSE;
					goto cleanup;
				}
			
				result = WideCharToMultiByte(
							CP_UTF8,					//CodePage
							0,							//Unmapped character flags
							configParams->pDevicePin,
							-1,							//null terminated string
							(LPSTR)pin,
							sizeof(pin),
							NULL,						//lpDefaultChar - use system default value
							NULL);						//lpUsedDefaultChar ignored
				if (result == 0 )
				{
					wprintf(L"\nERROR: Failed to convert the pin to multibyte.");
					goto cleanup;
				}

				pinLen = sizeof(pin) - 1 ;

			}
			else
			{
				wprintf(L"\nERROR: The [%s] device does not supprot the pin method", WCNDeviceInformation.wszDeviceName);
				hr = S_FALSE;
				goto cleanup;
			}
			break;

		default:
			break;
	}
	
	//The IWCNDevice::SetPassword method configures the authentication method value, and if required, 
	//a password used for the pending session. This method may only be called prior to IWCNDevice::Connect.
	hr = pDevice->SetPassword(
								configParams->enumConfigType, 
								pinLen,
								(BYTE*)pin);

	if(hr != S_OK)
	{	
		wprintf(L"\nERROR: IWCNDevice::SetPassword failed with error code [0x%x]", hr);
		goto cleanup;
	}
	else
	{
		wprintf(L"\nINFO: IWCNDevice::SetPassword succeeded with result [0x%x]", hr);
	}


	//The IWCNDevice::Connect method initiates the session.
	hr = pDevice->Connect(pWcnConNotif);
	if(hr != S_OK)
	{
		//Device Push button configuration is only supported on SoftAP capable wireless Nics 
		if (hr == HRESULT_FROM_WIN32(ERROR_CONNECTION_UNAVAIL) 
			&& 	configParams->enumConfigScenario == DeviceConfigPushButton)
		{
			wprintf(L"\nERROR: PushButton Configuration of non AP devices is only supported on");
			wprintf(L"\n       SoftAP capable wireless network cards.");
		}
		else
		{
			wprintf(L"\nERROR: IWCNDevice::Connect failed with error code [0x%x]", hr);
		}
		goto cleanup;
	}
	else
	{
		wprintf(L"\nINFO: IWCNDevice::Connect succeeded with result [0x%x]", hr);
	}

	//wait for the configuration result
	hr = pWcnConNotif->WaitForConnectionResult();
	if (hr != S_OK)
	{
		wprintf(L"ERROR: WaitforconnectionResult returned the following error [ox%x]", hr);
		goto cleanup;
	}

	//check to see which connection callbacks were called
	if(pWcnConNotif->connectSucceededCallBackInvoked)
	{
		wprintf(L"\nINFO: IWCNConnectNotify::ConnectSucceeded was invoked");		
	}
	else if(pWcnConNotif->connectFailedCallBackInvoked)
	{
		wprintf(L"\nERROR: IWCNConnectNotify::ConnectFailed was invoked");
		hr = S_FALSE;
		goto cleanup;
	}

	
	//save the profile from the IWCNDevice instance to the WLAN store if doing a PCConfigPushButton 
	//or a PCConfigPin scenario

	// this is the profile that was received from the router
	if (configParams->enumConfigScenario == PCConfigPushButton  || configParams->enumConfigScenario == PCConfigPin)
	{	
		//The IWCNDevice::GetNetworkProfile method gets a network profile from the device.
		hr = pDevice->GetNetworkProfile(ARRAYSIZE(profileBuffer), profileBuffer);		
		if(hr != S_OK)
		{
			wprintf(L"\nERROR: IWCNDevice::GetNetworkProfile failed with  [0x%x]", hr);
			goto cleanup;
		}

		//save the profile to the system if doing a RouterConfig or a pushbutton scenario
		//The SoftapConfig and DeviceConfig scenarios will generally use a profile that is already on the system
		//save the profile to the wlan interface			
		status = WlanSetProfile(
								wlanHandle, 
								&interfaceGuid, 
								0,				//Flags - none
								profileBuffer, 
								NULL,			// Default Security - All user profile
								TRUE,			// Overwrite profile
								NULL,			// reserved
								&wlanResult);

		if (status != ERROR_SUCCESS)
		{
			wprintf(L"\nERROR: Failed to save the profile to the WLAN store, return code was [0x%x]", wlanResult);
			hr = S_FALSE;
		}
		else
		{
			wprintf(L"\nINFO: Successfully saved the profile to the WLAN store");
		}
	}
	
	//Display the SSID and passphrase used to configure the Router or device
	if (configParams->enumConfigScenario != PCConfigPin && configParams->enumConfigScenario != PCConfigPushButton)
	{
		wprintf(L"\nINFO: Profile SSID Used: [%s]", configParams->pProfileSSID);
		wprintf(L"\nINFO: Profile Passphrase Used: [%s]", configParams->pProfilePassphrase);
	}

cleanup:

	if(pWcnConNotif)
	{
		pWcnConNotif->Release();
		pWcnConNotif = 0;
	}

	if(wcnFdDiscoveryNotify)
	{
		wcnFdDiscoveryNotify->Release();
		wcnFdDiscoveryNotify = 0;
	}

	if (wlanHandle != NULL)
	{
		WlanCloseHandle(wlanHandle,NULL);
	}

	if (pInterfaceList != NULL)
	{
		WlanFreeMemory(pInterfaceList);
	}

	return hr;
}

void printUsage()
{
	wprintf(L"\nUSAGE:");
	wprintf(L"\n WCNConfigure.exe"); 
    wprintf(L"\n  Scenario=[DeviceConfigPin | DeviceConfigPushButton | RouterConfig |");
	wprintf(L"\n            PCConfigPushButton | PCConfigPin ]");
	wprintf(L"\n  [UUID=<uuid of device> | SEARCHSSID=<ssid of device to find>]");
	wprintf(L"\n  [PIN=<pin of device>]"); 
	wprintf(L"\n  [PROFILESSID=<ssid to use in profile>]");
	wprintf(L"\n  [PROFILEPASSPHRASE=<passphrase to use in profile>]");
	wprintf(L"\n");		
	wprintf(L"\nParameters:");
	wprintf(L"\n Scenario - choose the operation you wish to perform ");
	wprintf(L"\n     DeviceConfigPushButton - Configure a WCN enabled device, such as a picture");
	wprintf(L"\n                              frame using the button on the device");
	wprintf(L"\n     DeviceConfigPin - Configure a WCN enabled device, such as a picture frame");
	wprintf(L"\n                       using the device supplied pin");
	wprintf(L"\n     RouterConfig - Configure a WCN enabled Wireless Router");
	wprintf(L"\n     PCConfigPushButton - Get the wireless profile from a WCN enabled router");
	wprintf(L"\n                          using the Push Button on the device.");
	wprintf(L"\n     PCConfigPin - Get the wireless profile from a WCN enabled rotuer using the");
	wprintf(L"\n                   supplied pin.");
	wprintf(L"\n");
	wprintf(L"\n UUID - Enter a device UUID in the following format xxxx-xxxx-xxxx-xxxxxxxxxxxx");
	wprintf(L"\n        UUID is necessary for the DeviceConfigPushButton and DeviceConfigPin");
	wprintf(L"\n        scenarios. Use either UUID or SEARCHSSID for the RouterConfig, PCConfigPin");
	wprintf(L"\n        and PCConfigPushButton scenarios.");
	wprintf(L"\n");
	wprintf(L"\n SEARCHSSID - Enter in the SSID for the Router you are looking to configure.");
	wprintf(L"\n              SEARCHSSID is only valid in the RouterConfig, PCConfigPushButton and ");
	wprintf(L"\n              PCConfigPin scenarios. Use either UUID or SEARCHSSID for the these");
	wprintf(L"\n              scenarios. NOTE: Using SSID will return the first device");
	wprintf(L"\n              found with that ssid.  If there is more than one device with the");
	wprintf(L"\n              same ssid use the UUID instead");
	wprintf(L"\n");
	wprintf(L"\n PIN  - Enter the pin of the device");
	wprintf(L"\n        PIN is only valid when using the RouterConfig and DeviceConfigPIN");
	wprintf(L"\n        Scenarios.");
	wprintf(L"\n");
	wprintf(L"\n PROFILESSID - When present this SSID will be used in the WLAN profile that is");
	wprintf(L"\n               pushed to the router/device otherwise a default SSID of WCNSSID ");
	wprintf(L"\n               will be used");
	wprintf(L"\n");
	wprintf(L"\n PROFILEPASSPHRASE - when present this passphrase will be used in the wlan");
	wprintf(L"\n                     profile that is pushed to the router/device. Otherwise, a");
	wprintf(L"\n                     random default passphrase will be used\n\n");
}

BOOL validateParameters(__in CONFIGURATION_PARAMETERS* configParams)
{
	BOOL bReturnValue = FALSE;

	switch (configParams->enumConfigScenario)
	{
		//DeviceConfig and RouterConfig require both the uuid and the device pin
		case DeviceConfigPin:
			if (configParams->pDeviceUUID != GUID_NULL && configParams->pDevicePin != 0)
			{
				bReturnValue = TRUE;
			}
			break;

		case DeviceConfigPushButton:
			if (configParams->pDeviceUUID != GUID_NULL)
			{
				bReturnValue = TRUE;
			}
			break;

		case RouterConfig:
			//uuid or searchssid must be present in order to continue
			if ((configParams->pDeviceUUID != GUID_NULL || configParams->pSearchSSID != 0)
				&& configParams->pDevicePin != 0)
			{			
				bReturnValue = TRUE;
			}
			break;

		case PCConfigPushButton:
			if (configParams->pDeviceUUID != GUID_NULL || configParams->pSearchSSID != 0)
			{
				bReturnValue = TRUE;
			}
			break;

		case PCConfigPin:
			if ((configParams->pDeviceUUID != GUID_NULL || configParams->pSearchSSID != 0)
				&& configParams->pDevicePin != 0)
			{
				bReturnValue = TRUE;
			}
			break;

		default:
			break;
	}

	return bReturnValue; 
}


HRESULT parseCommandLineArguments(__out CONFIGURATION_PARAMETERS* configParameters, __in DWORD argc, __in_ecount(argc) WCHAR **pArg)
{
	HRESULT hr = ERROR_SUCCESS;

	
	
	//parse the command line inputs
	for(DWORD i = 1;i < argc;i++)
	{
		wchar_t* pCommand = 0;
		wchar_t* pContext = 0;

		pCommand = wcstok_s(
							pArg[i],
							L"=",
							&pContext);

		if(_wcsicmp(pCommand,L"Scenario") == 0)
		{
			if (pContext)
			{
				if ( _wcsicmp(pContext, L"RouterConfig") == 0)
				{
					configParameters->enumConfigScenario = RouterConfig;
					configParameters->enumConfigType = WCN_PASSWORD_TYPE_PIN;
				}
				else if ( _wcsicmp(pContext, L"DeviceConfigPushButton") == 0)
				{
					configParameters->enumConfigScenario = DeviceConfigPushButton;
					configParameters->enumConfigType = WCN_PASSWORD_TYPE_PUSH_BUTTON;
					configParameters->bTurnOnSoftAP = TRUE;
				}
				else if ( _wcsicmp(pContext, L"DeviceConfigPin") == 0)
				{
					configParameters->enumConfigScenario = DeviceConfigPin;
					configParameters->enumConfigType = WCN_PASSWORD_TYPE_PIN;
					configParameters->bTurnOnSoftAP = TRUE;
				}
				else if (_wcsicmp(pContext, L"PCConfigPushButton") == 0)
				{										
					configParameters->enumConfigScenario = PCConfigPushButton;
					configParameters->enumConfigType = WCN_PASSWORD_TYPE_PUSH_BUTTON;										
				}
				else if (_wcsicmp(pContext, L"PCConfigPin") == 0)
				{										
					configParameters->enumConfigScenario = PCConfigPin;
					configParameters->enumConfigType = WCN_PASSWORD_TYPE_PIN;										
				}
				else
				{
					wprintf(L"\nERROR: The supplied option for Scenairo is not valid\n\n");
					hr = S_FALSE;
					printUsage();
					goto cleanup;
				}
			}
		}
		
		if(_wcsicmp(pCommand,L"UUID") == 0)
		{						
			if (pContext)
			{
				if (wcslen(pContext) == UUID_LENGTH)
				{
					hr = UuidFromString((RPC_WSTR)pContext, &configParameters->pDeviceUUID);
					if (hr != ERROR_SUCCESS)
					{
						wprintf(L"\nERROR: Failed to convert supplied uuid:  HR=[0x%x]\n\n",hr);
						printUsage();
						goto cleanup;
					}
				}
				else
				{
					wprintf(L"\nERROR: The supplied UUID is not valid\n\n");
					hr = S_FALSE;
					printUsage();
					goto cleanup;
				}
			}
		}

		if (_wcsicmp(pCommand,L"PIN") == 0)
		{
			//valid pin lengths are 4 and 8
			if (pContext)
			{
				if (wcslen(pContext) == Pin_Length_8 || wcslen(pContext) == Pin_Length_4) 
				{		
					configParameters->pDevicePin = pContext;	
				}
				else
				{
					wprintf(L"\nERROR: The supplied PIN is not valid\n\n");
					hr = S_FALSE;
					printUsage();
					goto cleanup;
				}
			}
		}

		if (_wcsicmp(pCommand, L"SEARCHSSID") == 0)
		{
			if (pContext)
			{
				if (wcslen(pContext) > DOT11_SSID_MAX_LENGTH)
				{
					wprintf(L"\nERROR: Search SSID length is too long");
					hr = S_FALSE;
					goto cleanup;
				}

				configParameters->pSearchSSID = pContext;
			}
		}

		if (_wcsicmp(pCommand, L"ProfileSSID") == 0)
		{
			if (pContext)
			{
				if (wcslen(pContext) > DOT11_SSID_MAX_LENGTH + 1)
				{
					wprintf(L"\nERROR: profile SSID length is too long");
					hr = S_FALSE;
					goto cleanup;
				}

				configParameters->pProfileSSID = pContext;
			}
		}

		if (_wcsicmp(pCommand, L"ProfilePassphrase") == 0)
		{
			if (pContext)
			{
				if (wcslen(pContext) < PASSPHRASE_MIN_LENGTH || wcslen(pContext) > PASSPHRASE_MAX_LENGTH)
				{
					wprintf(L"\nERROR: Passphrase must be between 8 and 63 characters long");
					hr = S_FALSE;
					goto cleanup;
				}
				

				configParameters->pProfilePassphrase = pContext;
			}
		}
	}
	
	//if the ProfileSSID is not present add one
	if (configParameters->pProfileSSID == 0)
	{
		configParameters->pProfileSSID = L"WCNSSID";
	}

	//if ProfilePassphrase is not supplied generate one based on a random number 
	if (configParameters->pProfilePassphrase == 0)
	{
		
		WCHAR tempPassphrase[dwCharsToGenerate + 1] = { };
		for (int i = 0; i < dwCharsToGenerate - 1; ++i)
		{
			if(i == 4 || i== 9) //Apply dash separator for the passphrase
			{
				tempPassphrase[i] = '-';
			}
			else
			{
				unsigned int r;
				errno_t err = rand_s(&r);

				if (err != 0)
				{
					hr = S_FALSE;
					wprintf(L"\nERROR: Failed to generate a random number.");
					goto cleanup;
				}

				tempPassphrase[i] = PassphraseCharacterSet[r % 65];
				
			}
		}
		tempPassphrase[dwCharsToGenerate] = L'\0';

	    configParameters->pProfilePassphrase = _wcsdup(tempPassphrase);
		configParameters->bFreePassphrase = TRUE;
	}

	//validate the command line parameters
	if (!validateParameters(configParameters))
	{
		printUsage();	
		wprintf(L"\nERROR: Missing Parameter");
		hr = S_FALSE;
		goto cleanup;
	}

cleanup:
	return hr;

}


int  wmain(__in DWORD argc, __in_ecount(argc) WCHAR **pArg)
{
	HRESULT hr = ERROR_SUCCESS;
	UINT status = ERROR_SUCCESS;
	BOOL fUnintializeCom = FALSE;
	CONFIGURATION_PARAMETERS configParameters;
	DWORD dwVersion = 0;
	DWORD dwMajorVersion = 0;
	DWORD dwMinorVersion= 0;
	
	//check to make sure we are running on Windows 7 or later
	dwVersion = GetVersion();
	dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
	
	configParameters.enumConfigScenario = None;
	configParameters.pDeviceUUID = GUID_NULL;
	configParameters.pDevicePin = 0;
	configParameters.pSearchSSID = 0;
	configParameters.bTurnOnSoftAP = FALSE;
	configParameters.pProfilePassphrase = 0;
	configParameters.pProfileSSID = 0;
	configParameters.bFreePassphrase = FALSE;
	
	
	//dwMajorVersion must be 6 or greater and dwMinorVersion must be 1 or greater (Vista is 6.0)
	if (dwMajorVersion <= WINDOWS7_MAJOR_VERSION)
	{
		if ((dwMajorVersion == WINDOWS7_MAJOR_VERSION && dwMinorVersion < WINDOWS7_MINOR_VERSION) 
			|| dwMajorVersion < WINDOWS7_MAJOR_VERSION)
		{
			wprintf(L"\nERROR: This Application requires Windows 7 or later\n\n");
			goto cleanup;
		}
		
	}

	//initialize Com
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);
	if (hr != S_OK && hr != S_FALSE)
	{
		wprintf(L"\nERROR: Com failed to initialize with the following error [0x%x]\n\n", hr);
		goto cleanup;
	}
	fUnintializeCom = TRUE;

	//get the parameters from the command line
	hr = parseCommandLineArguments(&configParameters, argc, pArg);
	if (hr != S_OK)
	{
		wprintf(L"\nERROR: failed to parse command line arguments\n\n");
		goto cleanup;
	}
	
	//select the scenario you wish to run
	switch (configParameters.enumConfigScenario)
	{
		//configure a wireless device using the device supplied pin code
		case DeviceConfigPin: 
			status = RunScenario(&configParameters);

			if (status != ERROR_SUCCESS)
			{
				wprintf(L"\nERROR: Configuration of the wireless Device with a PIN Failed\n\n");
			}
			else
			{
				wprintf(L"\nINFO: Configuration of the wireless Device with a PIN Succeeded\n\n");
			}
			break;

		//configure a wireless device with by pushing the configuration button on the device 
		case DeviceConfigPushButton:
			status = RunScenario(&configParameters);
			if (status != ERROR_SUCCESS)
			{
				wprintf(L"\nERROR: Configuration of the Wireless device with push button Failed\n\n");
			}
			else
			{
				wprintf(L"\nINFO: Configuration of the Wireless device with push button Succeeded\n\n");
			}
			break;

		//configure a router using the router supplied pin code
		case RouterConfig:
			status = RunScenario(&configParameters);
			if (status != ERROR_SUCCESS)
			{
				wprintf(L"\nERROR: Configuration of the Wireless Router Failed\n\n");
			}
			else
			{
				wprintf(L"\nINFO: Configuration of the Wireless Router Succeeded\n\n");
			}
			break;
		
		// get the wireless profile from the router using the configuration button on the router
		case PCConfigPushButton:
			wprintf(L"\n\nINFO: Please push the 'WCN Configure Button' on the router and then hit enter.\n");
			if (getchar() == '\n')
			{
				wprintf(L"\nINFO: Attempting to get the Wireless profile from the router");
			}

			status = RunScenario(&configParameters);
			if (status != ERROR_SUCCESS)
			{
				wprintf(L"\nERROR: PC Configuration with the Push Button failed\n\n");
			}
			else
			{
				wprintf(L"\nINFO: PC Configuration with the Push Button succeeded\n\n");
			}
			break;

		case PCConfigPin:
			status = RunScenario(&configParameters);
			if (status != ERROR_SUCCESS)
			{
				wprintf(L"\nERROR: PC Configuration with a pin Failed\n\n");
			}
			else
			{
				wprintf(L"\nINFO: PC Configuration with a pin Succeeded\n\n");
			}
			break;

		default:
			break;
	}


cleanup:

	if(configParameters.bFreePassphrase)
	{
		free(configParameters.pProfilePassphrase);
	}
	
	if (fUnintializeCom)
	{
		CoUninitialize();
	}

	return 0;
}


