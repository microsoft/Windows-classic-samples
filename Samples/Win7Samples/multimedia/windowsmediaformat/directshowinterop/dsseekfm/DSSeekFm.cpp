//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DSSeekFm.cpp
//
// Abstract:            Windows Media / DirectShow sample code
//                      This sample shows how to seek by frame number when using
//                      DirectShow to play ASF files.
//
//*****************************************************************************

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include "control.h"
#include "evcode.h"
#include <vfwmsgs.h>

// Disable warning C4268, which is generated within <wmsdk.h>
#pragma warning(push)
#pragma warning(disable:4268)
#include <wmsdk.h>
#pragma warning(pop)

#include <wmsdkidl.h>
#include <atlbase.h>
#include <stdio.h>

#include <dshowasf.h>

// Function prototypes
HRESULT RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pFilter);
HRESULT FrameNumberToTime( IWMSyncReader * pSyncReader, WORD wFrameSeekStream, 
                           QWORD qwFrame, REFERENCE_TIME * prtFrameTime );
BOOL IsFrameSeekable( LPCWSTR wszFile, IWMSyncReader ** ppSyncReader, 
                      WORD *pwFrameSeekStream, QWORD * pqwTotalFrames );
void Msg(__in LPTSTR szFormat, ...);


//------------------------------------------------------------------------------
// Name: CreateFilterGraph()
// Desc: Create the DirectShow filter graph.
//
// pGraph: Receives a pointer to the IGraphBuilder interface.
//------------------------------------------------------------------------------
HRESULT CreateFilterGraph(IGraphBuilder **pGraph)
{
    HRESULT hr;

    // Create the Filter Graph Manager object.
    hr = CoCreateInstance(CLSID_FilterGraph, 
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
// Desc: Create a filter with the specified CLSID.
//
// clsid:   CLSID of the filter.
// pFilter: Receives a pointer to the filter's IBaseFilter interface.
//------------------------------------------------------------------------------
HRESULT CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{
    HRESULT hr;

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
// Name: WaitForCompletion()
// Desc: Desc: Waits for a media event that signifies completion or cancellation
//       of a task.
//
// pGraph: Pointer to the Filter Graph Manager's IGraphBuilder interface.
//------------------------------------------------------------------------------
LONG WaitForCompletion( IGraphBuilder *pGraph )
{
    HRESULT hr;
    LONG lEvCode = 0;
    IMediaEvent *pEvent;

    pGraph->QueryInterface(IID_IMediaEvent, (void **) &pEvent);

    do
    {
        MSG Message;

        // Pump queued Windows messages
        while(PeekMessage(&Message, NULL, 0, 0, TRUE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        // Wait 10ms for an event
        hr = pEvent->WaitForCompletion(10, &lEvCode);

    } while(lEvCode == 0);

    pEvent->Release();
    return lEvCode;
}

//------------------------------------------------------------------------------
// Name: IsFrameSeekable()
// Desc: Ascertains whether the file contains a stream that is seekable by frame.
//       If so, the method returns the stream number and the number of frames.
//
// wszFile:           Specifies the file name.
// ppSyncReader:      Receives a pointer to the synchronous reader's IWMSyncReader 
//                    interface.
// pwFrameSeekStream: Receives the stream number of the seekable stream.
// pqqTotalFrames:    Receives the number of frames in the stream.
//------------------------------------------------------------------------------
BOOL IsFrameSeekable( LPCWSTR wszFile, IWMSyncReader ** ppSyncReader, 
                      WORD *pwFrameSeekStream, QWORD *pqwTotalFrames )
{
    if( !ppSyncReader || !pwFrameSeekStream )
        return FALSE;

    HRESULT hr = E_FAIL;

    // Create a synchronous reader and load the specified file.
    hr = WMCreateSyncReader( 0, 0, ppSyncReader );
    if( FAILED( hr ) )
    {
        _tprintf(_T("Couldn't create the synchronous reader for ASF frame seeking! hr=0x%x\nPlayback aborted.\n\n"), hr);
    } 
    
    if( SUCCEEDED( hr ) )
    {
        // Open the source file.
        hr = (*ppSyncReader)->Open( wszFile );
        if( FAILED( hr ) )
        {
            _tprintf(_T("WMSDK Sync reader failed to open file! hr=0x%x\nPlayback aborted.\n\n"), hr);
        }
    }

    if( SUCCEEDED( hr ) )
    {
        CComPtr <IWMProfile> pProfile;

        hr = (*ppSyncReader)->QueryInterface( IID_IWMProfile, (void **) &pProfile );
        if( SUCCEEDED( hr ) ) 
        {

            // Loop through each stream and check the "NumberOfFrames" attribute in the 
            // ASF header for that stream number. This attribute is present if the stream
            // has been indexed by frame.

            DWORD cStreams;
            WORD wStreamNum;

            // Get the number of streams.
            hr = pProfile->GetStreamCount(&cStreams);
            if( SUCCEEDED( hr ) )
            {
                for( DWORD dw = 0; dw < cStreams; dw++ )
                {
                    CComPtr <IWMStreamConfig> pConfig;

                    hr = pProfile->GetStream( dw, &pConfig );
                    if( SUCCEEDED( hr ) )
                    {
                        // Get the stream number for this stream. 
                        hr = pConfig->GetStreamNumber( &wStreamNum );
                        if( SUCCEEDED( hr ) )
                        {
                            CComPtr <IWMHeaderInfo> pWMHI;

                            hr = (*ppSyncReader)->QueryInterface( IID_IWMHeaderInfo, (void **) &pWMHI );
                            if( SUCCEEDED( hr ) )
                            {
                                // Check whether the header contains a "NumberOfFrames" attribute 
                                // (g_wszWMNumberOfFrames) for this stream. If so, the value of the
                                // attribute is the number of frames in the stream.

                                WMT_ATTR_DATATYPE Type;
                                QWORD qwFrames;
                                WORD cbLength = sizeof(qwFrames);

                                hr = pWMHI->GetAttributeByName(&wStreamNum,
                                                               g_wszWMNumberOfFrames,
                                                               &Type,
                                                               (BYTE *) &qwFrames,
                                                               &cbLength);
                                if( SUCCEEDED( hr ) ) 
                                {
                                    // This stream is seekable by frame. Return the stream number
                                    // and the number of frames.
                                    *pwFrameSeekStream = wStreamNum;
                                    *pqwTotalFrames = qwFrames;
                                } 
                            }
                        }
                    }
                }
            }
        }
    }

    return ( S_OK == hr );
}

//------------------------------------------------------------------------------
// Name: FrameNumberToTime()
// Desc: Converts a frame number to a reference time (100-nanosecond units).
//
// pSyncReader:      Pointer to the synchronous reader's IWMSyncReader interface.
// wFrameSeekStream: Specifies the stream number. 
// qwFrame:          Specifies the frame number.
// prtFrameTime:     Receives the reference time.
//------------------------------------------------------------------------------
HRESULT FrameNumberToTime( IWMSyncReader * pSyncReader, WORD wFrameSeekStream, 
                           QWORD qwFrame, REFERENCE_TIME * prtFrameTime )
{
    if( !pSyncReader || !prtFrameTime )
        return E_POINTER;

    // Use the Windows Media Format SDK synchronous reader object to seek to
    // the specified frame. The reader object returns the presentation time
    // for the next sample.
    
    // Seek to the specified frame number.
    HRESULT hr = pSyncReader->SetRangeByFrame( wFrameSeekStream, qwFrame, 0 );
    if( FAILED( hr ) )
    {
        _tprintf(_T("SetRangeByFrameFailed... hr=0x%x\nPlayback aborted.\n\n"), hr);
    }
    else 
    {
        // Get the next sample and return the presentation time to the caller.
        INSSBuffer * pINSSBuffer;
        QWORD qwSampleTime, qwSampleDuration;
        DWORD dwFlags;
        WORD idStream;
        
        hr = pSyncReader->GetNextSample( 
                                       wFrameSeekStream,
                                       &pINSSBuffer,
                                       &qwSampleTime,
                                       &qwSampleDuration,
                                       &dwFlags,
                                       NULL,
                                       &idStream );
        if( SUCCEEDED( hr ) )
        {
            pINSSBuffer->Release();
            *prtFrameTime = qwSampleTime;                
        }
    }

    return hr;  
}
//------------------------------------------------------------------------------
// Name: RenderOutputPins()
// Desc: Renders every output pin on a specified filter. 
//
// pGB:     Pointer to the Filter Graph Manager.
// pFilter: Pointer to the filter.
//------------------------------------------------------------------------------
HRESULT RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pFilter)
{
    HRESULT         hr = S_OK;
    IEnumPins *     pEnumPin = NULL;
    IPin *          pConnectedPin = NULL, * pPin;
    PIN_DIRECTION   PinDirection;
    ULONG           ulFetched;

    // Enumerate all the pins on the filter.
    hr = pFilter->EnumPins( &pEnumPin );

    if(SUCCEEDED(hr))
    {
        // Step through every pin, looking for the output pins.
        while (S_OK == (hr = pEnumPin->Next( 1L, &pPin, &ulFetched)))
        {
            // Check whether this pin is already connected. If so, ignore it.
            hr = pPin->ConnectedTo(&pConnectedPin);
            if (pConnectedPin)
            {
                // Release the IPin interface on the connected pin.
                pConnectedPin->Release();
                pConnectedPin = NULL;
            }

            if (VFW_E_NOT_CONNECTED == hr)
            {
                // This pin is not connected to another filter yet. Check 
                // whether it is an output pin. If so, use the Filter Graph
                // Manager's Render() method to render the pin.

                hr = pPin->QueryDirection( &PinDirection );
                if ( ( S_OK == hr ) && ( PinDirection == PINDIR_OUTPUT ) )
                {
                    hr = pGB->Render(pPin);
                }
            }
            pPin->Release();

            // If there was an error, stop enumerating.
            if (FAILED(hr))                      
                break;
        }
    }

    // Release the pin enumerator object.
    pEnumPin->Release();
    return hr;
}

//------------------------------------------------------------------------------
// Name: SeekASF()
// Desc: Parse the command line and play the specified file, 
//       with optional seeking.
//------------------------------------------------------------------------------
HRESULT SeekASF(int argc, __in_ecount(argc) LPSTR argv[])
{
    HRESULT hr;    
    WCHAR wszFile[256];
    int i = 1;
    int iFrameStart = 0;
    int iFramesToPlay = -1;  // Default to an invalid value, to force user setting.
    int iFrameIncForLoopSeek = 0;
    BOOL fSeekByFrame = FALSE;
    BOOL fLoopSeekToEnd = FALSE;
    BOOL bAbortWithUsage = FALSE;
    QWORD qwTotalFrames = 0;

    // Parse the command line options
    while(i < argc && (argv[i][0] == '-' || argv[i][0] == '/'))
    {
        // Options
        if((i+3 <= argc) && lstrcmpiA(argv[i] + 1, "f") == 0)
        {
            fSeekByFrame = TRUE;
            iFrameStart   = atoi(argv[i+1]);
            if (isalnum(*argv[i+2]))
                iFramesToPlay = atoi(argv[i+2]);

            if( iFrameStart < 1 )
            {
                _tprintf(_T("Error: Start frame must be >= 1! \n"));
                bAbortWithUsage = TRUE;
                break;
            }
            if( iFramesToPlay < 0 )
            {
                _tprintf(_T("Error: Number of frames to play must be >= 0! \n"));
                bAbortWithUsage = TRUE;
                break;
            }

            // Skip 3 arguments here
            i += 2;
        } 
        else if((i+1 < argc) && lstrcmpiA(argv[i] + 1, "l") == 0)
        {
            fLoopSeekToEnd = TRUE;
            iFrameIncForLoopSeek = atoi(argv[i+1]);

            // Skip 2 arguments here
            i++;
        } 
        i++;
    }

    // Fail with usage information if improper number of arguments
    if(argc < i+1 || ( fLoopSeekToEnd && !fSeekByFrame ) || bAbortWithUsage ||
       (fSeekByFrame && iFramesToPlay < 0))
    {
        if( fLoopSeekToEnd && !fSeekByFrame )
        {
            _tprintf(_T("Error: /l option requires /f option\n\n"));
        }
        _tprintf(_T("Usage: DSSeekFm [/f FrameStart NumFramesToPlay] [/l LoopModeIncrement] Filename\n\n"));
        _tprintf(_T("       FrameStart: Frame on which to begin playback\n"));
        _tprintf(_T("       LoopModeIncrement: Adjust start point by this many frames when looping\n"));
        _tprintf(_T("       If NumFramesToPlay is 0, file will play to completion.\n\n"));
        _tprintf(_T("Example: DSSeekFm /f 100 50 frameseek.wmv\n"));
        _tprintf(_T("       This will seek to frame 100 and play 50 frames.\n\n"));
        _tprintf(_T("Definition of 'Loop Mode':\n"));
        _tprintf(_T("       Plays segments of length <NumFramesToPlay>, \n"));
        _tprintf(_T("       incrementing <FrameStart> by <LoopModeIncrement> until EOF.\n\n"));
        _tprintf(_T("Example: DSSeekFm /f 100 10 /l 50 frameseek.wmv\n"));
        _tprintf(_T("       This will seek to frame 100, play 10 frames, advance 50 frames,\n"));
        _tprintf(_T("       play 10 more frames, advance 50 frames, etc. until completion.\n"));
        return -1;
    }

    CComPtr <IGraphBuilder>     pGraph;   // Filter Graph Manager
    CComPtr <IBaseFilter>       pReader;  // ASF reader filter
    CComPtr <IFileSourceFilter> pFS;      // Sets the file name on the ASF readewr.
    CComPtr <IMediaControl>     pMC;      // Controls filter graph playback.
    CComPtr <IMediaSeeking>     pMS;      // Seeks the filter graph.

    // Convert the file name to Unicode.
    if(0 == MultiByteToWideChar(CP_ACP, 0, argv[argc - 1], -1, wszFile, 256))
    {
        _tprintf(_T("Couldn't convert file name to Unicode!\n"));
        return E_FAIL;
    }

    // Create the DirectShow Filter Graph Manager. 
    hr = CreateFilterGraph(&pGraph);
    if(FAILED(hr))
    {
        _tprintf(_T("Couldn't create filter graph! hr=0x%x\n"), hr);
        return hr;
    }

    // Create the ASF reader filter and add it to the Filter Graph.
    hr = CreateFilter(CLSID_WMAsfReader, &pReader);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to create WMAsfReader filter!  hr=0x%x\n"), hr);
        return hr;
    }

    hr = pGraph->AddFilter(pReader, L"ASF Reader");
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to add ASF Reader filter to graph!  hr=0x%x\n"), hr);
        return hr;
    }

    // Get the IFileSourceFilter interface, which is used to set the source file name.
    hr = pReader->QueryInterface(IID_IFileSourceFilter, (void **) &pFS);
    if(FAILED(hr))
    {
        _tprintf(_T("QI for IFileSourceFilter failed!  hr=0x%x\n"), hr);
        return hr;
    }
    
    // Load the source file.
    hr = pFS->Load( wszFile, NULL );
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to load file!  hr=0x%x\n"), hr);
        _tprintf(_T("Make sure this is an ASF file!!\n"));
        return hr;
    }
    else
    {
        // Render the output pins of the ASF reader to build the remainder of the
        // filter graph automatically. This step connects the ASF reader filter to
        // the video and audio renderer filters.
        hr = RenderOutputPins(pGraph, pReader);
        if( FAILED( hr ) )
        {
            _tprintf(_T("Failed to render output pins!  hr=0x%x\n"), hr);
            return hr;
        }
    }
    
    IWMSyncReader * pSyncReader = NULL;
    WORD wFrameSeekStream = 0;

    // Check whether the ASF file can be seeked by frame number. If so, this method
    // call returns the stream number and the total number of frames in the stream.
    if( !IsFrameSeekable( wszFile, &pSyncReader, &wFrameSeekStream, &qwTotalFrames ) 
        && fSeekByFrame)
    {
        _tprintf(_T("The selected ASF file isn't frame seekable.\n"));
        return E_FAIL;
    }
    if( qwTotalFrames > 0 )
    {
        _tprintf(_T("Total Frames in ASF file = %d\n"), (LONG) qwTotalFrames );
    }

    // Prepare to run the graph. Query for the necessary interfaces.
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **) &pMC);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to QI for IMediaControl!  hr=0x%x\n"), hr);
        return hr;
    }

    hr = pGraph->QueryInterface(IID_IMediaSeeking, (void **) &pMS);
    if(FAILED(hr))
    {
        _tprintf(_T("Failed to QI for IMediaSeeking!  hr=0x%x\n"), hr);
        return hr;
    }

    REFERENCE_TIME rtStart = 0;
    REFERENCE_TIME rtStop = 0;
    LONG lCompletionEvent;

    if( fSeekByFrame && iFrameStart > 0 )
    {
        // The WM ASF Reader filter does not directly support frame seeking. Therefore, we use
        // the Windows Media Format SDK to create a separate instance of the synchronous 
        // reader object. We seek the reader object to the desired frame number and retrieve the 
        // presentation time. Then we use that time vaue to seek the DirectShow filter graph.

        do
        {
            if( fSeekByFrame && iFrameStart > 0 )
            {
                // Calculate the frame at which to stop.
                int iFrameStop = iFrameStart + iFramesToPlay;

                // Check if we're trying to seek past the end of the file.
                if( ( iFrameStart > (int)qwTotalFrames ) ||
                    ( iFrameStop >  (int)qwTotalFrames ) )
                {
                    _tprintf(_T("Playback finished: Attempting to seek past last frame number!\n")
                             _T("         (requested: start %d, stop %d, total frames: %d \n"),  
                            iFrameStart, iFrameStop, (LONG)qwTotalFrames);
                    hr = S_OK;
                    break;
                }

                // Convert the requested frame number to a reference time (100-nanosecond units).

                hr = FrameNumberToTime( pSyncReader, wFrameSeekStream, 
                                        (QWORD) iFrameStart, &rtStart );
                if(FAILED(hr))
                {
                    _tprintf(_T("Failed to convert start frame number to frame time!  hr=0x%x\n"), hr);
                }
                else if( 0 < iFramesToPlay ) // If iFramesToPlay == 0, it means to play to the end of the file.
                {
                    // Convert the stop frame number to a reference time.
                    hr = FrameNumberToTime( pSyncReader, wFrameSeekStream, 
                                            (WORD) iFrameStop, &rtStop );
                    if(FAILED(hr))
                    {
                        _tprintf(_T("Failed to convert stop frame number to frame time!  hr=0x%x\n"), hr);
                    }       
                }

                // Perform the seek.
                if( SUCCEEDED( hr ) )
                {
                    if (iFramesToPlay > 0)
                        _tprintf(_T("Playing from frame #%d to frame #%d.\n"), iFrameStart, iFrameStop);
                    else
                        _tprintf(_T("Playing from frame #%d to completion.\n"), iFrameStart);

                    // Seek the graph to the calculated reference time.
                    hr = pMS->SetPositions( &rtStart, AM_SEEKING_AbsolutePositioning,
                                            &rtStop,  (iFramesToPlay > 0 ) ? AM_SEEKING_AbsolutePositioning : 0 );
                    if(FAILED(hr))
                    {
                        _tprintf(_T("Failed seek (rtStart = %dms, rtStop = %dms)!  hr=0x%x\n"), 
                                (LONG) (rtStart/10000), (LONG)(rtStop/10000), hr);
                    }       
                }

                // Begin playback of the filter graph.
                if( SUCCEEDED( hr ) )
                {
                    hr = pMC->Run();
                    if(FAILED(hr))
                    {       
                        _tprintf(_T("Failed to run the graph!  hr=0x%x\nPlayback aborted.\n\n"), hr);
                    }
                    else
                    {
                        // Wait for the graph to finish playing (or the user to abort).
                        if (!fLoopSeekToEnd)
                            _tprintf(_T("Waiting for completion...\n"));

                        lCompletionEvent = WaitForCompletion(pGraph);

                        if( EC_USERABORT  == lCompletionEvent || 
                            EC_ERRORABORT == lCompletionEvent )
                        {
                            _tprintf(_T("Playback aborted.\n"));
                            break;
                        }

                        if (!fLoopSeekToEnd)
                            _tprintf(_T("Playback complete.\n"));
                    }

                }
            }

            // If we are looping, calculate the next starting frame.
            iFrameStart += iFrameIncForLoopSeek; 

        } while( fLoopSeekToEnd && SUCCEEDED( hr ) );
        
        // Stop the graph.
        hr = pMC->Stop();
    }

    // Seeking was not requested, so just play the graph normally.
    else
    {
        _tprintf(_T("Seeking was not requested. Playing graph to completion.\n"));

        hr = pMC->Run();
        if(FAILED(hr))
        {       
            _tprintf(_T("Failed to run the graph!  hr=0x%x\nPlayback aborted.\n\n"), hr);
        }
        else
        {
            lCompletionEvent = WaitForCompletion(pGraph);
            if( EC_USERABORT  == lCompletionEvent || 
                EC_ERRORABORT == lCompletionEvent )
            {
                _tprintf(_T("Playback aborted.\n"));
            }
            else
                _tprintf(_T("Playback complete.\n"));
    
            // Stop the graph
            hr = pMC->Stop();
        }
    }

    return hr;
}

//------------------------------------------------------------------------------
// Name: main()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int __cdecl
main(
    int argc,
    __in_ecount(argc) LPSTR argv[]
    )
{
    // Initialize COM.
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // Since COM smart pointers are used, the main functionality is wrapped
    // in CopyASF().  When the function returns, the smart pointers will clean
    // up properly, and then we'll uninitialize COM.
    hr = SeekASF(argc, argv);

    CoUninitialize();
    return hr;
}
