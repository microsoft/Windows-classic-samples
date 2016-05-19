// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "stdafx.h"
#include <strsafe.h>

// Reads a string property from the IPortableDeviceProperties
// interface and returns it in the form of a CAtlStringW
HRESULT GetStringValue(
    IPortableDeviceProperties* pProperties,
    PCWSTR                     pszObjectID,
    REFPROPERTYKEY             key,
    CAtlStringW&               strStringValue)
{
    CComPtr<IPortableDeviceValues>        pObjectProperties;
    CComPtr<IPortableDeviceKeyCollection> pPropertiesToRead;

    // 1) CoCreate an IPortableDeviceKeyCollection interface to hold the the property key
    // we wish to read.
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pPropertiesToRead));

    // 2) Populate the IPortableDeviceKeyCollection with the keys we wish to read.
    // NOTE: We are not handling any special error cases here so we can proceed with
    // adding as many of the target properties as we can.
    if (SUCCEEDED(hr))
    {
        if (pPropertiesToRead != NULL)
        {
            HRESULT hrTemp = S_OK;
            hrTemp = pPropertiesToRead->Add(key);
            if (FAILED(hrTemp))
            {
                printf("! Failed to add PROPERTYKEY to IPortableDeviceKeyCollection, hr= 0x%lx\n", hrTemp);
            }
        }
    }

    // 3) Call GetValues() passing the collection of specified PROPERTYKEYs.
    if (SUCCEEDED(hr))
    {
        hr = pProperties->GetValues(pszObjectID,         // The object whose properties we are reading
                                    pPropertiesToRead,   // The properties we want to read
                                    &pObjectProperties); // Driver supplied property values for the specified object

        // The error is handled by the caller, which also displays an error message to the user.
    }

    // 4) Extract the string value from the returned property collection
    if (SUCCEEDED(hr))
    {
        PWSTR pszStringValue = NULL;
        hr = pObjectProperties->GetStringValue(key, &pszStringValue);
        if (SUCCEEDED(hr))
        {
            // Assign the newly read string to the CAtlStringW out
            // parameter.
            strStringValue = pszStringValue;
        }
        else
        {
            printf("! Failed to find property in IPortableDeviceValues, hr = 0x%lx\n",hr);
        }

        CoTaskMemFree(pszStringValue);
        pszStringValue = NULL;
    }

    return hr;
}

// Copies data from a source stream to a destination stream using the
// specified cbTransferSize as the temporary buffer size.
HRESULT StreamCopy(
    IStream*    pDestStream,
    IStream*    pSourceStream,
    DWORD       cbTransferSize,
    DWORD*      pcbWritten)
{

    HRESULT hr = S_OK;

    // Allocate a temporary buffer (of Optimal transfer size) for the read results to
    // be written to.
    BYTE*   pObjectData = new (std::nothrow) BYTE[cbTransferSize];
    if (pObjectData != NULL)
    {
        DWORD cbTotalBytesRead    = 0;
        DWORD cbTotalBytesWritten = 0;

        DWORD cbBytesRead    = 0;
        DWORD cbBytesWritten = 0;

        // Read until the number of bytes returned from the source stream is 0, or
        // an error occured during transfer.
        do
        {
            // Read object data from the source stream
            hr = pSourceStream->Read(pObjectData, cbTransferSize, &cbBytesRead);
            if (FAILED(hr))
            {
                printf("! Failed to read %d bytes from the source stream, hr = 0x%lx\n",cbTransferSize, hr);
            }

            // Write object data to the destination stream
            if (SUCCEEDED(hr))
            {
                cbTotalBytesRead += cbBytesRead; // Calculating total bytes read from device for debugging purposes only

                hr = pDestStream->Write(pObjectData, cbBytesRead, &cbBytesWritten);
                if (FAILED(hr))
                {
                    printf("! Failed to write %d bytes of object data to the destination stream, hr = 0x%lx\n",cbBytesRead, hr);
                }

                if (SUCCEEDED(hr))
                {
                    cbTotalBytesWritten += cbBytesWritten; // Calculating total bytes written to the file for debugging purposes only
                }
            }

            // Output Read/Write operation information only if we have received data and if no error has occured so far.
            if (SUCCEEDED(hr) && (cbBytesRead > 0))
            {
                printf("Read %d bytes from the source stream...Wrote %d bytes to the destination stream...\n", cbBytesRead, cbBytesWritten);
            }

        } while (SUCCEEDED(hr) && (cbBytesRead > 0));

        // If the caller supplied a pcbWritten parameter and we
        // and we are successful, set it to cbTotalBytesWritten
        // before exiting.
        if ((SUCCEEDED(hr)) && (pcbWritten != NULL))
        {
            *pcbWritten = cbTotalBytesWritten;
        }

        // Remember to delete the temporary transfer buffer
        delete [] pObjectData;
        pObjectData = NULL;
    }
    else
    {
        printf("! Failed to allocate %d bytes for the temporary transfer buffer.\n", cbTransferSize);
    }

    return hr;
}

// Transfers a selected object's data (WPD_RESOURCE_DEFAULT) to a temporary
// file.
void TransferContentFromDevice(
    IPortableDevice* pDevice)
{
	//<SnippetTransferFrom1>
    HRESULT                            hr                   = S_OK;
    WCHAR                              szSelection[81]      = {0};
    CComPtr<IPortableDeviceContent>    pContent;
    CComPtr<IPortableDeviceResources>  pResources;
    CComPtr<IPortableDeviceProperties> pProperties;
    CComPtr<IStream>                   pObjectDataStream;
    CComPtr<IStream>                   pFinalFileStream;
    DWORD                              cbOptimalTransferSize = 0;
    CAtlStringW                        strOriginalFileName;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }


    // Prompt user to enter an object identifier on the device to transfer.
    printf("Enter the identifer of the object you wish to transfer.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content transfer\n");
    }
	//</SnippetTransferFrom1>
	// 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
	//<SnippetTransferFrom2>
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetTransferFrom2>
    // 2) Get an IPortableDeviceResources interface from the IPortableDeviceContent interface to
    // access the resource-specific methods.
	//<SnippetTransferFrom3>
    if (SUCCEEDED(hr))
    {
        hr = pContent->Transfer(&pResources);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceResources from IPortableDeviceContent, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetTransferFrom3>
    // 3) Get the IStream (with READ access) and the optimal transfer buffer size
    // to begin the transfer.
	//<SnippetTransferFrom4>
    if (SUCCEEDED(hr))
    {
        hr = pResources->GetStream(szSelection,             // Identifier of the object we want to transfer
                                   WPD_RESOURCE_DEFAULT,    // We are transferring the default resource (which is the entire object's data)
                                   STGM_READ,               // Opening a stream in READ mode, because we are reading data from the device.
                                   &cbOptimalTransferSize,  // Driver supplied optimal transfer size
                                   &pObjectDataStream);
        if (FAILED(hr))
        {
            printf("! Failed to get IStream (representing object data on the device) from IPortableDeviceResources, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetTransferFrom4>

    // 4) Read the WPD_OBJECT_ORIGINAL_FILE_NAME property so we can properly name the
    // transferred object.  Some content objects may not have this property, so a
    // fall-back case has been provided below. (i.e. Creating a file named <objectID>.data )
	//<SnippetTransferFrom5>
    if (SUCCEEDED(hr))
    {
        hr = pContent->Properties(&pProperties);
        if (SUCCEEDED(hr))
        {
            hr = GetStringValue(pProperties,
                                szSelection,
                                WPD_OBJECT_ORIGINAL_FILE_NAME,
                                strOriginalFileName);
            if (FAILED(hr))
            {
                printf("! Failed to read WPD_OBJECT_ORIGINAL_FILE_NAME on object '%ws', hr = 0x%lx\n", szSelection, hr);
                strOriginalFileName.Format(L"%ws.data", szSelection);
                printf("* Creating a filename '%ws' as a default.\n", (PWSTR)strOriginalFileName.GetString());
                // Set the HRESULT to S_OK, so we can continue with our newly generated
                // temporary file name.
                hr = S_OK;
            }
        }
        else
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDeviceContent, hr = 0x%lx\n", hr);
        }
    }
	//</SnippetTransferFrom5>
    // 5) Create a destination for the data to be written to.  In this example we are
    // creating a temporary file which is named the same as the object identifier string.
	//<SnippetTransferFrom6>
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFile(strOriginalFileName, STGM_CREATE|STGM_WRITE, &pFinalFileStream);
        if (FAILED(hr))
        {
            printf("! Failed to create a temporary file named (%ws) to transfer object (%ws), hr = 0x%lx\n",(PWSTR)strOriginalFileName.GetString(), szSelection, hr);
        }
    }
	//</SnippetTransferFrom6>
    // 6) Read on the object's data stream and write to the final file's data stream using the
    // driver supplied optimal transfer buffer size.
	//<SnippetTransferFrom7>
    if (SUCCEEDED(hr))
    {
        DWORD cbTotalBytesWritten = 0;

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        hr = StreamCopy(pFinalFileStream,       // Destination (The Final File to transfer to)
                        pObjectDataStream,      // Source (The Object's data to transfer from)
                        cbOptimalTransferSize,  // The driver specified optimal transfer buffer size
                        &cbTotalBytesWritten);  // The total number of bytes transferred from device to the finished file
        if (FAILED(hr))
        {
            printf("! Failed to transfer object from device, hr = 0x%lx\n",hr);
        }
        else
        {
            printf("* Transferred object '%ws' to '%ws'.\n", szSelection, (PWSTR)strOriginalFileName.GetString());
        }
    }
	//</SnippetTransferFrom7>
}

// Deletes a selected object from the device.
//<SnippetDeleteContent1>
void DeleteContentFromDevice(
    IPortableDevice* pDevice)
{
    HRESULT                                       hr               = S_OK;
    WCHAR                                         szSelection[81]  = {0};
    CComPtr<IPortableDeviceContent>               pContent;
    CComPtr<IPortableDevicePropVariantCollection> pObjectsToDelete;
    CComPtr<IPortableDevicePropVariantCollection> pObjectsFailedToDelete;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    // Prompt user to enter an object identifier on the device to delete.
    printf("Enter the identifer of the object you wish to delete.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content deletion\n");
    }

    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
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
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pObjectsToDelete));
        if (SUCCEEDED(hr))
        {
            if (pObjectsToDelete != NULL)
            {
                PROPVARIANT pv = {0};
                PropVariantInit(&pv);

                // Initialize a PROPVARIANT structure with the object identifier string
                // that the user selected above. Notice we are allocating memory for the
                // PWSTR value.  This memory will be freed when PropVariantClear() is
                // called below.
                pv.vt      = VT_LPWSTR;
                pv.pwszVal = AtlAllocTaskWideString(szSelection);
                if (pv.pwszVal != NULL)
                {
                    // Add the object identifier to the objects-to-delete list
                    // (We are only deleting 1 in this example)
                    hr = pObjectsToDelete->Add(&pv);
                    if (SUCCEEDED(hr))
                    {
                        // Attempt to delete the object from the device
                        hr = pContent->Delete(PORTABLE_DEVICE_DELETE_NO_RECURSION,  // Deleting with no recursion
                                              pObjectsToDelete,                     // Object(s) to delete
                                              NULL);                                // Object(s) that failed to delete (we are only deleting 1, so we can pass NULL here)
                        if (SUCCEEDED(hr))
                        {
                            // An S_OK return lets the caller know that the deletion was successful
                            if (hr == S_OK)
                            {
                                printf("The object '%ws' was deleted from the device.\n", szSelection);
                            }

                            // An S_FALSE return lets the caller know that the deletion failed.
                            // The caller should check the returned IPortableDevicePropVariantCollection
                            // for a list of object identifiers that failed to be deleted.
                            else
                            {
                                printf("The object '%ws' failed to be deleted from the device.\n", szSelection);
                            }
                        }
                        else
                        {
                            printf("! Failed to delete an object from the device, hr = 0x%lx\n",hr);
                        }
                    }
                    else
                    {
                        printf("! Failed to delete an object from the device because we could no add the object identifier string to the IPortableDevicePropVariantCollection, hr = 0x%lx\n",hr);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    printf("! Failed to delete an object from the device because we could no allocate memory for the object identifier string, hr = 0x%lx\n",hr);
                }

                // Free any allocated values in the PROPVARIANT before exiting
                PropVariantClear(&pv);
            }
            else
            {
                printf("! Failed to delete an object from the device because we were returned a NULL IPortableDevicePropVariantCollection interface pointer, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to CoCreateInstance CLSID_PortableDevicePropVariantCollection, hr = 0x%lx\n",hr);
        }
    }
}
//</SnippetDeleteContent1>
// Moves a selected object (which is already on the device) to another location on the device.
void MoveContentAlreadyOnDevice(
    IPortableDevice* pDevice)
{
    HRESULT                                       hr                              = S_OK;
    WCHAR                                         szSelection[81]                 = {0};
    WCHAR                                         szDestinationFolderObjectID[81] = {0};
    CComPtr<IPortableDeviceContent>               pContent;
    CComPtr<IPortableDevicePropVariantCollection> pObjectsToMove;
    CComPtr<IPortableDevicePropVariantCollection> pObjectsFailedToMove;

    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    // Check if the device supports the move command needed to perform this operation
    if (SupportsCommand(pDevice, WPD_COMMAND_OBJECT_MANAGEMENT_MOVE_OBJECTS) == FALSE)
    {
        printf("! This device does not support the move operation (i.e. The WPD_COMMAND_OBJECT_MANAGEMENT_MOVE_OBJECTS command)\n");
        return;
    }

    // Prompt user to enter an object identifier on the device to move.
    printf("Enter the identifer of the object you wish to move.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content moving\n");
    }

    // Prompt user to enter an object identifier on the device to move.
    printf("Enter the identifer of the object you wish to move '%ws' to.\n>", szSelection);
    hr = StringCbGetsW(szDestinationFolderObjectID,sizeof(szDestinationFolderObjectID));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content moving\n");
    }

    // 1) get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
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
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pObjectsToMove));
        if (SUCCEEDED(hr))
        {
            if (pObjectsToMove != NULL)
            {
                PROPVARIANT pv = {0};
                PropVariantInit(&pv);

                // Initialize a PROPVARIANT structure with the object identifier string
                // that the user selected above. Notice we are allocating memory for the
                // PWSTR value.  This memory will be freed when PropVariantClear() is
                // called below.
                pv.vt      = VT_LPWSTR;
                pv.pwszVal = AtlAllocTaskWideString(szSelection);
                if (pv.pwszVal != NULL)
                {
                    // Add the object identifier to the objects-to-move list
                    // (We are only moving 1 in this example)
                    hr = pObjectsToMove->Add(&pv);
                    if (SUCCEEDED(hr))
                    {
                        // Attempt to move the object on the device
                        hr = pContent->Move(pObjectsToMove,              // Object(s) to move
                                            szDestinationFolderObjectID, // Folder to move to
                                            NULL);                       // Object(s) that failed to delete (we are only moving 1, so we can pass NULL here)
                        if (SUCCEEDED(hr))
                        {
                            // An S_OK return lets the caller know that the deletion was successful
                            if (hr == S_OK)
                            {
                                printf("The object '%ws' was moved on the device.\n", szSelection);
                            }

                            // An S_FALSE return lets the caller know that the move failed.
                            // The caller should check the returned IPortableDevicePropVariantCollection
                            // for a list of object identifiers that failed to be moved.
                            else
                            {
                                printf("The object '%ws' failed to be moved on the device.\n", szSelection);
                            }
                        }
                        else
                        {
                            printf("! Failed to move an object on the device, hr = 0x%lx\n",hr);
                        }
                    }
                    else
                    {
                        printf("! Failed to move an object on the device because we could no add the object identifier string to the IPortableDevicePropVariantCollection, hr = 0x%lx\n",hr);
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    printf("! Failed to move an object on the device because we could no allocate memory for the object identifier string, hr = 0x%lx\n",hr);
                }

                // Free any allocated values in the PROPVARIANT before exiting
                PropVariantClear(&pv);
            }
            else
            {
                printf("! Failed to move an object from the device because we were returned a NULL IPortableDevicePropVariantCollection interface pointer, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to CoCreateInstance CLSID_PortableDevicePropVariantCollection, hr = 0x%lx\n",hr);
        }
    }
}

// Fills out the required properties for ALL content types...
HRESULT GetRequiredPropertiesForAllContentTypes(
    IPortableDeviceValues*  pObjectProperties,
    PCWSTR                  pszParentObjectID,
    PCWSTR                  pszFilePath,
    IStream*                pFileStream)
{
    // Set the WPD_OBJECT_PARENT_ID
    HRESULT hr = pObjectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, pszParentObjectID);
    if (FAILED(hr))
    {
        printf("! Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\n",hr);
    }

    // Set the WPD_OBJECT_SIZE by requesting the total size of the
    // data stream.
    if (SUCCEEDED(hr))
    {
        STATSTG statstg = {0};
        hr = pFileStream->Stat(&statstg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
            if (FAILED(hr))
            {
                printf("! Failed to set WPD_OBJECT_SIZE, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to get file's total size, hr = 0x%lx\n",hr);
        }
    }

    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_ORIGINAL_FILE_NAME by splitting the file path
        // into a separate filename.
        WCHAR szFileName[MAX_PATH] = {0};
        WCHAR szFileExt[MAX_PATH]  = {0};
        if (_wsplitpath_s(pszFilePath, NULL,0,NULL,0,
                           szFileName,ARRAYSIZE(szFileName),
                           szFileExt, ARRAYSIZE(szFileExt)))
        {
            hr = E_INVALIDARG;
            printf("! Failed to split the file path, hr = 0x%lx\n",hr);    

        }

        if (SUCCEEDED(hr))
        {
            CAtlStringW strOriginalFileName;
            strOriginalFileName.Format(L"%ws%ws", szFileName, szFileExt);
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, strOriginalFileName);
            if (FAILED(hr))
            {
                printf("! Failed to set WPD_OBJECT_ORIGINAL_FILE_NAME, hr = 0x%lx\n",hr);
            }
        }

        // Set the WPD_OBJECT_NAME.  We are using the  file name without its file extension in this
        // example for the object's name.  The object name could be a more friendly name like
        // "This Cool Song" or "That Cool Picture".
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, szFileName);
            if (FAILED(hr))
            {
                printf("! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n",hr);
            }
        }
    }
    return hr;
}

// Fills out the required properties for WPD_CONTENT_TYPE_IMAGE
HRESULT GetRequiredPropertiesForImageContentTypes(
    IPortableDeviceValues*  pObjectProperties)
{
    // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_IMAGE because we are
    // creating/transferring image content to the device.
    HRESULT hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_IMAGE);
    if (FAILED(hr))
    {
        printf("! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_IMAGE, hr = 0x%lx\n",hr);
    }

    // Set the WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_EXIF because we are
    // creating/transferring image content to the device.
    if (SUCCEEDED(hr))
    {
        hr = pObjectProperties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_EXIF);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_EXIF, hr = 0x%lx\n",hr);
        }
    }

    return hr;
}

// Fills out the required properties for WPD_CONTENT_TYPE_AUDIO
HRESULT GetRequiredPropertiesForMusicContentTypes(
    IPortableDeviceValues*  pObjectProperties)
{
    // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_AUDIO because we are
    // creating/transferring music content to the device.
    HRESULT hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_AUDIO);
    if (FAILED(hr))
    {
        printf("! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_AUDIO, hr = 0x%lx\n",hr);
    }

    // Set the WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_WMA because we are
    // creating/transferring music content to the device.
    if (SUCCEEDED(hr))
    {
        hr = pObjectProperties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_WMA);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_WMA, hr = 0x%lx\n",hr);
        }
    }

    return hr;
}

// Fills out the required properties for WPD_CONTENT_TYPE_CONTACT
HRESULT GetRequiredPropertiesForContactContentTypes(
    IPortableDeviceValues*  pObjectProperties)
{
    // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT because we are
    // creating/transferring contact content to the device.
    HRESULT hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_CONTACT);
    if (FAILED(hr))
    {
        printf("! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT, hr = 0x%lx\n",hr);
    }

    // Set the WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_VCARD2 because we are
    // creating/transferring contact content to the device. (This is Version 2 of
    // the VCARD file.  If you have Version 3, use WPD_OBJECT_FORMAT_VCARD3 as the
    // format)
    if (SUCCEEDED(hr))
    {
        hr = pObjectProperties->SetGuidValue(WPD_OBJECT_FORMAT, WPD_OBJECT_FORMAT_VCARD2);
        if (FAILED(hr))
        {
            printf("! Failed to set WPD_OBJECT_FORMAT to WPD_OBJECT_FORMAT_VCARD2, hr = 0x%lx\n",hr);
        }
    }

    return hr;
}

// Fills out the required properties for specific WPD content types.
HRESULT GetRequiredPropertiesForContentType(
    REFGUID                 ContentType,
    PCWSTR                  pszParentObjectID,
    PCWSTR                  pszFilePath,
    IStream*                pFileStream,
    IPortableDeviceValues** ppObjectProperties)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pObjectProperties));
    if (SUCCEEDED(hr))
    {
        if (pObjectProperties != NULL)
        {
            // Fill out required properties for ALL content types
            hr = GetRequiredPropertiesForAllContentTypes(pObjectProperties,
                                                         pszParentObjectID,
                                                         pszFilePath,
                                                         pFileStream);
            if (SUCCEEDED(hr))
            {
                // Fill out required properties for specific content types.
                // NOTE: If the content type is unknown to this function then
                // only the required properties will be written.  This is enough
                // for transferring most generic content types.
                //

                if (IsEqualGUID(ContentType, WPD_CONTENT_TYPE_IMAGE))
                {
                    hr = GetRequiredPropertiesForImageContentTypes(pObjectProperties);
                }
                else if (IsEqualGUID(ContentType, WPD_CONTENT_TYPE_AUDIO))
                {
                    hr = GetRequiredPropertiesForMusicContentTypes(pObjectProperties);
                }
                else if (IsEqualGUID(ContentType, WPD_CONTENT_TYPE_CONTACT))
                {
                    hr = GetRequiredPropertiesForContactContentTypes(pObjectProperties);
                }
            }
            else
            {
                printf("! Failed to get required properties common to ALL content types, hr = 0x%lx\n",hr);
            }

            // If everything was successful above, QI for the IPortableDeviceValues to return
            // to the caller.  A temporary CComPtr IPortableDeviceValues was used for easy cleanup
            // in case of a failure.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
                if (FAILED(hr))
                {
                    printf("! Failed to QueryInterface for IPortableDeviceValues, hr = 0x%lx\n",hr);
                }
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            printf("! Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\n",hr);
        }
    }

    return hr;
}

// Transfers a user selected file to the device
void TransferContentToDevice(
    IPortableDevice* pDevice,
    REFGUID          guidContentType,
    PCWSTR           pszFileTypeFilter,
    PCWSTR           pszDefaultFileExtension)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }
	//<SnippetContentTransfer1>
    HRESULT                             hr = S_OK;
    WCHAR                               szSelection[81]        = {0};
    WCHAR                               szFilePath[MAX_PATH]   = {0};
    DWORD                               cbOptimalTransferSize   = 0;
    CComPtr<IStream>                    pFileStream;
    CComPtr<IPortableDeviceDataStream>  pFinalObjectDataStream;
    CComPtr<IPortableDeviceValues>      pFinalObjectProperties;
    CComPtr<IPortableDeviceContent>     pContent;
    CComPtr<IStream>                    pTempStream;  // Temporary IStream which we use to QI for IPortableDeviceDataStream

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    printf("Enter the identifer of the parent object which the file will be transferred under.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content transfer\n");
    }
	//</SnippetContentTransfer1>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
	//<SnippetContentTransfer2>
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetContentTransfer2>
    // 2) Present the user with a File Open dialog.  Our sample is
    // restricting the types to user-specified forms.
	//<SnippetContentTransfer3>
    if (SUCCEEDED(hr))
    {
        OPENFILENAME OpenFileNameInfo   = {0};

        OpenFileNameInfo.lStructSize    = sizeof(OPENFILENAME);
        OpenFileNameInfo.hwndOwner      = NULL;
        OpenFileNameInfo.lpstrFile      = szFilePath;
        OpenFileNameInfo.nMaxFile       = ARRAYSIZE(szFilePath);
        OpenFileNameInfo.lpstrFilter    = pszFileTypeFilter;
        OpenFileNameInfo.nFilterIndex   = 1;
        OpenFileNameInfo.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        OpenFileNameInfo.lpstrDefExt    = pszDefaultFileExtension;

        if (GetOpenFileName(&OpenFileNameInfo) == FALSE)
        {
            printf("The transfer operation was cancelled.\n");
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
        hr = SHCreateStreamOnFile(szFilePath, STGM_READ, &pFileStream);
        if (SUCCEEDED(hr))
        {
            // Get the required properties needed to properly describe the data being
            // transferred to the device.
            hr = GetRequiredPropertiesForContentType(guidContentType,           // Content type of the data
                                                     szSelection,              // Parent to transfer the data under
                                                     szFilePath,               // Full file path to the data file
                                                     pFileStream,               // Open IStream that contains the data
                                                     &pFinalObjectProperties);  // Returned properties describing the data
            if (FAILED(hr))
            {
                printf("! Failed to get required properties needed to transfer a file to the device, hr = 0x%lx\n", hr);
            }
        }

        if (FAILED(hr))
        {
            printf("! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n",szFilePath, hr);
        }
    }
	//</SnippetContentTransfer4>
	//<SnippetContentTransfer5>
    // 4) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = pContent->CreateObjectWithPropertiesAndData(pFinalObjectProperties,    // Properties describing the object data
                                                         &pTempStream,              // Returned object data stream (to transfer the data to)
                                                         &cbOptimalTransferSize,    // Returned optimal buffer size to use during transfer
                                                         NULL);

        // Once we have a the IStream returned from CreateObjectWithPropertiesAndData,
        // QI for IPortableDeviceDataStream so we can use the additional methods
        // to get more information about the object (i.e. The newly created object
        // identifier on the device)
        if (SUCCEEDED(hr))
        {
            hr = pTempStream->QueryInterface(IID_PPV_ARGS(&pFinalObjectDataStream));
            if (FAILED(hr))
            {
                printf("! Failed to QueryInterface for IPortableDeviceDataStream, hr = 0x%lx\n",hr);
            }
        }

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (SUCCEEDED(hr))
        {
            DWORD cbTotalBytesWritten = 0;

            hr = StreamCopy(pFinalObjectDataStream, // Destination (The Object to transfer to)
                            pFileStream,            // Source (The File data to transfer from)
                            cbOptimalTransferSize,  // The driver specified optimal transfer buffer size
                            &cbTotalBytesWritten);  // The total number of bytes transferred from file to the device
            if (FAILED(hr))
            {
                printf("! Failed to transfer object to device, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to get IStream (representing destination object data on the device) from IPortableDeviceContent, hr = 0x%lx\n",hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = pFinalObjectDataStream->Commit(0);
            if (FAILED(hr))
            {
                printf("! Failed to commit object to device, hr = 0x%lx\n",hr);
            }
        }

        // Some clients may want to know the object identifier of the newly created
        // object.  This is done by calling GetObjectID() method on the
        // IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            PWSTR pszNewlyCreatedObject = NULL;
            hr = pFinalObjectDataStream->GetObjectID(&pszNewlyCreatedObject);
            if (SUCCEEDED(hr))
            {
                printf("The file '%ws' was transferred to the device.\nThe newly created object's ID is '%ws'\n",szFilePath ,pszNewlyCreatedObject);
            }

            if (FAILED(hr))
            {
                printf("! Failed to get the newly transferred object's identifier from the device, hr = 0x%lx\n",hr);
            }

            // Free the object identifier string returned from the GetObjectID() method.
            CoTaskMemFree(pszNewlyCreatedObject);
            pszNewlyCreatedObject = NULL;
        }
    }
	//</SnippetContentTransfer5>
}

// Transfers a user selected file to the device as a new WPD_RESOURCE_CONTACT_PHOTO resource
void CreateContactPhotoResourceOnDevice(
    IPortableDevice* pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                             hr = S_OK;
    WCHAR                               szSelection[81]        = {0};
    WCHAR                               szFilePath[MAX_PATH]   = {0};
    DWORD                               cbOptimalTransferSize   = 0;
    CComPtr<IStream>                    pFileStream;
    CComPtr<IStream>                    pResourceStream;
    CComPtr<IPortableDeviceValues>      pResourceAttributes;
    CComPtr<IPortableDeviceContent>     pContent;
    CComPtr<IPortableDeviceResources>   pResources;

    // Prompt user to enter an object identifier for the object to which we will add a Resource.
    printf("Enter the identifer of the object to which we will add a photograph.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting resource creation\n");
    }

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 2) Get an IPortableDeviceResources interface from the IPortableDeviceContent to
    // access the resource-specific  methods.
    if (SUCCEEDED(hr))
    {
        hr = pContent->Transfer(&pResources);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceResources from IPortableDeviceContent, hr = 0x%lx\n",hr);
        }
    }

    // 3) Present the user with a File Open dialog.  Our sample is
    // restricting the types to user-specified forms.
    if (SUCCEEDED(hr))
    {
        OPENFILENAME OpenFileNameInfo   = {0};

        OpenFileNameInfo.lStructSize    = sizeof(OPENFILENAME);
        OpenFileNameInfo.hwndOwner      = NULL;
        OpenFileNameInfo.lpstrFile      = szFilePath;
        OpenFileNameInfo.nMaxFile       = ARRAYSIZE(szFilePath);
        OpenFileNameInfo.lpstrFilter    = L"JPEG (*.JPG)\0*.JPG\0JPEG (*.JPEG)\0*.JPEG\0JPG (*.JPE)\0*.JPE\0JPG (*.JFIF)\0*.JFIF\0\0";;
        OpenFileNameInfo.nFilterIndex   = 1;
        OpenFileNameInfo.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        OpenFileNameInfo.lpstrDefExt    = L"JPG";

        if (GetOpenFileName(&OpenFileNameInfo) == FALSE)
        {
            printf("The transfer operation was cancelled.\n");
            hr = E_ABORT;
        }
    }

    // 4) Open the file and add required properties about the resource being transferred
    if (SUCCEEDED(hr))
    {
        // Open the selected file as an IStream.  This will simplify reading the
        // data and writing to the device.
        hr = SHCreateStreamOnFile(szFilePath, STGM_READ, &pFileStream);
        if (SUCCEEDED(hr))
        {
            // CoCreate the IPortableDeviceValues to hold the resource attributes
            hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pResourceAttributes));
            if (SUCCEEDED(hr))
            {
                // Fill in the necessary information regarding this resource

                // Set the WPD_OBJECT_ID.  This informs the driver which object this request is intended for.
                hr = pResourceAttributes->SetStringValue(WPD_OBJECT_ID, szSelection);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_ID when creating a resource, hr = 0x%lx\n",hr);
                }

                // Set the WPD_RESOURCE_ATTRIBUTE_RESOURCE_KEY to WPD_RESOURCE_CONTACT_PHOTO
                if (SUCCEEDED(hr))
                {
                    hr = pResourceAttributes->SetKeyValue(WPD_RESOURCE_ATTRIBUTE_RESOURCE_KEY, WPD_RESOURCE_CONTACT_PHOTO);
                    if (FAILED(hr))
                    {
                        printf("! Failed to set WPD_RESOURCE_ATTRIBUTE_RESOURCE_KEY to WPD_RESOURCE_CONTACT_PHOTO, hr = 0x%lx\n",hr);
                    }
                }

                // Set the WPD_RESOURCE_ATTRIBUTE_TOTAL_SIZE by requesting the total size of the
                // data stream.
                if (SUCCEEDED(hr))
                {
                    STATSTG statstg = {0};
                    hr = pFileStream->Stat(&statstg, STATFLAG_NONAME);
                    if (SUCCEEDED(hr))
                    {
                        hr = pResourceAttributes->SetUnsignedLargeIntegerValue(WPD_RESOURCE_ATTRIBUTE_TOTAL_SIZE, statstg.cbSize.QuadPart);
                        if (FAILED(hr))
                        {
                            printf("! Failed to set WPD_RESOURCE_ATTRIBUTE_TOTAL_SIZE, hr = 0x%lx\n",hr);
                        }
                    }
                    else
                    {
                        printf("! Failed to get file's total size, hr = 0x%lx\n",hr);
                    }
                }

                // Set the WPD_RESOURCE_ATTRIBUTE_FORMAT to WPD_OBJECT_FORMAT_EXIF because we are
                // creating a contact photo resource with JPG image data.
                if (SUCCEEDED(hr))
                {
                    hr = pResourceAttributes->SetGuidValue(WPD_RESOURCE_ATTRIBUTE_FORMAT, WPD_OBJECT_FORMAT_EXIF);
                    if (FAILED(hr))
                    {
                        printf("! Failed to set WPD_RESOURCE_ATTRIBUTE_FORMAT to WPD_OBJECT_FORMAT_EXIF, hr = 0x%lx\n",hr);
                    }
                }
            }

        }

        if (FAILED(hr))
        {
            printf("! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n",szFilePath, hr);
        }
    }

    // 5) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = pResources->CreateResource(pResourceAttributes,    // Properties describing this resource
                                        &pResourceStream,       // Returned resource data stream (to transfer the data to)
                                        &cbOptimalTransferSize, // Returned optimal buffer size to use during transfer
                                        NULL);


        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (SUCCEEDED(hr))
        {
            DWORD cbTotalBytesWritten = 0;

            hr = StreamCopy(pResourceStream,        // Destination (The resource to transfer to)
                            pFileStream,            // Source (The File data to transfer from)
                            cbOptimalTransferSize,  // The driver specified optimal transfer buffer size
                            &cbTotalBytesWritten);  // The total number of bytes transferred from file to the device
            if (FAILED(hr))
            {
                printf("! Failed to transfer object to device, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to get IStream (representing destination object data on the device) from IPortableDeviceContent, hr = 0x%lx\n",hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = pResourceStream->Commit(0);
            if (FAILED(hr))
            {
                printf("! Failed to commit resource to device, hr = 0x%lx\n",hr);
            }
        }
    }
}

// Fills out the required properties for a properties-only
// contact named "John Kane".  This is a hard-coded
// contact.
HRESULT GetRequiredPropertiesForPropertiesOnlyContact(
    PCWSTR                  pszParentObjectID,
    IPortableDeviceValues** ppObjectProperties)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pObjectProperties));
    if (SUCCEEDED(hr))
    {
        if (pObjectProperties != NULL)
        {
            // Set the WPD_OBJECT_PARENT_ID
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, pszParentObjectID);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\n",hr);
                }
            }

            // Set the WPD_OBJECT_NAME.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, L"John Kane");
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n",hr);
                }
            }

            // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT because we are
            // creating contact content on the device.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_CONTACT);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_CONTACT, hr = 0x%lx\n",hr);
                }
            }

            // Set the WPD_CONTACT_DISPLAY_NAME to "John Kane"
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetStringValue(WPD_CONTACT_DISPLAY_NAME, L"John Kane");
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_CONTACT_DISPLAY_NAME, hr = 0x%lx\n",hr);
                }
            }

            // Set the WPD_CONTACT_PRIMARY_PHONE to "425-555-0123"
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetStringValue(WPD_CONTACT_PRIMARY_PHONE, L"425-555-0123");
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_CONTACT_PRIMARY_PHONE, hr = 0x%lx\n",hr);
                }
            }

            // If everything was successful above, QI for the IPortableDeviceValues to return
            // to the caller.  A temporary CComPtr IPortableDeviceValues was used for easy cleanup
            // in case of a failure.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
                if (FAILED(hr))
                {
                    printf("! Failed to QueryInterface for IPortableDeviceValues, hr = 0x%lx\n",hr);
                }
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            printf("! Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\n",hr);
        }
    }

    return hr;
}

HRESULT GetRequiredPropertiesForFolder(
    PCWSTR                  pszParentObjectID,
    PCWSTR                  pszFolderName,
    IPortableDeviceValues** ppObjectProperties)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pObjectProperties));
    if (SUCCEEDED(hr))
    {
        if (pObjectProperties != NULL)
        {
            // Set the WPD_OBJECT_PARENT_ID
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, pszParentObjectID);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\n",hr);
                }
            }

            // Set the WPD_OBJECT_NAME.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, pszFolderName);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n",hr);
                }
            }

            // Set the WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_FOLDER because we are
            // creating contact content on the device.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_FOLDER);
                if (FAILED(hr))
                {
                    printf("! Failed to set WPD_OBJECT_CONTENT_TYPE to WPD_CONTENT_TYPE_FOLDER, hr = 0x%lx\n",hr);
                }
            }

            // If everything was successful above, QI for the IPortableDeviceValues to return
            // to the caller.  A temporary CComPtr IPortableDeviceValues was used for easy cleanup
            // in case of a failure.
            if (SUCCEEDED(hr))
            {
                hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
                if (FAILED(hr))
                {
                    printf("! Failed to QueryInterface for IPortableDeviceValues, hr = 0x%lx\n",hr);
                }
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            printf("! Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\n",hr);
        }
    }

    return hr;
}

// Creates a properties-only object on the device which is
// WPD_CONTENT_TYPE_CONTACT specific.
// NOTE: This function creates a hard-coded contact for
// "John Doe" always.
void TransferContactToDevice(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }
	//<SnippetTransfer1>
    HRESULT                             hr = S_OK;
    WCHAR                               szSelection[81]        = {0};
    CComPtr<IPortableDeviceValues>      pFinalObjectProperties;
    CComPtr<IPortableDeviceContent>     pContent;

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    printf("Enter the identifer of the parent object which the contact will be transferred under.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content transfer\n");
    }
	//</SnippetTransfer1>
    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
	//<SnippetTransfer2>
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }
	//</SnippetTransfer2>
    // 2) Get the properties that describe the object being created on the device

    if (SUCCEEDED(hr))
    {
        hr = GetRequiredPropertiesForPropertiesOnlyContact(szSelection,              // Parent to transfer the data under
                                                           &pFinalObjectProperties);  // Returned properties describing the data
        if (FAILED(hr))
        {
            printf("! Failed to get required properties needed to transfer an image file to the device, hr = 0x%lx\n", hr);
        }
    }

    // 3) Transfer the content to the device by creating a properties-only object
	//<SnippetTransfer3>
    if (SUCCEEDED(hr))
    {
        PWSTR pszNewlyCreatedObject = NULL;
        hr = pContent->CreateObjectWithPropertiesOnly(pFinalObjectProperties,    // Properties describing the object data
                                                      &pszNewlyCreatedObject);
        if (SUCCEEDED(hr))
        {
            printf("The contact was transferred to the device.\nThe newly created object's ID is '%ws'\n",pszNewlyCreatedObject);
        }

        if (FAILED(hr))
        {
            printf("! Failed to transfer contact object to the device, hr = 0x%lx\n",hr);
        }

        // Free the object identifier string returned from CreateObjectWithPropertiesOnly
        CoTaskMemFree(pszNewlyCreatedObject);
        pszNewlyCreatedObject = NULL;
    }
	//</SnippetTransfer3>
}

// Creates a properties-only object on the device which is
// WPD_CONTENT_TYPE_FOLDER specific.
void CreateFolderOnDevice(
    IPortableDevice*    pDevice)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                             hr = S_OK;
    WCHAR                               szSelection[81]        = {0};
    WCHAR                               szFolderName[81]        = {0};
    CComPtr<IPortableDeviceValues>      pFinalObjectProperties;
    CComPtr<IPortableDeviceContent>     pContent;

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    printf("Enter the identifer of the parent object which the folder will be created under.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting folder creation\n");
    }

    // Prompt user to enter an object identifier for the parent object on the device to transfer.
    printf("Enter the name of the the folder to create.\n>");
    hr = StringCbGetsW(szFolderName,sizeof(szFolderName));
    if (FAILED(hr))
    {
        printf("An invalid folder name was specified, aborting folder creation\n");
    }

    // 1) Get an IPortableDeviceContent interface from the IPortableDevice interface to
    // access the content-specific methods.
    if (SUCCEEDED(hr))
    {
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
    }

    // 2) Get the properties that describe the object being created on the device
    if (SUCCEEDED(hr))
    {
        hr = GetRequiredPropertiesForFolder(szSelection,              // Parent to create the folder under
                                            szFolderName,             // Folder Name
                                            &pFinalObjectProperties);  // Returned properties describing the folder
        if (FAILED(hr))
        {
            printf("! Failed to get required properties needed to transfer an image file to the device, hr = 0x%lx\n", hr);
        }
    }

    // 3) Transfer the content to the device by creating a properties-only object
    if (SUCCEEDED(hr))
    {
        PWSTR pszNewlyCreatedObject = NULL;
        hr = pContent->CreateObjectWithPropertiesOnly(pFinalObjectProperties,    // Properties describing the object data
                                                      &pszNewlyCreatedObject);
        if (SUCCEEDED(hr))
        {
            printf("The folder was created on the device.\nThe newly created object's ID is '%ws'\n",pszNewlyCreatedObject);
        }

        if (FAILED(hr))
        {
            printf("! Failed to create a new folder on the device, hr = 0x%lx\n",hr);
        }

        // Free the object identifier string returned from CreateObjectWithPropertiesOnly
        CoTaskMemFree(pszNewlyCreatedObject);
        pszNewlyCreatedObject = NULL;
    }

}

// Fills out properties that accompany updating an object's data...
HRESULT GetPropertiesForUpdateData(
    PCWSTR                 pszFilePath,
    IStream*                pFileStream,
    IPortableDeviceValues** ppObjectProperties)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;

    // CoCreate an IPortableDeviceValues interface to hold the the object information
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pObjectProperties));

    if (SUCCEEDED(hr))
    {
        // Set the WPD_OBJECT_SIZE by requesting the total size of the
        // data stream.
        STATSTG statstg = {0};
        hr = pFileStream->Stat(&statstg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
            if (FAILED(hr))
            {
                printf("! Failed to set WPD_OBJECT_SIZE, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to get file's total size, hr = 0x%lx\n",hr);
        }

        // Set the WPD_OBJECT_ORIGINAL_FILE_NAME by splitting the file path
        // into a separate filename.
        WCHAR szFileName[MAX_PATH] = {0};
        WCHAR szFileExt[MAX_PATH]  = {0};
        if (_wsplitpath_s(pszFilePath, NULL,0,NULL,0,
                           szFileName,ARRAYSIZE(szFileName),
                           szFileExt, ARRAYSIZE(szFileExt)))
        {
            hr = E_INVALIDARG;
            printf("! Failed to split the file path, hr = 0x%lx\n",hr);
        }

        if (SUCCEEDED(hr))
        {
            CAtlStringW strOriginalFileName;
            strOriginalFileName.Format(L"%ws%ws", szFileName, szFileExt);
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, strOriginalFileName);
            if (FAILED(hr))
            {
                printf("! Failed to set WPD_OBJECT_ORIGINAL_FILE_NAME, hr = 0x%lx\n",hr);
            }
        }

        // Set the WPD_OBJECT_NAME.  We are using the  file name without its file extension in this
        // example for the object's name.  The object name could be a more friendly name like
        // "This Cool Song" or "That Cool Picture".
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, szFileName);
            if (FAILED(hr))
            {
                printf("! Failed to set WPD_OBJECT_NAME, hr = 0x%lx\n",hr);
            }
        }


        // If everything was successful above, QI for the IPortableDeviceValues to return
        // to the caller.  A temporary CComPtr IPortableDeviceValues was used for easy cleanup
        // in case of a failure.
        if (SUCCEEDED(hr))
        {
            hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
            if (FAILED(hr))
            {
                printf("! Failed to QueryInterface for IPortableDeviceValues, hr = 0x%lx\n",hr);
            }
        }
    }
    
    return hr;
}


// Updates a selected object's properties and data (WPD_RESOURCE_DEFAULT).  
void UpdateContentOnDevice(
    IPortableDevice* pDevice,
    REFGUID          ContentType,
    PCWSTR          pszFileTypeFilter,
    PCWSTR          pszDefaultFileExtension)
{
    if (pDevice == NULL)
    {
        printf("! A NULL IPortableDevice interface pointer was received\n");
        return;
    }

    HRESULT                             hr = S_OK;
    WCHAR                               szSelection[81]        = {0};
    WCHAR                               szFilePath[MAX_PATH]   = {0};
    DWORD                               cbOptimalTransferSize  = 0;
    CComPtr<IStream>                    pFileStream;
    CComPtr<IPortableDeviceDataStream>  pFinalObjectDataStream;
    CComPtr<IPortableDeviceValues>      pFinalObjectProperties;
    CComPtr<IPortableDeviceContent2>    pContent2;
    CComPtr<IStream>                    pTempStream;  // Temporary IStream which we use to QI for IPortableDeviceDataStream

    // Prompt user to enter an object identifier for the object on the device to update.
    printf("Enter the identifer of the object to update.\n>");
    hr = StringCbGetsW(szSelection,sizeof(szSelection));
    if (FAILED(hr))
    {
        printf("An invalid object identifier was specified, aborting content update\n");
    }

    // 1) Get an IPortableDeviceContent2 interface from the IPortableDevice interface to
    // access the UpdateObjectWithPropertiesAndData method.
    if (SUCCEEDED(hr))
    {
        CComPtr<IPortableDeviceContent> pContent;
    
        hr = pDevice->Content(&pContent);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceContent from IPortableDevice, hr = 0x%lx\n",hr);
        }
        else
        {
            hr = pContent->QueryInterface(IID_PPV_ARGS(&pContent2));
            if (FAILED(hr))
            {
                printf("! Failed to get IPortableDeviceContent2 from IPortableDeviceContent, hr = 0x%lx\n",hr);
            }
        }
    }

    // 2) (Optional) Check if the object is of the correct content type. This also ensures the user-specified object ID is valid.
    if (SUCCEEDED(hr))
    {
        CComPtr<IPortableDeviceProperties> pProperties;

        hr = pContent2->Properties(&pProperties);
        if (FAILED(hr))
        {
            printf("! Failed to get IPortableDeviceProperties from IPortableDeviceContent2, hr = 0x%lx\n",hr);
        }
        else
        {
            CComPtr<IPortableDeviceValues> pObjectProperties;

            hr = pProperties->GetValues(szSelection, NULL, &pObjectProperties);
            if (FAILED(hr))
            {
                printf("! Failed to get all properties for object (%ws), hr = 0x%lx\n", szSelection, hr);
            }
            else
            {
                GUID objectContentType;
                hr = pObjectProperties->GetGuidValue(WPD_OBJECT_CONTENT_TYPE, &objectContentType);
                if (FAILED(hr))
                {
                    printf("! Failed to get WPD_OBJECT_CONTENT_TYPE for object (%ws), hr = 0x%lx\n", szSelection, hr);
                }
                else if (objectContentType != ContentType)
                {
                    hr = E_INVALIDARG;
                    printf("! Object (%ws) is not of the correct content type, hr = 0x%lx\n", szSelection, hr);
                }
            }
        }
    }

    // 3) Present the user with a File Open dialog.  Our sample is
    // restricting the types to user-specified forms.
    if (SUCCEEDED(hr))
    {
        OPENFILENAME OpenFileNameInfo   = {0};

        OpenFileNameInfo.lStructSize    = sizeof(OPENFILENAME);
        OpenFileNameInfo.hwndOwner      = NULL;
        OpenFileNameInfo.lpstrFile      = szFilePath;
        OpenFileNameInfo.nMaxFile       = ARRAYSIZE(szFilePath);
        OpenFileNameInfo.lpstrFilter    = pszFileTypeFilter;
        OpenFileNameInfo.nFilterIndex   = 1;
        OpenFileNameInfo.Flags          = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        OpenFileNameInfo.lpstrDefExt    = pszDefaultFileExtension;

        if (GetOpenFileName(&OpenFileNameInfo) == FALSE)
        {
            printf("The update operation was cancelled.\n");
            hr = E_ABORT;
        }
    }

    // 4) Open the file and add required properties about the file being transferred
    if (SUCCEEDED(hr))
    {
        // Open the selected file as an IStream.  This will simplify reading the
        // data and writing to the device.
        hr = SHCreateStreamOnFile(szFilePath, STGM_READ, &pFileStream);
        if (SUCCEEDED(hr))
        {
            // Get the required properties needed to properly describe the data being
            // transferred to the device.
            hr = GetPropertiesForUpdateData(szFilePath,               // Full file path to the data file
                                            pFileStream,               // Open IStream that contains the data
                                            &pFinalObjectProperties);  // Returned properties describing the data

            if (FAILED(hr))
            {
                printf("! Failed to get properties needed to transfer a file to the device, hr = 0x%lx\n", hr);
            }
        }

        if (FAILED(hr))
        {
            printf("! Failed to open file named (%ws) to transfer to device, hr = 0x%lx\n",szFilePath, hr);
        }
    }

    // 5) Transfer for the content to the device
    if (SUCCEEDED(hr))
    {
        hr = pContent2->UpdateObjectWithPropertiesAndData(szSelection,
                                                         pFinalObjectProperties,    // Properties describing the object data
                                                         &pTempStream,              // Returned object data stream (to transfer the data to)
                                                         &cbOptimalTransferSize);   // Returned optimal buffer size to use during transfer

        // Once we have a the IStream returned from UpdateObjectWithPropertiesAndData,
        // QI for IPortableDeviceDataStream so we can use the additional methods
        // to get more information about the object (i.e. The newly created object
        // identifier on the device)
        if (SUCCEEDED(hr))
        {
            hr = pTempStream->QueryInterface(IID_PPV_ARGS(&pFinalObjectDataStream));
            if (FAILED(hr))
            {
                printf("! Failed to QueryInterface for IPortableDeviceDataStream, hr = 0x%lx\n",hr);
            }
        }

        // Since we have IStream-compatible interfaces, call our helper function
        // that copies the contents of a source stream into a destination stream.
        if (SUCCEEDED(hr))
        {
            DWORD cbTotalBytesWritten = 0;

            hr = StreamCopy(pFinalObjectDataStream, // Destination (The Object to transfer to)
                            pFileStream,            // Source (The File data to transfer from)
                            cbOptimalTransferSize,  // The driver specified optimal transfer buffer size
                            &cbTotalBytesWritten);  // The total number of bytes transferred from file to the device
            if (FAILED(hr))
            {
                printf("! Failed to transfer object data to device, hr = 0x%lx\n",hr);
            }
        }
        else
        {
            printf("! Failed to get IStream (representing destination object data on the device) from UpdateObjectWithPropertiesAndData, hr = 0x%lx\n",hr);
        }

        // After transferring content to the device, the client is responsible for letting the
        // driver know that the transfer is complete by calling the Commit() method
        // on the IPortableDeviceDataStream interface.
        if (SUCCEEDED(hr))
        {
            hr = pFinalObjectDataStream->Commit(0);
            if (SUCCEEDED(hr))
            {
                printf("The file '%ws' was transferred to the device to object '%ws'\n",szFilePath,szSelection);
            }
            else
            {
                printf("! Failed to commit new object data to device, hr = 0x%lx\n",hr);
            }
        }
    }
}
