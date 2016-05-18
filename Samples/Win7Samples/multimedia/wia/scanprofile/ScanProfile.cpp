//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//------------------------------------------------------------
//Please read the ReadME.txt which explains the purpose of the
//sample.
//-------------------------------------------------------------

#include "ScanProfile.h"


// This function creates a Scan Profile Dialog
HRESULT CreateScanProfileUI(HWND hwndParent)
{
    IScanProfileUI* pScanProfileUI = NULL;

    //Create ScanProfileUI instance
    HRESULT hr = CoCreateInstance(CLSID_ScanProfileUI, NULL,  CLSCTX_INPROC_SERVER, IID_IScanProfileUI, reinterpret_cast<LPVOID *>(&pScanProfileUI));
    if(SUCCEEDED(hr))
    {
        //Create ScanProfileDialog from IScanProfileUI interface
        hr = pScanProfileUI->ScanProfileDialog(hwndParent);
        if(FAILED(hr))
        {
            ReportError( TEXT("An error occurred while creating ScanProfileDialog"), hr );
        }
        
        //Release IScanProfileUI interface
        pScanProfileUI->Release();
        pScanProfileUI = NULL;
    }
    else
    {
        ReportError( TEXT("CoCreateInstance failed on CLSID_ScanProfileUI"), hr );
    }
    
    return hr;
}

// This function reads the Device ID (needed for creating scan profiles) and Device name. 
HRESULT ReadDevIDandDevName( IWiaPropertyStorage *pWiaPropertyStorage , BSTR* pbstrDevID , BSTR* pbstrDevName)
{
    // Validate arguments
    if ( (NULL == pWiaPropertyStorage) || (NULL == pbstrDevID) || (NULL == pbstrDevName) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid args passed to ReadDevIDandDevName()"),hr);
        return hr;
    }

    //Read the Device ID from the IWiaPropertyStorage interface
    HRESULT hr = ReadPropertyBSTR(pWiaPropertyStorage,WIA_DIP_DEV_ID, pbstrDevID);
    if(FAILED(hr))
    {
        ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_ID"),hr);
        *pbstrDevID = NULL;
        return hr;
    }
    
    //Read the Device Name from the IWiaPropertyStorage interface
    hr = ReadPropertyBSTR(pWiaPropertyStorage,WIA_DIP_DEV_NAME, pbstrDevName);
    if(FAILED(hr))
    {
        ReportError(TEXT("ReadPropertyBSTR() failed for WIA_DIP_DEV_NAME"),hr);
        *pbstrDevName = NULL;
    }
    return hr;
}

// This function gets the Device ID from the profile and prints it
HRESULT ProfileDisplayDeviceID(IScanProfile* pScanProfile)
{
    // Validate arguments
    if( (!pScanProfile) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid args passed to ProfileDisplayDeviceID()"),hr);
        return hr;
    }
    BSTR bstrDevID = NULL;
    HRESULT hr = pScanProfile->GetDeviceID(&bstrDevID);
    if(SUCCEEDED(hr))
    {
        _tprintf(TEXT("\nDeviceID retreived from the profile => %ws"),bstrDevID); 
    }
    else
    {
        ReportError(TEXT("Error calling pScanProfile->GetDeviceID()"),hr);
    }
    return  hr; 
}

// This function sets the name of the profile and then prints it
HRESULT ProfileSetAndDisplayName(IScanProfile* pScanProfile)
{
    // Validate arguments
    if( (!pScanProfile) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid args passed to ProfileSetAndDisplayName()"),hr);
        return hr;
    }
    BSTR bstrProfileName = SysAllocString(PROFILE_NAME);
    
    HRESULT hr = pScanProfile->SetName(bstrProfileName);
    if(SUCCEEDED(hr))
    {
        //Get the name of the profile
        BSTR bstrProfileGetName;
        hr = pScanProfile->GetName(&bstrProfileGetName);
        if(SUCCEEDED(hr))
        {
            _tprintf(TEXT("\nProfile name just created for the device => %ws"),bstrProfileGetName); 
        }
        else
        {
            ReportError(TEXT("Error calling pScanProfile->GetName()"),hr);
        }
    }
    else
    {
        ReportError(TEXT("Error calling pScanProfile->SetName()"),hr);
    }
    return hr;
}

// This function prints the no of profiles associated with the device
HRESULT GetNumberOfProfiles(IScanProfileMgr* pScanProfileMgr, BSTR bstrDevID, ULONG* lNumProfiles)
{
    // Validate arguments
    if( (!bstrDevID) || (!lNumProfiles) || (!pScanProfileMgr) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid args passed to GetNumberOfProfiles()"),hr);
        return hr;
    }

    //Initialize out variables
    *lNumProfiles = 0;

    HRESULT hr = pScanProfileMgr->GetNumProfilesforDeviceID(bstrDevID,lNumProfiles);
    if(SUCCEEDED(hr))
    {
        _tprintf(TEXT("\nNo of Profiles for this device => %ld"),*lNumProfiles);
    }
    else
    {
        ReportError(TEXT("Error calling pScanProfileMgr->GetNumProfilesforDeviceID()"),hr);
    }
    return hr;
}

// This function prints all the names of the profiles associated with the device
HRESULT DisplayAllProfilesForDevice(IScanProfileMgr* pScanProfileMgr,BSTR bstrDevID, ULONG lNumProfiles)
{
    // Validate arguments
    if( (!bstrDevID) || (!pScanProfileMgr) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid args passed to DisplayAllProfilesForDevice()"),hr);
        return hr;
    }
    HRESULT hr = S_OK;
    IScanProfile** arr_pScanProfileDevice = NULL;
    arr_pScanProfileDevice = new IScanProfile*[lNumProfiles * sizeof(IScanProfile*)];
    if(arr_pScanProfileDevice)
    {
        hr = pScanProfileMgr->GetProfilesforDeviceID(bstrDevID,&lNumProfiles,arr_pScanProfileDevice);
        if(SUCCEEDED(hr))
        {
            for( ULONG count=0 ; count < lNumProfiles ; count ++)
            {
                BSTR bstrProfDeviceName = NULL;
                //Print the names of all the profiles 
                hr  = arr_pScanProfileDevice[count]->GetName(&bstrProfDeviceName);
                if(SUCCEEDED(hr))
                {
                    _tprintf(TEXT("\nName of Profile No.%d for the device => %ws"),count+1,bstrProfDeviceName);

                    //release all the IScanProfile pointers
                    arr_pScanProfileDevice[count]->Release();
                    arr_pScanProfileDevice[count] = NULL;
                }
                else
                {
                    ReportError(TEXT("Error calling IScanProfile::GetName()"),hr);
                }
            }
        }
        else
        {
            ReportError(TEXT("Error calling pScanProfileMgr->GetProfilesforDeviceID()"),hr);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
        ReportError(TEXT("Could not allocate memory for arr_pScanProfileDevice"),hr);
    }
    return hr;
}
          

//This function creates the scan profiles for the device and also performs various operations on them
HRESULT CreateScanProfiles(BSTR bstrDeviceID , BSTR bstrDeviceName)
{
    // Validate arguments
    if( (!bstrDeviceName) || (!bstrDeviceID) )
    {
        HRESULT hr = E_INVALIDARG;
        ReportError(TEXT("Invalid args passed to CreateScanProfiles()"),hr);
        return hr;
    }
    
    //Create scan profile manager
    IScanProfileMgr* pScanProfileMgr = NULL;
    HRESULT hr = CoCreateInstance(CLSID_ScanProfileMgr, NULL,  CLSCTX_INPROC_SERVER, IID_IScanProfileMgr, reinterpret_cast<LPVOID *>(&pScanProfileMgr));
    
    _tprintf(TEXT("\n\n\nDevice Name => %ws \nDeviceID    => %ws"), bstrDeviceName, bstrDeviceID);
    
    if(SUCCEEDED(hr)){
        
        //create scan profile
        IScanProfile* pScanProfile = NULL;
        hr = pScanProfileMgr->CreateProfile(bstrDeviceID,bstrDeviceName,WIA_CATEGORY_FLATBED, &pScanProfile);
        if(SUCCEEDED(hr)){
            _tprintf(TEXT("\nSuccessfully created profile for the device"));
        
            // Display the deviceID back from the profile
            ProfileDisplayDeviceID(pScanProfile);
        
            // Set the name of the profile and display it.
            ProfileSetAndDisplayName(pScanProfile);                           

            //Set various properties in the profile
            PROPID propID[3] = {WIA_IPS_BRIGHTNESS,WIA_IPS_CONTRAST,WIA_IPA_FORMAT};
            PROPVARIANT propVar[3];
            propVar[0].vt = VT_I4;
            propVar[0].lVal = 75;
            propVar[1].vt = VT_I4;
            propVar[1].lVal = 60;
            propVar[2].vt = VT_CLSID;
            GUID guidFormat = WiaImgFmt_BMP;
            propVar[2].puuid = &guidFormat;
            hr = pScanProfile->SetProperty(3, propID, propVar);
            if(SUCCEEDED(hr))
            {
                //Get Various properties in the profile
                PROPID propid[] = {WIA_IPS_BRIGHTNESS,WIA_IPS_CONTRAST};
                PROPVARIANT propVariant[2];
                PropVariantInit(propVariant);
                hr = pScanProfile->GetProperty(2, propid, propVariant);
                if(SUCCEEDED(hr))
                {
                    //Print the Brightness and XRES 
                    _tprintf(TEXT("\nFor the profile just created, Brightness = %ld , WIA_IPS_CONTRAST = %ld") , propVariant[0].lVal , propVariant[1].lVal);
                }
                else
                {
                    ReportError(TEXT("Error calling pScanProfile->GetProperty()"),hr);
                }
            }
            else
            {
                ReportError(TEXT("Error calling pScanProfile->SetProperty()"),hr);
            }
           
            //Save the Scanprofile
            hr = pScanProfile->Save();
            if(SUCCEEDED(hr))
            {
                //Set the profile as default . Hence in push scanning this profile will be used.
                hr = pScanProfileMgr->SetDefault(pScanProfile);
                if(FAILED(hr))
                {
                    ReportError(TEXT("Error calling pScanProfileMgr->SetDefault()"),hr);
                }
            }
            else
            {
                ReportError(TEXT("Error calling pScanProfile->Save()"),hr);
            }
                                
            //Release pScanProfile
            pScanProfile->Release();
            pScanProfile = NULL;

        }
        else
        {
            ReportError(TEXT("pScanProfileMgr->CreateProfile() failed."),hr);
        }


        // get the no of profiles for the device
        ULONG lNumProfiles = 0;
        hr = GetNumberOfProfiles(pScanProfileMgr,bstrDeviceID,&lNumProfiles);
        if(SUCCEEDED(hr))
        {
            //Display all the profiles for the device 
            DisplayAllProfilesForDevice(pScanProfileMgr,bstrDeviceID,lNumProfiles);
        }
        else
        {
            ReportError(TEXT("Error calling GetNumberOfProfiles()"),hr);
        }


        //Release pScanProfileMgr 
        pScanProfileMgr->Release();
        pScanProfileMgr = NULL;
        
    }
    else
    {
        ReportError(TEXT("CoCreateInstance failed for ScanProfileMgr"),hr);
    }
    return hr;
}

    
//This function enumerates the WIA devices and then creates scan profiles for each.
HRESULT EnumWiaDevicesandCreateScanProfiles( IWiaDevMgr2 *pWiaDevMgr2 )
{
    // Validate arguments
    if (NULL == pWiaDevMgr2)
    {
        return E_INVALIDARG;
    }

    // Get a device enumerator interface
    IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
    HRESULT hr = pWiaDevMgr2->EnumDeviceInfo( WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo );
    if (SUCCEEDED(hr))
    {
        // Reset the device enumerator to the beginning of the list
        hr = pWiaEnumDevInfo->Reset();
        if (SUCCEEDED(hr))
        {
            // We will loop until we get an error or pWiaEnumDevInfo->Next returns
            // S_FALSE to signal the end of the list.
            while (S_OK == hr)
            {
                // Get the next device's property storage interface pointer
                IWiaPropertyStorage *pWiaPropertyStorage = NULL;
                hr = pWiaEnumDevInfo->Next( 1, &pWiaPropertyStorage, NULL );

                // pWiaEnumDevInfo->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                if (hr == S_OK)
                {
                    //Read Device name and Device ID
                    //We will use Device name to create profile names   
                    BSTR bstrDevID = NULL;
                    BSTR bstrDevName = NULL;
                    HRESULT hr1  = ReadDevIDandDevName(pWiaPropertyStorage , &bstrDevID ,&bstrDevName);
                    if(SUCCEEDED(hr1))
                    {
                        //Create and perform operations on profiles by calling various IScanProfile API's
                        hr1 = CreateScanProfiles(bstrDevID,bstrDevName);
                        if(FAILED(hr1))
                        {
                            ReportError(TEXT("Error calling CreateScanProfiles()"),hr1);
                        }
                    }

                    // Release the device's IWiaPropertyStorage*
                    pWiaPropertyStorage->Release();
                    pWiaPropertyStorage = NULL;
                }
                else if (FAILED(hr))
                {
                    // Report that an error occurred during enumeration
                    ReportError( TEXT("Error calling IEnumWIA_DEV_INFO::Next()"), hr );
                }
            }
            
            // If the result of the enumeration is S_FALSE, since this
            // is normal, we will change it to S_OK.
            if (S_FALSE == hr)
            {
                hr = S_OK;
            }
        }
        else
        {
            // Report that an error occurred calling Reset()
            ReportError( TEXT("Error calling IEnumWIA_DEV_INFO::Reset()"), hr );
        }

        // Release the enumerator
        pWiaEnumDevInfo->Release();
        pWiaEnumDevInfo = NULL;
    }
    else
    {
        // Report that an error occurred trying to create the enumerator
        ReportError( TEXT("Error calling IWiaDevMgr2::EnumDeviceInfo"), hr );
    }

    // Return the result of the enumeration
    return hr;
}

//Entry point of the application
extern "C" 
int __cdecl _tmain( int, TCHAR *[] )
{
    // Initialize COM
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        //Now Demonstrating how to create scan profiles and do operations on them by calling various IScanProfile API's, ie. without GUI.
        hr = CreateScanProfilesWithoutUI();
        if(FAILED(hr))
        {
            ReportError(TEXT("Error calling CreateScanProfilesWithoutUI()"),hr);
        }
        
        
        //Demonstrating how to create, edit and delete scan profiles through a ScanProfile dialog, ie. using a GUI interface IScanProfileUI. 
        hr = CreateScanProfileUI((HWND)0);
        if(FAILED(hr))
        {
            ReportError(TEXT("Error calling CreateScanProfileUI()"),hr);
        }

        // Uninitialize COM
        CoUninitialize();
    }
    else
    {
        ReportError(TEXT("CoInitialize() failed"),hr);
    }
    return 0;
}

// This function is the top level function for creating scan profiles .
HRESULT CreateScanProfilesWithoutUI()
{
    // Create the device manager
    IWiaDevMgr2 *pWiaDevMgr2 = NULL;
    HRESULT hr = CoCreateInstance( CLSID_WiaDevMgr2, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr2, (void**)&pWiaDevMgr2 );
    if (SUCCEEDED(hr))
    {
        //To create scan profiles without a ScanProfile dialog , deviceID is needed
        //Hence we will get the device ID's of all WIA devices and then create scan profiles for each device 
        
        //Enumerate all of the WIA devices, get deviceID's and create scan profiles for each device 
        hr = EnumWiaDevicesandCreateScanProfiles( pWiaDevMgr2 );
        if (FAILED(hr))
        {
            ReportError( TEXT("Error calling EnumWiaDevicesandCreateScanProfiles()"), hr );
        }
          // Release the device manager
        pWiaDevMgr2->Release();
        pWiaDevMgr2 = NULL;
    }
    else
    {
        ReportError( TEXT("CoCreateInstance() failed on CLSID_WiaDevMgr"), hr );
    }
    return hr;
}

