// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

#define CLIENT_NAME         L"WPD Services Sample Application"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     0

// Creates and populates an IPortableDeviceValues with information about
// this application.  The IPortableDeviceValues is used as a parameter
// when calling the IPortableDeviceService::Open() method.
void GetClientInformation(
    _Outptr_result_maybenull_ IPortableDeviceValues** clientInformation)
{
    // Client information is optional.  The client can choose to identify itself, or
    // to remain unknown to the driver.  It is beneficial to identify yourself because
    // drivers may be able to optimize their behavior for known clients. (e.g. An
    // IHV may want their bundled driver to perform differently when connected to their
    // bundled software.)
    // CoCreate an IPortableDeviceValues interface to hold the client information.
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(clientInformation));
    if (SUCCEEDED(hr))
    {
        // Attempt to set all bits of client information
        hr = (*clientInformation)->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_CLIENT_NAME, hr = 0x%lx\n", hr);
        }

        hr = (*clientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_CLIENT_MAJOR_VERSION, hr = 0x%lx\n", hr);
        }

        hr = (*clientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_CLIENT_MINOR_VERSION, hr = 0x%lx\n", hr);
        }

        hr = (*clientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_CLIENT_REVISION, hr = 0x%lx\n", hr);
        }

        //  Some device drivers need to impersonate the caller in order to function correctly.  Since our application does not
        //  need to restrict its identity, specify SECURITY_IMPERSONATION so that we work with all devices.
        hr = (*clientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, hr = 0x%lx\n", hr);
        }
    }
    else
    {
        wprintf(L"! Failed to CoCreateInstance CLSID_PortableDeviceValues, hr = 0x%lx\n", hr);
    }
}

// Reads and displays the device friendly name for the specified PnPDeviceID string
void DisplayFriendlyName(
    _In_ IPortableDeviceManager* deviceManager,
    _In_ PCWSTR                  pnpDeviceID)
{
    // 1) Pass nullptr as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    DWORD   friendlyNameLength = 0;
    HRESULT hr = deviceManager->GetDeviceFriendlyName(pnpDeviceID, nullptr, &friendlyNameLength);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get number of characters for device friendly name, hr = 0x%lx\n", hr);
    }
    else if (friendlyNameLength > 0)
    {
        // 2) Allocate the number of characters needed and retrieve the string value.
        PWSTR friendlyName = new (std::nothrow) WCHAR[friendlyNameLength];
        if (friendlyName != nullptr)
        {
            ZeroMemory(friendlyName, friendlyNameLength * sizeof(WCHAR));
            hr = deviceManager->GetDeviceFriendlyName(pnpDeviceID, friendlyName, &friendlyNameLength);
            if (SUCCEEDED(hr))
            {
                wprintf(L"Friendly Name: %ws\n", friendlyName);
            }
            else
            {
                wprintf(L"! Failed to get device friendly name, hr = 0x%lx\n", hr);
            }

            // Delete the allocated friendly name string
            delete [] friendlyName;
            friendlyName = nullptr;
        }
        else
        {
            wprintf(L"! Failed to allocate memory for the device friendly name string\n");
        }
    }
    else
    {
        wprintf(L"The device did not provide a friendly name.\n");
    }
}

// Reads and displays the device manufacturer for the specified PnPDeviceID string
void DisplayManufacturer(
    _In_ IPortableDeviceManager* deviceManager,
    _In_ PCWSTR                  pnpDeviceID)
{
    // 1) Pass nullptr as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    DWORD   manufacturerLength = 0;
    HRESULT hr = deviceManager->GetDeviceManufacturer(pnpDeviceID, nullptr, &manufacturerLength);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get number of characters for device manufacturer, hr = 0x%lx\n", hr);
    }
    else if (manufacturerLength > 0)
    {
        // 2) Allocate the number of characters needed and retrieve the string value.
        PWSTR manufacturer = new (std::nothrow) WCHAR[manufacturerLength];
        if (manufacturer != nullptr)
        {
            ZeroMemory(manufacturer, manufacturerLength * sizeof(WCHAR));
            hr = deviceManager->GetDeviceManufacturer(pnpDeviceID, manufacturer, &manufacturerLength);
            if (SUCCEEDED(hr))
            {
                wprintf(L"Manufacturer:  %ws\n", manufacturer);
            }
            else
            {
                wprintf(L"! Failed to get device manufacturer, hr = 0x%lx\n", hr);
            }

            // Delete the allocated manufacturer string
            delete [] manufacturer;
            manufacturer = nullptr;
        }
        else
        {
            wprintf(L"! Failed to allocate memory for the device manufacturer string\n");
        }
    }
    else
    {
        wprintf(L"The device did not provide a manufacturer.\n");
    }
}

// Reads and displays the device discription for the specified PnPDeviceID string
void DisplayDescription(
    _In_ IPortableDeviceManager* deviceManager,
    _In_ PCWSTR                  pnpDeviceID)
{
    // 1) Pass nullptr as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
    DWORD   descriptionLength = 0;
    HRESULT hr = deviceManager->GetDeviceDescription(pnpDeviceID, nullptr, &descriptionLength);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to get number of characters for device description, hr = 0x%lx\n", hr);
    }
    else if (descriptionLength > 0)
    {
        // 2) Allocate the number of characters needed and retrieve the string value.
        PWSTR description = new (std::nothrow) WCHAR[descriptionLength];
        if (description != nullptr)
        {
            ZeroMemory(description, descriptionLength * sizeof(WCHAR));
            hr = deviceManager->GetDeviceDescription(pnpDeviceID, description, &descriptionLength);
            if (SUCCEEDED(hr))
            {
                wprintf(L"Description:   %ws\n", description);
            }
            else
            {
                wprintf(L"! Failed to get device description, hr = 0x%lx\n", hr);
            }

            // Delete the allocated description string
            delete [] description;
            description = nullptr;
        }
        else
        {
            wprintf(L"! Failed to allocate memory for the device description string\n");
        }
    }
    else
    {
        wprintf(L"The device did not provide a description.\n");
    }
}

// Reads and displays information about the device
void DisplayDeviceInformation(
    _In_ IPortableDeviceServiceManager*  serviceManager,
    _In_ PCWSTR                          pnpServiceID)
{
    // Retrieve the PnP identifier of the device that contains this service
    PWSTR   pnpDeviceID = nullptr;
    HRESULT hr = serviceManager->GetDeviceForService(pnpServiceID, &pnpDeviceID);
    if (SUCCEEDED(hr))
    {
        // Retrieve the IPortableDeviceManager interface from IPortableDeviceServiceManager
        ComPtr<IPortableDeviceManager> deviceManager;
        hr = serviceManager->QueryInterface(IID_PPV_ARGS(&deviceManager));
        if (SUCCEEDED(hr))
        {
            DisplayFriendlyName(deviceManager.Get(), pnpDeviceID);
            wprintf(L"    ");
            DisplayManufacturer(deviceManager.Get(), pnpDeviceID);
            wprintf(L"    ");
            DisplayDescription(deviceManager.Get(), pnpDeviceID);
            wprintf(L"\n");
        }
        else
        {
            wprintf(L"! Failed to QueryInterface IID_IPortableDeviceManager, hr = 0x%lx\n", hr);
        }
    }
    else
    {
        wprintf(L"! Failed to get the device PnP identifier, hr = 0x%lx\n", hr);
    }
    
    CoTaskMemFree(pnpDeviceID);
    pnpDeviceID = nullptr;
}

//<SnippetContactServiceEnumeration1>
// Enumerates all Contacts Services, displaying the associated device of each service.
void EumerateContactsServices(
    _Out_ vector<PWSTR>& contactsServices)
{
    contactsServices.clear();

    ComPtr<IPortableDeviceManager>          deviceManager;
    ComPtr<IPortableDeviceServiceManager>   serviceManager;

    // CoCreate the IPortableDeviceManager interface to enumerate
    // portable devices and to get information about them.
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager,
                          nullptr,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&deviceManager));

    if (FAILED(hr))
    {
        wprintf(L"! Failed to CoCreateInstance CLSID_PortableDeviceManager, hr = 0x%lx\n", hr); 
    }
    else
    {
        // Retrieve the IPortableDeviceServiceManager interface to enumerate device services.
        hr = deviceManager.As(&serviceManager);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to QueryInterface IID_IPortableDeviceServiceManager, hr = 0x%lx\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // 1) Pass nullptr as the PWSTR array pointer to get the total number
        // of devices found on the system.
        DWORD pnpDeviceIDCount = 0;
        hr = deviceManager->GetDevices(nullptr, &pnpDeviceIDCount);

        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of devices on the system, hr = 0x%lx\n", hr);
        }
        else if (SUCCEEDED(hr) && (pnpDeviceIDCount > 0))
        {
            // 2) Allocate an array to hold the PnPDeviceID strings returned from
            // the IPortableDeviceManager::GetDevices method
            PWSTR* pnpDeviceIDs = new (std::nothrow) PWSTR[pnpDeviceIDCount];
            if (pnpDeviceIDs != nullptr)
            {
                DWORD retrievedDeviceIDCount = pnpDeviceIDCount;
                ZeroMemory(pnpDeviceIDs, pnpDeviceIDCount * sizeof(PWSTR));
                hr = deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
                if (SUCCEEDED(hr))
                {
                    _Analysis_assume_(retrievedDeviceIDCount <= pnpDeviceIDCount);
                    //<SnippetOpenService1>
                    // For each device found, find the contacts service
                    for (DWORD index = 0; index < retrievedDeviceIDCount; index++)
                    {
                        // Pass nullptr as the PWSTR array pointer to get the total number
                        // of contacts services (SERVICE_Contacts) found on the device.
                        // To find the total number of all services on the device, use GUID_DEVINTERFACE_WPD_SERVICE.
                        DWORD pnpServiceIDCount = 0;
                        hr = serviceManager->GetDeviceServices(pnpDeviceIDs[index], SERVICE_Contacts, nullptr, &pnpServiceIDCount);
                        if (SUCCEEDED(hr) && (pnpServiceIDCount > 0))
                        {
                            // For simplicity, we are only using the first contacts service on each device
                            PWSTR pnpServiceID = nullptr;
                            pnpServiceIDCount = 1;
                            hr = serviceManager->GetDeviceServices(pnpDeviceIDs[index], SERVICE_Contacts, &pnpServiceID, &pnpServiceIDCount);
                            if (SUCCEEDED(hr) && (pnpServiceID != nullptr))
                            {
                                // We've found the service, display it and save its PnP Identifier
                                contactsServices.push_back(pnpServiceID);

                                wprintf(L"[%u] ", static_cast<DWORD>(contactsServices.size()-1));

                                // Display information about the device that contains this service.
                                DisplayDeviceInformation(serviceManager.Get(), pnpServiceID);

                                // contactsServices now owns the memory for this string
                                pnpServiceID = nullptr;
                            }
                            else
                            {
                                wprintf(L"! Failed to get the first contacts service from '%ws, hr = 0x%lx\n", pnpDeviceIDs[index], hr);
                            }
                        }
                    }
                    //</SnippetOpenService1>
                }

                // Free all returned PnPDeviceID strings
                FreePortableDevicePnPIDs(pnpDeviceIDs, pnpDeviceIDCount);

                // Delete the array of PWSTR pointers
                delete [] pnpDeviceIDs;
                pnpDeviceIDs = nullptr;
            }
            else
            {
                wprintf(L"! Failed to allocate memory for PWSTR array\n");
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        wprintf(L"\n%u Contacts Device Service(s) found on the system\n\n", static_cast<DWORD>(contactsServices.size()));
    }
}
//</SnippetContactServiceEnumeration1>

// Frees the PnP service IDs allocated by EumerateContactsServices
void FreeServicePnPIDs(
    _In_ vector<PWSTR>& pnpServiceIDs)
{
    while (!pnpServiceIDs.empty())
    {
        CoTaskMemFree(pnpServiceIDs.back());
        pnpServiceIDs.pop_back();
    }
}


// Calls EnumerateDeviceServices() function to display device services on the system
// and to obtain the total number of device services found.  If 1 or more device services
// are found, this function prompts the user to choose a device service using
// a zero-based index.
void ChooseDeviceService(
    _Outptr_result_maybenull_ IPortableDeviceService** service)
{
    *service = nullptr;

    HRESULT                         hr                   = S_OK;
    UINT                            selectedServiceIndex = 0;
    WCHAR                           selection[SELECTION_BUFFER_SIZE] = {0};

    ComPtr<IPortableDeviceValues>   clientInformation;
    vector<PWSTR>                   contactsServices;

    // Fill out information about your application, so the device knows
    // who they are speaking to.
    GetClientInformation(&clientInformation);

    // Enumerate and display all Contacts services
    EumerateContactsServices(contactsServices);

    DWORD pnpServiceIDCount = static_cast<DWORD>(contactsServices.size());
    if (pnpServiceIDCount > 0)
    {
        ComPtr<IPortableDeviceService> serviceTemp;

        // Prompt user to enter an index for the device they want to choose.
        wprintf(L"Enter the index of the Contacts device service you wish to use.\n>");
        hr = StringCchGetsW(selection, ARRAYSIZE(selection));
        if (SUCCEEDED(hr))
        {
            selectedServiceIndex = static_cast<UINT>(_wtoi(selection));
            if (selectedServiceIndex >= pnpServiceIDCount)
            {
                wprintf(L"An invalid Contacts device service index was specified, defaulting to the first in the list.\n");
                selectedServiceIndex = 0;
            }
        }
        else
        {
            wprintf(L"An invalid Contacts device service index was specified, defaulting to the first in the list.\n");
            selectedServiceIndex = 0;
        }

        // CoCreate the IPortableDeviceService interface and call Open() with
        // the chosen PnPServiceID string.
        //<SnippetOpenService2>
        hr = CoCreateInstance(CLSID_PortableDeviceServiceFTM,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&serviceTemp));
        if (SUCCEEDED(hr))
        {
            hr = serviceTemp->Open(contactsServices[selectedServiceIndex], clientInformation.Get());
            if (FAILED(hr))
            {
                if (hr == E_ACCESSDENIED)
                {
                    wprintf(L"Failed to Open the service for Read Write access, will open it for Read-only access instead\n");

                    clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);

                    hr = serviceTemp->Open(contactsServices[selectedServiceIndex], clientInformation.Get());

                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to Open the service for Read access, hr = 0x%lx\n", hr);
                    }
                }
                else
                {
                    wprintf(L"! Failed to Open the service, hr = 0x%lx\n", hr);
                }
            }
            //</SnippetOpenService2>
            if (SUCCEEDED(hr))
            {
                wprintf(L"Successfully opened the selected Contacts service\n");

                // Successfully opened a service, set the result
                *service = serviceTemp.Detach();
            }
            
        }
        else
        {
            wprintf(L"! Failed to CoCreateInstance CLSID_PortableDeviceServiceFTM, hr = 0x%lx\n", hr);
        }

        // Release the PnP IDs allocated by EnumerateContactsServices
        FreeServicePnPIDs(contactsServices);
    }

    // If no devices with the Contacts service were found on the system, just exit this function.
}
