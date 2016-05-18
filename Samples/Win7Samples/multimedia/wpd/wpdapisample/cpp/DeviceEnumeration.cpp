// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <strsafe.h>
//<SnippetDeviceEnum6>
#define CLIENT_NAME         L"WPD Sample Application"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     2
//</SnippetDeviceEnum6>
// Reads and displays the device friendly name for the specified PnPDeviceID string
void DisplayFriendlyName(
    IPortableDeviceManager* pPortableDeviceManager,
    PCWSTR                  pPnPDeviceID)
{
    DWORD   cchFriendlyName = 0;
    PWSTR   pszFriendlyName = NULL;

    // First, pass NULL as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    HRESULT hr = pPortableDeviceManager->GetDeviceFriendlyName(pPnPDeviceID, NULL, &cchFriendlyName);
    if (FAILED(hr))
    {
        printf("! Failed to get number of characters for device friendly name, hr = 0x%lx\n",hr);
    }

    // Second allocate the number of characters needed and retrieve the string value.
    if ((hr == S_OK) && (cchFriendlyName > 0))
    {
        pszFriendlyName = new (std::nothrow) WCHAR[cchFriendlyName];
        if (pszFriendlyName != NULL)
        {
            hr = pPortableDeviceManager->GetDeviceFriendlyName(pPnPDeviceID, pszFriendlyName, &cchFriendlyName);
            if (SUCCEEDED(hr))
            {
                printf("Friendly Name: %ws\n", pszFriendlyName);
            }
            else
            {
                printf("! Failed to get device friendly name, hr = 0x%lx\n",hr);
            }

            // Delete the allocated friendly name string
            delete [] pszFriendlyName;
            pszFriendlyName = NULL;
        }
        else
        {
            printf("! Failed to allocate memory for the device friendly name string\n");
        }
    }

    if (SUCCEEDED(hr) && (cchFriendlyName == 0))
    {
        printf("The device did not provide a friendly name.\n");
    }
}

// Reads and displays the device manufacturer for the specified PnPDeviceID string
void DisplayManufacturer(
    IPortableDeviceManager* pPortableDeviceManager,
    PCWSTR                  pPnPDeviceID)
{
    DWORD   cchManufacturer = 0;
    PWSTR   pszManufacturer = NULL;

    // First, pass NULL as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    HRESULT hr = pPortableDeviceManager->GetDeviceManufacturer(pPnPDeviceID, NULL, &cchManufacturer);
    if (FAILED(hr))
    {
        printf("! Failed to get number of characters for device manufacturer, hr = 0x%lx\n",hr);
    }

    // Second allocate the number of characters needed and retrieve the string value.
    if ((hr == S_OK) && (cchManufacturer > 0))
    {
        pszManufacturer = new (std::nothrow) WCHAR[cchManufacturer];
        if (pszManufacturer != NULL)
        {
            hr = pPortableDeviceManager->GetDeviceManufacturer(pPnPDeviceID, pszManufacturer, &cchManufacturer);
            if (SUCCEEDED(hr))
            {
                printf("Manufacturer:  %ws\n", pszManufacturer);
            }
            else
            {
                printf("! Failed to get device manufacturer, hr = 0x%lx\n",hr);
            }

            // Delete the allocated manufacturer string
            delete [] pszManufacturer;
            pszManufacturer = NULL;
        }
        else
        {
            printf("! Failed to allocate memory for the device manufacturer string\n");
        }
    }

    if (SUCCEEDED(hr) && (cchManufacturer == 0))
    {
        printf("The device did not provide a manufacturer.\n");
    }
}

// Reads and displays the device discription for the specified PnPDeviceID string
void DisplayDescription(
    IPortableDeviceManager* pPortableDeviceManager,
    PCWSTR                  pPnPDeviceID)
{
    DWORD   cchDescription = 0;
    PWSTR   pszDescription = NULL;

    // First, pass NULL as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    HRESULT hr = pPortableDeviceManager->GetDeviceDescription(pPnPDeviceID, NULL, &cchDescription);
    if (FAILED(hr))
    {
        printf("! Failed to get number of characters for device description, hr = 0x%lx\n",hr);
    }

    // Second allocate the number of characters needed and retrieve the string value.
    if ((hr == S_OK) && (cchDescription > 0))
    {
        pszDescription = new (std::nothrow) WCHAR[cchDescription];
        if (pszDescription != NULL)
        {
            hr = pPortableDeviceManager->GetDeviceDescription(pPnPDeviceID, pszDescription, &cchDescription);
            if (SUCCEEDED(hr))
            {
                printf("Description:   %ws\n", pszDescription);
            }
            else
            {
                printf("! Failed to get device description, hr = 0x%lx\n",hr);
            }

            // Delete the allocated description string
            delete [] pszDescription;
            pszDescription = NULL;
        }
        else
        {
            printf("! Failed to allocate memory for the device description string\n");
        }
    }

    if (SUCCEEDED(hr) && (cchDescription == 0))
    {
        printf("The device did not provide a description.\n");
    }
}

// Enumerates all Windows Portable Devices, displays the friendly name,
// manufacturer, and description of each device.  This function also
// returns the total number of devices found.
DWORD EnumerateAllDevices()
{
    DWORD                           cPnPDeviceIDs = 0;
    PWSTR*                          pPnpDeviceIDs = NULL;
    CComPtr<IPortableDeviceManager> pPortableDeviceManager;

    // CoCreate the IPortableDeviceManager interface to enumerate
    // portable devices and to get information about them.
	//<SnippetDeviceEnum1>
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pPortableDeviceManager));
    if (FAILED(hr))
    {
        printf("! Failed to CoCreateInstance CLSID_PortableDeviceManager, hr = 0x%lx\n",hr);
    }
	//</SnippetDeviceEnum1>

    // First, pass NULL as the PWSTR array pointer to get the total number
    // of devices found on the system.
	//<SnippetDeviceEnum2>
    if (SUCCEEDED(hr))
    {
        hr = pPortableDeviceManager->GetDevices(NULL, &cPnPDeviceIDs);
        if (FAILED(hr))
        {
            printf("! Failed to get number of devices on the system, hr = 0x%lx\n",hr);
        }
    }

    // Report the number of devices found.  NOTE: we will report 0, if an error
    // occured.

    printf("\n%d Windows Portable Device(s) found on the system\n\n", cPnPDeviceIDs);
	//</SnippetDeviceEnum2>
    // Second, allocate an array to hold the PnPDeviceID strings returned from
    // the IPortableDeviceManager::GetDevices method
    if (SUCCEEDED(hr) && (cPnPDeviceIDs > 0))
    {
        pPnpDeviceIDs = new (std::nothrow) PWSTR[cPnPDeviceIDs];
        if (pPnpDeviceIDs != NULL)
        {
            DWORD dwIndex = 0;

            hr = pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &cPnPDeviceIDs);
            if (SUCCEEDED(hr))
            {
                // For each device found, display the devices friendly name,
                // manufacturer, and description strings.
                for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
                {
                    printf("[%d] ", dwIndex);
                    DisplayFriendlyName(pPortableDeviceManager, pPnpDeviceIDs[dwIndex]);
                    printf("    ");
                    DisplayManufacturer(pPortableDeviceManager, pPnpDeviceIDs[dwIndex]);
                    printf("    ");
                    DisplayDescription(pPortableDeviceManager, pPnpDeviceIDs[dwIndex]);
                }
            }
            else
            {
                printf("! Failed to get the device list from the system, hr = 0x%lx\n",hr);
            }

            // Free all returned PnPDeviceID strings by using CoTaskMemFree.
            // NOTE: CoTaskMemFree can handle NULL pointers, so no NULL
            //       check is needed.
			//<SnippetDeviceEnum3>
            for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
            {
                CoTaskMemFree(pPnpDeviceIDs[dwIndex]);
                pPnpDeviceIDs[dwIndex] = NULL;
            }

            // Delete the array of PWSTR pointers
            delete [] pPnpDeviceIDs;
            pPnpDeviceIDs = NULL;
			//</SnippetDeviceEnum3>
        }
        else
        {
            printf("! Failed to allocate memory for PWSTR array\n");
        }
    }

    return cPnPDeviceIDs;
}

// Creates and populates an IPortableDeviceValues with information about
// this application.  The IPortableDeviceValues is used as a parameter
// when calling the IPortableDevice::Open() method.
void GetClientInformation(
    IPortableDeviceValues** ppClientInformation)
{
    // Client information is optional.  The client can choose to identify itself, or
    // to remain unknown to the driver.  It is beneficial to identify yourself because
    // drivers may be able to optimize their behavior for known clients. (e.g. An
    // IHV may want their bundled driver to perform differently when connected to their
    // bundled software.)

    // CoCreate an IPortableDeviceValues interface to hold the client information.
	//<SnippetDeviceEnum7>
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(ppClientInformation));
	//</SnippetDeviceEnum7>
	//<SnippetDeviceEnum8>
    if (SUCCEEDED(hr))
    {
        // Attempt to set all bits of client information
        hr = (*ppClientInformation)->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_CLIENT_NAME, hr = 0x%lx\n",hr);
        }

        hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_CLIENT_MAJOR_VERSION, hr = 0x%lx\n",hr);
        }

        hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_CLIENT_MINOR_VERSION, hr = 0x%lx\n",hr);
        }

        hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_CLIENT_REVISION, hr = 0x%lx\n",hr);
        }

        //  Some device drivers need to impersonate the caller in order to function correctly.  Since our application does not
        //  need to restrict its identity, specify SECURITY_IMPERSONATION so that we work with all devices.
        hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, hr = 0x%lx\n",hr);
        }
    }
    else
    {
        printf("! Failed to CoCreateInstance CLSID_PortableDeviceValues, hr = 0x%lx\n",hr);
    }
	//</SnippetDeviceEnum8>
}

// Calls EnumerateDevices() function to display devices on the system
// and to obtain the total number of devices found.  If 1 or more devices
// are found, this function prompts the user to choose a device using
// a zero-based index.
void ChooseDevice(
    IPortableDevice** ppDevice)
{
    HRESULT                         hr              = S_OK;
    UINT                            uiCurrentDevice = 0;
    CHAR                            szSelection[81] = {0};
    DWORD                           cPnPDeviceIDs   = 0;
    PWSTR*                          pPnpDeviceIDs   = NULL;

    CComPtr<IPortableDeviceManager> pPortableDeviceManager;
    CComPtr<IPortableDeviceValues>  pClientInformation;

    if (ppDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    if (*ppDevice != NULL)
    {
        // To avoid operating on potiential bad pointers, reject any non-null
        // IPortableDevice interface pointers.  This will force the caller
        // to properly release the interface before obtaining a new one.
        printf("! A non-NULL IPortableDevice interface pointer was received, please release this interface before obtaining a new one.\n");
        return;
    }

    // Fill out information about your application, so the device knows
    // who they are speaking to.

    GetClientInformation(&pClientInformation);

    // Enumerate and display all devices.
    cPnPDeviceIDs = EnumerateAllDevices();

    if (cPnPDeviceIDs > 0)
    {
        // Prompt user to enter an index for the device they want to choose.
        printf("Enter the index of the device you wish to use.\n>");
        hr = StringCbGetsA(szSelection,sizeof(szSelection));
        if (SUCCEEDED(hr))
        {
            uiCurrentDevice = (UINT) atoi(szSelection);
            if (uiCurrentDevice >= cPnPDeviceIDs)
            {
                printf("An invalid device index was specified, defaulting to the first device in the list.\n");
                uiCurrentDevice = 0;
            }
        }
        else
        {
            printf("An invalid device index was specified, defaulting to the first device in the list.\n");
            uiCurrentDevice = 0;
        }

        // CoCreate the IPortableDeviceManager interface to enumerate
        // portable devices and to get information about them.
        hr = CoCreateInstance(CLSID_PortableDeviceManager,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pPortableDeviceManager));
        if (FAILED(hr))
        {
            printf("! Failed to CoCreateInstance CLSID_PortableDeviceManager, hr = 0x%lx\n",hr);
        }

        // Allocate an array to hold the PnPDeviceID strings returned from
        // the IPortableDeviceManager::GetDevices method
        if (SUCCEEDED(hr) && (cPnPDeviceIDs > 0))
        {
            pPnpDeviceIDs = new (std::nothrow)PWSTR[cPnPDeviceIDs];
            if (pPnpDeviceIDs != NULL)
            {
                DWORD dwIndex = 0;

                hr = pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &cPnPDeviceIDs);
                if (SUCCEEDED(hr))
                {
					//<SnippetDeviceEnum5>
                    // CoCreate the IPortableDevice interface and call Open() with
                    // the chosen PnPDeviceID string.
                    hr = CoCreateInstance(CLSID_PortableDeviceFTM,
                                          NULL,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(ppDevice));
                    if (SUCCEEDED(hr))
                    {
                        hr = (*ppDevice)->Open(pPnpDeviceIDs[uiCurrentDevice], pClientInformation);
                        if (FAILED(hr))
                        {
                            if (hr == E_ACCESSDENIED)
                            {
                                printf("Failed to Open the device for Read Write access, will open it for Read-only access instead\n");
                                pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
                                hr = (*ppDevice)->Open(pPnpDeviceIDs[uiCurrentDevice], pClientInformation);
                                if (FAILED(hr))
                                {
                                    printf("! Failed to Open the device, hr = 0x%lx\n",hr);
                                    // Release the IPortableDevice interface, because we cannot proceed
                                    // with an unopen device.
                                    (*ppDevice)->Release();
                                    *ppDevice = NULL;
                                }
                            }
                            else
                            {
                                printf("! Failed to Open the device, hr = 0x%lx\n",hr);
                                // Release the IPortableDevice interface, because we cannot proceed
                                // with an unopen device.
                                (*ppDevice)->Release();
                                *ppDevice = NULL;
                            }
                        }
                    }
                    else
                    {
                        printf("! Failed to CoCreateInstance CLSID_PortableDeviceFTM, hr = 0x%lx\n",hr);
                    }
					//</SnippetDeviceEnum5>
                }
                else
                {
                    printf("! Failed to get the device list from the system, hr = 0x%lx\n",hr);
                }

                // Free all returned PnPDeviceID strings by using CoTaskMemFree.
                // NOTE: CoTaskMemFree can handle NULL pointers, so no NULL
                //       check is needed.
				//<SnippetDeviceEnum4>
                for (dwIndex = 0; dwIndex < cPnPDeviceIDs; dwIndex++)
                {
                    CoTaskMemFree(pPnpDeviceIDs[dwIndex]);
                    pPnpDeviceIDs[dwIndex] = NULL;
                }

                // Delete the array of PWSTR pointers
                delete [] pPnpDeviceIDs;
                pPnpDeviceIDs = NULL;
				//</SnippetDeviceEnum4>
            }
            else
            {
                printf("! Failed to allocate memory for PWSTR array\n");
            }
        }

    }

    // If no devices were found on the system, just exit this function.
}
