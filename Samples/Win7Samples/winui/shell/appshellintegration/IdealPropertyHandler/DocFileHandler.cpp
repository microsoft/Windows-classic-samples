// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "dll.h"
#include "RegisterExtension.h"

const char c_szEmptyDocFileBase64[] =
"0M8R4KGxGuEAAAAAAAAAAAAAAAAAAAAAPgADAP7/CQAGAAAAAAAAAAAAAAABAAAA"
"AQAAAAAAAAAAEAAA/v///wAAAAD+////AAAAAAAAAAD/////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"///////////////////////////////////////////9/////v//////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////"
"/////////////////////1IAbwBvAHQAIABFAG4AdAByAHkAAAAAAAAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAWAAUA////////////////"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/v///wAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAD///////////////8AAAAAAAAAAAAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"AAAAAP///////////////wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA////////////////"
"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

const WCHAR c_szDocFileExtension[] = L".docfile-ms";
const WCHAR c_szDocFileProgID[] = L"Windows.DocFile";
const WCHAR c_szDocFileDescription[] = L"DocFile File";

HRESULT RegisterDocFile()
{
    // populate missing object registration.
    CRegisterExtension re(__uuidof(DocFilePropertyHandler), HKEY_LOCAL_MACHINE);

    // Property Handler and Kind registrations use a different mechanism than the rest of the filetype association system, and do not use ProgIDs
    HRESULT hr = re.RegisterPropertyHandler(c_szDocFileExtension);
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterKind(c_szDocFileExtension, L"document");
    }

    // Associate our ProgID with the file extension, and write the remainder of the registration data to the ProgID to minimize conflicts with other applications and facilitate easy unregistration
    if (SUCCEEDED(hr))
    {
        hr = re.RegisterExtensionWithProgID(c_szDocFileExtension, c_szDocFileProgID);
        if (SUCCEEDED(hr))
        {
            hr = re.RegisterProgID(c_szDocFileProgID, c_szDocFileDescription, IDI_ICON_DOCFILE);
            if (SUCCEEDED(hr))
            {
                hr = re.RegisterNewMenuData(c_szDocFileExtension, c_szDocFileProgID, c_szEmptyDocFileBase64);
                if (SUCCEEDED(hr))
                {
                    hr = re.RegisterProgIDValue(c_szDocFileProgID, L"NoOpen", L"This is a sample file type and does not have any apps installed to handle it");
                    if (SUCCEEDED(hr))
                    {
                        hr = re.RegisterProgIDValue(c_szDocFileProgID, L"FullDetails", c_szDocFullDetails);
                        if (SUCCEEDED(hr))
                        {
                            hr = re.RegisterProgIDValue(c_szDocFileProgID, L"InfoTip", c_szDocInfoTip);
                            if (SUCCEEDED(hr))
                            {
                                hr = re.RegisterProgIDValue(c_szDocFileProgID, L"PreviewDetails", c_szDocPreviewDetails);
                            }
                        }
                    }
                }
            }
        }
    }

    // also register the property-driven thumbnail handler on the ProgID
    if (SUCCEEDED(hr))
    {
        re.SetHandlerCLSID(__uuidof(PropertyThumbnailHandler));
        hr = re.RegisterThumbnailHandler(c_szDocFileProgID);
    }
    return hr;
}

HRESULT UnregisterDocFile()
{
    CRegisterExtension re(__uuidof(DocFilePropertyHandler), HKEY_LOCAL_MACHINE);

    // Remove the whole ProgID since we own all of those settings.
    // Don't try to remove the file extension association since some other application may have overridden it with their own ProgID in the meantime.
    // Leaving the association to a non-existing ProgID is handled gracefully by the Shell.
    // NOTE: If the file extension is unambiguously owned by this application, the association to the ProgID could be safely removed as well,
    //       along with any other association data stored on the file extension itself.
    HRESULT hr = re.UnRegisterProgID(c_szDocFileProgID, c_szDocFileExtension);
    if (SUCCEEDED(hr))
    {
        // Remove Property Handler and Kind registrations from their respective locations as well.
        hr = re.UnRegisterPropertyHandler(c_szDocFileExtension);
        if (SUCCEEDED(hr))
        {
            hr = re.UnRegisterKind(c_szDocFileExtension);
        }
    }
    return hr;
}
