// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"

// Reads a string property from the IPortableDeviceProperties
// interface and returns it
HRESULT GetStringValue(
    _In_  IPortableDeviceProperties* properties,
    _In_  PCWSTR                     objectID,
    _In_  REFPROPERTYKEY             key,
    _Out_ PWSTR*                     value)
{
    ComPtr<IPortableDeviceValues>        objectProperties;
    ComPtr<IPortableDeviceKeyCollection> propertiesToRead;

    // 1) CoCreate an IPortableDeviceKeyCollection interface to hold the the property key
    // we wish to read.
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&propertiesToRead));

    // 2) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
    // NOTE: We are not handling any special error cases here so we can proceed with
    // adding as many of the target properties as we can.
    if (SUCCEEDED(hr))
    {
        HRESULT tempHr = S_OK;
        tempHr = propertiesToRead->Add(key);
        if (FAILED(tempHr))
        {
            wprintf(L"! Failed to add PROPERTYKEY to IPortableDeviceKeyCollection, hr= 0x%lx\n", tempHr);
        }
    }

    // 3) Call GetValues() passing the collection of specified PROPERTYKEYs.
    if (SUCCEEDED(hr))
    {
        hr = properties->GetValues(objectID,                // The object whose properties we are reading
                                   propertiesToRead.Get(),  // The properties we want to read
                                   &objectProperties);      // Driver supplied property values for the specified object

        // The error is handled by the caller, which also displays an error message to the user.
    }

    // 4) Extract the string value from the returned property collection
    if (SUCCEEDED(hr))
    {
        PWSTR stringValue = nullptr;
        hr = objectProperties->GetStringValue(key, &stringValue);
        if (SUCCEEDED(hr))
        {
            // Assign the newly read string to the result
            *value = stringValue;
            stringValue = nullptr;
        }
        else
        {
            wprintf(L"! Failed to find property in IPortableDeviceValues, hr = 0x%lx\n", hr);
        }

        CoTaskMemFree(stringValue);
        stringValue = nullptr;
    }

    return hr;
}

// Copies data from a source stream to a destination stream using the
// specified transferSizeBytes as the temporary buffer size.
HRESULT StreamCopy(
    _In_  IStream*    destStream,
    _In_  IStream*    sourceStream,
          DWORD       transferSizeBytes,
    _Out_ DWORD*      bytesWrittenOut)
{
    *bytesWrittenOut = 0;
    HRESULT hr = S_OK;

    // Allocate a temporary buffer (of Optimal transfer size) for the read results to
    // be written to.
    BYTE*   objectData = new (std::nothrow) BYTE[transferSizeBytes];
    if (objectData != nullptr)
    {
        DWORD totalBytesRead    = 0;
        DWORD totalBytesWritten = 0;

        DWORD bytesRead    = 0;
        DWORD bytesWritten = 0;

        // Read until the number of bytes returned from the source stream is 0, or
        // an error occured during transfer.
        do
        {
            // Read object data from the source stream
            hr = sourceStream->Read(objectData, transferSizeBytes, &bytesRead);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to read %u bytes from the source stream, hr = 0x%lx\n", transferSizeBytes, hr);
            }

            // Write object data to the destination stream
            if (SUCCEEDED(hr))
            {
                totalBytesRead += bytesRead; // Calculating total bytes read from device for debugging purposes only

                hr = destStream->Write(objectData, bytesRead, &bytesWritten);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to write %u bytes of object data to the destination stream, hr = 0x%lx\n", bytesRead, hr);
                }

                if (SUCCEEDED(hr))
                {
                    totalBytesWritten += bytesWritten; // Calculating total bytes written to the file for debugging purposes only
                }
            }

            // Output Read/Write operation information only if we have received data and if no error has occured so far.
            if (SUCCEEDED(hr) && (bytesRead > 0))
            {
                wprintf(L"Read %u bytes from the source stream...Wrote %u bytes to the destination stream...\n", bytesRead, bytesWritten);
            }

        } while (SUCCEEDED(hr) && (bytesRead > 0));

        // If we are successful, set bytesWrittenOut before exiting.
        if (SUCCEEDED(hr))
        {
            *bytesWrittenOut = totalBytesWritten;
        }

        // Remember to delete the temporary transfer buffer
        delete [] objectData;
        objectData = nullptr;
    }
    else
    {
        wprintf(L"! Failed to allocate %u bytes for the temporary transfer buffer.\n", transferSizeBytes);
    }

    return hr;
}

// Transfers a selected object's data (WPD_RESOURCE_DEFAULT) to a temporary
// file.
void TransferContentFromDevice(
    _In_ IPortableDevice* device)
{
    //<SnippetTransferFrom1>
    HRESULT                           hr                        = S_OK;
    DWORD                             optimalTransferSizeBytes  = 0;
    PWSTR                             originalFileName          = nullptr;
    WCHAR                             selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceContent>    content;
    ComPtr<IPortableDeviceResources>  resources;
    ComPtr<IPortableDeviceProperties> properties;
    ComPtr<IStream>                   objectDataStream;
    ComPtr<IStream>                   finalFileStream;

    // Prompt user to enter an object identifier on the device to transfer.
    wprintf(L"Enter the identifier of the object you wish to transfer.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content transfer\n");
    }
    //</SnippetTransferFrom1>
    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    //<SnippetTransferFrom2>
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetTransferFrom2>
    // 2) Get an IPortableDeviceResources interface from the IPortableDeviceContent interface to
    // access the resource-specific methods.
    //<SnippetTransferFrom3>
    if (SUCCEEDED(hr))
    {
        hr = content->Transfer(&resources);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceResources from IPortableDeviceContent, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetTransferFrom3>
    // 3) Get the IStream (with READ access) and the optimal transfer buffer size
    // to begin the transfer.
    //<SnippetTransferFrom4>
    if (SUCCEEDED(hr))
    {
        hr = resources->GetStream(selection,                    // Identifier of the object we want to transfer
                                  WPD_RESOURCE_DEFAULT,         // We are transferring the default resource (which is the entire object's data)
                                  STGM_READ,                    // Opening a stream in READ mode, because we are reading data from the device.
                                  &optimalTransferSizeBytes,    // Driver supplied optimal transfer size
                                  &objectDataStream);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IStream (representing object data on the device) from IPortableDeviceResources, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetTransferFrom4>

    // 4) Read the WPD_OBJECT_ORIGINAL_FILE_NAME property so we can properly name the
    // transferred object.  Some content objects may not have this property, so a
    // fall-back case has been provided below. (i.e. Creating a file named <objectID>.data )
    //<SnippetTransferFrom5>
    if (SUCCEEDED(hr))
    {
        hr = content->Properties(&properties);
        if (SUCCEEDED(hr))
        {
            hr = GetStringValue(properties.Get(),
                                selection,
                                WPD_OBJECT_ORIGINAL_FILE_NAME,
                                &originalFileName);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to read WPD_OBJECT_ORIGINAL_FILE_NAME on object '%ws', hr = 0x%lx\n", selection, hr);
                // Create a temporary file name
                originalFileName = reinterpret_cast<PWSTR>(CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR)));
                if (originalFileName)
                {
                    hr = StringCchPrintfW(originalFileName, MAX_PATH, L"%ws.data", selection);
                    if (SUCCEEDED(hr))
                    {
                        wprintf(L"* Creating a filename '%ws' as a default.\n", originalFileName);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        else
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDeviceContent, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetTransferFrom5>
    // 5) Create a destination for the data to be written to.  In this example we are
    // creating a temporary file which is named the same as the object identifier string.
    //<SnippetTransferFrom6>
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFileEx(originalFileName, STGM_CREATE|STGM_WRITE, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &finalFileStream);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to create a temporary file named (%ws) to transfer object (%ws), hr = 0x%lx\n", originalFileName, selection, hr);
        }
    }
    //</SnippetTransferFrom6>
    // 6) Read on the object's data stream and write to the final file's data stream using the
    // driver supplied optimal transfer buffer size.
    //<SnippetTransferFrom7>
    if (SUCCEEDED(hr))
    {
        DWORD totalBytesWritten = 0;

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        hr = StreamCopy(finalFileStream.Get(),       // Destination (The Final File to transfer to)
                        objectDataStream.Get(),      // Source (The Object's data to transfer from)
                        optimalTransferSizeBytes,    // The driver specified optimal transfer buffer size
                        &totalBytesWritten);         // The total number of bytes transferred from device to the finished file
        if (FAILED(hr))
        {
            wprintf(L"! Failed to transfer object from device, hr = 0x%lx\n", hr);
        }
        else
        {
            wprintf(L"* Transferred object '%ws' to '%ws'.\n", selection, originalFileName);
        }
    }

    CoTaskMemFree(originalFileName);
    originalFileName = nullptr;
    //</SnippetTransferFrom7>
}

// Deletes a selected object from the device.
//<SnippetDeleteContent1>
void DeleteContentFromDevice(
    _In_ IPortableDevice* device)
{
    HRESULT                                       hr = S_OK;
    WCHAR                                         selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceContent>                content;
    ComPtr<IPortableDevicePropVariantCollection>  objectsToDelete;
    ComPtr<IPortableDevicePropVariantCollection>  objectsFailedToDelete;

    // Prompt user to enter an object identifier on the device to delete.
    wprintf(L"Enter the identifier of the object you wish to delete.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content deletion\n");
    }

    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 2) CoCreate an IPortableDevicePropVariantCollection interface to hold the the object identifiers
    // to delete.
    //
    // NOTE: This is a collection interface so more than 1 object can be deleted at a time.
    //       This sample only deletes a single object.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&objectsToDelete));
        if (SUCCEEDED(hr))
        {
            // Initialize a PROPVARIANT structure with the object identifier string
            // that the user selected above. Notice we are allocating memory for the
            // PWSTR value.  This memory will be freed when PropVariantClear() is
            // called below.
            PROPVARIANT pv = {0};
            hr = InitPropVariantFromString(selection, &pv);
            if (SUCCEEDED(hr))
            {
                // Add the object identifier to the objects-to-delete list
                // (We are only deleting 1 in this example)
                hr = objectsToDelete->Add(&pv);
                if (SUCCEEDED(hr))
                {
                    // Attempt to delete the object from the device
                    hr = content->Delete(PORTABLE_DEVICE_DELETE_NO_RECURSION,   // Deleting with no recursion
                                            objectsToDelete.Get(),                 // Object(s) to delete
                                            nullptr);                              // Object(s) that failed to delete (we are only deleting 1, so we can pass nullptr here)
                    if (SUCCEEDED(hr))
                    {
                        // An S_OK return lets the caller know that the deletion was successful
                        if (hr == S_OK)
                        {
                            wprintf(L"The object '%ws' was deleted from the device.\n", selection);
                        }

                        // An S_FALSE return lets the caller know that the deletion failed.
                        // The caller should check the returned IPortableDevicePropVariantCollection
                        // for a list of object identifiers that failed to be deleted.
                        else
                        {
                            wprintf(L"The object '%ws' failed to be deleted from the device.\n", selection);
                        }
                    }
                    else
                    {
                        wprintf(L"! Failed to delete an object from the device, hr = 0x%lx\n", hr);
                    }
                }
                else
                {
                    wprintf(L"! Failed to delete an object from the device because we could no add the object identifier string to the IPortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
                wprintf(L"! Failed to delete an object from the device because we could no allocate memory for the object identifier string, hr = 0x%lx\n", hr);
            }

            // Free any allocated values in the PROPVARIANT before exiting
            PropVariantClear(&pv);
        }
        else
        {
            wprintf(L"! Failed to delete an object from the device because we were returned a nullptr IPortableDevicePropVariantCollection interface pointer, hr = 0x%lx\n", hr);
        }
    }
    else
    {
        wprintf(L"! Failed to CoCreateInstance CLSID_PortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
    }
}
//</SnippetDeleteContent1>
// Moves a selected object (which is already on the device) to another location on the device.
void MoveContentAlreadyOnDevice(
    _In_ IPortableDevice* device)
{
    HRESULT                                       hr = S_OK;
    WCHAR                                         selection[SELECTION_BUFFER_SIZE]                 = {0};
    WCHAR                                         destinationFolderObjectID[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceContent>                content;
    ComPtr<IPortableDevicePropVariantCollection>  objectsToMove;
    ComPtr<IPortableDevicePropVariantCollection>  objectsFailedToMove;

    // Check if the device supports the move command needed to perform this operation
    if (SupportsCommand(device, WPD_COMMAND_OBJECT_MANAGEMENT_MOVE_OBJECTS) == FALSE)
    {
        wprintf(L"! This device does not support the move operation (i.e. The WPD_COMMAND_OBJECT_MANAGEMENT_MOVE_OBJECTS command)\n");
        return;
    }

    // Prompt user to enter an object identifier on the device to move.
    wprintf(L"Enter the identifier of the object you wish to move.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content moving\n");
    }

    // Prompt user to enter an object identifier on the device to move.
    wprintf(L"Enter the identifier of the object you wish to move '%ws' to.\n>", selection);
    hr = StringCchGetsW(destinationFolderObjectID, ARRAYSIZE(destinationFolderObjectID));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content moving\n");
    }

    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 2) CoCreate an IPortableDevicePropVariantCollection interface to hold the the object identifiers
    // to move.
    //
    // NOTE: This is a collection interface so more than 1 object can be moved at a time.
    //       This sample only moves a single object.
    if (SUCCEEDED(hr))
    {
        hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&objectsToMove));
        if (SUCCEEDED(hr))
        {
            // Initialize a PROPVARIANT structure with the object identifier string
            // that the user selected above. Notice we are allocating memory for the
            // PWSTR value.  This memory will be freed when PropVariantClear() is
            // called below.
            PROPVARIANT pv = {0};
            hr = InitPropVariantFromString(selection, &pv);
            if (SUCCEEDED(hr))
            {
                // Add the object identifier to the objects-to-move list
                // (We are only moving 1 in this example)
                hr = objectsToMove->Add(&pv);
                if (SUCCEEDED(hr))
                {
                    // Attempt to move the object on the device
                    hr = content->Move(objectsToMove.Get(),       // Object(s) to move
                                        destinationFolderObjectID, // Folder to move to
                                        nullptr);                  // Object(s) that failed to delete (we are only moving 1, so we can pass nullptr here)
                    if (SUCCEEDED(hr))
                    {
                        // An S_OK return lets the caller know that the deletion was successful
                        if (hr == S_OK)
                        {
                            wprintf(L"The object '%ws' was moved on the device.\n", selection);
                        }

                        // An S_FALSE return lets the caller know that the move failed.
                        // The caller should check the returned IPortableDevicePropVariantCollection
                        // for a list of object identifiers that failed to be moved.
                        else
                        {
                            wprintf(L"The object '%ws' failed to be moved on the device.\n", selection);
                        }
                    }
                    else
                    {
                        wprintf(L"! Failed to move an object on the device, hr = 0x%lx\n", hr);
                    }
                }
                else
                {
                    wprintf(L"! Failed to move an object on the device because we could no add the object identifier string to the IPortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
                wprintf(L"! Failed to move an object on the device because we could no allocate memory for the object identifier string, hr = 0x%lx\n", hr);
            }

            // Free any allocated values in the PROPVARIANT before exiting
            PropVariantClear(&pv);
        }
        else
        {
            wprintf(L"! Failed to move an object from the device because we were returned a nullptr IPortableDevicePropVariantCollection interface pointer, hr = 0x%lx\n", hr);
        }
    }
    else
    {
        wprintf(L"! Failed to CoCreateInstance CLSID_PortableDevicePropVariantCollection, hr = 0x%lx\n", hr);
    }
}

// Fills out the required properties for ALL content types...
HRESULT GetRequiredPropertiesForAllContentTypes(
    _In_ IPortableDeviceValues*  objectProperties,
    _In_ PCWSTR                  parentObjectID,
    _In_ PCWSTR                  filePath,
    _In_ IStream*                fileStream)
{
    // Set the WPD_OBJECT_PARENT_ID
    HRESULT hr = objectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjectID);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\n", hr);
    }

    // Set the WPD_OBJECT_SIZE by requesting the total size of the
    // data stream.
    if (SUCCEEDED(hr))
    {
        STATSTG statstg = {0};
        hr = fileStream->Stat(&statstg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            hr = objectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_SIZE, hr = 0x%lx\n", hr);
            }
        }
        else
        {
            wprintf(L"! Failed to get file's total size, hr = 0x%lx\n", hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_ORIGINAL_FILE_NAME by splitting the file path
        // into a separate filename.
        WCHAR fileName[MAX_PATH] = {0};
        WCHAR fileExt[MAX_PATH]  = {0};
        if (_wsplitpath_s(filePath, nullptr, 0, nullptr, 0,
                          fileName,ARRAYSIZE(fileName),
                          fileExt, ARRAYSIZE(fileExt)))
        {
            hr = E_INVALIDARG;
            wprintf(L"! Failed to split the file path, hr = 0x%lx\n", hr);

        }

        if (SUCCEEDED(hr))
        {
            WCHAR originalFileName[MAX_PATH] = {0};
            hr = StringCchPrintfW(originalFileName, ARRAYSIZE(originalFileName), L"%ws%ws", fileName, fileExt);
            if (SUCCEEDED(hr))
            {
                hr = objectProperties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, originalFileName);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to set WPD_OBJECT_ORIGINAL_FILE_NAME, hr = 0x%lx\n", hr);
                }
            }
        }

        // Set the WPD_OBJECT_NAME.  We are using the  file name without its file extension in this
        // example for the object's name.  The object name could be a more friendly name like
        // "This Cool Song" or "That Cool Picture".
        if (SUCCEEDED(hr))
        {
            hr = objectProperties->SetStringValue(WPD_OBJECT_NAME, fileName);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n", hr);
            }
        }
    }
    return hr;
}

// Fills out the required properties for WPD_CONTENT_TYPE_IMAGE
HRESULT GetRequiredPropertiesForImageContentTypes(
    _In_ IPortableDeviceValues*  pObjectProperties)
{
    // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_IMAGE because we are
    // creating/transferring image content to the device.
    HRESULT hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_IMAGE);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_IMAGE, hr = 0x%lx\n", hr);
    }

    // Set the WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_EXIF because we are
    // creating/transferring image content to the device.
    if (SUCCEEDED(hr))
    {
        hr = pObjectProperties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_EXIF);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_EXIF, hr = 0x%lx\n", hr);
        }
    }

    return hr;
}

// Fills out the required properties for WPD_CONTENT_TYPE_AUDIO
HRESULT GetRequiredPropertiesForMusicContentTypes(
    _In_ IPortableDeviceValues*  objectProperties)
{
    // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_AUDIO because we are
    // creating/transferring music content to the device.
    HRESULT hr = objectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_AUDIO);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_AUDIO, hr = 0x%lx\n", hr);
    }

    // Set the WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_MP3 because we are
    // creating/transferring music content to the device.
    if (SUCCEEDED(hr))
    {
        hr = objectProperties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_MP3);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_MP3, hr = 0x%lx\n", hr);
        }
    }

    return hr;
}

// Fills out the required properties for WPD_CONTENT_TYPE_CONTACT
HRESULT GetRequiredPropertiesForContactContentTypes(
    _In_ IPortableDeviceValues*  objectProperties)
{
    // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT because we are
    // creating/transferring contact content to the device.
    HRESULT hr = objectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_CONTACT);
    if (FAILED(hr))
    {
        wprintf(L"! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT, hr = 0x%lx\n", hr);
    }

    // Set the WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_VCARD2 because we are
    // creating/transferring contact content to the device. (This is Version 2 of
    // the VCARD file.  If you have Version 3, use WPD_OBJECT_FORMAT_VCARD3 as the
    // format)
    if (SUCCEEDED(hr))
    {
        hr = objectProperties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_VCARD2);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to set WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_VCARD2, hr = 0x%lx\n", hr);
        }
    }

    return hr;
}

// Fills out the required properties for specific WPD content types.
HRESULT GetRequiredPropertiesForContentType(
    _In_         REFGUID                 contentType,
    _In_         PCWSTR                  parentObjectID,
    _In_         PCWSTR                  filePath,
    _In_         IStream*                fileStream,
    _COM_Outptr_ IPortableDeviceValues** objectProperties)
{
    *objectProperties = nullptr;
    ComPtr<IPortableDeviceValues> objectPropertiesTemp;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&objectPropertiesTemp));
    if (SUCCEEDED(hr))
    {
        if (objectPropertiesTemp != nullptr)
        {
            // Fill out required properties for ALL content types
            hr = GetRequiredPropertiesForAllContentTypes(objectPropertiesTemp.Get(),
                                                         parentObjectID,
                                                         filePath,
                                                         fileStream);
            if (SUCCEEDED(hr))
            {
                // Fill out required properties for specific content types.
                // NOTE: If the content type is unknown to this function then
                // only the required properties will be written.  This is enough
                // for transferring most generic content types.
                if (IsEqualGUID(contentType, WPD_CONTENT_TYPE_IMAGE))
                {
                    hr = GetRequiredPropertiesForImageContentTypes(objectPropertiesTemp.Get());
                }
                else if (IsEqualGUID(contentType, WPD_CONTENT_TYPE_AUDIO))
                {
                    hr = GetRequiredPropertiesForMusicContentTypes(objectPropertiesTemp.Get());
                }
                else if (IsEqualGUID(contentType, WPD_CONTENT_TYPE_CONTACT))
                {
                    hr = GetRequiredPropertiesForContactContentTypes(objectPropertiesTemp.Get());
                }
            }
            else
            {
                wprintf(L"! Failed to get required properties common to ALL content types, hr = 0x%lx\n", hr);
            }

            if (SUCCEEDED(hr))
            {
                *objectProperties = objectPropertiesTemp.Detach();
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            wprintf(L"! Failed to create property information because we were returned a nullptr IPortableDeviceValues interface pointer, hr = 0x%lx\n", hr);
        }
    }

    return hr;
}

// Transfers a user selected file to the device
void TransferContentToDevice(
    _In_ IPortableDevice* device,
    _In_ REFGUID          contentType,
    _In_ PCWSTR           fileTypeFilter,
    _In_ PCWSTR           defaultFileExtension)
{
    //<SnippetContentTransfer1>
    HRESULT                             hr                       = S_OK;
    WCHAR                               filePath[MAX_PATH]       = {0};
    DWORD                               optimalTransferSizeBytes = 0;
    WCHAR                               selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IStream>                     fileStream;
    ComPtr<IPortableDeviceDataStream>   finalObjectDataStream;
    ComPtr<IPortableDeviceValues>       finalObjectProperties;
    ComPtr<IPortableDeviceContent>      content;
    ComPtr<IStream>                     tempStream;  // Temporary IStream which we use to QI for IPortableDeviceDataStream

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    wprintf(L"Enter the identifier of the parent object which the file will be transferred under.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content transfer\n");
    }
    //</SnippetContentTransfer1>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    //<SnippetContentTransfer2>
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetContentTransfer2>
    // 2) Present the user with a File Open dialog.  Our sample is
    // restricting the types to user-specified forms.
    //<SnippetContentTransfer3>
    if (SUCCEEDED(hr))
    {
        OPENFILENAME openFileNameInfo   = {0};

        openFileNameInfo.lStructSize    = sizeof(OPENFILENAME);
        openFileNameInfo.hwndOwner      = nullptr;
        openFileNameInfo.lpstrFile      = filePath;
        openFileNameInfo.nMaxFile       = ARRAYSIZE(filePath);
        openFileNameInfo.lpstrFilter    = fileTypeFilter;
        openFileNameInfo.nFilterIndex   = 1;
        openFileNameInfo.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        openFileNameInfo.lpstrDefExt    = defaultFileExtension;

        if (GetOpenFileName(&openFileNameInfo) == FALSE)
        {
            wprintf(L"The transfer operation was cancelled.\n");
            hr = E_ABORT;
        }
    }
    //</SnippetContentTransfer3>

    // 3) Open the image file and add required properties about the file being transferred
    //<SnippetContentTransfer4>

    if (SUCCEEDED(hr))
    {
        // Open the selected file as an IStream.  This will simplify reading the
        // data and writing to the device.
        hr = SHCreateStreamOnFileEx(filePath, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &fileStream);
        if (SUCCEEDED(hr))
        {
            // Get the required properties needed to properly describe the data being
            // transferred to the device.
            hr = GetRequiredPropertiesForContentType(contentType,              // Content type of the data
                                                     selection,                // Parent to transfer the data under
                                                     filePath,                 // Full file path to the data file
                                                     fileStream.Get(),         // Open IStream that contains the data
                                                     &finalObjectProperties);  // Returned properties describing the data
            if (FAILED(hr))
            {
                wprintf(L"! Failed to get required properties needed to transfer a file to the device, hr = 0x%lx\n", hr);
            }
        }

        if (FAILED(hr))
        {
            wprintf(L"! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n", filePath, hr);
        }
    }
    //</SnippetContentTransfer4>
    //<SnippetContentTransfer5>
    // 4) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = content->CreateObjectWithPropertiesAndData(finalObjectProperties.Get(),    // Properties describing the object data
                                                        &tempStream,                    // Returned object data stream (to transfer the data to)
                                                        &optimalTransferSizeBytes,      // Returned optimal buffer size to use during transfer
                                                        nullptr);

        // Once we have a the IStream returned from CreateObjectWithPropertiesAndData,
        // QI for IPortableDeviceDataStream so we can use the additional methods
        // to get more information about the object (i.e. The newly created object
        // identifier on the device)
        if (SUCCEEDED(hr))
        {
            hr = tempStream.As(&finalObjectDataStream);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to QueryInterface for IPortableDeviceDataStream, hr = 0x%lx\n", hr);
            }
        }

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (SUCCEEDED(hr))
        {
            DWORD totalBytesWritten = 0;

            hr = StreamCopy(finalObjectDataStream.Get(),    // Destination (The Object to transfer to)
                            fileStream.Get(),               // Source (The File data to transfer from)
                            optimalTransferSizeBytes,       // The driver specified optimal transfer buffer size
                            &totalBytesWritten);            // The total number of bytes transferred from file to the device
            if (FAILED(hr))
            {
                wprintf(L"! Failed to transfer object to device, hr = 0x%lx\n", hr);
            }
        }
        else
        {
            wprintf(L"! Failed to get IStream (representing destination object data on the device) from IPortableDeviceContent, hr = 0x%lx\n", hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = finalObjectDataStream->Commit(STGC_DEFAULT);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to commit object to device, hr = 0x%lx\n", hr);
            }
        }

        // Some clients may want to know the object identifier of the newly created
        // object.  This is done by calling GetObjectID() method on the
        // IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            PWSTR newlyCreatedObject = nullptr;
            hr = finalObjectDataStream->GetObjectID(&newlyCreatedObject);
            if (SUCCEEDED(hr))
            {
                wprintf(L"The file '%ws' was transferred to the device.\nThe newly created object's ID is '%ws'\n", filePath, newlyCreatedObject);
            }
            else
            {
                wprintf(L"! Failed to get the newly transferred object's identifier from the device, hr = 0x%lx\n", hr);
            }

            // Free the object identifier string returned from the GetObjectID() method.
            CoTaskMemFree(newlyCreatedObject);
            newlyCreatedObject = nullptr;
        }
    }
    //</SnippetContentTransfer5>
}

// Transfers a user selected file to the device as a new WPD_RESOURCE_CONTACT_PHOTO resource
void CreateContactPhotoResourceOnDevice(
    _In_ IPortableDevice* device)
{
    HRESULT                             hr                               = S_OK;
    DWORD                               optimalTransferSizeBytes         = 0;
    WCHAR                               filePath[MAX_PATH]               = {0};
    WCHAR                               selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IStream>                     fileStream;
    ComPtr<IStream>                     resourceStream;
    ComPtr<IPortableDeviceValues>       resourceAttributes;
    ComPtr<IPortableDeviceContent>      content;
    ComPtr<IPortableDeviceResources>    resources;

    // Prompt user to enter an object identifier for the object to which we will add a Resource.
    wprintf(L"Enter the identifier of the object to which we will add a photograph.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting resource creation\n");
    }

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 2) Get an IPortableDeviceResources interface from the IPortableDeviceContent to
    // access the resource-specific  methods.
    if (SUCCEEDED(hr))
    {
        hr = content->Transfer(&resources);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceResources from IPortableDeviceContent, hr = 0x%lx\n", hr);
        }
    }

    // 3) Present the user with a File Open dialog.  Our sample is
    // restricting the types to user-specified forms.
    if (SUCCEEDED(hr))
    {
        OPENFILENAME openFileNameInfo   = {0};

        openFileNameInfo.lStructSize    = sizeof(OPENFILENAME);
        openFileNameInfo.hwndOwner      = nullptr;
        openFileNameInfo.lpstrFile      = filePath;
        openFileNameInfo.nMaxFile       = ARRAYSIZE(filePath);
        openFileNameInfo.lpstrFilter    = L"JPEG (*.JPG)\0*.JPG\0JPEG (*.JPEG)\0*.JPEG\0JPG (*.JPE)\0*.JPE\0JPG (*.JFIF)\0*.JFIF\0\0";;
        openFileNameInfo.nFilterIndex   = 1;
        openFileNameInfo.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        openFileNameInfo.lpstrDefExt    = L"JPG";

        if (GetOpenFileName(&openFileNameInfo) == FALSE)
        {
            wprintf(L"The transfer operation was cancelled.\n");
            hr = E_ABORT;
        }
    }

    // 4) Open the file and add required properties about the resource being transferred
    if (SUCCEEDED(hr))
    {
        // Open the selected file as an IStream.  This will simplify reading the
        // data and writing to the device.
        hr = SHCreateStreamOnFileEx(filePath, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &fileStream);
        if (SUCCEEDED(hr))
        {
            // CoCreate the IPortableDeviceValues to hold the resource attributes
            hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&resourceAttributes));
            if (SUCCEEDED(hr))
            {
                // Fill in the necessary information regarding this resource

                // Set the WPD_OBJECT_ID.  This informs the driver which object this request is intended for.
                hr = resourceAttributes->SetStringValue(WPD_OBJECT_ID, selection);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to set WPD_OBJECT_ID when creating a resource, hr = 0x%lx\n", hr);
                }

                // Set the WPD_RESOURCE_ATTRIBUTE_RESOURCE_KEY to WPD_RESOURCE_CONTACT_PHOTO
                if (SUCCEEDED(hr))
                {
                    hr = resourceAttributes->SetKeyValue(WPD_RESOURCE_ATTRIBUTE_RESOURCE_KEY, WPD_RESOURCE_CONTACT_PHOTO);
                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to set WPD_RESOURCE_ATTRIBUTE_RESOURCE_KEY to WPD_RESOURCE_CONTACT_PHOTO, hr = 0x%lx\n", hr);
                    }
                }

                // Set the WPD_RESOURCE_ATTRIBUTE_TOTAL_SIZE by requesting the total size of the
                // data stream.
                if (SUCCEEDED(hr))
                {
                    STATSTG statstg = {0};
                    hr = fileStream->Stat(&statstg, STATFLAG_NONAME);
                    if (SUCCEEDED(hr))
                    {
                        hr = resourceAttributes->SetUnsignedLargeIntegerValue(WPD_RESOURCE_ATTRIBUTE_TOTAL_SIZE, statstg.cbSize.QuadPart);
                        if (FAILED(hr))
                        {
                            wprintf(L"! Failed to set WPD_RESOURCE_ATTRIBUTE_TOTAL_SIZE, hr = 0x%lx\n", hr);
                        }
                    }
                    else
                    {
                        wprintf(L"! Failed to get file's total size, hr = 0x%lx\n", hr);
                    }
                }

                // Set the WPD_RESOURCE_ATTRIBUTE_FORMAT to WPD_OBJECT_FORMAT_EXIF because we are
                // creating a contact photo resource with JPG image data.
                if (SUCCEEDED(hr))
                {
                    hr = resourceAttributes->SetGuidValue(WPD_RESOURCE_ATTRIBUTE_FORMAT, WPD_OBJECT_FORMAT_EXIF);
                    if (FAILED(hr))
                    {
                        wprintf(L"! Failed to set WPD_RESOURCE_ATTRIBUTE_FORMAT to WPD_OBJECT_FORMAT_EXIF, hr = 0x%lx\n", hr);
                    }
                }
            }

        }

        if (FAILED(hr))
        {
            wprintf(L"! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n", filePath, hr);
        }
    }

    // 5) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = resources->CreateResource(resourceAttributes.Get(),  // Properties describing this resource
                                       &resourceStream,           // Returned resource data stream (to transfer the data to)
                                       &optimalTransferSizeBytes, // Returned optimal buffer size to use during transfer
                                       nullptr);


        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (SUCCEEDED(hr))
        {
            DWORD totalBytesWritten = 0;

            hr = StreamCopy(resourceStream.Get(),       // Destination (The resource to transfer to)
                            fileStream.Get(),           // Source (The File data to transfer from)
                            optimalTransferSizeBytes,   // The driver specified optimal transfer buffer size
                            &totalBytesWritten);        // The total number of bytes transferred from file to the device
            if (FAILED(hr))
            {
                wprintf(L"! Failed to transfer object to device, hr = 0x%lx\n", hr);
            }
        }
        else
        {
            wprintf(L"! Failed to get IStream (representing destination object data on the device) from IPortableDeviceContent, hr = 0x%lx\n", hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = resourceStream->Commit(STGC_DEFAULT);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to commit resource to device, hr = 0x%lx\n", hr);
            }
        }
    }
}

// Fills out the required properties for a properties-only
// contact named "John Kane".  This is a hard-coded
// contact.
HRESULT GetRequiredPropertiesForPropertiesOnlyContact(
    _In_         PCWSTR                  parentObjectID,
    _COM_Outptr_ IPortableDeviceValues** objectProperties)
{
    *objectProperties = nullptr;
    ComPtr<IPortableDeviceValues> objectPropertiesTemp;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&objectPropertiesTemp));
    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_PARENT_ID
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjectID);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\n", hr);
            }
        }

        // Set the WPD_OBJECT_NAME.
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_OBJECT_NAME, L"John Kane");
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n", hr);
            }
        }

        // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT because we are
        // creating contact content on the device.
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_CONTACT);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT, hr = 0x%lx\n", hr);
            }
        }

        // Set the WPD_CONTACT_DISPLAY_NAME to "John Kane"
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_CONTACT_DISPLAY_NAME, L"John Kane");
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_CONTACT_DISPLAY_NAME, hr = 0x%lx\n", hr);
            }
        }

        // Set the WPD_CONTACT_PRIMARY_PHONE to "425-555-0123"
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_CONTACT_PRIMARY_PHONE, L"425-555-0123");
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_CONTACT_PRIMARY_PHONE, hr = 0x%lx\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            *objectProperties = objectPropertiesTemp.Detach();
        }
    }
    else
    {
        hr = E_UNEXPECTED;
        wprintf(L"! Failed to create property information because we were returned a nullptr IPortableDeviceValues interface pointer, hr = 0x%lx\n", hr);
    }

    return hr;
}

HRESULT GetRequiredPropertiesForFolder(
    _In_         PCWSTR                  parentObjectID,
    _In_         PCWSTR                  folderName,
    _COM_Outptr_ IPortableDeviceValues** objectProperties)
{
    *objectProperties = nullptr;
    ComPtr<IPortableDeviceValues> objectPropertiesTemp;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&objectPropertiesTemp));
    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_PARENT_ID
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjectID);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\n", hr);
            }
        }

        // Set the WPD_OBJECT_NAME.
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_OBJECT_NAME, folderName);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n", hr);
            }
        }

        // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_FOLDER because we are
        // creating contact content on the device.
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_FOLDER);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_FOLDER, hr = 0x%lx\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            *objectProperties = objectPropertiesTemp.Detach();
        }
    }
    else
    {
        hr = E_UNEXPECTED;
        wprintf(L"! Failed to create property information because we were returned a nullptr IPortableDeviceValues interface pointer, hr = 0x%lx\n", hr);
    }

    return hr;
}

// Creates a properties-only object on the device which is
// WPD_CONTENT_TYPE_CONTACT specific.
// NOTE: This function creates a hard-coded contact for
// "John Kane" always.
void TransferContactToDevice(
    _In_ IPortableDevice*    device)
{
    //<SnippetTransfer1>
    HRESULT                         hr = S_OK;
    WCHAR                           selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceValues>   finalObjectProperties;
    ComPtr<IPortableDeviceContent>  content;

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    wprintf(L"Enter the identifier of the parent object which the contact will be transferred under.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content transfer\n");
    }
    //</SnippetTransfer1>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.

    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }
    //<SnippetTransfer2>
    // 2) Get the properties that describe the object being created on the device

    if (SUCCEEDED(hr))
    {
        hr = GetRequiredPropertiesForPropertiesOnlyContact(selection,                // Parent to transfer the data under
                                                           &finalObjectProperties);  // Returned properties describing the data
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get required properties needed to transfer an image file to the device, hr = 0x%lx\n", hr);
        }
    }
    //</SnippetTransfer2>
    // 3) Transfer the content to the device by creating a properties-only object
    //<SnippetTransfer3>
    if (SUCCEEDED(hr))
    {
        PWSTR newlyCreatedObject = nullptr;
        hr = content->CreateObjectWithPropertiesOnly(finalObjectProperties.Get(),   // Properties describing the object data
                                                     &newlyCreatedObject);
        if (SUCCEEDED(hr))
        {
            wprintf(L"The contact was transferred to the device.\nThe newly created object's ID is '%ws'\n", newlyCreatedObject);
        }
        else
        {
            wprintf(L"! Failed to transfer contact object to the device, hr = 0x%lx\n", hr);
        }

        // Free the object identifier string returned from CreateObjectWithPropertiesOnly
        CoTaskMemFree(newlyCreatedObject);
        newlyCreatedObject = nullptr;
    }
    //</SnippetTransfer3>
}

// Creates a properties-only object on the device which is
// WPD_CONTENT_TYPE_FOLDER specific.
void CreateFolderOnDevice(
    _In_ IPortableDevice*    device)
{
    HRESULT                         hr = S_OK;
    WCHAR                           selection[SELECTION_BUFFER_SIZE]  = {0};
    WCHAR                           folderName[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IPortableDeviceValues>   finalObjectProperties;
    ComPtr<IPortableDeviceContent>  content;

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    wprintf(L"Enter the identifier of the parent object which the folder will be created under.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting folder creation\n");
    }

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    wprintf(L"Enter the name of the the folder to create.\n>");
    hr = StringCchGetsW(folderName, ARRAYSIZE(folderName));
    if (FAILED(hr))
    {
        wprintf(L"An invalid folder name was specified, aborting folder creation\n");
    }

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
    }

    // 2) Get the properties that describe the object being created on the device
    if (SUCCEEDED(hr))
    {
        hr = GetRequiredPropertiesForFolder(selection,                // Parent to create the folder under
                                            folderName,               // Folder Name
                                            &finalObjectProperties);  // Returned properties describing the folder
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get required properties needed to transfer an image file to the device, hr = 0x%lx\n", hr);
        }
    }

    // 3) Transfer the content to the device by creating a properties-only object
    if (SUCCEEDED(hr))
    {
        PWSTR newlyCreatedObject = nullptr;
        hr = content->CreateObjectWithPropertiesOnly(finalObjectProperties.Get(), // Properties describing the object data
                                                     &newlyCreatedObject);
        if (SUCCEEDED(hr))
        {
            wprintf(L"The folder was created on the device.\nThe newly created object's ID is '%ws'\n", newlyCreatedObject);
        }
        else
        {
            wprintf(L"! Failed to create a new folder on the device, hr = 0x%lx\n", hr);
        }

        // Free the object identifier string returned from CreateObjectWithPropertiesOnly
        CoTaskMemFree(newlyCreatedObject);
        newlyCreatedObject = nullptr;
    }

}

// Fills out properties that accompany updating an object's data...
HRESULT GetPropertiesForUpdateData(
    _In_         PCWSTR                  filePath,
    _In_         IStream*                fileStream,
    _COM_Outptr_ IPortableDeviceValues** objectProperties)
{
    *objectProperties = nullptr;
    ComPtr<IPortableDeviceValues> objectPropertiesTemp;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&objectPropertiesTemp));

    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_SIZE by requesting the total size of the
        // data stream.
        STATSTG statstg = {0};
        hr = fileStream->Stat(&statstg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_SIZE, hr = 0x%lx\n", hr);
            }
        }
        else
        {
            wprintf(L"! Failed to get file's total size, hr = 0x%lx\n", hr);
        }

        // Set the WPD_OBJECT_ORIGINAL_FILE_NAME by splitting the file path
        // into a separate filename.
        WCHAR fileName[MAX_PATH] = {0};
        WCHAR fileExt[MAX_PATH]  = {0};
        if (_wsplitpath_s(filePath, nullptr, 0, nullptr, 0,
                          fileName,ARRAYSIZE(fileName),
                          fileExt, ARRAYSIZE(fileExt)))
        {
            hr = E_INVALIDARG;
            wprintf(L"! Failed to split the file path, hr = 0x%lx\n", hr);
        }

        if (SUCCEEDED(hr))
        {
            WCHAR originalFileName[MAX_PATH] = {0};
            hr = StringCchPrintfW(originalFileName, ARRAYSIZE(originalFileName), L"%ws%ws", fileName, fileExt);
            if (SUCCEEDED(hr))
            {
                hr = objectPropertiesTemp->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, originalFileName);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to set WPD_OBJECT_ORIGINAL_FILE_NAME, hr = 0x%lx\n", hr);
                }
            }
        }

        // Set the WPD_OBJECT_NAME.  We are using the  file name without its file extension in this
        // example for the object's name.  The object name could be a more friendly name like
        // "This Cool Song" or "That Cool Picture".
        if (SUCCEEDED(hr))
        {
            hr = objectPropertiesTemp->SetStringValue(WPD_OBJECT_NAME, fileName);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            *objectProperties = objectPropertiesTemp.Detach();
        }
    }
    
    return hr;
}


// Updates a selected object's properties and data (WPD_RESOURCE_DEFAULT).
void UpdateContentOnDevice(
    _In_ IPortableDevice* device,
    _In_ REFGUID          contentType,
    _In_ PCWSTR           fileTypeFilter,
    _In_ PCWSTR           defaultFileExtension)
{
    HRESULT                             hr                       = S_OK;
    DWORD                               optimalTransferSizeBytes = 0;
    WCHAR                               filePath[MAX_PATH]       = {0};
    WCHAR                               selection[SELECTION_BUFFER_SIZE] = {0};
    ComPtr<IStream>                     fileStream;
    ComPtr<IPortableDeviceDataStream>   finalObjectDataStream;
    ComPtr<IPortableDeviceValues>       finalObjectProperties;
    ComPtr<IPortableDeviceContent2>     content2;
    ComPtr<IStream>                     tempStream;  // Temporary IStream which we use to QI for IPortableDeviceDataStream

    // Prompt user to enter an object identifier for the object on the device to update.
    wprintf(L"Enter the identifier of the object to update.\n>");
    hr = StringCchGetsW(selection, ARRAYSIZE(selection));
    if (FAILED(hr))
    {
        wprintf(L"An invalid object identifier was specified, aborting content update\n");
    }

    // 1) Get an IPortableDeviceContent2 interface from the IPortableDevice interface to
    // access the UpdateObjectWithPropertiesAndData method.
    if (SUCCEEDED(hr))
    {
        ComPtr<IPortableDeviceContent> content;
    
        hr = device->Content(&content);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n", hr);
        }
        else
        {
            hr = content.As(&content2);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to get IPortableDeviceContent2 from IPortableDeviceContent, hr = 0x%lx\n", hr);
            }
        }
    }

    // 2) (Optional) Check if the object is of the correct content type. This also ensures the user-specified object ID is valid.
    if (SUCCEEDED(hr))
    {
        ComPtr<IPortableDeviceProperties> properties;

        hr = content2->Properties(&properties);
        if (FAILED(hr))
        {
            wprintf(L"! Failed to get IPortableDeviceProperties from IPortableDeviceContent2, hr = 0x%lx\n", hr);
        }
        else
        {
            ComPtr<IPortableDeviceValues> objectProperties;

            hr = properties->GetValues(selection, nullptr, &objectProperties);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to get all properties for object (%ws), hr = 0x%lx\n", selection, hr);
            }
            else
            {
                GUID objectContentType;
                hr = objectProperties->GetGuidValue(WPD_OBJECT_CONTENT_TYPE, &objectContentType);
                if (FAILED(hr))
                {
                    wprintf(L"! Failed to get WPD_OBJECT_CONTENT_TYPE for object (%ws), hr = 0x%lx\n", selection, hr);
                }
                else if (objectContentType != contentType)
                {
                    hr = E_INVALIDARG;
                    wprintf(L"! Object (%ws) is not of the correct content type, hr = 0x%lx\n", selection, hr);
                }
            }
        }
    }

    // 3) Present the user with a File Open dialog.  Our sample is
    // restricting the types to user-specified forms.
    if (SUCCEEDED(hr))
    {
        OPENFILENAME openFileNameInfo   = {0};

        openFileNameInfo.lStructSize    = sizeof(OPENFILENAME);
        openFileNameInfo.hwndOwner      = nullptr;
        openFileNameInfo.lpstrFile      = filePath;
        openFileNameInfo.nMaxFile       = ARRAYSIZE(filePath);
        openFileNameInfo.lpstrFilter    = fileTypeFilter;
        openFileNameInfo.nFilterIndex   = 1;
        openFileNameInfo.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        openFileNameInfo.lpstrDefExt    = defaultFileExtension;

        if (GetOpenFileName(&openFileNameInfo) == FALSE)
        {
            hr = E_ABORT;
        }
    }

    // 4) Open the file and add required properties about the file being transferred
    if (SUCCEEDED(hr))
    {
        // Open the selected file as an IStream.  This will simplify reading the
        // data and writing to the device.
        hr = SHCreateStreamOnFileEx(filePath, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &fileStream);
        if (SUCCEEDED(hr))
        {
            // Get the required properties needed to properly describe the data being
            // transferred to the device.
            hr = GetPropertiesForUpdateData(filePath,                // Full file path to the data file
                                            fileStream.Get(),        // Open IStream that contains the data
                                            &finalObjectProperties); // Returned properties describing the data

            if (FAILED(hr))
            {
                wprintf(L"! Failed to get properties needed to transfer a file to the device, hr = 0x%lx\n", hr);
            }
        }

        if (FAILED(hr))
        {
            wprintf(L"! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n", filePath, hr);
        }
    }

    // 5) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = content2->UpdateObjectWithPropertiesAndData(selection,
                                                         finalObjectProperties.Get(),   // Properties describing the object data
                                                         &tempStream,                   // Returned object data stream (to transfer the data to)
                                                         &optimalTransferSizeBytes);    // Returned optimal buffer size to use during transfer

        // Once we have a the IStream returned from UpdateObjectWithPropertiesAndData,
        // QI for IPortableDeviceDataStream so we can use the additional methods
        // to get more information about the object (i.e. The newly created object
        // identifier on the device)
        if (SUCCEEDED(hr))
        {
            hr = tempStream.As(&finalObjectDataStream);
            if (FAILED(hr))
            {
                wprintf(L"! Failed to QueryInterface for IPortableDeviceDataStream, hr = 0x%lx\n", hr);
            }
        }

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (SUCCEEDED(hr))
        {
            DWORD totalBytesWritten = 0;

            hr = StreamCopy(finalObjectDataStream.Get(), // Destination (The Object to transfer to)
                            fileStream.Get(),            // Source (The File data to transfer from)
                            optimalTransferSizeBytes,    // The driver specified optimal transfer buffer size
                            &totalBytesWritten);         // The total number of bytes transferred from file to the device
            if (FAILED(hr))
            {
                wprintf(L"! Failed to transfer object data to device, hr = 0x%lx\n", hr);
            }
        }
        else
        {
            wprintf(L"! Failed to get IStream (representing destination object data on the device) from UpdateObjectWithPropertiesAndData, hr = 0x%lx\n", hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = finalObjectDataStream->Commit(STGC_DEFAULT);
            if (SUCCEEDED(hr))
            {
                wprintf(L"The file '%ws' was transferred to the device to object '%ws'\n", filePath, selection);
            }
            else
            {
                wprintf(L"! Failed to commit new object data to device, hr = 0x%lx\n", hr);
            }
        }
    }
}
