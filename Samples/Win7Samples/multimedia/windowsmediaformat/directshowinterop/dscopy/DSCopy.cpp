//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DSCopy.cpp
//
// Abstract:            Windows Media / DirectShow sample code
//
//*****************************************************************************

#include <windows.h>
#include <strmif.h>
#include <vfwmsgs.h>
#include <uuids.h>
#include "control.h"
#include "evcode.h"

#include <atlbase.h>
#include <stdio.h>

// Disable warning C4268, which is generated within <wmsdk.h>
#pragma warning(disable:4268)
#include <wmsdk.h>
#include <dshowasf.h>
#pragma warning(default:4268)

#pragma warning(disable: 4100)

// 
// Constants
//
#define DEFAULT_PROFILE_VERSION     WMT_VER_8_0
#define PROFILE_VERSION_NUM        (g_WMTVersion >> 16)

// Unique string name for CreateEvent() (simply a GUID)
#define WMVCOPY_INDEX_EVENT   TEXT("{78268A45-34B2-489a-838B-38833C949CBF}")

#define USAGE_STRING \
_T("Usage: DSCopy [/v] [/l] [/f] [/m] [/n ProfileVersion]\n")                            \
_T("/p ProfileNumber Source1 [Source2 ...] Target\n\n")                                  \
_T("The following command-line switches are supported:\n")                               \
_T("    /v Verbose mode\n")                                                              \
_T("    /l Lists all available system profiles (versions 4,7,8)\n")                      \
_T("    /f Selects frame-based indexing (instead of temporal)\n")                        \
_T("    /m Enables multipass encoding\n")                                                \
_T("    /n Selects a system profile version (4, 7, or 8)\n")                             \
_T("    /p Specifies the profile number\n\n")                                            \
_T("Specify an ASF profile using the /p switch.  If you omit this switch, ASFCopy \n")   \
_T("displays a list of the standard system profiles and exits.\n\n")                     \
_T("Specify the name of one or more source files and the name of the target file. \n")   \
_T("If you specify more than one source file, the application multiplexes all of \n")    \
_T("the source files.  You must specify a profile that matches the streams \n")          \
_T("contained in the source files, or else the application will not work correctly. \n") \
_T("For example, if you specify Video for Web Servers (56 Kbps), the combined \n")       \
_T("source files must have exactly one video stream and one audio stream.\n")            \

//
// Macros
//
#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

//
// Global data
//
WMT_VERSION g_WMTVersion = DEFAULT_PROFILE_VERSION;

BOOL fVerbose = FALSE, fListProfiles = TRUE;
BOOL fListAllProfiles = FALSE, fFrameIndexing = FALSE, fMultipassEncode = FALSE;

//
// Function prototypes
//
void ListAllProfiles(void);
void ListProfiles(WMT_VERSION ProfileVersion);
LONG WaitForCompletion( IGraphBuilder *pGraph );

HRESULT CreateFilterGraph(IGraphBuilder **pGraph);
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter);
HRESULT SetNoClock(IFilterGraph *pGraph);
HRESULT MapProfileIdToProfile(int iProfile, IWMProfile **ppProfile);
HRESULT IndexFileByFrames(__in LPWSTR wszTargetFile);


//////////////////////////////////////////////////////////////////////////////////
// This class implements the methods of the IWMStatusCallback interface               //
//////////////////////////////////////////////////////////////////////////////////

class CIndexCallback : public IWMStatusCallback
{
public:
    CIndexCallback()
    {
        phr = NULL ;
        hEvent = NULL;
    }

    ~CIndexCallback(){}

    virtual HRESULT STDMETHODCALLTYPE OnStatus(
                          /* [in] */ WMT_STATUS Status,
                          /* [in] */ HRESULT hr,
                          /* [in] */ WMT_ATTR_DATATYPE dwType,
                          /* [in] */ BYTE __RPC_FAR *pValue,
                          /* [in] */ void __RPC_FAR *pvContext)
    {
        switch ( Status )
        {
            case WMT_INDEX_PROGRESS:
                // Display the indexing progress as a percentage.
                // Use "carriage return" (\r) to reuse the status line.
                _tprintf(_T("Indexing in progress (%d%%)\r"), *pValue);
                break ;

            case WMT_CLOSED:
                *phr = hr;
                SetEvent(hEvent) ;
                _tprintf(_T("\n"));   // Move to new line (past progress line)
                break;

            case WMT_ERROR:
                *phr = hr;
                SetEvent(hEvent) ;
                _tprintf(_T("\nError during indexing operation! hr=0x%x\n"), hr);
                break;

            // Ignore these messages
            case WMT_OPENED:
            case WMT_STARTED:
            case WMT_STOPPED:
                break;
        }
        return S_OK;
    }

    //------------------------------------------------------------------------------
    // Implementation of IUnknown methods
    //------------------------------------------------------------------------------
    ULONG STDMETHODCALLTYPE AddRef( void )
    {
        return 1;
    }

    ULONG STDMETHODCALLTYPE Release( void )
    {
        return 1;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
                        /* [in] */ REFIID riid,
                        /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
    {
        if ( riid == IID_IWMStatusCallback )
        {
            *ppvObject = ( IWMStatusCallback * )this;
        }
        else
        {
            *ppvObject = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

public:
    HANDLE    hEvent ;
    HRESULT   *phr ;
};

//////////////////////////////////////////////////////////////////////////////////
// End callback class
//////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// Name: ListAllProfiles()
// Desc: Displays profiles for all versions.
//------------------------------------------------------------------------------
void ListAllProfiles(void)
{
    WMT_VERSION ver[3] = { WMT_VER_4_0, WMT_VER_7_0, WMT_VER_8_0};

    // List all system profiles supported for each Windows Media version
    for (int i=0; i < 3; i++)
    {
        ListProfiles(ver[i]);
    }
}

//------------------------------------------------------------------------------
// Name: ListProfiles()
// Desc: Displays profiles for a specified version.
//------------------------------------------------------------------------------
void ListProfiles(WMT_VERSION ProfileVersion)
{
    USES_CONVERSION;

    DWORD cProfiles = 0;
    DWORD cchName, cchDescription;
    CComPtr <IWMProfileManager> pIWMProfileManager;

    // Create a profile manager object
    HRESULT hr = WMCreateProfileManager(&pIWMProfileManager);
    if(FAILED(hr))
    {
        _tprintf(_T("ListProfiles: Failed to create profile manager!  hr=0x%x\n"), hr);
        return;
    }

    CComQIPtr<IWMProfileManager2, &IID_IWMProfileManager2> pIPM2(pIWMProfileManager);
    if(!pIPM2) 
    {
        _tprintf(_T("ListProfiles: Failed to QI IWMProfileManager2!  hr=0x%x\n"), hr);
        return;
    }

    // Set to the requested system profile version
    hr = pIPM2->SetSystemProfileVersion( ProfileVersion );
    if(FAILED(hr)) 
    {
        _tprintf(_T("ListProfiles: Failed to set system profile version!  hr=0x%x\n"), hr);
        return;
    }

    // Read back the current version to verify and save it to global variable
    hr = pIPM2->GetSystemProfileVersion( &g_WMTVersion );
    if(FAILED(hr)) 
    {
        _tprintf(_T("ListProfiles: Failed to set system profile version!  hr=0x%x\n"), hr);
        return;
    }

    // How many system profiles exist for this version?
    hr = pIWMProfileManager->GetSystemProfileCount(&cProfiles);
    if(FAILED(hr))
    {
        _tprintf(_T("ListProfiles: Failed to read system profile count!  hr=0x%x\n"), hr);
        return;
    }
    else
        _tprintf(_T("There are %d system profiles available for version %d.\n"), 
               cProfiles, g_WMTVersion >> 16);

    // Load the profile strings
    for(int i = 0; i < (int) cProfiles; ++i)
    {
        CComPtr <IWMProfile> pIWMProfile;

        hr = pIWMProfileManager->LoadSystemProfile(i, &pIWMProfile);
        if(FAILED(hr))
        {
            _tprintf(_T("ListProfiles: Failed to load system profile!  hr=0x%x\n"), hr);
            return;
        }

        // How large is the profile name?
        hr = pIWMProfile->GetName(NULL, &cchName);
        if(FAILED(hr))
        {
            _tprintf(_T("ListProfiles: Failed to read profile name size!  hr=0x%x\n"), hr);
            return;
        }

        // Allocate a string to hold the profile name
        WCHAR *wszProfile = new WCHAR[ cchName ];
        if(NULL == wszProfile)
            return;

        // Read the profile name into the newly allocated string
        hr = pIWMProfile->GetName(wszProfile, &cchName);
        if(FAILED(hr))
        {
            _tprintf(_T("ListProfiles: Failed to read profile name!  hr=0x%x\n"), hr);
            delete [] wszProfile;
            return;
        }

        if (fVerbose)
        {
            // How large is the description?
            hr = pIWMProfile->GetDescription(NULL, &cchDescription);
            if(FAILED(hr))
            {
                _tprintf(_T("ListProfiles: Failed to read profile description size!  hr=0x%x\n"), hr);
                delete [] wszProfile;
                return;
            }

            // Allocate a string to hold the profile description
            WCHAR *wszDescription = new WCHAR[ cchDescription ];
            if(NULL == wszDescription)
            {
                delete [] wszProfile;
                return;
            }

            // Read the description into the newly allocated string
            hr = pIWMProfile->GetDescription(wszDescription, &cchDescription);
            if(FAILED(hr))
            {
                _tprintf(_T("ListProfiles: Failed to read profile description!  hr=0x%x\n"), hr);
                delete [] wszProfile;
                delete [] wszDescription;
                return;
            }

            // Display the profile name and description
            _tprintf(_T("  %3d:  %ls \n[%ls]\n\n"), i, wszProfile, wszDescription);
            delete[] wszDescription;
        }

        // Not verbose mode, so display only the profile's name
        else
            _tprintf(_T("  %3d:  %ls\n"), i, wszProfile);

        delete[] wszProfile;
    }

    _tprintf(_T("\n"));
}


//------------------------------------------------------------------------------
// Name: CreateFilterGraph()
// Desc: Create a DirectShow filter graph.
//------------------------------------------------------------------------------
HRESULT CreateFilterGraph(IGraphBuilder **pGraph)
{
    HRESULT hr;

    if (!pGraph)
        return E_POINTER;

    hr = CoCreateInstance(CLSID_FilterGraph, // get the graph object
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IGraphBuilder,
                          (void **) pGraph);

    if(FAILED(hr))
    {
        _tprintf(_T("CreateFilterGraph: Failed to create graph!  hr=0x%x\n"), hr);
        *pGraph = NULL;
        return hr;
    }

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: CreateFilter()
// Desc: Create a DirectShow filter.
//------------------------------------------------------------------------------
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{
    HRESULT hr;

    if (!ppFilter)
        return E_POINTER;

    hr = CoCreateInstance(clsid,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IBaseFilter,
                          (void **) ppFilter);

    if(FAILED(hr))
    {
        _tprintf(_T("CreateFilter: Failed to create filter!  hr=0x%x\n"), hr);
        *ppFilter = NULL;
        return hr;
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: SetNoClock()
// Desc: Prevents an unnecessary clock from being created.
// This speeds up the copying process, since the renderer won't wait
// for the proper time to render a sample; instead, the data will
// be processed as fast as possible.
//------------------------------------------------------------------------------
HRESULT SetNoClock(IFilterGraph *pGraph)
{
    if (!pGraph)
        return E_POINTER;

    IMediaFilter *pFilter=NULL;
    HRESULT hr = pGraph->QueryInterface(IID_IMediaFilter, (void **) &pFilter);

    if(SUCCEEDED(hr))
    {
        // Set to "no clock"
        hr = pFilter->SetSyncSource(NULL);
        if (FAILED(hr))
            _tprintf(_T("SetNoClock: Failed to set sync source!  hr=0x%x\n"), hr);

        pFilter->Release();
    }
    else
    {
        _tprintf(_T("SetNoClock: Failed to QI for media filter!  hr=0x%x\n"), hr);
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: MapProfileIdToProfile()
// Desc: Retrieves a specified profile.
//------------------------------------------------------------------------------
HRESULT MapProfileIdToProfile(int iProfile, IWMProfile **ppProfile)
{
    DWORD cProfiles;

    if (!ppProfile)
        return E_POINTER;
        
    *ppProfile = 0;
    
    CComPtr <IWMProfileManager> pIWMProfileManager;
    HRESULT hr = WMCreateProfileManager( &pIWMProfileManager );
    if(FAILED(hr)) 
    {
        _tprintf(_T("MapProfile: Failed to create profile manager!  hr=0x%x\n"), hr);
        return hr;
    }

    CComQIPtr<IWMProfileManager2, &IID_IWMProfileManager2> pIPM2(pIWMProfileManager);
    if(!pIPM2) 
    {
        _tprintf(_T("MapProfile: Failed to QI IWMProfileManager2!\n"));
        return E_UNEXPECTED;
    }

    // Set the system profile version (4, 7, 8)
    hr = pIPM2->SetSystemProfileVersion( g_WMTVersion );
    if(FAILED(hr))
    {
        _tprintf(_T("MapProfile: Failed to set system profile version!  hr=0x%x\n"), hr);
        return hr;
    }

    // Read back the current version to verify
    hr = pIPM2->GetSystemProfileVersion( &g_WMTVersion );
    if(FAILED(hr)) 
    {
        _tprintf(_T("ListProfiles: Failed to read system profile version!  hr=0x%x\n"), hr);
        return hr;
    }

    // How many profiles exist for this version?
    hr = pIWMProfileManager->GetSystemProfileCount( &cProfiles );
    if(FAILED(hr))
    {
        _tprintf(_T("MapProfile: Failed to get system profile count!  hr=0x%x\n"), hr);
        return hr;
    }

    // Invalid profile requested?
    if( (DWORD)iProfile >= cProfiles ) 
    {
        _tprintf(_T("Invalid profile: %d\n"), iProfile);
        return E_INVALIDARG;
    }

    // Load the requested systme profile and return its interface
    // in the ppProfile parameter
    return (pIWMProfileManager->LoadSystemProfile( iProfile, ppProfile ));
}

//------------------------------------------------------------------------------
// Name: FindPinOnFilter()
// Desc: Retrieves a specified pin.
//------------------------------------------------------------------------------
HRESULT FindPinOnFilter( IBaseFilter * pFilter, PIN_DIRECTION PinDir,
                         DWORD dwPin, BOOL fConnected, IPin ** ppPin )
{
    HRESULT         hr = S_OK;
    IEnumPins *     pEnumPin = NULL;
    IPin *          pConnectedPin = NULL;
    PIN_DIRECTION   PinDirection;
    ULONG           ulFetched;
    DWORD           nFound = 0;

    if (!pFilter || !ppPin)
        return E_POINTER;
        
    *ppPin = NULL;

    // Get a pin enumerator for the filter's pins
    hr = pFilter->EnumPins( &pEnumPin );
    if(SUCCEEDED(hr))
    {
        // Explicitly check for S_OK instead of SUCCEEDED, since IEnumPins::Next()
        // will return S_FALSE if it does not retrieve as many pins as requested.
        while ( S_OK == ( hr = pEnumPin->Next( 1L, ppPin, &ulFetched ) ) )
        {
            hr = (*ppPin)->ConnectedTo( &pConnectedPin );
            if (pConnectedPin)
            {
                pConnectedPin->Release();
                pConnectedPin = NULL;
            }

            if ( ( ( VFW_E_NOT_CONNECTED == hr ) && !fConnected ) ||
                 ( ( SUCCEEDED(hr) ) && fConnected ) )
            {
                hr = (*ppPin)->QueryDirection( &PinDirection );
                if ( ( SUCCEEDED(hr) ) && ( PinDirection == PinDir ) )
                {
                    if ( nFound == dwPin ) 
                        break;

                    nFound++;
                }
            }

            (*ppPin)->Release();
        }

        // Release enumerator
        pEnumPin->Release();
    }

    return hr;   
}

//------------------------------------------------------------------------------
// Name: WaitForCompletion()
// Desc: Waits for a media event that signifies completion or cancellation
//       of a task.
//------------------------------------------------------------------------------
LONG WaitForCompletion( IGraphBuilder *pGraph )
{
    HRESULT hr;
    LONG levCode = 0;
    CComPtr <IMediaEvent> pME;

    if (!pGraph)
        return -1;
        
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **) &pME);
    if (SUCCEEDED(hr))
    {
        _tprintf(_T("Waiting for completion...\n  This could take several minutes, ")
                 _T("depending on file size and selected profile.\n"));
        HANDLE hEvent;
        
        hr = pME->GetEventHandle((OAEVENT *)&hEvent);
        if(SUCCEEDED(hr)) 
        {
            // Wait for completion and dispatch messages for any windows
            // created on our thread.
            for(;;)
            {
                while(MsgWaitForMultipleObjects(
                    1,
                    &hEvent,
                    FALSE,
                    INFINITE,
                    QS_ALLINPUT) != WAIT_OBJECT_0)
                {
                    MSG Message;

                    while (PeekMessage(&Message, NULL, 0, 0, TRUE))
                    {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                    }
                }

                // Event signaled. See if we're done.
                LONG_PTR lp1, lp2;

                if(pME->GetEvent(&levCode, &lp1, &lp2, 0) == S_OK)
                {
                    pME->FreeEventParams(levCode, lp1, lp2);
                
                    if(EC_COMPLETE == levCode)
                    {
                        // Display received event information
                        if (fVerbose)
                        {
                            _tprintf(_T("WaitForCompletion: Received EC_COMPLETE.\n"));
                        }                            
                        break;
                    }
                    else if(EC_ERRORABORT == levCode)
                    {
                        if (fVerbose)
                        {
                            _tprintf(_T("WaitForCompletion: Received EC_ERRORABORT.\n"));
                        }                            
                        break;
                    }
                    else if(EC_USERABORT == levCode)
                    {
                        if (fVerbose)
                        {
                            _tprintf(_T("WaitForCompletion: Received EC_USERABORT.\n"));
                        }
                        break;
                    }
                    else if( EC_PREPROCESS_COMPLETE == levCode)
                    {        
                        if (fVerbose)
                        {
                            _tprintf(_T("WaitForCompletion: Received EC_PREPROCESS_COMPLETE.\n"));
                        }
                        break;
                    }
                    else
                    {        
                        if (fVerbose)
                        {
                            _tprintf(_T("WaitForCompletion: Received event %d.\n"), levCode);
                        }
                    }
                }
            }
        }
        else
        {
            _tprintf(_T("Unexpected failure (GetEventHandle failed)...\n"));
        }
    }        
    else
        _tprintf(_T("QI failed for IMediaEvent interface!\n"));

    return levCode;
}

//------------------------------------------------------------------------------
// Name: IndexFileByFrames()
// Desc: Index the file.
//------------------------------------------------------------------------------
HRESULT IndexFileByFrames(__in LPWSTR wszTargetFile)
{
    HRESULT hr;
    IWMIndexer *pIndexer;

    hr = WMCreateIndexer(&pIndexer);
    if (SUCCEEDED(hr))
    {
        // Get an IWMIndexer2 interface to configure for frame indexing
        CComQIPtr<IWMIndexer2, &IID_IWMIndexer2> pIndexer2(pIndexer);

        if(!pIndexer2) 
        {
            _tprintf(_T("CopyASF: Failed to QI for IWMIndexer2!  hr=0x%x\n"), hr);
            return hr;
        }

        // Configure for frame-based indexing
        WORD wIndexType = WMT_IT_NEAREST_CLEAN_POINT;

        hr = pIndexer2->Configure(0, WMT_IT_FRAME_NUMBERS, NULL,
                                  &wIndexType);
        if (SUCCEEDED(hr))
        {
            HANDLE hIndexEvent = CreateEvent( NULL, FALSE, FALSE, WMVCOPY_INDEX_EVENT );
            if ( NULL == hIndexEvent )
            {
                _tprintf(_T("Failed to create index event!\n"));
                return E_FAIL;
            }

            // Create and configure a callback object
            HRESULT hrIndex = S_OK;

            CIndexCallback callbackIndex;
            callbackIndex.hEvent = hIndexEvent;
            callbackIndex.phr    = &hrIndex;

            if (fVerbose)
                _tprintf(_T("\nStarting the frame indexing process.\n"));

            hr = pIndexer->StartIndexing(wszTargetFile, &callbackIndex, NULL);
            if (SUCCEEDED(hr))
            {
                // Wait for indexing operation to complete
                WaitForSingleObject( hIndexEvent, INFINITE );
                if ( FAILED( hrIndex ) )
                {
                    _tprintf(_T("Indexing Failed (hr=0x%08x)!\n"), hrIndex );
                    return hr;
                }
                else
                    _tprintf(_T("Frame indexing completed.\n"));
            }
            else
            {
                _tprintf(_T("StartIndexing failed (hr=0x%08x)!\n"), hr);
                return hr;
            }
        }
        else
        {
            _tprintf(_T("Failed to configure frame indexer! hr=0x%x\n"), hr);
            return hr;
        }
    }

    if (pIndexer)
        pIndexer->Release();

    return hr;
}

//------------------------------------------------------------------------------
// Name: CopyASF()
// Desc: Play the file through a DirectShow filter graph.
//------------------------------------------------------------------------------
HRESULT CopyASF(int argc, __in_ecount(argc) LPTSTR argv[])
{
    HRESULT hr;    
    WCHAR wszSourceFile[MAX_PATH];
    WCHAR wszTargetFile[MAX_PATH];
    DWORD dwRequestedProfile=0;
    DWORD dwRequestedVersion;
    int i = 1;

    //
    // Parse command line options
    //
    while(i < argc && (argv[i][0] == _T('-') || argv[i][0] == _T('/')))
    {
        // Verbose mode (to display profile descriptions and detailed output)
        if(lstrcmpi(argv[i] + 1, _T("v")) == 0)
        {
            fVerbose = TRUE;
            _tprintf(_T("Verbose mode enabled.\n"));
        }

        // List all profiles (all profile versions) and exit the loop
        else if(lstrcmpi(argv[i] + 1, _T("l")) == 0)
        {
            fListAllProfiles = TRUE;
            break;
        }

        // Select frame-based indexing (instead of the default
        // temporal [time-based] indexing)
        else if(lstrcmpi(argv[i] + 1, _T("f")) == 0)
        {
            fFrameIndexing = TRUE;
            _tprintf(_T("Requesting frame-based indexing.\n"));
        }

        else if(lstrcmpi(argv[i] + 1, _T("m")) == 0)
        {
            fMultipassEncode = TRUE;
            _tprintf(_T("Requesting multipass encoding.\n"));
        }

        // Select a system profile
        else if((i+1 < argc) && lstrcmpi(argv[i] + 1, _T("p")) == 0)
        {
            fListProfiles = FALSE;
            dwRequestedProfile = _ttoi(argv[i+1]);
            _tprintf(_T("Requesting profile #%d.\n"), dwRequestedProfile);
            i++;  // skip two args here
        } 

        // Select a system profile version
        else if((i+1 < argc) && lstrcmpi(argv[i] + 1, _T("n")) == 0)
        {
            dwRequestedVersion = _ttoi(argv[i+1]);

            // If this is a valid request, save the version number
            if (dwRequestedVersion == 4 || dwRequestedVersion == 7 ||
                dwRequestedVersion == 8)
            {
                _tprintf(_T("Requesting Windows Media System Profile version %d.\n"), 
                        dwRequestedVersion);
                g_WMTVersion = (WMT_VERSION) (dwRequestedVersion << 16);
            }
            else
            {
                _tprintf(_T("Invalid profile version.  Only 4, 7, and 8 are valid.\n"));
                _tprintf(_T("%s"), USAGE_STRING);
                return -1;
            }

            i++;  // skip two args here
        } 

        i++;
    }

    // List all system profiles (all supported versions)?
    if(fListAllProfiles)
    {
        _tprintf(_T("%s"), USAGE_STRING);
        ListAllProfiles();
        return -1;
    }

    // List system profiles only (then exit)?
    if(fListProfiles)
    {
        _tprintf(_T("%s"), USAGE_STRING);
        ListProfiles(g_WMTVersion);
        return -1;
    }

    // Fail with usage information if improper number of arguments
    if(argc < i+2)
    {
        _tprintf(_T("%s"), USAGE_STRING);
        return -1;
    }

    
    //
    // Command-line processing is complete.
    // Create the interfaces needed to copy and configure the ASF file.
    //

    CComPtr <IGraphBuilder>    pGraph;
    CComPtr <IFileSinkFilter>  pFileSink;
    CComPtr <IBaseFilter>      pASFWriter;
    CComPtr <IConfigAsfWriter> pConfigAsfWriter;
    CComPtr <IMediaControl>    pMC;

    // Convert target filename to a wide character string
#ifndef UNICODE
    MultiByteToWideChar(CP_ACP, 0, argv[argc - 1], -1, 
                        wszTargetFile, NUMELMS(wszTargetFile));
#else
    wcsncpy_s(wszTargetFile, MAX_PATH, argv[argc-1], NUMELMS(wszTargetFile));
#endif

    // Create an empty DirectShow filter graph
    hr = CreateFilterGraph(&pGraph);
    if(FAILED(hr))
    {
        _tprintf(_T("Couldn't create filter graph! hr=0x%x"), hr);
        return hr;
    }

    // Add the ASF Writer filter to the graph
    hr = CreateFilter(CLSID_WMAsfWriter, &pASFWriter);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to create WMAsfWriter filter!  hr=0x%x\n"), hr);
        return hr;
    }

    // Get a file sink filter interface from the ASF Writer filter
    hr = pASFWriter->QueryInterface(IID_IFileSinkFilter, (void **) &pFileSink);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to create QI IFileSinkFilter!  hr=0x%x\n"), hr);
        return hr;
    }

    // Set the target file name (from command line user input)
    hr = pFileSink->SetFileName(wszTargetFile, NULL);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to set target filename!  hr=0x%x\n"), hr);
        return hr;
    }

    // Add the MUX filter (ASF writer) to the graph
    hr = pGraph->AddFilter(pASFWriter, L"Mux");
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to add Mux filter to graph!  hr=0x%x\n"), hr);
        return hr;
    }

    //
    // Load and configure a Windows Media Profile
    //
    // We should only require a profile if we're using a filter which needs it
    hr = pASFWriter->QueryInterface(IID_IConfigAsfWriter, (void **) &pConfigAsfWriter);
    if(SUCCEEDED(hr))
    {
        if (fVerbose)
            _tprintf(_T("Setting profile to %d\r\n"), dwRequestedProfile);

        CComPtr<IWMProfile> pProfile;

        // Convert the numerical profile index into a IWMProfile interface pointer
        hr = MapProfileIdToProfile(dwRequestedProfile, &pProfile);
        if(FAILED(hr)) {
            _tprintf(_T("Failed to map profile ID!  hr=0x%x\n"), hr);
            return hr;
        }

        // Note that the ASF writer will not run if the number of streams
        // does not match the profile.
        hr = pConfigAsfWriter->ConfigureFilterUsingProfile(pProfile);
        if(FAILED(hr)) {
            _tprintf(_T("Failed to configure filter to use profile!  hr=0x%x\n"), hr);
            return hr;
        }       

        // If frame-based indexing was requested, disable the default
        // time-based (temporal) indexing
        if (fFrameIndexing)
        {
            hr = pConfigAsfWriter->SetIndexMode(FALSE);
            if(FAILED(hr)) {
                _tprintf(_T("Failed to disable time-based indexing!  hr=0x%x\n"), hr);
                return hr;
            }       
        }
    }
    else
    {
        _tprintf(_T("Failed to QI for IConfigAsfWriter!  hr=0x%x\n"), hr);
        return hr;
    }

    // Set sync source to NULL to speed processing
    SetNoClock(pGraph);

    _tprintf(_T("Using version %d system profiles.\n"), PROFILE_VERSION_NUM);

    // Enable multipass encoding if requested
    CComQIPtr<IConfigAsfWriter2,   &IID_IConfigAsfWriter2>   pConfigAsfWriter2(pASFWriter);
    CComQIPtr<IWMWriterPreprocess, &IID_IWMWriterPreprocess> pPreprocess(pASFWriter);

    if (fMultipassEncode)
    {
        // Verify that QIs were successful
        if(!pConfigAsfWriter2) 
        {
            _tprintf(_T("ListProfiles: Failed to QI IConfigAsfWriter2!  hr=0x%x\n"), hr);
            return E_FAIL;
        }
        if(!pPreprocess) 
        {
            _tprintf(_T("ListProfiles: Failed to QI IWMWriterPreprocess!  hr=0x%x\n"), hr);
            return E_FAIL;
        }

        // Enable multipass encoding
        hr = pConfigAsfWriter2->SetParam(AM_CONFIGASFWRITER_PARAM_MULTIPASS, TRUE, 0);
        if (FAILED(hr))
        {
            _tprintf(_T("Failed to enable multipass encoding param!  hr=0x%x\n"), hr);
            return hr;
        }

        // Read back for debugging purposes
        DWORD dwSetting;
        hr = pConfigAsfWriter2->GetParam(AM_CONFIGASFWRITER_PARAM_MULTIPASS, &dwSetting, 0);
        if (FAILED(hr))
        {
            _tprintf(_T("Failed to read multipass encoding param!  hr=0x%x\n"), hr);
            return hr;
        }
    }

    // Render all source files listed on the command line
    while(i < argc - 1)
    {
#ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0, argv[i], -1, wszSourceFile, NUMELMS(wszSourceFile));
#else
        wcsncpy_s(wszSourceFile, MAX_PATH, argv[i], NUMELMS(wszSourceFile));
#endif

        _tprintf(_T("\nCopying [%ls] to [%ls] with profile #%d\n"), 
                wszSourceFile, wszTargetFile, dwRequestedProfile);

        // Let DirectShow render the source file
        hr = pGraph->RenderFile(wszSourceFile, NULL);

        if(FAILED(hr))
        {
            _tprintf(_T("Failed to render source file %s!  hr=0x%x\n"), argv[i], hr);
            return hr;
        }
        else if (fVerbose)
            _tprintf(_T("RenderFile('%ls') returned hr=0x%x\n"), wszSourceFile, hr);

        ++i;
    }

    // Get the media control interface to control the graph
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to QI for IMediaControl!  hr=0x%x\n"), hr);
        return hr;
    }

    // Run the graph
    hr = pMC->Run();

    if(SUCCEEDED(hr))
    {
        // How many passes can we make?
        DWORD dwMaxPasses=1;

        hr = pPreprocess->GetMaxPreprocessingPasses(0, 0, &dwMaxPasses);
        if (SUCCEEDED(hr))
            _tprintf(_T("Max supported preprocessing passes: %d\n"), dwMaxPasses);


        // Wait for the copy operation to complete
        int nEvent = WaitForCompletion(pGraph);

        // Stop the graph
        hr = pMC->Stop();
        if (FAILED(hr))
            _tprintf(_T("Failed to stop filter graph!  hr=0x%x\n"), hr);
        
        if (fMultipassEncode && (nEvent != EC_PREPROCESS_COMPLETE))
        {
            _tprintf(_T("ERROR: Failed to recieve expected EC_PREPROCESSCOMPLETE.\n"));
            return E_FAIL;        
        }
                
        // If we're using multipass encode, run again
        if (fMultipassEncode)
        {
            _tprintf(_T("Preprocessing complete.\n"));

            // Seek to beginning of file for actual encoding pass
            CComQIPtr<IMediaSeeking, &IID_IMediaSeeking> pMS(pMC);
            if (pMS)
            {
                LONGLONG pos=0;

                hr = pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning ,
                                       NULL, AM_SEEKING_NoPositioning);

                // Now that preprocessing is done, write the file normally
                if (SUCCEEDED(hr))
                {
                    hr = pMC->Run();
                }
                if (SUCCEEDED(hr))
                {
                    WaitForCompletion(pGraph);           

                    hr = pMC->Stop();
                    if (FAILED(hr))
                        _tprintf(_T("Failed to stop filter graph after completion!  hr=0x%x\n"), hr);

                    _tprintf(_T("Copy complete.\n"));
                }
                else
                    _tprintf(_T("Failed to run the graph!  hr=0x%x\n"), hr);
            }
            else
            {
                _tprintf(_T("Failed to QI for IMediaSeeking!\n"));
            }
            
            // Turn off multipass encoding
            hr = pConfigAsfWriter2->SetParam(AM_CONFIGASFWRITER_PARAM_MULTIPASS, FALSE, 0);
            if (FAILED(hr))
            {
                _tprintf(_T("Failed to disable multipass encoding!  hr=0x%x\n"), hr);
                return hr;
            }
        }
    }
    else
    {
        _tprintf(_T("Failed to run the graph!  hr=0x%x\nCopy aborted.\n\n"), hr);
        _tprintf(_T("Please check that you have selected the correct profile for copying.\n")
                 _T("Note that if your source ASF file is audio-only, then selecting a\n")
                 _T("video profile will cause a failure when running the graph.\n\n"));
    }

    // If frame-based indexing was requested, this must be performed
    // manually after the file is created
    if (fFrameIndexing)
    {
        IndexFileByFrames(wszTargetFile);
    }

    return hr;
}


int __cdecl _tmain(int argc, __in_ecount(argc) LPTSTR argv[])
{
    USES_CONVERSION;

    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return hr;

    // Since COM smart pointers are used, the main functionality is wrapped
    // in CopyASF().  When the function returns, the smart pointers will clean
    // up properly, and then we'll uninitialize COM.
    hr = CopyASF(argc, argv);

    CoUninitialize();
    return hr;
}

