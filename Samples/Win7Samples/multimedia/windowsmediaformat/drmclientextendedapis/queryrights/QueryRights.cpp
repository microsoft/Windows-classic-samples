///////////////////////////////////////////////////////////////////////////////
//
// QueryRights.cpp : Application entry for the QureyRights sample application.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <wmdrmsdk.h>
#include "DRMSampleUtils.h"
#include "CRightsReporter.h"
#include <strsafe.h>



///////////////////////////////////////////////////////////////////////////////
// Function: wmain
// Description: Application entry point. The QueryRights application accepts
//  a single argument specifying the name of a text file containing a list of
//  key IDs. The application create a report text file that contains the 
//  license status for each right for each KID.
///////////////////////////////////////////////////////////////////////////////
void wmain(int argc, wchar_t **argv)
{
    HRESULT hr = S_OK;

    // Rights reporter object.
    CRightsReporter RightsReporter;

    // Value to hold file information used in validating the input.
    DWORD   DirectoryFlags = 0;

    // Array of KID strings.
    WCHAR** ppKIDStrings = NULL;
    int     NumKids      = 0;

    // Initialize the rights reporter object.
    // This call starts COM and WMDRM as well as allocating internal resources.
    hr = RightsReporter.Initialize();
    
    // Validate the command-line arguments.
    if (SUCCEEDED(hr))
    {
        if (argc == 3)
        {
            // The first argument should be a valid filename.
            DirectoryFlags = GetFileAttributes(argv[1]);

            if (DirectoryFlags == INVALID_FILE_ATTRIBUTES)
            {
                hr = E_INVALIDARG;
                DisplayError(hr, L"Specified input filename is invalid.\n");
            }

            // The second argument needn't be an existing file, but it must be
            //  a valid file path. To test this, have the rights reporter 
            //  object open the output file.
            if (SUCCEEDED(hr))
            {
                hr = RightsReporter.SetReportFile(argv[2]);

                if (FAILED(hr))
                {                
                    DisplayError(hr, L"Specified output filename is invalid.\n");
                }                   
            }
        }
        else
        {
            // An invalid number of arguments was passed.
            hr = E_INVALIDARG;
            DisplayError(hr, L"Program called with incorrect number of paramters.\n");
        }
    }

    // Parse the input file.
    if (SUCCEEDED(hr))
    {
        hr = ParseKIDFile(argv[1], &ppKIDStrings, &NumKids);
    }

    // Set the KID list in the rights reporter object.
    if (SUCCEEDED(hr))
    {
        hr = RightsReporter.SetKIDStrings(ppKIDStrings, NumKids);
    }

    // Create the rights report.
    if (SUCCEEDED(hr))
    {
        hr = RightsReporter.CreateRightsReport();
    }

    // Print acknowledgement of success, if applicable.
    if (SUCCEEDED(hr))
    {
        wprintf(L"Report successfully generated.\n");
    }
    else
    {
        wprintf(L"Report not completed.\n");
    }

    // Print usage informaiton, if execution failed because of bad parameters.
    if (hr == E_INVALIDARG)
    {
        wprintf(L"Usage:\n\tQueryRights.exe <inputfile> <outputfile>\n");
    }

    
    // Delete the array of KIDs.
    if (ppKIDStrings != NULL)
    {
        for (int i = 0; i < NumKids; i++)
        {
            SAFE_ARRAY_DELETE(ppKIDStrings[i]);
        }

        SAFE_ARRAY_DELETE(ppKIDStrings);
    }
}

