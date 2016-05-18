///////////////////////////////////////////////////////////////////////////////
//
// CRightsReporter.cpp : Contains the implementation of the CRightsReporter 
//  class.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include "CRightsReporter.h"

///////////////////////////////////////////////////////////////////////////////
//
// CRightsReporter constructor
//
///////////////////////////////////////////////////////////////////////////////
CRightsReporter::CRightsReporter()
{
    // Initialize the member variables.
    m_pProvider     = NULL;
    m_pLicenseQuery = NULL;

    m_KIDStrings = NULL;
    m_NumKIDs    = 0;

    m_IsAllowedRights     = NULL;
    m_LicenseStateRights  = NULL;
    m_cIsAllowedRights    = 0;
    m_cLicenseStateRights = 0;

    m_IsAllowedResults    = NULL;
    m_LicenseStateResults = NULL;

    m_pFile = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CRightsReporter destructor
//
///////////////////////////////////////////////////////////////////////////////
CRightsReporter::~CRightsReporter()
{
    ShutDown();
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: Initialize
// Description: Creates the rights arrays and instantiates the required object.
// Parameters: None.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::Initialize()
{
    HRESULT hr = S_OK;

    // Check to ensure that the object isn't already initialized.
    // Check to ensure that the object has been initialized.
    if (m_pLicenseQuery != NULL)
    {
        hr = E_FAIL;
    }

    // Create the rights strings that will be used for the queries.

    // Note: The following code creates arrays of strings containing the rights
    //  to query for. In this code, the values have been hard-coded to include
    //  all of the available rights for each query. In your code, you may wish
    //  to dynamically select the rights to query for.

    if (SUCCEEDED(hr))
    {
        m_cIsAllowedRights = 5;

        m_IsAllowedRights = new BSTR[m_cIsAllowedRights];

        if (m_IsAllowedRights == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Initialize the new array.
            ZeroMemory((void*)m_IsAllowedRights, 
                    m_cIsAllowedRights * sizeof(BSTR));
        }
    }

    // Set the members to the available "is allowed" rights strings.

    if (SUCCEEDED(hr))
    {       
        m_IsAllowedRights[0] = 
            SysAllocString(g_wszWMDRM_ActionAllowed_Playback);

        if (m_IsAllowedRights[0] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {       
        m_IsAllowedRights[1] = 
            SysAllocString(g_wszWMDRM_ActionAllowed_Copy);

        if (m_IsAllowedRights[1] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {       
        m_IsAllowedRights[2] = 
            SysAllocString(g_wszWMDRM_ActionAllowed_PlaylistBurn);

        if (m_IsAllowedRights[2] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {       
        m_IsAllowedRights[3] = 
            SysAllocString(g_wszWMDRM_ActionAllowed_CreateThumbnailImage);

        if (m_IsAllowedRights[3] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {       
        m_IsAllowedRights[4] = 
            SysAllocString(g_wszWMDRM_ActionAllowed_CopyToCD);

        if (m_IsAllowedRights[4] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Allocate an array to hold the "is allowed" query results.
    if (SUCCEEDED(hr))
    {
        m_IsAllowedResults = new DWORD[m_cIsAllowedRights];

        if (m_IsAllowedResults == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Clear the newly allocated array.
            ZeroMemory(m_IsAllowedResults, m_cIsAllowedRights * sizeof(DWORD));
        }
    }

    // Create the rights list for license state queries.
    if (SUCCEEDED(hr))
    {
        m_cLicenseStateRights = 9;

        m_LicenseStateRights = new BSTR[m_cLicenseStateRights];

        if (m_LicenseStateRights == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Initialize the new array.
            ZeroMemory((void*)m_LicenseStateRights, 
                    m_cLicenseStateRights * sizeof(BSTR));
        }
    }

    // Set the members to the available "license state" rights strings.

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[0] =
            SysAllocString(g_wszWMDRM_LicenseState_Playback);

        if (m_LicenseStateRights[0] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[1] =
            SysAllocString(g_wszWMDRM_LicenseState_Copy);

        if (m_LicenseStateRights[1] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[2] =
            SysAllocString(g_wszWMDRM_LicenseState_CopyToCD);

        if (m_LicenseStateRights[2] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[3] =
            SysAllocString(g_wszWMDRM_LicenseState_PlaylistBurn);

        if (m_LicenseStateRights[3] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[4] =
            SysAllocString(g_wszWMDRM_LicenseState_CreateThumbnailImage);

        if (m_LicenseStateRights[4] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[5] =
            SysAllocString(g_wszWMDRM_LicenseState_CollaborativePlay);

        if (m_LicenseStateRights[5] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[6] =
            SysAllocString(g_wszWMDRM_LicenseState_CopyToSDMIDevice);

        if (m_LicenseStateRights[6] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[7] =
            SysAllocString(g_wszWMDRM_LicenseState_CopyToNonSDMIDevice);

        if (m_LicenseStateRights[7] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_LicenseStateRights[8] =
            SysAllocString(g_wszWMDRM_LicenseState_Backup);

        if (m_LicenseStateRights[8] == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    // Allocate an array to hold the license state query results.
    if (SUCCEEDED(hr))
    {
        m_LicenseStateResults = 
            new DRM_LICENSE_STATE_DATA[m_cLicenseStateRights];

        if (m_LicenseStateResults == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Clear the newly allocated array.
            ZeroMemory(m_IsAllowedResults, m_cIsAllowedRights * sizeof(DWORD));
        }
    }

    // Start DRM and get the needed interface.

    // Initialize DRM.
    if (SUCCEEDED(hr))
    {
        hr = WMDRMStartup();
    }

    // Create a DRM provider.
    if (SUCCEEDED(hr))
    {
        hr = WMDRMCreateProvider(&m_pProvider);
    }

    // Get an interface for querying rights.
    if (SUCCEEDED(hr))
    {
        hr = m_pProvider->CreateObject(__uuidof(IWMDRMLicenseQuery),
                                       (void**)&m_pLicenseQuery);
    }

    // Configure the interface.
    if (SUCCEEDED(hr))
    {
        hr = m_pLicenseQuery->SetActionAllowedQueryParams(FALSE, 150, FALSE, NULL);
    }

    // If anything has failed at this point, release any allocated resources.
    if (FAILED(hr))
    {
        DisplayError(hr, L"Failed to initialize.");
        ShutDown();
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: SetReportFile
// Description: Opens the specified file for output. If the file exists, it
//  will be overwritten.
// Parameters: pwszFilename - The name of the output file.
// Reutrns: S_OK - The file was created.
//          E_FAIL - The object needs to be initialized.
//          E_INVALIDARG - The filename passed is invalid.
// Notes: This method assumes that any problem opening the output file is due
//  to an invalid filename. This may not be correct in practice.
//
//  If a file is already open in the object, this method simply closes it and
//   opens the new file. No warning is given.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::SetReportFile(const WCHAR* pwszFilename)
{
    HRESULT hr = S_OK;

    // Check to ensure that the object has been initialized.
    if (m_pLicenseQuery == NULL)
    {
        hr = E_FAIL;
    }

    // Check for an existing output file. If there is one, close it.
    if (SUCCEEDED(hr))
    {
        if (m_pFile != NULL)
        {
            SAFE_FILE_CLOSE(m_pFile);
        }
    }

    // Create/overwrite the file. 
    if (SUCCEEDED(hr))
    {
        if(_wfopen_s(&m_pFile, pwszFilename, L"w") != 0)
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: SetKIDStrings
// Description: Copies the array of KID strings.
// Parameters: ppKIDStrings - Pointer to the array of strings.
//             NumKIDs - Number of strings in the array.
// Returns: S_OK - String copy succeeded.
//          E_INVALIDARG - Either the array pointer is NULL or the number of
//           KIDs is set to 0.
//          E_FAIL - The object needs to be initialized.
//          E_OUTOFMEMORY - Could not copy one of the strings.
//
// Notes: If a single string fails to copy, all strings are cleared and an 
//  error code is returned. This method treats all SysAllocString failures as
//  out of memory errors. However, if one of the KID strings is NULL the same
//  result occurs. None of the KID strings should be NULL, but if one is it 
//  will cause an error.
// 
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::SetKIDStrings(WCHAR** ppKIDStrings, int NumKIDs)
{
    HRESULT hr = S_OK;

    // Check parameters.
    if (ppKIDStrings == NULL || NumKIDs == 0)
    {
        hr = E_INVALIDARG;
    }

    // Check to ensure that the object has been initialized.
    if (SUCCEEDED(hr))
    {
        if (m_pLicenseQuery == NULL)
        {
            hr = E_FAIL;
        }
    }

    // Check to see if there is an existing array to delete.
    if (SUCCEEDED(hr))
    {
        if (m_KIDStrings != NULL)
        {
            // Free the strings in the old array.
            for (int i = 0; i < m_NumKIDs; i++)
            {
                SysFreeString(m_KIDStrings[i]);
            }

            SAFE_ARRAY_DELETE(m_KIDStrings);

            m_NumKIDs = 0;
        }
    }

    // Allocate the internal array to hold the KID strings.
    if (SUCCEEDED(hr))
    {
        m_NumKIDs = NumKIDs;
        m_KIDStrings = new BSTR[m_NumKIDs];
        
        if (m_KIDStrings == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Initialize the new array.
            ZeroMemory(m_KIDStrings, m_NumKIDs * sizeof(BSTR));
        }
    }

    // Copy the strings to the new array.
    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < m_NumKIDs; i++)
        {
            // Creat a new BSTR from the next KID string.
            m_KIDStrings[i] = SysAllocString(ppKIDStrings[i]);

            if (m_KIDStrings[i] == NULL)
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }

    // Clean up.
    if (FAILED(hr))
    {
        // Delete any allocated strings.
        for (int i = 0; i < m_NumKIDs; i++)
        {
            if (m_KIDStrings[i] != NULL)
            {
                SysFreeString(m_KIDStrings[i]);
            }
        }
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: CreateRightsReport
// Description: Generates the report file.
// Parameters: None.
// Returns: S_OK - The file was written without incident.
//          E_FAIL - The object needs wasn't fully initialized before calling.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::CreateRightsReport()
{
    HRESULT hr = S_OK;
    
    // Check for initialization conditions.
    if ((m_pLicenseQuery == NULL) 
        || (m_KIDStrings == NULL) 
        || (m_pFile == NULL))
    {
        hr = E_FAIL;
    }

    // Write the header to the output file.
    if (SUCCEEDED(hr))
    {
        hr = WriteReportHeader();
    }

    // Loop through the KIDs, getting and reporting rights for each.
    for (int i = 0; i < m_NumKIDs; i++)
    {
        // Write a header for the KID.
        fwprintf(m_pFile, L"****************************************");
        fwprintf(m_pFile, L"**********\n");
        fwprintf(m_pFile, L"* KID : %s\n", m_KIDStrings[i]);
        fwprintf(m_pFile, L"****************************************");
        fwprintf(m_pFile, L"**********\n");
        
        // Get the "is allowed" rights.
        hr = GetRightsForKID(m_KIDStrings[i]);

        if (FAILED(hr))
        {
            break;
        }

        // Get the "license state" rights.
        hr = GetDetailedRightsForKID(m_KIDStrings[i]);

        if (FAILED(hr))
        {
            break;
        }

        // Spacing.
        fwprintf(m_pFile, L"\n");
    }

    if ((FAILED(hr)) && (m_pFile != NULL))
    {
        fwprintf(m_pFile, L"An error occurred while getting rights.\n");
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: ShutDown
// Description: Releases internal resources of the CRightsReporter class.
// Parameters: None.
//
///////////////////////////////////////////////////////////////////////////////
void CRightsReporter::ShutDown()
{
    // Index used by various loops.
    int i = 0;

    // Release the KID strings.
    if (m_KIDStrings != NULL)
    {
        for (i = 0; i < m_NumKIDs; i++)
        {
            SysFreeString(m_KIDStrings[i]);
        }

        SAFE_ARRAY_DELETE(m_KIDStrings);

        m_NumKIDs = 0;
    }

    // Close the output file.
    SAFE_FILE_CLOSE(m_pFile);

    // Release the "is allowed" rights strings.
    if (m_IsAllowedRights != NULL)
    {
        for (i = 0; i < m_cIsAllowedRights; i++)
        {
            SysFreeString(m_IsAllowedRights[i]);
        }

        SAFE_ARRAY_DELETE(m_IsAllowedRights);

        m_cIsAllowedRights = 0;
    }

    // Release the "license state" rights strings.
    if (m_LicenseStateRights != NULL)
    {
        for (i = 0; i < m_cLicenseStateRights; i++)
        {
            SysFreeString(m_LicenseStateRights[i]);
        }

        SAFE_ARRAY_DELETE(m_LicenseStateRights);

        m_cLicenseStateRights = 0;
    }

    // Release the query results arrays.
    SAFE_ARRAY_DELETE(m_IsAllowedResults);
    SAFE_ARRAY_DELETE(m_LicenseStateResults);

    // Release the interfaces.
    SAFE_RELEASE(m_pLicenseQuery);
    SAFE_RELEASE(m_pProvider);

    // Shut down WMDRM.
    WMDRMShutdown();
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: WriteReportHeader
// Description: Writes header information to the output file. This method 
//  should be called only once, before any rights are added tot he report.
// Parameters: None.
// Returns: S_OK - Header written properly.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::WriteReportHeader()
{
    HRESULT hr = S_OK;

    SYSTEMTIME Time;

    // Clear the time structure.
    ZeroMemory(&Time, sizeof(Time));

    // Ensure that there is a file to write to.
    if (m_pFile == NULL)
    {
        return E_FAIL;
    }

    // Get the local time.
    GetLocalTime(&Time);

    fwprintf(m_pFile, L"****************************************************");
    fwprintf(m_pFile, L"****************************\n**\n");
    fwprintf(m_pFile, L"** Windows Media DRM Rights Report\n");
    fwprintf(m_pFile, 
             L"** Created %02d/%02d/%4d %02d:%02d:%02d\n", 
             Time.wMonth, 
             Time.wDay, 
             Time.wYear, 
             Time.wHour, 
             Time.wMinute, 
             Time.wSecond);
    fwprintf(m_pFile, L"**\n************************************************");
    fwprintf(m_pFile, L"********************************\n\n");

    return hr;    
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: GetRightsForKID
// Description: Prints "is allowed" rights information to a report file for 
//  a given Key ID.
// Parameters: KID - Key ID string.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::GetRightsForKID(BSTR KID)
{
    HRESULT hr = S_OK;

    // Get the rights information.
    if (SUCCEEDED(hr))
    {
        hr = m_pLicenseQuery->QueryActionAllowed(KID, 
                                                NULL, 
                                                m_cIsAllowedRights, 
                                                m_IsAllowedRights, 
                                                m_IsAllowedResults);
    }

    // Print the results to the output file.
    if (SUCCEEDED(hr))
    {
        // Loop through the results, reporting on each.
        for (int i = 0; i < m_cIsAllowedRights; i++)
        {
            fwprintf(m_pFile, L"%-35.35s - ", m_IsAllowedRights[i]);

            // Check the query result.
            if (m_IsAllowedResults[i] == 0)
            {
                fwprintf(m_pFile, L"Allowed\n\n");
            }
            else
            {
                // The action is not allowed.
                fwprintf(m_pFile, L"Not Allowed\n");

                // If the action is not allowed, check the result against the
                //  possible failure flags and report the reasons.
                
                // The DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED flag is set for any
                //  not allowed result. Only list it if no other reason is 
                //  provided.
                if ((m_IsAllowedResults[i] & 0xFFFFFFFF)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED)
                {
                    fwprintf(m_pFile, L"   Not allowed for an unspecified reason.\n");
                }

                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_LICENSE)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_LICENSE)
                {
                    fwprintf(m_pFile, L"   No license for this KID.\n");
                }

                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_RIGHT)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_RIGHT)
                {
                    fwprintf(m_pFile, L"   License does not grant this right.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_EXHAUSTED)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_EXHAUSTED)
                {
                    fwprintf(m_pFile, L"   Licensed counts for this right exhausted.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_EXPIRED)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_EXPIRED)
                {
                    fwprintf(m_pFile, L"   License expired.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NOT_STARTED)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NOT_STARTED)
                {
                    fwprintf(m_pFile, L"   License is not yet in effect.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_APPSEC_TOO_LOW)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_APPSEC_TOO_LOW)
                {
                    fwprintf(m_pFile, L"   Application security level is too low.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_REQ_INDIV)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_REQ_INDIV)
                {
                    fwprintf(m_pFile, L"   License requires security update.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_COPY_OPL_TOO_LOW)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_COPY_OPL_TOO_LOW)
                {
                    fwprintf(m_pFile, L"   Cannot copy to the configured device.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_COPY_OPL_EXCLUDED)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_COPY_OPL_EXCLUDED)
                {
                    fwprintf(m_pFile, L"   Configured device excluded from license.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_CLOCK_SUPPORT)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_CLOCK_SUPPORT)
                {
                    fwprintf(m_pFile, L"   License requires a secure clock.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_METERING_SUPPORT)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_NO_METERING_SUPPORT)
                {
                    fwprintf(m_pFile, L"   License requires metering.\n");
                }


                if ((m_IsAllowedResults[i] & DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_CHAIN_DEPTH_TOO_HIGH)
                    == DRM_ACTION_ALLOWED_QUERY_NOT_ENABLED_CHAIN_DEPTH_TOO_HIGH)
                {
                    fwprintf(m_pFile, L"   Incomplete license chain.\n");
                }

                // Print a newline character for spacing between entries.
                fwprintf(m_pFile, L"\n");
            }
        } // End for loop.
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Method: GetDetailedRightsForKID
// Description: Gets license state information for all rights granted for a 
//  Key ID.
// Parameters: KID - Key ID string.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CRightsReporter::GetDetailedRightsForKID(BSTR KID)
{
    HRESULT hr = S_OK;

    SYSTEMTIME FromDate;
    SYSTEMTIME UntilDate;

    DWORD      LicenseStateFlags = 0;

    // Initialize the date structures.
    ZeroMemory(&FromDate, sizeof(FromDate));
    ZeroMemory(&UntilDate, sizeof(UntilDate));

    // Get the license state information.
    hr = m_pLicenseQuery->QueryLicenseState(KID, 
                                            m_cLicenseStateRights,
                                            m_LicenseStateRights,
                                            m_LicenseStateResults);

    if (SUCCEEDED(hr))
    {
        // Loop through the rights, reporting data for each.
        for (int i = 0; i < m_cLicenseStateRights; i++)
        {
            // Report the right.
            fwprintf(m_pFile, L"%s\n", m_LicenseStateRights[i]);

            // Print data from the license state structure based on the 
            //  license state category.
            switch (m_LicenseStateResults[i].dwCategory)
            {
            case WM_DRM_LICENSE_STATE_NORIGHT:
                {
                    fwprintf(m_pFile, L"   Action not allowed.\n");
                    break;
                }
            case WM_DRM_LICENSE_STATE_UNLIM:
                {
                    fwprintf(m_pFile, L"   Action is allowed without restriction.\n");
                    break;
                }
            case WM_DRM_LICENSE_STATE_COUNT:
                {
                    fwprintf(m_pFile, 
                             L"   Action is allowed %d more times.\n", 
                             m_LicenseStateResults[i].dwCount[0]);
                    break;
                }
            case WM_DRM_LICENSE_STATE_FROM:
                {
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[0]), &FromDate);
                    fwprintf(m_pFile, 
                             L"   Action is allowed without restriction beginning %02d/%02d/%4d.\n",
                             FromDate.wMonth, 
                             FromDate.wDay, 
                             FromDate.wYear);
                    break;
                }
            case WM_DRM_LICENSE_STATE_UNTIL:
                {
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[0]), &UntilDate);
                    fwprintf(m_pFile, 
                             L"   Action is allowed without restriction until %02d/%02d/%4d.\n",
                             UntilDate.wMonth, 
                             UntilDate.wDay, 
                             UntilDate.wYear);
                    break;
                }
            case WM_DRM_LICENSE_STATE_FROM_UNTIL:
                {
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[0]), &FromDate);
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[1]), &UntilDate);
                    fwprintf(m_pFile, L"   Action is allowed without restriction between ");
                    fwprintf(m_pFile, 
                             L"%02d/%02d/%4d and %02d/%02d/%4d.\n",
                             FromDate.wMonth,
                             FromDate.wDay,
                             FromDate.wYear,
                             UntilDate.wMonth,
                             UntilDate.wDay,
                             UntilDate.wYear);
                    break;
                }
            case WM_DRM_LICENSE_STATE_COUNT_FROM:
                {
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[0]), &FromDate);
                    fwprintf(m_pFile, 
                             L"   Action is allowed %d more times beginning %02d/%02d/%4d.\n",
                             m_LicenseStateResults[i].dwCount[0],
                             FromDate.wMonth, 
                             FromDate.wDay, 
                             FromDate.wYear);
                    break;
                }
            case WM_DRM_LICENSE_STATE_COUNT_UNTIL:
                {
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[0]), &UntilDate);
                    fwprintf(m_pFile, 
                             L"   Action is allowed %d more times until %02d/%02d/%4d.\n",
                             m_LicenseStateResults[i].dwCount[0],
                             UntilDate.wMonth, 
                             UntilDate.wDay, 
                             UntilDate.wYear);
                    break;
                }
            case WM_DRM_LICENSE_STATE_COUNT_FROM_UNTIL:
                {
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[0]), &FromDate);
                    FileTimeToSystemTime(&(m_LicenseStateResults[i].datetime[1]), &UntilDate);
                    fwprintf(m_pFile, 
                             L"   Action is allowed %d more times between ",
                             m_LicenseStateResults[i].dwCount[0]);
                    fwprintf(m_pFile, 
                             L"%02d/%02d/%4d and %02d/%02d/%4d.\n",
                             FromDate.wMonth,
                             FromDate.wDay,
                             FromDate.wYear,
                             UntilDate.wMonth,
                             UntilDate.wDay,
                             UntilDate.wYear);
                    break;
                }
            case WM_DRM_LICENSE_STATE_EXPIRATION_AFTER_FIRSTUSE:
                {
                    fwprintf(m_pFile, L"   Action expires after the first use.\n");
                    break;
                }
            } // End switch.

            // Check for other license information.
            
            LicenseStateFlags = m_LicenseStateResults[i].dwVague;

            // Vague.
            if ((LicenseStateFlags & DRM_LICENSE_STATE_DATA_VAGUE)
                == DRM_LICENSE_STATE_DATA_VAGUE)
            {
                fwprintf(m_pFile, 
                    L"   Note: License state data aggregated from multiple licenses.\n");
            }

            // OPLs.
            if ((LicenseStateFlags & DRM_LICENSE_STATE_DATA_OPL_PRESENT)
                == DRM_LICENSE_STATE_DATA_OPL_PRESENT)
            {
                fwprintf(m_pFile, 
                    L"   Note: This action is further restricted with Output Protection Levels.\n");
            }

            // SAP.
            if ((LicenseStateFlags & DRM_LICENSE_STATE_DATA_SAP_PRESENT)
                == DRM_LICENSE_STATE_DATA_SAP_PRESENT)
            {
                fwprintf(m_pFile, L"    Note: This action may only be performed using SAP.\n");
            }

            fwprintf(m_pFile, L"\n");

        } // End for.
    }

    return hr;
}