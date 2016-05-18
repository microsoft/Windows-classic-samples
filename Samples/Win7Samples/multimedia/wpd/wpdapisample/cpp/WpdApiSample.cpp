// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Defines the entry point for the console application.
//

#include "stdafx.h"
#include <strsafe.h>

// Device enumeration
DWORD EnumerateAllDevices();
void ChooseDevice(IPortableDevice** ppDevice);

// Content enumeration
void EnumerateAllContent(IPortableDevice* pDevice);
void ReadHintLocations(IPortableDevice* pDevice);

// Content transfer
void TransferContentFromDevice(IPortableDevice* pDevice);
void TransferContentToDevice(
    IPortableDevice* pDevice,
    REFGUID          guidContentType,
    PCWSTR           pszFileTypeFilter,
    PCWSTR           pszDefaultFileExtension);
void TransferContactToDevice(IPortableDevice* pDevice);
void CreateFolderOnDevice(IPortableDevice* pDevice);
void CreateContactPhotoResourceOnDevice(IPortableDevice* pDevice);

// Content deletion
void DeleteContentFromDevice(IPortableDevice* pDevice);

// Content moving
void MoveContentAlreadyOnDevice(IPortableDevice* pDevice);

// Content update (properties and data simultaneously)
void UpdateContentOnDevice(
    IPortableDevice* pDevice,
    REFGUID          ContentType,
    PCWSTR           pszFileTypeFilter,
    PCWSTR           pszDefaultFileExtension);
    
// Content properties
void ReadContentProperties(IPortableDevice* pDevice);
void WriteContentProperties(IPortableDevice* pDevice);
void ReadContentPropertiesBulk(IPortableDevice* pDevice);
void WriteContentPropertiesBulk(IPortableDevice* pDevice);
void ReadContentPropertiesBulkFilteringByFormat(IPortableDevice* pDevice);

// Functional objects
void ListFunctionalObjects(IPortableDevice* pDevice);
void ListFunctionalCategories(IPortableDevice* pDevice);
void ListSupportedContentTypes(IPortableDevice* pDevice);
void ListRenderingCapabilityInformation(IPortableDevice* pDevice);

// Device events
void ListSupportedEvents(IPortableDevice* pDevice);
void RegisterForEventNotifications(IPortableDevice* pDevice);
void UnregisterForEventNotifications(IPortableDevice* pDevice);

// Misc.
void GetObjectIdentifierFromPersistentUniqueIdentifier(IPortableDevice* pDevice);

void DoMenu()
{
    HRESULT hr              = S_OK;
    UINT    uiSelection     = 0;
    CHAR    szSelection[81] = {0};
    CComPtr<IPortableDevice> pIPortableDevice;

    ChooseDevice(&pIPortableDevice);

    if (pIPortableDevice == NULL)
    {
        // No device was selected, so exit immediately.
        return;
    }

    while (uiSelection != 99)
    {
        ZeroMemory(szSelection, sizeof(szSelection));
        printf("\n\n");
        printf("WPD Sample Application \n");
        printf("=======================================\n\n");
        printf("0.  Enumerate all Devices\n");
        printf("1.  Choose a Device\n");
        printf("2.  Enumerate all content on the device\n");
        printf("3.  Transfer content from the device\n");
        printf("4.  Delete content from the device\n");
        printf("5.  Move content already on the device to another location on the device\n");
        printf("6   Transfer Image content to the device\n");
        printf("7.  Transfer Music content to the device\n");
        printf("8.  Transfer Contact (VCARD file) content to the device\n");
        printf("9.  Transfer Contact (Defined by Properties Only) to the device\n");
        printf("10. Create a folder on the device\n");
        printf("11. Add a Contact Photo resource to an object\n");
        printf("12. Read properties on a content object\n");
        printf("13. Write properties on a content object\n");
        printf("14. Get an object identifier from a Persistent Unique Identifier (PUID)\n");
        printf("15. List all functional categories supported by the device\n");
        printf("16. List all functional objects on the device\n");
        printf("17. List all content types supported by the device\n");
        printf("18. List rendering capabilities supported by the device\n");
        printf("19. Register to receive device event notifications\n");
        printf("20. Unregister from receiving device event notifications\n");
        printf("21. List all events supported by the device\n");
        printf("22. List all hint locations supported by the device\n");
        printf("==(Advanced BULK property operations)==\n");
        printf("23. Read properties on multiple content objects\n");
        printf("24. Write properties on multiple content objects\n");
        printf("25. Read properties on multiple content objects using object format\n");
        printf("==(Update content operations)==\n");
        printf("26. Update Image content (properties and data) on the device\n");
        printf("27. Update Music content (properties and data) on the device\n");
        printf("28. Update Contact content (properties and data) on the device\n");
        printf("99. Exit\n");
        hr = StringCbGetsA(szSelection,sizeof(szSelection));
        if (SUCCEEDED(hr))
        {
            uiSelection = (UINT) atoi(szSelection);
            switch (uiSelection)
            {
                case 0:
                    EnumerateAllDevices();
                    break;
                case 1:
                    // Unregister any device event registrations before
                    // creating a new IPortableDevice
                    UnregisterForEventNotifications(pIPortableDevice);

                    // Release the old IPortableDevice interface before
                    // obtaining a new one.
                    pIPortableDevice = NULL;
                    ChooseDevice(&pIPortableDevice);
                    break;
                case 2:
                    EnumerateAllContent(pIPortableDevice);
                    break;
                case 3:
                    TransferContentFromDevice(pIPortableDevice);
                    break;
                case 4:
                    DeleteContentFromDevice(pIPortableDevice);
                    break;
                case 5:
                    MoveContentAlreadyOnDevice(pIPortableDevice);
                    break;
                case 6:
                    TransferContentToDevice(pIPortableDevice,
                                            WPD_CONTENT_TYPE_IMAGE,
                                            L"JPEG (*.JPG)\0*.JPG\0JPEG (*.JPEG)\0*.JPEG\0JPG (*.JPE)\0*.JPE\0JPG (*.JFIF)\0*.JFIF\0\0",
                                            L"JPG");
                    break;
                case 7:
                    TransferContentToDevice(pIPortableDevice,
                                            WPD_CONTENT_TYPE_AUDIO,
                                            L"WMA (*.WMA)\0*.WMA\0\0",
                                            L"WMA");
                    break;
                case 8:
                    TransferContentToDevice(pIPortableDevice,
                                            WPD_CONTENT_TYPE_CONTACT,
                                            L"VCARD (*.VCF)\0*.VCF\0\0",
                                            L"VCF");
                    break;
                case 9:
                    TransferContactToDevice(pIPortableDevice);
                    break;
                case 10:
                    CreateFolderOnDevice(pIPortableDevice);
                    break;
                case 11:
                    CreateContactPhotoResourceOnDevice(pIPortableDevice);
                    break;
                case 12:
                    ReadContentProperties(pIPortableDevice);
                    break;
                case 13:
                    WriteContentProperties(pIPortableDevice);
                    break;
                case 14:
                    GetObjectIdentifierFromPersistentUniqueIdentifier(pIPortableDevice);
                    break;
                case 15:
                    ListFunctionalCategories(pIPortableDevice);
                    break;
                case 16:
                    ListFunctionalObjects(pIPortableDevice);
                    break;
                case 17:
                    ListSupportedContentTypes(pIPortableDevice);
                    break;
                case 18:
                    ListRenderingCapabilityInformation(pIPortableDevice);
                    break;
                case 19:
                    RegisterForEventNotifications(pIPortableDevice);
                    break;
                case 20:
                    UnregisterForEventNotifications(pIPortableDevice);
                    break;
                case 21:
                    ListSupportedEvents(pIPortableDevice);
                    break;
                case 22:
                    ReadHintLocations(pIPortableDevice);
                    break;
                case 23:
                    ReadContentPropertiesBulk(pIPortableDevice);
                    break;
                case 24:
                    WriteContentPropertiesBulk(pIPortableDevice);
                    break;
                case 25:
                    ReadContentPropertiesBulkFilteringByFormat(pIPortableDevice);
                    break;
                case 26:
                    UpdateContentOnDevice(pIPortableDevice,
                                          WPD_CONTENT_TYPE_IMAGE,
                                          L"JPEG (*.JPG)\0*.JPG\0JPEG (*.JPEG)\0*.JPEG\0JPG (*.JPE)\0*.JPE\0JPG (*.JFIF)\0*.JFIF\0\0",
                                          L"JPG");
                    break;
                case 27:
                    UpdateContentOnDevice(pIPortableDevice,
                                          WPD_CONTENT_TYPE_AUDIO,
                                          L"WMA (*.WMA)\0*.WMA\0\0",
                                          L"WMA");
                    break;
                case 28:
                    UpdateContentOnDevice(pIPortableDevice,
                                          WPD_CONTENT_TYPE_CONTACT,
                                          L"VCARD (*.VCF)\0*.VCF\0\0",
                                          L"VCF");
                    break;
                default:
                    break;
            }
        }
        else
        {
            printf("! Failed to read menu selection string input, hr = 0x%lx\n",hr);
        }
    }
}

int _cdecl wmain(int /*argc*/, wchar_t* /*argv[]*/)
{
    // Enable the heap manager to terminate the process on heap error.
    (void)HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // Initialize COM for COINIT_MULTITHREADED
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        // Enter the menu processing loop
        DoMenu();

        // Uninitialize COM
        CoUninitialize();
    }

    return 0;
}
