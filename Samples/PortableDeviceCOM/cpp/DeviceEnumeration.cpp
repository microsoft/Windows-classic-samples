// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

//<SnippetDeviceEnum6>
#define CLIENT_NAME         L"WPD Sample Application"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    0
#define CLIENT_REVISION     2
//</SnippetDeviceEnum6>
// Reads and displays the device friendly name for the specified PnPDeviceID string
void DisplayFriendlyName(
    _In_ IPortableDeviceManager* deviceManager,
    _In_ PCWSTR                  pnpDeviceID)
{
    DWORD friendlyNameLength = 0;

    // 1) Pass nullptr as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
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
    DWORD manufacturerLength = 0;

    // 1) Pass nullptr as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
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
    DWORD descriptionLength = 0;

    // 1) Pass nullptr as the PWSTR return string parameter to get the total number
    // of characters to allocate for the string value.
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

// Enumerates all Windows Portable Devices, displays the friendly name,
// manufacturer, and description of each device.  This function also
// returns the total number of devices found.
DWORD EnumerateAllDevices()
{
    DWORD                           pnpDeviceIDCount    = 0;
    ComPtr<IPortableDeviceManager>  deviceManager;

    // CoCreate the IPortableDeviceManager interface to enumerate
    // portable devices and to get information about them.
    //<SnippetDeviceEnum1>
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&deviceManager));
    if (FAILED(hr))
    {
        wprintf(L"! Failed to CoCreateInstance CLSID_PortableDeviceManager, hr = 0x%lx\n", hr);
    }
    //</SnippetDeviceEnum1>

    // 1) Pass nullptr as the PWSTR array pointer to get the total number
    // of devices found on the system.
    //<SnippetDeviceEnum2>
    if (SUCCEEDED(hr))
    {
        hr = deviceManager->GetDevices(nullptr, &pnpDeviceIDCount);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get number of devices on the system, hr = 0x%lx\n", hr);
        }
    }

    // Report the number of devices found.  NOTE: we will report 0, if an error
    // occured.

    wprintf(L"\n%u Windows Portable Device(s) found on the system\n\n", pnpDeviceIDCount);
    //</SnippetDeviceEnum2>
    // 2) Allocate an array to hold the PnPDeviceID strings returned from
    // the IPortableDeviceManager::GetDevices method
    //<SnippetDeviceEnum3>
    if (SUCCEEDED(hr) && (pnpDeviceIDCount > 0))
    {
        PWSTR* pnpDeviceIDs = new (std::nothrow) PWSTR[pnpDeviceIDCount];
        if (pnpDeviceIDs != nullptr)
        {
            ZeroMemory(pnpDeviceIDs, pnpDeviceIDCount * sizeof(PWSTR));
            DWORD retrievedDeviceIDCount = pnpDeviceIDCount;
            hr = deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
            if (SUCCEEDED(hr))
            {
                _Analysis_assume_(retrievedDeviceIDCount <= pnpDeviceIDCount);
                // For each device found, display the devices friendly name,
                // manufacturer, and description strings.
                for (DWORD index = 0; index < retrievedDeviceIDCount; index++)
                {
                    wprintf(L"[%u] ", index);
                    DisplayFriendlyName(deviceManager.Get(), pnpDeviceIDs[index]);
                    wprintf(L"    ");
                    DisplayManufacturer(deviceManager.Get(), pnpDeviceIDs[index]);
                    wprintf(L"    ");
                    DisplayDescription(deviceManager.Get(), pnpDeviceIDs[index]);
                }
            }
            else
            {
                wprintf(L"! Failed to get the device list from the system, hr = 0x%lx\n", hr);
            }
            //</SnippetDeviceEnum3>

            // Free all returned PnPDeviceID strings by using CoTaskMemFree.
            // NOTE: CoTaskMemFree can handle nullptr pointers, so no nullptr
            //       check is needed.
            for (DWORD index = 0; index < pnpDeviceIDCount; index ++)
            {
                CoTaskMemFree(pnpDeviceIDs[index]);
                pnpDeviceIDs[index] = nullptr;
            }

            // Delete the array of PWSTR pointers
            delete [] pnpDeviceIDs;
            pnpDeviceIDs = nullptr;
        }
        else
        {
            wprintf(L"! Failed to allocate memory for PWSTR array\n");
        }
    }

    return pnpDeviceIDCount;
}

// Creates and populates an IPortableDeviceValues with information about
// this application.  The IPortableDeviceValues is used as a parameter
// when calling the IPortableDevice::Open() method.
void GetClientInformation(
    _Outptr_result_maybenull_ IPortableDeviceValues** clientInformation)
{
    // Client information is optional.  The client can choose to identify itself, or
    // to remain unknown to the driver.  It is beneficial to identify yourself because
    // drivers may be able to optimize their behavior for known clients. (e.g. An
    // IHV may want their bundled driver to perform differently when connected to their
    // bundled software.)

    // CoCreate an IPortableDeviceValues interface to hold the client information.
    //<SnippetDeviceEnum7>
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(clientInformation));
    //</SnippetDeviceEnum7>
    //<SnippetDeviceEnum8>
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
    //</SnippetDeviceEnum8>
}

// Calls EnumerateDevices() function to display devices on the system
// and to obtain the total number of devices found.  If 1 or more devices
// are found, this function prompts the user to choose a device using
// a zero-based index.
void ChooseDevice(
    _Outptr_result_maybenull_ IPortableDevice** device)
{
    *device = nullptr;

    HRESULT                        hr                  = S_OK;
    UINT                           currentDeviceIndex  = 0;
    DWORD                          pnpDeviceIDCount    = 0;
    PWSTR*                         pnpDeviceIDs        = nullptr;
    WCHAR                          selection[SELECTION_BUFFER_SIZE] = {0};

    ComPtr<IPortableDeviceManager> deviceManager;
    ComPtr<IPortableDeviceValues>  clientInformation;

    // Fill out information about your application, so the device knows
    // who they are speaking to.

    GetClientInformation(&clientInformation);

    // Enumerate and display all devices.
    pnpDeviceIDCount = EnumerateAllDevices();

    if (pnpDeviceIDCount > 0)
    {
        // Prompt user to enter an index for the device they want to choose.
        wprintf(L"Enter the index of the device you wish to use.\n>");
        hr = StringCchGetsW(selection, ARRAYSIZE(selection));
        if (SUCCEEDED(hr))
        {
            currentDeviceIndex = static_cast<UINT>(_wtoi(selection));
            if (currentDeviceIndex >= pnpDeviceIDCount)
            {
                wprintf(L"An invalid device index was specified, defaulting to the first device in the list.\n");
                currentDeviceIndex = 0;
            }
        }
        else
        {
            wprintf(L"An invalid device index was specified, defaulting to the first device in the list.\n");
            currentDeviceIndex = 0;
        }

        // CoCreate the IPortableDeviceManager interface to enumerate
        // portable devices and to get information about them.
        hr = CoCreateInstance(CLSID_PortableDeviceManager,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&deviceManager));
        if (FAILED(hr))
        {
            wprintf(L"! Failed to CoCreateInstance CLSID_PortableDeviceManager, hr = 0x%lx\n", hr);
        }

        // Allocate an array to hold the PnPDeviceID strings returned from
        // the IPortableDeviceManager::GetDevices method
        if (SUCCEEDED(hr) && (pnpDeviceIDCount > 0))
        {
            pnpDeviceIDs = new (std::nothrow)PWSTR[pnpDeviceIDCount];
            if (pnpDeviceIDs != nullptr)
            {
                ZeroMemory(pnpDeviceIDs, pnpDeviceIDCount * sizeof(PWSTR));
                DWORD retrievedDeviceIDCount = pnpDeviceIDCount;
                hr = deviceManager->GetDevices(pnpDeviceIDs, &retrievedDeviceIDCount);
                if (SUCCEEDED(hr))
                {
                    _Analysis_assume_(retrievedDeviceIDCount <= pnpDeviceIDCount);
                    //<SnippetDeviceEnum5>
                    // CoCreate the IPortableDevice interface and call Open() with
                    // the chosen PnPDeviceID string.
                    hr = CoCreateInstance(CLSID_PortableDeviceFTM,
                                          nullptr,
                                          CLSCTX_INPROC_SERVER,
                                          IID_PPV_ARGS(device));
                    if (SUCCEEDED(hr))
                    {
                        hr = (*device)->Open(pnpDeviceIDs[currentDeviceIndex], clientInformation.Get());

                        if (hr == E_ACCESSDENIED)
                        {
                            wprintf(L"Failed to Open the device for Read Write access, will open it for Read-only access instead\n");
                            clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
                            hr = (*device)->Open(pnpDeviceIDs[currentDeviceIndex], clientInformation.Get());
                        }

                        if (FAILED(hr))
                        {
                            wprintf(L"! Failed to Open the device, hr = 0x%lx\n", hr);
                            // Release the IPortableDevice interface, because we cannot proceed
                            // with an unopen device.
                            (*device)->Release();
                            *device = nullptr;
                        }
                    }
                    else
                    {
                        wprintf(L"! Failed to CoCreateInstance CLSID_PortableDeviceFTM, hr = 0x%lx\n", hr);
                    }
                    //</SnippetDeviceEnum5>
                }
                else
                {
                    wprintf(L"! Failed to get the device list from the system, hr = 0x%lx\n", hr);
                }

                // Free all returned PnPDeviceID strings by using CoTaskMemFree.
                // NOTE: CoTaskMemFree can handle nullptr pointers, so no nullptr
                //       check is needed.
                //<SnippetDeviceEnum4>
                for (DWORD index = 0; index < pnpDeviceIDCount; index ++)
                {
                    CoTaskMemFree(pnpDeviceIDs[index]);
                    pnpDeviceIDs[index] = nullptr;
                }

                // Delete the array of PWSTR pointers
                delete [] pnpDeviceIDs;
                pnpDeviceIDs = nullptr;
                //</SnippetDeviceEnum4>
            }
            else
            {
                wprintf(L"! Failed to allocate memory for PWSTR array\n");
            }
        }
    }

    // If no devices were found on the system, just exit this function.
}
