//+--------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//      Class method definitions of WCS color samples
//
//----------------------------------------------------------------------------

#include "precomp.h"
#include "ColorSamples.h"
#include "Utilities.h"
#include "Resource.h"

//
// The required bfType value (equal to 'BM')
// in BITMAPFILEHEADER structures
//
#define BFT_BITMAP 0x4d42

//+---------------------------------------------------------------------------
//
//  Member:
//      IWcsDemo::LoadsRGBImage
//
//  Synopsis:
//      This method loads an sRGB image from resources. It provides a pointer
//      to the bitmap data.
//
//      This is not intended for generalized use or templating, and works
//      with the assumption of the known properties of the specific images
//      compiled as resources in the executable.
//
//      The caller must *not* free any returned data.  The resource
//      memory is freed when the application terminates.
//
//----------------------------------------------------------------------------
HRESULT
IWcsDemo::LoadsRGBImage(
   __deref_out_bcount(*pcbBitmapData) BYTE **ppaBitmapData,
    // Will contain the pointer to the data buffer, caller must not free
    __out DWORD *pcbBitmapData,
    // Size of data buffer returned
    __out DWORD *pdwWidth,
    // Image width
    __out DWORD *pdwHeight,
    // Image height
    __out DWORD *pdwHorizontalResolution,
    // Image horizontal resolution
    __out DWORD *pdwVerticalResolution
    // Image vertical resolution
    )
{
    HRESULT hr = S_OK;
    if (!ppaBitmapData || !pcbBitmapData || !pdwWidth ||
         !pdwHeight || !pdwHorizontalResolution || !pdwVerticalResolution)
    {
        return E_INVALIDARG;
    }

    HRSRC hFind = NULL;
    HGLOBAL hResource = NULL;
    BYTE *pBitmapResourceData = NULL;
    DWORD cbBitmapResourceData = 0;
    BITMAPFILEHEADER *pBitmapFileHeader = NULL;
    BITMAPINFOHEADER *pBitmapInfoHeader = NULL;

    //
    // Find the resource for the image
    //
    hFind = FindResource(NULL, MAKEINTRESOURCE(IDR_BITMAP_SRGBIMAGE), L"SAMPLEIMAGE");
    if (!hFind)
    {
        hr = HRESULTFromLastError();
    }
    if (SUCCEEDED(hr))
    {
        hResource = LoadResource(NULL, hFind);
        if (!hResource)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Look up and check the size of the image
        //
        cbBitmapResourceData = SizeofResource(NULL, hFind);
        if (0 == cbBitmapResourceData)
        {
            hr = HRESULTFromLastError();
        }
        if (SUCCEEDED(hr))
        {
            //
            // Check that the size of the resource is large enough that
            // it will not overrun the buffer to read the headers.
            //
            if (cbBitmapResourceData < sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER))
            {
                hr = E_FAIL;
            }
        }
    }
    if (SUCCEEDED(hr))
    {

        //
        // Lock the resource and store its pointer
        //
        pBitmapResourceData = (BYTE*)LockResource(hResource);
        if (!pBitmapResourceData)
        {
            hr = HRESULTFromLastError();
        }
        pBitmapFileHeader = (BITMAPFILEHEADER*) pBitmapResourceData;
    }
    if (SUCCEEDED(hr))
    {
        //
        // This sample does not support generalized bitmap loading.
        // Check that the input image is valid and indeed matches the
        // assumptions that are made:
        // 1. The data is really a bitmap
        // 2. It's header type is BITMAPINFOHEADER
        // 3. It is 24bpp
        // 4. There is no compression
        // 5. There are no color tables
        //
        pBitmapInfoHeader = (BITMAPINFOHEADER*) (((PBYTE)pBitmapFileHeader)+sizeof(BITMAPFILEHEADER));
        if ((BFT_BITMAP != pBitmapFileHeader->bfType) ||
             (sizeof(BITMAPINFOHEADER) != pBitmapInfoHeader->biSize) ||
              (24 != pBitmapInfoHeader->biBitCount) ||
               (BI_RGB != pBitmapInfoHeader->biCompression) ||
                (0 != pBitmapInfoHeader->biClrUsed))
        {
            hr = E_UNEXPECTED;
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Get the header data from the bitmap
        //
        *pdwWidth = pBitmapInfoHeader->biWidth;

        //
        // Take the absolute value of the height, because it may
        // be negative depending if the image is "top-down" or not.
        //
        *pdwHeight = (DWORD)abs(pBitmapInfoHeader->biHeight);
        *pdwHorizontalResolution = pBitmapInfoHeader->biXPelsPerMeter;
        *pdwVerticalResolution = pBitmapInfoHeader->biYPelsPerMeter;

        //
        // Get the pixel data from the bitmap
        //
        DWORD cbPixelDataOffset = pBitmapFileHeader->bfOffBits;

        //
        // Check for buffer overflow
        //
        if (cbPixelDataOffset >= cbBitmapResourceData)
        {
            hr = E_FAIL;
        }
        if (SUCCEEDED(hr))
        {
            *ppaBitmapData = ((BYTE *)pBitmapFileHeader) + cbPixelDataOffset;
            *pcbBitmapData = cbBitmapResourceData - cbPixelDataOffset;
        }
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      IWcsDemo::SaveImage
//
//  Synopsis:
//      This method saves an RGB image to disk given the pixel data and
//      information about the horizontal and vertical extents and resolutions
//      of the image.
//
//----------------------------------------------------------------------------
HRESULT
IWcsDemo::SaveImage(
    __in PCWSTR pwszImageFilename,
    // Filename to write to disk
    __in SampleBitmapType BitmapType,
    // Bitmap data type
     __in_bcount(cbBitmapData) BYTE *paBitmapData,
    // Bitmap data
    __in DWORD cbBitmapData,
    // Size in bytes of Bitmap Data
    __in DWORD dwWidth,
    // Image width
    __in DWORD dwHeight,
    // Image height
    __in DWORD dwHorizontalResolution,
    // Image horizontal resolution
    __in DWORD dwVerticalResolution
    // Image vertical resolution
    )
{
    HRESULT hr = S_OK;
    if (!pwszImageFilename || !paBitmapData)
    {
        return E_INVALIDARG;
    }

    if (BasicRGBTripletBitmap != BitmapType)
    {
        return E_INVALIDARG;
    }

    HANDLE hFile = CreateFile(
                        pwszImageFilename,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = HRESULTFromLastError();
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwWrite = 0;
        BITMAPFILEHEADER BasicBitmapFileHeader =
        {
            BFT_BITMAP,
            sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+cbBitmapData,
            0,
            0,
            sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)
        };
        BITMAPINFOHEADER BasicBitmapInfoHeader =
        {
            sizeof(BITMAPINFOHEADER),
            dwWidth,
            dwHeight,
            1,
            24,
            BI_RGB,
            sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+cbBitmapData,
            dwHorizontalResolution,
            dwVerticalResolution,
            0,
            0
        };

        //
        // Write the bitmap header data
        //
        if (!WriteFile(hFile,
             (LPCVOID)&BasicBitmapFileHeader,
              sizeof(BITMAPFILEHEADER),
               &dwWrite,
                NULL))
        {
            hr = HRESULTFromLastError();
        }
        if (SUCCEEDED(hr) && (sizeof(BITMAPFILEHEADER) != dwWrite))
        {
            hr = E_FAIL;
        }
        if (SUCCEEDED(hr))
        {
            //
            // Write the Bitmap Info header data
            //
            if (!WriteFile(hFile,
                 (LPCVOID)&BasicBitmapInfoHeader,
                  sizeof(BITMAPINFOHEADER),
                   &dwWrite,
                    NULL))
            {
                hr = HRESULTFromLastError();
            }
            if (SUCCEEDED(hr) && (sizeof(BITMAPINFOHEADER) != dwWrite))
            {
                hr = E_FAIL;
            }
        }
        if (SUCCEEDED(hr))
        {
            //
            // Write Bitmap bits.
            //
            if (!WriteFile(hFile,
                 paBitmapData,
                  cbBitmapData,
                   &dwWrite,
                    NULL))
            {
                hr = HRESULTFromLastError();
            }
        }
    }

    //
    // Cleanup
    //
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      IWcsDemo::SavePixelData
//
//  Synopsis:
//      This method saves raw pixel data to a file with the specified name
//
//----------------------------------------------------------------------------
HRESULT
IWcsDemo::SavePixelData(
    __in PCWSTR pwszFilename,
    // Filename to write to disk
     __in_bcount(cbData) BYTE *paData,
    // Data to write
    __in DWORD cbData
    // Size in bytes of Data
    )
{
    HRESULT hr = S_OK;
    if (!pwszFilename || !paData)
    {
        return E_INVALIDARG;
    }

    HANDLE hFile = CreateFile(
                        pwszFilename,
                        GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = HRESULTFromLastError();
    }
    if (SUCCEEDED(hr))
    {
        //
        // Write the data
        //
        DWORD dwWrite = 0;
        if (!WriteFile(hFile,
              paData,
              cbData,
               &dwWrite,
                NULL))
        {
            hr = HRESULTFromLastError();
        }
        if (SUCCEEDED(hr) && cbData != dwWrite)
        {
            hr = E_FAIL;
        }
    }

    //
    // Cleanup
    //
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      IWcsDemo::GetProfileFromResource
//
//  Synopsis:
//      This method builds a PROFILE structure for the profile with the given
//      ID. The caller must not free pProfileData in *pProfile.
//
//----------------------------------------------------------------------------
HRESULT IWcsDemo::GetProfileFromResource(
    __in ResourceProfileType ProfileID,
    // Which type of profile to load
    __out PROFILE *pProfile
    // PROFILE structure built
    )
{
    HRESULT hr = S_OK;
    if (!pProfile)
    {
        return E_INVALIDARG;
    }

    ZeroMemory(pProfile, sizeof(PROFILE));
    pProfile->dwType = PROFILE_MEMBUFFER;
    PTSTR pwszResourceName = NULL;
    HRSRC hFind = NULL;
    HGLOBAL hResource = NULL;
    switch (ProfileID)
    {
        //
        // Resolve the resource name
        //
        case SampleGmmpWithPlugin:
            pwszResourceName = MAKEINTRESOURCE(IDR_PROFILE_SAMPLEGMMPWITHPLUGIN);
            break;
        case SampleRGBPrinter:
            pwszResourceName = MAKEINTRESOURCE(IDR_PROFILE_SAMPLERGBPRINTER);
            break;
        case SampleCMYKPrinter:
            pwszResourceName = MAKEINTRESOURCE(IDR_PROFILE_SAMPLECMYKPRINTER);
            break;
        case SampleCRTProfileWithPlugin:
            pwszResourceName = MAKEINTRESOURCE(IDR_PROFILE_SAMPLECRTPROFILEWITHPLUGIN);
            break;
        default:
            //
            // This is not a known profile type
            //
            hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        //
        // Look up the resource data and size to build the PROFILE struct
        //
        hFind = FindResource(NULL, pwszResourceName, L"PROFILE");
        if (!hFind)
        {
            hr = HRESULTFromLastError();
         }
        if (SUCCEEDED(hr))
        {
            hResource = LoadResource(NULL, hFind);
            if (!hResource)
            {
                hr = HRESULTFromLastError();
            }
        }
        if (SUCCEEDED(hr))
        {
            pProfile->cbDataSize = SizeofResource(NULL, hFind);
            if (0 == pProfile->cbDataSize)
            {
                hr = HRESULTFromLastError();
            }
        }
        if (SUCCEEDED(hr))
        {
            pProfile->pProfileData = (LPSTR)LockResource(hResource);
            if (!pProfile->pProfileData)
            {
                hr = HRESULTFromLastError();
            }
        }
    }

    if (FAILED(hr))
    {
        ZeroMemory(pProfile, sizeof(PROFILE));
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::GetD50CAMP
//
//  Synopsis:
//      This method returns a PROFILE for the system default D50 CAMP
//
//----------------------------------------------------------------------------
PPROFILE IWcsDemo::GetD50CAMP()
{
    static WCHAR wszD50Profile[] = L"D50.camp";
    static PROFILE D50ViewingConditionProfile = { PROFILE_FILENAME, wszD50Profile, sizeof(wszD50Profile) };
    
    return &D50ViewingConditionProfile;

}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::GetD65CAMP
//
//  Synopsis:
//      This method returns a PROFILE for the system default D65 CAMP
//
//----------------------------------------------------------------------------
PPROFILE IWcsDemo::GetD65CAMP()
{
    static WCHAR wszD65Profile[] = L"D65.camp";
    static PROFILE D65ViewingConditionProfile = { PROFILE_FILENAME, wszD65Profile, sizeof(wszD65Profile) };
    
    return &D65ViewingConditionProfile;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::RunDemos
//
//  Synopsis:
//      This method simply launches the demos of this class
//
//----------------------------------------------------------------------------
HRESULT
CTransformCreationDemo::RunDemos()
{
    HRESULT hr = S_OK;

    hr = DemonstrateSystemGmmp();
    if (SUCCEEDED(hr))
    {
        hr = DemonstrateCustomPluginProfiles();
    }
    if (SUCCEEDED(hr))
    {
        hr = DemonstrateSystemDefaultIntent();
    }
    if (SUCCEEDED(hr))
    {
        hr = DemonstrateIccWcsMixedMode();
    }
    if (SUCCEEDED(hr))
    {
        hr = DemonstrateIccOnlyWcsTransform();
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::DemonstrateSystemGmmp
//
//  Synopsis:
//      This method demonstrates how to use a system GMMP color profile
//      to cause a specific gamut mapping algorithm to be used.
//
//      System WCS profiles are installed on Windows Vista and later operating
//      systems. Because they are system-protected, using them will
//      guarantee that a specific gamut mapping algorithm is invoked, such
//      as Sigmoid Gaussian Cusp Knee, Hue-Mapping, etc.
//
//----------------------------------------------------------------------------
HRESULT
CTransformCreationDemo::DemonstrateSystemGmmp()
{
    HRESULT hr = S_OK;

    wprintf(L"Demonstrating usage of system GMMP profiles\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateSystemGmmp Output.bmp";

    //
    // For demonstration purposes, do a simple transform from sRGB
    // to an RGB Printer that uses the the Photo gamut mapper.
    //
    // First, open WCS profile handles for those profiles that are needed.
    //
    const TCHAR sRGBWcsProfilename[] = L"wsRGB.cdmp";


    PROFILE InputProfile = {};
    PROFILE OutputProfile = {};
    PROFILE SystemPhotoGmmpProfile = {};

    InputProfile.pProfileData = (PVOID)sRGBWcsProfilename;
    InputProfile.cbDataSize = sizeof(sRGBWcsProfilename);
    InputProfile.dwType = PROFILE_FILENAME;

    hr = GetProfileFromResource(SampleRGBPrinter, &OutputProfile);

    const WCHAR PhotoProfileName[] = L"Photo.gmmp";
    if (SUCCEEDED(hr))
    {
        //
        // When a PROFILE structure does not contain a path,
        // WCS opens the profile from the system color directory.
        // This is where system WCS profiles are installed.
        //
        SystemPhotoGmmpProfile.pProfileData = (PVOID)PhotoProfileName;
        SystemPhotoGmmpProfile.cbDataSize = sizeof(PhotoProfileName);
        SystemPhotoGmmpProfile.dwType = PROFILE_FILENAME;
    }

    HPROFILE hInputProfile = NULL;
    HPROFILE hOutputProfile = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // The current user's default GMMP profile for the Absolute Colorimetric
        // intent will be embedded in the HPROFILE because a NULL parameter for
        // the GMMP profile is passed in.  BUT, it will not matter to WCS during
        // transform creation.  This is because CreateMultiProfileTransform will
        // be told to use the embedded GMMPs, and in this case the
        // DESTINATION profile handle for each pair of profiles is used.
        // (When there are two profiles, there is one pair and one GMMP used.  When
        // there are three profiles, there are two pairs, and two GMMPs used).
        //
        hInputProfile = WcsOpenColorProfile(
                            &InputProfile,
                            GetD65CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hInputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now, make the destination handle contain the specific system
        // GMMP to be used.  As described above, this is the HPROFILE
        // that will determine which GMMP is used.
        //
        hOutputProfile = WcsOpenColorProfile(
                            &OutputProfile,
                            GetD50CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            &SystemPhotoGmmpProfile,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hOutputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }

    HTRANSFORM hTransform = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // WCS handles the intent selection for this API based first on the size of the
        // intentarray - it is either 1, nProfiles-1, or nProfiles in size (The first two
        // being equivalent in this case).
        //
        // When using nProfiles-1 intents, each of the intents indicate how to look up the
        // gamut mapper for each respective pair of device profiles. (When there are two
        // profiles, there is one pair and one GMMP used.  When there are three profiles,
        // there are two pairs, and two GMMPs used).
        //
        // In this case, using -1 (equivalent to DWORD_MAX) as the intent value tells WCS
        // to use the GMMP that is embedded in the HPROFILE of the destination handle for
        // each pair of profiles.  Here, this means that the system Photo.gmmp profile will
        // be used because it was embedded it into the destination HPROFILE using
        // WcsOpenColorProfile.
        //
        DWORD dwIntent = (DWORD) -1;

        //
        // This creates a simple transform.  However, different flags could be used at
        // this point to customize advanced options such as quality mode, black preservation,
        // gamut checking support, etc.
        //
        HPROFILE hProfileList[2] = {hInputProfile, hOutputProfile};
        hTransform = CreateMultiProfileTransform(
                        hProfileList,
                        2,
                        &dwIntent,
                        1,
                        NORMAL_MODE,
                        INDEX_DONT_CARE);
        if (!hTransform)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now do a simple translation and write the results to disk
        //
        hr = TranslateAndOutputRGBImage(
                        hTransform,
                        pwszFilenameOfResult);
    }

    //
    // Clean up.
    //
    CloseColorProfile(hInputProfile);
    CloseColorProfile(hOutputProfile);
    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::DemonstrateCustomPluginProfiles
//
//  Synopsis:
//      This method demonstrates how to use plugins while creating a color
//      transform.
//
//      Specific custom DMP and GMMP color profiles are used that
//      reference the plug-in Device Model and Gamut Mapper that are
//      installed with the seperate plugin sample package.
//
//      If the plugins are not installed, baseline fallback models are used
//      as indicated in the DMP and GMMP color profiles.
//
//----------------------------------------------------------------------------
HRESULT
CTransformCreationDemo::DemonstrateCustomPluginProfiles()
{
    HRESULT hr = S_OK;

    wprintf(L"Demonstrating usage of custom GMMP profiles with plugins\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateCustomPluginProfiles Output.bmp";

    //
    // For demonstration purposes, do a simple transform from a Device Model
    // Plugin to an RGB Printer using a Gamut Map Model Plugin.
    //
    // First, open WCS profile handles for those profiles that are needed.
    //
    PROFILE InputProfile = {};
    PROFILE OutputProfile = {};
    PROFILE PluginGmmpProfile = {};


    hr = GetProfileFromResource(SampleCRTProfileWithPlugin, &InputProfile);
    if (SUCCEEDED(hr))
    {
        hr = GetProfileFromResource(SampleRGBPrinter, &OutputProfile);
    }
    const WCHAR ProfileName[] = L"Photo.gmmp";
    if (SUCCEEDED(hr))
    {
        hr = GetProfileFromResource(SampleGmmpWithPlugin, &PluginGmmpProfile);
    }

    HPROFILE hInputProfile = NULL;
    HPROFILE hOutputProfile = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // The current user's default GMMP profile for the Absolute Colorimetric
        // intent will be embedded in the HPROFILE because a NULL parameter for
        // the GMMP profile is passed in.  BUT, it will not matter to WCS during
        // transform creation.  This is because CreateMultiProfileTransform will
        // be told to use the embedded GMMPs, and in this case the
        // DESTINATION profile handle for each pair of profiles is used.
        // (When there are two profiles, there is one pair and one GMMP used.  When
        // there are three profiles, there are two pairs, and two GMMPs used).
        //
        hInputProfile = WcsOpenColorProfile(
                            &InputProfile,
                            GetD65CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hInputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now, make the destination handle contain the specific plugin
        // GMMP to be used.  As described above, this is the HPROFILE
        // that will determine which GMMP is used.
        //
        hOutputProfile = WcsOpenColorProfile(
                            &OutputProfile,
                            GetD50CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            &PluginGmmpProfile,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hOutputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }

    HTRANSFORM hTransform = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // WCS handles the intent selection for this API based first on the size of the
        // intentarray - it is either 1, nProfiles-1, or nProfiles in size (The first two
        // being equivalent in this case).
        //
        // When using nProfiles-1 intents, each of the intents indicate how to look up the
        // gamut mapper for each respective pair of device profiles. (When there are two
        // profiles, there is one pair and one GMMP used.  When there are three profiles,
        // there are two pairs, and two GMMPs used).
        //
        // In this case, using -1 (equivalent to DWORD_MAX) as the intent value tells WCS
        // to use the GMMP that is mbedded in the HPROFILE of the destination handle for
        // each pair of profiles. Here, this means that the plugin GMMP profile will be
        // used because it was embedded into the destination HPROFILE using
        // WcsOpenColorProfile.
        //
        DWORD dwIntent = (DWORD) -1;

        //
        // This creates a simple transform.  However, different flags could be used at
        // this point to customize advanced options such as quality mode, black preservation,
        // gamut checking support, etc.
        //
        HPROFILE hProfileList[2] = {hInputProfile, hOutputProfile};
        hTransform = CreateMultiProfileTransform(
                        hProfileList,
                        2,
                        &dwIntent,
                        1,
                        NORMAL_MODE,
                        INDEX_DONT_CARE);
        if (!hTransform)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now do a simple translation and write the results to disk
        //
        hr = TranslateAndOutputRGBImage(
                hTransform,
                pwszFilenameOfResult);
    }

    //
    // Clean up.
    //
    CloseColorProfile(hInputProfile);
    CloseColorProfile(hOutputProfile);
    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::DemonstrateSystemDefaultIntent
//
//  Synopsis:
//      This method demonstrates how to use the system default rendering
//      intent to select the GMMP profile used during transform creation.
//
//----------------------------------------------------------------------------
HRESULT
CTransformCreationDemo::DemonstrateSystemDefaultIntent()
{
    HRESULT hr = S_OK;

    wprintf(L"Demonstrating usage of system default rendering intent\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateSystemDefaultIntent Output.bmp";

    //
    // For demonstration purposes, do a simple transform from sRGB
    // to an RGB Printer.
    //
    // First, open WCS profile handles for those profiles that are needed.
    //
    PROFILE OutputProfile;

    hr = GetProfileFromResource(SampleRGBPrinter, &OutputProfile);

    //
    // Declare and initialize PROFILE InputProfile
    //
    static WCHAR sRGBWcsProfilename[] = L"wsRGB.cdmp";
    PROFILE InputProfile;
    InputProfile.pProfileData = (PVOID)sRGBWcsProfilename;
    InputProfile.cbDataSize = sizeof(sRGBWcsProfilename);
    InputProfile.dwType = PROFILE_FILENAME;

    HPROFILE hInputProfile = NULL;
    HPROFILE hOutputProfile = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // The system default GMMP profile will be used because a NULL parameter
        // is passed in.  This profile handle will contain the system
        // default GMMP Profile, BUT, it will not matter to WCS during
        // transform creation.
        //
        // The current user's default GMMP profile for the Absolute Colorimetric
        // intent will be embedded in the HPROFILE because a NULL parameter for
        // the GMMP profile is passed in.  BUT, it will not matter to WCS during
        // transform creation.  This is because the intent of -1 will not be used
        // in the intent array to CreateMultiProfileTransform, and so "embedded"
        // intents will not be used.
        //
        hInputProfile = WcsOpenColorProfile(
                            &InputProfile,
                            GetD65CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hInputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now make the destination handle using the default GMMP
        // (again with the GMMP not being used later).
        //
        hOutputProfile = WcsOpenColorProfile(
                            &OutputProfile,
                            GetD50CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hOutputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }

    HTRANSFORM hTransform = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // WCS handles the intent selection for this API based first on the size of the
        // intentarray - it is either 1, nProfiles-1, or nProfiles in size (The first two
        // being equivalent in this case).
        //
        // When using nProfiles-1 intents, each of the intents indicate how to look up the
        // gamut mapper for each respective pair of device profiles. (When there are two
        // profiles, there is one pair and one GMMP used.  When there are three profiles,
        // there are two pairs, and two GMMPs used).
        //
        // In this case, WCS is told to use the GMMP that is set as the current user's
        // default selection for the rendering intent in the array.  The rendering intent
        // that will be used here is the user's default intent.  It will be found using the
        // new API, WcsGetDefaultRenderingIntent.
        //
        DWORD dwSystemDefaultRenderingIntent = 0;
        if (!WcsGetDefaultRenderingIntent(WCS_PROFILE_MANAGEMENT_SCOPE_CURRENT_USER,
                        &dwSystemDefaultRenderingIntent))
        {
            hr = HRESULTFromLastError();
        }

        //
        // This creates a simple transform.  However, different flags could be used at
        // this point to customize advanced options such as quality mode, black preservation,
        // gamut checking support, etc.
        //
        HPROFILE hProfileList[2] = {hInputProfile, hOutputProfile};
        if (SUCCEEDED(hr))
        {
            hTransform = CreateMultiProfileTransform(
                            hProfileList,
                            2,
                            &dwSystemDefaultRenderingIntent,
                            1,
                            NORMAL_MODE,
                            INDEX_DONT_CARE);
            if (!hTransform)
            {
                hr = HRESULTFromLastError();
            }
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now do a simple translation and write the results to disk
        //
        hr = TranslateAndOutputRGBImage(
                        hTransform,
                        pwszFilenameOfResult);
    }

    //
    // Clean up.
    //
    CloseColorProfile(hInputProfile);
    CloseColorProfile(hOutputProfile);
    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::DemonstrateIccWcsMixedMode
//
//  Synopsis:
//      This method demonstrates how to create a WCS transform with
//      an ICC profile and WCS profile in combination
//
//----------------------------------------------------------------------------
HRESULT CTransformCreationDemo::DemonstrateIccWcsMixedMode()
{
    HRESULT hr = S_OK;

    wprintf(L"Demonstrating usage of mixed mode ICC/WCS profiles\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateIccWcsMixedMode Output.bmp";

    //
    // For demonstration purposes, do a simple transform from the
    // ICC sRGB profile to a WCS profile for RGB Printer.
    //
    // First, open WCS profile handles for those profiles that are needed.
    //
    PROFILE InputProfile = {};
    PROFILE OutputProfile = {};

    //
    // When a PROFILE structure does not contain a path,
    // WCS opens the profile from the system color directory.
    // This is where system WCS profiles are installed.
    //
    static TCHAR sRGBICCProfilename[] = L"sRGB Color Space Profile.icm";
    InputProfile.pProfileData = (PVOID)sRGBICCProfilename;
    InputProfile.cbDataSize = sizeof(sRGBICCProfilename);
    InputProfile.dwType = PROFILE_FILENAME;

    hr = GetProfileFromResource(SampleRGBPrinter, &OutputProfile);

    HPROFILE hInputProfile = NULL;
    HPROFILE hOutputProfile = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // When using an ICC profile, the CAMP in the HPROFILE is filled using
        // specific values most appropriate for the ICC profile connection space.
        //
        // The Gamut Mapper in the HPROFILE is filled by reading the rendering
        // intent from the ICC profile itself. The specific baseline GMM algorithm
        // corresponding to this intent is embedded in the HPROFILE handle.
        // (As opposed to embedding the default GMMP profile associated with the
        // intent).  BUT, it will not matter to WCS during
        // transform creation.  This is because the intent of -1 will not be used
        // in the intent array to CreateMultiProfileTransform, and so "embedded"
        // intents will not be used.
        //
        hInputProfile = WcsOpenColorProfile(
                            &InputProfile,
                            NULL,
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hInputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now make the destination handle using the default
        // GMMP (again with the GMMP not being used later).
        //
        hOutputProfile = WcsOpenColorProfile(
                            &OutputProfile,
                            GetD50CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hOutputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }

    HTRANSFORM hTransform = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // WCS handles the intent selection for this API based first on the size of the
        // intentarray - it is either 1, nProfiles-1, or nProfiles in size (The first two
        // being equivalent in this case).
        //
        // When using nProfiles-1 intents, each of the intents indicate how to look up the
        // gamut mapper for each respective pair of device profiles. (When there are two
        // profiles, there is one pair and one GMMP used.  When there are three profiles,
        // there are two pairs, and two GMMPs used).
        //
        // In this case, WCS is told to use the GMMP that is set as the current user's
        // default selection for the rendering intent in the array.  In this case, the
        // rendering intent used will be Absolute Colorimetric.
        //
        DWORD dwIntent = INTENT_ABSOLUTE_COLORIMETRIC;

        //
        // This creates a simple transform.  However, different flags could be used at
        // this point to customize advanced options such as quality mode, black preservation,
        // gamut checking support, etc.
        //
        HPROFILE hProfileList[2] = {hInputProfile, hOutputProfile};
        hTransform = CreateMultiProfileTransform(
                            hProfileList,
                            2,
                            &dwIntent,
                            1,
                            NORMAL_MODE,
                            INDEX_DONT_CARE);
        if (!hTransform)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now do a simple translation and write the results to disk
        //
        hr = TranslateAndOutputRGBImage(
                            hTransform,
                            pwszFilenameOfResult);
    }

    //
    // Clean up.
    //
    CloseColorProfile(hInputProfile);
    CloseColorProfile(hOutputProfile);
    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::DemonstrateIccOnlyWcsTransform
//
//  Synopsis:
//      This method demonstrates how to create a color transform
//      using only ICC profiles, but that uses the WCS transform
//      engine.
//
//      This transform will use WCS color apperance models, gamut mappers,
//      and transform flags, and support WCS translation APIs and pixel
//      formats.
//
//----------------------------------------------------------------------------
HRESULT
CTransformCreationDemo::DemonstrateIccOnlyWcsTransform()
{
    HRESULT hr = S_OK;

    wprintf(L"Demonstrating usage of ICC-only profile sequence with WCS engine\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateIccOnlyWcsTransform Output.bmp";

    //
    // For demonstration purposes, do a simple transform from the
    // syste ICC sRGB profile to itself.
    //
    // First, open WCS profile handles for those profiles that are needed.
    //
    PROFILE sRGBProfile = {};

    //
    // When a PROFILE structure does not contain a path,
    // WCS opens the profile from the system color directory.
    // This is where system WCS profiles are installed.
    //
    static TCHAR sRGBICCProfilename[] = L"sRGB Color Space Profile.icm";
    sRGBProfile.pProfileData = (PVOID)sRGBICCProfilename;
    sRGBProfile.cbDataSize = sizeof(sRGBICCProfilename);
    sRGBProfile.dwType = PROFILE_FILENAME;

    HPROFILE hProfile = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // This API call is equivalent to calling WcsOpenColorProfile
        // using NULL arguments for the CAMP and GMMP.
        //
        //
        // When using an ICC profile, the CAMP in the HPROFILE is filled using
        // specific values most appropriate for the ICC profile connection space.
        //
        // The Gamut Mapper in the HPROFILE is filled by reading the rendering
        // intent from the ICC profile itself. The specific baseline GMM algorithm
        // corresponding to this intent is embedded in the HPROFILE handle.
        // (As opposed to embedding the default GMMP profile associated with the
        // intent).  BUT, it will not matter to WCS during
        // transform creation.  This is because the intent of -1 will not be used
        // in the intent array to CreateMultiProfileTransform, and so "embedded"
        // intents will not be used.
        //
        hProfile = OpenColorProfile(
                            &sRGBProfile,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING);
        if (!hProfile)
        {
            hr = HRESULTFromLastError();
        }
    }

    HTRANSFORM hTransform = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // WCS handles the intent selection for this API based first on the size of the
        // intentarray - it is either 1, nProfiles-1, or nProfiles in size (The first two
        // being equivalent in this case).
        //
        // When using nProfiles-1 intents, each of the intents indicate how to look up the
        // gamut mapper for each respective pair of device profiles. (When there are two
        // profiles, there is one pair and one GMMP used.  When there are three profiles,
        // there are two pairs, and two GMMPs used).
        //
        // In this case, WCS is told to use the GMMP that is set as the current user's
        // default selection for the rendering intent in the array.  In this case, the
        // rendering intent used will be Absolute Colorimetric.
        //
        DWORD dwIntent = INTENT_ABSOLUTE_COLORIMETRIC;

        //
        // The WCS_ALWAYS flag means that WCS processing will be used instead of the
        // ICM engine (otherwise the ICM engine is invoked whenever all of the
        // profiles are ICC profiles).
        //
        // This creates a simple transform.  However, different flags could be used at
        // this point to customize advanced options such as quality mode, black preservation,
        // gamut checking support, etc.
        //
        HPROFILE hProfileList[2] = {hProfile, hProfile};
        hTransform = CreateMultiProfileTransform(
                        hProfileList,
                        2,
                        &dwIntent,
                        1,
                        WCS_ALWAYS | NORMAL_MODE,
                        INDEX_DONT_CARE);
        if (!hTransform)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        //
        // Now do a simple translation and write the results to disk
        //
        hr = TranslateAndOutputRGBImage(
                hTransform,
                pwszFilenameOfResult);
    }

    //
    // Clean up. In some cases PROFILE structures might
    // need to be freed, but not in this case.
    //
    CloseColorProfile(hProfile);
    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTransformCreationDemo::TranslateAndOutputRGBImage
//
//  Synopsis:
//      This method does a very basic RGB image translation and outputs
//      the translated image
//
//----------------------------------------------------------------------------
HRESULT
CTransformCreationDemo::TranslateAndOutputRGBImage(
    __in HTRANSFORM hTransform,
    __in PCWSTR pwszOutputFilename
    )
{
    HRESULT hr = S_OK;
    if (!hTransform || !pwszOutputFilename)
    {
        return E_INVALIDARG;
    }

    BYTE *paBitmapData = NULL;
    DWORD cbBitmapData = 0;
    DWORD dwWidth = 0;
    DWORD dwHeight = 0;
    DWORD dwHorizontalResolution = 0;
    DWORD dwVerticalResolution = 0;

    hr = LoadsRGBImage(
                &paBitmapData,
                &cbBitmapData,
                &dwWidth,
                &dwHeight,
                &dwHorizontalResolution,
                &dwVerticalResolution);

    BYTE *paOutputBitmapData = NULL;
    if (SUCCEEDED(hr))
    {
        paOutputBitmapData = new BYTE[cbBitmapData];
        if (!paOutputBitmapData)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        if (!TranslateBitmapBits(
                hTransform,
                paBitmapData,
                BM_RGBTRIPLETS,
                dwWidth,
                dwHeight,
                0,
                paOutputBitmapData,
                BM_RGBTRIPLETS,
                0,
                NULL,
                0))
        {
            hr = HRESULTFromLastError();
        }

    }
    if (SUCCEEDED(hr))
    {
        hr = SaveImage(
                pwszOutputFilename,
                BasicRGBTripletBitmap,
                paOutputBitmapData,
                cbBitmapData,
                dwWidth,
                dwHeight,
                dwHorizontalResolution,
                dwVerticalResolution);
    }

    //
    // Cleanup
    //
    delete[] paOutputBitmapData;
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTranslationDemo::RunDemos
//
//  Synopsis:
//      This method simply launches the demos of this class
//
//----------------------------------------------------------------------------
HRESULT
CTranslationDemo::RunDemos()
{
    HRESULT hr = S_OK;

    hr = DemonstrateS2Dot13BitmapTranslation();
    if (SUCCEEDED(hr))
    {
        hr = DemonstrateFloatColorTranslation();
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTranslationDemo::DemonstrateS2Dot13BitmapTranslation
//
//  Synopsis:
//      This method demonstrates doing a bitmap translation
//      in S2Dot13 fixed point format.
//
//----------------------------------------------------------------------------
HRESULT
CTranslationDemo::DemonstrateS2Dot13BitmapTranslation()
{
    HRESULT hr = S_OK;
    wprintf(L"Demonstrating S2Dot13 Bitmap translation\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateS2Dot13BitmapTranslation Output.dat";

    //
    // Build a basic CRT/Plugin to RGB Printer transform on which to do the translation.
    //
    HTRANSFORM hTransform = NULL;
    hr = BuildBasicTransform(
                    SampleCRTProfileWithPlugin,
                    GetD65CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                    SampleRGBPrinter,
                    GetD50CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                    &hTransform);
    if (SUCCEEDED(hr))
    {
        //
        // For demonstration, translate the image from RGB Triplet format matched
        // to the source profile, into S2Dot13 format matched to the destination profile
        //
        BYTE *paBitmapData = NULL;
        DWORD cbBitmapData = 0;
        DWORD dwWidth = 0;
        DWORD dwHeight = 0;
        DWORD dwHorizontalResolution = 0;
        DWORD dwVerticalResolution = 0;

        hr = LoadsRGBImage(
                    &paBitmapData,
                    &cbBitmapData,
                    &dwWidth,
                    &dwHeight,
                    &dwHorizontalResolution,
                    &dwVerticalResolution);
        if (SUCCEEDED(hr))
        {
            //
            // S2Dot13 requires two bytes per pixel per color
            //
            const DWORD dwS2Dot13PixelSizePerColor = 2;
            const DWORD dwNumOutputChannels = 3;
            DWORD TotalOuputPixelSize = dwS2Dot13PixelSizePerColor * dwNumOutputChannels;

            //
            // We will use stride equal to zero, which means to DWORD-align.  Equivalent
            // stride is calculated below, and used to find the size of output buffer
            // to allocate
            //
            DWORD dwEquivalentStride = AlignToDWORD(dwWidth*TotalOuputPixelSize);
            DWORD dwOutputDataSize = dwEquivalentStride*dwHeight;
            BYTE *paOutputBitmapData = new BYTE[dwOutputDataSize];
            if (!paOutputBitmapData)
            {
                hr = E_OUTOFMEMORY;
            }
            if (SUCCEEDED(hr))
            {
                if (!TranslateBitmapBits(
                        hTransform,
                        paBitmapData,
                        BM_RGBTRIPLETS,
                        dwWidth,
                        dwHeight,
                        0,
                        paOutputBitmapData,
                        BM_S2DOT13FIXED_scRGB,
                        0,
                        NULL,
                        0))
                {
                    hr = HRESULTFromLastError();
                }
            }
            if (SUCCEEDED(hr))
            {
                //
                // Standard bitmap formats don't support S2Dot13.  The pixel data
                // will be written to a data file on disk
                //
                hr = SavePixelData(pwszFilenameOfResult, paOutputBitmapData, dwOutputDataSize);
            }
            delete[] paOutputBitmapData;
        }
    }

    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTranslationDemo::DemonstrateFloatColorTranslation
//
//  Synopsis:
//      This method demonstrates doing a color translation using the
//      new WcsTranslateColors API and floating point format
//
//----------------------------------------------------------------------------
HRESULT
CTranslationDemo::DemonstrateFloatColorTranslation()
{
    HRESULT hr = S_OK;
    wprintf(L"Demonstrating translation from RGB BYTE colors to CMYK FLOAT colors\n");
    PCWSTR pwszFilenameOfResult = L"DemonstrateFloatColorTranslation Output.dat";

    //
    // Build a basic CRT/Plugin to CMYK transform on which to do the translation
    //
    HTRANSFORM hTransform = NULL;
    hr = BuildBasicTransform(
                        SampleCRTProfileWithPlugin,
                        GetD65CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                        SampleCMYKPrinter,
                        GetD50CAMP(), // Use CAMPs designed for the device, viewing conditions, and task
                        &hTransform);
    if (SUCCEEDED(hr))
    {
        BYTE *paBitmapData = NULL;
        DWORD cbBitmapData = 0;
        DWORD dwWidth = 0;
        DWORD dwHeight = 0;
        DWORD dwHorizontalResolution = 0;
        DWORD dwVerticalResolution = 0;

        hr = LoadsRGBImage(
                    &paBitmapData,
                    &cbBitmapData,
                    &dwWidth,
                    &dwHeight,
                    &dwHorizontalResolution,
                    &dwVerticalResolution);
        //
        // For demonstration:
        // 1. From the bitmap, build an array of COLOR_BYTE values suitable for use in WcsTranslateColors
        // 2. Input the COLOR_BYTE values in the transform, and output COLOR_FLOAT values
        //
        BYTE *paInputColorData = NULL;
        FLOAT *paOutputColorData = NULL;
        const DWORD dwNumRGBChannels = 3;
        const DWORD dwNumCmykChannels = 4;
        if (SUCCEEDED(hr))
        {
            //
            // Check for integer overflow before allocating
            //
            ULONG ulInputColorElements = 0;
            hr = SafeULongMult(dwWidth, dwHeight, &ulInputColorElements);
            if (SUCCEEDED(hr))
            {
                hr = SafeULongMult(ulInputColorElements, dwNumRGBChannels, &ulInputColorElements);
            }
            if (SUCCEEDED(hr))
            {
                paInputColorData = new BYTE[ulInputColorElements];
                if (!paInputColorData)
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }
        if (SUCCEEDED(hr))
        {
            //
            // Since the image is known to be in BM_RGBTRIPLETS format, it is possible to
            // directly copy the BYTE color values after re-ordering, since 3-channel COLOR_BYTE 
            // data is ordered as RGB in memory, and BM_RGBTRIPLETS data is ordered as BGR.
            BYTE *pCurrentColorBufferPosition = paInputColorData;
            BYTE *pCurrentBitmapBufferPosition = paBitmapData;
            DWORD dwRgbTripletPixelSizePerColor = 1;
            DWORD TotalOuputPixelSize = dwRgbTripletPixelSizePerColor * dwNumRGBChannels;
            DWORD dwEquivalentStride = AlignToDWORD(dwWidth*TotalOuputPixelSize);
            DWORD dwSizeOfPixelDataPerScanline = sizeof(BYTE)*dwNumRGBChannels*dwWidth;
            for (DWORD nRow=0; nRow<dwHeight; nRow++)
            {
                BYTE *pSrcPixels = pCurrentBitmapBufferPosition;
                BYTE *pDestPixels = pCurrentColorBufferPosition;
                for (UINT i = 0; i < dwWidth; i++)
                {
                    pDestPixels[0] = pSrcPixels[2];
                    pDestPixels[1] = pSrcPixels[1];
                    pDestPixels[2] = pSrcPixels[0];

                    pSrcPixels += 3;
                    pDestPixels += 3;
                }
                pCurrentColorBufferPosition += dwSizeOfPixelDataPerScanline;
                pCurrentBitmapBufferPosition += dwEquivalentStride;
            }

            //
            // Now the array of colors suitable for WcsTranslateColors.
            // For demonstration, use these colors in the transform and output floating point
            // CMYK values
            //
            DWORD dwNumColors = dwHeight*dwWidth;

            //
            // Check for integer overflow before allocating
            //
            DWORD dwInputSize = sizeof(BYTE)*dwNumColors*dwNumRGBChannels;
            DWORD dwOutputSize = 0;
            ULONG ulOutputColorElements = 0;
            hr = SafeULongMult(dwNumColors, dwNumCmykChannels, &ulOutputColorElements);
            if (SUCCEEDED(hr))
            {
                hr = SafeULongMult(ulOutputColorElements, sizeof(FLOAT), &dwOutputSize);
            }
            if (SUCCEEDED(hr))
            {
                paOutputColorData = new FLOAT[ulOutputColorElements];
                if (!paOutputColorData)
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if (SUCCEEDED(hr))
            {
                if (!WcsTranslateColors(
                                    hTransform,
                                    // Handle to the transform to use
                                    dwNumColors,
                                    // Total number of pixels to translate
                                    dwNumRGBChannels,
                                    // Number of input channels
                                    COLOR_BYTE,
                                    // Color format of input array
                                    dwInputSize,
                                    // Size in bytes of the input array
                                    paInputColorData,
                                    // The input color array
                                    dwNumCmykChannels,
                                    // Number of Output channels
                                    (COLORDATATYPE) COLOR_FLOAT,
                                    // Color format of the output array
                                    dwOutputSize,
                                    // Size in bytes of the output array
                                    paOutputColorData))
                                    // The output color array
                {
                    hr = HRESULTFromLastError();
                }
            }
            if (SUCCEEDED(hr))
            {
                //
                // Standard bitmap formats don't floating point CMYK data.  Write the
                // pixel data to a binary file on disk
                //
                hr = SavePixelData(pwszFilenameOfResult, (BYTE*)paOutputColorData, dwOutputSize);
            }
        }
        delete[] paOutputColorData;
        delete[] paInputColorData;
    }

    DeleteColorTransform(hTransform);
    return hr;
}

//+---------------------------------------------------------------------------
//
//  Member:
//      CTranslationDemo::BuildBasicTransform
//
//  Synopsis:
//      This method creates a basic color transform for the specified
//      profiles that are resources in the executable.  This method
//      uses the system Photo/Perceptual GMMP profile in the transform.
//
//----------------------------------------------------------------------------
HRESULT
CTranslationDemo::BuildBasicTransform(
    __in ResourceProfileType InputProfileType,
    __in PPROFILE InputViewingConditionsProfile,
    __in ResourceProfileType OutputProfileType,
    __in PPROFILE OutputViewingConditionsProfile,
    __deref_out HTRANSFORM *phTransform
    )
{
    HRESULT hr = S_OK;
    if (!phTransform)
    {
        return E_INVALIDARG;
    }

    //
    // Create profile handles
    //
    PROFILE InputProfile = {};
    PROFILE OutputProfile = {};
    PROFILE SystemPhotoGmmpProfile = {};
    hr = GetProfileFromResource(InputProfileType, &InputProfile);
    if (SUCCEEDED(hr))
    {
        hr = GetProfileFromResource(OutputProfileType, &OutputProfile);
    }

    WCHAR ProfileName[] = L"Photo.gmmp";
    if (SUCCEEDED(hr))
    {
        SystemPhotoGmmpProfile.pProfileData = (PVOID)ProfileName;
        SystemPhotoGmmpProfile.cbDataSize = sizeof(ProfileName);
        SystemPhotoGmmpProfile.dwType = PROFILE_FILENAME;
    }

    HPROFILE hInputProfile = NULL;
    HPROFILE hOutputProfile = NULL;
    if (SUCCEEDED(hr))
    {
        hInputProfile = WcsOpenColorProfile(
                            &InputProfile,
                            InputViewingConditionsProfile,
                            NULL,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hInputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }
    if (SUCCEEDED(hr))
    {
        hOutputProfile = WcsOpenColorProfile(
                            &OutputProfile,
                            OutputViewingConditionsProfile,
                            &SystemPhotoGmmpProfile,
                            PROFILE_READ,
                            FILE_SHARE_READ,
                            OPEN_EXISTING,
                            0);
        if (!hOutputProfile)
        {
            hr = HRESULTFromLastError();
        }
    }

    HTRANSFORM hTransform = NULL;
    if (SUCCEEDED(hr))
    {
        //
        // Now create the transform
        //
        HPROFILE hProfileList[2] = {hInputProfile, hOutputProfile};

        //
        // The intent value of -1 is equivalent to DWORD_MAX and
        // tells WCS to use the GMMP embedded in the destination
        // handle.  Here, that means it will use the system
        // Photo GMMP profile.
        //
        DWORD dwIntent = (DWORD) -1;
        *phTransform = CreateMultiProfileTransform(
                        hProfileList,
                        2,
                        &dwIntent,
                        1,
                        NORMAL_MODE,
                        INDEX_DONT_CARE);
        if (!*phTransform)
        {
            hr = HRESULTFromLastError();
        }
    }

    //
    // Cleanup
    //
    CloseColorProfile(hInputProfile);
    CloseColorProfile(hOutputProfile);
    return hr;
}
