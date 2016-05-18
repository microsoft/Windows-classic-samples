// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <strsafe.h>

#define CLIENT_NAME         L"WPD Services Sample Application"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     0

// Creates and populates an IPortableDeviceValues with information about
// this application.  The IPortableDeviceValues is used as a parameter
// when calling the IPortableDeviceService::Open() method.
void GetClientInformation(
    IPortableDeviceValues** ppClientInformation)
{
    // Client information is optional.  The client can choose to identify itself, or
    // to remain unknown to the driver.  It is beneficial to identify yourself because
    // drivers may be able to optimize their behavior for known clients. (e.g. An
    // IHV may want their bundled driver to perform differently when connected to their
    // bundled software.)

    HRESULT hr = S_OK;

    if (ppClientInformation == NULL)
    {
        printf("! A NULL IPortableDeviceValues interface pointer was received\n");
        return;
    }

    *ppClientInformation = NULL;

    // CoCreate an IPortableDeviceValues interface to hold the client information.
    hr = CoCreateInstance(CLSID_PortableDeviceValues,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(ppClientInformation));
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
}

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


// Reads and displays information about the device
void DisplayDeviceInformation(
    IPortableDeviceServiceManager*  pServiceManager,
    PCWSTR                          pPnpServiceID)
{
    PWSTR                           pPnpDeviceID    = NULL;
    CComPtr<IPortableDeviceManager> pIPortableDeviceManager;

    // Retrieve the PnP identifier of the device that contains this service
    HRESULT hr = pServiceManager->GetDeviceForService(pPnpServiceID, &pPnpDeviceID);
    if (SUCCEEDED(hr))
    {
        // Retrieve the IPortableDeviceManager interface from IPortableDeviceServiceManager
        hr = pServiceManager->QueryInterface(IID_PPV_ARGS(&pIPortableDeviceManager));
        if (SUCCEEDED(hr))
        {        
            DisplayFriendlyName(pIPortableDeviceManager, pPnpDeviceID);
            printf("    ");
            DisplayManufacturer(pIPortableDeviceManager, pPnpDeviceID);
            printf("    ");
            DisplayDescription(pIPortableDeviceManager, pPnpDeviceID);
            printf("\n");
        }
        else
        {
            printf("! Failed to QueryInterface IID_IPortableDeviceManager, hr = 0x%lx\n",hr);
        }
    }
    else
    {
        printf("! Failed to get the device PnP identifier, hr = 0x%lx\n",hr);
    }
    
    CoTaskMemFree(pPnpDeviceID);
}


// Enumerates all Contacts Services, displaying the associated device of each service.
void EumerateContactsServices(CAtlArray<PWSTR>& ContactsServicePnpIDs)
{
    HRESULT                                 hr              = S_OK;
    DWORD                                   cPnpDeviceIDs   = 0;
    PWSTR*                                  pPnpDeviceIDs   = NULL;
    CComPtr<IPortableDeviceManager>         pPortableDeviceManager;
    CComPtr<IPortableDeviceServiceManager>  pServiceManager;

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

    // Retrieve the IPortableDeviceServiceManager interface to enumerate device services.
    if (SUCCEEDED(hr))
    {
        hr = pPortableDeviceManager->QueryInterface(IID_PPV_ARGS(&pServiceManager));
        if (FAILED(hr))
        {
            printf("! Failed to QueryInterface IID_IPortableDeviceServiceManager, hr = 0x%lx\n",hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // First, pass NULL as the PWSTR array pointer to get the total number
        // of devices found on the system.
        hr = pPortableDeviceManager->GetDevices(NULL, &cPnpDeviceIDs);

        if (FAILED(hr))
        {
            printf("! Failed to get number of devices on the system, hr = 0x%lx\n",hr);
        }

        if (SUCCEEDED(hr) && (cPnpDeviceIDs > 0))
        {
            // Second, allocate an array to hold the PnPDeviceID strings returned from
            // the IPortableDeviceManager::GetDevices method
            pPnpDeviceIDs = new (std::nothrow) PWSTR[cPnpDeviceIDs];

            if (pPnpDeviceIDs != NULL)
            {
                DWORD dwIndex = 0;

                hr = pPortableDeviceManager->GetDevices(pPnpDeviceIDs, &cPnpDeviceIDs);
                if (SUCCEEDED(hr))
                {   
					//<SnippetOpenService1>
                    // For each device found, find the contacts service
                    for (dwIndex = 0; dwIndex < cPnpDeviceIDs; dwIndex++)
                    {
                        DWORD   cPnpServiceIDs = 0;
                        PWSTR   pPnpServiceID  = NULL;

                        // First, pass NULL as the PWSTR array pointer to get the total number
                        // of contacts services (SERVICE_Contacts) found on the device.
                        // To find the total number of all services on the device, use GUID_DEVINTERFACE_WPD_SERVICE.
                        hr = pServiceManager->GetDeviceServices(pPnpDeviceIDs[dwIndex], SERVICE_Contacts, NULL, &cPnpServiceIDs);
                        
                        if (SUCCEEDED(hr) && (cPnpServiceIDs > 0))
                        {                               
                            // For simplicity, we are only using the first contacts service on each device
                            cPnpServiceIDs = 1;
                            hr = pServiceManager->GetDeviceServices(pPnpDeviceIDs[dwIndex], SERVICE_Contacts, &pPnpServiceID, &cPnpServiceIDs);

                            if (SUCCEEDED(hr))
                            {
                                // We've found the service, display it and save its PnP Identifier
                                ContactsServicePnpIDs.Add(pPnpServiceID);

                                printf("[%d] ", static_cast<DWORD>(ContactsServicePnpIDs.GetCount()-1));

                                // Display information about the device that contains this service.
                                DisplayDeviceInformation(pServiceManager, pPnpServiceID);

                                // ContactsServicePnpIDs now owns the memory for this string
                                pPnpServiceID = NULL;
                            }
                            else
                            {
                                printf("! Failed to get the first contacts service from '%ws, hr = 0x%lx\n",pPnpDeviceIDs[dwIndex],hr);
                            }
                        }
                    }
					//</SnippetOpenService1>
                }

                // Free all returned PnPDeviceID strings
                FreePortableDevicePnPIDs(pPnpDeviceIDs, cPnpDeviceIDs);

                // Delete the array of PWSTR pointers
                delete [] pPnpDeviceIDs;
                pPnpDeviceIDs = NULL;

            }
            else
            {
                printf("! Failed to allocate memory for PWSTR array\n");
            }            
        }
    }
    
    printf("\n%d Contacts Device Service(s) found on the system\n\n", static_cast<DWORD>(ContactsServicePnpIDs.GetCount()));
}


// Frees the service PnP IDs allocated by EumerateContactsServices
void FreeServicePnPIDs(CAtlArray<PWSTR>& ServicePnpIDs)
{
    size_t cServicePnPIDs = ServicePnpIDs.GetCount();

    for (size_t i=0; i<cServicePnPIDs; i++)
    {
        CoTaskMemFree(ServicePnpIDs[i]);        
    }
    ServicePnpIDs.RemoveAll();
}


// Calls EnumerateDeviceServices() function to display device services on the system
// and to obtain the total number of device services found.  If 1 or more device services
// are found, this function prompts the user to choose a device service using
// a zero-based index.
void ChooseDeviceService(IPortableDeviceService** ppService)
{
    HRESULT                         hr                  = S_OK;
    UINT                            uiCurrentService    = 0;
    CHAR                            szSelection[81]     = {0};

    CComPtr<IPortableDeviceValues>  pClientInformation;
    CAtlArray<PWSTR>                ContactsServicesArray;

    if (ppService == NULL)
    {
        printf("! A NULL IPortableDeviceService interface pointer was received\n");
        return;
    }

    if (*ppService != NULL)
    {
        // To avoid operating on potiential bad pointers, reject any non-null
        // IPortableDeviceService interface pointers.  This will force the caller
        // to properly release the interface before obtaining a new one.
        printf("! A non-NULL IPortableDeviceService interface pointer was received, please release this interface before obtaining a new one.\n");
        return;
    }

    // Fill out information about your application, so the device knows
    // who they are speaking to.

    GetClientInformation(&pClientInformation);

    // Enumerate and display all Contacts services
    EumerateContactsServices(ContactsServicesArray);

    DWORD cServicePnPIDs = static_cast<DWORD>(ContactsServicesArray.GetCount());
    
    if (cServicePnPIDs > 0)
    {
        CComPtr<IPortableDeviceService> pService;

        // Prompt user to enter an index for the device they want to choose.
        printf("Enter the index of the Contacts device service you wish to use.\n>");
        hr = StringCbGetsA(szSelection,sizeof(szSelection));
        if (SUCCEEDED(hr))
        {
            uiCurrentService = (UINT) atoi(szSelection);
            if (uiCurrentService >= cServicePnPIDs)
            {
                printf("An invalid Contacts device service index was specified, defaulting to the first in the list.\n");
                uiCurrentService = 0;
            }
        }
        else
        {
            printf("An invalid Contacts device service index was specified, defaulting to the first in the list.\n");
            uiCurrentService = 0;
        }

        // CoCreate the IPortableDeviceService interface and call Open() with
        // the chosen PnPServiceID string.
		//<SnippetOpenService2>
        hr = CoCreateInstance(CLSID_PortableDeviceServiceFTM,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pService));
        if (SUCCEEDED(hr))
        {
            hr = pService->Open(ContactsServicesArray[uiCurrentService], pClientInformation);
            if (FAILED(hr))
            {
                if (hr == E_ACCESSDENIED)
                {
                    printf("Failed to Open the service for Read Write access, will open it for Read-only access instead\n");

                    pClientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);

                    hr = pService->Open(ContactsServicesArray[uiCurrentService], pClientInformation);

                    if (FAILED(hr))
                    {
                        printf("! Failed to Open the service for Read access, hr = 0x%lx\n",hr);
                    }
                }
                else
                {
                    printf("! Failed to Open the service, hr = 0x%lx\n",hr);
                }
            }
			//</SnippetOpenService2>
            if (SUCCEEDED(hr))
            {
                printf("Successfully opened the selected Contacts service\n");

                // Successfully opened a service, set the result
                *ppService = pService.Detach();
            }
            
        }
        else
        {
            printf("! Failed to CoCreateInstance CLSID_PortableDeviceServiceFTM, hr = 0x%lx\n",hr);
        }

        // Release the service PnP IDs allocated by EumerateContactsServices
        FreeServicePnPIDs(ContactsServicesArray);
    }

    // If no devices with the Contacts service were found on the system, just exit this function.
}
