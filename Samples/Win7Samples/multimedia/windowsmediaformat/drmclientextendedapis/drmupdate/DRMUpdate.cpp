///////////////////////////////////////////////////////////////////////////////
//
// DRMUpdate.cpp : Contains application entry and functions for the DRMUpdate
//  sample.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <wmdrmsdk.h>
#include "DRMSampleUtils.h"
#include <strsafe.h>

///////////////////////////////////////////////////////////////////////////////
// Function Prototypes
///////////////////////////////////////////////////////////////////////////////
HRESULT DisplaySecurityVersion(IWMDRMSecurity* pSecurity);
HRESULT Individualize(IWMDRMSecurity* pSecurity, BOOL fForce, FILE* pLogFile);
HRESULT TryOpenFile(wchar_t* FileName, FILE** ppFile);

///////////////////////////////////////////////////////////////////////////////
//
// Function: wmain
//
// Description: Application entry point. The DRMUpdate application is a simple,
//  command-line way to individualize the DRM component. DRMUpdate displays the
//  current security version, then performs individualization and displayes the 
//  post-update security version. Status messages are displayed as they are
//  received.
//
// Parameters: The application take a single optional parameter, -force. If 
//  set, the -force parameter causes the application to force individualization
//  instead of letting the DRM subsystem determine whether individualization is
//  required.
//
///////////////////////////////////////////////////////////////////////////////
void wmain(int argc, wchar_t **argv)
{
    HRESULT hr     = S_OK;

    BOOL    fForce = FALSE;
    FILE*   pFile  = NULL;

    IWMDRMProvider*         pProvider       = NULL;
    IWMDRMSecurity*         pSecurity       = NULL;
 
    // Check the arguments.
    if (argc == 1)
    {
        // Invalid number of arguments.
        hr = E_INVALIDARG;
    }
    else if (argc == 2)
    {
        // If a valid call, there will be an output filename only.
        hr = TryOpenFile(argv[1], &pFile);
    }
    else if (argc == 3)
    {
        // The first argument should be the output filename.
        hr = TryOpenFile(argv[1], &pFile);
        
        // Check the second argument for the force flag.        
        if (wcsstr(argv[2], L"-force") != NULL)
        {
            fForce = TRUE;
        }        
    }
    else
    {
        // Invalid number of arguments.
        hr = E_INVALIDARG;
    }

    // Several branches of the argument checking above can set hr to 
    //  E_INVALIDARG. In all cases, this indicates that the passed arguments
    //  were incorrect. Print a message to that effect.
    if (hr == E_INVALIDARG)
    {
        printf("Invalid command-line arguments. Use the following format:\n");
        printf("DRMUpdate.exe <output file name> [-force]\n");
        printf("Where <> indicates vriable text and [] indicates an optional argument.\n");
    }
   
    // Initialize DRM.
    if (SUCCEEDED(hr))
    {
        hr = WMDRMStartup();
    }

    // Create a DRM provider.
    if (SUCCEEDED(hr))
    {
        hr = WMDRMCreateProvider(&pProvider);
    }

    // Create a security object.
    if (SUCCEEDED(hr))
    {
        hr = pProvider->CreateObject(IID_IWMDRMSecurity, (void**)&pSecurity);
    }

    // Display the security version before updating.
    if (SUCCEEDED(hr))
    {
        hr = DisplaySecurityVersion(pSecurity);
    }

    // Perform individualization.
    if (SUCCEEDED(hr))
    {
        hr = Individualize(pSecurity, fForce, pFile);
    }

    // The security interface must be re-initialized in order to get an 
    //  accurate security version after indiv.
    if (SUCCEEDED(hr))
    {  
        SAFE_RELEASE(pSecurity);

        hr = pProvider->CreateObject(IID_IWMDRMSecurity, (void**)&pSecurity);
    }

    // Display the security version again now that an update happened.
    if (SUCCEEDED(hr))
    {
        hr = DisplaySecurityVersion(pSecurity);
    }

    // Clean up.
    SAFE_RELEASE(pSecurity);
    SAFE_RELEASE(pProvider);
    SAFE_FILE_CLOSE(pFile);

    WMDRMShutdown();

}

///////////////////////////////////////////////////////////////////////////////
//
// Function: DisplaySecurityVersion
// Description: Retrieves the current DRM security version and prints it to the
//  console.
// Parameters: pSecurity - Security interface pointer.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT DisplaySecurityVersion(IWMDRMSecurity* pSecurity)
{
    HRESULT hr = S_OK;

    BSTR    bstrVersion = NULL;

    // Get the security version.
    hr = pSecurity->GetSecurityVersion(&bstrVersion);

    if (SUCCEEDED(hr))
    {
        printf("Security version = %S\n", bstrVersion);
    }

    // Release the string.
    if (bstrVersion != NULL)
    {
        SysFreeString(bstrVersion);
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function: Individualize
// Description: Individualizes the DRM subsystem, reporting progress to the 
//  specified log file.
// Parameters: pSecurity - Security interface to use for individualization.
//             fForce    - Flag indicating whether to force individualization.
//             pLogFile  - Pointer to an opened file that will be overwritten 
//                         with the individualization event log.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT Individualize(IWMDRMSecurity* pSecurity, BOOL fForce, FILE* pLogFile)
{
    HRESULT hr = S_OK;


    IMFMediaEvent* pEvent          = NULL;
    IUnknown*      pCancelCookie   = NULL;
    IWMDRMIndividualizationStatus*  pIndivStatus = NULL;
    MediaEventType EventType       = MEUnknown;
    HRESULT        EventHR         = E_FAIL;

    DWORD          cEvents         = 0;

    WM_INDIVIDUALIZE_STATUS IndivStatusStruct;

    // Intialize structures.
    ZeroMemory(&IndivStatusStruct, sizeof(IndivStatusStruct));
    PROPVARIANT    EventValue;
    
    DWORD dwFlags = 0;  

    // Initialize the property variant.
    PropVariantInit(&EventValue);

    // Check input pointers.
    if (pSecurity == NULL || pLogFile == NULL)
    {
        return E_POINTER;
    }

    // Configure the flag for individualization.
    if (fForce == TRUE)
    {
        dwFlags = WMDRM_SECURITY_PERFORM_FORCE_INDIV;
        printf("Performing forced individualization.\n"); 
    }
    else
    {
        dwFlags = WMDRM_SECURITY_PERFORM_INDIV;
        printf("Performing as-needed individualization.\n");
    }

    // Write a header for the log file.
    fprintf(pLogFile, "Windows Media DRM Individualization Log\n\n");

    // Start the security update.
    hr = pSecurity->PerformSecurityUpdate(dwFlags, &pCancelCookie);

    // Status loop.
    if (SUCCEEDED(hr))
    { 
        do
        {
            // Release the previous event (if there is one).
            SAFE_RELEASE(pEvent);
            SAFE_RELEASE(pIndivStatus);

            // Check for an event.
            hr = pSecurity->GetEvent(MF_EVENT_FLAG_NO_WAIT, &pEvent);

            // If an event was not found, wait a second and try again.
            if (FAILED(hr))
            {
                printf(".");
                Sleep(1000);
                continue;
            }

            // Got an event. Increment the event count.
            cEvents ++;

            // Get the event type.
            if (SUCCEEDED(hr))
            {
                hr = pEvent->GetType(&EventType);
            }

            if (SUCCEEDED(hr))
            {
                // Display a message depending on the event.
                switch (EventType)
                {
                case MEWMDRMIndividualizationProgress:
                    
                    fprintf(pLogFile, "%d.) Received a progress event.\n", cEvents);
                    
                    hr = pEvent->GetValue(&EventValue);
                    if (SUCCEEDED(hr))
                    {
                        if (EventValue.vt == VT_UNKNOWN)
                        {
                            // Get the extended status interface.
                            hr = EventValue.punkVal->QueryInterface(
                                IID_IWMDRMIndividualizationStatus, 
                                (void**)&pIndivStatus);

                            // If the interface can't be gotten, make a note in the log and continue.
                            if (FAILED(hr))
                            {
                                fprintf(pLogFile, "   Unable to get the extended status interface.\n");
                                PropVariantClear(&EventValue);
                                break;
                            }

                            // Get the status structure from the newly acquired interface.
                            hr = pIndivStatus->GetStatus(&IndivStatusStruct);

                            // If the structure can't be gotten, make a note in the log and continue.
                            if (FAILED(hr))
                            {
                                fprintf(pLogFile, "   Unable to get the status structure.\n");
                                PropVariantClear(&EventValue);
                                break;
                            }

                            //////////
                            //
                            // Log the individualization status (all items for now).

                            // The HRESULT.
                            if (IndivStatusStruct.hr == S_OK)
                            {
                                fprintf(pLogFile, "   hr = S_OK\n");
                            }
                            else
                            {
                                fprintf(pLogFile, "   hr = 0x%08X\n", IndivStatusStruct.hr);
                            }

                            // Current status of the overall individualization process.
                            switch (IndivStatusStruct.enIndiStatus)
                            {
                            case INDI_BEGIN:
                                fprintf(pLogFile, "   Indiv Status = INDI_BEGIN\n");
                                break;
                            case INDI_DOWNLOAD:
                                fprintf(pLogFile, "   Indiv Status = INDI_DOWNLOAD\n");
                                break;
                            case INDI_INSTALL:
                                fprintf(pLogFile, "   Indiv Status = INDI_INSTALL\n");
                                break;
                            case INDI_SUCCEED:
                                fprintf(pLogFile, "   Indiv Status = INDI_SUCCEED\n");
                                break;
                            case INDI_FAIL:
                                fprintf(pLogFile, "   Indiv Status = INDI_FAIL\n");
                                break;
                            case INDI_CANCEL:
                                fprintf(pLogFile, "   Indiv Status = INDI_CANCEL\n");
                                break;
                            case INDI_UNDEFINED:
                                fprintf(pLogFile, "   Indiv Status = INDI_UNDEFINED\n");
                                break;
                            default:
                                fprintf(pLogFile, "   Indiv Status = Invalid member.\n");
                            }

                            // Individualization response URL.
                            fprintf(pLogFile, "   URL = %s\n", IndivStatusStruct.pszIndiRespUrl);

                            // Number of HTTP requests (round-trip).
                            fprintf(pLogFile, "   HTTP Requests = %d\n", IndivStatusStruct.dwHTTPRequest);

                            // HTTP status.
                            switch (IndivStatusStruct.enHTTPStatus)
                            {
                            case HTTP_NOTINITIATED:
                                fprintf(pLogFile, "   HTTP Status = HTTP_NOTINITIATED.\n");
                                break;
                            case HTTP_CONNECTING:
                                fprintf(pLogFile, "   HTTP Status = HTTP_CONNECTING.\n");
                                break;
                            case HTTP_REQUESTING:
                                fprintf(pLogFile, "   HTTP Status = HTTP_REQUESTING.\n");
                                break;
                            case HTTP_RECEIVING:
                                fprintf(pLogFile, "   HTTP Status = HTTP_RECEIVING.\n");
                                break;
                            case HTTP_COMPLETED:
                                fprintf(pLogFile, "   HTTP Status = HTTP_COMPLETED.\n");
                                break;
                            default:
                                fprintf(pLogFile, "   HTTP Status = Invalid member.\n");
                            }

                            // HTTP progress.
                            fprintf(pLogFile, 
                                "   HTTP Progress = %d of %d bytes downloaded.\n\n", 
                                IndivStatusStruct.dwHTTPReadProgress, 
                                IndivStatusStruct.dwHTTPReadTotal);

                            //
                            //////////

                            // Clear the event value for the next iteration.
                            PropVariantClear(&EventValue);
                        }
                        else
                        {
                            // All progress events should have a value of type
                            //  VT_UNKNOWN. Note this in the log and continue.
                            fprintf(pLogFile, "   Unexpected Value Type.\n");
                            PropVariantClear(&EventValue);
                            break;
                        }
                    }
                    break;
                case MEWMDRMIndividualizationCompleted:
                    fprintf(pLogFile, "%d. Individualization completed.\n", cEvents);
                    break;
                default:
                    fprintf(pLogFile, "Received event number %d.\n", EventType);
                } 
            }

            // Give up CPU time to stay out of a busy loop.
            Sleep(0);

        } while (EventType != MEWMDRMIndividualizationCompleted);

        // Print a newline character to the console so that subsequent events 
        //  will display properly.
        printf("\n");
 
    }

    SAFE_RELEASE(pEvent);
    SAFE_RELEASE(pCancelCookie);

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// Function: TryOpenFile
// Description: Attempts to open a file. This function is used not only to open
//  a file for output, but also as a way of validating a filename entered by
//  the user.
// Parameters: FileName - Name of the flie to open.
//             ppFile   - Address of a variable that will hold the address of
//                         the newly created file.
// Remarks: If the file cannot be created, this function resturns E_INVALIDARG.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT TryOpenFile(wchar_t* FileName, FILE** ppFile)
{
    HRESULT hr = S_OK;

    if (_wfopen_s(ppFile, FileName, L"w") != 0)
    {
       hr = E_INVALIDARG; 
    }

    return hr;
}